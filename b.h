
//
// (cnBitsPerDigit, cnDigitsAtBottom, cwListPopCntMax)
//
// Quick tested:
// -m32 -O2 -g -Wall -Werror
// JUDY_DEFINES += -DRAM_METRICS -DJUDYB -UGUARDBAND -UNDEBUG -DEBUG
// B_DEFINES += -DSKIP_LINKS -DSKIP_PREFIX_CHECK -DSORT_LISTS
// (cnBitsPerDigit, cnDigitsAtBottom, cwListPopCntMax)
// ( 0, X1,  X1), ( 1, 26,   1), ( 2, 10,   1), ( 3,  5,   1), ( 4,  2,   1)
// ( 5,  1,   1), ( 6,  0,   1), ( 8,  0,   1),
// (16,  0,   1) malloc error; can't get full pop with cnDigitsAtBottom = 0,
// ( 6,  1,   1), ( 7,  1,   1), ( 8,  1,   1), (16,  1,   1), (20,  1,   1),
// ( 1, 31,   1), ( 2, 15,   1), ( 3, 10,   1), ( 4,  7,   1), ( 5, 6,   1),
// ( 6,  5,   1), ( 8,  3,   1), ( 7,  4,   1),
// ( 1, 26,   0), ( 8,  3,   0),
// ( 1, 26, 999), ( 5,  2, 999),
// -USORT_LISTS ( 5,  1, bPD),
// -USORT_LISTS -DMIN_MAX_LISTS ( 5,  1, bPD),
// -USORT_LISTS -DMIN_MAX_LISTS ( 8,  1, bPD),

// Choose bits per digit.  Any value from zero through max is ok.
// Zero is one big bitmap.  Max is where malloc fails when we can't allocate
// the one big switch implied by cnBitsPerDigit of more than half a word.
// Default is cnLogBitsPerWord because a bitmap is the size of a word when
// cnDigitsAtBottom is one and we can embed the bitmap.
#define cnBitsPerDigit  (cnLogBitsPerWord)

// Choose bottom.
// Bottom is where Bitmap is created.  Maybe we should change the meaning.
// Can we support bits at bottom instead of digits at bottom and count digits
// up (and maybe down) from there?
// Minimum digits at bottom:  (cnDigitsPerWord - cnMallocMask + 1)
// Maximum digits at bottom:  (cnDigitsPerWord - 1)
// Min and max are good values to test.
// Zero works (as long as it is not smaller than the minimum) but max pop
// cannot be reached because we never transition to bitmap.
// Default is one because a bitmap is the size of a word when cnBitsPerDigit
// is cnLogBitsPerWord and we can embed the bitmap.
#define cnDigitsAtBottom  (1U)

// Choose max list length.
// 0, 1, 2, 3, 4 and greater than 255 are all good values to test.
// Default is EXP(cnBitsPerDigit + 1) to try to offset the memory cost of a
// new switch when max list length is reached.
// But it doesn't work because we can end up with a new switch at every
// depth with only the bottom list having more than one key.
// We could vary the max length based on depth or be even more sophisticated.
#define cwListPopCntMax  EXP(cnBitsPerDigit + 1)
//#define cwListPopCntMax  0L

// Choose features.
// SKIP_LINKS, SKIP_PREFIX_CHECK, SORT_LISTS
// -UNDEBUG, RAM_METRICS, GUARDBAND

