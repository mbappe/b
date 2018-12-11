
#if ( ! defined(_B_H_INCLUDED) )
#define _B_H_INCLUDED

// Work on list alignment independent of parallel search.
// 1. Change list init code to pad to aligned length rather than aligned
//    ending address.

#include <stdio.h>  // printf
#include <string.h> // memcpy
#include "Judy.h"   // Word_t, Judy1Test, JudyMalloc, ...
#include "bdefines.h"  // May define NDEBUG for assert.h.
#include <assert.h> // NDEBUG must be defined before including assert.h.

// Let's start with general purpose macros that aren't really specific
// to our program.

// Do these have any effect?
#if ! defined(likely)
#define   likely(_b) (__builtin_expect((_b), 1))
#define unlikely(_b) (__builtin_expect((_b), 0))
#endif // ! defined(likely)

// Do integer division by a power of two, but round up instead of down.
#define DIV_UP(_idend, _isor) \
    (/*assert(__builtin_popcountll(_isor) == 1),*/ \
        (((_idend) + (_isor) - 1) / (_isor)))

#define DIV_UP_X(_idend, _isor)  (((_idend) + (_isor) - 1) / (_isor))

// Align up to the next specified power of two alignment boundary.
#define ALIGN_UP(_idend, _isor) \
    (assert(((_isor) & -(_isor)) == (_isor)), \
        (((_idend) + (_isor) - 1) & ~((_isor) - 1)))

#define ALIGN_UP_X(_idend, _isor)  (((_idend) + (_isor) - 1) & ~((_isor) - 1))

#define cnBitsPerByte  8

// Default cnLogBitsPerWord is determined by __LP64__ and _WIN64.
#if ! defined(cnBitsPerWord)
  #if defined(__LP64__) || defined(_WIN64)
    #define cnBitsPerWord  64
  #else // defined(__LP64__) || defined(_WIN64)
    #define cnBitsPerWord  32
  #endif // defined(__LP64__) || defined(_WIN64)
#endif // ! defined(cnBitsPerWord)

#if (cnBitsPerWord != 64) && (cnBitsPerWord != 32)
    #error Unsupported cnBitsPerWord.
#endif

#define cnBytesPerWord  (cnBitsPerWord / cnBitsPerByte)

#define cnLogBitsPerByte  3

#if (cnBitsPerWord == 64)
    #define cnLogBitsPerWord 6
    #define cnLogBytesPerWord 3
#else // cnBitsPerWord
    #define cnLogBitsPerWord 5
    #define cnLogBytesPerWord 2
#endif // cnBitsPerWord

// This LOG macro works for 64-bit and 32-bit on Linux and Windows without
// any ifdef because we subtract clzll from 63 independent of cnBitsPerWord.
// This way we avoid using ifdef to choose the variant of __builtin_clz[l][l]
// we need. The cost is that we have to write this long comment.
// And what happens if clzll becomes 128-bit someday?
// __builtin_clzll is undefined for zero which allows the compiler to use bsr.
// So this macro is undefined for _x == 0.
#define LOG(_x)  (63 - (int)__builtin_clzll(_x))
#define EXP(_x)  (assert((_x) <= cnBitsPerWord), (Word_t)1 << (_x))
// NBPW - Not Bits Per Word - can't handle nBits == cnBitsPerWord
#define NBPW_MSK(_nBits) \
     (EXP(_nBits) - 1) // can't handle nBits == cnBitsPerWord
// NZ - Not Zero - can't handle nBits == 0
#define NZ_MSK(_nBits) \
    (assert((_nBits) > 0), (Word_t)-1 >> (cnBitsPerWord - (_nBits)))
// ANY_MSK can handle both _nBits == cnBitsPerWord AND _nBits == 0.
#if 1
#define ANY_MSK(_nBits) \
    ((((Word_t)1 << (_nBits)) - 1) - ((_nBits) == cnBitsPerWord))
#else
#define ANY_MSK(_nBits) \
    ((Word_t)-1 >> cnBitsPerWord - (_nBits)) + ((_nBits) == 0)
#endif
#define MSK(_nBits)  NBPW_MSK(_nBits)

// dlmalloc.c uses MALLOC_ALIGNMENT.
// Default MALLOC_ALIGNMENT is 2 * sizeof(void *).
// cnBitsMallocMask must be consitent with MALLOC_ALIGNMENT.
#ifdef cnBitsMallocMask
  #ifdef MALLOC_ALIGNMENT
    #if (1 << cnBitsMallocMask) != MALLOC_ALIGNMENT
      #error cnBitsMallocMask is inconsistent with MALLOC_ALIGNMENT
    #endif // ((1 << cnBitsMallocMask) - 1) != MALLOC_ALIGNMENT
  #else // MALLOC_ALIGNMENT
    #if cnBitsMallocMask != (cnLogBytesPerWord + 1)
      #error cnBitsMallocMask is inconsistent with MALLOC_ALIGNMENT
    #endif // cnBitsMallocMask != (cnLogBytesPerWord + 1)
  #endif // MALLOC_ALIGNMENT
#else // cnBitsMallocMask
  #if (MALLOC_ALIGNMENT == 4)
    #define cnBitsMallocMask 2
  #elif (MALLOC_ALIGNMENT == 8)
    #define cnBitsMallocMask 3
  #elif (MALLOC_ALIGNMENT == 16)
    #define cnBitsMallocMask 4
  #elif (MALLOC_ALIGNMENT == 32)
    #define cnBitsMallocMask 5
  #elif ! defined(MALLOC_ALIGNMENT)
    #define cnBitsMallocMask (cnLogBytesPerWord + 1)
  #else // MALLOC_ALIGNMENT
    #error Unsupported MALLOC_ALIGNMENT.
  #endif // MALLOC_ALIGNMENT
#endif // cnBitsMallocMask

#define cnLogMallocAlignment  cnBitsMallocMask
#define cnMallocAlignment  EXP(cnBitsMallocMask)
#define cnMallocMask  MSK(cnBitsMallocMask)

// Shorthand for common parameters.
// The parameters are all related to each other.
// nBL is the number of bits left to decode after identifying the given link.
// nBL does not include the bits skipped if the link is a skip link.
// pLn is NULL if nBL == cnBitsPerWord sizeof(Link_t) > sizeof(Word_t).
// Sure would like to get rid of pwRoot.
// And possibly add nBLR.
// And wPopCnt would be an option.
// And nBW would be an option.
// And how about wBytesUsed?
#define  qp \
    int   nBL, Link_t  * pLn, Word_t   wRoot, int   nType, Word_t  * pwr
#define pqp \
    int *pnBL, Link_t **ppLn, Word_t *pwRoot, int *pnType, Word_t **ppwr

// Shorthand for common arguments.
// Why is "qy" not "qa"? Because "qa" is harder to type?
#define  qy   nBL,  pLn,  wRoot,  nType,  pwr
#define pqy  &nBL, &pLn, &wRoot, &nType, &pwr

// Common arguments to printf.
#define qyp   nBL, (void*)pLn, wRoot, nType, (void*)pwr

#define  qyLoop \
    nBLLoop, pLnLoop, wRootLoop, nTypeLoop, pwrLoop

// Shorthand to silence not-used compiler warnings.
// And to validate assumptions.
#define  qv \
    Word_t *pwRoot = &pLn->ln_wRoot; \
    (void)nBL; (void)pLn; (void)pwRoot; (void)wRoot; (void)nType; (void)pwr; \
    assert(wRoot == pLn->ln_wRoot); \
    assert(nType == wr_nType(wRoot) || (nBL <= cnLogBitsPerLink)); \
    assert(pwr == wr_pwr(wRoot) || (nBL <= cnLogBitsPerLink))
#define pqv \
    (void)pnBL; (void)ppLn; (void)pwRoot; (void)pnType; (void)ppwr; \
    assert(*pwRoot == (*ppLn)->ln_wRoot); \
    assert(*pnType == wr_nType(*pwRoot) || (*pnBL <= cnLogBitsPerLink)); \
    assert(*ppwr == wr_pwr(*pwRoot) || (*pnBL <= cnLogBitsPerLink))

#define qfmt "nBL %2d pLn %p wRoot 0x%016zx nType %x pwr %p"

// Default is -USKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX.
// Default is -USAVE_PREFIX -USAVE_PREFIX_TEST_RESULT
// Default is -UALWAYS_CHECK_PREFIX_AT_LEAF.
// -DALWAYS_CHECK_PREFIX_AT_LEAF appears to be best for 64-bit Judy1 with
// 16-bit digits and a 16-bit bitmap. More digits and -DSKIP_LINKS of any
// flavor seems similar.
// -USKIP_LINKS is a lot better for 32-bit Judy1 with 16-bit digits and a
// 16-bit bitmap. Less so for 8-bit digits. Maybe.

// Default is -USEARCH_FROM_WRAPPER.

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)
#undef  SKIP_PREFIX_CHECK
#define SKIP_PREFIX_CHECK
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF) || defined(SAVE_PREFIX)

#if defined(SKIP_PREFIX_CHECK)
    #define cbSkipPrefixCheck  1
    #define IF_SKIP_PREFIX_CHECK(_expr)  (_expr)
#else // defined(SKIP_PREFIX_CHECK)
    #define cbSkipPrefixCheck  0
    #define IF_SKIP_PREFIX_CHECK(_expr)
#endif // defined(SKIP_PREFIX_CHECK)

// Default cn2dBmMaxWpkPercent.
// Create a 2-digit/big bm leaf when wpk gets below cn2dBmMaxWpkPercent/100.
#if ! defined(cn2dBmMaxWpkPercent)
  #if (cnBitsPerWord == 32)
#define cn2dBmMaxWpkPercent  30
  #else // (cnBitsPerWord == 32)
      #ifdef B_JUDYL
#define cn2dBmMaxWpkPercent  0  // For JudyL turn on.
      #else // B_JUDYL
#define cn2dBmMaxWpkPercent  15
      #endif // B_JUDYL
  #endif // (cnBitsPerWord == 32)
#endif // ! defined(cn2dBmMaxWpkPercent)

#if !defined(cnBmSwConvert)
#define cnBmSwConvert 9
#endif // !defined(cnBmSwConvert)
#if !defined(cnBmSwRetain)
#define cnBmSwRetain 2
#endif // !defined(cnBmSwRetain)

#if defined(USE_BM_SW)
// USE_BM_SW means always use a bm sw when creating a switch with no skip.
// Default is -DBM_SW_FOR_REAL iff -DUSE_BM_SW.
// Default is -UBM_IN_LINK.
  #undef  CODE_BM_SW
  #define CODE_BM_SW
  #if ! defined(NO_BM_SW_FOR_REAL)
      #define BM_SW_FOR_REAL
  #endif // ! defined(NO_BM_SW_FOR_REAL)

// Default is -URETYPE_FULL_BM_SW.
// There is no need for RETYPE_FULL_BM_SW. Switches will be converted sooner.
// Default is -UBM_IN_NON_BM_SW.
// BM_IN_NON_BM_SW with RETYPE_FULL_BM_SW allows retype to T_SWITCH so
// BmSwitch_t and Switch_t have to be the same.
// But RETYPE_FULL_BM_SW without BM_IN_NON_BM_SW uses T_FULL_BM_SW so
// BmSwitch_t and Switch_t don't have to be the same until the end of sw_awBm
// and after.
#endif // defined(USE_BM_SW)

#if defined(USE_LIST_SW)
// Default is -USW_LIST_IN_LINK.
  #define CODE_LIST_SW
#endif // defined(USE_LIST_SW)

// Default is NO_USE_XX_SW because test runs take so long because of
// insert times.
// USE_XX_SW doesn't work on 32-bit yet.
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

// Default is -DPSPLIT_PARALLEL.
// It causes PSPLIT_SEARCH to use a parallel search.
// It affects the alignment of the list of keys in a list leaf and the
// amount of memory allocated for it and the padding of any unused key slots.
// The size of a parallel search bucket is determined by PARALLEL_128 (or
// PARALLEL_64 for 32-bit).
// PSPLIT_PARALLEL applies to lists of all key sizes except lists which use
// full word size key slots.
//
// For lists of full word size key slots we use PARALLEL_SEARCH_WORD to
// cause PSPLIT_SEARCH_WORD to use a parallel search. PARALLEL_SEARCH_WORD
// also affects the alignment of lists with full word size key slots and the
// padding of any unused key slots.
//
// PSPLIT_SEARCH_BY_KEY(...)
// may be used to avoid a parallel search independent of PSPLIT_PARALLEL.
// Ultimately, we'd like be able to override the default for any attribute of
// the type of search to use for any situation independently. But we're not
// there yet. The ifdef complexity is already horrifying.
#ifndef NO_PSPLIT_PARALLEL
  #undef PSPLIT_PARALLEL
  #define PSPLIT_PARALLEL
#endif // NO_PSPLIT_PARALLEL

// Default is -DPARALLEL_128.
#if !defined(PARALLEL_64) && !defined(NO_PARALLEL_128)
  #undef PARALLEL_128
  #define PARALLEL_128
#endif // !defined(PARALLEL_64) && !defined(NO_PARALLEL_128)

// UA_PARALLEL_128, i.e. unaligned parallel 128, was designed to save memory
// by eliminating the requirement that lists be padded to an integral number
// of 16-byte bucket lengths while preserving our ability to use 128-bit
// parallel searches.
// This proof-of-concept very limited and has not been hardened.
// It is enabled by default if and only if cnBitsPerWord==32 and
// cnBitsMallocMask >= 4.
// And then only lists of 16-bit keys that fit in 12 bytes are made T_LIST_UA.
//
#if defined(PSPLIT_PARALLEL) && defined(PARALLEL_128)
  #ifndef NO_UA_PARALLEL_128
    #if (cnBitsPerWord == 32) && (cnBitsMallocMask >= 4)
      #undef UA_PARALLEL_128
      #define UA_PARALLEL_128
    #endif // (cnBitsPerWord == 32) && (cnBitsMallocMask >= 4)
  #endif // NO_UA_PARALLEL_128
#endif // defined(PSPLIT_PARALLEL) && defined(PARALLEL_128)

#if defined(UA_PARALLEL_128)
  #if (cnBitsMallocMask < 4)
    #error UA_PARALLEL_128 requires MALLOC_ALIGNMENT >= 16
  #endif // (cnBitsMallocMask < 4)
  #if (cnBitsPerWord != 32)
    #error UA_PARALLEL_128 is for (cnBitsPerWord == 32) only
  #endif // (cnBitsPerWord != 32)
  #ifndef PARALLEL_128
    #error UA_PARALLEL_128 requires PARALLEL_128
  #endif // PARALLEL_128
#endif // defined(UA_PARALLEL_128)

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

// Default is -DPSPLIT_EARLY_OUT which is applicable only if PSPLIT_PARALLEL.
#if ! defined(NO_PSPLIT_EARLY_OUT)
#undef  PSPLIT_EARLY_OUT
#define PSPLIT_EARLY_OUT
#endif // ! defined(NO_PSPLIT_EARLY_OUT)

#include <immintrin.h> // __m128i
#if defined(PARALLEL_128)
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

#if defined(CODE_XX_SW)
// Default is -DSKIP_TO_XX_SW.
#if ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#undef SKIP_TO_XX_SW
#define SKIP_TO_XX_SW
#endif // ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#endif // defined(CODE_XX_SW)

#if defined(SKIP_TO_XX_SW)
    #define cbSkipToXxSw  1
    #define IF_SKIP_TO_XX_SW(_expr)  (_expr)
#else // defined(SKIP_TO_XX_SW)
    #define cbSkipToXxSw  0
    #define IF_SKIP_TO_XX_SW(_expr)
#endif // defined(SKIP_TO_XX_SW)

// Default is SKIP_TO_BM_SW if USE_BM_SW and (LVL_IN_SW or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)
  #if defined(USE_BM_SW)
      #if defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
          #undef SKIP_TO_BM_SW
          #define SKIP_TO_BM_SW
      #endif // defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_BM_SW)
#endif // ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)

// Default is SKIP_TO_LIST_SW if USE_LIST_SW and (LVL_IN_SW or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_LIST_SW) && defined(SKIP_LINKS)
  #if defined(USE_LIST_SW)
      #if defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
          #undef SKIP_TO_LIST_SW
          #define SKIP_TO_LIST_SW
      #endif // defined(LVL_IN_SW) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_LIST_SW)
#endif // ! defined(NO_SKIP_TO_LIST_SW) && defined(SKIP_LINKS)

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
  #if ! defined(NO_LIST_POP_IN_PREAMBLE)
      #ifndef OLD_LISTS
#error "Sorry, LIST_POP_IN_PREAMBLE requires OLD_LISTS."
      #endif // OLD_LISTS
    #undef LIST_POP_IN_PREAMBLE
    #define LIST_POP_IN_PREAMBLE
  #endif // ! defined(NO_LIST_POP_IN_PREAMBLE)
#endif // ! defined(POP_IN_WR_HB)

#if defined(POP_IN_WR_HB) && ! defined(OLD_LISTS)
  #error Must have OLD_LISTS with POP_IN_WR_HB.
#endif // defined(POP_IN_WR_HB)

// Choose max list lengths.
// Respect the maximum value implied by the size of the pop count field.

// Default cnListPopCntMax64.
#if (cnBitsPerWord >= 64)
  #if ! defined(cnListPopCntMax64)
    #define cnListPopCntMax64  256
  #endif // ! defined(cnListPopCntMax64)
#endif // (cnBitsPerWord >= 64)

// Default cnListPopCntMax32.
#if ! defined(cnListPopCntMax32)
      #define cnListPopCntMax32  256
#endif // ! defined(cnListPopCntMax32)

// Default cnListPopCntMax16.
#if ! defined(cnListPopCntMax16)
      #define cnListPopCntMax16  256
#endif // ! defined(cnListPopCntMax16)

// Default cnListPopCntMax8.
// An 8-bit bitmap uses only 32-bytes plus malloc overhead.
// Does it make sense to have a list that uses as much or more?
#if ! defined(cnListPopCntMax8)
    #ifdef BITMAP
  #define cnListPopCntMax8  0x10
    #else // BITMAP
  #define cnListPopCntMax8  256 // assert in SearchList8 should be fixed
    #endif // BITMAP
#endif // ! defined(cnListPopCntMax8)

// Default cnListPopCntMaxDl1 is 0x10 for cnBitsInD1 = 8.
#if ! defined(cnListPopCntMaxDl1)
  // I'm confused. Should this be !defined(USE_XX_SW)?
  #if !defined(USE_XX_SW)
        #ifdef B_JUDYL
      #define cnListPopCntMaxDl1  256  // For JudyL turn on.
        #else // B_JUDYL
      #define cnListPopCntMaxDl1  0x10
        #endif // B_JUDYL
  #else // defined(USE_XX_SW)
    #  if (cnBitsInD1 == 7)
      #define cnListPopCntMaxDl1  0x08
    #elif (cnBitsInD1 == 8)
      #define cnListPopCntMaxDl1  0x10
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

#ifndef cnListPopCntMax24
    #define cnListPopCntMax24  cnListPopCntMax32
#endif // cnListPopCntMax24

#ifndef cnListPopCntMax56
    #define cnListPopCntMax56  cnListPopCntMax64
#endif // cnListPopCntMax56

#ifndef cnListPopCntMax48
    #define cnListPopCntMax48  cnListPopCntMax56
