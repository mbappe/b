
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
    int nBitsPrefixSz;
    Word_t wNodePrefix;
    int nBitsIndexSz;
    Word_t *pwPtrs;
    Word_t wKeyPrefix;
    int nIndex;

#if defined(INSERT)
    DBGI(printf("\n# Insert "));
#else // defined(INSERT)
    DBGL(printf("\n# Lookup "));
#endif // defined(INSERT)

again:

#if defined(INSERT)
    DBGI(printf( "# pwRoot %p ", pwRoot));
#endif // defined(INSERT)
    DBGI(printf("# wRoot "OWx" wKey "OWx" nBitsLeft %d\n",
            wRoot, wKey, nBitsLeft));

    if (wRoot != 0)
    {
        nBitsPrefixSz = wr_nBitsPrefixSz(wRoot); // size of prefix
        wNodePrefix = wr_wPrefix(wRoot); // prefix of switch
        nBitsIndexSz = wr_nBitsIndexSz(wRoot); // size of array index
        pwPtrs = wr_pwPtrs(wRoot);

        if (wr_nType(wRoot) == Switch)
        {
            DBGI(printf("switch\n"));
            DBGI(printf("nBitsPrefixSz %d\n", nBitsPrefixSz));

            if (nBitsPrefixSz != 0) // shift by cnBitsPerWord doesn't work
            {
                nBitsLeft = cnBitsPerWord - nBitsPrefixSz;
                DBGI(printf("nBitsLeft %d\n", nBitsLeft));
            }
            DBGI(printf("nBitsIndexSz %d\n", nBitsIndexSz));

            wKeyPrefix = (nBitsLeft == cnBitsPerWord)
                        ? 0 : (wKey >> nBitsLeft) << nBitsLeft;
            DBGI(printf("wKeyPrefix "OWx"\n", wKeyPrefix));

            nIndex = (wKey << nBitsPrefixSz) >> (cnBitsPerWord - nBitsIndexSz);
            DBGI(printf(" nIndex %d &pwPtrs[nIndex] %p\n",
                nIndex, &pwPtrs[nIndex]));
            
            if (wKeyPrefix == wNodePrefix)
            {
                DBGI(printf("prefix match\n"));
                pwRoot = &pwPtrs[nIndex];
                wRoot = *pwRoot;
                nBitsLeft -= nBitsIndexSz;

                goto again;
            }

            assert(wr_nType(wRoot) == Leaf);
            DBGI(printf(
                "prefix mismatch wNodePrefix "Owx" wKeyPrefix "Owx"\n",
                    wNodePrefix, wKeyPrefix));
        }
        else
        {
            DBGI(printf("leaf\n"));
            if (wr_wKey(wRoot) == wKey) return KeyExisted;
            DBGI(printf("leaf mismatch wr_wKey "Owx" wKey "Owx"\n",
                wr_wKey(wRoot), wKey));
        }

    }

#if defined(INSERT)

    if (wRoot != 0)
    {
        // prefix (or key) mismatch
        // insert a node at bit where prefix doesn't match
        nBitsLeft = LOG(wKey ^ wNodePrefix) + 1; // below branch
        nBitsPrefixSz = cnBitsPerWord - nBitsLeft;

        pw = (Word_t *)JudyMalloc(cnPtrsOff + 2);
        DBGI(printf("new switch node pw %p\n", pw));
        DBGI(printf("nBitsPrefixSz %d\n", nBitsPrefixSz));
        DBGI(printf("nBitsLeft %d\n", nBitsLeft));
        set_wr_nType(pw, Switch);
        set_wr_nBitsPrefixSz(pw, nBitsPrefixSz);
        set_wr_wPrefix(pw,
            (nBitsLeft == cnBitsPerWord)
                ? 0 : (wKey >> nBitsLeft) << nBitsLeft);
        DBGI(printf("wPrefix "Owx"\n", wr_wPrefix(pw)));
        nBitsIndexSz = 1;
        set_wr_nBitsIndexSz(pw, nBitsIndexSz); // Use zero for immediate?
        nIndex = (wNodePrefix << nBitsPrefixSz)
                    >> (cnBitsPerWord - nBitsIndexSz);
        DBGI(printf("old node nIndex %d\n", nIndex));
        wr_pwPtrs(pw)[nIndex] = wRoot;
        DBGI(printf("install old node at "Owx"\n",
            (Word_t)&wr_pwPtrs(pw)[nIndex]));
        nIndex = (wKey << nBitsPrefixSz) >> (cnBitsPerWord - nBitsIndexSz);
        DBGI(printf("new key nIndex %d\n", nIndex));
        wr_pwPtrs(pw)[nIndex] = 0;

        DBGI(printf("pw %p &pw %p\n", pw, &pw));
        Insert((Word_t *)&pw, wKey, nBitsLeft);
    }
    else
    {
        DBGI(printf("null\n"));

        // wRoot == 0 insert
        pw = (Word_t *)JudyMalloc(2);
        DBGI(printf("new leaf node pw %p\n", pw));
        set_wr_nType(pw, Leaf);
        set_wr_nBitsPrefixSz(pw, 0);
        set_wr_nBitsIndexSz(pw, 0);
        set_wr_wKey(pw, wKey);
    }

    DBGI(printf("installing pw %p pwRoot %p\n", pw, pwRoot));
    *pwRoot = (Word_t)pw; // install

#endif // defined(INSERT)

    return ! KeyExisted;
}

#undef KeyExisted

