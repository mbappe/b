
// @(#) $Id: bli.c,v 1.519 2014/12/26 01:08:07 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/bli.c,v $

//#include <emmintrin.h>
//#include <smmintrin.h>
#include <immintrin.h> // __m128i

#if defined(PARALLEL_128)

#define BUCKET_HAS_KEY(_pxBucket, _wKey, _nBL) \
    ((sizeof(*(_pxBucket)) == sizeof(Word_t)) \
        ? WordHasKey((void *)(_pxBucket), (_wKey), (_nBL)) \
        : HasKey128((void *)(_pxBucket), (_wKey), (_nBL)))

#else // defined(PARALLEL_128)

#define BUCKET_HAS_KEY(_pxBucket, _wKey, _nBL) \
    WordHasKey((_pxBucket), (_wKey), (_nBL))

#endif // defined(PARALLEL_128)

#if defined(TRY_MEMCHR)
#include <wchar.h>
#endif // defined(TRY_MEMCHR)

// This file is #included in other .c files three times.
// Once with #define LOOKUP, #undef INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #define INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #undef INSERT and #define REMOVE.

// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#if (cnDigitsPerWord > 1)
//#if (cnBitsPerDigit < cnBitsPerWord)
//#if (cnBitsInD1 < cnBitsPerWord)

#if defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Success)
#if defined(LOOKUP)
#define strLookupOrInsertOrRemove  "Lookup"
#define DBGX  DBGL
#else // defined(REMOVE)
#define strLookupOrInsertOrRemove  "Remove"
#define DBGX  DBGR
#define InsertRemove  Remove
#if defined(RECURSIVE_REMOVE)
#define RECURSIVE
#endif // defined(RECURSIVE_REMOVE)
#endif // defined(REMOVE)
#else // defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Failure)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#define InsertRemove  Insert
#if defined(RECURSIVE_INSERT)
#define RECURSIVE
#endif // defined(RECURSIVE_INSERT)
#endif // defined(LOOKUP) || defined(REMOVE)

#if (cwListPopCntMax != 0)

// Simple forward linear search of a sub-list.
// It assumes the list contains a key that is greater than or equal to _xKey.
// _pxKeys is the beginning of the whole list.
// The search starts at _pxKeys[_nPos].
#define SSEARCHF(_pxKeys, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); } \
    if ((_pxKeys)[_nPos] > (_xKey)) { (_nPos) ^= -1; } \
}

// Simple backward linear search of a sub-list.
// It assumes the list contains a key that is less than or equal to _xKey.
// _pxKeys is the beginning of the whole list.
// The search starts at _pxKeys[_nPos].
#define SSEARCHB(_pxKeys, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] > (_xKey)) { --(_nPos); } \
    if ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); (_nPos) ^= -1; } \
}

#if defined(LIST_END_MARKERS)

  #if defined(TEST_PAST_END)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] >= &(_pxKeys)[_nPopCnt])

#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] < (_pxKeys))

  #elif defined(TEST_SPLIT_EQ_KEY)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  ((_pxKeys)[nSplit] == (_xKey))

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #else // TEST_KEY_IS_MAX_MIN

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

// BUG: Refine this with nBL; this won't work for non-native sizes as it is.
#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey) \
    ((_xKey) == (_x_t)-1)

#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  ((_xKey) == 0)

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #endif // ...

// Simple forward linear search of a sub-list.
// It assumes the key past the end of the list is a marker equal to the
// the maximum key so there is no need to do the bounds
// check until we've found a key that is greater than or equal to _xKey.
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); } \
    if (((_pxKeys)[_nPos] > (_xKey)) \
        || PAST_ENDF(((_pxKeys) + (_nPos)), _nPopCnt, _pxKeys, _nPos)) \
    { \
        (_nPos) ^= -1; \
    } \
}

// Simple backward linear search of a sub-list.
// It assumes the key before the beginning of the list is zero so there is no
// need to do the bounds check until we've found a key that is less than or
// equal to _xKey.
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos + _nPopCnt - 1].
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    while ((_xKey) < (_pxKeys)[_nPos]) { --(_nPos); } \
    if (((_xKey) > (_pxKeys)[_nPos]) \
        || PAST_ENDB(((_pxKeys) + (_nPos)), _pxKeys, _nPos)) \
    { \
        ++(_nPos); (_nPos) ^= -1; \
    } \
}

#else // defined(LIST_END_MARKERS)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

// Forward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHFX(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_pxKeys)[(_nPos) + (_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SSEARCHF((_pxKeys), (_xKey), (_nPos)); \
    } \
}

//#define TRY_MEMCHR
#if defined(TRY_MEMCHR)

// Backward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if (sizeof(_x_t) == sizeof(unsigned char)) { \
        _x_t *px; \
        px = (_x_t *)memchr(((_pxKeys) + (_nPos)), (_xKey), (_nPopCnt)); \
        (_nPos) = (px == NULL) ? -1 : px - (_pxKeys); \
    } else if (sizeof(_x_t) == sizeof(wchar_t)) { \
        _x_t *px; \
        px = (_x_t *)wmemchr((wchar_t *)((_pxKeys) + (_nPos)), \
                                         (_xKey), (_nPopCnt)); \
        (_nPos) = (px == NULL) ? -1 : px - (_pxKeys); \
    } else { \
        SEARCHFX(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos); \
    } \
}

#else // defined(TRY_MEMCHR)

#define SEARCHF(_x_t, _pxKeys0, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_pxKeys0)[(_nPos) + (_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SSEARCHF((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

#endif // defined(TRY_MEMCHR)

// Backward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos + _nPopCnt - 1].
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_xKey) < (_pxKeys)[_nPos]) { \
        (_nPos) ^= -1; \
    } else { \
        (_nPos) += (_nPopCnt) - 1; SSEARCHB((_pxKeys), (_xKey), (_nPos)); \
    } \
}

#endif // defined(LIST_END_MARKERS)

// Pick a bucket position directly rather than picking a key position first
// then deriving the bucket position from the key position.
// PSPLIT_NN picks a key position.
#define PSPLIT_P(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    (_nSplit) = (((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt) * sizeof(_xKey)) \
                    /* + ((_nPopCnt) * sizeof(_xKey) / 2) */ ) \
                    / sizeof(Word_t)) \
                >> 9); \
}

// One old method:
// nSplit = (((_xKey) & MSK(_nBL)) * (_nPopCnt) + (_nPopCnt) / 2) >> (_nBL);
// Current method:
// Take the (_nBL - _nn) most significant bits of the _nBL least significant
// bits in _xKey times _nPopCnt and divide by EXP(_nBL - _nn).
// If _nBL < _nn then shift _wKey left as needed.
// The rounding term is probably insignificant and unnecessary at least in
// some cases.
// The trick is finding an expression for _nn that will work for all values
// of _nBL.
#define PSPLIT_NN(_nPopCnt, _nBL, _xKey, _nn, _nSplit) \
{ \
    /* make sure we don't overflow when we shift _nPopCnt with big _nBL */ \
    assert((_nPopCnt) <= (1 << (cnBitsPerWord - (_nBL) + (_nn)))); \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord + (_nn) - (_nBL))) \
                        * (_nPopCnt)) \
                    + ((_nPopCnt) / 2)) \
                >> ((_nBL) - (_nn))); \
}

// I think this value for _nn may work for everything except:
//    ((_nBL == cnBitsPerWord) && (_nPopCnt == 1))
// But PSPLIT isn't effective when the set of values is concentrated
// at one end of the expanse.  For example, the top of a 64-bit tree
// and 32-bit values.
// We need to make sure we are passing a constant in for _nBL for the
// performance path cases so the compiler can simplify this.
#define PSPLIT(_nPopCnt, _nBL, _xKey, _nSplit) \
        PSPLIT_NN((_nPopCnt), (_nBL), (_xKey), \
                  LOG((((_nPopCnt) << 1) - 1)) - cnBitsPerWord + (_nBL), \
                  (_nSplit))

#if defined(SPLIT_SEARCH_BINARY)

  #define SPLIT(_nPopCnt, _nBL, _xKey, _nSplit)  ((_nSplit) = (_nPopCnt) >> 1)

#else // defined(SPLIT_SEARCH_BINARY)

  #define SPLIT(_nPopCnt, _nBL, _xKey, _nSplit) \
      PSPLIT(_nPopCnt, _nBL, _xKey, _nSplit)

#endif // defined(SPLIT_SEARCH_BINARY)


#if 0
pop <= 2 ^ (bpw - nbl + nn)
ceil(log(pop)) <= bpw - nbl + nn
LOG(pop * 2 - 1) <= bpw - nbl + nn
nn >= LOG(pop * 2 - 1) - bpw + nbl
nn  = LOG(pop * 2 - 1) - bpw + nbl
#endif

#if 0

#define SPLIT(_nPopCnt, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
{ \
    unsigned nBL = LOG(((_xKeyMin) ^ (_xKeyMax)) | 1) + 1; \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt)) \
                    /* + ((_nPopCnt) / 2) */ ) \
                / EXP(9)); \
}

#endif

#if 0

#define SPLIT(_nn, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
{ \
    (_nSplit) \
        = ((((Word_t)(_xKey) - (_xKeyMin)) * (_nn) + (_nn) / 2) \
            / ((Word_t)(_xKeyMax) - (_xKeyMin) + 1)); \
    if ((_nSplit) < 0) { (_nSplit) = 0; } \
    else if ((_nSplit) >= (_nn)) { (_nSplit) = (_nn) - 1; } \
}

#endif

#if defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

// Little endian:
// 00:      10: smallest key
// 02:    32  : next smallest key
// 04:  54
// 06:76
// 08:fedcba98
// 10:76543210
// 18:fedcba98

//   key               key
// | big              small|
// |fe:dc|ba:98|76:54|32:10|

// Simple linear parallel search of a list that assumes the list contains a
// key that is greater than or equal to the key we're searching for.
// Key size is some power of two bytes.
// It also assumes the last key in the last _b_t is equal to the largest
// key in the list even if it is beyond the population.
// _nPopCnt is relative to _nPos
#define PSSEARCHF(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* starting _b_t */ \
    Word_t wEnd = (Word_t)&(_pxKeys)[_nPos + _nPopCnt]; \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key in starting _b_t */ \
    (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; \
    while ( ! BUCKET_HAS_KEY(px, (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the last key in the _b_t to see if we've gone too far */ \
        if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; } \
        ++px; (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
        if ((Word_t)px >= wEnd) { (_nPos) ^= -1; break; } \
    } \
}

#if defined(PSPLIT_EARLY_OUT)

// Simple backward linear parallel search of a list that assumes the list
// contains a key that is less than or equal to the key we're searching for.
// Key size is some power of two bytes.
#define PSSEARCHB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* starting _b_t */ \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in starting _b_t */ \
    Word_t wEnd = (Word_t)&(_pxKeys)[_nPos]; \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
        if ((Word_t)&px[nxPos] < wEnd) { (_nPos) = -1; break; } \
    } \
}