#endif // cnListPopCntMax48

#ifndef cnListPopCntMax40
    #define cnListPopCntMax40  cnListPopCntMax48
#endif // cnListPopCntMax40

// If we don't support a bitmap for the last digit then the list must
// be capable of holding all the keys.
#ifndef BITMAP
  #if       cnListPopCntMaxDl1 < (1 << cnBitsInD1)
    #undef  cnListPopCntMaxDl1
    #define cnListPopCntMaxDl1   (1 << cnBitsInD1)
  #endif // cnListPopCntMaxDl1 < (1 << cnBitsInD1)
#endif // BITMAP

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
   MAX(cnListPopCntMax56, \
   MAX(cnListPopCntMax48, \
   MAX(cnListPopCntMax40, \
       MAX(cnListPopCntMax32, \
           MAX(cnListPopCntMax16, \
               MAX(cnListPopCntMax8, 0)))))))
  #else // (cnBitsPerWord >= 64)
#define cwListPopCntMax \
       MAX(cnListPopCntMax32, \
       MAX(cnListPopCntMax24, \
           MAX(cnListPopCntMax16, \
               MAX(cnListPopCntMax8, 0))))
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
    (DIV_UP_X(cnBitsPerWord - cnBitsLeftAtDl3, cnBitsPerDigit) + 3)
#else // (cnBitsInD3 != cnBitsPerDigit)
#define cnDigitsPerWord \
    (DIV_UP_X(cnBitsPerWord - cnBitsInD1 - cnBitsInD2, cnBitsPerDigit) + 2)
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
  #if defined(UA_PARALLEL_128)
    T_LIST_UA, // external list of keys
  #endif // defined(UA_PARALLEL_128)
#endif // (cwListPopCntMax != 0)
#if defined(BITMAP)
    // T_BITMAP may not be needed if it is implied by the level/depth.
    T_BITMAP, // external (not embedded) bitmap leaf
  #if defined(SKIP_TO_BITMAP)
    T_SKIP_TO_BITMAP, // skip to external bitmap leaf
  #endif // defined(SKIP_TO_BITMAP)
#endif // defined(BITMAP)
#if defined(EMBED_KEYS)
    T_EMBEDDED_KEYS, // keys are embedded in the link
#endif // defined(EMBED_KEYS)
#if defined(CODE_LIST_SW)
    T_LIST_SW,
#endif // defined(CODE_LIST_SW)
#if defined(SKIP_TO_LIST_SW)
    T_SKIP_TO_LIST_SW,
#endif // defined(SKIP_TO_LIST_SW)
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

#if defined(CODE_XX_SW)
    #define bnTypeIsXxSw(_nType)  ((_nType) == T_XX_SW)
#else // defined(CODE_XX_SW)
    #define bnTypeIsXxSw(_nType)  0
#endif // defined(CODE_XX_SW)

// Define and optimize nBitsIndexSz_from_nDL, nBitsIndexSz_from_nBL,
// nBL_from_nDL, nBL_from_nDL, et. al. based on ifdef parameters.

// NAX means not at top and not at bottom,
// i.e. nBL != cnBitsPerWord and nBL != cnBitsInD1.

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

    // Rounding up is not free since we don't otherwise need to add a
    // constant before (or after) dividing.
    // But we have to round up when this is used to figure the depth of a skip
    // link from a common prefix which may be a non-integral number of digits.
    #define nDL_from_nBL(_nBL)  (DIV_UP_X((_nBL), cnBitsPerDigit))

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

#else // defined(BPD_TABLE)

  #define nDL_to_nBitsIndexSz(_nDL)  (nBitsIndexSz_from_nDL(_nDL))
  #define nBL_to_nBitsIndexSz(_nBL)  (nBitsIndexSz_from_nBL(_nBL))
  #define nDL_to_nBL(_nDL)           (nBL_from_nDL(_nDL))
  #define nBL_to_nDL(_nBL)           (nDL_from_nBL(_nBL))

#endif // defined(BPD_TABLE)

#define nBL_to_nBW(_nBL)  nBL_to_nBitsIndexSz(_nBL)

#define nDL_to_nBL_NAX(_nDL)          (nBL_from_nDL_NAX(_nDL))
#define nBL_to_nBitsIndexSzNAX(_nBL)  (nBitsIndexSz_from_nBL_NAX(_nBL))
#define nBL_to_nBitsIndexSzNAB(_nBL)  (nBitsIndexSz_from_nBL_NAB(_nBL))
#define nDL_to_nBitsIndexSzNAX(_nDL)  (nBitsIndexSz_from_nDL_NAX(_nDL))
#define nDL_to_nBL_NAT(_nDL)          (nBL_from_nDL_NAT(_nDL))
#define nDL_to_nBitsIndexSzNAT(_nDL)  (nDL_to_nBitsIndexSz(_nDL))

#define nBL_to_nBWNAB(_nBL)  (nBitsIndexSz_from_nBL_NAB(_nBL))

#if defined(RAMMETRICS)
  #define METRICS(x)  (x)
#else // defined(RAMMETRICS)
  #define METRICS(x)
#endif // defined(RAMMETRICS)

#ifdef SEARCHMETRICS
  #define SMETRICS(x)  x
extern Word_t j__SearchPopulation;
extern Word_t j__GetCalls;
extern Word_t j__DirectHits;
extern Word_t j__GetCallsP;
extern Word_t j__GetCallsM;
extern Word_t j__MisComparesP;
extern Word_t j__MisComparesM;
#else // SEARCHMETRICS)
  #define SMETRICS(x)
#endif // SEARCHMETRICS

#if defined(DEBUG)
  #define DBG(x)  x
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

#define set_wr_nType(_wr, _nType) ((_wr) = ((_wr) & ~cnMallocMask) | (_nType))

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
  #if defined(SKIP_TO_LIST_SW)
    case T_SKIP_TO_LIST_SW: Set_nType(pwRoot, T_LIST_SW); break;
  #endif // defined(SKIP_TO_LIST_SW)
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
#define set_wr(_wr, _pwr, _nType) \
        ((_wr) = ((_wr) & ~cwVirtAddrMask) | (Word_t)(_pwr) | (_nType))

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
  ((int)((((_wr) >> nBL_to_nBitsType(_nBL)) \
          & MSK(nBL_to_nBitsPopCntSz(_nBL))) \
      + 1))

#define set_wr_nPopCnt(_wr, _nBL, _nPopCnt) \
    SetBits(&(_wr), nBL_to_nBitsPopCntSz(_nBL), nBL_to_nBitsType(_nBL), \
            (_nPopCnt) - 1)

  #endif // defined(EK_CALC_POP)

static inline int
EmbeddedListPopCntMax(int nBL)
{
    int nBitsOverhead = nBL_to_nBitsType(nBL) + nBL_to_nBitsPopCntSz(nBL);
  #ifdef B_JUDYL
    return nBL <= (cnBitsPerWord - nBitsOverhead);
  #else // B_JUDYL
    return (cnBitsPerWord - nBitsOverhead) / nBL;
  #endif // B_JUDYL
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
#if defined(CODE_LIST_SW)
  #if defined(SKIP_TO_LIST_SW)
    case T_SKIP_TO_LIST_SW:
  #endif // defined(SKIP_TO_LIST_SW)
    case T_LIST_SW:
#endif // defined(CODE_LIST_SW)
#if defined(CODE_BM_SW)
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW:
  #endif // defined(SKIP_TO_BM_SW)
    case T_BM_SW:
#endif // defined(CODE_BM_SW)
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

#if defined(CODE_LIST_SW)
    // Is (_tp) a list switch or skip to one?
  #if defined(SKIP_TO_LIST_SW)
    #define tp_bIsListSw(_tp) \
         (((_tp) == T_LIST_SW) || ((_tp) == T_SKIP_TO_LIST_SW))
  #else // defined(SKIP_TO_LIST_SW)
    #define tp_bIsListSw(_tp)  ((_tp) == T_LIST_SW)
  #endif // defined(SKIP_TO_LIST_SW)
#endif // defined(CODE_LIST_SW)

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
  #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
    switch (nType) {
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
    case T_SKIP_TO_SWITCH:
      #endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
      #if defined(SKIP_TO_LIST_SW)
    case T_SKIP_TO_LIST_SW:
      #endif // defined(SKIP_TO_LIST_SW)
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
  #else // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
    if (nType >= T_SKIP_TO_SWITCH) { return 1;}
  #endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
#endif // defined(SKIP_LINKS)
    return 0;
}

#if defined(BITMAP)
static inline int
tp_bIsBitmap(int nType)
{
    return ((nType == T_BITMAP)
  #ifdef SKIP_TO_BITMAP
         || (nType == T_SKIP_TO_BITMAP)
  #endif // SKIP_TO_BITMAP
            );
}
#endif // defined(BITMAP)

// Bit fields in the upper bits of of wRoot.
// Lvl is the level of the node pointed to.
// Can we use lvl to id skip instead of a bit in the type field?
// XxSwWidth is the width of the switch.
// ListPopCnt is the number of keys in the list.
// ListSwPopM1 is the number of links in the list switch minus one.
// A field at the end is faster to extract than a field in the middle.

#if (cnBitsInD1 == 9) // useful for BM_IN_LINK and no embedded bitmap
  #define cnBitsListPopCnt  9
  #define cnBitsLvl  7
#else // (cnBitsInD1 == 9)
  #define cnBitsListPopCnt  8
  #define cnBitsLvl  8 // 8 is easier to read in debug output than 7
#endif // #else (cnBitsInD1 == 9)

#if defined(SKIP_TO_LIST)
  #define cnLsbListPopCnt  cnBitsVirtAddr
#else // defined(SKIP_TO_LIST)
  #define cnLsbListPopCnt  (cnBitsPerWord - cnBitsListPopCnt)
#endif // defined(SKIP_TO_LIST)

#define cnLsbLvl  (cnBitsPerWord - cnBitsLvl)

#define cnBitsXxSwWidth   6

#if defined(SKIP_TO_XX_SW)
  #define cnLsbXxSwWidth  cnBitsVirtAddr
#else // defined(SKIP_TO_XX_SW)
  #define cnLsbXxSwWidth  (cnBitsPerWord - cnBitsXxSwWidth)
#endif // defined(SKIP_TO_XX_SW)

#define cnBitsListSwPopM1  8 // for T_LIST_SW

#if defined(SKIP_TO_LIST_SW)
    #define cnLsbListSwPopM1  cnBitsVirtAddr
#else // defined(SKIP_TO_LIST)
    #define cnLsbListSwPopM1  (cnBitsPerWord - cnBitsListSwPopM1)
#endif // defined(SKIP_TO_LIST)

#if defined(CODE_XX_SW)
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
// Why not simply use a separate word for LVL_IN_SW instead of the more
// complicated approach of displacing pop from sw_wPrefixPop? We will be
// able to combine it with the word we are planning to add for memory usage
// of the subtree, sw_wMem.
// Is it because we want to be able to put lvl in the PrefixPop word for
// PP_IN_LINK?
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
      ((int)(assert(tp_bIsSkip(wr_nType(_wr))), \
       w_wPopCntBL(PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)), \
                   cnBitsLeftAtDl2)))

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

// #define LVL_IN_TYPE

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

#ifdef SKIP_LINKS
static inline void
Set_nBLR(Word_t *pwRoot, int nBLR)
{
    set_wr_nBL(*pwRoot, nBLR);
}
#endif // SKIP_LINKS

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

// PP_IN_LINK and POP_WORD_IN_LINK don't work without OLD_LISTS.
// The whole purpose of new lists was to move pop count to the end of
// the list so we don't have to waste a bucket at the beginning just for
// pop count.  But we don't put the pop count in the list for PP_IN_LINK
// or POP_WORD_IN_LINK. Except at the top.
// We'll still be wasting that bucket for PP_IN_LINK and POP_WORD_IN_LINK if
// we are aligning word-size lists.  But we're not currently aligning
// word-size lists because we're not currently doing psplit search word.
// We can worry about aligned word size lists later.  It should be easy
// to distinguish the top level list from other word size lists and treat
// them differently.
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #if ! defined(OLD_LISTS)
#error Sorry, PP_IN_LINK and POP_WORD_IN_LINK require OLD_LISTS.
  #endif // ! defined(OLD_LISTS)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

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
    (STRUCT_OF((assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)), (_pwRoot)), \
               Link_t, ln_wRoot)->ln_wPrefixPop)

  #if defined(LVL_IN_SW) || defined(POP_WORD)
// The main purpose of new lists was to move pop count to the end of
// the list so we don't have to waste a bucket at the beginning just for
// pop count. But we don't put the pop count in the list for PP_IN_LINK
// Except at the top.  We're not currently aligning word-size
// lists because we're not currently doing parallel search for lists
// with full word size key slots.
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

  #ifdef POP_WORD_IN_LINK
    #define PWR_wPopWord(_pwRoot, _pwr) \
        (STRUCT_OF((assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)), \
                   (_pwRoot)), \
            Link_t, ln_wRoot)->ln_wPopWord)
    #define set_PWR_wPopWord(_pwRoot, _pwr, _ww) \
        (PWR_wPopWord((_pwRoot), (_pwr)) = (_ww))
    #define PWR_wPopWordBL(_pwRoot, _pwr, _nBL) \
        (assert((_nBL) < cnBitsPerWord), \
            assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)), \
            STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_wPopWord)
    #define PWR_wPopWordDL(_pwRoot, _pwr, _nDL) \
        (assert((_nDL) < cnDigitsPerWord), \
            assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)), \
            STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_wPopWord)
  #else // POP_WORD_IN_LINK
    #define PWR_wPopWord(_pwRoot, _pwr)  (((Switch_t*)(_pwr))->sw_wPopWord)
    #define PWR_wPopWordBL(_pwRoot, _pwr, _nBL) \
        PWR_wPopWord((_pwRoot), (_pwr))
    #define PWR_wPopWordDL(_pwRoot, _pwr, _nDL) \
        PWR_wPopWord((_pwRoot), (_pwr))
  #endif // POP_WORD_IN_LINK

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (w_wPopCnt(PWR_wPopWord((_pwRoot), (_pwr)), (_nDL)))

#define PWR_wPopCntBL(_pwRoot, _pwr, _nBL) \
    (w_wPopCntBL(PWR_wPopWordBL((_pwRoot), (_pwr), (_nBL)), (_nBL)))

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) \
        = ((PWR_wPopWordDL((_pwRoot), (_pwr), (_nDL)) \
                & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) \
        = ((PWR_wPopWordBL((_pwRoot), (_pwr), (_nBL)) \
                & ~wPrefixPopMaskBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskBL(_nBL))))

#else // defined(POP_WORD)

#define PWR_wPopCnt(_pwRoot, _pwr, _nDL) \
    (/*assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)),*/ \
        (w_wPopCnt(PWR_wPrefixPop((_pwRoot), (_pwr)), (_nDL))))

#define PWR_wPopCntBL(_pwRoot, _pwr, _nBL) \
    (w_wPopCntBL(PWR_wPrefixPop( \
        (/*assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)),*/ \
            (_pwRoot)), (_pwr)), (_nBL)))

  #if defined(PP_IN_LINK)

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
  (assert((_nDL) < cnDigitsPerWord), \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL)))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (assert((_nBL) < cnBitsPerWord), \
        /*assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)),*/ \
        (PWR_wPrefixPop((_pwRoot), (_pwr)) \
            = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
                | ((_cnt) & wPrefixPopMaskBL(_nBL)))))

  #else // defined(PP_IN_LINK)

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPrefixPop((_pwRoot), (_pwr)) \
        = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMask(_nDL)) \
            | ((_cnt) & wPrefixPopMask(_nDL))))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (/*assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)),*/ \
        (PWR_wPrefixPop((_pwRoot), (_pwr)) \
            = ((PWR_wPrefixPop((_pwRoot), (_pwr)) & ~wPrefixPopMaskBL(_nBL)) \
                | ((_cnt) & wPrefixPopMaskBL(_nBL)))))

 #endif // defined(PP_IN_LINK)

#endif // defined(POP_WORD)

#if defined(POP_WORD_IN_LINK) && !defined(POP_WORD)
#error Sorry, POP_WORD_IN_LINK requires POP_WORD.
#endif // defined(POP_WORD_IN_LINK) && !defined(POP_WORD)

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
#define     PWR_pwBm(_pwRoot, _pwr)  (&(_pwr)[-N_WORDS_SWITCH_BM])
#endif // defined(BM_IN_LINK)

#if defined(PSPLIT_PARALLEL)
    #define cbPsplitParallel 1
#else // defined(PSPLIT_PARALLEL)
    #define cbPsplitParallel 0
#endif // defined(PSPLIT_PARALLEL)

#if defined(PARALLEL_SEARCH_WORD)
    #define cbParallelSearchWord 1
#else // defined(PARALLEL_SEARCH_WORD)
    #define cbParallelSearchWord 0
#endif // defined(PARALLEL_SEARCH_WORD)

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

#if defined(ALIGN_LIST_LENS)
    #define cbAlignListLens 1
#else // defined(ALIGN_LIST_LENS)
    #define cbAlignListLens 0
#endif // defined(ALIGN_LIST_LENS)

#if defined(LIST_END_MARKERS)
    #define cbListEndMarkers 1
#else // defined(LIST_END_MARKERS)
    #define cbListEndMarkers 0
#endif // defined(LIST_END_MARKERS)

// POP_SLOT tells ListWords if we need a slot in the leaf for a pop count
// that is not included in N_LIST_HDR_KEYS, i.e. a slot that occurs after
// ll_a[csiw]Keys[N_LIST_HDR_KEYS].
// Maybe it should be called N_LIST_FOOTER_KEYS?
// There is a problem if POP_SLOT is at the beginning of the list and
// we're aligning lists. Our code doesn't account for aligning the
// list again after the pop slot.
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    // Be careful: _nBL here is BEFORE any skip is applied
    // Do we allow skip from top for PP_IN_LINK? Looks like we allow
    // skip if prefix is zero. It means we'd need a pop slot.
    // But do we allow/use skip to list from the top?
    #define POP_SLOT(_nBL) \
        (((_nBL) >= cnBitsPerWord) && (cnDummiesInList == 0))
#else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #if defined(OLD_LISTS)
    // N_HDR_KEYS incorporates this for ! PP_IN_LINK so don't add it again.
    #define POP_SLOT(_nBL)  (0)
  #else // defined(OLD_LISTS)
    #define POP_SLOT(_nBL)  (1)
  #endif // defined(OLD_LISTS)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// ALIGN_LIST is about the alignment of the first real key in a list leaf.
// The first real key in a list leaf may follow a pop count and/or an
// end-of-list marker key and/or dummy keys and/or whatever else we decide
// to implement or experiment with.
// One reason for doing this alignment is to satisfy any alignment
// requirement of the list search functions, e.g. the load 128-bit register
// instruction used by gcc requires 128-bit alignment.
#define ALIGN_LIST(_nBytesKeySz) \
    ( cbAlignLists /* independent of psplit parallel */ \
        || ( (_nBytesKeySz) == cnBytesPerWord \
            ? cbParallelSearchWord : cbPsplitParallel ) )

