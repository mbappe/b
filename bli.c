
// @(#) $Id: bli.c,v 1.347 2014/11/09 14:30:18 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/bli.c,v $

// This file is #included in other .c files three times.
// Once with #define LOOKUP, #undef INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #define INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #undef INSERT and #define REMOVE.

// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#if (cnDigitsPerWord > 1)
//#if (cnBitsPerDigit < cnBitsPerWord)
//#if (cnBitsAtBottom < cnBitsPerWord)

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

// Simple linear search of a list that assumes the list contains a
// key that is greater than or equal to the key we're searching for.
#define SSEARCHF(_pxKeys, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); } \
    if ((_pxKeys)[_nPos] > (_xKey)) { (_nPos) ^= -1; } \
}

// Simple backward linear search of a list that assumes the list contains a
// key that is less than or equal to the key we're searching for.
#define SSEARCHB(_pxKeys, _xKey, _nPos) \
{ \
    while ((_xKey) < (_pxKeys)[_nPos]) { --(_nPos); } \
    if ((_xKey) > (_pxKeys)[_nPos]) { ++(_nPos); (_nPos) ^= -1; } \
}

#if defined(LIST_END_MARKERS)

  #if defined(TEST_PAST_END)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_MIN(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] >= &(_pxKeys)[_nPopCnt])

#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] < (_pxKeys))

  #elif defined(TEST_SPLIT_EQ_KEY)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  ((_pxKeys)[nSplit] == (_xKey))

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_MIN(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #else // TEST_KEY_IS_MAX_MIN

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

// BUG: Refine this with nBL; this won't work for non-native sizes as it is.
#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey) \
    ((_xKey) == (_x_t)-1)

#define TEST_AND_KEY_IS_MIN(_x_t, _pxKeys, _nPopCnt, _xKey)  ((_xKey) == 0)

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #endif // ...

// Is nPos ^= -1 equivalent to nPos |= nPos?
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    while ((_pxKeys0)[_nPos] < (_xKey)) { ++(_nPos); } \
    if (((_pxKeys0)[_nPos] > (_xKey)) \
        || PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)) \
    { \
        (_nPos) ^= -1; \
    } \
}

#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    (_nPos) += (_nPopCnt) - 1; \
    while ((_xKey) < (_pxKeys0)[_nPos]) { --(_nPos); } \
    if (((_xKey) > (_pxKeys0)[_nPos]) \
        || PAST_ENDB(_pxKeys, _pxKeys0, _nPos)) \
    { \
        ++(_nPos); (_nPos) ^= -1; \
    } \
}

#else // defined(LIST_END_MARKERS)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_MIN(_x_t, _pxKeys, _nPopCnt, _xKey)  0

// Linear search of list (for any size key and with end check).
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    if ((_pxKeys)[(_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SSEARCHF((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

// Backward linear search of list (for any size key and with end check).
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    if ((_xKey) < *(_pxKeys)) { \
        (_nPos) ^= -1; \
    } else { \
        (_nPos) += (_nPopCnt) - 1; SSEARCHB((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

#endif // defined(LIST_END_MARKERS)

#define PSPLIT_P(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    (_nSplit) = (((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt) * sizeof(_xKey)) \
                    /* + ((_nPopCnt) * sizeof(_xKey) / 2) */ ) \
                    / sizeof(Word_t)) \
                >> 9); \
}

// Old method:
// nSplit = (((_xKey) & MSK(_nBL)) * (_nPopCnt) + (_nPopCnt) / 2) >> (_nBL);
// New method:
// Take N most significant bits of key times pop count and divide by 2^N.
// If key has fewer than N significant bits, then shift key left as needed.
// Where N = 9.
// The rounding term is probably insignificant and unnecessary.
#define PSPLIT_X(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt)) \
                    /* + ((_nPopCnt) / 2) */ ) \
                >> 9); \
}

#define PSPLIT_0(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    assert((_nBL) < cnBitsPerWord); \
    assert((_nPopCnt) <= (1 << (cnBitsPerWord - (_nBL)))); \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord - (_nBL))) \
                        * (_nPopCnt)) \
                    + ((_nPopCnt) / 2)) \
                >> (_nBL)); \
}

#define PSPLIT_1(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    assert((_nPopCnt) <= (1 << (cnBitsPerWord - (_nBL) + 1))); \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord + 1 - (_nBL))) \
                        * (_nPopCnt)) \
                    + ((_nPopCnt) / 2)) \
                >> ((_nBL) - 1)); \
}

// The trick is finding an expression for _nn.
#define PSPLIT_NN(_nPopCnt, _nBL, _xKey, _nSplit, _nn) \
{ \
    assert((_nPopCnt) <= (1 << (cnBitsPerWord - (_nBL) + (_nn)))); \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord + (_nn) - (_nBL))) \
                        * (_nPopCnt)) \
                    + ((_nPopCnt) / 2)) \
                >> ((_nBL) - (_nn))); \
}

// I think this may work for everything except:
//    (_nBL == cnBitsPerWord) && (_nPopCnt == 1)
#define PSPLIT(_nPopCnt, _nBL, _xKey, _nSplit) \
        PSPLIT_NN((_nPopCnt), (_nBL), (_xKey), (_nSplit), \
            LOG((((_nPopCnt) << 1) - 1)) - cnBitsPerWord + (_nBL))

#if 0
pop <= 2 ^ (bpw - nbl + nn)
ceil(log(pop)) <= bpw - nbl + nn
LOG(pop * 2 - 1) <= bpw - nbl + nn
nn >= LOG(pop * 2 - 1) - bpw + nbl
nn  = LOG(pop * 2 - 1) - bpw + nbl
#endif

#if 0

#define PSPLIT(_nPopCnt, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
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

#define PSPLIT(_nn, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
{ \
    (_nSplit) \
        = ((((Word_t)(_xKey) - (_xKeyMin)) * (_nn) + (_nn) / 2) \
            / ((Word_t)(_xKeyMax) - (_xKeyMin) + 1)); \
    if ((_nSplit) < 0) { (_nSplit) = 0; } \
    else if ((_nSplit) >= (_nn)) { (_nSplit) = (_nn) - 1; } \
}

#endif

// unsigned nSplit; PSPLIT((_nPopCnt), 0, (_x_t)-1, (_xKey), nSplit);

#if 0
    if (WordHasKey(*(Word_t *)((uintptr_t)&(_pxKeys)[nSplit] & ~7), \
                   (_xKey), sizeof(_x_t) * 8)) \
    { \
        (_nPos) = nSplit; \
    } \
    else \

#endif // 0

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
// It also assumes the last key in the last word is equal to the largest
// key in the list even if it is beyond the population.
#define PSSEARCHF(_pxKeys, _xKey, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(cnLogBytesPerWord)) == 0); \
    Word_t *pw = (Word_t *)(_pxKeys); \
    /* starting word */ \
    int nwPos = (_nPos) * sizeof(_xKey) / sizeof(Word_t); \
    /* number of last key in starting word */ \
    (_nPos) = (nwPos * sizeof(Word_t) | MSK(cnLogBytesPerWord)) \
                / sizeof(_xKey); \
    while ( ! WordHasKey(pw[nwPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the last key in the word to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] > (_xKey)) { (_nPos) ^= -1; break; } \
        ++nwPos; (_nPos) += sizeof(Word_t) / sizeof(_xKey); \
    } \
}

#if defined(PSPLIT_EARLY_OUT)