#if 0
// Amazingly, the variant above was the best performing in my tests.
#define PSSEARCHB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    _b_t *px = (_b_t *)(_pxKeys); \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    do { \
        if ((_xKey) >= (_pxKeys)[_nPos]) { \
            if (!BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8)) { \
                (_nPos = -1); \
            } \
            break; \
        } \
    } while (((_nPos) -= sizeof(_b_t) / sizeof(_xKey)), (nxPos-- >= 0)); \
}
#define PSSEARCHB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    _b_t *px = (_b_t *)(_pxKeys); \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if (((_xKey) > (_pxKeys)[_nPos]) || (nxPos-- == 0)) { \
            (_nPos) = -1; break; \
        } \
        (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
    } \
}
#endif // 0

#else // defined(PSPLIT_EARLY_OUT)

// Can't have a non-PSPLIT_EARLY_OUT version of PSSEARCHF because because
// we don't know where the end of the list is. 
// #define PSSEARCHF(_b_t, _pxKeys, _xKey, _nPos, _xKeySplit, _xKeyEnd)

// Simple backward linear parallel search of a list that assumes the list
// contains a key that is less than or equal to the key we're searching for.
// Key size is some power of two bytes.
#define PSSEARCHB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* starting _b_t */ \
    int nxPos = (_nPos) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in starting _b_t */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check to see if we've reached the beginning of the list */ \
        if (nxPos <= 0) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
    } \
}

#endif // defined(PSPLIT_EARLY_OUT)

#if defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeys0, _nPos) \
{ \
    SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos); \
}

// Backward linear search of list (for any size key and with end check).
#define PSEARCHB(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
    SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
}

#else // defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
/* Is it wise to check the end here ? */ \
/* Or should we consider a search that checks if we're too far each time? */ \
    _x_t xKeyEnd = (_pxKeys)[(_nPos) + (_nPopCnt) - 1]; \
/* now we know the value of the key at the start and end of the range */ \
    if (xKeyEnd < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        PSSEARCHF(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
/*PSPLIT_SEARCH_RANGE(_xKey, _pxKeys, _nPopCnt, _xKeySplit, xKeyEnd, _nPos)*/ \
    } \
}

// Backward linear parallel search (for any size key and with end check).
#define PSEARCHB(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
/* Is it wise to check the start here ? */ \
/* Or should we consider a search that checks if we're too far each time? */ \
    _x_t xKey0 = (_pxKeys)[_nPos]; \
/* now we know the value of the key at the start and end of the range */ \
    if ((_xKey) < xKey0) { \
        (_nPos) ^= -1; \
    } else { \
        /*(_nPos) += (_nPopCnt) - 1;*/ \
        PSSEARCHB(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
    } \
}

#endif // defined(PSPLIT_HYBRID)

// Split search with a parallel search of the bucket at the split point.
// A bucket is a Word_t or an __m128i.  Or whatever else we decide to pass
// into _b_t in the future.
// nSplit is a bucket number.
// We need a function we can call iteratively.  The position returned
// ultimately must be relative to the original beginning of the list.
// What parameters must we pass?
// We have to pass the following information:
// - First key/bucket of the sub-list to search: _pxKeys.
// - Number of keys/buckets in the sub-list to search: _nPopCnt. 
// - What is the offset within the full list of the sub-list to search: _nPos.
// We'd also like to be able to pass information about the key at the
// beginning and/or end of the list if we already know either or both of them.
// Can we use 0 and -1 for cases where we don't know them?  It might be
// more efficient to have separate macros for all cases.  Or maybe just
// a special case for neither is known.
#define SPLIT_SEARCH_GUTS(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    _b_t *px = (_b_t *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    unsigned nSplit; SPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(_b_t); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    if (BUCKET_HAS_KEY(&px[nSplitP], (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
    } \
    else \
    { \
        nSplit = nSplitP * sizeof(_b_t) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
/* now we know the value of a key in the middle */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(_b_t)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* search the tail of the list */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(_b_t) / sizeof(_x_t); \
                PSSEARCHF(_b_t, (_xKey), \
                          (_pxKeys), (_nPopCnt) - (_nPos), (_nPos)); \
            } \
        } \
        else \
        { \
            /* search the head of the list */ \
            if (nSplitP == 0) { \
                /* we searched the first bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                PSSEARCHB(_b_t, (_xKey), (_pxKeys), nSplit, (_nPos)); \
            } \
        } \
        assert(((_nPos) < 0) \
            || BUCKET_HAS_KEY((_b_t *) \
                                  ((Word_t)&(_pxKeys)[_nPos] \
                                      & ~MSK(LOG(sizeof(_b_t)))), \
                              (_xKey), sizeof(_x_t) * 8)); \
        /* everything below is just assertions */ \
        if ((_nPos) < 0) { \
            /* assert(~(_nPos) <= (int)(_nPopCnt)); not true */ \
            assert((~(_nPos) == (int)(_nPopCnt)) \
                    || (~(_nPos == 0)) \
                    || (~(_nPos) \
                        < (int)((_nPopCnt + sizeof(_b_t) - 1) \
                            & ~MSK(sizeof(_b_t))))); \
            for (int ii = 0; ii < (_nPopCnt); \
                 ii += sizeof(_b_t) / sizeof(_xKey)) \
            { \
                assert( ! BUCKET_HAS_KEY((_b_t *)&(_pxKeys)[ii], \
                          (_xKey), sizeof(_x_t) * 8) ); \
            } \
        } \
    } \
}

#if 0
static int
PSplitSearch16(int nBL,
               uint16_t *psKeys, int nPopCnt, uint16_t sKey, int nPos)
{
again:
    assert(nPopCnt > 0);
    assert(nPos >= 0); assert((nPos & ~MSK(sizeof(Bucket_t))) == 0);

    int nSplit; SPLIT(nPopCnt - nPos, nBL, sKey, nSplit);
    // nSplit is a portion of (nPopCnt - nPos)
    assert(nSplit >= 0); assert(nSplit < (nPopCnt - nPos));
    nSplit &= ~MSK(sizeof(Bucket_t)); // first key in bucket
    nSplit += nPos; // make relative to psKeys

    if (BUCKET_HAS_KEY((Bucket_t *)&psKeys[nSplit], sKey, sizeof(sKey) * 8)) {
        return 0; // key exists, but we don't know the exact position
    }

    uint16_t sKeySplit = psKeys[nSplit];
    if (sKey > sKeySplit)
    {
        // bucket number of split
        int nSplitP = nSplit * sizeof(sKey) / sizeof(Bucket_t);
        int nSplitPLast = (nPopCnt - 1) * sizeof(sKey) / sizeof(Bucket_t);
        if (nSplitP == nSplitPLast) {
            // we searched the last bucket and the key is not there
            return -1; // we don't know where to insert
        }
        nPos = (int)nSplit + sizeof(Bucket_t) / sizeof(sKey);
        goto again;
    }

    if (nSplit == nPos) { return -1; }

    nPopCnt = nSplit;
    goto again;
}
#endif

#if defined(PARALLEL_128)
#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    SPLIT_SEARCH_GUTS(__m128i, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) 
#else // defined(PARALLEL_128)
#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    SPLIT_SEARCH_GUTS(Word_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) 
#endif // defined(PARALLEL_128)

#else // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    int nSplit; SPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    if (TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)) \
    { \
        (_nPos) += nSplit; \
    } \
    else if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        if (nSplit == (_nPopCnt) - 1) \
        { \
            (_nPos) = ~((_nPos) + (_nPopCnt)); \
        } \
        else if (TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            (_nPos) += ((_pxKeys)[(_nPopCnt) - 1] == (_x_t)-1) \
                        ? (_nPopCnt) - 1 : ~(_nPopCnt); \
        } \
        else \
        { \
            (_nPos) = nSplit + 1; \
            SEARCHF(_x_t, (_pxKeys), (_nPopCnt) - nSplit - 1, \
                    (_xKey), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        if (TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            if ((_pxKeys)[0] != 0) { (_nPos) ^= -1; } \
        } \
        else \
        { \
            assert((_nPos) == 0); \
            SEARCHB(_x_t, (_pxKeys), nSplit + 1, (_xKey), (_nPos)); \
        } \
    } \
}

#endif // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if 0
static Status_t
TwoWordsHaveKey(Word_t *pw, Word_t wKey, unsigned nBL)
{
    Word_t wLsbs = (Word_t)-1 / (EXP(nBL) - 1); // lsb in each key
    Word_t wReplicatedKey = (wKey & MSK(nBL)) * wLsbs;
    Word_t awXor[2] = { wReplicatedKey ^ pw[0], wReplicatedKey ^ pw[1] };
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key
    int bHasKey = ((((awXor[0] - wLsbs) & ~awXor[0] & wMsbs) != 0)
                || (((awXor[1] - wLsbs) & ~awXor[1] & wMsbs) != 0));
    return bHasKey ? Success : Failure;
}
#endif

#if defined(COMPRESSED_LISTS)
#if defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

// Search an array of words packed with keys that are smaller than a word
// using a parallel search of each word.
// WordArrayHasKey expects the keys to be packed towards the most significant
// bits, and it assumes all slots in every word have valid keys, i.e. the
// would-be empty slots have been padded with copies of some key/keys that
// is/are present.
// It also assumes that keys do not cross word boundaries.
static Status_t
WordArrayHasKey(Word_t *pw, unsigned nWords, Word_t wKey, unsigned nBL)
{
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    unsigned nn = 0;
    do {
        Word_t wXor = wKeys ^ pw[nn]; // get zero in slot with matching key
        int bXorHasZero = (((wXor - wLsbs) & ~wXor & wMsbs) != 0); // magic
        if (bXorHasZero) { return Success; } // found the key
    } while (++nn < nWords);
    return Failure;
}

// Do a parallel search of a word for a key that is smaller than a word.
// WordHasKey expects the keys to be packed towards the most significant bits,
// and it assumes all slots in the word have valid keys, i.e. the would-be
// empty slots have been padded with copies of some key/keys that is/are
// present.
static Status_t
WordHasKey(Word_t *pw, Word_t wKey, unsigned nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    return WordArrayHasKey(pw, /* nWords */ 1, wKey, nBL);
}

#else // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

// There are a lot of ways we can represent a bucket.
// Which way will be fastest?
// Should we require that the keys in the list be sorted?  Why?
// Is has-key faster if we fill empty slots with a present key?
// What about offset?  Assumes key is present.
// What about has-key-and-offset?  No offset returned if key is not present.
// What about offset-of-missing-key?  Assumes key is not present.
// What about has-key-or-offset-of-missing-key?  No off ret if key is present.
// What about has-key-and-offset-or-offset-of-missing-key?
//
// If keys are sorted and key[n+1] < key[n] && k[n+1] != 0, then bucket is
// illegal and we could use that to mean the bucket is empty.  It only
// requires that buckets can always hold more than one key,
// i.e. key_size <= word_size / 2.
// The second half of the test isn't necessary if we fill empty slots
// with the biggest present key.
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
// empty slots are on most-significant end
// empty slots are on least-significant end
// empty slots are anywhere; can't imagine this helping search
//
// remainder bits are at the most-significant end
// remainder bits are at the least-significant end
//
// The value(s) we can use to represent an empty bucket depend
// on the choices made for the others.

// Do a parallel search of a word for a key that is smaller than a word.
// WordHasKey expects the keys to be packed towards the most significant bits,
// and it assumes all slots in the word have valid keys, i.e. the would-be
// empty slots have been padded with copies of some key/keys that is/are
// present.
// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.
static Word_t // bool
WordHasKey(Word_t *pw, Word_t wKey, unsigned nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    Word_t ww = *pw;
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask; // get rid of already-decoded bits
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
    return wMagic; // bXorHasZero = (wMagic != 0);
}

