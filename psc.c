
#include <immintrin.h> // _mm_movemask_epi8, __m128i

// _mm_loadu_si128 is SSE2
// _mm_lddqu_si128 is SSE3
// _mm_lddqu_si128 "may perform better than _mm_loadu_si128 when the data
// crosses a cache line boundary".

// v_t is a vector of 16 chars. __m128i is a vector of 2 long longs.
// We need the char variant so we can compare with a char using '==' or '>='.
#ifdef __clang__
// clang has some support for gcc attribute "vector_size" but it doesn't work
// as well as its own ext_vector_type.
// For example, it won't promote a scalar to a vector for compare.
typedef char __attribute__((ext_vector_type(16))) v_t;
#else // __clang__
// gcc has no support for clang attribute "ext_vector_type".
typedef char __attribute__((vector_size(16))) v_t;
#endif // __clang__

// HasKey tests to see if Bucket has Key.
// HasKey returns (1 << matching slot number) if sorted full Bucket
// has Key or zero if Bucket does not have Key.
// Keys must be sorted with lowest key at vector index zero.
int
HasKey(v_t Bucket, char Key)
{
    v_t vEq = (Bucket == Key);
    return _mm_movemask_epi8((__m128i)vEq); // (1 << matching slot) or 0
}

// HasKey_Mask uses the zero bits in Mask to specify which slots to ignore.
int
HasKey_Mask(v_t Bucket, char Key, int Mask)
{
    return HasKey(Bucket, Key) & Mask;
}

// HasKey_PopCnt tests to see if sorted partial Bucket has Key.
// HasKey_PopCnt ignores slots past the first PopCnt slots.
// HasKey_PopCnt returns (1 << matching slot number) if sorted partial Bucket
// has Key or zero if partial Bucket does not have Key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
HasKey_PopCnt(v_t Bucket, char Key, int nPopCnt)
{
    return HasKey_Mask(Bucket, Key, ~(-1 << nPopCnt));
}

// HasKey_Start_Len test to see if sorted partial Bucket has Key.
// HasKey_Start_Len ignores slots before Start and after Start plus Len.
// HasKey_PopCnt returns (1 << matching absolute slot number) if sorted
// partial Bucket has Key or zero if partial Bucket does not have Key.
// Relevant keys must be packed and sorted with lowest key at Start.
int
HasKey_Start_Len(v_t Bucket, char Key, int nStart, int nLen)
{
    return HasKey_Mask(Bucket, Key, ~(-1 << nLen) << nStart);
}

// LocateKey returns the matching slot number if sorted full Bucket
// has Key or -1 if Bucket does not have Key.
// Keys must be sorted with lowest key at vector index zero.
int
LocateKey(v_t Bucket, char Key)
{
    // get (matching byte num + 1) or 0 if no match
    return __builtin_ffsll(HasKey(Bucket, Key)) - 1;
}

// LocateKey_PopCnt returns the matching slot number if sorted parital Bucket
// has Key or -1 if partial Bucket does not have Key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
LocateKey_PopCnt(v_t Bucket, char Key, int nPopCnt)
{
    // get (matching byte num + 1) or 0 if no match
    return __builtin_ffsll(HasKey_PopCnt(Bucket, Key, nPopCnt)) - 1;
}

// HasGeKey returns (-1 << matching slot number) & 0xffff)
// if sorted full Bucket has a key that is greater than or equal to Key
// or zero if Bucket does not have such a key.
// Keys must be sorted with lowest key at vector index zero.
int
HasGeKey(v_t Bucket, char Key)
{
    v_t vGe = (Bucket >= Key); // compare Key with all
    return _mm_movemask_epi8((__m128i)vGe); // (1 << matching slot) or 0
}

// HasGeKey_PopCnt returns (-1 << matching slot number) & 0xffff)
// if sorted partial Bucket has a key that is greater than or equal to Key
// or zero if partial Bucket does not have such a key.
// Keys must be packed and sorted with lowest key at vector index zero.
int
HasGeKey_PopCnt(v_t Bucket, char Key, int nPopCnt)
{
    int n = HasGeKey(Bucket, Key);
    n &= (1 << nPopCnt) - 1;
    return n;
}

// Use LocateSlot if we know the bucket does not have the key
// but we need to know where the key belongs.
int
LocateSlot(v_t Bucket, char Key)
{
    return (__builtin_ffsll(HasGeKey(Bucket, Key)) + 16) % 17;
}

// Use LocateSlot_PopCnt if we know the bucket does not have the key
// but we need to know where the key belongs.
int
LocateSlot_PopCnt(v_t Bucket, char Key, int nPopCnt)
{
    // ffs ==> result: { 0, 1, ..., 16 } ==> { 16, 0, ..., 15 }
    return (__builtin_ffsll(HasGeKey_PopCnt(Bucket, Key, nPopCnt)) + 16) % 17;
}

// Search returns the matching slot number if sorted full Bucket
// has Key or ~(slot number of first slot with key greater than Key)
// if Bucket has such a key or ~16 if Bucket has no such key.
// Keys must be sorted with lowest key at vector index zero.
int
Search(v_t Bucket, char Key)
{
    // get (matching byte num + 1) or 0 if no match
    int n = __builtin_ffsll(HasGeKey(Bucket, Key));
    // 0 => ~16
    // 1 => 0 if there and ~0 if not there
    // 2 => 1 if there and ~1 if not there
    // 4 => 2 if there and ~2 if not there
    // 0x8000 => 15 if there and ~15 if not there
#ifdef METHOD_TEST // Has je with gcc, but short branch forward.
    int z = (n == 0);
    --n; n = z ? ~(int)sizeof(v_t) : n ^ ((Bucket[n] == Key) - 1);
#elif defined(METHOD_LS)
    int eq = __builtin_ffsll(HasKey(Bucket, Key));
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
Search_PopCnt(v_t Bucket, char Key, int nPopCnt)
{
    // get (matching byte num + 1) or 0 if no match
    int n = __builtin_ffsll(HasGeKey_PopCnt(Bucket, Key, nPopCnt));
#ifdef METHOD_TEST // has je with gcc, but short branch forward
    int z = (n == 0);
    --n; n = z ? ~nPopCnt : n ^ ((Bucket[n] == Key) - 1);
#elif defined(METHOD_LS)
    int eq = __builtin_ffsll(HasKey_PopCnt(Bucket, Key, nPopCnt));
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

#include <stdio.h>

v_t v = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32 };

char BucketBuffer[17] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32
};

int
main(int argc, char **argv)
{
    char cKey = strtoul(argv[1], 0, 0);

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
