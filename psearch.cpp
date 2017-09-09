
//
// Notes and code for thinking about parallel searching.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <typeinfo>
#include <sstream>
#include <iostream>
#include <iomanip>

#if defined(__AVX2__) || defined(__AVX__)
#include "immintrin.h" // __m256i, mm256_set1_epi64x
#elif defined(__SSE4_2__)
#include "nmmintrin.h"
#elif defined(__SSE4_1__)
#include "smmintrin.h"
#elif defined(__SSE3__)
#include "pmmintrin.h"
#elif defined(__SSE2__)
#include "emmintrin.h" // __m128i
#elif defined(__MMX__)
#include "mmintrin.h" // MMX
#endif // defined(__AVX2__)

// Overloading is complicated when depending on different sizes of ints for
// function resolution.
// Trying to write code that compiles on different platforms, e.g. LP64, WIN64
// and 32-bit, probably adds complexity.
// The types we use, e.g. Word_t, uintptr_t, uint64_t, uint32_t, unsigned,
// unsigned long, unsigned long long, and size_t are not always unique or
// unambiguous w.r.t. each other.
// char, short, int, and long long all mean the same thing independent of
// LP64, WIN64 and 32-bit.
// The meanings of long, int64_t, intptr_t, size_t and Word_t all depend on
// LP64, WIN64 or 32-bit.
// I defined u6_t to help understand and solve the problems.
#if defined(WORD_IS_UINTPTR)
typedef uintptr_t Word_t; // uint on 32-bit, ulong on LP64, ulonglong on WIN64
  #if defined(__LP64__)
#define WORD_FORMAT_SIZE "l"
  #elif defined(_WIN64)
#define WORD_FORMAT_SIZE "ll"
  #else
#define WORD_FORMAT_SIZE ""
  #endif
#elif defined(WORD_IS_SIZE)
typedef size_t Word_t;
#define WORD_FORMAT_SIZE "z"
#else
  #if defined(__LP64__) || defined(_WIN64)
typedef unsigned long long Word_t;
#define WORD_FORMAT_SIZE "ll"
  #else // defined(__LP64__) || defined(_WIN64)
typedef unsigned long Word_t;
#define WORD_FORMAT_SIZE "l"
  #endif // defined(__LP64__) || defined(_WIN64)
#endif // defined(WORD_IS_UINTPTR)

#if defined(U6_IS_UINT64)
typedef uint64_t u6_t; // ulong on LP64, ulonglong otherwise?
#else // defined(U6_IS_UINT64)
typedef unsigned long long u6_t;
#endif // defined(U6_IS_UINT64)

#if defined(__LP64__) || defined(_WIN64)
#define WORD_FORMAT_WIDTH "16"
#else // defined(__LP64__) || defined(_WIN64)
#define WORD_FORMAT_WIDTH "8"
#endif // defined(__LP64__) || defined(_WIN64)

#define Owx "0" WORD_FORMAT_WIDTH WORD_FORMAT_SIZE "x"

#if defined(__LP64__) && defined(U6_IS_UINT64)
#define O6x "016lx"
#else // defined(__LP64__) && defined(U6_IS_UINT64)
#define O6x "016llx"
#endif // defined(__LP64__) && defined(U6_IS_UINT64)

#if defined(_WIN64)
#define WORD_ONE  1ULL
#else // defined(_WIN64)
#define WORD_ONE  1UL
#endif // defined(_WIN64)

#define EXP(_x)  (WORD_ONE << (_x))
#define MSK(_x)  (EXP(_x) - 1)

#define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

#if !defined(__SSE2__)
// What is a vector?  Is it something better than a struct for this?
typedef struct { u6_t u6[2]; } __m128i;
#endif // !defined(__SSE2__)
#if !defined(__AVX__)
typedef struct { __m128i m7[2]; } __m256i;
typedef struct { __m256i m8[2]; } __m512i;
#endif // !defined(__AVX__)

using namespace std;

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
// HasKey expects the slots to be packed towards the most significant bits, i.e.
// extra bits, (sizeof(Word_t) * 8 % nBL), are at the low end.
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
// What about non-power-of-two-bit-size slots and the extra bits?
//
// Does it follow that if we have sorted keys that there will never be
// more than two bits set (unless we're padding with the last key and
// the last key matches)?
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

template <typename mx_t>
string
mx_toString(const mx_t rmx)
{
    stringstream sstr;
    sstr << hex << setfill('0');
    const u6_t* pu6 = (const u6_t*)&rmx;
    for (unsigned int i = 0; i < sizeof(mx_t) / sizeof(u6_t); i++) {
        if (i != 0) { sstr << "_"; }
        sstr << setw(16) << pu6[i];
    }
    return sstr.str();
}

template<typename ux_t, typename uk_t>
void
Set1(ux_t &rux, uk_t uk)
{
    assert(sizeof(uk) != sizeof(rux)); // call is a no-op
    rux = uk * ((ux_t)-1 / (uk_t )-1);
}

#if defined(__SSE2__)

void Set1(__m128i& rm7, uint8_t u3) { rm7 = _mm_set1_epi8(u3); }
void Set1(__m128i& rm7, uint16_t u4) { rm7 = _mm_set1_epi16(u4); }
void Set1(__m128i& rm7, uint32_t u5) { rm7 = _mm_set1_epi32(u5); }
void Set1(__m128i& rm7, u6_t u6) { rm7 = _mm_set1_epi64x(u6); }

#else // defined(__SSE2__)

template<typename uk_t>
void
Set1(__m128i& rm7, uk_t uk)
{
    Set1(((u6_t *)&rm7)[0], uk);
    Set1(((u6_t *)&rm7)[1], uk);
}

#endif // defined(__SSE2__)

#if defined(__AVX__)

void Set1(__m256i& rm8, uint8_t u3) { rm8 = _mm256_set1_epi8  (u3); }
void Set1(__m256i& rm8, uint16_t u4) { rm8 = _mm256_set1_epi16 (u4); }
void Set1(__m256i& rm8, uint32_t u5) { rm8 = _mm256_set1_epi32 (u5); }
void Set1(__m256i& rm8, u6_t u6) { rm8 = _mm256_set1_epi64x(u6); }

#else // defined(__AVX__)

template<typename uk_t>
void
Set1(__m256i& rm8, uk_t uk)
{
    Set1(((__m128i *)&rm8)[0], uk);
    Set1(((__m128i *)&rm8)[1], uk);
}

#endif // defined(__AVX__)