#endif // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

#if defined(PARALLEL_128)

#define HAS_KEY_128_SETUP(_wKey, _nBL, _xLsbs, _xMsbs, _xKeys) \
{ \
    Word_t wMask = MSK(_nBL); /* (1 << nBL) - 1 */ \
    _wKey &= wMask; \
    Word_t wLsbs = (Word_t)-1 / wMask; \
    _xLsbs = _mm_set1_epi64((__m64)wLsbs); \
    Word_t wMsbs = wLsbs << (nBL - 1); /* msb in each key slot */ \
    _xMsbs = _mm_set1_epi64((__m64)wMsbs); \
    Word_t wKeys = wKey * wLsbs; /* replicate key; put in every slot */ \
    _xKeys = _mm_set1_epi64((__m64)wKeys); \
}

static Word_t // bool
HasKey128Tail(__m128i *pxBucket,
    __m128i xLsbs,
    __m128i xMsbs,
    __m128i xKeys)
{
    __m128i xBucket = *pxBucket;
    __m128i xXor = xKeys ^ xBucket;
    __m128i xMagic = (xXor - xLsbs) & ~xXor & xMsbs;
    __m128i xZero = _mm_setzero_si128();
    return ! _mm_testc_si128(xZero, xMagic);
}

// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.
static Word_t // bool
HasKey128(__m128i *pxBucket, Word_t wKey, unsigned nBL)
{
    __m128i xLsbs, xMsbs, xKeys;
    HAS_KEY_128_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys); 
    return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
}

#endif // defined(PARALLEL_128)

#endif // defined(COMPRESSED_LISTS)

#endif // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)

#if defined(COMPRESSED_LISTS) && (cnBitsInD1 <= 8)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList8(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

  #if defined(PARALLEL_128) // sizeof(__m128i) == 16 bytes
    assert(ls_xPopCnt(pwr, 8) <= 16);
    int nPopCnt = 16; // Sixteen fit so why do less?
  #else // defined(PARALLEL_128)
      #if defined(PP_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, NULL, nBL);
      #else // defined(PP_IN_LINK)
    int nPopCnt = ls_xPopCnt(pwr, 8);
      #endif // defined(PP_IN_LINK)
  #endif // defined(PARALLEL_128)
    uint8_t *pcKeys = ls_pcKeysNATX(pwr, nPopCnt);