// The length of a list from the first key through the last (including unused
// slots filled with the last real key in the list) must be an integral
// number of parallel search buckets so we don't need any special handling in
// the parallel search code to handle a partial bucket at the end.
// This is independent of any header or footer in the list or any alignment
// of the first key in the list, e.g. if ALIGN_LIST allows a list to start on
// an odd word boundary and ALIGN_LIST_LEN requires the list to be an integral
// number of buckets long, then the end of the list may fall on an odd word
// boundary.
#define ALIGN_LIST_LEN(_nBytesKeySz) \
    ( cbAlignListLens /* independent of psplit parallel */ \
        || ( (_nBytesKeySz) == cnBytesPerWord \
            ? cbParallelSearchWord : cbPsplitParallel ) )

// pop cnt in preamble iff OLD_LISTS && !POP_IN_WR_HB && LIST_POP_IN_PREAMBLE;
// don't care about PP_IN_LINK
// - LIST_POP_IN_PREAMBLE should imply OLD_LISTS and !POP_IN_WR_HB (What do
//   we do with pop field in PP?)
// ListLeaf_t
// - ll_awDummies
//   - last dummy contains pop cnt iff there is a dummy && PP_IN_LINK && at top
//     && OLD_LISTS && !POP_IN_WR_HB && !LIST_POP_IN_PREAMBLE
// - ll_a[csiw]Keys
//   - first slot (or partial slot) contains pop cnt
//     iff OLD_LISTS && !POP_IN_WR_HB && !LIST_POP_IN_PREAMBLE
//     && (!PP_IN_LINK || (at top && there are no dummies))
//   - unused pad slots for alignment if aligning
//   - list end marker = 0 (Aligned whole bucket?) so search neednt test for
//     starting address of list
//   - keys
//   - pad slots filled with biggest key if psplit parallel
//   - list end marker = -1 (Whole bucket?) so search needn't test for length
//     of list
//   - pop cnt in last word iff !OLD_LISTS

#if defined(OLD_LISTS)

// Use ls_sPopCnt in the performance path when we know the keys are bigger
// than one byte.
#define     ls_sPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_asKeys[0])
#define set_ls_sPopCnt(_ls, _cnt)  (ls_sPopCnt(_ls) = (_cnt))

// Use ls_cPopCnt in the performance path when we know the keys are one byte.
// PopCnt fits in a single key slot.
#define     ls_cPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_acKeys[0])
#define set_ls_cPopCnt(_ls, _cnt)  (ls_cPopCnt(_ls) = (_cnt))

  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

      // Number of key slots needed for header info after cnDummiesInList
      // (for nBL != cnBitsPerWord).
      #if defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  1
      #else // defined(LIST_END_MARKERS)
#define N_LIST_HDR_KEYS  0
      #endif // defined(LIST_END_MARKERS)

  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

      // Number of key slots needed for header info after cnDummiesInList.
      #if defined(LIST_END_MARKERS)
          #if defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  1
          #else // defined(POP_IN_WR_HB)
#define N_LIST_HDR_KEYS  2 // one slot needed for pop
          #endif // defined(POP_IN_WR_HB)
      #else // defined(LIST_END_MARKERS)
          #if defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
#define N_LIST_HDR_KEYS  0
          #else // defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
#define N_LIST_HDR_KEYS  1  // one slot needed for pop
          #endif // defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
      #endif // defined(LIST_END_MARKERS)

  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// NAT is relevant only for PP_IN_LINK and POP_WORD_IN_LINK where POP_SLOT
// depends on whether we are at the top or not.
#define ls_pwKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_awKeys[N_LIST_HDR_KEYS])

  #if ALIGN_LIST(cnBytesPerWord)
#define ls_pwKeysNAT(_ls) \
    ((Word_t *)ALIGN_UP((Word_t)ls_pwKeysNAT_UA(_ls), sizeof(Bucket_t)))
  #else // ALIGN_LIST(cnBytesPerWord)
#define ls_pwKeysNAT(_ls)  ls_pwKeysNAT_UA(_ls)
  #endif // ALIGN_LIST(cnBytesPerWord)

  #if defined(COMPRESSED_LISTS)

#define ls_pcKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_acKeys[N_LIST_HDR_KEYS])

#define ls_psKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_asKeys[N_LIST_HDR_KEYS])

      #if (cnBitsPerWord > 32)
#define ls_piKeysNAT_UA(_ls) \
    (&((ListLeaf_t *)(_ls))->ll_aiKeys[N_LIST_HDR_KEYS])
      #endif // (cnBitsPerWord > 32)

      #if ALIGN_LIST(1)
#define ls_pcKeysNAT(_ls) \
    ((uint8_t *)ALIGN_UP((Word_t)ls_pcKeysNAT_UA(_ls), sizeof(Bucket_t)))
      #else // ALIGN_LIST(1)
#define ls_pcKeysNAT(_ls)  ls_pcKeysNAT_UA(_ls)
      #endif // ALIGN_LIST(1)

      #if ALIGN_LIST(2)
#define ls_psKeysNAT(_ls) \
    ((uint16_t *)ALIGN_UP((Word_t)ls_psKeysNAT_UA(_ls), sizeof(Bucket_t)))
      #else // ALIGN_LIST(2)
#define ls_psKeysNAT(_ls)  ls_psKeysNAT_UA(_ls)
      #endif // ALIGN_LIST(2)

      #if (cnBitsPerWord > 32)
          #if ALIGN_LIST(4)
#define ls_piKeysNAT(_ls) \
    ((uint32_t *)ALIGN_UP((Word_t)ls_piKeysNAT_UA(_ls), sizeof(Bucket_t)))
          #else // ALIGN_LIST(4)
#define ls_piKeysNAT(_ls)  ls_piKeysNAT_UA(_ls)
          #endif // ALIGN_LIST(4)
      #endif // (cnBitsPerWord > 32)

  #endif // defined(COMPRESSED_LISTS)

  // ls_pxKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
  // ls_pcKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
  // ls_psKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
  // ls_piKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
  // ls_pwKeys(_ls, _nBL) is valid -- even for PP_IN_LINK at the top
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

      #if defined(COMPRESSED_LISTS)

          #if ALIGN_LIST(1)
#define ls_pcKeys(_ls, _nBL) \
    ((uint8_t *)ALIGN_UP((Word_t)(ls_pcKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                         sizeof(Bucket_t)))
          #else // ALIGN_LIST(1)
#define ls_pcKeys(_ls, _nBL)  (ls_pcKeysNAT_UA(_ls) + POP_SLOT(_nBL))
          #endif // ALIGN_LIST(1)

          #if ALIGN_LIST(2)
#define ls_psKeys(_ls, _nBL) \
    ((uint16_t *)ALIGN_UP((Word_t)(ls_psKeysNAT_UA(_ls) + POP_SLOT(_nBL)),  \
                          sizeof(Bucket_t)))
          #else // ALIGN_LIST(2)
#define ls_psKeys(_ls, _nBL)  (ls_psKeysNAT_UA(_ls) + POP_SLOT(_nBL))
          #endif // ALIGN_LIST(2)

          #if (cnBitsPerWord > 32)
              #if ALIGN_LIST(4)
#define ls_piKeys(_ls, _nBL) \
    ((uint32_t *)ALIGN_UP((Word_t)(ls_piKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                          sizeof(Bucket_t)))
              #else // ALIGN_LIST(4)
#define ls_piKeys(_ls, _nBL)  (ls_piKeysNAT_UA(_ls) + POP_SLOT(_nBL))
              #endif // ALIGN_LIST(4)
          #endif // (cnBitsPerWord > 32)

      #endif // defined(COMPRESSED_LISTS)

      #if ALIGN_LIST(8)
#define ls_pwKeys(_ls, _nBL) \
    ((Word_t *)ALIGN_UP((Word_t)(ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                        sizeof(Bucket_t)))
      #else // ALIGN_LIST(8)
#define ls_pwKeys(_ls, _nBL)  (ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL))
      #endif // ALIGN_LIST(8)

  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#define ls_pwKeys(_ls, _nBL)  ls_pwKeysNAT(_ls)
#define ls_piKeys(_ls, _nBL)  ls_piKeysNAT(_ls)
#define ls_psKeys(_ls, _nBL)  ls_psKeysNAT(_ls)
#define ls_pcKeys(_ls, _nBL)  ls_pcKeysNAT(_ls)

  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#define ls_pcKeysNATX(_pwr, _nPopCnt)  ls_pcKeysNAT(_pwr)
#define ls_psKeysNATX(_pwr, _nPopCnt)  ls_psKeysNAT(_pwr)
#define ls_piKeysNATX(_pwr, _nPopCnt)  ls_piKeysNAT(_pwr)
#define ls_pwKeysNATX(_pwr, _nPopCnt)  ls_pwKeysNAT(_pwr)

#define ls_pcKeysX(_ls, _nBL, _nPopCnt)  ls_pcKeys(_ls, _nBL)
#define ls_psKeysX(_ls, _nBL, _nPopCnt)  ls_psKeys(_ls, _nBL)
#define ls_piKeysX(_ls, _nBL, _nPopCnt)  ls_piKeys(_ls, _nBL)
#define ls_pwKeysX(_ls, _nBL, _nPopCnt)  ls_pwKeys(_ls, _nBL)

#else // defined(OLD_LISTS)

// We want the number of words that will hold our leaf.
// Plus any word(s) that we get free that would allow us to hold another key.
// The code isn't quite there yet.

// We want an odd number of words (for dlmalloc efficiency) that will
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
// What if POP_SLOT is for PP_IN_LINK or POP_WORD_IN_LINK at top and goes at
// the beginning of the list? Wouldn't it need a whole bucket of
// it's own? It seems like we might have a problem with
// PP_IN_LINK or POP_WORD_IN_LINK and ALIGN_LISTS.
#define ls_nSlotsInListGuts(_wPopCnt, _nBL, _nBytesKeySz) \
( \
    ( ! ALIGN_LIST(_nBytesKeySz) \
    ? ( assert(cbAlignLists == cbAlignListLens), \
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
                     + ALIGN_LIST_LEN(_nBytesKeySz) \
                         * ALIGN_UP((_wPopCnt), \
                                    sizeof(Bucket_t) / (_nBytesKeySz)) \
                     + (!ALIGN_LIST_LEN(_nBytesKeySz)) * (_wPopCnt) \
                     + POP_SLOT(_nBL) \
                     + sizeof(Word_t) / (_nBytesKeySz), \
                 2 * sizeof(Word_t) / (_nBytesKeySz)) \
         - sizeof(Word_t) / (_nBytesKeySz) ) ) \
)

#define ls_nSlotsInList(_wPopCnt, _nBL, _nBytesKeySz) \
    MAX(ls_nSlotsInListGuts(_wPopCnt, _nBL, _nBytesKeySz), \
        3U * cnBytesPerWord / (_nBytesKeySz))

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
             /*(int)ls_nSlotsInList(ls_xPopCnt((_pwr), (_nBL)), \
                                    (_nBL), sizeof(uint32_t))),*/ \
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
#define ls_pwKeyX(_ls, _nBL, _nPopCnt, _ii) \
    (ls_pwKeysX(_ls, _nBL, _nPopCnt)[_ii])
#define ls_piKey(_ls, _nBL, _ii)  (ls_piKeys(_ls, _nBL)[_ii])
#define ls_piKeyX(_ls, _nBL, _nPopCnt, _ii) \
    (ls_piKeysX(_ls, _nBL, _nPopCnt)[_ii])
#define ls_psKey(_ls, _nBL, _ii)  (ls_psKeys(_ls, _nBL)[_ii])
#define ls_psKeyX(_ls, _nBL, _nPopCnt, _ii) \
    (ls_psKeysX(_ls, _nBL, _nPopCnt)[_ii])
#define ls_pcKey(_ls, _nBL, _ii)  (ls_pcKeys(_ls, _nBL)[_ii])
#define ls_pcKeyX(_ls, _nBL, _nPopCnt, _ii) \
    (ls_pcKeysX(_ls, _nBL, _nPopCnt)[_ii])

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
// Since we never test without COMPRESSED_LISTS the
// || !PP_IN_LINK (and relationship to POP_WORD_IN_LINK) is suspect.
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

#if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
  #define N_WORDS_SWITCH_BM \
      (int)DIV_UP_X(((Word_t)2 << cnBitsPerDigit), cnBitsPerWord)
#else // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
  #define N_WORDS_SWITCH_BM \
      (int)DIV_UP_X(((Word_t)1 << cnBitsPerDigit), cnBitsPerWord)
#endif // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)

// Default is -UPOP_WORD_IN_LINK.
// It doesn't matter unless POP_WORD is defined.
// POP_WORD is defined automatically if LVL_IN_SW is defined.

typedef struct {
    Word_t ln_wRoot;
#if defined(PP_IN_LINK)
    Word_t ln_wPrefixPop;
#endif // defined(PP_IN_LINK)
#if defined(POP_WORD) && defined(POP_WORD_IN_LINK)
    Word_t ln_wPopWord;
#endif // defined(POP_WORD) && defined(POP_WORD_IN_LINK)
#if defined(BM_IN_LINK)
    Word_t ln_awBm[N_WORDS_SWITCH_BM];
#endif // defined(BM_IN_LINK)
#if (cnDummiesInLink != 0)
    Word_t ln_awDummies[cnDummiesInLink];
#endif // (cnDummiesInLink != 0)
} Link_t;

#define cnLogBitsPerLink  ((int)LOG(sizeof(Link_t)) + cnLogBitsPerByte)

#ifdef ALLOW_EMBEDDED_BITMAP
#define cbEmbeddedBitmap  (cnBitsInD1 <= cnLogBitsPerLink)
#else // ALLOW_EMBEDDED_BITMAP
#define cbEmbeddedBitmap  (0)
#endif // ALLOW_EMBEDDED_BITMAP

// Get the width of the branch in bits.
// nTypeBase is type without skip, if any.
// nBLR includes any skip specified in the qp link.
static inline int
gnBW(qp, int nTypeBase, int nBLR)
{
    qv; (void)nTypeBase; (void)nBLR;
    int nBW;
    if (bnTypeIsXxSw(nTypeBase)) {
        if (cnBitsPerWord == 64) {
            // WIDTH_IN_WR_HB
            nBW = GetBits(wRoot, cnBitsXxSwWidth, cnLsbXxSwWidth);
        } else {
            // use the malloc preamble word
            nBW = GetBits(pwr[-1], cnBitsXxSwWidth, cnLsbXxSwWidth);
        }
    } else {
        nBW = nBL_to_nBW(nBLR);
    }
    return nBW;
}

#define Get_nBW(_pwRoot) \
    gnBW(/* nBL */ 0, STRUCT_OF((_pwRoot), Link_t, ln_wRoot), \
         *(_pwRoot), /* nType */ 0, wr_pwr(*(_pwRoot)), T_XX_SW, 0)

#define pwr_nBW  Get_nBW

// Set the width of the branch in bits.
static inline void
snBW(qp, int nTypeBase, int nBW)
{
    qv; (void)nTypeBase;
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
    if (cnBitsPerWord == 64) {
        // WIDTH_IN_WR_HB
        SetBits(&pLn->ln_wRoot, cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
    } else {
        // use the malloc preamble word
        SetBits(&pwr[-1], cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
    }
}

#define Set_nBW(_pwRoot, _nBW) \
    snBW(/* nBL */ 0, STRUCT_OF((_pwRoot), Link_t, ln_wRoot), \
         *(_pwRoot), /* nType */ 0, wr_pwr(*(_pwRoot)), T_XX_SW, (_nBW))

#define set_pwr_nBW  Set_nBW

#if defined(SW_LIST_IN_LINK)
    #define SW_LIST
#else // defined(SW_LIST_IN_LINK)
    #define SW_LIST  uint8_t sw_aKeys[1<<(sizeof(uint8_t)*8)];
#endif // defined(SW_LIST_IN_LINK)

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

// Fields that all switch types have at the same address as each other.
#define SW_COMMON_HDR \
    SW_PREFIX_POP \
    SW_POP_WORD \
    SW_DUMMIES

// Uncompressed, basic switch.
typedef struct {
    SW_COMMON_HDR
#if defined(USE_BM_SW) && defined(BM_IN_NON_BM_SW)
    SW_BM
#endif // defined(USE_BM_SW) && defined(BM_IN_NON_BM_SW)
    Link_t sw_aLinks[1]; // variable size
} Switch_t;

// List switch.
// A switch with a list of subkeys the specifies the links that are
// present in the switch.
typedef struct {
    SW_COMMON_HDR
    SW_LIST // variable size so sw_aLinks follows this
    // Link_t sw_aLinks[1]; // variable size
} ListSw_t;

#if ! defined(cnDummiesInBmSw)
#define cnDummiesInBmSw  0
#endif // ! defined(cnDummiesInBmSw)

// Bitmap switch.
// Using the same struct as Switch_t allows for minimal work
// to RETYPE_FULL_BM_SW.
typedef Switch_t BmSwitch_t;

#ifdef SKIP_LINKS

// Get the level of the object in number of bits left to decode.
// qp must specify a skip link.
static inline int
gnBLRSkip(qp)
{
    qv;
    assert(tp_bIsSkip(nType));
  #ifdef LVL_IN_WR_HB
    return GetBits(wRoot, cnBitsLvl, cnLsbLvl);
  #elif defined(POP_WORD)
    return wr_nBL(wRoot);
  #else // LVL_IN_WR_HB
    return nDL_to_nBL(tp_to_nDL(nType));
  #endif // LVL_IN_WR_HB
}

#endif // SKIP_LINKS

// Get the level of the object in number of bits left to decode.
static inline int
gnBLR(qp)
{
    qv;
    return
  #ifdef SKIP_LINKS
        tp_bIsSkip(nType) ? gnBLRSkip(qy) :
  #endif // SKIP_LINKS
        nBL;
}

static inline Word_t
gwPrefix(qp)
{
    qv;
    return PWR_wPrefixNATBL(pwRoot, pwr, nBL);
}

#define cnBitsPreListPopCnt cnBitsListPopCnt
#define cnLsbPreListPopCnt (cnBitsPerWord - cnBitsListPopCnt)
#define cnBitsPreListSwPopM1 cnBitsListSwPopM1
#define cnLsbPreListSwPopM1 (cnBitsPerWord - cnBitsListSwPopM1)

static inline Word_t
gwPopCnt(qp, int nBLR)
{
    qv;
    return PWR_wPopCntBL(&pLn->ln_wRoot, pwr, nBLR);
}

static inline Word_t
Get_wPopCntBL(Word_t *pwRoot, int nBL)
{
    return PWR_wPopCntBL(pwRoot, wr_pwr(*pwRoot), nBL);
}

static inline void
swPopCnt(qp, int nBLR, Word_t wPopCnt)
{
    qv;
    set_PWR_wPopCntBL(&pLn->ln_wRoot, pwr, nBLR, wPopCnt);
}

static inline void
Set_wPopCntBL(Word_t *pwRoot, int nBL, Word_t wPopCnt)
{
    set_PWR_wPopCntBL(pwRoot, wr_pwr(*pwRoot), nBL, wPopCnt);
}

static inline int
gnListPopCnt(qp, int nBLR)
{
    qv; (void)nBLR;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBL < cnBitsPerWord) {
        return PWR_wPopCntBL(pwRoot, NULL, nBLR);
    }
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(POP_IN_WR_HB) // 64-bit default
    int nPopCnt = GetBits(wRoot, cnBitsListPopCnt, cnLsbListPopCnt) + 1;
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    int nPopCnt = GetBits(pwr[-1],
                          cnBitsPreListPopCnt, cnLsbPreListPopCnt) + 1;
#elif defined(OLD_LISTS)
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
      #if cnDummiesInList == 0
    int nPopCnt = ((ListLeaf_t *)pwr)->ll_awKeys[0];
      #else // cnDummiesInList == 0
    int nPopCnt = ((ListLeaf_t *)pwr)->ll_awDummies[cnDummiesInList - 1];
      #endif // cnDummiesInList == 0
  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = (nBLR > 8) ? ls_sPopCnt(pwr) : ls_cPopCnt(pwr);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#else // POP_IN_WR_HB ...
    int nPopCnt = (nBLR > 8)
        ? ((uint16_t *)((Word_t *)pwr + 1))[-1]
        : ((uint8_t  *)((Word_t *)pwr + 1))[-1];
#endif // POP_IN_WR_HB ...
    return nPopCnt;
}

static inline int
Get_xListPopCnt(Word_t *pwRoot, int nBL)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot);
    Word_t wRoot = *pwRoot;
    int nType = wr_nType(wRoot);
    Word_t *pwr = wr_pwr(wRoot);
    return gnListPopCnt(qy, /* nBLR */ nBL);
}

// pwr aka ls points to the highest malloc-aligned address in the
// list buffer.  We have to use an aligned address because we use the low
// bits of the pointer as a type field.
// Pop count is in the last pop-size slot in the word pointed to by pwr.
// Other code assumes pop count is not bigger than a single key in the list.
static inline int
ls_xPopCnt(void *pwr, int nBL)
{
    return Get_xListPopCnt((Word_t*)&pwr, nBL);
}

static inline void
Set_xListPopCnt(Word_t *pwRoot, int nBL, int nPopCnt)
{
    (void)nBL;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    // Set_xListPopCnt is valid only at top, i.e. nBL >= cnBitsPerWord,
    // for PP_IN_LINK, and only for T_LIST.
    assert(nBL >= cnBitsPerWord);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    Word_t *pwr = wr_pwr(*pwRoot); (void)pwr;
#if defined(POP_IN_WR_HB) // 64-bit default
    assert(nPopCnt - 1 <= (int)MSK(cnBitsListPopCnt));
    SetBits(pwRoot, cnBitsListPopCnt, cnLsbListPopCnt, nPopCnt - 1);
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    assert(nPopCnt - 1 <= (int)MSK(cnBitsPreListPopCnt));
    SetBits(&pwr[-1], cnBitsPreListPopCnt, cnLsbPreListPopCnt, nPopCnt - 1);
#elif defined(OLD_LISTS)
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
      #if cnDummiesInList == 0
    ((ListLeaf_t *)pwr)->ll_awKeys[0] = nPopCnt;
      #else // cnDummiesInList == 0
    ((ListLeaf_t *)pwr)->ll_awDummies[cnDummiesInList - 1] = nPopCnt;
      #endif // cnDummiesInList == 0
  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBL > 8) {
        ls_sPopCnt(pwr) = nPopCnt;
    } else {
        ls_cPopCnt(pwr) = nPopCnt;
    }
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#else // POP_IN_WR_HB ...
    if (nBL > 8) {
        ((uint16_t *)((Word_t *)pwr + 1))[-1] = nPopCnt;
    } else {
        ((uint8_t  *)((Word_t *)pwr + 1))[-1] = nPopCnt;
    }
#endif // POP_IN_WR_HB ...
}