template<typename uk_t>
void
Set1(__m512i& rm9, uk_t uk)
{
    Set1(((__m256i *)&rm9)[0], uk);
    Set1(((__m256i *)&rm9)[1], uk);
}

// Set1 for arbitray size key slots.
// w contains the value and the bits above nBits are zero.
// bPackLo applies if nBits is not 8, 16, 32 or 64.
// It means pack the slots/values at the low end of the int
// and leave the extra bits at the high end.
// The number of extra bits will be:
// (MIN(sizeof(x_t), sizeof(u6_t)) * 8 % nBits)
template<typename x_t>
void
Set1(x_t& rx, Word_t w, int nBits, bool bPackLo)
{
    assert(nBits <= sizeof(Word_t) * 8);
    if (nBits == 8) {
        Set1(rx, (uint8_t)w);
    } else if (nBits == 16) {
        Set1(rx, (uint16_t)w);
#if defined(__LP64__) || defined(_WIN64)
    } else if (nBits == 32) {
        Set1(rx, (uint32_t)w);
#endif // defined(__LP64__) || defined(_WIN64)
#if !defined(ONLY_NATIVE_SIZES)
    } else if (nBits != sizeof(Word_t) * 8) {
        int nBitsShift = bPackLo ? MIN(sizeof(x_t), sizeof(u6_t)) * 8 % nBits : 0;
        u6_t u6 = w * (u6_t)((-1ULL / ((Word_t)1 << nBits) - 1) >> nBitsShift);
        if (sizeof(x_t) > 6) {
            Set1(rx, u6);
        }
#endif // !defined(ONLY_NATIVE_SIZES)
    } else /* if (nBits == sizeof(Word_t) * 8) */ {
        Set1(rx, (Word_t)w);
    }
}

// bPackSlotsHi means extra bits are at the low, least-significant, end.
// Sort order doesn't matter.
// Search assumes all full slots contain valid keys.
// Search assumes the bits in wKey above nBL are zero.
template<typename ux_t>
ux_t
Search(ux_t ux, Word_t wKey, int nBL, bool bPackSlotsHi)
{
    Word_t wMask = (1ULL << (nBL - 1)) * 2 - 1; // (1 << nBL) - 1
    ux_t uxLsbs = -1ULL / wMask; // lsb in each key slot
    // shift to handle non-power-of-two-bit-size keys if necessary
#if 1
    if (!bPackSlotsHi) {
        uxLsbs >>= sizeof(ux_t) * 8 % nBL;
    }
#else
    // Does this get compiled out?
    uxLsbs >>= (sizeof(ux_t) * 8 % nBL) * !bPackSlotsHi;
#endif
    ux_t uxKeys = wKey * uxLsbs; // replicate key; put in every slot
    ux_t uxMsbs = uxLsbs << (nBL - 1); // msb in each key slot
    ux_t uxXor = uxKeys ^ ux; // get zero in slot with matching key
    ux_t uxMagic = (uxXor - uxLsbs) & ~uxXor & uxMsbs;
    return uxMagic; // bHasKey == bXorHasZero == (ixMagic != 0);
}

template<typename ux_t>
ux_t
Search(ux_t *pux, Word_t wKey, int nBL, bool bPackSlotsHi)
{
    return Search(*pux, wKey, nBL, bPackSlotsHi);
}

// For native size keys. Infer nBL from sizeof(ukKey).
template<typename ux_t, typename uk_t>
ux_t
Search(ux_t ux, uk_t ukKey)
{
    return Search(ux, ukKey, sizeof(ukKey) * 8, /* bPackSlotsHi */ 1);
}

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

// Intel BSF     finds   bit number          of lsb set; undefined for 0.
// __builtin_ffs returns bit number plus one of lsb set;         0 for 0.
// __builtin_ffs(0)      = 0
// __builtin_ffs(1<< 0)  = 1
// __builtin_ffs(1<<63) = 64

// Locate key returns found key position relative to the low key.
// bPackHi means the extra bits in Word_t or u6_t that do not make a
// whole slot are at the low aka least significant end.
// bSortHi means the low key is at the high aka most significant end.
template<typename ux_t>
int // nPos
LocateKey(ux_t *pux, Word_t wKey, int nBL, bool bPackHi, bool bSortHi)
{
    ux_t ux = *pux;
    ux_t uxMagic = Search(ux, wKey, nBL, /* bPackHi */ 1);
    //printf("uxMagic 0x%" O6x"\n", (u6_t)uxMagic);
    if (uxMagic) {
        if (!bSortHi) {
            uxMagic >>= ((nBL - 1) * bPackHi);
            return __builtin_ctzll(uxMagic) / nBL;
        } else {
            // We might have one extra msb set higher than the matching key.
            // We might have multiple extra msbs set lower than the matching
            // key due to padding the list with the highest key.
            // Makes it hard to find the position of the correct matching
            // key without an extra test.
            // If there is an extra higher msb set, then the key in the slot
            // is even and is one less than wKey.
            int clz = __builtin_clzll(uxMagic);
            int nPos = (clz - (sizeof(long long) - sizeof(ux_t)) * 8) / nBL;
            Word_t wSlot = ux >> (sizeof(ux) * 8 - clz - nBL);
            return nPos + ((wKey ^ wSlot) & 1);
        }
    } else {
        return -1;
    }
}

template<typename ux_t>
int // nPos
LocateHole(ux_t *pux, Word_t wKey, int nBL)
{
    // TBD
    return -1;
}

// Is it better to assign to a reference parameter than to simply
// return a value? For __m128i, __m256i, __m512i?
// A function can't return an array.
// What is a vector? Is it better than a struct for this?
template<typename x_t> void Load(x_t& rx, x_t* px) { rx = *px; }
template<typename x_t> x_t Load(x_t* px) { return *px; }

template<typename x_t> int IsZero(x_t x) { return x == 0; }

#if defined(__SSE3__)

void Load(__m128i& rm7, __m128i *pm7) { rm7 = _mm_lddqu_si128(pm7); }
__m128i Load(__m128i *pm7) { return _mm_lddqu_si128(pm7); }

#elif defined(__SSE2__)

void Load(__m128i& rm7, __m128i *pm7) { rm7 = _mm_loadu_si128(pm7); }
__m128i Load(__m128i *pm7) { return _mm_loadu_si128(pm7); }

#endif // defined(__SSE3__)

#if defined(__SSE4_1__)

int /* bool */ IsZero(__m128i m7) {
    __m128i m7Zero = _mm_setzero_si128(); // get zero for compare
    return _mm_testc_si128(m7Zero, m7); // m7 == 0
}

#elif defined(__SSE2__)