// To do:
//
// - Constraints: cache size; goal is only one cache miss per get;
//   only bottom level can be out of cache; 
//   memory usage must be no more than two words per key;
//   if list leaf must be larger than cache line size, then might as
//   well add a branch
// - 3MB/4B/link ~ 750,000 links at full pop ~ 375,000 Bitmaps ~ 18-19 bits
//   decoded by switches; 13-14 bits per Bitmap. 
// - What about tlb entries?
// - log(xor) for prefix check
// - nDigitsLeftRoot - nDigitsLeft == -1 means double the size
//   of the index for the next switch
// - nDigitsLeftRoot - nDigitsLeft == -2 means quadruple the
//   size of the index for the next switch
// - nDigitsLeftRoot - nDigitsLeft == -3 means times eight the
//   size of the index for the next switch
// - 1-byte, 2-byte, 4-byte lists
// - pop count
// - special variant of Lookup for undoing pop count increment
//   of failed insert (or decrement of failed remove)
// - list switches?  for wide switches?
// - Bitmap switches?  for wide switches?

#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#include "Judy.h"   // Word_t, JudyMalloc, ...

#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

#if defined RAM_METRICS
#define METRICS(x)  (x)
#else // defined RAM_METRICS
#define METRICS(x)
#endif // defined RAM_METRICS

#if defined SEARCH_METRICS
#define SMETRICS(x)  (x)
#else // defined SEARCH_METRICS
#define SMETRICS(x)
#endif // defined SEARCH_METRICS

#if defined(DEBUG_INSERT)
#define DBGI(x)  if (wInserts >= cwDebugThreshold) (x)
#else // defined(DEBUG_INSERT)
#define DBGI(x)
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_LOOKUP)
#define DBGL(x)  (x)
#else // defined(DEBUG_LOOKUP)
#define DBGL(x)
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_REMOVE)
#define DBGR(x)  if (wInserts >= cwDebugThreshold) (x)
#else // defined(DEBUG_REMOVE)
#define DBGR(x)
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_MALLOC)
#define DBGM(x)  if (wInserts >= cwDebugThreshold) (x)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

#if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || defined(DEBUG_MALLOC)
#if !defined(DEBUG)
#define DEBUG
#endif // !defined(DEBUG)
#endif // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...

#if defined(DEBUG)
#define INLINE
#define DBG(x)  (x)
#define cwDebugThreshold  0ULL
#else // defined(DEBUG)
#define INLINE static inline
#define DBG(x)
#endif // defined(DEBUG)

#if defined(_WIN64)
//typedef unsigned long long Word_t;
#define EXP(_x)  (1ULL << (_x))
#define Owx   "%016llx"
#define OWx "0x%016llx"
#define wx "%llx"
#define wd "%lld"
#else // defined(_WIN64)
//typedef unsigned long Word_t;
#define EXP(_x)  (1UL << (_x))
#if defined(__LP64__)
#define Owx   "%016lx"
#define OWx "0x%016lx"
#else // defined(__LP64__)
#define Owx   "%08lx"
#define OWx "0x%08lx"
#endif // defined(__LP64__)
#define wx "%lx"
#define wd "%ld"
#endif // defined(_WIN64)

// 64 - 1 - leading zeros
#define LOG(x)  ((Word_t)63 - __builtin_clzll(x))
#define MASK(_x)  ((_x) - 1)

#define COPY(_tgt, _src, _cnt) \
    memcpy((_tgt), (_src), sizeof(*(_src)) * (_cnt))

#define MOVE(_tgt, _src, _cnt) \
    memmove((_tgt), (_src), sizeof(*(_src)) * (_cnt))

#define SET(_p, _v, _cnt) \
    memset((_p), (_v), sizeof(*(_p)) * (_cnt))

// Data structure constants and macros.

#define     wr_nType(_wr)         ((_wr) & cnMallocMask)
#define set_wr_nType(_wr, _type)  ((_wr) = ((_wr) & ~cnMallocMask) | (_type))

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#define     wr_pwr(_wr)          ((Word_t *)((_wr) & ~cnMallocMask))
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#define     wr_pwr(_wr)          ((Word_t *)(_wr))
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#define set_wr_pwr(_wr, _pwr) \
    ((_wr) = ((_wr) & cnMallocMask) | (Word_t)(_pwr))

#define set_wr(_wr, _pwr, _type)  ((_wr) = (Word_t)(_pwr) | (_type))

