
// @(#) $Id: b.h,v 1.400 2016/07/15 22:57:11 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/b.h,v $

#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

// Default is cnBitsPerWord = 64.
#if !defined(cnBitsPerWord)
#if defined(__LP64__) || defined(_WIN64)
#define cnBitsPerWord  64
#else // defined(__LP64__) || defined(_WIN64)
#define cnBitsPerWord  32
#endif // defined(__LP64__) || defined(_WIN64)
#endif // !defined(cnBitsPerWord)

#if ! defined(likely)
#define   likely(_b) (__builtin_expect((_b), 1))
#define unlikely(_b) (__builtin_expect((_b), 0))
#endif // ! defined(likely)

// NO_SKIP_LINKS means no skip links of any kind.
// SKIP_LINKS allows the type-specific SKIP_TO_<BLAH> to be defined.
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

// Default cn2dBmWpkPercent; create 2-digit bm at cn2dBmWpkPercent  wpk.
#if ! defined(cn2dBmWpkPercent)
  #if (cnBitsPerWord == 32)
#define cn2dBmWpkPercent  60
  #else // (cnBitsPerWord == 32)
#define cn2dBmWpkPercent  80
  #endif // (cnBitsPerWord == 32)
#endif // ! defined(cn2dBmWpkPercent)

#if defined(USE_BM_SW)
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

// Default is USE_XX_SW unless 32-bit.
#if ! defined(NO_USE_XX_SW) && (cnBitsPerWord > 32)
  #undef USE_XX_SW
  #define USE_XX_SW
#endif // ! defined(NO_USE_XX_SW) && (cnBitsPerWord > 32)

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

// Notes on cnListPopCntMax<blah>:
// If nBL == cnListPopCntMaxDl<x>, then cnListPopCntMaxDl<x> governs maximum
// external list size, where nBL == cnBitsLeftAtDl<x>.
// If nBL != cnBitsLeftAtDl<x> for any <x>, then maximum
// external list size is governed by cnListPopCntMax8, cnListPopCntMax16,
// cnListPopCntMax32 or cnListPopCntMax64.
// EmbeddedListPopCntMax(nBL) governs maximum embedded list size unless
// defined(POP_CNT_MAX_IS_KING) in which case the rules above for maximum
// external list size also govern the maximum embedded list size.

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
#if ! defined(NO_PARALLEL_128) && ! defined(PARALLEL_64)
#undef PARALLEL_128
#define PARALLEL_128
#endif // ! defined(NO_PARALLEL_128) && ! defined(PARALLEL_64)

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

// Default is -DNDEBUG -UDEBUG_ALL -UDEBUG
// -UDEBUG_INSERT -UDEBUG_REMOVE -UDEBUG_LOOKUP -UDEBUG_MALLOC
// -UDEBUG_COUNT -UDEBUG_NEXT
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
    #define  DEBUG_NEXT

#else // defined(DEBUG_ALL)

  #if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) \
          || defined(DEBUG_REMOVE) || defined(DEBUG_MALLOC) \
          || defined(DEBUG_COUNT) || defined(DEBUG_NEXT)

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

// Count leading zeros.
// __builtin_clzll is undefined for zero which allows the compiler to use bsr.
// Actual x86 clz instruction is defined for zero.
// This LOG macro is undefined for zero.
#define LOG(_x)  ((Word_t)63 - __builtin_clzll(_x))

#define cnLogBitsPerByte  3
#define cnBitsPerByte  (EXP(cnLogBitsPerByte))

#if (cnBitsPerWord == 64)
#define cnLogBytesPerWord  3
#else // (cnBitsPerWord == 64)
#define cnLogBytesPerWord  2
#endif // (cnBitsPerWord == 64)

#define cnLogBitsPerWord  (cnLogBytesPerWord + cnLogBitsPerByte)

// dlmalloc.c uses MALLOC_ALIGNMENT.
// Default MALLOC_ALIGNMENT is 2 * sizeof(void *).
// We have to set cnBitsMallocMask to be consitent with MALLOC_ALIGNMENT.
#if defined(MALLOC_ALIGNMENT)
  #if (MALLOC_ALIGNMENT == 4)
    #define cnBitsMallocMask 2
  #elif (MALLOC_ALIGNMENT == 8)
    #define cnBitsMallocMask 3
  #elif (MALLOC_ALIGNMENT == 16)
    #define cnBitsMallocMask 4
  #elif (MALLOC_ALIGNMENT == 32)
    #define cnBitsMallocMask 5
  #else // MALLOC_ALIGNMENT
    #error Unsupported MALLOC_ALIGNMENT
  #endif // MALLOC_ALIGNMENT
#else // defined(MALLOC_ALIGNMENT)
    #define cnBitsMallocMask (cnLogBytesPerWord + 1)
#endif // defined(MALLOC_ALIGNMENT)

#define cnMallocMask  MSK(cnBitsMallocMask)
#if (cnBitsPerWord == 64)
#define cnBitsVirtAddr  48
#define cwVirtAddrMask  MSK(cnBitsVirtAddr)
#else // (cnBitsPerWord == 64)
#if cnBitsPerWord != 32
#error Invalid cnBitsPerWord
#endif // cnBitsPerWord != 32
#define cnBitsVirtAddr  32
#define cwVirtAddrMask  ((Word_t)-1)
#endif // (cnBitsPerWord == 64)

#define EXP(_x)  (assert((_x) <= cnBitsPerWord), (Word_t)1 << (_x))
#define MSK(_x)  (EXP(_x) - 1)

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

// We'd like to ignore ALIGN_LISTS for lists of word-size keys
// if PSPLIT_SEARCH_WORD is not defined and the only reason
// ALIGN_LISTS is defined is that PSPLIT_PARALLEL is defined.
// We use _ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL to
// indicate that ALIGN_LISTS was defined independent of
// PSPLIT_PARALLEL and should not be so ignored.
#if defined(ALIGN_LISTS)
#define _ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL
#endif // defined(ALIGN_LISTS)

// Define ALIGN_LISTS if PSPLIT_PARALLEL is defined.
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
#elif defined(PARALLEL_64) // defined(PARALLEL_128)
typedef uint64_t Bucket_t;
#define cnLogBytesPerBucket  3
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

// Default is -DLVL_IN_WR_HB unless -DLVL_IN_SW or -DNO_LEVEL_IN_WR_HB
// or cnBitsPerWord == 32.
#if ! defined(LVL_IN_SW) && ! defined(NO_LVL_IN_WR_HB) && (cnBitsPerWord > 32)
  #undef LVL_IN_WR_HB
  #define LVL_IN_WR_HB
#endif // ! defined(LVL_IN_SW) && ! defined(NO_LVL_IN_WR_HB) && ...

#if defined(CODE_XX_SW)
// Default is -DSKIP_TO_XX_SW.
#if ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#undef SKIP_TO_XX_SW
#define SKIP_TO_XX_SW
#endif // ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#endif // defined(CODE_XX_SW)

// Default is SKIP_TO_BITMAP if LVL_IN_WR_HB
#if ! defined(NO_SKIP_TO_BITMAP) && defined(SKIP_LINKS)
  #if defined(LVL_IN_WR_HB)
    #undef SKIP_TO_BITMAP
    #define SKIP_TO_BITMAP
  #endif // defined(LVL_IN_WR_HB)
#endif // ! defined(NO_SKIP_TO_BITMAP) && defined(SKIP_LINKS)

// Default is SKIP_TO_BM_SW if USE_BM_SW and (LVL_IN_SW or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)
  #if defined(USE_BM_SW)
      #if defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
          #undef SKIP_TO_BM_SW
          #define SKIP_TO_BM_SW
      #endif // defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_BM_SW)
#endif // ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)

// Default is -DOLD_LISTS.
#if ! defined(NO_OLD_LISTS)
  #undef  OLD_LISTS
  #define OLD_LISTS
#endif // ! defined(NO_OLD_LISTS)

// Default is -DPOP_IN_WR_HB. List pop count in wRoot high bits.
#if ! defined(NO_POP_IN_WR_HB) && (cnBitsPerWord != 32)
  #undef  POP_IN_WR_HB
  #define POP_IN_WR_HB
#endif // ! defined(NO_POP_IN_WR_HB) && (cnBitsPerWord != 32)

#if ! defined(POP_IN_WR_HB)
  #undef LIST_POP_IN_PREAMBLE
  #define LIST_POP_IN_PREAMBLE
#endif // ! defined(POP_IN_WR_HB)

#if defined(POP_IN_WR_HB) && ! defined(OLD_LISTS)
  #error Must have OLD_LISTS with POP_IN_WR_HB.
#endif // defined(POP_IN_WR_HB)

// Choose max list lengths.
// Mind sizeof(ll_nPopCnt) and the maximum value it implies.

#if (cnBitsPerWord >= 64)
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
#endif // (cnBitsPerWord >= 64)

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
        #if (cnBitsPerWord == 64)
      #define cnListPopCntMaxDl1  0x07
        #else // (cnBitsPerWord == 64)
      #define cnListPopCntMaxDl1  0x03
        #endif // (cnBitsPerWord == 64)
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
  #if (cnBitsPerWord >= 64)
#define cwListPopCntMax \
   MAX(cnListPopCntMax64, \
       MAX(cnListPopCntMax32, \
           MAX(cnListPopCntMax16, \
               MAX(cnListPopCntMax8, 0))))
  #else // (cnBitsPerWord >= 64)
#define cwListPopCntMax \
       MAX(cnListPopCntMax32, \
           MAX(cnListPopCntMax16, \
               MAX(cnListPopCntMax8, 0)))
  #endif // (cnBitsPerWord >= 64)
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

// Default is -DEMBED_KEYS.
#if ! defined(NO_EMBED_KEYS)
#undef  EMBED_KEYS
#define EMBED_KEYS
#endif // ! defined(NO_EMBED_KEYS)

#if defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)
  #error Sorry, no SKIP_TO_BM_SW without USE_BM_SW.
#endif // defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)

#if defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)
  #if ! defined(LVL_IN_SW) && ! defined(LVL_IN_WR_HB)
      #error Sorry, no SKIP_TO_BM_SW without LVL_IN_SW or NO_LVL_IN_WR_HB.
  #endif // ! defined(LVL_IN_SW) && ! defined(LVL_IN_WR_HB)
#endif // defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)

// Values for nType.
enum {
    // Put T_NULL and T_LIST at beginning of enum so one of them gets
    // type == 0 if either exists. For no reason other than a dump with
    // a NULL will have a somewhat intuitive meaning.
#if defined(SEPARATE_T_NULL)
    T_NULL, // no keys below
#endif // defined(SEPARATE_T_NULL)
#if (cwListPopCntMax != 0)
    T_LIST, // external list of keys
  #if defined(SKIP_TO_LIST)
    T_SKIP_TO_LIST, // skip to external list of keys
  #endif // defined(SKIP_TO_LIST)
#endif // (cwListPopCntMax != 0)
    // T_BITMAP may not be needed if it is implied by the level/depth.
    T_BITMAP, // external (not embedded) bitmap leaf
#if defined(SKIP_TO_BITMAP)
    T_SKIP_TO_BITMAP, // skip to external bitmap leaf
#endif // defined(SKIP_TO_BITMAP)
#if defined(EMBED_KEYS)
    T_EMBEDDED_KEYS, // keys are embedded in the link
#endif // defined(EMBED_KEYS)
#if defined(CODE_BM_SW)
    T_BM_SW,
#endif // defined(CODE_BM_SW)
#if defined(SKIP_TO_BM_SW)
    T_SKIP_TO_BM_SW,
#endif // defined(SKIP_TO_BM_SW)
#if defined(CODE_XX_SW)
    T_XX_SW,
#endif // defined(CODE_XX_SW)
#if defined(SKIP_TO_XX_SW) // doesn't work yet
    T_SKIP_TO_XX_SW,
#endif // defined(SKIP_TO_XX_SW) // doesn't work yet
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    // All link bits set, i.e. all links present.
    T_FULL_BM_SW,
  #if defined(SKIP_TO_BM_SW)
    T_SKIP_TO_FULL_BM_SW,
  #endif // defined(SKIP_TO_BM_SW)
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    T_SWITCH, // Uncompressed, close (i.e. no-skip) switch.
#if defined(SKIP_LINKS)
    // T_SKIP_TO_SWITCH has to have the biggest value in this enum
    // if not LVL_IN_WR_HB and not LVL_IN_SW.  All of the bigger
    // values have a meaning relative to T_SKIP_TO_SWITCH.
    // Depth/level is determined by (nType - T_SKIP_TO_SWITCH).
    T_SKIP_TO_SWITCH
#endif // defined(SKIP_LINKS)
};

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
        ( ((_nBL) <= cnBitsInD1) ? 1 \
        : 1 + DIV_UP((_nBL) - cnBitsLeftAtDl1, cnBitsPerDigit) )
  #elif (cnBitsInD3 == cnBitsPerDigit)
    // Rounding up is free since we already have to add a constant before
    // (or after) dividing.
    #define nDL_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsLeftAtDl1) ? 1 \
        : ((_nBL) <= cnBitsLeftAtDl2) ? 2 \
        : 2 + DIV_UP((_nBL) - cnBitsLeftAtDl2, cnBitsPerDigit) )
  #else // (cnBitsInD2 == cnBitsPerDigit) && ...
    // Rounding up is free since we already have to add a constant before
    // (or after) dividing.
    #define nDL_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsLeftAtDl1) ? 1 \
        : ((_nBL) <= cnBitsLeftAtDl2) ? 2 \
        : ((_nBL) <= cnBitsLeftAtDl3) ? 3 \
        : 3 + DIV_UP((_nBL) - cnBitsLeftAtDl3, cnBitsPerDigit) )
  #endif // (cnBitsInD2 == cnBitsPerDigit) ...

    #define nDL_from_nBL_NIB(_nBL)  nDL_from_nBL(_nBL)