int IsZero(__m128i m7) {
    //__m128i m7Zero ^= m7Zero; // get zero for compare
    __m128i m7Zero = _mm_setzero_si128(); // get zero for compare
    __m128i m7eq = _mm_cmpeq_epi8(m7, m7Zero);
    return _mm_movemask_epi8(m7eq) ^ 0xffff;
}

#endif // defined(__SSE4_1__)

#if defined(__AVX__)

void Load(__m256i& rm8, __m256i *pm8) { rm8 = _mm256_lddqu_si256(pm8); }
__m256i Load(__m256i *pm8) { return _mm256_lddqu_si256(pm8); }

int /* bool */ IsZero(__m256i m8) {
    __m256i m8Zero = _mm256_setzero_si256(); // get zero for compare
    return _mm256_testc_si256(m8Zero, m8); // m8 == 0
}

#endif // defined(__AVX__)

// What is the best way to handle the multiple-step setup for parallel
// search process?
// We have to replicate lsb, msb and key. Into what?
// We don't know how to have keys cross 64-bit boundaries easily yet.
// So we can replicate into 64-bit groups, then replicate those
// groups into 128-bit or 256-bit buckets.
// One issue is that we also want to handle 32-bit buckets.
// Another is that we might be able to streamline the two steps into
// one for power-of-two-bit size keys.

// A Bucket_t contains a power of two Word_ts.
// A Word_t contains an integral number of Key_ts.
// nBits doesn't have to be a power of two so
// there may be left over bits in Word_t.
// bPackLow means the keys are packed at the low
// end of the word.
template<typename ux_t> // Word_t or u6_t
void
PreSetup(int nBits, int bPackLow, ux_t& ruxLsbs, ux_t& ruxMsbs)
{
    Word_t wMask = ((Word_t)1 << (nBits - 1)) * 2 - 1;
    ruxLsbs = (ux_t)-1 / wMask;
    if (bPackLow) {
        ruxLsbs >>= sizeof(ux_t) * 8 % nBits;
    }
    ruxMsbs = ruxLsbs << (nBits - 1);
}

template<typename ux_t>
void
Setup(Word_t wKey, int nBits, int bPackLow,
          ux_t& ruxLsbs, ux_t& ruxMsbs, ux_t& ruxKeys)
{
    PreSetup(nBits, bPackLow, ruxLsbs, ruxMsbs);
    ruxKeys = wKey * ruxLsbs; // replicate key; put in every slot
}

#if 1
// This helps avoid no-op calls to Set1. Why?
void
SetupMx(Word_t wKey, int nBits, int bPackLow,
      Word_t& rwLsbs, Word_t& rwMsbs, Word_t& rwKeys)
{
    Setup(wKey, nBits, bPackLow, rwLsbs, rwMsbs, rwKeys);
}
#endif

// This version of SetupMx works only for native size keys.
template<typename uk_t, typename mx_t>
void
SetupMx(uk_t ukKey, mx_t& rmxLsbs, mx_t& rmxMsbs, mx_t& rmxKeys)
{
    uk_t ukLsb = 1;
    Set1(rmxLsbs, ukLsb);
    //cout << "rmxLsbs " << mx_toString(rmxLsbs) << endl;
    uk_t ukMsb = (uk_t)1 << (sizeof(uk_t) * 8 - 1);
    Set1(rmxMsbs, ukMsb);
    //cout << "rmxMsbs " << mx_toString(rmxMsbs) << endl;
    Set1(rmxKeys, ukKey);
    //cout << "rmxKeys " << mx_toString(rmxKeys) << endl;
}

template<typename mx_t>
void
SetupMx(Word_t wKey, int nBits, int bPackLow,
      mx_t& rmxLsbs, mx_t& rmxMsbs, mx_t& rmxKeys)
{
    u6_t u6Lsbs, u6Msbs, u6Keys;
    Setup(wKey, nBits, bPackLow, u6Lsbs, u6Msbs, u6Keys);
    Set1(rmxLsbs, u6Lsbs);
    Set1(rmxMsbs, u6Msbs);
    Set1(rmxKeys, u6Keys);
}

template<typename x_t>
x_t
Search(x_t xBucket, x_t xLsbs, x_t xMsbs, x_t xKeys)
{
    x_t xXor = xKeys ^ xBucket; // get zero in slots with matching key
    return (xXor - xLsbs) & ~xXor & xMsbs; // xMatches
}

template<typename x_t>
x_t
Search(x_t* pxBucket, x_t xLsbs, x_t xMsbs, x_t xKeys)
{
    return Search(Load(pxBucket), xLsbs, xMsbs, xKeys);
}

template<typename x_t>
x_t
Search(x_t xBucket, Word_t wKey, int nBits, int bPackLow)
{
    x_t xLsbs, xMsbs, xKeys;
    SetupMx(wKey, nBits, bPackLow, xLsbs, xMsbs, xKeys);
    return Search(xBucket, xLsbs, xMsbs, xKeys);
}

template<typename x_t>
x_t
Search(x_t* pxBucket, Word_t wKey, int nBits, int bPackLow)
{
    return Search(Load(pxBucket), wKey, nBits, bPackLow);
}

// Should we allow HasKey to return any non-zero value if key is present?
// Or should we have HasKey always convert non-zero to 1?
// Will the compiler remove the conversion if it is not really needed?

// This version of HasKey works only for native size keys.
template<typename x_t, typename uk_t>
int
HasKey(x_t xBucket, uk_t ukKey)
{
    x_t xLsbs, xMsbs, xKeys;
    SetupMx(ukKey, xLsbs, xMsbs, xKeys);
    return !IsZero(Search(xBucket, xLsbs, xMsbs, xKeys));
}

// This version of HasKey works only for native size keys.
template<typename x_t, typename uk_t>
int
HasKey(x_t *pxBucket, uk_t ukKey)
{
    x_t xBucket; Load(xBucket, pxBucket);
    return HasKey(xBucket, ukKey);
}

template<typename x_t>
int
HasKey(x_t xBucket, Word_t wKey, int nBits, int bPackLo)
{
    return !IsZero(Search(xBucket, wKey, nBits, bPackLo));
}

template<typename x_t>
int
HasKey(x_t *pxBucket, Word_t wKey, int nBits, int bPackLo)
{
    x_t xBucket; Load(xBucket, pxBucket);
    return HasKey(xBucket, wKey, nBits, bPackLo);
}

template<typename ux_t>
int
LeastSignificantNonZeroByte(ux_t ux)
{
    return __builtin_ctzll(ux) / 8;
}

