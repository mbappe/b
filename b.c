
#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#include "Judy.h"   // JudyMalloc, ...

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

const int log_bpB = 3;

#if        defined(_WIN64)

//typedef unsigned long long Word_t;
#define EXP(_x)  (1LL << (_x))
#define _f0wx  "%016llx"
#define _fwx      "%llx"

#else   // defined(_WIN64)

//typedef unsigned long Word_t;
#define EXP(_x)  (1L << (_x))

#if        defined(__LP64__)
#define _f0wx  "%016lx"
#define _fwx      "%lx"
#else   // defined(__LP64__)
#define _f0wx  "%08lx"
#define _fwx     "%lx"
#endif  // defined(__LP64__)

#endif  // defined(_WIN64)

#if       defined(__LP64__) || defined(_WIN64)
const int log_Bpw = 3;
#else   // defined(__LP64__) || defined(_WIN64)
const int log_Bpw = 2;
#endif  // defined(__LP64__) || defined(_WIN64)

#define mask(_pow)  ((_pow) - 1)

const int mm = (EXP(log_Bpw + 1) - 1);

const int cnDigitsAtBottom = 1;

// digits per node -- later
#define wr_dpn(_wR)  (1)
// bits per digit; log Bytes per word; log bits per Byte
//const int bpd = (log_Bpw + log_bpB);
const int bpd = 8;
// bits per word
const int bpw = EXP(log_Bpw + log_bpB);
// digits per word
const int dpw = ((bpw + bpd - 1) / bpd);

typedef struct {
    Word_t nd_wKeyPop; // key prefix and population count
    Word_t nd_awRoot[EXP(bpd)];
} Node_t;

#define wr_dl(_wr)                (int) ((_wr) &  mm)
#define wr_pw(_wr)           ((Word_t *)((_wr) & ~mm))
#define wr_pn(_wr)           ((Node_t *)((_wr) & ~mm))

#define wr_set(_wr, _pw, _dl)  ((_wr) = (Word_t)(_pw) | (_dl))

#define wr_nDigitsLeft(_wr)       (int)(((_wr) &  mm) + cnDigitsAtBottom)

#define wr_set_nDigitsLeft(_wr, _pw, _nDigitsLeft) \
    ((_wr) = (Word_t)(_pw) | (_nDigitsLeft) - cnDigitsAtBottom)

typedef enum { Failure = 0, Success = 1 } Status_t;

#define lf_cnt(_pw)        (_pw)[0]
#define lf_pwKeys(_pw)      (&(_pw)[1])

#define lf_cnt_set(_pw, _cnt)  ((_pw)[0] = (_cnt))

#define pn_wPrefix(_pn)  ((_pn)->nd_wKeyPop)

#define nd_set_key(_pNd, _wKey, _nDigitsLeft) \
    ((_pNd)->nd_wKeyPop = (_wKey) & mask(EXP((_nDigitsLeft) * bpd)))

#define copy(_tgt, _src, _cnt) \
    memcpy((_tgt), (_src), sizeof(*(_src)) * (_cnt))

static const Word_t wLeafPopCntMax = EXP(bpd);

static Word_t *pwRootLast;

INLINE Word_t *
NewLeaf(Word_t wPopCnt)
{
    Word_t *pw = (Word_t *)JudyMalloc(wPopCnt + 1);

    lf_cnt_set(pw, wPopCnt);

    //printf("New pwLeaf %p wPopCnt "_f0wx"\n", pw, wPopCnt);

    return pw;
}

INLINE void
OldLeaf(Word_t *pw)
{
    //printf("Old pwLeaf %p\n", pw);

    JudyFree(pw, lf_cnt(pw) + 1);
}

INLINE Node_t *
NewNode(Word_t wKey, int nDigitsLeft)
{
    Node_t *pNd = (Node_t *)JudyMalloc(sizeof(Node_t) / sizeof(Word_t));

    //printf("NewNode pNd %p\n", pNd);

    nd_set_key(pNd, wKey, nDigitsLeft);

    memset((Word_t *)pNd, 0, sizeof(*pNd));

    return pNd;
}

#if 0
INLINE void
OldNode(Node_t *pNd)
{
    JudyFree((Word_t *)pNd, sizeof(*pNd) / sizeof(Word_t));
}
#endif

void
dump(Word_t *pw, int nWords)
{
    int i;

    printf("pw "_f0wx, (Word_t)pw);
    for (i = 0; i < nWords; i++)
        printf(" "_f0wx, pw[i]);
}