#endif // (cnBitsInD1 == cnBitsPerDigit) && ...

#define nBitsIndexSz_from_nBL_NAB(_nBL) \
    ( (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) != 0) \
        && ((_nBL) == cnBitsPerWord) ? cnBitsIndexSzAtTop \
    : ((cnBitsInD2 != cnBitsPerDigit) && ((_nBL) <= 2)) ? cnBitsInD2 \
    : ((cnBitsInD3 != cnBitsPerDigit) && ((_nBL) <= 3)) ? cnBitsInD3 \
    : cnBitsPerDigit )

#if (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)
// cnBitsIndexSzAtTop == cnBitsPerDigit

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
// cnBitsIndexSzAtTop != cnBitsPerDigit

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
#define nBL_to_nBitsIndexSzNAB(_nBL)  (nBitsIndexSz_from_nBL_NAB(_nBL))
#define nDL_to_nBitsIndexSzNAX(_nDL)  (nBitsIndexSz_from_nDL_NAX(_nDL))
#define nDL_to_nBL_NAT(_nDL)          (nBL_from_nDL_NAT(_nDL))
#define nDL_to_nBitsIndexSzNAT(_nDL)  (nDL_to_nBitsIndexSz(_nDL))

#if defined(RAMMETRICS)
  #define METRICS(x)  (x)
#else // defined(RAMMETRICS)
  #define METRICS(x)
#endif // defined(RAMMETRICS)

#if defined(SEARCHMETRICS)
  #define SMETRICS(x)  (x)
#else // defined(SEARCHMETRICS)
  #define SMETRICS(x)
#endif // defined(SEARCHMETRICS)

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

#if defined(DEBUG_COUNT)
  #if (cwDebugThreshold != 0)
    #define DBGC(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGC(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_COUNT)
  #define DBGC(x)
#endif // defined(DEBUG_COUNT)

#if defined(DEBUG_NEXT)
  #if (cwDebugThreshold != 0)
    #define DBGN(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGN(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_NEXT)
  #define DBGN(x)
#endif // defined(DEBUG_NEXT)

#if defined(DEBUG_MALLOC)
#if (cwDebugThreshold != 0)
#define DBGM(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGM(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_MALLOC)
#define DBGM(x)
#endif // defined(DEBUG_MALLOC)

// Shorthand
#if !defined(Owx)
#if defined(__LP64__) || defined(_WIN64)
#define Owx   "%016" PRIxPTR
#define OWx   "0x%016" PRIxPTR
#else // defined(__LP64__) || defined(_WIN64)
#define Owx   "%08" PRIxPTR
#define OWx   "0x%08" PRIxPTR
#endif // defined(__LP64__) || defined(_WIN64)
#if defined(__PRIPTR_PREFIX)
#define _fw  __PRIPTR_PREFIX // _fw -- format word
#else // defined(__PRIPTR_PREFIX)
#define _fw  "l" // _fw -- format word
#endif // defined(__PRIPTR_PREFIX)
#endif // !defined(Owx)


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
    (void)pwRoot;
#if defined(SKIP_LINKS)
    int nType = wr_nType(*pwRoot);
  #if ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
    if (nType >= T_SKIP_TO_SWITCH) { Set_nType(pwRoot, T_SWITCH); return; }
  #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
    switch (nType) {
  #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
    case T_SKIP_TO_SWITCH: Set_nType(pwRoot, T_SWITCH); break;
  #endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: Set_nType(pwRoot, T_BM_SW); break;
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: Set_nType(pwRoot, T_XX_SW); break;
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: Set_nType(pwRoot, T_BITMAP); break;
  #endif // defined(SKIP_TO_BITMAP)
    default:
        DBG(printf("\nnType %d\n", nType));
        assert(0);
    }
#endif // defined(SKIP_LINKS)
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

// Default is -DEK_CALC_POP.
// EK_CALC_POP means we don't use any bits in a wRoot/link/bucket to
// hold the pop count.  We calculate the pop count based on the contents
// of the key suffix slots.
#if ! defined(NO_EK_CALC_POP)
#undef EK_CALC_POP
#define EK_CALC_POP
#endif // ! defined(NO_EK_CALC_POP)

// Default is -UFILL_W_KEY.
// FILL_W_KEY means unused key suffix slots in a T_EMBEDDED_KEYS
// wRoot/link/bucket are filled with a copy of the smallest key suffix in
// the list. This is independent of the order in which the key suffixes are
// sorted.
// If FILL_W_KEY is not defined, then the empty slots are filled with zero
// or -1 depending on FILL_WITH_ONES.
#if defined(FILL_W_KEY) && defined(EK_CALC_POP)
#error Sorry, no FILL_W_KEY and EK_CALC_POP.
#endif // defined(FILL_W_KEY) && defined(EK_CALC_POP)

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
// could and we thought it might come in handy. It assumes T_EMBEDDED_KEYS
// is not zero.
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

#if defined(EMBED_KEYS)
  #if defined(EK_CALC_POP)

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

  #else // defined(EK_CALC_POP)

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

  #endif // defined(EK_CALC_POP)

static inline int
EmbeddedListPopCntMax(int nBL)
{
    int nBitsForKeys = cnBitsPerWord;
    nBitsForKeys -= nBL_to_nBitsType(nBL) + nBL_to_nBitsPopCntSz(nBL);
    return nBitsForKeys / nBL;
}

#endif // defined(EMBED_KEYS)

static inline int
tp_bIsSwitch(int nType)
{
#if ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
  #if defined(SKIP_LINKS)
    if (nType >= T_SKIP_TO_SWITCH) { return 1;}
  #endif // defined(SKIP_LINKS)
#endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
    switch (nType) {
    case T_SWITCH:
#if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
  #if defined(SKIP_LINKS)
    case T_SKIP_TO_SWITCH:
  #endif // defined(SKIP_LINKS)
#endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
#if defined(CODE_BM_SW)
    case T_BM_SW:
#endif // defined(CODE_BM_SW)
#if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW:
#endif // defined(SKIP_TO_BM_SW)
#if defined(CODE_XX_SW)
    case T_XX_SW:
#endif // defined(CODE_XX_SW)
#if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW:
#endif // defined(SKIP_TO_XX_SW)
        return 1;
    }
    return 0;
}

#define wr_bIsSwitch(_wr)  (tp_bIsSwitch(wr_nType(_wr)))

#if defined(CODE_BM_SW)
    // Is (_tp) a bitmap switch or skip to one?
  #if defined(SKIP_TO_BM_SW)
    #define tp_bIsBmSw(_tp)  (((_tp) == T_BM_SW) || ((_tp) == T_SKIP_TO_BM_SW))
  #else // defined(SKIP_TO_BM_SW)
    #define tp_bIsBmSw(_tp)  ((_tp) == T_BM_SW)
  #endif // defined(SKIP_TO_BM_SW)
#endif // defined(CODE_BM_SW)

#if defined(CODE_XX_SW)
    // Is (_tp) a doubling switch or skip to one?
  #if defined(SKIP_TO_XX_SW)
    #define tp_bIsXxSw(_tp)  (((_tp) == T_XX_SW) || ((_tp) == T_SKIP_TO_XX_SW))
  #else // defined(SKIP_TO_XX_SW)
    #define tp_bIsXxSw(_tp)  ((_tp) == T_XX_SW)
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)

static inline int
tp_bIsSkip(int nType)
{
    (void)nType;
#if defined(SKIP_LINKS)
  #if ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
    if (nType >= T_SKIP_TO_SWITCH) { return 1;}
  #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
    switch (nType) {
  #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
    case T_SKIP_TO_SWITCH:
  #endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW:
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW:
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP:
  #endif // defined(SKIP_TO_BITMAP)
        return 1;
    }
#endif // defined(SKIP_LINKS)
    return 0;
}

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
  #if (cnBitsPerWord >= 64)
    int nBW = GetBits(*pwRoot, cnBitsXxSwWidth, cnLsbXxSwWidth);
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
  #else // (cnBitsPerWord >= 64)
    (void)pwRoot;
    int nBW = cnBitsPerDigit / 2;
  #endif // (cnBitsPerWord >= 64)
    return nBW;
}

#define pwr_nBW(_pwRoot)  Get_nBW(_pwRoot)

// Set the width of the branch in bits.
static inline void
Set_nBW(Word_t *pwRoot, int nBW)
{
  #if (cnBitsPerWord >= 64)
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
    SetBits(pwRoot, cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
  #else // (cnBitsPerWord >= 64)
    (void)pwRoot; (void)nBW;
  #endif // (cnBitsPerWord >= 64)
}

#define set_pwr_nBW(_pwRoot, _nBW)  Set_nBW((_pwRoot), (_nBW))

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

// Get the level of the object in number of bits left to decode.
// This is valid only when *pwRoot is a skip link.
static inline int
Get_nBLR(Word_t *pwRoot)
{
    int nBLR;
    assert(tp_bIsSkip(wr_nType(*pwRoot)));
#if defined(LVL_IN_WR_HB)
    nBLR = GetBits(*pwRoot, cnBitsLvl, cnLsbLvl);
#else // defined(LVL_IN_WR_HB)
  #define tp_to_nDL(_tp) ((_tp) - T_SKIP_TO_SWITCH + 2)
    nBLR = nDL_to_nBL(tp_to_nDL(wr_nType(*pwRoot)));
#endif // defined(LVL_IN_WR_HB)


    return nBLR;
}

#if defined(LVL_IN_WR_HB)

// Set the level of the object in number of bits left to decode.
// Use this only when *pwRoot is a skip link.
static inline void
Set_nBLR(Word_t *pwRoot, int nBLR)
{
    assert(nBLR <= (int)MSK(cnBitsLvl));
    SetBits(pwRoot, cnBitsLvl, cnLsbLvl, nBLR);
}

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

#if defined(LVL_IN_SW)
// LVL_IN_SW directs us to use the low bits of sw_wPrefixPop for absolute
// depth instead of encoding it into the type field directly.
// It means we can't use the low bits of sw_wPrefixPop for pop.  So we
// define POP_WORD and use a separate word.
// We assume the value we put into the low bits will will fit in the number
// of bits used for the pop count at nDL == 2.  Or maybe
// it doesn't matter since we always create an embedded bitmap when
// EXP(nBL) <= sizeof(Link_t) * 8.
#define POP_WORD

// As it stands we always get the absolute type from sw_wPrefixPop if
// LVL_IN_SW.  We assume the macro is used only when it is known that
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

#else // defined(LVL_IN_SW)

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
// lookup table, ...  But why?  We're going to use LVL_IN_SW.  This code
// is an anachronism.
  #define tp_to_nDL(_tp)   ((_tp)  - T_SKIP_TO_SWITCH + 2)
  #define nDL_to_tp(_nDL)  ((_nDL) + T_SKIP_TO_SWITCH - 2)

  #define wr_nDL(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), tp_to_nDL(wr_nType(_wr)))

  #define wr_nBL(_wr)  nDL_to_nBL(tp_to_nDL(wr_nType(_wr)))

  #define set_wr_nDL(_wr, _nDL) \
      (assert(nDL_to_tp(_nDL) >= T_SKIP_TO_SWITCH), \
       set_wr_nType((_wr), nDL_to_tp(_nDL)))

  #define set_wr_nBL(_wr, _nBL) \
      set_wr_nDL((_wr), nBL_to_nDL(_nBL))

#endif // defined(LVL_IN_SW)

#endif // defined(LVL_IN_WR_HB)


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
#if defined(LVL_IN_SW) || defined(POP_WORD)
// Relocating the pop out of PP requires quite a few code changes.
// It would be nice for depth, prefix and pop to share the same word.
#error Sorry, no PP_IN_LINK && (LVL_IN_SW || POP_WORD).
#endif // defined(LVL_IN_SW) || defined(POP_WORD)
#else // defined(PP_IN_LINK)
// This cast assumes sw_wPrefixPop is the same for all types of switch.
// There should be assertions in Initialize validating the assumption.
#define PWR_wPrefixPop(_pwRoot, _pwr)  (((Switch_t *)(_pwr))->sw_wPrefixPop)
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

#if defined(PP_IN_LINK)
#define set_PWR_wPrefix(_pwRoot, _pwr, _nDL, _key) \
  (assert((_nDL) < cnDigitsPerWord), \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL)))))