// 2^127 => 2^15
// 2^119 => 2^14
// 2^111 => 2^13
// 2^103 => 2^12
// 2^ 95 => 2^11
// 2^ 87 => 2^10
// 2^ 79 => 2^ 9
// 2^ 71 => 2^ 8
// 2^ 63 => 2^ 7
// 2^ 55 => 2^ 6
// 2^ 47 => 2^ 5
// 2^ 39 => 2^ 4
// 2^ 31 => 2^ 3
// 2^ 23 => 2^ 2
// 2^ 15 => 2^ 1
// 2^  7 => 2^ 0

// Name this function:
//   f(2^(8*n+7)) = 2^n
//   f(2^(8*n-1)) = 2^(n-1)
//   f((8*n+7)) = n
//   f-1(n) = 8*n+7

#if defined(__SSE2__)

int
LeastSignificantNonZeroByte(__m128i& rm7)
{
    cout << "LeastSignificantNonZeroByte(" << mx_toString(rm7) << "):\n";

#if defined(__SSE4_2__)
    // returns 16 if no match
    cout << "cmpistri(~rm7, ~zero, EACH|NEG) "
         << _mm_cmpistri(~rm7, ~_mm_setzero_si128(),
                         _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY)
         << endl;

    // returns 16 if no match
    cout << "cmpestri(rm7, 16, zero, 16, EACH|NEG) "
         << _mm_cmpestri(rm7, 16, _mm_setzero_si128(), 16,
                         _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY)
         << endl;
#endif // defined(__SSE4_2__)

    cout << "ctz(movemask(rm7 | 0x10000)) "
         << __builtin_ctz(_mm_movemask_epi8(rm7) | 0x10000)
         << endl;

#if defined(__SSE4_2__)
    // _mm_cmpestri is SSE4.2
    // _mm_setzero_si128 is SSE4.2
    return _mm_cmpestri(rm7, 16, _mm_setzero_si128(), 16,
                         _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY);
#else // defined(__SSE4_2__
    // compress with movemask
    // count with ctz
    // or with 0x10000 to avoid undefined
    // _mm_movemask_epi8 is SSE2
    // _bit_scan_forward is immintrin.h
    return __builtin_ctz(_mm_movemask_epi8(rm7) | 0x10000);
#endif // defined(__SSE4_2__)
}

int
LeastSignificantNonZeroByte(__m128i* pm7)
{
    //__m128i m7; return LeastSignificantNonZeroByte((Load(m7, pm7), m7));
    //__m128i m7 = Load(pm7); return LeastSignificantNonZeroByte(m7);
    __m128i m7; Load(m7, pm7); return LeastSignificantNonZeroByte(m7);
}

#endif // defined(__SSE2__)

#if defined(__AVX__)

int
LeastSignificantNonZeroByte(__m256i& rm8)
{

#if defined(__AVX2__)
    // _mm256_movemask_epi8 is AVX2
    return __builtin_ctzll(_mm256_movemask_epi8(rm8) | 0x100000000ULL);
    // _mm256_extract128_si256 is AVX2
#else // defined(__AVX2__)
  #if defined(LSNZB_EXTRACT)
      #if defined(__LP64__) || defined(_WIN64)
    // _mm256_extract_epi64 is AVX. Unless -m32?
    __m128i m7_0 = _mm_set_epi64x(_mm256_extract_epi64(rm8, 1),
                                  _mm256_extract_epi64(rm8, 0));
    __m128i m7_1 = _mm_set_epi64x(_mm256_extract_epi64(rm8, 3),
                                  _mm256_extract_epi64(rm8, 2));
      #else // defined(__LP64__) || defined(_WIN64)
    // _mm256_extract_epi64 is AVX2. For -m32?
    // _mm256_extract_epi32 is AVX
    __m128i m7_0 = _mm_set_epi32(_mm256_extract_epi32(rm8, 3),
                                 _mm256_extract_epi32(rm8, 2),
                                 _mm256_extract_epi32(rm8, 1),
                                 _mm256_extract_epi32(rm8, 0));
    __m128i m7_1 = _mm_set_epi32(_mm256_extract_epi32(rm8, 7),
                                 _mm256_extract_epi32(rm8, 6),
                                 _mm256_extract_epi32(rm8, 5),
                                 _mm256_extract_epi32(rm8, 4));
      #endif // defined(__LP64__) || defined(_WIN64)
  #else // defined(LSNZB_EXTRACT)
    __m128i m7_0; Load(m7_0, (__m128i*)&rm8);
    __m128i m7_1; Load(m7_1, (__m128i*)&rm8 + 1);
  #endif // defined(LSNZB_EXTRACT)
    int n0 = LeastSignificantNonZeroByte(m7_0);
  #if defined(LSNZB_IF)
    return (n0 == 16) ? n0 + LeastSignificantNonZeroByte(m7_1) : n0;
  #else // defined(LSNZB_IF)
    return n0 + LeastSignificantNonZeroByte(m7_1) * (n0 >> 4);
  #endif // defined(LSNZB_IF)
#endif // defined(__AVX2__)
}

int
LeastSignificantNonZeroByte(__m256i* pm8)
{
    __m256i m8; Load(m8, pm8);
#if defined(__AVX2__) || !defined(LSNZB_DUP)
    return LeastSignificantNonZeroByte(m8);
#else // defined(__AVX2__)
    __m128i* pm7 = (__m128i*)pm8;
    int n0 = LeastSignificantNonZeroByte(pm7);
  #if defined(LSNZB_IF)
    return (n0 == 16) ? n0 + LeastSignificantNonZeroByte(pm7 + 1) : n0;
  #else // defined(LSNZB_IF)
    return n0 + LeastSignificantNonZeroByte(pm7 + 1) * (n0 >> 4);
  #endif // defined(LSNZB_IF)
#endif // defined(__AVX2__)
}

#endif // defined(__AVX__)