#if defined(CODE_LIST_SW)

// Get the number of links in a list switch.
static inline int
gnListSwPop(qp)
{
    qv;
#if defined(POP_IN_WR_HB) // 64-bit default
    int nPopCnt = GetBits(wRoot, cnBitsListSwPopM1, cnLsbListSwPopM1) + 1;
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    int nPopCnt
        = GetBits(pwr[-1], cnBitsPreListSwPopM1, cnLsbPreListSwPopM1) + 1;
#else // POP_IN_WR_HB ...
    #error No place for ListSwPop.
    // We'll have to put it at the beginning of the switch, but there is no
    // such accommodation yet.
#endif // POP_IN_WR_HB ...
    return nPopCnt;
}

static inline void
snListSwPop(qp, int nPopCnt)
{
    qv;
#if defined(POP_IN_WR_HB) // 64-bit default
    assert(nPopCnt <= (int)EXP(cnBitsListSwPopM1));
    SetBits(pwRoot, cnBitsListSwPopM1, cnLsbListSwPopM1, nPopCnt - 1);
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
    assert(nPopCnt <= (int)EXP(cnBitsPreListSwPopM1));
    SetBits(&pwr[-1], cnBitsPreListSwPopM1, cnLsbPreListSwPopM1, nPopCnt - 1);
#else // POP_IN_WR_HB ...
    #error No place for ListSwPop.
    // We'll have to put it at the beginning of the switch, but there is no
    // such accommodation yet.
#endif // POP_IN_WR_HB ...
}

#endif // defined(CODE_LIST_SW)

#define     PWR_xListPopCnt(_pwRoot, _pwr, _nBL) \
    (assert(wr_pwr(*(_pwRoot)) == (_pwr)), Get_xListPopCnt((_pwRoot), (_nBL)))

#define set_PWR_xListPopCnt(_pwRoot, _pwr, _nBL, _cnt) \
    (assert(wr_pwr(*(_pwRoot)) == (_pwr)), \
    Set_xListPopCnt((_pwRoot), (_nBL), (_cnt)))

#ifdef B_JUDYL
Word_t* InsertL(int nBL, Link_t *pLn, Word_t wKey);
#define Remove  RemoveL
Word_t CountL(int nBL, Link_t *pLn, Word_t wKey);
#else // B_JUDYL
Status_t Insert1(int nBL, Link_t *pLn, Word_t wKey);
#define Remove  Remove1
Word_t Count1(int nBL, Link_t *pLn, Word_t wKey);
#endif // B_JUDYL

Status_t Next(Word_t *pwRoot, Word_t wKey, int nBL);

#ifdef B_JUDYL
#define InsertGuts  InsertGutsL
#define InsertAtBitmap  InsertAtBitmapL
#define InflateEmbeddedList  InflateEmbeddedListL
#define InsertCleanup  InsertCleanupL
#define RemoveGuts  RemoveGutsL
#define RemoveCleanup  RemoveCleanupL
#define Dump  DumpL
#else // B_JUDYL
#define InsertGuts  InsertGuts1
#define InsertAtBitmap  InsertAtBitmap1
#define InflateEmbeddedList  InflateEmbeddedList1
#define InsertCleanup  InsertCleanup1
#define RemoveGuts  RemoveGuts1
#define RemoveCleanup  RemoveCleanup1
#define Dump  Dump1
#endif // B_JUDYL

#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertGuts(qp, Word_t wKey, int nPos
#if defined(CODE_XX_SW)
         , Link_t *pLnUp
  #if defined(SKIP_TO_XX_SW)
         , int nBLUp
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)
#if defined(B_JUDYL) && defined(EMBED_KEYS)
         , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
           );

Status_t RemoveGuts(qp, Word_t wKey
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                  , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                    );

#if defined(B_JUDYL) && defined(EMBED_KEYS)
Word_t*
#else // defined(B_JUDYL) && defined(EMBED_KEYS)
void
#endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
InsertCleanup(qp, Word_t wKey);

void RemoveCleanup(Word_t wKey, int nBL, int nBLR,
                   Word_t *pwRoot, Word_t wRoot);

#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtBitmap(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot);

//Word_t FreeArrayGuts(Word_t *pwRoot,
//                     Word_t wPrefix, int nBL, int bDump);

#if defined(EMBED_KEYS)
Word_t InflateEmbeddedList(Word_t *pwRoot,
                           Word_t wKey, int nBL, Word_t wRoot
  #ifdef B_JUDYL
                         , Word_t *pwValueUp
  #endif // B_JUDYL
                           );
#endif // defined(EMBED_KEYS)

#if defined(DEBUG)
#ifdef B_JUDYL
#define bHitDebugThreshold  bHitDebugThresholdL
#else // B_JUDYL
#define bHitDebugThreshold  bHitDebugThreshold1
#endif // B_JUDYL
extern int bHitDebugThreshold;
#endif // defined(DEBUG)

//int ListWords(int nPopCnt, int nBL);
//Word_t *NewList(int nPopCnt, int nBL);
//int OldList(Word_t *pwList, int nPopCnt, int nBL, int nType);

#if defined(DEBUG)
void Dump(Word_t *pwRoot, Word_t wPrefix, int nBL);
#endif // defined(DEBUG)

#endif // (cnDigitsPerWord != 1)

#if defined(DEBUG)
#ifdef B_JUDYL
#define pwRootLast  pwRootLastL
#else // B_JUDYL
#define pwRootLast  pwRootLast1
#endif // B_JUDYL
extern Word_t *pwRootLast; // allow dumping of tree when root is not known
#endif // defined(DEBUG)

#ifdef B_JUDYL
#define wPopCntTotal  wPopCntTotalL
#else // B_JUDYL
#define wPopCntTotal  wPopCntTotal1
#endif // B_JUDYL

extern Word_t wPopCntTotal;
#ifdef B_JUDYL
extern int bPopCntTotalIsInvalid;
#endif // B_JUDYL

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
#define InsertRemove1  Remove1
#define InsertRemoveL  RemoveL
      #if defined(RECURSIVE_REMOVE)
#define RECURSIVE
      #endif // defined(RECURSIVE_REMOVE)
  #endif // defined(REMOVE)
#else // defined(LOOKUP) || defined(REMOVE)
#define KeyFound  (Failure)
  #if defined(INSERT)
#define strLookupOrInsertOrRemove  "Insert"
#define DBGX  DBGI
#define InsertRemove1  Insert1
#define InsertRemoveL  InsertL
      #if defined(RECURSIVE_INSERT)
#define RECURSIVE
      #endif // defined(RECURSIVE_INSERT)
  #elif defined(COUNT) // defined(INSERT)
// Count returns the number of keys present in the array up to but
// excluding the key that is passed as a parameter.
#define strLookupOrInsertOrRemove  "Count"
#define DBGX  DBGC
#define InsertRemoveL  CountL
#define InsertRemove1  Count1
  #else // defined(INSERT) elif defined(COUNT)
#define strLookupOrInsertOrRemove  "Next"
#define DBGX  DBGN
#define InsertRemove  Next
  #endif // defined(INSERT)
#endif // defined(LOOKUP) || defined(REMOVE)

#if defined(PARALLEL_128)

#define BUCKET_HAS_KEY HasKey128
#define BUCKET_LOCATE_KEY LocateKey128

#elif defined(PARALLEL_64) // defined(PARALLEL_128)

#define BUCKET_HAS_KEY HasKey64
#define BUCKET_LOCATE_KEY LocateKey64

#else // defined(PARALLEL_128)

#define BUCKET_HAS_KEY WordHasKey

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

// Forward linear search of sub-list (for any size key and with end check).
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

// Forward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The search starts at _pxKeys0[_nPos].
#define SEARCHF(_x_t, _pxKeys0, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_pxKeys0)[(_nPos) + (_nPopCnt) - 1] < (_xKey)) { \
        (_nPos) = ~((_nPos) + (_nPopCnt)); \
    } else { \
        SMETRICS(int nPosStart = (_nPos)); \
        SSEARCHF((_pxKeys0), (_xKey), (_nPos)); \
        /* include end of list compare even if equal */ \
        SMETRICS(j__MisComparesP += ((_nPos) - nPosStart + 1)); \
    } \
}

#endif // defined(TRY_MEMCHR)

// Backward linear search of sub-list (for any size key and with end check).
// _nPopCnt is the number of keys in the whole list minus _nPos.
// The sub-list to search starts at (_nPos + _nPopCnt - 1) and ends at _nPos.
#define SEARCHB(_x_t, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    if ((_xKey) < (_pxKeys)[_nPos]) { \
        (_nPos) ^= -1; \
    } else { \
        (_nPos) += (_nPopCnt) - 1; \
        SMETRICS(int nPosStart = (_nPos)); \
        SSEARCHB((_pxKeys), (_xKey), (_nPos)); \
        /* include end of list compare even if equal */ \
        SMETRICS(j__MisComparesM += (nPosStart - (_nPos) + 1)); \
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

// PSPLIT_NNT - Not Near Top. Far enough to avoid overflow with max pop.
// Use NBPW_MSK because we know _nBL != cnBitsPerWord.
#define PSPLIT_NNT(_nPopCnt, _nBL, _xKey, _nPsplit) \
{ \
    /* make sure we don't overflow */ \
    assert(((_nPopCnt) <= 1) /* LOG is undefined for 0 */ \
        || (_nBL) + LOG((_nPopCnt) - 1) < cnBitsPerWord); \
    (_nPsplit) = (Word_t)((_xKey) & NBPW_MSK(_nBL)) * (_nPopCnt) >> (_nBL); \
}

// PSPLIT_NNB - Not Near Bottom - is for maxPop <= EXP(keyBits - 8).
// Use NZ_MSK because we know _nBL != 0.
#define PSPLIT_NNB(_nPopCnt, _nBL, _xKey, _nPsplit) \
{ \
    /* make sure we don't throw away too many bits */ \
    assert((_nBL) > LOG(_nPopCnt) + 8); \
    /* make sure we don't overflow */ \
    /* assert((_nBL) - 8 + LOG(_nPopCnt) < cnBitsPerWord); */ \
    /* LOG((_nPopCnt - 1) | 1) allows maxpop=256 at top */ \
    assert((_nBL) - 8 + LOG(((_nPopCnt) - 1) | 1) < cnBitsPerWord); \
    /* assert((Word_t)(_nPopCnt) << ((_nBL) - 9) */ \
        /* <= (Word_t)1 << (cnBitsPerWord - 1)); */ \
    (_nPsplit) \
        = (((_xKey) & NZ_MSK(_nBL)) >> 8) * (_nPopCnt) >> ((_nBL) - 8); \
}

// If list may be at top and is not at bottom.
//  slot = pop * (key & MSK(nBL)) >> XX / EXP(nBL - XX)
// If list may be at bottom and is not at top.
//  slot = pop * (key & MSK(nBL)) / EXP(nBL)
// Psplit search doesn't usually make sense at top.
// If key is already masked and list is not at top.
//  slot = pop * key / EXP(nBL)
// If expanse is already calculated and key is already masked
// and list is not at top.
//  slot = pop * key / exp
#define PSPLIT(_nPopCnt, _nBL, _xKey, _nPsplit) \
    PSPLIT_NNT((_nPopCnt), (_nBL), (_xKey), (_nPsplit))

static int
PsplitAny(int nPopCnt, int nBL, Word_t wKey)
{
    int nPos;
    if (nBL <= cnBitsPerWord - 16) {
        PSPLIT_NNT(nPopCnt, nBL, wKey, nPos);
    } else {
        PSPLIT_NNB(nPopCnt, nBL, wKey, nPos);
    }
    return nPos;
}

#define PSPLIT_ANY(_nPopCnt, _nBL, _xKey, _nPsplit) \
{ \
    if ((_nBL) <= cnBitsPerWord - 16) { \
        PSPLIT_NNT((_nPopCnt), (_nBL), (_xKey), (_nPsplit)); \
    } else { \
        PSPLIT_NNB((_nPopCnt), (_nBL), (_xKey), (_nPsplit)); \
    } \
}

#ifdef PSPLIT_AT_TOP
    // PSPLIT_ANY variant throws a compile error for EXP(cnBitsPerWord)
    // which we addressed by creating PsplitAny.
    #define PSPLIT_WORD(_nPopCnt, _nBL, _xKey, _nPsplit) \
        ((_nPsplit) = PsplitAny((_nPopCnt), (_nBL), (_xKey)))
#else // PSPLIT_AT_TOP
    #define PSPLIT_WORD(_nPopCnt, _nBL, _xKey, _nPsplit) \
        PSPLIT_NNB(_nPopCnt, _nBL, _xKey, _nPsplit)
#endif // #else PSPLIT_AT_TOP

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

// This is a non-parallel psplit search that calculates a descriptive _nPos.
// It has to work for small nBL in case of no COMPRESSED_LISTS.
// It doesn't usually have to work for nBL == cnBitsPerWord since psplit
// search is usually a bad choice for that case.
#define PSPLIT_SEARCH_BY_KEY_WORD(_x_t, _nBL, \
                                  _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    int nSplit; PSPLIT_WORD((_nPopCnt), (_nBL), (_xKey), nSplit); \
    /* if (TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)) */\
    if ((_pxKeys)[nSplit] == (_xKey)) \
    { \
        (_nPos) += nSplit; \
        SMETRICS(++j__DirectHits); \
    } \
    else if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        SMETRICS(++j__GetCallsP); \
        SMETRICS(++j__MisComparesP); \
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
            SEARCHF(_x_t, (_pxKeys), (_nPopCnt) - (_nPos), (_xKey), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        SMETRICS(++j__GetCallsM); \
        SMETRICS(++j__MisComparesM); \
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
PsplitSearchByKeyWord(int nBL,
                      Word_t *pwKeys, int nPopCnt, Word_t wKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY_WORD(Word_t, nBL, pwKeys, nPopCnt, wKey, nPos);
    return nPos;
}

// This is a non-parallel psplit search that calculates a descriptive _nPos.
#define PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    int nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    /* if (TEST_AND_SPLIT_EQ_KEY(_pxKeys, _xKey)) */\
    if ((_pxKeys)[nSplit] == (_xKey)) \
    { \
        SMETRICS(++j__DirectHits); \
        (_nPos) += nSplit; \
    } \
    else if ((_pxKeys)[nSplit] < (_xKey)) \
    { \
        SMETRICS(++j__GetCallsP); \
        SMETRICS(++j__MisComparesP); \
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
            SEARCHF(_x_t, (_pxKeys), (_nPopCnt) - (_nPos), (_xKey), (_nPos)); \
        } \
    } \
    else /* here if (_xKey) < (_pxKeys)[nSplit] (and possibly if equal) */ \
    { \
        SMETRICS(++j__GetCallsM); \
        SMETRICS(++j__MisComparesM); \
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

#if (cnBitsPerWord > 32)
static int
PsplitSearchByKey32(uint32_t *piKeys, int nPopCnt, uint32_t iKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY(uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    return nPos;
}
#endif // (cnBitsPerWord > 32)

static int
PsplitSearchByKey16(uint16_t *psKeys, int nPopCnt, uint16_t sKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    return nPos;
}

static int
PsplitSearchByKey8(uint8_t *pcKeys, int nPopCnt, uint8_t cKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY(uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    return nPos;
}

#if (defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)) && ! defined(LIST_END_MARKERS)

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
#define EARLY_OUT(x)  x
#define SMETRICS_OR_EARLY_OUT(x)  x
  #else // defined(PSPLIT_EARLY_OUT)
#define EARLY_OUT(x)
#define SMETRICS_OR_EARLY_OUT  SMETRICS
  #endif // defined(PSPLIT_EARLY_OUT)

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
    _b_t *pbEnd = (_b_t *)&(_pxKeys)[_nPopCnt]; \
    /* address of first bucket to search */ \
    _b_t *pb = (_b_t *)&(_pxKeys)[_nPos]; \
    SMETRICS(int nPosStart = (_nPos)); \
    /* number of last key in first bucket to search */ \
    SMETRICS_OR_EARLY_OUT((_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1); \
    for (;;) { \
        if (BUCKET_HAS_KEY(pb, (_xKey), sizeof(_xKey) * 8)) { \
            SMETRICS(j__MisComparesP \
                += ((_nPos) - nPosStart + 1) * sizeof(_xKey) / sizeof(_b_t)); \
            break; \
        } \
        /* check the last key in the _b_t to see if we've gone too far */ \
        EARLY_OUT(if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; }); \
        SMETRICS_OR_EARLY_OUT((_nPos) += sizeof(_b_t) / sizeof(_xKey)); \
        if (++pb >= pbEnd) { (_nPos) ^= -1; break; } \
    } \
}

#define LOCATEKEYF(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* first address beyond address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[_nPos + _nPopCnt]; \
    /* address of first bucket to search */ \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key in first bucket to search */ \
    /* (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; */ \
    int nBPos; \
    while ((nBPos = BUCKET_LOCATE_KEY(px, (_xKey), sizeof(_xKey) * 8)) < 0) \
    { \
        /* check the last key in the _b_t to see if we've gone too far */ \
        if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; } \
        ++px; (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
        if (px >= pxEnd) { (_nPos) ^= -1; break; } \
    } \
    (_nPos) += nBPos; \
}

#define HASKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *pb = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nbPos = (_nPos) * sizeof(_xKey) / sizeof(_b_t); \
    SMETRICS(int nbPosStart = nbPos); \
    /* number of first key in first bucket to search */ \
    EARLY_OUT((_nPos) = nbPos * sizeof(_b_t) / sizeof(_xKey)); \
    for (;;) { \
        if (BUCKET_HAS_KEY(&pb[nbPos], (_xKey), sizeof(_xKey) * 8)) { \
            SMETRICS(j__MisComparesM += nbPosStart - nbPos + 1); \
            break; \
        } \
        /* check the first key in the _b_t to see if we've gone too far */ \
        EARLY_OUT(if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; }); \
        EARLY_OUT((_nPos) -= sizeof(_b_t) / sizeof(_xKey)); \
        if (&pb[--nbPos] < (_b_t *)(_pxKeys)) { (_nPos) = -1; break; } \
    } \
}

