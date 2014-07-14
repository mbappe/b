
// @(#) $Id: bli.c,v 1.256 2014/07/14 15:58:46 mike Exp mike $
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
#if defined(SEARCH_METRICS)
Word_t j__SearchCompares;            // number times LGet/1Test called
Word_t j__SearchPopulation;          // Population of Searched object
Word_t j__TreeDepth;                 // number time Branch_U called
#endif // defined(SEARCH_METRICS)
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

#if defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 8)

// Return non-negative index, x, for key found at index x.
// Return negative (index + 1) for key not found, and index is where
// key should be.
// Lookup doesn't need to know where key should be.
// Only Insert and Remove benefit from that information.
static Status_t
SearchList8(uint8_t *pcKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
    uint8_t cKey = wKey;
    uint8_t cKeyLoop;
#if defined(OLD_SEARCH_8)
      #if defined(SORT_LISTS)
        #if defined(SIMPLE_SEARCH_8) // two tests per iteration
    cKeyLoop = *pcKeys;
    if (nPopCnt != 1)
    {
        uint8_t *psLastKey = &pcKeys[nPopCnt-1];
        while (cKeyLoop < cKey) {
            cKeyLoop = *++pcKeys;
            if (pcKeys == psLastKey) {
                break;
            }
        }
    }
        #else // defined(SIMPLE_SEARCH_8)
          #if defined(SPLIT_SEARCH_8) \
                  && (cnSplitSearchThresholdShort > 1)
              #if defined(SPLIT_SEARCH_LOOP_8)
    while
              #else // defined(SPLIT_SEARCH_LOOP_8)
    if
              #endif // defined(SPLIT_SEARCH_LOOP_8)
       (nPopCnt >= cnSplitSearchThresholdShort)
    {
// To do: Try to minimize the number of cache lines we hit.
// If ! PP_IN_LINK then we already hit the first one to get the pop count.
// Let's try aligning these lists.
        // pick a starting point
              #if ! defined(RATIO_SPLIT_8) \
                  || defined(SPLIT_SEARCH_LOOP_8)
        unsigned nSplit = nPopCnt / 2;
              #else // ! defined(RATIO_SPLIT_8) || ...
        unsigned nSplit
            = wKey % EXP(nBL) * nPopCnt / EXP(nBL);
              #endif // ! defined(RATIO_SPLIT_8) || ...
        if (pcKeys[nSplit] <= cKey) {
            pcKeys = &pcKeys[nSplit];
            nPopCnt -= nSplit;
// To do: Shouldn't we go backwards if we exit the loop after this step?
// It might be very important.
// What about cache line alignment?
        } else {
            nPopCnt = nSplit;
            goto loop;
        }
    }
          #endif // defined(SPLIT_SEARCH_8) && ...
    if ((cKeyLoop = pcKeys[nPopCnt - 1]) > cKey)
    {
          #if defined(SPLIT_SEARCH_8) \
                  && (cnSplitSearchThresholdShort > 1)
loop:
          #endif // defined(SPLIT_SEARCH_8) && ...
        while ((cKeyLoop = *pcKeys++) < cKey) { }
    }
        #endif // defined(SIMPLE_SEARCH_8)
      #else // defined(SORT_LISTS)
    uint8_t *pcKeysEnd = &pcKeys[nPopCnt];
    while (cKeyLoop = *pcKeys, pcKeys++ < pcKeysEnd)
      #endif // defined(SORT_LISTS)
    {
        if (cKeyLoop == cKey)
        {
            return Success;
        }
    }
#else // defined(OLD_SEARCH_8)
  #if defined(BACKWARD_SEARCH_8)
    uint8_t *pcKeysEnd = pcKeys;
    uint8_t *pcKeys = &pcKeysEnd[nPopCnt - 1];
      #if defined(END_CHECK_8)
    if (*pcKeysEnd > cKey) { return Failure; }
    for (;;)
      #else // defined(END_CHECK_8)
    do
      #endif // defined(END_CHECK_8)
    {
        cKeyLoop = *pcKeys--;
      #if defined(CONTINUE_FIRST)
        if (cKeyLoop > cKey) { continue; }
      #elif defined(FAIL_FIRST)
        if (cKeyLoop < cKey) { break; }
      #endif
        if (cKeyLoop == cKey) { return Success; }
      #if defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        if (cKeyLoop < cKey) { break; }
      #endif // defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
    }
      #if ! defined(END_CHECK_8)
    while (pcKeys >= pcKeysEnd);
      #endif // ! defined(END_CHECK_8)
  #else // defined(BACKWARD_SEARCH_8)
    uint8_t *pcKeysEnd = &pcKeys[nPopCnt - 1];
      #if defined(END_CHECK_8)
    if (*pcKeysEnd < cKey) { return Failure; }
    for (;;)
      #else // defined(END_CHECK_8)
    do
      #endif // defined(END_CHECK_8)
    {
        cKeyLoop = *pcKeys++;
      #if defined(CONTINUE_FIRST)
        if (cKeyLoop < cKey) { continue; }
      #elif defined(FAIL_FIRST)
        if (cKeyLoop > cKey) { break; }
      #endif
        if (cKeyLoop == cKey) { return Success; }
      #if defined(SUCCEED_FIRST)
        if (cKeyLoop > cKey)
      #endif // defined(SUCCEED_FIRST)
      #if defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        { break; }
      #endif // defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        // We're cheating here if we ignore the fact that the list is
        // sorted and look only for a match rather than checking to
        // see if we're passed the point where the key would be.
        // It's cheating because it's probably faster when the key
        // is present (which is the case for our performance testing)
        // and slower if the key is not present.
    }
      #if ! defined(END_CHECK_8)
    while (pcKeys <= pcKeysEnd);
      #endif // ! defined(END_CHECK_8)
  #endif // defined(BACKWARD_SEARCH_8)
#endif // defined(OLD_SEARCH_8)

    return Failure;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 8)