// Native size keys, sorted from lsb to msb in u6_t.
template<typename x_t, typename uk_t>
int
LocateKey(x_t& rxBucket, uk_t ukKey)
{
    x_t xLsbs, xMsbs, xKeys;
    SetupMx(ukKey, xLsbs, xMsbs, xKeys);
    x_t xMatches = Search(rxBucket, xLsbs, xMsbs, xKeys);
    //printf("xMatches %s\n", mx_toString(xMatches).c_str());
    int n = LeastSignificantNonZeroByte(xMatches);
    printf("LeastSignificantNonZeroByte %d\n", n);
    return n / sizeof(ukKey);

#if 0
    // cmpeq does not compress
    // compress with movemask
    // count with ctz
    __m128i m7eq = _mm_cmpeq_epi8(m7, _mm_setzero_si128());
    cout << "cmpeq(" << mx_toString(m7) << ", zero) "
         << mx_toString(m7eq)
         << endl;
    cout << "ctz(movemask(~" << mx_toString(m7eq) << " | (1 << 16)) "
         << __builtin_ctz(_mm_movemask_epi8(~m7eq) | (1 << 16))
         << endl;
#endif

//_mm_cmpeq_epi8
// cmpeq gives -1 if eq and 0 if not eq
// cmpgt gives -1 if gt and 0 if not eq
// cmplt gives -1 if lt and 0 if not eq
// cmpeq, cmpgt and cmplt do not compress
}

// If key is present return nPos.
// Else return a negative number.
template<typename x_t, typename uk_t>
int
LocateKey(x_t *pxBucket, uk_t ukKey)
{
    x_t xBucket; Load(xBucket, pxBucket);
    return LocateKey(xBucket, ukKey);
}

#if defined(__SSE2__)

int
LeastSignificantGreaterThan(__m128i& rm7Bucket, __m128i& rm7Keys, int nBits)
{
    printf("LeastSignificantGreaterThan(m7, m7, nBits\n");
    __m128i m7 = (nBits ==  8) ? _mm_cmpgt_epi8(rm7Bucket, rm7Keys)
              // (nBits ==  8) ? _mm_cmpgt_epi8(~rm7Keys, ~m7Bucket)
               : (nBits == 16) ? _mm_cmpgt_epi16(rm7Bucket, rm7Keys)
  #if defined(__SSE4_2)
               : (nBits == 64) ? _mm_cmpgt_epi64(rm7Bucket, rm7Keys)
  #endif // defined(__SSE4_2)
               : /*        32 */ _mm_cmpgt_epi32(rm7Bucket, rm7Keys);
    return LeastSignificantNonZeroByte(m7) / (nBits >> 3);
}

int
LocateSlot(__m128i& rm7Bucket, __m128i& rm7Keys, int nBits)
{
    printf("LocateSlot(m7, m7, nBits)\n");
    return LeastSignificantGreaterThan(rm7Bucket, rm7Keys, nBits);
}

#endif // defined(__SSE2__)

#if defined(__AVX__)

int
LocateSlot(__m256i& rm8Bucket, __m256i& rm8Keys, int nBits)
{
    printf("LocateSlot(m8, m8, nBits)\n");
    __m128i* pm7 = (__m128i*)&rm8Bucket;
    //__m128i m7_0 = CmpGt(pm7, m7Keys, nBits);;

    return 0;
}

#endif // defined(__AVX__)

// If key is not present return nPos of slot where it should be inserted.
// Else return a negative number.
template<typename x_t, typename uk_t>
int
LocateSlot(x_t xBucket, uk_t ukKey)
{
    x_t xKeys;
    Set1(xKeys, ukKey);
    int n = LocateSlot(xBucket, xKeys, sizeof(ukKey) * 8);
    printf("LocateSlot %d\n", n);
    return n;
}

template<typename x_t, typename uk_t>
int
LocateSlot(x_t* pxBucket, uk_t ukKey)
{
    x_t xBucket; Load(xBucket, pxBucket);
    return LocateSlot(xBucket, ukKey);
}

#if defined(__SSE2__)

// cmpestri
//
// Compare each character from the first string with every character from
// the second string for equality to create a matrix of boolean results.
// Default character size and type is UBYTE.
// Also supports UWORD, SBYTE and SWORD.
//
// For EQUAL_ANY (default) or RANGES, override the result in the matrix
// with 0 if either character is past the end of its string.
// For EQUAL_EACH, override the result in the matrix with 0 if just one
// character is past the end of its string and with 1 if both characters
// are past the end of their strings.
// For EQUAL_ORDERED, override the result in the matrix with 1 if the
// character in the first string is past the end of the string else with
// 0 if the character in the second string is past the end of the string.
//
// Create a result bit array from the matrix as follows:
// For EQUAL_ANY, set the nth bit in the result if the nth character in
// the first string matches ANY character in the second string.
// For RANGES, set the nth bit of the result if the nth character in the
// first string matches the 2*nth or 2*n+1th character in the 2nd string.
// For EQUAL_EACH, set the nth bit in the result if the nth character
// in the first string matches the nth character in the second string.
// For EQUAL_ORDERED, set the nth bit of the result if the tail of
// the first string is the same as the beginning of the second string
// starting at the nth character of the first string.
//
// For NEGATE, complement the bits in the result.
// For MASKED_NEGATE, complement the 1st n bits in the result where n
// is the length of the second string.
//
// For LEAST_SIGNIFICANT (default), return the index of the least
// significant 1 in the result.
// For MOST_SIGNIFICANT, return the index of the most significant 1
// in the result.

// HasKey 2^7 x 2^6 x SSE2
// A lot of code to get rid of one test.
int
HasKey762a(__m128i* pm7b, uint64_t u6k)
{
    __m128i m7b = _mm_load_si128(pm7b); // load the bucket; use loadu for unaligned
    __m128i m7ks = _mm_set_epi64x(u6k, u6k); // replicate key for compare
    __m128i m7eq = _mm_cmpeq_epi32(m7b, m7ks); // no cmpeq_epi64 in SSE2
    int32_t i5 = _mm_movemask_epi8(m7eq); // compress into int
    i5 = ~i5; // make each 0xff into 0 for single test with multiply
    i5 = (i5 >> 8) * (uint8_t)i5; // i5 <= 0xff * 0xff
    // return !i5; // Does the compiler implement ! without a test?
    return ((i5 + 0xffff) >> 16) ^ 1; // reduce to 1 or leave as 0
}

int
HasKey762b(__m128i* pm7b, uint64_t u6k)
{
    uint64_t *pu6b = (uint64_t *)pm7b;
    __m128i m7k = _mm_set_epi64x(0, u6k);
    __m128i m7b = _mm_set_epi64x(0, pu6b[0]);
    __m128i m7diff = m7b - m7k;
    __m128i m7u6max = _mm_set_epi64x(0, -1LL); // prep to reduce
    __m128i m7 = m7diff + m7u6max; // set bit plus 1
    int bHasKeyNot = _mm_extract_epi16(m7, 4);
    m7b = _mm_set_epi64x(0, pu6b[1]);
    m7diff = m7b - m7k;
    m7 = m7diff + m7u6max;
    bHasKeyNot &= _mm_extract_epi16(m7, 4);
    return bHasKeyNot ^ 1;
}

