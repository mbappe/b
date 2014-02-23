
// Todo:
//
// "pwr" is a confusing name.  We use it for pointer extracted from wRoot.
// But it can be confused with pwRoot which is a pointer to a wRoot.
// Especially since we use psw for pointer to Switch.
// I think "wrp" might be better.  What do we use for other things extracted
// from wRoot?  wr_nBitsLeft we use nBitsLeft.
// How do we get "pwr"?  With wr_pwr.  Maybe wr_pw and pw would be better?
// Or wr_pwNext and pwNext?

#include "b.h"

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, int nBitsLeft)
{
    Word_t *pwr;
    int nBitsIndexSz;
    Word_t *pwRoots;
    int i;

    // bitmaps shouldn't be zero
    // but it should be fine to print nothing if they are
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
        printf("\n");

        return;
    }

    pwr = wr_pwr(wRoot);

    if ( ! wr_bIsSwitchBL(wRoot, nBitsLeft) )
    {
        int nPopCnt = (int)pwr_wPopCnt(pwr);
        Word_t *pwKeys = pwr_pwKeys(pwr);

        assert(wr_nType(wRoot) == List);

        printf(" wPopCnt %3d", nPopCnt);
        for (i = 0; i < (nPopCnt &= 7); i++) printf(" "Owx, pwKeys[i]);
        printf("\n");

        return;
    }

    // Switch

    wPrefix = pwr_wPrefix(pwr);
    nBitsIndexSz = pwr_nBitsIndexSz(pwr);
    pwRoots = pwr_pwRoots(pwr);

    printf(" wr_nBitsLeft %2d", nBitsLeft);
    // should enhance this to check for zeros in suffix and to print
    // dots for suffix.
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

INLINE Switch_t *
NewSwitch(Word_t wKey)
{
    Switch_t *pSw = (Switch_t *)JudyMalloc(sizeof(Switch_t) / sizeof(Word_t));

    DBGM(printf("NewSwitch pSw %p\n", pSw));

    set_sw_wKey(pSw, wKey);

    SET(pSw, /* val */ 0, /* cnt */ 1);
    memset(pSw, /* val */ 0, /* cnt */ sizeof(*pSw));

    return pSw;
}

#if 0
INLINE void
OldSwitch(Switch_t *pSw)
{
    JudyFree((Word_t *)pSw, sizeof(*pSw) / sizeof(Word_t));
}
#endif

static Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, int nDigitsLeft, Word_t wRoot)
{
    int nBitsLeft = nDigitsLeft * cnBitsPerDigit;
    Word_t *pwList;
    Word_t wPopCnt;
    Word_t *pwKeys;
    Switch_t *pSw;
    int w;

    DBGI(printf("InsertGuts pwRoot %p ", pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        assert(cnBitsAtBottom <= cnBitsPerWord);
        assert( ! BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)) );

        SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

        return Success;
    }

    if ((pwList = wr_pwr(wRoot)) != NULL) // pointer to old List
    {
        wPopCnt = ls_wPopCnt(pwList);
        pwKeys = ls_pwKeys(pwList); // list of keys in old List
    }
    else
    {
         wPopCnt = 0; // make compiler happy about uninitialized variable
         pwKeys = NULL; // make compiler happy about uninitialized variable
    }

    if (wPopCnt <= cwListPopCntMax)
    {
        // allocate a new list and init pop count in the first word
        Word_t *pwListNew = NewList(wPopCnt + 1);
        Word_t *pwKeysNew = ls_pwKeys(pwListNew); // pointer to the keys

        if (wPopCnt != 0) COPY(pwKeysNew, pwKeys, wPopCnt); // copy keys
        pwKeysNew[wPopCnt] = wKey; // append the key
        set_wr(*pwRoot, pwListNew, List); // install new list
    }
    else
    {
        // List is full; insert a switch

        pSw = NewSwitch(wKey);
        set_wr_pwr(wRoot, (Word_t *)pSw);
        set_wr_nDigitsLeft(wRoot, nDigitsLeft);
        set_sw_wPrefix(pSw,
           wKey & ~(((nBitsLeft >= cnBitsPerWord) ? 0 : EXP(nBitsLeft)) - 1));

        for (w = 0; w < wPopCnt; w++)
        {
            Insert(&wRoot, pwKeys[w], nDigitsLeft);
        }

        Insert(&wRoot, wKey, nDigitsLeft);

        *pwRoot = wRoot; // install new
    }

    if (wPopCnt != 0) OldList(pwList); // free old

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

