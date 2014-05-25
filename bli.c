
// @(#) $Id: bli.c,v 1.127 2014/04/26 00:50:38 mike Exp $
// @(#) $Source: /Users/mike/b/RCS/bli.c,v $

// This file is #included in other .c files three times.
// Once with #define LOOKUP, #undef INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #define INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #undef INSERT and #define REMOVE.
// The ifdefs make the code hard to read.  But not all combinations are
// valid so the best way to read the code is to pick a set of defines
// and ignore the code that gets thrown away.  Or maybe even use unifdef.
//
// Here are the valid combinations:
//
//     LOOKUP
//     LOOKUP && SKIP_LINKS
//     LOOKUP && SKIP_LINKS && SKIP_PREFIX_CHECK
//
//     INSERT && SKIP_LINKS
//     INSERT && SKIP_LINKS && RECURSIVE_INSERT
//
//     REMOVE && SKIP_LINKS
//     REMOVE && SKIP_LINKS && RECURSIVE_REMOVE

// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#if (cnDigitsPerWord != 1)
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

#if defined(LOOKUP)

    unsigned nDigitsLeft = cnDigitsPerWord;
#if defined(SKIP_LINKS)
#if defined(SKIP_PREFIX_CHECK)
    unsigned bNeedPrefixCheck = 0;
#endif // defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
    Word_t *pwRoot;
#if defined(BM_IN_LINK)
    pwRoot = NULL; // used for top detection
#else // defined(BM_IN_LINK)
#if defined(PP_IN_LINK) && ( ! defined(SKIP_PREFIX_CHECK) )
    // silence unwarranted gcc used before initialized warning
    // it is only uninitialized on the first time through the loop
    // and we only use it if nBitsLeft != cnBitsPerWord
    pwRoot = NULL;
#endif // defined(PP_IN_LINK) && ( ! defined(SKIP_PREFIX_CHECK) )
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
    unsigned nDigitsLeftUp = nDigitsLeft; (void)nDigitsLeftUp; // silence gcc
#endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)

#endif // defined(LOOKUP)

#if !defined(RECURSIVE)
#if !defined(LOOKUP) || defined(BM_IN_LINK)
    Word_t *pwRootOrig = pwRoot;
#endif // !defined(LOOKUP) || defined(BM_IN_LINK)
#endif // !defined(RECURSIVE)

    unsigned nDigitsLeftRoot;

#if !defined(LOOKUP)
    Word_t wPopCnt;
#else // !defined(LOOKUP)
#if (cwListPopCntMax != 0)
#if !defined(LOOKUP_NO_LIST_DEREF) || defined(PP_IN_LINK)
    Word_t wPopCnt;
#endif // !defined(LOOKUP_NO_LIST_DEREF) || defined(PP_IN_LINK)
#endif // (cwListPopCntMax != 0)
#endif // !defined(LOOKUP) ... #elif ...
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    unsigned nType;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(REMOVE)
    int bCleanup = 0;
#endif // defined(REMOVE)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwr = pwr; // suppress "uninitialized" compiler warning
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if !defined(LOOKUP)
#if !defined(RECURSIVE)
top:
#endif // !defined(RECURSIVE)
    wRoot = *pwRoot;
#endif // !defined(LOOKUP)
#if defined(LOOKUP) || !defined(RECURSIVE)
again:
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if ( ! defined(LOOKUP) )
    assert(nDigitsLeft > nBL_to_nDL(cnBitsAtBottom)); // valid for LOOKUP too
    DBGX(printf("# pwRoot %p ", (void *)pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    nType = wr_nType(wRoot);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
    {
        DBGX(printf("List nDigitsLeft %d\n", nDigitsLeft));
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMask(nDigitsLeft)));

#if defined(REMOVE)
        if (bCleanup)
        {
            // RemoveGuts already removed the list if necessary.
            return KeyFound;
        }
#endif // defined(REMOVE)

#if defined(PP_IN_LINK)
        // What about defined(RECURSIVE)?
        if (nDigitsLeft != cnDigitsPerWord)
        {
            wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft);
#if ! defined(LOOKUP)
            DBGX(printf("wPopCnt (before incr) %zd\n", (size_t)wPopCnt));
            set_PWR_wPopCnt(pwRoot, NULL, nDigitsLeft, wPopCnt + nIncr);
            DBGX(printf("wPopCnt (after incr) %zd\n",
                        (size_t)PWR_wPopCnt(pwRoot, NULL, nDigitsLeft)));
#endif // ! defined(LOOKUP)
        }
