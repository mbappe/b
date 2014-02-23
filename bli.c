
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
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Insert(Word_t *pwRoot, Word_t wKey, int nBitsLeft)
#endif // defined(LOOKUP)
{
#if defined(LOOKUP)
    int nBitsLeft = cnBitsPerWord;
    int bNeedPrefixCheck = 0;
    Word_t *pwRoot;
#else // defined(LOOKUP)
    Word_t wRoot = *pwRoot;
#endif // defined(LOOKUP)
    int nBitsLeftRoot;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

    assert(nBitsLeft > cnBitsAtBottom);

again:

#if ( ! defined(LOOKUP) )
    DBGX(printf("# pwRoot %p", pwRoot));
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nBitsLeft %d\n",
            wRoot, wKey, nBitsLeft));

    // Strange behavior can creep in if/when
    // cnBitsPerDigit is not a factor of cnBitsPerWord.
    assert(nBitsLeft <= cnBitsPerWord);

    if (wr_bIsSwitchBL(wRoot, nBitsLeftRoot))
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

        assert(pwr != 0);
        assert(wRoot != 0);

        DBGX(printf("Switch"));
        DBGX(printf(" nBitsLeft%d", nBitsLeft));
        DBGX(printf(" nBitsLeftRoot %d", nBitsLeftRoot));
        DBGX(printf(" pwr %p", pwr));
        DBGX(printf("\n"));

        assert(nBitsLeftRoot <= nBitsLeft); // reserved for later

#if defined(LOOKUP)
        if (nBitsLeftRoot != nBitsLeft)
        {
            // Record that there were prefix bits that were not checked.
            bNeedPrefixCheck = 1;
        }
        // !! there is no "else" here in the LOOKUP case !!
#else // defined(LOOKUP)
        if ((nBitsLeftRoot != nBitsLeft)
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

            nBitsLeft = nBitsLeftRoot - nBitsIndexSz;

            // In case nBitsLeft is not an integral number of digits.
            // Round it.
            // I'd like to get rid of this.
            // The first test should go away at compile time.
            if (cnBitsPerWord % cnBitsPerDigit != 0)
            {
                nBitsLeft = (nBitsLeft + nBitsIndexSz - 1)
                        / nBitsIndexSz * nBitsIndexSz;
            }

            nIndex = ((wKey >> nBitsLeft) & (EXP(nBitsIndexSz) - 1));

            DBGX(printf("Next"));
            //DBGX(printf(" nBitsIndexSz %d", nBitsIndexSz));
            DBGX(printf(" nBitsLeft%d", nBitsLeft));
            DBGX(printf(" nIndex %d", nIndex));
            DBGX(printf(" pwr %p pwRoots %p", pwr, pwr_pwRoots(pwr)));
            DBGX(printf("\n"));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            if (nBitsLeft > cnBitsAtBottom) goto again;

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a bitmap.  Not a pointer.
            // Not a key.

#if defined(LOOKUP)
            if (( ! bNeedPrefixCheck )
                || (pwr_wPrefix(pwr) == (wKey & ~(EXP(cnBitsAtBottom) - 1))))
#endif // defined(LOOKUP)
            {
                if (BitIsSet(pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)))
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
    return InsertGuts(pwRoot, wKey, nBitsLeft, wRoot);
#endif // defined(LOOKUP)

}

#undef InsertGuts
#undef Insert
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

