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

// Use ASSERT for assertions that might be in the performance path and
// would slow down regression testing with DEBUG.
#ifdef DEBUG_ASSERT
  #define ASSERT(_p)  assert(_p)
#else // DEBUG_ASSERT
  #define ASSERT(_p)  0
#endif // DEBUG_ASSERT

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

// This LOG macro works for 64-bit and 32-bit on Linux and Windows without
// any ifdef because we subtract clzll from 63 independent of cnBitsPerWord.
// This way we avoid using ifdef to choose the variant of __builtin_clz[l][l]
// we need. The cost is that we have to write this long comment.
// And what happens if clzll becomes 128-bit someday?
// __builtin_clzll is undefined for zero which allows the compiler to use bsr.
// So this macro is undefined for _x == 0.
#define LOG(_x)  (63 - (int)__builtin_clzll(_x))
// BPW - Bits Per Word - can handle _x == cnBitsPerWord
#define BPW_EXP(_x) \
    (assert((_x) <= cnBitsPerWord), ((Word_t)1 << ((_x) - 1)) << 1)
// NBPW - Not Bits Per Word - can't handle _x == cnBitsPerWord
#define NBPW_EXP(_x)  (assert((_x) < cnBitsPerWord), (Word_t)1 << (_x))
#define EXP(_x)  NBPW_EXP(_x)
// NBPW - Not Bits Per Word - can't handle nBits == cnBitsPerWord
#define NBPW_MSK(_nBits) \
    (NBPW_EXP(_nBits) - 1) // can't handle nBits == cnBitsPerWord
// NZ - Not Zero - can't handle nBits == 0
#define NZ_MSK(_nBits) \
    (assert((_nBits) > 0), (Word_t)-1 >> (cnBitsPerWord - (_nBits)))
// ANY_MSK can handle both _nBits == cnBitsPerWord AND _nBits == 0.
#if 0
// I think this worked with gcc when I tested it. But it is not defined
// for _nBits == cnBitsPerWord.
// And I have seen it fail with clang.
#define ANY_MSK(_nBits) \
    ((((Word_t)1 << (_nBits)) - 1) - ((_nBits) == cnBitsPerWord))
#endif
#if 0
// This is not defined for _nBits == 0.
#define ANY_MSK(_nBits) \
    ((Word_t)-1 >> (cnBitsPerWord - (_nBits))) + ((_nBits) == 0)
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
// pLn is NULL if nBL == cnBitsPerWord && sizeof(Link_t) > sizeof(Word_t).
// Sure would like to get rid of pwRoot.
// And possibly add nBLR.
// And wPopCnt would be an option.
// And nBW would be an option.
// And how about wBytesUsed?
#define  qp  int   nBL, Link_t  * pLn
#define pqp \
    int *pnBL, Link_t **ppLn, Word_t *pwRoot, int *pnType, Word_t **ppwr

#define  qpx(Suffix) \
    int nBL##Suffix, Link_t *pLn##Suffix

// Shorthand for common arguments.
// Why is "qy" not "qa"? Because "qa" is harder to type?
#define  qy   nBL,  pLn
#define pqy  &nBL, &pLn, &wRoot, &nType, &pwr

#define qyx(Suffix)  nBL##Suffix, pLn##Suffix

#define  qv \
    Word_t* pwRoot = &pLn->ln_wRoot; \
    Word_t wRoot = pLn->ln_wRoot; \
    int nType = wr_nType(wRoot); \
    Word_t* pwr = wr_pwr(wRoot); \
    (void)nBL; (void)pLn; (void)pwRoot; (void)wRoot; (void)nType; (void)pwr

// qv is shorthand to silence not-used compiler warnings.
// And to initialize local variables.
// And to validate assumptions.
#define  qvx(Suffix) \
    Word_t* pwRoot##Suffix = &pLn##Suffix->ln_wRoot; \
    Word_t wRoot##Suffix = pLn##Suffix->ln_wRoot; \
    int nType##Suffix = wr_nType(wRoot##Suffix); \
    Word_t* pwr##Suffix = wr_pwr(wRoot##Suffix); \
    (void)nBL##Suffix; (void)pLn##Suffix; (void)pwRoot##Suffix; \
    (void)wRoot##Suffix; (void)nType##Suffix; (void)pwr##Suffix

#define pqv \
    (void)pnBL; (void)ppLn; (void)pwRoot; (void)pnType; (void)ppwr; \
    ASSERT(*pwRoot == (*ppLn)->ln_wRoot); \
    ASSERT(*pnType == wr_nType(*pwRoot) || (*pnBL <= cnLogBitsPerLink)); \
    ASSERT(*ppwr == wr_pwr(*pwRoot) || (*pnBL <= cnLogBitsPerLink))

// Common arguments to printf.
#define qyp   nBL, (void*)pLn, wRoot, nType, (void*)pwr

// Format string for qyp
#define qfmt "nBL %2d pLn %p wRoot 0x%016zx nType %x pwr %p"

// Default is -USKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX.
// Default is -USAVE_PREFIX -USAVE_PREFIX_TEST_RESULT
// Default is -UALWAYS_CHECK_PREFIX_AT_LEAF.
// -DALWAYS_CHECK_PREFIX_AT_LEAF appears to be best for 64-bit Judy1 with
// 16-bit digits and a 16-bit bitmap. More digits and -DSKIP_LINKS of any
// flavor seems similar.
// -USKIP_LINKS is a lot better for 32-bit Judy1 with 16-bit digits and a
// 16-bit bitmap. Less so for 8-bit digits. Maybe.

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
#ifdef BITMAP
  #if !defined(cn2dBmMaxWpkPercent)
      #ifdef B_JUDYL // JudyL has no 2-digit bitmap yet.
    #define cn2dBmMaxWpkPercent  0 // JudyL has no 2-digit bitmap yet.
      #else // B_JUDYL
        #if (cnBitsPerWord == 32)
    #define cn2dBmMaxWpkPercent  30
        #else // (cnBitsPerWord == 32)
    #define cn2dBmMaxWpkPercent  15
        #endif // #else (cnBitsPerWord == 32)
      #endif // #else B_JUDYL
  #endif // !defined(cn2dBmMaxWpkPercent)
#endif // BITMAP

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
// If nBL == cnBitsLeftAtDl<x> and cnListPopCntMaxDl<x> is defined, then
// cnListPopCntMaxDl<x> governs maximum external list size.
// If cnListPopCntMaxDl<x> is not defined or nBL != cnBitsLeftAtDl<x> for
// any <x>, then maximum external list size is governed by
// cnListPopCntMax(4|8|12|16|20|24|28|32|36|40|44|48|52|56|60|64).
// For example, cnListPopCntMax24 governs maximum external list size for
// 20 < nBL <= 24.
// EmbeddedListPopCntMax(nBL) governs maximum embedded list size unless
// defined(POP_CNT_MAX_IS_KING) in which case the rules above for maximum
// external list size also govern the maximum embedded list size.
// I wonder if we should extend the reach of cnListPopCntMaxDl<blah>.

// USE_XX_SW_ONLY_AT_DL2 should ignore anListPopCntMax
// for nBL <= cnBitsLeftAtDl2.

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
#if !defined(PARALLEL_64) && !defined(NO_PARALLEL_128)
  #undef PARALLEL_128
  #define PARALLEL_128
#endif // !defined(PARALLEL_64) && !defined(NO_PARALLEL_128)

// UA_PARALLEL_128, i.e. unaligned parallel 128, was designed to save memory
// by eliminating the requirement that lists be padded to an integral number
// of 16-byte bucket lengths while preserving our ability to use 128-bit
// parallel searches.
// This proof-of-concept is very limited and has not been hardened.
// It is enabled by default if and only if cnBitsPerWord==32 and
// cnBitsMallocMask >= 4.
// And then only lists of 16-bit keys that fit in 12 bytes are made T_LIST_UA.
//
#if defined(PSPLIT_PARALLEL) && defined(PARALLEL_128)
  #ifdef COMPRESSED_LISTS
  #ifndef NO_UA_PARALLEL_128
    #if (cnBitsPerWord == 32) && (cnBitsMallocMask >= 4)
      #undef UA_PARALLEL_128
      #define UA_PARALLEL_128
    #endif // (cnBitsPerWord == 32) && (cnBitsMallocMask >= 4)
  #endif // NO_UA_PARALLEL_128
  #endif // COMPRESSED_LISTS
#endif // defined(PSPLIT_PARALLEL) && defined(PARALLEL_128)

#ifndef COMPRESSED_LISTS
  #ifdef UA_PARALLEL_128
    #error UA_PARALLEL_128 without COMPRESSED_LISTS
  #endif // UA_PARALLEL_128
#endif // #ifndef COMPRESSED_LISTS

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

// Default is cnBinarySearchThresholdWord is 16.
#if ! defined(cnBinarySearchThresholdWord)
#define cnBinarySearchThresholdWord  16
#endif // ! defined(cnBinarySearchThresholdWord)

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

static int
ExtListBytesPerKey(int nBL)
{
  #if defined(COMPRESSED_LISTS)
    // log(56-1) = 5, log(40-1) = 5, exp(5+1) = 64
    // log(32-1) = 4, log(24-1) = 4, exp(4+1) = 32
    // log(16-1) = 3, exp(3+1) = 16
    // log(8-1) = 2, exp(2+1) = 8
    //assert(nBL >= 5);
    //return EXP(LOG(nBL-1)-2);
    // Will the compiler get rid of LOG and EXP if nBL is a constant?
    // Or are we better off with the old way?
    #if 0
    return (nBL <= 16) ? (nBL <=  8) ? 1 : 2
        : ((cnBitsPerWord > 32) && (nBL <= 32)) ? 4 : sizeof(Word_t);
    #else
      #if 1
    return EXP((nBL > 8) + (nBL > 16) + ((cnBitsPerWord > 32) && (nBL > 32)));
      #else
    return (nBL <=  8) ? 1 : (nBL <= 16) ? 2 :
        ((cnBitsPerWord > 32) && (nBL <= 32)) ? 4 : sizeof(Word_t);
    // Or should we just assume that nBL is a multiple of 8?
      #endif
    #endif
  #else // defined(COMPRESSED_LISTS)
    (void)nBL;
    return sizeof(Word_t);
  #endif // defined(COMPRESSED_LISTS)
}

// Default is -DPSPLIT_EARLY_OUT which is applicable only if PSPLIT_PARALLEL.
#if ! defined(NO_PSPLIT_EARLY_OUT)
#undef  PSPLIT_EARLY_OUT
#define PSPLIT_EARLY_OUT
#endif // ! defined(NO_PSPLIT_EARLY_OUT)

#include <immintrin.h> // __m128i
#ifdef B_JUDYL
  #include <xmmintrin.h> // _MM_HINT_ET0, _MM_HINT_T0, _MM_HINT_NTA
#endif // B_JUDYL
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
// Default is -DSKIP_TO_XX_SW iff -DSKIP_LINKS && -DCODE_XX_SW.
// And lvl not in type.
#if ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
#if defined(BITMAP) && !defined(NO_SKIP_TO_BITMAP)
#undef SKIP_TO_XX_SW
#define SKIP_TO_XX_SW
#endif // defined(BITMAP) && !defined(NO_SKIP_TO_BITMAP)
#endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
#endif // ! defined(NO_SKIP_TO_XX_SW) && defined(SKIP_LINKS)
#endif // defined(CODE_XX_SW)

// SKIP_TO_XX_SW implies SKIP_TO_BITMAP.
#ifdef SKIP_TO_XX_SW
  #ifdef USE_XX_SW_ONLY_AT_DL2
    #undef  SKIP_TO_BITMAP
    #define SKIP_TO_BITMAP
  #endif // USE_XX_SW_ONLY_AT_DL2
#endif // SKIP_TO_XX_SW

#if defined(CODE_XX_SW)
    #define cbCodeXxSw  1
    #define IF_CODE_XX_SW(_expr)  (_expr)
#else // defined(CODE_XX_SW)
    #define cbCodeXxSw  0
    #define IF_CODE_XX_SW(_expr)
#endif // defined(CODE_XX_SW)

#if defined(SKIP_TO_XX_SW)
    #define cbSkipToXxSw  1
    #define IF_SKIP_TO_XX_SW(_expr)  (_expr)
#else // defined(SKIP_TO_XX_SW)
    #define cbSkipToXxSw  0
    #define IF_SKIP_TO_XX_SW(_expr)
#endif // defined(SKIP_TO_XX_SW)

// Default is SKIP_TO_BM_SW if USE_BM_SW and (LVL_IN_PP or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)
  #if defined(USE_BM_SW)
      #if defined(LVL_IN_PP) || defined(LVL_IN_WR_HB)
          #undef SKIP_TO_BM_SW
          #define SKIP_TO_BM_SW
      #endif // defined(LVL_IN_PP) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_BM_SW)
#endif // ! defined(NO_SKIP_TO_BM_SW) && defined(SKIP_LINKS)

// Default is SKIP_TO_LIST_SW if USE_LIST_SW and (LVL_IN_PP or LVL_IN_WR_HB).
#if ! defined(NO_SKIP_TO_LIST_SW) && defined(SKIP_LINKS)
  #if defined(USE_LIST_SW)
      #if defined(LVL_IN_PP) || defined(LVL_IN_WR_HB)
          #undef SKIP_TO_LIST_SW
          #define SKIP_TO_LIST_SW
      #endif // defined(LVL_IN_PP) || defined(LVL_IN_WR_HB)
  #endif // defined(USE_LIST_SW)
#endif // ! defined(NO_SKIP_TO_LIST_SW) && defined(SKIP_LINKS)

// Default is -DOLD_LISTS.
#if ! defined(NO_OLD_LISTS)
  #undef  OLD_LISTS
  #define OLD_LISTS
#endif // ! defined(NO_OLD_LISTS)

// Default is -DPOP_IN_WR_HB. List pop count in wRoot high bits.
#if ! defined(NO_POP_IN_WR_HB) && (cnBitsPerWord > 32)
  #undef  POP_IN_WR_HB
  #define POP_IN_WR_HB
#endif // ! defined(NO_POP_IN_WR_HB) && (cnBitsPerWord > 32)

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

// Default cnListPopCntMaxIncr is 0.
#ifndef cnListPopCntMaxIncr
  #define cnListPopCntMaxIncr  0
#endif // #ifndef cnListPopCntMaxIncr

// Default cnListPopCntMax64 is 256.
#ifndef cnListPopCntMax64
  #define cnListPopCntMax64  256
#endif // #ifndef cnListPopCntMax64

// Default cnListPopCntMax[n] is cnListPopCntMax[n+4] + cnListPopCntMaxIncr.
#ifndef cnListPopCntMax60
  #define cnListPopCntMax60  (cnListPopCntMax64 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax60

#ifndef cnListPopCntMax56
  #define cnListPopCntMax56  (cnListPopCntMax60 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax56

#ifndef cnListPopCntMax52
  #define cnListPopCntMax52  (cnListPopCntMax56 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax52

#ifndef cnListPopCntMax48
  #define cnListPopCntMax48  (cnListPopCntMax52 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax48

#ifndef cnListPopCntMax44
  #define cnListPopCntMax44  (cnListPopCntMax48 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax44

#ifndef cnListPopCntMax40
  #define cnListPopCntMax40  (cnListPopCntMax44 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax40

#ifndef cnListPopCntMax36
  #define cnListPopCntMax36  (cnListPopCntMax40 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax36

#ifndef cnListPopCntMax32
  #define cnListPopCntMax32  (cnListPopCntMax36 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax32

#ifndef cnListPopCntMax28
  #define cnListPopCntMax28  (cnListPopCntMax32 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax28

#ifndef cnListPopCntMax24
  #define cnListPopCntMax24  (cnListPopCntMax28 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax24

#ifndef cnListPopCntMax20
  #define cnListPopCntMax20  (cnListPopCntMax24 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax20

#ifndef cnListPopCntMax16
  #define cnListPopCntMax16  (cnListPopCntMax20 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax16

#ifndef cnListPopCntMax12
  #define cnListPopCntMax12  (cnListPopCntMax16 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax12

#ifndef cnListPopCntMax8
  #define cnListPopCntMax8  (cnListPopCntMax12 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax8

#ifndef cnListPopCntMax4
  #define cnListPopCntMax4  (cnListPopCntMax8 + cnListPopCntMaxIncr)
#endif // #ifndef cnListPopCntMax4

// If we're not using bitmaps then we must have some other way of getting to
// full pop. The only options are embedded or external list at Dl1 or higher.
#ifdef BITMAP
  #ifndef cnListPopCntMaxDl1
    #ifdef B_JUDYL
      // Default is to transition from list leaf one to uncompressed bitmap
      // leaf when the leaf is half way to full pop.
      #define cnListPopCntMaxDl1  128
    #else // B_JUDYL
      #define cnListPopCntMaxDl1  0x10
    #endif // #else B_JUDYL
  #endif // cnListPopCntMaxDl1
#else // BITMAP
  #ifdef cnListPopCntMaxDl1
    #if !defined(EMBED_KEYS) || defined(POP_CNT_MAX_IS_KING)
      #if (cnListPopCntMaxDl1 < (1 << cnBitsInD1))
        // It is possible that a higher level list makes Dl1 irrelevant.
        // If that is the case then why define cnListPopCntMaxDl1 at all?
        // Work around this error by not defining cnListPopCntMaxDl1.
        #error (cnListPopCntMaxDl1 < (1 << cnBitsInD1)) with no BITMAP
      #endif // (cnListPopCntMaxDl1 < (1 << cnBitsInD1))
    #else // !defined(EMBED_KEYS) || defined(POP_CNT_MAX_IS_KING)
      // Let's hope EmbeddedListPopCntMax(cnBitsInD1) >= (1 << cnBitsInD1).
      // Or that we are covered by a higher level list.
      // Let's check it in Initialize.
    #endif // #else !defined(EMBED_KEYS) || defined(POP_CNT_MAX_IS_KING)
  #else // cnListPopCntMaxDl1
    // Setting cnListPopCntMaxDl1 if we're covered by embedded keys and
    // !POP_CNT_MAX_IS_KING or by a higher level list does no harm.
    #define cnListPopCntMaxDl1  (1 << cnBitsInD1)
  #endif // #else cnListPopCntMaxDl1 < (1 << cnBitsInD1)
