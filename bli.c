
#if defined(INSERT)
#undef KeyExisted
#define KeyExisted  (Failure)
#else // defined(INSERT)
#undef KeyExisted
#define KeyExisted  (Success)
#endif // defined(INSERT)

INLINE Status_t
#if        defined(INSERT)
Insert(Word_t *pwRoot, Word_t wKey, int nBitsLeft)
#else   // defined(INSERT)
Lookup(Word_t wRoot, Word_t wKey, int nBitsLeft)
#endif  // defined(INSERT)
{
#if defined(INSERT)
    Word_t wRoot = *pwRoot;
    Word_t *pw;
#else // defined(INSERT)
    Word_t *pwRoot;
#endif // defined(INSERT)

#if defined(INSERT)
    DBGI(printf("\n# Insert(pwRoot %p) ", pwRoot));
#else // defined(INSERT)
    DBGL(printf("\n# Lookup \n"));
#endif // defined(INSERT)

again:

#if defined(INSERT)
    DBGI(printf( "# pwRoot %p ", pwRoot));
#endif // defined(INSERT)
    DBG(printf("# wRoot "OWx" wKey "OWx" nBitsLeft %d\n",
            wRoot, wKey, nBitsLeft));

    if (wRoot != 0)
    {
        int nBitsPrefixSz = wr_nBitsPrefixSz(wRoot); // size of prefix
        Word_t wNodePrefix = wr_wPrefix(wRoot); // prefix of switch
        int nBitsIndexSz = wr_nBitsIndexSz(wRoot); // size of array index
        Word_t *pwPtrs = wr_pwPtrs(wRoot);
        Word_t wKeyPrefix;
        int nIndex;

        if (nBitsPrefixSz != 0) // shift by cnBitsPerWord doesn't work
        {
            nBitsLeft = cnBitsPerWord - nBitsPrefixSz;
DBG(printf("nBitsLeft %d\n", nBitsLeft));
        }
DBG(printf("nBitsPrefixSz %d\n", nBitsPrefixSz));
DBG(printf("nBitsIndexSz %d\n", nBitsIndexSz));

        nIndex = (wKey << nBitsPrefixSz) >> (cnBitsPerWord - nBitsIndexSz);

        wKeyPrefix = (nBitsLeft == cnBitsPerWord)
                        ? 0 : (wKey >> nBitsLeft) << nBitsLeft;
DBG(printf("wKeyPrefix "OWx"\n", wKeyPrefix));

        if (wKeyPrefix == wNodePrefix)
        {
DBG(printf("prefix match\n"));
            if (wr_nType(wRoot) == Switch)
            {
DBG(printf("switch pwPtrs %p nIndex %d & %p\n",
    pwPtrs, nIndex, &pwPtrs[nIndex]));
                pwRoot = &pwPtrs[nIndex];
                wRoot = *pwRoot;
                nBitsLeft -= nBitsIndexSz;

                goto again;
            }
DBG(printf("leaf\n"));

            assert(wr_nType(wRoot) == Leaf);

            if (wr_wKey(wRoot) == wKey) return KeyExisted;
        }
DBG(printf("prefix mismatch wNodePrefix "Owx" wKeyPrefix "Owx"\n",
    wNodePrefix, wKeyPrefix));

#if defined(INSERT)

        // prefix mismatch
        // insert a node at bit where prefix doesn't match
        nBitsLeft = LOG(wKeyPrefix ^ wNodePrefix) + 1; // below branch
        nBitsPrefixSz = cnBitsPerWord - nBitsLeft;

        pw = (Word_t *)JudyMalloc(cnPtrsOff + 2);
DBG(printf("new switch node pw %p\n", pw));
DBG(printf("nBitsPrefixSz %d\n", nBitsPrefixSz));
        set_wr_nType(pw, Switch);
        set_wr_nBitsPrefixSz(pw, nBitsPrefixSz);
        set_wr_wPrefix(pw, (wKey >> nBitsLeft) << nBitsLeft);
        nBitsIndexSz = 1;
        set_wr_nBitsIndexSz(pw, nBitsIndexSz); // Use zero for immediate?
        nIndex = wNodePrefix >> (nBitsLeft - nBitsIndexSz);
        wr_pwPtrs(pw)[nIndex] = wRoot;
        nIndex = wKey >> (nBitsLeft - nBitsIndexSz);
        wr_pwPtrs(pw)[nIndex] = 0;

DBG(printf("pw %p &pw %p\n", pw, &pw));
        Insert((Word_t *)&pw, wKey, nBitsLeft);

#endif // defined(INSERT)

    }
#if defined(INSERT)
    else
    {
DBG(printf("null\n"));

        // wRoot == 0 insert
        pw = (Word_t *)JudyMalloc(2);
DBG(printf("new leaf node pw %p\n", pw));
        set_wr_nType(pw, Leaf);
        set_wr_nBitsPrefixSz(pw, 32);
        set_wr_nBitsIndexSz(pw, 0);
        set_wr_wKey(pw, wKey);
    }

DBG(printf("installing pw %p pwRoot %p\n", pw, pwRoot));
    *pwRoot = (Word_t)pw; // install

#endif // defined(INSERT)

    return ! KeyExisted;
}

#undef KeyExisted

