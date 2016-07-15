
// @(#) $Id: b.h,v 1.399 2016/07/11 10:18:50 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/b.h,v $

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

// Default is -USEARCH_FROM_WRAPPER.

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)
#undef  SKIP_PREFIX_CHECK
#define SKIP_PREFIX_CHECK
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)

// Default cn2dBmWpkPercent is 80, create 2-digit bm at 80% wpk.
#if ! defined(cn2dBmWpkPercent)
#undef cn2dBmWpkPercent
#define cn2dBmWpkPercent  80
#else
#undef cn2dBmWpkPercent
#define cn2dBmWpkPercent  0
#endif // ! defined(cn2dBmWpkPercent)

#if defined(USE_BM_SW)
// Default is -UUSE_BM_SW.
// USE_BM_SW means always use a bm sw when creating a switch with no skip.
// Default is -DBM_SW_FOR_REAL iff -DUSE_BM_SW.
// Default is -UBM_IN_LINK.
  #define CODE_BM_SW
  #if ! defined(NO_BM_SW_FOR_REAL)
      #define BM_SW_FOR_REAL
  #endif // ! defined(NO_BM_SW_FOR_REAL)

// Default is -URETYPE_FULL_BM_SW.
// Default is -UBM_IN_NON_BM_SW.
// There is no need.  It will be converted sooner.
// Retype without BM_IN_NON_BM_SW requires the
// guts of BmSwitch_t to look just like Switch_t
// starting just after sw_awBm.

  #if ! defined(cnBmSwLinksPercent)
      // Default uncompress bm sw threshold is 33% full (cnBmSwLinksPercent).
      // Never uncompress with less than 33% of links present.
      // Otherwise we can overshoot the wpk target.
      #define cnBmSwLinksPercent  33
  #endif // ! defined(cnBmSwLinksPercent)

  #if ! defined(cnBmSwWpkPercent)
      // Default uncompress bm sw threshold is 1 word/key (cnBmSwWpkPercent).
      // If the switch satisfies cnBmSwLinksPercent, then uncompress if and
      // only if the words-per-key target is met.
      #define cnBmSwWpkPercent  100
  #endif // ! defined(cnBmSwWpkPercent)

#endif // defined(USE_BM_SW)

// Default is USE_XX_SW.
#if ! defined(NO_USE_XX_SW)
  #undef USE_XX_SW
  #define USE_XX_SW
#endif // ! defined(NO_USE_XX_SW)

#if defined(USE_XX_SW)
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // defined(USE_XX_SW)

// Default cnBW is 1 if CODE_XX_SW.
// cnBW is the minimum width of a narrow switch.
#if defined(CODE_XX_SW)
  #if ! defined(cnBW)
      #define cnBW  1
  #endif // ! defined(cnBW)
#endif // defined(CODE_XX_SW)

// Default cnBWIncr is 1 if CODE_XX_SW.
#if defined(CODE_XX_SW)
  #if ! defined(cnBWIncr)
      #define cnBWIncr  1
  #endif // ! defined(cnBWIncr)
#endif // defined(CODE_XX_SW)

// Default is -UNO_TYPE_IN_XX_SW.

// Default is XX_SHORTCUT if USE_XX_SW.
// Default cnListPopCntMaxDl2 is 0 if USE_XX_SW.
// Default cnListPopCntMax16  is 0 if USE_XX_SW.
// Default cnListPopCntMaxDl1 is 0 if USE_XX_SW.
// Default cnListPopCntMax8   is 0 if USE_XX_SW.
#if defined(USE_XX_SW)
  #if ! defined(NO_XX_SHORTCUT)
      #undef XX_SHORTCUT
      #define XX_SHORTCUT
  #endif // defined(NO_XX_SHORTCUT)
  #if ! defined(cnListPopCntMaxDl2)
      #define cnListPopCntMaxDl2  0
  #endif // ! defined(cnListPopCntMaxDl2)
  #if ! defined(cnListPopCntMax16)
      #define cnListPopCntMax16  0
  #endif // ! defined(cnListPopCntMax16)
  #if ! defined(cnListPopCntMaxDl1)
      #define cnListPopCntMaxDl1  0
  #endif // ! defined(cnListPopCntMaxDl1)
  #if ! defined(cnListPopCntMax8)
      #define cnListPopCntMax8  0
  #endif // ! defined(cnListPopCntMax8)
#endif // defined(USE_XX_SW)
// Default is XX_SHORTCUT_GOTO if XX_SHORTCUT
#if defined(XX_SHORTCUT)
  #if ! defined(NO_XX_SHORTCUT_GOTO)
      #undef XX_SHORTCUT_GOTO
      #define XX_SHORTCUT_GOTO
  #endif // ! defined(NO_XX_SHORTCUT_GOTO)
#endif // defined(XX_SHORTCUT)

// Default is -DDL_SPECIFIC_T_ONE.
#if ! defined(NO_DL_SPECIFIC_T_ONE)
#undef DL_SPECIFIC_T_ONE
#define DL_SPECIFIC_T_ONE
#endif // ! defined(NO_DL_SPECIFIC_T_ONE)

// Default is -DBL_SPECIFIC_PSPLIT_SEARCH.
#if ! defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
#undef BL_SPECIFIC_PSPLIT_SEARCH
#define BL_SPECIFIC_PSPLIT_SEARCH
#endif // ! defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)

// Default is -DEMBEDDED_KEYS_PARALLEL_FOR_LOOKUP.
// It applies to Insert AND Remove.
#if ! defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP) \
  && ! defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)
#undef EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
#define EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
#endif

// Default is -DEMBEDDED_KEYS_PARALLEL_FOR_INSERT.
// It applies to Insert AND Remove.
#if ! defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT) \
  && ! defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)
#undef EMBEDDED_KEYS_PARALLEL_FOR_INSERT
#define EMBEDDED_KEYS_PARALLEL_FOR_INSERT
#endif

// Default is -DPARALLEL_128.
#if ! defined(NO_PARALLEL_128)
#undef PARALLEL_128
#define PARALLEL_128
#endif // ! defined(NO_PARALLEL_128)

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
    #undef   DEBUG_COUNT

    #define  DEBUG
    #define  DEBUG_INSERT
    #define  DEBUG_REMOVE
    #define  DEBUG_MALLOC
    #define  DEBUG_COUNT

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
//   only leaf level can be out of cache; 
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

// Do integer division, but round up instead of down.
#define DIV_UP(_idend, _isor)  (((_idend) + (_isor) - 1) / (_isor))
// Do integer division, but round up instead of down.
// Pass in the log of the power of 2 divisor.
#define DIV_UP_X(_idend, _log)  (((_idend) + (1 << (_log)) - 1) >> (_log))
// ALIGN_UP assumes _isor is a power of 2.
#define ALIGN_UP(_idend, _isor)  (((_idend) + (_isor) - 1) & ~((_isor) - 1))

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

#define cnLogBitsPerWord  (cnLogBytesPerWord + cnLogBitsPerByte)
#define cnBitsMallocMask  (cnLogBytesPerWord + 1)
#define cnMallocMask  MSK(cnBitsMallocMask)
#if (cnBitsPerWord == 64)
#define cnBitsVirtAddr  48
#else // (cnBitsPerWord == 64)
#define cnBitsVirtAddr  32
#endif // (cnBitsPerWord == 64)
#define cwVirtAddrMask  MSK(cnBitsVirtAddr)

// Bits are numbered 0-63 with 0 being the least significant.
static inline Word_t
GetBits(Word_t ww, int nBits, int nLsb)
{
    return ww << (cnBitsPerWord - (nLsb + nBits)) >> (cnBitsPerWord - nBits);
}

static inline void
SetBits(Word_t *pw, int nBits, int nLsb, Word_t wVal)
{
    *pw &= ~(MSK(nBits) << nLsb); // clear the field
    *pw |= (wVal & MSK(nBits)) << nLsb; // set the field
}

// Default is -DPSPLIT_PARALLEL which forces -DALIGN_LISTS -DALIGN_LIST_ENDS.
#if ! defined(NO_PSPLIT_PARALLEL)
#undef  PSPLIT_PARALLEL
#define PSPLIT_PARALLEL
#endif // ! defined(NO_PSPLIT_PARALLEL)

#if defined(PSPLIT_PARALLEL)
#undef ALIGN_LISTS
#define ALIGN_LISTS
#undef ALIGN_LIST_ENDS
#define ALIGN_LIST_ENDS
#endif // defined(PSPLIT_PARALLEL)

// Default is -DPSPLIT_EARLY_OUT which is applicable only if PSPLIT_PARALLEL.
#if ! defined(NO_PSPLIT_EARLY_OUT)
#undef  PSPLIT_EARLY_OUT
#define PSPLIT_EARLY_OUT
#endif // ! defined(NO_PSPLIT_EARLY_OUT)

#if defined(PARALLEL_128)
#include <immintrin.h> // __m128i
typedef __m128i Bucket_t;
#define cnLogBytesPerBucket  4
#else // defined(PARALLEL_128)
typedef Word_t Bucket_t;
#define cnLogBytesPerBucket  cnLogBytesPerWord
#endif // defined(PARALLEL_128)

// Bits-per-digit.
#if JUNK
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
#endif // JUNK

// Choose the number of bits in the least significant digit of the key.
// Default cnBitsInD1 is cnBitsPerDigit.  We count digits up from there.
#if ! defined(cnBitsInD1)
#define cnBitsInD1  cnBitsPerDigit
#endif // ! defined(cnBitsInD1)

// Default is -DLVL_IN_WR_HB unless -DDEPTH_IN_SW.
#if ! defined(DEPTH_IN_SW) && ! defined(NO_LVL_IN_WR_HB)
  #undef LVL_IN_WR_HB
  #define LVL_IN_WR_HB
#endif // ! defined(DEPTH_IN_SW) && ! defined(NO_LVL_IN_WR_HB)

#if defined(CODE_XX_SW)
// Default is -DSKIP_TO_XX_SW.
#if ! defined(NO_SKIP_TO_XX_SW)
#undef SKIP_TO_XX_SW
#define SKIP_TO_XX_SW
#endif // ! defined(NO_SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)

// Default is -DSKIP_TO_BITMAP if -DSKIP_TO_XX_SW.
#if defined(SKIP_TO_XX_SW)
#undef SKIP_TO_BITMAP
#define SKIP_TO_BITMAP
#endif // ! defined(SKIP_TO_XX_SW)

