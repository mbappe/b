
// @(#) $Id: bli.c,v 1.196 2014/06/10 14:28:09 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/bli.c,v $

// This file is #included in other .c files three times.
// Once with #define LOOKUP, #undef INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #define INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #undef INSERT and #define REMOVE.
// The best way to read the code is to pick a set of defines
// and ignore the code that gets thrown away.  Or maybe even use unifdef.

// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#if (cnDigitsPerWord <= 1)
#if (cnBitsPerDigit >= cnBitsPerWord)
#if (cnBitsAtBottom >= cnBitsPerWord)

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

Status_t
#if defined(LOOKUP)
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
InsertRemove(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft)
#endif // defined(LOOKUP)
{
    unsigned nDigitsLeftUp; (void)nDigitsLeftUp; // silence gcc
#if defined(LOOKUP)
    unsigned nDigitsLeft = cnDigitsPerWord;
  #if ! defined(NO_SKIP_LINKS)
      #if defined(SKIP_PREFIX_CHECK) && ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    unsigned bNeedPrefixCheck = 0;
      #endif // defined(SKIP_PREFIX_CHECK) && ! ALWAYS_CHECK_PREFIX_AT_LEAF
  #endif // ! defined(NO_SKIP_LINKS)
    Word_t *pwRoot;
  #if defined(BM_IN_LINK)
    pwRoot = NULL; // used for top detection
  #else // defined(BM_IN_LINK)
          #if defined(PP_IN_LINK)
    // Silence unwarranted gcc used before initialized warning.
    // pwRoot is only uninitialized on the first time through the loop.
    // And we only use it if nBitsLeft != cnBitsPerWord
    // or if bNeedPrefixCheck is true.
    // And both of those imply it's not the first time through the loop.
    pwRoot = NULL;
          #endif // defined(PP_IN_LINK)
  #endif // defined(BM_IN_LINK)
#else // defined(LOOKUP)
    Word_t wRoot;
  #if !defined(RECURSIVE)
    unsigned nDigitsLeftOrig = nDigitsLeft;
          #if defined(INSERT)
    int nIncr = 1;
          #else // defined(INSERT)
    int nIncr = -1;
          #endif // defined(INSERT)
  #endif // !defined(RECURSIVE)
  #if defined(PP_IN_LINK) || defined(BM_IN_LINK)
    nDigitsLeftUp = nDigitsLeft;
  #endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)
#endif // defined(LOOKUP)
#if !defined(RECURSIVE)
  #if !defined(LOOKUP) || defined(BM_IN_LINK)
    Word_t *pwRootOrig = pwRoot;
  #endif // !defined(LOOKUP) || defined(BM_IN_LINK)
#endif // !defined(RECURSIVE)
    unsigned nDigitsLeftRoot;
    Word_t wPopCnt; (void)wPopCnt;
    unsigned nType;
#if defined(REMOVE)
    int bCleanup = 0;
#endif // defined(REMOVE)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwr = pwr; // suppress "uninitialized" compiler warning
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrPrev = pwrPrev; // suppress "uninitialized" compiler warning
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)

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
#if ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
    nDigitsLeftRoot = nDigitsLeft;
#endif // ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)

#if defined(LOOKUP) || !defined(RECURSIVE)
again:
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
    assert(nDigitsLeftRoot == nDigitsLeft);
