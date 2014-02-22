
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

#if defined(DEBUG_DUMP)
#define DBGD(x)  (x)
#else // defined(DEBUG_DUMP)
#define DBGD(x)
#endif // defined(DEBUG_DUMP)

#if defined(DEBUG_MALLOC)
#define DBGM(x)  (x)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

#if defined(DEBUG)
#define DBG(x)  (x)
#else // defined(DEBUG)
#define DBG(x)
#endif // defined(DEBUG)

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

//const Word_t cwListPopCntMax = EXP(cnBitsPerDigit);
//const Word_t cwListPopCntMax = 1024; // bug at pop near 1M
const Word_t cwListPopCntMax = 0;

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

#define     ws_nBitsLeft(_ws)          ((_ws) & 0x7f)
#define set_ws_nBitsLeft(_ws, _nBL)    ((_ws) = ((_ws) & ~0x7f) | (_nBL))

#define     ws_bNeedPrefixCheck(_ws)      ((_ws) & 0x80)
#define set_ws_bNeedPrefixCheck(_ws, _b)  ((_ws) = ((_ws) & ~0x80) | (_b))

#define     wr_bIsSwitch(_wr)          (wr_nType(_wr) != List)
#define     wr_bIsSwitchBL(_wr, _nBL)  ((_nBL) = wr_nBitsLeft(_wr))

static Word_t *pwRootLast;

#define BitMapByteNum(_key)  ((_key) >> cnLogBitsPerByte)

#define BitMapByteMask(_key)  (1 << ((_key) % cnBitsPerByte))

#define BitTest(_pBitMap, _key) \
    ((((char *)(_pBitMap))[BitMapByteNum(_key)] & BitMapByteMask(_key)) \
        != 0)

#define BitIsSet(_pBitMap, _key)  BitTest((_pBitMap), (_key))

#define SetBit(_pBitMap, _key) \
    (((char *)(_pBitMap))[BitMapByteNum(_key)] |= BitMapByteMask(_key))

#define BitTestAndSet(_pBitMap, _key, _bSet) \
    (((_bSet) = BitTest((_pBitMap), (_key))), \
        BitSet((_pBitMap), (_key)), (_bSet))

void
dump(Word_t *pw, int nWords)
{
    int i;

    printf("pw "OWx, (Word_t)pw);
    for (i = 0; i < nWords; i++)
        printf(" "Owx, pw[i]);
}

void
Dump(Word_t wRoot, Word_t wPrefix, Word_t wState)
{
    int nBitsLeftState = ws_nBitsLeft(wState);
    int nType;
    Word_t *pwr;
    int nBitsIndexSz;
    int bIsSwitch;
    int nBitsLeft;
    Word_t *pwRoots;
    int i;

    // bitmaps shouldn't be zero
    // but it should be fine to print nothing if they are
    if (wRoot == 0)
    {
        return;
    }

    printf(" wr "OWx, wRoot);
    printf(" wPrefix "OWx, wPrefix);
    printf(" nBitsLeftState %2d", nBitsLeftState);

    if (nBitsLeftState < cnBitsAtBottom)
    {
        return;
    }

    nType = wr_nType(wRoot);
    bIsSwitch = wr_bIsSwitch(wRoot);
    pwr = wr_pwr(wRoot);
    nBitsIndexSz = pwr_nBitsIndexSz(pwr);

    printf(" nType %d", nType);
    //printf(" nBitsIndexSz %2d", nBitsIndexSz);

    if ( ! bIsSwitch )
    {
        assert(nType == List);
    }
    else
    {
        wPrefix = pwr_wPrefix(pwr);
        pwRoots = pwr_pwRoots(pwr);
    }

    nBitsLeft = wr_nBitsLeft(wRoot);

    printf(" ");
    if (bIsSwitch)
    {
        for (i = 0; i < cnBitsPerWord - nBitsLeft; i++)
        {
            printf(wx, (wPrefix << i) >> (cnBitsPerWord - 1));
        }
        for (i = 0; i < nBitsLeft; i++)
        {
            printf(".");
        }
    }
    else
    {
        for (i = 0; i < cnBitsPerWord; i++)
        {
            printf(".");
        }
    }

    if ( ! bIsSwitch )
    {
        printf(" ");
        dump(pwr, pwr_wPopCnt(pwr) + 1);
        printf("\n");

        return;
    }

    printf(" pwRoots "OWx, (Word_t)pwRoots);
    printf("\n");

    nBitsLeft -= nBitsIndexSz;
    // In case nBitsLeftState is not an integral number of digits.
    nBitsLeft = (nBitsLeft + nBitsIndexSz - 1) / nBitsIndexSz * nBitsIndexSz;

    for (i = 0; i < EXP(nBitsIndexSz); i++)
    {
        Dump(pwr_pwRoots(pwr)[i], wPrefix | (i << nBitsLeft), nBitsLeft);
    }
}