// Default is SKIP_TO_BM_SW if USE_BM_SW and (DEPTH_IN_SW or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_BM_SW)
  #if defined(USE_BM_SW)
      #if defined(DEPTH_IN_SW) || defined(LVL_IN_WR_HB)
          #define SKIP_TO_BM_SW
      #endif // defined(DEPTH_IN_SW) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_BM_SW)
#endif // ! defined(NO_SKIP_TO_BM_SW)

// Default is -DPOP_IN_WR_HB.
#if ! defined(NO_POP_IN_WR_HB)
  #undef  POP_IN_WR_HB
  #define POP_IN_WR_HB
#endif // ! defined(NO_POP_IN_WR_HB)

// Choose max list lengths.
// Mind sizeof(ll_nPopCnt) and the maximum value it implies.

// Default is cnListPopCntMax64 is 0x40 (0xec if NO_SKIP_[TO_BM_SW|AT_TOP]).
#if ! defined(cnListPopCntMax64)
  #if defined(SKIP_TO_BM_SW) && ! defined(NO_SKIP_AT_TOP)
      #define cnListPopCntMax64  0x40
  #else // defined(SKIP_TO_BM_SW) && ! defined(NO_SKIP_AT_TOP)
      #if defined(POP_IN_WR_HB)
          #define cnListPopCntMax64  0x7c // field size is limited
      #else // defined(POP_IN_WR_HB)
          #define cnListPopCntMax64  0xec
      #endif // defined(POP_IN_WR_HB)
  #endif // defined(SKIP_TO_BM_SW) && ! defined(NO_SKIP_AT_TOP)
#endif // ! defined(cnListPopCntMax64)

// Default is cnListPopCntMax32 is 0x30 (0xf0 if NO_USE_BM_SW).
#if ! defined(cnListPopCntMax32)
  #if defined(USE_BM_SW)
      #define cnListPopCntMax32  0x30
  #else // defined(USE_BM_SW)
      #if defined(POP_IN_WR_HB)
          #define cnListPopCntMax32  0x7c // field size is limited
      #else // defined(POP_IN_WR_HB)
          #define cnListPopCntMax32  0xf0
      #endif // defined(POP_IN_WR_HB)
  #endif  // defined(USE_BM_SW)
#endif // ! defined(cnListPopCntMax32)

// Default is cnListPopCntMax16 is 0x40 (0x70 if NO_USE_BM_SW).
#if ! defined(cnListPopCntMax16)
  #if defined(USE_BM_SW)
      #define cnListPopCntMax16  0x40
  #else // defined(USE_BM_SW)
      #define cnListPopCntMax16  0x70
  #endif // defined(USE_BM_SW)
#endif // ! defined(cnListPopCntMax16)

// An 8-bit bitmap uses only 32-bytes plus malloc overhead.
// It makes no sense to have a list that uses as much.
#if ! defined(cnListPopCntMax8)
  #define cnListPopCntMax8  0x10
#endif // ! defined(cnListPopCntMax8)

// Default cnListPopCntMaxDl1 is 7 for cnBitsInD1 = 8 (embedded keys only).
#if ! defined(cnListPopCntMaxDl1)
  #if defined(USE_XX_SW)
      #define cnListPopCntMaxDl1  0x10
  #else // defined(USE_XX_SW)
    #  if (cnBitsInD1 == 7)
      #define cnListPopCntMaxDl1  0x08
    #elif (cnBitsInD1 == 8)
      #define cnListPopCntMaxDl1  0x07
    #elif (cnBitsInD1 == 9)
      #define cnListPopCntMaxDl1  0x06
    #elif (cnBitsInD1 <= 11)
      #define cnListPopCntMaxDl1  0x05
    #elif (cnBitsInD1 <= 16)
      #define cnListPopCntMaxDl1  0x04
    #elif (cnBitsInD1 <= 19)
      #define cnListPopCntMaxDl1  0x03
    #elif (cnBitsInD1 <= 29)
      #define cnListPopCntMaxDl1  0x02
    #else
      #define cnListPopCntMaxDl1  0x01
    #endif // cnBitsInD1
  #endif // defined(USE_XX_SW)
#endif // ! defined(cnListPopCntMaxDl1)

// cwListPopCntMax is mostly used as a boolean that indicates whether
// or not we are using lists at all; embedded or external.
// But it is also used to size the temporary buffer used when copying
// one list to another hence it must be at least as big as the
// biggest list.
// 8 is the maximum number of keys we can embed (assuming at least two
// bits of type info in the link).  Hence the 8 in the expression below.
#if ! defined(cwListPopCntMax)
#define MAX(_x, _y)  ((_x) > (_y) ? (_x) : (_y))
#define cwListPopCntMax \
   MAX(cnListPopCntMax64, \
       MAX(cnListPopCntMax32, \
           MAX(cnListPopCntMax16, \
               MAX(cnListPopCntMax8, 0))))
#endif // ! defined(cwListPopCntMax)

#define cnBitsLeftAtDl1     (cnBitsInD1)

// Bits in the second least significant digit of the key.  Not bits left.
#if ! defined(cnBitsInD2)
#define cnBitsInD2 \
    (((cnBitsLeftAtDl1) + (cnBitsPerDigit) <= (cnBitsPerWord)) \
        ? (cnBitsPerDigit) : (cnBitsPerWord) - (cnBitsLeftAtDl1))
#endif // ! defined(cnBitsInD2)

#define cnBitsLeftAtDl2     (cnBitsLeftAtDl1 + cnBitsInD2)

#if ! defined(cnBitsInD3)
#define cnBitsInD3 \
    (((cnBitsLeftAtDl2) + (cnBitsPerDigit) <= (cnBitsPerWord)) \
        ? (cnBitsPerDigit) : (cnBitsPerWord) - (cnBitsLeftAtDl2))
#endif // ! defined(cnBitsInD3)

#define cnBitsLeftAtDl3     (cnBitsLeftAtDl2 + cnBitsInD3)

#define cnBitsIndexSzAtTop \
    (cnBitsPerWord - nBL_from_nDL(cnDigitsPerWord - 1))

// cnDigitsPerWord makes assumptions about anDL_to_nBitsIndexSz[] and
// anDL_to_nBL[].  Yuck.
#if (cnBitsInD3 != cnBitsPerDigit)
#define cnDigitsPerWord \
    (DIV_UP(cnBitsPerWord - cnBitsLeftAtDl3, cnBitsPerDigit) + 3)
#else // (cnBitsInD3 != cnBitsPerDigit)
#define cnDigitsPerWord \
    (DIV_UP(cnBitsPerWord - cnBitsInD1 - cnBitsInD2, cnBitsPerDigit) + 2)
#endif // (cnBitsInD3 != cnBitsPerDigit)

// Default is -DEMBED_KEYS which implies -DUSE_T_ONE.
#if ! defined(NO_EMBED_KEYS)
#undef  EMBED_KEYS
#define EMBED_KEYS
#endif // ! defined(NO_EMBED_KEYS)

#if defined(EMBED_KEYS)
#undef  USE_T_ONE
#define USE_T_ONE
#endif // defined(EMBED_KEYS)

#if defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)
  #error Sorry, no SKIP_TO_BM_SW without USE_BM_SW.
#endif // defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)

#if defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)
  #if ! defined(DEPTH_IN_SW) && ! defined(LVL_IN_WR_HB)
      #error Sorry, no SKIP_TO_BM_SW without DEPTH_IN_SW or LEVEL_IN_WROOT...
  #endif // ! defined(DEPTH_IN_SW) && ! defined(LVL_IN_WR_HB)
#endif // defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)

// Default is -DTYPE_IS_ABSOLUTE.  Use -DTYPE_IS_RELATIVE to change it.
#if ! defined(TYPE_IS_RELATIVE)
#undef TYPE_IS_ABSOLUTE
#define TYPE_IS_ABSOLUTE
#endif // ! defined(TYPE_IS_RELATIVE)

#define T_SWITCH_BIT         0x08
#define T_SKIP_BIT           0x04
#if defined(CODE_BM_SW)
#define T_BM_SW_BIT          0x02
#endif // defined(CODE_BM_SW)
#define T_SW_OTHER_BIT       0x01

#define T_FULL_BM_SW_BIT  T_SW_OTHER_BIT

#if defined(CODE_XX_SW)
#define T_XX_SW_BIT       T_SW_OTHER_BIT
#endif // defined(CODE_XX_SW)

// Values for nType.
enum {
#if defined(SEPARATE_T_NULL)
    T_NULL, // no keys below
#endif // defined(SEPARATE_T_NULL)
#if (cwListPopCntMax != 0)
    T_LIST, // external list of keys
#endif // (cwListPopCntMax != 0)
#if defined(USE_T_ONE)
    T_ONE, // one-key external list when key is too big to be embedded
#endif // defined(USE_T_ONE)
    T_EMBEDDED_KEYS, // keys are embedded in the link
    T_BITMAP, // external bitmap leaf
#if defined(SKIP_TO_BITMAP)
    T_SKIP_TO_BITMAP = T_SKIP_BIT | T_BITMAP, // external bitmap leaf
#endif // defined(SKIP_TO_BITMAP)

    // All of the type values less than T_SWITCH are not switches.
    // All type values at T_SWITCH and greater are switches.
    T_SWITCH = T_SWITCH_BIT, // Uncompressed, close (i.e. no-skip) switch.
#if defined(CODE_XX_SW)
    T_XX_SW = T_SWITCH_BIT | T_XX_SW_BIT,
  #if defined(SKIP_TO_XX_SW) // doesn't work yet
    T_SKIP_TO_XX_SW = T_SWITCH_BIT | T_SKIP_BIT | T_XX_SW_BIT,
  #endif // defined(SKIP_TO_XX_SW) // doesn't work yet
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
    T_BM_SW = T_SWITCH_BIT | T_BM_SW_BIT,
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    // All link bits set, i.e. all links present.
    T_FULL_BM_SW = T_SWITCH_BIT | T_BM_SW_BIT | T_FULL_BM_SW_BIT,
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
#endif // defined(CODE_BM_SW)
    // T_SKIP_TO_SWITCH has to have the biggest value in this enum
    // if not DEPTH_IN_SW.  All of the bigger values have a meaning relative
    // to T_SKIP_TO_SWITCH.
    // Depth/level is determined by (nType - T_SKIP_TO_SWITCH).
    T_SKIP_TO_SWITCH = T_SWITCH_BIT | T_SKIP_BIT,
#if defined(SKIP_TO_BM_SW)
    T_SKIP_TO_BM_SW = T_SWITCH_BIT | T_SKIP_BIT | T_BM_SW_BIT,
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    T_SKIP_TO_FULL_BM_SW
        = T_SWITCH_BIT | T_SKIP_BIT | T_BM_SW_BIT | T_FULL_BM_SW_BIT,
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
#endif // defined(SKIP_TO_BM_SW)
};

