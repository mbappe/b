
#include <stdio.h>

#include <inttypes.h>
#include <xmmintrin.h> // _mm_max_pu8, __m64
#include <immintrin.h> // _mm_movemask_epi8, __m128i

// _mm_loadu_si128 is SSE2
// _mm_lddqu_si128 is SSE3
// _mm_lddqu_si128 "may perform better than _mm_loadu_si128 when the data
// crosses a cache line boundary".

// _mm_cmpge_epi* all do signed compares.
// The compiler generates the code necessary to do unsigned compares.

// v_t is a vector of 16 chars. __m128i is a vector of 2 long longs.
// We need the char variant so we can compare with a char using '==' or '>='.
#ifdef __clang__
// clang has some support for gcc attribute "vector_size" but it doesn't work
// as well as its own ext_vector_type.
// For example, it won't promote a scalar to a vector for compare.
typedef unsigned char __attribute__((ext_vector_type(16))) v_t;
#else // __clang__
// gcc has no support for clang attribute "ext_vector_type".
typedef unsigned char __attribute__((vector_size(16))) v_t;
#endif // __clang__

// HasKey tests to see if Bucket has Key.
// HasKey returns (1 << matching slot number) if sorted full Bucket
// has Key or zero if Bucket does not have Key.
// Keys must be sorted with lowest key at vector index zero.
int
HasKey(v_t Bucket, unsigned char Key)
{
    v_t vEq = (v_t)(Bucket == Key);
    return _mm_movemask_epi8((__m128i)vEq); // (1 << matching slot) or 0
}

// HasKey_Mask uses the zero bits in Mask to specify which slots to ignore.
int
HasKey_Mask(v_t Bucket, unsigned char Key, int Mask)
{
    return HasKey(Bucket, Key) & Mask;
}

// HasKey_PopCnt tests to see if sorted partial Bucket has Key.
// HasKey_PopCnt ignores slots past the first PopCnt slots.
// HasKey_PopCnt returns (1 << matching slot number) if sorted partial Bucket
// has Key or zero if partial Bucket does not have Key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
HasKey_PopCnt(v_t Bucket, unsigned char Key, int nPopCnt)
{
    return HasKey_Mask(Bucket, Key, ~(-1 << nPopCnt));
}

// HasKey_Start_Len test to see if sorted partial Bucket has Key.
// HasKey_Start_Len ignores slots before Start and after Start plus Len.
// HasKey_PopCnt returns (1 << matching absolute slot number) if sorted
// partial Bucket has Key or zero if partial Bucket does not have Key.
// Relevant keys must be packed and sorted with lowest key at Start.
int
HasKey_Start_Len(v_t Bucket, unsigned char Key, int nStart, int nLen)
{
    return HasKey_Mask(Bucket, Key, ~(-1 << nLen) << nStart);
}

// LocateKey returns the matching slot number if sorted full Bucket
// has Key or -1 if Bucket does not have Key.
// Keys must be sorted with lowest key at vector index zero.
int
LocateKey(v_t Bucket, unsigned char Key)
{
    // get (matching byte num + 1) or 0 if no match
    // It looks like gcc doesn't use tzcnt for ffs even with -mbmi and the
    // compile fails with _tzcnt_u32 unless -mbmi.
    // I don't know how to control this without using my own ifdef yet.
    // Maybe gcc knows better.
    // It looks like gcc defines _tzcnt_u32 as __builtin_ctz so gcc has
    // fixed __builtin_ctz to return the correct thing for zero, but the
    // same is not true for clang.
    return __builtin_ffs(HasKey(Bucket, Key)) - 1;
    //return _tzcnt_u32(HasKey(Bucket, Key)) - 1;
}

// LocateKey_PopCnt returns the matching slot number if sorted parital Bucket
// has Key or -1 if partial Bucket does not have Key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
LocateKey_PopCnt(v_t Bucket, unsigned char Key, int nPopCnt)
{
    // get (matching byte num + 1) or 0 if no match
    return __builtin_ffs(HasKey_PopCnt(Bucket, Key, nPopCnt)) - 1;
}