#define set_PWR_wPrefixBL(_pwRoot, _pwr, _nBL, _key) \
  (assert((_nBL) < cnBitsPerWord), \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMaskBL(_nBL)) \
            | ((_key) & ~wPrefixPopMaskBL(_nBL)))))
#else // defined(PP_IN_LINK)
#define set_PWR_wPrefix(_pwRoot, _pwr, _nDL, _key) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMask(_nDL)) \
            | ((_key) & ~wPrefixPopMask(_nDL))))

#define set_PWR_wPrefixBL(_pwRoot, _pwr, _nBL, _key) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & wPrefixPopMaskBL(_nBL)) \
            | ((_key) & ~wPrefixPopMaskBL(_nBL))))
#endif // defined(PP_IN_LINK)

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

#if defined(PP_IN_LINK)
#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
  (assert((_nDL) < cnDigitsPerWord), \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL)))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
  (assert((_nBL) < cnBitsPerWord), \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL)))))
#else // defined(PP_IN_LINK)
#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL))))
#endif // defined(PP_IN_LINK)

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

#if defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)
#define cbAlignListsIOPP 1
#else // defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)
#define cbAlignListsIOPP 0
#endif // defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)

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

// POP_SLOT tells ListWords if we need a slot in the leaf for a pop count
// that is not included in N_LIST_HDR_KEYS, i.e. a slot that occurs after
// ll_a[csik]Keys[N_LIST_HDR_KEYS].
// There is a problem if POP_SLOT is at the beginning of the list and
// we're aligning lists as with PP_IN_LINK and a list at the top with
// PSPLIT_PARALLEL at top. Our code doesn't account for aligning the
// list again after the pop slot.
#if defined(PP_IN_LINK)
    // Be careful: _nBL here is BEFORE any skip is applied
    // Do we allow skip from top for PP_IN_LINK? Looks like we allow
    // skip if prefix is zero. It means we'd need a pop slot.
    #define POP_SLOT(_nBL) \
        (((_nBL) >= cnBitsPerWord) && (cnDummiesInList == 0))
#else // defined(PP_IN_LINK)
  #if defined(OLD_LISTS)
    // N_HDR_KEYS incorporates this for ! PP_IN_LINK so don't add it again.
    #define POP_SLOT(_nBL)  (0)
  #else // defined(OLD_LISTS)
    #define POP_SLOT(_nBL)  (1)
  #endif // defined(OLD_LISTS)
#endif // defined(PP_IN_LINK)

// align non-word iff align
// align word iff (psword && psp) || aligniopp
// (1)        align     if !word iff (                         align    )
// (2)        align     if  word iff ( ( psword and  psp) or   aligniopp)
// (1a)       align     if !word and (                         align    )
// (2a)       align     if  word and ( ( psword and  psp) or   aligniopp)
// (1b) don't align     if !word and (                        !align    )
// (2b) don't align     if  word and (!( psword and  psp) and !aligniopp)
// (3)  !align => !psp && !aligniopp
// (4)  from (2b) and (3): don't align if word and !align
// (5)  from (1b) and (4): don't align if !align
// (6)  don't align     if  word and ( (!psword or  !psp) and !aligniopp)
#define ALIGN_LIST(_nBytesKeySz) \
    ( cbAlignLists \
        && (((_nBytesKeySz) != sizeof(Word_t)) \
            || (cbPsplitSearchWord && cbPsplitParallel) \
            ||  cbAlignListsIOPP) )

// cbAlignListEnds is consulted only if ALIGN_LIST is true.
// We don't support cbAlignListEnds without cbAlignLists.
// We don't support ALIGN_LIST_ENDS without ALIGN_LISTS.
#define ALIGN_LIST_END(_nBytesKeySz) \
    ( assert(cbAlignLists || !cbAlignListEnds), \
      (ALIGN_LIST(_nBytesKeySz) && cbAlignListEnds) )

// We want an odd number of words (for malloc efficiency) that will
// hold our list (add one, align to two, then subtract one).
// 1 => 1, 1.1 => 3, 3 => 3
// If aligning, align end of dummies on bucket boundary,
// and allocate a whole bucket to each list-end-marker, if any,
// and if aligning end align end of keys on bucket boundary,
// else simply make room for the keys.
// Add a slot for POP_SLOT -- hmm -- if aligning end, pop-slot
// at end for new list will be after the last bucket containing
// a key or list-end-marker.
// Then do the same trick to align to an odd word boundary
// What if POP_SLOT is for PP_IN_LINK at top and goes at the
// beginning of the list? Wouldn't it need a whole bucket of
// it's own? It seems like we might have a problem with
// PP_IN_LINK and ALIGN_LISTS.
#define ls_nSlotsInList(_wPopCnt, _nBL, _nBytesKeySz) \
( \
    ( ! ALIGN_LIST(_nBytesKeySz) \
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
         - sizeof(Word_t) / (_nBytesKeySz) ) ) \
)

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
  // nDL == cnDigitsPerWord, and only for T_LIST.
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
      // (for nBL != cnBitsPerWord).
      #if defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  1
      #else // defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  0
      #endif // defined(LIST_END_MARKERS)

  #else // defined(PP_IN_LINK)

      #if ! defined(POP_IN_WR_HB)
    #define     ls_xPopCnt(_ls, _nBL) \
        (((_nBL) > 8) ? ls_sPopCnt(_ls) : ls_cPopCnt(_ls))
      #endif // ! defined(POP_IN_WR_HB)

      #if ! defined(POP_IN_WR_HB)
#define set_ls_xPopCnt(_ls, _nBL, _cnt) \
    (((_nBL) > 8) ? set_ls_sPopCnt((_ls), (_cnt)) \
                  : set_ls_cPopCnt((_ls), (_cnt)))
      #endif // ! defined(POP_IN_WR_HB)

      // Number of key slots needed for header info after cnDummiesInList.
      #if defined(LIST_END_MARKERS)
          #if defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  1
          #else // defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  2 // one slot needed for pop
          #endif // defined(POP_IN_WR_HB)
      #else // defined(LIST_END_MARKERS)
          #if defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  0
          #else // defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  1  // one slot needed for pop
          #endif // defined(POP_IN_WR_HB)
      #endif // defined(LIST_END_MARKERS)

  #endif // defined(PP_IN_LINK)

// NAT is relevant only for PP_IN_LINK where POP_SLOT depends on
// whether we are at the top or not.
#define ls_pwKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_awKeys[N_LIST_HDR_KEYS])

  // Align lists of word-size keys only if PSPLIT_SEARCH_WORD and
  // PSPLIT_PARALLEL are defined or ALIGN_LISTS was defined independent
  // of PSPLIT_PARALLEL. In other words, don't align lists of word-size
  // keys if ALIGN_LISTS is defined only because PSPLIT_PARALLEL is
  // defined unless PSPLIT_SEARCH_WORD is also defined.
  #if ( defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL) ) \
      || defined(_ALIGN_LISTS_INDEPENDENT_OF_PSPLIT_PARALLEL)

#define ls_pwKeysNAT(_ls) \
    ((Word_t *)ALIGN_UP((Word_t)ls_pwKeysNAT_UA(_ls), sizeof(Bucket_t)))

  #else // ( defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL) ) || ...

#define ls_pwKeysNAT(_ls)  ls_pwKeysNAT_UA(_ls)

  #endif // ( defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL) ) || ...

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

          #else // (defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL))..

#define ls_pwKeys(_ls, _nBL)  (ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL))

          #endif // (defined(PSPLIT_SEARCH_WORD) && defined(PSPLIT_PARALLEL)).

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

#else // defined(OLD_LISTS)

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
    ( /*printf("ls_xPopCnt %d\n", (int)ls_xPopCnt((_pwr), (_nBL))),*/ \
      /*printf("ls_nSlotsInList %d\n",*/ \
             /*(int)ls_nSlotsInList(ls_xPopCnt((_pwr), (_nBL)), (_nBL), sizeof(uint32_t))),*/ \
      ((uint32_t *)((Word_t *)(_pwr) + 1) \
          - ls_nSlotsInList(ls_xPopCnt((_pwr), (_nBL)), \
                            (_nBL), sizeof(uint32_t))))

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

#define ls_pwKey(_ls, _nBL, _ii)  (ls_pwKeys(_ls, _nBL)[_ii])
#define ls_piKey(_ls, _nBL, _ii)  (ls_piKeys(_ls, _nBL)[_ii])
#define ls_psKey(_ls, _nBL, _ii)  (ls_psKeys(_ls, _nBL)[_ii])
#define ls_pcKey(_ls, _nBL, _ii)  (ls_pcKeys(_ls, _nBL)[_ii])

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

#define N_WORDS_SWITCH_BM  DIV_UP(((Word_t)1 << cnBitsPerDigit), cnBitsPerWord)

// Default is -UPOP_WORD_IN_LINK.
// It doesn't matter unless POP_WORD is defined.
// POP_WORD is defined automatically if LVL_IN_SW is defined.
// I wonder if PP_IN_LINK should cause POP_WORD_IN_LINK to be defined.

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

#define cnBitsPreListPopCnt cnBitsListPopCnt
#define cnLsbPreListPopCnt (cnBitsPerWord - cnBitsListPopCnt)

static inline int
Get_xListPopCnt(Word_t *pwRoot, int nBL)
{
    (void)nBL;
#if defined(POP_IN_WR_HB) // 64-bit default
    int nPopCnt = GetBits(*pwRoot, cnBitsListPopCnt, cnLsbListPopCnt);
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    int nPopCnt = GetBits(wr_pwr(*pwRoot)[-1],
                          cnBitsPreListPopCnt, cnLsbPreListPopCnt);
#else // POP_IN_WR_HB ...
    int nPopCnt = ls_xPopCnt(wr_pwr(*pwRoot), nBL);
#endif // POP_IN_WR_HB ...
    return nPopCnt;
}

static inline void
Set_xListPopCnt(Word_t *pwRoot, int nBL, int nPopCnt)
{
    (void)nBL;
#if defined(POP_IN_WR_HB) // 64-bit default
    assert(nPopCnt <= (int)MSK(cnBitsListPopCnt));
    SetBits(pwRoot, cnBitsListPopCnt, cnLsbListPopCnt, nPopCnt);
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    assert(nPopCnt <= (int)MSK(cnBitsPreListPopCnt));
    Word_t *pwr = wr_pwr(*pwRoot);
    SetBits(&pwr[-1], cnBitsPreListPopCnt, cnLsbPreListPopCnt, nPopCnt);
#else // POP_IN_WR_HB ...
    set_ls_xPopCnt(wr_pwr(*pwRoot), nBL, nPopCnt);
#endif // POP_IN_WR_HB ...
}