#define T_SW_BASE  T_SKIP_TO_SWITCH // compatibility with old code

// Define and optimize nBitsIndexSz_from_nDL, nBitsIndexSz_from_nBL,
// nBL_from_nDL, nBL_from_nDL, et. al. based on ifdef parameters.

// nBitsIndexSz_from_nDL_NAX(_nDL)
#if ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))
  #define nBitsIndexSz_from_nDL_NAX(_nDL)  (cnBitsPerDigit)
  #define nBitsIndexSz_from_nBL_NAX(_nBL)  (cnBitsPerDigit)
#elif (cnBitsInD3 == cnBitsPerDigit)
  #define nBitsIndexSz_from_nDL_NAX(_nDL) \
    (((_nDL) == 2) ? cnBitsInD2 : cnBitsPerDigit)
  #define nBitsIndexSz_from_nBL_NAX(_nBL) \
    (((_nBL) == cnBitsLeftAtDl2) ? cnBitsInD2 : cnBitsPerDigit)
#else // ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))
  #define nBitsIndexSz_from_nDL_NAX(_nDL) \
    ( ((_nDL) == 2) ? cnBitsInD2 \
    : ((_nDL) == 3) ? cnBitsInD3 \
    : cnBitsPerDigit )
  #define nBitsIndexSz_from_nBL_NAX(_nBL) \
    ( ((_nBL) == cnBitsLeftAtDl2) ? cnBitsInD2 \
    : ((_nBL) == cnBitsLeftAtDl3) ? cnBitsInD3 \
    : cnBitsPerDigit )
#endif // ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))

// nBL_from_nDL_NAX(_nDL)
#if ( (cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit) \
                                      && (cnBitsInD1 == cnBitsPerDigit) )
    #define nBL_from_nDL_NAX(_nDL)  (cnBitsPerDigit * (_nDL))
#elif (cnBitsInD3 == cnBitsPerDigit)
    #define nBL_from_nDL_NAX(_nDL) \
        ( cnBitsLeftAtDl2 + ((_nDL) - 2) * cnBitsPerDigit )
#else // (cnBitsInD3 == cnBitsPerDigit) && ...
    #define nBL_from_nDL_NAX(_nDL) \
        ( (_nDL) >= 3 ? cnBitsLeftAtDl3 + ((_nDL) - 3) * cnBitsPerDigit \
        : cnBitsLeftAtDl2 )
#endif // (cnBitsInD3 == cnBitsPerDigit) && ...

// nBL_from_nDL_NAT(_nDL)
#if ( (cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit) \
                                      && (cnBitsInD1 == cnBitsPerDigit) )
    #define nBL_from_nDL_NAT(_nDL)  (cnBitsPerDigit * (_nDL))
#elif ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))
    #define nBL_from_nDL_NAT(_nDL) \
        ( (_nDL) == 1 ? cnBitsLeftAtDl1 \
        : cnBitsLeftAtDl1 + ((_nDL) - 1) * cnBitsPerDigit )
#elif (cnBitsInD3 == cnBitsPerDigit)
    #define nBL_from_nDL_NAT(_nDL) \
        ( (_nDL) == 1 ? cnBitsLeftAtDl1 \
        : (_nDL) == 2 ? cnBitsLeftAtDl2 \
        : cnBitsLeftAtDl2 + ((_nDL) - 2) * cnBitsPerDigit )
#else // (cnBitsInD3 == cnBitsPerDigit) && ...
    #define nBL_from_nDL_NAT(_nDL) \
        ( (_nDL) == 1 ? cnBitsLeftAtDl1 \
        : (_nDL) == 2 ? cnBitsLeftAtDl2 \
        : (_nDL) == 3 ? cnBitsLeftAtDl3 \
        : cnBitsLeftAtDl3 + ((_nDL) - 3) * cnBitsPerDigit )
#endif // (cnBitsInD3 == cnBitsPerDigit) && ...

// nDL_from_nBL(_nBL)
// nDL_from_nBL_NIB(_nBL) _nBL must be an integral number of digits 
#if ( (cnBitsInD1 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit) \
                                      && (cnBitsInD3 == cnBitsPerDigit) )

    // Rounding up is not free since we don't otherwise need to add a               // constant before (or after) dividing.
    // But we have to round up when this is used to figure the depth of a skip
    // link from a common prefix which may be a non-integral number of digits.
    #define nDL_from_nBL(_nBL)  (DIV_UP((_nBL), cnBitsPerDigit))

    // In cases where rounding is not necessary we can use this.
    #define nDL_from_nBL_NIB(_nBL)  ((_nBL) / cnBitsPerDigit)

#else // (cnBitsInD1 == cnBitsPerDigit) ...

  // nDL_from_nBL(_nBL)
  #if ((cnBitsInD2 == cnBitsPerDigit) && (cnBitsInD3 == cnBitsPerDigit))
    // Rounding up is free since we already have to add a constant before
    // (or after) dividing.
    #define nDL_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsInD1 ) ? 1 \
        : 1 + DIV_UP((_nBL) - cnBitsLeftAtDl1, cnBitsPerDigit) )
  #elif (cnBitsInD3 == cnBitsPerDigit)
    // Rounding up is free since we already have to add a constant before
    // (or after) dividing.
    #define nDL_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsLeftAtDl1 ) ? 1 \
        : ((_nBL) <= cnBitsLeftAtDl2) ? 2 \
        : 2 + DIV_UP((_nBL) - cnBitsLeftAtDl2, cnBitsPerDigit) )
  #else // (cnBitsInD2 == cnBitsPerDigit) && ...
    // Rounding up is free since we already have to add a constant before
    // (or after) dividing.
    #define nDL_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsLeftAtDl1 ) ? 1 \
        : ((_nBL) <= cnBitsLeftAtDl2) ? 2 \
        : ((_nBL) <= cnBitsLeftAtDl3) ? 3 \
        : 3 + DIV_UP((_nBL) - cnBitsLeftAtDl3, cnBitsPerDigit) )
  #endif // (cnBitsInD2 == cnBitsPerDigit) ...

    #define nDL_from_nBL_NIB(_nBL)  nDL_from_nBL(_nBL)

#endif // (cnBitsInD1 == cnBitsPerDigit) && ...

#if (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)

// nBitsIndexSz_from_nDL(_nDL)
#if (cnBitsInD1 == cnBitsPerDigit)
    #define nBitsIndexSz_from_nDL(_nDL)  (nBitsIndexSz_from_nDL_NAX(_nDL))
    #define nBitsIndexSz_from_nBL(_nBL)  (nBitsIndexSz_from_nBL_NAX(_nBL))
#else // (cnBitsInD1 == cnBitsPerDigit)
    #define nBitsIndexSz_from_nDL(_nDL) \
        ( ((_nDL) <= 1) ? cnBitsLeftAtDl1 : nBitsIndexSz_from_nDL_NAX(_nDL) )
    #define nBitsIndexSz_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsInD1) ? cnBitsLeftAtDl1 \
                                 : nBitsIndexSz_from_nBL_NAX(_nBL) )
#endif // (cnBitsInD1 == cnBitsPerDigit)

// nBL_from_nDL(_nDL)
#define nBL_from_nDL(_nDL)  (nBL_from_nDL_NAT(_nDL))

#else // (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)

// nBitsIndexSz_from_nDL(_nDL)
#if (cnBitsInD1 == cnBitsPerDigit)
  #define nBitsIndexSz_from_nDL(_nDL) \
    ( ((_nDL) < cnDigitsPerWord) ? nBitsIndexSz_from_nDL_NAX(_nDL) \
    : cnBitsIndexSzAtTop )
  #define nBitsIndexSz_from_nBL(_nBL) \
    ( ((_nBL) < cnBitsPerWord) ? nBitsIndexSz_from_nBL_NAX(_nBL) \
    : cnBitsIndexSzAtTop )
#else // (cnBitsInD1 == cnBitsPerDigit)
  // Do we need this to be valid for _nDL < 1?
  #define nBitsIndexSz_from_nDL(_nDL) \
    ( ((_nDL) <= 1) ? cnBitsInD1 \
    : ((_nDL) < cnDigitsPerWord) ? nBitsIndexSz_from_nDL_NAX(_nDL) \
    : cnBitsIndexSzAtTop )
  #define nBitsIndexSz_from_nBL(_nBL) \
    ( ((_nBL) <= cnBitsInD1) ? cnBitsInD1 \
    : ((_nBL) < cnBitsPerWord) ? nBitsIndexSz_from_nBL_NAX(_nBL) \
    : cnBitsIndexSzAtTop )
#endif // (cnBitsInD1 == cnBitsPerDigit)

// nBL_from_nDL(_nDL)
#define nBL_from_nDL(_nDL) \
    ( (_nDL) < cnDigitsPerWord ? nBL_from_nDL_NAT(_nDL) : cnBitsPerWord )

#endif // (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)

#if (cnBitsIndexSzAtTop == cnBitsPerDigit)
  #define nBW_from_nBL_NAB3(_nBL)  (cnBitsPerDigit)
#else // (cnBitsIndexSzAtTop == cnBitsPerDigit)
  #define nBW_from_nBL_NAB3(_nBL) \
    (((_nBL) >= cnBitsPerWord) ? cnBitsIndexSzAtTop : (cnBitsPerDigit))
#endif // (cnBitsIndexSzAtTop == cnBitsPerDigit)

// Default is -UBPD_TABLE.  This causes the table to exist and allows
// us to reference it in cases when we think it will be faster.
// Doing a table lookup is slower than doing a calculation, but it
// may be faster than evaluating a complicated conditional expression.
// I have not seen any evidence that the table lookup is ever faster.
// Lookup tables theoretically support depth-based bits per digit
// instead of a constant bits per digit throughout the tree.
// But we have created tables and macros that allow us to mix/match
// table use and calculation.

