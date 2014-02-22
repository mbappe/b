
#if defined(LOOKUP) || defined(REMOVE)
#undef KeyFound
#define KeyFound  (Success)
#endif // defined(LOOKUP) || defined(REMOVE)

#if defined(INSERT)
#undef KeyFound
#define KeyFound  (Failure)
#endif // defined(INSERT)

INLINE Status_t
#if defined(LOOKUP)
Lookup(Word_t wRoot, Word_t wKey, Word_t wState)
#endif // defined(LOOKUP)
#if defined(INSERT)
Insert(Word_t *pwRoot, Word_t wKey, Word_t wState)
#endif // defined(INSERT)
#if defined(REMOVE)
Remove(Word_t *pwRoot, Word_t wKey, Word_t wState)
#endif // defined(REMOVE)
{
#if defined(LOOKUP)
    Word_t *pwRoot;
#else // defined(LOOKUP)
    Word_t wRoot = *pwRoot;
#endif // defined(LOOKUP)
    int nBitsLeftState = ws_nBitsLeft(wState);

#if defined(LOOKUP)
    DBGL(printf("\n# Lookup "));
#endif // defined(LOOKUP)
#if defined(INSERT)
    DBGI(printf("\n# Insert "));
#endif // defined(INSERT)
#if defined(REMOVE)
    DBGR(printf("\n# Remove "));
#endif // defined(REMOVE)

again:

#if defined(INSERT) || defined(REMOVE)
    DBG(printf("# pwRoot %p ", pwRoot));
#endif // defined(INSERT) || defined(REMOVE)
    DBG(printf("# wRoot "OWx" wKey "OWx" wState "OWx"\n",
            wRoot, wKey, wState));

    assert(ws_nBitsLeft(wState) <= cnBitsPerWord);

    if (nBitsLeftState <= cnBitsAtBottom)
    {
        assert( ! ws_bNeedPrefixCheck(wState) );

        if (BitIsSet(pwRoot, wKey & (EXP(nBitsLeftState) - 1)))
        {
            return KeyFound;
        }
    }
    else if (wr_bIsSwitch(wRoot))
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot

        assert(pwr != 0);
        assert(wRoot != 0);

        int nBitsLeftRoot = wr_nBitsLeft(wRoot);

        DBG(printf("Switch"));
        DBG(printf(" nBitsLeftState %d", nBitsLeftState));
        DBG(printf(" nBitsLeftRoot %d", nBitsLeftRoot));
        DBG(printf(" pwr %p", pwr));
        DBG(printf("\n"));

        assert(nBitsLeftRoot <= nBitsLeftState); // reserved for later

#if defined(LOOKUP)
        if (nBitsLeftRoot != nBitsLeftState)
        {
            // Record that there were prefix bits that were not checked.
            set_ws_bNeedPrefixCheck(wState, /* bNeedPrefixCheck */ 1);
        }
        // !! there is no "else" here in the LOOKUP case !!
#else // defined(LOOKUP)
        if ((nBitsLeftRoot != nBitsLeftState)
            && (pwr_wPrefix(pwr) != (wKey & ~(EXP(nBitsLeftRoot) - 1))))
        {
            DBG(printf("prefix mismatch wPrefix "Owx"\n",
                pwr_wPrefix(pwr)));
        }
        else // !! the "else" here is only for the INSERT/REMOVE case !!
#endif // defined(LOOKUP)
        {
            // size of array index
            int nBitsIndexSz = pwr_nBitsIndexSz(pwr);
            int nIndex;

            nBitsLeftState = nBitsLeftRoot - nBitsIndexSz;

            // In case nBitsLeftState is not an integral number of
            // digits.
            nBitsLeftState
                = (nBitsLeftState + nBitsIndexSz - 1)
                    / nBitsIndexSz * nBitsIndexSz;

            set_ws_nBitsLeft(wState, nBitsLeftState);

            nIndex = (wKey >> (nBitsLeftState))
                & (EXP(nBitsIndexSz) - 1);

            DBG(printf("Next"));
            //DBG(printf(" nBitsIndexSz %d", nBitsIndexSz));
            DBG(printf(" nBitsLeftState %d", nBitsLeftState));
            DBG(printf(" nIndex %d", nIndex));
            DBG(printf(" pwr %p pwRoots %p", pwr, pwr_pwRoots(pwr)));
            DBG(printf("\n"));

            pwRoot = &pwr_pwRoots(pwr)[nIndex];
            wRoot = *pwRoot;

            // We have to do the prefix check here if we're at the
            // bottom because wRoot contains a bitmap.  Not a pointer.
            // Not a key.
            if ((nBitsLeftState > cnBitsAtBottom)
                || ( ! ws_bNeedPrefixCheck(wState) )
                || (pwr_wPrefix(pwr)
                        == (wKey & ~(EXP(nBitsLeftRoot) - 1))))
            {
                set_ws_bNeedPrefixCheck(wState, /* bNeedPrefixCheck */ 0);

                goto again;
            }
        }
    }
    else if (wRoot != 0)
    {
        Word_t *pwr = wr_pwr(wRoot); // pointer extracted from wRoot
        int i;

#if defined(INSERT)
        DBG(printf("List\n"));
#endif // defined(INSERT)

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
#endif // defined(LOOKUP)

#if defined(INSERT)
    return InsertGuts(pwRoot, wKey, wState, wRoot);
#endif // defined(INSERT)

#if defined(REMOVE)
    // Will have to use a state bit to tell remove that we only want
    // to undo the pop count increments done by a failed insert and
    // we don't actually want to remove the key.
    return RemoveGuts(pwRoot, wKey, wState, wRoot);
#endif // defined(REMOVE)

}

#undef KeyFound

