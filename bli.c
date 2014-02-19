
#if        defined(INSERT)
#undef KeyExisted
#define KeyExisted  (Failure)
#else   // defined(INSERT)
#undef KeyExisted
#define KeyExisted  (Success)
#endif  // defined(INSERT)

INLINE Status_t
#if        defined(INSERT)
Insert(Word_t *pwRoot, Word_t wKey, int nDigitsLeft)
#else   // defined(INSERT)
Lookup(Word_t wRoot, Word_t wKey, int nDigitsLeft)
#endif  // defined(INSERT)
{
#if defined(INSERT)
    Word_t wRoot = *pwRoot;
#else // defined(INSERT)
    Word_t *pwRoot;
#endif // defined(INSERT)
    Word_t *pwLeaf;
    Word_t *pwKeys;
    Word_t wPopCnt;
    Word_t w;

again:

#if defined(INSERT)
    DBGI(printf(
        "\n# Insert(pwR 0x"_f0wx" wK 0x"_f0wx" nDL %d) wR 0x"_f0wx"\n",
            (Word_t)pwRoot, wKey, nDigitsLeft, wRoot));
#else // defined(INSERT)
    DBGL(printf("\n# Lookup(wR 0x"_f0wx" wK 0x"_f0wx")\n", wRoot, wKey));
#endif // defined(INSERT)

    if (wRoot != 0)
    {
        if (wr_dl(wRoot) != mm)
        {
            Node_t *pNd = wr_pn(wRoot);
            Word_t wDigit = ((wKey >> ((nDigitsLeft - 1) * bpd)) % EXP(bpd));
            pwRoot = &pNd->nd_awRoot[wDigit];
            wRoot = *pwRoot;
            nDigitsLeft--;

            goto again;
        }

        // list leaf with population count word at the beginning

        pwLeaf = wr_pw(wRoot);
        pwKeys = lf_pwKeys(pwLeaf);
        wPopCnt = lf_cnt(pwLeaf);

        // look for the key in the list
        for (w = 0; w < wPopCnt; w++)
        {
            if (pwKeys[w] == wKey)
            {
                return KeyExisted; // found the key
            }
        }
    }

#if defined(INSERT)
    InsertInLeaf(pwRoot, wKey, nDigitsLeft);
#endif // defined(INSERT)

    return ! KeyExisted;
}

#undef KeyExisted