#endif // #else BITMAP

#define MAX(_x, _y)  ((_x) > (_y) ? (_x) : (_y))

#if (cnBitsPerWord >= 64)
  #define _cnListPopCntMax0 \
    MAX(cnListPopCntMax64, \
    MAX(cnListPopCntMax56, \
    MAX(cnListPopCntMax48, \
    MAX(cnListPopCntMax40, \
        MAX(cnListPopCntMax32, \
            MAX(cnListPopCntMax16, \
                MAX(cnListPopCntMax8, 0)))))))
#else // (cnBitsPerWord >= 64)
  #define _cnListPopCntMax0 \
    MAX(cnListPopCntMax32, \
    MAX(cnListPopCntMax24, \
        MAX(cnListPopCntMax16, \
            MAX(cnListPopCntMax8, 0))))
#endif // (cnBitsPerWord >= 64)

#ifdef cnListPopCntMaxDl1
  #define _cnListPopCntMax1  MAX(_cnListPopCntMax0, cnListPopCntMaxDl1)
#else // cnListPopCntMaxDl1
  #define _cnListPopCntMax1  _cnListPopCntMax0
#endif // #else cnListPopCntMaxDl1
#ifdef cnListPopCntMaxDl2
  #define _cnListPopCntMax2  MAX(_cnListPopCntMax1, cnListPopCntMaxDl2)
#else // cnListPopCntMaxDl2
  #define _cnListPopCntMax2  _cnListPopCntMax1
#endif // #else cnListPopCntMaxDl2
#ifdef cnListPopCntMaxDl3
  #define _cnListPopCntMax3  MAX(_cnListPopCntMax2, cnListPopCntMaxDl3)
#else // cnListPopCntMaxDl3
  #define _cnListPopCntMax3  _cnListPopCntMax2
#endif // #else cnListPopCntMaxDl3

#if defined(EMBED_KEYS) && !defined(POP_CNT_MAX_IS_KING)
  #define _cnListPopCntMaxEK  MAX(_cnListPopCntMax3, 1)
#else // defined(EMBED_KEYS && !defined(POP_CNT_MAX_IS_KING)
  #define _cnListPopCntMaxEK  _cnListPopCntMax3
#endif // #else defined(EMBED_KEYS && !defined(POP_CNT_MAX_IS_KING)

#define cwListPopCntMax  _cnListPopCntMaxEK

#if (cwListPopCntMax != 0)
  // ListPopCntMax of 255 allows us to use uint8_t for uPopCntMax_t.
  // It might be faster.
  #if (cwListPopCntMax < 256)
typedef uint8_t uListPopCntMax_t;
  #else // (cwListPopCntMax < 256)
typedef uint16_t uListPopCntMax_t;
  #endif // #else (cwListPopCntMax < 256)
  #ifdef POP_CNT_MAX_IS_KING
      #ifdef B_JUDYL
#define auListPopCntMax  auListPopCntMaxL
      #else // B_JUDYL
#define auListPopCntMax  auListPopCntMax1
      #endif // #else B_JUDYL
// Max list length as a function of nBL.
// Array is indexed by nBL.
extern uListPopCntMax_t auListPopCntMax[];
  #endif // POP_CNT_MAX_IS_KING
#endif // (cwListPopCntMax != 0)

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

#define cnBWAtTop \
    (cnBitsPerWord - nBL_from_nDL(cnDigitsPerWord - 1))

// cnDigitsPerWord makes assumptions about anDL_to_nBW[] and
// anDL_to_nBL[].  Yuck.
#if (cnBitsInD3 != cnBitsPerDigit)
#define cnDigitsPerWord \
    (DIV_UP_X(cnBitsPerWord - cnBitsLeftAtDl3, cnBitsPerDigit) + 3)
#else // (cnBitsInD3 != cnBitsPerDigit)
#define cnDigitsPerWord \
    (DIV_UP_X(cnBitsPerWord - cnBitsInD1 - cnBitsInD2, cnBitsPerDigit) + 2)
#endif // (cnBitsInD3 != cnBitsPerDigit)

#if defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)
  #error Sorry, no SKIP_TO_BM_SW without USE_BM_SW.
#endif // defined(SKIP_TO_BM_SW) && ! defined(USE_BM_SW)

#if defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)
  #if ! defined(LVL_IN_PP) && ! defined(LVL_IN_WR_HB)
      #error Sorry, no SKIP_TO_BM_SW without LVL_IN_PP or NO_LVL_IN_WR_HB.
  #endif // ! defined(LVL_IN_PP) && ! defined(LVL_IN_WR_HB)
#endif // defined(SKIP_TO_BM_SW) && defined(USE_BM_SW)

// Values for nType.
// Be very careful with enumerators in cpp #if statements. I got bit by the
// fact that enumerators all have a value of zero at preprocessor time.
// The preprocessor does not generate an error if identifiers ENUMERATOR_1
// and ENUMERATOR_2 are not defined as macros and are used like
// "#if ENUMERATOR_1 == ENUMERATOR_2".
// The preprocessor will simply always take the condition is true path.
// Guess I need to go through the code with a fine-tooth comb.
// Or maybe define nType values in a different way?
enum {
    // Put T_NULL and T_LIST at beginning of enum so one of them gets
    // type == 0 if either exists. For no reason other than a dump with
    // a NULL will have a somewhat intuitive meaning.
#if defined(SEPARATE_T_NULL)
    T_NULL, // no keys below
#endif // defined(SEPARATE_T_NULL)
#if (cwListPopCntMax != 0)
    T_LIST, // external list of keys
  #ifdef XX_LISTS
    // T_XX_LIST is shared by multiple links.
    // It allows deferred splay when doubling-down a switch.
    // It allows better memory efficiency for JudyL with no embedded keys.
    // The log of the number of links that share the list is specified by
    // gnListBLR(qy) - nBL.
    T_XX_LIST,
  #endif // XX_LISTS
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
  #ifdef UNPACK_BM_VALUES
    T_UNPACKED_BM, // external bitmap leaf with unpacked value area
  #endif // UNPACK_BM_VALUES
#endif // defined(BITMAP)
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
#if defined(EMBED_KEYS)
    T_EMBEDDED_KEYS, // keys are embedded in the link
#endif // defined(EMBED_KEYS)
    T_SWITCH, // Uncompressed, close (i.e. no-skip) switch.
#if defined(SKIP_LINKS)
    // T_SKIP_TO_SWITCH has to have the biggest value in this enum
    // if not LVL_IN_WR_HB and not LVL_IN_PP.  All of the bigger
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

// Define and optimize nBW_from_nDL, nBW_from_nBL,
// nBL_from_nDL, nBL_from_nDL, et al. based on ifdef parameters.

// NAX means not at top and not at bottom,
// i.e. nBL != cnBitsPerWord and nBL != cnBitsInD1.

// nBW_from_nDL_NAX(_nDL)
#if ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))
  #define nBW_from_nDL_NAX(_nDL)  (cnBitsPerDigit)
  #define nBW_from_nBL_NAX(_nBL)  (cnBitsPerDigit)
#elif (cnBitsInD3 == cnBitsPerDigit)
  #define nBW_from_nDL_NAX(_nDL) \
    (((_nDL) == 2) ? cnBitsInD2 : cnBitsPerDigit)
  #define nBW_from_nBL_NAX(_nBL) \
    (((_nBL) == cnBitsLeftAtDl2) ? cnBitsInD2 : cnBitsPerDigit)
#else // ((cnBitsInD3 == cnBitsPerDigit) && (cnBitsInD2 == cnBitsPerDigit))
  #define nBW_from_nDL_NAX(_nDL) \
    ( ((_nDL) == 2) ? cnBitsInD2 \
    : ((_nDL) == 3) ? cnBitsInD3 \
    : cnBitsPerDigit )
  #define nBW_from_nBL_NAX(_nBL) \
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

#define nBW_from_nBL_NAB(_nBL) \
    ( (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) != 0) \
        && ((_nBL) == cnBitsPerWord) ? cnBWAtTop \
    : ((cnBitsInD2 != cnBitsPerDigit) && ((_nBL) <= 2)) ? cnBitsInD2 \
    : ((cnBitsInD3 != cnBitsPerDigit) && ((_nBL) <= 3)) ? cnBitsInD3 \
    : cnBitsPerDigit )

#if (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)
// cnBWAtTop == cnBitsPerDigit

// nBW_from_nDL(_nDL)
#if (cnBitsInD1 == cnBitsPerDigit)
    #define nBW_from_nDL(_nDL)  (nBW_from_nDL_NAX(_nDL))
    #define nBW_from_nBL(_nBL)  (nBW_from_nBL_NAX(_nBL))
#else // (cnBitsInD1 == cnBitsPerDigit)
    #define nBW_from_nDL(_nDL) \
        ( ((_nDL) <= 1) ? cnBitsLeftAtDl1 : nBW_from_nDL_NAX(_nDL) )
    #define nBW_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsInD1) ? cnBitsLeftAtDl1 \
                                 : nBW_from_nBL_NAX(_nBL) )
#endif // (cnBitsInD1 == cnBitsPerDigit)

// nBL_from_nDL(_nDL)
#define nBL_from_nDL(_nDL)  (nBL_from_nDL_NAT(_nDL))

#else // (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)
// cnBWAtTop != cnBitsPerDigit

  // nBW_from_nDL(_nDL)
  #if (cnBitsInD1 == cnBitsPerDigit)
    #define nBW_from_nDL(_nDL) \
        ( ((_nDL) < cnDigitsPerWord) ? nBW_from_nDL_NAX(_nDL) \
        : cnBWAtTop )
    #define nBW_from_nBL(_nBL) \
        ( ((_nBL) < cnBitsPerWord) ? nBW_from_nBL_NAX(_nBL) \
        : cnBWAtTop )
  #else // (cnBitsInD1 == cnBitsPerDigit)
    // Do we need this to be valid for _nDL < 1?
    #define nBW_from_nDL(_nDL) \
        ( ((_nDL) <= 1) ? cnBitsInD1 \
        : ((_nDL) < cnDigitsPerWord) ? nBW_from_nDL_NAX(_nDL) \
        : cnBWAtTop )
    #define nBW_from_nBL(_nBL) \
        ( ((_nBL) <= cnBitsInD1) ? cnBitsInD1 \
        : ((_nBL) < cnBitsPerWord) ? nBW_from_nBL_NAX(_nBL) \
        : cnBWAtTop )
  #endif // (cnBitsInD1 == cnBitsPerDigit)

    // nBL_from_nDL(_nDL)
    #define nBL_from_nDL(_nDL) \
        ( (_nDL) < cnDigitsPerWord ? nBL_from_nDL_NAT(_nDL) : cnBitsPerWord )

#endif // (((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0)

#if (cnBWAtTop == cnBitsPerDigit)
  #define nBW_from_nBL_NAB3(_nBL)  (cnBitsPerDigit)
#else // (cnBWAtTop == cnBitsPerDigit)
  #define nBW_from_nBL_NAB3(_nBL) \
    (((_nBL) >= cnBitsPerWord) ? cnBWAtTop : (cnBitsPerDigit))
#endif // (cnBWAtTop == cnBitsPerDigit)

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

  #define nDLR_to_nBW(_nDLR)  (anDL_to_nBW[_nDLR])
  #define nDL_to_nBL(_nDL)           (anDL_to_nBL[_nDL])
  #define nBL_to_nDL(_nBL)           (anBL_to_nDL[_nBL])

  #define nBLR_to_nBW(_nBLR)  nDL_to_nBW(nBL_to_nDL(_nBLR))

#else // defined(BPD_TABLE)

  #define nDLR_to_nBW(_nDLR)  (nBW_from_nDL(_nDLR))
  #define nBLR_to_nBW(_nBLR)  (nBW_from_nBL(_nBLR))
  #define nDL_to_nBL(_nDL)           (nBL_from_nDL(_nDL))
  #define nBL_to_nDL(_nBL)           (nDL_from_nBL(_nBL))

#endif // defined(BPD_TABLE)

#define nDL_to_nBL_NAX(_nDL)          (nBL_from_nDL_NAX(_nDL))
#define nBL_to_nBWNAX(_nBL)  (nBW_from_nBL_NAX(_nBL))
#define nBL_to_nBWNAB(_nBL)  (nBW_from_nBL_NAB(_nBL))
#define nDL_to_nBWNAX(_nDL)  (nBW_from_nDL_NAX(_nDL))
#define nDL_to_nBL_NAT(_nDL)          (nBL_from_nDL_NAT(_nDL))
#define nDL_to_nBWNAT(_nDL)  (nDLR_to_nBW(_nDL))

#define nBL_to_nBWNAB(_nBL)  (nBW_from_nBL_NAB(_nBL))

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
  #if ! defined(cwDebugThresholdMax)
    #define cwDebugThresholdMax  0ULL
  #endif // ! defined(cwDebugThresholdMax)
#else // defined(DEBUG)
  #define DBG(x)
#endif // defined(DEBUG)

#if defined(DEBUG_INSERT)
  #if (cwDebugThreshold || cwDebugThresholdMax)
    #define DBGI(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGI(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_INSERT)
  #define DBGI(x)
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_LOOKUP)
  #if (cwDebugThreshold || cwDebugThresholdMax)
    #define DBGL(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGL(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_LOOKUP)
  #define DBGL(x)
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_REMOVE)
#if (cwDebugThreshold || cwDebugThresholdMax)
#define DBGR(x)  if (bHitDebugThreshold) (x)
#else // (cwDebugThreshold != 0)
#define DBGR(x)  (x)
#endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_REMOVE)
#define DBGR(x)
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_COUNT)
  #if (cwDebugThreshold || cwDebugThresholdMax)
    #define DBGC(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGC(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_COUNT)
  #define DBGC(x)
#endif // defined(DEBUG_COUNT)

#if defined(DEBUG_NEXT)
  #if (cwDebugThreshold || cwDebugThresholdMax)
    #define DBGN(x)  if (bHitDebugThreshold) (x)
  #else // (cwDebugThreshold != 0)
    #define DBGN(x)  (x)
  #endif // (cwDebugThreshold != 0)
#else // defined(DEBUG_NEXT)
  #define DBGN(x)
#endif // defined(DEBUG_NEXT)

#if defined(DEBUG_MALLOC)
#if (cwDebugThreshold || cwDebugThresholdMax)
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

// COPYX is for copying keys betweeen lists with different size keys.
#define COPYX(_pTgt, _pSrc, _nCnt) \
{ \
    for (int nn = 0; nn < (int)(_nCnt); ++nn) { \
        (_pTgt)[nn] = (_pSrc)[nn]; \
    } \
}

#define COPY(_pTgt, _pSrc, _cnt) \
{ \
    if (sizeof(*(_pTgt)) == sizeof(*(_pSrc))) { \
        memcpy((_pTgt), (_pSrc), sizeof(*(_pSrc)) * (_cnt)); \
    } else { \
        COPYX(_pTgt, _pSrc, _cnt); \
    } \
}