static Status_t Insert(Word_t *pwRoot, Word_t wKey, Word_t wStatus);

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
InsertGuts(Word_t *pwRoot, Word_t wKey, Word_t wState, Word_t wRoot)
{
    int nBitsLeft = ws_nBitsLeft(wState);
    Word_t *pwList;
    Word_t wPopCnt;
    Word_t *pwKeys;
    Switch_t *pSw;
    int w;

    DBGI(printf("\n# InsertGuts "));
    DBGI(printf("# pwRoot %p ", pwRoot));
    DBGI(printf("# wRoot "OWx" wKey "OWx" wState "OWx"\n",
            wRoot, wKey, wState));

    if (nBitsLeft <= cnBitsAtBottom)
    {
        assert( ! ws_bNeedPrefixCheck(wState) );
        assert( ! BitIsSet(pwRoot, wKey & (EXP(nBitsLeft) - 1)) );

        DBGI(printf("SetBit pwRoot %p "OWx"\n",
            pwRoot, wKey & (EXP(nBitsLeft) - 1)));

        SetBit(pwRoot, wKey & (EXP(nBitsLeft) - 1));

        return Success;
    }

    if (cwListPopCntMax != 0)
    {
        if (wRoot == 0)
        {
            // allocate new list List and init pop count in the first word
            pwList = NewList(/* nKeys */ 1);
            pwKeys = ls_pwKeys(pwList); // get a pointer to the keys
            pwKeys[0] = wKey;
            set_wr(*pwRoot, pwList, List); // install new list

            return Success;
        }

        pwList = wr_pwr(wRoot); // pointer to old List
        wPopCnt = ls_wPopCnt(pwList); // population count of old List
        pwKeys = ls_pwKeys(pwList); // list of keys in old List

        DBGI(printf("pwList %p wPopCnt "OWx" pwKeys %p\n",
            pwList, wPopCnt, pwKeys));

        if ((wPopCnt < cwListPopCntMax) || (nBitsLeft == cnBitsAtBottom))
        {
            // allocate space for bigger list and init pop count
            Word_t *pwListNew = NewList( /* nKeys */ wPopCnt + 1);
            Word_t *pwKeysNew = ls_pwKeys(pwListNew);
            pwKeys = ls_pwKeys(pwList); // keys in old list
            COPY(pwKeysNew, pwKeys, wPopCnt); // copy keys
            pwKeysNew[wPopCnt] = wKey; // add new key to end
            set_wr(*pwRoot, pwListNew, List); // install list
            OldList(pwList); // free old list

            return Success;
        }
    }
    else
    {
        wPopCnt = 0;
        pwList = NULL; // make compiler happy about uninitialized variable
        pwKeys = NULL; // make compiler happy about uninitialized variable
    }

    // List is full; insert a switch

    pSw = NewSwitch(wKey, nBitsLeft);
    set_wr_pwr(wRoot, (Word_t *)pSw);
    set_wr_nBitsLeft(wRoot, nBitsLeft);
    assert(wr_bIsSwitch(wRoot));
    set_sw_wPrefix(pSw, wKey & ~((EXP(nBitsLeft - 1) << 1) - 1));
    DBGI(printf("wPrefix "OWx"\n", sw_wPrefix(pSw)));

    for (w = 0; w < wPopCnt; w++)
    {
        Insert(&wRoot, pwKeys[w], wState);
    }

    Insert(&wRoot, wKey, wState);

    *pwRoot = wRoot; // install new

    if (wPopCnt != 0)
    {
        OldList(pwList); // free old
    }

    return Success;
}

static Status_t Remove(Word_t *pwRoot, Word_t wKey, Word_t wStatus);

static Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, Word_t wState, Word_t wRoot)
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
    return Lookup((Word_t)pcvRoot, wKey, /* wState */ cnBitsPerWord);
}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    int status;

    pwRootLast = (Word_t *)ppvRoot;

    status = Insert((Word_t *)ppvRoot, wKey, /* wState */ cnBitsPerWord);

#if defined(DEBUG_INSERT)
    printf("\n# After Insert(wKey "OWx")\n", wKey);
    Dump((Word_t)*ppvRoot, 0, cnBitsPerWord);
    printf("\n");
#endif // defined(DEBUG_INSERT)

    return status;
}

int
Judy1Unset( PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    printf("Judy1Unset\n");

    return Remove((Word_t *)ppvRoot, wKey, /* wState */ cnBitsPerWord);
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

