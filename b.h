
//
// (cnBitsPerDigit, cnDigitsAtBottom)
//
// ( 1, 31) all the way, -s0 -S1 all the way
// ( 1, 30) all the way, -s0 -S1 all the way
// ( 2, 15) all the way, -s0 -S1 all the way
// (16,  1) all the way, -s0 -S1 all the way
// (16,  0) Bus error    15853, -s0 -S1 Bus error 912010843
// ( 8,  0) Bus error  3981080, -s0 -S1 Bus error 912010843
// ( 6,  0) Bus error 14454402, -s0 -S1 Bus error 912010843
//

// Choose bits per digit.
#define cnBitsPerDigit  (5U)

// Choose bottom.
// Bottom is where bitmap is created.
// Can we support bits at bottom instead of digits at bottom?
// Minimum digits at bottom:  (cnDigitsPerWord - cnMallocMask + 1)
// Maximum digits at bottom:  (cnDigitsPerWord - 1)
#define cnDigitsAtBottom  (2U)

// To do:
//
// - Constraints: cache size; goal is only one cache miss per get;
//   only bottom level can be out of cache; 
//   memory usage must be no more than two words per key;
//   if list leaf must be larger than cache line size, then might as
//   well add a branch
// - 3MB/4B/link ~ 750,000 links at full pop ~ 375,000 bitmaps ~ 18-19 bits
//   decoded by switches; 13-14 bits per bitmap. 
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
// - bitmap switches?  for wide switches?

#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#include "Judy.h"   // Word_t, JudyMalloc, ...

#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

#if defined RAMMETRICS
#define METRICS(x)  (x)
#else // defined RAMMETRICS
#define METRICS(x)
#endif // defined RAMMETRICS

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
#define INLINE
#else // defined(DEBUG)
#define INLINE static inline
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

#define     wr_pwr(_wr)          ((Word_t *)((_wr) & ~cnMallocMask))
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
    ((((_nDL) == cnDigitsPerWord) ? 0 : EXP((_nDL) * cnBitsPerDigit)) - 1)

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

#define     ls_wPopCnt(_ls)        ((_ls)[0])
#define set_ls_wPopCnt(_ls, _cnt)  ((_ls)[0] = (_cnt))

#define     ls_pwKeys(_ls)    (&(_ls)[1])
#define     pwr_pwKeys(_pwr)  (ls_pwKeys(_pwr))

#define BitMapByteNum(_key)  ((_key) >> cnLogBitsPerByte)

#define BitMapByteMask(_key)  (1 << ((_key) % cnBitsPerByte))

#define BitIsSetInWord(_w, _b)  (((_w) & (1 << (_b))) != 0)

#define SetBitInWord(_w, _b)  ((_w) |= (1 << (_b)))

#define TestBit(_pBitMap, _key) \
    ((((char *)(_pBitMap))[BitMapByteNum(_key)] & BitMapByteMask(_key)) \
        != 0)

#define BitIsSet  TestBit

#define SetBit(_pBitMap, _key) \
    (((char *)(_pBitMap))[BitMapByteNum(_key)] |= BitMapByteMask(_key))

#define BitTestAndSet(_pBitMap, _key, _bSet) \
    (((_bSet) = TestBit((_pBitMap), (_key))), \
        BitSet((_pBitMap), (_key)), (_bSet))

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

// Bus error at 912,010,843 with 255, 256 or 1024.
// None with 128, 192, 224, 240.
//const Word_t cwListPopCntMax = EXP(cnBitsPerDigit);
//const Word_t cwListPopCntMax = 255;
#define cwListPopCntMax  (240L)

typedef enum { Failure = 0, Success = 1 } Status_t;

#if cnBitsPerDigit != 0

typedef enum { List = 0 } Type_t;

typedef struct {
    Word_t sw_awRoots[EXP(cnBitsPerDigit)];
    Word_t sw_wPrefixPop;
} Switch_t;

Status_t Lookup(Word_t wRoot, Word_t wKey);
Status_t Insert(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);
Status_t Remove(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);

Status_t InsertGuts(Word_t *pwRoot,
    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

Status_t RemoveGuts(Word_t *pwRoot,
    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

#endif // cnBitsPerDigit != 0

#endif // ( ! defined(_B_H_INCLUDED) )