#endif // defined(PP_IN_LINK)

        if (wRoot != 0)
        {
#if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
// This short-circuit is for analysis only.
            return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

#if defined(PP_IN_LINK)
            if (nDigitsLeft == cnDigitsPerWord)
            {
                // Will have to figure this out to get rid
                // of pop count in list.
                wPopCnt = ls_wPopCnt(wRoot);
            }
#else // defined(PP_IN_LINK)
            wPopCnt = ls_wPopCnt(wRoot);
            DBGX(printf("List wPopCnt %"_fw"u\n", wPopCnt));
#if defined(LOOKUP)
            SMETRICS(j__SearchPopulation += wPopCnt);
#endif // defined(LOOKUP)
#endif // defined(PP_IN_LINK)

#if defined(COMPRESSED_LISTS)
#if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
            // nDigitsLeft is relative to the bottom of the switch
            // containing the pointer to the leaf.
            unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
#endif // !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            // We don't support skip links directly to leaves -- yet.
            // Even with defined(PP_IN_LINK).
            // It is sufficient to check the prefix at the switch just
            // above the leaf.
            // pwr is left from the previous iteration of the goto again loop.
            // Would like to combine the source code for this prefix
            // check and the one done in the bitmap section if possible.
            Word_t wPrefix;
#if (cnBitsPerWord > 32)
            if ((nBitsLeft > 32) // leaf has whole key
#else // (cnBitsPerWord > 32)
            if ((nBitsLeft > 16) // leaf has whole key
#endif // (cnBitsPerWord > 32)
                // leaf does not have whole key
                // What if there were no skips in the part that is missing?
                || ( ! bNeedPrefixCheck ) // we followed no skip links
                || ((wPrefix = PWR_wPrefixNotAtTop(pwRoot, pwr, nDigitsLeft),
                    LOG(1 | (wPrefix ^ wKey))
                        // prefix in parent switch doesn't contain last digit
                        // for !defined(PP_IN_LINK) case
                        < (nBitsLeft
#if !defined(PP_IN_LINK)
                                + nDL_to_nBitsIndexSzNAT(nDigitsLeft + 1)
#endif // !defined(PP_IN_LINK)
                           ))))
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(COMPRESSED_LISTS)
            {
#if defined(DL_IN_LL)
                assert(ll_nDigitsLeft(wRoot) == nDigitsLeft);
#endif // defined(DL_IN_LL)
#if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
// This short-circuit is for analysis only.  We have retrieved the pop count
// and prefix but we have not dereferenced the list itself.
#if defined(PP_IN_LINK)
                return KeyFound;
#else // defined(PP_IN_LINK)
                return wPopCnt ? KeyFound : ! KeyFound;
#endif // defined(PP_IN_LINK)
#else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

                // todo: save insertion point in sorted list and pass it to
                // InsertGuts
                // todo: possibly do insertion right here if list isn't full
                for (unsigned n = 0; n < wPopCnt; n++)
                {
#if defined(LOOKUP)
                    SMETRICS(j__SearchCompares++);
#endif // defined(LOOKUP)

#if defined(COMPRESSED_LISTS)
                    if ((nBitsLeft <= 8)
                            ? (wr_pcKeys(wRoot)[n] == (unsigned char)wKey)
                        : (nBitsLeft <= 16)
                            ? (wr_psKeys(wRoot)[n] == (unsigned short)wKey)
#if (cnBitsPerWord > 32)
                        : (nBitsLeft <= 32)
                            ? (wr_piKeys(wRoot)[n] == (unsigned int)wKey)
#endif // (cnBitsPerWord > 32)
                        : (wr_pwKeys(wRoot)[n] == wKey))
#else // defined(COMPRESSED_LISTS)
                    if (wr_pwKeys(wRoot)[n] == wKey)
#endif // defined(COMPRESSED_LISTS)
                    {
#if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                        goto cleanup;
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) goto undo; // undo counting
#endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
#if defined(SORT_LISTS)
#if defined(COMPRESSED_LISTS)
                    if ((nBitsLeft <= 8)
                            ? (wr_pcKeys(wRoot)[n] > (unsigned char)wKey)
                        : (nBitsLeft <= 16)
                            ? (wr_psKeys(wRoot)[n] > (unsigned short)wKey)
#if (cnBitsPerWord > 32)
                        : (nBitsLeft <= 32)
                            ? (wr_piKeys(wRoot)[n] > (unsigned int)wKey)
#endif // (cnBitsPerWord > 32)
                        : (wr_pwKeys(wRoot)[n] > wKey))
#else // defined(COMPRESSED_LISTS)
                    if (wr_pwKeys(wRoot)[n] > wKey)
#endif // defined(COMPRESSED_LISTS)
                    {
                        break;
                    }
#endif // defined(SORT_LISTS)
                }
#endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
            }
#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            else
            {
                DBGX(printf("Mismatch at list wPrefix "OWx" nDL %d\n",
                    wPrefix, nDigitsLeft));
            }
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#endif // defined(COMPRESSED_LISTS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        }
    }
    else
#else // (cwListPopCntMax != 0)
    if (wRoot != 0)
#endif // (cwListPopCntMax != 0)
    {
        // basic switch

        pwr = wr_tp_pwr(wRoot, nType); // pointer extracted from wRoot

#if defined(SKIP_LINKS)
        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);
        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved
#else // defined(SKIP_LINKS)
        nDigitsLeftRoot = nDigitsLeft; // prev
#endif // defined(SKIP_LINKS)

        DBGX(printf("Switch nDLR %d pwr %p\n", nDigitsLeftRoot, (void *)pwr));

#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // Record that there were prefix bits that were not checked.
        bNeedPrefixCheck |= (nDigitsLeftRoot < nDigitsLeft);
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        Word_t wPrefix;
        if ((nDigitsLeftRoot < nDigitsLeft)
            && ((wPrefix = PWR_wPrefixNotAtTop(pwRoot, pwr, nDigitsLeftRoot)),
                (LOG(1 | (wPrefix ^ wKey))
                    >= nDL_to_nBL_NotAtTop(nDigitsLeftRoot))))
        {
            DBGX(printf("Mismatch wPrefix "Owx"\n", wPrefix));
        }
        else // !! the "else" here is only for the INSERT/REMOVE case !!
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
        {
#if !defined(LOOKUP)
#if defined(PP_IN_LINK) || defined(BM_IN_LINK)
            nDigitsLeftUp = nDigitsLeft;
#endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)
#endif // !defined(LOOKUP)
            nDigitsLeft = nDigitsLeftRoot - 1;

            Word_t wIndex = ((wKey >> nDL_to_nBL_NotAtTop(nDigitsLeft))
                // we can use NAT here even though we might be at top because
                // we're using it to mask off high bits and if we're at the
                // top then none of the high bits will be set anyway;
                // it's faster to do arithmetic than test to avoid it
                & (EXP(nDL_to_nBitsIndexSzNAT(nDigitsLeftRoot)) - 1));

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
//printf("nBmOffset %d\n", nBmOffset);
//printf("pwr %p\n", (void *)pwr);
//printf("PWR_pwBm %p\n", (void *)PWR_pwBm(pwRoot, pwr));
                Word_t wBm = PWR_pwBm(pwRoot, pwr)[nBmOffset];
//printf("wBm "OWx"\n", wBm);
                Word_t wBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
//printf("wBit "OWx"\n", wBit);
                // Test to see if link exists before figuring out where it is.
                if ( ! (wBm & wBit) )
                {
#if defined(BM_SWITCH_FOR_REAL)
                    DBGX(printf("missing link\n"));
                    nDigitsLeft = nDigitsLeftRoot; // back up for InsertGuts
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
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                        DBGX(printf("wr_nDL %"_fw"d",
                                        wr_nDigitsLeft(*pwRootLn)));
                        DBGX(printf(" PWR_wPopCnt %"_fw"d\n",
                            PWR_wPopCnt(pwRootLn,
                                        NULL,
                                        wr_bIsSwitch(*pwRootLn)
                                            ? wr_nDigitsLeft(*pwRootLn)
                                            : nDigitsLeft)));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                        if (((*pwRootLn != 0) && (ww != wIndex))
                                || (
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                                    PWR_wPopCnt(pwRootLn,
                                            NULL,
                                            wr_bIsSwitch(*pwRootLn)
                                                ? wr_nDigitsLeft(*pwRootLn)
                                                : nDigitsLeft)
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                                    PWR_wPopCnt(pwRootLn, NULL, nDigitsLeft)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
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
                        FreeArrayGuts(pwRoot, wKey,
#if defined(BM_IN_LINK)
                            nDL_to_nBL(nDigitsLeftUp),
#else // defined(BM_IN_LINK)
                            nDL_to_nBL(nDigitsLeftRoot),
#endif // defined(BM_IN_LINK)
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

// Simplify this to nDigitsLeft > 1 when we get rid of non-1 cnDigitsAtBottom.
            if (nDigitsLeft > nBL_to_nDL(cnBitsAtBottom))
            {
                // We may need to check the prefix of the switch we just
                // visited in the next iteration of the loop
                // #if defined(COMPRESSED_LISTS).
                // We preserve the value of pwr.
#if defined(LOOKUP) || !defined(RECURSIVE)
                goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE)
                return InsertRemove(pwRoot, wKey, nDigitsLeft);
#endif // defined(LOOKUP) || !defined(RECURSIVE)
            }

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
#else // defined(PP_IN_LINK)
#endif // defined(PP_IN_LINK)
#endif // !defined(LOOKUP)

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a Bitmap.  Not a pointer.
            // Not a key.
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
            return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            // Would like to combine the source code for this prefix
            // check and the one done in the compressed_lists section.
            // Notice that we're using pwr which was extracted from
            // the previous wRoot -- not the current wRoot -- if
            // not PP_IN_LINK.
            // If PP_IN_LINK, then we are using the current pwRoot.
            // nDigitsLeft is different for the two cases.
            Word_t wPrefix;
            if (( ! bNeedPrefixCheck )
                || ((wPrefix = PWR_wPrefixNotAtTop(pwRoot, pwr, nDigitsLeft),
                    LOG(1 | (wPrefix ^ wKey)))
                        // pwr_nBitsIndexSz term is necessary because pwr
                        // prefix does not contain any less significant bits.
                        < (cnBitsAtBottom
#if !defined(PP_IN_LINK)
                                + nDL_to_nBitsIndexSzNAT(nDigitsLeft + 1)
#endif // !defined(PP_IN_LINK)
                           )))
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
            {
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
#if 0
                // Haven't really thought out use of cnDigitsAtBottom here.
                // Probably need cnDigitsAtBottom + 1 unless PP_IN_LINK.
                // But cnDigitsAtBottom + 1 is probably just a waste of
                // code since the switch probably won't exist in that case.
                return PWR_wPopCntNotAtTop(pwRoot, pwr, cnDigitsAtBottom + 1)
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
                if (wRoot != 0)
                {
                    DBGX(printf(
                        "Evaluating BitIsSet(wRoot "OWx" wKey "OWx") ...\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

                    if (BitIsSet(wRoot,
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
                }
#endif // (cnBitsAtBottom <= cnLogBitsPerWord)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            }
#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            else
            {
                DBGX(printf("Mismatch at bitmap wPrefix "OWx"\n",
                    PWR_wPrefixNotAtTop(pwRoot, pwr, nDigitsLeftRoot)));
            }
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        }
    }

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

#endif // (cnDigitsPerWord != 1)

#if defined(LOOKUP)

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnDigitsPerWord != 1)

#if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf.
    // Do not assume the list is sorted -- so this code doesn't have to
    // be ifdef'd.
    Word_t wRoot = (Word_t)pcvRoot;
    unsigned nType = wr_nType(wRoot);
    if (!tp_bIsSwitch(nType) && (wRoot != 0))
    {
        Word_t wPopCnt = ls_wPopCnt(wRoot);
        Word_t *pwKeys = ls_pwKeys(wRoot) + 1;
        for (unsigned nn = 0; nn < wPopCnt; nn++)
        {
            if (pwKeys[nn] == wKey) { return Success; }
        }
        return Failure;
    }
#endif // (cwListPopCntMax != 0) && defined(PP_IN_LINK)

    return Lookup((Word_t)pcvRoot, wKey);

#else // (cnDigitsPerWord != 1)

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
    return (((Word_t *)pcvRoot)[wWordNum] & wWordMask) ? Success : Failure;
#endif // defined(LOOKUP_NO_BITMAP_DEREF)

#endif // defined(BITMAP_BY_BYTE)

#endif // (cnDigitsPerWord != 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(LOOKUP)

#if defined(INSERT)

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnDigitsPerWord != 1)

    int status;

    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

#if defined(DEBUG)
    pwRootLast = (Word_t *)ppvRoot;
#endif // defined(DEBUG)

#if (cwListPopCntMax != 0) && defined(PP_IN_LINK)
    // Handle the top level list leaf.
    // Do not assume the list is sorted.
    // But if we do the insert here, and the list is sorted, then leave it
    // sorted -- so we don't have to worry about ifdefs in this code.
    Word_t *pwRoot = (Word_t *)ppvRoot;
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);
    if (!tp_bIsSwitch(nType))
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Success)
        {
            status = Failure;
        }
        else
        {
            Word_t wPopCnt = (wRoot != 0) ? ls_wPopCnt(wRoot) : 0;
            if (wPopCnt == cwListPopCntMax)
            {
                status = InsertGuts(pwRoot, wKey, cnDigitsPerWord, wRoot);
            }
            else
            {
                Word_t *pwListNew
                    = NewList(wPopCnt + 1, cnDigitsPerWord, wKey);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew) + 1;
                Word_t *pwKeys = ls_pwKeys(wRoot) + 1;
                unsigned nn;
                for (nn = 0; (nn < wPopCnt) && (pwKeys[nn] < wKey); nn++) { }
                set_ls_wPopCnt(pwKeysNew, wPopCnt + 1);
                COPY(pwKeysNew, pwKeys, nn);
                pwKeysNew[nn] = wKey;
                COPY(&pwKeysNew[nn + 1], &pwKeys[nn], wPopCnt - nn);
                if (wPopCnt != 0)
                {
                    OldList((Word_t *)*pwRoot, wPopCnt, cnDigitsPerWord);
                }
                *pwRoot = (Word_t)pwListNew;
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
    if (status == Success)
    {
        wInserts++; // count successful inserts
    }
#endif // defined(DEBUG)

#if defined(DEBUG_INSERT)
    {
        printf("\n# After Insert(wKey "OWx") Dump\n", wKey);
        Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        printf("\n");
    }
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG)
    {
        Word_t *pwRoot = (Word_t *)ppvRoot;
        Word_t wRoot = *pwRoot;
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        unsigned nType = wr_nType(wRoot);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        Word_t *pwr = wr_tp_pwr(wRoot, nType);
        Word_t wPopCnt;

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        if (!tp_bIsSwitch(nType))
        {
            wPopCnt = wr_ls_wPopCnt(wRoot);
        }
        else
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        {
#if defined(PP_IN_LINK)
            // no skip links at root for PP_IN_LINK -- no place for prefix
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            assert(tp_to_nDigitsLeft(nType) == cnDigitsPerWord);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            // add up the pops in the links

#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
    Word_t xx = 0;
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
            wPopCnt = 0;
            for (unsigned nn = 0;
                 // PP_IN_LINK implies no skip link at top.
                 nn < EXP(nDL_to_nBitsIndexSz(cnDigitsPerWord));
                 nn++)
            {
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
        if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
        {
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[xx].ln_wRoot;
                xx++;
#else // defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[nn].ln_wRoot;
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)

// *pwRootLn may not be a pointer to a switch
// It may be a pointer to a list leaf.
// And if cnDigitsAtBottom == cnDigitsPerWord - 1, then it could be a
// pointer to a bitmap?
                Word_t wPopCntLn;
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                unsigned nTypeLn = wr_nType(*pwRootLn);
                if (tp_bIsSwitch(nTypeLn))
                {
                    wPopCntLn
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      wr_nDigitsLeft(*pwRootLn));
                }
                else
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                {
// 0 will come here.
                    wPopCntLn
                        = PWR_wPopCnt(pwRootLn, NULL, cnDigitsPerWord - 1);
                }

#if defined(DEBUG_INSERT)
                if (wPopCntLn != 0)
                {
                    printf("Pop sum");
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" mask "OWx" %"_fw"d",
                        wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                        wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" nn %3d wPopCntLn %"_fw"d "OWx"\n",
                           nn, wPopCntLn, wPopCntLn);
                }
#endif // defined(DEBUG_INSERT)

                wPopCnt += wPopCntLn;

                // We use pwr_pLinks(pwr)[nn].ln_wRoot != 0 to disambiguate
                // wPopCnt == 0.  Hence we cannot allow Remove to leave
                // pwr_pLinks(pwr)[nn].ln_wRoot != 0 unless the actual
                // population count is not zero.
                if ((wPopCntLn == 0) && (*pwRootLn != 0))
                {
#if defined(DEBUG_INSERT)
                    printf("Pop sum (full)");
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" mask "Owx" %zd\n",
                        wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                        (size_t)wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
                    printf("nn %d wPopCntLn %zd "OWx"\n",
                        nn,
                        (size_t)wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1,
                        wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#endif // defined(DEBUG_INSERT)

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    wPopCnt += wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1;
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    wPopCnt += wPrefixPopMask(cnDigitsPerWord - 1) + 1;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                }
        }
            }
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            assert(wPopCnt - 1 <= wPrefixPopMask(tp_to_nDigitsLeft(nType)));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#else // defined(PP_IN_LINK)
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            wPopCnt = PWR_wPopCnt(NULL, pwr, tp_to_nDigitsLeft(nType));
            if (wPopCnt == 0)
            {
                wPopCnt = wPrefixPopMask(tp_to_nDigitsLeft(nType)) + 1;
            }
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            wPopCnt = PWR_wPopCnt(NULL, pwr, cnDigitsPerWord);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#endif // defined(PP_IN_LINK)
        }

        if (wPopCnt != wInserts)
        {
            printf("\nwPopCnt %"_fw"d wInserts %"_fw"d\n", wPopCnt, wInserts);
        }
        assert(wPopCnt == wInserts);
    }
#endif // defined(DEBUG)

    return status;

#else // (cnDigitsPerWord != 1)

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

#endif // (cnDigitsPerWord != 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(INSERT)

#if defined(REMOVE)

int
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
#if (cnDigitsPerWord != 1)

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
    if (!tp_bIsSwitch(nType))
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Failure)
        {
            status = Failure;
        }
        else
        {
            Word_t *pwListNew;
            Word_t wPopCnt = ls_wPopCnt(wRoot);
            if (wPopCnt != 1)
            {
                pwListNew = NewList(wPopCnt - 1, cnDigitsPerWord, wKey);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew) + 1;
                Word_t *pwKeys = ls_pwKeys(wRoot) + 1;
                unsigned nn;
                for (nn = 0; pwKeys[nn] != wKey; nn++) { }
                set_ls_wPopCnt(pwListNew, wPopCnt - 1);
                COPY(pwKeysNew, pwKeys, nn);
                COPY(&pwKeysNew[nn], &pwKeys[nn + 1], wPopCnt - nn - 1);
            }
            else
            {
                pwListNew = NULL;
            }
            OldList((Word_t *)*pwRoot, wPopCnt, cnDigitsPerWord);
            *pwRoot = (Word_t)pwListNew;
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

    return status;

#else // (cnDigitsPerWord != 1)

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

#endif // (cnDigitsPerWord != 1)

    (void)PJError; // suppress "unused parameter" compiler warnings
}

#endif // defined(REMOVE)

