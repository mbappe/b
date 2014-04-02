
#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

#if defined(SKIP_PREFIX_CHECK) || defined(NO_UNNECESSARY_PREFIX)
#if ! defined(SKIP_LINKS)
#define SKIP_LINKS
#endif // ! defined(SKIP_LINKS)
#endif // defined(SKIP_PREFIX_CHECK) || defined(NO_UNNECESSARY_PREFIX)

#if defined(BM_IN_LINK) || defined(BM_SWITCH_FOR_REAL)
#if ! defined(BM_SWITCH)
#define BM_SWITCH
#endif // ! defined(BM_SWITCH)
#endif // defined(BM_IN_LINK) || defined(BM_SWITCH_FOR_REAL)

// Choose bits per digit.  Any value from zero through max is ok.
// Zero is one big bitmap.  Max is where malloc fails when we can't allocate
// the one big switch implied by cnBitsPerDigit of more than half a word.
// Default is cnLogBitsPerWord because a bitmap is the size of a word when
// cnDigitsAtBottom is one and we can embed the bitmap.
#if !defined(cnBitsPerDigit)
#define cnBitsPerDigit  (cnLogBitsPerWord)
#endif // !defined(cnBitsPerDigit)

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
// I think I should change this to be relative to the minimum digits at
// bottom based on cnBitsPerDigit and cnBitsPerWord.
#if !defined(cnDigitsAtBottom)
#define cnDigitsAtBottom  1
#endif // !defined(cnDigitsAtBottom)

// Choose max list length.
// 0, 1, 2, 3, 4 and greater than 255 are all good values to test.
// Default is EXP(cnBitsPerDigit + 1) to try to offset the memory cost of a
// new switch when max list length is reached.
// But it doesn't work because we can end up with a new switch at every
// depth with only the bottom list having more than one key.
// We could vary the max length based on depth or be even more sophisticated.
#if !defined(cwListPopCntMax)
// ListLeaf_t has only one byte to store number of words in list.
#if (cnBitsPerDigit <= 8)
#define cwListPopCntMax  (EXP(cnBitsPerDigit) / 2)
#else // (cnBitsPerDigit <= 8)
#define cwListPopCntMax  250
#endif // (cnBitsPerDigit <= 8)
#endif // !defined(cwListPopCntMax)

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

#if defined(_WIN64)
#define WORD_ONE  1ULL
#else // defined(_WIN64)
#define WORD_ONE  1UL
#endif // defined(_WIN64)

#define EXP(_x)  (WORD_ONE << (_x))

// Count leading zeros.
// __builtin_clzll is undefined for zero which allows the compiler to use bsr.
// Actual x86 clz instruction is defined for zero.
// This LOG macro is undefined for zero.
#define LOG(_x)  ((Word_t)63 - __builtin_clzll(_x))
#define MASK(_x)  ((_x) - 1)

#define cnLogBitsPerByte  3
#define cnBitsPerByte  (EXP(cnLogBitsPerByte))

#if !defined(cnBitsPerWord)
#if defined(__LP64__) || defined(_WIN64)
#define cnBitsPerWord  64
#else // defined(__LP64__) || defined(_WIN64)
#define cnBitsPerWord  32
#endif // defined(__LP64__) || defined(_WIN64)
#endif // !defined(cnBitsPerWord)

#if (cnBitsPerWord == 64)
#define cnLogBytesPerWord  3
#else // (cnBitsPerWord == 64)
#define cnLogBytesPerWord  2
#endif // (cnBitsPerWord == 64)

#define cnBytesPerWord  (EXP(cnLogBytesPerWord))
#define cnLogBitsPerWord  (cnLogBytesPerWord + cnLogBitsPerByte)
#define cnMallocMask  ((cnBytesPerWord * 2) - 1)

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
#if (cwDebugThreshold != 0)
#define DBGI(x)  if (wInserts >= cwDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGI(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_INSERT)
#define DBGI(x)
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_LOOKUP)
#if (cwDebugThreshold != 0)
#define DBGL(x)  if (wInserts >= cwDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGL(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_LOOKUP)
#define DBGL(x)
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_REMOVE)
#if (cwDebugThreshold != 0)
#define DBGR(x)  if (wInserts >= cwDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGR(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_REMOVE)
#define DBGR(x)
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_MALLOC)
#if (cwDebugThreshold != 0)
#define DBGM(x)  if (wInserts >= cwDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGM(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