#define     PWR_xListPopCnt(_pwRoot, _pwr, _nBL) \
    (assert(wr_pwr(*(_pwRoot)) == (_pwr)), Get_xListPopCnt((_pwRoot), (_nBL)))

#define set_PWR_xListPopCnt(_pwRoot, _pwr, _nBL, _cnt) \
    (assert(wr_pwr(*(_pwRoot)) == (_pwr)), \
    Set_xListPopCnt((_pwRoot), (_nBL), (_cnt)))

Status_t Insert(Word_t *pwRoot, Word_t wKey, int nBL);
Status_t Remove(Word_t *pwRoot, Word_t wKey, int nBL);
Word_t Count(Word_t *pwRoot, Word_t wKey, int nBL);
Status_t Next(Word_t *pwRoot, Word_t wKey, int nBL);

Status_t InsertGuts(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot
                    , int nPos
#if defined(CODE_XX_SW)
                    , Word_t *pwRootPrev
  #if defined(SKIP_TO_XX_SW)
                    , int nBLPrev
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)
                    );

Status_t RemoveGuts(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot);

void InsertCleanup(Word_t wKey, int nBL, Word_t *pwRoot, Word_t wRoot);

void RemoveCleanup(Word_t wKey, int nBL, int nBLR,
                   Word_t *pwRoot, Word_t wRoot);

Status_t InsertAtBitmap(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot);

Word_t FreeArrayGuts(Word_t *pwRoot,
                     Word_t wPrefix, int nBL, int bDump);

#if defined(EMBED_KEYS)
Word_t InflateEmbeddedList(Word_t *pwRoot,
                           Word_t wKey, int nBL, Word_t wRoot);
#endif // defined(EMBED_KEYS)

#if defined(DEBUG)
extern int bHitDebugThreshold;
#endif // defined(DEBUG)

int ListWords(int nPopCnt, int nBL);
Word_t *NewList(int nPopCnt, int nBL);
int OldList(Word_t *pwList, int nPopCnt, int nBL, int nType);

#if defined(DEBUG)
void Dump(Word_t *pwRoot, Word_t wPrefix, int nBL);
#endif // defined(DEBUG)

#endif // (cnDigitsPerWord != 1)

#if defined(DEBUG)
extern Word_t *pwRootLast; // allow dumping of tree when root is not known
#endif // defined(DEBUG)

extern Word_t wPopCntTotal;

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

#if (cnDigitsPerWord > 1)

#if defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Success)
  #if defined(LOOKUP)
#define strLookupOrInsertOrRemove  "Lookup"
#define DBGX  DBGL
  #else // defined(REMOVE)
#define strLookupOrInsertOrRemove  "Remove"
#define DBGX  DBGR
#define InsertRemove  Remove
      #if defined(RECURSIVE_REMOVE)
#define RECURSIVE
      #endif // defined(RECURSIVE_REMOVE)
  #endif // defined(REMOVE)
#else // defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Failure)
  #if defined(INSERT)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#define InsertRemove  Insert
      #if defined(RECURSIVE_INSERT)
#define RECURSIVE
      #endif // defined(RECURSIVE_INSERT)
  #elif defined(COUNT) // defined(INSERT)
// Count returns the number of keys present in the array up to but
// excluding the key that is passed as a parameter.
#define strLookupOrInsertOrRemove  "Count"
#define DBGX  DBGC
#define InsertRemove  Count
  #else // defined(INSERT) elif defined(COUNT)
#define strLookupOrInsertOrRemove  "Next"
#define DBGX  DBGN
#define InsertRemove  Next
  #endif // defined(INSERT)
#endif // defined(LOOKUP) || defined(REMOVE)

#if defined(PARALLEL_128)

// The assertion is here because the macro used to test the size of the
// bucket and use WordHasKey if not equal to the size of __m128i.
// I replaced the test with an assertion because I couldn't remember
// why we were doing the test.
#define BUCKET_HAS_KEY(_pxBucket, _wKey, _nBL) \
    (assert(sizeof(*(_pxBucket)) == sizeof(__m128i)), \
        HasKey128((__m128i *)(_pxBucket), (_wKey), (_nBL)))

#elif defined(PARALLEL_64) // defined(PARALLEL_128)

#define BUCKET_HAS_KEY(_pxBucket, _wKey, _nBL) \
    HasKey64((_pxBucket), (_wKey), (_nBL))

#else // defined(PARALLEL_128)

#define BUCKET_HAS_KEY(_pxBucket, _wKey, _nBL) \
    WordHasKey((_pxBucket), (_wKey), (_nBL))

#endif // defined(PARALLEL_128)

#if defined(TRY_MEMCHR)
#include <wchar.h>
#endif // defined(TRY_MEMCHR)

#if (cwListPopCntMax != 0)

// Simple forward linear search of a sub-list.
// It assumes the list contains a key that is greater than or equal to _xKey.
// _pxKeys is the beginning of the whole list.
// The search starts at _pxKeys[_nPos].
#define SSEARCHF(_pxKeys, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); } \
    if ((_pxKeys)[_nPos] > (_xKey)) { (_nPos) ^= -1; } \
}

// Simple backward linear search of a sub-list.
// It assumes the list contains a key that is less than or equal to _xKey.
// _pxKeys is the beginning of the whole list.
// The search starts at _pxKeys[_nPos].
#define SSEARCHB(_pxKeys, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] > (_xKey)) { --(_nPos); } \
    if ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); (_nPos) ^= -1; } \
}

#if defined(LIST_END_MARKERS)

  #if defined(TEST_PAST_END)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] >= &(_pxKeys)[_nPopCnt])

#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos) \
    (&(_pxKeys0)[_nPos] < (_pxKeys))

  #elif defined(TEST_SPLIT_EQ_KEY)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  ((_pxKeys)[nSplit] == (_xKey))

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #else // TEST_KEY_IS_MAX_MIN

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

// BUG: Refine this with nBL; this won't work for non-native sizes as it is.
#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey) \
    ((_xKey) == (_x_t)-1)

#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  ((_xKey) == 0)

#define PAST_ENDF(_pxKeys, _nPopCnt, _pxKeys0, _nPos)  0
#define PAST_ENDB(_pxKeys, _pxKeys0, _nPos)  0

  #endif // ...

// Simple forward linear search of a sub-list.
// It assumes the key past the end of the list is a marker equal to the
// the maximum key so there is no need to do the bounds
// check until we've found a key that is greater than or equal to _xKey.
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    while ((_pxKeys)[_nPos] < (_xKey)) { ++(_nPos); } \
    if (((_pxKeys)[_nPos] > (_xKey)) \
        || PAST_ENDF(((_pxKeys) + (_nPos)), _nPopCnt, _pxKeys, _nPos)) \
    { \
        (_nPos) ^= -1; \
    } \
}

// Simple backward linear search of a sub-list.
// It assumes the key before the beginning of the list is zero so there is no
// need to do the bounds check until we've found a key that is less than or
// equal to _xKey.
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos + _nPopCnt - 1].
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    while ((_xKey) < (_pxKeys)[_nPos]) { --(_nPos); } \
    if (((_xKey) > (_pxKeys)[_nPos]) \
        || PAST_ENDB(((_pxKeys) + (_nPos)), _pxKeys, _nPos)) \
    { \
        ++(_nPos); (_nPos) ^= -1; \
    } \
}

#else // defined(LIST_END_MARKERS)

#define TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)  0

#define TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)  0
#define TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)  0

// Forward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHFX(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_pxKeys)[(_nPos) + (_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SSEARCHF((_pxKeys), (_xKey), (_nPos)); \
    } \
}

//#define TRY_MEMCHR
#if defined(TRY_MEMCHR)

// Backward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if (sizeof(_x_t) == sizeof(unsigned char)) { \
        _x_t *px; \
        px = (_x_t *)memchr(((_pxKeys) + (_nPos)), (_xKey), (_nPopCnt)); \
        (_nPos) = (px == NULL) ? -1 : px - (_pxKeys); \
    } else if (sizeof(_x_t) == sizeof(wchar_t)) { \
        _x_t *px; \
        px = (_x_t *)wmemchr((wchar_t *)((_pxKeys) + (_nPos)), \
                                         (_xKey), (_nPopCnt)); \
        (_nPos) = (px == NULL) ? -1 : px - (_pxKeys); \
    } else { \
        SEARCHFX(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos); \
    } \
}

#else // defined(TRY_MEMCHR)

#define SEARCHF(_x_t, _pxKeys0, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_pxKeys0)[(_nPos) + (_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SSEARCHF((_pxKeys0), (_xKey), (_nPos)); \
    } \
}

#endif // defined(TRY_MEMCHR)

// Backward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos + _nPopCnt - 1].
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_xKey) < (_pxKeys)[_nPos]) { \
        (_nPos) ^= -1; \
    } else { \
        (_nPos) += (_nPopCnt) - 1; SSEARCHB((_pxKeys), (_xKey), (_nPos)); \
    } \
}

#endif // defined(LIST_END_MARKERS)

#if 0

// Pick a bucket position directly rather than picking a key position first
// then deriving the bucket position from the key position.
// PSPLIT_NN picks a key position.
#define PSPLIT_P(_nPopCnt, _nBL, _xKey, _nSplit) \
{ \
    (_nSplit) = (((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt) * sizeof(_xKey)) \
                    /* + ((_nPopCnt) * sizeof(_xKey) / 2) */ ) \
                    / sizeof(Word_t)) \
                >> 9); \
}

#endif // 0

#if defined(SPLIT_SEARCH_BINARY)

#define PSPLIT(_nPopCnt, _nBL, _xKey, _nSplit)  ((_nSplit) = (_nPopCnt) >> 1)

#else // defined(SPLIT_SEARCH_BINARY)

  #if defined(OLD_PSPLIT)

// One old method:
// nSplit = (((_xKey) & MSK(_nBL)) * (_nPopCnt) + (_nPopCnt) / 2) >> (_nBL);
// Current method:
// Take the (_nBL - _nn) most significant bits of the _nBL least significant
// bits in _xKey times _nPopCnt and divide by EXP(_nBL - _nn).
// If _nBL < _nn then shift _wKey left as needed.
// The rounding term is probably insignificant and unnecessary at least in
// some cases.
// The trick is finding an expression for _nn that will work for all values
// of _nBL.
#define PSPLIT_NN(_nPopCnt, _nBL, _xKey, _nn, _nSplit) \
{ \
    /* make sure we don't overflow when we shift _nPopCnt with big _nBL */ \
    assert((_nPopCnt) <= (1 << (cnBitsPerWord - (_nBL) + (_nn)))); \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (_nBL))) \
                            >> (cnBitsPerWord + (_nn) - (_nBL))) \
                        * (_nPopCnt)) \
                    + ((_nPopCnt) / 2)) \
                >> ((_nBL) - (_nn))); \
}

// I think this value for _nn may work for everything except:
//    ((_nBL == cnBitsPerWord) && (_nPopCnt == 1))
// But PSPLIT isn't effective when the set of values is concentrated
// at one end of the expanse.  For example, the top of a 64-bit tree
// and 32-bit values.
// We need to make sure we are passing a constant in for _nBL for the
// performance path cases so the compiler can simplify this.
#define PSPLIT(_nPopCnt, _nBL, _xKey, _nSplit) \
        PSPLIT_NN((_nPopCnt), (_nBL), (_xKey), \
                  LOG((((_nPopCnt) << 1) - 1)) - cnBitsPerWord + (_nBL), \
                  (_nSplit))

  #else // defined(OLD_PSPLIT)

#define PSPLIT(_nPopCnt, _nBL, _xKey, _nPsplit) \
{ \
    /* make sure we don't overflow */ \
    DBG( (((_nBL) + LOG(_nPopCnt) + 1 > cnBitsPerWord) \
        ? printf("_nPopCnt %d\n", _nPopCnt) : 0) );   \
    assert((_nBL) + LOG(_nPopCnt) + 1 <= cnBitsPerWord); \
    (_nPsplit) = ((Word_t)((_xKey) & MSK(_nBL)) * (_nPopCnt) / EXP(_nBL)); \
}

  #endif // defined(OLD_PSPLIT)

