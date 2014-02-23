
// This file is #included into the main .c file three times.
// Once with #define LOOKUP, #undef INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #define INSERT and #undef REMOVE.
// Once with #undef LOOKUP, #undef INSERT and #define REMOVE.

#if defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Success)
#if defined(LOOKUP)
#define strLookupOrInsertOrRemove  "Lookup"
#define DBGX  DBGL
#else // defined(REMOVE)
#define strLookupOrInsertOrRemove  "Remove"
#define DBGX  DBGR
#define Insert      Remove
#define InsertGuts  RemoveGuts
#endif // defined(REMOVE)
#else // defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Failure)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#endif // defined(LOOKUP) || defined(REMOVE)

INLINE Status_t
#if defined(LOOKUP)
Lookup(Word_t wRoot, Word_t wKey, Word_t wState)
#else // defined(LOOKUP)
Insert(Word_t *pwRoot, Word_t wKey, Word_t wState)
#endif // defined(LOOKUP)
{
#if defined(LOOKUP)
    Word_t *pwRoot;
#else // defined(LOOKUP)
    Word_t wRoot = *pwRoot;
#endif // defined(LOOKUP)
    int nBitsLeftState = ws_nBitsLeft(wState);
    int nBitsLeftRoot;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

    assert(nBitsLeftState > cnBitsAtBottom);

again:

#if ( ! defined(LOOKUP) )
    DBGX(printf("# pwRoot %p ", pwRoot));
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" wState "OWx"\n",
            wRoot, wKey, wState));

    // Strange behavior can creep in if/when
    // cnBitsPerDigit is not a factor of cnBitsPerWord.
    assert(nBitsLeftState <= cnBitsPerWord);

    if (wr_bIsSwitchBL(wRoot, nBitsLeftRoot))
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

        assert(pwr != 0);
        assert(wRoot != 0);

        DBGX(printf("Switch"));
        DBGX(printf(" nBitsLeftState %d", nBitsLeftState));
        DBGX(printf(" nBitsLeftRoot %d", nBitsLeftRoot));
        DBGX(printf(" pwr %p", pwr));
        DBGX(printf("\n"));

        assert(nBitsLeftRoot <= nBitsLeftState); // reserved for later

#if defined(LOOKUP)
        if (nBitsLeftRoot != nBitsLeftState)
        {
            // Record that there were prefix bits that were not checked.
            set_ws_bNeedPrefixCheck(wState, 1);
        }
        // !! there is no "else" here in the LOOKUP case !!
#else // defined(LOOKUP)
        if ((nBitsLeftRoot != nBitsLeftState)
            && (pwr_wPrefix(pwr) != (wKey & ~(EXP(nBitsLeftRoot) - 1))))
        {
            DBGX(printf("Prefix mismatch wPrefix "Owx"\n", pwr_wPrefix(pwr)));
        }
        else // !! the "else" here is only for the INSERT/REMOVE case !!
#endif // defined(LOOKUP)
        {
            // size of array index
            int nBitsIndexSz = pwr_nBitsIndexSz(pwr);
            int nIndex;

            nBitsLeftState = nBitsLeftRoot - nBitsIndexSz;

            // In case nBitsLeftState is not an integral number of
            // digits.  Round it.
            nBitsLeftState = (nBitsLeftState + nBitsIndexSz - 1)
                    / nBitsIndexSz * nBitsIndexSz;

            set_ws_nBitsLeft(wState, nBitsLeftState);

            nIndex = ((wKey >> nBitsLeftState) & (EXP(nBitsIndexSz) - 1));

            DBGX(printf("Next"));
            //DBGX(printf(" nBitsIndexSz %d", nBitsIndexSz));
            DBGX(printf(" nBitsLeftState %d", nBitsLeftState));
            DBGX(printf(" nIndex %d", nIndex));
            DBGX(printf(" pwr %p pwRoots %p", pwr, pwr_pwRoots(pwr)));
            DBGX(printf("\n"));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a bitmap.  Not a pointer.
            // Not a key.
            if ((nBitsLeftState > cnBitsAtBottom)
                || ( ! ws_bNeedPrefixCheck(wState) )
                || (set_ws_bNeedPrefixCheck(wState, 0),
                    (pwr_wPrefix(pwr) == (wKey & ~(EXP(nBitsLeftRoot) - 1)))))
            {
                if (nBitsLeftState > cnBitsAtBottom) goto again;

                if (BitIsSet(pwRoot, wKey & (EXP(nBitsLeftState) - 1)))
                {
                    return KeyFound;
                }
            }
        }
    }
    else if (wRoot != 0)
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot
        int i;

        DBGX(printf("List\n"));

        for (i = 0; i < pwr_wPopCnt(pwr); i++)
        {
            if (pwr_pwKeys(pwr)[i] == wKey)
            {
                return KeyFound;
            }
        }
    }

#if defined(LOOKUP)
    return ! KeyFound;
#else // defined(LOOKUP)
    return InsertGuts(pwRoot, wKey, wState, wRoot);
#endif // defined(LOOKUP)

}

#undef InsertGuts
#undef Insert
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

