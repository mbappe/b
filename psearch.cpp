
//
// Notes and code for thinking about parallel searching.
//

// Is it better to use a different packing/padding strategy for
// power-of-two-size keys than for non-power-of-two-size keys?
// Power-of-two doesn't need to worry about non-key bits when trying
// to determine if a key is present or when trying to find the position
// of a key or a slot where a key would be.

// For power-of-two-size keys:
// Pack low; sort from low; 
//
// Ideally we'd avoid extra shift for non-power-of-2-size keys during
// WordHasKey by packing keys at high end.
// Ideally we'd use ctz since it is a direct map of BSF. It means counting
// from the low end.
// Fastest is probably keys packed at high end with highest key at high end
// and 
// __builtin_ffs has to do an expensive test for zero internally since BSF
// does not do one.
// HasKeyLo has to do an expensive mod and shift.

#include <stdlib.h>
#include <stdio.h>
#include <typeinfo>
#include <sstream>
#include <iostream>
#include "emmintrin.h" // SSE2
#include "smmintrin.h"
#if defined(__AVX__)
#include "immintrin.h" // mm256_set1_epi64x
#endif // defined(__AVX__)

#if defined(__SSE2__)
template <typename T>
std::string __m128i_toString(const __m128i var) {
    std::stringstream sstr;
    const T* values = (const T*)&var;
    if (sizeof(T) == 1) {
        for (unsigned int i = 0; i < sizeof(__m128i); i++) {
            sstr << (int)values[i] << " ";
        }
    } else {
        for (unsigned int i = 0; i < sizeof(__m128i) / sizeof(T); i++) {
            sstr << values[i] << " ";
        }
    }
    return sstr.str();
}
#endif

#if defined(__AVX__)
template <typename T>
std::string __m256i_toString(const __m256i var) {
    std::stringstream sstr;
    const T* values = (const T*)&var;
//#if 0
    if (sizeof(T) == 1) {
        for (unsigned int i = 0; i < sizeof(__m256i); i++) {
            sstr << (int)values[i] << " ";
        }
    } else
//#endif // 0
    {
        for (unsigned int i = 0; i < sizeof(__m256i) / sizeof(T); i++) {
            sstr << values[i] << " ";
        }
    }
    return sstr.str();
}
#endif // defined(__AVX__)

#include <Judy.h>

#if defined(_WIN64)
#define WORD_ONE  1ULL
#else // defined(_WIN64)
#define WORD_ONE  1UL
#endif // defined(_WIN64)

#define EXP(_x)  (WORD_ONE << (_x))
#define MSK(_x)  (EXP(_x) - 1)

#define MSK(_x)  (EXP(_x) - 1)

// There are a lot of ways we can represent a one-word bucket for parallel
// search.
//
// The biggest issue with parallel search is locating where a missing
// key belongs.
// Locating a present key can be fast.
// Is there any way to leverage parallel search for locating where a missing
// key belongs?
// Or would it be best if insert simply assumes the key is not present and
// doesn't bother with a parallel search?
//
// empty slots are on most-significant end of the word
// empty slots are on least-significant end of the word
// empty slots are anywhere; can't imagine this helping search
//
// remainder bits are at the most-significant end of the word
// remainder bits are at the least-significant end of the word
//
// use fixed number of bits for pop count
// use variable number of bits for pop count
// calculate pop count
//
// fill empty slots with 0
// fill empty slots with smallest key
// fill empty slots with largest key
// fill empty slots with -1
//
// sort with most-significant non-empty slot having the smallest key
// sort with least-significant slot having the smallest key
// don't sort; couldn't possibly help search
//
// If keys are sorted then there are a lot of bucket values that are illegal
// and could be used to have other meanings.
// It only requires that buckets can always hold more than one key,
// i.e. key_size <= word_size / 2.
//
// The value(s) we can use to represent an empty bucket depend
// on the choices made for the others.
//
// Which way will be fastest?
// Should we require that the keys in the list be sorted?  Why?
// Is has-key faster if we fill empty slots with a present key?
// What about offset?  Assumes key is present.
// What about has-key-and-offset?  No offset returned if key is not present.
// What about offset-of-missing-key?  Assumes key is not present.
// What about has-key-or-offset-of-missing-key?  No off ret if key is present.
// What about has-key-and-offset-or-offset-of-missing-key?

