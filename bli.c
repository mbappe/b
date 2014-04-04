
// @(#) $Id: bli.c,v 1.116 2014/04/04 14:50:25 mike Exp mike $
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
#if (cnBitsPerDigit != 0)
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
    Word_t *pwRoot = NULL; // shouldn't be necessary; gcc thinks it is

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
#if !defined(LOOKUP_NO_LIST_DEREF)
    Word_t wPopCnt;
#endif // !defined(LOOKUP_NO_LIST_DEREF)
#endif // (cwListPopCntMax != 0)
#endif // !defined(LOOKUP) ... #elif ...
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    unsigned nType;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(REMOVE)
    int bCleanup = 0;
#endif // defined(REMOVE)
    Word_t *pwr = pwr; // suppress "uninitialized" compiler warning

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
    assert(nDigitsLeft > cnDigitsAtBottom); // keep LOOKUP lean
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
#if 0
            unsigned nTypeX = oh_nTypeX(wRoot);
            unsigned nDigitsLeft = oh_nDigitsLeft(wRoot);
#endif
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
            unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
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
                                + pwr_nBitsIndexSz(pwr)
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
                    >= (nDigitsLeftRoot * cnBitsPerDigit))))
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
            nDigitsLeft
                = nDigitsLeftRoot - (pwr_nBitsIndexSz(pwr) / cnBitsPerDigit);

            Word_t wIndex = ((wKey >> (nDigitsLeft * cnBitsPerDigit))
                                & (EXP(pwr_nBitsIndexSz(pwr)) - 1));

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
                    for (Word_t ww = 0; ww < EXP(cnBitsPerDigit); ww++)
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
                        DBGX(printf("wr_nDL %"_fw"d\n",
                                        wr_nDigitsLeft(*pwRootLn)));
                        DBGX(printf("PWR_wPopCnt %"_fw"d\n",
                            PWR_wPopCnt(pwRootLn,
                                        NULL, wr_nDigitsLeft(*pwRootLn))));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                        if ((*pwRootLn != 0) && (((ww != wIndex))
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                                || (PWR_wPopCnt(pwRootLn,
                                        NULL,
                                        wr_bIsSwitch(*pwRootLn)
                                            ? wr_nDigitsLeft(*pwRootLn)
                                            : nDigitsLeft)
                                    != 0)
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                                || (PWR_wPopCnt(pwRootLn, NULL, nDigitsLeft)
                                    != 0)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                                ))
                        {
                            DBGX(printf("Not empty ww %zd wIndex %zd\n",
                                 (size_t)ww, (size_t)wIndex));
                            goto notEmpty; // switch pop is not zero
                        }
                    }
                    // switch pop is zero
                    FreeArrayGuts(pwRoot, wKey,
                        (nDigitsLeftUp == cnDigitsPerWord)
                            ? cnBitsPerWord : nDigitsLeftUp * cnBitsPerDigit,
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
                // I should check for the pop count decrementing to zero.
                // And add the switch to a list for removal in case the remove
                // is successful.
                // BUG:  What if attempting to insert a dup and
                // we're already at max pop?
                // if (wPopCnt == 0) && at least one link pop count is not 0
                // BUG:  What if attempting to remove a key that isn't present
                // and we're already at pop zero?
                // What about empty subtrees with non-zero pointers left
                // around after remove?
                // if (wPopCnt == 0) && all links full) return KeyFound;
                wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDigitsLeftRoot);
#if defined(REMOVE)
                if (bCleanup)
                {
                    if (wPopCnt == 0)
                    {
                        FreeArrayGuts(pwRoot, wKey,
#if defined(BM_IN_LINK)
                            (nDigitsLeftUp == cnDigitsPerWord)
                                ? cnBitsPerWord
                                : nDigitsLeftUp * cnBitsPerDigit,
#else // defined(BM_IN_LINK)
                            nDigitsLeftRoot * cnBitsPerDigit,
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

            if (nDigitsLeft > cnDigitsAtBottom)
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
                                + pwr_nBitsIndexSz(pwr)
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
                if (cnBitsAtBottom <= cnLogBitsPerWord) // compile time
                {
                    DBGX(printf(
                        "BitIsSetInWord(wRoot "OWx" wKey "OWx")\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

                    if (BitIsSetInWord(wRoot,
                        wKey & (EXP(cnBitsAtBottom) - 1UL)))
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
                }
                else if (wRoot != 0)
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

#endif // (cnBitsPerDigit != 0)

#if defined(LOOKUP)

#if !defined(INTERN_JUDY1) && !defined(EXTERN_JUDY1)
#include "judy1x.c"
#endif // !defined(INTERN_JUDY1) && !defined(EXTERN_JUDY1)

#endif // defined(LOOKUP)

#if defined(REMOVE)

int
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
#if (cnBitsPerDigit != 0)

    int status;

    DBGR(printf("\n\n# Judy1Unset wKey "OWx"\n", wKey));

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
        Dump((Word_t)*ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        printf("\n");
    }
#endif // defined(DEBUG_REMOVE)

    return status;

#else // (cnBitsPerDigit != 0)

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

#endif // (cnBitsPerDigit != 0)

    (void)PJError; // suppress "unused parameter" compiler warnings
}

#endif // defined(REMOVE)

