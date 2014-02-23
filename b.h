
#include <stdio.h>  // printf
#include <string.h> // memcpy
#include <assert.h> // NDEBUG must be defined before including assert.h.
#include "Judy.h"   // Word_t, JudyMalloc, ...

#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

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
const Word_t cwListPopCntMax = 255;

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

INLINE Status_t Lookup(Word_t wRoot, Word_t wKey);

INLINE Status_t Insert(Word_t *pwRoot, Word_t wKey, int nBitsLeft);
INLINE Status_t Remove(Word_t *pwRoot, Word_t wKey, int nBitsLeft);

#endif // ( ! defined(_B_H_INCLUDED) )
