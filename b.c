
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

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, int nBitsLeft)
{
    int nDigitsLeft;
    Word_t *pwr;
    int nBitsIndexSz;
    Word_t *pwRoots;
    int i;

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
            for (i = 0; i < EXP(cnBitsAtBottom) / cnBitsPerWord; i++)
            {
                printf(" "Owx, ((Word_t *)wRoot)[i]);
            }
        }
        printf("\n");

        return;
    }

    pwr = wr_pwr(wRoot);

    if ( ! wr_bIsSwitchDL(wRoot, nDigitsLeft) )
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

    set_ls_wPopCnt(pwList, wPopCnt);

    DBGM(printf("New pwList %p wPopCnt "OWx"\n", pwList, wPopCnt));

    return pwList;
}

INLINE void
OldList(Word_t *pwList)
{
    DBGM(printf("Old pwList %p\n", pwList));

    JudyFree(pwList, ls_wPopCnt(pwList) + 1);
}

INLINE Word_t
NewBitMap(void)
{
    Word_t w = JudyMalloc(EXP(cnBitsAtBottom) / cnBitsPerWord);

    DBGM(printf("NewBitMap w "OWx"\n", w));

    memset((void *)w, 0, EXP(cnBitsAtBottom) / cnBitsPerByte);

    return w;
}

INLINE Switch_t *
NewSwitch(Word_t wKey, int nDigitsLeft)
{
    Switch_t *pSw = (Switch_t *)JudyMalloc(sizeof(*pSw) / sizeof(Word_t));

    assert((sizeof(*pSw) % sizeof(Word_t)) == 0);

    DBGM(printf("NewSwitch(wKey "OWx" nDigitsLeft %d) pSw %p\n",
        wKey, nDigitsLeft, pSw));

    memset(pSw, 0, sizeof(*pSw));

    set_sw_wKey(pSw, nDigitsLeft, wKey);

    DBGM(printf("NewSwitch pSw->sw_wPrefixPop "OWx"\n", pSw->sw_wPrefixPop));

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
CopyWithInsert(Word_t *pTgt, Word_t *pSrc, int nWords, Word_t wKey)
{
    Word_t aw[cwListPopCntMax]; // buffer for move if pSrc == pTgt
    int i;

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

static Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, int nDigitsLeft, Word_t wRoot)
{
    int nDigitsLeftRoot;
    Word_t *pwr;
    Switch_t *pSw;

    DBGI(printf("InsertGuts pwRoot %p ", pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        if (cnBitsAtBottom <= cnLogBitsPerWord) // compile time
        {
            assert(!BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
                *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

            assert(BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
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

    if ( ! wr_bIsSwitchDL(wRoot, nDigitsLeftRoot))
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
            int w;

            // List is full; insert a switch

#if defined(SKIP_LINKS)
#if defined(SORT_LISTS)
            if (cwListPopCntMax != 0) // use const to get compile time check
            {
                nDigitsLeft
                    = LOG(1 | ((wKey ^ pwKeys[0])
                            | (wKey ^ pwKeys[wPopCnt - 1])))
                        / cnBitsPerDigit + 1;
            }
            else
            {
                // can't dereference list if there isn't one
                nDigitsLeft = cnDigitsAtBottom + 1; // go directly to bitmap
            }
#else // defined(SORT_LISTS)
            assert(0); // later
#endif // defined(SORT_LISTS)
#endif // defined(SKIP_LINKS)

            if (nDigitsLeft <= cnDigitsAtBottom)
            {
                DBGI(printf("InsertGuts nDigitsLeft <= cnDigitsAtBottom\n"));

                nDigitsLeft = cnDigitsAtBottom + 1;
            }

            pSw = NewSwitch(wKey, nDigitsLeft);

            set_wr(wRoot, (Word_t *)pSw, nDigitsLeft);

            for (w = 0; w < wPopCnt; w++)
            {
                Insert(&wRoot, pwKeys[w], nDigitsLeft);
            }

            Insert(&wRoot, wKey, nDigitsLeft);
        }

        if (wPopCnt != 0) OldList(pwr); // free old
    }
    else
    {
        assert(nDigitsLeftRoot < nDigitsLeft);
        // prefix mismatch; insert a switch so we can add just one key
        // seems like a waste
        // figure new nDigitsLeft for old link
        nDigitsLeft = LOG(1 | (pwr_wKey(pwr, nDigitsLeftRoot) ^ wKey))
                / cnBitsPerDigit + 1;

        pSw = NewSwitch(wKey, nDigitsLeft);

        set_sw_wPopCnt(pSw, nDigitsLeft, sw_wPopCnt(pwr, nDigitsLeftRoot));

        // copy old link to new switch
        // todo nBitsIndexSz; wide switch
        assert(pwr_nBitsIndexSz(pwr) == cnBitsPerDigit);
        pSw->sw_awRoots
            [(pwr_wKey(pwr, nDigitsLeftRoot)
                    >> ((nDigitsLeft - 1) * cnBitsPerDigit))
                & (EXP(cnBitsPerDigit) - 1)] = wRoot;

        set_wr(wRoot, (Word_t *)pSw, nDigitsLeft);

        Insert(&wRoot, wKey, nDigitsLeft);
    }

    *pwRoot = wRoot; // install new

    return Success;
}

static Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, int nBitsLeft, Word_t wRoot)
{
    assert(0);

    return Failure;
}

#define LOOKUP
#include "bli.c"
#undef LOOKUP
#define INSERT
#include "bli.c"
#undef INSERT
#define REMOVE
#include "bli.c"
#undef REMOVE

// ****************************************************************************
// JUDY1 FUNCTIONS:

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, P_JE)
{
    return Lookup((Word_t)pcvRoot, wKey);
}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    int status = Insert((Word_t *)ppvRoot, wKey, cnDigitsPerWord);

#if defined(DEBUG_INSERT)
    printf("\n# After Insert(wKey "OWx") Dump\n", wKey);
    Dump((Word_t)*ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord);
    printf("\n");
#endif // defined(DEBUG_INSERT)

    return status;
}


int
Judy1Unset( PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    return Remove((Word_t *)ppvRoot, wKey, cnBitsPerWord);
}

int Judy1SetArray(PPvoid_t PPArray,
    Word_t Count, const Word_t * const PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t Judy1Count(Pcvoid_t PArray, Word_t Index1, Word_t Index2, P_JE)
{ printf("\nJudy1Count\n\n"); exit(0); }

int Judy1ByCount(Pcvoid_t PArray, Word_t Count, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t Judy1FreeArray(PPvoid_t PPArray, P_JE)
{ printf("\nJudy1FreeArray\n\n"); exit(1); }

Word_t Judy1MemUsed(Pcvoid_t PArray) { printf("n/a\n"); exit(5); }

Word_t Judy1MemActive(Pcvoid_t PArray) { printf("n/a\n"); exit(6); }

int Judy1First(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(7); }

int Judy1Next(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(8); }

int Judy1Last(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(9); }

int Judy1Prev(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(10); }

int Judy1FirstEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(11); }

int Judy1NextEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(12); }

int Judy1LastEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(13); }