#if defined(LIST_END_MARKERS)
    assert(pcKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint8_t *)(((Word_t)&pcKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint8_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(pcKeys[nPopCnt] == (uint8_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint8_t cKey = (uint8_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_8)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 8) {
        PSPLIT_SEARCH(uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_SEARCH(uint8_t, nBL, pcKeys, nPopCnt, cKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_8)
    SEARCHB(uint8_t, pcKeys, nPopCnt, cKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint8_t, pcKeys, nPopCnt, cKey, nPos); (void)nBL;
#endif // ...
    return nPos;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsInD1 <= 8)

#if defined(COMPRESSED_LISTS) && (cnBitsInD1 <= 16)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList16(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

  #if defined(PP_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
  #else // defined(PP_IN_LINK)
  #if defined(PARALLEL_128) // sizeof(__m128i) == 16 bytes
      #if (cnListPopCntMax16 <= 8)
    int nPopCnt = 8; // Eight fit so why do less?
    assert((cnListPopCntMaxDl1 <= 8) || (cnBitsInD1 <= 8));
      #elif (cnBitsInD1 > 8) // nDL == 1 is handled here
          #if (cnListPopCntMaxDl1 <= 8) // list fits in one __m128i
              #if (cnBitsLeftAtDl2 <= 16) // need to test nDL
    int nPopCnt = (nBL == cnBitsInD1) ? 8 : ls_xPopCnt(pwr, 16);
              #else // (cnBitsLeftAtDl2 <= 16)
    int nPopCnt = 8; // Eight fit so why do less?
              #endif // (cnBitsLeftAtDl2 <= 16)
          #else // (cnListPopCntMaxDl1 <= 8)
    int nPopCnt = ls_xPopCnt(pwr, 16);
          #endif // (cnListPopCntMaxDl1 <= 8)
      #else // (cnListPopCntMax16 <= 8)
    int nPopCnt = ls_xPopCnt(pwr, 16);
      #endif // (cnListPopCntMax16 <= 8)
  #else // defined(PARALLEL_128)
    int nPopCnt = ls_xPopCnt(pwr, 16);
  #endif // defined(PARALLEL_128)
  #endif // defined(PP_IN_LINK)
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);

    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_16)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 16) {
        //nPos = PSplitSearch16(nBL, psKeys, nPopCnt, sKey, nPos);
        PSPLIT_SEARCH(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        //nPos = PSplitSearch16(nBL, psKeys, nPopCnt, sKey, nPos);
        PSPLIT_SEARCH(uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
#endif // ...
    return nPos;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsInD1 <= 16)

#if defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) \
    && (cnBitsInD1 <= 32)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList32(uint32_t *piKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // ! defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_32)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 32) {
        PSPLIT_SEARCH(uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBL == 24) {
        PSPLIT_SEARCH(uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_SEARCH(uint32_t, nBL, piKeys, nPopCnt, iKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#endif // ...
    return nPos;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) && ...

#endif // ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)
#endif // ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#endif // ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)

//
// Valid combinations:
// ==================
//
// (qty  2) [ratio-]split-loop-w-threshold=2
// (qty 80) no-split|[ratio-]split-w-no-loop|[ratio-]split-loop-w-threshold>2
//            x [no-]end-check
//            x (for|back)ward
//            x (continue|succeed|fail)-first|succeed-only
//
// split-loop-w-threshold=2 is a binary search
//
// no-split => no-split-loop
// ratio-split => split
// split-loop => split
// succeed-only <=> no-(continue|succeed|fail)-first
// continue-first => no-(succeed|fail)-first
// fail-first => no-(continue|succeed)-first
// succeed-first => no-(continue|fail)-first
// no-sort => no-split && no-end-check && succeed-only
//
// Common combinations:
// ===================
//
// default: no-split, no-end-check, succeed-only, forward
// split-loop-w-threshold=20, end-check, continue-first
//

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchListWord(Word_t *pwKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    DBGI(printf("SLW pwKeys %p wKey "OWx" nBL %d nPopCnt %d\n",
                (void *)pwKeys, wKey, nBL, nPopCnt));
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(pwKeys[-1] == 0);
    assert(pwKeys[nPopCnt] == (Word_t)-1);
#endif // defined(LIST_END_MARKERS)
    int nPos = 0;
#if defined(PSPLIT_SEARCH_WORD)
#if defined(PSPLIT_SEARCH_XOR_WORD)
    Word_t wKeyMin = pwKeys[0];
    Word_t wKeyMax = pwKeys[nPopCnt - 1];
    // Or in 1 to handle nPopCnt==1 else we'd be taking the LOG of zero.
    nBL = LOG((wKeyMin ^ wKeyMax) | 1) + 1;
    // nBL could be 64 and it could be 0.
#endif // defined(PSPLIT_SEARCH_XOR_WORD)
    if (nBL <= (cnBitsPerWord - 8)) {
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
        if (nBL == 32) {
            PSPLIT_SEARCH(Word_t, 32, pwKeys, nPopCnt, wKey, nPos);
        } else
#if (cnBitsPerWord > 32)
        if (nBL == 40) {
            PSPLIT_SEARCH(Word_t, 40, pwKeys, nPopCnt, wKey, nPos);
        } else
#else // (cnBitsPerWord > 32)
        if (nBL == 24) {
            PSPLIT_SEARCH(Word_t, 24, pwKeys, nPopCnt, wKey, nPos);
        } else
#endif // (cnBitsPerWord > 32)
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
        {
            PSPLIT_SEARCH(Word_t, nBL, pwKeys, nPopCnt, wKey, nPos);
        }
    } else { // here to avoid overflow
        unsigned nSplit
            = ((wKey & MSK(nBL)) >> 8) * nPopCnt + nPopCnt / 2 >> (nBL - 8);
        if (pwKeys[nSplit] < wKey) {
            if (nSplit == nPopCnt - 1) { return ~nPopCnt; }
            SEARCHF(Word_t, pwKeys, nPopCnt - nSplit - 1,
                       wKey, nPos);
        } else { // here if wKey <= pwKeys[nSplit]
            SEARCHB(Word_t, pwKeys, nSplit + 1, wKey, nPos);
        }
    }
#else // defined(PSPLIT_SEARCH_WORD)
    Word_t *pwKeysOrig = pwKeys;
    (void)nPos;
  #if defined(BINARY_SEARCH_WORD)
  // BINARY_SEARCH narrows the scope of the linear search that follows.
    unsigned nSplit;
    // Looks like we might want a loop threshold of 8 for
    // 64-bit keys at the top level.
    // And there's not much difference with threshold of
    // 16 or 64.
    // Not sure about 64-bit keys at a lower level or
    // 64-bit keys at the top level.
    while (nPopCnt >= cnBinarySearchThresholdWord)
    {
        nSplit = nPopCnt / 2;
        DBGI(printf("SLW nSplit %d\n", nSplit));
        if (pwKeys[nSplit] <= wKey) {
            pwKeys = &pwKeys[nSplit];
            nPopCnt -= nSplit;
        } else {
            nPopCnt = nSplit;
            if (nPopCnt == 0) {
                assert(~(pwKeys - pwKeysOrig) < 0);
                return ~(pwKeys - pwKeysOrig);
            }
        }
    }
  #endif // defined(BINARY_SEARCH_WORD)
    nPos = pwKeys - pwKeysOrig;
  #if defined(BACKWARD_SEARCH_WORD)
    SEARCHB(Word_t, pwKeysOrig, nPopCnt, wKey, nPos);
  #else // defined(BACKWARD_SEARCH_WORD)
    SEARCHF(Word_t, pwKeysOrig, nPopCnt, wKey, nPos);
  #endif // defined(BACKWARD_SEARCH_WORD)
#endif // defined(PSPLIT_SEARCH_WORD)
    DBGX(printf("SLW: return pwKeysOrig %p nPos %d\n", (void *)pwKeysOrig, nPos));
    return nPos;
}

#if 0
#define MAGIC1(_nBL)  MAXUINT / ((1 << (_nBL)) - 1)
#define MAGIC1(_nBL)  (cnMagic[_nBL])
#define MAGIC2(_nBL)  (MAGIC1(_nBL) << ((_nBL) - 1))
// Index cnMagic by nBitsLeft.
// Keys are stored by packing them at the high-order end of the word
// and leaving enough room for pop count and type and the low end.
Word_t cnMagic[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0x0010010010010000,
    0x0008004002001000,
    0x0004001000400100,
    0x0002000400080000,
};
#endif

// Magics for replicating keys and anding:
// Second one is first one shifted left by (nBitsLeft - 1).

// 6+6: 0x0010010010010000
// 6+6: 0x8008008008000000

// 6+7: 0x0008004002001000
// 6+7: 0x8004002001000000

// 6+8: 0x0004001000400100
// 6+8: 0x8002000800200000

// 6+9: 0x0002000400080000
// 6+9: 0x8001000200040000
//
// HasZero subtracts one from each which will leave all ones.
// And it complements the whole value (all ones) and ands it with high bit.
// Then it ands the two intermediate results.
//
// (abc - 001) & (ABC & 100)
// (a-b+C)(b-C)C    &  A00
//  000  111 & 100 => 100
//  001  000 & 100 => 000
//  010  001 & 100 => 000
//  011  010 & 100 => 000
//  100  011 & 000 => 000
//  101  100 & 000 => 000
//  110  101 & 000 => 000
//  111  110 & 000 => 000
#ifdef  TBD

#define repbyte1(s) (((-((Word_t)1))/255) * (s))
#define haszero1(v) (((v) - repbyte1(0x1)) & ~(v) & repbyte1(0x80))
#define hasvalue1(x,n) haszero1((x) ^ (n))

// For finding a zero byte in a Word_t may be usefull someday
// Note: untested
// #define haszero(v)        (((v) - 0x01010101UL) & ~(v) & 0x80808080UL)

// Does Word x have a byte == v
// #define hasvalue(x,n)     (haszero((x) ^ (~0UL/255 * (n))))

// For finding if Word_t has a byte == VALUE
#define haszero(VALUE)                                                  \
    ((VALUE) ((-((Word_t)1)) * 255) & ~(v) & (-((((Word_t)1) * 255) * 8)))

#define hasvalue(x,n) (haszero((x) ^ (-((Word_t)1))/255 * (n)))
#endif  // TBD

#if ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList(Word_t *pwr, Word_t wKey, unsigned nBL, Word_t *pwRoot)
{
    (void)pwRoot;

    DBGL(printf("SearchList\n"));

    int nPopCnt;
    int nPos;

  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
    if (nBL <= 8) {
        nPos = SearchList8(pwRoot, pwr, wKey, nBL);
    } else
      #endif // (cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBL <= 16) {
        nPos = SearchList16(pwRoot, pwr, wKey, nBL);
    } else
      #endif // (cnBitsInD1 <= 16)
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBL <= 32) {
          #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
          #else // defined(PP_IN_LINK)
        nPopCnt = ls_xPopCnt(pwr, 32);
          #endif // defined(PP_IN_LINK)
        nPos = SearchList32(ls_piKeysNATX(pwr, nPopCnt), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    {
  #if defined(SEARCH_FROM_WRAPPER)
      #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
      #else // defined(PP_IN_LINK)
        nPopCnt = ls_xPopCnt(pwr, cnBitsPerWord);
      #endif // ! defined(PP_IN_LINK)
        nPos = SearchListWord(ls_pwKeysNATX(pwr, nPopCnt),
                              wKey, nBL, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER)
      #if defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord) {
            nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
        } else
      #endif // ! defined(PP_IN_LINK)
        { nPopCnt = ls_xPopCnt(pwr, cnBitsPerWord); }
        nPos = SearchListWord(ls_pwKeys(pwr, nBL), wKey, nBL, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER)
    }

  #if defined(LOOKUP)
    SMETRICS(j__SearchPopulation += nPopCnt);
  #endif // defined(LOOKUP)

    return nPos;
}

#endif // ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)
#endif // ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#endif // ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)

#endif // (cwListPopCntMax != 0)

#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if (cwListPopCntMax != 0) && defined(EMBED_KEYS) && defined(EMBEDDED_KEYS_PARALLEL)

// Do a parallel search of a list embedded in a link given the key size.
// EmbeddedListHasKey expects the keys to be packed towards the most
// significant bits.
// The cnBitsMallocMask least-significant bits of the word are used for a
// type field and the next least-significant nBL_to_nBitsPopCntSz(nBL) bits
// of the word are used for a population count.
// It helps Lookup performance to eliminate the need to know nPopCnt.
// So, if PAD_T_ONE, we replicate the last key in the list into the unused
// slots at insert time to make sure the unused slots don't cause a false
// bXorHasZero.
// But how do we make sure the type and pop count bits don't
// cause a false bXorHasZero due to a slot that can't really be used?
// Or'ing MSK(nBL_to_nBitsPopCntSz(nBL) + cnBitsMallocMask)
// would be sufficient, but it may be expensive.
// Can we do something simpler/faster?  Something at insert time?
// Unfortunately, it doesn't matter how the pop and type bits are set
// in the word since we are xoring them with the key we're looking for
// before calculating bXorHasZero.  And whatever they are set to will
// match the key/keys that is/are the same.
// I wonder if the next best thing is to have a constant that we can
// or into wXor before calculating bXorHasZero.
// Does cnMallocMask work?  It will cover any key slot that extends
// into cnMallocMask.  But what about a slot that extends into the pop
// count field and not into cnMallocMask?
// Sure would be nice if we had a constant width pop field.  What would
// be the cost?  3-bits of pop for 64-bit costs one 29-bit key slot.
// 2-bits of pop for 32-bit costs one 14-bit key slot.
// If we're not using those key sizes, then there is no cost.
// What if we have no valid-key fill?  And no pop field?
static int // bool
EmbeddedListHasKey(Word_t wRoot, Word_t wKey, unsigned nBL)
{
#if defined(EMBEDDED_LIST_FIXED_POP)
    // Reminder about losing a slot with fixed-size pop field.
    assert((cnBitsPerWord != 32) || (nBL != 14));
    assert((cnBitsPerWord != 64) || (nBL != 29));
#endif // defined(EMBEDDED_LIST_FIXED_POP)
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask; // discard already-decoded bits
#if ! defined(PAD_T_ONE) && ! defined(T_ONE_MASK)
    // If we're filling empty slots with zero, then check for wKey == 0
    // here so we don't have to worry about a false positive later.
    // We still have to mask off the type and pop count bits from wXor later
    // but that is a constant.
    if (wKey == 0) { return ((wRoot >> (cnBitsPerWord - nBL)) == 0); }
#endif // ! defined(PAD_T_ONE) && ! defined(T_ONE_MASK)
    Word_t wLsbs = (Word_t)-1 / wMask;
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ wRoot; // get zero in slot with matching key
#if defined(PAD_T_ONE) || ! defined(T_ONE_MASK)
    wXor |= MSK(cnBitsMallocMask + nBL_to_nBitsPopCntSz(nBL)); // pop and type
#endif // defined(PAD_T_ONE) || ! defined(T_ONE_MASK)
// Looks like ! PAD_T_ONE, T_ONE_MASK (and ! EMBEDDED_LIST_FIXED_POP) is a
// bad combination.
#if ! defined(PAD_T_ONE) && defined(T_ONE_MASK)
    // If we're filling empty slots with zero, then we have to mask off
    // the empty slots so we don't get a false positive if/when wKey == 0.
#if defined(T_ONE_CALC_POP)
    Word_t ww = wRoot & ~MSK(cnBitsMallocMask + nBL_to_nBitsPopCntSz(nBL));
    ww |= (Word_t)1 << (cnBitsPerWord - 1);
    unsigned nPopCnt = ((cnBitsPerWord - __builtin_ffsll(ww)) / nBL) + 1;
#if 0
if (nPopCnt != wr_nPopCnt(wRoot, nBL)) {
    printf("nPopCnt %d wr %ld\n", nPopCnt, wr_nPopCnt(wRoot, nBL));
    printf("wRoot %lx ww %lx nBL %d\n", wRoot, ww, nBL);
}
#endif
#else // defined(T_ONE_CALC_POP)
    unsigned nPopCnt = wr_nPopCnt(wRoot, nBL); // number of keys present
#endif // defined(T_ONE_CALC_POP)
    unsigned nBitsOfKeys = nPopCnt * nBL;
    wXor |= (Word_t)-1 >> nBitsOfKeys; // type and empty slots
#endif // ! defined(PAD_T_ONE) && defined(T_ONE_MASK)
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    int bXorHasZero = (((wXor - wLsbs) & ~wXor & wMsbs) != 0); // magic
    return bXorHasZero;
}

#endif // (cwListPopCntMax != 0) && defined(EMBED_KEYS) && defined(EMBEDDED_KEYS_PARALLEL)
#endif // ! defined(LOOKUP_NO_LIST_DEREF)

#if defined(SKIP_LINKS)

static int
PrefixMismatch(Word_t *pwRoot, Word_t wRoot, Word_t *pwr, Word_t wKey,
               int nBL,
               int *pnBLR,
               Word_t **ppwRootPrefix,
               Word_t **ppwrPrefix,
               int *pnBLRPrefix,
               int *pbNeedPrefixCheck)
{
    (void)pwRoot; (void)pwr; (void)wKey; (void)nBL; (void)pnBLR;
    (void)ppwRootPrefix; (void)ppwrPrefix; (void)pnBLRPrefix;
    (void)pbNeedPrefixCheck;

  #if defined(TYPE_IS_RELATIVE)
    int nBLR = nDL_to_nBL_NAT(nBL_to_nDL(nBL) - wr_nDS(wRoot));
  #else // defined(TYPE_IS_RELATIVE)
    int nBLR = wr_nBL(wRoot);
  #endif // defined(TYPE_IS_RELATIVE)
    assert(nBLR < nBL); // reserved

  #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #if defined(SAVE_PREFIX)
        // Save info needed for prefix check at leaf.
        // Does this obviate the need for requiring a branch above the
        // bitmap as a place holder for the prefix check at the leaf?
        // It just might.
        // Maybe it's faster to use a word that is shared by all
        // than one that is shared by fewer.
          #if defined(PP_IN_LINK)
        *ppwRootPrefix = pwRoot;
          #else // defined(PP_IN_LINK)
        *ppwrPrefix = pwr;
          #endif // defined(PP_IN_LINK)
        *pnBLRPrefix = nBLR;
      #endif // defined(SAVE_PREFIX)
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        // Record that there were prefix bits that were not checked.
        *pbNeedPrefixCheck |= 1;
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #if ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
        int bPrefixMismatch = (1
            && ((int)LOG(1 | (PWR_wPrefixNATBL(pwRoot,
                                          (Switch_t *)pwr, nBLR) ^ wKey))
                    >= nBLR));
        if (bPrefixMismatch)
        {
            DBGX(printf("Mismatch wPrefix "Owx"\n",
                        PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwr, nBLR)));
            // Caller doesn't need/get an updated *pnBLR in this case.
            return 1; // prefix mismatch
        }
      #endif // ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)

    *pnBLR = nBLR;

    return 0; // no prefix mismatch
}

#endif // defined(SKIP_LINKS)

#if defined(LOOKUP)
static Status_t
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Status_t
InsertRemove(Word_t *pwRoot, Word_t wKey, int nBL)
#endif // defined(LOOKUP)
{
    int nBLUp; (void)nBLUp; // silence gcc
    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
#if defined(LOOKUP)
    int nBL = cnBitsPerWord;
    Word_t *pwRoot;
  #if defined(BM_IN_LINK)
    pwRoot = NULL; // used for top detection
  #else // defined(BM_IN_LINK)
          #if defined(PP_IN_LINK)
    // Silence unwarranted gcc used before initialized warning.
    // pwRoot is only uninitialized on the first time through the loop.
    // And we only use it if nBL != cnBitsPerWord
    // or if bNeedPrefixCheck is true.
    // And both of those imply it's not the first time through the loop.
    pwRoot = NULL;
          #endif // defined(PP_IN_LINK)
  #endif // defined(BM_IN_LINK)
#else // defined(LOOKUP)
    Word_t wRoot;
  #if !defined(RECURSIVE)
          #if defined(INSERT)
    int nIncr = 1;
          #else // defined(INSERT)
    int nIncr = -1;
          #endif // defined(INSERT)
  #endif // !defined(RECURSIVE)
#endif // defined(LOOKUP)
#if !defined(RECURSIVE)
  #if !defined(LOOKUP) || defined(BM_IN_LINK)
    Word_t *pwRootOrig = pwRoot; (void)pwRootOrig;
  #endif // !defined(LOOKUP) || defined(BM_IN_LINK)
#endif // !defined(RECURSIVE)
    int nBLR;
    Word_t wPopCnt; (void)wPopCnt;
#if ! defined(LOOKUP)
    int bCleanup = 0;
#endif // ! defined(LOOKUP)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrPrev = pwrPrev; // suppress "uninitialized" compiler warning
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    int nBLOrig = nBL; (void)nBLOrig;
    Word_t *pwRootPrefix = NULL; (void)pwRootPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    int nBLRPrefix = 0; (void)nBLRPrefix;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if !defined(LOOKUP)
  #if !defined(RECURSIVE)
top:
  #endif // !defined(RECURSIVE)
    wRoot = *pwRoot;
#endif // !defined(LOOKUP)
    nBLR = nBL;

#if defined(LOOKUP) || !defined(RECURSIVE)
again:
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if defined(SKIP_LINKS)
    assert(nBLR == nBL);
#endif // defined(SKIP_LINKS)
#if ( ! defined(LOOKUP) )
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
    DBGX(printf("# pwRoot %p ", (void *)pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nBL %d\n", wRoot, wKey, nBL));

    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
  #if defined(EXTRA_TYPES)
    switch (wRoot & MSK(cnBitsMallocMask + 1))
  #else // defined(EXTRA_TYPES)
    switch (nType)
  #endif // defined(EXTRA_TYPES)
    {

#if defined(SKIP_LINKS)

    default: // skip link
    // case T_SKIP_TO_SWITCH:
    {
        // pwr points to a switch

        if (PrefixMismatch(pwRoot, wRoot, pwr, wKey, nBL,
                           &nBLR, &pwRootPrefix,
                           &pwrPrefix, &nBLRPrefix, &bNeedPrefixCheck))
        {
            break;
        }

        // Logically, if we could arrange the source code accordingly,
        // we could just fall through to T_SWITCH.
        // But, with so many ifdefs, it is easier to use goto.
        // Is falling through to the next case faster than a goto?
        // Does the compiler turn a fall-through into a goto?
        // Does the compiler rearrange code to make gotos (in and/or out
        // of switch statements) be fall-throughs when possible?
        // Is it a waste of energy for us to try to figure out how
        // to ifdef the code to prefer fall-through to goto?
        // I have a limited amount of empirical evidence suggesting that
        // a goto is equivalent to a fall-through in this particular case.
        goto t_switch;

    } // end of default case

#if defined(SKIP_TO_BM_SW)

    case T_SKIP_TO_BM_SW:
    {
        if (PrefixMismatch(pwRoot, wRoot, pwr, wKey, nBL,
                           &nBLR, &pwRootPrefix,
                           &pwrPrefix, &nBLRPrefix, &bNeedPrefixCheck))
        {
            break;
        }

        goto t_bm_sw;

    } // end of T_SKIP_TO_BM_SW case

#endif // defined(SKIP_TO_BM_SW)

#endif // defined(SKIP_LINKS)

    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
#if defined(EXTRA_TYPES)
    case T_SWITCH | EXP(cnBitsMallocMask): // close switch w/o bm
#endif // defined(EXTRA_TYPES)
    {
        goto t_switch; // silence cc in case other the gotos are ifdef'd out
t_switch:
#if ! defined(LOOKUP)
        nBLUp = nBL;
#endif // ! defined(LOOKUP)
        // nBLR is bits left at the top of this switch
        // nBL is bits left below this sw (not including skip bits)
        nBL = nBLR - nBL_to_nBitsIndexSzNAX(nBL);

        Word_t wIndex = ((wKey >> nBL)
            // It is ok to use NAX here even though we might be at top because
            // we don't care if it returns an index size that is too big.
            // Of course, this assumes that NAX will yield nBitsIndexSz
            // greater than or equal to the actual value and won't cause
            // a crash.
            // perf: EXP(cnBitsPerDigit) - 1
            & (MSK(nBL_to_nBitsIndexSzNAX(nBLR))));

        DBGX(printf("T_SWITCH nBLR %d pLinks %p wIndex %d 0x%x\n", nBLR,
             (void *)pwr_pLinks((Switch_t *)pwr), (int)wIndex, (int)wIndex));

#if !defined(LOOKUP)
        if (bCleanup) {
  #if defined(INSERT)
            InsertCleanup(wKey, nBLUp, pwRoot, wRoot);
  #else // defined(INSERT)
            RemoveCleanup(wKey, nBLUp, nBLR, pwRoot, wRoot);
  #endif // defined(INSERT)
            if (*pwRoot != wRoot) { goto restart; }
        } else {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR, wPopCnt + nIncr);
            DBGX(printf("wPopCnt %zd\n",
                        (size_t)PWR_wPopCntBL(pwRoot,
                                              (Switch_t *)pwr, nBLR)));
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks((Switch_t *)pwr)[wIndex].ln_wRoot;

        goto switchTail;
switchTail:

        wRoot = *pwRoot;
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just visited in
        // the next iteration of the loop if we've reached a leaf so we
        // preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // first test is done at compile time and might make the rest go away
        if ((EXP(cnBitsInD1) <= sizeof(Link_t) * 8) && (nBL == cnBitsInD1)) {
            goto embeddedBitmap;
        }
        DBGX(printf("Next pwRoot %p wRoot "OWx" nBL %d\n",
                    (void *)pwRoot, wRoot, nBL));

        // Advance nBLR to the bottom of this switch now just in case
        // we have a non-skip link to a switch.  We could do it later.
        nBLR -= nBL_to_nBitsIndexSzNAX(nBLR);
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again;
        // nType = wr_nType(wRoot); *pwr = wr_tp_pwr(wRoot, nType); switch
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nBL);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of case T_SWITCH

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_FULL_BM_SW:
      #if defined(EXTRA_TYPES)
    case T_FULL_BM_SW | EXP(cnBitsMallocMask): // no skip switch
      #endif // defined(EXTRA_TYPES)
    {
      #if defined(LOOKUP)
        pwr = (Word_t *)&((BmSwitch_t *)pwr)->sw_wPrefixPop;
        goto t_switch;
      #endif // defined(LOOKUP)
        goto t_bm_sw;

    } // end of case T_FULL_BM_SW

  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_BM_SW:
  #if defined(EXTRA_TYPES)
    case T_BM_SW | EXP(cnBitsMallocMask): // no skip switch
  #endif // defined(EXTRA_TYPES)
    {
        goto t_bm_sw; // silence cc in case other the gotos are ifdef'd out
t_bm_sw:
  #if defined(BM_SW_FOR_REAL) || ! defined(LOOKUP)
        nBLUp = nBL;
  #endif // defined(BM_SW_FOR_REAL) || ! defined(LOOKUP)
        nBL = nBLR - nBL_to_nBitsIndexSzNAX(nBL);

        Word_t wIndex = ((wKey >> nBL)
            // It is ok to use NAX here even though we might be at top because
            // we don't care if it returns an index size that is too big.
            // Of course, this assumes that NAX will yield nBitsIndexSz
            // greater than or equal to the actual value and won't cause
            // a crash.
            & (MSK(nBL_to_nBitsIndexSzNAX(nBLR))));

        DBGX(printf("T_BM_SW nBLR %d pLinks %p wIndex %d 0x%x\n", nBLR,
             (void *)pwr_pLinks((Switch_t *)pwr), (int)wIndex, (int)wIndex));

  #if defined(BM_IN_LINK)
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nBL == cnBitsPerWord and pwRoot not at the top.
        // We need to know if there is a link surrounding *pwRoot.
        // InsertGuts always calls back into Insert with the same pwRoot
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nBL == cnBitsPerWord and pwRoot not at the top.
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nBL == cnBitsPerWord)
      #else // defined(RECURSIVE)
                && (pwRoot == pwRootOrig)
          #if !defined(LOOKUP)
                && (nBLOrig == cnBitsPerWord)
          #endif // !defined(LOOKUP)
      #endif // defined(RECURSIVE)
            ) )
  #endif // defined(BM_IN_LINK)
        {
  // Is this ifdef necessary?  Or will the compiler figure it out?
  #if (cnBitsPerDigit > cnLogBitsPerWord)
            unsigned nBmOffset = wIndex >> cnLogBitsPerWord;
  #else // (cnBitsPerDigit > cnLogBitsPerWord)
            unsigned nBmOffset = 0;
  #endif // (cnBitsPerDigit > cnLogBitsPerWord)
           Word_t wBm = PWR_pwBm(pwRoot, pwr)[nBmOffset];
           Word_t wBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
           // Test to see if link exists before figuring out where it is.
           if ( ! (wBm & wBit) )
           {
  #if defined(BM_SW_FOR_REAL)
                DBGX(printf("missing link\n"));
                nBL = nBLUp; // back up for InsertGuts
                goto notFound;
  #else // defined(BM_SW_FOR_REAL)
                assert(0); // only for now
  #endif // defined(BM_SW_FOR_REAL)
            }
            Word_t wBmMask = wBit - 1;
            wIndex = 0;
  #if (cnBitsPerDigit > cnLogBitsPerWord)
            for (unsigned nn = 0; nn < nBmOffset; nn++)
            {
                wIndex += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
            }
  #endif // (cnBitsPerDigit > cnLogBitsPerWord)
            DBGX(printf("\npwRoot %p PWR_pwBm %p\n",
                        (void *)pwRoot, (void *)PWR_pwBm(pwRoot, pwr)));
            wIndex += __builtin_popcountll(wBm & wBmMask);
        }

#if !defined(LOOKUP)
        if (bCleanup) {
  #if defined(INSERT)
            InsertCleanup(wKey, nBLUp, pwRoot, wRoot);
  #else // defined(INSERT)
            RemoveCleanup(wKey, nBLUp, nBLR, pwRoot, wRoot);
  #endif // defined(INSERT)
            if (*pwRoot != wRoot) { goto restart; }
        } else {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
            set_PWR_wPopCntBL(pwRoot,
                              (BmSwitch_t *)pwr, nBLR, wPopCnt + nIncr);
            DBGX(printf("wPopCnt %zd\n",
                (size_t)PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks((BmSwitch_t *)pwr)[wIndex].ln_wRoot;

        goto switchTail;

    } // end of case T_BM_SW

#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

#if (cwListPopCntMax != 0)

    case T_LIST:
#if defined(EXTRA_TYPES)
    case T_LIST | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        goto t_list;
t_list:
        DBGX(printf("T_LIST nBL %d\n", nBL));
  #if ! defined(LOOKUP)
        DBGX(printf("T_LIST bCleanup %d nIncr %d\n", bCleanup, nIncr));
  #endif // ! defined(LOOKUP)
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMaskBL(nBL)));

  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

      #if defined(PP_IN_LINK)
          #if defined(INSERT)
        if (nIncr == -1) { return Failure; }
          #endif // defined(INSERT)
          #if defined(REMOVE)
        if (nIncr == 1) { return Failure; }
          #endif // defined(REMOVE)
      #endif // defined(PP_IN_LINK)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COMPRESSED_LISTS)
          #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
        // nBL is relative to the bottom of the switch
        // containing the pointer to the leaf.
        // Can we use NAT here since bNeedPrefixCheck will never
        // be true if we are at the top?
        // If the top digit is smaller than the rest, then NAT will
        // return nBL > cnBitsPerWord which works out perfectly.
          #endif // !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
          #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We don't support skip links directly to leaves -- yet.
        // Even with defined(PP_IN_LINK).
        // It is sufficient to check the prefix at the switch just
        // above the leaf.
        // pwrPrev is left from the previous iteration of the goto again
        // loop.
        // Would like to combine the source code for this prefix
        // check and the one done in the bitmap section if possible.
        if ( 0
              #if (cnBitsPerWord > 32)
            || (nBL > 32) // leaf has whole key
              #else // (cnBitsPerWord > 32)
            || (nBL > 16) // leaf has whole key
              #endif // (cnBitsPerWord > 32)
          // can't skip nBL check above or we might be at top
          // hmm; check nBL or check at top; which is better?
              #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            // leaf does not have whole key
            // What if there were no skips in the part that is missing?
            || ( ! bNeedPrefixCheck ) // we followed no skip links
              #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            // If we need a prefix check, then we're not at the top.
            // And pwRoot is initialized despite what gcc might think.
              #if defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNATBL(pwRootPrefix,
                                        (Switch_t *)pwrPrefix, nBLRPrefix)
                            ^ wKey))
                < nBLRPrefix)
              #else // defined(SAVE_PREFIX)
            || ((int)LOG(1
                    | (PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwrPrev, nBL)
                        ^ wKey))
                < (nBL
                  #if ! defined(PP_IN_LINK)
                    // prefix in parent switch doesn't contain last digit
                    // for ! defined(PP_IN_LINK) case
                    + nDL_to_nBitsIndexSzNAX(nBL_to_nDL(nBL) + 1)
                  #endif // ! defined(PP_IN_LINK)
                ))
              #endif // defined(SAVE_PREFIX)
            )
          #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #endif // defined(COMPRESSED_LISTS)
        {

      // LOOKUP_NO_LIST_SEARCH is for analysis only.  We have retrieved the
      // pop count and prefix but we have not dereferenced the list itself.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
#if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
#endif // ! defined(SEPARATE_T_NULL)
                && (SearchList(pwr, wKey, nBL, pwRoot) >= 0))
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
                DBGI(printf("found key\n"));
          #if defined(REMOVE)
              #if defined(PP_IN_LINK)
                // Adjust wPopCnt in link to leaf for PP_IN_LINK.
                // wPopCnt in link to switch is adjusted elsewhere,
                // i.e. in the same place as wPopCnt in switch is
                // adjusted for pp-in-switch.
                assert(nIncr == -1);
                set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                                  PWR_wPopCntBL(pwRoot,
                                                (Switch_t *)NULL, nBL) - 1);
              #endif // defined(PP_IN_LINK)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(INSERT) && ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
          #endif // defined(INSERT) && ! defined(RECURSIVE)
                return KeyFound;
            }
        }
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) \
          && defined(COMPRESSED_LISTS)
        else
        {
            DBGX(printf("Mismatch at list wPrefix "OWx" nBL %d\n",
                 PWR_wPrefixNATBL(pwRoot, pwrPrev, nBL), nBL));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && ...

      #if defined(PP_IN_LINK) && defined(INSERT)
        // Adjust wPopCnt in link to leaf for PP_IN_LINK.
        // wPopCnt in link to switch is adjusted elsewhere,
        // i.e. in the same place as wPopCnt in switch is
        // adjusted for pp-in-switch.
        // pwRoot is initialized despite what gcc might think.
        // Would be nice to be able to get the current pop count from
        // SearchList because chances are it will have read it.
        // But it is more important to avoid getting it when not necessary
        // during lookup.
        assert((nBL == cnBitsPerWord) // there is no link with pop count
            || (pwr != NULL) // non-NULL implies non-zero pop count
            || (PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) == 0));
        assert(nIncr == 1);
        DBGI(printf("did not find key\n"));
        set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                        PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + 1);
      #endif // defined(PP_IN_LINK) && defined(INSERT)

        break;

    } // end of case T_LIST