#if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || defined(DEBUG_MALLOC)
#if !defined(DEBUG)
#define DEBUG
#endif // !defined(DEBUG)
#endif // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...

#if defined(DEBUG)
#define DBG(x)  (x)
#define cwDebugThreshold  0ULL
#else // defined(DEBUG)
#define DBG(x)
#endif // defined(DEBUG)

#if !defined(Owx)
#if defined(_WIN64)
#define Owx   "%016llx"
#define OWx "0x%016llx"
#else // defined(_WIN64)
#if defined(__LP64__)
#define Owx   "%016lx"
#define OWx "0x%016lx"
#else // defined(__LP64__)
#define Owx   "%08lx"
#define OWx "0x%08lx"
#endif // defined(__LP64__)
#endif // defined(_WIN64)
#endif // !defined(Owx)

#if defined(_WIN64)
#define _fw  "ll"
#else // defined(_WIN64)
#define _fw  "l"
#endif // defined(_WIN64)

// Do integer division, but round up instead of down.
#define DIV_UP(_idend, _isor)  (((_idend) + (_isor) - 1) / (_isor))
#define ALIGN_UP(_x, _a)  (DIV_UP((_x), (_a)) * (_a))

#define OFFSET_OF(_type, _field) ((size_t)&((_type *)NULL)->_field)
#define STRUCT_OF(_p, _type, _field) \
    ((_type *)((char *)(_p) - OFFSET_OF(_type, _field)))

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

#define     wr_tp_pwr(_wr, _tp)          ((Word_t *)((_wr) ^ (_tp)))
#define     wr_pwr(_wr)                  ((Word_t *)((_wr) & ~cnMallocMask))

#define set_wr_pwr(_wr, _pwr) \
                ((_wr) = ((_wr) & cnMallocMask) | (Word_t)(_pwr))

#define set_wr(_wr, _pwr, _type)  ((_wr) = (Word_t)(_pwr) | (_type))

#define tp_to_nDigitsLeft(_tp)   ((_tp) + cnDigitsAtBottom - 1)
#define nDigitsLeft_to_tp(_nDL)  ((_nDL) + 1 - cnDigitsAtBottom)

#define     wr_nDigitsLeft(_wr)     (tp_to_nDigitsLeft(wr_nType(_wr)))
#define set_wr_nDigitsLeft(_wr, _nDL) \
    (set_wr_nType((_wr), nDigitsLeft_to_tp((_nDL) + 1 - cnDigitsAtBottom)))

#define     tp_bIsSwitch(_tp)          ((_tp) != 0)
#define     wr_bIsSwitch(_wr)          (tp_bIsSwitch(wr_nType(_wr)))

#define     wr_bIsSwitchDL(_wr, _tp, _nDL) \
   ((_tp) = wr_nType(_wr), (_nDL) = tp_to_nDigitsLeft(_tp), tp_bIsSwitch(_tp))

#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#define     wr_pwr(_wr)                  ((Word_t *)(_wr))
#define set_wr_pwr(_wr, _pwr)            ((_wr) = (Word_t)(_pwr))

#define     wr_tp_pwr(_wr, _tp)          ((Word_t *)(_wr))

#define set_wr(_wr, _pwr, _tp)         ((_wr) = (Word_t)(_pwr))

#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#define     pwr_nBitsIndexSz(_pwr)       (cnBitsPerDigit)
#define set_pwr_nBitsIndexSz(_pwr, _sz)  (assert((_sz) == cnBitsPerDigit))

// methods for Switch (and aliases)

#define wPrefixPopMask(_nDL) \
    ((((_nDL) == cnDigitsPerWord) \
        ? (Word_t)-1 : (EXP((_nDL) * cnBitsPerDigit)) - (Word_t)1))

#define wPrefixPopMaskNotAtTop(_nDL) \
    ((EXP((_nDL) * cnBitsPerDigit)) - 1)

#define w_wPrefix(_w, _nDL)  ((_w) & ~wPrefixPopMask(_nDL))
#define w_wPopCnt(_w, _nDL)  ((_w) &  wPrefixPopMask(_nDL))

#define w_wPrefixNotAtTop(_w, _nDL)  ((_w) & ~wPrefixPopMaskNotAtTop(_nDL))
#define w_wPopCntNotAtTop(_w, _nDL)  ((_w) &  wPrefixPopMaskNotAtTop(_nDL))