// Simple backward linear parallel search of a list that assumes the list
// contains a key that is less than or equal to the key we're searching for.
// Key size is some power of two bytes.
#define PSSEARCHB(_pxKeys, _xKey, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(cnLogBytesPerWord)) == 0); \
    Word_t *pw = (Word_t *)(_pxKeys); \
    /* starting word */ \
    int nwPos = (_nPos) * sizeof(_xKey) / sizeof(Word_t); \
    /* number of first key in starting word */ \
    (_nPos) = nwPos * sizeof(Word_t) / sizeof(_xKey); \
    while ( ! WordHasKey(pw[nwPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the word to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nwPos; (_nPos) -= sizeof(Word_t) / sizeof(_xKey); \
    } \
}

#else // defined(PSPLIT_EARLY_OUT)

// Simple backward linear parallel search of a list that assumes the list
// contains a key that is less than or equal to the key we're searching for.
// Key size is some power of two bytes.
#define PSSEARCHB(_pxKeys, _xKey, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(cnLogBytesPerWord)) == 0); \
    Word_t *pw = (Word_t *)(_pxKeys); \
    /* starting word */ \
    int nwPos = (_nPos) * sizeof(_xKey) / sizeof(Word_t); \
    /* number of first key in starting word */ \
    (_nPos) = nwPos * sizeof(Word_t) / sizeof(_xKey); \
    while ( ! WordHasKey(pw[nwPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check to see if we've reached the beginning of the list */ \
        if (nwPos <= 0) { (_nPos) ^= -1; break; } \
        --nwPos; (_nPos) -= sizeof(Word_t) / sizeof(_xKey); \
    } \
}

#endif // defined(PSPLIT_EARLY_OUT)

#if defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF  SEARCHF

// Backward linear search of list (for any size key and with end check).
#define PSEARCHB  SEARCHB

#else // defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    if ((_pxKeys)[(_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        PSSEARCHF((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

// Backward linear search of list (for any size key and with end check).
#define PSEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _pxKeys0, _nPos) \
{ \
    (_nPos) = (_pxKeys) - (_pxKeys0); \
    if ((_xKey) < *(_pxKeys)) { \
        (_nPos) ^= -1; \
    } else { \
        (_nPos) += (_nPopCnt) - 1; PSSEARCHB((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

#endif // defined(PSPLIT_HYBRID)

// nSplit is a word number
#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    Word_t *pw = (Word_t *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(cnLogBytesPerWord)) == 0); \
    /*unsigned nSplitP; PSPLIT_P((_nPopCnt), (_nBL), (_xKey), nSplitP);*/ \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    unsigned nSplitP = nSplit * sizeof(_x_t) >> cnLogBytesPerWord; \
    assert(((nSplit * sizeof(_x_t)) >> cnLogBytesPerWord) == nSplitP); \
    if (WordHasKey(pw[nSplitP], (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = nSplitP * sizeof(Word_t) / sizeof(_x_t); \
    } \
    else /*if(pw[nSplitP] < (_xKey) << (cnBitsPerWord - sizeof(_x_t) * 8))*/ \
    { \
    if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(Word_t)) { \
            (_nPos) = ~(_nPopCnt); \
        } else { \
            PSEARCHF(_x_t, &(_pxKeys)[nSplit + 1], (_nPopCnt) - nSplit - 1, \
                    (_xKey), (_pxKeys), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        PSEARCHB(_x_t, (_pxKeys), nSplit + 1, (_xKey), (_pxKeys), (_nPos)); \
    } \
    assert(((_nPos) < 0) \
        || WordHasKey(*(Word_t *) \
                        ((Word_t)&(_pxKeys)[_nPos] \
                            & ~MSK(cnLogBytesPerWord)), \
                      (_xKey), sizeof(_x_t) * 8)); \
    if ((_nPos) < 0) { \
        assert(~(_nPos) >= 0); \
        /*assert(~(_nPos) <= (int)(_nPopCnt))*/; \
        for (unsigned ii = 0; ii < (_nPopCnt); \
             ii += sizeof(Word_t) / sizeof(_xKey)) \
        { \
            if ( ! ( ! WordHasKey(*(Word_t *)&(_pxKeys)[ii], \
                      (_xKey), sizeof(_x_t) * 8) ) ) { \
printf("\npxKeys %p nPopCnt %d xKey "OWx" nPos %d\n", \
       (void *)(_pxKeys), (_nPopCnt), (Word_t)(_xKey), (_nPos)); \
HexDump("pw", (Word_t *)(_pxKeys), \
        ((_nPopCnt) * sizeof(_xKey) + sizeof(Word_t) - 1) / sizeof(Word_t)); \
printf("nSplit %d nSplitP %d\n", nSplit, nSplitP); \
            } \
            assert( ! WordHasKey(*(Word_t *)&(_pxKeys)[ii], \
                      (_xKey), sizeof(_x_t) * 8) ); \
        } \
    } \
    } \
}

#else // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    if (TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)) \
    { \
        (_nPos) = nSplit; \
    } \
    else if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        if (nSplit == (_nPopCnt) - 1) \
        { \
            (_nPos) = ~(_nPopCnt); \
        } \
        else if (TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            (_nPos) = ((_pxKeys)[(_nPopCnt) - 1] == (_x_t)-1) \
                        ? (_nPopCnt) - 1 : ~(_nPopCnt); \
        } \
        else \
        { \
            SEARCHF(_x_t, &(_pxKeys)[nSplit + 1], (_nPopCnt) - nSplit - 1, \
                    (_xKey), (_pxKeys), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        if (TEST_AND_KEY_IS_MIN(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            (_nPos) = ((_pxKeys)[0] == 0) ? 0 : ~0; \
        } \
        else \
        { \
            SEARCHB(_x_t, (_pxKeys), nSplit + 1, \
                    (_xKey), (_pxKeys), (_nPos)); \
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

#if defined(USE_WORD_ARRAY_HAS_KEY)

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
WordHasKey(Word_t ww, Word_t wKey, unsigned nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    return WordArrayHasKey(&ww, /* nWords */ 1, wKey, nBL);
}

#else // defined(USE_WORD_ARRAY_HAS_KEY)

// Do a parallel search of a word for a key that is smaller than a word.
// WordHasKey expects the keys to be packed towards the most significant bits,
// and it assumes all slots in the word have valid keys, i.e. the would-be
// empty slots have been padded with copies of some key/keys that is/are
// present.
static Status_t
WordHasKey(Word_t ww, Word_t wKey, unsigned nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask; // get rid of already-decoded bits
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
    int bXorHasZero = (wMagic != 0);
    return bXorHasZero ? Success : Failure;
}

#endif // defined(USE_WORD_ARRAY_HAS_KEY)

#endif // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)

#if defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 8)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList8(uint8_t *pcKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(pcKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint8_t *)(((Word_t)&pcKeys[nPopCnt] + sizeof(Word_t) - 1)
            & ~MSK(cnLogBytesPerWord))
        == (uint8_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(pcKeys[nPopCnt] == (uint8_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint8_t cKey = (uint8_t)wKey;
    int nPos;
#if defined(PSPLIT_SEARCH_8)
    PSPLIT_SEARCH(uint8_t, nBL, pcKeys, nPopCnt, cKey, nPos);
#elif defined(BACKWARD_SEARCH_8)
    SEARCHB(uint8_t, pcKeys, nPopCnt, cKey, pcKeys, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint8_t, pcKeys, nPopCnt, cKey, pcKeys, nPos); (void)nBL;
#endif // ...
    return nPos;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 8)

#if defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 16)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList16(uint16_t *psKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Word_t) - 1)
            & ~MSK(cnLogBytesPerWord))
        == (uint16_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos;
#if defined(PSPLIT_SEARCH_16)
    PSPLIT_SEARCH(uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
#elif defined(BACKWARD_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, psKeys, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, psKeys, nPos); (void)nBL;
#endif // ...
    return nPos;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 16)

#if defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) \
    && (cnBitsAtBottom <= 32)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList32(uint32_t *piKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Word_t) - 1)
            & ~MSK(cnLogBytesPerWord))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // ! defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos;
#if defined(PSPLIT_SEARCH_32)
    PSPLIT_SEARCH(uint32_t, nBL, piKeys, nPopCnt, iKey, nPos);
#elif defined(BACKWARD_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, piKeys, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, piKeys, nPos); (void)nBL;
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
SearchListWord(Word_t *pwKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(pwKeys[-1] == 0);
    assert(pwKeys[nPopCnt] == (Word_t)-1);
#endif // defined(LIST_END_MARKERS)
    int nPos;
#if defined(PSPLIT_SEARCH_WORD)
#if defined(PSPLIT_SEARCH_XOR_WORD)
    Word_t wKeyMin = pwKeys[0];
    Word_t wKeyMax = pwKeys[nPopCnt - 1];
    // Or in 1 to handle nPopCnt==1 else we'd be taking the LOG of zero.
    nBL = LOG((wKeyMin ^ wKeyMax) | 1) + 1;
    // nBL could be 64 and it could be 0.
#endif // defined(PSPLIT_SEARCH_XOR_WORD)
    if (nBL <= (cnBitsPerWord - 8)) {
        PSPLIT_SEARCH(Word_t, nBL, pwKeys, nPopCnt, wKey, nPos);
    } else { // here to avoid overflow
        unsigned nSplit
            = ((wKey & MSK(nBL)) >> 8) * nPopCnt + nPopCnt / 2 >> (nBL - 8);
        if (pwKeys[nSplit] < wKey) {
            if (nSplit == nPopCnt - 1) { return ~nPopCnt; }
            SEARCHF(Word_t, &pwKeys[nSplit + 1], nPopCnt - nSplit - 1,
                       wKey, pwKeys, nPos);
        } else { // here if wKey <= pwKeys[nSplit]
            SEARCHB(Word_t, pwKeys, nSplit + 1, wKey, pwKeys, nPos);
        }
    }
#else // defined(PSPLIT_SEARCH_WORD)
    Word_t *pwKeysOrig = pwKeys;
    (void)nBL;
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
  #if defined(BACKWARD_SEARCH_WORD)
    SEARCHB(Word_t, pwKeys, nPopCnt, wKey, pwKeysOrig, nPos);
  #else // defined(BACKWARD_SEARCH_WORD)
    SEARCHF(Word_t, pwKeys, nPopCnt, wKey, pwKeysOrig, nPos);
  #endif // defined(BACKWARD_SEARCH_WORD)
#endif // defined(PSPLIT_SEARCH_WORD)
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
// (a-b+C))(b-C)C    &  A00
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
SearchList(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    DBGL(printf("SearchList\n"));
  #if defined(COMPRESSED_LISTS)
      #if 0
      #if (cnBitsAtBottom <= 16)
    if (nBL == 16) {
        return SearchList16(pwr_psKeys(pwr), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsAtBottom <= 16)
      #endif // 0
      // Could be more strict if NO_LIST_AT_DL1.
      #if (cnBitsAtBottom <= 8)
    if (nBL <= 8) {
        return SearchList8(pwr_pcKeys(pwr), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsAtBottom <= 8)
      #if (cnBitsAtBottom <= 16)
    if (nBL <= 16) {
        return SearchList16(pwr_psKeys(pwr), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsAtBottom <= 16)
      #if (cnBitsAtBottom <= 32) && (cnBitsPerWord > 32)
    if (nBL <= 32) {
        return SearchList32(pwr_piKeys(pwr), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsAtBottom <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    {
        return SearchListWord(pwr_pwKeys(pwr), wKey, nBL, nPopCnt);
    }
}

#endif // ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)
#endif // ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#endif // ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)

#endif // (cwListPopCntMax != 0)

#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if (cwListPopCntMax != 0) && defined(EMBED_KEYS) && defined(HAS_KEY)

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

#endif // (cwListPopCntMax != 0) && defined(EMBED_KEYS) && defined(HAS_KEY)
#endif // ! defined(LOOKUP_NO_LIST_DEREF)

static int
PrefixMismatch(Word_t *pwRoot, Word_t wRoot, Word_t wKey, unsigned nDL,
               unsigned *pnDLR,
               Word_t **ppwRootPrefix,
               Word_t **ppwrPrefix,
               unsigned *pnDLRPrefix,
               int *pbNeedPrefixCheck)
{
    (void)pwRoot; (void)wKey; (void)pnDLR;
    (void)ppwRootPrefix; (void)ppwrPrefix; (void)pnDLRPrefix;
    (void)pbNeedPrefixCheck;

    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType); (void)pwr;
    unsigned nDLR;
    int bPrefixMismatch; (void)bPrefixMismatch;

#if defined(SKIP_LINKS)
  #if defined(TYPE_IS_RELATIVE)
        nDLR = nDL - tp_to_nDS(nType);
  #else // defined(TYPE_IS_RELATIVE)
        nDLR = tp_to_nDL(nType);
  #endif // defined(TYPE_IS_RELATIVE)
        assert(nDLR <= nDL); // reserved
#else // defined(SKIP_LINKS)
        nDLR = nDL; // prev
#endif // defined(SKIP_LINKS)

#if defined(SKIP_LINKS)
  #if defined(TYPE_IS_RELATIVE)
        assert(nDLR < nDL);
  #endif // defined(TYPE_IS_RELATIVE)
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
        *pnDLRPrefix = nDLR;
      #endif // defined(SAVE_PREFIX)
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        // Record that there were prefix bits that were not checked.
          #if defined(TYPE_IS_RELATIVE)
        *pbNeedPrefixCheck |= 1;
          #else // defined(TYPE_IS_RELATIVE)
        *pbNeedPrefixCheck |= (nDLR < nDL);
          #endif // defined(TYPE_IS_RELATIVE)
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        bPrefixMismatch = (1
      #if ! defined(TYPE_IS_RELATIVE)
            && (nDLR < nDL)
      #endif // ! defined(TYPE_IS_RELATIVE)
            && (LOG(1 | (PWR_wPrefixNAT(pwRoot,
                                        (Switch_t *)pwr, nDLR) ^ wKey))
                    >= nDL_to_nBL_NAT(nDLR)));
      #if ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
        if (bPrefixMismatch)
        {
            DBGX(printf("Mismatch wPrefix "Owx"\n",
                        PWR_wPrefixNAT(pwRoot, (Switch_t *)pwr, nDLR)));
            return 1; // prefix mismatch
        }
      #endif // ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)

    *pnDLR = nDLR;

    return 0; // no prefix mismatch
}

#if defined(LOOKUP)
static Status_t
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Status_t
InsertRemove(Word_t *pwRoot, Word_t wKey, unsigned nDL)
#endif // defined(LOOKUP)
{
    unsigned nDLUp; (void)nDLUp; // silence gcc
    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
    unsigned nBL; (void)nBL;
#if defined(LOOKUP)
    unsigned nDL = cnDigitsPerWord;
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
    unsigned nDLOrig = nDL;
          #if defined(INSERT)
    int nIncr = 1;
          #else // defined(INSERT)
    int nIncr = -1;
          #endif // defined(INSERT)
  #endif // !defined(RECURSIVE)
  #if defined(PP_IN_LINK) || defined(BM_IN_LINK)
    nDLUp = nDL;
  #endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)
#endif // defined(LOOKUP)
#if !defined(RECURSIVE)
  #if !defined(LOOKUP) || defined(BM_IN_LINK)
    Word_t *pwRootOrig = pwRoot;
  #endif // !defined(LOOKUP) || defined(BM_IN_LINK)
#endif // !defined(RECURSIVE)
    unsigned nDLR;
    Word_t wPopCnt; (void)wPopCnt;
#if defined(REMOVE)
    int bCleanup = 0;
#endif // defined(REMOVE)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrPrev = pwrPrev; // suppress "uninitialized" compiler warning
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    int bPrefixMismatch = 0; (void)bPrefixMismatch;

    Word_t *pwRootPrefix = NULL; (void)pwRootPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    unsigned nDLRPrefix = 0; (void)nDLRPrefix;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if !defined(LOOKUP)
  #if !defined(RECURSIVE)
top:
  #endif // !defined(RECURSIVE)
    wRoot = *pwRoot;
#endif // !defined(LOOKUP)
    nDLR = nDL;

#if defined(LOOKUP) || !defined(RECURSIVE)
again:
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
    assert(nDLR == nDL);
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if ( ! defined(LOOKUP) )
    assert(nDL >= 1); // valid for LOOKUP too
    DBGX(printf("# pwRoot %p ", (void *)pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDL %d\n",
            wRoot, wKey, nDL));

    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
#if defined(EXTRA_TYPES)
    switch (wRoot & MSK(cnBitsMallocMask + 1))
#else // defined(EXTRA_TYPES)
    switch (nType)
#endif // defined(EXTRA_TYPES)
    {
    default: // skip link (if -DSKIP_LINKS && -DTYPE_IS_RELATIVE)
    {
        // pwr points to a switch

        if (PrefixMismatch(pwRoot, wRoot, wKey, nDL,
                           &nDLR, &pwRootPrefix,
                           &pwrPrefix, &nDLRPrefix, &bNeedPrefixCheck))
        {
            break;
        }

#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        // fall into next case
    }
    case T_SW_BASE: // no skip switch
#if defined(EXTRA_TYPES)
    case T_SW_BASE | EXP(cnBitsMallocMask): // no skip switch
#endif // defined(EXTRA_TYPES)
    {
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)

#if ( ! defined(LOOKUP) && defined(PP_IN_LINK) || defined(REMOVE) )
        nDLUp = nDL;
#endif // ( ! defined(LOOKUP) && defined(PP_IN_LINK) || defined(REMOVE) )
        nDL = nDLR - 1;
        nBL = nDL_to_nBL_NAT(nDL);

        Word_t wIndex = ((wKey >> nBL)
            // It is ok to use NAX here even though we might be at top because
            // we don't care if it returns an index size that is too big.
            // Of course, this assumes that NAX will yield nBitsIndexSz
            // greater than or equal to the actual value and won't cause
            // a crash.
            & (EXP(nDL_to_nBitsIndexSzNAX(nDLR)) - 1));

#if !defined(LOOKUP)
  #if defined(PP_IN_LINK)
// What if nDL was cnDigitsPerWord before it was updated?
// Don't we have to walk the switch in that case too?
        if (nDLUp == cnDigitsPerWord)
        {
      #if defined(REMOVE)
            if (bCleanup)
            {
                DBGX(printf("Cleanup\n"));

                for (Word_t ww = 0; ww < EXP(cnBitsIndexSzAtTop); ww++)
                {
                    Word_t *pwRootLn
                                = &pwr_pLinks((Switch_t *)pwr)[ww].ln_wRoot;
// looking at the next pwRoot seems like something that should be deferred
// but if we defer, then we won't have the previous pwRoot, but if this
// only happens at the top, then the previous pwRoot will be pwRootOrig?

// What if ln_wRoot is a list?
// nDL cannot be obtained from ln_wRoot.
// We must use nDL in that case.
                    // Do we really need a new variable here?
                    // Or can we just use nDL?
                    int nDLX = wr_bIsSwitch(*pwRootLn) ?
              #if defined(TYPE_IS_RELATIVE)
                                       nDL - wr_nDS(*pwRootLn)
              #else // defined(TYPE_IS_RELATIVE)
                                       wr_nDL(*pwRootLn)
              #endif // defined(TYPE_IS_RELATIVE)
                                   : nDL;
                    DBGX(printf("wr_nDLX %d", nDLX));
                    DBGX(printf(" PWR_wPopCnt %"_fw"d\n",
                                PWR_wPopCnt(pwRootLn, NULL, nDLX)
                                ));
                    if (((*pwRootLn != 0) && (ww != wIndex))
                            || (
                                PWR_wPopCnt(pwRootLn, NULL, nDLX)
                                    != 0)
                        )
                    {
                        DBGX(printf("Not empty ww %zd wIndex %zd\n",
                             (size_t)ww, (size_t)wIndex));
                        goto notEmpty; // switch pop is not zero
                    }
                }
                // switch pop is zero
                FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDLUp),
                    /* bDump */ 0);
          #if defined(PP_IN_LINK)
                assert(PWR_wPrefix(pwRoot, NULL, nDLUp) == 0);
          #endif // defined(PP_IN_LINK)

                *pwRoot = 0;
                return KeyFound;
notEmpty:;
            }
      #endif // defined(REMOVE)
        }
        else
  #endif // defined(PP_IN_LINK)
        {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDLR);
  #if defined(REMOVE)
            if (bCleanup)
            {
                if (wPopCnt == 0)
                {
                    FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDLUp),
                        /* bDump */ 0);
      #if defined(PP_IN_LINK)
                if (PWR_wPrefix(pwRoot, NULL, nDLR) != 0)
                {
                    DBGR(printf("wPrefixPop "OWx"\n",
                                PWR_wPrefixPop(pwRoot, NULL)));
                }
                assert(PWR_wPrefix(pwRoot, NULL, nDLR) == 0);
      #endif // defined(PP_IN_LINK)
                    *pwRoot = 0;
                    return KeyFound;
                }
            }
            else
  #endif // defined(REMOVE)
            {
                set_PWR_wPopCnt(pwRoot, (Switch_t *)pwr,
                                nDLR, wPopCnt + nIncr);
                DBGX(printf("wPopCnt %zd\n",
                     (size_t)PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDLR)));
            }
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks((Switch_t *)pwr)[wIndex].ln_wRoot;
        wRoot = *pwRoot;

        DBGX(printf("Next pLinks %p wIndex %d 0x%x\n",
            (void *)pwr_pLinks((Switch_t *)pwr), (int)wIndex, (int)wIndex));

        DBGX(printf("pwRoot %p wRoot "OWx"\n", (void *)pwRoot, wRoot));

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just visited in
        // the next iteration of the loop if we've reached a leaf so we
        // preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#if (cnBitsAtBottom <= cnLogBitsPerWord)
        // We have to get rid of this 'if' by putting it in the switch.  How?
        if (nBL <= cnLogBitsPerWord) { goto embeddedBitmap; }
#endif // (cnBitsAtBottom <= cnLogBitsPerWord)
#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        nDLR = nDL;
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nDL);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of default case

    case T_BM_SW:
#if defined(EXTRA_TYPES)
    case T_BM_SW | EXP(cnBitsMallocMask): // no skip switch
#endif // defined(EXTRA_TYPES)
    {

#if defined(BM_SWITCH_FOR_REAL) \
    || ( ! defined(LOOKUP) \
        && (defined(PP_IN_LINK) || defined(BM_IN_LINK)) \
            || (defined(REMOVE) && ! defined(BM_IN_LINK)) )
        nDLUp = nDL;
#endif // defined(BM_SWITCH_FOR_REAL) ...
        nDL = nDLR - 1;
        nBL = nDL_to_nBL_NAT(nDL);

        Word_t wIndex = ((wKey >> nBL)
            // It is ok to use NAX here even though we might be at top because
            // we don't care if it returns an index size that is too big.
            // Of course, this assumes that NAX will yield nBitsIndexSz
            // greater than or equal to the actual value and won't cause
            // a crash.
            & (EXP(nDL_to_nBitsIndexSzNAX(nDLR)) - 1));

  #if defined(BM_IN_LINK)
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nDL == cnDigitsPerWord and pwRoot not at the top.
        // We need to know if there is a link surrounding *pwRoot.
        // InsertGuts always calls back into Insert with the same pwRoot
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nDL == cnDigitsPerWord and pwRoot not at the top.
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nDL == cnDigitsPerWord)
      #else // defined(RECURSIVE)
                && (pwRoot == pwRootOrig)
          #if !defined(LOOKUP)
                && (nDLOrig == cnDigitsPerWord)
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
  #if defined(BM_SWITCH_FOR_REAL)
                DBGX(printf("missing link\n"));
                nDL = nDLUp; // back up for InsertGuts
                goto notFound;
  #else // defined(BM_SWITCH_FOR_REAL)
                assert(0); // only for now
  #endif // defined(BM_SWITCH_FOR_REAL)
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
  #if defined(PP_IN_LINK)
// What if nDL was cnDigitsPerWord before it was updated?
// Don't we have to walk the switch in that case too?
        if (nDLUp == cnDigitsPerWord)
        {
      #if defined(REMOVE)
            if (bCleanup)
            {
                DBGX(printf("Cleanup\n"));

          #if ! defined(BM_IN_LINK)
                Word_t xx = 0;
          #endif // ! defined(BM_IN_LINK)
                for (Word_t ww = 0; ww < EXP(cnBitsIndexSzAtTop); ww++)
                {
          #if !defined(BM_IN_LINK)
                    Word_t *pwRootLn
                                = &pwr_pLinks((BmSwitch_t *)pwr)[xx].ln_wRoot;
                    xx++;
          #else // ! defined(BM_IN_LINK)
                    Word_t *pwRootLn
                                = &pwr_pLinks((BmSwitch_t *)pwr)[ww].ln_wRoot;
          #endif // ! defined(BM_IN_LINK)
// looking at the next pwRoot seems like something that should be deferred
// but if we defer, then we won't have the previous pwRoot, but if this
// only happens at the top, then the previous pwRoot will be pwRootOrig?

// What if ln_wRoot is a list?
// nDL cannot be obtained from ln_wRoot.
// We must use nDL in that case.
                    // Do we really need a new variable here?
                    // Or can we just use nDL?
                    int nDLX = wr_bIsSwitch(*pwRootLn) ?
              #if defined(TYPE_IS_RELATIVE)
                                       nDL - wr_nDS(*pwRootLn)
              #else // defined(TYPE_IS_RELATIVE)
                                       wr_nDL(*pwRootLn)
              #endif // defined(TYPE_IS_RELATIVE)
                                   : nDL;
                    DBGX(printf("wr_nDLX %d", nDLX));
                    DBGX(printf(" PWR_wPopCnt %"_fw"d\n",
                                PWR_wPopCnt(pwRootLn, NULL, nDLX)
                                ));
                    if (((*pwRootLn != 0) && (ww != wIndex))
                            || (
                                PWR_wPopCnt(pwRootLn, NULL, nDLX)
                                    != 0)
                        )
                    {
                        DBGX(printf("Not empty ww %zd wIndex %zd\n",
                             (size_t)ww, (size_t)wIndex));
                        goto notEmptyBm; // switch pop is not zero
                    }
                }
                // switch pop is zero
                FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDLUp),
                    /* bDump */ 0);
          #if defined(PP_IN_LINK)
                assert(PWR_wPrefix(pwRoot, NULL, nDLUp) == 0);
          #endif // defined(PP_IN_LINK)

                *pwRoot = 0;
                return KeyFound;
notEmptyBm:;
            }
      #endif // defined(REMOVE)
        }
        else
  #endif // defined(PP_IN_LINK)
        {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDLR);
  #if defined(REMOVE)
            if (bCleanup)
            {
                if (wPopCnt == 0)
                {
                    FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDLUp),
                        /* bDump */ 0);
      #if defined(PP_IN_LINK)
          #if defined(BM_IN_LINK)
                assert(PWR_wPrefix(pwRoot, NULL, nDLUp) == 0);
          #else // defined(BM_IN_LINK)
                if (PWR_wPrefix(pwRoot, NULL, nDLR) != 0)
                {
                    DBGR(printf("wPrefixPop "OWx"\n",
                                PWR_wPrefixPop(pwRoot, NULL)));
                }
                assert(PWR_wPrefix(pwRoot, NULL, nDLR) == 0);
          #endif // defined(BM_IN_LINK)
      #endif // defined(PP_IN_LINK)
                    *pwRoot = 0;
                    return KeyFound;
                }
            }
            else
  #endif // defined(REMOVE)
            {
                set_PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr,
                                nDLR, wPopCnt + nIncr);
                DBGX(printf("wPopCnt %zd\n",
                     (size_t)PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDLR)));
            }
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks((BmSwitch_t *)pwr)[wIndex].ln_wRoot;
        wRoot = *pwRoot;

        DBGX(printf("Next pLinks %p wIndex %d\n",
            (void *)pwr_pLinks((BmSwitch_t *)pwr), (int)wIndex));

        DBGX(printf("pwRoot %p wRoot "OWx"\n", (void *)pwRoot, wRoot));

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just visited in
        // the next iteration of the loop if we've reached a leaf so we
        // preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#if (cnBitsAtBottom <= cnLogBitsPerWord)
#if defined(BM_SW_AT_DL2_ONLY)
        assert(nBL <= cnLogBitsPerWord);
#else // defined(BM_SW_AT_DL2_ONLY)
        // We have to get rid of this 'if' by putting it in the switch.  How?
        if (nBL <= cnLogBitsPerWord)
#endif // defined(BM_SW_AT_DL2_ONLY)
        {
            goto embeddedBitmap;
        }
#endif // (cnBitsAtBottom <= cnLogBitsPerWord)
#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        nDLR = nDL;
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nDL);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of case T_BM_SW

#if (cwListPopCntMax != 0)

    case T_LIST:
#if defined(EXTRA_TYPES)
    case T_LIST | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        DBGX(printf("List nDL %d\n", nDL));
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMask(nDL)));

  #if defined(LOOKUP) && defined(ONE_DEREF_AT_LIST)

        if (*pwr != 0) { return  KeyFound; }

  #else // defined(LOOKUP) && defined(ONE_DEREF_AT_LIST)

  #if defined(REMOVE)
        if (bCleanup) { return Success; } // cleanup is complete
  #endif // defined(REMOVE)

  #if defined(PP_IN_LINK)
        // What about defined(RECURSIVE)?
        assert(nDL != cnDigitsPerWord); // handled in wrapper
        // If nDL != cnDigitsPerWord then we're not at the top.
        // And pwRoot is initialized despite what gcc might think.
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL);
      #if ! defined(LOOKUP)
        DBGX(printf("wPopCnt in link (before incr) %zd\n", (size_t)wPopCnt));
        set_PWR_wPopCnt(pwRoot, NULL, nDL, wPopCnt + nIncr);
        DBGX(printf("wPopCnt in link (after incr) %zd\n",
                    (size_t)PWR_wPopCnt(pwRoot, NULL, nDL)));
      #endif // ! defined(LOOKUP)
  #endif // defined(PP_IN_LINK)

  #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        // This short-circuit is for analysis only.
        return KeyFound;

  #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

      #if defined(PP_IN_LINK)
          // Adjust wPopCnt to actual list size for undo case.
          // There must be a better way to do this.
          #if defined(INSERT)
        if (nIncr == -1) { --wPopCnt; }
          #endif // defined(INSERT)
          #if defined(REMOVE)
        if (nIncr == 1) { ++wPopCnt; }
          #endif // defined(REMOVE)
      #else // defined(PP_IN_LINK)
        wPopCnt = ls_wPopCnt(pwr);
      #endif // defined(PP_IN_LINK)

      #if defined(LOOKUP)
        SMETRICS(j__SearchPopulation += wPopCnt);
      #endif // defined(LOOKUP)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COMPRESSED_LISTS)
          #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
        // nDL is relative to the bottom of the switch
        // containing the pointer to the leaf.
        // Can we use NAT here since bNeedPrefixCheck will never
        // be true if we are at the top?
        // If the top digit is smaller than the rest, then NAT will
        // return nBL > cnBitsPerWord which works out perfectly.
        unsigned nBL = nDL_to_nBL_NAT(nDL);
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
            || (LOG(1 | (PWR_wPrefixNAT(pwRootPrefix,
                                        (Switch_t *)pwrPrefix, nDLRPrefix)
                            ^ wKey))
                < nDL_to_nBL(nDLRPrefix))
              #else // defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, (Switch_t *)pwrPrev, nDL)
                    ^ wKey))
                < (nBL
                  #if ! defined(PP_IN_LINK)
                    // prefix in parent switch doesn't contain last digit
                    // for ! defined(PP_IN_LINK) case
                    + nDL_to_nBitsIndexSzNAT(nDL + 1)
                  #endif // ! defined(PP_IN_LINK)
                ))
              #endif // defined(SAVE_PREFIX)
            )
          #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #endif // defined(COMPRESSED_LISTS)
        {
      #if defined(DL_IN_LL)
            assert(ll_nDL(wRoot) == nDL);
      #endif // defined(DL_IN_LL)

      // LOOKUP_NO_LIST_SEARCH is for analysis only.  We have retrieved the
      // pop count and prefix but we have not dereferenced the list itself.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (SearchList(pwr, wKey,
#if defined(COMPRESSED_LISTS)
                           nBL,
#else // defined(COMPRESSED_LISTS)
                           cnBitsPerWord,
#endif // defined(COMPRESSED_LISTS)
                           wPopCnt)
                >= 0)
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
          #if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDL, wRoot); goto cleanup;
          #endif // defined(REMOVE)
          #if defined(INSERT) && ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
          #endif // defined(INSERT) && ! defined(RECURSIVE)
          #if defined(LOOKUP) && defined(SAVE_PREFIX_TEST_RESULT)
                if ( ! bPrefixMismatch )
          #endif // defined(INSERT) && defined(SAVE_PREFIX_TEST_RESULT)
                {
                    return KeyFound;
                }
            }
        }
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) \
          && defined(COMPRESSED_LISTS)
        else
        {
            DBGX(printf("Mismatch at list wPrefix "OWx" nDL %d\n",
                 PWR_wPrefixNAT(pwRoot, pwrPrev, nDL), nDL));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && ...

  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

  #endif // defined(LOOKUP) && defined(ONE_DEREF_AT_LIST)

        break;

    } // end of case T_LIST