#define MOVE(_pTgt, _pSrc, _cnt) \
    memmove((_pTgt), (_pSrc), sizeof(*(_pSrc)) * (_cnt))

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
  #if ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
    if (nType >= T_SKIP_TO_SWITCH) { Set_nType(pwRoot, T_SWITCH); return; }
  #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
    switch (nType) {
  #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
    case T_SKIP_TO_SWITCH: Set_nType(pwRoot, T_SWITCH); break;
  #endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
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

// Notes on ZERO_POP_MAGIC and NO_TYPE_IN_XX_SW:
//
// NO_TYPE_IN_XX_SW means no type in XX_SW links with nBL < DL2.
//
// We need some way to represent an empty list when we have no type field,
// i.e. for embedded keys with NO_TYPE_IN_XX_SW.
// Can we find a value that is not otherwise a valid list of embedded keys?
// A single value that is the same for all key sizes woud be ideal.
//
// Consider that we bitwise pack and sort the embedded list with the smallest
// key at the most significant end of the word and pad empty slots and the
// remainder bits with zero. This makes it easy to calculate the population
// so we don't have to store it.
//
// Zero is not a candidate for our invalid value because it is a valid wRoot
// for an embedded list with a single key and that key being zero.
// But 0x800000004xxxxxxx is an invalid value for wRoot for any key size less
// than or equal to one half the size of a word. It is invalid because the
// 0x40000000 bit implies there are at least two keys in the list, and if the
// high bit is one in the smallest key in the list then it must also be one
// for the second key, but with this number the msb of the second key in the
// list is zero.
//
// For keys up to 63 bits we can create an invalid value by setting the least
// significant bit in the word because we are zero-filling valid lists.
// So we can create a magic number that supports up to 60-bit keys and use
// any valid type value in the type field that has at least one bit set.
// Or we can support up to 59-bit keys and use any type field value by setting
// the bit just above the type field.
//
//       f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
// 0b 1000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxxxx1<ek>
// 0b 1000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxxxx0<ek>
//
// I wonder if it makes sense to use this value in wRoot for a link with an
// empty expanse even in non-XX_SW switches and even w/o NO_TYPE_IN_XX_SW.
//
// Can we do even better than simply being able to use the whole word
// for embedded keys without wasting any bits on a type field or pop count?
// And devise a test that would allow us to also have a regular link with
// a type field and pointer and maybe even other other stuff in the word?
// If we limit ourselves to keys of size 7 bits to 14 bits we could use
// the following:
//
//       f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
// 0b 1xxxxxxx0x0x0001xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx<ek>
//
// This number is invalid for 7-bit keys because it implies the existence
// of a third key with the high bit being 0.
// It is invalid for 8-14-bit keys because it implies the existence of
// a second key with the high bit being 0.
//
// We have a whopping 9 bits for extending the type field.
//
// Could we support larger keys? Or get more spare bits? Could we coopt
// swaths of pointer values that we know won't be used for our objects?
// Would it make sense to increase the alignment requirement for objects
// pointed to by an XX_SW. Or get them from a specific malloc arena.

// Consider if we were to bitwise pack and sort the embedded list with the
// smallest key at the most significant end of the word and pad empty slots
// with the biggest key in the list and zero fill any left over bits.
// We don't do it that way, but consider if we did.
//
// Is there a fast way to calculate the population of a list padded with the
// biggest key?
//
// Zero is not a candidate for our invalid value because it is a valid wRoot
// for an embedded list with a single key and that key being zero.
// But 0x80..0 is an invalid value for wRoot for any key size less than or
// equal to one half the size of a word. It is invalid because if the high
// bit is one in the smallest key in the list then it must also be one for
// all other keys or empty slots in the rest of the list. In fact, we could
// choose any number for the magic number with the the most significant half
// of the word being all zeroes except the most significant bit being one
// and the least significant half of the word being any value with the most
// sigificant bit being zero.
// Furthermore, if the smallest keys we ever put in the list for a 64-bit
// word are seven bits, then the magic number could have anything in the
// six least significant bits of the first key slot.
//
//       f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
// 0b 1xxxxxx000000000xxxxxxxxxxxxxxxx0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
// #define MAGIC  EXP(cnBitsInWord - 1)
// #define LINK_IS_EMPTY(_wRoot)  ((_wRoot) == MAGIC)
//
// There are a lot of options for choosing a magic number that is the same
// for all key sizes less than or equal to one half the size of a word.
// But we can do better. Consider keys that are larger than one half
// the size of a word. Since we have no type field and we are zero filling
// the unused bits at the least significant end of the word, if our magic
// number has a one in the least significant bit it becomes an invalid value
// for any key size up to one bit less than the size of a word.
// We have 2**36 values to choose from for a 64-bit word. 30 don't care bits
// in the low half of the word and 6 don't care bits in the high half of
// the word.

// In a 64-bit wRoot that does not contain embedded keys  we use the least
// significant bits, cnMallocMask, for a type field and the next most
// significant 44 bits as a malloc-aligned pointer. If we want to allow the
// 48 pointer and type bits to assume any value, then we must devise a test
// using the most significant 16 bits to tell us we have a link without
// embedded keys. Any 64-bit number with a one in the most significant bit,
// bit 63, and zeros in bits 48-56 is an invalid value for keys of 15-bits
// or less.
//
//       f   e   d   c   b   a   9   8   7   6   5   4   3   2   1   0
// 0b 1xxxxxx000000000xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
// #define HAS_TYPE(_wRoot)  (((_wRoot) & (0x81ff << 48)) == MAGIC)
//
// That leaves us bits 57-62 for whatever other purpose we want.
// If six bits isn't enough for augmenting the type field we could use
// a more expensive test and get that number up to 14 bits for extending
// the type field (exactly which eight extra bits would depend on nBL).
// And remember that we might be able to coopt
// T_EMBEDDED_KEYS for a different use in this type of link.
//
// #define MAGIC_MSK(_nBL)  ((EXP(_nBL) | 1) << (cnBitsPerWord - (_nBL) - 1))
// #define HAS_TYPE(_wRoot, _nBL)  (((wRoot) & FAST_MAGIC_MSK(_nBL)) == MAGIC)
// #define FAST_XTYPE_MSK(_nBL)  ~(MAGIC_MSK(_nBL) | MSK(48))
// #define FAST_XTYPE_VAL(_wRoot, _nBL)  ((_wRoot) & XTYPE_MSK(_nBL))
//
// It would be possible to extend this functionality to support slightly
// larger keys in exchange for slightly fewer bits for type and extended type.
// Or we could extend it if we could somehow coopt swaths of addresses that
// we know we would never use for a pointer to an object in our array.

// I wonder if we could use this encoding for all links rather than only
// those with smaller nBL in an XX_SW.
// We'd be limited to 14 extended type bits instead of our current 16 for
// nBL < 16. It would be a little more expensive to get them since the exact
// bits would be dependent on nBL.
// If we choose MAGIC and T_EMBEDDED_KEYS such that
// gnType(MAGIC) == T_EMBEDDED_KEYS we'd only have to check for an empty
// link in the T_EMBEDDED_KEYS case.
// But our MAGIC would require the least significant bit above the type
// field be one and it would limit our key size for embedded keys to
// 64 - 4 - 1 = 59 bits. If we need that 60th bit then we could consider
// using a different type value.
//
// Any link type that uses only six of the high bits would suffer no
// performance penalty.
// For link types that can exist at nBL < 16 we'd have to use HAS_TYPE
// For link types that can exist at nBL < 16 and nBL >= 16 we'd have to
// test nBL and HAS_TYPE.

// Our old embedded list with a type field and a 3-bit pop count where
// pop-field=0 means pop=1 cannot represent an empty list using only the
// pop count.  In that case we could represent an empty list for nBL small
// enough that two keys will fit with a magic number.
#ifdef WROOT_NULL_IS_EK
  #if defined(REVERSE_SORT_EMBEDDED_KEYS)
    #define ZERO_POP_MAGIC  1
  #else // defined(REVERSE_SORT_EMBEDDED_KEYS)
    #define ZERO_POP_MAGIC \
        (((Word_t)0x800000004 << 28) + T_EMBEDDED_KEYS)
  #endif // #else defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // WROOT_NULL_IS_EK

// Default for WROOT_NULL is 0 if no SEPARATE_T_NULL.
// It will be handled by whatever type is 0.
// wRoot == 0 must be an otherwise unused value for the type.
#if defined(SEPARATE_T_NULL)
  #define WROOT_NULL  T_NULL
#elif defined(WROOT_NULL_IS_EK) // defined(SEPARATE_T_NULL)
  #ifndef EMBED_KEYS
    #error WROOT_NULL_IS_EK without EMBED_KEYS
  #endif // #ifndef EMBED_KEYS
  #define WROOT_NULL  ZERO_POP_MAGIC
#elif defined(WROOT_NULL_IS_LIST) // #elif defined(WROOT_NULL_IS_EK)
  #if (cwListPopCntMax == 0)
    #error WROOT_NULL_IS_LIST with cwListPopCntMax == 0
  #endif // (cwListPopCntMax == 0)
  #define WROOT_NULL  T_LIST
#elif defined(WROOT_NULL_IS_BITMAP) // #elif defined(WROOT_NULL_IS_LIST)
  #ifndef BITMAP
    #error WROOT_NULL_IS_BITMAP without BITMAP
  #endif // #ifndef BITMAP
  #define WROOT_NULL  T_BITMAP
#elif defined(WROOT_NULL_IS_SWITCH) // #elif defined(WROOT_NULL_IS_BITMAP)
  #define WROOT_NULL  T_SWITCH
#else // #elif defined(WROOT_NULL_IS_SWITCH)
  #define WROOT_NULL  0
#endif // MAGIC_WROOT_NULL

#if defined(NO_TYPE_IN_XX_SW) // && defined(HANDLE_BLOWOUTS)
    // Identify blowouts using (wRoot & BLOWOUT_MASK(nBL) == ZERO_POP_MAGIC).
    #define BLOWOUT_MASK(_nBL) \
        ((EXP(_nBL) + 1) << (cnBitsPerWord - (_nBL) - 1))
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
  #endif // #else defined(NO_TYPE_IN_XX_SW)
    {
        // The code below assumes the pop count is not zero.
        // Why is it ok to assume the link is non-empty here but not for the
        // NO_TYPE_IN_XX_SW with (nBL >= nDL_to_nBL(2)) case above?
        assert(wRoot != WROOT_NULL);
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
    int nKeysMax = (nBL <= (cnBitsPerWord - nBitsOverhead));
  #else // B_JUDYL
    int nKeysMax = (cnBitsPerWord - nBitsOverhead) / nBL;
  #endif // B_JUDYL
  #ifdef POP_CNT_MAX_IS_KING
    if (auListPopCntMax[nBL] < nKeysMax) {
        nKeysMax = auListPopCntMax[nBL];
    }
  #endif // POP_CNT_MAX_IS_KING
    return nKeysMax;
}

#endif // defined(EMBED_KEYS)

static inline int
tp_bIsSwitch(int nType)
{
#if ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
  #if defined(SKIP_LINKS)
    if (nType >= T_SKIP_TO_SWITCH) { return 1;}
  #endif // defined(SKIP_LINKS)
#endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
    switch (nType) {
    case T_SWITCH:
#if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
  #if defined(SKIP_LINKS)
    case T_SKIP_TO_SWITCH:
  #endif // defined(SKIP_LINKS)
#endif // defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
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
  #define LIST_SW(_x)  _x
#else // defined(CODE_LIST_SW)
  #define tp_bIsListSw(_tp)  0
  #define LIST_SW(_x)
#endif // #else defined(CODE_LIST_SW)

#if defined(CODE_BM_SW)
    // Is (_tp) a bitmap switch or skip to one?
  #if defined(SKIP_TO_BM_SW)
    #define tp_bIsBmSw(_tp)  (((_tp) == T_BM_SW) || ((_tp) == T_SKIP_TO_BM_SW))
  #else // defined(SKIP_TO_BM_SW)
    #define tp_bIsBmSw(_tp)  ((_tp) == T_BM_SW)
  #endif // defined(SKIP_TO_BM_SW)
  #define BM_SW(_x)  _x
#else // defined(CODE_BM_SW)
  #define tp_bIsBmSw(_tp)  0
  #define BM_SW(_x)
#endif // #else defined(CODE_BM_SW)


#if defined(CODE_XX_SW)
    // Is (_tp) a doubling switch or skip to one?
  #if defined(SKIP_TO_XX_SW)
    #define tp_bIsXxSw(_tp)  (((_tp) == T_XX_SW) || ((_tp) == T_SKIP_TO_XX_SW))
  #else // defined(SKIP_TO_XX_SW)
    #define tp_bIsXxSw(_tp)  ((_tp) == T_XX_SW)
  #endif // defined(SKIP_TO_XX_SW)
  #define XX_SW(_x)  _x
#else // defined(CODE_XX_SW)
  #define tp_bIsXxSw(_tp)  0
  #define XX_SW(_x)
#endif // #else defined(CODE_XX_SW)

static inline int
tp_bIsSkip(int nType)
{
    (void)nType;
  #if defined(SKIP_LINKS)
    switch (nType) {
    case T_SKIP_TO_SWITCH:
      #if defined(SKIP_TO_LIST_SW)
    case T_SKIP_TO_LIST_SW:
      #endif // defined(SKIP_TO_LIST_SW)
      #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW:
      #endif // defined(SKIP_TO_BM_SW)
      #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW:
      #endif // defined(SKIP_TO_XX_SW)
      #ifdef SKIP_TO_LIST
    case T_SKIP_TO_LIST:
      #endif // SKIP_TO_LIST
      #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP:
      #endif // defined(SKIP_TO_BITMAP)
        return 1;
      #ifndef LVL_IN_WR_HB
      #ifndef LVL_IN_PP
    default: return (nType > T_SKIP_TO_SWITCH);
      #endif // #ifndef LVL_IN_PP
      #endif // #ifndef LVL_IN_WR_HB
    }
  #endif // defined(SKIP_LINKS)
    return 0;
}

// External list. T_EMBEDDED_KEYS does not qualify.
static inline int
tp_bIsList(int nType)
{
    return
  // Even though _cnListPopCntMax3 might be zero we still have to concern
  // ourselves with temporarily inflated lists.
  #if (cwListPopCntMax > 0)
        ((nType == T_LIST)
      #ifdef UA_PARALLEL_128
             || (nType == T_LIST_UA)
      #endif // UA_PARALLEL_128
      #ifdef SKIP_TO_LIST
             || (nType == T_SKIP_TO_LIST)
      #endif // SKIP_TO_LIST
      #ifdef XX_LISTS
             || (nType == T_XX_LIST)
      #endif // XX_LISTS
         )
  #else // (cwListPopCntMax > 0)
        0
  #endif // #else (cwListPopCntMax > 0)
        ;
}

static inline int
tp_bIsBitmap(int nType)
{
    (void)nType;
    return
  #if defined(BITMAP)
        ((nType == T_BITMAP)
  #ifdef UNPACK_BM_VALUES
             || (nType == T_UNPACKED_BM)
  #endif // UNPACK_BM_VALUES
  #ifdef SKIP_TO_BITMAP
             || (nType == T_SKIP_TO_BITMAP)
  #endif // SKIP_TO_BITMAP
         )
  #else // defined(BITMAP)
        0
  #endif // #else defined(BITMAP)
        ;
}

#ifdef B_JUDYL
  #ifdef UNPACK_BM_VALUES
  #ifdef PACK_BM_VALUES
  #if (cnBitsPerWord > 32)
    #define _TEST_BM_UNPACKED
  #endif // (cnBitsPerWord > 32)
  #endif // PACK_BM_VALUES
  #endif // UNPACK_BM_VALUES

  #ifdef _TEST_BM_UNPACKED
    #define BM_UNPACKED(_wRoot)  (wr_nType(_wRoot) == T_UNPACKED_BM)
  #elif defined(UNPACK_BM_VALUES) // _TEST_BM_UNPACKED
    #define BM_UNPACKED(_wRoot)  1
  #else // #elif defined(UNPACK_BM_VALUES) // _TEST_BM_UNPACKED
    #define BM_UNPACKED(_wRoot)  0
  #endif // #else // #elif defined(UNPACK_BM_VALUES) // _TEST_BM_UNPACKED
#endif // B_JUDYL

// Bit fields in the upper bits of of wRoot.
#define cnBitsCnt  8
#define cnLsbCnt  (cnBitsPerWord - cnBitsCnt)

// (cnBitsLvlM1, cnLsbLvlM1) is the level of the node pointed to.
#define cnBitsLvlM1  8
#define cnLsbLvlM1  cnBitsVirtAddr

// ListPopCnt is the number of keys in the list minus 1.
#define cnBitsListPopCnt  cnBitsCnt
#define cnLsbListPopCnt   cnLsbCnt

// XxSwWidth is the log of the number of virtual links in the switch.
// For 32-bit, cn[Bits|Lsb]XxSwWidth applies to the preamble
// word -- not to wRoot.
#define cnBitsXxSwWidth  cnBitsCnt
#define cnLsbXxSwWidth   cnLsbCnt

// ListSwPopM1 is the number of links in the list switch minus one.
#define cnBitsListSwPopM1  cnBitsCnt // for T_LIST_SW
#define cnLsbListSwPopM1   cnLsbCnt

#if (cn2dBmMaxWpkPercent != 0) && (cnBitsLeftAtDl2 > cnBitsCnt)
  #undef  BM_POP_IN_WR_HB
#endif // (cn2dBmMaxWpkPercent != 0) && (cnBitsLeftAtDl2 > cnBitsCnt)
#if (cnBitsInD1 > cnBitsCnt)
  #undef BM_POP_IN_WR_HB
#endif // (cnBitsInD1 > cnBitsCnt)

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

    #define wr_nBLR(_wr) \
        (assert(tp_bIsSkip(wr_nType(_wr))), \
            (int)GetBits((_wr), cnBitsLvlM1, cnLsbLvlM1) + 1)

  #define wr_nDLR(_wr)  nBL_to_nDL(wr_nBLR(_wr))

  #define set_wr_nBLR(_wr, _nBLR) \
      (assert((_nBLR) <= (int)MSK(cnBitsLvlM1)), \
          assert(tp_bIsSkip(wr_nType(_wr))), \
          SetBits(&(_wr), cnBitsLvlM1, cnLsbLvlM1, (_nBLR) - 1))

  #define set_wr_nDLR(_wr, _nDLR)  set_wr_nBLR((_wr), nDL_to_nBL(_nDLR))

#else // defined(LVL_IN_WR_HB)

#if defined(LVL_IN_PP)
// LVL_IN_PP directs us to use the low bits of sw_wPrefixPop for absolute
// depth instead of encoding it into the type field directly.
// It means we can't use the low bits of sw_wPrefixPop for pop.  So we
// define POP_WORD and use a separate word.
// We assume the value we put into the low bits will will fit in the number
// of bits used for the pop count at nDL == 2.  Or maybe
// it doesn't matter since we always create an embedded bitmap when
// EXP(nBL) <= sizeof(Link_t) * 8.
// Why not simply use a separate word for LVL_IN_PP instead of the more
// complicated approach of displacing pop from sw_wPrefixPop? We will be
// able to combine it with the word we are planning to add for memory usage
// of the subtree, sw_wMem.
// Is it because we want to be able to put lvl in the PrefixPop word for
// PP_IN_LINK?
#define POP_WORD

// We coopt the pop field in sw_wPrefixPop and use it for absolute level if
// LVL_IN_PP. This seems risky from a code maintenance perspective. Some
// renaming might be in order.
// We assume wr_n[BD]L and set_wr_n[BD]L are used only when it is
// known that we have a skip link.  We could enhance it to use one type value
// to indicate that we have to go to sw_wPrefixPop and use any other values
// that we have available to represent some key absolute levels.
// But why? Is there a performance win since we have to look at the
// prefix word anyway.
// Should we enhance wr_nDL to take pwRoot and wRoot and nDL?
  #define wr_nBLR(_wr) \
      ((int)(assert(tp_bIsSkip(wr_nType(_wr))), \
       w_wPopCntBL(PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)), \
                   cnBitsLeftAtDl2)))

  #define wr_nDLR(_wr)  nBL_to_nDL(wr_nBLR(_wr))

  #define set_wr_nBLR(_wr, _nBLR) \
      (assert(tp_bIsSkip(wr_nType(_wr))), \
          (PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
              = ((PWR_wPrefixPop(NULL, (Switch_t *)wr_pwr(_wr)) \
                      & ~wPrefixPopMaskBL(cnBitsLeftAtDl2)) \
                  | (_nBLR))))

  #define set_wr_nDLR(_wr, _nDLR)  set_wr_nBLR((_wr), nDL_to_nBL(_nDLR))

#else // defined(LVL_IN_PP)

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
// lookup table, ...  But why?  We're going to use LVL_IN_PP.  This code
// is an anachronism.
  #define tp_to_nDLR(_tp)   ((_tp)  - T_SKIP_TO_SWITCH + 2)
  #define nDL_to_tp(_nDL)  ((_nDL) + T_SKIP_TO_SWITCH - 2)

  #define wr_nDLR(_wr) \
      (assert(tp_bIsSkip(wr_nType(_wr))), tp_to_nDLR(wr_nType(_wr)))

  #define wr_nBLR(_wr)  nDL_to_nBL(tp_to_nDLR(wr_nType(_wr)))

  #define set_wr_nDLR(_wr, _nDLR) \
      (assert(nDL_to_tp(_nDLR) >= T_SKIP_TO_SWITCH), \
       set_wr_nType((_wr), nDL_to_tp(_nDLR)))

  #define set_wr_nBLR(_wr, _nBLR) \
      set_wr_nDLR((_wr), nBL_to_nDL(_nBLR))

#endif // defined(LVL_IN_PP)

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
#define w_wPopCntNATBL(_w, _nBL)  ((_w) &  wPrefixPopMaskNotAtTopBL(_nBL))

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#ifndef POP_IN_WR_HB
#ifndef LIST_POP_IN_PREAMBLE
  // List leaf pop is in link but not in ln_wRoot hence not at top.
  #define _LIST_POP_IN_LINK_X
#endif // #ifndef LIST_POP_IN_PREAMBLE
#endif // #ifndef POP_IN_WR_HB
#ifdef BITMAP
  // Bitmap leaf pop is in link but not in ln_wRoot.
  #define _BM_POP_IN_LINK_X
#endif // BITMAP
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// _LIST_POP_IN_LINK_X doesn't need to work with !OLD_LISTS.
// The whole purpose of new lists was to move pop count to the end of
// the list so we don't have to waste a bucket at the beginning just for
// pop count.  But we don't put the pop count in the list for
// _LIST_POP_IN_LINK_X. Except at the top.
// We'll still be wasting that bucket for _LIST_POP_IN_LINK_X if
// we are aligning word-size lists.
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #if ! defined(OLD_LISTS)
#error Sorry, PP_IN_LINK and POP_WORD_IN_LINK require OLD_LISTS.
  #endif // ! defined(OLD_LISTS)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// It is a bit of a bummer that the macros for extracting fields that might
// be in the switch or in the link depending on ifdefs require a mask and
// an extra dereference in one of the cases if the only parameter is pwRoot.
// It would be nice if the compiler could optimize them out, but I'm not
// optimistic so I chose to make both pwRoot and pwr be parameters.
// Only one will be used, for each field, in the compiled code, depending
// on ifdefs.
// Default is -UPP_IN_LINK, i.e. -DPP_IN_SWITCH.
#if defined(PP_IN_LINK)

#define PWR_wPrefixPop(_pwRoot, _pwr) \
    (STRUCT_OF((assert(wr_nType(*(_pwRoot) != T_EMBEDDED_KEYS)), (_pwRoot)), \
               Link_t, ln_wRoot)->ln_wPrefixPop)

  #if defined(LVL_IN_PP) || defined(POP_WORD)
// The main purpose of new lists was to move pop count to the end of
// the list so we don't have to waste a bucket at the beginning just for
// pop count. But we don't put the pop count in the list for PP_IN_LINK
// Except at the top.  We're not currently aligning word-size
// lists because we're not currently doing parallel search for lists
// with full word size key slots.
// Relocating the pop out of PP requires quite a few code changes.
// It would be nice for depth, prefix and pop to share the same word.
#error Sorry, no PP_IN_LINK && (LVL_IN_PP || POP_WORD).
  #endif // defined(LVL_IN_PP) || defined(POP_WORD)

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
    (PWR_wPopWord((_pwRoot), (_pwr)))

#define PWR_wPopCntBL(_pwRoot, _pwr, _nBL) \
    (PWR_wPopWordBL((_pwRoot), (_pwr), (_nBL)))

#define set_PWR_wPopCnt(_pwRoot, _pwr, _nDL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) = (_cnt))

#define set_PWR_wPopCntBL(_pwRoot, _pwr, _nBL, _cnt) \
    (PWR_wPopWord((_pwRoot), (_pwr)) = (_cnt))

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

#define set_w_wPrefixNATBL(_w, _nBL, _key) \
    ((_w) = (((_w) & wPrefixPopMaskNotAtTopBL(_nBL)) \
            | ((_key) & ~wPrefixPopMaskNotAtTopBL(_nBL))))

#define set_w_wPopCntNotAtTop(_w, _nDL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMaskNotAtTop(_nDL)) \
            | ((_cnt) & wPrefixPopMaskNotAtTop(_nDL))))