#endif // (cwListPopCntMax != 0)

    case T_BITMAP:
#if defined(EXTRA_TYPES)
    case T_BITMAP | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
embeddedBitmap:
  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

#if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (EXP(cnBitsInD1) > sizeof(Link_t) * 8) {
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL, wPopCnt + nIncr);
        }
#endif // !defined(LOOKUP) && defined(PP_IN_LINK)

#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

  #if defined(SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We have to do the prefix check here.
        if ( 0
          #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            || ! bNeedPrefixCheck
          #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
          #if defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNATBL(pwRootPrefix, pwrPrefix, nBLRPrefix)
                    ^ wKey))
                < nBLRPrefix)
          #else // defined(SAVE_PREFIX)
            // Notice that we're using pwr which was extracted from the
            // previous wRoot -- not the current wRoot -- to find the prefix,
            // if not PP_IN_LINK.  If PP_IN_LINK, then we are using the
            // current pwRoot to find the prefix.
            // nBL is different for the two cases.
            || ((int)LOG(1
                | (PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwrPrev, cnBitsInD1)
                            ^ wKey))
                // The +1 is necessary because the pwrPrev
                // prefix does not contain any less significant bits.
              #if defined(PP_IN_LINK)
                    < nBL
              #else // defined(PP_IN_LINK)
                    < nDL_to_nBL_NAX(nBL_to_nDL(nBL) + 1)
              #endif // defined(PP_IN_LINK)
                )
          #endif // defined(SAVE_PREFIX)
            )
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
        {
  #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            // BUG?: Is pwrPrev valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
              #if defined(PP_IN_LINK)
            assert(PWR_wPopCnt(pwRoot, pwrPrev, cnBitsInD1) != 0);
              #else // defined(PP_IN_LINK)
            assert(PWR_wPopCnt(pwRoot, pwrPrev, cnBitsInD2) != 0);
              #endif // defined(PP_IN_LINK)
            return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            int bBitIsSet
                = (cnBitsInD1 <= cnLogBitsPerWord)
                    ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsInD1))
                : (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)
                    ? BitIsSet(STRUCT_OF(pwRoot, Link_t, ln_wRoot),
                               wKey & MSK(cnBitsInD1))
                : BitIsSet(wr_pwr(wRoot), wKey & MSK(nBL));
            if (bBitIsSet)
            {
      #if defined(REMOVE)
                goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT) && !defined(RECURSIVE)
                if (nIncr > 0)
                {
                    DBGX(printf("Bit is set!\n"));
                    goto undo; // undo counting 
                }
      #endif // defined(INSERT) && !defined(RECURSIVE)
                return KeyFound;
            }
            DBGX(printf("Bit is not set.\n"));
  #endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
        }
  #if defined(SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        else
        {
            DBGX(printf("Mismatch at bitmap wPrefix "OWx"\n",
                        PWR_wPrefixNATBL(pwRoot, pwrPrev, nBL)));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

        break;

    } // end of case T_BITMAP