#endif // (cwListPopCntMax != 0)

    case T_BITMAP:
#if defined(EXTRA_TYPES)
    case T_BITMAP | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
#if (cnBitsAtBottom <= cnLogBitsPerWord)
embeddedBitmap:
#endif // (cnBitsAtBottom <= cnLogBitsPerWord)
        // This case has been enhanced to handle a bitmap at any level.
        // It used to assume we were at nDL == 1.

#if defined(REMOVE)
        if (bCleanup) { return KeyFound; } // cleanup is complete
#endif // defined(REMOVE)

#if ! defined(LOOKUP) && defined(PP_IN_LINK)
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL);
        set_PWR_wPopCnt(pwRoot, NULL, nDL, wPopCnt + nIncr);
#endif // !defined(LOOKUP) && defined(PP_IN_LINK)

#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

  #if defined(SKIP_LINKS)
      // Code below uses NAT and we don't really enforce it so we put an
      // assertion here to remind us that not all values of cnBitsAtBottom
      // and cnBitsPerDigit will work for type-is-absolute aka
      // ! defined(TYPE_IS_RELATIVE).
      #if defined(PP_IN_LINK)
        assert(nDL < cnDigitsPerWord);
      #else // defined(PP_IN_LINK)
        assert(nDL + 1 < cnDigitsPerWord);
      #endif // defined(PP_IN_LINK)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We have to do the prefix check here.
        if ( 0
          #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            || ! bNeedPrefixCheck
          #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
          #if defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNAT(pwRootPrefix, pwrPrefix, nDLRPrefix)
                    ^ wKey))
                < nDL_to_nBL(nDLRPrefix))
          #else // defined(SAVE_PREFIX)
            // Notice that we're using pwr which was extracted from the
            // previous wRoot -- not the current wRoot -- to find the prefix,
            // if not PP_IN_LINK.  If PP_IN_LINK, then we are using the
            // current pwRoot to find the prefix.
            // nDL is different for the two cases.
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, (Switch_t *)pwrPrev, nDL)
                            ^ wKey))
                // The +1 is necessary because the pwrPrev
                // prefix does not contain any less significant bits.
              #if defined(PP_IN_LINK)
                < nDL_to_nBL_NAT(nDL    )
              #else // defined(PP_IN_LINK)
                < nDL_to_nBL_NAT(nDL + 1)
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
            assert(PWR_wPopCnt(pwRoot, pwrPrev,
              #if defined(PP_IN_LINK)
                nDL_to_nBL_NAT(nDL    )
              #else // defined(PP_IN_LINK)
                nDL_to_nBL_NAT(nDL + 1)
              #endif // defined(PP_IN_LINK)
                               ) != 0);
            return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            nBL = nDL_to_nBL_NAT(nDL);
      #if (cnBitsAtBottom <= cnLogBitsPerWord)
            // If nBL == cnLogBitsPerWord, then do we really need to
            // mask wKey or would the shift in the macro take care of it?
            // We definitely need the mask if nBL < cnLogBitsPerWord.
            if (BitIsSetInWord(wRoot, wKey & (EXP(nBL) - 1UL)))
      #else // (cnBitsAtBottom <= cnLogBitsPerWord)
            if (BitIsSet(wr_pwr(wRoot), wKey & (EXP(nBL) - 1UL)))
      #endif // (cnBitsAtBottom <= cnLogBitsPerWord)
            {
      #if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDL, wRoot);
                goto cleanup;
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
                        PWR_wPrefixNAT(pwRoot, pwrPrev, nDL)));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

        break;

    } // end of case T_BITMAP

