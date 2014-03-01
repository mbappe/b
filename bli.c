
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
//     INSERT && SKIP_LINKS && RECURSIVE_TWEAK
//
//     REMOVE && SKIP_LINKS
//     REMOVE && SKIP_LINKS && RECURSIVE_TWEAK

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
#define Tweak  Remove
#endif // defined(REMOVE)
#else // defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (assert(0), Failure)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#define Tweak  Insert
#endif // defined(LOOKUP) || defined(REMOVE)

Status_t
#if defined(LOOKUP)
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Tweak(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, int bCountOnly)
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
#if !defined(RECURSIVE_TWEAK)
    Word_t *pwRootOrig = pwRoot;
    unsigned nDigitsLeftOrig = nDigitsLeft;
    int bUndo = 0;
#endif // !defined(RECURSIVE_TWEAK)
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

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if !defined(LOOKUP)
#if !defined(RECURSIVE_TWEAK)
top:
#endif // !defined(RECURSIVE_TWEAK)
    wRoot = *pwRoot;
#endif // !defined(LOOKUP)
#if defined(LOOKUP) || !defined(RECURSIVE_TWEAK)
again:
#endif // defined(LOOKUP) || !defined(RECURSIVE_TWEAK)

#if ( ! defined(LOOKUP) )
    assert(nDigitsLeft > cnDigitsAtBottom); // keep LOOKUP lean
    DBGX(printf("# pwRoot %p ", pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if ((nType = wr_nType(wRoot)) != List)
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if (wRoot != 0)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

#if defined(SKIP_LINKS)
        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

        DBGX(printf("Switch nDigitsLeft %d nDigitsLeftRoot %d pwr %p\n",
            nDigitsLeft, nDigitsLeftRoot, pwr));

#if ( ! defined(LOOKUP) )
        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved; keep lookup lean
#endif // ( ! defined(LOOKUP) )

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        if (nDigitsLeftRoot < nDigitsLeft)
        {
            // Record that there were prefix bits that were not checked.
            bNeedPrefixCheck = 1;
        }
        // !! there is no "else" here in the LOOKUP case !!
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        if ((nDigitsLeftRoot < nDigitsLeft)
            && (LOG(1 | (sw_wPrefix(pwr, nDigitsLeftRoot) ^ wKey))
                >= (nDigitsLeftRoot * cnBitsPerDigit)))
        {
            DBGX(printf("Prefix mismatch wPrefix "Owx"\n",
                sw_wPrefix(pwr, nDigitsLeftRoot)));
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
            // increment or decrement population count on the way in
            {
#if defined(INSERT)
                Word_t wIncr = 1;
#else // defined(INSERT)
                Word_t wIncr = -1;
#endif // defined(INSERT)

                wPopCnt = sw_wPopCnt(pwr, nDigitsLeft);
#if 0
                // BUG:  What if attempting to insert a dup and
                // we're already at max pop?
                if (wPopCnt == 0) && at least one pwRoots is not 0
                // BUG:  What if attempting to remove a key that isn't present
                // and we're already at pop zero?
                // What about empty subtrees with non-zero pointers left
                // around after remove?
                if (wPopCnt == 0) && all pwRoots are 0
                {
                    // subtree is at full population or zero population
                    return KeyFound;
                }
#endif

#if !defined(RECURSIVE_TWEAK)
                if (bUndo) wIncr *= -1;
#endif // !defined(RECURSIVE_TWEAK)

                set_sw_wPopCnt(pwr, nDigitsLeft, wPopCnt + wIncr);

                assert(sw_wPopCnt(pwr, nDigitsLeft)
                    == ((wPopCnt + wIncr) & wPrefixPopMask(nDigitsLeft)));

                DBGI(printf("sw_wPopCnt "wd"\n",
                    sw_wPopCnt(pwr, nDigitsLeft)));
            }
#endif // !defined(LOOKUP)

            nDigitsLeft -= (pwr_nBitsIndexSz(pwr) / cnBitsPerDigit);

            nIndex
                = ((wKey >> (nDigitsLeft * cnBitsPerDigit))
                    & (EXP(pwr_nBitsIndexSz(pwr)) - 1));

            DBGX(printf("Next nDigitsLeft %d nIndex %d pwr %p pwRoots %p\n",
                nDigitsLeft, nIndex, pwr, pwr_pwRoots(pwr)));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            DBGX(printf("pwRoot %p wRoot "OWx"\n", pwRoot, wRoot));

            if (nDigitsLeft > cnDigitsAtBottom)
            {
#if defined(LOOKUP) || !defined(RECURSIVE_TWEAK)
                goto again;
#else // defined(LOOKUP) || !defined(RECURSIVE_TWEAK)
                return Tweak(pwRoot, wKey, nDigitsLeft, bCountOnly);
#endif // defined(LOOKUP) || !defined(RECURSIVE_TWEAK)
            }

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a Bitmap.  Not a pointer.
            // Not a key.
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
            return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(SKIP_LINKS)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            if (( ! bNeedPrefixCheck )
                || (LOG(1 | (sw_wPrefixNotAtTop(pwr, nDigitsLeftRoot) ^ wKey))
                    < (cnBitsAtBottom + pwr_nBitsIndexSz(pwr))))
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
            {
#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
                return KeyFound;
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
                        // BUG:  We should check if the switch is empty
                        // and free it and so on.
                        ClrBitInWord(wRoot,
                            wKey & ((EXP(cnBitsAtBottom)) - 1UL));
                        *pwRoot = wRoot;
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE_TWEAK)
                        if ( ! bUndo ) goto undo; // undo counting
#endif // defined(INSERT) && !defined(RECURSIVE_TWEAK)
                        return KeyFound;
                    }

                    DBGX(printf("! BitIsSetInWord\n"));
                }
                else if (wRoot != 0)
                {
                    DBGX(printf(
                        "BitIsSet(wRoot "OWx" wKey "OWx")\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)));

                    if (BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1UL)))
                    {
#if defined(REMOVE)
                        if (wPopCnt == 1)
                        {
                            OldBitmap(wRoot); *pwRoot = 0;
                            // BUG:  We should check if the switch is empty
                            // and free it and so on.
                        }
                        else
                        {
                            ClrBit(wRoot,
                                wKey & ((EXP(cnBitsAtBottom)) - 1UL));
                        }
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE_TWEAK)
                        if ( ! bUndo ) goto undo; // undo counting 
#endif // defined(INSERT) && !defined(RECURSIVE_TWEAK)
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
                    sw_wPrefix(pwr, nDigitsLeftRoot)));
            }
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
#endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        }
    }