// Do a parallel search of a word for a key that is no bigger than half a word.
// WordHasKey expects the keys to be packed towards the most significant bits.
// It assumes all slots in the word have valid keys, i.e. the would-be empty
// slots have been padded with copies of some key/keys that is/are present.
//
// Key observations about HasKey:
// 1. HasKey creates a magic number with the high bit set in the key slots
//    that match the target key.
// 2. It also sets the high bit in the key slot to the left of any other
//    slot with its high bit set if the key in that slot is one less than
//    the target key. Uh oh. Not always. An extra bit is not set for even
//    4-bit keys tested. Same goes for even 8-bit keys tested. That is the
//    extent of my testing.
// Observation (2) means sorting from left to right or filling with zeros
// or the lowest key on the left may result in multiple bits set.
//
// Doug's observation:
// If searching for 0xfe and the the next more significant slot contains
// 0xff, then the high bit of the 0xff is also set. As is the high bit of
// the next more significant slot if it contains 0xff and so on.
// psearch64 0xfffffffe03000000 8 0xfe gives multiple bits set.
//
// My analysis:
//
// Consider the case of no matching slots.
// For each slot:
// Since slot does not match, then (Key^Slot) is not zero.
// So subtracting one from (Key^Slot) does not borrow from the next slot.
// Also, subtracting one from (Key^Slot) doesn't set an otherwise clear msb.
// If the msb is set in (Key^Slot), then the msb is not set in ~(Key^Slot).
// So anding the difference with ~(Key^Slot) yields a result with msb clear.
// Then anding that result with a value that has only the msb set yields zero.
//
// QED
//
// Now consider the case of at least one matching slot.
// (Keys^Slots) will be zero in each of the matching slots.
// For all slots less signficant than the least significant matching slot the
// result is the same as for the no matching slots case considered above.
// Starting with the least significant matching slot we classify the
// possibilities for the next more significant slot into four cases.
// 1.  (Key&1) && (Slot == Key-1); diff is -1 and borrows from next slot.
// 2. !(Key&1) && (Slot == Key+1); diff is -1 and borrows from next slot.
// 3.             (Slot == Key  ); diff is -2 and borrows from next slot.
// 4. Everything else; does not borrow from the next slot.
// Then for cases 1-3 we have the same four possibilities for the next more
// significant slot.
// And so on.
//
// In the end, the least significant slot with its msb set represents
// (Slot == Key). And more significant slots with their msb set represent
// (Slot == Key) or (Key&1) && (Slot == Key-1) or !(Key&1) && (Slot == Key+1).
//
// In other words.
// In the end, the least significant slot with its msb set represents
// (Slot == Key) and more significant slots with their msb set represent
// (Slot == Key) or (Slot^Key == 1).
//
// In other words.
// In the end, the least significant slot with its msb set represents
// (Slot == Key) and more significant slots with their msb set represent
// ((unsigned)(Slot^Key) <= 1)
//
// In other words.
// In the end, slots with their msb set represent ((unsigned)(Slot^Key) <= 1).
// And the least significant slot with its msb set represents (Slot == Key).
//
// QED
//
// Can we say anything else?
//
// What about matching slots other than the least significant matching one?
//
// Any slot with its msb set and the adjacent less significant slot with its
// msb clear represents (Slot == Key).
//
// Any slot with its msb set represents (abs(Slot-Key) == 1) and
// the adjacent less significant slot has its msb set) or ((Slot == Key)
// and the adjacent less significant slot has its msb clear).
//
// There must be a more elegant way to summarize.
//