#define LOCATEKEYB(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nxPos = ((_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in first bucket to search */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    int nBPos; \
    while ((nBPos = BUCKET_LOCATE_KEY(&px[nxPos], \
                                      (_xKey), sizeof(_xKey) * 8)) < 0) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
        if (&px[nxPos] < (_b_t *)(_pxKeys)) { (_nPos) = -1; break; } \
    } \
    (_nPos) += nBPos; \
}

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

#if 0
// Do 128-bit parallel has-key independent of sizeof(Bucket_t).
#define HASKEYF_128_UA(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    int nPopCntHere = (_nPopCnt); \
    /* printf("HKF _pxKeys %p _nPopCnt %d nPopCnt %d _nPos %d\n",
        (void *)(_pxKeys), (_nPopCnt), nPopCnt, (_nPos)); */ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* first address beyond address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[(_nPos) + nPopCntHere]; \
    /* address of first bucket to search */ \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key in first bucket to search */ \
    (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; \
    int bHasKey; \
    while ( ! (bHasKey = HasKey128(px, (_xKey), sizeof(_xKey) * 8)) ) { \
        /* check the last key in the _b_t to see if we've gone too far */ \
        if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; } \
        ++px; (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
        if (px >= pxEnd) { (_nPos) ^= -1; break; } \
    } \
    /* printf("a px %p _pxKeys %p _nPopCnt %d nPopCnt %d _nPos %d\n",
           (void *)px, (void *)(_pxKeys), (_nPopCnt), nPopCnt, (_nPos)); */ \
    if (bHasKey \
        && ((px - (_b_t *)&(_pxKeys)[(_nPos)-sizeof(_b_t)/sizeof(_xKey)+1]) \
            == (int)((nPopCntHere - 1) * sizeof(_xKey) / sizeof(__m128i))) \
                /* last bucket */ \
        && ((nPopCntHere - 1) * sizeof(_xKey) / sizeof(Word_t) % 4 != 3) \
            /* incomplete bucket */ \
        && ! HasKey96(px, (_xKey), sizeof(_xKey) * 8)) \
            /* key in garbage word */ \
    { \
        (_nPos) ^= -1; \
    } \
}
#endif // 0

#define HASKEYF_128(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    int nPopCntHere = (_nPopCnt); \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    assert((((_nPos) * sizeof(_xKey)) % sizeof(_b_t)) == 0); \
    /* first address beyond address of last bucket to search */ \
    _b_t *pxEnd = (_b_t *)&(_pxKeys)[(_nPos) + nPopCntHere]; \
    /* address of first bucket to search */ \
    _b_t *px = (_b_t *)&(_pxKeys)[_nPos]; \
    /* number of last key in first bucket to search */ \
    (_nPos) += sizeof(_b_t) / sizeof(_xKey) - 1; \
    while ( ! HasKey128(px, (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the last key in the _b_t to see if we've gone too far */ \
        if ((_xKey) < (_pxKeys)[_nPos]) { (_nPos) ^= -1; break; } \
        ++px; (_nPos) += sizeof(_b_t) / sizeof(_xKey); \
        if (px >= pxEnd) { (_nPos) ^= -1; break; } \
    } \
}

// Do 128-bit parallel has-key independent of sizeof(Bucket_t).
#define HASKEYB_128(_b_t, _xKey, _pxKeys, _nPopCnt, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) % sizeof(_b_t)) == 0); \
    _b_t *px = (_b_t *)(_pxKeys); \
    /* bucket number of first bucket to search */ \
    int nxPos = ((_nPopCnt) - 1) * sizeof(_xKey) / sizeof(_b_t); \
    /* number of first key in first bucket to search */ \
    (_nPos) = nxPos * sizeof(_b_t) / sizeof(_xKey); \
    while ( ! HasKey128(&px[nxPos], (_xKey), sizeof(_xKey) * 8) ) { \
        /* check the first key in the _b_t to see if we've gone too far */ \
        if ((_pxKeys)[_nPos] < (_xKey)) { (_nPos) ^= -1; break; } \
        --nxPos; (_nPos) -= sizeof(_b_t) / sizeof(_xKey); \
        if (&px[nxPos] < (_b_t *)(_pxKeys)) { (_nPos) = -1; break; } \
    } \
}

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

// If psplit slot is in the 1st or 4th quarters of the aligned bucket then
// we want to add or subtract 1/2 bucket if it would not take us
// off the end of the list.
// For Leaf16 with 128-bit buckets we have 8 keys per bucket.
// (slot & ~0x3) == 0 || (slot & ~0x3) == 3
// Add or subtract 0x4.
// For Leaf8 with 128-bit buckets we have 8 keys per bucket.
// (slot & ~0x6) == 0 || (slot & ~0x6) == 6
// Add or subtract 0x8.
// How to trim at front or back of list?
//
// Questions:
// - end of list 16
//   - do we want to exchange the 1st half of the 2nd to last
//     bucket for the 1st half of the last bucket?
//     - list has only one key in new bucket
//       - psplit slot is last key in previous bucket;
//         - exchange gives us one extra key adjacent to the psplit slot
//           and we lose n/2 keys n/2-1 slots away
//       - psplit slot is 2nd to last key in previous bucket;
//         - exchange gives us one extra key one away from the psplit slot
//           and we lose n/2 keys n/2-2 slots away
//       - psplit slot is 3rd to last key in previous bucket;
//         - exchange gives us one extra key two away from the psplit slot
//           and we lose n/2 keys n/2-3 slots away
//     - list has only two keys in new bucket
//       - psplit slot is last key in previous bucket;
//         - exchange gives us two extra keys adjacent to the psplit slot
//           and we lose n/2 keys n/2-1 slots away
//       - psplit slot is 2nd to last key in previous bucket;
//         - exchange gives us two extra keys one awy from the psplit slot
//           and we lose n/2 keys n/2-2 slots away
//       - psplit slot is 3rd to last key in previous bucket;
//     - list has only three keys in new bucket
//       - psplit slot is last key in previous bucket;
//       - psplit slot is 2nd to last key in previous bucket;
//       - psplit slot is 3rd to last key in previous bucket;
//   - would we ever want to give up the half-bucket with the psplit slot
//       - it has only one key
//   - would we ever want to give up the whole bucket with the psplit slot
//       - it has only one key

// Notes on searching lists:
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

// PSPLIT parallel search of a bucket-aligned list of keys to see if a
// key exists in the list.
// It does not locate the position of the key or the slot where
// the key would be.
// It returns a non-negative number if the key is in the list and a
// negative number if it is not.
// A bucket is a Word_t or an __m128i or whatever else we decide to pass
// into _b_t in the future.
// _b_t specifies the type of buckets in the list, e.g. Word_t, __m128i.
// _x_t specifies the type of the keys in the list, e.g. uint8_t, uint16_t.
// _nBL specifies the range of keys, i.e. the size of the expanse.
#if 0
    if ((nSplitP == 1) \
        && BUCKET_HAS_KEY((_b_t*)&_pxKeys[sizeof(_b_t)/sizeof(_x_t)/2], \
                          (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
        SMETRICS(++j__DirectHits); \
    } else \

#endif
#define PSPLIT_HASKEY_GUTS(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    _b_t *px = (_b_t *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    /* nSplit is the key chosen by PSPLIT */ \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    /* nSplitP is nSplit rounded down to the first key in the bucket */ \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(_b_t); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    if (BUCKET_HAS_KEY(&px[nSplitP], (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
        SMETRICS(++j__DirectHits); \
    } else { \
        nSplit = nSplitP * sizeof(_b_t) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we have the value of a key in the list */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(_b_t)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* we are doing a search of the bucket after the original */ \
                /* nSplitP that would be avoidable if */ \
                /* (_xKey) <= pxKeys[nSplit+sizeof(_b_t)/sizeof(_x_t)-1] */ \
                /* and we were willing to do the test */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(_b_t) / sizeof(_x_t); \
                HASKEYF(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
                SMETRICS(++j__GetCallsP); \
            } \
        } else { \
            if (nSplitP == 0) { \
                /* we searched the first bucket and the key is not there */ \
                (_nPos) = -1; /* this is where to insert */ \
            } else { \
                /* parallel search the head of the list */ \
                (_nPos) = nSplit - sizeof(_b_t)/sizeof(_xKey); \
                HASKEYB(_b_t, (_xKey), (_pxKeys), (_nPopCnt), (_nPos)); \
                SMETRICS(++j__GetCallsM); \
            } \
        } \
    } \
    /* everything below is just assertions */ \
    DBG(int nCnt = 0); \
    DBG(for (int i = 0; i < (_nPopCnt); i += sizeof(_b_t) / sizeof(_xKey))) { \
        DBG(nCnt += (BUCKET_HAS_KEY((_b_t*)&(_pxKeys)[i], \
                                (_xKey), sizeof(_x_t) * 8) \
                    != 0)); \
    } \
    assert(nCnt == ((_nPos) >= 0)); \
}

#define PSPLIT_LOCATEKEY_GUTS(_b_t, \
                              _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    _b_t *px = (_b_t *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    /* nSplit is the key chosen by PSPLIT */ \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    /* nSplitP is nSplit rounded down to the first key in the bucket */ \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(_b_t); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    /*int nPos = _nPos;*/ \
    if (((_nPos) = BUCKET_LOCATE_KEY(&px[nSplitP], \
                                     (_xKey), sizeof(_x_t) * 8)) >= 0) { \
        /* keys per bucket * number of buckets */ \
        _nPos += sizeof(_b_t) / sizeof(_x_t) * nSplitP; \
        SMETRICS(++j__DirectHits); \
    } \
    else \
    { \
        nSplit = nSplitP * sizeof(_b_t) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we have the value of a key in the list */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(_b_t)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(_b_t) / sizeof(_x_t); \
                LOCATEKEYF(_b_t, (_xKey), \
                          (_pxKeys), (_nPopCnt) - (_nPos), (_nPos)); \
                SMETRICS(++j__GetCallsP); \
            } \
        } \
        else \
        { \
            if (nSplitP == 0) { \
                /* we searched the first bucket and the key is not there */ \
                (_nPos) = -1; /* this is where to insert */ \
            } else { \
                /* parallel search the head of the list */ \
                LOCATEKEYB(_b_t, (_xKey), (_pxKeys), nSplit, (_nPos)); \
                SMETRICS(++j__GetCallsM); \
            } \
        } \
        /* everything below is just assertions */ \
        if ((_nPos) >= 0) { \
            assert((_nPos) < (_nPopCnt)); \
            assert((_pxKeys)[_nPos] == (_xKey)); \
        } else { \
            for (int ii = 0; ii < (_nPopCnt); \
                 ii += sizeof(_b_t) / sizeof(_xKey)) \
            { \
                assert( ! BUCKET_HAS_KEY((_b_t *)&(_pxKeys)[ii], (_xKey), \
                                         sizeof(_x_t) * 8) ); \
            } \
        } \
    } \
}

#if 0
// Do 128-bit parallel has-key independent of sizeof(Bucket_t).
// Try to get it working with an incomplete final bucket.
// For 32-bit With MALLOC_ALIGNMENT=16 the size of our list buffers modulo
// 128 bits is always three words.
// The only time we have a 128-bit aligned rear end is when the last key
// in the list is in the last word of the 128 bits.
// We could use type values to distinguish this aligned rear end case from
// the other cases.
// How do we handle the other cases?
// Use HasKey96.
#define PSPLIT_HASKEY_GUTS_128_UA(_x_t, \
                                  _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    __m128i *px = (__m128i *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(__m128i); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(__m128i))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    if (HasKey96(&px[nSplitP], (_xKey), sizeof(_x_t) * 8) \
        /* check if nSplitP is the last bucket */ \
        || (((nSplitP != ((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(__m128i)) \
                || (((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(Word_t) % 4 \
                       == 3)) \
            && HasKey128(&px[nSplitP], (_xKey), sizeof(_x_t) * 8))) \
    { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
    } \
    else /* key could be in final word */ \
    { \
        nSplit = nSplitP * sizeof(__m128i) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we have the value of a key in the list */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t)/sizeof(__m128i)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(__m128i) / sizeof(_x_t); \
                HASKEYF_128_UA(__m128i, (_xKey), \
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
                HASKEYB_128(__m128i, (_xKey), (_pxKeys), nSplit, (_nPos)); \
            } \
        } \
        assert(((_nPos) < 0) \
            || HasKey128((__m128i *) \
                                  ((Word_t)&(_pxKeys)[_nPos] \
                                      & ~MSK(LOG(sizeof(__m128i)))), \
                              (_xKey), sizeof(_x_t) * 8)); \
    } \
}
#endif // 0

#define PSPLIT_HASKEY_GUTS_128_96(_x_t, \
                                  _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    if (HasKey96((__m128i *)(_pxKeys), (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists */ \
    } else { \
        (_nPos) = -1; /* key does not exist */ \
    } \
}

#define PSPLIT_HASKEY_GUTS_128(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    __m128i *px = (__m128i *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    unsigned nSplit; PSPLIT((_nPopCnt), (_nBL), (_xKey), nSplit); \
    unsigned nSplitP = nSplit * sizeof(_x_t) / sizeof(__m128i); \
    assert(((nSplit * sizeof(_x_t)) >> LOG(sizeof(__m128i))) == nSplitP); \
    /*__m128i xLsbs, xMsbs, xKeys;*/ \
    /*HAS_KEY_128_SETUP((_xKey), sizeof(_x_t) * 8, xLsbs, xMsbs, xKeys);*/ \
    if (HasKey128(&px[nSplitP], (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists, but we don't know the exact position */ \
    } \
    else /* key could be in final word */ \
    { \
        nSplit = nSplitP * sizeof(__m128i) / sizeof(_x_t); \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we have the value of a key in the list */ \
        if ((_xKey) > xKeySplit) \
        { \
            if (nSplitP == ((_nPopCnt) - 1) * sizeof(_x_t)/sizeof(__m128i)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* ++nSplitP; */ \
                (_nPos) = (int)nSplit + sizeof(__m128i) / sizeof(_x_t); \
                HASKEYF_128(__m128i, (_xKey), \
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
                HASKEYB_128(__m128i, (_xKey), (_pxKeys), nSplit, (_nPos)); \
            } \
        } \
        assert(((_nPos) < 0) \
            || HasKey128((__m128i *) \
                                  ((Word_t)&(_pxKeys)[_nPos] \
                                      & ~MSK(LOG(sizeof(__m128i)))), \
                              (_xKey), sizeof(_x_t) * 8)); \
    } \
}

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

#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH_BY_KEY(Bucket_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#else // (defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)) && ...

#define PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#define PSPLIT_HASKEY_GUTS(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#define PSPLIT_LOCATEKEY_GUTS(_b_t, \
                              _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#define PSPLIT_HASKEY_GUTS_128(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
    PSPLIT_SEARCH(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)

#endif // #else (defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)) ...

#if defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)
#if ! defined(LIST_END_MARKERS)

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

#endif // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)
#endif // defined(COMPRESSED_LISTS)

#if cnBitsPerWord == 64
    #define MM_SET1_EPW(_ww)  _mm_set1_epi64((__m64)(_ww))
#else // cnBitsPerWord == 64
    #define MM_SET1_EPW(_ww)  _mm_set1_epi32((_ww))
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

// The word with the biggest address may contain garbage.
// We don't want to get a false match based on the garbage.
#define HAS_KEY_96_SETUP(_wKey, _nBL, _xLsbs, _xMsbs, _xKeys) \
{ \
    Word_t wMask = MSK(_nBL); /* (1 << nBL) - 1 */ \
    _wKey &= wMask; /* zero insignificant high bits of _wKey */ \
    Word_t wLsbs = (Word_t)-1 / wMask; /* only lsb of each slot is set */ \
    Word_t wKeys = wKey * wLsbs; /* replicate key into every slot in wKeys */ \
    _xKeys = MM_SET1_EPW(wKeys); /* replicate wKeys into _xKeys */ \
    _xLsbs = MM_SET1_EPW(wLsbs); /* replicate wLsbs into _xLsbs */ \
    Word_t wMsbs = wLsbs << (nBL - 1); /* only msb of each slot is set  */ \
    _xMsbs = _mm_set_epi32(0, wMsbs, wMsbs, wMsbs); \
}

//
// Notes on the parallel search of a bucket algorithm.
//
// Consider the case of no matching slots.
// For each slot:
// Since slot does not match, then (Key^Slot) is not zero.
// So subtracting one from (Key^Slot) does not borrow from the next slot.
// Also, subtracting one from (Key^Slot) doesn't set an otherwise clear msb.
// If the msb is set in (Key^Slot), then the msb is not set in ~(Key^Slot).
// So anding the difference with ~(Key^Slot) yields a result with msb clear.
// Then anding that result with a value that has only the msb set yields zero.
//
// QED
//
// Now consider the case of at least one matching slot.
// (Keys^Slots) will be zero in each of the matching slots.
// For all slots less signficant than the least significant matching slot the
// result is the same as for the no matching slots case considered above.
// Starting with the least significant matching slot we classify the
// possibilities for the next more significant slot into four cases.
// 1.  (Key&1) && (Slot == Key-1); diff is -1 and borrows from next slot.
// 2. !(Key&1) && (Slot == Key+1); diff is -1 and borrows from next slot.
// 3.             (Slot == Key  ); diff is -2 and borrows from next slot.
// 4. Everything else; does not borrow from the next slot.
// Then for cases 1-3 we have the same four possibilities for the next more
// significant slot.
// And so on.
//
// In the end, the least significant slot with its msb set represents
// (Slot == Key) and more significant slots with their msb set represent
// (Slot == Key) or (Key&1) && (Slot == Key-1) or !(Key&1) && (Slot == Key+1).
//
// In other words.
// In the end, the least significant slot with its msb set represents
// (Slot == Key) and more significant slots with their msb set represent
// (Slot == Key) or (Slot^Key == 1).
//
// In other words.
// In the end, the least significant slot with its msb set represents
// (Slot == Key) and more significant slots with their msb set represent
// ((Slot^Key) <= 1)
//
// In other words.
// In the end, slots with their msb set represent ((Slot^Key) <= 1).
// And the least significant slot with its msb set represents (Slot == Key).
//
// QED
//
// Can we say anything else?
//
// What about matching slots other than the least significant matching one?
//
// Any slot with its msb set and the adjacent less significant slot with its
// msb clear represents (Slot == Key).
//
// Any slot with its msb set represents (abs(Slot-Key) == 1) and
// the adjacent less significant slot has its msb set) or ((Slot == Key)
// and the adjacent less significant slot has its msb clear).
//
// There must be a more elegant way to summarize.
//


static __m128i
HasKey128MagicTail(__m128i *pxBucket,
    __m128i xLsbs,
    __m128i xMsbs,
    __m128i xKeys)
{
    __m128i xBucket = *pxBucket;
    __m128i xXor = xKeys ^ xBucket; // zero slots with matching keys
    //__m128i xMagic = (xXor - xLsbs) & ~xXor & xMsbs;
    __m128i xMagic = (xXor - xLsbs); // -1 in least significant matching slot
    xMagic &= ~xXor; // clear msbs in less significant unmatching slots
    xMagic &= xMsbs; // clear other bits in less significant unmatching slots
    return xMagic;
}

static Word_t // bool
HasKey128Tail(__m128i *pxBucket,
    __m128i xLsbs,
    __m128i xMsbs,
    __m128i xKeys)
{
    __m128i xMagic = HasKey128MagicTail(pxBucket, xLsbs, xMsbs, xKeys);
    // Can we use _mm_packs_epi32 here?
    return _mm_movemask_epi8(xMagic);
#if 0 // the old way; before LocateKey used HasKey
    __m128i xZero = _mm_setzero_si128(); // get zero for compare
    return ! _mm_testc_si128(xZero, xMagic); // compare with zero
#endif
}

// v_t is a vector of 16 chars. __m128i is a vector of 2 long longs.
// We need the char variant so we can compare with a char using '==' or '>='.
#ifdef WORD_ALIGNED_VECTORS
  #ifdef __clang__
typedef unsigned char  __attribute__((ext_vector_type(16), aligned(8))) v_t;
typedef unsigned short __attribute__((ext_vector_type( 8), aligned(8))) v41_t;
typedef unsigned int   __attribute__((ext_vector_type( 4), aligned(8))) v42_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(16), aligned(8))) v_t;
typedef unsigned short __attribute__((vector_size(16), aligned(8))) v41_t;
typedef unsigned int   __attribute__((vector_size(16), aligned(8))) v42_t;
  #endif // __clang__
  #ifdef __clang__