#define set_w_wPopCntNATBL(_w, _nBL, _cnt) \
    ((_w) = (((_w) & ~wPrefixPopMaskNotAtTopBL(_nBL)) \
            | ((_cnt) & wPrefixPopMaskNotAtTopBL(_nBL))))

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
// It may be before or after the slots used for keys.
// There is a problem if it is at the beginning of the list and
// we're aligning lists. Our code doesn't account for aligning the
// list again after the pop slot.
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    // Be careful: _nBL here is BEFORE any skip is applied
    // Do we allow skip from top for PP_IN_LINK? Looks like we allow
    // skip if prefix is zero. It means we'd need a pop slot.
    // But do we allow/use skip to list from the top?
  #if defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
    #define POP_SLOT(_nBL)  0
  #else // defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
    #define POP_SLOT(_nBL) \
        (((_nBL) >= cnBitsPerWord) && (cnDummiesInList == 0))
  #endif // #else defined(POP_IN_WR_HB) || defined(LIST_POP_IN_PREAMBLE)
#else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #if defined(OLD_LISTS)
    // N_HDR_KEYS incorporates this for ! PP_IN_LINK so don't add it again.
    #define POP_SLOT(_nBL)  (0)
  #else // defined(OLD_LISTS)
    #define POP_SLOT(_nBL)  (1)
  #endif // defined(OLD_LISTS)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// ALIGN_LIST is about the alignment of the first real key in a list leaf
// to a sizeof(Bucket_t) boundary.
// The first real key in a list leaf may follow a pop count and/or an
// end-of-list marker key and/or dummy keys and/or whatever else we decide
// to implement or experiment with.
// One reason for doing this alignment is to satisfy any alignment
// requirement of the list search functions, e.g. the fastest load 128-bit
// register instruction requires 128-bit alignment.
#define ALIGN_LIST(_nBytesKeySz) \
    ( cbAlignLists /* independent of parallel search */ \
        || ( (_nBytesKeySz) == cnBytesPerWord \
            ? cbParallelSearchWord : cbPsplitParallel ) )

// ALIGN_LIST_LEN is about the length of the key area in a list.
// The length of a list from the first key through the last (including unused
// slots filled with the last real key in the list) must be an integral
// number of parallel search buckets so we don't need any special handling in
// the parallel search code to handle a partial bucket at the end.
// This is independent of any header or footer in the list or any alignment
// of the first key in the list, e.g. if ALIGN_LIST allows a list to start on
// an odd word boundary and ALIGN_LIST_LEN requires the list to be an integral
// number of buckets long, then the end of the list may fall on an odd word
// boundary.
#ifdef B_JUDYL
  // Low pop cnt JudyL lists are very memory inefficient.
  // Hence cnParallelSearchWordPopCntMinL.
  #ifndef cnParallelSearchWordPopCntMinL
    #define cnParallelSearchWordPopCntMinL  6
  #endif // cnParallelSearchWordPopCntMinL
  #define ALIGN_LIST_LEN(_nBytesKeySz, _nPopCnt) \
    (cbAlignListLens /* independent of parallel search */ \
        || ((_nBytesKeySz) == cnBytesPerWord \
            ? (cbParallelSearchWord \
                && ((_nPopCnt) >= cnParallelSearchWordPopCntMinL)) \
            : cbPsplitParallel))
#else // B_JUDYL
  #define ALIGN_LIST_LEN(_nBytesKeySz, _nPopCnt) \
    ( cbAlignListLens /* independent of parallel search */ \
        || ( (_nBytesKeySz) == cnBytesPerWord \
            ? cbParallelSearchWord : cbPsplitParallel ) )
#endif // // B_JUDYL

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

  #ifndef LIST_POP_IN_PREAMBLE

// Use ls_sPopCnt in the performance path when we know the keys are bigger
// than one byte.
#define     ls_sPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_asKeys[0])
#define set_ls_sPopCnt(_ls, _cnt)  (ls_sPopCnt(_ls) = (_cnt))

// Use ls_cPopCnt in the performance path when we know the keys are one byte.
// PopCnt fits in a single key slot.
#define     ls_cPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_acKeys[0])
#define set_ls_cPopCnt(_ls, _cnt)  (ls_cPopCnt(_ls) = (_cnt))

  #endif // LIST_POP_IN_PREAMBLE

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

  #ifndef OLD_LIST_WORD_CNT
      #if (N_LIST_HDR_KEYS != 0)
#error ListWordCnt cannot handle N_LIST_HDR_KEYS != 0.
      #endif // (N_LIST_HDR_KEYS != 0)
  #endif // OLD_LIST_WORD_CNT

// NAT is relevant only for _LIST_POP_IN_LINK_X where POP_SLOT at the
// beginning of the list depends on whether we are at the top or not.
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

  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#define ls_piKeys(_ls, _nBL)  ls_piKeysNAT(_ls)
#define ls_psKeys(_ls, _nBL)  ls_psKeysNAT(_ls)
#define ls_pcKeys(_ls, _nBL)  ls_pcKeysNAT(_ls)

  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#ifdef _LIST_POP_IN_LINK_X
  #if ALIGN_LIST(cnBytesPerWord) // ALIGN_LIST(cnBytesPerKey)
    #define ls_pwKeys(_ls, _nBL) \
        ((Word_t *)ALIGN_UP((Word_t)(ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL)), \
                            sizeof(Bucket_t)))
  #else // ALIGN_LIST(cnBytesPerWord)
    #define ls_pwKeys(_ls, _nBL)  (ls_pwKeysNAT_UA(_ls) + POP_SLOT(_nBL))
  #endif // #else ALIGN_LIST(cnBytesPerWord)
#else // _LIST_POP_IN_LINK_X
  #define ls_pwKeys(_ls, _nBL)  ls_pwKeysNAT(_ls)
#endif // #else _LIST_POP_IN_LINK_X

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
// Do we ever use ll_asKeys or ll_acKeys for list pop count or anything
// else if !defined(COMPRESSED_LISTS)?
#if defined(COMPRESSED_LISTS)
        uint16_t ll_asKeys[0];
#endif // defined(COMPRESSED_LISTS)
#if defined(COMPRESSED_LISTS)
        uint8_t  ll_acKeys[0];
  #if (cnBitsPerWord > 32)
        uint32_t ll_aiKeys[0];
  #endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        Word_t   ll_awKeys[0];
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
// POP_WORD is defined automatically if LVL_IN_PP is defined.

typedef struct {
#ifndef DEBUG
    Word_t ln_wRoot;
#endif // #ifndef DEBUG
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
#ifdef DEBUG
    Word_t ln_wRoot;
#endif // #ifdef DEBUG
} Link_t;

#define cnLogBitsPerLink  ((int)LOG(sizeof(Link_t)) + cnLogBitsPerByte)

#ifdef USE_XX_SW_ONLY_AT_DL2
  #ifndef ALLOW_EMBEDDED_BITMAP
    #error USE_XX_ONLY_AT_DL2 requires ALLOW_EMBEDDED_BITMAP
  #endif // #ifndef ALLOW_EMBEDDED_BITMAP
  #define cbEmbeddedBitmap  1
#elif defined(ALLOW_EMBEDDED_BITMAP)
  #define cbEmbeddedBitmap  (cnBitsInD1 <= cnLogBitsPerLink)
#else // #ifdef USE_XX_SW_ONLY_AT_DL2 #elif defined(ALLOW_EMBEDDED_BITMAP)
  #define cbEmbeddedBitmap  0
#endif // ALLOW_EMBEDDED_BITMAP

// Get the width of the switch in bits.
// nBLR includes any skip specified in the qp link.
static inline int
gnBW(qp, int nBLR)
{
    // nType may not be wr_nType(wRoot).
    // It may be T_XX_SW for (wr_nType(wRoot) == T_SKIP_TO_XX_SW) or
    // it may be T_SWITCH for (wr_nType(wRoot) == T_SKIP_TO_SWITCH) or ...
    // It is a silly performance hack for Lookup where we use a literal in
    // case T_XX_SW and T_SWITCH and ... because we have already 'switch'ed
    // on nType.
    qv; (void)nBLR;
    int nBW;
  #ifdef CODE_XX_SW
    if ((nType == T_XX_SW)
      #ifdef SKIP_TO_XX_SW
        || (nType == T_SKIP_TO_XX_SW)
      #endif // SKIP_TO_XX_SW
        )
    {
        if (cnBitsPerWord == 64) {
            // WIDTH_IN_WR_HB
            nBW = GetBits(wRoot, cnBitsXxSwWidth, cnLsbXxSwWidth);
        } else {
            // use the malloc preamble word
            nBW = GetBits(pwr[-1], cnBitsXxSwWidth, cnLsbXxSwWidth);
        }
    } else
  #endif // CODE_XX_SW
    { nBW = nBLR_to_nBW(nBLR); }
    return nBW;
}

#define Get_nBW(_pwRoot) \
    gnBW(/*nBL*/ 0, STRUCT_OF((_pwRoot), Link_t, ln_wRoot), /*nBLR*/ 0)

#define pwr_nBW  Get_nBW