#if defined(EMBED_KEYS)

    case T_EMBEDDED_KEYS:
#if defined(EXTRA_TYPES)
    case T_EMBEDDED_KEYS | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        assert(nBL
            <= cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL));
  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

  #if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord)
        {
            // Adjust pop count in the link on the way in.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
  #endif // ! defined(LOOKUP) && defined(PP_IN_LINK)

  #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_DEREF)

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        //
        // How many keys will fit?  And how many bits do we need for pop
        // count in each case.  Need space for the keys plus type plus
        // pop count.
        //
        //  - (cnBitsPerWord - cnBitsPerType - cnBitsPerPopCnt) / nBL
        //
        //      1 x 60-bit key                      (0 to 0-bit pop cnt)
        //      1 x 59-bit key                      (0 or 1-bit pop cnt)
        //      1 x 58-bit key                      (0 to 2-bit pop cnt)
        //      1 x 57-bit key                      (0 to 3-bit pop cnt)
        //      1 x 56-bit key  ... 1 x 30-bit key  (0 to 4-bit pop cnt)
        //      2 x 29-bit keys                     (1 to 2-bit pop cnt)
        //      2 x 28-bit keys ... 2 x 20-bit keys (1 to 4-bit pop cnt)
        //      3 x 19-bit keys                     (2 or 3-bit pop cnt)
        //      3 x 18-bit keys ... 3 x 15-bit keys (2 to 6-bit pop cnt)
        //      4 x 14-bit keys ... 4 x 12-bit keys (2 to 8-bit pop cnt)
        //      5 x 11-bit keys ... 5 x 10-bit keys (3 to 5-bit pop cnt)
        //      6 x  9-bit keys                     (3 to 6-bit pop cnt)
        //      7 x  8-bit keys                     (3 or 4-bit pop cnt)
        //      8 x  7-bit keys                     (3 or 4-bit pop cnt)
        //     64 x  6-bit keys in embedded bitmap
        //
        // Does LOG(X / nBL) work?
        //
        //                  ... LOG(119/60) = 0
        //      LOG(119/59) ... LOG(119/30) = 1
        //      LOG(119/29) ... LOG(119/15) = 2
        //      LOG(119/14) ... LOG(119/ 8) = 3
        //      LOG(119/ 7) ... LOG(119/ 4) = 4
        //
        //      LOG( 64/64) ... LOG( 64/33) = 0
        //      LOG( 64/32) ... LOG( 64/17) = 1 (19 needs at least 2)
        //
        //      LOG( 76/64) ... LOG( 76/39) = 0
        //      LOG( 76/38) ... LOG( 76/20) = 1
        //      LOG( 76/19) ... LOG( 76/10) = 2 (11 needs at least 3)
        //
        //      LOG( 88/64) ... LOG( 88/45) = 0
        //      LOG( 88/44) ... LOG( 88/23) = 1
        //      LOG( 88/22) ... LOG( 88/12) = 2
        //      LOG( 88/11) ... LOG( 88/ 6) = 3
        //      LOG( 88/ 5) ... LOG( 88/ 3) = 4
        //      LOG( 88/ 2) ... LOG( 88/ 2) = 5
        //
        // Looks like anything from 88 - 119 will work.
        //
        //      1 x 29-bit key                      (0 to 0-bit pop cnt)
        //      1 x 28-bit key  ... 1 x 15-bit key  (0 or 1-bit pop cnt)
        //      2 x 14-bit keys                     (1 to 1-bit pop cnt)
        //      2 x 13-bit keys                     (1 to 3-bit pop cnt)
        //      2 x 12-bit keys ... 2 x 10-bit keys (1 to 5-bit pop cnt)
        //      3 x  9-bit keys                     (2 to 2-bit pop cnt)
        //      3 x  8-bit keys ... 3 x  7-bit keys (2 to 5-bit pop cnt)
        //      4 x  6-bit keys                     (2 to 5-bit pop cnt)
        //     32 x  5-bit keys in embedded bimtap
        //
        //                  ... LOG( 36/19) = 0
        //      LOG( 36/18) ... LOG( 36/10) = 1
        //      LOG( 36/ 9) ... LOG( 36/ 5) = 2
        //      LOG( 36/ 4) ... LOG( 36/ 3) = 3
        //      LOG( 36/ 2) ... LOG( 36/ 2) = 4
        //
        //                  ... LOG( 36/19) = 0
        //      LOG( 44/22) ... LOG( 36/12) = 1
        //      LOG( 44/11) ... LOG( 36/ 6) = 2
        //      LOG( 44/ 5) ... LOG( 36/ 3) = 3
        //      LOG( 44/ 2) ... LOG( 36/ 2) = 4
        //
      #if defined(EMBEDDED_KEYS_PARALLEL)

          #if defined(DL_SPECIFIC_T_ONE)

        if (nBL == cnBitsInD1) {
            if (EmbeddedListHasKey(wRoot, wKey, cnBitsInD1)) {
                goto foundIt;
            }
        } else if (nBL == cnBitsInD2) {
            if (EmbeddedListHasKey(wRoot, wKey, cnBitsInD2)) {
                goto foundIt;
            }
        } else

          #endif // defined(DL_SPECIFIC_T_ONE)

        if (EmbeddedListHasKey(wRoot, wKey, nBL)) {
            goto foundIt;
        }

      #else // defined(EMBEDDED_KEYS_PARALLEL)

        // I wonder if PAD_T_ONE and not needing to know the pop count
        // would help this code like it does EMBEDDED_KEYS_PARALLEL.
        unsigned nPopCnt = wr_nPopCnt(wRoot, nBL);

        Word_t wKeyRoot;

        switch (nPopCnt) {
          #if (cnBitsPerWord == 64)
        case 8: // max for 7-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (8 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 7: // max for 8-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (7 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 6: // max for 9-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (6 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 5: // max for 10 to 11-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (5 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
          #endif // (cnBitsPerWord == 64)
        case 4: // max for 12 to 14-bit keys and 64 bits; 6 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (4 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 3: // max for 15 to 19-bit keys and 64 bits; 7-9 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (3 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 2: // max for 20 to 29-bit keys and 64 bits; 10-14 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (2 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        default: // max for 30 to 60-bit keys and 64 bits; 15-29 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (1 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        }

      #endif // defined(EMBEDDED_KEYS_PARALLEL)

        break;

foundIt:

      #if defined(REMOVE)
        goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT) && !defined(RECURSIVE)
        if (nIncr > 0) { goto undo; } // undo counting
      #endif // defined(INSERT) && !defined(RECURSIVE)

  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        return KeyFound;

    } // end of case T_EMBEDDED_KEYS