#if defined(USE_T_ONE)

    // T_ONE is a one-key/word external leaf or an embedded/internal list.
    // The latter is only possible if EMBED_KEYS is defined.  In the latter
    // case an embedded list is assumed for one key if the key will fit.
    case T_ONE:
#if defined(EXTRA_TYPES)
    case T_ONE | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
  #if defined(REMOVE)
        if (bCleanup) { return Success; } // cleanup is complete
  #endif // defined(REMOVE)

  #if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (nDL != cnDigitsPerWord)
        {
            // Adjust pop count in the link on the way in.
            set_PWR_wPopCnt(pwRoot, NULL, nDL,
                PWR_wPopCnt(pwRoot, NULL, nDL) + nIncr);
        }
  #endif // ! defined(LOOKUP) && defined(PP_IN_LINK)

  #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

      #if defined(EMBED_KEYS)
        //
        // How many keys will fit?  Need space for the keys plus type plus
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
        unsigned nBL = nDL_to_nBL(nDL);
  #if defined(HAS_KEY)
        if (nBL == cnBitsAtDl1) {
            if (EmbeddedListHasKey(wRoot, wKey, nBL)) goto foundIt;
        } else if (nBL == cnBitsAtDl2) {
            if (EmbeddedListHasKey(wRoot, wKey, nBL)) goto foundIt;
#if defined(cnBitsAtDl3)
        } else if (nBL == cnBitsAtDl3) {
#else // defined(cnBitsAtDl3)
        } else if (nBL == cnBitsAtDl2 + cnBitsPerDigit) {
#endif // defined(cnBitsAtDl3)
            if (EmbeddedListHasKey(wRoot, wKey, nBL)) goto foundIt;
        } else
  #endif // defined(HAS_KEY)
        if (nBL <= cnBitsPerWord - cnBitsMallocMask) {
  #if defined(HAS_KEY)
            if (EmbeddedListHasKey(wRoot, wKey, nBL)) goto foundIt;
  #else // defined(HAS_KEY)
            // I wonder if PAD_T_ONE and not needing to know the pop count
            // would help this code like it does HAS_KEY.
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
  #endif // defined(HAS_KEY)
        } else
      #endif // defined(EMBED_KEYS)
        if (*pwr == wKey)
        {
  #if defined(EMBED_KEYS)
foundIt:
  #endif // defined(EMBED_KEYS)
      #if defined(REMOVE)
            RemoveGuts(pwRoot, wKey, nDL, wRoot);
            goto cleanup; // free memory or reconfigure tree if necessary
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

    case T_NULL:
#if defined(EXTRA_TYPES)
    case T_NULL | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        assert(wRoot == 0);

  // Adjust wPopCnt in link to leaf for PP_IN_LINK.
  // wPopCnt in link to switch is adjusted elsewhere, i.e. in the same place
  // as wPopCnt in switch is adjusted for pp-in-switch.
  #if defined(PP_IN_LINK)
      #if defined(REMOVE)
        // Can we combine bCleanup context with nType in switch variable?
        if ( ! bCleanup )
      #endif // defined(REMOVE)
        {
      #if ! defined(LOOKUP)
            // What about defined(RECURSIVE)?
            if (nDL != cnDigitsPerWord)
            {
                // If nDL != cnDigitsPerWord then we're not at top.
                // And pwRoot is initialized despite what gcc might think.
                if (PWR_wPopCnt(pwRoot, NULL, nDL) != 0) {
                    printf("\nhuh wPopCnt %d nIncr %d\n",
                          (int)PWR_wPopCnt(pwRoot, NULL, nDL), nIncr);
                }
                assert(PWR_wPopCnt(pwRoot, NULL, nDL) == 0);
                set_PWR_wPopCnt(pwRoot, NULL, nDL, nIncr);
            }
      #endif // ! defined(LOOKUP)
        }
  #endif // defined(PP_IN_LINK)

        break;

    } // end of case T_NULL

    } // end of switch