#if defined(BPD_TABLE)

  #define nDL_to_nBitsIndexSz(_nDL)  (anDL_to_nBitsIndexSz[_nDL])
  #define nDL_to_nBL(_nDL)           (anDL_to_nBL[_nDL])
  #define nBL_to_nDL(_nBL)           (anBL_to_nDL[_nBL])

  #define nBL_to_nBitsIndexSz(_nBL)  nDL_to_nBitsIndexSz(nBL_to_nDL(_nBL))

#endif // defined(BPD_TABLE)

#if ! defined(BPD_TABLE)

  #define nDL_to_nBitsIndexSz(_nDL)  (nBitsIndexSz_from_nDL(_nDL))
  #define nBL_to_nBitsIndexSz(_nBL)  (nBitsIndexSz_from_nBL(_nBL))
  #define nDL_to_nBL(_nDL)           (nBL_from_nDL(_nDL))
  #define nBL_to_nDL(_nBL)           (nDL_from_nBL(_nBL))

#endif // ! defined(BPD_TABLE)

#define nDL_to_nBL_NAX(_nDL)          (nBL_from_nDL_NAX(_nDL))
#define nBL_to_nBitsIndexSzNAX(_nBL)  (nBitsIndexSz_from_nBL_NAX(_nBL))
#define nDL_to_nBitsIndexSzNAX(_nDL)  (nBitsIndexSz_from_nDL_NAX(_nDL))
#define nDL_to_nBL_NAT(_nDL)          (nBL_from_nDL_NAT(_nDL))
#define nDL_to_nBitsIndexSzNAT(_nDL)  (nDL_to_nBitsIndexSz(_nDL))

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

// Extract nType from *pwRoot.
static inline int wr_nType(Word_t wRoot) { return wRoot & cnMallocMask; }
static inline int Get_nType(Word_t* pwRoot) { return wr_nType(*pwRoot); }

#define set_wr_nType(_wr, _type)  ((_wr) = ((_wr) & ~cnMallocMask) | (_type))
// Set  nType in *pwRoot.
static inline void
Set_nType(Word_t* pwRoot, int nType)
{
    set_wr_nType(*pwRoot, nType);
}

// Change the type field in *pwRoot from a skip a non-skip.
static inline void
Clr_bIsSkip(Word_t* pwRoot)
{
    Set_nType(pwRoot, Get_nType(pwRoot) & ~T_SKIP_BIT);
}

// Extract pwr, i.e. the next pwRoot, from *pwRoot.
static inline Word_t* wr_pwr(Word_t wRoot) {
    return (Word_t*)(wRoot & cwVirtAddrMask & ~cnMallocMask);
}
static inline Word_t* Get_pwr(Word_t* pwRoot) { return wr_pwr(*pwRoot); }

// Set nType in *pwRoot.
static inline void set_pwr_nType(Word_t *pwRoot, int nType) {
    *pwRoot = (*pwRoot & ~cnMallocMask) | nType;
}

// Set pwRoot (aka pwr) in *pwRoot.
static inline void set_pwr_pwr(Word_t *pwRoot, Word_t *pwr) {
    *pwRoot = (*pwRoot & (~cwVirtAddrMask | cnMallocMask)) | (Word_t)pwr;
}

static inline void set_pwr_pwr_nType(Word_t *pwRoot, Word_t *pwr, int nType) {
    *pwRoot = (*pwRoot & ~cwVirtAddrMask) | (Word_t)pwr | nType;
}

// Set the pwRoot field in wRoot.
#define set_wr_pwr(_wr, _pwr) \
        ((_wr) = ((_wr) & (~cwVirtAddrMask | cnMallocMask)) | (Word_t)(_pwr))

// Set the pwRoot and nType fields in wRoot.
#define set_wr(_wr, _pwr, _type) \
        ((_wr) = ((_wr) & ~cwVirtAddrMask) | (Word_t)(_pwr) | (_type))

// Default is -DT_ONE_CALC_POP.
// T_ONE_CALC_POP means we don't use any bits in a wRoot/link/bucket to
// hold the pop count.  We calculate the pop count based on the contents
// of the key suffix slots.
#if ! defined(NO_T_ONE_CALC_POP)
#undef T_ONE_CALC_POP
#define T_ONE_CALC_POP
#endif // ! defined(NO_T_ONE_CALC_POP)

// Default is -UFILL_W_KEY.
// FILL_W_KEY means unused key suffix slots in a T_EMBEDDED_KEYS
// wRoot/link/bucket are filled with a copy of the smallest key suffix in
// the list. This is independent of the order in which the key suffixes are
// sorted.
// If FILL_W_KEY is not defined, then the empty slots are filled with zero
// or -1 depending on FILL_WITH_ONES.
#if defined(FILL_W_KEY) && defined(T_ONE_CALC_POP)
#error Sorry, no FILL_W_KEY and T_ONE_CALC_POP.
#endif // defined(FILL_W_KEY) && defined(T_ONE_CALC_POP)

#if defined(FILL_W_KEY) && defined(FILL_WITH_ONES)
#error Sorry, no FILL_W_KEY and FILL_WITH_ONES.
#endif // defined(FILL_W_KEY) && defined(FILL_WITH_ONES)

// Default is -UMASK_EMPTIES.
// See EmbeddedListHasKey.

#if defined(NO_TYPE_IN_XX_SW)
// We need some way to represent an empty list when we have no type field.
// Zero is no good because it is a valid wRoot for nBL == 8 representing
// a list with a single key and that key being zero.
// We use ZERO_POP_MAGIC which would otherwise be an invalid value for wRoot
// if nBL is less than or equal to cnBitsPerWord - cnBitsMallocMask.
// We make sure at least one bit is set in cnMallocMask which is either an
// invalid zero-fill bit (making the value invalid all by itself) or it
// indicates that the list is full.  With the high bit in wRoot set and
// all of the bits between the high bit and the type field bits clear
// means the first key is bigger than the second so the list is not sorted
// hence the value is invalid.
// We gave ZERO_POP_MAGIC type value of T_EMBEDDED_KEYS just because we
// could and we thought it might come in handy.
// Our old embedded list with a type field and a 3-bit pop count where
// pop-field=0 means pop=1 cannot represent an empty list using only the
// pop count.  In that case we could represent an empty list for nBL small
// enough that two keys will fit with
// (EXP(63) + EXP(cnBitsMallocMask) + T_EMBEDDED_KEYS).
// I don't know if that code works anymore.
// Is it possible that we are going to want to sort the list in the other
// order for JudyL?
// Enough talk for now.  We'll come back to these other cases.
#if defined(REVERSE_SORT_EMBEDDED_KEYS)
#define ZERO_POP_MAGIC  1
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)
#define ZERO_POP_MAGIC  (EXP(cnBitsPerWord - 1) + T_EMBEDDED_KEYS)
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // defined(NO_TYPE_IN_XX_SW)

#if defined(NO_TYPE_IN_XX_SW) // && defined(HANDLE_BLOWOUTS)
    // Identify blowouts using (wRoot & BLOWOUT_MASK(nBL) == ZERO_POP_MAGIC).
    #define BLOWOUT_MASK(_nBL) \
        (((EXP(_nBL) + 1) << (cnBitsPerWord - (_nBL) - 1)) + cnMallocMask)
#endif // defined(NO_TYPE_IN_XX_SW) && defined(HANDLE_BLOWOUTS)

#if defined(CODE_XX_SW) && defined(NO_TYPE_IN_XX_SW)
    #define nBL_to_nBitsType(_nBL) \
        (((_nBL) < nDL_to_nBL(2)) ? 0 : cnBitsMallocMask)
#else // defined(CODE_XX_SW) && defined(NO_TYPE_IN_XX_SW)
    #define nBL_to_nBitsType(_nBL)  cnBitsMallocMask
#endif // defined(CODE_XX_SW) && defined(NO_TYPE_IN_XX_SW)

#if defined(USE_T_ONE)
  #if defined(T_ONE_CALC_POP)

#define nBL_to_nBitsPopCntSz(_nBL)  0

#if defined(REVERSE_SORT_EMBEDDED_KEYS)
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)

static inline int
wr_nPopCnt(Word_t wRoot, int nBL)
{
    Word_t wKeys = wRoot;
#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        if (wRoot == ZERO_POP_MAGIC) { return 0; }
    } else
#endif // defined(NO_TYPE_IN_XX_SW)
    {
        // The code below assumes the pop count is not zero.
        // Why do we know the link is non-empty here but not for the
        // NO_TYPE_IN_XX_SW with (nBL >= nDL_to_nBL(2)) case above?
        // Because, in this case, the type field exists and tells the
        // caller the link is not empty and the caller does not call us.
        // Unfortunately, ZERO_POP_MAGIC is a valid value when there is a
        // real type value so we can't use it in that case.  I think we
        // could devise a magic number that would work in both cases (see
        // above) but I think it might make it just a little trickier to
        // handle blowouts without having a type field and there is no big
        // motivator to make it work in both cases at this point.
        assert(wr_nType(wRoot) == T_EMBEDDED_KEYS);
        wKeys &= ~MSK(nBL_to_nBitsType(nBL) + nBL_to_nBitsPopCntSz(nBL));
    }
    wKeys |= EXP(cnBitsPerWord - 1);
    int ffs = __builtin_ffsll(wKeys);
    int nPopCnt = ((cnBitsPerWord - ffs) / nBL) + 1;
    return nPopCnt;
}

#define set_wr_nPopCnt(_wr, _nBL, _nPopCnt)

#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)

  #else // defined(T_ONE_CALC_POP)

// Default is -DEMBEDDED_LIST_FIXED_POP.
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

// wr_nPopCnt(_wr, _nBL) gets the pop count for a list of embedded keys.
// For embedded keys the pop cnt bits are just above the type field.
// A value of zero means a pop cnt of one. 
#define     wr_nPopCnt(_wr, _nBL) \
  ((int)((((_wr) >> nBL_to_nBitsType(_nBL)) & MSK(nBL_to_nBitsPopCntSz(_nBL))) + 1))

#define set_wr_nPopCnt(_wr, _nBL, _nPopCnt) \
    SetBits(&(_wr), nBL_to_nBitsPopCntSz(_nBL), nBL_to_nBitsType(_nBL), \
            (_nPopCnt) - 1)

  #endif // defined(T_ONE_CALC_POP)
