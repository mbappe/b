
#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#define __STDC_FORMAT_MACROS
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

#if defined(DEBUG_DUMP)
#define DBGD(x)  (x)
#else // defined(DEBUG_DUMP)
#define DBGD(x)
#endif // defined(DEBUG_DUMP)

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

const int cnLogBitsPerByte = 3;

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

#if defined(__LP64__) || defined(_WIN64)
const int cnLogBytesPerWord = 3;
#else // defined(__LP64__) || defined(_WIN64)
const int cnLogBytesPerWord = 2;
#endif // defined(__LP64__) || defined(_WIN64)

#define mask(_x)  ((_x) - 1)

const int cMallocMask = (EXP(cnLogBytesPerWord + 1) - 1);

const int cnBitsPerWord = EXP(cnLogBytesPerWord + cnLogBitsPerByte);

typedef enum { Failure = 0, Success = 1 } Status_t;

#define copy(_tgt, _src, _cnt) \
    memcpy((_tgt), (_src), sizeof(*(_src)) * (_cnt))

#define move(_tgt, _src, _cnt) \
    memmove((_tgt), (_src), sizeof(*(_src)) * (_cnt))

// Data structure constants and macros.

const int cnTypeByteOff         = 2; // node type goes here
const int cnBitsPrefixSzByteOff = 1; // prefix size goes here
const int cnBitsIndexSzByteOff  = 0; // index size goes here

const int cnPrefixOff           = 1; // prefix goes here
const int cnKeyOff              = 1; // key goes here
const int cnPtrsOff             = 2; // array of pointers begins here

#define     wr_nType(_wr)              (((char *)(_wr))[cnTypeByteOff])
#define set_wr_nType(_wr, _t)           (wr_nType(_wr) = (_t))
#define     wr_nBitsPrefixSz(_wr)      (((char *)(_wr))[cnBitsPrefixSzByteOff])
#define set_wr_nBitsPrefixSz(_wr, _sz)  (wr_nBitsPrefixSz(_wr) = (_sz))
#define     wr_nBitsIndexSz(_wr)       (((char *)(_wr))[cnBitsIndexSzByteOff])
#define set_wr_nBitsIndexSz(_wr, _sz)   (wr_nBitsIndexSz(_wr) = (_sz))

#define     wr_wPrefix(_wr)            ( ((Word_t *)(_wr))[cnPrefixOff])
#define set_wr_wPrefix(_wr, _prefix)    (wr_wPrefix(_wr) = (_prefix))
#define     wr_wKey(_wr)               ( ((Word_t *)(_wr))[cnKeyOff])
#define set_wr_wKey(_wr, _key)          (wr_wKey(_wr) = (_key))

#define     wr_pwPtrs(_wr)             (&((Word_t *)(_wr))[cnPtrsOff])

const int cnBitsPerDigit = 2;

typedef enum { Switch, Leaf } Type_t;

static Word_t *pwRootLast;

void
dump(Word_t *pw, int nWords)
{
    int i;

    printf("pw "OWx, (Word_t)pw);
    for (i = 0; i < nWords; i++)
        printf(" "Owx, pw[i]);
}

void
Dump(Word_t wRoot, int nBitsLeft)
{
    int nType = wr_nType(wRoot);
    int nBitsPrefixSz = wr_nBitsPrefixSz(wRoot);
    int nBitsIndexSz = wr_nBitsIndexSz(wRoot);
    Word_t wKey;
    Word_t wPrefix;
    Word_t *pwPtrs;
    int i;

    if (wRoot == 0)
    {
        return;
    }

    printf(" wr "OWx, wRoot);
    printf(" nBitsLeft %2d", nBitsLeft);
    printf(" nType %d", nType);
    printf(" nBitsPrefixSz %2d", nBitsPrefixSz);
    printf(" nBitsIndexSz %2d", nBitsIndexSz);

    if (nType == Leaf)
    {
        wKey = wr_wKey(wRoot);
    }
    else
    {
        wPrefix = wr_wPrefix(wRoot);
        pwPtrs = wr_pwPtrs(wRoot);
    }

    printf(" ");
    if (nType == Switch)
    {
        for (i = 0; i < nBitsPrefixSz; i++)
        {
            printf(wx, (wPrefix << i) >> (cnBitsPerWord - 1));
        }
        for (i = 0; i < cnBitsPerWord - nBitsPrefixSz; i++)
        {
            printf(".");
        }
    }
    else
    {
        for (i = 0; i < cnBitsPerWord; i++)
        {
            printf(wx, (wKey << i) >> (cnBitsPerWord - 1));
        }
    }

    if (nType == Leaf)
    {
        printf(" wKey "OWx"", wKey);
        printf("\n");

        return;
    }

    printf(" wPrefix "OWx, wPrefix);
    printf(" pwPtrs "OWx, (Word_t)pwPtrs);
    printf("\n");

    for (i = 0; i < EXP(nBitsIndexSz); i++)
    {
        Dump(wr_pwPtrs(wRoot)[i], nBitsLeft - nBitsIndexSz);
    }
}