#if defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 16)

// Return non-negative index, x, for key found at index x.
// Return negative (index + 1) for key not found, and index is where
// key should be.
// Lookup doesn't need to know where key should be.
// Only Insert and Remove benefit from that information.
static Status_t
SearchList16(uint16_t *psKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    DBGL(printf("SearchList16\n"));
    (void)nBL;
    uint16_t sKey = wKey;
    uint16_t sKeyLoop;
#if defined(OLD_SEARCH_16)
      #if defined(SORT_LISTS)
        #if defined(SIMPLE_SEARCH_16) // two tests per iteration
    sKeyLoop = *psKeys;
    if (nPopCnt != 1)
    {
        uint16_t *psLastKey = &psKeys[nPopCnt-1];
        while (sKeyLoop < sKey) {
            sKeyLoop = *++psKeys;
            if (psKeys == psLastKey) {
                break;
            }
        }
    }
        #else // defined(SIMPLE_SEARCH_16)
          #if defined(SPLIT_SEARCH_16) \
                  && (cnSplitSearchThresholdShort > 1)
              #if defined(SPLIT_SEARCH_LOOP_16)
    while
              #else // defined(SPLIT_SEARCH_LOOP_16)
    if
              #endif // defined(SPLIT_SEARCH_LOOP_16)
       (nPopCnt >= cnSplitSearchThresholdShort)
    {
// To do: Try to minimize the number of cache lines we hit.
// If ! PP_IN_LINK then we already hit the first one to get the pop count.
// Let's try aligning these lists.
        // pick a starting point
              #if ! defined(RATIO_SPLIT_16) \
                  || defined(SPLIT_SEARCH_LOOP_16)
        unsigned nSplit = nPopCnt / 2;
              #else // ! defined(RATIO_SPLIT_16) || ...
        unsigned nSplit
            = wKey % EXP(nBL) * nPopCnt / EXP(nBL);
              #endif // ! defined(RATIO_SPLIT_16) || ...
        if (psKeys[nSplit] <= sKey) {
            psKeys = &psKeys[nSplit];
            nPopCnt -= nSplit;
// To do: Shouldn't we go backwards if we exit the loop after this step?
// It might be very important.
// What about cache line alignment?
        } else {
            nPopCnt = nSplit;
            goto loop;
        }
    }
          #endif // defined(SPLIT_SEARCH_16) && ...
    if ((sKeyLoop = psKeys[nPopCnt - 1]) > sKey)
    {
          #if defined(SPLIT_SEARCH_16) \
                  && (cnSplitSearchThresholdShort > 1)
loop:
          #endif // defined(SPLIT_SEARCH_16) && ...
        while ((sKeyLoop = *psKeys++) < sKey) { }
    }
        #endif // defined(SIMPLE_SEARCH_16)
      #else // defined(SORT_LISTS)
    uint16_t *psKeysEnd = &psKeys[nPopCnt];
    while (sKeyLoop = *psKeys, psKeys++ < psKeysEnd)
      #endif // defined(SORT_LISTS)
    {
        if (sKeyLoop == sKey)
        {
            return Success;
        }
    }
#else // defined(OLD_SEARCH_16)
  #if defined(BACKWARD_SEARCH_16)
    uint16_t *psKeysEnd = psKeys;
    uint16_t *psKeys = &psKeysEnd[nPopCnt - 1];
      #if defined(END_CHECK_16)
    if (*psKeysEnd > sKey) { return Failure; }
    for (;;)
      #else // defined(END_CHECK_16)
    do
      #endif // defined(END_CHECK_16)
    {
        sKeyLoop = *psKeys--;
      #if defined(CONTINUE_FIRST)
        if (sKeyLoop > sKey) { continue; }
      #elif defined(FAIL_FIRST)
        if (sKeyLoop < sKey) { break; }
      #endif
        if (sKeyLoop == sKey) { return Success; }
      #if defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        if (sKeyLoop < sKey) { break; }
      #endif // defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
    }
      #if ! defined(END_CHECK_16)
    while (psKeys >= psKeysEnd);
      #endif // ! defined(END_CHECK_16)
  #else // defined(BACKWARD_SEARCH_16)
    uint16_t *psKeysEnd = &psKeys[nPopCnt - 1];
DBGL(printf("psKeysEnd %p\n", (void *)psKeysEnd));
      #if defined(END_CHECK_16)
    if (*psKeysEnd < sKey) { return Failure; }
DBGL(printf("got past end check\n"));
    for (;;)
      #else // defined(END_CHECK_16)
    do
      #endif // defined(END_CHECK_16)
    {
        sKeyLoop = *psKeys++;
      #if defined(CONTINUE_FIRST)
        if (sKeyLoop < sKey) { continue; }
      #elif defined(FAIL_FIRST)
        if (sKeyLoop > sKey) { break; }
      #endif
        if (sKeyLoop == sKey) { return Success; }
      #if defined(SUCCEED_FIRST)
        if (sKeyLoop > sKey)
      #endif // defined(SUCCEED_FIRST)
      #if defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        { break; }
      #endif // defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        // We're cheating here if we ignore the fact that the list is
        // sorted and look only for a match rather than checking to
        // see if we're passed the point where the key would be.
        // It's cheating because it's probably faster when the key
        // is present (which is the case for our performance testing)
        // and slower if the key is not present.
    }
      #if ! defined(END_CHECK_16)
    while (psKeys <= psKeysEnd);
      #endif // ! defined(END_CHECK_16)
  #endif // defined(BACKWARD_SEARCH_16)
#endif // defined(OLD_SEARCH_16)

    return Failure;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsAtBottom <= 16)