#endif // defined(USE_T_ONE)

static inline int
EmbeddedListPopCntMax(int nBL)
{
    int nBitsForKeys = cnBitsPerWord;
    nBitsForKeys -= nBL_to_nBitsType(nBL) + nBL_to_nBitsPopCntSz(nBL);
    return nBitsForKeys / nBL;
}

#if defined(EXTRA_TYPES)
  #define tp_bIsSwitch(_tp)  (((_tp) & cnMallocMask) >= T_SWITCH)
#else // defined(EXTRA_TYPES)
  #define tp_bIsSwitch(_tp)  ((_tp) >= T_SWITCH)
#endif // defined(EXTRA_TYPES)
  #define tp_bIsSkip(_tp)  (((_tp) & T_SKIP_BIT) != 0)
#if defined(SKIP_TO_BM_SW)
  #define tp_bIsBmSw(_tp)  (((_tp) & T_BM_SW_BIT) != 0)
#else // defined(SKIP_TO_BM_SW)
  #define tp_bIsBmSw(_tp)  (((_tp) & T_BM_SW_BIT) && ! ((_tp) & T_SKIP_BIT))
#endif // defined(SKIP_TO_BM_SW)
#if defined(CODE_XX_SW)
  #if defined(USE_BM_SW)
    #define tp_bIsXxSw(_tp)  (!((_tp) & T_BM_SW_BIT) && ((_tp) & T_XX_SW_BIT))
  #else // defined(USE_BM_SW)
    #define tp_bIsXxSw(_tp)  ((_tp) & T_XX_SW_BIT)
  #endif // defined(USE_BM_SW)
#endif // defined(CODE_XX_SW)

#define wr_bIsSwitch(_wr)  (tp_bIsSwitch(wr_nType(_wr)))

// Bit fields in the upper bits of of wRoot.
// Lvl is the level of the node pointed to.
// Can we use lvl to id skip instead of a bit in the type field?
// XxSwWidth is the width of the switch.
// ListPopCnt is the number of keys in the list.
// A field at the end is faster to extract than a field in the middle.
#define cnBitsLvl  8 // 8 is easier to read in debug output than 7
#define cnLsbLvl  (cnBitsPerWord - cnBitsLvl)
#define cnBitsXxSwWidth   6
#if defined(SKIP_TO_XX_SW)
#define cnLsbXxSwWidth  cnBitsVirtAddr
#else // defined(SKIP_TO_XX_SW)
#define cnLsbXxSwWidth  (cnBitsPerWord - cnBitsXxSwWidth)
#endif // defined(SKIP_TO_XX_SW)
#define cnBitsListPopCnt  8
#if defined(SKIP_TO_LIST)
#define cnLsbListPopCnt  cnBitsVirtAddr
#else // defined(SKIP_TO_LIST)
#define cnLsbListPopCnt  (cnBitsPerWord - cnBitsListPopCnt)
#endif // defined(SKIP_TO_LIST)

#if defined(CODE_XX_SW)

// Get the width of the branch in bits.
static inline int
Get_nBW(Word_t *pwRoot)
{
    int nBW = GetBits(*pwRoot, cnBitsXxSwWidth, cnLsbXxSwWidth);
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
    return nBW;
}

#define pwr_nBW(_pwRoot)  Get_nBW(_pwRoot)

// Set the width of the branch in bits.
static inline void
Set_nBW(Word_t *pwRoot, int nBW)
{
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
    SetBits(pwRoot, cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
}

#define set_pwr_nBW(_pwRoot, _nBW)  Set_nBW((_pwRoot), (_nBW))

#if ! defined(TYPE_IS_RELATIVE)

// Get the level of the branch in bits.
static inline int
pwr_nBL(Word_t *pwRoot)
{
    int nBL = GetBits(*pwRoot, cnBitsLvl, cnLsbLvl);
    assert(nBL <= (int)MSK(cnBitsLvl));
    return nBL;
}

// Set the level of the branch in bits.
static inline void
set_pwr_nBL(Word_t *pwRoot, int nBL)
{
    assert(nBL <= (int)MSK(cnBitsLvl));
    SetBits(pwRoot, cnBitsLvl, cnLsbLvl, nBL);
}

#endif // ! defined(TYPE_IS_RELATIVE)

static inline Word_t
pw_wPrefix(Word_t *pw, int nBL)
{
    return GetBits(*pw, /* nBits */ cnBitsPerWord - nBL, /* lsb */ nBL);
}

static inline void
set_pw_wPrefix(Word_t *pw, int nBL, Word_t wKey)
{
    SetBits(pw, /* nBits */ cnBitsPerWord - nBL, /* lsb */ nBL, wKey);
}

static inline Word_t
pw_wPopCnt(Word_t *pw, int nBL)
{
    return GetBits(*pw, /* nBits */ nBL, /* lsb */ 0) + 1;
}

static inline void
set_pw_wPopCnt(Word_t *pw, int nBL, Word_t wPopCnt)
{
    SetBits(pw, /* nBits */ nBL, /* lsb */ 0, wPopCnt - 1);
}

#endif // defined(CODE_XX_SW)

#if defined(TYPE_IS_ABSOLUTE)

#if defined(LVL_IN_WR_HB)

  #if defined(SKIP_TO_BITMAP)
    #define wr_nBL(_wr) \
        (assert((tp_bIsSwitch(wr_nType(_wr)) && tp_bIsSkip(wr_nType(_wr))) \
                || (wr_nType(_wr) == T_SKIP_TO_BITMAP)), \
            (int)GetBits((_wr), cnBitsLvl, cnLsbLvl))
  #else // defined(SKIP_TO_BITMAP)
    #define wr_nBL(_wr) \
        (assert(tp_bIsSwitch(wr_nType(_wr)) && tp_bIsSkip(wr_nType(_wr))), \
            (int)GetBits((_wr), cnBitsLvl, cnLsbLvl))
  #endif // defined(SKIP_TO_BITMAP)

  #define wr_nDL(_wr)  nBL_to_nDL(wr_nBL(_wr))

  #define set_wr_nBL(_wr, _nBL) \
      (assert((_nBL) <= (int)MSK(cnBitsLvl)), \
          set_wr_nType((_wr), T_SKIP_TO_SWITCH), \
          SetBits(&(_wr), cnBitsLvl, cnLsbLvl, (_nBL)))

  #define set_wr_nDL(_wr, _nDL)  set_wr_nBL((_wr), nDL_to_nBL(_nDL))

#else // defined(LVL_IN_WR_HB)

#if defined(DEPTH_IN_SW)
// DEPTH_IN_SW directs us to use the low bits of sw_wPrefixPop for absolute
// depth instead of encoding it into the type field directly.
// It means we can't use the low bits of sw_wPrefixPop for pop.  So we
// define POP_WORD and use a separate word.
// We assume the value we put into the low bits will will fit in the number
// of bits used for the pop count at nDL == 2.  Or maybe
// it doesn't matter since we always create an embedded bitmap when
// EXP(nBL) <= sizeof(Link_t) * 8.
#define POP_WORD

// As it stands we always get the absolute type from sw_wPrefixPop if
// DEPTH_IN_SW.  We assume the macro is used only when it is known that
// we have a skip link.  We could enhance it to use one type value to indicate
// that we have to go to sw_wPrefixPop and use any other values that we
// have available to represent some key absolute depths.
// But why?  There is no real performance win since we have to look at the
// prefix word anyway.
// Should we enhance wr_nDL to take pwRoot and wRoot and nDL?
  #define wr_nBL(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), \
       w_wPopCntBL(PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)), \
                   cnBitsLeftAtDl2))

  #define wr_nDL(_wr)  nBL_to_nDL(wr_nBL(_wr))

  #define set_wr_nBL(_wr, _nBL) \
      (assert((_nBL) >= cnBitsLeftAtDl2), \
       set_wr_nType((_wr), T_SKIP_TO_SWITCH), \
       (PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
           = ((PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
                   & ~wPrefixPopMaskBL(cnBitsLeftAtDl2)) \
               | (_nBL))))

  #define set_wr_nDL(_wr, _nDL)  set_wr_nBL((_wr), nDL_to_nBL(_nDL))

#else // defined(DEPTH_IN_SW)

// What is the minimum value of nDL we need nType to represent?
// We don't use tp_to_nDL for non-skip links and we don't support skip
// directly to bitmap so the smallest nDL we need is nDL == 2.  Also,
// We have to test for nDL == 1 before looping back to the switch statement
// that checks nType if EXP(cnBitsInD1) <= sizeof(Link_t) * 8 because there
// is no room for a type field when all the bits are used for an embedded
// bitmap.  But the nDL we use for this test is one less than the nDL of
// the containing switch which can't be lower than nDL == 2.
// If EXP(cnBitsInD1) > sizeof(Link_t) * 8 we don't want to
// waste the conditional branch so we rely on T_BITMAP but we still have to
// support skipping to a switch at nDL == 2.
// It wouldn't be hard to avoid the conditional branch in a case
// where EXP(cnBitsInD1) < sizeof(Link_t) * 8.
// There would be an extra word in the link and we could push the bitmap to
// the end of the link and use the regular type field.  But that has not
// been coded yet.  And it is probably not worth even the small effort.
// We could also just coopt the type field from the embedded
// bitmap and relocate the bitmap bits.  But I don't think that would save us
// any conditional branches.
// We could be more creative w.r.t. mapping our scarce type values to nDL
// values.  E.g. start at the top instead of the bottom, count by twos,
// lookup table, ...  But why?  We're going to use DEPTH_IN_SW.  This code
// is an anachronism.
  #define tp_to_nDL(_tp)   ((_tp)  - T_SW_BASE + 2)
  #define nDL_to_tp(_nDL)  ((_nDL) + T_SW_BASE - 2)

  #define wr_nDL(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), tp_to_nDL(wr_nType(_wr)))

  #define wr_nBL(_wr)  nDL_to_nBL(tp_to_nDL(wr_nType(_wr)))

  #define set_wr_nDL(_wr, _nDL) \
      (assert(nDL_to_tp(_nDL) >= T_SKIP_TO_SWITCH), \
       set_wr_nType((_wr), nDL_to_tp(_nDL)))

  #define set_wr_nBL(_wr, _nBL) \
      set_wr_nDL((_wr), nBL_to_nDL(_nBL))