#endif // defined(SPLIT_SEARCH_BINARY)

#if JUNK
pop <= 2 ^ (bpw - nbl + nn)
ceil(log(pop)) <= bpw - nbl + nn
LOG(pop * 2 - 1) <= bpw - nbl + nn
nn >= LOG(pop * 2 - 1) - bpw + nbl
nn  = LOG(pop * 2 - 1) - bpw + nbl
#endif

#if JUNK

#define PSPLIT(_nPopCnt, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
{ \
    unsigned nBL = LOG(((_xKeyMin) ^ (_xKeyMax)) | 1) + 1; \
    (_nSplit) = ((((((Word_t)(_xKey) << (cnBitsPerWord - (nBL))) \
                            >> (cnBitsPerWord - 9)) \
                        * (_nPopCnt)) \
                    /* + ((_nPopCnt) / 2) */ ) \
                / EXP(9)); \
}

#endif

#if JUNK

#define PSPLIT(_nn, _xKeyMin, _xKeyMax, _xKey, _nSplit) \
{ \
    (_nSplit) \
        = ((((Word_t)(_xKey) - (_xKeyMin)) * (_nn) + (_nn) / 2) \
            / ((Word_t)(_xKeyMax) - (_xKeyMin) + 1)); \
    if ((_nSplit) < 0) { (_nSplit) = 0; } \
    else if ((_nSplit) >= (_nn)) { (_nSplit) = (_nn) - 1; } \
}

#endif

#define PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    int nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    if (TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)) \
    { \
        (_nPos) += nSplit; \
    } \
    else if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        if (nSplit == (_nPopCnt) - 1) \
        { \
            (_nPos) = ~((_nPos) + (_nPopCnt)); \
        } \
        else if (TEST_AND_KEY_IS_MAX(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            (_nPos) += ((_pxKeys)[(_nPopCnt) - 1] == (_x_t)-1) \
                        ? (_nPopCnt) - 1 : ~(_nPopCnt); \
        } \
        else \
        { \
            (_nPos) = nSplit + 1; \
            SEARCHF(_x_t, (_pxKeys), (_nPopCnt) - nSplit - 1, \
                    (_xKey), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        if (TEST_AND_KEY_IS_ZERO(_x_t, _pxKeys, _nPopCnt, _xKey)) \
        { \
            if ((_pxKeys)[0] != 0) { (_nPos) ^= -1; } \
        } \
        else \
        { \
            assert((_nPos) == 0); \
            SEARCHB(_x_t, (_pxKeys), nSplit + 1, (_xKey), (_nPos)); \
        } \
    } \
}

static int
PsplitSearchByKey16(uint16_t *psKeys, int nPopCnt, uint16_t sKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    return nPos;
}

#if defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

// Little endian:
// 00:      10: smallest key
// 02:    32  : next smallest key
// 04:  54
// 06:76
// 08:fedcba98
// 10:76543210
// 18:fedcba98

//   key               key
// | big              small|
// |fe:dc|ba:98|76:54|32:10|

  #if defined(PSPLIT_EARLY_OUT)

// Has-key forward scan of a sub-list.
// With a check and early-out if we've gone past the key we want.
// It assumes _pxKeys and _nPos are bucket-size aligned.
// It assumes the last bucket is padded with keys that are in the list.
// It might be performance preferable if the last key in the last bucket is
// equal to the largest key in the list, even if it is just padding.
// sizeof(_xKey) is used to determine the size of the keys in the list.
// _nPopCnt is relative to _nPos
#define HASKEYF(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* first address beyond address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[_nPos + _nPopCnt]; \
    /* address of first bucket to search */ \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key first bucket to search */ \
    (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; \
    while ( ! BUCKET_HAS_KEY(px, (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the last key in the _b_t to see if we've gone too far */ \
        if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; } \
        ++px; (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
        if (px >= pxEnd) { (_nPos) ^= -1; break; } \
    } \
}

      #if defined(SUB_LIST)

// Has-key backward scan of a sub-list.
#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    /* address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of first key in first bucket to search */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
        if (&px[nxPos] < pxEnd) { (_nPos) = -1; break; } \
    } \
}

      #else // defined(SUB_LIST)

#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nxPos = ((_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in first bucket to search */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
        if (&px[nxPos] < (_b_t *)(_pxKeys)) { (_nPos) = -1; break; } \
    } \
}

      #endif // defined(SUB_LIST)

      #if JUNK
// Amazingly, the variant above was the best performing in my tests.
#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    _b_t *px = (_b_t *)(_pxKeys); \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    do { \
        if ((_xKey) >= (_pxKeys)[_nPos]) { \
            if (!BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8)) { \
                (_nPos = -1); \
            } \
            break; \
        } \
    } while (((_nPos) -= sizeof(_b_t) / sizeof(_xKey)), (nxPos-- >= 0)); \
}
#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    _b_t *px = (_b_t *)(_pxKeys); \
    int nxPos = ((_nPos) + (_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if (((_xKey) > (_pxKeys)[_nPos]) || (nxPos-- == 0)) { \
            (_nPos) = -1; break; \
        } \
        (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
    } \
}
      #endif // JUNK

  #else // defined(PSPLIT_EARLY_OUT)

// Has-key forward scan of a whole sub-list.
// Without a check and early-out if we've gone past the key we want.
#define HASKEYF(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* first address beyond address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[_nPos + _nPopCnt]; \
    /* address of first bucket to search */ \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key first bucket to search */ \
    (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; \
    while ( ! BUCKET_HAS_KEY(px, (_xKey), sizeof(_xKey) * 8) ) { \
        /* check to see if we've reached the end of the list */ \
        if (++px >= pxEnd) { (_nPos) ^= -1; break; } \
        (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
    } \
}

// Has-key backward scan of a whole sub-list.
// Without a check and early-out if we've gone past the key we want.
#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nxPos = (_nPos) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in first bucket to search */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! BUCKET_HAS_KEY(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check to see if we've reached the beginning of the list */ \
        if (nxPos <= 0) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
    } \
}

  #endif // defined(PSPLIT_EARLY_OUT)

  #if defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeys0, _nPos) \
{ \
    SEARCHF(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos); \
}

// Backward linear search of list (for any size key and with end check).
#define PSEARCHB(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
    SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
}

  #else // defined(PSPLIT_HYBRID)

// Linear parallel search of list (for any size key and with end check).
#define PSEARCHF(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
/* Is it wise to check the end here ? */ \
/* Or should we consider a search that checks if we're too far each time? */ \
    _x_t xKeyEnd = (_pxKeys)[(_nPos) + (_nPopCnt) - 1]; \
/* now we know the value of the key at the start and end of the range */ \
    if (xKeyEnd < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        HASKEYF(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
/*PSPLIT_SEARCH_RANGE(_xKey, _pxKeys, _nPopCnt, _xKeySplit, xKeyEnd, _nPos)*/\
    } \
}

// Backward linear parallel search (for any size key and with end check).
#define PSEARCHB(_b_t, _x_t, \
                 _pxKeys, _nPopCnt, _xKey, _xKeySplit, _nPos) \
{ \
/* Is it wise to check the start here ? */ \
/* Or should we consider a search that checks if we're too far each time? */ \
    _x_t xKey0 = (_pxKeys)[_nPos]; \
/* now we know the value of the key at the start and end of the range */ \
    if ((_xKey) < xKey0) { \
        (_nPos) ^= -1; \
    } else { \
        /*(_nPos) += (_nPopCnt) - 1;*/ \
        HASKEYB(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
    } \
}

  #endif // defined(PSPLIT_HYBRID)

  #if defined(SIMPLE_PSPLIT_HASKEY_GUTS)

#define PSPLIT_HASKEY_GUTS(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    unsigned nPsplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nPsplit); \
    nPsplit &= ~MSK(sizeof(_b_t)/sizeof(_x_t)); \
    _x_t xKeyPsplit = (_pxKeys)[nPsplit]; \
    if (xKeyPsplit <= (_xKey)) { \
        (_nPos) += nPsplit; (_nPopCnt) -= nPsplit; \
        HASKEYF(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
    } else if (nPsplit == 0) { (_nPos) = ~0; } else { \
        HASKEYB(_b_t, (_xKey), (_pxKeys), nPsplit, (_nPos)); \
    } \
}

  #else // defined(SIMPLE_PSPLIT_HASKEY_GUTS)

// Split search with a parallel search of the bucket at the split point.
// A bucket is a Word_t or an __m128i.  Or whatever else we decide to pass
// into _b_t in the future.
// nSplit is a bucket number.
// We need a function we can call iteratively.  The position returned
// ultimately must be relative to the original beginning of the list.
// What parameters must we pass?
// We have to pass the following information:
// - First key/bucket of the sub-list to search: _pxKeys.
// - Number of keys/buckets in the sub-list to search: _nPopCnt.
// - What is the offset within the full list of the sub-list to search: _nPos.
// We'd also like to be able to pass information about the key at the
// beginning and/or end of the list if we already know either or both of them.
// Can we use 0 and -1 for cases where we don't know them?  It might be
// more efficient to have separate macros for all cases.  Or maybe just
// a special case for neither is known.
//
// _b_t specifies the size of buckets in the list, e.g. Word_t, __m128i.
// _x_t specifies the size of the keys in the list, e.g. uint8_t, uint16_t.
// _nBL specifies the range of keys, i.e. the size of the expanse.
#define PSPLIT_HASKEY_GUTS(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    _b_t *px = (_b_t *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(_b_t); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    if (BUCKET_HAS_KEY(&px[nSplitP], (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
    } \
    else \
    { \
        nSplit = nSplitP * sizeof(_b_t) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we know the value of a key in the middle */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(_b_t)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(_b_t) / sizeof(_x_t); \
                HASKEYF(_b_t, (_xKey), \
                          (_pxKeys), (_nPopCnt) - (_nPos), (_nPos)); \
            } \
        } \
        else \
        { \
            if (nSplitP == 0) { \
                /* we searched the first bucket and the key is not there */ \
                (_nPos) = -1; /* this is where to insert */ \
            } else { \
                /* parallel search the head of the list */ \
                HASKEYB(_b_t, (_xKey), (_pxKeys), nSplit, (_nPos)); \
            } \
        } \
        assert(((_nPos) < 0) \
            || BUCKET_HAS_KEY((_b_t *) \
                                  ((Word_t)&(_pxKeys)[_nPos] \
                                      & ~MSK(LOG(sizeof(_b_t)))), \
                              (_xKey), sizeof(_x_t) * 8)); \
        /* everything below is just assertions */ \
        if ((_nPos) < 0) { \
            /* assert(~(_nPos) <= (int)(_nPopCnt)); not true */ \
            assert((~(_nPos) == (int)(_nPopCnt)) \
                    || (~(_nPos == 0)) \
                    || (~(_nPos) \
                        < (int)((_nPopCnt + sizeof(_b_t) - 1) \
                            & ~MSK(sizeof(_b_t))))); \
            for (int ii = 0; ii < (_nPopCnt); \
                 ii += sizeof(_b_t) / sizeof(_xKey)) \
            { \
                assert( ! BUCKET_HAS_KEY((_b_t *)&(_pxKeys)[ii], \
                          (_xKey), sizeof(_x_t) * 8) ); \
            } \
        } \
    } \
}

  #endif // defined(SIMPLE_PSPLIT_HASKEY_GUTS)

  #if JUNK
static int
PSplitSearch16(int nBL,
               uint16_t *psKeys, int nPopCnt, uint16_t sKey, int nPos)
{
again:;
    assert(nPopCnt > 0);
    assert(nPos >= 0); assert((nPos & ~MSK(sizeof(Bucket_t))) == 0);

    int nSplit; PSPLIT(nPopCnt - nPos, nBL, sKey, nSplit);
    // nSplit is a portion of (nPopCnt - nPos)
    assert(nSplit >= 0); assert(nSplit < (nPopCnt - nPos));
    nSplit &= ~MSK(sizeof(Bucket_t)); // first key in bucket
    nSplit += nPos; // make relative to psKeys

    if (BUCKET_HAS_KEY((Bucket_t *)&psKeys[nSplit], sKey, sizeof(sKey) * 8)) {
        return 0; // key exists, but we don't know the exact position
    }

    uint16_t sKeySplit = psKeys[nSplit];
    if (sKey > sKeySplit)
    {
        // bucket number of split
        int nSplitP = nSplit * sizeof(sKey) / sizeof(Bucket_t);
        int nSplitPLast = (nPopCnt - 1) * sizeof(sKey) / sizeof(Bucket_t);
        if (nSplitP == nSplitPLast) {
            // we searched the last bucket and the key is not there
            return -1; // we don't know where to insert
        }
        nPos = (int)nSplit + sizeof(Bucket_t) / sizeof(sKey);
        goto again;
    }

    if (nSplit == nPos) { return -1; }

    nPopCnt = nSplit;
    goto again;
}
  #endif // JUNK

  #if defined(COUNT)