#define  tp_to_nDigitsLeft(_tp)   ((_tp) + cnDigitsAtBottom - 1)
#define  nDigitsLeft_to_tp(_nDL)  ((_nDL) + 1 - cnDigitsAtBottom)

#define     wr_nDigitsLeft(_wr)     (tp_to_nDigitsLeft(wr_nType(_wr)))
#define set_wr_nDigitsLeft(_wr, _nDL) \
    (set_wr_nType((_wr), nDigitsLeft_to_tp((_nDL) + 1 - cnDigitsAtBottom)))

#define     wr_bIsSwitch(_wr)          (wr_nType(_wr) != List)

#define     wr_bIsSwitchDL(_wr, _tp, _nDL) \
    ((_tp) = wr_nType(_wr), (_nDL) = tp_to_nDigitsLeft(_tp), (_tp))

#define     pwr_nBitsIndexSz(_pwr)       (cnBitsPerDigit)
#define set_pwr_nBitsIndexSz(_pwr, _sz)  (assert((_sz) == cnBitsPerDigit))

// methods for Switch (and aliases)

#define wPrefixPopMask(_nDL) \
    ((((_nDL) == cnDigitsPerWord) \
        ? (Word_t)-1 : (EXP((_nDL) * cnBitsPerDigit)) - (Word_t)1))

#define wPrefixPopMaskNotAtTop(_nDL) \
    ((EXP((_nDL) * cnBitsPerDigit)) - 1)

#define sw_wPrefixPop(_psw)  (((Switch_t *)(_psw))->sw_wPrefixPop)
#define sw_wPrefix(_psw, _nDL)  (sw_wPrefixPop(_psw) & ~wPrefixPopMask(_nDL))
#define sw_wPopCnt(_psw, _nDL)  (sw_wPrefixPop(_psw) &  wPrefixPopMask(_nDL))

#define sw_wPrefixNotAtTop(_psw, _nDL) \
    (sw_wPrefixPop(_psw) & ~wPrefixPopMaskNotAtTop(_nDL))

#define sw_wPopCntNotAtTop(_psw, _nDL) \
    (sw_wPrefixPop(_psw) &  wPrefixPopMaskNotAtTop(_nDL))