#endif // defined(DEPTH_IN_SW)

#endif // defined(LVL_IN_WR_HB)

#else // defined(TYPE_IS_ABSOLUTE)

#if defined(LVL_IN_WR_HB)

  #define wr_nDS(_wr)  (assert(tp_bIsSkip(wr_nType(_wr))), (_wr) >> 58)

  #define set_wr_nDS(_wr, _nDS) \
          (set_wr_nType((_wr), T_SKIP_TO_SWITCH), \
           ((_wr) = ((_wr) & MSK(58) | (Word_t)(_nDS) << 58)))

#else // defined(LVL_IN_WR_HB)

// These two macros should be used sparingly outside of wr_nDS and set_wr_nDS.
// Why?  Because the type field in wRoot does not contain this information
// for DEPTH_IN_SW so code that uses these macros may need to be ifdef'd
// to do something like what is done in wr_nDS anyway.
#define tp_to_nDS(_tp)   ((_tp)  - T_SW_BASE + 1)
#define nDS_to_tp(_nDS)  ((_nDS) + T_SW_BASE - 1)

#if defined(DEPTH_IN_SW)
// DEPTH_IN_SW directs us to use the low bits of sw_wPrefixPop for skip count
// for skip links instead of encoding skip count into the type field directly.
// We use T_SW_BASE to indicate no skip.  And T_SW_BASE + 1 to indicate that
// a non-zero skip count is in sw_wPrefixPop.
// It means we can't use the low bits of sw_wPrefixPop for pop.  So we
// define POP_WORD and use a separate word.
// We assume the value we put into the low bits will will fit in the number
// of bits used for the pop count in the switch so we have
// to be careful when choosing the level of the lowest level switch and
// the number of digits above it.
#define POP_WORD

  // As it stands we always get the skip count from sw_wPrefixPop if
  // DEPTH_IN_SW.  We could enhance it to use one type value to indicate
  // that we have to go to sw_wPrefixPop and use any other values that we
  // have available to represent some key skip counts.  But why?
  #define wr_nDS(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), \
       assert(w_wPopCnt(PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)), \
                        /*nDL*/ 1) \
                      >= 1), \
       w_wPopCnt(PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)), /*nDL*/ 1))

  #define set_wr_nDS(_wr, _nDS) \
      (assert((_nDS) >= 1), \
       set_wr_nType((_wr), T_SKIP_TO_SWITCH), \
       /* put skip cnt in the PP pop field but use DL=1 for mask */ \
       (PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
           = ((PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
                   & ~wPrefixPopMask(/* nDL */ 2)) \
               | (_nDS))))

#else // defined(DEPTH_IN_SW)

  #define wr_nDS(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), \
       assert(tp_to_nDS(wr_nType(_wr)) >= 1), \
       tp_to_nDS(wr_nType(_wr)))

  #define set_wr_nDS(_wr, _nDS) \
      (assert(nDS_to_tp(_nDS) >= T_SKIP_TO_SWITCH), \
       set_wr_nType((_wr), nDS_to_tp(_nDS)))

#endif // defined(DEPTH_IN_SW)

#endif // defined(LVL_IN_WR_HB)

#endif // defined(TYPE_IS_ABSOLUTE)

// methods for Switch (and aliases)

#define wPrefixPopMaskNotAtTop(_nDL)    (MSK(nDL_to_nBL_NAT(_nDL)))
#define wPrefixPopMaskNotAtTopBL(_nBL)  (MSK(_nBL))

#define wPrefixPopMask(_nDL) \
    (((_nDL) == cnDigitsPerWord) ? (Word_t)-1 : wPrefixPopMaskNotAtTop(_nDL))

#define wPrefixPopMaskBL(_nBL) \
    (((_nBL) == cnBitsPerWord) ? (Word_t)-1 : wPrefixPopMaskNotAtTopBL(_nBL))

#define w_wPrefix(  _w, _nDL)  ((_w) & ~wPrefixPopMask  (_nDL))
#define w_wPrefixBL(_w, _nBL)  ((_w) & ~wPrefixPopMaskBL(_nBL))
#define w_wPopCnt(  _w, _nDL)  ((_w) &  wPrefixPopMask  (_nDL))
#define w_wPopCntBL(_w, _nBL)  ((_w) &  wPrefixPopMaskBL(_nBL))

#define w_wPrefixNotAtTop(_w, _nDL)  ((_w) & ~wPrefixPopMaskNotAtTop(_nDL))
#define w_wPrefixNotAtTopBL(_w, _nBL)  ((_w) & ~wPrefixPopMaskNotAtTopBL(_nBL))
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
// PP_IN_LINK doesn't work without OLD_LISTS.
// The whole purpose of new lists was to move pop count to the end of
// the list so we don't have to waste a bucket at the beginning just for
// pop count.  But we don't put the pop count in the list for PP_IN_LINK.
// Except at the top.  We'll still be wasting that bucket for PP_IN_LINK if we
// are aligning word-size lists.  But we're not currently aligning word-size
// lists because we're not currently doing psplit search word.
// We can worry about aligned word size lists later.  It should be easy
// to distinguish the top level list from other word size lists and treat
// them differently.
  #if ! defined(OLD_LISTS)
      #define OLD_LISTS
  #endif // ! defined(OLD_LISTS)
#if defined(DEPTH_IN_SW) || defined(POP_WORD)
// Relocating the pop out of PP requires quite a few code changes.
// It would be nice for depth, prefix and pop to share the same word.
#error Sorry, no PP_IN_LINK && (DEPTH_IN_SW || POP_WORD). 
#endif // defined(DEPTH_IN_SW) || defined(POP_WORD)
#else // defined(PP_IN_LINK)
#define PWR_wPrefixPop(_pwRoot, _pwr)  ((_pwr)->sw_wPrefixPop)
#endif // defined(PP_IN_LINK)