#if defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) \
    && (cnBitsAtBottom <= 32)

// Return non-negative index, x, for key found at index x.
// Return negative (index + 1) for key not found, and index is where
// key should be.
// Lookup doesn't need to know where key should be.
// Only Insert and Remove benefit from that information.
static Status_t
SearchList32(uint32_t *piKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
    uint32_t iKey = wKey;
    uint32_t iKeyLoop;
          #if defined(SPLIT_SEARCH_32)
              #if defined(SPLIT_SEARCH_LOOP_32)
    while
              #else // defined(SPLIT_SEARCH_LOOP_32)
    if
              #endif // defined(SPLIT_SEARCH_LOOP_32)
       (nPopCnt >= cnSplitSearchThresholdInt)
    {
        if (piKeys[nPopCnt / 2] <= iKey) {
            piKeys = &piKeys[nPopCnt / 2];
            nPopCnt -= nPopCnt / 2;
        } else {
            nPopCnt /= 2;
        }
    }
          #endif // defined(SPLIT_SEARCH_32)
          #if defined(END_CHECK_32)
    if ((piKeys[nPopCnt - 1]) < iKey) { return Failure; }
    while ((iKeyLoop = *piKeys++) < iKey) { }
          #else // defined(END_CHECK_32)
    uint32_t *piKeysEnd = &piKeys[nPopCnt - 1];
    while (iKeyLoop = *piKeys, piKeys++ <= piKeysEnd)
          #endif // defined(END_CHECK_32)
    {
        if (iKeyLoop == iKey) { return Success; }
    }

    return Failure;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) && ...

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