// Set the width of the branch in bits.
static inline void
snBW(qp, int nBW)
{
    qv;
  #if BPW > 32
    assert(nBW <= (int)MSK(cnBitsXxSwWidth));
  #endif // BPW > 32
    if (cnBitsPerWord == 64) {
        // WIDTH_IN_WR_HB
        SetBits(&pLn->ln_wRoot, cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
    } else {
        // use the malloc preamble word
        SetBits(&pwr[-1], cnBitsXxSwWidth, cnLsbXxSwWidth, nBW);
    }
}

#define Set_nBW(_pwRoot, _nBW) \
    snBW(/*nBL*/ 0, STRUCT_OF((_pwRoot), Link_t, ln_wRoot), /*nBW*/ (_nBW))

#define set_pwr_nBW  Set_nBW

#if defined(SW_LIST_IN_LINK)
    #define SW_LIST
#else // defined(SW_LIST_IN_LINK)
    #define SW_LIST  uint8_t sw_aKeys[1<<(sizeof(uint8_t)*8)];
#endif // defined(SW_LIST_IN_LINK)

#if !defined(PP_IN_LINK) && (defined(SKIP_LINKS) || !defined(POP_WORD))
    #define SW_PREFIX_POP  Word_t sw_wPrefixPop;
#else // !defined(PP_IN_LINK) && (defined(SKIP_LINKS) || !defined(POP_WORD))
    // We don't need sw_wPrefixPop if we don't have SKIP_LINKS and
    // we do have POP_WORD so we're not using sw_wPrefixPop for pop count.
    #define SW_PREFIX_POP
#endif // #else !def(PP_IN_LINK) && (def(SKIP_LINKS) || !def(POP_WORD))

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
// Also used for BmSwitch_t. Bitmap resides in front of SW_COMMON_HDR.
typedef struct {
    SW_COMMON_HDR
    Link_t sw_aLinks[0]; // variable size
} Switch_t;

// List switch.
// A switch with a list of subkeys the specifies the links that are
// present in the switch.
typedef struct {
    SW_COMMON_HDR
    SW_LIST // variable size so sw_aLinks follows this
    // Link_t sw_aLinks[]; // variable size
} ListSw_t;

#if ! defined(cnDummiesInBmSw)
#define cnDummiesInBmSw  0
#endif // ! defined(cnDummiesInBmSw)

// Bitmap switch.
// Using the same struct as Switch_t allows for minimal work
// to RETYPE_FULL_BM_SW.
typedef Switch_t BmSwitch_t;

typedef struct {
  // Emulate sw_wPrefixPop and sw_wPopWord at the beginning of BmLeaf_t.
  #ifdef SKIP_TO_BITMAP
  #ifndef PP_IN_LINK
    Word_t bmlf_wPrefixPop;
  #endif // #ifndef PP_IN_LINK
  #endif // SKIP_TO_BITMAP
  #ifndef BM_POP_IN_WR_HB
  #if !defined(SKIP_TO_BITMAP) || defined(PREFIX_WORD_IN_BITMAP_LEAF)
    Word_t bmlf_wPopCnt; // gwBitmapPopCnt doesn't mask
  #elif defined(POP_WORD) && !defined(POP_WORD_IN_LINK)
    Word_t bmlf_wPopCnt; // gwBitmapPopCnt doesn't mask
  #else // #elif defined(POP_WORD) && !defined(POP_WORD_IN_LINK)
    #define bmlf_wPopCnt  bmlf_wPrefixPop
  #endif // #else defined(POP_WORD) && !defined(POP_WORD_IN_LINK)
  #endif // #ifndef BM_POP_IN_WR_HB
  #ifdef BMLF_CNTS // implied B_JUDYL
      #ifndef B_JUDYL
    #error BMLF_CNTS without B_JUDYL
      #endif // #ifndef B_JUDYL
  #if cnDummiesInLink == 0
      #ifdef BMLF_POP_COUNT_8
    uint8_t bmlf_au8Cnts[1 << (cnBitsInD1 - cnLogBitsPerByte)];
      #elif defined(BMLF_POP_COUNT_1)
    uint8_t bmlf_au8Cnts[1 << cnBitsInD1];
      #else // BMLF_POP_COUNT_8
          #if !defined(EMBED_KEYS) || defined(BMLF_POP_COUNT_32)
    uint8_t bmlf_au8Cnts[cnBytesPerWord];
          #endif // !defined(EMBED_KEYS) || defined(BMLF_POP_COUNT_32)
      #endif // #else BMLF_POP_COUNT_8
  #endif // cnDummiesInLink == 0
  #endif // BMLF_CNTS
    Word_t bmlf_awBitmap[0];
} BmLeaf_t;

static int GetBLR(Word_t *pwRoot, int nBL);

// Set the level of the object in number of bits left to decode.
// Use this only when *pwRoot is a skip link.
// Hmm. We also use it for XX_LIST aka deferred splay which makes it kind of
// a skip up, but we haven't decided if it will be a SKIP_LINK from the
// ifdef perspective yet.
//#ifdef SKIP_LINKS
static inline void
Set_nBLR(Word_t *pwRoot, int nBLR)
{
  #ifdef LVL_IN_WR_HB
    // nBLR may get set to cnBitsPerWord and it will be masked and later
    // read as zero. But we'll never read it because we never skip to
    // cnBitsPerWord.
    assert(cnLogBitsPerWord <= cnBitsLvlM1 + 1);
    assert(nBLR <= cnBitsPerWord);
    SetBits(pwRoot, cnBitsLvlM1, cnLsbLvlM1, nBLR - 1);
  #else // LVL_IN_WR_HB
      #ifndef PP_IN_LINK
      // POP_WORD <==> _LVL_IN_PP, but is there a PP field in a list?
    assert(!tp_bIsList(wr_nType(*pwRoot)));
      #endif // #ifndef PP_IN_LINK
    set_wr_nBLR(*pwRoot, nBLR);
  #endif // #else LVL_IN_WR_HB
}
//#endif // SKIP_LINKS

static const unsigned char BitsSetTable256[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

static int
PopCount8(uint8_t v)
{
#ifdef MOD_POP_COUNT_8
    return (v * 0x200040008001ULL & 0x111111111111111ULL) % 0xf;
#else // MOD_POP_COUNT_8
    return BitsSetTable256[v];
#endif // #else MOD_POP_COUNT_8
}

static int
PopCount32(uint32_t v)
{
  #ifdef MOD_POP_COUNT_32
    uint64_t c;
    c = ((v & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
    c += (((v & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL)
             % 0x1f;
    c += ((v >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
    return c;
  #elif BEST_POP_COUNT_32
    // The best method for counting bits in a 32-bit integer v is the following:
    v = v - ((v >> 1) & 0x55555555); // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333); // temp
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
  #elif TABLE_POP_COUNT_32
    return BitsSetTable256[v & 0xff] + BitsSetTable256[(v >> 8) & 0xff]
        + BitsSetTable256[(v >> 16) & 0xff] + BitsSetTable256[v >> 24];
  #else // POP_COUNT_32
    return (sizeof(int) == 4)
        ? __builtin_popcount((unsigned int)v)
        : __builtin_popcountl((unsigned int)v);
  #endif // #else POP_COUNT_32
}

static inline int BmSwLinkCnt(qp);

#ifdef B_JUDYL

  #ifdef EMBED_KEYS
// qp is a pointer to the switch containing the link that contains the
// embedded key.
// nLinks is the number of links in the switch. This may not be the same as
// EXP(gnBW(qp, gnBLR(qp)), e.g. for a bitmap switch.
// nIndex is the link number in the switch. It is NOT necessarily the same
// as the digit of the key represented by the link, e.g. for a bitmap switch.
static Word_t*
gpwEmbeddedValue(qp, int nLinks, int nIndex)
{
    qv;
      #if defined(VALUE_IN_DUMMY) && (cnDummiesInLink > 0)
    (void)nLinks;
    return pwr_pLinks((Switch_t*)pwr)[nIndex].ln_awDummies;
      #else // defined(VALUE_IN_DUMMY) && (cnDummiesInLink > 0)
    // The following assertion is bogus during Splay which doesn't bother to
    // initialize the bitmap in the switch being used to stage the splay.
    //assert(!tp_bIsBmSw(nType) || (BmSwLinkCnt(qy) == nLinks));
    return &((Word_t*)&pwr_pLinks((Switch_t *)pwr)[nLinks])[nIndex];
      #endif // defined(VALUE_IN_DUMMY) && (cnDummiesInLink > 0)
}
  #endif // EMBED_KEYS

static Word_t*
gpwValues(qp) // gpwListValues
{
    qv;
  #ifdef SKIP_TO_LIST
    if (nType == T_SKIP_TO_LIST) {
        assert(GetBLR(pwRoot, nBL) <= 32);
        return &pwr[-1];
    }
  #endif // SKIP_TO_LIST
  #ifdef LIST_POP_IN_PREAMBLE
    return &pwr[-1];
  #else // LIST_POP_IN_PREAMBLE
    return pwr;
  #endif // #else LIST_POP_IN_PREAMBLE
}

static int
PopCount64(uint64_t v)
{
  #ifdef BEST_POP_COUNT_64 // definitely not best
    v = v - ((v >> 1) & (uint64_t)~(uint64_t)0/3); // temp
    v = (v & (uint64_t)~(uint64_t)0/15*3)
        + ((v >> 2) & (uint64_t)~(uint64_t)0/15*3); // temp
    v = (v + (v >> 4)) & (uint64_t)~(uint64_t)0/255*15; // temp
    return (uint64_t)(v * ((uint64_t)~(uint64_t)0/255))
        >> (sizeof(uint64_t) - 1) * CHAR_BIT; // count
  #elif defined(POP_COUNT_64)
    // Calculate each nibble to have counts of 0..4 bits in each nibble.
    v -= (v >> 1) & (uint64_t)0x5555555555555555;
    v = ((v >> 2) & (uint64_t)0x3333333333333333) +
                    (v & (uint64_t)0x3333333333333333);

    // Odd nibbles += even nibbles (in parallel)
    v += v >> 4;

    // Clean out the even nibbles for some calculating space
    v &= (uint64_t)0x0F0F0F0F0F0F0F0F; // sums bytes (1 instruction)

    // Now sum the 8 bytes of bit counts of 0..8 bits each in odd nibble.
    v *= (uint64_t)0x0101010101010101;
    v = v >> (64 - 8); // sum in high byte

    return ((int)v); // 0..64
  #else // BEST_POP_COUNT_64 #elifdef POP_COUNT_64
    return __builtin_popcountll(v);
  #endif // #else BEST_POP_COUNT_64 #elifdef POP_COUNT_64
}

  #ifdef BITMAP

static Word_t *
gpwBitmapValues(qp, int nBLR)
{
    qv; (void)nBLR;
    // The value area follows the bitmap and the prefix-pop word.
    // What if (nBLR < cnLogBitsPerWord) and (cnBitsInD1 >= cnLogBitsPerWord)?
    // This might happen for USE_XX_SW_ONLY_AT_DL2 if we are not careful.
    // Our code should manipulate the XX_SW nBW so that we get an embedded
    // bitmap at (nBLR == cnBitsInD1) for ALLOW_EMBEDDED_BITMAP, or replace
    // the XX_SW with a T_BITMAP at (nBLR == cnBitsLeftAtDl2).
    // What if (cnBitsLeftAtDl2 < cnLogBitsPerWord)? TBD, but this assertion
    // will catch it if we forget about something.
    assert((nBLR >= cnLogBitsPerWord) || (cnBitsInD1 < cnLogBitsPerWord));
#if defined(KISS_BM) || defined(KISS)
    Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t)
                         + EXP(MAX(1, nBLR - cnLogBitsPerWord));
#else // defined(KISS_BM) || defined(KISS)
    // We only ever have bitmaps for B_JUDYL at cnBitsInD1.
    assert(nBLR == cnBitsInD1);
    Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t)
                         + EXP(MAX(0, cnBitsInD1 - cnLogBitsPerWord));
#endif // #else defined(KISS_BM) || defined(KISS)
    return &pwr[wWordsHdr];
}

// How many keys precede the key we are looking for in the bitmap?
static int
BmIndex(qp, int nBLR, Word_t wKey
  #ifdef EMBED_KEYS
      , Word_t* pwValueUp
  #endif // EMBED_KEYS
        )
{
    qv;
  #ifdef EMBED_KEYS
    (void)pwValueUp;
  #endif // EMBED_KEYS
    assert(!cbEmbeddedBitmap);
    assert(tp_bIsBitmap(nType));
    Word_t wDigit = wKey & MSK(nBLR);
  #ifdef BMLF_POP_COUNT_32
    uint32_t *pu32Bms = (uint32_t*)((BmLeaf_t*)pwr)->bmlf_awBitmap;
    // The bitmap may have more than one uint32_t.
    // nBmNum is the number of the uint32_t which contains the bit we want.
    int nBmNum = wDigit >> 5;
    uint32_t u32Bm = pu32Bms[nBmNum]; // uint32_t we want
    uint32_t u32BmBitMask = EXP(wDigit & (32 - 1));
      #ifdef BMLF_CNTS
          #if cnDummiesInLink > 0
    Word_t wSums = *pLn->ln_awDummies;
          #else // cnDummiesInLink > 0
    Word_t wSums = *(Word_t*)((BmLeaf_t*)pwr)->bmlf_au8Cnts;
          #endif // #else cnDummiesInLink > 0
    wSums *= 0x0101010101010100;
    uint8_t *pu8Sums = (void*)&wSums;
    int nIndex = pu8Sums[nBmNum];
      #else // BMLF_CNTS
    int nIndex = 0;
    for (int nn = 0; nn < nBmNum; nn++) {
        nIndex += PopCount32(pu32Bms[nn]);
    }
      #endif // #else BMLF_CNTS
    nIndex += PopCount32(u32Bm & (u32BmBitMask - 1));
    if ((u32Bm & u32BmBitMask) == 0) {
        nIndex ^= -1;
    }
  #else // BMLF_POP_COUNT_32
      #ifndef BMLF_POP_COUNT_8
    Word_t *pwBmWords = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
    // The bitmap may have more than one word.
    // nBmWordNum is the number of the word which contains the bit we want.
    int nBmWordNum = wDigit >> cnLogBitsPerWord;
    Word_t wBmWord = pwBmWords[nBmWordNum]; // word we want
    Word_t wBmBitMask = EXP(wDigit & (cnBitsPerWord - 1));
      #endif // #ifndef BMLF_POP_COUNT_8
      #ifdef BMLF_CNTS
          #ifdef BMLF_POP_COUNT_8
    uint8_t *pu8BmBytes = (uint8_t*)((BmLeaf_t*)pwr)->bmlf_awBitmap;
    int nBmByteNum = wDigit >> 3;
    uint8_t u8BmByte = pu8BmBytes[nBmByteNum]; // byte we want
    uint8_t u8BmBitMask = EXP(wDigit & 7);
    uint8_t* pu8Sums = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
    int nIndex = pu8Sums[nBmByteNum];
    nIndex += PopCount8(u8BmByte & (u8BmBitMask - 1));
    if ((u8BmByte & u8BmBitMask) == 0) {
        nIndex ^= -1;
    }
          #elif defined(BMLF_POP_COUNT_1)
    uint8_t* pu8Sums = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
    int nIndex = pu8Sums[wDigit];
          #else // BMLF_POP_COUNT_8
              #if cnDummiesInLink > 0
    Word_t wSums = *pLn->ln_awDummies;
              #else // cnDummiesInLink > 0
        #ifdef EMBED_KEYS
    Word_t wSums = *pwValueUp;
        #else // EMBED_KEYS
    Word_t wSums = *(Word_t*)((BmLeaf_t*)pwr)->bmlf_au8Cnts;
        #endif // #else EMBED_KEYS
              #endif // #else cnDummiesInLink > 0
              #ifndef BMLF_CNTS_CUM
    wSums *= 0x01010100;
              #endif // #ifndef BMLF_CNTS_CUM
    int nIndex = ((uint8_t*)&wSums)[nBmWordNum];
          #endif // BMLF_POP_COUNT_8
      #else // BMLF_CNTS
    int nIndex = 0;
    for (int nn = 0; nn < nBmWordNum; nn++) {
        nIndex += PopCount64(pwBmWords[nn]);
    }
      #endif // #else BMLF_CNTS
      #ifndef BMLF_POP_COUNT_8
      #ifndef BMLF_POP_COUNT_1
    nIndex += PopCount64(wBmWord & (wBmBitMask - 1));
      #endif // #ifndef BMLF_POP_COUNT_1
      #endif // #ifndef BMLF_POP_COUNT_8
  #endif // #else BMLF_POP_COUNT_32
    return nIndex;
}

  #endif // BITMAP

#endif // B_JUDYL

#ifdef SKIP_LINKS

// Get the level of the object in number of bits left to decode.
// qp must specify a skip link.
static inline int
gnBLRSkip(qp)
{
    qv;
    assert(tp_bIsSkip(nType));
  #ifdef LVL_IN_WR_HB
    return GetBits(wRoot, cnBitsLvlM1, cnLsbLvlM1) + 1;
  #elif defined(POP_WORD)
    return wr_nBLR(wRoot);
  #else // LVL_IN_WR_HB
    return nDL_to_nBL(tp_to_nDLR(nType));
  #endif // LVL_IN_WR_HB
}

#endif // SKIP_LINKS

// Get the level of the object in number of bits left to decode.
static inline int
gnBLR(qp)
{
    qv;
    assert(!tp_bIsList(nType));
    return
  #ifdef SKIP_LINKS
        tp_bIsSkip(nType) ? gnBLRSkip(qy) :
  #endif // SKIP_LINKS
        nBL;
}

static int
gnListBLR(qp)
{
    qv;
    assert(tp_bIsList(nType));
    if (tp_bIsList(wr_nType(WROOT_NULL)) && (nType == WROOT_NULL)) {
        return nBL;
    }
  #ifdef LVL_IN_WR_HB
    int nBLR = GetBits(wRoot, cnBitsLvlM1, cnLsbLvlM1) + 1;
    return nBLR;
  #elif defined(PP_IN_LINK)
    return wr_nBLR(wRoot);
  #else // LVL_IN_WR_HB #elif def(PP_IN_LINK)
    return nBL;
  #endif // LVL_IN_WR_HB #elif def(PP_IN_LINK) #else
}

static void
snListBLR(qp, int nBLR)
{
    qv; (void)nBLR;
  #if defined(LVL_IN_WR_HB) || defined(PP_IN_LINK)
    Set_nBLR(pwRoot, nBLR);
  #endif // defined(LVL_IN_WR_HB) || defined(PP_IN_LINK)
}

#ifdef SKIP_LINKS
static inline Word_t
gwPrefix(qp)
{
    qv;
    return PWR_wPrefixNATBL(pwRoot, pwr, nBL);
}
#endif // SKIP_LINKS

#define cnBitsPreListPopCnt cnBitsListPopCnt
#define cnLsbPreListPopCnt (cnBitsPerWord - cnBitsListPopCnt)
#define cnBitsPreListSwPopM1 cnBitsListSwPopM1
#define cnLsbPreListSwPopM1 (cnBitsPerWord - cnBitsListSwPopM1)

static inline Word_t
gwPopCnt(qp, int nBLR)
{
    qv; (void)nBLR;
    if (wRoot == WROOT_NULL) { return 0; }
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    assert(nBL < cnBitsPerWord);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #ifdef POP_WORD
    Word_t wPopCnt = PWR_wPopWordBL(&pLn->ln_wRoot, pwr, nBLR);
  #else // POP_WORD
    Word_t wPopCnt = PWR_wPopCntBL(&pLn->ln_wRoot, pwr, nBLR);
    if (wPopCnt == 0) {
        return NZ_MSK(nBLR) + 1; // Must handle nBLR == cnBitsPerWord.
    }
  #endif // #else POP_WORD
    return wPopCnt;
}

static inline void
swPopCnt(qp, int nBLR, Word_t wPopCnt)
{
    qv; (void)nBLR;
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    assert(nBL < cnBitsPerWord);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #ifdef POP_WORD
    PWR_wPopWordBL(&pLn->ln_wRoot, pwr, nBLR) = wPopCnt;
  #else // POP_WORD
    set_PWR_wPopCntBL(&pLn->ln_wRoot, pwr, nBLR, wPopCnt);
  #endif // #else POP_WORD
}

static inline int
gnListPopCnt(qp, int nBLR)
{
    qv; (void)nBLR;
#ifdef _LIST_POP_IN_LINK_X
    if (nBL < cnBitsPerWord) {
        return PWR_wPopCntBL(pwRoot, NULL, nBLR);
    }
#endif // _LIST_POP_IN_LINK_X
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
    return gnListPopCnt(qy, GetBLR(pwRoot, nBL));
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
#ifdef _LIST_POP_IN_LINK_X
    assert(nBL < cnBitsPerWord); // not coded yet
#endif // _LIST_POP_IN_LINK_X
    Word_t *pwr = wr_pwr(*pwRoot); (void)pwr;
#if defined(POP_IN_WR_HB) // 64-bit default
    assert(nPopCnt - 1 <= (int)MSK(cnBitsListPopCnt));
    SetBits(pwRoot, cnBitsListPopCnt, cnLsbListPopCnt, nPopCnt - 1);
#elif defined(LIST_POP_IN_PREAMBLE) // 32-bit default
  #if BPW > 32
    assert(nPopCnt - 1 <= (int)MSK(cnBitsPreListPopCnt));
  #endif // BPW > 32
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

static int
GetBLR(Word_t *pwRoot, int nBL)
{
    (void)pwRoot;

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) { return nBL; }
#endif // defined(NO_TYPE_IN_XX_SW)

    return
  #ifdef SKIP_LINKS
        tp_bIsSkip(wr_nType(*pwRoot)) ? (int)wr_nBLR(*pwRoot) :
  #endif // SKIP_LINKS
            nBL;
}

  #ifdef SKIP_TO_LIST
static Word_t
gwListPrefix(qp, int nBLR)
{
    qv; (void)nBLR;
    assert(nType == T_SKIP_TO_LIST);
  #if defined(PP_IN_LINK)
    return PWR_wPrefixBL(pwRoot, pwr, nBLR);
  #else // defined(PP_IN_LINK)
    assert(nBLR == 32);
    return w_wPrefixNotAtTopBL(pwr[-1], nBLR);
  #endif // #else defined(PP_IN_LINK)
}
  #endif // SKIP_TO_LIST

#if defined(BITMAP)

  #ifdef SKIP_TO_BITMAP

static Word_t
gwBitmapPrefix(qp, int nBLR)
{
    qv; (void)nBLR;
    assert(nType == T_SKIP_TO_BITMAP);
  #if defined(PP_IN_LINK)
    return PWR_wPrefixBL(pwRoot, pwr, nBLR);
  #else // defined(PP_IN_LINK)
    BmLeaf_t *pBmLeaf = (BmLeaf_t*)pwr;
      #ifdef PREFIX_WORD_IN_BITMAP_LEAF
    return pBmLeaf->bmlf_wPrefixPop;
      #else // PREFIX_WORD_IN_BITMAP_LEAF
    return w_wPrefixNotAtTopBL(pBmLeaf->bmlf_wPrefixPop, nBLR);
      #endif // #else PREFIX_WORD_IN_BITMAP_LEAF
  #endif // #else defined(PP_IN_LINK)
}

// Prefix is in the word following the bitmap.
static void
swBitmapPrefix(qp, int nBLR, Word_t wPrefix)
{
    qv; (void)nBLR;
  #if defined(PP_IN_LINK)
    set_PWR_wPrefixBL(pwRoot, pwr, nBLR, wPrefix);
  #else // defined(PP_IN_LINK)
    BmLeaf_t *pBmLeaf = (BmLeaf_t*)pwr;
      #ifdef PREFIX_WORD_IN_BITMAP_LEAF
    pBmLeaf->bmlf_wPrefixPop = wPrefix & ~MSK(nBLR);
      #else // PREFIX_WORD_IN_BITMAP_LEAF
    set_w_wPrefixNATBL(pBmLeaf->bmlf_wPrefixPop, nBLR, wPrefix);
      #endif // #else PREFIX_WORD_IN_BITMAP_LEAF
  #endif // #else defined(PP_IN_LINK)
}

  #endif // SKIP_TO_BITMAP

static Word_t
gwBitmapPopCnt(qp, int nBLR)
{
    qv; (void)nBLR;
    Word_t wPopCnt;
  #ifdef _BM_POP_IN_LINK_X
    wPopCnt = PWR_wPopCntBL(pwRoot, pwr, nBLR);
    if (wPopCnt == 0) { wPopCnt = EXP(nBLR); } // full pop
  #elif defined(BM_POP_IN_WR_HB)
    wPopCnt = GetBits(*pwRoot, cnBitsCnt, cnLsbCnt);
    if (wPopCnt == 0) { wPopCnt = EXP(nBLR); } // full pop
  #else // _BM_POP_IN_LINK_X
    // No need to handle embedded bitmaps here.
    assert(!cbEmbeddedBitmap || (nBLR > cnLogBitsPerLink));
    assert(tp_bIsBitmap(nType));
    assert((wr_nType(WROOT_NULL) != T_BITMAP) || (wRoot != WROOT_NULL));
    BmLeaf_t *pBmLeaf = (BmLeaf_t*)pwr;
    wPopCnt = pBmLeaf->bmlf_wPopCnt;
      #ifdef SKIP_TO_BITMAP
      #ifndef PREFIX_WORD_IN_BITMAP_LEAF
          #if !defined(KISS_BM) && !defined(KISS)
    assert((nBLR == cnBitsInD1) || (nBLR == cnBitsLeftAtDl2));
    if (cbEmbeddedBitmap) {
        assert(nBLR == cnBitsLeftAtDl2);
        wPopCnt = pBmLeaf->bmlf_wPopCnt & MSK(cnBitsLeftAtDl2);
        if (wPopCnt == 0) { wPopCnt = EXP(cnBitsLeftAtDl2); } // full pop
    } else if (cn2dBmMaxWpkPercent == 0) {
        assert(nBLR == cnBitsInD1);
        wPopCnt = pBmLeaf->bmlf_wPopCnt & MSK(cnBitsInD1);
        if (wPopCnt == 0) { wPopCnt = EXP(cnBitsInD1); } // full pop
    } else
          #endif // !defined(KISS_BM) && !defined(KISS)
    {
        wPopCnt = pBmLeaf->bmlf_wPopCnt & MSK(nBLR);
        // Avoiding a conditional branch here might be more straightforward if
        // we were storing pop - 1 in the PrefixPop word.
        // Are we just obfuscating the conditional branch with !wPopCnt?
        // Is the compiler smart enough to get rid of the conditional branch
        // even if we leave it in the source?
        // A quick comparison of the resulting objects leads me to believe
        // either version of the source results in the same object code.
        // wPopCnt += EXP(nBLR) * !wPopCnt; // full pop
        if (wPopCnt == 0) { wPopCnt = EXP(nBLR); } // full pop
    }
      #endif // #ifndef PREFIX_WORD_IN_BITMAP_LEAF
      #endif // SKIP_TO_BITMAP
  #endif // #else _BM_POP_IN_LINK_X
    assert(wPopCnt <= EXP(nBLR));
    return wPopCnt;
}

// Pop cnt is in the word following the bitmap.
// Full pop gets masked to zero if pop is sharing a word with prefix.
static void
swBitmapPopCnt(qp, int nBLR, Word_t wPopCnt)
{
    qv; (void)nBLR;
  #ifdef _BM_POP_IN_LINK_X
    set_PWR_wPopCntBL(pwRoot, pwr, nBLR, wPopCnt);
  #elif defined(BM_POP_IN_WR_HB)
    SetBits(pwRoot, cnBitsCnt, cnLsbCnt, wPopCnt);
  #else // _BM_POP_IN_LINK_X
    assert((wr_nType(WROOT_NULL) != T_BITMAP) || (wRoot != WROOT_NULL));
    BmLeaf_t *pBmLeaf = (BmLeaf_t*)pwr;
      #if !defined(SKIP_TO_BITMAP) || defined(PREFIX_WORD_IN_BITMAP_LEAF)
    pBmLeaf->bmlf_wPopCnt = wPopCnt;
      #else // !defined(SKIP_TO_BITMAP) || defined(PREFIX_WORD_IN_BITMAP_LEAF)
          #if !defined(KISS_BM) && !defined(KISS)
    assert((nBLR == cnBitsInD1) || (nBLR == cnBitsLeftAtDl2));
    if (cbEmbeddedBitmap) {
        assert(nBLR == cnBitsLeftAtDl2);
        SetBits(&pBmLeaf->bmlf_wPopCnt, cnBitsLeftAtDl2, 0, wPopCnt);
    } else if (cn2dBmMaxWpkPercent == 0) {
        assert(nBLR == cnBitsInD1);
        SetBits(&pBmLeaf->bmlf_wPopCnt, cnBitsInD1, 0, wPopCnt);
    } else
          #endif // !defined(KISS_BM) && !defined(KISS)
    {
        SetBits(&pBmLeaf->bmlf_wPopCnt, nBLR, 0, wPopCnt);
    }
      #endif // #else !defined(SKIP_TO_BITMAP) || defined(PREFIX_WORD_...)
  #endif // #else _BM_POP_IN_LINK_X
}

#endif // defined(BITMAP)

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
    #define GetPopCnt  GetPopCntL
    #define CountSwLoop  CountSwLoopL
    #define SumPopCnt  SumPopCntL
  #else // B_JUDYL
    #define GetPopCnt  GetPopCnt1
    #define CountSwLoop  CountSwLoop1
    #define SumPopCnt  SumPopCnt1
  #endif // #else B_JUDYL

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
static Word_t SumPopCnt(Word_t *pwRoot, int nBL);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#ifdef CODE_BM_SW

// Bitmap switches are meant to handle data that would otherwise cause us to
// use too much memory.
// We don't want bitmap switches with uniformly distributed random data.
// Choose defaults empirically to cause conversion at lowest population
// possible while avoiding bitmap switches for uniformly distributed random
// data.
#ifdef B_JUDYL
  #if !defined(cnBmSwConvertL)
      #ifdef EMBED_KEYS
    #define cnBmSwConvertL 7
      #else // EMBED_KEYS
    #define cnBmSwConvertL 5
      #endif // EMBED_KEYS
  #endif // !defined(cnBmSwConvertL)
    #define cnBmSwConvert  cnBmSwConvertL
  #if !defined(cnBmSwRetainL)
    #define cnBmSwRetainL 2
  #endif // !defined(cnBmSwRetainL)
  #define cnBmSwRetain  cnBmSwRetainL
#else // B_JUDYL
  #if !defined(cnBmSwConvert1)
    #define cnBmSwConvert1 3
  #endif // !defined(cnBmSwConvert1)
    #define cnBmSwConvert  cnBmSwConvert1
  #if !defined(cnBmSwRetain1)
    #define cnBmSwRetain1 2
  #endif // !defined(cnBmSwRetain1)
    #define cnBmSwRetain  cnBmSwRetain1
#endif // #else B_JUDYL

static int // bool
InflateBmSwTest(qp) // qp points to BM switch
{
    qv;
    int nBLR = gnBLR(qy);
    int nBW = gnBW(qy, nBLR); // BW is width of switch
    Word_t wPopCnt;
#if defined(PP_IN_LINK) && !defined(NO_SKIP_AT_TOP)
    if (nBL >= cnBitsPerWord) {
        wPopCnt = SumPopCnt(pwRoot, cnBitsPerWord);
    } else
#endif // defined(PP_IN_LINK) && !defined(NO_SKIP_AT_TOP)
    { wPopCnt = gwPopCnt(qy, nBLR); }

    int nBytesPerPop = ExtListBytesPerKey(nBLR - nBW); // accumulator
    int nBytesPerLink = sizeof(Link_t); // accumulator
#ifdef B_JUDYL
    nBytesPerPop += sizeof(Word_t); // value
  #ifdef EMBED_KEYS
  #ifndef VALUE_IN_DUMMY
    nBytesPerLink += sizeof(Word_t); // value
  #endif // VALUE_IN_DUMMY
  #endif // EMBED_KEYS
#endif // B_JUDYL

// inflate <==> inflated total-words / pop < words-per-key threshold
// How do we estimate inflated total words?
// (bytes-in-sw + bytes-below-sw) / sizeof(Word_t) / pop < wpk
// (bytes-in-sw + bytes-below-sw) / sizeof(Word_t) / pop < convert/retain
// (bytes-in-sw + bytes-below-sw) / pop < sizeof(Word_t) * convert/retain
// (bytes-in-sw + bytes-below-sw) * retain < sizeof(Word_t) * convert * pop
// convert * pop * sizeof(Word_t) > retain * (bytes-in-sw + bytes-below-sw)

    return cnBmSwConvert * wPopCnt * sizeof(Word_t)
        > cnBmSwRetain * (EXP(nBW) * nBytesPerLink + wPopCnt * nBytesPerPop);
}

#endif // CODE_BM_SW

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
#define ListSlotCnt  ListSlotCntL
#define NewList  NewListL
#define OldList  OldListL
#define InsertGuts  InsertGutsL
#define InsertAtList  InsertAtListL
#define InsertAtBitmap  InsertAtBitmapL
#define InflateEmbeddedList  InflateEmbeddedListL
#define InsertCleanup  InsertCleanupL
#define RemoveGuts  RemoveGutsL
#define RemoveCleanup  RemoveCleanupL
#define Dump  DumpL
#else // B_JUDYL
#define ListSlotCnt  ListSlotCnt1
#define NewList  NewList1
#define OldList  OldList1
#define InsertGuts  InsertGuts1
#define InsertAtList  InsertAtList1
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
         , int nBLUp
#endif // defined(CODE_XX_SW)
#if defined(B_JUDYL) && defined(EMBED_KEYS)
         , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
           );

#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
void
#endif // B_JUDYL
InsertAtList(qp, Word_t wKey, int nPos
#if defined(CODE_XX_SW)
           , Link_t *pLnUp
           , int nBLUp
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

#if 0
#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtBitmap(qp, Word_t wKey);
#endif

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

int ListSlotCnt(int nPopCnt, int nBLR);
Word_t *NewList(Word_t wPopCnt, int nBL);
int OldList(Word_t *pwList, int nPopCnt, int nBLR, int nType);

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
extern unsigned anDL_to_nBW[ cnBitsPerWord + 1 ];
extern unsigned anDL_to_nBL[ cnBitsPerWord + 1 ];
extern unsigned anBL_to_nDL[ cnBitsPerWord * 2 ];
  #else // defined(BPD_TABLE_RUNTIME_INIT)
extern const unsigned anDL_to_nBW[];
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

  #if (cnBitsPerWord == 64)
    #define BUCKET_LOCATE_KEY LocateKey64
  #endif // (cnBitsPerWord == 64)

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

static int
Psplit(int nPopCnt, int nBL, int nShift, Word_t wKey)
{
    /* overflow */
    assert((Word_t)nPopCnt <= (Word_t)-1 / NZ_MSK(nBL - nShift));
    assert((nBL - nShift >= 16) || (nShift == 0));
    assert((Word_t)nPopCnt <= EXP(nBL - nShift)); /* underflow */
    // key * pop / expanse
    return ((wKey & NZ_MSK(nBL)) >> nShift) * nPopCnt >> (nBL - nShift);
}

// This is a non-parallel psplit search that calculates a descriptive _nPos.
// It has to work for small nBL in case of no COMPRESSED_LISTS.
// It doesn't usually have to work for nBL == cnBitsPerWord since psplit
// search is usually a bad choice for that case.
#define PSPLIT_SEARCH_BY_KEY_GUTS(_x_t, _nBL, /* nPsplitShift */ _x, \
                                  _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    int nSplit = Psplit((_nPopCnt), (_nBL), (_x), (_xKey)); \
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

// This is a non-parallel psplit search that calculates a descriptive _nPos.
#define PSPLIT_SEARCH_BY_KEY(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    assert((_nBL) <= cnBitsPerWord); \
    PSPLIT_SEARCH_BY_KEY_GUTS(_x_t, (_nBL), /* nPsplitShift */ 0, \
                              (_pxKeys), (_nPopCnt), (_xKey), (_nPos)); \
}

#define PSPLIT_SEARCH_BY_KEY_WORD(_nBL, _pwKeys, _nPopCnt, _wKey, _nPos) \
{ \
    assert((Word_t)((_pwKeys) + 1) == (Word_t)(_pwKeys) + sizeof(Word_t)); \
    PSPLIT_SEARCH_BY_KEY_GUTS(Word_t, (_nBL), /* nPsplitShift */ (_nBL) - 16, \
                              (_pwKeys), (_nPopCnt), (_wKey), (_nPos)); \
}

static int
PsplitSearchByKeyWord(int nBL,
                      Word_t *pwKeys, int nPopCnt, Word_t wKey, int nPos)
{
    PSPLIT_SEARCH_BY_KEY_WORD(nBL, pwKeys, nPopCnt, wKey, nPos);
    return nPos;
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

// The second argument to __builtin_prefetch is ignored.
#if defined(BUILTIN_PREFETCH_0)
  #define PREFETCH(_p)  __builtin_prefetch(_p, 0, 0) // low temporal locality
#elif defined(BUILTIN_PREFETCH_1)
  #define PREFETCH(_p)  __builtin_prefetch(_p, 0, 1)
#elif defined(BUILTIN_PREFETCH_2)
  #define PREFETCH(_p)  __builtin_prefetch(_p, 0, 2)
#elif defined(BUILTIN_PREFETCH_3)
  #define PREFETCH(_p)  __builtin_prefetch(_p, 0, 3) // high temporal locality
#else
  #define BUILTIN_PREFETCH_0
  #define PREFETCH(_p)  __builtin_prefetch(_p, 0, 0)
#endif

#if defined(PSPLIT_PARALLEL) || defined(PARALLEL_SEARCH_WORD)
#if !defined(LIST_END_MARKERS)

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

// LOCATEKEYF assumes _pxKeys and _nPos are _b_t-aligned and the
// the list is padded to _b_t alignment.
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
// _x is the number of least significant bits of the key we can sacrifice
// when doing the psplit computation.
#define PSPLIT_HASKEY_GUTS(_b_t, _x_t, _nBL, /* nPsplitShift */ _x, \
                           _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    _b_t *px = (_b_t *)(_pxKeys); \
    ASSERT(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    /* nSplit is the key chosen by PSPLIT */ \
    int nSplit = Psplit((_nPopCnt), (_nBL), (_x), (_xKey)); \
    /* nSplitP is nSplit rounded down to the first key in the bucket */ \
    int nSplitP = nSplit * sizeof(_x_t) / sizeof(_b_t); \
    ASSERT((int)((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitP); \
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
            if (nSplitP \
                == (int)(((_nPopCnt) - 1) * sizeof(_x_t) / sizeof(_b_t))) \
            { \
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
    ASSERT(nCnt == ((_nPos) >= 0)); \
}

#define PSPLIT_HASKEY(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    assert((_nBL) <= cnBitsPerWord); \
    PSPLIT_HASKEY_GUTS(_b_t, _x_t, (_nBL), /* nPsplitShift */ 0, \
                       (_pxKeys), (_nPopCnt), (_xKey), (_nPos)); \
}

#define PSPLIT_HASKEY_WORD(_b_t, _nBL, _pwKeys, _nPopCnt, _wKey, _nPos) \
{ \
    assert((Word_t)((_pwKeys) + 1) == (Word_t)(_pwKeys) + sizeof(Word_t)); \
    PSPLIT_HASKEY_GUTS(_b_t, Word_t, (_nBL), /* nPsplitShift */ (_nBL) - 16, \
                       (_pwKeys), (_nPopCnt), (_wKey), (_nPos)); \
}

#ifdef PREFETCH_LOCATEKEY_PSPLIT_VAL
#define _PF_LK(_x)  (_x)
#else // PREFETCH_LOCATEKEY_PSPLIT_VAL
#define _PF_LK(_x)
#endif // #else PREFETCH_LOCATEKEY_PSPLIT_VAL

#ifdef PREFETCH_LOCATEKEY_NEXT_VAL
#define _PF_LK_NX(_x)  (_x)
#else // PREFETCH_LOCATEKEY_NEXT_VAL
#define _PF_LK_NX(_x)
#endif // #else PREFETCH_LOCATEKEY_NEXT_VAL

#ifdef PREFETCH_LOCATEKEY_PREV_VAL
#define _PF_LK_PV(_x)  (_x)
#else // PREFETCH_LOCATEKEY_PREV_VAL
#define _PF_LK_PV(_x)
#endif // #else PREFETCH_LOCATEKEY_PREV_VAL

// PSPLIT_LOCATEKEY_GUTS uses qy for prefetch but qy is not in the parameter
// list. Shame on us.
#define PSPLIT_LOCATEKEY_GUTS(_b_t, _x_t, _nBL, /* nPsplitShift */ _xShift, \
                              _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPLK(nBL %d pxKeys %p nPopCnt %d xKey 0x%zx nPos %d\n", */ \
        /* _nBL, (void*)_pxKeys, _nPopCnt, (Word_t)_xKey, _nPos); */ \
    _b_t *pb = (_b_t *)(_pxKeys); /* bucket pointer */ \
    /* _pxKeys must be aligned on a bucket boundary */ \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(_b_t)))) == 0); \
    /* nSplit is the key chosen by Psplit */ \
    int nSplit = Psplit((_nPopCnt), (_nBL), (_xShift), (_xKey)); \
    BJL(char* pcPrefetch = (char*)&gpwValues(qy)[~nSplit]; (void)pcPrefetch); \
    _PF_LK(BJL(PREFETCH(pcPrefetch))); \
    _PF_LK_NX(BJL(PREFETCH(pcPrefetch - 64))); \
    _PF_LK_PV(BJL(PREFETCH(pcPrefetch + 64))); \
    int nKeysPerBucket = sizeof(_b_t) / sizeof(_x_t); \
    /* nSplitB is number of the bucket chosen by Psplit */ \
    int nSplitB = nSplit / nKeysPerBucket; \
    assert((int)((nSplit * sizeof(_x_t)) >> LOG(sizeof(_b_t))) == nSplitB); \
    if (((_nPos) = BUCKET_LOCATE_KEY(&pb[nSplitB], \
                                     (_xKey), sizeof(_x_t) * 8)) >= 0) { \
        /* add the number of keys in the buckets before nSplitB */ \
        _nPos += nSplitB * nKeysPerBucket; \
        SMETRICS(++j__DirectHits); \
    } \
    else \
    { \
        nSplit = nSplitB * nKeysPerBucket; \
        _x_t xKeySplit = (_pxKeys)[nSplit]; \
        /* now we have the value of a key in the list */ \
        if ((_xKey) > xKeySplit) { \
            if (nSplitB == (int)(((_nPopCnt) - 1) / nKeysPerBucket)) { \
                /* we searched the last bucket and the key is not there */ \
                (_nPos) = -1; /* we don't know where to insert */ \
            } else { \
                /* parallel search the tail of the list */ \
                /* ++nSplitB; */ \
                (_nPos) = (int)nSplit + nKeysPerBucket; \
                LOCATEKEYF(_b_t, (_xKey), \
                          (_pxKeys), (_nPopCnt) - (_nPos), (_nPos)); \
                SMETRICS(++j__GetCallsP); \
            } \
        } \
        else \
        { \
            if (nSplitB == 0) { \
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
            for (int ii = 0; ii < (_nPopCnt); ii += nKeysPerBucket) { \
                assert( ! BUCKET_HAS_KEY((_b_t *)&(_pxKeys)[ii], (_xKey), \
                                         sizeof(_x_t) * 8) ); \
            } \
        } \
    } \
}

#define PSPLIT_LOCATEKEY(_b_t, _x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    assert((_nBL) <= cnBitsPerWord); \
    PSPLIT_LOCATEKEY_GUTS(_b_t, _x_t, (_nBL), /* nPsplitShift */ 0, \
                          (_pxKeys), (_nPopCnt), (_xKey), (_nPos)); \
}

#define PSPLIT_LOCATEKEY_WORD(_b_t, _nBL, _pwKeys, _nPopCnt, _wKey, _nPos) \
{ \
    assert((Word_t)((_pwKeys) + 1) == (Word_t)(_pwKeys) + sizeof(Word_t)); \
    PSPLIT_LOCATEKEY_GUTS(_b_t, Word_t, (_nBL), /* nPsplitShift */ (_nBL) - 16, \
                          (_pwKeys), (_nPopCnt), (_wKey), (_nPos)); \
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
#define PSPLIT_HASKEY_128_UA(_x_t, \
                                  _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    __m128i *px = (__m128i *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    int nSplit = Psplit((_nPopCnt), (_nBL), (_xKey)); \
    int nSplitP = nSplit * sizeof(_x_t) / sizeof(__m128i); \
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

// PSPLIT_HASKEY_128_96 is for UA_PARALLEL_128 and T_LIST_UA.
#define PSPLIT_HASKEY_128_96(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    if (HasKey96((__m128i *)(_pxKeys), (_xKey), sizeof(_x_t) * 8)) { \
        (_nPos) = 0; /* key exists */ \
    } else { \
        (_nPos) = -1; /* key does not exist */ \
    } \
}

#define PSPLIT_HASKEY_128(_x_t, _nBL, _pxKeys, _nPopCnt, _xKey, _nPos) \
{ \
    /* printf("PSPHK(nBL %d pxKeys %p nPopCnt %d xKey 0x%x nPos %d\n", */ \
        /* _nBL, (void *)_pxKeys, _nPopCnt, _xKey, _nPos); */ \
    __m128i *px = (__m128i *)(_pxKeys); \
    assert(((Word_t)(_pxKeys) & MSK(LOG(sizeof(__m128i)))) == 0); \
    int nSplit = Psplit((_nPopCnt), (_nBL), 0, (_xKey)); \
    int nSplitP = nSplit * sizeof(_x_t) / sizeof(__m128i); \
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
    Word_t wMask = NZ_MSK(nBL);
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

// vBKA_t: vector of 2^B bytes of 2^K-byte elements with 2^A-byte alignment.
  #if defined(__clang__) && !defined(GCC_VECTORS)
typedef unsigned char  __attribute__((ext_vector_type(16))) v404_t;
typedef unsigned short __attribute__((ext_vector_type(8))) v414_t;
typedef unsigned int   __attribute__((ext_vector_type(4))) v424_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(16))) v404_t;
typedef unsigned short __attribute__((vector_size(16))) v414_t;
typedef unsigned int   __attribute__((vector_size(16))) v424_t;
  #endif // #else __clang__
  #if defined(__clang__) && !defined(GCC_VECTORS)
typedef unsigned char  __attribute__((ext_vector_type(16), aligned(8))) v403_t;
typedef unsigned short __attribute__((ext_vector_type( 8), aligned(8))) v413_t;
typedef unsigned int   __attribute__((ext_vector_type( 4), aligned(8))) v423_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(16), aligned(8))) v403_t;
typedef unsigned short __attribute__((vector_size(16), aligned(8))) v413_t;
typedef unsigned int   __attribute__((vector_size(16), aligned(8))) v423_t;
  #endif // #else __clang__

#ifdef WORD_ALIGNED_VECTORS
  #if defined(__clang__) && !defined(GCC_VECTORS)
typedef unsigned char  __attribute__((ext_vector_type(16), aligned(8))) v_t;
typedef unsigned short __attribute__((ext_vector_type( 8), aligned(8))) v41_t;
typedef unsigned int   __attribute__((ext_vector_type( 4), aligned(8))) v42_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(16), aligned(8))) v_t;
typedef unsigned short __attribute__((vector_size(16), aligned(8))) v41_t;
typedef unsigned int   __attribute__((vector_size(16), aligned(8))) v42_t;
  #endif // __clang__
