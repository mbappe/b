
#if defined(INSERT)
#undef KeyFound
#define KeyFound  (Failure)
#else // defined(INSERT)
#undef KeyFound
#define KeyFound  (Success)
#endif // defined(INSERT)

INLINE Status_t
#if        defined(INSERT)
Insert(Word_t *pwRoot, Word_t wKey)
#else   // defined(INSERT)
Lookup(Word_t wRoot, Word_t wKey)
#endif  // defined(INSERT)
{
#if defined(INSERT)
    Word_t wRoot = *pwRoot;
#else // defined(INSERT)
    Word_t *pwRoot;
#endif // defined(INSERT)

#if defined(INSERT)
    DBGI(printf("\n# Insert "));
#else // defined(INSERT)
    DBGL(printf("\n# Lookup "));
#endif // defined(INSERT)

again:

#if defined(INSERT)
    DBGI(printf( "# pwRoot %p ", pwRoot));
#endif // defined(INSERT)
    DBGI(printf("# wRoot "OWx" wKey "OWx"\n", wRoot, wKey));

    if (wRoot != 0)
    {
        if (wr_nType(wRoot) == Switch)
        {
            int nBitsLeft = wr_nBitsLeft(wRoot); // size of prefix

            DBGI(printf("switch nBitsLeft %d\n", nBitsLeft));

            {
#if defined(INSERT)
                Word_t wKeyPrefix;

                // shifting by word size is a problem
                wKeyPrefix = ((wKey >> 1) >> (nBitsLeft - 1));
                wKeyPrefix = ((wKeyPrefix << (nBitsLeft - 1)) << 1);
                DBGI(printf("wKeyPrefix "OWx"\n", wKeyPrefix));

                if (wKeyPrefix != wr_wPrefix(wRoot))
                {
                    DBGI(printf(
                        "prefix mismatch wr_wPrefix "Owx" wKeyPrefix "Owx"\n",
                            wr_wPrefix(wRoot), wKeyPrefix));
                }
                else
#endif // defined(INSERT)
                {
                    // size of array index
                    int nBitsIndexSz = wr_nBitsIndexSz(wRoot);
                    int nIndex = (wKey >> (nBitsLeft - nBitsIndexSz))
                                                & (EXP(nBitsIndexSz) - 1);
                    DBGI(printf("prefix match\n"));
                    DBGI(printf("nBitsIndexSz %d\n", nBitsIndexSz));
                    DBGI(printf("nIndex %d\n", nIndex));
                    pwRoot = &wr_pwPtrs(wRoot)[nIndex];
                    wRoot = *pwRoot;

                    goto again;
                }
            }
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
    return InsertAt(pwRoot, wKey, wRoot);
#else // defined(INSERT)
    return ! KeyFound;
#endif // defined(INSERT)
}

#undef KeyExisted