#endif // defined(EMBED_KEYS)

#if defined(USE_T_ONE)

    case T_ONE:
#if defined(EXTRA_TYPES)
    case T_ONE | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        assert(nBL
            > cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL));

  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

  #if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord)
        {
            // Adjust pop count in the link on the way in.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
  #endif // ! defined(LOOKUP) && defined(PP_IN_LINK)

  #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        if (*pwr == wKey)
        {
      #if defined(REMOVE)
            goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT) && !defined(RECURSIVE)
            if (nIncr > 0) { goto undo; } // undo counting
      #endif // defined(INSERT) && !defined(RECURSIVE)
            return KeyFound;
        }

  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        break;

    } // end of case T_ONE

#endif // defined(USE_T_ONE)

#if defined(SEPARATE_T_NULL)

    case T_NULL:
#if defined(EXTRA_TYPES)
    case T_NULL | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        assert(wRoot == 0);

      #if ! defined(LOOKUP)
        if ( bCleanup ) { return Success; }
      #endif // ! defined(LOOKUP)

  // Adjust wPopCnt in link to leaf for PP_IN_LINK.
  // wPopCnt in link to switch is adjusted elsewhere, i.e. in the same place
  // as wPopCnt in switch is adjusted for pp-in-switch.
  #if defined(PP_IN_LINK)
      #if ! defined(LOOKUP)
        // What about defined(RECURSIVE)?
        if (nBL != cnBitsPerWord)
        {
            // If nBL != cnBitsPerWord then we're not at top.
            // And pwRoot is initialized despite what gcc might think.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
      #endif // ! defined(LOOKUP)
  #endif // defined(PP_IN_LINK)

        break;

    } // end of case T_NULL

#endif // defined(SEPARATE_T_NULL)

    } // end of switch

#if defined(BM_SW_FOR_REAL)
notFound:
#endif // defined(BM_SW_FOR_REAL)
#if defined(INSERT)
  #if defined(BM_IN_LINK)
    // If InsertGuts calls Insert, then it is always with the same
    // pwRoot and nBL that Insert passed to InsertGuts.
      #if !defined(RECURSIVE)
    assert((nBL != cnBitsPerWord) || (pwRoot == pwRootOrig));
      #endif // !defined(RECURSIVE)
  #endif // defined(BM_IN_LINK)
    // InsertGuts is called with a pwRoot and nBL indicates the
    // bits that were not decoded in identifying pwRoot.  nBL
    // does not include any skip indicated in the type field of *pwRoot.
    InsertGuts(pwRoot, wKey, nBL, wRoot); goto cleanup;
undo:
#endif // defined(INSERT)
#if defined(REMOVE) && !defined(RECURSIVE)
    if (nIncr < 0)
#endif // defined(REMOVE) && !defined(RECURSIVE)
  #if !defined(LOOKUP) && !defined(RECURSIVE)
    {
        // Undo the counting we did on the way in.
        nIncr *= -1;
  #if ! defined(LOOKUP)
restart:
  #endif // ! defined(LOOKUP)
        pwRoot = pwRootOrig;
        nBL = nBLOrig;
        goto top;
    }
  #endif // !defined(LOOKUP) && !defined(RECURSIVE)
    return Failure;
  #if ! defined(LOOKUP)
      #if defined(REMOVE)
removeGutsAndCleanup:
    RemoveGuts(pwRoot, wKey, nBL, wRoot);
      #else // defined(REMOVE)
cleanup:
      #endif // defined(REMOVE)
    bCleanup = 1; // ?? nIncr == 0 ??
    DBGX(printf("Cleanup pwRO "OWx" nBLO %d\n",
                (Word_t)pwRootOrig, nBLOrig));
    DBGX(Dump(pwRootOrig, /* wPrefix */ (Word_t)0, nBLOrig));
    goto restart;
  #endif // ! defined(LOOKUP)
}

#undef RECURSIVE
#undef InsertGuts
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

//#endif // (cnBitsInD1 < cnBitsPerWord)
//#endif // (cnBitsPerDigit < cnBitsPerWord)
#endif // (cnDigitsPerWord > 1)

#if defined(LOOKUP)

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnDigitsPerWord > 1)

  #if (cwListPopCntMax != 0)
      #if defined(SEARCH_FROM_WRAPPER)
    // Handle a top level T_LIST leaf here.
    // For PP_IN_LINK a T_LIST leaf at the top has a pop count field and
    // T_LIST leaves not at the top do not.
    // And, for PP_IN_LINK there is no Link_t at the top -- only wRoot.
    // SEARCH_FROM_WRAPPER allows us avoid making the mainline T_LIST leaf
    // handling code have to know or test if it is at the top.
    // Do not assume the list is sorted here -- so this code doesn't have to
    // be ifdef'd.
    // SEARCH_FROM_WRAPPER is a bit faster, 1-2 ns out of
    // 2-16 ns, if all we have is a T_LIST leaf, but there is a very small,
    // sub-nanosecond, cost for all other cases.
    unsigned nType = wr_nType((Word_t)pcvRoot);
    if (nType == T_LIST)
    {
        Word_t *pwr = wr_tp_pwr((Word_t)pcvRoot, nType);

        // ls_wPopCount is valid only at the top for PP_IN_LINK
        // the first word in the list is used for pop count at the top
        return (1
#if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
#endif // ! defined(SEPARATE_T_NULL)
                && (SearchListWord(ls_pwKeys(pwr, cnBitsPerWord),
                                   wKey, cnBitsPerWord,
                                   ls_xPopCnt(pwr, cnBitsPerWord))
                    >= 0))
            ? Success : Failure;
    }
      #endif // defined(SEARCH_FROM_WRAPPER)
  #endif // (cwListPopCntMax != 0)

    return Lookup((Word_t)pcvRoot, wKey);

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    DBGL(printf("\nJudy1Test(pcvRoot %p)\n", (void *)pcvRoot));

    if (pcvRoot == NULL)
    {
        return Failure;
    }

  #if defined(BITMAP_BY_BYTE)

    Word_t wByteNum = BitmapByteNum(wKey);
    Word_t wByteMask = BitmapByteMask(wKey);     

    DBGL(printf("Judy1Test num "OWx" mask "OWx"\n", wByteNum, wByteMask));
    DBGL(printf("val %x\n", (int)(((char *)pcvRoot)[wByteNum] & wByteMask)));

      #if defined(LOOKUP_NO_BITMAP_DEREF)
        return Success;
      #else // defined(LOOKUP_NO_BITMAP_DEREF)
        return (((char *)pcvRoot)[wByteNum] & wByteMask) ? Success : Failure;
      #endif // defined(LOOKUP_NO_BITMAP_DEREF)

  #else // defined(BITMAP_BY_BYTE)

    Word_t wWordNum = BitmapWordNum(wKey);
    Word_t wWordMask = BitmapWordMask(wKey);     

    DBGL(printf("Judy1Test num "OWx" mask "OWx"\n", wWordNum, wWordMask));
    DBGL(printf("val %x\n",
        (int)(((Word_t *)pcvRoot)[wWordNum] & wWordMask)));

      #if defined(LOOKUP_NO_BITMAP_DEREF)
        return Success;
      #else // defined(LOOKUP_NO_BITMAP_DEREF)
        return (((Word_t *)pcvRoot)[wWordNum] & wWordMask)
            ? Success : Failure;
      #endif // defined(LOOKUP_NO_BITMAP_DEREF)

  #endif // defined(BITMAP_BY_BYTE)

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(LOOKUP)

#if defined(INSERT)

static int bInitialized;