// HasKeyGe returns (-1 << matching slot number) & 0xffff)
// if sorted full Bucket has a key that is greater than or equal to Key
// or zero if Bucket does not have such a key.
// Keys must be sorted with lowest key at vector index zero.
int
HasKeyGe(v_t Bucket, unsigned char Key)
{
    v_t vGe = (v_t)(Bucket >= Key); // compare Key with all
    return _mm_movemask_epi8((__m128i)vGe); // (1 << matching slot) or 0
}

// HasKeyGe_PopCnt returns (-1 << matching slot number) & 0xffff)
// if sorted partial Bucket has a key that is greater than or equal to Key
// or zero if partial Bucket does not have such a key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
HasKeyGe_PopCnt(v_t Bucket, unsigned char Key, int nPopCnt)
{
    int n = HasKeyGe(Bucket, Key);
    n &= (1 << nPopCnt) - 1;
    return n;
}

// Use LocateSlot if we know the bucket does not have the key
// but we need to know where the key belongs.
int
LocateSlot(v_t Bucket, unsigned char Key)
{
    return (__builtin_ffs(HasKeyGe(Bucket, Key)) + 16) % 17;
}

// Use LocateSlot_PopCnt if we know the bucket does not have the key
// but we need to know where the key belongs.
int
LocateSlot_PopCnt(v_t Bucket, unsigned char Key, int nPopCnt)
{
    // ffs ==> result: { 0, 1, ..., 16 } ==> { 16, 0, ..., 15 }
    return (__builtin_ffs(HasKeyGe_PopCnt(Bucket, Key, nPopCnt)) + 16) % 17;
}

// Search returns the matching slot number if sorted full Bucket
// has Key or ~(slot number of first slot with key greater than Key)
// if Bucket has such a key or ~16 if Bucket has no such key.
// Keys must be sorted with lowest key at vector index zero.
int
Search(v_t Bucket, unsigned char Key)
{
    // get (matching byte num + 1) or 0 if no match
    int n = __builtin_ffs(HasKeyGe(Bucket, Key));
    // 0 => ~16
    // 1 => 0 if there and ~0 if not there
    // 2 => 1 if there and ~1 if not there
    // 4 => 2 if there and ~2 if not there
    // 0x8000 => 15 if there and ~15 if not there
#ifdef METHOD_TEST // Has je with gcc, but short branch forward.
    int z = (n == 0);
    --n; n = z ? ~(int)sizeof(v_t) : n ^ ((Bucket[n] == Key) - 1);
#elif defined(METHOD_LS)
    int eq = __builtin_ffs(HasKey(Bucket, Key));
    return eq ? eq - 1 : ~LocateSlot(Bucket, Key);
#elif defined(METHOD_DECR)
    int z = (n == 0); // key is greater than all others in the bucket
    n = (z - 1) & (n - 1); // slot or 0
    n ^= ((Bucket[n] == Key) - 1); // slot or ~slot or 0
    n ^= ~(z - 1) & nPopCnt; // slot or ~slot or ~nPopCnt
#else // METHOD_MASK // One instruction less than METHOD_DECR with gcc.
    int nz = (n != 0); // bucket has a greater or equal key
    --n; n ^= (Bucket[n & (sizeof(v_t) - 1)] == Key) - 1;
    n |= (nz - 1) & ~sizeof(v_t);
#endif
    return n;
}

// Search_PopCnt returns the matching slot number if sorted parital Bucket
// has Key or ~(slot number of first slot with key greater than Key)
// if partial Bucket has such a key or ~16 if partial Bucket has no such key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
Search_PopCnt(v_t Bucket, unsigned char Key, int nPopCnt)
{
    // get (matching byte num + 1) or 0 if no match
    int n = __builtin_ffs(HasKeyGe_PopCnt(Bucket, Key, nPopCnt));
#ifdef METHOD_TEST // has je with gcc, but short branch forward
    int z = (n == 0);
    --n; n = z ? ~nPopCnt : n ^ ((Bucket[n] == Key) - 1);
#elif defined(METHOD_LS)
    int eq = __builtin_ffs(HasKey_PopCnt(Bucket, Key, nPopCnt));
    return eq ? eq - 1 : ~LocateSlot_PopCnt(Bucket, Key, nPopCnt);
#elif !defined(METHOD_DECR)
    int z = (n == 0); // key is greater than all others in the bucket
    n = (z - 1) & (n - 1); // slot or 0
    n ^= ((Bucket[n] == Key) - 1); // slot or ~slot or 0
    n ^= ~(z - 1) & nPopCnt; // slot or ~slot or ~nPopCnt
#else // METHOD_MASK // Doesn't work for LocateSlot_PopCnt.
    int nz = (n != 0); // partial bucket has a greater or equal key
    --n; n ^= (Bucket[n & (sizeof(v_t) - 1)] == Key) - 1;
    n |= (nz - 1) & ~nPopCnt;
#endif

    return n;
}