#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)
  #else // defined(COUNT)
#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_HASKEY_GUTS(Bucket_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)
  #endif // defined(COUNT)

#else // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#endif // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if JUNK
static Status_t
TwoWordsHaveKey(Word_t *pw, Word_t wKey, int nBL)
{
    Word_t wLsbs = (Word_t)-1 / (EXP(nBL) - 1); // lsb in each key
    Word_t wReplicatedKey = (wKey & MSK(nBL)) * wLsbs;
    Word_t awXor[2] = { wReplicatedKey ^ pw[0], wReplicatedKey ^ pw[1] };
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key
    int bHasKey = ((((awXor[0] - wLsbs) & ~awXor[0] & wMsbs) != 0)
                || (((awXor[1] - wLsbs) & ~awXor[1] & wMsbs) != 0));
    return bHasKey ? Success : Failure;
}
#endif

#if defined(COMPRESSED_LISTS)
#if defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

// Search an array of words packed with keys that are smaller than a word
// using a parallel search of each word.
// WordArrayHasKey expects the keys to be packed towards the most significant
// bits, and it assumes all slots in every word have valid keys, i.e. the
// would-be empty slots have been padded with copies of some key/keys that
// is/are present.
// It also assumes that keys do not cross word boundaries.
static Status_t
WordArrayHasKey(Word_t *pw, unsigned nWords, Word_t wKey, int nBL)
{
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    unsigned nn = 0;
    do {
        Word_t wXor = wKeys ^ pw[nn]; // get zero in slot with matching key
        int bXorHasZero = (((wXor - wLsbs) & ~wXor & wMsbs) != 0); // magic
        if (bXorHasZero) { return Success; } // found the key
    } while (++nn < nWords);
    return Failure;
}

// Do a parallel search of a word for a key that is smaller than a word.
// WordHasKey expects the keys to be packed towards the most significant bits,
// and it assumes all slots in the word have valid keys, i.e. the would-be
// empty slots have been padded with copies of some key/keys that is/are
// present.
static Status_t
WordHasKey(Word_t *pw, Word_t wKey, int nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    return WordArrayHasKey(pw, /* nWords */ 1, wKey, nBL);
}

#else // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

// There are a lot of ways we can represent a bucket.
// Which way will be fastest?
// Should we require that the keys in the list be sorted?  Why?
// Is has-key faster if we fill empty slots with a present key?
// What about offset?  Assumes key is present.
// What about has-key-and-offset?  No offset returned if key is not present.
// What about offset-of-missing-key?  Assumes key is not present.
// What about has-key-or-offset-of-missing-key?  No off ret if key is present.
// What about has-key-and-offset-or-offset-of-missing-key?
//
// If keys are sorted and key[n+1] < key[n] && k[n+1] != 0, then bucket is
// illegal and we could use that to mean the bucket is empty.  It only
// requires that buckets can always hold more than one key,
// i.e. key_size <= word_size / 2.
// The second half of the test isn't necessary if we fill empty slots
// with the biggest present key.
//
// fill empty slots with 0
// fill empty slots with smallest key
// fill empty slots with largest key
// fill empty slots with -1
//
// sort with most-significant non-empty slot having the smallest key
// sort with least-significant slot having the smallest key
// don't sort; couldn't possibly help search
//
// empty slots are on most-significant end
// empty slots are on least-significant end
// empty slots are anywhere; can't imagine this helping search
//
// remainder bits are at the most-significant end
// remainder bits are at the least-significant end
//
// The value(s) we can use to represent an empty bucket depend
// on the choices made for the others.

// Do a parallel search of a word for a key that is smaller than a word.
// WordHasKey expects the keys to be packed towards the most significant bits,
// and it assumes all slots in the word have valid keys, i.e. the would-be
// empty slots have been padded with copies of some key/keys that is/are
// present.
// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.
static Word_t
WordHasKey(Word_t *pw, Word_t wKey, int nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    Word_t ww = *pw;
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask; // get rid of already-decoded bits
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
    return wMagic; // bXorHasZero = (wMagic != 0);
}

#if 0
// LocateKeyInWord is a work-in-progress.
// The rest of the code doesn't really set up embedded lists that are
// conducive to this operation yet.
static int
LocateKeyInWord(Word_t *pw, Word_t wKey, int nBL)
{
    Word_t wMagic = WordHasKey(pw, wKey, nBL);
    if (wMagic) {
        unsigned nBits = wr_nPopCnt(*pw, nBL) * nBL;
        assert(nBits != cnBitsPerWord);
        wMagic &= (MSK(nBits) << (cnBitsPerWord - nBits));
        wMagic &= -wMagic;
printf("*pw " OWx" wKey " Owx" nBL %d wMagic "OWx"\n", *pw, wKey, nBL, wMagic);
        return __builtin_clzll(wMagic) / nBL;
    } else {
        return ~0;
    }
}
#endif

#if defined(EMBED_KEYS)
// Find key or hole and return it's position.
static int
SearchEmbeddedX(Word_t *pw, Word_t wKey, int nBL)
{
    int ii;
    for (ii = 0; ii < wr_nPopCnt(*pw, nBL); ii++) {
        Word_t wSuffixLoop
               = GetBits(*pw, /* nBits */ nBL,
                         /* nLsb */ cnBitsPerWord - (nBL * (ii + 1)));
        if ((wKey & MSK(nBL)) <= wSuffixLoop) {
            if ((wKey & MSK(nBL)) == wSuffixLoop) { return ii; }
            break;
        }
    }
    return ~ii;
}
#endif // defined(EMBED_KEYS)

#endif // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

#if defined(PARALLEL_128)

  #if cnBitsPerWord == 64
#define MM_SET1_EPW(_ww) \
    _mm_set1_epi64((__m64)_ww)
  #else // cnBitsPerWord == 64
#define MM_SET1_EPW(_ww) \
    _mm_set1_epi32(_ww)
  #endif // cnBitsPerWord == 64

#define HAS_KEY_128_SETUP(_wKey, _nBL, _xLsbs, _xMsbs, _xKeys) \
{ \
    Word_t wMask = MSK(_nBL); /* (1 << nBL) - 1 */ \
    _wKey &= wMask; \
    Word_t wLsbs = (Word_t)-1 / wMask; \
    _xLsbs = MM_SET1_EPW(wLsbs); \
    Word_t wMsbs = wLsbs << (nBL - 1); /* msb in each key slot */ \
    _xMsbs = MM_SET1_EPW(wMsbs); \
    Word_t wKeys = wKey * wLsbs; /* replicate key; put in every slot */ \
    _xKeys = MM_SET1_EPW(wKeys); \
}

static Word_t // bool
HasKey128Tail(__m128i *pxBucket,
    __m128i xLsbs,
    __m128i xMsbs,
    __m128i xKeys)
{
    __m128i xBucket = *pxBucket;
    __m128i xXor = xKeys ^ xBucket;
    __m128i xMagic = (xXor - xLsbs) & ~xXor & xMsbs;
    __m128i xZero = _mm_setzero_si128();
    return ! _mm_testc_si128(xZero, xMagic);
}

// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.
static Word_t // bool
HasKey128(__m128i *pxBucket, Word_t wKey, int nBL)
{
    __m128i xLsbs, xMsbs, xKeys;
    HAS_KEY_128_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys);
    return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
}

#elif defined(PARALLEL_64) // defined(PARALLEL_128)

static uint64_t
HasKey64(uint64_t *px, Word_t wKey, int nBL)
{
    // It helps Lookup performance to eliminate the need to know nPopCnt.
    // So we replicate the first key in the list into the unused slots
    // at insert time to make sure the unused slots don't cause a false
    // bXorHasZero.
    uint64_t xx = *px;
    uint64_t xMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= xMask; // get rid of already-decoded bits
    uint64_t xLsbs = (uint64_t)-1 / xMask; // lsb in each key slot
    uint64_t xKeys = wKey * xLsbs; // replicate key; put in every slot
    uint64_t xMsbs = xLsbs << (nBL - 1); // msb in each key slot
    uint64_t xXor = xKeys ^ xx; // get zero in slot with matching key
    uint64_t xMagic = (xXor - xLsbs) & ~xXor & xMsbs;
    return xMagic; // bXorHasZero = (xMagic != 0);
}

#endif // defined(PARALLEL_128)

#endif // defined(COMPRESSED_LISTS)

#endif // defined(PSPLIT_PARALLEL) && ! defined(LIST_END_MARKERS)

#if ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)

#if defined(COMPRESSED_LISTS)
  #if (cnBitsInD1 <= 8)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList8(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

    assert(nBL <= 8);
    // sizeof(__m128i) == 16 bytes
  #if defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL) \
      && defined(PARALLEL_128) && (cnListPopCntMax8 <= 8)
    // By simply setting nPopCnt = 16 here we are assuming, while not
    // ensuring, that pop count never exceeds 16 here.
    // We do it because reading the pop count is so much slower.
    assert(PWR_xListPopCnt(pwRoot, pwr, 8) <= 16);
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 8);
    //int nPopCnt = 16; // Sixteen fit so why do less?
  #else // defined(PSPLIT_SEARCH_8) && ...
      #if defined(PP_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, NULL, nBL);
      #else // defined(PP_IN_LINK)
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 8);
      #endif // defined(PP_IN_LINK)
  #endif // defined(PSPLIT_SEARCH8) && ...
    uint8_t *pcKeys = ls_pcKeysNATX(pwr, nPopCnt);