#if defined(BM_SWITCH_FOR_REAL)
notFound:
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(INSERT)
  #if defined(BM_IN_LINK)
    // If InsertGuts calls Insert, then it is always with the same
    // pwRoot and nDL that Insert passed to InsertGuts.
      #if !defined(RECURSIVE)
    assert((nDL != cnDigitsPerWord) || (pwRoot == pwRootOrig));
      #endif // !defined(RECURSIVE)
  #endif // defined(BM_IN_LINK)
    // InsertGuts is called with a pwRoot and nDL indicates the
    // bits that were not decoded in identifying pwRoot.  nDL
    // does not include any skip indicated in the type field of *pwRoot.
    return InsertGuts(pwRoot, wKey, nDL, wRoot);
undo:
#endif // defined(INSERT)
#if defined(REMOVE) && !defined(RECURSIVE)
    if (nIncr < 0)
#endif // defined(REMOVE) && !defined(RECURSIVE)
#if !defined(LOOKUP) && !defined(RECURSIVE)
    {
  #if defined(REMOVE)
        if (bCleanup) { return KeyFound; } // nothing to clean up
        printf("\n# Not bCleanup -- Remove failure wRoot "OWx"!\n", wRoot);
  #endif // defined(REMOVE)
        // Undo the counting we did on the way in.
        nIncr *= -1;
  #if defined(REMOVE)
restart:
  #endif // defined(REMOVE)
        pwRoot = pwRootOrig;
        nDL = nDLOrig;
        goto top;
    }
  #endif // !defined(LOOKUP) && !defined(RECURSIVE)
    return Failure;
  #if defined(REMOVE)
cleanup:
    bCleanup = 1; // ?? nIncr == 0 ??
    DBGX(printf("Cleanup pwRO "OWx" nDLO %d\n",
                (Word_t)pwRootOrig, nDLOrig));
    DBGR(Dump(pwRootOrig, /* wPrefix */ (Word_t)0, nDL_to_nBL(nDLOrig)));
    goto restart;
  #endif // defined(REMOVE)
}

#undef RECURSIVE
#undef InsertGuts
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

//#endif // (cnBitsAtBottom < cnBitsPerWord)
//#endif // (cnBitsPerDigit < cnBitsPerWord)
#endif // (cnDigitsPerWord > 1)

#if defined(LOOKUP)

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnDigitsPerWord > 1)

  #if (cwListPopCntMax != 0)
      #if defined(PP_IN_LINK) || defined(SEARCH_FROM_J1T)
    // Handle the top level T_LIST leaf here because for PP_IN_LINK a T_LIST
    // at the top has a pop count byte at the beginning and T_LISTs not at
    // the top do not.  I didn't want to have to have all of the mainline
    // list handling code have to know or test if it is at the top.
    // Do not assume the list is sorted here -- so this code doesn't have to
    // be ifdef'd.
    unsigned nType = wr_nType((Word_t)pcvRoot);
    if (nType == T_LIST)
    {
        Word_t *pwr = wr_tp_pwr((Word_t)pcvRoot, nType);

        // ls_wPopCount is valid only at the top for PP_IN_LINK
        // the first word in the list is used for pop count at the top
        return (SearchListWord(ls_pwKeys(pwr)
          #if defined(PP_IN_LINK)
                                  + (cnDummiesInList == 0)
          #endif // defined(PP_IN_LINK)
                                  ,
                          wKey, cnBitsPerWord, ls_wPopCnt(pwr)) >= 0)
                   ? Success : Failure;
    }
      #endif // defined(PP_IN_LINK) || defined(SEARCH_FROM_J1T)
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

#if defined(DEBUG)

static int bInitialized;

static void
Initialize(void)
{
#if defined(cnBitsInD1)
    // There may be an issue with dlmalloc and greater than 2MB (size of huge
    // page) requests. Dlmalloc may mmap something other than an integral
    // multiple of 2MB. Since our bitmaps contain an extra word at the end
    // we need to be careful about bitmaps that 2MB plus one word and bigger.
    assert((cnBitsLeftAtDl2 < 24)
        || ((cn2dBmWpkPercent == 0) && (cnBitsInD1 < 24)));
#endif // defined(cnBitsInD1)

    // We don't support NO_EMBED_KEYS with cnListPopCntMax<X> == 0.
#if ! defined(EMBED_KEYS)
    assert(cnListPopCntMaxDl1 != 0);
  #if defined(cnListPopCntMaxDl2)
    assert(cnListPopCntMaxDl2 != 0);
  #endif // defined(cnListPopCntMaxDl2)
  #if defined(cnListPopCntMaxDl3)
    assert(cnListPopCntMaxDl3 != 0);
  #endif // defined(cnListPopCntMaxDl3)
    assert(cnListPopCntMax8   != 0);
    assert(cnListPopCntMax16  != 0);
    assert(cnListPopCntMax32  != 0);
    assert(cnListPopCntMax64  != 0);
#endif // ! defined(EMBED_KEYS)

    // Search assumes lists are sorted if LIST_END_MARKERS is defined.
#if defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)
    assert(0);
#endif // defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)
#if defined(cnBitsInD1)
    // Why would we want to be able to fit a whole digit's worth of
    // keys into a Link_t as an embedded bitmap?
    // A real bitmap encompassing the whole switch would use no more
    // space and it would be faster.  Does JudyL change the situation?
    if (EXP(cnBitsInD1) <= sizeof(Link_t) * 8) {
        printf("Warning: (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)"
               " makes no sense.\n");
        printf("Try increasing cnBitsInD1 or decreasing sizeof(Link_t).\n");
    }
    assert(EXP(cnBitsInD1) > sizeof(Link_t) * 8);
    // Why would we want to be able to fit more than one digits' worth of
    // keys into a Link_t as an embedded bitmap?
    if (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8) {
        printf("Warning: (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8)"
               " makes no sense.\n");
        printf("Try increasing cnBitsInD1 or decreasing sizeof(Link_t).\n");
        printf("Or try increasing cnBitsPerDigit.\n");
    }
    assert(EXP(cnBitsLeftAtDl2) > sizeof(Link_t) * 8);
#endif // defined(cnBitsInD1)
#if ! defined(LVL_IN_WR_HB)
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
#endif // ! defined(LVL_IN_WR_HB)

#if defined(T_SWITCH_BIT)
#if defined(SEPARATE_T_NULL)
    assert(((T_SKIP_BIT | T_SWITCH_BIT) & T_NULL) == 0);
#endif // defined(SEPARATE_T_NULL)
    assert(((T_SKIP_BIT | T_SWITCH_BIT) & T_LIST) == 0);
#if defined(USE_T_ONE)
    assert(((T_SKIP_BIT | T_SWITCH_BIT) & T_ONE) == 0);
#endif // defined(USE_T_ONE)
#if defined(EMBED_KEYS)
    assert(((T_SKIP_BIT | T_SWITCH_BIT) & T_EMBEDDED_KEYS) == 0);
#endif // defined(EMBED_KEYS)
    assert(((T_SKIP_BIT | T_SWITCH_BIT) & T_BITMAP) == 0);
#endif // defined(T_SWITCH_BIT)

  #if defined(NO_TYPE_IN_XX_SW)
      #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
    // Not sure if/why this matters.
    assert (wr_nType(ZERO_POP_MAGIC) == T_EMBEDDED_KEYS);
      #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)
  #endif // defined(NO_TYPE_IN_XX_SW)

  #if defined(CODE_XX_SW)
    // Make sure nBW field is big enough.
    assert((cnBitsLeftAtDl2 - cnBW - (cnLogBitsPerWord + 1))
        <= MSK(cnBitsXxSwWidth));
  #endif // defined(CODE_XX_SW)

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

#if defined(POP_IN_WR_HB)
    printf("#    POP_IN_WR_HB\n");
#else // defined(POP_IN_WR_HB)
    printf("# NO POP_IN_WR_HB\n");
#endif // defined(POP_IN_WR_HB)

#if defined(LVL_IN_WR_HB)
    printf("#    LVL_IN_WR_HB\n");
#else // defined(LVL_IN_WR_HB)
    printf("# NO LVL_IN_WR_HB\n");
#endif // defined(LVL_IN_WR_HB)

#if defined(TYPE_IS_ABSOLUTE)
    printf("#    TYPE_IS_ABSOLUTE\n");
#else // defined(TYPE_IS_ABSOLUTE)
    printf("# NO TYPE_IS_ABSOLUTE\n");
#endif // defined(TYPE_IS_ABSOLUTE)

#if defined(COMPRESSED_LISTS)
    printf("#    COMPRESSED_LISTS\n");
#else // defined(COMPRESSED_LISTS)
    printf("# NO COMPRESSED_LISTS\n");
#endif // defined(COMPRESSED_LISTS)

#if defined(SORT_LISTS)
    printf("#    SORT_LISTS\n");
#else // defined(SORT_LISTS)
    printf("# NO SORT_LISTS\n");
#endif // defined(SORT_LISTS)

#if defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)
    printf("#    _ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL\n");
#else // defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)
    printf("# NO _ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL\n");
#endif // defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)

#if defined(ALIGN_LISTS)
    printf("#    ALIGN_LISTS\n");
#else // defined(ALIGN_LISTS)
    printf("# NO ALIGN_LISTS\n");
#endif // defined(ALIGN_LISTS)

#if defined(ALIGN_LIST_ENDS)
    printf("#    ALIGN_LIST_ENDS\n");
#else // defined(ALIGN_LIST_ENDS)
    printf("# NO ALIGN_LIST_ENDS\n");
#endif // defined(ALIGN_LIST_ENDS)

#if defined(SKIP_LINKS)
    printf("#    SKIP_LINKS\n");
#else // defined(SKIP_LINKS)
    printf("# NO SKIP_LINKS\n");
#endif // defined(SKIP_LINKS)

#if defined(SKIP_TO_BITMAP)
    printf("#    SKIP_TO_BITMAP\n");
#else // defined(SKIP_TO_BITMAP)
    printf("# NO SKIP_TO_BITMAP\n");
#endif // defined(SKIP_TO_BITMAP)

#if defined(USE_XX_SW)
    printf("#    USE_XX_SW\n");
#else // defined(USE_XX_SW)
    printf("# NO USE_XX_SW\n");
#endif // defined(USE_XX_SW)

#if defined(CODE_XX_SW)
    printf("#    CODE_XX_SW\n");
#else // defined(CODE_XX_SW)
    printf("# NO CODE_XX_SW\n");
#endif // defined(CODE_XX_SW)

#if defined(SKIP_TO_XX_SW)
    printf("#    SKIP_TO_XX_SW\n");
#else // defined(SKIP_TO_XX_SW)
    printf("# NO SKIP_TO_XX_SW\n");
#endif // defined(SKIP_TO_XX_SW)

#if defined(XX_SHORTCUT)
    printf("#    XX_SHORTCUT\n");
#else // defined(XX_SHORTCUT)
    printf("# NO XX_SHORTCUT\n");
#endif // defined(XX_SHORTCUT)

#if defined(XX_SHORTCUT_GOTO)
    printf("#    XX_SHORTCUT_GOTO\n");
#else // defined(XX_SHORTCUT_GOTO)
    printf("# NO XX_SHORTCUT_GOTO\n");
#endif // defined(XX_SHORTCUT_GOTO)

#if defined(EMBED_KEYS)
    printf("#    EMBED_KEYS\n");
#else // defined(EMBED_KEYS)
    printf("# NO EMBED_KEYS\n");
#endif // defined(EMBED_KEYS)

#if defined(USE_T_ONE)
    printf("#    USE_T_ONE\n");
#else // defined(USE_T_ONE)
    printf("# NO USE_T_ONE\n");
#endif // defined(USE_T_ONE)

#if defined(T_ONE_CALC_POP)
    printf("#    T_ONE_CALC_POP\n");
#else // defined(T_ONE_CALC_POP)
    printf("# NO T_ONE_CALC_POP\n");
#endif // defined(T_ONE_CALC_POP)

#if defined(DL_SPECIFIC_T_ONE)
    printf("#    DL_SPECIFIC_T_ONE\n");
#else // defined(DL_SPECIFIC_T_ONE)
    printf("# NO DL_SPECIFIC_T_ONE\n");
#endif // defined(DL_SPECIFIC_T_ONE)

#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    printf("#    BL_SPECIFIC_PSPLIT_SEARCH\n");
#else // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    printf("# NO BL_SPECIFIC_PSPLIT_SEARCH\n");
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)