typedef unsigned char  __attribute__((ext_vector_type(8), aligned(8))) v30_t;
typedef unsigned short __attribute__((ext_vector_type(4), aligned(8))) v31_t;
typedef unsigned int   __attribute__((ext_vector_type(2), aligned(8))) v32_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(8), aligned(8))) v30_t;
typedef unsigned short __attribute__((vector_size(8), aligned(8))) v31_t;
typedef unsigned int   __attribute__((vector_size(8), aligned(8))) v32_t;
  #endif // __clang__
#else // WORD_ALIGNED_VECTORS
  #ifdef __clang__
// clang has some support for gcc attribute "vector_size" but it doesn't work
// as well as its own ext_vector_type.
// For example, it won't promote a scalar to a vector for compare.
typedef unsigned char  __attribute__((ext_vector_type(16))) v_t;
typedef unsigned short __attribute__((ext_vector_type(8))) v41_t;
typedef unsigned int   __attribute__((ext_vector_type(4))) v42_t;
  #else // __clang__
// gcc has no support for clang attribute "ext_vector_type".
typedef unsigned char  __attribute__((vector_size(16))) v_t;
//typedef unsigned char __attribute__((vector_size(16), aligned(4))) v_t;
//typedef unsigned char
//  __attribute__((vector_size(16), aligned(1), __may_alias__)) v_t;
typedef unsigned short __attribute__((vector_size(16))) v41_t;
typedef unsigned int   __attribute__((vector_size(16))) v42_t;
  #endif // __clang__
// vBK_t: vector of 2^B bytes of 2^K-byte elements.
// v64c_t, v64uc_t: vector of 64 bits of char  or unsigned char.
// v64s_t, v64us_t, vector of 64 bits of short or unsigned short.
// v64i_t, v64ui_t, vector of 64 bits of int   or unsigned int.
  #ifdef __clang__
typedef unsigned char  __attribute__((ext_vector_type(8))) v30_t;
typedef unsigned short __attribute__((ext_vector_type(4))) v31_t;
typedef unsigned int   __attribute__((ext_vector_type(2))) v32_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(8))) v30_t;
typedef unsigned short __attribute__((vector_size(8))) v31_t;
typedef unsigned int   __attribute__((vector_size(8))) v32_t;
  #endif // __clang__
#endif // WORD_ALIGNED_VECTORS

#if (cnBitsPerWord < 64)
#undef HK_MOVEMASK
#define HK_MOVEMASK
#endif // (cnBitsPerWord < 64)

// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.
static Word_t // bool
HasKey128(__m128i *pxBucket, Word_t wKey, int nBL)
{
#ifndef OLD_HK_128
    if (nBL == 16) {
        v41_t vEq = (v41_t)(*(v41_t*)pxBucket == (unsigned short)wKey);
  #ifdef HK_MOVEMASK
        return _mm_movemask_epi8((__m128i)vEq);
  #else // HK_MOVEMASK
        // seems marginally faster at startup (in cache)
        return _mm_packs_epi16((__m128i)vEq, (__m128i)vEq)[0];
  #endif // HK_MOVEMASK
    }
    if (nBL == 8) {
        v_t vEq = (v_t)(*(v_t*)pxBucket == (unsigned char)wKey);
        return _mm_movemask_epi8((__m128i)vEq);
    }
    if (nBL <= 32) {
        assert(nBL == 32);
        v42_t vEq = (v42_t)(*(v42_t*)pxBucket == (unsigned int)wKey);
  #ifdef HK_MOVEMASK
        return _mm_movemask_epi8((__m128i)vEq);
  #else // HK_MOVEMASK
        return _mm_packs_epi32((__m128i)vEq, (__m128i)vEq)[0];
  #endif // HK_MOVEMASK
    }
    assert(nBL == 64);
#endif // ifndef OLD_HK_128
    // this appears to be a little slower out of the cache
#if 0
    if (nBL == 16) {
        __m128i xLsbs, xMsbs, xKeys;
        HAS_KEY_128_SETUP(wKey, 16, xLsbs, xMsbs, xKeys);
        return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
    }
    if (nBL == 32) {
        __m128i xLsbs, xMsbs, xKeys;
        HAS_KEY_128_SETUP(wKey, 32, xLsbs, xMsbs, xKeys);
        return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
    }
#endif
    __m128i xLsbs, xMsbs, xKeys;
    if (nBL == cnBitsPerWord) {
        xLsbs = MM_SET1_EPW((Word_t)1);
        xMsbs = MM_SET1_EPW((Word_t)1 << (cnBitsPerWord - 1));
        xKeys = MM_SET1_EPW(wKey);
    } else {
        HAS_KEY_128_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys);
    }
    return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
}

static int
LocateKey128(__m128i *pxBucket, Word_t wKey, int nBL)
{
    Word_t wHasKey = HasKey128(pxBucket, wKey, nBL);
    if (nBL == 8) {
        return __builtin_ffsll(wHasKey) - 1;
    }
    if (wHasKey == 0) {
        return -1;
    }
    int nFirstSetBit = __builtin_ffsll(wHasKey) - 1;
    if (nBL == 16) {
  #ifdef HK_MOVEMASK
        return nFirstSetBit / 2;
  #else // HK_MOVEMASK
        return nFirstSetBit / 8;
  #endif // HK_MOVEMASK
    }
    if (nBL <= 32) {
        assert(nBL == 32);
  #ifdef HK_MOVEMASK
        return nFirstSetBit / 4;
  #else // HK_MOVEMASK
        return nFirstSetBit / 16;
  #endif // HK_MOVEMASK
    }
    assert(nBL == 64);
    return nFirstSetBit / 8;
}

#ifdef HK40_EXPERIMENT // HasKey128 is 1st; DS1 3rd
// HasKey returns non-zero if sorted full Bucket
// has Key or zero if Bucket does not have Key.
// Keys are sorted with lowest key at vector index zero.
static inline int
HasKey40(void *pvBucket, unsigned char Key)
{
    // PSPLIT_BY_KEY is missing from the options here.
#if defined(HK40_NO_MM) // 2nd; DS1 5th slowest
    // Use v4i[0] | v4i[1] != 0 instead of movemask.
    // Movemask is faster than (v4i[0] | v4i[1]) != 0.
    v_t vEq = (v_t)(*(v_t*)pvBucket == Key);
    return (((int64_t*)&vEq)[0] | ((int64_t*)&vEq)[1]) != 0;
#elif defined(HK40_EQ_OUT) // 3rd; DS1 1st (always hitting first key?)
    for (int i = 0; i < 16; i++) {
        if (((unsigned char*)pvBucket)[i] == Key) { return 1; }
    }
    return 0;
#elif defined(HK40_GE_OUT) // 4th; DS1 2nd (always hitting first key?)
    unsigned char uc;
    for (int i = 0; i < 16; i++) {
        if ((uc = ((unsigned char*)pvBucket)[i]) >= Key) {
            return (uc == Key);
        }
    }
    return 0;
#elif defined(HK40_128_HAS_KEY) // 2nd; DS1 4th slowest
    unsigned __int128 Lsbs = (__int128)-1; Lsbs /= 0xff;
    unsigned __int128 Msbs = Lsbs << 7;
    unsigned __int128 Keys = Key * Lsbs;
    unsigned __int128 Xor = Keys ^ *(unsigned __int128*)pvBucket;
    return ((Xor - Lsbs) & ~Xor & Msbs) != 0;
#elif defined(HK40_LL_HAS_KEY) // 2nd; DS1 4th
    return (WordHasKey(&((Word_t*)pvBucket)[0], Key, 8)
          | WordHasKey(&((Word_t*)pvBucket)[1], Key, 8)) != 0;
#elif defined(HK40_LL_MAGIC) // 2nd; DS1 4th
    // Literals are no faster than caclulating with WordHasKey.
    uint64_t Lsbs = 0x0101010101010101ULL;
    uint64_t Msbs = 0x1010101010101010ULL;
    uint64_t Keys = Key * Lsbs;
    uint64_t Xor = Keys ^ ((uint64_t*)pvBucket)[0];
    uint64_t wMagic = (Xor - Lsbs) & ~Xor & Msbs;
    Xor = Keys ^ ((uint64_t*)pvBucket)[1];
    return (wMagic | ((Xor - Lsbs) & ~Xor & Msbs)) != 0;
#elif defined(HK40_LL_SUM_CHAR_ARRAY) // slowest; DS1 3rd slowest
    // Pulling chars from words by pointer casting is slow.
    uint64_t abHk[2];
    for (int i = 0; i < 16; i++) {
        ((unsigned char*)abHk)[i] = (((unsigned char*)pvBucket)[i] == Key);
    }
    return (abHk[0] | abHk[1]) != 0;
#elif defined(HK40_BUCKET_SUM) // slowest; DS1 2nd slowest
    // Big load is not helpful.
    v_t Bucket = *(v_t*)pvBucket;
    int bHk = 0;
    for (int i = 0; i < 16; i++) {
        bHk |= (Bucket[i] == Key);
    }
    return bHk;
#else // HK40_EQ_OUT, HK40_GE_OUT; 2nd slowest; DS1 2nd slowest
    // Individual compares and summing results is terrible.
    int bHk = 0;
    for (int i = 0; i < 16; i++) {
        bHk |= (((unsigned char*)pvBucket)[i] == Key);
    }
    return bHk;
#endif // HK40_...
}
#endif // HK40_EXPERIMENT

static Word_t // bool
HasKey96(__m128i *pxBucket, Word_t wKey, int nBL)
{
    __m128i xLsbs, xMsbs, xKeys;
    HAS_KEY_96_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys);
    return HasKey128Tail(pxBucket, xLsbs, xMsbs, xKeys);
}

static uint64_t
HasKey64(uint64_t *px, Word_t wKey, int nBL)
{
#ifndef OLD_HK_64
    if (nBL == 16) {
#ifdef __clang__
        v31_t vBucket = *px;
        v31_t v31 = (v31_t)(vBucket == (unsigned short)wKey);
        return *(uint64_t*)&v31;
#else // __clang__
        return (uint64_t)(*(v31_t*)px == (unsigned short)wKey);
#endif // __clang__

    }
    if (nBL == 8) {
#ifdef __clang__
        v30_t vBucket = *px;
        v30_t v30 = (v30_t)(vBucket == (unsigned char)wKey);
        return *(uint64_t*)&v30;
#else // __clang__
        return (uint64_t)(*(v30_t*)px == (unsigned char)wKey);
#endif // __clang__
    }
    if (nBL <= 32) {
        assert(nBL == 32);
#ifdef __clang__
        v32_t vBucket = *px;
        v32_t v32 = (v32_t)(vBucket == (unsigned char)wKey);
        return *(uint64_t*)&v32;
#else // __clang__
        return (uint64_t)(*(v32_t*)px == (unsigned int)wKey);
#endif // __clang__
    }
    assert(nBL == 64);
#endif // ifndef OLD_HK_64
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

static int
LocateKey64(uint64_t *px, Word_t wKey, int nBL)
{
    Word_t wHasKey = HasKey64(px, wKey, nBL);
    if (wHasKey == 0) {
        return -1;
    }
    int nFirstSetBit = __builtin_ffsll(wHasKey) - 1;
    return nFirstSetBit / nBL;
}

static __m128i
HasKey128Magic(__m128i *pxBucket, Word_t wKey, int nBL)
{
    __m128i xLsbs, xMsbs, xKeys;
    HAS_KEY_128_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys);
    return HasKey128MagicTail(pxBucket, xLsbs, xMsbs, xKeys);
}

#endif // ! defined(LIST_END_MARKERS)
#endif // defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)

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
SearchList8(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBL <= 8);
    // sizeof(__m128i) == 16 bytes
  #if defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL) \
          && defined(PARALLEL_128) && (cnListPopCntMax8 <= 16)
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, NULL, nBL);
      #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    // By simply setting nPopCnt = 16 here we are assuming, while not
    // ensuring, that pop count never exceeds 16 here.
    // We do it because reading the pop count is so much slower.
    assert(gnListPopCnt(qy, 8) <= 16);
    int nPopCnt = PWR_xListPopCnt(&wRoot, pwr, 8);
    //int nPopCnt = 16; // Sixteen fit so why do less?
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #else // defined(PSPLIT_SEARCH_8) && ...
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, NULL, nBL);
      #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_xListPopCnt(&wRoot, pwr, 8);
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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
    SMETRICS(++j__GetCallsM);
#else // here for forward linear search with end check
    SEARCHF(uint8_t, pcKeys, nPopCnt, cKey, nPos); (void)nBL;
    SMETRICS(++j__GetCallsP);
#endif // ...
    return nPos;
}

static int
ListHasKey8(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

// HasKey128 assumes the list of keys starts at a 128-bit aligned address.
// SearchList8 makes no such assumption.
#if !defined(POP_IN_WR_HB) && !defined(LIST_POP_IN_PREAMBLE)
// Should these ifdefs say (cnDummiesInList != 0)?
#if !defined(PP_IN_LINK) || (cnDummiesInList == 0)
#if !defined(POP_WORD_IN_LINK) || (cnDummiesInList == 0)
#if defined(OLD_LISTS)
    return SearchList8(qy, nBLR, wKey) >= 0;
#endif // defined(OLD_LISTS)
#endif // !defined(POP_WORD_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(PP_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(POP_IN_WR_HB) && !defined(LIST_POP_IN_PREAMBLE)

#if defined(PSPLIT_SEARCH_8)
#if defined(PSPLIT_PARALLEL)

#if defined(PARALLEL_128)
#if cnBitsInD1 == 8
#if cnListPopCntMaxDl1 == 16
#if cnBitsMallocMask >= 4
#if cnDummiesInList == 0
  // ls_pcKeys is valid only at the top for pop in link.
  // Hence it's not really necessary to ifdef out these assertions at the top,
  // but making the exception is more work than I want to do right now.
  #if !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
  #if defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
    assert(ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8)) == (uint8_t*)pwr);
    assert(PWR_xListPopCnt(&wRoot, pwr, 8) <= 16);
  #endif // defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
  #endif // !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
    assert(((Word_t)pwr & ~((Word_t)-1 << 4)) == 0);
  #if defined(OLD_LISTS) && defined(HK40_EXPERIMENT)
    return HasKey40(pwr, wKey);
  #else // defined(OLD_LISTS) && defined(HK40_EXPERIMENT)
      #ifdef OLD_LISTS // includes PP_IN_LINK and POP_WORD_IN_LINK
    return HasKey128((__m128i*)pwr, wKey, 8);
      #else // OLD_LISTS
    return HasKey128((__m128i*)ls_pcKeysNATX(pwr, 16), wKey, 8);
      #endif // OLD_LISTS
  #endif // HK40_EXPERIMENT
