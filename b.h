
#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

// Default is -DSKIP_LINKS -USKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX.
// Default is -USAVE_PREFIX -USAVE_PREFIX_TEST_RESULT
// Default is -UALWAYS_CHECK_PREFIX_AT_LEAF.
// -DALWAYS_CHECK_PREFIX_AT_LEAF appears to be best for 64-bit Judy1 with
// 16-bit digits and a 16-bit bitmap.  More digits and -DSKIP_LINKS of any
// flavor seems similar.
// -USKIP_LINKS is a lot better for 32-bit Judy1 with 16-bit digits and a
// 16-bit bitmap.  Less so for 8-bit digits.  Maybe.
#if ! defined(NO_SKIP_LINKS)
#undef  SKIP_LINKS
#define SKIP_LINKS
#endif // ! defined(NO_SKIP_LINKS)

// Default is -DSEARCH_FROM_J1T.
#if ! defined(NO_SEARCH_FROM_J1T)
#undef  SEARCH_FROM_J1T
#define SEARCH_FROM_J1T
#endif // ! defined(NO_SEARCH_FROM_J1T)

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)
#undef  SKIP_PREFIX_CHECK
#define SKIP_PREFIX_CHECK
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)

// Default is -UBM_SWITCH_FOR_REAL -UBM_IN_LINK.

// Default is -DHAS_KEY.
#if ! defined(NO_HAS_KEY)
#define HAS_KEY
#endif // ! defined(NO_HAS_KEY)

// Default is -DPAD_T_ONE.
#if ! defined(NO_PAD_T_ONE)
#define PAD_T_ONE
#endif // ! defined(NO_PAD_T_ONE)

// Default is -DSORT_LISTS.
#if ! defined(NO_SORT_LISTS)
#undef  SORT_LISTS
#define SORT_LISTS
#endif // defined(NO_SORT_LISTS)

// Default is cnBinarySearchThresholdWord is 16.
#if ! defined(cnBinarySearchThresholdWord)
#define cnBinarySearchThresholdWord  16
#endif // ! defined(cnBinarySearchThresholdWord)

// Default is -DCOMPRESSED_LISTS.
#if ! defined(COMPRESSED_LISTS) && ! defined(NO_COMPRESSED_LISTS)
#define COMPRESSED_LISTS
#endif // ! defined(COMPRESSED_LISTS) && ! defined(NO_COMPRESSED_LISTS)

// Default is -UPLACE_LISTS.

// Default is -DRAMMETRICS.
#if ! defined(RAMMETRICS) && ! defined(NO_RAMMETRICS)
#define RAMMETRICS
#endif // ! defined(RAMMETRICS) && ! defined(RAMMETRICS)

// Default is -DJUDYA -UJUDYB.
#if ! defined(JUDYA) && ! defined(JUDYB)
#define JUDYA
#endif // ! defined(JUDYA) && ! defined(JUDYB)

// Default is -DNDEBUG -UDEBUG_ALL -UDEBUG.
// Default is -UDEBUG_INSERT -UDEBUG_REMOVE -UDEBUG_LOOKUP -UDEBUG_MALLOC.
#if defined(DEBUG_ALL)

    #undef  NDEBUG
    #undef   DEBUG
    #undef   DEBUG_INSERT
    #undef   DEBUG_REMOVE
    #undef   DEBUG_MALLOC

    #define  DEBUG
    #define  DEBUG_INSERT
    #define  DEBUG_REMOVE
    #define  DEBUG_MALLOC

#else // defined(DEBUG_ALL)

  #if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) \
          || defined(DEBUG_REMOVE) || defined(DEBUG_MALLOC)

    #undef  NDEBUG
    #undef   DEBUG
    #define  DEBUG

  #endif // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...

#endif // defined(DEBUG_ALL)

#if ! defined(DEBUG)

    #undef NDEBUG
    #define NDEBUG

#endif // ! defined(DEBUG)

// Choose features.
// SKIP_LINKS, SKIP_PREFIX_CHECK, SORT_LISTS
// -UNDEBUG, RAMMETRICS, GUARDBAND

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
// - nDLRoot - nDL == -1 means double the size
//   of the index for the next switch
// - nDLRoot - nDL == -2 means quadruple the
//   size of the index for the next switch
// - nDLRoot - nDL == -3 means times eight the
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
#define MSK(_x)  (EXP(_x) - 1)

