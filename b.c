
// Todo:
//
// "pwr" is a confusing name.  We use it for pointer extracted from wRoot.
// Do not confuse it with pointer to wRoot.  Even though they will be the
// same when nType is zero.

#include "b.h"

#if defined(RAM_METRICS)
Word_t j__AllocWordsJBU;  // JUDYA
Word_t j__AllocWordsJLB1; // JUDYA
Word_t j__AllocWordsJLLW; // JUDYA  JUDYB
Word_t j__AllocWordsJBU4; //        JUDYB
Word_t j__AllocWordsJV12; //        JUDYB
#endif // defined(RAM_METRICS)

// From Judy1LHTime.c for convenience.

#if 0

Word_t    j__SearchCompares;            // number times LGet/1Test called
Word_t    j__SearchPopulation;          // Population of Searched object
Word_t    j__TreeDepth;                 // number time Branch_U called

#ifdef  JUDYA
Word_t    j__AllocWordsJBB;
Word_t    j__AllocWordsJBU;
Word_t    j__AllocWordsJBL;
Word_t    j__AllocWordsJLB1;
Word_t    j__AllocWordsJLL1;
Word_t    j__AllocWordsJLL2;
Word_t    j__AllocWordsJLL3;

Word_t    j__AllocWordsJLL4;
Word_t    j__AllocWordsJLL5;
Word_t    j__AllocWordsJLL6;
Word_t    j__AllocWordsJLL7;
#endif  // JUDYA  

#ifdef  JUDYB
Word_t    j__AllocWordsJBU4;
Word_t    j__AllocWordsJBU8;
Word_t    j__AllocWordsJBU16;
Word_t    j__AllocWordsJV12;
Word_t    j__AllocWordsJL12;
Word_t    j__AllocWordsJL16;
Word_t    j__AllocWordsJL32;
#endif  // JUDYB   

Word_t    j__AllocWordsJLLW;

#ifdef  JUDYA
Word_t    j__AllocWordsJV;
#endif  // JUDYA 

#endif // 0

#if cnBitsPerDigit != 0

static Word_t *
NewList(Word_t wPopCnt, unsigned nDigitsLeft, Word_t wKey)
{
    DBGM(printf("NewList wPopCnt "OWx"\n", wPopCnt));

    unsigned nWords = sizeof(LeafWord_t) / sizeof(Word_t) + wPopCnt;

    DBGM(printf("NewList nWords %d\n", nWords));

    Word_t *pwList = (Word_t *)JudyMalloc(nWords);

    (void)nDigitsLeft;
    (void)wKey;

    DBGM(printf("NewList pwList %p wPopCnt "OWx" nWords %d\n",
        pwList, wPopCnt, nWords));

    set_ls_wPopCnt(pwList, wPopCnt);
    set_ls_wLen(pwList, nWords);

    METRICS(j__AllocWordsJLLW += (ls_wLen(pwList)));

    return pwList;
}

static Word_t
OldList(Word_t *pwList)
{
    Word_t wLen = ls_wLen(pwList);

    DBGM(printf("Old pwList %p wLen "OWx" wPopCnt "OWx"\n",
        pwList, wLen, ls_wPopCnt(pwList)));

    METRICS(j__AllocWordsJLLW -= (ls_wLen(pwList)));

    JudyFree(pwList, wLen);

    return wLen * sizeof(Word_t);
}

static Word_t
NewBitmap(void)
{
    Word_t w = JudyMalloc(EXP(cnBitsAtBottom) / cnBitsPerWord);

    METRICS(j__AllocWordsJLB1 += (EXP(cnBitsAtBottom) / cnBitsPerWord));

    DBGM(printf("NewBitmap nBitsAtBottom %u nBits "OWx
      " nBytes "OWx" nWords "OWx" w "OWx"\n",
        cnBitsAtBottom, EXP(cnBitsAtBottom),
        EXP(cnBitsAtBottom) / cnBitsPerByte,
        EXP(cnBitsAtBottom) / cnBitsPerWord, w));

    if (w == 0) { fprintf(stderr, "NewBitmap malloc.\n"); exit(1); }

    memset((void *)w, 0, EXP(cnBitsAtBottom) / cnBitsPerByte);

    return w;
}