#else // WORD_ALIGNED_VECTORS
  #if defined(__clang__) && !defined(GCC_VECTORS)
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
#endif // WORD_ALIGNED_VECTORS

// vBK_t: vector of 2^B bytes of 2^K-byte elements.
  #if defined(__clang__) && !defined(GCC_VECTORS)
typedef unsigned char  __attribute__((ext_vector_type(8))) v30_t;
typedef unsigned short __attribute__((ext_vector_type(4))) v31_t;
typedef unsigned int   __attribute__((ext_vector_type(2))) v32_t;
  #else // __clang__
typedef unsigned char  __attribute__((vector_size(8))) v30_t;
typedef unsigned short __attribute__((vector_size(8))) v31_t;
typedef unsigned int   __attribute__((vector_size(8))) v32_t;
  #endif // __clang__

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
        ASSERT(nBL == 32);
        v42_t vEq = (v42_t)(*(v42_t*)pxBucket == (unsigned int)wKey);
  #ifdef HK_MOVEMASK
        return _mm_movemask_epi8((__m128i)vEq);
  #else // HK_MOVEMASK
        return _mm_packs_epi32((__m128i)vEq, (__m128i)vEq)[0];
  #endif // HK_MOVEMASK
    }
    ASSERT(nBL == 64);
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

