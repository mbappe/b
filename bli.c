
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
Word_t j__SearchCompares;
Word_t j__SearchPopulation;
Word_t j__TreeDepth;
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
#define KeyFound  (assert(0), Failure)
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
#else // defined(LOOKUP)
    Word_t wRoot;
#if !defined(RECURSIVE)
    Word_t *pwRootOrig = pwRoot;
    unsigned nDigitsLeftOrig = nDigitsLeft;
#if defined(INSERT)
    int nIncr = 1;
#else // defined(INSERT)
    int nIncr = -1;
#endif // defined(INSERT)
#endif // !defined(RECURSIVE)
#endif // defined(LOOKUP)
#if defined(SKIP_LINKS)
    unsigned nDigitsLeftRoot;
#endif // defined(SKIP_LINKS)
#if !defined(LOOKUP)
    Word_t wPopCnt;
#elif (cwListPopCntMax != 0) && !defined(LOOKUP_NO_LIST_DEREF)
    Word_t wPopCnt;
#endif // !defined(LOOKUP) ... #elif ...
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    unsigned nType;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    Word_t *pwr; // pointer extracted from wRoot

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
    DBGX(printf("# pwRoot %p ", pwRoot));
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

            DBGX(printf("List\n"));

            wPopCnt = ls_wPopCnt(wRoot);

#if defined(LOOKUP)
            SMETRICS(j__SearchPopulation += wPopCnt);
#endif // defined(LOOKUP)

#if defined(COMPRESSED_LISTS)
            unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            // We don't support skip links directly to leaves yet so
            // it is sufficient to check the prefix at the branch just
            // above the leaf.
            // Would like to combine the source code for this prefix
            // check and the one done in the bitmap section if possible.
            // pwr is left from the previous iteration of the goto again loop.
            if ((nBitsLeft > 16) // leaf has whole key
                || ( ! bNeedPrefixCheck ) // we followed no skip links
                || (LOG(1 | (pwr_wPrefixNotAtTop(pwr, nDigitsLeft) ^ wKey))
// We can change nBitsLeft to be a better function of the size of the keys
// in the leaf.  How would it help?
                    < (nBitsLeft + pwr_nBitsIndexSz(pwr)))) // prefix matches
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#endif // defined(COMPRESSED_LISTS)
            {
#if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
// This short-circuit is for analysis only.  We have retrieved the pop count
// and prefix but we have not dereferenced the list itself.
                return wPopCnt ? KeyFound : ! KeyFound;
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
                    if ((nBitsLeft > 16) ? (wr_pwKeys(wRoot)[n] == wKey)
                        : (nBitsLeft > 8)
                            ? (wr_psKeys(wRoot)[n] == (unsigned short)wKey)
                            : (wr_pcKeys(wRoot)[n] == (unsigned char)wKey))
#else // defined(COMPRESSED_LISTS)
                    if (wr_pwKeys(wRoot)[n] == wKey)
#endif // defined(COMPRESSED_LISTS)
                    {
#if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) goto undo; // undo counting
#endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }
                }
#endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
            }