#endif // defined(__SSE2__)

int
HasKey640(uint64_t *pu6b, uint16_t u4k)
{
    uint64_t u6lsbs = -1ULL / (uint16_t)-1; // presetup
    uint64_t u6msbs = u6lsbs << 15; // presetup
    uint64_t u6keys = u4k * u6lsbs; // setup
    uint64_t u6xor = u6keys ^ *pu6b; // get zero in slots with matching key
    // get slot=1<<15 in least slot with matching key if one exists
    uint64_t u6Matches = (u6xor - u6lsbs) & ~u6xor & u6msbs;
    //return u6Matches;
    return (u6Matches | (u6Matches + (-1ULL >> 1))) >> 63; // reduce to 1 bit
}

int
HasKey650(uint64_t *pu6b, uint32_t u5k)
{
    uint32_t *pu5b = (uint32_t *)pu6b;
    uint64_t u6hasKeyNot = (uint64_t)(pu5b[0] ^ u5k) * (pu5b[1] ^ u5k);
    // Does the compiler implement !u6HasKeyNot with a test as costly as a
    // conditional branch? Does it combine the test before the return and
    // the test in the caller into a single test if the function is inlined?
    //return !bHasKeyNot;
    int bHasKeyNot = (u6hasKeyNot | (u6hasKeyNot + (-1ULL >> 1))) >> 63;
    return bHasKeyNot ^ 1;
}

int
HasKey750(uint64_t *pu6b, uint32_t u5k)
{
    uint32_t *pu5b = (uint32_t *)pu6b;
    uint64_t u6prod = (uint64_t)(pu5b[0] ^ u5k) * (pu5b[1] ^ u5k);
    int bHasKeyNot = (u6prod | (u6prod + (-1ULL >> 1))) >> 63;
    u6prod = (uint64_t)(pu5b[2] ^ u5k) * (pu5b[3] ^ u5k);
    bHasKeyNot = (u6prod | (u6prod + (-1ULL >> 1))) >> 63;
    return bHasKeyNot ^ 1;
}