static Word_t // bool
WordHasKeyHi(Word_t *pw, Word_t wKey, unsigned nBL)
{
printf("WordHasKeyHi\n");
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    Word_t ww = *pw;
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
//printf("wMask 0x%lx\n", wMask);
    wKey &= wMask; // get rid of already-decoded bits
//printf("wKey 0x%lx\n", wKey);
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
printf("wLsbs   0x%016lx\n", wLsbs);
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
//printf("wKeys   0x%016lx\n", wKeys);
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
//printf("wMsbs   0x%016lx\n", wMsbs);
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
//printf("wXor    0x%016lx\n", wXor);
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
printf("wMagic  0x%016lx\n", wMagic); 
    return wMagic; // bXorHasZero = (wMagic != 0);
}

static Word_t // bool
WordHasKeyLo(Word_t *pw, Word_t wKey, unsigned nBL)
{
printf("WordHasKeyLo\n");
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    Word_t ww = *pw;
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
//printf("wMask 0x%lx\n", wMask);
    wKey &= wMask; // get rid of already-decoded bits
//printf("wKey 0x%lx\n", wKey);
    Word_t wQuotient = (Word_t)-1 / wMask; // lsb in each key slot
printf("wQuotient   0x%016lx\n", wQuotient);
    int nMod = 64 % nBL; // extra bits
//printf("nMod %d\n", 64 % nBL);
    Word_t wLsbs = (Word_t)-1 / wMask >> nMod; // lsb in each key slot
printf("wLsbs   0x%016lx\n", wLsbs);
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
//printf("wKeys   0x%016lx\n", wKeys);
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
//printf("wMsbs   0x%016lx\n", wMsbs);
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
//printf("wXor    0x%016lx\n", wXor);
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
printf("wMagic  0x%016lx\n", wMagic); 
    return wMagic; // bXorHasZero = (wMagic != 0);
}


// Intel BSF     finds   bit number          of lsb set; undefined for 0.
// __builtin_ffs returns bit number plus one of lsb set;         0 for 0.
// __builtin_ffs(0)      = 0
// __builtin_ffs(1<< 0)  = 1
// __builtin_ffs(1<<63) = 64

// Intel BSF     finds   bit number of lsb set; undefined for 0.
// __builtin_ctz returns bit number of lsb set; undefined for 0.
// __builtin_ctz(0)     =  0 but undefined
// __builtin_ctz(1<< 0) =  0
// __builtin_ctz(1<<63) = 63

// Intel BSR     finds   bit number of msb set; undefined for 0
// __builtin_clz returns bit number of msb set; undefined for 0.
// __builtin_clz(0)     = 63 but undefined
// __builtin_clz(1<< 0) = 63
// __builtin_clz(1<<63) =  0

// LocateKeyHi assumes keys are sorted with low key at the high end.
// It returns key position relative to the lowest key.
// Can't use clz here w/o first clearing any high bits by
// 2's complement and and because of the observation that
// we could get higher bits set than we want.
// But we also have a problem with lower bits being set
// if the list is not full and it is padded with the key we're
// looking for.
static int // bool
LocateKeyHi(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyHi(pw, wKey, nBL);
    if (wMagic) {
        Word_t w = wMagic & -wMagic;
        return __builtin_clzll(w) / nBL;
    } else {
        return -1;
    }
}

// LocateKeyHiAlt assumes keys are sorted with low key at the high end.
// It returns key position relative to the lowest key.
// Can't use ctz unless word is padded with lowest present key.
static int // bool
LocateKeyHiAlt(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyHi(pw, wKey, nBL);
    if (wMagic) {
        return (63 - __builtin_ctzll(wMagic)) / nBL;
    } else {
        return -1;
    }
}

// LocateKeyHiR assumes keys are sorted with high key at the high end.
// It returns key position relative to the high key.
static int // bool
LocateKeyHiR(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyHi(pw, wKey, nBL);
    if (wMagic) {
        return __builtin_clzll(wMagic) / nBL;
    } else {
        return -1;
    }
}

// LocateKey2Lo assumes power-of-two-size keys are sorted with low key at
// the low end.
// It returns key position relative to the low key.
static int // bool
LocateKey2Lo(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyLo(pw, wKey, nBL);
    if (wMagic) {
        return __builtin_ctzll(wMagic) / nBL;
    } else {
        return -1;
    }
}