int Judy1PrevEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(14); }

// ****************************************************************************
// JUDYL FUNCTIONS:

PPvoid_t JudyLGet(Pcvoid_t PArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLIns(PPvoid_t PPArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLDel(PPvoid_t PPArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyLCount(Pcvoid_t PArray, Word_t Index1, Word_t Index2, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLByCount(Pcvoid_t PArray, Word_t Count, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyLFreeArray(PPvoid_t PPArray, P_JE) { printf("n/a\n"); exit(1); }

Word_t JudyLMemUsed(Pcvoid_t PArray) { printf("n/a\n"); exit(1); }

Word_t JudyLMemActive(Pcvoid_t PArray) { printf("n/a\n"); exit(1); }

PPvoid_t JudyLFirst(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLNext(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLLast(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLPrev(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLFirstEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLNextEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLLastEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLPrevEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

// ****************************************************************************
// JUDYHS FUNCTIONS:

PPvoid_t JudyHSGet(Pcvoid_t x, void *y, Word_t z)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyHSIns( PPvoid_t x, void *y, Word_t z, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyHSDel(PPvoid_t x, void *y, Word_t z, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyHSFreeArray(PPvoid_t x, P_JE) { printf("n/a\n"); exit(1); }

const char *Judy1MallocSizes = "Judy1MallocSizes go here.";
const char *JudyLMallocSizes = "JudyLMallocSizes go here.";

