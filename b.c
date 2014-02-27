
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
Word_t j__AllocWordsJLLW;
Word_t j__AllocWordsJBU;
Word_t j__AllocWordsJLB1;
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

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, unsigned nBitsLeft)
{
    unsigned nDigitsLeft;
    Word_t *pwr;
    unsigned nBitsIndexSz;
    Word_t *pwRoots;
    unsigned nType;
    unsigned i;

    if (wRoot == 0)
    {
        return;
    }

    printf(" nBitsLeft %2d", nBitsLeft);
    // should enhance this to check for zeros in suffix and to print
    // dots for suffix.
    printf(" wPrefix "OWx, wPrefix);
    printf(" wr "OWx, wRoot);

    if (nBitsLeft <= cnBitsAtBottom)
    {
        if (cnBitsAtBottom > cnLogBitsPerWord)
        {
            for (i = 0;
                (i < EXP(cnBitsAtBottom) / cnBitsPerWord) && (i < 8);
                 i++)
            {
                printf(" "Owx, ((Word_t *)wRoot)[i]);
            }
        }
        printf("\n");

        return;
    }

    pwr = wr_pwr(wRoot);

    if ((nType = wr_nType(wRoot)) == List)
    {
        Word_t wPopCnt = ls_wPopCnt(pwr);
        Word_t *pwKeys = pwr_pwKeys(pwr);

        assert(wr_nType(wRoot) == List);

        printf(" wPopCnt %3llu", (unsigned long long)wPopCnt);
        for (i = 0; (i < wPopCnt) && (i < 8); i++) printf(" "Owx, pwKeys[i]);
        printf("\n");

        return;
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

    nBitsLeft -= nBitsIndexSz;
    // In case nBitsLeftState is not an integral number of digits.
    if (cnBitsPerWord % cnBitsPerDigit != 0)
    {
        nBitsLeft = (nBitsLeft + nBitsIndexSz - 1)
            / nBitsIndexSz * nBitsIndexSz;
    }

    for (i = 0; i < EXP(nBitsIndexSz); i++)
    {
        Dump(pwRoots[i], wPrefix | (i << nBitsLeft), nBitsLeft);
    }
}
#endif // defined(DEBUG)

INLINE Word_t *
NewList(Word_t wPopCnt)
{
    Word_t *pwList = (Word_t *)JudyMalloc(wPopCnt + 1);

    METRICS(j__AllocWordsJLLW += (ls_wPopCnt(pwList) + 1));

    set_ls_wPopCnt(pwList, wPopCnt);

    DBGM(printf("New pwList %p wPopCnt "OWx"\n", pwList, wPopCnt));

    return pwList;
}

INLINE void
OldList(Word_t *pwList)
{
    DBGM(printf("Old pwList %p\n", pwList));

    METRICS(j__AllocWordsJLLW -= (ls_wPopCnt(pwList) + 1));

    JudyFree(pwList, ls_wPopCnt(pwList) + 1);
}

INLINE Word_t
NewBitMap(void)
{
    Word_t w = JudyMalloc(EXP(cnBitsAtBottom) / cnBitsPerWord);

    METRICS(j__AllocWordsJLB1 += (EXP(cnBitsAtBottom) / cnBitsPerWord));

    DBGM(printf("NewBitMap nBitsAtBottom %u nBits "OWx
      " nBytes "OWx" nWords "OWx" w "OWx"\n",
        cnBitsAtBottom, EXP(cnBitsAtBottom),
        EXP(cnBitsAtBottom) / cnBitsPerByte,
        EXP(cnBitsAtBottom) / cnBitsPerWord, w));

    if (w == 0) { fprintf(stderr, "NewBitMap malloc.\n"); exit(1); }

    memset((void *)w, 0, EXP(cnBitsAtBottom) / cnBitsPerByte);

    return w;
}

INLINE Switch_t *
NewSwitch(Word_t wKey, unsigned nDigitsLeft)
{
    Switch_t *pSw;

#if cnDigitsAtBottom < (cnDigitsPerWord - 1)
    pSw = (Switch_t *)JudyMalloc(sizeof(*pSw) / sizeof(Word_t));
#else // cnDigitsAtBottom < (cnDigitsPerWord - 1)
    posix_memalign((void **)&pSw, 128, sizeof(*pSw)); // double cache line
#endif // cnDigitsAtBottom < (cnDigitsPerWord - 1)
    assert(pSw != NULL);

    METRICS(j__AllocWordsJBU += sizeof(*pSw) / sizeof(Word_t));

    assert((sizeof(*pSw) % sizeof(Word_t)) == 0);

    DBGM(printf("NewSwitch(wKey "OWx" nDigitsLeft %d) pSw %p\n",
        wKey, nDigitsLeft, pSw));

    memset(pSw, 0, sizeof(*pSw));

    return pSw;
}

#if 0
INLINE void
OldSwitch(Switch_t *pSw)
{
    JudyFree((Word_t *)pSw, sizeof(*pSw) / sizeof(Word_t));
}
#endif

#if defined(SORT_LISTS)
// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
INLINE void
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
    assert(cnDigitsAtBottom > 0); // can't get to full pop
    assert(cnDigitsAtBottom + 1 <= cnDigitsPerWord);
    assert(cnDigitsAtBottom + cnMallocMask >= cnDigitsPerWord + 1);

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
                *pwRoot = wRoot = NewBitMap();
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
    if (nType == List)
#else // defined(SKIP_LINKS)
    assert(nType == List);
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

        if (wPopCnt < cwListPopCntMax)
        {
            // allocate a new list and init pop count in the first word
            Word_t *pwList = NewList(wPopCnt + 1);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            { CopyWithInsert(ls_pwKeys(pwList), pwKeys, wPopCnt, wKey); } else
#else // defined(SORT_LISTS)
            { COPY(ls_pwKeys(pwList), pwKeys, wPopCnt); }
#endif // defined(SORT_LISTS)
            { ls_pwKeys(pwList)[wPopCnt] = wKey; }

            set_wr(wRoot, pwList, List);
        }
        else
        {
#if defined(SKIP_LINKS)
            unsigned nDigitsLeftOld = nDigitsLeft;
#endif // defined(SKIP_LINKS)
            Word_t w;

            // List is full; insert a switch

#if defined(SKIP_LINKS)
            if (cwListPopCntMax != 0) // use const for compile time check
            {
                Word_t wMax, wMin;
#if defined(SORT_LISTS)
                wMin = pwKeys[0];
                wMax = pwKeys[wPopCnt - 1];
#else // defined(SORT_LISTS)
                // walk the list to find max and min
                wMin = (Word_t)-1;
                wMax = 0;

                for (w = 0; w < wPopCnt; w++)
                {
                    if (wKey < wMin) wMin = wKey;
                    if (wKey > wMax) wMax = wKey;
                }
#endif // defined(SORT_LISTS)

                nDigitsLeft
                    = LOG(1 | ((wKey ^ wMin) | (wKey ^ wMax)))
                        / cnBitsPerDigit + 1;
            }
            else
            {
                // can't dereference list if there isn't one
                nDigitsLeft = cnDigitsAtBottom + 1; // go directly to bitmap
            }

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
// Would like to get rid of this install of the prefix when it is not
// necessary.  It should not be necessary.
            //if (nDigitsLeft == nDigitsLeftOld)
            {
//printf("\n\nNot installing prefix left %d old %d\n",
//    nDigitsLeft, nDigitsLeftOld);
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
    // suppress "unused" compiler warnings
    (void)pwRoot; (void)wKey; (void)nBitsLeft; (void)wRoot;

    assert(0);

    return Failure;
}

#endif // cnBitsPerDigit != 0

// ****************************************************************************
// JUDY1 FUNCTIONS:

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warning

#if cnBitsPerDigit != 0

    return Lookup((Word_t)pcvRoot, wKey);

#else // cnBitsPerDigit != 0

    // one big bitmap
    {
        Word_t wByteNum, wByteMask;

        if (pcvRoot == NULL)
        {
            return Failure;
        }

        wByteNum = BitMapByteNum(wKey);
        wByteMask = BitMapByteMask(wKey);     

        return (((char *)pcvRoot)[wByteNum] & wByteMask) ? Success : Failure;
    }

#endif // cnBitsPerDigit != 0

}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warning

#if cnBitsPerDigit != 0

    {
        int status = Insert((Word_t *)ppvRoot, wKey, cnDigitsPerWord);

#if defined(DEBUG)
        if (status == Success) wInserts++; // count successful inserts
#endif // defined(DEBUG)

#if defined(DEBUG_INSERT)
        if (wInserts >= cwDebugThreshold)
        {
            printf("\n# After Insert(wKey "OWx") Dump\n", wKey);
            Dump((Word_t)*ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord);
            printf("\n");
        }
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG)
        {
            Word_t wRoot = *(Word_t *)ppvRoot;
            unsigned nType = wr_nType(wRoot);

            if (nType != List)
            {
                assert(sw_wPopCnt(wr_pwr(wRoot), tp_to_nDigitsLeft(nType))
                    == (wInserts & wPrefixPopMask(tp_to_nDigitsLeft(nType))));
            }
            else
            {
                assert(wr_ls_wPopCnt(wRoot) == wInserts);
            }
        }
#endif // defined(DEBUG)

        return status;
    }