// LocateKeyLo assumes keys are sorted with low key at the low end.
// It returns key position relative to the low key.
static int // bool
LocateKeyLo(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyLo(pw, wKey, nBL);
    if (wMagic) {
        return __builtin_ctzll(wMagic) / nBL;
    } else {
        return -1;
    }
}

// LocateKeyLoR assumes keys are sorted with low key at the low end.
// Can't use clz here w/o first clearing any high bits by
// 2's complement and and.
static int // bool
LocateKeyLoR(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKeyLo(pw, wKey, nBL);
    if (wMagic) {
        // ffs can locate key with either direction of sort
        // clz may be wrong if small keys are in high-order slots
printf("(ffs - 1) / nBL = %d\n", (__builtin_ffsll(wMagic) - 1) / nBL);
printf("ctz / nBL = %d\n", __builtin_ctzll(wMagic) / nBL);
        Word_t w = wMagic & -wMagic;
printf("w       0x%016lx\n", w); 
printf("63 - clz / nBL = %d\n", (63 - __builtin_clzll(w)) / nBL);
        return __builtin_ctzll(wMagic) / nBL;
    } else {
        return -1;
    }
}

#if 0
static Word_t // bool
LocateHole(Word_t *pw, Word_t wKey, unsigned nBL)
{
}

static Word_t // bool
LocateKeyInWord(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wMagic = WordHasKey(&wBucket, wKey, nBL); 
    if (wMagic) {
        int nKeySlot = __builtin_ffsll(wMagic) / nBL;
    } else {
    }
}
#endif

// A Bucket_t contains a power of two Word_ts.
// A Word_t contains an integral number of Key_ts.
// nBits doesn't have to be a power of two so
// there may be left over bits in Word_t.
// bPackLow means the keys are packed at the low
// end of the word.
template<typename Word_t>
static inline void
SearchPreSetupWord(int nBits, int bPackLow, Word_t& rwLsbs, Word_t& rwMsbs)
{
    Word_t wMask = ((Word_t)1 << nBits) - 1;
    rwLsbs = (Word_t)-1 / wMask;
    if (bPackLow) {
        int nMod = sizeof(Word_t) * 8 % nBits;
        rwLsbs >>= nMod;
    }
    rwMsbs = rwLsbs << (nBits - 1);
}

template<typename Word_t>
static inline void
SearchSetupWord(Word_t wKey, int nBits, int bKeyAlreadyMasked,
                int bPackLow, Word_t& rwLsbs, Word_t& rwMsbs, Word_t& rwKeys)
{
    SearchPreSetupWord(nBits, bPackLow, rwLsbs, rwMsbs);
    if (!bKeyAlreadyMasked) {
        Word_t wMask = ((Word_t)1 << nBits) - 1;
        wKey &= wMask;
    }
    rwKeys = wKey * rwLsbs; // replicate key; put in every slot
}

#if defined(__AVX__)
static inline __m256i
Mm256Set1Epi64x(uint64_t u64Src)
{
    return _mm256_set1_epi64x(u64Src);
}
#endif // defined(__AVX__)

template<typename Bucket_t>
static inline Bucket_t
MmSet1(__m64 mSrc)
{
    if (typeid(Bucket_t) == typeid(__m128i)) {
        return _mm_set1_epi64(mSrc);
#if 0
    } else if (typeid(Bucket_t) == typeid(__m256i)) {
        return _mm256_set1_epi64x(mSrc);
#endif
    }
    Bucket_t x; return x;
}

template<typename Bucket_t, typename Word_t>
static inline void
SearchSetupBucket(Word_t wKey, int nBits, int bKeyAlreadyMasked, int bPackLow,
                  Bucket_t& rxLsbs, Bucket_t& rxMsbs, Bucket_t& rxKeys)
{
    Word_t wLsbs, wMsbs, wKeys;
    SearchSetupWord(wKey, nBits, bKeyAlreadyMasked, bPackLow,
                     wLsbs, wMsbs, wKeys);
    Bucket_t xLsbs = MmSet1<Bucket_t>((__m64)wLsbs);
    Bucket_t xMsbs = MmSet1<Bucket_t>((__m64)wMsbs);
    Bucket_t xKeys = MmSet1<Bucket_t>((__m64)wKeys);
}