// Find wKey in the list.  If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
static int
SearchListWord(Word_t *pwKeys, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
    Word_t *pwKeysOrig = pwKeys;
  // SPLIT_SEARCH narrows the scope of the linear search that follows, if any.
  #if defined(SPLIT_SEARCH)
    unsigned nSplit;
      #if defined(RATIO_SPLIT)
    if (nPopCnt >= cnSplitSearchThresholdWord) {
          #if (cnBitsPerWord == 64)
        if (nBL >= cnBitsPerWord) {
            nSplit = wKey / cnListPopCntMax64 * nPopCnt
                   / ((Word_t)-1 / cnListPopCntMax64);
        } else {
            nSplit = wKey % EXP(nBL) / cnListPopCntMax64 * nPopCnt
                   / (EXP(nBL) / cnListPopCntMax64);
        }
          #else // (cnBitsPerWord == 64)
        if (nBL >= cnBitsPerWord) {
            nSplit = wKey / cnListPopCntMax32 * nPopCnt
                   / ((Word_t)-1 / cnListPopCntMax32);
        } else {
            nSplit = wKey % EXP(nBL) / cnListPopCntMax32 * nPopCnt
                   / (EXP(nBL) / cnListPopCntMax32);
        }
          #endif // (cnBitsPerWord == 64)
        goto split;
    }
      #endif // defined(RATIO_SPLIT)
      #if defined(SPLIT_SEARCH_LOOP)
    while
      #else // defined(SPLIT_SEARCH_LOOP)
    if
      #endif // defined(SPLIT_SEARCH_LOOP)
        // Looks like we might want a loop threshold of 8 for
        // 64-bit keys at the top level.
        // And there's not much difference with threshold of
        // 16 or 32.
        // Not sure about 64-bit keys at a lower level or
        // 32-bit keys at the top level.
        (nPopCnt >= cnSplitSearchThresholdWord)
    {
        nSplit = nPopCnt / 2;
      #if defined(RATIO_SPLIT)
split: // should go backwards if key is in first part
      #endif // defined(RATIO_SPLIT)
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
  #endif // defined(SPLIT_SEARCH)
    Word_t *pwKeysEnd = &pwKeys[nPopCnt-1]; (void)pwKeysEnd;
    Word_t wKeyLoop;
  #if defined(SPLIT_SEARCH_LOOP) && (cnSplitSearchThresholdWord <= 2)
    assert(nPopCnt == 1);
    wKeyLoop = *pwKeys;
  #else // defined(SPLIT_SEARCH_LOOP) && (cnSplitSearchThresholdWord <= 2)
      // Should we add a threshold for END_CHECK?
      #if defined(END_CHECK)
          #if defined(BACKWARD_SEARCH)
    if (*pwKeys > wKey) {
        assert(~(pwKeys - pwKeysOrig) < 0);
        return ~(pwKeys - pwKeysOrig);
    }
    while ((wKeyLoop = *pwKeysEnd--) > wKey) { }
          #else // defined(BACKWARD_SEARCH)
    if (*pwKeysEnd < wKey) {
        assert(~(pwKeysEnd + 1 - pwKeysOrig) < 0);
        return ~(pwKeysEnd + 1 - pwKeysOrig);
    }
    while ((wKeyLoop = *pwKeys++) < wKey) { }
          #endif // defined(BACKWARD_SEARCH)
      #else // defined(END_CHECK)
    do
      #endif // defined(END_CHECK)
  #endif // defined(SPLIT_SEARCH_LOOP) && (cnSplitSearchThresholdWord <= 2)
    {
  #if ( ! defined(SPLIT_SEARCH_LOOP) || (cnSplitSearchThresholdWord > 2) ) \
          && ! defined(END_CHECK)
      #if defined(BACKWARD_SEARCH)
        wKeyLoop = *pwKeysEnd;
      #else // defined(BACKWARD_SEARCH)
        wKeyLoop = *pwKeys;
      #endif // defined(BACKWARD_SEARCH)
      // Should we add a threshold for <XYZ>_FIRST?
      #if ! defined(BACKWARD_SEARCH) && defined(CONTINUE_FIRST)
        if (wKeyLoop < wKey) { continue; }
      #endif // ! defined(BACKWARD_SEARCH) && defined(CONTINUE_FIRST)
      #if defined(BACKWARD_SEARCH) && defined(CONTINUE_FIRST)
        if (wKeyLoop > wKey) { continue; }
      #endif // defined(BACKWARD_SEARCH) && defined(CONTINUE_FIRST)

      #if defined(FAIL_FIRST)
          #if defined(BACKWARD_SEARCH)
        if (wKeyLoop < wKey) { break; }
          #else // defined(BACKWARD_SEARCH)
        if (wKeyLoop > wKey) { break; }
          #endif // defined(BACKWARD_SEARCH)
      #endif // defined(FAIL_FIRST)
  #endif // ( ! defined(SPLIT_SEARCH_LOOP) || ... ) && ! defined(END_CHECK)

#if defined(BACKWARD_SEARCH)
        if (wKeyLoop == wKey) {
            assert(pwKeysEnd - pwKeysOrig >= 0);
            return pwKeysEnd - pwKeysOrig;
        }
#else // defined(BACKWARD_SEARCH)
        if (wKeyLoop == wKey) {
            assert(pwKeys - pwKeysOrig >= 0);
            return pwKeys - pwKeysOrig;
        }
#endif // defined(BACKWARD_SEARCH)

  #if ( ! defined(SPLIT_SEARCH_LOOP) || (cnSplitSearchThresholdWord > 2) ) \
          && ! defined(END_CHECK)
      #if defined(SUCCEED_FIRST)
          #if defined(BACKWARD_SEARCH)
        if (wKeyLoop < wKey)
          #else // defined(BACKWARD_SEARCH)
        if (wKeyLoop > wKey)
          #endif // defined(BACKWARD_SEARCH)
      #endif // defined(SUCCEED_FIRST)
      #if defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
        { break; }
      #endif // defined(SUCCEED_FIRST) || defined(CONTINUE_FIRST)
  #endif // ( ! defined(SPLIT_SEARCH_LOOP) || ... ) && ! defined(END_CHECK)
    }
  #if ( ! defined(SPLIT_SEARCH_LOOP) || (cnSplitSearchThresholdWord > 2) ) \
          && ! defined(END_CHECK)
      #if defined(BACKWARD_SEARCH)
    while (--pwKeysEnd >= pwKeys);
      #else // defined(BACKWARD_SEARCH)
    while (++pwKeys <= pwKeysEnd);
      #endif // defined(BACKWARD_SEARCH)
  #endif // ( ! defined(SPLIT_SEARCH_LOOP) || ... ) && ! defined(END_CHECK)

#if defined(BACKWARD_SEARCH)
    assert(~(pwKeysEnd + 1 - pwKeysOrig) < 0);
    return ~(pwKeysEnd + 1 - pwKeysOrig);
#else // defined(BACKWARD_SEARCH)
    assert(~(pwKeys - pwKeysOrig) < 0);
    return ~(pwKeys - pwKeysOrig);
#endif // defined(BACKWARD_SEARCH)
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

// Return non-negative index, x, for key found at index x.
// Return negative (index + 1) for key not found, and index is where
// key should be.
// Lookup doesn't need to know where key should be.
// Only Insert and Remove benefit from that information.
static Status_t
SearchList(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    DBGL(printf("SearchList\n"));
  #if defined(COMPRESSED_LISTS)
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
        return (SearchListWord(pwr_pwKeys(pwr), wKey, nBL, nPopCnt) >= 0)
                   ? Success : Failure;
    }
}

#endif // (cwListPopCntMax != 0)

#if defined(LOOKUP)
static Status_t
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Status_t
InsertRemove(Word_t *pwRoot, Word_t wKey, unsigned nDL)
#endif // defined(LOOKUP)
{
    unsigned nDLUp; (void)nDLUp; // silence gcc
#if defined(LOOKUP)
    unsigned nDL = cnDigitsPerWord;
  #if defined(SKIP_LINKS)
      #if defined(SKIP_PREFIX_CHECK) && ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    unsigned bNeedPrefixCheck = 0;
      #endif // defined(SKIP_PREFIX_CHECK) && ! ALWAYS_CHECK_PREFIX_AT_LEAF
  #endif // defined(SKIP_LINKS)
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

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && defined(SAVE_PREFIX)
  #if defined(PP_IN_LINK)
    Word_t *pwRootPrefix = NULL;
  #else // defined(PP_IN_LINK)
    Word_t *pwrPrefix = NULL;
  #endif // defined(PP_IN_LINK)
    Word_t nDLRPrefix = 0;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && defined(SAVE_PREFIX)

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if !defined(LOOKUP)
  #if !defined(RECURSIVE)
top:
  #endif // !defined(RECURSIVE)
    wRoot = *pwRoot;
#endif // !defined(LOOKUP)
#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
    nDLR = nDL;
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)

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
    switch (nType)
    {
    default: // skip link (if -DSKIP_LINKS && -DTYPE_IS_RELATIVE)
    {
        // pwr points to a switch

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

        DBGX(printf("Switch nDLR %d pwr %p\n", nDLR, (void *)pwr));

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
        pwRootPrefix = pwRoot;
          #else // defined(PP_IN_LINK)
        pwrPrefix = pwr;
          #endif // defined(PP_IN_LINK)
        nDLRPrefix = nDLR;
      #endif // defined(SAVE_PREFIX)
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        // Record that there were prefix bits that were not checked.
          #if defined(TYPE_IS_RELATIVE)
        bNeedPrefixCheck |= 1;
          #else // defined(TYPE_IS_RELATIVE)
        bNeedPrefixCheck |= (nDLR < nDL);
          #endif // defined(TYPE_IS_RELATIVE)
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        bPrefixMismatch = (1
      #if ! defined(TYPE_IS_RELATIVE)
            && (nDLR < nDL)
      #endif // ! defined(TYPE_IS_RELATIVE)
            && (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwr, nDLR) ^ wKey))
                    >= nDL_to_nBL_NAT(nDLR)));
      #if ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
        if (bPrefixMismatch)
        {
            DBGX(printf("Mismatch wPrefix "Owx"\n",
                        PWR_wPrefixNAT(pwRoot, pwr, nDLR)));
            break;
        }
      #endif // ! defined(LOOKUP) || ! defined(SAVE_PREFIX_TEST_RESULT)
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        // fall into next case
    }
    case T_SW_BASE: // no skip switch
    {
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(BM_SWITCH_FOR_REAL) \
    || ( ! defined(LOOKUP) \
        && (defined(PP_IN_LINK) || defined(BM_IN_LINK)) \
            || (defined(REMOVE) && ! defined(BM_IN_LINK)) )
        nDLUp = nDL;
#endif // defined(BM_SWITCH_FOR_REAL) ...
        nDL = nDLR - 1;

        Word_t wIndex = ((wKey >> nDL_to_nBL_NAT(nDL))
            // we can use NAT here even though we might be at top because
            // we're using it to mask off high bits and if we're at the
            // top then none of the high bits will be set anyway;
            // it's faster to do arithmetic than test to avoid it
            & (EXP(nDL_to_nBitsIndexSzNAX(nDLR)) - 1));

#if defined(BM_SWITCH)
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
#endif // defined(BM_SWITCH)

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

          #if defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t xx = 0;
          #endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
                for (Word_t ww = 0; ww < EXP(cnBitsIndexSzAtTop); ww++)
                {
          #if defined(BM_SWITCH) && !defined(BM_IN_LINK)
                    Word_t *pwRootLn = &pwr_pLinks(pwr)[xx].ln_wRoot;
                    xx++;
          #else // defined(BM_SWITCH) && !defined(BM_IN_LINK)
                    Word_t *pwRootLn = &pwr_pLinks(pwr)[ww].ln_wRoot;
          #endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
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
            wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDLR);
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
                set_PWR_wPopCnt(pwRoot, pwr,
                                nDLR, wPopCnt + nIncr);
                DBGX(printf("wPopCnt %zd\n",
                     (size_t)PWR_wPopCnt(pwRoot, pwr, nDLR)));
            }
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks(pwr)[wIndex].ln_wRoot;
        wRoot = *pwRoot;

        DBGX(printf("Next pLinks %p wIndex %d\n",
            (void *)pwr_pLinks(pwr), (int)wIndex));

        DBGX(printf("pwRoot %p wRoot "OWx"\n", (void *)pwRoot, wRoot));

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just
        // visited in the next iteration of the loop
        // #if defined(COMPRESSED_LISTS)
        // so we preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#if defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        nDLR = nDL;