#else // cnBitsPerDigit != 0

    // one big bitmap
    {
        Word_t wRoot;
        Word_t wByteNum, wByteMask;
        char c;

        if ((wRoot = (Word_t)*ppvRoot) == 0)
        {
            wRoot = JudyMalloc(EXP
                (cnBitsPerWord - cnLogBitsPerByte - cnLogBytesPerWord));

            *ppvRoot = (PPvoid_t)wRoot;
        }

        wByteNum = BitMapByteNum(wKey);
        wByteMask = BitMapByteMask(wKey);     

        if ((c = ((char *)wRoot)[wByteNum]) & wByteMask)
        {
            return Failure; // dup
        }

        ((char *)wRoot)[wByteNum] = c | wByteMask;

        return Success;
    }

#endif // cnBitsPerDigit != 0

}

int
Judy1Unset( PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    (void)PJError; // suppress "unused" compiler warnings

#if cnBitsPerDigit != 0

    return Remove((Word_t *)ppvRoot, wKey, cnBitsPerWord);

#else // cnBitsPerDigit != 0

    // one big bitmap
    {

        Word_t wRoot;
        Word_t wByteNum, wByteMask;
        char c;

        if ((wRoot = (Word_t)*ppvRoot) == 0)
        {
            return Failure; // not present
        }

        wByteNum = BitMapByteNum(wKey);
        wByteMask = BitMapByteMask(wKey);     

        if ( ! ((c = ((char *)wRoot)[wByteNum]) & wByteMask) )
        {
            return Failure; // not present
        }

        ((char *)wRoot)[wByteNum] = c & ~wByteMask;

        return Success;
    }

#endif // cnBitsPerDigit != 0

}