// Count leading zeros.
// __builtin_clzll is undefined for zero which allows the compiler to use bsr.
// Actual x86 clz instruction is defined for zero.
// This LOG macro is undefined for zero.
#define LOG(_x)  ((Word_t)63 - __builtin_clzll(_x))

#define cnLogBitsPerByte  3
#define cnBitsPerByte  (EXP(cnLogBitsPerByte))

// Default is cnBitsPerWord = 64.
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
#define cnBitsMallocMask  (cnLogBytesPerWord + 1)
#define cnMallocMask  MSK(cnBitsMallocMask)

// Bits-per-digit.
#if 0
// If default is cnBitsPerDigit = cnLogBitsPerWord.
#if ! defined(cnBitsPerDigit)
    #define cnBitsPerDigit cnLogBitsPerWord
#else // ! defined(cnBitsPerDigit)
    #if (cnBitsPerDigit <= 0) || (cnBitsPerDigit > cnBitsPerWord)
        #undef  cnBitsPerDigit
        #define cnBitsPerDigit  cnBitsPerWord
    #endif // (cnBitsPerDigit <= 0) || (cnBitsPerDigit > cnBitsPerWord)
#endif // ! defined(cnBitsPerDigit)
#else // 0
// Default is cnBitsPerDigit = 8.
#if ! defined(cnBitsPerDigit)
    #define cnBitsPerDigit 8
#endif // ! defined(cnBitsPerDigit)
#endif // 0

// Choose bottom, i.e.  the number of bits in the least significant digit.
// We count digits up from there.
// Default is cnBitsAtBottom = 8.
#if ! defined(cnBitsAtBottom)
#undef  cnBitsAtBottom
#define cnBitsAtBottom  8
#endif // ! defined(cnBitsAtBottom)

// Choose max list lengths.
// Mind sizeof(ll_nPopCnt) and the maximum value it implies.

// Default is cnListPopCntMax64 = 0xec.
#if ! defined(cnListPopCntMax64)
#define cnListPopCntMax64  0xec
#endif // ! defined(cnListPopCntMax64)

// Default is cnListPopCntMax32 = 0x3d.
#if ! defined(cnListPopCntMax32)
// One  64-bit word  is 0x01.  Three 64-bit words is 0x05.
// Five 64-bit words is 0x09.  Seven 64-bit words is 0x0d.
// Nine 64-bit words is 0x11.  11    64-bit words is 0x15.
// 13   64-bit words is 0x19.  15    64-bit words is 0x1d.
// 17   64-bit word  is 0x21.  19    64-bit words is 0x25.
// 21   64-bit words is 0x29.  23    64-bit words is 0x2d.
// 25   64-bit words is 0x31.  27    64-bit words is 0x35.
// 29   64-bit words is 0x39.  31    64-bit words is 0x3d.
// 33   64-bit words is 0x41.  35    64-bit words is 0x45.
// 37   64-bit words is 0x49.  39    64-bit words is 0x4d.
// 41   64-bit words is 0x51.  43    64-bit words is 0x55.
// 45   64-bit words is 0x59.  47    64-bit words is 0x5d.
#define cnListPopCntMax32  0x3d
#endif // ! defined(cnListPopCntMax32)

// Default is cnListPopCntMax16 = 0x3b.
#if ! defined(cnListPopCntMax16)
// One  64-bit word  is 0x03.  Three 64-bit words is 0x0b.
// Five 64-bit words is 0x13.  Seven 64-bit words is 0x1b.
// Nine 64-bit words is 0x23.  11    64-bit words is 0x2b.
// 13   64-bit words is 0x33.  15    64-bit words is 0x4b.
// 17   64-bit words is 0x43.  19    64-bit words is 0x5b.
// 21   64-bit words is 0x53.  23    64-bit words is 0x6b.
#define cnListPopCntMax16  0x3b
#endif // ! defined(cnListPopCntMax16)

// Default is cnListPopCntMax8  = 0x37.
#if ! defined(cnListPopCntMax8)
// One  64-bit word  is 0x07.  Three 64-bit words is 0x17.
// Five 64-bit words is 0x27.  Seven 64-bit words is 0x37.
// Nine 64-bit words is 0x47.  11    64-bit words is 0x57.
#define cnListPopCntMax8   0x37
#endif // ! defined(cnListPopCntMax8)