int
HasKey760(uint64_t *pu6b, uint64_t u6k)
{
    uint64_t u6diff = pu6b[0] ^ u6k;
    int bHasKeyNot = (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    u6diff = pu6b[1] ^ u6k;
    bHasKeyNot &= (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    return bHasKeyNot ^ 1;
}

int
HasKey860(uint64_t *pu6b, uint64_t u6k)
{
    uint64_t u6diff = pu6b[0] ^ u6k;
    int bHasKeyNot = (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    u6diff = pu6b[1] ^ u6k;
    bHasKeyNot &= (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    u6diff = pu6b[2] ^ u6k;
    bHasKeyNot &= (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    u6diff = pu6b[3] ^ u6k;
    bHasKeyNot &= (u6diff | (u6diff + (-1ULL >> 1))) >> 63;
    return bHasKeyNot ^ 1;
}

void
Locate(void)
{
    // _[load|loadu]_si128 - SSE2
    // m7aa = _set1_epi[8|16|32|64x](a) - SSE2
    // m7aa = _set_epi64x(e1, e0) - SSE2
    // m7gt = _cmpgt_epi[8|16|32](a, b) - SSE2 - r[x] = a[x] > b[x] ? -1 : 0
    // m7eq = _cmpeq_epi[8|16|32](a, b) - SSE2
    // m7msbs = _movemask_epi8(a) - SSE2 - compress msbs
    // m7zero = _setzero_si128(void) - SSE2

    // HasKey_x, where x is 8 or 16 or 32:
    // m7 = load(pm7) or loadu(pm7);
    // m7keys = _set1_epix(xKey); // 8|16|32
    // m7eq = _cmpeq_epix(m7, m7keys);
    // iHasKey = _movemask(m7eq)

    // HasKey_64:
    // m7 = load(pm7) or loadu(pm7);
    // m7keys = _set_epi64x(key, key);
    // m7eq = _cmpeq_epi32(m7, m7keys);
    // imsbs = _movemask_epi8(a);
    // m7 = _set1_epi16(imsbs);
    // m7keys = _set1_epi8(ff);
    // m7eq = _cmpeq_epi8(m7, m7keys);
    // iHasKey = _movemask_epi8(m7eq);

    // imsbs & ff == ff || imsbs & ff00 == ff00

    // HasKey_64:
    // m7 = load or loadu;
    // m7keys = _set_epi64x(key, key);
    // m7lsbs = _set_epi64x(1, 1);
    // m7msbs = m7lsbs << 63;
    // m7xor = m7keys ^ m7;
    // m7magic = m7xor - m7lsbs;
    // m7magic &= ~m7xor;
    // m7magic &= m7msbs;
    // setzero
    // testc

    // Locate[8|16|32]:
    // m7 = _load or _loadu
    // m7keys = _set1_epix(key); // 8|16|32
    // m7gt = _cmpgt(m7, m7keys);
    // m7eq = _cmpeq(m7, m7keys);
    // m7ge = m7gt | m7eq;
    // nPos = ctz/bsf(movemask_epi8(m7ge)) / sizeof(ux_t);
    // if (aux[nPos] != key) { nPos ^= -1; }

    // n = _cmpestri(m7, 16, m7Keys, 16, EACH|NEG) - SSE4_2 - idx of 8|16 key
    // _lddqu_si128 - SSE3
    // _stream_load_si128 - SSE4_1
    // i = _testc_si128 - SSE4_1
    // i = _test_all_ones(a) - SSE4_1
    // m7gt = _cmpgt_epi64(a, b) - SSE4_2
    // m7eq = _cmpeq_epi64(a, b) - SSE4_2

    // What is the fastest
    //   - if (_test_all_ones(~m7eq) != 0), or
    //   - if (_testc_si128(_setzero_si128(), m7eq) != 0)

    // 256_set1_epi[8|16|32|64x] - AVX
    // 256_[load|loadu|lddqu]_si256 - AVX

    // m8gt = 256_cmpgt_epi[8|16|32|64](a, b) - AVX2
    // m8eq = 256_cmpeq_epi[8|16|32|64](a, b) - AVX2
    // m8msbs = 256_movemask_epi8(a) - AVX2 - compress msbs

    // m7 = _insert_epi16(a, i, index) - SSE2
    // m7 = _insert_epi[8|32|64](a, i, index) - SSE4_1
    // m8 = 256_insert_epi[8|16|32|64](a, i, index) - AVX
    // m8 = 256_inserti128_si256(a, m7, imm8) - AVX2
    // i = _extract_epi16(a, index) - SSE2
    // i = _extract_epi[8|32|64](a, index) - SSE4_1
    // i = 256_extract_epi[8|16|32|64](a, index) - AVX
    // m7 = 256_extracti128_si256(a, imm8) - AVX2
    // 256_stream_load_si256 - AVX2
}

// Usage: psearch64 <# of bits> <key> <bucket> [ <bucket>, ... ]
int
main(int argc, char **argv)
{
    if ((argc != 4) && (argc != 5) && (argc != 7) && (argc != 11)) {
        printf("usage: %s <# of bits> <key> <bucket>...\n", argv[0]);
        exit(1);
    }

    unsigned nBL = strtoul(argv[1], NULL, 0); printf("nBL %d\n", nBL);
    Word_t wKey = strtoul(argv[2], NULL, 0); printf("wKey 0x%" Owx"\n", wKey);

  #if 1
    __m512i pm9Bucket[1];
    __m256i *pm8Bucket = (__m256i *)pm9Bucket;
    __m128i *pm7Bucket = (__m128i *)pm8Bucket;
    u6_t *pu6Bucket = (u6_t *)pm7Bucket;
  #else
    // test unaligned
    char pucBucket[128];
    u6_t *pu6Bucket = (u6_t *)pucBucket[1];
    __m128i *pm7Bucket = (__m128i *)pucBucket[1];
    __m256i *pm8Bucket = (__m256i *)pucBucket[1];
  #endif

    char **v = &argv[3];

    printf("pu6Bucket %p\n", (void *)pu6Bucket);
    printf("*pu6Bucket");
    for (int ii = 0; ii < argc - (v - argv); ii++) {
        pu6Bucket[ii] = strtoull(v[ii], NULL, 0);
        printf(" 0x%" O6x, pu6Bucket[ii]);
    }
    printf("\n");

    u6_t u6Bucket = pu6Bucket[0];
    printf("u6Bucket 0x%" O6x"\n", u6Bucket);
    Word_t wBucket = (Word_t)u6Bucket;
    printf("wBucket 0x%" Owx"\n", wBucket);

#if defined(__SSE2__)
    __m128i m7; Load(m7, pm7Bucket);
#endif // defined(__SSE2__)

    if ((64 % nBL) != 0) {
        printf("LocateKey pack=hi sort=lo wBucket 0x%" Owx" nPos %d\n",
               wBucket, LocateKey(&wBucket, wKey, nBL,
               /* bPackHi */ 1, /* bSortHi */ 0));
        printf("LocateKey pack=hi sort=lo u6Bucket 0x%" O6x" nPos %d\n",
               u6Bucket, LocateKey(&u6Bucket, wKey, nBL,
               /* bPackHi */ 1, /* bSortHi */ 0));
        printf("LocateKey pack=hi sort=hi wBucket 0x%" Owx" nPos %d\n",
               wBucket, LocateKey(&wBucket, wKey, nBL,
               /* bPackHi */ 1, /* bSortHi */ 1));
        printf("LocateKey pack=hi sort=hi u6Bucket 0x%" O6x" nPos %d\n",
               u6Bucket, LocateKey(&u6Bucket, wKey, nBL,
               /* bPackHi */ 1, /* bSortHi */ 1));
    }

    {
        printf("LocateKey pack=lo sort=lo wBucket 0x%" Owx" nPos %d\n",
               wBucket, LocateKey(&wBucket, wKey, nBL,
               /* bPackHi */ 0, /* bSortHi */ 0));
        printf("LocateKey pack=lo sort=lo u6Bucket 0x%" O6x" nPos %d\n",
               u6Bucket, LocateKey(&u6Bucket, wKey, nBL,
               /* bPackHi */ 0, /* bSortHi */ 0));
        printf("LocateKey pack=lo sort=lo wBucket 0x%" Owx" nPos %d\n",
               wBucket, LocateKey(&wBucket, wKey, nBL,
               /* bPackHi */ 0, /* bSortHi */ 1));
        printf("LocateKey pack=lo sort=lo u6Bucket 0x%" O6x" nPos %d\n",
               u6Bucket, LocateKey(&u6Bucket, wKey, nBL,
               /* bPackHi */ 0, /* bSortHi */ 1));
    }

    {
        u6_t u6Lsbs, u6Msbs;
        PreSetup(nBL, /* bPackLow */ 0, u6Lsbs, u6Msbs);
//printf("PreSetup(nBL, bPackLow = 0) u6Lsbs 0x%" O6x" u6Msbs 0x%" O6x"\n", u6Lsbs, u6Msbs);

        u6_t u6Keys;
        Setup(wKey, nBL, /* bPackLow */ 0, u6Lsbs, u6Msbs, u6Keys);
// printf("Setup(nBL, bPackLow = 0) u6Lsbs 0x%" O6x" u6Msbs 0x%" O6x" u6Keys 0x%" O6x"\n", u6Lsbs, u6Msbs, u6Keys);

        unsigned long ulBucket = wBucket;
        u6_t u6Matches = Search(wBucket, wKey, nBL, /* bPackLow */ 0);
// printf("u6Matches 0x%" O6x"\n", u6Matches);
        printf("HasKey(wBucket, ..., bPackLow = 0) %d\n",
               HasKey(wBucket, wKey, nBL, /* bPackLow */ 0));

        PreSetup(nBL, /* bPackLow */ 1, u6Lsbs, u6Msbs);
// printf("PreSetup(nBL, bPackLow = 1) u6Lsbs 0x%" O6x" u6Msbs 0x%" O6x"\n", u6Lsbs, u6Msbs);

        u6Matches = Search(wBucket, wKey, nBL, /* bPackLow */ 1);
// printf("u6Matches 0x%" O6x"\n", u6Matches);
        printf("HasKey(wBucket, ..., bPackLow = 1) %d\n",
               HasKey(wBucket, wKey, nBL, /* bPackLow */ 1));

    }

    //{
        //__m64 wBucket = (__m64)(uintptr_t)0; // vector of one long long
        printf("HasKey(wBucket, (uint8_t)wKey) %d\n", HasKey(wBucket, (uint8_t)wKey));
        printf("HasKey(u6Bucket, (uint8_t)wKey) %d\n", HasKey(u6Bucket, (uint8_t)wKey));
        printf("LocateKey(u6Bucket, (uint8_t)wKey) %d\n", LocateKey(u6Bucket, (uint8_t)wKey));

        //__m128i m7Lsbs, m7Msbs, m7Keys;
        //SetupMx(wKey, nBL, /* PackLo */ 0, m7Lsbs, m7Msbs, m7Keys);
        //cout << "SetupMx" << endl;
        //cout << "m7Lsbs " << mx_toString(m7Lsbs) << endl;
        //cout << "m7Msbs " << mx_toString(m7Msbs) << endl;
        //cout << "m7Keys " << mx_toString(m7Keys) << endl;

#if defined(__SSE2__)
        __m128i m7Bucket; // vector of two long long values
        //m7Bucket = *pm7Bucket;
        Load(m7Bucket, pm7Bucket);
        cout << "m7Bucket " << mx_toString(m7Bucket) << endl;
        printf("HasKey(m7Bucket, ...) %d\n",
               HasKey(m7Bucket, wKey, nBL, /* bPackLow */ 0));
#endif // defined(__SSE2__)
#if defined(__AVX__)
        //__m256i m8Lsbs, m8Msbs, m8Keys;
        //SetupMx(wKey, nBL, /* PackLo */ 0, m8Lsbs, m8Msbs, m8Keys);
        //cout << "SetupMx" << endl;
        //cout << "m8Lsbs " << mx_toString(m8Lsbs) << endl;
        //cout << "m8Msbs " << mx_toString(m8Msbs) << endl;
        //cout << "m8Keys " << mx_toString(m8Keys) << endl;

        __m256i m8Bucket; // vector of four long long
        //m8Bucket = *pm8Bucket; // must be aligned
        //m8Bucket = _mm256_load_si256(pm8Bucket); // must be aligned
        //m8Bucket = _mm256_stream_load_si256(pm8Bucket); // align !cached avx2
        //m8Bucket = _mm256_loadu_si256(pm8Bucket);
        //m8Bucket = _mm256_lddqu_si256(pm8Bucket); // cache line crossing
        Load(m8Bucket, pm8Bucket);
        cout << "m8Bucket " << mx_toString(m8Bucket) << endl;
        printf("HasKey(m8Bucket, ...) %d\n",
               HasKey(m8Bucket, wKey, nBL, /* bPackLow */ 0));
#endif // defined(__AVX__)
    //}

#if defined(__SSE2__)
    if (__builtin_popcountll(nBL) == 1) {
        if (__builtin_ctzll(nBL) == 3) {
            printf("HasKey(pm7Bucket, (uint8_t)wKey) %d\n",
                   HasKey(pm7Bucket, (uint8_t)wKey));
            printf("LocateKey(pm7Bucket, (uint8_t)wKey) %d\n",
                   LocateKey(pm7Bucket, (uint8_t)wKey));
        } else if (__builtin_ctzll(nBL) == 4) {
            printf("HasKey(pm7Bucket, (uint16_t)wKey) %d\n",
                   HasKey(pm7Bucket, (uint16_t)wKey));
            printf("LocateKey(pm7Bucket, (uint16_t)wKey) %d\n",
                   LocateKey(pm7Bucket, (uint16_t)wKey));
        } else if (__builtin_ctzll(nBL) == 5) {
            printf("HasKey(pm7Bucket, (uint32_t)wKey) %d\n",
                   HasKey(pm7Bucket, (uint32_t)wKey));
            printf("LocateKey(pm7Bucket, (uint32_t)wKey) %d\n",
                   LocateKey(pm7Bucket, (uint32_t)wKey));
        } else if (__builtin_ctzll(nBL) == 6) {
            printf("HasKey(pm7Bucket, (u6_t)wKey) %d\n",
                   HasKey(pm7Bucket, (u6_t)wKey));
            printf("LocateKey(pm7Bucket, (u6_t)wKey) %d\n",
                   LocateKey(pm7Bucket, (u6_t)wKey));
        }
    }
#endif // defined(__SSE2__)

#if defined(__AVX__)
    if (__builtin_popcountll(nBL) == 1) {
        if (__builtin_ctzll(nBL) == 3) {
            printf("HasKey(pm8Bucket, (uint8_t)wKey) %d\n",
                   HasKey(pm8Bucket, (uint8_t)wKey));
            printf("LocateKey(pm8Bucket, (uint8_t)wKey) %d\n",
                   LocateKey(pm8Bucket, (uint8_t)wKey));
        } else if (__builtin_ctzll(nBL) == 4) {
            printf("HasKey(pm8Bucket, (uint16_t)wKey) %d\n",
                   HasKey(pm8Bucket, (uint16_t)wKey));
            printf("LocateKey(pm8Bucket, (uint16_t)wKey) %d\n",
                   LocateKey(pm8Bucket, (uint16_t)wKey));
        } else if (__builtin_ctzll(nBL) == 5) {
            printf("HasKey(pm8Bucket, (uint32_t)wKey) %d\n",
                   HasKey(pm8Bucket, (uint32_t)wKey));
            printf("LocateKey(pm8Bucket, (uint32_t)wKey) %d\n",
                   LocateKey(pm8Bucket, (uint32_t)wKey));
        } else if (__builtin_ctzll(nBL) == 6) {
            printf("HasKey(m8Bucket, (u6_t)wKey) %d\n",
                   HasKey(m8Bucket, (u6_t)wKey));
            printf("LocateKey(pm8Bucket, (u6_t)wKey) %d\n",
                   LocateKey(pm8Bucket, (u6_t)wKey));
        }
    }
#endif // defined(__AVX__)
#if defined(__SSE2__)
    if (nBL == 8) {
        LocateSlot(pm7Bucket, (uint8_t)wKey);
    } else if (nBL == 16) {
        LocateSlot(pm7Bucket, (uint16_t)wKey);
    } else if (nBL == 32) {
        LocateSlot(pm7Bucket, (uint32_t)wKey);
    } else if (nBL == 64) {
        LocateSlot(pm7Bucket, (u6_t)wKey);
    }
#endif // defined(__SSE2__)
}

// Is it better to use a different packing/padding strategy for
// power-of-two-bit-size keys than for non-power-of-two-bit-size keys?
// Power-of-two-bit-size doesn't need to worry about non-key bits when trying
// to determine if a key is present or when trying to find the position
// of a key or a slot where a key would be.

// For power-of-two-bit-size keys:
// Pack low; sort from low; pad hi.
//
// Ideally we'd avoid extra shift for non-power-of-two-bit-size keys during
// HasKey by packing keys at high end.
// Ideally we'd use ctz since it is a direct map of BSF. It means counting
// from the low end.
// Fastest is probably keys packed at high end with highest key at high end
// and
// __builtin_ffs has to do an expensive test for zero internally since BSF
// does not do one.
// HasKeyLo has to do an expensive mod and shift.