#if (cwListPopCntMax != 0)
    else if (wRoot != 0)
    {
#if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        unsigned n;

        DBGX(printf("List\n"));

        wPopCnt = ls_wPopCnt(wRoot);

#if defined(LOOKUP)
        SMETRICS(j__SearchPopulation += wPopCnt);
#endif // defined(LOOKUP)

#if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wPopCnt ? KeyFound : ! KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        // todo: save insertion point in sorted list and pass it to InsertGuts
        // todo: possibly do insertion right here if list isn't full
        for (n = 0; n < wPopCnt; n++)
        {
#if defined(LOOKUP)
            SMETRICS(j__SearchCompares++);
#endif // defined(LOOKUP)

            if (wr_pwKeys(wRoot)[n] == wKey)
            {
#if defined(REMOVE)
                RemoveGuts(pwRoot, wKey, nDigitsLeft, wRoot);
                // BUG:  We should check if the switch is empty
                // and free it and so on.
#endif // defined(REMOVE)
#if defined(INSERT) && !defined(RECURSIVE_TWEAK)
                if ( ! bUndo ) goto undo; // undo counting
#endif // defined(INSERT) && !defined(RECURSIVE_TWEAK)
                return KeyFound;
            }
        }
#endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
    }
#endif // (cwListPopCntMax != 0)

#if defined(INSERT)
    return InsertGuts(pwRoot, wKey, nDigitsLeft, wRoot);
undo:
#endif // defined(INSERT)
#if defined(REMOVE) && !defined(RECURSIVE_TWEAK)
    if ( ! bUndo )
#endif // defined(REMOVE) && !defined(RECURSIVE_TWEAK)
#if !defined(LOOKUP) && !defined(RECURSIVE_TWEAK)
    {
        // Undo the counting we did on the way in.
        bUndo = 1;
        pwRoot = pwRootOrig;
        nDigitsLeft = nDigitsLeftOrig;
        goto top;
    }
#endif // !defined(LOOKUP) && !defined(RECURSIVE_TWEAK)
    return Failure;
}

#undef InsertGuts
#undef Tweak
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