#endif // cnDummiesInList == 0
#endif // cnBitsMallocMask >= 4
#endif // cnListPopCntMaxDl1 == 16
#endif // cnBitsInD1 == 8
#endif // defined(PARALLEL_128)

    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint8_t *pcKeys = ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8));
    uint8_t cKey = (uint8_t)wKey;
    int nPos = 0;
    PSPLIT_HASKEY_GUTS(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    return nPos >= 0;

#endif // defined(PSPLIT_PARALLEL)
#endif // defined(PSPLIT_SEARCH_8)

    return SearchList8(qy, nBLR, wKey) >= 0;
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
SearchList16(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBLR >   8);
    assert(nBLR <= 16);
  #if 1
    int nPopCnt = gnListPopCnt(qy, nBLR);
  #else
      #if (cnBitsLeftAtDl2 <= 16)
      #if /* defined(PSPLIT_SEARCH_16) && */ defined(PSPLIT_PARALLEL) \
              && defined(PARALLEL_128) && !defined(INSERT)
          // sizeof(__m128i) == 16 bytes
          #if ! defined(cnListPopCntMaxDl2) || (cnListPopCntMaxDl2 <= 8)
          #if (cnListPopCntMax16 <= 8)
    assert(PWR_xListPopCnt(pwRoot, nBLR) <= 8);
    int nPopCnt = 8; // Eight fit so why do less?
    assert((cnListPopCntMaxDl1 <= 8) || (cnBitsInD1 <= 8));
          #elif (cnBitsInD1 > 8) // nDL == 1 is handled here
              #if (cnListPopCntMaxDl1 <= 8) // list fits in one __m128i
                  #if (cnBitsLeftAtDl2 <= 16) // need to test nDL
    int nPopCnt = (nBLR == cnBitsInD1) ? 8 : PWR_xListPopCnt(pwRoot, 16);
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
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("SearchList16 nPopCnt %d psKeys %p\n",
                nPopCnt, (void *)psKeys));

  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16)
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 16) {
        PSPLIT_SEARCH_BY_KEY(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        //nPos = PSplitSearch16(nBLR, psKeys, nPopCnt, sKey, nPos);
        PSPLIT_SEARCH_BY_KEY(uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16) // defined(PSPLIT_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsM);
  #else // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsP);
  #endif // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    return nPos;
}

#if 0
static int
ListHasKey16Ua(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

    assert(nBL >   8);
    assert(nBL <= 16);
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 16);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    assert(nPopCnt <= 6);
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);

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
  #if defined(UA_PARALLEL_128)
        PSPLIT_HASKEY_GUTS_128_UA(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #else // defined(UA_PARALLEL_128)
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #endif // defined(UA_PARALLEL_128)
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
#endif // 0

static int
ListHasKey1696(Word_t *pwRoot, Word_t *pwr, Word_t wKey, int nBL)
{
    (void)nBL; (void)pwRoot;

    assert(nBL >   8);
    assert(nBL <= 16);
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, 16);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    assert(nPopCnt <= 6);
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("ListHasKey1696 nPopCnt %d psKeys %p\n",
         nPopCnt, (void *)psKeys));

    (void)nBL;
  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16)
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBL == 16) {
  #if defined(UA_PARALLEL_128)
        PSPLIT_HASKEY_GUTS_128_96(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #else // defined(UA_PARALLEL_128)
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #endif // defined(UA_PARALLEL_128)
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16) // defined(PSPLIT_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #else // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #endif // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    return nPos >= 0;
}

static int
ListHasKey16(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBLR >   8);
    assert(nBLR <= 16);
    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("ListHasKey16 nPopCnt %d psKeys %p\n",
                nPopCnt, (void *)psKeys));
  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16)
      #ifdef UA_PARALLEL_128
    if ((nPopCnt <= 6) && (nBLR == 16)) {
        PSPLIT_HASKEY_GUTS_128_96(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // UA_PARALLEL_128
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 16) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    { PSPLIT_HASKEY_GUTS(Bucket_t,
                         uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos); }
  #elif defined(BACKWARD_SEARCH_16) // defined(PSPLIT_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsM);
  #else // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsP);
  #endif // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
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
#endif // defined(PSPLIT_PARALLEL)
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
#elif defined(BACKWARD_SEARCH_32) // defined(PSPLIT_PARALLEL_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
    SMETRICS(++j__GetCallsM);
#else // defined(PSPLIT_PARALLEL_32) elif defined(BACKWARD_SEARCH_32) else
    // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos); (void)nBL;
    SMETRICS(++j__GetCallsP);
#endif // defined(PSPLIT_PARALLEL_32) elif defined(BACKWARD_SEARCH_32) else
    return nPos;
}

static int
ListHasKey32(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBLR >  16);
    assert(nBLR <= 32);
    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint32_t *piKeys = ls_piKeysNATX(pwr, nPopCnt);
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_32)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 32) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBLR == 24) {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY_GUTS(Bucket_t,
                           uint32_t, nBLR, piKeys, nPopCnt, iKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos);
    SMETRICS(++j__GetCallsM);
#else // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos);
    SMETRICS(++j__GetCallsP);
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

#if defined(PARALLEL_SEARCH_WORD)
  #define PSPLIT_SEARCH_W(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
      PSPLIT_SEARCH_BY_KEY_WORD(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)
#else // defined(PARALLEL_SEARCH_WORD)
  #define PSPLIT_SEARCH_W(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
      PSPLIT_SEARCH_BY_KEY_WORD(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos)
#endif // defined(PARALLEL_SEARCH_WORD)

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
    if (nBL != cnBitsPerWord) {
  #if defined(PSPLIT_SEARCH_XOR_WORD)
        Word_t wKeyMin = pwKeys[0];
        Word_t wKeyMax = pwKeys[nPopCnt - 1];
        // Or in 1 to handle nPopCnt==1 else we'd be taking the LOG of zero.
        nBL = LOG((wKeyMin ^ wKeyMax) | 1) + 1;
        // nBL could be 64 and it could be 0.
        // need a special psplit here that starts at wKeyMin
        #error Need a special PSPLIT for PSPLIT_SEARCH_XOR_WORD
  #endif // defined(PSPLIT_SEARCH_XOR_WORD)
        return PsplitSearchByKeyWord(nBL, pwKeys, nPopCnt, wKey, 0);
    }
#endif // defined(PSPLIT_SEARCH_WORD)
    // We want binary search for nBL == cnBitsPerWord by default.
    // We want binary search for nBL != cnBitsPerWord if !PSPLIT_SEARCH_WORD
    SMETRICS(int nCompares = 0);
#if !defined(NO_BINARY_SEARCH_WORD)
    Word_t *pwKeysOrig = pwKeys;
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
        SMETRICS(++nCompares);
    }
    nPos = pwKeys - pwKeysOrig;
    pwKeys = pwKeysOrig;
#endif // !defined(NO_BINARY_SEARCH_WORD)
  #if defined(BACKWARD_SEARCH_WORD)
    SEARCHB(Word_t, pwKeys, nPopCnt, wKey, nPos);
    SMETRICS(j__MisComparesM += nCompares);
    SMETRICS(++j__GetCallsM);
  #else // defined(BACKWARD_SEARCH_WORD)
    SEARCHF(Word_t, pwKeys, nPopCnt, wKey, nPos);
    SMETRICS(j__MisComparesP += nCompares);
    SMETRICS(++j__GetCallsP);
  #endif // defined(BACKWARD_SEARCH_WORD)
    DBGX(printf("SLW: return nPos %d\n", nPos));
    return nPos;
}

#ifdef PARALLEL_SEARCH_WORD
static int
BinaryHasKeyWord(Word_t *pwKeys, Word_t wKey, int nBL, int nPopCnt)
{
    (void)nBL;
    int nPos = 0;
    Word_t *pwKeysOrig = pwKeys;
  #ifdef PARALLEL_SEARCH_WORD
    int nPopCntOrig = nPopCnt; (void)nPopCntOrig;
  #endif // PARALLEL_SEARCH_WORD
  // BINARY_SEARCH narrows the scope of the linear search that follows.
    unsigned nSplit;
    // Looks like we might want a loop threshold of 8 for
    // 64-bit keys at the top level.
    // And there's not much difference with threshold of
    // 16 or 64.
    // Not sure about 64-bit keys at a lower level or
    // 64-bit keys at the top level.
//printf("pwKeys %p nPopCnt %d\n", pwKeys, nPopCnt);
    SMETRICS(int nCompares = 0);
    while (nPopCnt >= cnBinarySearchThresholdWord)
    {
        nSplit = nPopCnt / 2;
//printf("nSplit %d\n", nSplit);
//printf("nSplit & ~1 %d\n", nSplit & ~1);
//printf("pwKeys %p nPopCnt %d\n", pwKeys, nPopCnt);
//printf("pBucket %p\n", (void*)(((Word_t)&pwKeys[nSplit])&~0xf));
//fflush(stdout);
        if (BUCKET_HAS_KEY((Bucket_t *)(((Word_t)&pwKeys[nSplit])&~0xf), wKey, nBL)) {
//printf("ret\n"); fflush(stdout);
  #ifdef SEARCHMETRICS
            if (nPopCnt == nPopCntOrig) {
                ++j__DirectHits;
            } else if (nPos < nPopCntOrig / 2) {
                ++j__GetCallsM;
                j__MisComparesM += nCompares;
            } else {
                ++j__GetCallsP;
                j__MisComparesP += nCompares;
            }
  #endif // SEARCHMETRICS
            return 1;
        }
//printf("cont\n"); fflush(stdout);
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
        SMETRICS(++nCompares);
    }
    nPos = pwKeys - pwKeysOrig;
    pwKeys = pwKeysOrig;
    // What if one of nComparesB is not a miscompare? */
    // Call it a miscompare because it is an extra conditional branch.
  #if defined(BACKWARD_SEARCH_WORD)
      #ifdef PARALLEL_SEARCH_WORD
    assert(sizeof(Bucket_t) == sizeof(Word_t) * 2);
    // assumes bucket size is two words
    nPos = (nPos + 1) & ~1; // won't be required when code above is fixed
    HASKEYB(Bucket_t, wKey, pwKeys, nPopCntOrig, nPos);
      #else // PARALLEL_SEARCH_WORD
    SEARCHB(Word_t, pwKeys, nPopCnt, wKey, nPos);
      #endif // PARALLEL_SEARCH_WORD
    SMETRICS(j__MisComparesM += nCompares);
    SMETRICS(++j__GetCallsM);
  #else // defined(BACKWARD_SEARCH_WORD)
      #ifdef PARALLEL_SEARCH_WORD
    assert(sizeof(Bucket_t) == sizeof(Word_t) * 2);
    // assumes bucket size is two words
    nPos &= ~1; // won't be required when code above is fixed
    HASKEYF(Bucket_t, wKey, pwKeys, nPopCntOrig, nPos);
      #else // PARALLEL_SEARCH_WORD
    SEARCHF(Word_t, pwKeys, nPopCnt, wKey, nPos);
      #endif // PARALLEL_SEARCH_WORD
    SMETRICS(j__MisComparesP += nCompares);
    SMETRICS(++j__GetCallsP);
  #endif // defined(BACKWARD_SEARCH_WORD)
    return nPos >= 0;
}
#endif // PARALLEL_SEARCH_WORD

#ifdef LOOKUP
#if !defined(B_JUDYL) || defined(HASKEY_FOR_JUDYL_LOOKUP)

static int
ListHasKeyWord(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    int nPopCnt = gnListPopCnt(qy, nBLR);
  #if defined(SEARCH_FROM_WRAPPER)
    Word_t *pwKeys = ls_pwKeysNATX(pwr, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER)
    Word_t *pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER)
    DBGI(printf("LHKW pwKeys %p wKey " OWx" nBL %d nPopCnt %d\n",
                (void *)pwKeys, wKey, nBL, nPopCnt));
    int nPos;
#if defined(PSPLIT_SEARCH_WORD)
  #ifdef SEARCH_FROM_WRAPPER
    assert(nBLR != cnBitsPerWord);
  #else // SEARCH_FROM_WRAPPER
    if (nBLR != cnBitsPerWord)
  #endif // #else SEARCH_FROM_WRAPPER
    {
        nPos = 0;
  #ifdef PARALLEL_SEARCH_WORD
  #if defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
      #if (cnBitsPerWord > 32)
        if (nBLR == 56) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 56, pwKeys, nPopCnt, wKey, nPos);
        } else
        if (nBLR == 48) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 48, pwKeys, nPopCnt, wKey, nPos);
        } else
        if (nBLR == 40) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 40, pwKeys, nPopCnt, wKey, nPos);
        } else
          #ifndef COMPRESSED_LISTS
        if (nBLR == 32) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 32, pwKeys, nPopCnt, wKey, nPos);
        } else
          #endif // COMPRESSED_LISTS
      #else // (cnBitsPerWord > 32)
        if (nBLR == 24) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 24, pwKeys, nPopCnt, wKey, nPos);
        } else
          #ifndef COMPRESSED_LISTS
        if (nBLR == 16) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, 16, pwKeys, nPopCnt, wKey, nPos);
        } else if (nBLR == 8) {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t,  8, pwKeys, nPopCnt, wKey, nPos);
        } else
          #endif // COMPRESSED_LISTS
      #endif // (cnBitsPerWord > 32)
  #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
        {
            PSPLIT_HASKEY_GUTS(Bucket_t,
                               Word_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
        }
  #else // PARALLEL_SEARCH_WORD
        PSPLIT_SEARCH_BY_KEY_WORD(Word_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
  #endif // PARALLEL_SEARCH_WORD
        DBGX(printf("LHKW: returning %d\n", nPos >= 0));
        return nPos >= 0;
    }
#endif // defined(PSPLIT_SEARCH_WORD)
#if !defined(NO_BINARY_SEARCH_WORD) && defined(PARALLEL_SEARCH_WORD)
    if (sizeof(Bucket_t) > sizeof(Word_t)) {
        return BinaryHasKeyWord(pwKeys, wKey, nBLR, nPopCnt);
    } else
#endif // !defined(NO_BINARY_SEARCH_WORD) && defined(PARALLEL_SEARCH_WORD)
    {
        nPos = SearchListWord(pwKeys, wKey, nBLR, nPopCnt);
    }
    DBGX(printf("LHKW: returning %d\n", nPos >= 0));
    return nPos >= 0;
}