static Status_t Insert(Word_t *pwRoot, Word_t wKey, int nBitsLeft);

static Status_t
InsertAt(Word_t *pwRoot, Word_t wKey, int nBitsLeft, Word_t wRoot)
{
    Word_t *pw;

    if (wRoot != 0)
    {
        int nBitsPrefixSz;
        int nBitsIndexSz = cnBitsPerDigit;
        Word_t wPrefix;
        Word_t *pwPtrs;
        int nIndex;

        pw = (Word_t *)JudyMalloc(cnPtrsOff + EXP(nBitsIndexSz));
        DBGI(printf("new switch node pw %p\n", pw));
        set_wr_nType(pw, Switch);
        set_wr_nBitsIndexSz(pw, nBitsIndexSz); // Use zero for immediate?

        // prefix (or key) mismatch
        // insert a node at bit where prefix doesn't match
        if (wr_nType(wRoot) == Leaf)
        {
            nBitsLeft = LOG(wKey ^ wr_wKey(wRoot)) + 1; // below branch
        }
        else
        {
            nBitsLeft = LOG(wKey ^ wr_wPrefix(wRoot)) + 1; // below branch
        }
        DBGI(printf("nBitsLeft %d\n", nBitsLeft));
        // align -- for now
        nBitsLeft = ((nBitsLeft + nBitsIndexSz - 1) / nBitsIndexSz)
                        * nBitsIndexSz;

        nBitsPrefixSz = cnBitsPerWord - nBitsLeft;
        set_wr_nBitsPrefixSz(pw, nBitsPrefixSz);
        DBGI(printf("wr_nBitsPrefixSz %d\n", wr_nBitsPrefixSz(pw)));

        // there must be a better way to handle nBitsLeft == cnBitsPerWord
        wPrefix = wKey >> 1;
        wPrefix >>= nBitsLeft - 1;
        wPrefix <<= nBitsLeft - 1;
        wPrefix <<= 1;

        set_wr_wPrefix(pw, wPrefix);
        DBGI(printf("wPrefix "Owx"\n", wr_wPrefix(pw)));

        pwPtrs = wr_pwPtrs(pw);
        memset(pwPtrs, 0, EXP(nBitsIndexSz) * sizeof(*pwPtrs));
        nIndex = (wr_wPrefix(wRoot) << nBitsPrefixSz)
                    >> (cnBitsPerWord - nBitsIndexSz);
        DBGI(printf("old node nIndex %d\n", nIndex));
        pwPtrs[nIndex] = wRoot;
        DBGI(printf("install old node at "Owx"\n", (Word_t)&pwPtrs[nIndex]));

        Insert((Word_t *)&pw, wKey, nBitsLeft);
    }
    else
    {
        DBGI(printf("null\n"));

        // wRoot == 0 insert
        pw = (Word_t *)JudyMalloc(2);
        DBGI(printf("new leaf node pw %p\n", pw));
        set_wr_nType(pw, Leaf);
        set_wr_nBitsPrefixSz(pw, 0);
        set_wr_nBitsIndexSz(pw, 0);
        set_wr_wKey(pw, wKey);
    }

    DBGI(printf("installing pw %p pwRoot %p\n", pw, pwRoot));
    *pwRoot = (Word_t)pw; // install

    return Success;
}

#define INSERT
#include "bli.c"
#undef INSERT
#include "bli.c"

// ****************************************************************************
// JUDY1 FUNCTIONS:

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, P_JE)
{
    return Lookup((Word_t)pcvRoot, wKey, cnBitsPerWord);
}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    int status;

    pwRootLast = (Word_t *)ppvRoot;

    status = Insert((Word_t *)ppvRoot, wKey, cnBitsPerWord);

#if defined(DEBUG_INSERT)
    printf("\n# After Insert(wKey "Owx")\n", wKey);
    Dump((Word_t)*ppvRoot, cnBitsPerWord);
    printf("\n");
#endif // defined(DEBUG_INSERT)

    return status;
}

int Judy1SetArray(PPvoid_t PPArray,
    Word_t Count, const Word_t * const PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int Judy1Unset( PPvoid_t PPArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(2); }

Word_t Judy1Count(Pcvoid_t PArray, Word_t Index1, Word_t Index2, P_JE)
{ printf("\nJudy1Count\n\n"); exit(1); }

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

