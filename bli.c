
#if defined(INSERT)
#undef KeyFound
#define KeyFound  (Failure)
#else // defined(INSERT)
#undef KeyFound
#define KeyFound  (Success)
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
            if (wr_wKey(wRoot) == wKey) return KeyFound;
            DBGI(printf("leaf mismatch wr_wKey "Owx" wKey "Owx"\n",
                wr_wKey(wRoot), wKey));
        }

    }

#if defined(INSERT)
    return InsertAt(pwRoot, wKey, nBitsLeft, wRoot);
#else // defined(INSERT)
    return ! KeyFound;
#endif // defined(INSERT)
}

#undef KeyExisted