#define set_sw_wPrefix(_psw, _nDL, _key) \
    (((Switch_t *)(_psw))->sw_wPrefixPop \
        = ((sw_wPrefixPop(_psw) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_sw_wPopCnt(_psw, _nDL, _cnt) \
    (((Switch_t *)(_psw))->sw_wPrefixPop \
        = ((sw_wPrefixPop(_psw) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_sw_wPrefixNotAtTop(_psw, _nDL, _key) \
    (((Switch_t *)(_psw))->sw_wPrefixPop \
        = ((sw_wPrefixPop(_psw) & wPrefixPopMaskNotAtTop(_nDL)) \
            | ((_key) & ~wPrefixPopMaskNotAtTop(_nDL))))

#define set_sw_wPopCntNotAtTop(_psw, _nDL, _cnt) \
    (((Switch_t *)(_psw))->sw_wPrefixPop \
        = ((sw_wPrefixPop(_psw) & ~wPrefixPopMaskNotAtTop(_nDL)) \
            | ((_cnt) & wPrefixPopMaskNotAtTop(_nDL))))

#define     sw_wKey                   sw_wPrefix
#define set_sw_wKey               set_sw_wPrefix

#define     pwr_wPrefixPop               sw_wPrefixPop
#define     pwr_wKey                  sw_wKey
#define set_pwr_wKey              set_sw_wKey
#define     pwr_wPrefix               sw_wKey
#define set_pwr_wPrefix           set_sw_wKey

#define     pwr_pwRoots(_pwr)  (((Switch_t *)(_pwr))->sw_awRoots)

// These assume List == 0.
#define     wr_pwKeys(_wr)   (&((Word_t *)(_wr))[1])

#define     ls_wPopCnt(_ls)        (((Word_t *)(_ls))[0] & 0xffff)
#define set_ls_wPopCnt(_ls, _cnt) \
    (((Word_t *)(_ls))[0] \
        = (((Word_t *)(_ls))[0] & ~0xffff) | ((_cnt) & 0xffff))

#define     ls_wLen(_ls)        (((Word_t *)(_ls))[0] >> 16)
#define set_ls_wLen(_ls, _len) \
    (((Word_t *)(_ls))[0] \
        = (((Word_t *)(_ls))[0] & 0xffff) | ((_len) << 16))

// Assume List == 0, i.e. wRoot is a valid pointer with no mask.
#define     wr_ls_wPopCnt(_wr)        (ls_wPopCnt(_wr))

#define     ls_pwKeys(_ls)    (&(_ls)[1])
#define     pwr_pwKeys(_pwr)  (ls_pwKeys(_pwr))

#define BitmapByteNum(_key)  ((_key) >> cnLogBitsPerByte)

#define BitmapByteMask(_key)  (1 << ((_key) % cnBitsPerByte))

#define BitIsSetInWord(_w, _b)  (((_w) & (1 << (_b))) != 0)

#define SetBitInWord(_w, _b)  ((_w) |=  (1 << (_b)))
#define ClrBitInWord(_w, _b)  ((_w) &= ~(1 << (_b)))

#define TestBit(_pBitmap, _key) \
    ((((char *)(_pBitmap))[BitmapByteNum(_key)] & BitmapByteMask(_key)) \
        != 0)

#define BitIsSet  TestBit

#define SetBit(_pBitmap, _key) \
    (((char *)(_pBitmap))[BitmapByteNum(_key)] |=  BitmapByteMask(_key))
#define ClrBit(_pBitmap, _key) \
    (((char *)(_pBitmap))[BitmapByteNum(_key)] &= ~BitmapByteMask(_key))

#define BitTestAndSet(_pBitmap, _key, _bSet) \
    (((_bSet) = TestBit((_pBitmap), (_key))), \
        BitSet((_pBitmap), (_key)), (_bSet))

#define cnLogBitsPerByte  (3U)
#define cnBitsPerByte  (EXP(cnLogBitsPerByte))

#if defined(__LP64__) || defined(_WIN64)
#define cnLogBytesPerWord  (3U)
#else // defined(__LP64__) || defined(_WIN64)
#define cnLogBytesPerWord  (2U)
#endif // defined(__LP64__) || defined(_WIN64)

#define cnBytesPerWord  (EXP(cnLogBytesPerWord))
#define cnLogBitsPerWord  (cnLogBytesPerWord + cnLogBitsPerByte)
#define cnBitsPerWord  (EXP(cnLogBitsPerWord))
#define cnMallocMask  ((cnBytesPerWord * 2) - 1)

#define cnBitsAtBottom  (cnDigitsAtBottom * cnBitsPerDigit)

#define cnDigitsPerWord  (((cnBitsPerWord - 1) / cnBitsPerDigit) + 1)

typedef enum { Failure = 0, Success = 1 } Status_t;

#if (cnBitsPerDigit != 0)

typedef enum { List = 0 } Type_t;

typedef struct {
    Word_t sw_awRoots[EXP(cnBitsPerDigit)];
    Word_t sw_wPrefixPop;
} Switch_t;

Status_t Lookup(Word_t   wRoot, Word_t wKey);
Status_t Insert(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);
Status_t Remove(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);

Status_t InsertGuts(Word_t *pwRoot,
    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

Status_t RemoveGuts(Word_t *pwRoot,
    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

Word_t OldBitmap(Word_t wRoot);

#endif // (cnBitsPerDigit != 0)

Word_t wInserts;

#endif // ( ! defined(_B_H_INCLUDED) )