// It is a bit of a bummer that the macros for extracting fields that might
// be in the switch or in the link depending on ifdefs require a mask and
// and extra dereference in one of the cases if the only parameter is pwRoot.
// It would be nice if the compiler could optimize them out, but I'm not
// optimistic so I chose to make both pwRoot and pwr be parameters.
// Only one will be used, for each field, in the compiled code, depending
// on ifdefs.
#if defined(PP_IN_LINK)
#define PWR_wPrefixPop(_pwRoot, _pwr) \
    (STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_wPrefixPop)
#else // defined(PP_IN_LINK)
#define PWR_wPrefixPop(_pwRoot, _pwr)  (((Switch_t *)(_pwr))->sw_wPrefixPop)
#endif // defined(PP_IN_LINK)

#define PWR_wPrefix(_pwRoot, _pwr, _nDL) \
    (w_wPrefix(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (w_wPopCnt(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPrefixNotAtTop(_pwRoot, _pwr, _nDL) \
    (w_wPrefixNotAtTop(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define set_w_wPrefix(_w, _nDL, _key) \
    ((_w) = (((_w) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_w_wPopCnt(_w, _nDL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPrefix(_pwRoot, _pwr, _nDL, _key) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_w_wPrefixNotAtTop(_w, _nDL, _key) \
    ((_w) = (((_w) & wPrefixPopMaskNotAtTop(_nDL)) \
            | ((_key) & ~wPrefixPopMaskNotAtTop(_nDL))))

#define set_w_wPopCntNotAtTop(_w, _nDL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMaskNotAtTop(_nDL)) \
            | ((_cnt) & wPrefixPopMaskNotAtTop(_nDL))))

#define     pwr_pLinks(_pwr)  (((Switch_t *)(_pwr))->sw_aLinks)

#if defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr) \
    (STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_awBm)
#else // defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr)  (((Switch_t *)(_pwr))->sw_awBm)
#endif // defined(BM_IN_LINK)

#define     ll_nDigitsLeft(_wr)  (((ListLeaf_t *)(_wr))->ll_nDigitsLeft)
#define set_ll_nDigitsLeft(_wr, _nDL) \
    (((ListLeaf_t *)(_wr))->ll_nDigitsLeft = (_nDL))

#define     ls_wPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_nPopCnt)
#define set_ls_wPopCnt(_ls, _cnt)  (ls_wPopCnt(_ls) = (_cnt))

#if defined(COMPRESSED_LISTS)
#define     ls_pcKeys(_ls)    (((ListLeaf_t *)(_ls))->ll_acKeys)
#define     ls_psKeys(_ls)    (((ListLeaf_t *)(_ls))->ll_asKeys)
#if (cnBitsPerWord > 32)
#define     ls_piKeys(_ls)    (((ListLeaf_t *)(_ls))->ll_aiKeys)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
#define     ls_pwKeys(_ls)    (((ListLeaf_t *)(_ls))->ll_awKeys)

// these are just aliases as long as wRoot is a pointer to a list
#define     pwr_pwKeys(_pwr)    (ls_pwKeys(_pwr))
#define     wr_ls_wPopCnt(_wr)  (ls_wPopCnt(_wr))
#if defined(COMPRESSED_LISTS)
#define     wr_pcKeys(_wr)      (ls_pcKeys(_wr))
#define     wr_psKeys(_wr)      (ls_psKeys(_wr))
#if (cnBitsPerWord > 32)
#define     wr_piKeys(_wr)      (ls_piKeys(_wr))
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
#define     wr_pwKeys(_wr)      (ls_pwKeys(_wr))

#define BitmapByteNum(_key)  ((_key) >> cnLogBitsPerByte)
#define BitmapWordNum(_key)  ((_key) >> cnLogBitsPerWord)

#define BitmapByteMask(_key)  (1 << ((_key) % cnBitsPerByte))
#define BitmapWordMask(_key)  ((Word_t)1 << ((_key) % cnBitsPerWord))

#define BitIsSetInWord(_w, _b)  (((_w) & ((Word_t)1 << (_b))) != 0)

#define SetBitInWord(_w, _b)  ((_w) |=  ((Word_t)1 << (_b)))
#define ClrBitInWord(_w, _b)  ((_w) &= ~((Word_t)1 << (_b)))

#define BitIsSetByByte(_pBitmap, _key) \
    ((((char *)(_pBitmap))[BitmapByteNum(_key)] & BitmapByteMask(_key)) \
        != 0)

#define SetBitByByte(_pBitmap, _key) \
    (((char *)(_pBitmap))[BitmapByteNum(_key)] |=  BitmapByteMask(_key))
#define ClrBitByByte(_pBitmap, _key) \
    (((char *)(_pBitmap))[BitmapByteNum(_key)] &= ~BitmapByteMask(_key))

#define BitIsSetByWord(_pBitmap, _key) \
    ((((Word_t *)(_pBitmap))[BitmapWordNum(_key)] & BitmapWordMask(_key)) \
        != 0)

#define SetBitByWord(_pBitmap, _key) \
    (((Word_t *)(_pBitmap))[BitmapWordNum(_key)] |=  BitmapWordMask(_key))
#define ClrBitByWord(_pBitmap, _key) \
    (((Word_t *)(_pBitmap))[BitmapWordNum(_key)] &= ~BitmapWordMask(_key))

#define BitIsSet  BitIsSetByWord

#define SetBit  SetBitByWord
#define ClrBit  ClrBitByWord

#define TestBit  BitIsSetByWord

#define BitTestAndSet(_pBitmap, _key, _bSet) \
    (((_bSet) = TestBit((_pBitmap), (_key))), \
        SetBitByWord((_pBitmap), (_key)), (_bSet))

#define cnBitsAtBottom  (cnDigitsAtBottom * cnBitsPerDigit)

#define cnDigitsPerWord  (((cnBitsPerWord - 1) / cnBitsPerDigit) + 1)

typedef enum { Failure = 0, Success = 1 } Status_t;

#if (cnBitsPerDigit != 0)

typedef struct {
    uint16_t ll_nPopCnt; // includes prefix, node type and nDigitsLeft
    uint8_t ll_nDigitsLeft;
    union {
#if defined(COMPRESSED_LISTS)
        uint8_t  ll_acKeys[1];
        uint16_t ll_asKeys[1];
#if (cnBitsPerWord > 32)
        uint32_t ll_aiKeys[1];
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        Word_t   ll_awKeys[1];
    };
} ListLeaf_t;

typedef struct {
    Word_t ln_wRoot;
#if defined(BM_IN_LINK)
    Word_t ln_awBm [ DIV_UP ( EXP(cnBitsPerDigit) , cnBitsPerWord ) ] ;
#endif // defined(BM_IN_LINK)
#if defined(PP_IN_LINK)
    Word_t ln_wPrefixPop;
#endif // defined(PP_IN_LINK)
} Link_t;

// Uncompressed, basic switch.
typedef struct {
#if !defined(PP_IN_LINK)
    Word_t sw_wPrefixPop;
#endif // !defined(PP_IN_LINK)
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
    Word_t sw_awBm [ DIV_UP ( EXP(cnBitsPerDigit) , cnBitsPerWord ) ] ;
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
#if defined(BM_SWITCH_FOR_REAL)
    Link_t sw_aLinks[1];
#else // defined(BM_SWITCH_FOR_REAL)
    Link_t sw_aLinks[EXP(cnBitsPerDigit)];
#endif // defined(BM_SWITCH_FOR_REAL)
} Switch_t;

Status_t Lookup(Word_t   wRoot, Word_t wKey);

Status_t Insert(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);
Status_t Remove(Word_t *pwRoot, Word_t wKey, unsigned nBitsLeft);

Status_t InsertGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

Status_t RemoveGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDigitsLeft, Word_t wRoot);

Word_t FreeArrayGuts(Word_t *pwRoot,
                     Word_t wPrefix, unsigned nBitsLeft, int bDump);

Word_t OldBitmap(Word_t wRoot);

#if defined(DEBUG)
void Dump(Word_t wRoot, Word_t wPrefix, unsigned nBL);
#endif // defined(DEBUG)

#endif // (cnBitsPerDigit != 0)

#if defined(DEBUG)
Word_t *pwRootLast; // allow dumping of tree when root is not known
Word_t wInserts; // sanity checking
#endif // defined(DEBUG)

#endif // ( ! defined(_B_H_INCLUDED) )