#if defined(LIST_END_MARKERS)
    assert(pcKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint8_t *)(((Word_t)&pcKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint8_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(pcKeys[nPopCnt] == (uint8_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint8_t cKey = (uint8_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_8)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 8) {
        PSPLIT_SEARCH_BY_KEY(uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_SEARCH_BY_KEY(uint8_t, nBL, pcKeys, nPopCnt, cKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_8)
    SEARCHB(uint8_t, pcKeys, nPopCnt, cKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint8_t, pcKeys, nPopCnt, cKey, nPos); (void)nBL;
#endif // ...
    return nPos;
}

static int
ListHasKey8(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    return SearchList8(pwRoot, pwr, wKey, nBL) >= 0;
}

  #endif // (cnBitsInD1 <= 8)
#endif // defined(COMPRESSED_LISTS)

#if defined(COMPRESSED_LISTS)
  #if (cnBitsInD1 <= 16)
// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList16(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

    assert(nBL >   8);
    assert(nBL <= 16);
  #if defined(PP_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
  #else // defined(PP_IN_LINK)
      #if 0
      #if (cnBitsLeftAtDl2 <= 16)
      #if /* defined(PSPLIT_SEARCH_16) && */ defined(PSPLIT_PARALLEL) \
              && defined(PARALLEL_128) && !defined(INSERT)
          // sizeof(__m128i) == 16 bytes
          #if ! defined(cnListPopCntMaxDl2) || (cnListPopCntMaxDl2 <= 8)
          #if (cnListPopCntMax16 <= 8)
    assert(PWR_xListPopCnt(pwRoot, nBL) <= 8);
    int nPopCnt = 8; // Eight fit so why do less?
    assert((cnListPopCntMaxDl1 <= 8) || (cnBitsInD1 <= 8));
          #elif (cnBitsInD1 > 8) // nDL == 1 is handled here
              #if (cnListPopCntMaxDl1 <= 8) // list fits in one __m128i
                  #if (cnBitsLeftAtDl2 <= 16) // need to test nDL
    int nPopCnt = (nBL == cnBitsInD1) ? 8 : PWR_xListPopCnt(pwRoot, 16);
                  #else // (cnBitsLeftAtDl2 <= 16)
    int nPopCnt = 8; // Eight fit so why do less?
                  #endif // (cnBitsLeftAtDl2 <= 16)
              #else // (cnListPopCntMaxDl1 <= 8)
    int nPopCnt = PWR_xListPopCnt(pwRoot, 16);
              #endif // (cnListPopCntMaxDl1 <= 8)
          #else // (cnListPopCntMax16 <= 8)
    int nPopCnt = PWR_xListPopCnt(pwRoot, 16);
          #endif // (cnListPopCntMax16 <= 8)
          #else // ! defined(cnListPopCntMaxDl2) || (cnListPopCntMaxDl2 <= 8)
    int nPopCnt = PWR_xListPopCnt(pwRoot, 16);
          #endif // ! defined(cnListPopCntMaxDl2) || (cnListPopCntMaxDl2 <= 8)
      #else // defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL) && ...
    int nPopCnt = PWR_xListPopCnt(pwRoot, 16);
      #endif // defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL) && ...
      #else // (cnBitsLeftAtDl2 <= 16)
    int nPopCnt = PWR_xListPopCnt(pwRoot, 16);
      #endif // (cnBitsLeftAtDl2 <= 16)
      #else
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 16);
      #endif
  #endif // defined(PP_IN_LINK)
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("SearchList16 nPopCnt %d psKeys %p\n", nPopCnt, (void *)psKeys));

    (void)nBL;
  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL) && !defined(INSERT)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16) && !defined(INSERT)
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 16) {
        PSPLIT_SEARCH_BY_KEY(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        //nPos = PSplitSearch16(nBL, psKeys, nPopCnt, sKey, nPos);
        PSPLIT_SEARCH_BY_KEY(uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #else // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #endif // ...
    return nPos;
}

static int
ListHasKey16(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

    assert(nBL >   8);
    assert(nBL <= 16);
  #if defined(PP_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
  #else // defined(PP_IN_LINK)
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 16);
  #endif // defined(PP_IN_LINK)
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("SearchList16 nPopCnt %d psKeys %p\n", nPopCnt, (void *)psKeys));

    (void)nBL;
  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL) && !defined(INSERT)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16) && !defined(INSERT)
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 16) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #else // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #endif // ...
    return nPos >= 0;
}

  #endif // (cnBitsInD1 <= 16)
#endif // defined(COMPRESSED_LISTS)

#if defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) \
    && (cnBitsInD1 <= 32)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList32(uint32_t *piKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    (void)nBL;
    assert(nBL >  16);
    assert(nBL <= 32);
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // ! defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_32)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 32) {
        PSPLIT_SEARCH_BY_KEY(uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBL == 24) {
        PSPLIT_SEARCH_BY_KEY(uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_SEARCH_BY_KEY(uint32_t, nBL, piKeys, nPopCnt, iKey, nPos);
        DBGX(printf("SearchList32 nPos %d\n", nPos));
    }
#elif defined(BACKWARD_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#endif // ...
    return nPos;
}

static int
ListHasKey32(uint32_t *piKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    (void)nBL;
    assert(nBL >  16);
    assert(nBL <= 32);
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // ! defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_32)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 32) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBL == 24) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, nBL, piKeys, nPopCnt, iKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#else // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
#endif // ...
    return nPos >= 0;
}

#endif // defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) && ...

#endif // ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)
#endif // ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#endif // ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)

//
// Valid combinations:
// ==================
//
// (qty  2) [ratio-]split-loop-w-threshold=2
// (qty 80) no-split|[ratio-]split-w-no-loop|[ratio-]split-loop-w-threshold>2
//            x [no-]end-check
//            x (for|back)ward
//            x (continue|succeed|fail)-first|succeed-only
//
// split-loop-w-threshold=2 is a binary search
//
// no-split => no-split-loop
// ratio-split => split
// split-loop => split
// succeed-only <=> no-(continue|succeed|fail)-first
// continue-first => no-(succeed|fail)-first
// fail-first => no-(continue|succeed)-first
// succeed-first => no-(continue|fail)-first
// no-sort => no-split && no-end-check && succeed-only
//
// Common combinations:
// ===================
//
// default: no-split, no-end-check, succeed-only, forward
// split-loop-w-threshold=20, end-check, continue-first
//

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchListWord(Word_t *pwKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    DBGI(printf("SLW pwKeys %p wKey " OWx" nBL %d nPopCnt %d\n",
                (void *)pwKeys, wKey, nBL, nPopCnt));
    (void)nBL;
#if defined(LIST_END_MARKERS)
    assert(pwKeys[-1] == 0);
    assert(pwKeys[nPopCnt] == (Word_t)-1);
#endif // defined(LIST_END_MARKERS)
    int nPos = 0;
#if defined(PSPLIT_SEARCH_WORD)
#if defined(PSPLIT_SEARCH_XOR_WORD)
    Word_t wKeyMin = pwKeys[0];
    Word_t wKeyMax = pwKeys[nPopCnt - 1];
    // Or in 1 to handle nPopCnt==1 else we'd be taking the LOG of zero.
    nBL = LOG((wKeyMin ^ wKeyMax) | 1) + 1;
    // nBL could be 64 and it could be 0.
#endif // defined(PSPLIT_SEARCH_XOR_WORD)
    if (nBL <= (cnBitsPerWord - 8)) {
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
        if (nBL == 32) {
            PSPLIT_SEARCH(Word_t, 32, pwKeys, nPopCnt, wKey, nPos);
        } else
#if (cnBitsPerWord > 32)
        if (nBL == 40) {
            PSPLIT_SEARCH(Word_t, 40, pwKeys, nPopCnt, wKey, nPos);
        } else
#else // (cnBitsPerWord > 32)
        if (nBL == 24) {
            PSPLIT_SEARCH(Word_t, 24, pwKeys, nPopCnt, wKey, nPos);
        } else
#endif // (cnBitsPerWord > 32)
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
        {
            PSPLIT_SEARCH(Word_t, nBL, pwKeys, nPopCnt, wKey, nPos);
        }
    } else { // here to avoid overflow
        unsigned nSplit
            = ((wKey & MSK(nBL)) >> 8) * nPopCnt + nPopCnt / 2 >> (nBL - 8);
        if (pwKeys[nSplit] < wKey) {
            if (nSplit == nPopCnt - 1) { return ~nPopCnt; }
            SEARCHF(Word_t, pwKeys, nPopCnt - nSplit - 1,
                       wKey, nPos);
        } else { // here if wKey <= pwKeys[nSplit]
            SEARCHB(Word_t, pwKeys, nSplit + 1, wKey, nPos);
        }
    }
#else // defined(PSPLIT_SEARCH_WORD)
    Word_t *pwKeysOrig = pwKeys;
    (void)nPos;
  #if defined(BINARY_SEARCH_WORD)
  // BINARY_SEARCH narrows the scope of the linear search that follows.
    unsigned nSplit;
    // Looks like we might want a loop threshold of 8 for
    // 64-bit keys at the top level.
    // And there's not much difference with threshold of
    // 16 or 64.
    // Not sure about 64-bit keys at a lower level or
    // 64-bit keys at the top level.
    while (nPopCnt >= cnBinarySearchThresholdWord)
    {
        nSplit = nPopCnt / 2;
        //DBGI(printf("SLW nSplit %d\n", nSplit));
        if (pwKeys[nSplit] <= wKey) {
            pwKeys = &pwKeys[nSplit];
            nPopCnt -= nSplit;
        } else {
            nPopCnt = nSplit;
            if (nPopCnt == 0) {
                assert(~(pwKeys - pwKeysOrig) < 0);
                return ~(pwKeys - pwKeysOrig);
            }
        }
    }
  #endif // defined(BINARY_SEARCH_WORD)
    nPos = pwKeys - pwKeysOrig;
  #if defined(BACKWARD_SEARCH_WORD)
    SEARCHB(Word_t, pwKeysOrig, nPopCnt, wKey, nPos);
  #else // defined(BACKWARD_SEARCH_WORD)
    SEARCHF(Word_t, pwKeysOrig, nPopCnt, wKey, nPos);
  #endif // defined(BACKWARD_SEARCH_WORD)
#endif // defined(PSPLIT_SEARCH_WORD)
    DBGX(printf("SLW: return pwKeysOrig %p nPos %d\n",
                (void *)pwKeysOrig, nPos));
    return nPos;
}

static int
ListHasKeyWord(Word_t *pwKeys, Word_t wKey, unsigned nBL, int nPopCnt)
{
    return SearchListWord(pwKeys, wKey, nBL, nPopCnt) >= 0;
}

#if JUNK
#define MAGIC1(_nBL)  MAXUINT / ((1 << (_nBL)) - 1)
#define MAGIC1(_nBL)  (cnMagic[_nBL])
#define MAGIC2(_nBL)  (MAGIC1(_nBL) << ((_nBL) - 1))
// Index cnMagic by nBitsLeft.
// Keys are stored by packing them at the high-order end of the word
// and leaving enough room for pop count and type and the low end.
Word_t cnMagic[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    0x0010010010010000,
    0x0008004002001000,
    0x0004001000400100,
    0x0002000400080000,
};
#endif

// Magics for replicating keys and anding:
// Second one is first one shifted left by (nBitsLeft - 1).

// 6+6: 0x0010010010010000
// 6+6: 0x8008008008000000

// 6+7: 0x0008004002001000
// 6+7: 0x8004002001000000

// 6+8: 0x0004001000400100
// 6+8: 0x8002000800200000

// 6+9: 0x0002000400080000
// 6+9: 0x8001000200040000
//
// HasZero subtracts one from each which will leave all ones.
// And it complements the whole value (all ones) and ands it with high bit.
// Then it ands the two intermediate results.
//
// (abc - 001) & (ABC & 100)
// (a-b+C)(b-C)C    &  A00
//  000  111 & 100 => 100
//  001  000 & 100 => 000
//  010  001 & 100 => 000
//  011  010 & 100 => 000
//  100  011 & 000 => 000
//  101  100 & 000 => 000
//  110  101 & 000 => 000
//  111  110 & 000 => 000
#ifdef  TBD

#define repbyte1(s) (((-((Word_t)1))/255) * (s))
#define haszero1(v) (((v) - repbyte1(0x1)) & ~(v) & repbyte1(0x80))
#define hasvalue1(x,n) haszero1((x) ^ (n))

// For finding a zero byte in a Word_t may be usefull someday
// Note: untested
// #define haszero(v)        (((v) - 0x01010101UL) & ~(v) & 0x80808080UL)

// Does Word x have a byte == v
// #define hasvalue(x,n)     (haszero((x) ^ (~0UL/255 * (n))))

// For finding if Word_t has a byte == VALUE
#define haszero(VALUE)                                                  \
    ((VALUE) ((-((Word_t)1)) * 255) & ~(v) & (-((((Word_t)1) * 255) * 8)))

#define hasvalue(x,n) (haszero((x) ^ (-((Word_t)1))/255 * (n)))
#endif  // TBD

#if ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)

// Find wKey (the undecoded bits) in the list.
// If it exists, then return its index in the list.
// If it does not exist, then return the one's complement of the index where
// it belongs.
// Lookup doesn't need to know where key should be if it is not in the list.
// Only Insert and Remove benefit from that information.
// And even Insert and Remove don't need to know where the key is if it is
// in the list (until we start thinking about JudyL).
static int
SearchList(Word_t *pwr, Word_t wKey, unsigned nBL, Word_t *pwRoot)
{
    (void)pwRoot;

    DBGL(printf("SearchList pwRoot %p wRoot " OWx" wKey " Owx" nBL %d\n",
                (void *)pwRoot, *pwRoot, wKey, nBL));

    int nPopCnt;
    int nPos;

  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
      // There is no need for a key size that is equal to or smaller than
      // whatever size yields a bitmap that will fit in a link.
    if (nBL <= 8) {
        nPos = SearchList8(pwRoot, pwr, wKey, nBL);
    } else
      #endif // defined(cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBL <= 16) {
        assert(nBL > 8);
        nPos = SearchList16(pwRoot, pwr, wKey, nBL);
    } else
      #endif // defined(cnBitsInD1 <= 16)
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBL <= 32) {
        assert(nBL > 16);
          #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
          #else // defined(PP_IN_LINK)
        nPopCnt = Get_xListPopCnt(pwRoot, 32);
          #endif // defined(PP_IN_LINK)
        nPos = SearchList32(ls_piKeysNATX(pwr, nPopCnt), wKey, nBL, nPopCnt);
    } else
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    {
  #if defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
      #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
      #else // defined(PP_IN_LINK)
        nPopCnt = Get_xListPopCnt(pwRoot, cnBitsPerWord);
      #endif // ! defined(PP_IN_LINK)
        nPos = SearchListWord(ls_pwKeysNATX(pwr, nPopCnt),
                              wKey, nBL, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
      #if defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord) {
            nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
        } else
      #endif // ! defined(PP_IN_LINK)
        { nPopCnt = Get_xListPopCnt(pwRoot, cnBitsPerWord); }
        //printf("pwRoot %p pwr %p\n", (void *)pwRoot, (void *)pwr);
        nPos = SearchListWord(ls_pwKeys(pwr, nBL), wKey, nBL, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    }

  #if defined(LOOKUP)
    SMETRICS(j__SearchPopulation += nPopCnt);
  #endif // defined(LOOKUP)

    return nPos;
}

