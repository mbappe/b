
// Todo:
//
// "pwr" is a confusing name.  We use it for pointer extracted from wRoot.
// But it can be confused with pwRoot which is a pointer to a wRoot.
// Especially since we use psw for pointer to Switch.
// I think "wrp" might be better.  What do we use for other things extracted
// from wRoot?  wr_nDigitsLeft we use nDigitsLeft.
// How do we get "pwr"?  With wr_pwr.  Maybe wr_pw and pw would be better?
// Or wr_pwNext and pwNext?

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
NewList(Word_t wPopCnt)
{
    Word_t *pwList
        = (Word_t *)JudyMalloc(wPopCnt + sizeof(List_t) / sizeof(Word_t));

    DBGM(printf("New pwList %p wPopCnt "OWx"\n", pwList, wPopCnt));

    set_ls_wPopCnt(pwList, wPopCnt);
    set_ls_wLen(pwList, wPopCnt + sizeof(List_t) / sizeof(Word_t));

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

static Switch_t *
NewSwitch(Word_t wKey, unsigned nDigitsLeft)
{
    Switch_t *pSw;

    // wKey is provided in case we decide to initialize prefix here.
    // But we don't have enough info to implement NO_UNNECESSARY_PREFIX here.
    (void)wKey; // fix "unused parameter" compiler warning

#if cnDigitsAtBottom < (cnDigitsPerWord - 1)
    pSw = (Switch_t *)JudyMalloc(sizeof(*pSw) / sizeof(Word_t));
#else // cnDigitsAtBottom < (cnDigitsPerWord - 1)
    posix_memalign((void **)&pSw, 128, sizeof(*pSw)); // double cache line
#endif // cnDigitsAtBottom < (cnDigitsPerWord - 1)
    assert(pSw != NULL);

#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 += sizeof(*pSw) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  += sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 += sizeof(*pSw) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    assert((sizeof(*pSw) % sizeof(Word_t)) == 0);

    DBGM(printf("NewSwitch(wKey "OWx" nDigitsLeft %d) pSw %p\n",
        wKey, nDigitsLeft, pSw));

    memset(pSw, 0, sizeof(*pSw));

    return pSw;
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
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, unsigned nBitsLeft, int bDump)
{
    Word_t wRoot = *pwRoot;
    unsigned nDigitsLeft;
    Word_t *pwr;
    unsigned nBitsIndexSz;
    Word_t *pwRoots;
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

    pwr = wr_pwr(wRoot);

    nType = wr_nType(wRoot);

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
            printf(" "Owx, pwKeys[n]);
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

    wPrefix = sw_wPrefix(pwr, nDigitsLeft);
    nBitsIndexSz = pwr_nBitsIndexSz(pwr);
    pwRoots = pwr_pwRoots(pwr);

    if (bDump)
    {
        printf(" wPopCnt %3llu",
            (unsigned long long)sw_wPopCnt(pwr, nDigitsLeft));
        printf(" wr_nDigitsLeft %2d", nDigitsLeft);
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefixPop "OWx, pwr_wPrefixPop(pwr));
        printf(" wKeyPopMask "OWx, wPrefixPopMask(nDigitsLeft));
        printf(" wr_wPrefix "OWx, wPrefix);
        //printf(" pwRoots "OWx, (Word_t)pwRoots);
        printf("\n");
    }

    nBitsLeft -= nBitsIndexSz;
    // In case nBitsLeftState is not an integral number of digits.
    if (cnBitsPerWord % cnBitsPerDigit != 0)
    {
        nBitsLeft = (nBitsLeft + nBitsIndexSz - 1)
            / nBitsIndexSz * nBitsIndexSz;
    }

    for (n = 0; n < EXP(nBitsIndexSz); n++)
    {
        wBytes += FreeArrayGuts(&pwRoots[n],
            wPrefix | (n << nBitsLeft), nBitsLeft, bDump);
    }

#if defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr, nDigitsLeft) + wBytes);
#else // defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr) + wBytes);
#endif // defined(RAM_METRICS)
}

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, unsigned nBitsLeft)
{
    FreeArrayGuts(&wRoot, wPrefix, nBitsLeft, /* bDump */ 1);
}
#endif // defined(DEBUG)

