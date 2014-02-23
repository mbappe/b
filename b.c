
// Todo:
//
// "pwr" is a confusing name.  We use it for pointer extracted from wRoot.
// But it can be confused with pwRoot which is a pointer to a wRoot.
// Especially since we use psw for pointer to Switch.
// I think "wrp" might be better.  What do we use for other things extracted
// from wRoot?  wr_nBitsLeft we use nBitsLeft.
// How do we get "pwr"?  With wr_pwr.  Maybe wr_pw and pw would be better?
// Or wr_pwNext and pwNext?

#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#include "Judy.h"   // Word_t, JudyMalloc, ...

#if defined(DEBUG_INSERT)
#define DBGI(x)  (x)
#else // defined(DEBUG_INSERT)
#define DBGI(x)
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_LOOKUP)
#define DBGL(x)  (x)
#else // defined(DEBUG_LOOKUP)
#define DBGL(x)
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_REMOVE)
#define DBGR(x)  (x)
#else // defined(DEBUG_REMOVE)
#define DBGR(x)
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_MALLOC)
#define DBGM(x)  (x)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

#if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || defined(DEBUG_MALLOC)
#define DBG(x)  (x)
#define DEBUG
#else // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...
#define DBG(x)
#endif // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...

#if defined(DEBUG)
#define INLINE static
#else // defined(DEBUG)
#define INLINE static inline
#endif // defined(DEBUG)

#if defined(_WIN64)
//typedef unsigned long long Word_t;
#define EXP(_x)  (1LL << (_x))
#define Owx   "%016llx"
#define OWx "0x%016llx"
#define wx "%llx"
#else // defined(_WIN64)
//typedef unsigned long Word_t;
#define EXP(_x)  (1L << (_x))
#if defined(__LP64__)
#define Owx   "%016lx"
#define OWx "0x%016lx"
#else // defined(__LP64__)
#define Owx   "%08lx"
#define OWx "0x%08lx"
#endif // defined(__LP64__)
#define wx "%lx"
#endif // defined(_WIN64)

#define LOG(x)  ((Word_t)64 - 1 - __builtin_clzll(x))

const int cnLogBitsPerByte = 3;
const int cnBitsPerByte = EXP(cnLogBitsPerByte);

#if defined(__LP64__) || defined(_WIN64)
const int cnLogBytesPerWord = 3;
#else // defined(__LP64__) || defined(_WIN64)
const int cnLogBytesPerWord = 2;
#endif // defined(__LP64__) || defined(_WIN64)

#define MASK(_x)  ((_x) - 1)

const int cnBytesPerWord = EXP(cnLogBytesPerWord);
const int cnBitsPerWord = EXP(cnLogBytesPerWord + cnLogBitsPerByte);
const int cnMallocMask = ((cnBytesPerWord * 2) - 1);

typedef enum { Failure = 0, Success = 1 } Status_t;

#define COPY(_tgt, _src, _cnt) \
    memcpy((_tgt), (_src), sizeof(*(_src)) * (_cnt))

#define MOVE(_tgt, _src, _cnt) \
    memmove((_tgt), (_src), sizeof(*(_src)) * (_cnt))

#define SET(_p, _v, _cnt) \
    memset((_p), (_v), sizeof(*(_p)) * (_cnt))

// Data structure constants and macros.

const int cnDigitsAtBottom = 2;
const int cnBitsPerDigit = 5;
const int cnBitsAtBottom = 5;

// Bus error at 912010843 with 255, 256 or 1024.
// None with 128, 192.
//const Word_t cwListPopCntMax = EXP(cnBitsPerDigit);
const Word_t cwListPopCntMax = 192;

typedef struct {
    Word_t sw_awRoots[EXP(cnBitsPerDigit)];
    Word_t sw_wKey;
} Switch_t;

typedef enum { List, Sw1, Sw2, Sw3, Sw4, Sw5, Sw6, Sw7, } Type_t;

#define     wr_nType(_wr)         ((_wr) & cnMallocMask)
#define set_wr_nType(_ws, _type)  ((_wr) = ((_wr) & ~cnMallocMask) | (_type))

#define     wr_pwr(_wr)          ((Word_t *)((_wr) & ~cnMallocMask))
#define set_wr_pwr(_wr, _pwr) \
    ((_wr) = ((_wr) & cnMallocMask) | (Word_t)(_pwr))

#define set_wr(_wr, _pwr, _type)  ((_wr) = (Word_t)(_pwr) | (_type))