#if defined(PSPLIT_SEARCH_8)
    printf("#    PSPLIT_SEARCH_8\n");
#else // defined(PSPLIT_SEARCH_8)
    printf("# NO PSPLIT_SEARCH_8\n");
#endif // defined(PSPLIT_SEARCH_8)

#if defined(PSPLIT_SEARCH_16)
    printf("#    PSPLIT_SEARCH_16\n");
#else // defined(PSPLIT_SEARCH_16)
    printf("# NO PSPLIT_SEARCH_16\n");
#endif // defined(PSPLIT_SEARCH_16)

#if defined(PSPLIT_SEARCH_32)
    printf("#    PSPLIT_SEARCH_32\n");
#else // defined(PSPLIT_SEARCH_32)
    printf("# NO PSPLIT_SEARCH_32\n");
#endif // defined(PSPLIT_SEARCH_32)

#if defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)
    printf("# NO EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_PARALLEL_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)
    printf("# NO EMBEDDED_KEYS_PARALLEL_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)

#if defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)
    printf("# NO EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)
    printf("# NO EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)

#if defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)
    printf("# NO EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_UNROLLED_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)
    printf("# NO EMBEDDED_KEYS_UNROLLED_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)

#if defined(PSPLIT_PARALLEL)
    printf("#    PSPLIT_PARALLEL\n");
#else // defined(PSPLIT_PARALLEL)
    printf("# NO PSPLIT_PARALLEL\n");
#endif // defined(PSPLIT_PARALLEL)

#if defined(PARALLEL_128)
    printf("#    PARALLEL_128\n");
#else // defined(PARALLEL_128)
    printf("# NO PARALLEL_128\n");
#endif // defined(PARALLEL_128)

#if defined(PSPLIT_EARLY_OUT)
    printf("#    PSPLIT_EARLY_OUT\n");
#else // defined(PSPLIT_EARLY_OUT)
    printf("# NO PSPLIT_EARLY_OUT\n");
#endif // defined(PSPLIT_EARLY_OUT)

#if defined(BINARY_SEARCH_WORD)
    printf("#    BINARY_SEARCH_WORD\n");
#else // defined(BINARY_SEARCH_WORD)
    printf("# NO BINARY_SEARCH_WORD\n");
#endif // defined(BINARY_SEARCH_WORD)

#if defined(RAMMETRICS)
    printf("#    RAMMETRICS\n");
#else // defined(RAMMETRICS)
    printf("# NO RAMMETRICS\n");
#endif // defined(RAMMETRICS)

#if defined(JUDYA)
    printf("#    JUDYA\n");
#else // defined(JUDYA)
    printf("# NO JUDYA\n");
#endif // defined(JUDYA)

#if defined(NDEBUG)
    printf("#    NDEBUG\n");
#else // defined(NDEBUG)
    printf("# NO NDEBUG\n");
#endif // defined(NDEBUG)

#if defined(SPLIT_SEARCH_BINARY)
    printf("#    SPLIT_SEARCH_BINARY\n");
#else // defined(SPLIT_SEARCH_BINARY)
    printf("# NO SPLIT_SEARCH_BINARY\n");
#endif // defined(SPLIT_SEARCH_BINARY)

#if defined(PSPLIT_HYBRID)
    printf("#    PSPLIT_HYBRID\n");
#else // defined(PSPLIT_HYBRID)
    printf("# NO PSPLIT_HYBRID\n");
#endif // defined(PSPLIT_HYBRID)

#if defined(PSPLIT_SEARCH_WORD)
    printf("#    PSPLIT_SEARCH_WORD\n");
#else // defined(PSPLIT_SEARCH_WORD)
    printf("# NO PSPLIT_SEARCH_WORD\n");
#endif // defined(PSPLIT_SEARCH_WORD)

#if defined(PSPLIT_SEARCH_XOR_WORD)
    printf("#    PSPLIT_SEARCH_XOR_WORD\n");
#else // defined(PSPLIT_SEARCH_XOR_WORD)
    printf("# NO PSPLIT_SEARCH_XOR_WORD\n");
#endif // defined(PSPLIT_SEARCH_XOR_WORD)

#if defined(LIST_END_MARKERS)
    printf("#    LIST_END_MARKERS\n");
#else // defined(LIST_END_MARKERS)
    printf("# NO LIST_END_MARKERS\n");
#endif // defined(LIST_END_MARKERS)

#if defined(OLD_LISTS)
    printf("#    OLD_LISTS\n");
#else // defined(OLD_LISTS)
    printf("# NO OLD_LISTS\n");
#endif // defined(OLD_LISTS)

#if defined(BACKWARD_SEARCH_WORD)
    printf("#    BACKWARD_SEARCH_WORD\n");
#else // defined(BACKWARD_SEARCH_WORD)
    printf("# NO BACKWARD_SEARCH_WORD\n");
#endif // defined(BACKWARD_SEARCH_WORD)

#if defined(TRY_MEMCHR)
    printf("#    TRY_MEMCHR\n");
#else // defined(TRY_MEMCHR)
    printf("# NO TRY_MEMCHR\n");
#endif // defined(TRY_MEMCHR)

#if defined(RECURSIVE)
    printf("#    RECURSIVE\n");
#else // defined(RECURSIVE)
    printf("# NO RECURSIVE\n");
#endif // defined(RECURSIVE)

#if defined(RECURSIVE_INSERT)
    printf("#    RECURSIVE_INSERT\n");
#else // defined(RECURSIVE_INSERT)
    printf("# NO RECURSIVE_INSERT\n");
#endif // defined(RECURSIVE_INSERT)

#if defined(RECURSIVE_REMOVE)
    printf("#    RECURSIVE_REMOVE\n");
#else // defined(RECURSIVE_REMOVE)
    printf("# NO RECURSIVE_REMOVE\n");
#endif // defined(RECURSIVE_REMOVE)

#if defined(LOOKUP_NO_LIST_DEREF)
    printf("#    LOOKUP_NO_LIST_DEREF\n");
#else // defined(LOOKUP_NO_LIST_DEREF)
    printf("# NO LOOKUP_NO_LIST_DEREF\n");
#endif // defined(LOOKUP_NO_LIST_DEREF)

#if defined(LOOKUP_NO_LIST_SEARCH)
    printf("#    LOOKUP_NO_LIST_SEARCH\n");
#else // defined(LOOKUP_NO_LIST_SEARCH)
    printf("# NO LOOKUP_NO_LIST_SEARCH\n");
#endif // defined(LOOKUP_NO_LIST_SEARCH)

#if defined(ONE_DEREF_AT_LIST)
    printf("#    ONE_DEREF_AT_LIST\n");
#else // defined(ONE_DEREF_AT_LIST)
    printf("# NO ONE_DEREF_AT_LIST\n");
#endif // defined(ONE_DEREF_AT_LIST)

#if defined(LOOKUP_NO_BITMAP_DEREF)
    printf("#    LOOKUP_NO_BITMAP_DEREF\n");
#else // defined(LOOKUP_NO_BITMAP_DEREF)
    printf("# NO LOOKUP_NO_BITMAP_DEREF\n");
#endif // defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(LOOKUP_NO_BITMAP_SEARCH)
    printf("#    LOOKUP_NO_BITMAP_SEARCH\n");
#else // defined(LOOKUP_NO_BITMAP_SEARCH)
    printf("# NO LOOKUP_NO_BITMAP_SEARCH\n");
#endif // defined(LOOKUP_NO_BITMAP_SEARCH)

#if defined(ZERO_POP_CHECK_BEFORE_GOTO)
    printf("#    ZERO_POP_CHECK_BEFORE_GOTO\n");
#else // defined(ZERO_POP_CHECK_BEFORE_GOTO)
    printf("# NO ZERO_POP_CHECK_BEFORE_GOTO\n");
#endif // defined(ZERO_POP_CHECK_BEFORE_GOTO)

#if defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    printf("#    HANDLE_DL2_IN_EMBEDDED_KEYS\n");
#else // defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    printf("# NO HANDLE_DL2_IN_EMBEDDED_KEYS\n");
#endif // defined(HANDLE_DL2_IN_EMBEDDED_KEYS)

#if defined(HANDLE_BLOWOUTS)
    printf("#    HANDLE_BLOWOUTS\n");
#else // defined(HANDLE_BLOWOUTS)
    printf("# NO HANDLE_BLOWOUTS\n");
#endif // defined(HANDLE_BLOWOUTS)

#if defined(NO_TYPE_IN_XX_SW)
    printf("#    NO_TYPE_IN_XX_SW\n");
#else // defined(NO_TYPE_IN_XX_SW)
    printf("# NO NO_TYPE_IN_XX_SW\n");
#endif // defined(NO_TYPE_IN_XX_SW)

#if defined(FILL_W_KEY)
    printf("#    FILL_W_KEY\n");
#else // defined(FILL_W_KEY)
    printf("# NO FILL_W_KEY\n");
#endif // defined(FILL_W_KEY)

#if defined(FILL_W_BIG_KEY)
    printf("#    FILL_W_BIG_KEY\n");
#else // defined(FILL_W_BIG_KEY)
    printf("# NO FILL_W_BIG_KEY\n");
#endif // defined(FILL_W_BIG_KEY)

#if defined(FILL_WITH_ONES)
    printf("#    FILL_WITH_ONES\n");
#else // defined(FILL_WITH_ONES)
    printf("# NO FILL_WITH_ONES\n");
#endif // defined(FILL_WITH_ONES)

#if defined(MASK_EMPTIES)
    printf("#    MASK_EMPTIES\n");
#else // defined(MASK_EMPTIES)
    printf("# NO MASK_EMPTIES\n");
#endif // defined(MASK_EMPTIES)

#if defined(EMBEDDED_LIST_FIXED_POP)
    printf("#    EMBEDDED_LIST_FIXED_POP\n");
#else // defined(EMBEDDED_LIST_FIXED_POP)
    printf("# NO EMBEDDED_LIST_FIXED_POP\n");
#endif // defined(EMBEDDED_LIST_FIXED_POP)

#if defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)
    printf("#    USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL\n");
#else // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)
    printf("# NO USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL\n");
#endif // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

#if defined(REVERSE_SORT_EMBEDDED_KEYS)
    printf("#    REVERSE_SORT_EMBEDDED_KEYS\n");
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)
    printf("# NO REVERSE_SORT_EMBEDDED_KEYS\n");
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)

#if defined(EXTRA_TYPES)
    printf("#    EXTRA_TYPES\n");
#else // defined(EXTRA_TYPES)
    printf("# NO EXTRA_TYPES\n");
#endif // defined(EXTRA_TYPES)

#if defined(POP_WORD)
    printf("#    POP_WORD\n");
#else // defined(POP_WORD)
    printf("# NO POP_WORD\n");
#endif // defined(POP_WORD)

#if defined(POP_WORD_IN_LINK)
    printf("#    POP_WORD_IN_LINK\n");
#else // defined(POP_WORD_IN_LINK)
    printf("# NO POP_WORD_IN_LINK\n");
#endif // defined(POP_WORD_IN_LINK)

#if defined(SEARCHMETRICS)
    printf("#    SEARCHMETRICS\n");
#else // defined(SEARCHMETRICS)
    printf("# NO SEARCHMETRICS\n");
#endif // defined(SEARCHMETRICS)

#if defined(JUDYB)
    printf("#    JUDYB\n");
#else // defined(JUDYB)
    printf("# NO JUDYB\n");
#endif // defined(JUDYB)

#if defined(PWROOT_ARG_FOR_LOOKUP)
    printf("#    PWROOT_ARG_FOR_LOOKUP\n");
#else // defined(PWROOT_ARG_FOR_LOOKUP)
    printf("# NO PWROOT_ARG_FOR_LOOKUP\n");
#endif // defined(PWROOT_ARG_FOR_LOOKUP)

#if defined(PWROOT_AT_TOP_FOR_LOOKUP)
    printf("#    PWROOT_AT_TOP_FOR_LOOKUP\n");
#else // defined(PWROOT_AT_TOP_FOR_LOOKUP)
    printf("# NO PWROOT_AT_TOP_FOR_LOOKUP\n");
#endif // defined(PWROOT_AT_TOP_FOR_LOOKUP)

#if defined(USE_PWROOT_FOR_LOOKUP)
    printf("#    USE_PWROOT_FOR_LOOKUP\n");
#else // defined(USE_PWROOT_FOR_LOOKUP)
    printf("# NO USE_PWROOT_FOR_LOOKUP\n");
#endif // defined(USE_PWROOT_FOR_LOOKUP)

#if defined(DEPTH_IN_SW)
    printf("#    DEPTH_IN_SW\n");
#else // defined(DEPTH_IN_SW)
    printf("# NO DEPTH_IN_SW\n");
#endif // defined(DEPTH_IN_SW)

#if defined(TYPE_IS_RELATIVE)
    printf("#    TYPE_IS_RELATIVE\n");
#else // defined(TYPE_IS_RELATIVE)
    printf("# NO TYPE_IS_RELATIVE\n");
#endif // defined(TYPE_IS_RELATIVE)

#if defined(SEPARATE_T_NULL)
    printf("#    SEPARATE_T_NULL\n");
#else // defined(SEPARATE_T_NULL)
    printf("# NO SEPARATE_T_NULL\n");
#endif // defined(SEPARATE_T_NULL)

#if defined(SKIP_TO_LIST)
    printf("#    SKIP_TO_LIST\n");
#else // defined(SKIP_TO_LIST)
    printf("# NO SKIP_TO_LIST\n");
#endif // defined(SKIP_TO_LIST)

#if defined(BPD_TABLE)
    printf("#    BPD_TABLE\n");
#else // defined(BPD_TABLE)
    printf("# NO BPD_TABLE\n");
#endif // defined(BPD_TABLE)

#if defined(BPD_TABLE_RUNTIME_INIT)
    printf("#    BPD_TABLE_RUNTIME_INIT\n");
#else // defined(BPD_TABLE_RUNTIME_INIT)
    printf("# NO BPD_TABLE_RUNTIME_INIT\n");
#endif // defined(BPD_TABLE_RUNTIME_INIT)

#if defined(USE_BM_SW)
    printf("#    USE_BM_SW\n");
#else // defined(USE_BM_SW)
    printf("# NO USE_BM_SW\n");
#endif // defined(USE_BM_SW)

#if defined(CODE_BM_SW)
    printf("#    CODE_BM_SW\n");
#else // defined(CODE_BM_SW)
    printf("# NO CODE_BM_SW\n");
#endif // defined(CODE_BM_SW)

#if defined(BM_SW_FOR_REAL)
    printf("#    BM_SW_FOR_REAL\n");
#else // defined(BM_SW_FOR_REAL)
    printf("# NO BM_SW_FOR_REAL\n");
#endif // defined(BM_SW_FOR_REAL)

#if defined(BITMAP_BY_BYTE)
    printf("#    BITMAP_BY_BYTE\n");
#else // defined(BITMAP_BY_BYTE)
    printf("# NO BITMAP_BY_BYTE\n");
#endif // defined(BITMAP_BY_BYTE)

#if defined(SKIP_TO_BM_SW)
    printf("#    SKIP_TO_BM_SW\n");
#else // defined(SKIP_TO_BM_SW)
    printf("# NO SKIP_TO_BM_SW\n");
#endif // defined(SKIP_TO_BM_SW)

#if defined(NO_OLD_LISTS)
    printf("#    NO_OLD_LISTS\n");
#else // defined(NO_OLD_LISTS)
    printf("# NO NO_OLD_LISTS\n");
#endif // defined(NO_OLD_LISTS)

#if defined(NO_SKIP_AT_TOP)
    printf("#    NO_SKIP_AT_TOP\n");
#else // defined(NO_SKIP_AT_TOP)
    printf("# NO NO_SKIP_AT_TOP\n");
#endif // defined(NO_SKIP_AT_TOP)

#if defined(PACK_KEYS_RIGHT)
    printf("#    PACK_KEYS_RIGHT\n");
#else // defined(PACK_KEYS_RIGHT)
    printf("# NO PACK_KEYS_RIGHT\n");
#endif // defined(PACK_KEYS_RIGHT)

#if defined(RETYPE_FULL_BM_SW)
    printf("#    RETYPE_FULL_BM_SW\n");
#else // defined(RETYPE_FULL_BM_SW)
    printf("# NO RETYPE_FULL_BM_SW\n");
#endif // defined(SKIP_TO_BM_SW)

#if defined(BM_IN_NON_BM_SW)
    printf("#    BM_IN_NON_BM_SW\n");
#else // defined(BM_IN_NON_BM_SW)
    printf("# NO BM_IN_NON_BM_SW\n");
#endif // defined(BM_IN_NON_BM_SW)

#if defined(BM_IN_LINK)
    printf("#    BM_IN_LINK\n");
#else // defined(BM_IN_LINK)
    printf("# NO BM_IN_LINK\n");
#endif // defined(BM_IN_LINK)

#if defined(PP_IN_LINK)
    printf("#    PP_IN_LINK\n");
#else // defined(PP_IN_LINK)
    printf("# NO PP_IN_LINK\n");
#endif // defined(PP_IN_LINK)

#if defined(SKIP_PREFIX_CHECK)
    printf("#    SKIP_PREFIX_CHECK\n");
#else // defined(SKIP_PREFIX_CHECK)
    printf("# NO SKIP_PREFIX_CHECK\n");
#endif // defined(SKIP_PREFIX_CHECK)

#if defined(NO_UNNECESSARY_PREFIX)
    printf("#    NO_UNNECESSARY_PREFIX\n");
#else // defined(NO_UNNECESSARY_PREFIX)
    printf("# NO NO_UNNECESSARY_PREFIX\n");
#endif // defined(NO_UNNECESSARY_PREFIX)

#if defined(SAVE_PREFIX)
    printf("#    SAVE_PREFIX\n");
#else // defined(SAVE_PREFIX)
    printf("# NO SAVE_PREFIX\n");
#endif // defined(SAVE_PREFIX)

#if defined(SAVE_PREFIX_TEST_RESULT)
    printf("#    SAVE_PREFIX_TEST_RESULT\n");
#else // defined(SAVE_PREFIX_TEST_RESULT)
    printf("# NO SAVE_PREFIX_TEST_RESULT\n");
#endif // defined(SAVE_PREFIX_TEST_RESULT)

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    printf("#    ALWAYS_CHECK_PREFIX_AT_LEAF\n");
#else // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    printf("# NO ALWAYS_CHECK_PREFIX_AT_LEAF\n");
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)

#if defined(SEARCH_FROM_WRAPPER)
    printf("#    SEARCH_FROM_WRAPPER\n");
#else // defined(SEARCH_FROM_WRAPPER)
    printf("# NO SEARCH_FROM_WRAPPER\n");
#endif // defined(SEARCH_FROM_WRAPPER)

#if defined(DEBUG)
    printf("#    DEBUG\n");
#else // defined(DEBUG)
    printf("# NO DEBUG\n");
#endif // defined(DEBUG)

#if defined(DEBUG_ALL)
    printf("#    DEBUG_ALL\n");
#else // defined(DEBUG_ALL)
    printf("# NO DEBUG_ALL\n");
#endif // defined(DEBUG_ALL)

#if defined(DEBUG_INSERT)
    printf("#    DEBUG_INSERT\n");
#else // defined(DEBUG_INSERT)
    printf("# NO DEBUG_INSERT\n");
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_REMOVE)
    printf("#    DEBUG_REMOVE\n");
#else // defined(DEBUG_REMOVE)
    printf("# NO DEBUG_REMOVE\n");
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_LOOKUP)
    printf("#    DEBUG_LOOKUP\n");
#else // defined(DEBUG_LOOKUP)
    printf("# NO DEBUG_LOOKUP\n");
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_MALLOC)
    printf("#    DEBUG_MALLOC\n");