#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            else
            {
                DBGX(printf("Prefix mismatch at List wPrefix "OWx
                  " nDigitsLeft %d\n",
                    pwr_wPrefixNotAtTop(wRoot, nDigitsLeft), nDigitsLeft));
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

        pwr = wr_pwr(wRoot, nType); // pointer extracted from wRoot

#if defined(SKIP_LINKS)
        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

        DBGX(printf("Switch nDigitsLeft %d nDigitsLeftRoot %d pwr %p\n",
            nDigitsLeft, nDigitsLeftRoot, pwr));

#if ( ! defined(LOOKUP) )
        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved; keep lookup lean
#endif // ( ! defined(LOOKUP) )

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // Record that there were prefix bits that were not checked.
        bNeedPrefixCheck |= (nDigitsLeftRoot < nDigitsLeft);
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        if ((nDigitsLeftRoot < nDigitsLeft)
            && (LOG(1 | (pwr_wPrefix(pwr, nDigitsLeftRoot) ^ wKey))
                >= (nDigitsLeftRoot * cnBitsPerDigit)))
        {
            DBGX(printf("Prefix mismatch wPrefix "Owx"\n",
                pwr_wPrefix(pwr, nDigitsLeftRoot)));
        }
        else // !! the "else" here is only for the INSERT/REMOVE case !!
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
        {
            // size of array index
            unsigned nIndex;

#if defined(SKIP_LINKS)
            nDigitsLeft = nDigitsLeftRoot;
#endif // defined(SKIP_LINKS)

#if !defined(LOOKUP)
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


            wPopCnt = pwr_wPopCnt(pwr, nDigitsLeft);
            set_pwr_wPopCnt(pwr, nDigitsLeft, wPopCnt + nIncr);
            DBGI(printf("wPopCnt "wd"\n", pwr_wPopCnt(pwr, nDigitsLeft)));

#endif // !defined(LOOKUP)

            nDigitsLeft -= (pwr_nBitsIndexSz(pwr) / cnBitsPerDigit);
            nIndex = ((wKey >> (nDigitsLeft * cnBitsPerDigit))
                        & (EXP(pwr_nBitsIndexSz(pwr)) - 1));
            pwRoot = &pwr_pLinks(pwr)[nIndex].ln_wRoot;
            wRoot = *pwRoot;

            DBGX(printf("Next nDigitsLeft %d nIndex %d pwr %p pLinks %p\n",
                nDigitsLeft, nIndex, pwr, pwr_pLinks(pwr)));

            DBGX(printf("pwRoot %p wRoot "OWx"\n", pwRoot, wRoot));

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
            // the previous wRoot -- not the current wRoot.
            // The current wRoot might be an embedded bitmap.
            if (( ! bNeedPrefixCheck )
                || (LOG(1 | (pwr_wPrefixNotAtTop(pwr, nDigitsLeftRoot)
                        ^ wKey))
                    // pwr_nBitsIndexSz term is necessary because pwr prefix
                    // does not contain any less significant bits.
                    < (cnBitsAtBottom + pwr_nBitsIndexSz(pwr))))
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
            {
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
#if 0
                // Haven't really thought out use of cnDigitsAtBottom here.
                return pwr_wPopCntNotAtTop(pwr, cnDigitsAtBottom)
                    ? KeyFound : ! KeyFound;
#else
                // Remove is incomplete and may leave the switch in
                // place even after all keys in all lists have been removed.
                // This makes it cumbersome to disambiguate a zero value
                // returned from pwr_wPopCntNotAtTop.
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
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) goto undo; // undo counting
#endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }

                    DBGX(printf("! BitIsSetInWord\n"));
                }
                else if (wRoot != 0)
                {
                    DBGX(printf(
                        "BitIsSet(wRoot "OWx" wKey "OWx")\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

                    if (BitIsSet(wRoot,
                        wKey & (EXP(cnBitsAtBottom) - 1UL)))
                    {
#if defined(REMOVE)
                        RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE)
                        if (nIncr > 0) goto undo; // undo counting 
#endif // defined(INSERT) && !defined(RECURSIVE)
                        return KeyFound;
                    }

                    DBGX(printf("! BitIsSet\n"));
                }
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            }
#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            else
            {
                DBGX(printf("Prefix mismatch at Bitmap wPrefix "OWx"\n",
                    pwr_wPrefixNotAtTop(pwr, nDigitsLeftRoot)));
            }
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        }
    }

#if defined(INSERT)
    return InsertGuts(pwRoot, wKey, nDigitsLeft, wRoot);
undo:
#endif // defined(INSERT)
#if defined(REMOVE) && !defined(RECURSIVE)
    if (nIncr < 0)
#endif // defined(REMOVE) && !defined(RECURSIVE)
#if !defined(LOOKUP) && !defined(RECURSIVE)
    {
        // Undo the counting we did on the way in.
        nIncr *= -1;
        pwRoot = pwRootOrig;
        nDigitsLeft = nDigitsLeftOrig;
        goto top;
    }
#endif // !defined(LOOKUP) && !defined(RECURSIVE)
    return Failure;
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

    DBGR(printf("\nJudy1Unset wKey "OWx"\n", wKey));

    status = Remove((Word_t *)ppvRoot, wKey, cnDigitsPerWord);

#if defined(DEBUG)
    if (status == Success) wInserts--; // count successful inserts
#endif // defined(DEBUG)

#if defined(DEBUG_REMOVE)
    if (wInserts >= cwDebugThreshold)
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