// Default cnListPopCntMaxDl1 is 7 for cnBitsAtBottom = 8.
// Default cnListPopCntMaxDl1 is embedded keys only.
#if ! defined(cnListPopCntMaxDl1)
  #  if (cnBitsAtBottom == 7)
      #define cnListPopCntMaxDl1  0x08
  #elif (cnBitsAtBottom == 8)
      #define cnListPopCntMaxDl1  0x07
  #elif (cnBitsAtBottom == 9)
      #define cnListPopCntMaxDl1  0x06
  #elif (cnBitsAtBottom <= 11)
      #define cnListPopCntMaxDl1  0x05
  #elif (cnBitsAtBottom <= 16)
      #define cnListPopCntMaxDl1  0x04
  #elif (cnBitsAtBottom <= 19)
      #define cnListPopCntMaxDl1  0x03
  #elif (cnBitsAtBottom <= 29)
      #define cnListPopCntMaxDl1  0x02
  #else
      #define cnListPopCntMaxDl1  0x01
  #endif // cnBitsAtBottom
#endif // ! defined(cnListPopCntMaxDl1)

#if ! defined(cwListPopCntMax)
#define cwListPopCntMax \
    (cnListPopCntMax64 + cnListPopCntMax32 \
   + cnListPopCntMax16 + cnListPopCntMax8 \
   + cnListPopCntMaxDl1)
#endif // ! defined(cwListPopCntMax)

#define cnBitsAtDl1  (cnBitsAtBottom)

// Bits in the digit next to the bottom.
#if ! defined(cnBitsAtDl2)
#define cnBitsAtDl2  (cnBitsPerDigit)
#endif // ! defined(cnBitsAtDl2)

// cnDigitsPerWord makes assumptions about anDL_to_nBitsIndexSz[] and
// anDL_to_nBL.  Yuck.
#if defined(cnBitsAtDl3)
#define cnDigitsPerWord \
    (DIV_UP(cnBitsPerWord - cnBitsAtDl1 - cnBitsAtDl2 - cnBitsAtDl3, \
            cnBitsPerDigit) \
        + 3)
#else // defined(cnBitsAtDl3)
#define cnDigitsPerWord \
    (DIV_UP(cnBitsPerWord - cnBitsAtDl1 - cnBitsAtDl2, cnBitsPerDigit) + 2)
#endif // defined(cnBitsAtDl3)

// Default is -DEMBED_KEYS which implies -DUSE_T_ONE.
#if ! defined(NO_EMBED_KEYS)
#undef  EMBED_KEYS
#define EMBED_KEYS
#endif // ! defined(NO_EMBED_KEYS)

#if defined(EMBED_KEYS)
#undef  USE_T_ONE
#define USE_T_ONE
#endif // defined(EMBED_KEYS)

// Values for nType.
enum {
    T_NULL,
#if defined(USE_T_ONE)
    T_ONE,
#endif // defined(USE_T_ONE)
    T_LIST,
    T_BITMAP,
    T_BM_SW,
    T_OTHER,
    T_SW_BASE,
};

// Default is -UBPD_TABLE, i.e. -DNO_BPD_TABLE.
#if defined(BPD_TABLE)

// Use lookup tables (which theoretically support depth-based bits per digit)
// instead of a constant bits-per-digit throughout the tree.

extern const unsigned anDL_to_nBL[];
extern const unsigned anDL_to_nBitsIndexSz[];

#define nDL_to_nBL_NAT(_nDL)  anDL_to_nBL[_nDL]

#define nDL_to_nBL(_nDL) \
    (((_nDL) < cnDigitsPerWord) ? anDL_to_nBL[_nDL] : cnBitsPerWord)

#define nDL_to_nBitsIndexSz(_nDL) \
    (((_nDL) < cnDigitsPerWord) \
        ? anDL_to_nBitsIndexSz[_nDL] \
        : cnBitsPerWord - nDL_to_nBL_NAT((_nDL) - 1))

#define nDL_to_nBitsIndexSzNAX(_nDL)  anDL_to_nBitsIndexSz[_nDL]
#define nDL_to_nBitsIndexSzNAB(_nDL)  anDL_to_nBitsIndexSz[_nDL]
#define nDL_to_nBitsIndexSzNAT(_nDL)  anDL_to_nBitsIndexSz[_nDL]

// this one is not used in the lookup performance path
#define cnBitsIndexSzAtTop  nDL_to_nBitsIndexSz(cnDigitsPerWord)