#else // defined(DEBUG_MALLOC)
    printf("# NO DEBUG_MALLOC\n");
#endif // defined(DEBUG_MALLOC)

#if defined(GUARDBAND)
    printf("#    GUARDBAND\n");
#else // defined(GUARDBAND)
    printf("# NO GUARDBAND\n");
#endif // defined(GUARDBAND)

#if defined(DEBUG_COUNT)
    printf("#    DEBUG_COUNT\n");
#else // defined(DEBUG_COUNT)
    printf("# NO DEBUG_COUNT\n");
#endif // defined(DEBUG_COUNT)

#if defined(NO_SKIP_AT_TOP)
    printf("#    NO_SKIP_AT_TOP\n");
#else // defined(NO_SKIP_AT_TOP)
    printf("# NO NO_SKIP_AT_TOP\n");
#endif // defined(NO_SKIP_AT_TOP)

#if defined(NO_USE_XX_SW)
    printf("#    NO_USE_XX_SW\n");
#else // defined(NO_USE_XX_SW)
    printf("# NO NO_USE_XX_SW\n");
#endif // defined(NO_USE_XX_SW)

#if defined(NO_XX_SHORTCUT)
    printf("#    NO_XX_SHORTCUT\n");
#else // defined(NO_XX_SHORTCUT)
    printf("# NO NO_XX_SHORTCUT\n");
#endif // defined(NO_XX_SHORTCUT)

#if defined(NO_XX_SHORTCUT_GOTO)
    printf("#    NO_XX_SHORTCUT_GOTO\n");
#else // defined(NO_XX_SHORTCUT_GOTO)
    printf("# NO NO_XX_SHORTCUT_GOTO\n");
#endif // defined(NO_XX_SHORTCUT_GOTO)

#if defined(NO_DL_SPECIFIC_T_ONE)
    printf("#    NO_DL_SPECIFIC_T_ONE\n");
#else // defined(NO_DL_SPECIFIC_T_ONE)
    printf("# NO NO_DL_SPECIFIC_T_ONE\n");
#endif // defined(NO_DL_SPECIFIC_T_ONE)

#if defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("#    NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#else // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("# NO NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#endif // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)

#if defined(NO_PARALLEL_128)
    printf("#    NO_PARALLEL_128\n");
#else // defined(NO_PARALLEL_128)
    printf("# NO NO_PARALLEL_128\n");
#endif // defined(NO_PARALLEL_128)

#if defined(NO_SORT_LISTS)
    printf("#    NO_SORT_LISTS\n");
#else // defined(NO_SORT_LISTS)
    printf("# NO NO_SORT_LISTS\n");
#endif // defined(NO_SORT_LISTS)