#endif // ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if ( ! defined(LOOKUP) )
    assert(nDigitsLeft >= nBL_to_nDL(cnBitsAtBottom)); // valid for LOOKUP too
    DBGX(printf("# pwRoot %p ", (void *)pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    nType = wr_nType(wRoot);
    pwr = wr_tp_pwr(wRoot, nType);
    switch (nType)
    {
    default: // skip link (if -UNO_SKIP_LINKS && -DTYPE_IS_RELATIVE)
    {
        // pwr points to a switch

#if defined(NO_SKIP_LINKS)
        nDigitsLeftRoot = nDigitsLeft; // prev
#else // defined(NO_SKIP_LINKS)
  #if defined(TYPE_IS_RELATIVE)
        nDigitsLeftRoot = nDigitsLeft - tp_to_nDS(nType);
  #else // defined(TYPE_IS_RELATIVE)
        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);
  #endif // defined(TYPE_IS_RELATIVE)
        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved
#endif // defined(NO_SKIP_LINKS)

        DBGX(printf("Switch nDLR %d pwr %p\n", nDigitsLeftRoot, (void *)pwr));

#if ! defined(NO_SKIP_LINKS)
  #if defined(TYPE_IS_RELATIVE)
        assert(nDigitsLeftRoot < nDigitsLeft);
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
        nDLRPrefix = nDigitsLeftRoot;
      #endif // defined(SAVE_PREFIX)
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        // Record that there were prefix bits that were not checked.
          #if defined(TYPE_IS_RELATIVE)
        bNeedPrefixCheck |= 1;
          #else // defined(TYPE_IS_RELATIVE)
        bNeedPrefixCheck |= (nDigitsLeftRoot < nDigitsLeft);
          #endif // defined(TYPE_IS_RELATIVE)
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        if (1
      #if ! defined(TYPE_IS_RELATIVE)
            && (nDigitsLeftRoot < nDigitsLeft)
      #endif // ! defined(TYPE_IS_RELATIVE)
            && (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwr, nDigitsLeftRoot) ^ wKey))
                    >= nDL_to_nBL_NAT(nDigitsLeftRoot)))
        {
            DBGX(printf("Mismatch wPrefix "Owx"\n",
                        PWR_wPrefixNAT(pwRoot, pwr, nDigitsLeftRoot)));
            break;
        }
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // ! defined(NO_SKIP_LINKS)
#if ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        // fall into next case
    }
    case T_NO_SKIP_SWITCH:
    {
#endif // ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(BM_SWITCH_FOR_REAL) \
    || ( ! defined(LOOKUP) \
        && (defined(PP_IN_LINK) || defined(BM_IN_LINK)) \
            || (defined(REMOVE) && ! defined(BM_IN_LINK)) )
        nDigitsLeftUp = nDigitsLeft;
#endif // defined(BM_SWITCH_FOR_REAL) ...
        nDigitsLeft = nDigitsLeftRoot - 1;

        Word_t wIndex = ((wKey >> nDL_to_nBL_NAT(nDigitsLeft))
            // we can use NAT here even though we might be at top because
            // we're using it to mask off high bits and if we're at the
            // top then none of the high bits will be set anyway;
            // it's faster to do arithmetic than test to avoid it
            & (EXP(nDL_to_nBitsIndexSzNAX(nDigitsLeftRoot)) - 1));

#if defined(BM_SWITCH)
  #if defined(BM_IN_LINK)
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nDigitsLeft == cnDigitsPerWord and pwRoot not at the top.
        // We need to know if there is a link surrounding *pwRoot.
        // InsertGuts always calls back into Insert with the same pwRoot
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nDigitsLeft == cnDigitsPerWord and pwRoot not at the top.
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nDigitsLeft == cnDigitsPerWord)
      #else // defined(RECURSIVE)
                && (pwRoot == pwRootOrig)
          #if !defined(LOOKUP)
                && (nDigitsLeftOrig == cnDigitsPerWord)
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
                nDigitsLeft = nDigitsLeftUp; // back up for InsertGuts
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
// What if nDigitsLeft was cnDigitsPerWord before it was updated?
// Don't we have to walk the switch in that case too?
        if (nDigitsLeftUp == cnDigitsPerWord)
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
// We must use nDigitsLeft in that case.
                    // Do we really need a new variable here?
                    // Or can we just use nDigitsLeft?
                    int nDigitsLeftX = wr_bIsSwitch(*pwRootLn) ?
              #if defined(TYPE_IS_RELATIVE)
                                       nDigitsLeft - wr_nDS(*pwRootLn)
              #else // defined(TYPE_IS_RELATIVE)
                                       wr_nDigitsLeft(*pwRootLn)
              #endif // defined(TYPE_IS_RELATIVE)
                                   : nDigitsLeft;
                    DBGX(printf("wr_nDLX %d", nDigitsLeftX));
                    DBGX(printf(" PWR_wPopCnt %"_fw"d\n",
                                PWR_wPopCnt(pwRootLn, NULL, nDigitsLeftX)
                                ));
                    if (((*pwRootLn != 0) && (ww != wIndex))
                            || (
                                PWR_wPopCnt(pwRootLn, NULL, nDigitsLeftX)
                                    != 0)
                        )
                    {
                        DBGX(printf("Not empty ww %zd wIndex %zd\n",
                             (size_t)ww, (size_t)wIndex));
                        goto notEmpty; // switch pop is not zero
                    }
                }
                // switch pop is zero
                FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDigitsLeftUp),
                    /* bDump */ 0);
          #if defined(PP_IN_LINK)
                assert(PWR_wPrefix(pwRoot, NULL, nDigitsLeftUp) == 0);
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
            wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDigitsLeftRoot);
  #if defined(REMOVE)
            if (bCleanup)
            {
                if (wPopCnt == 0)
                {
                    FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDigitsLeftUp),
                        /* bDump */ 0);
      #if defined(PP_IN_LINK)
          #if defined(BM_IN_LINK)
                assert(PWR_wPrefix(pwRoot, NULL, nDigitsLeftUp) == 0);
          #else // defined(BM_IN_LINK)
                if (PWR_wPrefix(pwRoot, NULL, nDigitsLeftRoot) != 0)
                {
                    DBGR(printf("wPrefixPop "OWx"\n",
                                PWR_wPrefixPop(pwRoot, NULL)));
                }
                assert(PWR_wPrefix(pwRoot, NULL, nDigitsLeftRoot) == 0);
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
                                nDigitsLeftRoot, wPopCnt + nIncr);
                DBGX(printf("wPopCnt %zd\n",
                     (size_t)PWR_wPopCnt(pwRoot, pwr, nDigitsLeftRoot)));
            }
        }