#define     wr_nBitsLeft(_wr) \
    (((wr_nType(_wr) * cnBitsPerDigit) > cnBitsPerWord) \
        ? cnBitsPerWord : (wr_nType(_wr) * cnBitsPerDigit))

#define set_wr_nBitsLeft(_wr, _nBL) \
    ((_wr) = ((_wr) & ~cnMallocMask) \
                | (((_nBL) + cnBitsPerDigit - 1) / cnBitsPerDigit))

#define     pwr_nBitsIndexSz(_pwr)       (cnBitsPerDigit)
#define set_pwr_nBitsIndexSz(_pwr, _sz)  (assert((_sz) == cnBitsPerDigit))

// methods for Switch (and aliases)
#define     sw_wKey(_psw)        (((Switch_t *)(_psw))->sw_wKey)
#define set_sw_wKey(_psw, _key)  (sw_wKey(_psw) = (_key))
#define     sw_wPrefix                sw_wKey
#define set_sw_wPrefix            set_sw_wKey
#define     pwr_wKey                  sw_wKey
#define set_pwr_wKey              set_sw_wKey
#define     pwr_wPrefix               sw_wKey
#define set_pwr_wPrefix           set_sw_wKey

#define     pwr_pwRoots(_pwr)  (((Switch_t *)(_pwr))->sw_awRoots)

#define     ls_wPopCnt(_ls)          ((_ls)[0] & (Word_t)-1)
#define set_ls_wPopCnt(_ls, _cnt) \
    ((_ls)[0] = ((_ls)[0] & ~(Word_t)-1) | (_cnt))

#define     pwr_wPopCnt      ls_wPopCnt
#define set_pwr_wPopCnt  set_ls_wPopCnt

#define     ls_pwKeys(_ls)    (&(_ls)[1])
#define     pwr_pwKeys(_pwr)  (ls_pwKeys(_pwr))

#define     wr_bIsSwitch(_wr)          (wr_nType(_wr) != List)
#define     wr_bIsSwitchBL(_wr, _nBL)  ((_nBL) = wr_nBitsLeft(_wr))

#define BitMapByteNum(_key)  ((_key) >> cnLogBitsPerByte)

#define BitMapByteMask(_key)  (1 << ((_key) % cnBitsPerByte))

#define TestBit(_pBitMap, _key) \
    ((((char *)(_pBitMap))[BitMapByteNum(_key)] & BitMapByteMask(_key)) \
        != 0)

#define BitIsSet  TestBit

#define SetBit(_pBitMap, _key) \
    (((char *)(_pBitMap))[BitMapByteNum(_key)] |= BitMapByteMask(_key))

#define BitTestAndSet(_pBitMap, _key, _bSet) \
    (((_bSet) = TestBit((_pBitMap), (_key))), \
        BitSet((_pBitMap), (_key)), (_bSet))

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

static Status_t Insert(Word_t *pwRoot, Word_t wKey, int nBitsLeft);
static Status_t Remove(Word_t *pwRoot, Word_t wKey, int nBitsLeft);

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
NewSwitch(Word_t wKey, int nBitsLeft)
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
InsertGuts(Word_t *pwRoot, Word_t wKey, int nBitsLeft, Word_t wRoot)
{
    Word_t *pwList;
    Word_t wPopCnt;
    Word_t *pwKeys;
    Switch_t *pSw;
    int w;

    DBGI(printf("InsertGuts pwRoot %p ", pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nBitsLeft %d\n",
            wRoot, wKey, nBitsLeft));

    if (nBitsLeft <= cnBitsAtBottom)
    {
        assert( ! BitIsSet(pwRoot, wKey & (EXP(nBitsLeft) - 1)) );

        SetBit(pwRoot, wKey & (EXP(nBitsLeft) - 1));

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

        pSw = NewSwitch(wKey, nBitsLeft);
        set_wr_pwr(wRoot, (Word_t *)pSw);
        set_wr_nBitsLeft(wRoot, nBitsLeft);
        set_sw_wPrefix(pSw, wKey & ~((EXP(nBitsLeft - 1) << 1) - 1));

        for (w = 0; w < wPopCnt; w++)
        {
            Insert(&wRoot, pwKeys[w], nBitsLeft);
        }

        Insert(&wRoot, wKey, nBitsLeft);

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
    int status = Insert((Word_t *)ppvRoot, wKey, cnBitsPerWord);

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