#if defined(NO_PSPLIT_PARALLEL)
    printf("#    NO_PSPLIT_PARALLEL\n");
#else // defined(NO_PSPLIT_PARALLEL)
    printf("# NO NO_PSPLIT_PARALLEL\n");
#endif // defined(NO_PSPLIT_PARALLEL)

#if defined(NO_PSPLIT_EARLY_OUT)
    printf("#    NO_PSPLIT_EARLY_OUT\n");
#else // defined(NO_PSPLIT_EARLY_OUT)
    printf("# NO NO_PSPLIT_EARLY_OUT\n");
#endif // defined(NO_PSPLIT_EARLY_OUT)

#if defined(NO_LVL_IN_WR_HB)
    printf("#    NO_LVL_IN_WR_HB\n");
#else // defined(NO_LVL_IN_WR_HB)
    printf("# NO NO_LVL_IN_WR_HB\n");
#endif // defined(NO_LVL_IN_WR_HB)

#if defined(NO_SKIP_TO_XX_SW)
    printf("#    NO_SKIP_TO_XX_SW\n");
#else // defined(NO_SKIP_TO_XX_SW)
    printf("# NO NO_SKIP_TO_XX_SW\n");
#endif // defined(NO_SKIP_TO_XX_SW)

#if defined(NO_SKIP_TO_BM_SW)
    printf("#    NO_SKIP_TO_BM_SW\n");
#else // defined(NO_SKIP_TO_BM_SW)
    printf("# NO NO_SKIP_TO_BM_SW\n");
#endif // defined(NO_SKIP_TO_BM_SW)

#if defined(NO_POP_IN_WR_HB)
    printf("#    NO_POP_IN_WR_HB\n");
#else // defined(NO_POP_IN_WR_HB)
    printf("# NO NO_POP_IN_WR_HB\n");
#endif // defined(NO_POP_IN_WR_HB)

#if defined(NO_EMBED_KEYS)
    printf("#    NO_EMBED_KEYS\n");
#else // defined(NO_EMBED_KEYS)
    printf("# NO NO_EMBED_KEYS\n");
#endif // defined(NO_EMBED_KEYS)

#if defined(NO_T_ONE_CALC_POP)
    printf("#    NO_T_ONE_CALC_POP\n");
#else // defined(NO_T_ONE_CALC_POP)
    printf("# NO NO_T_ONE_CALC_POP\n");
#endif // defined(NO_T_ONE_CALC_POP)

#if defined(NO_PSPLIT_SEARCH_8)
    printf("#    NO_PSPLIT_SEARCH_8\n");
#else // defined(NO_PSPLIT_SEARCH_8)
    printf("# NO NO_PSPLIT_SEARCH_8\n");
#endif // defined(NO_PSPLIT_SEARCH_8)

#if defined(NO_PSPLIT_SEARCH_16)
    printf("#    NO_PSPLIT_SEARCH_16\n");
#else // defined(NO_PSPLIT_SEARCH_16)
    printf("# NO NO_PSPLIT_SEARCH_16\n");
#endif // defined(NO_PSPLIT_SEARCH_16)

#if defined(NO_PSPLIT_SEARCH_32)
    printf("#    NO_PSPLIT_SEARCH_32\n");
#else // defined(NO_PSPLIT_SEARCH_32)
    printf("# NO NO_PSPLIT_SEARCH_32\n");
#endif // defined(NO_PSPLIT_SEARCH_32)

#if defined(NO_BINARY_SEARCH_WORD)
    printf("#    NO_BINARY_SEARCH_WORD\n");
#else // defined(NO_BINARY_SEARCH_WORD)
    printf("# NO NO_BINARY_SEARCH_WORD\n");
#endif // defined(NO_BINARY_SEARCH_WORD)

#if defined(BM_IN_LINK)
    printf("\n");
    printf("# BM_IN_LINK is buggy.\n");
    printf("# With two-digit bitmap leaf conversion.\n");
    printf("# With bmsw to uncompressed conversion.\n");
    printf("# With retype full bmsw if BM_IN_NON_BM_SW.\n");
    printf("# With skip at top?\n");
    printf("# With PP_IN_LINK unless NO_SKIP_AT_TOP?\n");
#endif // defined(BM_IN_LINK)

    printf("\n");
#if defined(cnBitsInD1)
    printf("# cnBitsInD1 %d\n", cnBitsInD1);
    printf("# cnBitsInD2 %d\n", cnBitsInD2);
    printf("# cnBitsInD3 %d\n", cnBitsInD3);
#endif // defined(cnBitsInD1)
    printf("\n");
    printf("# cnBitsPerWord %d\n", cnBitsPerWord);
    printf("# cnBitsPerDigit %d\n", cnBitsPerDigit);
    printf("# cnDigitsPerWord %d\n", cnDigitsPerWord);
    printf("\n");
    printf("# cnListPopCntMax8  %d\n", cnListPopCntMax8);
    printf("# cnListPopCntMax16 %d\n", cnListPopCntMax16);
    printf("# cnListPopCntMax32 %d\n", cnListPopCntMax32);
    printf("# cnListPopCntMax64 %d\n", cnListPopCntMax64);
    printf("\n");
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
    printf("\n");
    printf("# cwListPopCntMax %d\n", cwListPopCntMax);
#if defined(cnNonBmLeafPopCntMax)
    printf("# cnNonBmLeafPopCntMax %d\n", cnNonBmLeafPopCntMax);
#else // defined(cnNonBmLeafPopCntMax)
    printf("# cnNonBmLeafPopCntMax n/a\n");
#endif // defined(cnNonBmLeafPopCntMax)
    //printf("# cn2dBmWpkPercent %d\n", cn2dBmWpkPercent);

#if defined(CODE_XX_SW)
    printf("\n");
    printf("# cnBW %d\n", cnBW);
    printf("# cnBWIncr %d\n", cnBWIncr);
#endif // defined(CODE_XX_SW)

#if defined(USE_BM_SW)
    printf("\n");
    printf("# cnBmSwLinksPercent %d\n", cnBmSwLinksPercent);
    printf("# cnBmSwWpkPercent %d\n", cnBmSwWpkPercent);
#endif // defined(USE_BM_SW)

    printf("\n");
    printf("# cnBinarySearchThresholdWord %d\n", cnBinarySearchThresholdWord);

#if 0 // Missing EmbeddedListPopCntMax.
#if defined(EMBED_KEYS)
    printf("\n");
    //int nPopCntMaxPrev = -1;
    for (int nBL = cnBitsPerWord; nBL > 0; --nBL) {
        int nPopCntMax;
        nPopCntMax = EmbeddedListPopCntMax(nBL);
        //if (nPopCntMax != nPopCntMaxPrev)
        {
            printf("# EmbeddedListPopCntMax(%2d)  %2d\n", nBL, nPopCntMax);
            //nPopCntMaxPrev = nPopCntMax;
        }
    }
#endif // defined(EMBED_KEYS)
#endif // 0

#if 0 // ListWordsTypeList is static in b.c.
    // How big are T_LIST leaves.
    for (int nBL = cnBitsPerWord; nBL >= 8; nBL >>= 1) {
        printf("\n");
        int nWords = 0; int nWordsPrev = 0;
        //for (int nPopCnt = 1; nPopCnt <= cnBitsPerWord * 5 / nBL; nPopCnt++)
        for (int nPopCnt = 1; nWords < 7; nPopCnt++)
        {
            if ((nWords = ListWordsTypeList(nPopCnt, nBL)) != nWordsPrev) {
                if (nPopCnt > 2) {
                    printf("# ListWordsTypeList(nBL %2d, nPopCnt %3d) %d\n",
                           nBL, nPopCnt - 1,
                           ListWordsTypeList(nPopCnt - 1, nBL));
                }
                printf("# ListWordsTypeList(nBL %2d, nPopCnt %3d) %d\n",
                       nBL, nPopCnt, nWords);
                nWordsPrev = nWords;
            }
        }
    }
#endif // 0

    printf("\n");
    printf("# cnDummiesInList %d\n", cnDummiesInList);
    printf("# cnDummiesInSwitch %d\n", cnDummiesInSwitch);
    printf("# cnDummiesInLink %d\n", cnDummiesInLink);
#if defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords %d\n", cnMallocExtraWords);
#else // defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords n/a\n");
#endif // defined(cnMallocExtraWords)

    printf("\n# cnBitsIndexSzAtTop %d\n", cnBitsIndexSzAtTop);
    for (int dd = 1; dd <= cnDigitsPerWord; dd++) {
        printf("# nDL_to_nBitsIndexSz(%d) %d\n", dd, nDL_to_nBitsIndexSz(dd));
    }

    printf("\n");

    bInitialized= 1;
}

#endif // defined(DEBUG)

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

    if ((*pwRoot == (Word_t)0) && ! bInitialized ) {
        Initialize();
    }

  #endif // defined(DEBUG)

  #if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf before calling Insert.  Why?
    // To simplify Insert for PP_IN_LINK.  Does it still apply?
    // Do not assume the list is sorted.
    // But if we do the insert here, and the list is sorted, then leave it
    // sorted -- so we don't have to worry about ifdefs in this code.
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);

    if ((T_LIST == nType)
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
                wPopCnt = ls_wPopCnt(pwr);
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
                Word_t *pwKeysNew = ls_pwKeys(pwListNew);
                set_wr(wRoot, pwListNew, T_LIST);
                // pop count is in first element at top
                pwKeysNew += (cnDummiesInList == 0);
                Word_t *pwKeys;
      #if defined(USE_T_ONE)
                if (nType == T_ONE) {
                    pwKeys = pwr;
                } else
      #endif // defined(USE_T_ONE)
                {
                    pwKeys = ls_pwKeys(pwr) + (cnDummiesInList == 0);
                }

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
  #endif // (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    {
        status = Insert(pwRoot, wKey, cnDigitsPerWord);
    }

  #if defined(DEBUG)
    if (status == Success) {
        // count successful inserts minus successful removes
        wDebugPopCnt++;
        if (!bHitDebugThreshold && (wDebugPopCnt > cwDebugThreshold)) {
            bHitDebugThreshold = 1;
            if (cwDebugThreshold != 0) {
                printf("\nHit debug threshold.\n");
            }
        }
    }
  #endif // defined(DEBUG)

    DBGI(printf("\n# After Insert(wKey "OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGI(printf("\n"));

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wDebugPopCnt);
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
#if (cnDigitsPerWord > 1)

    int status;

    DBGR(printf("\n\n# Judy1Unset ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf.
    // Do not assume the list is sorted, but maintain the current order so
    // we don't have to bother with ifdefs in this code.
    Word_t *pwRoot = (Word_t *)ppvRoot;
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
            Word_t wPopCnt = ls_wPopCnt(pwr);
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
                    pwKeysNew = ls_pwKeys(pwListNew);
                    // pop count is in 1st element at top
                    pwKeysNew += (cnDummiesInList == 0);
                }

                Word_t *pwKeys = ls_pwKeys(pwr) + (cnDummiesInList == 0);

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
                set_wr(wRoot, NULL, T_NULL);
            }
            OldList(pwr, wPopCnt, cnDigitsPerWord, nType);
            *pwRoot = wRoot;
            status = Success;
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    {
        status = Remove((Word_t *)ppvRoot, wKey, cnDigitsPerWord);
    }

  #if defined(DEBUG)
    if (status == Success) { wDebugPopCnt--; }
  #endif // defined(DEBUG)

  #if defined(DEBUG_REMOVE)
    DBGR(printf("\n# After Remove(wKey "OWx") %s Dump\n", wKey,
            status == Success ? "Success" : "Failure"));
    DBGR(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGR(printf("\n"));
  #endif // defined(DEBUG_REMOVE)

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wDebugPopCnt);
  #endif // defined(DEBUG)

    return status;

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wRoot;
    Word_t wByteNum, wByteMask;
    char c;

    if ((wRoot = (Word_t)*ppvRoot) == 0)
    {
        return Failure; // not present
    }

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