#if defined(SORT_LISTS)
// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsert(Word_t *pTgt, Word_t *pSrc, unsigned nWords, Word_t wKey)
{
    Word_t aw[cwListPopCntMax]; // buffer for move if pSrc == pTgt
    unsigned i;

    // find the insertion point
    for (i = 0; i < nWords; i++)
    {
        if (pSrc[i] >= wKey)
        {
            assert(pSrc[i] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, i); // copy the head
    }
    else
    {
        COPY(&aw[i], &pSrc[i], nWords - i); // save the tail
        pSrc = aw;
    }

    pTgt[i] = wKey; // insert the key

    COPY(&pTgt[i+1], &pSrc[i], nWords - i); // copy the tail
}
#endif // defined(SORT_LISTS)

Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    Word_t *pwr;
    Switch_t *pSw;
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

    pwr = wr_pwr(wRoot);

    nType = wr_nType(wRoot);

#if defined(SKIP_LINKS)
    if (!tp_bIsSwitch(nType))
#else // defined(SKIP_LINKS)
    assert(nType == 0);
#endif // defined(SKIP_LINKS)
    {
        Word_t wPopCnt;
        Word_t *pwKeys;

        if (pwr != NULL) // pointer to old List
        {
            wPopCnt = ls_wPopCnt(pwr);
            pwKeys = ls_pwKeys(pwr); // list of keys in old List
        }
        else
        {
            wPopCnt = 0;
            pwKeys = NULL; // make compiler happy about uninitialized variable
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
            Word_t *pwList = NewList(wPopCnt + 1);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            { CopyWithInsert(ls_pwKeys(pwList), pwKeys, wPopCnt, wKey); } else
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
            { ls_pwKeys(pwList)[wPopCnt] = wKey; }

            set_wr(wRoot, pwList, /* nType */ 0); // !tp_bIsSwitch
        }
        else
        {
#if defined(SKIP_LINKS)
#if defined(NO_UNNECESSARY_PREFIX) || !defined(NDEBUG)
            unsigned nDigitsLeftOld = nDigitsLeft;
#endif // defined(NO_UNNECESSARY_PREFIX) || !defined(NDEBUG)
#endif // defined(SKIP_LINKS)
            Word_t w;

            // List is full; insert a switch

#if defined(SKIP_LINKS)
            if (cwListPopCntMax != 0) // use const for compile time check
            {
                Word_t wMax, wMin;
#if defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
                wMin = pwKeys[0];
                wMax = pwKeys[wPopCnt - 1];
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

                nDigitsLeft
                    = LOG(1 | ((wKey ^ wMin) | (wKey ^ wMax)))
                        / cnBitsPerDigit + 1;
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

            pSw = NewSwitch(wKey, nDigitsLeft);

            set_sw_wPopCnt(pSw, nDigitsLeft, 0);

#if defined(SKIP_LINKS)
            assert(nDigitsLeft <= nDigitsLeftOld);
#if defined(NO_UNNECESSARY_PREFIX)
            // We could get rid of the bottom check if we enhance INSERT
            // to keep track of any prefix checks done along the way and
            // pass that info to InsertGuts.
            if ((nDigitsLeft == nDigitsLeftOld)
                && (nDigitsLeft > cnDigitsAtBottom + 1))
            {
                DBGI(printf(
                  "Not installing prefix left %d old %d wKey "OWx"\n",
                    nDigitsLeft, nDigitsLeftOld, wKey));
            }
            else
#endif // defined(NO_UNNECESSARY_PREFIX)
            {
                set_sw_wKey(pSw, nDigitsLeft, wKey);
            }
#endif // defined(SKIP_LINKS)

            DBGM(printf("NewSwitch pSw->sw_wPrefixPop "OWx"\n",
                pSw->sw_wPrefixPop));

            set_wr(wRoot, (Word_t *)pSw, nDigitsLeft_to_tp(nDigitsLeft));

            for (w = 0; w < wPopCnt; w++)
            {
                Insert(&wRoot, pwKeys[w], nDigitsLeft);
            }

            Insert(&wRoot, wKey, nDigitsLeft);
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

        // figure new nDigitsLeft for old parent link
        nDigitsLeft = LOG(1 | (pwr_wKey(pwr, nDigitsLeftRoot) ^ wKey))
                / cnBitsPerDigit + 1;

        assert(nDigitsLeft > nDigitsLeftRoot);

        pSw = NewSwitch(wKey, nDigitsLeft);

        if ((wPopCnt = sw_wPopCnt(pwr, nDigitsLeftRoot)) == 0)
        {
            wPopCnt = wPrefixPopMask(nDigitsLeftRoot) + 1;
        }

        set_sw_wPopCnt(pSw, nDigitsLeft, wPopCnt);

        set_sw_wKey(pSw, nDigitsLeft, wKey);

        // copy old link to new switch
        // todo nBitsIndexSz; wide switch
        assert(pwr_nBitsIndexSz(pwr) == cnBitsPerDigit);
        pSw->sw_awRoots
            [(pwr_wKey(pwr, nDigitsLeftRoot)
                    >> ((nDigitsLeft - 1) * cnBitsPerDigit))
                & (EXP(cnBitsPerDigit) - 1)] = wRoot;

        set_wr(wRoot, (Word_t *)pSw, nDigitsLeft_to_tp(nDigitsLeft));

        Insert(&wRoot, wKey, nDigitsLeft);
    }
#endif // defined(SKIP_LINKS)

    *pwRoot = wRoot; // install new

    return Success;
}

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft, Word_t wRoot)
{
    Word_t wPopCnt = ls_wPopCnt(wRoot);
    unsigned n;

    DBGR(printf("RemoveGuts\n"));

    if (wPopCnt == 1)
    {
        OldList((Word_t *)wRoot); *pwRoot = 0;
        // BUG:  We should check if the switch is empty and free it and so on.
    }
    else
    {
        Word_t *pwKeys = wr_pwKeys(wRoot);

        for (n = 0; pwKeys[n] != wKey; n++);

#if defined(MAX_MIN_LISTS)
        assert(0); // later
#else // defined(MAX_MIN_LISTS)
        // BUG:  We should shrink the list.
        MOVE(&pwKeys[n], &pwKeys[n + 1], wPopCnt - n - 1);
#endif // defined(MAX_MIN_LISTS)

        set_ls_wPopCnt(wRoot, wPopCnt - 1);
    }

    // suppress "unused" compiler warnings
    (void)pwRoot; (void)wKey; (void)nBitsLeft; (void)wRoot;

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
        /* wPrefix */ 0, cnBitsPerWord, /* bDump */ 0);
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