#endif // defined(SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nDL);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of default case

#if (cwListPopCntMax != 0)

    case T_LIST:
    {
        DBGX(printf("List nDL %d\n", nDL));
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMask(nDL)));

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
            || (LOG(1 | (PWR_wPrefixNAT(pwRootPrefix, pwrPrefix, nDLRPrefix)
                    ^ wKey))
                < nDL_to_nBL(nDLRPrefix))
              #else // defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwrPrev, nDL)
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
                == Success)
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

        break;

    } // end of case T_LIST

#endif // (cwListPopCntMax != 0)

    case T_BITMAP:
    {
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
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwrPrev, nDL) ^ wKey))
                // The +1 is necessary because the pwrPrev
                // prefix does not contain any less significant bits.
              #if defined(BITMAP_ANYWHERE)
                  #if defined(PP_IN_LINK)
                < nDL_to_nBL_NAT(nDL    )
                  #else // defined(PP_IN_LINK)
                < nDL_to_nBL_NAT(nDL + 1)
                  #endif // defined(PP_IN_LINK)
              #else // defined(BITMAP_ANYWHERE)
                  #if defined(PP_IN_LINK)
                < (cnBitsAtBottom                 )
                  #else // defined(PP_IN_LINK)
                < (cnBitsAtBottom + cnBitsPerDigit)
                  #endif // defined(PP_IN_LINK)
              #endif // defined(BITMAP_ANYWHERE)
                                  )
          #endif // defined(SAVE_PREFIX)
            )
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
        {
  #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            assert(PWR_wPopCnt(pwRoot, pwrPrev,
              #if defined(PP_IN_LINK)
                nDL_to_nBL_NAT(nDL    )
              #else // defined(PP_IN_LINK)
                nDL_to_nBL_NAT(nDL + 1)
              #endif // defined(PP_IN_LINK)
                               ) != 0);
            return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
      #if defined(BITMAP_ANYWHERE)
            if (BitIsSet(wr_pwr(wRoot),
                    wKey & (EXP(nDL_to_nBL_NAT(nDL)) - 1UL)))
      #else // defined(BITMAP_ANYWHERE)
            if (BitIsSet(wr_pwr(wRoot), wKey & (EXP(cnBitsAtBottom) - 1UL)))
      #endif // defined(BITMAP_ANYWHERE)
            {
      #if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDL, wRoot);
                goto cleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT) && !defined(RECURSIVE)
                if (nIncr > 0)
                {
                    DBGX(printf(
                      "BitmapWordNum %"_fw"d BitmapWordMask "OWx"\n",
                       BitmapWordNum(wKey), BitmapWordMask(wKey)));
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

#if defined(T_ONE)

    // T_ONE is a one-key/word external leaf or an embedded/internal list.
    // The latter is only possible if EMBED_KEYS is defined.  In the latter
    // case an embedded list is assumed for one key if the key will fit.
    case T_ONE:
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
        if (nBL <= cnBitsPerWord - cnBitsMallocMask) {
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

#endif // defined(T_ONE)

    case T_NULL:
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

static int bInitializedForDebug;

static void
InitializeForDebug(void)
{
    // cnBitsAtBottom less than or equal to cnLogBitsPerWord makes
    // no sense anymore.  It's equivalent to cnBitsAtBottom equals
    // cnLogBitsPerWord plus cnBitsPerDigit -- only worse.
    assert(cnBitsAtBottom > cnLogBitsPerWord);

    bInitializedForDebug = 1;
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

    if ((*pwRoot == (Word_t)0) && ! bInitializedForDebug ) {
        InitializeForDebug();
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
      #if defined(T_ONE)
        || (nType == T_ONE)
      #else // defined(T_ONE)
        || (nType == T_NULL)
      #endif // defined(T_ONE)
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

      #if defined(T_ONE)
            if (nType == T_ONE) {
                wPopCnt = 1;
            } else
      #else // defined(T_ONE)
            if (nType == T_NULL) {
                assert(pwr == NULL);
                wPopCnt = 0;
            } else
      #endif // defined(T_ONE)
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
      #if defined(T_ONE)
                if (nType == T_ONE) {
                    pwKeys = pwr;
                } else
      #endif // defined(T_ONE)
                {
                    pwKeys = ls_pwKeys(pwr) + (cnDummiesInList == 0);
                }

 // Isn't this chunk of code already in InsertGuts?
                unsigned nn;
                for (nn = 0; (nn < wPopCnt) && (pwKeys[nn] < wKey); nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                pwKeysNew[nn] = wKey;
                COPY(&pwKeysNew[nn + 1], &pwKeys[nn], wPopCnt - nn);
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
#if defined(T_ONE)
                if (wPopCnt == 2) {
                    set_wr(wRoot, pwListNew, T_ONE);
                    pwKeysNew = pwListNew;
                } else
#endif // defined(T_ONE)
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