void
Dump(Word_t wRoot, int nDigitsLeft)
{
    Word_t wPrefix;
    int i, j;

    if (wRoot == 0)
    {
        return;
    }

    printf(" wr 0x"_f0wx, wRoot);

    if (wr_dl(wRoot) == mm)
    {
        Word_t wPopCnt = lf_cnt(wr_pw(wRoot));

        printf(" ");
        if (wPopCnt > 4)
        {
            dump(wr_pw(wRoot), 5); printf(" ...");
        }
        else
        {
            dump(wr_pw(wRoot), wPopCnt + 1);
        }
        printf("\n");

        return;
    }

    wPrefix = pn_wPrefix(wr_pn(wRoot)) >> (wr_nDigitsLeft(wRoot) * bpd);
    wPrefix >>= bpd;

    printf(" nd wKeyPop 0x"_f0wx" wr_nDigitsLeft %d\n",
        wr_pn(wRoot)->nd_wKeyPop, wr_nDigitsLeft(wRoot));

    for (i = 0; i < EXP(bpd); i++)
    {
        if (wr_pn(wRoot)->nd_awRoot[i] != 0)
        {
            for (j = dpw; j > nDigitsLeft; j--)
            {
                printf("..");
            }
            printf("%02x", i);
            for (j = 1; j < nDigitsLeft; j++)
            {
                printf("..");
            }

            Dump(wr_pn(wRoot)->nd_awRoot[i], nDigitsLeft - 1);
        }
    }
#if 0
    printf("  <-     wr 0x"_f0wx, wRoot);
    printf(" nd wPrefix 0x"_f0wx" wr_nDigitsLeft %d\n",
        wPrefix, wr_nDigitsLeft(wRoot));
#endif
}

INLINE Status_t Insert(Word_t *pwRoot, Word_t wKey, int nDigitsLeft);

INLINE Status_t
InsertInLeaf(Word_t *pwRoot, Word_t wKey, int nDigitsLeft)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwLeaf;
    Word_t wPopCnt;
    Word_t *pwKeys;
    Node_t *pNd;
    int w;

    if (wRoot == 0)
    {
        // allocate new list leaf and init pop count in the first word
        pwLeaf = NewLeaf(/* nKeys */ 1);
        pwKeys = lf_pwKeys(pwLeaf); // get a pointer to the keys
        pwKeys[0] = wKey;
        wr_set(*pwRoot, /* pw */ pwLeaf, /* dl */ mm); // install new list

        return Success;
    }

    pwLeaf = wr_pw(wRoot); // pointer to old leaf
    wPopCnt = lf_cnt(pwLeaf); // population count of old leaf
    pwKeys = lf_pwKeys(pwLeaf); // list of keys in old leaf

    if ((wPopCnt < wLeafPopCntMax) || (nDigitsLeft == cnDigitsAtBottom))
    {
        // allocate space for bigger list and init pop count
        Word_t *pwLeafNew = NewLeaf( /* nKeys */ wPopCnt + 1);
        Word_t *pwKeysNew = lf_pwKeys(pwLeafNew);
        pwKeys = lf_pwKeys(pwLeaf); // keys in old list
        copy(pwKeysNew, pwKeys, wPopCnt); // copy keys
        pwKeysNew[wPopCnt] = wKey; // add new key to end
        wr_set(*pwRoot, /* pw */ pwLeafNew, /* dl */ mm); // install list
        OldLeaf(pwLeaf); // free old list

        goto done;
    }

    // leaf is full; insert a node

    pNd = NewNode(wKey, nDigitsLeft);
    wr_set_nDigitsLeft(wRoot, (Word_t *)pNd, nDigitsLeft);

    for (w = 0; w < wPopCnt; w++)
    {
        Insert(&wRoot, pwKeys[w], nDigitsLeft);
    }

    Insert(&wRoot, wKey, nDigitsLeft);

    *pwRoot = wRoot; // install new
    OldLeaf(pwLeaf); // free old

done:

#if defined(DEBUG_INSERT)
    printf("\n\n# After InsertInLeaf\n        "); Dump(*pwRootLast, dpw);
    printf("\n");
#endif // defined(DEBUG_INSERT)

    return Success;
}

#define INSERT
#include "li.c"
#undef INSERT
#include "li.c"

// ****************************************************************************
// JUDY1 FUNCTIONS:

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, P_JE)
{
    return Lookup((Word_t)pcvRoot, wKey, dpw);
}

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    int status;

    pwRootLast = (Word_t *)ppvRoot;

    status = Insert((Word_t *)ppvRoot, wKey, dpw);

#if defined(DEBUG_INSERT)
    printf("\n\n# After Insert\n        "); Dump((Word_t)*ppvRoot, dpw);
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
{ printf("\nJudy1Count\n\n"); exit(0); }

int Judy1ByCount(Pcvoid_t PArray, Word_t Count, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t Judy1FreeArray(PPvoid_t PPArray, P_JE)
{ printf("\nJudy1FreeArray\n\n"); exit(0); }

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