Word_t
OldBitmap(Word_t wRoot)
{
    JudyFree((Word_t *)wRoot, EXP(cnBitsAtBottom) / cnBitsPerWord);

    METRICS(j__AllocWordsJLB1 -= (EXP(cnBitsAtBottom) / cnBitsPerWord));

    return EXP(cnBitsAtBottom) / cnBitsPerWord * sizeof(Word_t);
}

static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft,
          unsigned nDigitsLeftUp)
{
    Word_t *pwr;

    // wKey and nDigitsLeft are provided in case we decide to initialize
    // prefix here.
    // But we don't have enough info to implement NO_UNNECESSARY_PREFIX here.
    (void)wKey; // fix "unused parameter" compiler warning
    (void)nDigitsLeft; // nDigitsLeft is not used for all ifdef combos
    (void)nDigitsLeftUp; // nDigitsLeftUp is not used for all ifdef combos

    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);

    pwr = (Word_t *)JudyMalloc(sizeof(Switch_t) / sizeof(Word_t));
    assert(pwr != NULL);

#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(Switch_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 += sizeof(Switch_t) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  += sizeof(Switch_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 += sizeof(Switch_t) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx" nDigitsLeft %d) pwr %p\n",
        pwRoot, wKey, nDigitsLeft, pwr));

    set_wr(*pwRoot, pwr, nDigitsLeft_to_tp(nDigitsLeft));

    memset(pwr_pLinks(pwr), 0, sizeof(pwr_pLinks(pwr)));

#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
    if (nDigitsLeftUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
        memset(pwR_pwBm(pwRoot), -1,
               DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord)
                   * cnBytesPerWord);
    }
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)

    return pwr;
}

static Word_t
#if defined(RAM_METRICS)
OldSwitch(Switch_t *pSw, unsigned nDigitsLeft)
#else // defined(RAM_METRICS)
OldSwitch(Switch_t *pSw)
#endif // defined(RAM_METRICS)
{
#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 -= sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 -= sizeof(*pSw) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  -= sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 -= sizeof(*pSw) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    JudyFree((Word_t *)pSw, sizeof(*pSw) / sizeof(Word_t));

    return sizeof(*pSw);
}

static Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, unsigned nBitsLeft,
              unsigned nBitsLeftUp, int bDump)
{
    Word_t wRoot = *pwRoot;
    unsigned nDigitsLeft;
    Word_t *pwr;
    unsigned nBitsIndexSz;
    Link_t *pLinks;
    unsigned nType;
    unsigned n;
    Word_t wBytes = 0;

    if (wRoot == 0)
    {
        return 0;
    }

    if (bDump)
    {
        printf(" nBitsLeft %2d", nBitsLeft);
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefix "OWx, wPrefix);
        printf(" wr "OWx, wRoot);
    }

    if (nBitsLeft <= cnBitsAtBottom)
    {
        if (cnBitsAtBottom > cnLogBitsPerWord)
        {
            if (!bDump)
            {
                return OldBitmap(wRoot);
            }

            for (n = 0;
                (n < EXP(cnBitsAtBottom) / cnBitsPerWord) && (n < 8);
                 n++)
            {
                printf(" "Owx, ((Word_t *)wRoot)[n]);
            }
        }

        if (bDump)
        {
            printf("\n");
        }

        return 0;
    }

    nType = wr_nType(wRoot);

    pwr = wr_tp_pwr(wRoot, nType);

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt = ls_wPopCnt(pwr);
        Word_t *pwKeys = pwr_pwKeys(pwr);

        assert(nType == 0);

        if (!bDump)
        {
            return OldList(pwr);
        }

        printf(" wLen %3llu", (unsigned long long)ls_wLen(wRoot));
        printf(" wPopCnt %3llu", (unsigned long long)wPopCnt);

        for (n = 0; (n < wPopCnt) && (n < 8); n++)
        {
#if defined(COMPRESSED_LISTS)
            if (nBitsLeft <= 8) {
                printf(" %02x", ls_pcKeys(wRoot)[n]);
            } else if (nBitsLeft <= 16) {
                printf(" %04x", ls_psKeys(wRoot)[n]);
            } else
#endif // defined(COMPRESSED_LISTS)
            { printf(" "Owx, pwKeys[n]); }
        }
        printf("\n");

        return 0;
    }

    // Switch

    nDigitsLeft = tp_to_nDigitsLeft(nType);

    if ((nBitsLeft = nDigitsLeft * cnBitsPerDigit) > cnBitsPerWord)
    {
        nBitsLeft = cnBitsPerWord;
    }

    wPrefix = pwR_wPrefix(pwRoot, nDigitsLeft);
    nBitsIndexSz = pwr_nBitsIndexSz(pwr);
    pLinks = pwr_pLinks(pwr);

    if (bDump)
    {
        printf(" wPopCnt %3llu",
            (unsigned long long)pwR_wPopCnt(pwRoot, nDigitsLeft));
        printf(" wr_nDigitsLeft %2d", nDigitsLeft);
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefixPop "OWx, pwR_wPrefixPop(pwRoot));
        printf(" wKeyPopMask "OWx, wPrefixPopMask(nDigitsLeft));
        printf(" wr_wPrefix "OWx, wPrefix);
        //printf(" pLinks "OWx, (Word_t)pLinks);
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if (nBitsLeftUp < cnBitsPerWord)
#endif // defined(BM_IN_LINK)
        {
            printf(" Bm");
            for (unsigned nn = 0;
                          nn < DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord);
                          nn ++)
            {
                printf(" "OWx, pwR_pwBm(pwRoot)[nn]);
            }
        }