// Figure out if the key is in the sorted list.
// Return any non-negative number if the key is in the list.
// Return any negative number if the key is not in the list.
static int
ListHasKey(Word_t *pwr, Word_t wKey, unsigned nBL, Word_t *pwRoot)
{
    (void)pwRoot;

    DBGL(printf("ListHasKey pwRoot %p wRoot " OWx" wKey " Owx" nBL %d\n",
                (void *)pwRoot, *pwRoot, wKey, nBL));

    int nPopCnt;
    int bHasKey;

  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
      // There is no need for a key size that is equal to or smaller than
      // whatever size yields a bitmap that will fit in a link.
    if (nBL <= 8) {
        bHasKey = ListHasKey8(pwRoot, pwr, wKey, nBL);
    } else
      #endif // defined(cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBL <= 16) {
        assert(nBL > 8);
        bHasKey = ListHasKey16(pwRoot, pwr, wKey, nBL);
    } else
      #endif // defined(cnBitsInD1 <= 16)
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBL <= 32) {
        assert(nBL > 16);
          #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
          #else // defined(PP_IN_LINK)
        nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 32);
          #endif // defined(PP_IN_LINK)
        bHasKey = ListHasKey32(ls_piKeysNATX(pwr, nPopCnt), wKey, nBL,
                               nPopCnt);
    } else
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    {
  #if defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
      #if defined(PP_IN_LINK)
        nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
      #else // defined(PP_IN_LINK)
        nPopCnt = PWR_xListPopCnt(pwRoot, pwr, cnBitsPerWord);
      #endif // ! defined(PP_IN_LINK)
        bHasKey = ListHasKeyWord(ls_pwKeysNATX(pwr, nPopCnt),
                                 wKey, nBL, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
      #if defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord) {
            nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
        } else
      #endif // ! defined(PP_IN_LINK)
        { nPopCnt = PWR_xListPopCnt(pwRoot, pwr, cnBitsPerWord); }
        //printf("pwRoot %p pwr %p\n", (void *)pwRoot, (void *)pwr);
        bHasKey = ListHasKeyWord(ls_pwKeys(pwr, nBL), wKey, nBL, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    }

  #if defined(LOOKUP)
    SMETRICS(j__SearchPopulation += nPopCnt);
  #endif // defined(LOOKUP)

    return bHasKey;
}

// Locate the key in the sorted list.
// Return the position of the key in the list.
// Return any negative number if the key is not in the list.
static int
LocateKey(Word_t *pwr, Word_t wKey, unsigned nBL, Word_t *pwRoot)
{
    return SearchList(pwr, wKey, nBL, pwRoot);
}

// Locate the slot in the sorted list where the key should be.
// Return the position of the slot in the list.
// Return any negative number if the key is already in the slot.
static int
LocateHole(Word_t *pwr, Word_t wKey, unsigned nBL, Word_t *pwRoot)
{
    return ~SearchList(pwr, wKey, nBL, pwRoot);
}

#endif // ! defined(LOOKUP_NO_LIST_SEARCH) || ! defined(LOOKUP)
#endif // ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#endif // ! defined(ONE_DEREF_AT_LIST) || ! defined(LOOKUP)

#endif // (cwListPopCntMax != 0)

#if ! defined(LOOKUP_NO_LIST_DEREF) || ! defined(LOOKUP)
#if defined(EMBED_KEYS) \
      && (   (defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)) \
          || (defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP)))

// Do a parallel search of a list embedded in wRoot given the key size.
// The least-significant nBL_to_nBitsType(nBL) bits of the word are used for
// a type field and the next least-significant nBL_to_nBitsPopCntSz(nBL) bits
// of the word are used for a population count.
// EmbeddedListHasKey expects the keys to be packed towards the most
// significant bits unless PACK_KEYS_RIGHT in which case they are packed
// towards the least significant bits leaving room for the type and pop count.
// It helps Lookup performance to eliminate the need to know nPopCnt.
// So, if FILL_W_KEY, we replicate the smallest key in the list into the
// unused slots at insert time to make sure the unused slots don't cause a
// false bXorHasZero.
// But how do we make sure the type and pop count bits don't
// cause a false bXorHasZero due to a slot that can't really be used?
// Or'ing MSK(nBL_to_nBitsPopCntSz(nBL) + cnBitsMallocMask)
// would be sufficient, but it may be expensive.
// Can we do something simpler/faster?  Something at insert time?
// Unfortunately, it doesn't matter how the pop and type bits are set
// in the word since we are xoring them with the key we're looking for
// before calculating bXorHasZero.  And whatever they are set to will
// match the key/keys that is/are the same.
// I wonder if the next best thing is to have a constant that we can
// or into wXor before calculating bXorHasZero.
// Does cnMallocMask work?  It will cover any key slot that extends
// into cnMallocMask.  But what about a slot that extends into the pop
// count field and not into cnMallocMask?
// Sure would be nice if we had a constant width pop field.  What would
// be the cost?  3-bits of pop for 64-bit costs one 29-bit key slot.
// 2-bits of pop for 32-bit costs one 14-bit key slot.
// If we're not using those key sizes, then there is no cost.
// What if we have no valid-key fill?  And no pop field?
static Word_t
EmbeddedListMagic(Word_t wRoot, Word_t wKey, int nBL)
{
#if defined(NO_TYPE_IN_XX_SW)
    assert((wRoot != ZERO_POP_MAGIC)
        || ((nBL_to_nBitsType(nBL) != 0) && (wRoot != 0)));
#else // defined(NO_TYPE_IN_XX_SW)
    assert(nBL_to_nBitsType(nBL) != 0);
    assert(wRoot != 0);
#endif // defined(NO_TYPE_IN_XX_SW)
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask; // Discard already-decoded bits.  Have caller do it?
#if ! defined(FILL_W_KEY) && ! defined(MASK_EMPTIES)
    // If we're filling empty slots with 0 or -1, and we're not masking off
    // the empty slots later, then check for wKey == fill
    // here so we don't have to worry about a false positive later.
    // We still have to mask off the type and pop count bits from wXor later
    // even if we're not masking off the empty slots.
  #if defined(REVERSE_SORT_EMBEDDED_KEYS)
      #if defined(FILL_WITH_ONES)
    if (wKey == MSK(nBL)) {
          #if defined(PACK_KEYS_RIGHT)
        int nPopCntMax = EmbeddedListPopCntMax(nBL);
        int nPopCnt = wr_nPopCnt(wRoot, nBL);
        int nSlot = nPopCntMax - nPopCnt + 1;
          #else // defined(PACK_KEYS_RIGHT)
        int nSlot = 1;
          #endif // defined(PACK_KEYS_RIGHT)
        return (((wRoot >> (cnBitsPerWord - nSlot * nBL)) & MSK(nBL))
                    == MSK(nBL));
    }
      #else // defined(FILL_WITH_ONES)
    if (wKey == 0) {
          #if defined(PACK_KEYS_RIGHT)
        int nSlot = EmbeddedListPopCntMax(nBL);
          #else // defined(PACK_KEYS_RIGHT)
        int nSlot = wr_nPopCnt(wRoot, nBL);
          #endif // defined(PACK_KEYS_RIGHT)
        return
            (((wRoot >> (cnBitsPerWord - nSlot * nBL)) & MSK(nBL)) == 0);
    }
      #endif // defined(FILL_WITH_ONES)
  #else // defined(REVERSE_SORT_EMBEDDED_KEYS)
    if (wKey == 0) { return ((wRoot >> (cnBitsPerWord - nBL)) == 0); }
  #endif // defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // ! defined(FILL_W_KEY) && ! defined(MASK_EMPTIES)
    Word_t wLsbs = (Word_t)-1 / wMask;
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ wRoot; // get zero in slot with matching key
#if defined(MASK_EMPTIES)
  // If we're filling empty slots with 0 or -1, then we have to mask off the
  // empty slots so we don't get a false positive if/when wKey == 0 or -1.
  #if defined(FILL_W_KEY)
      // When would FILL_W_KEY and MASK_EMPTIES make sense?
      // Why bother filling with key if we're going to mask them?
      #error MASK_EMPTIES and FILL_W_KEY makes no sense.
  #endif // defined(FILL_W_KEY)
    // Looks like MASK_EMPTIES and ! EMBEDDED_LIST_FIXED_POP may be an
    // especially bad combination.
    int nPopCnt = wr_nPopCnt(wRoot, nBL); // number of keys present
    int nBitsOfKeys = nPopCnt * nBL;
    wXor |= (Word_t)-1 >> nBitsOfKeys; // type and empty slots
#else // defined(MASK_EMPTIES)
    wXor |= MSK(nBL_to_nBitsType(nBL) + nBL_to_nBitsPopCntSz(nBL));
#endif // defined(MASK_EMPTIES)
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    return (wXor - wLsbs) & ~wXor & wMsbs; // wMagic
}

static int // bool
EmbeddedListHasKey(Word_t wRoot, Word_t wKey, int nBL)
{
    return EmbeddedListMagic(wRoot, wKey, nBL) != 0;
}

#endif // defined(EMBED_KEYS) ...
#endif // ! defined(LOOKUP_NO_LIST_DEREF)

#if defined(COMPRESSED_LISTS)
static Word_t
ls_pxKey(Word_t *pwr, int nBL, int ii)
{
    if (nBL <=  8) { return ls_pcKey(pwr, nBL, ii); }
    if (nBL <= 16) { return ls_psKey(pwr, nBL, ii); }
  #if cnBitsPerWord != 32
    if (nBL <= 32) { return ls_piKey(pwr, nBL, ii); }
  #endif // cnBitsPerWord != 32
    return ls_pwKey(pwr, nBL, ii);
}
#else // defined(COMPRESSED_LISTS)
#define ls_pxKey(_ls, _nBL, _ii)  (ls_pwKeys((_ls), (_nBL))[_ii])
#endif // defined(COMPRESSED_LISTS)

// Get bitmap switch link index/offset from (virtual) index extracted from key.
// If the index is not present then return the index at which it would be.
static inline void
BmSwIndex(Word_t *pwRoot, Word_t wIndex,
          Word_t *pwBmSwIndex, Word_t *pwBmSwBit)
{
    Word_t *pwBmWords = PWR_pwBm(pwRoot, wr_pwr(*pwRoot));
    // The bitmap may have more than one word.
    // nBmWordNum is the number of the word which contains the bit we want.
    int nBmWordNum = wIndex >> cnLogBitsPerWord;
    Word_t wBmWord = pwBmWords[nBmWordNum]; // word we want
    Word_t wBmBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
    if (pwBmSwIndex != NULL) {
        *pwBmSwIndex = 0;
        for (int nn = 0; nn < nBmWordNum; nn++) {
            *pwBmSwIndex += __builtin_popcountll(pwBmWords[nn]);
        }
        *pwBmSwIndex += __builtin_popcountll(wBmWord & (wBmBit - 1));
    }
    if (pwBmSwBit != NULL) {
        *pwBmSwBit = wBmWord & wBmBit;
    }
}

#endif // (cnDigitsPerWord > 1)

#endif // ( ! defined(_B_H_INCLUDED) )