#define nBL_to_nDL(_nBL) \
    (((_nBL) <= cnBitsAtBottom) ? 1 \
        : ((_nBL) <= cnBitsAtBottom + cnBitsAtDl2) ? 2 \
        : (DIV_UP((_nBL) - cnBitsAtBottom - cnBitsAtDl2, cnBitsPerDigit) + 2))

#else // defined(BPD_TABLE)

#define cnBitsIndexSzAtTop \
    (cnBitsPerWord - cnBitsAtBottom - cnBitsAtDl2 \
        - (cnDigitsPerWord - 3) * cnBitsPerDigit)

#if (cnBitsAtDl2 == cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAX(_nDL)  (cnBitsPerDigit)

#define nDL_to_nBL_NAT(_nDL) \
    (((_nDL) - 1) * cnBitsPerDigit + cnBitsAtBottom)

#define nBL_to_nDL(_nBL) \
    (DIV_UP((_nBL) + (cnBitsPerDigit * 64) - cnBitsAtBottom, cnBitsPerDigit) \
        - 63)
 
#else // (cnBitsAtDl2 == cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAX(_nDL) \
    ((_nDL) == 2 ? cnBitsAtDl2 : cnBitsPerDigit)

#define nDL_to_nBL_NAT(_nDL) \
    (((_nDL) == 1) ? cnBitsAtBottom \
                   : cnBitsAtBottom + cnBitsAtDl2 \
                       + ((_nDL) - 2) * cnBitsPerDigit)

#define nBL_to_nDL(_nBL) \
    ((_nBL) <= cnBitsAtBottom ? 1 \
        : (DIV_UP((_nBL) + (cnBitsPerDigit * 64) \
                    - cnBitsAtBottom - cnBitsAtDl2, \
                cnBitsPerDigit) \
            - 62))

#endif // (cnBitsAtDl2 == cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAB(_nDL) \
    (((_nDL) == cnDigitsPerWord) \
        ? cnBitsIndexSzAtTop \
        : nDL_to_nBitsIndexSzNAX(_nDL))

#define nDL_to_nBitsIndexSzNAT(_nDL) \
    (((_nDL) == 1) ? cnBitsAtBottom : nDL_to_nBitsIndexSzNAX(_nDL))

#define nDL_to_nBitsIndexSz(_nDL) \
    (((_nDL) == cnDigitsPerWord) \
        ? cnBitsIndexSzAtTop : nDL_to_nBitsIndexSzNAT(_nDL))

#define nDL_to_nBL(_nDL) \
    (((_nDL) == cnDigitsPerWord) ? cnBitsPerWord : nDL_to_nBL_NAT(_nDL))

#endif // defined(BPD_TABLE)

// this one is not used in the lookup performance path
#define nBL_to_nDL_NotAtTop(_nBL)  nBL_to_nDL(_nBL)

#if defined RAMMETRICS
#define METRICS(x)  (x)
#else // defined RAMMETRICS
#define METRICS(x)
#endif // defined RAMMETRICS

// Default is -USEARCHMETRICS.
#if defined SEARCHMETRICS
#define SMETRICS(x)  (x)
#else // defined SEARCHMETRICS
#define SMETRICS(x)
#endif // defined SEARCHMETRICS

#if defined(DEBUG)
#define DBG(x)  (x)
// Default is cwDebugThreshold = 0.
#if ! defined(cwDebugThreshold)
#define cwDebugThreshold  0ULL
#endif // ! defined(cwDebugThreshold)
#else // defined(DEBUG)
#define DBG(x)
#endif // defined(DEBUG)

#if defined(DEBUG_INSERT)
#if (cwDebugThreshold != 0)
#define DBGI(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGI(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_INSERT)
#define DBGI(x)
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_LOOKUP)
#if (cwDebugThreshold != 0)
#define DBGL(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGL(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_LOOKUP)
#define DBGL(x)
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_REMOVE)
#if (cwDebugThreshold != 0)
#define DBGR(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGR(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_REMOVE)
#define DBGR(x)
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_MALLOC)
#if (cwDebugThreshold != 0)
#define DBGM(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGM(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

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

// Extract nType from wRoot.
#define     wr_nType(_wr)         ((_wr) & cnMallocMask)
// Set the nType field in wRoot.
#define set_wr_nType(_wr, _type)  ((_wr) = ((_wr) & ~cnMallocMask) | (_type))

// Extract the pwRoot field from wRoot given the value of the type field.
#define     wr_tp_pwr(_wr, _tp)          ((Word_t *)((_wr) ^ (_tp)))
// Extract the pwRoot field from wRoot.
#define     wr_pwr(_wr)                  ((Word_t *)((_wr) & ~cnMallocMask))