typedef size_t Word_t;

#define MSK(_x) (((Word_t)1 << _x) - 1)

#ifdef PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK

static int // nPos
LocateGeKey8InEk64(Word_t wRoot, Word_t wKey)
{
    // convert 16-bit unsigned integers in wRoot to 32-bit signed integers
    wRoot = ~MSK(56) | (wRoot >> 8);
    __m128i m128Zero = _mm_set_epi64x(0, 0);
    __m128i m128Root = _mm_unpacklo_epi8(_mm_set_epi64x(0, wRoot), m128Zero);
    __m128i m128Key = _mm_set1_epi16((uint8_t)wKey);
    __m128i m128Gt = _mm_cmpgt_epi16(m128Root, m128Key);
    __m128i m128Eq = _mm_cmpeq_epi16(m128Root, m128Key);
    __m128i m128GE = m128Gt | m128Eq;
    uint64_t u64GE = _mm_packs_epi16(m128GE, /* don't care */ m128GE)[0];
    return __builtin_ctzll(u64GE) / 8;
}

#else // PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK

#define _mm_cmpge_pu8(a, b) \
    _mm_cmpeq_pi8(_mm_max_pu8(a, b), a)

static int // nPos
LocateGeKey8InEk64(Word_t wRoot, Word_t wKey)
{
    __m64 m64List = (__m64)(~MSK(56) | (wRoot >> 8));
    __m64 m64Key = _mm_set1_pi8(wKey);
    return __builtin_ctzll((uint64_t)_mm_cmpge_pu8(m64List, m64Key)) / 8;
}

#endif // PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK

// https://stackoverflow.com/questions/32945410
//     /sse2-intrinsics-comparing-unsigned-integers/33302457#33302457
inline __m128i NotEqual8u(__m128i a, __m128i b)
{
    return _mm_andnot_si128(_mm_cmpeq_epi8(a, b), _mm_set1_epi8(-1));
}
inline __m128i Greater8u(__m128i a, __m128i b)
{
    return _mm_andnot_si128(_mm_cmpeq_epi8(_mm_min_epu8(a, b), a),
                            _mm_set1_epi8(-1));
}
inline __m128i GreaterOrEqual8u(__m128i a, __m128i b)
{
    return _mm_cmpeq_epi8(_mm_max_epu8(a, b), a);
}
inline __m128i Lesser8u(__m128i a, __m128i b)
{
    return _mm_andnot_si128(_mm_cmpeq_epi8(_mm_max_epu8(a, b), a),
                            _mm_set1_epi8(-1));
}
inline __m128i LesserOrEqual8u(__m128i a, __m128i b)
{
    return _mm_cmpeq_epi8(_mm_min_epu8(a, b), a);
}

// Return the position of the least significant 16-bit key greater than or
// equal to the low 16 bits of wKey in a 64-bit wRoot.
// Ignore the least significant slot in wRoot and start counting with zero
// at the next least significant slot.
// If there is no key greater than or equal to then return three.
// _mm_cmpgt_pi16 compares signed 16-bit numbers in an __m64 for a > b.
// There is no instruction that compares unsigned 16-bit numbers in an __m64.
int // nPos
LocateGeKey16InEk64(Word_t wRoot, Word_t wKey)
{
    // convert 16-bit unsigned integers in wRoot to 32-bit signed integers
    wRoot = ~MSK(48) | (wRoot >> 16);
    __m128i m128Zero = _mm_set_epi64x(0, 0);
    __m128i m128Root = _mm_unpacklo_epi16(_mm_set_epi64x(0, wRoot), m128Zero);
    __m128i m128Key = _mm_set1_epi32((uint16_t)wKey);
    __m128i m128gt = _mm_cmpgt_epi32(m128Root, m128Key);
    __m128i m128eq = _mm_cmpeq_epi32(m128Root, m128Key);
    __m128i m128ge = m128gt | m128eq;
    uint64_t u64ge = _mm_packs_epi32(m128ge, /* don't care */ m128ge)[0];
    return __builtin_ctzll(u64ge) / 16;
}