#define PWR_wPrefix(_pwRoot, _pwr, _nDL) \
    (w_wPrefix(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPrefixBL(_pwRoot, _pwr, _nBL) \
    (w_wPrefixBL(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nBL)))

#define PWR_wPrefixNAT(_pwRoot, _pwr, _nDL) \
    (w_wPrefixNotAtTop(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPrefixNATBL(_pwRoot, _pwr, _nBL) \
    (w_wPrefixNotAtTopBL(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nBL)))

#define set_w_wPrefix(_w, _nDL, _key) \
    ((_w) = (((_w) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_w_wPrefixBL(_w, _nBL, _key) \
    ((_w) = (((_w) & wPrefixPopMaskBL(_nBL)) \
            | ((_key) & ~wPrefixPopMaskBL(_nBL))))

#define set_w_wPopCnt(_w, _nDL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_w_wPopCntBL(_w, _nBL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL))))

#define set_PWR_wPrefix(_pwRoot, _pwr, _nDL, _key) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_PWR_wPrefixBL(_pwRoot, _pwr, _nBL, _key) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMaskBL(_nBL)) \
            | ((_key) & ~wPrefixPopMaskBL(_nBL))))

#if defined(POP_WORD)

#define PWR_wPopWord(_pwRoot, _pwr)  ((_pwr)->sw_wPopWord)

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (w_wPopCnt(PWR_wPopWord((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPopCntBL(_pwRoot, _pwr, _nBL) \
    (w_wPopCntBL(PWR_wPopWord((_pwRoot), (_pwr)), (_nBL)))

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) \
        = ((PWR_wPopWord((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) \
        = ((PWR_wPopWord((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL))))

#else // defined(POP_WORD)

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (w_wPopCnt(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPopCntBL(_pwRoot, _pwr, _nBL) \
    (w_wPopCntBL(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nBL)))

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL))))

#endif // defined(POP_WORD)

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

#if defined(PSPLIT_PARALLEL)
#define cbPsplitParallel 1
#else // defined(PSPLIT_PARALLEL)
#define cbPsplitParallel 0
#endif // defined(PSPLIT_PARALLEL)

#if defined(PARALLEL_128)
#define cbParallel128 1
#else // defined(PARALLEL_128)
#define cbParallel128 0
#endif // defined(PARALLEL_128)

#if defined(PSPLIT_SEARCH_WORD)
#define cbPsplitSearchWord 1
#else // defined(PSPLIT_SEARCH_WORD)
#define cbPsplitSearchWord 0
#endif // defined(PSPLIT_SEARCH_WORD)

#if defined(ALIGN_LISTS)
#define cbAlignLists 1
#else // defined(ALIGN_LISTS)
#define cbAlignLists 0
#endif // defined(ALIGN_LISTS)

#if defined(ALIGN_LIST_ENDS)
#define cbAlignListEnds 1
#else // defined(ALIGN_LIST_ENDS)
#define cbAlignListEnds 0
#endif // defined(ALIGN_LIST_ENDS)

#if defined(LIST_END_MARKERS)
#define cbListEndMarkers 1
#else // defined(LIST_END_MARKERS)
#define cbListEndMarkers 0
#endif // defined(LIST_END_MARKERS)

#if defined(PP_IN_LINK)
  // POP_SLOT tells ListWords if we need a slot in the leaf for a pop count
  // that is not included in N_LIST_HDR_KEYS.
  #define POP_SLOT(_nBL)  (((_nBL) >= cnBitsPerWord) && (cnDummiesInList == 0))
#else // defined(PP_IN_LINK)
  // POP_SLOT tells ListWords if we need a slot in the leaf for a pop count
  // that is not included in N_LIST_HDR_KEYS.
  // N_HDR_KEYS incorporates this for ! PP_IN_LINK so don't add it again
  // for ls_pxKeys.
  #define POP_SLOT(_nBL)  (1)
#endif // defined(PP_IN_LINK)

#define ls_nSlotsInList(_wPopCnt, _nBL, _nBytesKeySz) \
( \
    ( ! cbAlignLists \
        || (cbPsplitParallel \
            && ! cbPsplitSearchWord && ((_nBytesKeySz) == sizeof(Word_t)))) \
    ? ( assert(cbAlignLists == cbAlignListEnds), \
        ALIGN_UP(cnDummiesInList * sizeof(Word_t) / (_nBytesKeySz) \
                     + 2 * cbListEndMarkers \
                     + (_wPopCnt) \
                     + POP_SLOT(_nBL) \
                     + sizeof(Word_t) / (_nBytesKeySz), \
                 2 * sizeof(Word_t) / (_nBytesKeySz)) \
             - sizeof(Word_t) / (_nBytesKeySz) ) \
    : ( ALIGN_UP(ALIGN_UP(cnDummiesInList * sizeof(Word_t) / (_nBytesKeySz), \
                          sizeof(Bucket_t) / (_nBytesKeySz)) \
                     + 2 * sizeof(Bucket_t) \
                         / (_nBytesKeySz) * cbListEndMarkers \
                     + cbAlignListEnds \
                         * ALIGN_UP((_wPopCnt), \
                                    sizeof(Bucket_t) / (_nBytesKeySz)) \
                     + ( ! cbAlignListEnds ) * (_wPopCnt) \
                     + POP_SLOT(_nBL) \
                     + sizeof(Word_t) / (_nBytesKeySz), \
                 2 * sizeof(Word_t) / (_nBytesKeySz)) \
         - sizeof(Word_t) / (_nBytesKeySz) ) \
)

// Pop count in wRoot high bits.
#if defined(POP_IN_WR_HB)

  #define PWR_xListPopCnt(_pwRoot, _nBL) \
      ((int)GetBits(*(_pwRoot), cnBitsListPopCnt, cnLsbListPopCnt))

  #define set_PWR_xListPopCnt(_pwRoot, _nBL, _cnt) \
      (assert((_cnt) <= (int)MSK(cnBitsListPopCnt)), \
          SetBits((_pwRoot), cnBitsListPopCnt, cnLsbListPopCnt, (_cnt)))

#else // defined(POP_IN_WR_HB)

  #define PWR_xListPopCnt(_pwRoot, _nBL) \
       ls_xPopCnt(wr_pwr(*(_pwRoot)), (_nBL))

  #define set_PWR_xListPopCnt(_pwRoot, _nBL, _cnt)

#endif // defined(POP_IN_WR_HB)

#if defined(OLD_LISTS)

// Use ls_sPopCnt in the performance path when we know the keys are bigger
// than one byte.
#define     ls_sPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_asKeys[0])
#define set_ls_sPopCnt(_ls, _cnt)  (ls_sPopCnt(_ls) = (_cnt))

// Use ls_cPopCnt in the performance path when we know the keys are one byte.
// PopCnt fits in a single key slot.
#define     ls_cPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_acKeys[0])
#define set_ls_cPopCnt(_ls, _cnt)  (ls_cPopCnt(_ls) = (_cnt))

#if defined(PP_IN_LINK)

// For PP_IN_LINK ls_xPopCnt macros are only valid at top, i.e.
// nDL == cnDigitsPerWord, and only for T_LIST - not for T_ONE.
#if (cnDummiesInList == 0)

#define     ls_xPopCnt(_ls, _nBL) \
    (assert((_nBL) == cnBitsPerWord), ((ListLeaf_t *)(_ls))->ll_awKeys[0])
#define set_ls_xPopCnt(_ls, _nBL, _cnt) \
    (assert((_nBL) == cnBitsPerWord), \
        ((ListLeaf_t *)(_ls))->ll_awKeys[0] = (_cnt))

#else // (cnDummiesInList == 0)

// Use the last dummy for pop count if we have at least one dummy.
#define     ls_xPopCnt(_ls, _nBL) \
    (assert((_nBL) == cnBitsPerWord), \
        ((ListLeaf_t *)(_ls))->ll_awDummies[cnDummiesInList - 1])
#define set_ls_xPopCnt(_ls, _nBL, _cnt) \
    (assert((_nBL) == cnBitsPerWord), \
        ((ListLeaf_t *)(_ls))->ll_awDummies[cnDummiesInList - 1] = (_cnt))

#endif // (cnDummiesInList == 0)

// Number of key slots needed for header info after cnDummiesInList
// (for nDL != cnDigitsPerWord).
#if defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  1
#else // defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  0
#endif // defined(LIST_END_MARKERS)

#else // defined(PP_IN_LINK)

#define     ls_xPopCnt(_ls, _nBL) \
  (((_nBL) > 8) ? ls_sPopCnt(_ls) : ls_cPopCnt(_ls))

#define set_ls_xPopCnt(_ls, _nBL, _cnt) \
  (((_nBL) > 8) ? set_ls_sPopCnt((_ls), (_cnt)) \
                : set_ls_cPopCnt((_ls), (_cnt)))

// Index of first key within leaf (for all cases).
// Or is it the number of key slots needed for header info after
// cnDummiesInList?
#if defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  2
#else // defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  1
#endif // defined(LIST_END_MARKERS)

#endif // defined(PP_IN_LINK)

#define ls_pwKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_awKeys[N_LIST_HDR_KEYS])

// For PP_IN_LINK ls_pxKeysNAT macros are only valid not at top or for
// T_ONE - not T_LIST - at top.
#if ( defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL) ) \
    || ( defined(ALIGN_LISTS) && ! defined(PSPLIT_PARALLEL) )

  #define ls_pwKeysNAT(_ls) \
      ((Word_t *)ALIGN_UP((Word_t)ls_pwKeysNAT_UA(_ls), sizeof(Bucket_t)))

#else // defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)

  // Ignore ALIGN_LISTS for (nBL >= cnBitsPerWord) unless PSPLIT_SEARCH_WORD
  // && PSPLIT_PARALLEL in order to improve memory usage at the top.
  #define ls_pwKeysNAT(_ls)  ls_pwKeysNAT_UA(_ls)

#endif // defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)

#if defined(COMPRESSED_LISTS)

#define ls_pcKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_acKeys[N_LIST_HDR_KEYS])

#define ls_psKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_asKeys[N_LIST_HDR_KEYS])

  #if (cnBitsPerWord > 32)
#define ls_piKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_aiKeys[N_LIST_HDR_KEYS])
  #endif // (cnBitsPerWord > 32)

  #if defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)
// What if we want 128-byte alignment and one-word parallel search?
// Ifdefs don't allow it at the moment.

#define ls_pcKeysNAT(_ls) \
    ((uint8_t *)ALIGN_UP((Word_t)ls_pcKeysNAT_UA(_ls), sizeof(Bucket_t)))

#define ls_psKeysNAT(_ls) \
    ((uint16_t *)ALIGN_UP((Word_t)ls_psKeysNAT_UA(_ls), sizeof(Bucket_t)))

      #if (cnBitsPerWord > 32)
#define ls_piKeysNAT(_ls) \
    ((uint32_t *)ALIGN_UP((Word_t)ls_piKeysNAT_UA(_ls), sizeof(Bucket_t)))
      #endif // (cnBitsPerWord > 32)

  #else // defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)

#define ls_pcKeysNAT(_ls)  ls_pcKeysNAT_UA(_ls)

#define ls_psKeysNAT(_ls)  ls_psKeysNAT_UA(_ls)

          #if (cnBitsPerWord > 32)
#define ls_piKeysNAT(_ls)  ls_piKeysNAT_UA(_ls)
          #endif // (cnBitsPerWord > 32)

  #endif // defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)
#endif // defined(COMPRESSED_LISTS)

// ls_pxKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
// ls_pcKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
// ls_psKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
// ls_piKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
// ls_pwKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
#if defined(PP_IN_LINK)

  #if defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)
      #if defined(COMPRESSED_LISTS)

#define ls_pcKeys(_ls, _nBL) \
    ((uint8_t *)ALIGN_UP((Word_t)(ls_pcKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                         sizeof(Bucket_t)))

#define ls_psKeys(_ls, _nBL) \
    ((uint16_t *)ALIGN_UP((Word_t)(ls_psKeysNAT_UA(_ls) + POP_SLOT(_nBL)),  \
                          sizeof(Bucket_t)))

          #if (cnBitsPerWord > 32)

#define ls_piKeys(_ls, _nBL) \
    ((uint32_t *)ALIGN_UP((Word_t)(ls_piKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                          sizeof(Bucket_t)))

          #endif // (cnBitsPerWord > 32)

      #endif // defined(COMPRESSED_LISTS)

      #if (defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)) \
          || ( defined(ALIGN_LISTS) && ! defined(PSPLIT_PARALLEL) )

#define ls_pwKeys(_ls, _nBL) \
    ((Word_t *)ALIGN_UP((Word_t)(ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                        sizeof(Bucket_t)))

      #else // (defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)) ...

#define ls_pwKeys(_ls, _nBL)  (ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL))

      #endif // (defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)) ...

  #else // defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)

#define ls_pwKeys(_ls, _nBL)  (ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL))

      #if defined(COMPRESSED_LISTS)

#define ls_pcKeys(_ls, _nBL)  (ls_pcKeysNAT_UA(_ls) + POP_SLOT(_nBL))

#define ls_psKeys(_ls, _nBL)  (ls_psKeysNAT_UA(_ls) + POP_SLOT(_nBL))

          #if (cnBitsPerWord > 32)

#define ls_piKeys(_ls, _nBL)  (ls_piKeysNAT_UA(_ls) + POP_SLOT(_nBL))

          #endif // (cnBitsPerWord > 32)

      #endif // defined(COMPRESSED_LISTS)
  #endif // defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)
#else // defined(PP_IN_LINK)

#define ls_pwKeys(_ls, _nBL)  ls_pwKeysNAT(_ls)
#define ls_piKeys(_ls, _nBL)  ls_piKeysNAT(_ls)
#define ls_psKeys(_ls, _nBL)  ls_psKeysNAT(_ls)
#define ls_pcKeys(_ls, _nBL)  ls_pcKeysNAT(_ls)

#endif // defined(PP_IN_LINK)

#define ls_pcKeysNATX(_pwr, _nPopCnt)  ls_pcKeysNAT(_pwr)
#define ls_psKeysNATX(_pwr, _nPopCnt)  ls_psKeysNAT(_pwr)
#define ls_piKeysNATX(_pwr, _nPopCnt)  ls_piKeysNAT(_pwr)
#define ls_pwKeysNATX(_pwr, _nPopCnt)  ls_pwKeysNAT(_pwr)

#else  // defined(OLD_LISTS)

// pwr aka ls points to the highest malloc-aligned address in the
// list buffer.  We have to use an aligned address because we use the low
// bits of the pointer as a type field.
// Pop count is in last pop-size slot in the word pointed to by pwr.
// Other code assumes pop count is not bigger than a single key in the list.
static inline int
ls_xPopCnt(void *pwr, int nBL)
{
    (void)nBL;
    int nPopCnt = (nBL > 8)
        ? (uint8_t)(((uint16_t *)((Word_t *)pwr + 1))[-1])
        :          (((uint8_t  *)((Word_t *)pwr + 1))[-1]);
    //printf("    ls_xPopCnt pwr %p nBL %2d nPopCnt %3d\n", pwr, nBL, nPopCnt);
    return nPopCnt;
    //return ((uint8_t *)((Word_t *)pwr + 1))[-1];
}

static inline void
set_ls_xPopCnt(void *pwr, int nBL, int nPopCnt)
{
    (void)nBL;
    //printf("set_ls_xPopCnt pwr %p nBL %2d nPopCnt %3d\n", pwr, nBL, nPopCnt);
    if (nBL > 8) {
        ((uint16_t *)((Word_t *)pwr + 1))[-1] = nPopCnt;
    } else {
        ((uint8_t  *)((Word_t *)pwr + 1))[-1] = nPopCnt;
    }
}

#define ls_pcKeys(_pwr, _nBL) \
    ((uint8_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList(ls_xPopCnt((_pwr), (_nBL)), (_nBL), sizeof(uint8_t)))

#define ls_pcKeysX(_pwr, _nBL, _nPopCnt) \
    ((uint8_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList((_nPopCnt), (_nBL), sizeof(uint8_t)))

#define ls_psKeys(_pwr, _nBL) \
    ((uint16_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList(ls_xPopCnt((_pwr), \
                          (_nBL)), (_nBL), sizeof(uint16_t)))

#define ls_psKeysX(_pwr, _nBL, _nPopCnt) \
    ((uint16_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList((_nPopCnt), (_nBL), sizeof(uint16_t)))

#define ls_piKeys(_pwr, _nBL) \
    ((uint32_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList(ls_xPopCnt((_pwr), (_nBL)), \
                          (_nBL), sizeof(uint32_t)))

#define ls_piKeysX(_pwr, _nBL, _nPopCnt) \
    ((uint32_t *)((Word_t *)(_pwr) + 1) \
        - ls_nSlotsInList((_nPopCnt), (_nBL), sizeof(uint32_t)))

#define ls_pwKeys(_pwr, _nBL) \
    (assert((_nBL) > (int)sizeof(Word_t)/2), \
        (Word_t *)((Word_t *)(_pwr) + 1) \
            - ls_nSlotsInList(ls_xPopCnt((_pwr), \
                              (_nBL)), (_nBL), sizeof(Word_t)))

#define ls_pwKeysX(_pwr, _nBL, _nPopCnt) \
    (assert((_nBL) > (int)sizeof(Word_t)/2), \
        (Word_t *)((Word_t *)(_pwr) + 1) \
            - ls_nSlotsInList((_nPopCnt), (_nBL), sizeof(Word_t)))

#define ls_pcKeysNAT(_pwr)  (ls_pcKeys((_pwr), 8))
#define ls_pcKeysNATX(_pwr, _nPopCnt)  (ls_pcKeysX((_pwr), 8, (_nPopCnt)))
#define ls_psKeysNAT(_pwr)  (ls_psKeys((_pwr), 16))
#define ls_psKeysNATX(_pwr, _nPopCnt)  (ls_psKeysX((_pwr), 16, (_nPopCnt)))
#define ls_piKeysNAT(_pwr)  (ls_piKeys((_pwr), 32))
#define ls_piKeysNATX(_pwr, _nPopCnt)  (ls_piKeysX((_pwr), 32, (_nPopCnt)))
#define ls_pwKeysNAT(_pwr)  (ls_pwKeys((_pwr), cnBitsPerWord-1))
#define ls_pwKeysNATX(_pwr, _nPopCnt) \
    (ls_pwKeysX((_pwr), cnBitsPerWord-1, (_nPopCnt)))

#endif // defined(OLD_LISTS)

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
        uint16_t ll_asKeys[1];
#endif // defined(COMPRESSED_LISTS) || ! defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
        uint8_t  ll_acKeys[1];
  #if (cnBitsPerWord > 32)
        uint32_t ll_aiKeys[1];
  #endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        Word_t   ll_awKeys[1];
    };
} ListLeaf_t;

#define N_WORDS_SWITCH_BM  DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord)

// Default is -UPOP_WORD_IN_LINK.
// It doesn't matter unless POP_WORD is defined.
// POP_WORD is defined automatically if DEPTH_IN_SW is defined.
// I wonder if PP_IN_LINK should cause POP_WORD_IN_LINK to be defined.

//
// When trying to make ! PP_IN_LINK use the same amount of memory as
// PP_IN_LINK:
//   -UPP_IN_LINK -DcnDummiesInLink=1 -DNO_SKIP_AT_TOP
//   -DPP_IN_LINK -DDUMMY_POP_CNT_IN_LIST
//
typedef struct {
#if defined(PP_IN_LINK)
    Word_t ln_wPrefixPop;
#endif // defined(PP_IN_LINK)
#if defined(POP_WORD) && defined(POP_WORD_IN_LINK)
    Word_t sw_wPopWord;
#endif // defined(POP_WORD) && defined(POP_WORD_IN_LINK)
#if defined(BM_IN_LINK)
    Word_t ln_awBm[N_WORDS_SWITCH_BM];
#endif // defined(BM_IN_LINK)
#if (cnDummiesInLink != 0)
    Word_t ln_awDummies[cnDummiesInLink];
#endif // (cnDummiesInLink != 0)
    Word_t ln_wRoot;
} Link_t;

#if defined(BM_IN_LINK)
    #define SW_BM
#else // defined(BM_IN_LINK)
    #define SW_BM  Word_t sw_awBm[N_WORDS_SWITCH_BM];
#endif // defined(BM_IN_LINK)

#if defined(PP_IN_LINK)
    #define SW_PREFIX_POP
#else // defined(PP_IN_LINK)
    #define SW_PREFIX_POP  Word_t sw_wPrefixPop;
#endif // defined(PP_IN_LINK)

#if defined(POP_WORD) && ! defined(POP_WORD_IN_LINK)
    #define SW_POP_WORD  Word_t sw_wPopWord;
#else // defined(POP_WORD) && ! defined(POP_WORD_IN_LINK)
    #define SW_POP_WORD
#endif // defined(POP_WORD) && ! defined(POP_WORD_IN_LINK)

#if (cnDummiesInSwitch != 0)
    #define SW_DUMMIES  Word_t sw_awDummies[cnDummiesInSwitch];
#else // (cnDummiesInSwitch != 0)
    #define SW_DUMMIES
#endif // (cnDummiesInSwitch != 0)

#define SWITCH_COMMON \
    SW_PREFIX_POP \
    SW_POP_WORD \
    SW_DUMMIES

// Uncompressed, basic switch.
typedef struct {
    SWITCH_COMMON
#if defined(USE_BM_SW) && defined(BM_IN_NON_BM_SW)
    SW_BM
#endif // defined(USE_BM_SW) && defined(BM_IN_NON_BM_SW)
    Link_t sw_aLinks[1]; // variable size
} Switch_t;

// Bitmap switch.
typedef struct {
    SWITCH_COMMON
    // sw_awBm must be first and the remainder of BmSwitch_t must be the same
    // as all of Switch_t for RETYPE_FULL_BM_SW without BM_IN_NON_BM_SW.
    // But this doesn't make it easy for us to handle SKIP_TO_BM_SW which
    // requires at least that sw_wPrefixPop have the same offset in both
    // BmSwitch_t and Switch_t.  SKIP_TO_BM_SW wins for the moment since
    // RETYPE_FULL_BM_SW isn't very important.
    SW_BM
    Link_t sw_aLinks[1]; // variable size
} BmSwitch_t;

Status_t Insert(Word_t *pwRoot, Word_t wKey, int nBL);
Status_t Remove(Word_t *pwRoot, Word_t wKey, int nBL);

Status_t InsertGuts(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot
                    , int nPos
#if defined(CODE_XX_SW)
                    , Word_t *pwRootPrev
  #if defined(SKIP_TO_XX_SW)
                    , int nBLPrev
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)
                    );

int PsplitSearchByKey16(uint16_t *psKeys, int nPopCnt, uint16_t sKey, int nPos);

Status_t RemoveGuts(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot);

void InsertCleanup(Word_t wKey, int nBL, Word_t *pwRoot, Word_t wRoot);

void RemoveCleanup(Word_t wKey, int nBL, int nBLR,
                   Word_t *pwRoot, Word_t wRoot);

Status_t InsertAtBitmap(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot);

Word_t FreeArrayGuts(Word_t *pwRoot,
                     Word_t wPrefix, int nBL, int bDump);

#if defined(DEBUG)
extern int bHitDebugThreshold;
#endif // defined(DEBUG)

int ListWords(int nPopCnt, int nBL);
Word_t *NewList(int nPopCnt, int nBL);
int OldList(Word_t *pwList, int nPopCnt, int nBL, int nType);

#if defined(DEBUG)
void Dump(Word_t *pwRoot, Word_t wPrefix, int nBL);
#endif // defined(DEBUG)

#else // (cnDigitsPerWord != 1)
Word_t wPopCntTotal;
#endif // (cnDigitsPerWord != 1)

#if defined(DEBUG)
Word_t *pwRootLast; // allow dumping of tree when root is not known
#endif // defined(DEBUG)
#if (cnDigitsPerWord != 1)
Word_t wPopCntTotal;
#endif // (cnDigitsPerWord != 1)

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

void HexDump(char *str, Word_t *pw, unsigned nWords);

#if defined(BPD_TABLE)
  #if defined(BPD_TABLE_RUNTIME_INIT)
extern unsigned anDL_to_nBitsIndexSz[ cnBitsPerWord + 1 ];
extern unsigned anDL_to_nBL[ cnBitsPerWord + 1 ];
extern unsigned anBL_to_nDL[ cnBitsPerWord * 2 ];
  #else // defined(BPD_TABLE_RUNTIME_INIT)
extern const unsigned anDL_to_nBitsIndexSz[];
extern const unsigned anDL_to_nBL[];
extern const unsigned anBL_to_nDL[];
  #endif // defined(BPD_TABLE_RUNTIME_INIT)
#endif // defined(BPD_TABLE)

#endif // ( ! defined(_B_H_INCLUDED) )