// Set the pwRoot field in wRoot.
#define set_wr_pwr(_wr, _pwr) \
                ((_wr) = ((_wr) & cnMallocMask) | (Word_t)(_pwr))

// Set the pwRoot and nType fields in wRoot.
#define set_wr(_wr, _pwr, _type)  ((_wr) = (Word_t)(_pwr) | (_type))

#if defined(USE_T_ONE)

// Fixed-size pop count field to make code simpler.
// We only give up one 29-bit slot in 64-bit and one 14-bit slot in 32-bit.
#if ! defined(NO_EMBEDDED_LIST_FIXED_POP)
#undef  EMBEDDED_LIST_FIXED_POP
#define EMBEDDED_LIST_FIXED_POP
#endif // ! defined(NO_EMBEDDED_LIST_FIXED_POP)
#if (cnBitsPerWord == 64)
#if defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  3
#else // defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(88 / (_nBL))
#endif // defined(EMBEDDED_LIST_FIXED_POP)
#elif (cnBitsPerWord == 32)
#if defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  2
#else // defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(44 / (_nBL))
#endif // defined(EMBEDDED_LIST_FIXED_POP)
#else
#error "Invalid cnBitsPerWord."
#endif

// Pop cnt bits are just above the type field.
// A value of zero means a pop cnt of one. 

#define     wr_nPopCnt(_wr, _nBL) \
    ((((_wr) >> cnBitsMallocMask) & MSK(nBL_to_nBitsPopCntSz(_nBL))) + 1)

#define set_wr_nPopCnt(_wr, _nBL, _nPopCnt) \
    ((_wr) &= ~(MSK(nBL_to_nBitsPopCntSz(_nBL)) << cnBitsMallocMask), \
        (_wr) |= ((_nPopCnt) - 1) << cnBitsMallocMask)

#endif // defined(USE_T_ONE)

// Default is -UTYPE_IS_ABSOLUTE, i.e. -DTYPE_IS_RELATIVE.
#if defined(TYPE_IS_ABSOLUTE)

// Why do we need nType to be able to represent nDL == 1?
// We have to test for nDL == 1 before looping back to the switch statement
// that checks nType if cnBitsAtBottom == cnLogBitsPerWord because there is
// no room for a type field when all the bits are used for an embedded bitmap.
// But if cnBitsAtBottom > cnLogBitsPerWord we don't want to waste the
// conditional branch.
#if (cnBitsAtBottom > cnLogBitsPerWord)
  #define tp_to_nDL(_tp)   ((_tp)  - T_SW_BASE + 1)
  #define nDL_to_tp(_nDL)  ((_nDL) + T_SW_BASE - 1)
#else // (cnBitsAtBottom > cnLogBitsPerWord)
  #define tp_to_nDL(_tp)   ((_tp)  - T_SW_BASE + 2)
  #define nDL_to_tp(_nDL)  ((_nDL) + T_SW_BASE - 2)
#endif // (cnBitsAtBottom > cnLogBitsPerWord)

#define     wr_nDL(_wr)     (tp_to_nDL(wr_nType(_wr)))
#define set_wr_nDL(_wr, _nDL) \
    (set_wr_nType((_wr), nDL_to_tp(_nDL)))

#define     wr_bIsSwitchDL(_wr, _tp, _nDL) \
   ((_tp) = wr_nType(_wr), (_nDL) = tp_to_nDL(_tp), tp_bIsSwitch(_tp))

#else // defined(TYPE_IS_ABSOLUTE)

#undef  TYPE_IS_RELATIVE
#define TYPE_IS_RELATIVE

#define tp_to_nDS(_tp)   ((_tp)  - T_SW_BASE)
#define nDS_to_tp(_nDS)  ((_nDS) + T_SW_BASE)

#define     wr_nDS(_wr)        (tp_to_nDS(wr_nType(_wr)))
#define set_wr_nDS(_wr, _nDS)  (set_wr_nType((_wr), nDS_to_tp(_nDS)))

#define     wr_bIsSwitchDS(_wr, _tp, _nDS) \
   ((_tp) = wr_nType(_wr), (_nDS) = tp_to_nDS(_tp), tp_bIsSwitch(_tp))

#endif // defined(TYPE_IS_ABSOLUTE)