// If 128-bit bucket has key then return the position of the key.
// Otherwise return -1.
static int
LocateKey128(__m128i *pxBucket, Word_t wKey, int nBL)
{
    Word_t wHasKey = HasKey128(pxBucket, wKey, nBL);
  #if defined(USE_POPCOUNT_IN_LK8) || !defined(USE_FFS_IN_LK8)
    if (wHasKey == 0) {
        return -1;
    }
  #endif // defined(USE_POPCOUNT_IN_LK8) || !defined(USE_FFS_IN_LK8)
    if (nBL == 8) {
  #ifdef USE_POPCOUNT_IN_LK8
        return __builtin_popcountll((wHasKey & -wHasKey) - 1);
  #elif defined(USE_FFS_IN_LK8)
        return __builtin_ffsll(wHasKey) - 1;
  #else // USE_POPCOUNT_IN_LK8 #elif USE_FFS_IN_LK8
        return __builtin_ctzll(wHasKey);
  #endif // USE_POPCOUNT_IN_LK8 #elif USE_FFS_IN_LK8 #else
    }
  #if !defined(USE_POPCOUNT_IN_LK8) && defined(USE_FFS_IN_LK8)
    if (wHasKey == 0) {
        return -1;
    }
  #endif // !defined(USE_POPCOUNT_IN_LK8) && defined(USE_FFS_IN_LK8)
    int nFirstSetBit = __builtin_ctzll(wHasKey);
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

// Find a key in a 96-bit bucket.
// HasKey96 is for UA_PARALLEL_128 and T_LIST_UA.
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
  #if defined(__clang__) && !defined(GCC_VECTORS)
        v31_t vBucket = *(v31_t*)px;
        v31_t v31 = (v31_t)(vBucket == (unsigned short)wKey);
        return *(uint64_t*)&v31;
  #else // __clang__
// 10/12/19: gcc generates horrible and horribly slow code for this.
        return (uint64_t)(*(v31_t*)px == (unsigned short)wKey);
  #endif // __clang__

    }
    if (nBL == 8) {
  #if defined(__clang__) && !defined(GCC_VECTORS)
        v30_t vBucket = *(v30_t*)px;
        v30_t v30 = (v30_t)(vBucket == (unsigned char)wKey);
        return *(uint64_t*)&v30;
  #else // __clang__
        return (uint64_t)(*(v30_t*)px == (unsigned char)wKey);
  #endif // __clang__
    }
    if (nBL <= 32) {
        assert(nBL == 32);
  #if defined(__clang__) && !defined(GCC_VECTORS)
        v32_t vBucket = *(v32_t*)px;
        v32_t v32 = (v32_t)(vBucket == (unsigned int)wKey);
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
    uint64_t xMask = NZ_MSK(nBL);
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
    int nFirstSetBit = __builtin_ctzll(wHasKey);
    return nFirstSetBit / nBL;
}

static __m128i
HasKey128Magic(__m128i *pxBucket, Word_t wKey, int nBL)
{
    __m128i xLsbs, xMsbs, xKeys;
    HAS_KEY_128_SETUP(wKey, nBL, xLsbs, xMsbs, xKeys);
    return HasKey128MagicTail(pxBucket, xLsbs, xMsbs, xKeys);
}

#endif // !defined(LIST_END_MARKERS)
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
  #if (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)

