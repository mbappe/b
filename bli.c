
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
Insert(Word_t *pwRoot, Word_t wKey, int nDigitsLeft)
#endif // defined(LOOKUP)
{
#if defined(LOOKUP)
    int nDigitsLeft = cnDigitsPerWord;
    int bNeedPrefixCheck = 0;
    Word_t *pwRoot;
#else // defined(LOOKUP)
    Word_t wRoot = *pwRoot;
#endif // defined(LOOKUP)
    int nBitsLeft = nDigitsLeft * cnBitsPerDigit;
    int nDigitsLeftRoot;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

    assert(nDigitsLeft > cnDigitsAtBottom);

again:

#if ( ! defined(LOOKUP) )
    DBGX(printf("# pwRoot %p", pwRoot));
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (wr_bIsSwitchDL(wRoot, nDigitsLeftRoot))
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

        DBGX(printf("Switch nDigitsLeft %d nDigitsLeftRoot %d pwr %p\n",
            nDigitsLeft, nDigitsLeftRoot, pwr));

        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved

#if defined(LOOKUP)
        if (nDigitsLeftRoot < nDigitsLeft)
        {
            // Record that there were prefix bits that were not checked.
            bNeedPrefixCheck = 1;
        }
        // !! there is no "else" here in the LOOKUP case !!
#else // defined(LOOKUP)
        if ((nDigitsLeftRoot < nDigitsLeft)
            && (pwr_wPrefix(pwr)
                != (wKey & ~(EXP(nDigitsLeftRoot * cnBitsPerDigit) - 1))))
        {
            DBGX(printf("Prefix mismatch wPrefix "Owx"\n", pwr_wPrefix(pwr)));
        }
        else // !! the "else" here is only for the INSERT/REMOVE case !!
#endif // defined(LOOKUP)
        {
            // size of array index
            int nBitsIndexSz = pwr_nBitsIndexSz(pwr);
            int nIndex;

            nDigitsLeft = nDigitsLeftRoot - (nBitsIndexSz / cnBitsPerDigit);
            nBitsLeft = nDigitsLeft * cnBitsPerDigit;

            nIndex = ((wKey >> nBitsLeft) & (EXP(nBitsIndexSz) - 1));

            DBGX(printf("Next nDigitsLeft %d nIndex %d pwr %p pwRoots %p\n",
                nDigitsLeft, nIndex, pwr, pwr_pwRoots(pwr)));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            if (nDigitsLeft > cnDigitsAtBottom) goto again;

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a bitmap.  Not a pointer.
            // Not a key.

#if defined(LOOKUP)
            if (( ! bNeedPrefixCheck )
                || (pwr_wPrefix(pwr) == (wKey & ~(EXP(cnBitsAtBottom) - 1))))
#endif // defined(LOOKUP)
            {
                assert(cnBitsAtBottom <= cnBitsPerWord);
                if (BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)))
                {
                    return KeyFound;
                }
            }
        }
    }
    else if (wRoot != 0)
    {
        int i;

        DBGX(printf("List\n"));

        for (i = 0; i < wr_wPopCnt(wRoot); i++)
        {
            if (wr_pwKeys(wRoot)[i] == wKey)
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