template<typename Word_t>
static inline Word_t
SearchWord(Word_t wBucket, Word_t wKey, int nBits, int bKeyAlreadyMasked,
           int bPackLow)
{
    Word_t wLsbs, wMsbs, wKeys;
    SearchSetupWord(wKey, nBits, bKeyAlreadyMasked, bPackLow,
                   wLsbs, wMsbs, wKeys);
    Word_t wXor = wKeys ^ wBucket; // get zero in slots with matching key
    return (wXor - wLsbs) & ~wXor & wMsbs; // wMatches
}

template<typename Bucket_t, typename Word_t>
static inline Bucket_t
SearchBucket(Bucket_t xBucket, Word_t wKey, int nBits, int bKeyAlreadyMasked,
             int bPackLow)
{
    Bucket_t xLsbs, xMsbs, xKeys;
    SearchSetupBucket(wKey, nBits, bKeyAlreadyMasked, bPackLow,
                      xLsbs, xMsbs, xKeys);
    Bucket_t xXor = xKeys ^ xBucket; // get zero in slots with matching key
    return (xXor - xLsbs) & ~xXor & xMsbs; // xMatches
}

template<typename Word_t>
static inline int
HasKeyWord(Word_t wBucket, Word_t wKey, int nBits,
           int bKeyOk, int bPackLow)
{
    return SearchWord(wBucket, wKey, nBits, bKeyOk, bPackLow) != 0;
}

template<typename Bucket_t, typename Word_t>
static inline int
HasKeyBucket(Bucket_t xBucket, Word_t wKey, int nBits,
           int bKeyOk, int bPackLow)
{
    __m128i xMatches = SearchBucket(xBucket, wKey, nBits, bKeyOk, bPackLow);
    __m128i xZero = _mm_setzero_si128(); // get zero for compare
    return ! _mm_testc_si128(xZero, xMatches); // compare with zero

}

#if 0

Pretemplate<typename Bucket_t, typename Key_t>
static inline void
SearchPreSetup(Key_t k, int nBits,
            Bucket_t& rxLsbs, Bucket_t& rxMsbs Bucket_t& rxKeys)
{
    uint64_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask;
    Bucket_t rbLsbs = (Bucket_t)-1 / wMask; // lsb in each key slot
    Bucket_t rbMsbs = rbLsbs << (nBL - 1); // msb in each key slot
    Bucket_t rbKeys = wKey * rbLsbs; // replicate key; put in every slot
}