#endif // defined(BM_SWITCH)
        printf("\n");
    }

    nBitsLeftUp = nBitsLeft;
    nBitsLeft -= nBitsIndexSz;
    // In case nBitsLeftState is not an integral number of digits.
    if (cnBitsPerWord % cnBitsPerDigit != 0)
    {
        nBitsLeft = (nBitsLeft + nBitsIndexSz - 1)
            / nBitsIndexSz * nBitsIndexSz;
    }

    for (n = 0; n < EXP(nBitsIndexSz); n++)
    {
        wBytes += FreeArrayGuts(&pLinks[n].ln_wRoot,
            wPrefix | (n << nBitsLeft), nBitsLeft, nBitsLeftUp, bDump);
    }

#if defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr, nDigitsLeft) + wBytes);
#else // defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr) + wBytes);
#endif // defined(RAM_METRICS)
}

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, unsigned nBitsLeft, unsigned nBitsLeftUp)
{
    FreeArrayGuts(&wRoot, wPrefix, nBitsLeft, nBitsLeftUp, /* bDump */ 1);
}
#endif // defined(DEBUG)

#if defined(SORT_LISTS)
// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsertWord(Word_t *pTgt, Word_t *pSrc, unsigned nKeys, Word_t wKey)
{
#if (cwListPopCntMax != 0)
    Word_t aw[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    Word_t aw[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&aw[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = aw;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}

#if defined(COMPRESSED_LISTS)
static void
CopyWithInsertShort(unsigned short *pTgt, unsigned short *pSrc,
    unsigned nKeys, unsigned short wKey)
{
#if (cwListPopCntMax != 0)
    unsigned short as[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned short as[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&as[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = as;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}

static void
CopyWithInsertChar(unsigned char *pTgt, unsigned char *pSrc,
    unsigned nKeys, unsigned short wKey)
{
#if (cwListPopCntMax != 0)
    unsigned char ac[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned char ac[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&ac[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = ac;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}
#endif // defined(COMPRESSED_LISTS)
#endif // defined(SORT_LISTS)

Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    Word_t *pwr;
    Word_t *pwSw;
    unsigned nType;

    // Validate global constant parameters set up in the header file.
#if 0
    assert(cnDigitsAtBottom > 0); // can't get to full pop
#endif
    assert(cnDigitsAtBottom + 1 <= cnDigitsPerWord);
#if defined(SKIP_LINKS)
    assert(cnDigitsAtBottom + cnMallocMask >= cnDigitsPerWord + 1);
#endif // defined(SKIP_LINKS)

    DBGI(printf("InsertGuts pwRoot %p ", pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        if (cnBitsAtBottom <= cnLogBitsPerWord) // compile time
        {
            assert(!BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSetInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSet(&wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSet(pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
                *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

            assert(BitIsSet(pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(BitIsSetInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
        }
        else
        {
            if (wRoot == 0)
            {
                *pwRoot = wRoot = NewBitmap();
            }

            assert(!BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            DBGI(printf("SetBit(wRoot "OWx" wKey "OWx") pwRoot %p\n",
                wRoot, wKey & (EXP(cnBitsAtBottom) - 1), pwRoot));

            SetBit(wRoot, wKey & (EXP(cnBitsAtBottom) - 1));

            assert(BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
        }

        return Success;
    }

    nType = wr_nType(wRoot);

    pwr = wr_tp_pwr(wRoot, nType);

#if defined(SKIP_LINKS)
    if (!tp_bIsSwitch(nType))
#else // defined(SKIP_LINKS)
    assert(nType == 0);
#endif // defined(SKIP_LINKS)
    {
        Word_t wPopCnt;
        Word_t *pwKeys;
#if defined(COMPRESSED_LISTS)
        unsigned short *psKeys;
        unsigned char *pcKeys;
#endif // defined(COMPRESSED_LISTS)

        if (pwr != NULL) // pointer to old List
        {
            wPopCnt = ls_wPopCnt(pwr);
            pwKeys = ls_pwKeys(pwr); // list of keys in old List
#if defined(COMPRESSED_LISTS)
            psKeys = ls_psKeys(pwr);
            pcKeys = ls_pcKeys(pwr);
#endif // defined(COMPRESSED_LISTS)
        }
        else
        {
            wPopCnt = 0;
            // make compiler happy about uninitialized variable
            // it doesn't recognize that (wPopCnt == 0) ==> pwKeys will not
            // be examined
            pwKeys = NULL;
#if defined(COMPRESSED_LISTS)
            psKeys = NULL;
            pcKeys = NULL;
#endif // defined(COMPRESSED_LISTS)
        }

// We don't support skip links to lists or bitmaps yet.  And don't have
// any criteria yet for converting from a list to a switch other than the
// list is full.  So we just add to an existing list or create a new one
// here.  But this is where we might make a decision to use a skip link
// to a list or a bitmap or convert a list to a switch or convert the
// other way or create a bitmap switch or ...
// We need a new type field the destination of a link with type zero.
//
//  - bitmap leaf -- depth, prefix and pop (depth implies width), bits
//  - list leaf -- depth, prefix, pop, capacity, key size, keys
//  - bitmap switch -- depth, prefix, pop, capacity, bitmap, links
//  - list switch -- depth, prefix, pop, capacity, (key, link) pairs

        if (wPopCnt < cwListPopCntMax)
        {
            // allocate a new list and init pop count in the first word
            Word_t *pwList = NewList(wPopCnt + 1, nDigitsLeft, wKey);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
                if (nBitsLeft <= 8) {
                    CopyWithInsertChar(ls_pcKeys(pwList),
                        (unsigned char *)pwKeys, wPopCnt,
                        (unsigned char)wKey);
                } else if (nBitsLeft <= 16) {
                    CopyWithInsertShort(ls_psKeys(pwList),
                        (unsigned short *)pwKeys, wPopCnt,
                        (unsigned short)wKey);
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    CopyWithInsertWord(ls_pwKeys(pwList),
                        pwKeys, wPopCnt, wKey);
                }
            } else
#elif defined(MIN_MAX_LISTS)
            {
                COPY(ls_pwKeys(pwList), pwKeys, wPopCnt);

                if (wKey < pwKeys[wPopCnt - 1])
                {
                    ls_pwKeys(pwList)[wPopCnt] = pwKeys[wPopCnt - 1];

                    if (wKey < pwKeys[0])
                    {
                        ls_pwKeys(pwList)[wPopCnt - 1] = pwKeys[0];
                        ls_pwKeys(pwList)[0] = wKey;
                    }
                    else
                    {
                        ls_pwKeys(pwList)[wPopCnt - 1] = wKey;
                    }
                }
                else
                {
                    ls_pwKeys(pwList)[wPopCnt] = wKey;
                }
            } else
#else // defined(MIN_MAX_LISTS)
            { COPY(ls_pwKeys(pwList), pwKeys, wPopCnt); }
#endif // defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
                if (nBitsLeft <= 8) {
                    ls_pcKeys(pwList)[wPopCnt] = wKey;
                } else if (nBitsLeft <= 16) {
                    ls_psKeys(pwList)[wPopCnt] = wKey;
                } else
#endif // defined(COMPRESSED_LISTS)
                { ls_pwKeys(pwList)[wPopCnt] = wKey; }
            }

            set_wr(wRoot, pwList, /* nType */ 0); // !tp_bIsSwitch

            *pwRoot = wRoot; // install new
        }
        else
        {
#if defined(SKIP_LINKS)
#if defined(NO_UNNECESSARY_PREFIX) || defined(COMPRESSED_LISTS) \
    || !defined(NDEBUG)
            unsigned nDigitsLeftOld = nDigitsLeft;
#endif // defined(NO_UNNECESSARY_PREFIX) || defined(COMPRESSED_LISTS) || ...
#endif // defined(SKIP_LINKS)
            Word_t w;

            // List is full; insert a switch

#if defined(SKIP_LINKS)
            if (cwListPopCntMax != 0) // use const for compile time check
            {
                Word_t wMax, wMin;
#if defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
                Word_t wSuffix;
                if (nBitsLeft <= 8) {
                    wMin = ls_pcKeys(wRoot)[0];
                    wMax = ls_pcKeys(wRoot)[wPopCnt - 1];
                    wSuffix = wKey & 0xff;
                } else if (nBitsLeft <= 16) {
                    wMin = ls_psKeys(wRoot)[0];
                    wMax = ls_psKeys(wRoot)[wPopCnt - 1];
                    wSuffix = wKey & 0xffff;
                } else 
#endif // defined(COMPRESSED_LISTS)
                { wMin = pwKeys[0]; wMax = pwKeys[wPopCnt - 1]; }
#else // defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
                // walk the list to find max and min
                wMin = (Word_t)-1;
                wMax = 0;

                for (w = 0; w < wPopCnt; w++)
                {
                    if (pwKeys[w] < wMin) wMin = pwKeys[w];
                    if (pwKeys[w] > wMax) wMax = pwKeys[w];
                }
#endif // defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
                DBGI(printf("wMin "OWx" wMax "OWx"\n", wMin, wMax));

#if defined(COMPRESSED_LISTS)
                if (nBitsLeft <= 16)
                {
                    nDigitsLeft
                        = LOG(1 | ((wSuffix ^ wMin) | (wSuffix ^ wMax)))
                            / cnBitsPerDigit + 1;
                }
                else
#endif // defined(COMPRESSED_LISTS)
                {
                    nDigitsLeft
                        = LOG(1 | ((wKey ^ wMin) | (wKey ^ wMax)))
                            / cnBitsPerDigit + 1;
                }
            }
            else
            {
                // can't dereference list if there isn't one
                nDigitsLeft = cnDigitsAtBottom + 1; // go directly to Bitmap
            }

            // We don't create a switch below cnDigitsAtBottom + 1.
            // Why?  Because we've defined cnDigitsAtBottom as automatic
            // bitmap (no switch) and we may need a prefix at
            // cnDigitsAtBottom + 1 since we don't have one in the bitmap.
            if (nDigitsLeft <= cnDigitsAtBottom)
            {
                DBGI(printf("InsertGuts nDigitsLeft <= cnDigitsAtBottom\n"));

                nDigitsLeft = cnDigitsAtBottom + 1;
            }
#else // defined(SKIP_LINKS)
            assert(nDigitsLeft > cnDigitsAtBottom);
#endif // defined(SKIP_LINKS)

            pwSw = NewSwitch(pwRoot, wKey, nDigitsLeft, nDigitsLeftOld);

            //set_pwr_wPopCnt(pwSw, nDigitsLeft, 0);
            set_pwR_wPopCnt(pwRoot, nDigitsLeft, 0);

#if defined(SKIP_LINKS)
            assert(nDigitsLeft <= nDigitsLeftOld);
#if defined(NO_UNNECESSARY_PREFIX)
            // We could get rid of the bottom check if we enhance Insert
            // to keep track of any prefix checks done along the way and
            // pass that info to InsertGuts.
            if ((nDigitsLeft == nDigitsLeftOld)
                && (nDigitsLeft > cnDigitsAtBottom + 1))
            {
                DBGI(printf(
                  "Not installing prefix left %d old %d wKey "OWx"\n",
                    nDigitsLeft, nDigitsLeftOld, wKey));

                //set_pwr_wPrefix(pwSw, nDigitsLeft, 0);
                set_pwR_wPrefix(pwRoot, nDigitsLeft, 0);
            }
            else
#endif // defined(NO_UNNECESSARY_PREFIX)
            {
                set_pwR_wPrefix(pwRoot, nDigitsLeft, wKey);
            }
#endif // defined(SKIP_LINKS)

            DBGM(printf("NewSwitch pwr_wPrefixPop "OWx"\n",
                pwr_wPrefixPop(pwSw)));

#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
            unsigned nBitsLeftOld = nDigitsLeftOld * cnBitsPerDigit;
#else // defined(SKIP_LINKS)
// Revisit the use of "Old" here.
            unsigned nBitsLeftOld = nDigitsLeft * cnBitsPerDigit;
#endif // defined(SKIP_LINKS)
            if (nBitsLeftOld <= 8) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pcKeys[w] | (wKey & ~0xff), nDigitsLeftOld);
                }
            } else if (nBitsLeftOld <= 16) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, psKeys[w] | (wKey & ~0xffff), nDigitsLeftOld);
                }
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pwKeys[w], nDigitsLeftOld);
                }
            }

            Insert(pwRoot, wKey, nDigitsLeftOld);
        }

        if (wPopCnt != 0) OldList(pwr); // free old
    }
#if defined(SKIP_LINKS)
    else
    {
        // prefix mismatch
        // insert a switch so we can add just one key; seems like a waste
// A bitmap switch would be great; no reason to consider converting the
// existing bitmap to a list if a bitmap switch is short.
        unsigned nDigitsLeftRoot;
        Word_t wPopCnt;

        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

        assert(nDigitsLeftRoot < nDigitsLeft);

        unsigned nDigitsLeftUp = nDigitsLeft;
        // figure new nDigitsLeft for old parent link
        nDigitsLeft = LOG(1 | (pwr_wPrefix(pwr, nDigitsLeftRoot) ^ wKey))
                / cnBitsPerDigit + 1;

        assert(nDigitsLeft > nDigitsLeftRoot);

        pwSw = NewSwitch(pwRoot, wKey, nDigitsLeft, nDigitsLeftUp);

        if ((wPopCnt = pwr_wPopCnt(pwr, nDigitsLeftRoot)) == 0)
        {
            wPopCnt = wPrefixPopMask(nDigitsLeftRoot) + 1;
        }

        set_pwr_wPopCnt(pwSw, nDigitsLeft, wPopCnt);

        set_pwr_wPrefix(pwSw, nDigitsLeft, wKey);

        // copy old link to new switch
        // todo nBitsIndexSz; wide switch
        assert(pwr_nBitsIndexSz(pwr) == cnBitsPerDigit);
// this needs work for Link_t
        pwr_pLinks(pwSw)
            [(pwr_wPrefix(pwr, nDigitsLeftRoot)
                    >> ((nDigitsLeft - 1) * cnBitsPerDigit))
                & (EXP(cnBitsPerDigit) - 1)].ln_wRoot = wRoot;

        Insert(pwRoot, wKey, nDigitsLeftUp);
    }
#endif // defined(SKIP_LINKS)

    return Success;
}

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    DBGR(printf("RemoveGuts\n"));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        if (cnBitsAtBottom <= cnLogBitsPerWord)
        {
            // BUG:  We should check if the switch is empty
            // and free it (and on up the tree as necessary).
            ClrBitInWord(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
            *pwRoot = wRoot;
        }
        else
        {
            ClrBit(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
            // BUG: We should check if the bitmap is
            // empty and free it if so.
        }
    }
    else
    {

#if defined(COMPRESSED_LISTS)
    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
#endif // defined(COMPRESSED_LISTS)
    Word_t wPopCnt = ls_wPopCnt(wRoot);
    unsigned n;

    if (wPopCnt == 1)
    {
        OldList((Word_t *)wRoot); *pwRoot = 0;
        // BUG:  We should check if the switch is empty and free it
        // (and on up the tree as necessary).
    }
    else
    {
        Word_t *pwKeys = wr_pwKeys(wRoot);

        for (n = 0;
#if defined(COMPRESSED_LISTS)
            (nBitsLeft > 16) ? (pwKeys[n] != wKey)
                : (nBitsLeft >= 8)
                    ? (wr_psKeys(wRoot)[n] != (unsigned short)wKey)
                    : (wr_pcKeys(wRoot)[n] != (unsigned char )wKey);
#else // defined(COMPRESSED_LISTS)
            pwKeys[n] != wKey;
#endif // defined(COMPRESSED_LISTS)
            n++)
            ; // semicolon on separate line to silence compiler warning

#if defined(MAX_MIN_LISTS)
        assert(0); // later
#else // defined(MAX_MIN_LISTS)
        // BUG:  We should shrink the list.
#if defined(COMPRESSED_LISTS)
        if (nBitsLeft <= 8) {
            MOVE(&wr_pcKeys(wRoot)[n],
                 &wr_pcKeys(wRoot)[n + 1], wPopCnt - n - 1);
        } else if (nBitsLeft <= 16) {
            MOVE(&wr_psKeys(wRoot)[n],
                 &wr_psKeys(wRoot)[n + 1], wPopCnt - n - 1);
        } else
#endif // defined(COMPRESSED_LISTS)
        { MOVE(&pwKeys[n], &pwKeys[n + 1], wPopCnt - n - 1); }
#endif // defined(MAX_MIN_LISTS)

        set_ls_wPopCnt(wRoot, wPopCnt - 1);
    }

    }

    (void)pwRoot; (void)wKey; (void)nDigitsLeft; (void)wRoot;

    return Success;
}

#endif // cnBitsPerDigit != 0

// ****************************************************************************
// JUDY1 FUNCTIONS:

Word_t
Judy1FreeArray(PPvoid_t PPArray, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("Judy1FreeArray\n"));

#if (cnBitsPerDigit != 0)
    return FreeArrayGuts((Word_t *)PPArray,
        /* wPrefix */ 0, cnBitsPerWord, cnBitsPerWord, /* bDump */ 0);
#else // (cnBitsPerDigit != 0)
    JudyFree(*PPArray,
       EXP(cnBitsPerWord - cnLogBitsPerByte - cnLogBytesPerWord));
    return EXP(cnBitsPerWord - cnLogBitsPerByte);
#endif // (cnBitsPerDigit != 0)
}

// Return count of present keys, inclusive.
Word_t
Judy1Count(Pcvoid_t PArray, Word_t Index1, Word_t Index2, P_JE)
{
    if (PJError != NULL)
    {
        JU_ERRNO(PJError) = JU_ERRNO_NONE; // zero pop
        JU_ERRNO(PJError) = JU_ERRNO_FULL; // full pop
        JU_ERRID(PJError) = __LINE__;
    }

    // Suppress "unused parameter" compiler warnings until we implement
    // this function.
    (void)PArray;
    (void)Index1;
    (void)Index2;

    DBGR(printf("Judy1Count\n"));

#if defined(DEBUG)
    return wInserts;
#else // defined(DEBUG)
    return 0; // Can't use a global in production code.
#endif // defined(DEBUG)
}