#if defined(EXTRA_TYPES)
#define     tp_bIsSwitch(_tp)          ((_tp) & cnMallocMask) >= T_BM_SW)
#else // defined(EXTRA_TYPES)
#define     tp_bIsSwitch(_tp)          ((_tp) >= T_BM_SW)
#endif // defined(EXTRA_TYPES)

#define     wr_bIsSwitch(_wr)          (tp_bIsSwitch(wr_nType(_wr)))

// methods for Switch (and aliases)

#define wPrefixPopMaskNotAtTop(_nDL) \
    (EXP(nDL_to_nBL_NAT(_nDL)) - (Word_t)1)

#define wPrefixPopMask(_nDL) \
    (((_nDL) == cnDigitsPerWord) ? (Word_t)-1 : wPrefixPopMaskNotAtTop(_nDL))

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
// Default is -UPP_IN_LINK, i.e. -DPP_IN_SWITCH.
#if defined(PP_IN_LINK)
#define PWR_wPrefixPop(_pwRoot, _pwr) \
    (STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_wPrefixPop)
#else // defined(PP_IN_LINK)
#define PWR_wPrefixPop(_pwRoot, _pwr)  ((_pwr)->sw_wPrefixPop)
#endif // defined(PP_IN_LINK)

#define PWR_wPrefix(_pwRoot, _pwr, _nDL) \
    (w_wPrefix(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (w_wPopCnt(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPrefixNAT(_pwRoot, _pwr, _nDL) \
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

#define     pwr_pLinks(_pwr)  ((_pwr)->sw_aLinks)

#if defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr) \
    (STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_awBm)
#else // defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr)  (((BmSwitch_t *)(_pwr))->sw_awBm)
#endif // defined(BM_IN_LINK)

#if defined(PP_IN_LINK)

// For PP_IN_LINK ls_wPopCnt macros are only valid at top, i.e.
// nDL == cnDigitsPerWord, and only for T_LIST - not for T_ONE.
#if (cnDummiesInList == 0)
#define     ls_wPopCnt(_ls)  (((ListLeaf_t *)(_ls))->ll_awKeys[0])
#else // (cnDummiesInList == 0)
// Use the last dummy for pop count if we have at least one dummy.
#define     ls_wPopCnt(_ls) \
    (((ListLeaf_t *)(_ls))->ll_awDummies[cnDummiesInList - 1])
#endif // (cnDummiesInList == 0)
#define set_ls_wPopCnt(_ls, _cnt)  (ls_wPopCnt(_ls) = (_cnt))

// Index of first key within leaf (for nDL != cnDigitsPerWord).
#if defined(LIST_END_MARKERS)
#define FIRST_KEY  1
#else // defined(LIST_END_MARKERS)
#define FIRST_KEY  0
#endif // defined(LIST_END_MARKERS)

#else // defined(PP_IN_LINK)

#define     ls_wPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_acKeys[0])
#define set_ls_wPopCnt(_ls, _cnt)  (ls_wPopCnt(_ls) = (_cnt))

// Index of first key within leaf (for all cases).
#if defined(LIST_END_MARKERS)
#define FIRST_KEY  2
#else // defined(LIST_END_MARKERS)
#define FIRST_KEY  1
#endif // defined(LIST_END_MARKERS)

#endif // defined(PP_IN_LINK)

// For PP_IN_LINK ls_pxKeys macros are only valid not at top or for
// T_ONE - not T_LIST - at top.
#define ls_pwKeys(_ls)  (&((ListLeaf_t *)(_ls))->ll_awKeys[FIRST_KEY])

#if ! defined(NO_PSPLIT_PARALLEL)
#undef  PSPLIT_PARALLEL
#define PSPLIT_PARALLEL
#endif // ! defined(NO_PSPLIT_PARALLEL)
#if ! defined(NO_PSPLIT_EARLY_OUT)
#undef  PSPLIT_EARLY_OUT
#define PSPLIT_EARLY_OUT
#endif // ! defined(NO_PSPLIT_EARLY_OUT)

#if defined(COMPRESSED_LISTS)
  #if defined(PSPLIT_PARALLEL)

#define ls_pcKeys(_ls) \
    ((uint8_t *)(((Word_t)&((ListLeaf_t *)(_ls))->ll_acKeys[FIRST_KEY] \
            + sizeof(Word_t) - 1) \
        & ~MSK(cnLogBytesPerWord)))

#define ls_psKeys(_ls) \
    ((uint16_t *)(((Word_t)&((ListLeaf_t *)(_ls))->ll_asKeys[FIRST_KEY] \
            + sizeof(Word_t) - 1) \
        & ~MSK(cnLogBytesPerWord)))

      #if (cnBitsPerWord > 32)
#define ls_piKeys(_ls) \
    ((uint32_t *)(((Word_t)&((ListLeaf_t *)(_ls))->ll_aiKeys[FIRST_KEY] \
            + sizeof(Word_t) - 1) \
        & ~MSK(cnLogBytesPerWord)))
      #endif // (cnBitsPerWord > 32)

  #else // defined(PSPLIT_PARALLEL)

#define ls_pcKeys(_ls)  (&((ListLeaf_t *)(_ls))->ll_acKeys[FIRST_KEY])

#define ls_psKeys(_ls)  (&((ListLeaf_t *)(_ls))->ll_asKeys[FIRST_KEY])

      #if (cnBitsPerWord > 32)
#define ls_piKeys(_ls)  (&((ListLeaf_t *)(_ls))->ll_aiKeys[FIRST_KEY])
      #endif // (cnBitsPerWord > 32)

  #endif // defined(PSPLIT_PARALLEL)
#endif // defined(COMPRESSED_LISTS)

// these are just aliases
#define     pwr_pwKeys(_pwr)    (ls_pwKeys(_pwr))
#if defined(COMPRESSED_LISTS)
#define     pwr_pcKeys(_pwr)     (ls_pcKeys(_pwr))
#define     pwr_psKeys(_pwr)     (ls_psKeys(_pwr))
#if (cnBitsPerWord > 32)
#define     pwr_piKeys(_pwr)     (ls_piKeys(_pwr))
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
#define     pwr_pwKeys(_pwr)     (ls_pwKeys(_pwr))

// Bitmap macros.
// Accessing a bitmap by byte can be more expensive than
// accessing it by word.

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

typedef enum { Failure = 0, Success = 1 } Status_t;

#if (cnDigitsPerWord != 1)

#if ! defined(cnDummiesInList)
#define cnDummiesInList  0
#endif // ! defined(cnDummiesInList)
#if ! defined(cnDummiesInSwitch)
#define cnDummiesInSwitch  0
#endif // ! defined(cnDummiesInSwitch)
#if ! defined(cnDummiesInLink)
#define cnDummiesInLink  0
#endif // ! defined(cnDummiesInLink)

typedef struct {
#if (cnDummiesInList != 0)
    Word_t ll_awDummies[cnDummiesInList];
#endif // (cnDummiesInList != 0)
    union {
#if defined(COMPRESSED_LISTS) || ! defined(PP_IN_LINK)
        uint8_t  ll_acKeys[FIRST_KEY+1];
#endif // defined(COMPRESSED_LISTS) || ! defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
        uint16_t ll_asKeys[FIRST_KEY+1];
#if (cnBitsPerWord > 32)
        uint32_t ll_aiKeys[FIRST_KEY+1];
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        Word_t   ll_awKeys[FIRST_KEY+1];
    };
} ListLeaf_t;

#define N_WORDS_SWITCH_BM  DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord)

//
// When trying to make ! PP_IN_LINK use the same amount of memory as
// PP_IN_LINK:
//   -UPP_IN_LINK -DcnDummiesInLink=1 -DNO_SKIP_AT_TOP
//   -DPP_IN_LINK -DDUMMY_POP_CNT_IN_LIST
//
typedef struct {
    Word_t ln_wRoot;
#if defined(BM_IN_LINK)
    Word_t ln_awBm[N_WORDS_SWITCH_BM];
#endif // defined(BM_IN_LINK)
#if defined(PP_IN_LINK)
    Word_t ln_wPrefixPop;
#endif // defined(PP_IN_LINK)
#if (cnDummiesInLink != 0)
    Word_t ln_awDummies[cnDummiesInLink];
#endif // (cnDummiesInLink != 0)
} Link_t;

// Uncompressed, basic switch.
typedef struct {
#if !defined(PP_IN_LINK)
    Word_t sw_wPrefixPop;
#endif // !defined(PP_IN_LINK)
#if (cnDummiesInSwitch != 0)
    Word_t sw_awDummies[cnDummiesInSwitch];
#endif // (cnDummiesInSwitch != 0)
    Link_t sw_aLinks[1]; // variable size
} Switch_t;

// Bitmap switch.
typedef struct {
#if !defined(PP_IN_LINK)
    Word_t sw_wPrefixPop;
#endif // !defined(PP_IN_LINK)
#if ! defined(BM_IN_LINK)
    Word_t sw_awBm[N_WORDS_SWITCH_BM];
#endif // ! defined(BM_IN_LINK)
#if (cnDummiesInSwitch != 0)
    Word_t sw_awDummies[cnDummiesInSwitch];
#endif // (cnDummiesInSwitch != 0)
    Link_t sw_aLinks[1]; // variable size
} BmSwitch_t;

Status_t Insert(Word_t *pwRoot, Word_t wKey, unsigned nBL);
Status_t Remove(Word_t *pwRoot, Word_t wKey, unsigned nBL);

Status_t InsertGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDL, Word_t wRoot);

Status_t RemoveGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDL, Word_t wRoot);

Word_t FreeArrayGuts(Word_t *pwRoot,
                     Word_t wPrefix, unsigned nBL, int bDump);

#if defined(DEBUG)
extern int bHitDebugThreshold;
#endif // defined(DEBUG)

unsigned ListWords(Word_t wPopCnt, unsigned nDL);
Word_t *NewList(Word_t wPopCnt, unsigned nDL);
Word_t OldList(Word_t *pwList, Word_t wPopCnt, unsigned nDL, unsigned nType);

#if defined(DEBUG)
void Dump(Word_t *pwRoot, Word_t wPrefix, unsigned nBL);
#endif // defined(DEBUG)

#else // (cnDigitsPerWord != 1)
Word_t wDebugPopCnt; // sanity checking
#endif // (cnDigitsPerWord != 1)

#if defined(DEBUG)
Word_t *pwRootLast; // allow dumping of tree when root is not known
#if (cnDigitsPerWord != 1)
Word_t wDebugPopCnt; // sanity checking
#endif // (cnDigitsPerWord != 1)
#endif // defined(DEBUG)

// Default is -DPSPLIT_SEARCH_8
// This depends on uniform distribution / flat spectrum data.
#if ! defined(NO_PSPLIT_SEARCH_8)
#undef  PSPLIT_SEARCH_8
#define PSPLIT_SEARCH_8
#endif // ! defined(NO_PSPLIT_SEARCH_8)

// Default is -DPSPLIT_SEARCH_16.
// This depends on uniform distribution / flat spectrum data.
#if ! defined(NO_PSPLIT_SEARCH_16)
#undef  PSPLIT_SEARCH_16
#define PSPLIT_SEARCH_16
#endif // ! defined(NO_PSPLIT_SEARCH_16)

// Default is -DPSPLIT_SEARCH_32.
// This depends on uniform distribution / flat spectrum data.
#if ! defined(NO_PSPLIT_SEARCH_32)
#undef  PSPLIT_SEARCH_32
#define PSPLIT_SEARCH_32
#endif // ! defined(NO_PSPLIT_SEARCH_32)

// Default is -DBINARY_SEARCH_WORD -UBACKWARD_SEARCH_WORD.
// Default is -UPSPLIT_SEARCH_WORD -UPSPLIT_SEARCH_XOR_WORD.
// -B32 is not uniform distribution / flat spectrum data at the top.
#if ! defined(NO_BINARY_SEARCH_WORD) && ! defined(PSPLIT_SEARCH_WORD)
#if ! defined(PSPLIT_SEARCH_XOR_WORD)
#undef  BINARY_SEARCH_WORD
#define BINARY_SEARCH_WORD
#endif // ! defined(PSPLIT_SEARCH_XOR_WORD)
#endif // ! defined(NO_BINARY_SEARCH_WORD) && ! defined(PSPLIT_SEARCH_WORD)

#if defined(PSPLIT_SEARCH_XOR_WORD)
#undef  PSPLIT_SEARCH_WORD
#define PSPLIT_SEARCH_WORD
#endif // defined(PSPLIT_SEARCH_XOR_WORD)

// Default is -UUSE_BM_SW, -UBM_SW_AT_DL2 and -UBM_SW_AT_DL2_ONLY.
#if defined(BM_SW_AT_DL2_ONLY)
#undef  USE_BM_SW
#undef  BM_SW_AT_DL2
#define BM_SW_AT_DL2
#endif // defined(BM_SW_AT_DL2_ONLY)

void HexDump(char *str, Word_t *pw, unsigned nWords);

#endif // ( ! defined(_B_H_INCLUDED) )