template<typename Bucket_t, typename Key_t>
static inline Bucket_t
BucketHasKey(Bucket_t& rb, Key_t k, int nBits) 
{
    Bucket_t bLsbs;
    Bucket_t bMsbs;
    Bucket_t bKeys;
    SearchSetup(k, nBits, bLsbs, bMsbs, bKeys

    Bucket_t bXor = bKeys ^ rb; // get zero in slot with matching key
    Bucket_t bMatches = (bXor - bLsbs) & ~wXor & wMsbs;

    return rb;
}
#endif

#if 0
Bucket<bytes-log2>HasKey<bits>
Bucket<bytes-log2>FindKey<bits>
Bucket<bytes-log2>FindSlot<bits>
#endif

// Usage: psearch64 <# of bits> <key> <bucket> [ <bucket>, ... ]
int
main(int argc, char **argv)
{
    if (argc != 4) {
        printf("usage: %s <# of bits> <key> <bucket>\n", argv[0]);
        exit(1);
    }

    //Word_t w = 0;
    //printf("ffs(%lx) = %d\n", w, __builtin_ffsll(w));
    //printf("ctz(%lx) = %d\n", w, __builtin_ctzll(w));
    //printf("clz(%lx) = %d\n", w, __builtin_clzll(w));
    //w = 1;
    //printf("ffs(%lx) = %d\n", w, __builtin_ffsll(w));
    //printf("ctz(%lx) = %d\n", w, __builtin_ctzll(w));
    //printf("clz(%lx) = %d\n", w, __builtin_clzll(w));
    //w = 1UL<<63;
    //printf("ffs(%lx) = %d\n", w, __builtin_ffsll(w));
    //printf("ctz(%lx) = %d\n", w, __builtin_ctzll(w));
    //printf("clz(%lx) = %d\n", w, __builtin_clzll(w));
    unsigned nBL = strtoul(argv[1], NULL, 0); printf("nBL %d\n", nBL);
    Word_t wKey = strtoul(argv[2], NULL, 0); printf("wKey 0x%lx\n", wKey);
    Word_t wBucket = strtoul(argv[3], NULL, 0);
    printf("wBucket 0x%016lx\n", wBucket);
    printf("LocateKeyHi  %d\n", LocateKeyHi(&wBucket, wKey, nBL)); 
    printf("LocateKeyHiAlt %d\n", LocateKeyHiAlt(&wBucket, wKey, nBL)); 
    printf("LocateKeyHiR %d\n", LocateKeyHiR(&wBucket, wKey, nBL)); 
    printf("LocateKey2Lo  %d\n", LocateKey2Lo(&wBucket, wKey, nBL)); 
    printf("LocateKeyLo  %d\n", LocateKeyLo(&wBucket, wKey, nBL)); 
    printf("LocateKeyLoR %d\n", LocateKeyLoR(&wBucket, wKey, nBL)); 

    {
        uintptr_t upLsbs, upMsbs;
        SearchPreSetupWord<uintptr_t>(nBL, /* bPackLow */ 0, upLsbs, upMsbs);
        printf("SearchPreSetupWord<uintptr_t>(nBL, bPackLow = 0)"
               " upLsbs 0x%016zx upMsbs 0x%016zx\n",
               upLsbs, upMsbs);

        uintptr_t wMatches
            = SearchWord<uintptr_t>(wBucket, wKey, nBL,
                                    /* bKeyAlreadyMasked */ 0,
                                    /* bPackLow */ 0);
        printf("wMatches 0x%016zx\n", wMatches);
        printf("HasKeyWord %d\n",
               HasKeyWord<uintptr_t>(wBucket, wKey, nBL,
                                     /* bKeyAlreadyMasked */ 0,
                                     /* bPackLow */ 0));

        SearchPreSetupWord<uintptr_t>(nBL, /* bPackLow */ 1, upLsbs, upMsbs);
        printf("SearchPreSetupWord<uintptr_t>(nBL, bPackLow = 1)"
               " upLsbs 0x%016zx upMsbs 0x%016zx\n",
               upLsbs, upMsbs);

        __m128i xLsbs, xMsbs, xKeys;
        SearchSetupBucket<__m128i, uintptr_t>(wKey,
            nBL, /* bAlreadyMasked */ 0, /* bPackLow */ 0,
            xLsbs, xMsbs, xKeys);

        //__m64 wBucket = (__m64)(uintptr_t)0; // vector of one long long
        __m128i xBucket; // vector of two long long values
        //__m256i yBucket = (__m256i)xBucket; // vector of four long long
        printf("HasKeyBucket %d\n",
               HasKeyBucket<__m128i, Word_t>(xBucket, wKey, nBL, 
                                             /* bKeyAlreadyMasked */ 0,
                                             /* bPackLow */ 0));

    }
#if defined(__AVX__)
    {
        __m256i m8;
        m8 = Mm256Set1Epi64x((uint64_t)0);
        std::cout << "m8 " << __m256i_toString<char>(m8) << std::endl;
    }
#endif // defined(__AVX__)
    std::cout << "(char)0 '" << (char)0 << "'" << std::endl;
    std::cout << "(int)(char)0 '" << (int)(char)0 << "'" << std::endl;
}