#endif // !defined(LOOKUP)

        pwRoot = &pwr_pLinks(pwr)[wIndex].ln_wRoot;
        wRoot = *pwRoot;

        DBGX(printf("Next nDigitsLeft %d wIndex %zd pwr %p pLinks %p\n",
            nDigitsLeft, (size_t)wIndex,
            (void *)pwr, (void *)pwr_pLinks(pwr)));

        DBGX(printf("pwRoot %p wRoot "OWx"\n", (void *)pwRoot, wRoot));

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just
        // visited in the next iteration of the loop
        // #if defined(COMPRESSED_LISTS)
        // so we preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#if ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
        nDigitsLeftRoot = nDigitsLeft;
#endif // ! defined(NO_SKIP_LINKS) && defined(TYPE_IS_RELATIVE)
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nDigitsLeft);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of default case

    case T_LIST:
    {
        DBGX(printf("List nDigitsLeft %d\n", nDigitsLeft));
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMask(nDigitsLeft)));

  #if defined(REMOVE)
        if (bCleanup) { return Success; }
  #endif // defined(REMOVE)

  #if defined(PP_IN_LINK)
        // What about defined(RECURSIVE)?
        assert(nDigitsLeft != cnDigitsPerWord); // handled in wrapper
        // If nDigitsLeft != cnDigitsPerWord then we're not at the top.
        // And pwRoot is initialized despite what gcc might think.
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft);
      #if ! defined(LOOKUP)
        DBGX(printf("wPopCnt (before incr) %zd\n", (size_t)wPopCnt));
        set_PWR_wPopCnt(pwRoot, NULL, nDigitsLeft, wPopCnt + nIncr);
        DBGX(printf("wPopCnt (after incr) %zd\n",
                    (size_t)PWR_wPopCnt(pwRoot, NULL, nDigitsLeft)));
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
        wPopCnt = ls_wPopCnt(wr_tp_pwr(wRoot, nType));
      #endif // defined(PP_IN_LINK)

      #if defined(LOOKUP)
        SMETRICS(j__SearchPopulation += wPopCnt);
      #endif // defined(LOOKUP)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COMPRESSED_LISTS)
          #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
        // nDigitsLeft is relative to the bottom of the switch
        // containing the pointer to the leaf.
        // Can we use NAT here since bNeedPrefixCheck will never
        // be true if we are at the top?
        // If the top digit is smaller than the rest, then NAT will
        // return nBitsLeft > cnBitsPerWord which works out perfectly.
        unsigned nBitsLeft = nDL_to_nBL_NAT(nDigitsLeft);
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
            || (nBitsLeft > 32) // leaf has whole key
              #else // (cnBitsPerWord > 32)
            || (nBitsLeft > 16) // leaf has whole key
              #endif // (cnBitsPerWord > 32)
          // can't skip nBitsLeft check above or we might be at top
          // hmm; check nBitsLeft or check at top; which is better?
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
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwrPrev, nDigitsLeft)
                    ^ wKey))
                < (nBitsLeft
                  #if ! defined(PP_IN_LINK)
                    // prefix in parent switch doesn't contain last digit
                    // for ! defined(PP_IN_LINK) case
                    + nDL_to_nBitsIndexSzNAT(nDigitsLeft + 1)
                  #endif // ! defined(PP_IN_LINK)
                ))
              #endif // defined(SAVE_PREFIX)
            )
          #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #endif // defined(COMPRESSED_LISTS)
        {
      #if defined(DL_IN_LL)
            assert(ll_nDigitsLeft(wRoot) == nDigitsLeft);
      #endif // defined(DL_IN_LL)
      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
          // This short-circuit is for analysis only.  We have retrieved the
          // pop count and prefix but we have not dereferenced the list
          // itself.
          #if defined(PP_IN_LINK)
            return KeyFound;
          #else // defined(PP_IN_LINK)
            return wPopCnt ? KeyFound : ! KeyFound;
          #endif // defined(PP_IN_LINK)
      #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
          #if defined(COMPRESSED_LISTS)
            switch ( LOG(nBitsLeft - 1) - 2)
            {
              #if (cnBitsAtBottom < 8)
            case 0:
            {
                unsigned char cKeyLoop;
                unsigned char cKey = wKey;
                unsigned char *pcKeys = pwr_pcKeys(pwr);
                  #if defined(SORT_LISTS)
                if ((cKeyLoop = pcKeys[wPopCnt - 1]) > cKey)
                {
                    while ((cKeyLoop = *pcKeys++) < cKey);
                }
                  #else // defined(SORT_LISTS)
                unsigned char *pcKeysEnd = &pcKeys[wPopCnt];
                while (cKeyLoop = *pcKeys, pcKeys++ < pcKeysEnd)
                  #endif // defined(SORT_LISTS)
                {
                    if (cKeyLoop == cKey)
                    {
                  #if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                        goto cleanup;
                  #endif // defined(REMOVE)
                  #if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) { goto undo; } // undo counting
                  #endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
                }
                break;
            }
              #endif // (cnBitsAtBottom < 8)
              #if (cnBitsAtBottom < 16)
            case 1:
            {
                unsigned short sKeyLoop;
                unsigned short sKey = wKey;
                unsigned short *psKeys = pwr_psKeys(pwr);
                  #if defined(SORT_LISTS)
                if ((sKeyLoop = psKeys[wPopCnt - 1]) > sKey)
                {
                    while ((sKeyLoop = *psKeys++) < sKey);
                }
                  #else // defined(SORT_LISTS)
                unsigned short *psKeysEnd = &psKeys[wPopCnt];
                while (sKeyLoop = *psKeys, psKeys++ < psKeysEnd)
                  #endif // defined(SORT_LISTS)
                {
                    if (sKeyLoop == sKey)
                    {
                  #if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                        goto cleanup;
                  #endif // defined(REMOVE)
                  #if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) { goto undo; } // undo counting
                  #endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
                }
                break;
            }
              #endif // (cnBitsAtBottom < 16)
              #if (cnBitsAtBottom < 32) && (cnBitsPerWord > 32)
            case 2:
            {
                // Looks like we might want a linear search of 32-bit
                // keys up to a population nearing 128.
                // And a single split is good enough up to 256.
                // But the threshold doesn't make a sigificant
                // difference.  And loop vs single split doesn't
                // make much difference.  And no splits is ok for
                // pops less than 128.
                unsigned int iKeyLoop;
                unsigned int iKey = wKey;
                unsigned int *piKeys = pwr_piKeys(pwr);
                  #if defined(SORT_LISTS)
                      #if defined(SPLIT_SEARCH)
                      #if defined(SPLIT_SEARCH_LOOP)
                while
                      #else // defined(SPLIT_SEARCH_LOOP)
                if
                      #endif // defined(SPLIT_SEARCH_LOOP)
                   (wPopCnt >= cnSplitSearchThresholdInt)
                {
                    if (piKeys[wPopCnt / 2] <= iKey)
                    {
                        piKeys = &piKeys[wPopCnt / 2];
                        wPopCnt -= wPopCnt / 2;
                    }
                    else
                    {
                        wPopCnt /= 2;
                    }
                }
                      #endif // defined(SPLIT_SEARCH)
                if ((iKeyLoop = piKeys[wPopCnt - 1]) > iKey)
                {
                    while ((iKeyLoop = *piKeys++) < iKey);
                }
                  #else // defined(SORT_LISTS)
                unsigned int *piKeysEnd = &piKeys[wPopCnt];
                while (iKeyLoop = *piKeys, piKeys++ < piKeysEnd)
                  #endif // defined(SORT_LISTS)
                {
                    if (iKeyLoop == iKey)
                    {
                  #if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                        goto cleanup;
                  #endif // defined(REMOVE)
                  #if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) { goto undo; } // undo counting
                  #endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
                }
                break;
            }
              #endif // (cnBitsAtBottom < 32) && (cnBitsPerWord > 32)
            default:
            {
          #endif // defined(COMPRESSED_LISTS)
                // Looks like we might want a loop threshold of 8 for
                // 64-bit keys at the top level.
                // And there's not much difference with threshold of
                // 16 or 32.
                // Not sure about 64-bit
                // keys at a lower level or 32-bit keys at the top level.
                Word_t wKeyLoop;
                Word_t *pwKeys = pwr_pwKeys(pwr);
          #if defined(SORT_LISTS)
              #if defined(SPLIT_SEARCH)
              #if defined(SPLIT_SEARCH_LOOP)
                while
              #else // defined(SPLIT_SEARCH_LOOP)
                if
              #endif // defined(SPLIT_SEARCH_LOOP)
                   (wPopCnt >= cnSplitSearchThresholdWord)
                {
                    if (pwKeys[wPopCnt / 2] <= wKey)
                    {
                        pwKeys = &pwKeys[wPopCnt / 2];
                        wPopCnt -= wPopCnt / 2;
                    }
                    else
                    {
                        wPopCnt /= 2;
                    }
                }
              #endif // defined(SPLIT_SEARCH)
              #if ! defined(SPLIT_SEARCH_LOOP) \
                  || (cnSplitSearchThresholdWord > 2)
                if ((wKeyLoop = pwKeys[wPopCnt - 1]) > wKey) {
                    while ((wKeyLoop = *pwKeys++) < wKey);
                }
              #else // ! defined(SPLIT_SEARCH_LOOP) || ...
                wKeyLoop = *pwKeys;
              #endif // ! defined(SPLIT_SEARCH_LOOP) || ...
          #else // defined(SORT_LISTS)
                Word_t *pwKeysEnd = &pwKeys[wPopCnt];
                while (wKeyLoop = *pwKeys, pwKeys++ < pwKeysEnd)
          #endif // defined(SORT_LISTS)
                {
                    if (wKeyLoop == wKey)
                    {
          #if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                        goto cleanup;
          #endif // defined(REMOVE)
          #if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) { goto undo; } // undo counting
          #endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
                }
          #if defined(COMPRESSED_LISTS)
                break;
            } // end of default case
            } // end of switch
          #endif // defined(COMPRESSED_LISTS)
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        }
      #if defined(COMPRESSED_LISTS)
          #if ! defined(NO_SKIP_LINKS)
              #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        else
        {
            DBGX(printf("Mismatch at list wPrefix "OWx" nDL %d\n",
                        PWR_wPrefixNAT(pwRoot, pwrPrev, nDigitsLeft),
                        nDigitsLeft));
        }
              #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
          #endif // ! defined(NO_SKIP_LINKS)
      #endif // defined(COMPRESSED_LISTS)
  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        break;

    } // end of case T_LIST

    case T_BITMAP:
    {
#if !defined(LOOKUP)
  #if defined(REMOVE)
        if (bCleanup)
        {
            // RemoveGuts already removed the bitmap if necessary.
            return KeyFound;
        }
  #endif // defined(REMOVE)
  #if defined(PP_IN_LINK)
        DBGX(printf("Bitmap nDigitsLeft %d\n", nDigitsLeft));
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft);
        DBGX(printf("wPopCnt (before incr) %zd\n", (size_t)wPopCnt));
        DBGX(printf("wKeyPopMask "OWx"\n",
             wPrefixPopMask(nDigitsLeft)));
        set_PWR_wPopCnt(pwRoot, NULL, nDigitsLeft, wPopCnt + nIncr);
        DBGX(printf("wPopCnt %zd\n",
             (size_t)PWR_wPopCnt(pwRoot, NULL, nDigitsLeft)));
  #endif // defined(PP_IN_LINK)
#endif // !defined(LOOKUP)

        // We have to do the prefix check here if we're at the
        // bottom because wRoot contains a Bitmap.  Not a pointer.
        // Not a key.
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

  #if ! defined(NO_SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // Would like to combine the source code for this prefix
        // check and the one done in the compressed_lists section.
        // Notice that we're using pwr which was extracted from
        // the previous wRoot -- not the current wRoot -- if
        // not PP_IN_LINK.
        // If PP_IN_LINK, then we are using the current pwRoot.
        // nDigitsLeft is different for the two cases.
        if ( 0
          #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            || ! bNeedPrefixCheck
          #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
          #if defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNAT(pwRootPrefix, pwrPrefix, nDLRPrefix)
                    ^ wKey))
                < nDL_to_nBL(nDLRPrefix))
          #else // defined(SAVE_PREFIX)
            || (LOG(1 | (PWR_wPrefixNAT(pwRoot, pwrPrev, nDigitsLeft) ^ wKey))
                    // pwr_nBitsIndexSz term is necessary because pwrPrev
                    // prefix does not contain any less significant bits.
                    < (cnBitsAtBottom
              #if ! defined(PP_IN_LINK)
                            + nDL_to_nBitsIndexSzNAT(nDigitsLeft + 1)
              #endif // ! defined(PP_IN_LINK)
                ))
          #endif // defined(SAVE_PREFIX)
            )
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // ! defined(NO_SKIP_LINKS)
        {
  #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
      #if 0
            // Haven't really thought out use of cnDigitsAtBottom here.
            // Probably need cnDigitsAtBottom + 1 unless PP_IN_LINK.
            // But cnDigitsAtBottom + 1 is probably just a waste of
            // code since the switch probably won't exist in that case.
            return PWR_wPopCntNotAtTop(pwRoot, pwrPrev, cnDigitsAtBottom + 1)
                ? KeyFound : ! KeyFound;
      #else
            // Remove is incomplete and may leave the switch in
            // place even after all keys in all lists have been removed.
            // This makes it cumbersome to disambiguate a zero value
            // returned from PWR_wPopCntNotAtTop.
            return KeyFound;
      #endif
  #else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
      #if (cnBitsAtBottom <= cnLogBitsPerWord)
            DBGX(printf(
                "BitIsSetInWord(wRoot "OWx" wKey "OWx")\n",
                    wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

            if (BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)))
            {
          #if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                goto cleanup;
          #endif // defined(REMOVE)
          #if defined(INSERT) && !defined(RECURSIVE)
                if (nIncr > 0)
                {
                    goto undo; // undo counting
                }
          #endif // defined(INSERT) && !defined(RECURSIVE)
                return KeyFound;
            }

            DBGX(printf("! BitIsSetInWord\n"));
      #else // (cnBitsAtBottom <= cnLogBitsPerWord)
            DBGX(printf(
                "Evaluating BitIsSet(wRoot "OWx" wKey "OWx") ...\n",
                    wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

            if (BitIsSet(wr_pwr(wRoot),
                wKey & (EXP(cnBitsAtBottom) - 1UL)))
            {
          #if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
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
      #endif // (cnBitsAtBottom <= cnLogBitsPerWord)
  #endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
        }
  #if ! defined(NO_SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        else
        {
            DBGX(printf("Mismatch at bitmap wPrefix "OWx"\n",
                        PWR_wPrefixNAT(pwRoot, pwrPrev, nDigitsLeft)));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // ! defined(NO_SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

        break;

    } // end of case T_BITMAP

#if defined(T_ONE)

    case T_ONE: // one key (full word) list
    {
  #if defined(REMOVE)
        if (bCleanup) { return Success; }
  #endif // defined(REMOVE)

  #if defined(PP_IN_LINK)
        assert(nDigitsLeft != cnDigitsPerWord); // handled in wrapper
      #if ! defined(LOOKUP)
        set_PWR_wPopCnt(pwRoot, NULL, nDigitsLeft,
            PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) + 1);
      #endif // ! defined(LOOKUP)
  #endif // defined(PP_IN_LINK)

  #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        if (*pwr == wKey)
        {
      #if defined(REMOVE)
            RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
            goto cleanup;
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
  // wPopCnt in switch and in link to switch are adjusted elsewhere.
  #if defined(PP_IN_LINK)
      #if defined(REMOVE)
        // Can we combine bCleanup context with nType in switch variable?
        if ( ! bCleanup )
      #endif // defined(REMOVE)
        {
      #if ! defined(LOOKUP)
            // What about defined(RECURSIVE)?
            if (nDigitsLeft != cnDigitsPerWord)
            {
                // If nDigitsLeft != cnDigitsPerWord then we're not at top.
                // And pwRoot is initialized despite what gcc might think.
                assert(PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) == 0);
                set_PWR_wPopCnt(pwRoot, NULL, nDigitsLeft, nIncr);
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
    // pwRoot and nDigitsLeft that Insert passed to InsertGuts.
      #if !defined(RECURSIVE)
    assert((nDigitsLeft != cnDigitsPerWord) || (pwRoot == pwRootOrig));
      #endif // !defined(RECURSIVE)
  #endif // defined(BM_IN_LINK)
    // InsertGuts is called with a pwRoot and nDigitsLeft indicates the
    // bits that were not decoded in identifying pwRoot.  nDigitsLeft
    // does not include any skip indicated in the type field of *pwRoot.
    return InsertGuts(pwRoot, wKey, nDigitsLeft, wRoot);
undo:
#endif // defined(INSERT)
#if defined(REMOVE) && !defined(RECURSIVE)
    if (nIncr < 0)
#endif // defined(REMOVE) && !defined(RECURSIVE)
#if !defined(LOOKUP) && !defined(RECURSIVE)
    {
  #if defined(REMOVE)
        if (bCleanup)
        {
            return KeyFound; // nothing to clean up
        }
        printf("\n# Not bCleanup -- Remove failure!\n");
  #endif // defined(REMOVE)
        // Undo the counting we did on the way in.
        nIncr *= -1;
  #if defined(REMOVE)
restart:
  #endif // defined(REMOVE)
        pwRoot = pwRootOrig;
        nDigitsLeft = nDigitsLeftOrig;
        goto top;
    }
  #endif // !defined(LOOKUP) && !defined(RECURSIVE)
    return Failure;
  #if defined(REMOVE)
cleanup:
    bCleanup = 1; // ?? nIncr == 0 ??
    DBGX(printf("Cleanup pwRO "OWx" nDLO %d\n",
                (Word_t)pwRootOrig, nDigitsLeftOrig));
    goto restart;
  #endif // defined(REMOVE)
}

#undef RECURSIVE
#undef InsertGuts
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

#endif // (cnBitsAtBottom >= cnBitsPerWord)
#endif // (cnBitsPerDigit >= cnBitsPerWord)
#endif // (cnDigitsPerWord <= 1)

#if defined(LOOKUP)

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnBitsAtBottom != cnBitsPerWord)

  #if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf.
    // Why?  So we don't have to handle lists with wPopCnt in them
    // in the mainline code for PP_IN_LINK?
    // Do not assume the list is sorted -- so this code doesn't have to
    // be ifdef'd.
    Word_t wRoot = (Word_t)pcvRoot;
    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    if (!tp_bIsSwitch(nType) && (wRoot != 0))
    {
        Word_t wPopCnt;
        Word_t *pwKeys;
#if defined(T_ONE)
        if (nType == T_ONE) {
            wPopCnt = 1;
            pwKeys = ls_pwKeys(pwr);
        } else
#endif // defined(T_ONE)
        {
            wPopCnt = ls_wPopCnt(pwr);
            pwKeys = ls_pwKeys(pwr) + 1; // pop count is in 1st element at top
        }

        for (unsigned nn = 0; nn < wPopCnt; nn++)
        {
            if (pwKeys[nn] == wKey) { return Success; }
        }
        return Failure;
    }
  #endif // (cwListPopCntMax != 0) && defined(PP_IN_LINK)

    return Lookup((Word_t)pcvRoot, wKey);

#else // (cnBitsAtBottom != cnBitsPerWord)

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

#endif // (cnBitsAtBottom != cnBitsPerWord)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(LOOKUP)

#if defined(INSERT)

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnBitsAtBottom != cnBitsPerWord)

    int status;

    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if defined(DEBUG)
    pwRootLast = (Word_t *)ppvRoot;
  #endif // defined(DEBUG)

  #if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf.  Why?
    // To simplify Lookup for PP_IN_LINK.  Does it still apply?
    // Do not assume the list is sorted.
    // But if we do the insert here, and the list is sorted, then leave it
    // sorted -- so we don't have to worry about ifdefs in this code.
    Word_t *pwRoot = (Word_t *)ppvRoot;
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    if (!tp_bIsSwitch(nType))
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Success)
        {
            status = Failure;
        }
        else
        {
            Word_t wPopCnt
                = (nType == T_NULL) ? 0
#if defined(T_ONE)
                : (nType == T_ONE) ? 1
#endif // defined(T_ONE)
                : ls_wPopCnt(pwr);

            if (wPopCnt == cwListPopCntMax)
            {
                status = InsertGuts(pwRoot, wKey, cnDigitsPerWord, wRoot);
            }
            else
            {
                Word_t *pwListNew
                            = NewList(wPopCnt + 1, cnDigitsPerWord, wKey);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew);
#if defined(T_ONE)
                if (wPopCnt == 0) {
                    set_wr(wRoot, pwListNew, T_ONE);
                } else
#endif // defined(T_ONE)
                {
                    ++pwKeysNew; // pop count is in first element at top
                    set_ls_wPopCnt(pwKeysNew, wPopCnt + 1);
                    set_wr(wRoot, pwListNew, T_LIST);
                }
                Word_t *pwKeys = ls_pwKeys(pwr);
#if defined(T_ONE)
                if (wPopCnt != 1)
#endif // defined(T_ONE)
                {
                    ++pwKeys; // pop count is in first element at top
                }
                unsigned nn;
                for (nn = 0; (nn < wPopCnt) && (pwKeys[nn] < wKey); nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                pwKeysNew[nn] = wKey;
                COPY(&pwKeysNew[nn + 1], &pwKeys[nn], wPopCnt - nn);
                if (wPopCnt != 0)
                {
                    OldList(pwr, wPopCnt, cnDigitsPerWord);
                }
                *pwRoot = wRoot;
                status = Success;
            }
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    {
        status = Insert((Word_t *)ppvRoot, wKey, cnDigitsPerWord);
    }

  #if defined(DEBUG)
    if (status == Success) { wInserts++; } // count successful inserts
  #endif // defined(DEBUG)

    DBGI(printf("\n# After Insert(wKey "OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGI(printf("\n"));

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wInserts);
  #endif // defined(DEBUG)

    return status;

#else // (cnBitsAtBottom != cnBitsPerWord)

    // one big Bitmap

    Word_t wRoot = (Word_t)*ppvRoot;
    Word_t wByteNum, wByteMask;
    char c;

    DBGI(printf("\nJudy1Set(ppvRoot %p wKey "OWx") wRoot "OWx"\n",
        (void *)ppvRoot, wKey, wRoot));

    if (wRoot == 0)
    {
        wRoot = JudyMalloc
            (EXP(cnBitsPerWord - cnLogBitsPerByte - cnLogBytesPerWord));
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

#endif // (cnBitsAtBottom != cnBitsPerWord)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(INSERT)

#if defined(REMOVE)

int
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
#if (cnBitsAtBottom != cnBitsPerWord)

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
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    if (!tp_bIsSwitch(nType))
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Failure)
        {
            status = Failure;
        }
        else
        {
            Word_t wPopCnt = 
#if defined(T_ONE)
                    (nType == T_ONE) ? 1 :
#endif // defined(T_ONE)
                    ls_wPopCnt(pwr);

            Word_t *pwListNew;
            if (wPopCnt != 1)
            {
                pwListNew = NewList(wPopCnt - 1, cnDigitsPerWord, wKey);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew);
#if defined(T_ONE)
                if (wPopCnt == 2) {
                    set_wr(wRoot, pwListNew, T_ONE);
                } else
#endif // defined(T_ONE)
                {
                    ++pwKeysNew; // pop count is in 1st element at top
                    set_ls_wPopCnt(pwKeysNew, wPopCnt - 1);
                    set_wr(wRoot, pwListNew, T_LIST);
                }

                Word_t *pwKeys = ls_pwKeys(pwr);
#if defined(T_ONE)
                if (wPopCnt != 1)
#endif // defined(T_ONE)
                {
                    ++pwKeys; // pop count is in first element at top
                }
                unsigned nn;
                for (nn = 0; pwKeys[nn] != wKey; nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                COPY(&pwKeysNew[nn], &pwKeys[nn + 1], wPopCnt - nn - 1);
            }
            else
            {
                set_wr(wRoot, NULL, T_NULL);
            }
            OldList(pwr, wPopCnt, cnDigitsPerWord);
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
    if (status == Success) wInserts--; // count successful inserts
  #endif // defined(DEBUG)

  #if defined(DEBUG_REMOVE)
    {
        printf("\n# After Remove(wKey "OWx") %s Dump\n", wKey,
            status == Success ? "Success" : "Failure");
        Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        printf("\n");
    }
  #endif // defined(DEBUG_REMOVE)

  #if defined(DEBUG)
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wInserts);
  #endif // defined(DEBUG)

    return status;

#else // (cnBitsAtBottom != cnBitsPerWord)

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

#endif // (cnBitsAtBottom != cnBitsPerWord)

    (void)PJError; // suppress "unused parameter" compiler warnings
}

#endif // defined(REMOVE)