#endif // !defined(B_JUDYL) || defined(HASKEY_FOR_JUDYL_LOOKUP)
#endif // LOOKUP

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
SearchList(qp, int nBLR, Word_t wKey)
{
    qv;

    DBGL(printf("SearchList pwRoot %p wRoot " OWx" wKey " Owx" nBL %d\n",
                (void *)pwRoot, *pwRoot, wKey, nBL));

    int nPopCnt;
    int nPos;

  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
      // There is no need for a key size that is equal to or smaller than
      // whatever size yields a bitmap that will fit in a link.
    if (nBLR <= 8) {
        nPos = SearchList8(qy, nBLR, wKey);
    } else
      #endif // defined(cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBLR <= 16) {
        assert(nBLR > 8);
        nPos = SearchList16(qy, nBLR, wKey);
    } else
      #endif // defined(cnBitsInD1 <= 16)
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBLR <= 32) {
        assert(nBLR > 16);
        nPopCnt = gnListPopCnt(qy, nBLR);
        nPos = SearchList32(ls_piKeysNATX(pwr, nPopCnt), wKey, nBLR, nPopCnt);
    } else
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    {
  #if defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
        nPopCnt = gnListPopCnt(qy, nBLR);
        nPos = SearchListWord(ls_pwKeysNATX(pwr, nPopCnt),
                              wKey, nBLR, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
        nPopCnt = gnListPopCnt(qy, nBLR);
        //printf("pwRoot %p pwr %p\n", (void *)pwRoot, (void *)pwr);
        nPos = SearchListWord(ls_pwKeysX(pwr, nBLR, nPopCnt),
                              wKey, nBLR, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    }

    return nPos;
}

#ifdef LOOKUP
#if !defined(B_JUDYL) || defined(HASKEY_FOR_JUDYL_LOOKUP)

// Figure out if the key is in the sorted list.
// Return any non-negative number if the key is in the list.
// Return any negative number if the key is not in the list.
// ListHasKey is the list search function called by Judy1Lookup.
// It may also be used for Time -LV experiments by JudyLLookup.
static int
ListHasKey(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

#if 0
    if (nBLR <= 32) {
        if (nBLR <= 16) {
            return (nBLR <=  8) ? ListHasKey8 (qy,  8, wKey)
                                : ListHasKey16(qy, 16, wKey);
        } else {
            return (nBLR <= 24) ? ListHasKey32(qy, 24, wKey)
                                : ListHasKey32(qy, 32, wKey);
        }
    } else {
        if (nBLR <= 48) {
            return (nBLR <= 40) ? ListHasKeyWord(qy, 40, wKey)
                                : ListHasKeyWord(qy, 48, wKey);
        } else {
            return (nBLR <= 56) ? ListHasKeyWord(qy, 56, wKey)
                                : ListHasKeyWord(qy, 64, wKey);
        }
    }
#elif 0
    return (nBLR <= 16)
         ? (nBLR <=  8) ? ListHasKey8   (qy,    8, wKey)
                        : ListHasKey16  (qy,   16, wKey)
         : (nBLR <= 32) ? ListHasKey32  (qy, nBLR, wKey)
                        : ListHasKeyWord(qy, nBLR, wKey);
#elif 0
    return (nBLR <=  8) ? ListHasKey8   (qy,    8, wKey)
         : (nBLR <= 16) ? ListHasKey16  (qy,   16, wKey)
         : (nBLR <= 32) ? ListHasKey32  (qy, nBLR, wKey)
         :                ListHasKeyWord(qy, nBLR, wKey) ;
#elif 0
    // The switch looks a little strange because we're trying to get the
    // compiler to skip the bounds check that it normally does.
    switch ((nBLR/8 - 1) & 7) {
      #if (cnBitsInD1 <= 8)
    case 0 : break;
      #endif // (cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    case 1 : return ListHasKey16  (qy, 16, wKey);
      #endif // (cnBitsInD1 <= 16)
      #if (cnBitsPerWord <= 32)
    case 2 : return ListHasKeyWord(qy, 24, wKey);
    case 3 : return ListHasKeyWord(qy, 32, wKey);
      #else // (cnBitsPerWord <= 32)
          #if (cnBitsInD1 <= 32)
    case 2 : return ListHasKey32  (qy, 24, wKey);
    case 3 : return ListHasKey32  (qy, 32, wKey);
          #endif // (cnBitsInD1 <= 32)
    case 4 : return ListHasKeyWord(qy, 40, wKey);
    case 5 : return ListHasKeyWord(qy, 48, wKey);
    case 6 : return ListHasKeyWord(qy, 56, wKey);
    case 7 : return ListHasKeyWord(qy, 64, wKey);
      #endif // (cnBitsPerWord <= 32)
    }
             return ListHasKey8   (qy,  8, wKey);
#elif 0
    // The switch looks a little strange because we're trying to get the
    // compiler to skip the bounds check that it normally does.
    //extern int abc(); abc();
    switch (((nBLR-1) >> 3) & 3) {
      #if (cnBitsInD1 <= 8)
    case 0 : break;
      #endif // (cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    case 1 : return ListHasKey16  (qy, 16, wKey);
      #endif // (cnBitsInD1 <= 16)
      #if (cnBitsPerWord <= 32)
    case 2 : return ListHasKeyWord(qy, nBLR, wKey);
      #else // (cnBitsPerWord <= 32)
          #if (cnBitsInD1 <= 32)
    case 2 : return ListHasKey32  (qy, nBLR, wKey);
          #endif // (cnBitsInD1 <= 32)
    case 3 : return ListHasKeyWord(qy, nBLR, wKey);
      #endif // (cnBitsPerWord <= 32)
    }
             return ListHasKey8   (qy,  8, wKey);
#elif 0
  #if (cnBitsInD1 <= 8)
    if (nBLR <= 8) { return ListHasKey8(qy, nBLR, wKey); }
  #endif // (cnBitsInD1 <= 8)
  #if (cnBitsInD1 <= 32) && (cnBitsPerWord >= 32)
    if (nBLR <= 32) {
  #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord >= 32)
  #if (cnBitsInD1 <= 16)
        if (nBLR <= 16) { return ListHasKey16(qy, nBLR, wKey); }
  #endif // (cnBitsInD1 <= 16)
  #if (cnBitsInD1 <= 32) && (cnBitsPerWord >= 32)
        return ListHasKey32(qy, nBLR, wKey);
    }
  #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord >= 32)
    return ListHasKeyWord(qy, nBLR, wKey);
#else
  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
    if (nBLR <= 8) { return ListHasKey8(qy, nBLR, wKey); }
      #endif // (cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBLR <= 16) { return ListHasKey16(qy, nBLR, wKey); }
      #endif // (cnBitsInD1 <= 16)
      #if (cnBitsPerWord > 32)
          #if (cnBitsInD1 <= 32)
    if (nBLR <= 32) { return ListHasKey32(qy, nBLR, wKey); }
          #endif // (cnBitsInD1 <= 32)
      #endif // (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    return ListHasKeyWord(qy, nBLR, wKey);
#endif
}

#endif // !defined(B_JUDYL) || defined(HASKEY_FOR_JUDYL_LOOKUP)
#endif // LOOKUP

#if 0
// Locate the key in the sorted list.
// Return the position of the key in the list.
// Return any negative number if the key is not in the list.
static int
LocateKey(qp, int nBLR, Word_t wKey)
{
    return SearchList(qy, nBLR, wKey);
}

// Locate the slot in the sorted list where the key should be.
// Return the position of the slot in the list.
// Return any negative number if the key is already in the slot.
static int
LocateHole(qp, int nBLR, Word_t wKey)
{
    return ~SearchList(qy, nBLR, wKey);
}
#endif

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
// I'm not sure why, but Insert performance seems worse if
// EmbeddedListMagic is inlined. I noticed the performance of -DDEBUG
// was better than -UDEBUG and this is where the investigation led me.
#ifdef INSERT
__attribute__ ((noinline))
#endif // INSERT
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

static Word_t
ls_pxKeyX(Word_t *pwr, int nBL, int nPopCnt, int ii)
{
    (void)nPopCnt;
#if defined(COMPRESSED_LISTS)
    if (nBL <=  8) { return ls_pcKeyX(pwr, nBL, nPopCnt, ii); }
    if (nBL <= 16) { return ls_psKeyX(pwr, nBL, nPopCnt, ii); }
  #if cnBitsPerWord != 32
    if (nBL <= 32) { return ls_piKeyX(pwr, nBL, nPopCnt, ii); }
  #endif // cnBitsPerWord != 32
#else // defined(COMPRESSED_LISTS)
    (void)nBL;
#endif // defined(COMPRESSED_LISTS)
    return ls_pwKeyX(pwr, nBL, nPopCnt, ii);
}

static Word_t
ls_pxKey(Word_t *pwr, int nBL, int ii)
{
#if defined(COMPRESSED_LISTS)
    if (nBL <=  8) { return ls_pcKey(pwr, nBL, ii); }
    if (nBL <= 16) { return ls_psKey(pwr, nBL, ii); }
  #if cnBitsPerWord != 32
    if (nBL <= 32) { return ls_piKey(pwr, nBL, ii); }
  #endif // cnBitsPerWord != 32
#else // defined(COMPRESSED_LISTS)
    (void)nBL;
#endif // defined(COMPRESSED_LISTS)
    return ls_pwKey(pwr, nBL, ii);
}

#ifdef CODE_LIST_SW

// Get list switch link index (offset) from digit aka virtual index aka
// subkey extracted from key.
// If the link is not present then return the index at which it would be.
static inline void
ListSwIndex(qp, int nBW, Word_t wDigit,
            Word_t *pwSwIndex, int *pbLinkPresent)
{
    qv; (void)nBW;
    ListSw_t *pListSw = (ListSw_t *)pwr;
    uint8_t *pKeys = pListSw->sw_aKeys;
    int nPos = 0;
#if 1
    PSPLIT_SEARCH_BY_KEY(uint8_t, 8, pKeys,
                         gnListSwPop(qy), (uint8_t)wDigit, nPos);
#else
    SEARCHF(uint8_t, pKeys, gnListSwPop(qy), (uint8_t)wDigit, nPos);
#endif
    if (pwSwIndex != NULL) {
        if (nPos < 0) { nPos = -nPos; }
        *pwSwIndex = nPos;
    }
    if (pbLinkPresent != NULL) {
        *pbLinkPresent = (nPos >= 0);
    }
}

static inline Link_t *
gpListSwLinks(qp)
{
    return (Link_t *)&((ListSw_t *)pwr)->sw_aKeys[gnListSwPop(qy)];
}

#endif // CODE_LIST_SW

// Which word of the bitmap in a bm switch contains the bit for wDigit?
static inline int
gnWordNumInSwBm(Word_t wDigit)
{
#if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
    // We use only one-half of a word for bitmap bits.
    // We use the other half for a starting offset in the array of links.
    return wDigit >> (cnLogBitsPerWord - 1);
#else // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
    return wDigit >> cnLogBitsPerWord;
#endif // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)

}

static inline int
gnBitNumInSwBmWord(Word_t wDigit)
{
#if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
    // We use only one-half of a word for bitmap bits.
    // We use the other half for a starting offset in the array of links.
    return wDigit & (cnBitsPerWord/2 - 1);
#else // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
    return wDigit & (cnBitsPerWord - 1);
#endif // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
}

static inline Word_t
gwBitMaskInSwBmWord(Word_t wDigit)
{
    return (Word_t)1 << gnBitNumInSwBmWord(wDigit);
}

static inline int
BmSwLinkCnt(qp)
{
    qv;
    Word_t *pwBmWords = PWR_pwBm(pwRoot, wr_pwr(*pwRoot));
    int nLinks = 0;
    for (int nn = 0; nn < N_WORDS_SWITCH_BM; ++nn) {
        nLinks += __builtin_popcountll(pwBmWords[nn]
  #ifdef X_SW_BM_HALF_WORDS
                          & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
  #endif // X_SW_BM_HALF_WORDS
                                       );
    }
  #ifndef BM_SW_FOR_REAL
    assert(nLinks
        == (cnBitsPerWord
      #ifdef X_SW_BM_HALF_WORDS
               / 2
      #endif // X_SW_BM_HALF_WORDS
               * N_WORDS_SWITCH_BM));
  #endif // BM_SW_FOR_REAL
    return nLinks;
}

// Get bitmap switch link index (offset) from digit (virtual index)
// extracted from key.
// If the index is not present then return the index at which it would be.
static inline void
BmSwIndex(qp, Word_t wDigit,
          Word_t *pwSwIndex, int *pbLinkPresent)
{
    qv;
    Word_t *pwBmWords = PWR_pwBm(pwRoot, wr_pwr(*pwRoot));
    // The bitmap may have more than one word.
    // nBmWordNum is the number of the word which contains the bit we want.
    int nBmWordNum = gnWordNumInSwBm(wDigit);
    Word_t wBmWord = pwBmWords[nBmWordNum]; // word we want
    Word_t wBmBitMask = gwBitMaskInSwBmWord(wDigit);
    if (pwSwIndex != NULL) {
#ifdef X_ADD_ALL_SW_BM_WORDS
  #if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
#error X_ADD_ALL_SW_BM_WORDS with OFFSET_IN_SW_BM_WORD or X_SW_BM_HALF_WORDS
  #endif // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
        Word_t wSwIndex = 0;
        for (int nn = 0; n < N_WORDS_SWITCH_BM; nn++) {
            wSwIndex += __builtin_popcountll(pwBmWords[nn])
                            * (wDigit > nn * cnBitsPerWord);
        }
        wSwIndex += __builtin_popcountll(wBmWord & (wBmBitMask - 1));
        *pwSwIndex = wSwIndex;
#else // X_ADD_ALL_SW_BM_WORDS
  #ifdef OFFSET_IN_SW_BM_WORD
        *pwSwIndex = (wBmWord >> (cnBitsPerWord / 2))
                      + __builtin_popcountll(wBmWord & (wBmBitMask - 1));
  #else // OFFSET_IN_SW_BM_WORD
        *pwSwIndex = 0;
        for (int nn = 0; nn < nBmWordNum; nn++) {
            *pwSwIndex += __builtin_popcountll(pwBmWords[nn]
      #ifdef X_SW_BM_HALF_WORDS
                              & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
      #endif // X_SW_BM_HALF_WORDS
                                               );
        }
        *pwSwIndex += __builtin_popcountll(wBmWord & (wBmBitMask - 1));
  #endif // OFFSET_IN_SW_BM_WORD
#endif // X_ADD_ALL_SW_BM_WORDS
    }
    if (pbLinkPresent != NULL) {
        *pbLinkPresent = ((wBmWord & wBmBitMask) != 0);
    }
}

#if defined(B_JUDYL)
  #if !defined(SEARCH_FOR_JUDYL_LOOKUP)
      #if !defined(HASKEY_FOR_JUDYL_LOOKUP)
#define LOCATEKEY_FOR_LOOKUP
      #endif // !defined(HASKEY_FOR_JUDYL_LOOKUP)
  #endif // !defined(SEARCH_FOR_JUDYL_LOOKUP)
#else // defined(B_JUDYL)
  #if defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
#define LOCATEKEY_FOR_LOOKUP
  #endif // defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
#endif // defined(B_JUDYL)

#if defined(LOCATEKEY_FOR_LOOKUP)

#if defined(COMPRESSED_LISTS)
  #if (cnBitsInD1 <= 8)

static int
LocateKeyInList8(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

// HasKey128 assumes the list of keys starts at a 128-bit aligned address.
// SearchList8 makes no such assumption.
#if !defined(POP_IN_WR_HB) && !defined(LIST_POP_IN_PREAMBLE)
#if !defined(PP_IN_LINK) || (cnDummiesInList == 0)
#if !defined(POP_WORD_IN_LINK) || (cnDummiesInList == 0)
#if defined(OLD_LISTS)
    return SearchList8(qy, nBLR, wKey);
#endif // defined(OLD_LISTS)
#endif // !defined(POP_WORD_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(PP_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(POP_IN_WR_HB) && !defined(LIST_POP_IN_PREAMBLE)

#if defined(PSPLIT_SEARCH_8)
#if defined(PSPLIT_PARALLEL)

#if defined(PARALLEL_128)
#if cnBitsInD1 == 8
#if cnListPopCntMaxDl1 == 16
#if cnBitsMallocMask >= 4
#if cnDummiesInList == 0
  // ls_pcKeys is valid only at the top for pop in link.
  // Hence it's not really necessary to ifdef out these assertions at the top,
  // but making the exception is more work than I want to do right now.
  #if !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
  #if defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
    assert(ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8)) == (uint8_t*)pwr);
    assert(PWR_xListPopCnt(&wRoot, pwr, 8) <= 16);
  #endif // defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
  #endif // !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
    assert(((Word_t)pwr & ~((Word_t)-1 << 4)) == 0);
  #if defined(OLD_LISTS) && defined(HK40_EXPERIMENT)
    return LocateKey40(pwr, wKey);
  #else // defined(OLD_LISTS) && defined(HK40_EXPERIMENT)
      #ifdef OLD_LISTS // includes PP_IN_LINK and POP_WORD_IN_LINK
    return LocateKey128((__m128i*)pwr, wKey, 8);
      #else // OLD_LISTS
    return LocateKey128((__m128i*)ls_pcKeysNATX(pwr, 16), wKey, 8);
      #endif // OLD_LISTS
  #endif // HK40_EXPERIMENT
#endif // cnDummiesInList == 0
#endif // cnBitsMallocMask >= 4
#endif // cnListPopCntMaxDl1 == 16
#endif // cnBitsInD1 == 8
#endif // defined(PARALLEL_128)

    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint8_t *pcKeys = ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8));
    uint8_t cKey = (uint8_t)wKey;
    int nPos = 0;
    PSPLIT_LOCATEKEY_GUTS(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    return nPos;

#endif // defined(PSPLIT_PARALLEL)
#endif // defined(PSPLIT_SEARCH_8)

    return SearchList8(qy, nBLR, wKey);
}
  #endif // (cnBitsInD1 <= 8)

static int
LocateKeyInList16(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBLR >   8);
    assert(nBLR <= 16);
    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
    DBGL(printf("LocateKeyInList16 nPopCnt %d psKeys %p\n",
                nPopCnt, (void *)psKeys));
  #if defined(LIST_END_MARKERS)
    assert(psKeys[-1] == 0);
      #if defined(PSPLIT_PARALLEL)
    assert(*(uint16_t *)(((Word_t)&psKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint16_t)-1);
      #else // defined(PSPLIT_PARALLEL)
    assert(psKeys[nPopCnt] == (uint16_t)-1);
      #endif // defined(PSPLIT_PARALLEL)
  #endif // defined(LIST_END_MARKERS)
    uint16_t sKey = (uint16_t)wKey;
    int nPos = 0;
  #if defined(PSPLIT_SEARCH_16)
      #ifdef UA_PARALLEL_128
    if ((nPopCnt <= 6) && (nBLR == 16)) {
        PSPLIT_LOCATEKEY_GUTS_128_96(uint16_t,
                                     16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // UA_PARALLEL_128
      #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 16) {
        PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                              uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    { PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                            uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos); }
  #elif defined(BACKWARD_SEARCH_16) // defined(PSPLIT_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsM);
  #else // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos);
    SMETRICS(++j__GetCallsP);
  #endif // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    return nPos;
}

#if (cnBitsPerWord > 32)

static int
LocateKeyInList32(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBLR >  16);
    assert(nBLR <= 32);
    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint32_t *piKeys = ls_piKeysNATX(pwr, nPopCnt);
#if defined(LIST_END_MARKERS)
    assert(piKeys[-1] == 0);
#if defined(PSPLIT_PARALLEL)
    assert(*(uint32_t *)(((Word_t)&piKeys[nPopCnt] + sizeof(Bucket_t) - 1)
            & ~(sizeof(Bucket_t) - 1))
        == (uint32_t)-1);
#else // defined(PSPLIT_PARALLEL)
    assert(piKeys[nPopCnt] == (uint32_t)-1);
#endif // defined(PSPLIT_PARALLEL)
#endif // defined(LIST_END_MARKERS)
    uint32_t iKey = (uint32_t)wKey;
    int nPos = 0;
#if defined(PSPLIT_SEARCH_32)
#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 32) {
        PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                              uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBLR == 24) {
        PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                              uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                              uint32_t, nBLR, piKeys, nPopCnt, iKey, nPos);
    }
#elif defined(BACKWARD_SEARCH_32) // defined(PSPLIT_SEARCH_32)
    SEARCHB(uint32_t, piKeys, nPopCnt, iKey, nPos);
    SMETRICS(++j__GetCallsM);
#else // defined(PSPLIT_SEARCH_32) elif defined(BACKWARD_SEARCH_32) else
    // here for forward linear search with end check
    SEARCHF(uint32_t, piKeys, nPopCnt, iKey, nPos);
    SMETRICS(++j__GetCallsP);
#endif // defined(PSPLIT_SEARCH_32) elif defined(BACKWARD_SEARCH_32) else
    return nPos;
}

#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)

static int
LocateKeyInListWord(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    int nPopCnt = gnListPopCnt(qy, nBLR);
  #if defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    Word_t *pwKeys = ls_pwKeysNATX(pwr, nPopCnt);
  #else // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    Word_t *pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
  #endif // defined(SEARCH_FROM_WRAPPER) && defined(LOOKUP)
    DBGI(printf("LKILW pwKeys %p wKey " OWx" nBL %d nPopCnt %d\n",
                (void *)pwKeys, wKey, nBL, nPopCnt));
    int nPos;
#if defined(PSPLIT_SEARCH_WORD)
  #ifdef SEARCH_FROM_WRAPPER
    assert(nBLR != cnBitsPerWord);
  #else // SEARCH_FROM_WRAPPER
    if (nBLR != cnBitsPerWord)
  #endif // #else SEARCH_FROM_WRAPPER
    {
        nPos = 0;
  #ifdef PARALLEL_SEARCH_WORD
  #if defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
      #if (cnBitsPerWord > 32)
        if (nBLR == 56) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 56, pwKeys, nPopCnt, wKey, nPos);
        } else if (nBLR == 48) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 48, pwKeys, nPopCnt, wKey, nPos);
        } else if (nBLR == 40) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 40, pwKeys, nPopCnt, wKey, nPos);
        } else
          #ifndef COMPRESSED_LISTS
        if (nBLR == 32) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 32, pwKeys, nPopCnt, wKey, nPos);
        } else
          #endif // COMPRESSED_LISTS
      #else // (cnBitsPerWord > 32)
        if (nBLR == 24) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 24, pwKeys, nPopCnt, wKey, nPos);
        } else
          #ifndef COMPRESSED_LISTS
        if (nBLR == 16) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, 16, pwKeys, nPopCnt, wKey, nPos);
        } else if (nBLR == 8) {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t,  8, pwKeys, nPopCnt, wKey, nPos);
        } else
          #endif // COMPRESSED_LISTS
      #endif // (cnBitsPerWord > 32)
  #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
        {
            PSPLIT_LOCATEKEY_GUTS(Bucket_t,
                                  Word_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
        }
        DBGX(printf("LKILW: returning %d\n", nPos));
  #else // PARALLEL_SEARCH_WORD
        PSPLIT_SEARCH_BY_KEY_WORD(Word_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
  #endif // #else PARALLEL_SEARCH_WORD
        return nPos;
    }
#endif // defined(PSPLIT_SEARCH_WORD)
    nPos = SearchListWord(pwKeys, wKey, nBLR, nPopCnt);
    DBGX(printf("LKILW: returning %d\n", nPos));
    return nPos;
}

static int
LocateKeyInList(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8)
    if (nBLR <= 8) {
        return LocateKeyInList8(qy, nBLR, wKey);
    }
      #endif // (cnBitsInD1 <= 8)
      #if (cnBitsInD1 <= 16)
    if (nBLR <= 16) {
        return LocateKeyInList16(qy, nBLR, wKey);
    }
      #endif // (cnBitsInD1 <= 16)
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBLR <= 32) {
        return LocateKeyInList32(qy, nBLR, wKey);
    }
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    return LocateKeyInListWord(qy, nBLR, wKey);
}

#endif // defined(LOCATEKEY_FOR_JUDY1_LOOKUP)

#endif // (cnDigitsPerWord > 1)

#if defined(RAMMETRICS)

extern Word_t j__AllocWordsTOT;
extern Word_t j__RequestedWordsTOT;
extern Word_t j__TotalBytesAllocated; // mmap

extern Word_t j__ExtraWordsTOT;
extern Word_t j__ExtraWordsCnt;

extern Word_t j__AllocWordsJLLW; // 1 word/key list leaf
//extern Word_t j__AllocWordsJBL;  // linear branch
extern Word_t j__AllocWordsJBB;  // bitmap branch
extern Word_t j__AllocWordsJBU;  // uncompressed branch
extern Word_t j__AllocWordsJLB1; // bitmap leaf
extern Word_t j__AllocWordsJLL1; // 1 byte/key list leaf
extern Word_t j__AllocWordsJLL2; // 2 bytes/key list leaf
extern Word_t j__AllocWordsJLL3; // B2 big bitmap leaf
extern Word_t j__AllocWordsJLL4; // 4 bytes/key list leaf
extern Word_t j__AllocWordsJLL5; // 5 bytes/key list leaf
extern Word_t j__AllocWordsJLL6; // 6 bytes/key list leaf
extern Word_t j__AllocWordsJLL7; // words requested

#ifndef B_JUDYL
// Coopt j__AllocWordsJV for JLB2 big bitmap at digit 2.
extern Word_t j__AllocWordsJV;   // value area
#define j__AllocWordsJLB2  j__AllocWordsJV
#endif // B_JUDYL

#endif // defined(RAMMETRICS)

#endif // ( ! defined(_B_H_INCLUDED) )