v_t v = { 0xf1, 0xf3, 0xf5, 0xf7, 0xf9, 0xfd, 0xfe, 0xff,
          0x00, 0x01, 0x02, 0x06, 0x08, 0x0a, 0x0c, 0x0e };

unsigned char BucketBuffer[17] = {
    0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
    0x01, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 0x0f, 0x11 };

int
main(int argc, char **argv)
{
    Word_t wRoot = strtoull(argv[1], 0, 0);
    Word_t wKey = strtoull(argv[2], 0, 0);
    printf("wRoot 0x%zx\n", wRoot);
    printf("wKey 0x%zx\n", wKey);
    int nPos = LocateGeKey16InEk64(wRoot, wKey);
    printf("LocateGeKey16InEk64 %d\n", nPos);
    nPos = LocateGeKey8InEk64(wRoot, wKey);
    printf("LocateGeKey8InEk64 %d\n", nPos);
    exit(0);

    unsigned char cKey = strtoul(argv[1], 0, 0);

    v_t *pBucket = (v_t *)BucketBuffer;
    v_t *pUaBucket = (v_t *)&BucketBuffer[1];

    printf("HK(*pBucket, cKey) 0x%x\n", HasKey(*pBucket, cKey));
    printf("HK((v_t)_mm_loadu_si128((__m128i *)pUaBucket), cKey) 0x%x\n",
           HasKey((v_t)_mm_loadu_si128((__m128i *)pUaBucket), cKey));

    printf("\n");
    printf("HK(v, cKey) 0x%x\n", HasKey(v, cKey));
    printf("\n");
    for (int nPopCnt = 16; nPopCnt > 0; --nPopCnt) {
        if ((1 << nPopCnt) & 0x18306) {
            printf("PHK(v, cKey, nPopCnt %d) 0x%x\n",
                   nPopCnt, HasKey_PopCnt(v, cKey, nPopCnt));
        }
    }

    printf("\n");
    printf("LK(v, cKey) %d\n", LocateKey(v, cKey));
    printf("\n");
    for (int nPopCnt = 16; nPopCnt > 0; --nPopCnt) {
        if ((1 << nPopCnt) & 0x18306) {
            printf("LKP(v, cKey, nPopCnt %d) %d\n",
                   nPopCnt, LocateKey_PopCnt(v, cKey, nPopCnt));
        }
    }

    int n;

    printf("\n");
    n = LocateSlot(v, cKey);
    printf("LS(v, cKey) %d", n);
    if (n < 0) { printf(" ==> %d", ~n); } 
    printf("\n");
    printf("\n");
    for (int nPopCnt = 16; nPopCnt > 0; --nPopCnt) {
        if ((1 << nPopCnt) & 0x18306) {
            n = LocateSlot_PopCnt(v, cKey, nPopCnt);
            printf("LSP(v, cKey, nPopCnt %d) %d", nPopCnt, n);
            if (n < 0) { printf(" ==> %d", ~n); } 
            printf("\n");
        }
    }

    printf("\n");
    n = Search(v, cKey);
    printf("S(v, cKey) %d", n);
    if (n < 0) { printf(" ==> %d", ~n); } 
    printf("\n");
    printf("\n");
    for (int nPopCnt = 16; nPopCnt > 0; --nPopCnt) {
        if ((1 << nPopCnt) & 0x18306) {
            n = Search_PopCnt(v, cKey, nPopCnt);
            printf("SP(v, cKey, nPopCnt %d) %d", nPopCnt, n);
            if (n < 0) { printf(" ==> %d", ~n); } 
            printf("\n");
        }
    }
}