static void
Initialize(void)
{
    // Search assumes lists are sorted if LIST_END_MARKERS is defined.
#if defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)
    assert(0);
#endif // defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)
    // Why would we want to be able to fit more than one digits' worth of
    // keys into a Link_t as an embedded bitmap?
    if (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8) {
        printf("Warning: (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8)"
               " makes no sense.\n");
        printf("Try increasing cnBitsInD1 or decreasing sizeof(Link_t).\n");
    }
    assert(EXP(cnBitsLeftAtDl2) > sizeof(Link_t) * 8);
#if ! defined(LEVEL_IN_WROOT_HIGH_BITS)
#if ! defined(DEPTH_IN_SW)
#if ! defined(TYPE_IS_RELATIVE)
    // We could be a lot more creative here w.r.t. mapping our scarce type
    // values to absolute depths.  But why?  We have to look at the prefix
    // in a different word anyway.  See comments at tp_to_nDL in b.h.
    if (nDL_to_tp(cnDigitsPerWord - 1) > cnMallocMask) {
        printf("\n");
        printf("nDL_to_tp(cnDigitsPerWord   %2d) 0x%02x\n",
               cnDigitsPerWord, nDL_to_tp(cnDigitsPerWord));
        printf("tp_to_nDL(cnMallocMask    0x%02x)   %2d\n",
               (int)cnMallocMask, (int)tp_to_nDL(cnMallocMask));
    }
#endif // ! defined(TYPE_IS_RELATIVE)
#endif // ! defined(DEPTH_IN_SW)
#endif // ! defined(LEVEL_IN_WROOT_HIGH_BITS)

#if defined(BPD_TABLE_RUNTIME_INIT)
    for (unsigned nDL = 0;
         nDL < sizeof(anDL_to_nBitsIndexSz) / sizeof(anDL_to_nBitsIndexSz[0]);
         nDL++)
    {
        anDL_to_nBitsIndexSz[nDL] = nBitsIndexSz_from_nDL(nDL);
    }
    for (unsigned nDL = 0;
         nDL < sizeof(anDL_to_nBL) / sizeof(anDL_to_nBL[0]); ++nDL)
    {
        anDL_to_nBL[nDL] = nBL_from_nDL(nDL);
    }
    for (unsigned nBL = 0;
         nBL < sizeof(anBL_to_nDL) / sizeof(anBL_to_nDL[0]); ++nBL)
    {
        anBL_to_nDL[nBL] = nDL_from_nBL(nBL);
    }
#endif // defined(BPD_TABLE_RUNTIME_INIT)

    printf("\n");
    printf("# cnBitsInD1 %d\n", cnBitsInD1);
    printf("# cnBitsInD2 %d\n", cnBitsInD2);
    printf("# cnBitsInD3 %d\n", cnBitsInD3);
    printf("# cnBitsPerDigit %d\n", cnBitsPerDigit);
    printf("# cnDigitsPerWord %d\n", cnDigitsPerWord);
    printf("# cnListPopCntMax8  %d\n", cnListPopCntMax8);
    printf("# cnListPopCntMax16 %d\n", cnListPopCntMax16);
    printf("# cnListPopCntMax32 %d\n", cnListPopCntMax32);
    printf("# cnListPopCntMax64 %d\n", cnListPopCntMax64);
    printf("# cnListPopCntMaxDl1 %d\n", cnListPopCntMaxDl1);
#if defined(cnListPopCntMaxDl2)
    printf("# cnListPopCntMaxDl2 %d\n", cnListPopCntMaxDl2);
#else // defined(cnListPopCntMaxDl2)
    printf("# cnListPopCntMaxDl2 n/a\n");
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
    printf("# cnListPopCntMaxDl3 %d\n", cnListPopCntMaxDl3);
#else // defined(cnListPopCntMaxDl3)
    printf("# cnListPopCntMaxDl3 n/a\n");
#endif // defined(cnListPopCntMaxDl3)

#if defined(USE_BM_SW)
    printf("# USE_BM_SW\n");
#else // defined(USE_BM_SW)
    printf("# NO USE_BM_SW\n");
#endif // defined(USE_BM_SW)

#if defined(BM_IN_LINK)
    printf("# BM_IN_LINK\n");
#else // defined(BM_IN_LINK)
    printf("# NO BM_IN_LINK\n");
#endif // defined(BM_IN_LINK)

#if defined(PP_IN_LINK)
    printf("# PP_IN_LINK\n");
#else // defined(PP_IN_LINK)
    printf("# NO PP_IN_LINK\n");
#endif // defined(PP_IN_LINK)
    printf("\n");

    bInitialized= 1;
}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
{
    Word_t *pwRoot = (Word_t *)ppvRoot;

#if (cnDigitsPerWord > 1)

    int status;

    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if defined(DEBUG)

    pwRootLast = pwRoot;

  #endif // defined(DEBUG)

    if ((*pwRoot == (Word_t)0) && ! bInitialized ) {
        Initialize();
    }

  #if (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER)
    // Handle the top level list leaf before calling Insert.  Why?
    // To simplify Insert for PP_IN_LINK.  Does it still apply?
    // Do not assume the list is sorted.
    // But if we do the insert here, and the list is sorted, then leave it
    // sorted -- so we don't have to worry about ifdefs in this code.
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);

    if (((T_LIST == nType)
#if ! defined(SEPARATE_T_NULL)
            && (wr_tp_pwr(wRoot, nType) != NULL)
#endif // ! defined(SEPARATE_T_NULL)
            && 1)
      #if defined(USE_T_ONE)
        || (nType == T_ONE)
      #else // defined(USE_T_ONE)
        || (nType == T_NULL)
      #endif // defined(USE_T_ONE)
        )
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Success)
        {
            status = Failure;
        }
        else
        {
            Word_t *pwr = wr_tp_pwr(wRoot, nType);
            Word_t wPopCnt;

      #if defined(USE_T_ONE)
            if (nType == T_ONE) {
                wPopCnt = 1;
            } else
      #else // defined(USE_T_ONE)
            if (nType == T_NULL) {
                assert(pwr == NULL);
                wPopCnt = 0;
            } else
      #endif // defined(USE_T_ONE)
            {
                wPopCnt = ls_xPopCnt(pwr, cnBitsPerWord);
            }

#if (cnBitsPerWord == 64)
            if (wPopCnt == cnListPopCntMax64)
#else // (cnBitsPerWord == 64)
            if (wPopCnt == cnListPopCntMax32)
#endif // (cnBitsPerWord == 64)
            {
                status = InsertGuts(pwRoot, wKey, cnDigitsPerWord, wRoot);
            }
            else
            {
                Word_t *pwListNew = NewList(wPopCnt + 1, cnDigitsPerWord);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew, cnBitsPerWord);
                set_wr(wRoot, pwListNew, T_LIST);
                Word_t *pwKeys;
      #if defined(USE_T_ONE)
                if (nType == T_ONE) {
                    pwKeys = pwr;
                } else
      #endif // defined(USE_T_ONE)
                { pwKeys = ls_pwKeys(pwr, cnBitsPerWord); }

 // Isn't this chunk of code already in InsertGuts?
                unsigned nn;
                for (nn = 0; (nn < wPopCnt) && (pwKeys[nn] < wKey); nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                pwKeysNew[nn] = wKey;
                COPY(&pwKeysNew[nn + 1], &pwKeys[nn], wPopCnt - nn);
      #if defined(LIST_END_MARKERS)
                // pwKeysNew incorporates top pop count and markers
                pwKeysNew[wPopCnt + 1] = -1;
      #endif // defined(LIST_END_MARKERS)

                OldList(pwr, wPopCnt, cnDigitsPerWord, nType);
                *pwRoot = wRoot;

                status = Success;
            }
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER)
    {
        status = Insert(pwRoot, wKey, cnBitsPerWord);
    }

    if (status == Success) {
        // count successful inserts minus successful removes
        wPopCntTotal++;
  #if defined(DEBUG)
        if (!bHitDebugThreshold && (wPopCntTotal > cwDebugThreshold)) {
            bHitDebugThreshold = 1;
            if (cwDebugThreshold != 0) {
                printf("\nHit debug threshold.\n");
            }
        }
  #endif // defined(DEBUG)
    }

    DBGI(printf("\n# After Insert(wKey "OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGI(printf("\n"));

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
  #endif // defined(DEBUG)

    return status;

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wRoot = *pwRoot;
    Word_t wByteNum, wByteMask;
    char c;

    DBGI(printf("\nJudy1Set(ppvRoot %p wKey "OWx") wRoot "OWx"\n",
        (void *)ppvRoot, wKey, wRoot));

    if (wRoot == 0)
    {
        wRoot = JudyMalloc(EXP(cnBitsPerWord - cnLogBitsPerWord));
        assert(wRoot != 0);
        assert((wRoot & cnMallocMask) == 0);

        DBGI(printf("Malloc wRoot "OWx"\n", wRoot));

        *ppvRoot = (PPvoid_t)wRoot;
    }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);     

    DBGI(printf("Judy1Set num "OWx" mask "OWx"\n", wByteNum, wByteMask));

    if ((c = ((char *)wRoot)[wByteNum]) & wByteMask)
    {
        return Failure; // dup
    }

    ((char *)wRoot)[wByteNum] = c | wByteMask;

    return Success;

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(INSERT)

#if defined(REMOVE)

int
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    Word_t *pwRoot = (Word_t *)ppvRoot;

#if (cnDigitsPerWord > 1)

    int status;

    DBGR(printf("\n\n# Judy1Unset ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER)
    // Handle the top level list leaf.
    // Do not assume the list is sorted, but maintain the current order so
    // we don't have to bother with ifdefs in this code.
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);
    if (nType == T_LIST)
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Failure)
        {
            status = Failure;
        }
        else
        {
            Word_t *pwr = wr_tp_pwr(wRoot, nType);
            Word_t wPopCnt = ls_xPopCnt(pwr, cnBitsPerWord);
            Word_t *pwListNew;
            if (wPopCnt != 1)
            {
                pwListNew = NewList(wPopCnt - 1, cnDigitsPerWord);
                Word_t *pwKeysNew;
      #if defined(USE_T_ONE)
                if (wPopCnt == 2) {
                    set_wr(wRoot, pwListNew, T_ONE);
                    pwKeysNew = pwListNew;
                } else
      #endif // defined(USE_T_ONE)
                {
                    set_wr(wRoot, pwListNew, T_LIST);
                    pwKeysNew = ls_pwKeys(pwListNew, cnBitsPerWord);
                }

                Word_t *pwKeys = ls_pwKeys(pwr, cnBitsPerWord);

 // Isn't this chunk of code already in RemoveGuts?
                unsigned nn;
                for (nn = 0; pwKeys[nn] != wKey; nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                COPY(&pwKeysNew[nn], &pwKeys[nn + 1], wPopCnt - nn - 1);
      #if defined(LIST_END_MARKERS)
          #if defined(USE_T_ONE)
                if (wPopCnt != 2)
          #endif // defined(USE_T_ONE)
                // pwKeysNew incorporates top pop count and markers
                { pwKeysNew[wPopCnt - 1] = -1; }
      #endif // defined(LIST_END_MARKERS)
            }
            else
            {
                wRoot = 0; // set_wr(wRoot, NULL, 0)
            }
            OldList(pwr, wPopCnt, cnDigitsPerWord, nType);
            *pwRoot = wRoot;
            status = Success;
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER)
    {
        status = Remove(pwRoot, wKey, cnBitsPerWord);
    }

    if (status == Success) { wPopCntTotal--; }

  #if defined(DEBUG_REMOVE)
    DBGR(printf("\n# After Remove(wKey "OWx") %s Dump\n", wKey,
            status == Success ? "Success" : "Failure"));
    DBGR(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGR(printf("\n"));
  #endif // defined(DEBUG_REMOVE)

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
  #endif // defined(DEBUG)

    return status;

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wRoot = *pwRoot;
    Word_t wByteNum, wByteMask;
    char c;

    if (wRoot == 0) { return Failure; }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);     

    if ( ! ((c = ((char *)wRoot)[wByteNum]) & wByteMask) )
    {
        return Failure; // not present
    }

    ((char *)wRoot)[wByteNum] = c & ~wByteMask;

    return Success;

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warnings
}

#endif // defined(REMOVE)

