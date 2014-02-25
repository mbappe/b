
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
#define KeyFound  (assert(0), Failure)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#endif // defined(LOOKUP) || defined(REMOVE)

INLINE Status_t
#if defined(LOOKUP)
Lookup(Word_t wRoot, Word_t wKey)
#else // defined(LOOKUP)
Insert(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft)
#endif // defined(LOOKUP)
{
#if defined(LOOKUP)
    unsigned nDigitsLeft = cnDigitsPerWord;
#if defined(SKIP_PREFIX_CHECK)
    unsigned bNeedPrefixCheck = 0;
#endif // defined(SKIP_PREFIX_CHECK)
    Word_t *pwRoot;
#else // defined(LOOKUP)
    Word_t wRoot = *pwRoot;
#endif // defined(LOOKUP)
    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
    unsigned nDigitsLeftRoot;
    unsigned nType;

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

    assert(nDigitsLeft > cnDigitsAtBottom);

again:

#if ( ! defined(LOOKUP) )
    DBGX(printf("# pwRoot %p", pwRoot));
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if ((nType = wr_nType(wRoot)) != List)
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

        DBGX(printf("Switch nDigitsLeft %d nDigitsLeftRoot %d pwr %p\n",
            nDigitsLeft, nDigitsLeftRoot, pwr));

        assert(nDigitsLeftRoot <= nDigitsLeft); // reserved

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
        {
            // size of array index
            unsigned nBitsIndexSz = pwr_nBitsIndexSz(pwr);
            unsigned nIndex;

#if defined(INSERT)
            {
                Word_t wPopCnt = sw_wPopCnt(pwr, nDigitsLeftRoot);
#if 0
                // increment population count on the way in
                if (wPopCnt == EXP(nDigitsLeftRoot * cnBitsPerDigit))
                {
                    // subtree is at full population
                    return KeyFound;
                }
#endif
                set_sw_wPopCnt(pwr, nDigitsLeftRoot, wPopCnt + 1);
            }
#endif // defined(INSERT)

            nDigitsLeft = nDigitsLeftRoot - (nBitsIndexSz / cnBitsPerDigit);
            nBitsLeft = nDigitsLeft * cnBitsPerDigit;

            nIndex = ((wKey >> nBitsLeft) & (EXP(nBitsIndexSz) - 1));

            DBGX(printf("Next nDigitsLeft %d nIndex %d pwr %p pwRoots %p\n",
                nDigitsLeft, nIndex, pwr, pwr_pwRoots(pwr)));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            DBGX(printf("pwRoot %p wRoot "OWx"\n", pwRoot, wRoot));

            if (nDigitsLeft > cnDigitsAtBottom) goto again;

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a bitmap.  Not a pointer.
            // Not a key.

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            if (( ! bNeedPrefixCheck )
                || (LOG(1 | (sw_wPrefixNotAtTop(pwr, nDigitsLeftRoot) ^ wKey))
                    < (cnBitsAtBottom + nBitsIndexSz)))
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            {
                if (cnBitsAtBottom <= cnLogBitsPerWord) // compile time
                {
                    DBGX(printf(
                        "BitIsSetInWord(wRoot "OWx" wKey "OWx")\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

                    if (BitIsSetInWord(wRoot,
                        wKey & (EXP(cnBitsAtBottom) - 1)))
                    {
                        return KeyFound;
                    }

                    DBGX(printf("! BitIsSetInWord\n"));
                }
                else if (wRoot != 0)
                {
                    DBGX(printf(
                        "BitIsSet(wRoot "OWx" wKey "OWx")\n",
                            wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

                    if (BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)))
                    {
                        return KeyFound;
                    }

                    DBGX(printf("! BitIsSet\n"));
                }
            }
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
            else
            {
                DBGX(printf("Prefix mismatch at bitmap wPrefix "OWx"\n",
                    sw_wPrefix(pwr, nDigitsLeftRoot)));
            }
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        }
    }
    else if (wRoot != 0)
    {
#if 0
#if defined(LOOKUP)
#if 0
        Word_t *pwKeys = wr_pwKeys(wRoot);
        if (pwKeys[0] != 0) return KeyFound;
#else
        return KeyFound;
#endif
#else // defined(LOOKUP)
#endif // defined(LOOKUP)
#endif
        unsigned i;

        DBGX(printf("List\n"));

        // todo: save insertion point in sorted list and pass it to InsertGuts
        // todo: possibly do insertion right here if list isn't full
        for (i = 0; i < ls_wPopCnt(wr_pwr(wRoot)); i++)
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
    return InsertGuts(pwRoot, wKey, nDigitsLeft, wRoot);
#endif // defined(LOOKUP)

}

#undef InsertGuts
#undef Insert
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