static int
SearchList8(qp, int nBLR, Word_t wKey)
{
    qv; (void)nBLR;

    assert(nBL <= 8);
    // sizeof(__m128i) == 16 bytes
    int nPopCnt = PWR_xListPopCnt(&wRoot, pwr, nBLR);
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
#ifndef USE_XX_SW_ONLY_AT_DL2
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
  #ifdef DEBUG
    if (PWR_xListPopCnt(&wRoot, pwr, 8) > 16) {
        printf("\nnBL %d nBLR %d nPopCnt %d\n",
               nBL, nBLR, PWR_xListPopCnt(&wRoot, pwr, 8));
    }
  #endif // DEBUG
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
#endif // #ifndef USE_XX_SW_ONLY_AT_DL2
#endif // defined(PARALLEL_128)

    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint8_t *pcKeys = ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8));
    uint8_t cKey = (uint8_t)wKey;
    int nPos = 0;
    PSPLIT_HASKEY(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
    return nPos >= 0;

#endif // defined(PSPLIT_PARALLEL)
#endif // defined(PSPLIT_SEARCH_8)

    return SearchList8(qy, nBLR, wKey) >= 0;
}

  #endif // (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
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
        PSPLIT_HASKEY_128_UA(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #else // defined(UA_PARALLEL_128)
        PSPLIT_HASKEY(Bucket_t, uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #endif // defined(UA_PARALLEL_128)
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY(Bucket_t, uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #else // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #endif // ...
    return nPos >= 0;
}
#endif // 0

#ifdef PSPLIT_PARALLEL
  #ifdef PARALLEL_128
// What is the purpose of ListHasKey1696?
// Is it for UA_PARALLEL_128?
// Then why is there an ifdef UA_PARALLEL_128 inside?
// Is it for T_LIST_UA?
//
//   8 < nBL <= 16, nPopCnt <= 6
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
        PSPLIT_HASKEY_128_96(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #else // defined(UA_PARALLEL_128)
        PSPLIT_HASKEY(Bucket_t, uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
  #endif // defined(UA_PARALLEL_128)
    } else
      #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY(Bucket_t, uint16_t, nBL, psKeys, nPopCnt, sKey, nPos);
    }
  #elif defined(BACKWARD_SEARCH_16) // defined(PSPLIT_SEARCH_16)
    SEARCHB(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #else // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    // here for forward linear search with end check
    SEARCHF(uint16_t, psKeys, nPopCnt, sKey, nPos); (void)nBL;
  #endif // defined(PSPLIT_SEARCH_16) elif defined(BACKWARD_SEARCH_16) else
    return nPos >= 0;
}
  #endif // PARALLEL_128
#endif // PSPLIT_PARALLEL

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
      #ifdef PSPLIT_PARALLEL
          #ifdef UA_PARALLEL_128
    if ((nPopCnt <= 6) && (nBLR == 16)) {
        PSPLIT_HASKEY_128_96(uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
          #endif // UA_PARALLEL_128
          #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 16) {
        PSPLIT_HASKEY(Bucket_t, uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
          #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    { PSPLIT_HASKEY(Bucket_t, uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos); }
      #else // PSPLIT_PARALLEL
    PSPLIT_SEARCH_BY_KEY(uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos);
      #endif // PSPLIT_PARALLEL
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
      #ifdef PSPLIT_PARALLEL
          #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 32) {
        PSPLIT_HASKEY(Bucket_t, uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBLR == 24) {
        PSPLIT_HASKEY(Bucket_t, uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
          #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_HASKEY(Bucket_t, uint32_t, nBLR, piKeys, nPopCnt, iKey, nPos);
    }
      #else // PSPLIT_PARALLEL
    PSPLIT_SEARCH_BY_KEY(uint32_t, nBL, piKeys, nPopCnt, iKey, nPos);
      #endif // #else PSPLIT_PARALLEL
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
    //Word_t *pwKeysOrig = pwKeys;
    int nPopCntOrig = nPopCnt; (void)nPopCntOrig;
    // BINARY_SEARCH narrows the scope of the linear search that follows.
    unsigned nSplit;
    // Looks like we might want a loop threshold of 8 for
    // 64-bit keys at the top level.
    // And there's not much difference with threshold of
    // 16 or 64.
    // Not sure about 64-bit keys at a lower level or
    // 64-bit keys at the top level.
    SMETRICS(int nCompares = 0);
    int nKeysPerBucket = sizeof(Bucket_t) / sizeof(Word_t);
    while (nPopCnt >= cnBinarySearchThresholdWord) {
        //nSplit = ALIGN_UP(nPopCnt / 2, nKeysPerBucket);
        nSplit = (nPopCnt / 2) & ~(nKeysPerBucket - 1);
        if (BUCKET_HAS_KEY((Bucket_t *)&pwKeys[nSplit], wKey, nBL)) {
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
        if (pwKeys[nSplit] <= wKey) {
            pwKeys = &pwKeys[nSplit + nKeysPerBucket];
            nPopCnt -= nSplit + nKeysPerBucket;
        } else {
            nPopCnt = nSplit;
        }
        SMETRICS(++nCompares);
    }
    // What if one of nComparesB is not a miscompare? */
    // Call it a miscompare because it is an extra conditional branch.
  #if defined(BACKWARD_SEARCH_WORD)
    nPos = nPopCnt - 1;
    HASKEYB(Bucket_t, wKey, pwKeys, nPopCnt, nPos);
    SMETRICS(j__MisComparesM += nCompares);
    SMETRICS(++j__GetCallsM);
  #else // defined(BACKWARD_SEARCH_WORD)
    HASKEYF(Bucket_t, wKey, pwKeys, nPopCnt, nPos);
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
    int nPos;
  #if defined(PSPLIT_SEARCH_WORD)
    // No PSPLIT for nBLR == cnBitsPerWord.
    // We probably don't have a uniform distribution over the expanse.
    // We prefer binary search.
    if (nBLR != cnBitsPerWord) {
        nPos = 0;
      #ifdef PARALLEL_SEARCH_WORD
// Are we using parallel PSPLIT_HASKEY_WORD when we shouldn't be?
//  if ((sizeof(Bucket_t) > sizeof(Word_t))
//    #ifdef B_JUDYL
//      && (nPopCnt >= cnParallelSearchWordPopCntMinL)
//    #endif // B_JUDYL
//      )
        PSPLIT_HASKEY_WORD(Bucket_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
      #else // PARALLEL_SEARCH_WORD
        PSPLIT_SEARCH_BY_KEY_WORD(nBLR, pwKeys, nPopCnt, wKey, nPos);
      #endif // PARALLEL_SEARCH_WORD
        DBGX(printf("LHKW: returning %d\n", nPos >= 0));
        return nPos >= 0;
    }
  #endif // defined(PSPLIT_SEARCH_WORD)
  #if (cnBitsPerWord > 32)
      #if !defined(NO_BINARY_SEARCH_WORD) && defined(PARALLEL_SEARCH_WORD)
    if ((sizeof(Bucket_t) > sizeof(Word_t))
      #ifdef B_JUDYL
        && (nPopCnt >= cnParallelSearchWordPopCntMinL)
      #endif // B_JUDYL
        )
    {
        return BinaryHasKeyWord(pwKeys, wKey, nBLR, nPopCnt);
    } else
      #endif // !defined(NO_BINARY_SEARCH_WORD) && ...
  #endif // (cnBitsPerWord > 32)
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
// Neither Lookup nor Remove need to know where key should be if it is not in
// the list. Only Insert benefits from that information.
// Remove needs to know where the key is if it is in the list as does
// Lookup for JudyL. See ListHasKey and LocateKeyInList.
static int
SearchList(qp, int nBLR, Word_t wKey)
{
    qv;
  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
      // There is no need for a key size that is equal to or smaller than
      // whatever size yields a bitmap that will fit in a link.
    if (nBLR <= 8) {
        return SearchList8(qy, nBLR, wKey);
    }
      #endif // (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
    assert(nBLR > 8);
      #if (cnBitsInD1 <= 16)
    if (nBLR <= 16) {
        return SearchList16(qy, nBLR, wKey);
    }
      #endif // defined(cnBitsInD1 <= 16)
    assert(nBLR > 16);
      #if (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
    if (nBLR <= 32) {
        int nPopCnt = gnListPopCnt(qy, nBLR);
        return SearchList32(ls_piKeysNATX(pwr, nPopCnt), wKey, nBLR, nPopCnt);
    }
      #endif // (cnBitsInD1 <= 32) && (cnBitsPerWord > 32)
  #endif // defined(COMPRESSED_LISTS)
    int nPopCnt = gnListPopCnt(qy, nBLR);
    return SearchListWord(ls_pwKeysX(pwr, nBLR, nPopCnt), wKey, nBLR, nPopCnt);
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
    qv;
  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
    if (nBLR <= 8) { return ListHasKey8(qy, nBLR, wKey); }
      #endif // (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
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
  #ifdef BM_SW_CNT_IN_WR
      #if (cnBitsPerWord <= 32)
          #error BM_SW_CNT_IN_WR with cnBitsPerWord <= 32.
      #endif // (cnBitsPerWord <= 32)
    return (int)GetBits(*pwRoot, cnBitsCnt, cnLsbCnt) + 1;
  #else // BM_SW_CNT_IN_WR
    Word_t *pwBmWords = PWR_pwBm(pwRoot, wr_pwr(*pwRoot));
    int nLinks = 0;
    for (int nn = 0; nn < N_WORDS_SWITCH_BM; ++nn) {
        nLinks += __builtin_popcountll(pwBmWords[nn]
      #if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
                          & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
      #endif // defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
                                       );
    }
      #ifndef BM_SW_FOR_REAL
    assert(nLinks
        == (cnBitsPerWord
          #if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
               / 2
          #endif // defined(OFFSET_IN_SW_BM_WORD) || def(X_SW_BM_HALF_WORDS)
               * N_WORDS_SWITCH_BM));
      #endif // BM_SW_FOR_REAL
    return nLinks;
  #endif // #else BM_SW_CNT_IN_WR
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
        for (int nn = 0; nn < N_WORDS_SWITCH_BM; nn++) {
            wSwIndex += __builtin_popcountll(pwBmWords[nn])
                            * ((int)wDigit >= (nn + 1) * cnBitsPerWord);
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

#if defined(COMPRESSED_LISTS)
  #if (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)

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
    #define _LKIL8_DONE
    return SearchList8(qy, nBLR, wKey);
#endif // defined(OLD_LISTS)
#endif // !defined(POP_WORD_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(PP_IN_LINK) || (cnDummiesInList == 0)
#endif // !defined(POP_IN_WR_HB) && !defined(LIST_POP_IN_PREAMBLE)

#if defined(PSPLIT_SEARCH_8)
#ifdef PARALLEL_LOCATEKEY_8

  #ifndef _LKIL8_DONE
    int nPos;
  #ifdef LKIL8_ONE_BUCKET
#if defined(PARALLEL_128)
#if cnBitsInD1 == 8
#if cnListPopCntMaxDl1 <= 16
#ifdef OLD_LISTS
#if cnBitsMallocMask >= 4
#if cnDummiesInList == 0
    #define _LKIL8_DONE
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
  #ifdef PREFETCH_LOCATE_KEY_8_BEG_VAL
    // Prefetch the cache line before the keys.
    // Fetching the keys brings in 0 - 6 values assuming there are
    // no words between keys and values.
    // We'll end up with 8 - 14 values.
    BJL(PREFETCH(pwr - 8));
  #endif // PREFETCH_LOCATE_KEY_8_BEG_VAL
  #ifdef PREFETCH_LOCATE_KEY_8_END_VAL
    // And the one before that.
    BJL(PREFETCH(pwr - 16));
  #endif // PREFETCH_LOCATE_KEY_8_END_VAL
  #ifdef HK40_EXPERIMENT
    nPos = LocateKey40(pwr, wKey);
  #else // HK40_EXPERIMENT
    nPos = LocateKey128((__m128i*)pwr, wKey, 8);
  #endif // #else HK40_EXPERIMENT
    SMETRICS(j__DirectHits += (nPos >= 0));
    return nPos;
#endif // cnDummiesInList == 0
#endif // cnBitsMallocMask >= 4
#endif // OLD_LISTS
#endif // cnListPopCntMaxDl1 <= 16
#endif // cnBitsInD1 == 8
#endif // defined(PARALLEL_128)
  #endif // LKIL8_ONE_BUCKET
  #endif // #ifndef _LKIL8_DONE

  #ifndef _LKIL8_DONE
    int nPopCnt = gnListPopCnt(qy, nBLR);
    uint8_t *pcKeys = ls_pcKeys(pwr, PWR_xListPopCnt(&wRoot, pwr, 8));
    uint8_t cKey = (uint8_t)wKey;
    nPos = 0;
    // PACK_L1_VALUES exists only because its absence is an incomplete quick
    // hack to see the performance of a list leaf with an uncompressed value
    // area. USE_LOCATE_FOR_NO_PACK chooses LOCATEKEY rather than HASKEY for
    // NO_PACK_L1_VALUES.
    // At present, LocateKeyInList is used for NextEmpty and B_JUDYL Lookup.
  #ifdef B_JUDYL
#if defined(PACK_L1_VALUES) || defined(USE_LOCATE_FOR_NO_PACK)
    PSPLIT_LOCATEKEY(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
#else // defined(PACK_L1_VALUES) || defined(USE_LOCATE_FOR_NO_PACK)
    PSPLIT_HASKEY(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
#endif // #else defined(PACK_L1_VALUES) || defined(USE_LOCATE_FOR_NO_PACK)
  #else // B_JUDYL
    PSPLIT_LOCATEKEY(Bucket_t, uint8_t, 8, pcKeys, nPopCnt, cKey, nPos);
  #endif // B_JUDYL
    return nPos;
  #endif // #ifndef _LKIL8_DONE

#endif // PARALLEL_LOCATEKEY_8
#endif // defined(PSPLIT_SEARCH_8)

  #ifndef _LKIL8_DONE
    return SearchList8(qy, nBLR, wKey);
  #endif // #ifndef _LKIL8_DONE
}
  #endif // (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)

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
      #ifdef PSPLIT_PARALLEL
          #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 16) {
        PSPLIT_LOCATEKEY(Bucket_t, uint16_t, 16, psKeys, nPopCnt, sKey, nPos);
    } else
          #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_LOCATEKEY(Bucket_t,
                         uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos);
    }
      #else // PSPLIT_PARALLEL
    PSPLIT_SEARCH_BY_KEY(uint16_t, nBLR, psKeys, nPopCnt, sKey, nPos);
      #endif // #else PSPLIT_PARALLEL
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
      #ifdef PSPLIT_PARALLEL
           #if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    if (nBLR == 32) {
        PSPLIT_LOCATEKEY(Bucket_t, uint32_t, 32, piKeys, nPopCnt, iKey, nPos);
    } else if (nBLR == 24) {
        PSPLIT_LOCATEKEY(Bucket_t, uint32_t, 24, piKeys, nPopCnt, iKey, nPos);
    } else
          #endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    {
        PSPLIT_LOCATEKEY(Bucket_t,
                         uint32_t, nBLR, piKeys, nPopCnt, iKey, nPos);
    }
      #else // PSPLIT_PARALLEL
    PSPLIT_SEARCH_BY_KEY(uint32_t, nBLR, piKeys, nPopCnt, iKey, nPos);
      #endif // #else PSPLIT_PARALLEL
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
    int nPos;
  #if defined(PSPLIT_SEARCH_WORD)
    if (nBLR != cnBitsPerWord) {
        nPos = 0;
      #ifdef PARALLEL_SEARCH_WORD
        PSPLIT_LOCATEKEY_WORD(Bucket_t, nBLR, pwKeys, nPopCnt, wKey, nPos);
      #else // PARALLEL_SEARCH_WORD
        PSPLIT_SEARCH_BY_KEY_WORD(nBLR, pwKeys, nPopCnt, wKey, nPos);
      #endif // PARALLEL_SEARCH_WORD
        DBGX(printf("LKILW: returning %d\n", nPos));
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
    qv;
  #if defined(COMPRESSED_LISTS)
      #if (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
    if (nBLR <= 8) {
        return LocateKeyInList8(qy, nBLR, wKey);
    }
      #endif // (cnBitsInD1 <= 8) || defined(USE_XX_SW_ONLY_AT_DL2)
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

#ifdef XX_LISTS
  #ifdef B_JUDYL
#define SubexpansePopCnt  SubexpansePopCntL
  #else // B_JUDYL
#define SubexpansePopCnt  SubexpansePopCnt1
  #endif // #else B_JUDYL
int SubexpansePopCnt(qp, Word_t wKey);
#endif // XX_LISTS

// Get the pop count of the tree/subtree represented by (*pwRoot, nBL).
// We can't handle T_XX_LIST by virtue of our inadequate parameter list.
static Word_t
GetPopCnt(Word_t *pwRoot, int nBL)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot);
    Word_t wRoot = *pwRoot;
    Word_t wPopCnt;
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        if (nBL <= cnLogBitsPerWord) {
            wPopCnt
                = __builtin_popcountll(wRoot
                    // Is this mask really necessary?
                    // Or can we count on the bits being zero?
                    & ((nBL >= cnLogBitsPerWord)
                        ? (Word_t)-1 : MSK(EXP(nBL))));
        } else {
            Word_t *pwLn = (Word_t*)pLn;
            wPopCnt = 0;
            for (int i = 0; i < (int)EXP(nBL - cnLogBitsPerWord); ++i)
            {
                wPopCnt += __builtin_popcountll(pwLn[i]);
            }
        }
    } else {
        Word_t *pwr = wr_pwr(wRoot);
        int nType = wr_nType(wRoot);
        if (pwr != NULL) {
            DBGC(printf("GetPopCnt nBL %d pwr %p nType %d\n", nBL, pwr, nType));
        }
        if (tp_bIsSwitch(nType)) {
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            if (nBL >= cnBitsPerWord) {
                wPopCnt = SumPopCnt(pwRoot, cnBitsPerWord);
            } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            { wPopCnt = gwPopCnt(qy, gnBLR(qy)); }
        } else switch (nType) {
      #if defined(EMBED_KEYS)
        case T_EMBEDDED_KEYS:
            wPopCnt
                = ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
                        && (wRoot == WROOT_NULL))
                    ? 0 : wr_nPopCnt(wRoot, nBL);
            break;
      #endif // defined(EMBED_KEYS)
        case T_LIST:
      #if defined(UA_PARALLEL_128)
        case T_LIST_UA:
      #endif // defined(UA_PARALLEL_128)
            wPopCnt = 0;
      #if ! defined(SEPARATE_T_NULL)
            if (pwr != NULL)
      #endif // ! defined(SEPARATE_T_NULL)
            {
                wPopCnt = gnListPopCnt(qy, gnListBLR(qy));
                assert(wPopCnt != 0);
            }
            break;
      #ifdef BITMAP
      #ifdef UNPACK_BM_VALUES
        case T_UNPACKED_BM:
      #endif // UNPACK_BM_VALUES
          #if defined(SKIP_TO_BITMAP)
        case T_SKIP_TO_BITMAP:
              #if defined(PP_IN_LINK)
            // From where should we get pop count for PP_IN_LINK?
            // It exists in the bitmap but also in the link.
            // But there is no link at the top. KISS.
              #endif // defined(PP_IN_LINK)
          #endif // defined(SKIP_TO_BITMAP)
        case T_BITMAP:
            wPopCnt = gwBitmapPopCnt(qy, gnBLR(qy));
            break;
      #endif // BITMAP
      #ifdef SEPARATE_T_NULL
        case T_NULL:
            break;
      #endif // SEPARATE_T_NULL
      #ifdef XX_LISTS
        case T_XX_LIST:
            assert(0); // Our parameters are inadequate. Use GetPopCntX.
            // break;
      #endif // XX_LISTS
        default:
            if (wPopCntTotal < 0x1000) {
                DBGC(Dump(pwRootLast, 0, cnBitsPerWord));
            }
            assert(0);
            wPopCnt = 0; // make cc -UDEBUG happy
        }
        if (pwr != NULL) {
            DBGC(printf("GetPopCnt %zd\n", wPopCnt));
        }
    }
    return wPopCnt;
}

// Return the total number of keys in the subtrees rooted by the specified
// consecutive links in a switch.
// nBLLoop is the number of bits left to decode after identifying each link
// before any skip specified by the link is applied.
// nBL applies to pLinks[x].
// If (pLinks, nLinkCnt) contains links to any shared lists, then it must
// contain all of the links to any shared lists it contains.
static Word_t
CountSwLoop(int nBL, Link_t *pLinks, int nLinkCnt)
{
    Word_t wPopCnt = 0;
    //DBGC(printf("\n# CountSwLoop nLinkCnt %d\n", nLinkCnt));
    for (int i = 0; i < nLinkCnt; ++i) {
        Link_t *pLn = &pLinks[i];
  #ifdef XX_LISTS
        Word_t wRoot = pLn->ln_wRoot;
        int nType = wr_nType(wRoot);
        if (nType == T_XX_LIST) {
            Word_t *pwr = wr_pwr(wRoot); (void)pwr;
            int nBLR = gnListBLR(qy);
            int nXxCnt = EXP(nBLR - nBL);
            if ((i + nXxCnt <= nLinkCnt)) {
                wPopCnt += PWR_xListPopCnt(&pLn->ln_wRoot, pwr, nBLR);
                i += nXxCnt - 1;
                continue;
            }
        }
  #endif // XX_LISTS
        wPopCnt += GetPopCnt(&pLn->ln_wRoot, nBL);
    }
    //DBGC(printf("# CountSwLoop returning wPopCnt %zd\n", wPopCnt));
    return wPopCnt;
}

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// Sum up the pop counts of all of the links in the specified switch.
// It assumes *pwRoot is in a link to a switch.
// nBL is the bits left to decode after getting to pwRoot.
// If *pwRoot is in a skip link, then nBL must be adjusted by the skip
// amount to get the digits left at the next node.
static Word_t
SumPopCnt(Word_t *pwRoot, int nBL)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot;
    int nType = wr_nType(wRoot); (void)nType;
    Word_t *pwr = wr_pwr(wRoot);
    int nBLR = GetBLR(pwRoot, nBL);
    int nBW = nBLR_to_nBW(nBLR);
    int nLinkCnt =
  #if defined(CODE_BM_SW)
        tp_bIsBmSw(nType) ? BmSwLinkCnt(qy) :
  #endif // defined(CODE_BM_SW)
        (int)EXP(nBW);
    return CountSwLoop(nBLR - nBW, pwr_pLinks((Switch_t*)pwr), nLinkCnt);
}

#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#endif // (cnDigitsPerWord > 1)

#if defined(RAMMETRICS)

extern Word_t j__RequestedWordsTOT;
extern Word_t j__AllocWordsTOT;
extern Word_t j__MmapWordsTOT; // mmap

//extern Word_t j__AllocWordsJBL;  // linear branch
extern Word_t j__AllocWordsJBB;  // bitmap branch
extern Word_t j__AllocWordsJBU;  // uncompressed branch
extern Word_t j__AllocWordsJLB1; // bitmap leaf at dl1
extern Word_t j__AllocWordsJLL[8];
//#define j__AllocWordsJLLW  j__AllocWordsJLL[0]
//#define j__AllocWordsJLL1  j__AllocWordsJLL[1]
//...
//#define j__AllocWordsJLL7  j__AllocWordsJLL[7]

#ifndef B_JUDYL
// Coopt j__AllocWordsJV for JLB2 big bitmap at digit 2.
extern Word_t j__AllocWordsJV;   // value area
#define j__AllocWordsJLB2  j__AllocWordsJV // big bitmap leaf at dl2
#endif // B_JUDYL

#endif // defined(RAMMETRICS)

// This is a good place to test ifdef constraints -- at the end of the last
// header file -- after all macros have been defined.
// This is not a good place to use one macro to affect the definition of
// another -- that should be done in bdefines.h.

// If we're using the type field to encode type and level then we can only
// skip to a regular switch.
#ifndef LVL_IN_WR_HB
#ifndef LVL_IN_PP
  // We have a different type value for each level. So we can only support
  // skip to a regular switch.
  #ifdef SKIP_TO_BM_SW
    #error No SKIP_TO_BM_SW with level in type.
  #endif // SKIP_TO_BM_SW
  #ifdef SKIP_TO_XX_SW
    #error No SKIP_TO_XX_SW with level in type.
  #endif // SKIP_TO_XX_SW
  #ifdef SKIP_TO_LIST_SW
    #error No SKIP_TO_LIST_SW with level in type.
  #endif // SKIP_TO_LIST_SW
  #ifdef SKIP_TO_LIST
    #error No SKIP_TO_LIST with level in type.
  #endif // SKIP_TO_LIST
  #ifdef SKIP_TO_BITMAP
    #error No SKIP_TO_BITMAP with level in type.
  #endif // SKIP_TO_BITMAP
#endif // #ifndef LVL_IN_PP
#endif // #ifndef LVL_IN_WR_HB

// Pad the list with copies of the last real key in the list so the
// length of the list from the first key through the last copy of the
// last real key is an integral multiple of cnBytesListLenAlign.
// cnBytesListLenAlign is set to the size of a parallel search bucket.
// This way we don't need any special handling in the parallel search
// code to handle a partial final bucket.
#define PAD(_pxKeys, _nPopCnt) \
{ \
    if (ALIGN_LIST_LEN(sizeof(*(_pxKeys)), (_nPopCnt))) { \
        for (int nn = (_nPopCnt); \
             (nn * sizeof(*(_pxKeys))) % sizeof(Bucket_t); ++nn) \
        { \
            (_pxKeys)[nn] = (_pxKeys)[nn - 1]; \
        } \
    } \
}

#endif // ( ! defined(_B_H_INCLUDED) )
