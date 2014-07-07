
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

// Default is -UBM_SWITCH -UBM_IN_LINK -UBM_SWITCH_FOR_REAL.
#if defined(BM_IN_LINK) || defined(BM_SWITCH_FOR_REAL)
#if ! defined(BM_SWITCH)
#define BM_SWITCH
#endif // ! defined(BM_SWITCH)
#endif // defined(BM_IN_LINK) || defined(BM_SWITCH_FOR_REAL)

// Choose max list length.
// Mind sizeof(ll_nPopCnt) and the maximum value it implies.
// Default is cnListPopCntMax16 = 64.
// Default is cnListPopCntMax32 = 32.
// Default is cnListPopCntMax64 = 236.
#if defined(cwListPopCntMax)
#undef  cnListPopCntMax64
#define cnListPopCntMax64  cwListPopCntMax
#undef  cnListPopCntMax32
#define cnListPopCntMax32  cwListPopCntMax
#undef  cnListPopCntMax16
#define cnListPopCntMax16  cwListPopCntMax
#endif // defined(cwListPopCntMax)

#if ! defined(cnListPopCntMax64)
#define cnListPopCntMax64  236
#endif // ! defined(cnListPopCntMax64)
#if ! defined(cnListPopCntMax32)
#define cnListPopCntMax32  32
#endif // ! defined(cnListPopCntMax32)
#if ! defined(cnListPopCntMax16)
#define cnListPopCntMax16  64
#endif // ! defined(cnListPopCntMax16)

#if ! defined(cwListPopCntMax)
#define cwListPopCntMax \
    (cnListPopCntMax64 + cnListPopCntMax32 + cnListPopCntMax16)
#endif // ! defined(cwListPopCntMax)

// Default is -DSORT_LISTS.
#if defined(NO_SORT_LISTS)
#if defined(CONTINUE_FIRST) || defined(FAIL_FIRST) || defined(SUCCEED_FIRST) \
    || defined(SPLIT_SEARCH) || defined(SPLIT_SEARCH_LOOP) \
    || defined(END_CHECK) \
    || defined(CONTINUE_FIRST_32) || defined(FAIL_FIRST_32) \
    || defined(SUCCEED_FIRST_32) \
    || defined(SPLIT_SEARCH_32) || defined(SPLIT_SEARCH_LOOP_32) \
    || defined(END_CHECK_32)
    || defined(CONTINUE_FIRST_16) || defined(FAIL_FIRST_16) \
    || defined(SUCCEED_FIRST_16) \
    || defined(SPLIT_SEARCH_16) || defined(SPLIT_SEARCH_LOOP_16) \
    || defined(END_CHECK_16)
#error "Can't have fancy search with NO_SORT_LISTS."
#endif // defined(CONTINUE_FIRST) || defined(FAIL_FIRST) || ...
#else // defined(NO_SORT_LISTS)
#undef  SORT_LISTS
#define SORT_LISTS
#endif // defined(NO_SORT_LISTS)

// Default is -DEND_CHECK.
#if ! defined(NO_END_CHECK)
#undef  END_CHECK
#define END_CHECK
#endif // defined(NO_END_CHECK)

// Default is -DEND_CHECK_32.
#if ! defined(NO_END_CHECK_32)
#undef  END_CHECK_32
#define END_CHECK_32
#endif // defined(NO_END_CHECK_32)

// Default is -DEND_CHECK_16.
#if ! defined(NO_END_CHECK_16)
#undef  END_CHECK_16
#define END_CHECK_16
#endif // defined(NO_END_CHECK_16)

// Default is -DSPLIT_SEARCH -DSPLIT_SEARCH_LOOP -URATIO_SPLIT.
#if ! defined(NO_SPLIT_SEARCH_LOOP)
#undef  SPLIT_SEARCH_LOOP
#define SPLIT_SEARCH_LOOP
#endif // ! defined(NO_SPLIT_SEARCH_LOOP)

#if defined(SPLIT_SEARCH_LOOP)
#undef  SPLIT_SEARCH
#define SPLIT_SEARCH
#endif // defined(SPLIT_SEARCH_LOOP)

// Default is -USPLIT_SEARCH_32 -USPLIT_SEARCH_LOOP_32 -URATIO_SPLIT_32.
#if defined(SPLIT_SEARCH_LOOP_32)
#undef  SPLIT_SEARCH_32
#define SPLIT_SEARCH_32
#endif // defined(SPLIT_SEARCH_LOOP_32)

// Default is -USPLIT_SEARCH_16 -USPLIT_SEARCH_LOOP_16 -URATIO_SPLIT_16.
#if defined(SPLIT_SEARCH_LOOP_16)
#undef  SPLIT_SEARCH_16
#define SPLIT_SEARCH_16
#endif // defined(SPLIT_SEARCH_LOOP_16)

// Default is -DCONTINUE_FIRST -UFAIL_FIRST -USUCCEED_FIRST.
#if defined(CONTINUE_FIRST)
  #if defined(FAIL_FIRST) || defined(SUCCEED_FIRST)
    #error "Can't have CONTINUE_FIRST with (FAIL|SUCCEED)_FIRST."
  #endif // defined(FAIL_FIRST) || defined(SUCCEED_FIRST)
#elif defined(FAIL_FIRST)
  #if defined(SUCCEED_FIRST)
    #error "Can't have FAIL_FIRST with SUCCEED_FIRST."
  #endif // defined(SUCCEED_FIRST)
#elif ! defined(SUCCEED_FIRST)
  #define CONTINUE_FIRST
#endif // ...

// Default is -DCONTINUE_FIRST_16 -UFAIL_FIRST_16 -USUCCEED_FIRST_16.
#if defined(CONTINUE_FIRST_16)
  #if defined(FAIL_FIRST_16) || defined(SUCCEED_FIRST_16)
    #error "Can't have CONTINUE_FIRST_16 with (FAIL|SUCCEED)_FIRST_16."
  #endif // defined(FAIL_FIRST_16) || defined(SUCCEED_FIRST_16)
#elif defined(FAIL_FIRST_16)
  #if defined(SUCCEED_FIRST_16)
    #error "Can't have FAIL_FIRST_16 with SUCCEED_FIRST_16."
  #endif // defined(SUCCEED_FIRST_16)
#elif ! defined(SUCCEED_FIRST)
  #define CONTINUE_FIRST_16
#endif // ...

#if defined(SPLIT_SEARCH)
// Default is cnSplitSearchThresholdWord = 16.
#if ! defined(cnSplitSearchThresholdWord)
#define cnSplitSearchThresholdWord  16
#endif // ! defined(cnSplitSearchThresholdWord)
// Default is cnSplitSearchThresholdInt = 40.
#if ! defined(cnSplitSearchThresholdInt)
#define cnSplitSearchThresholdInt  40
#endif // ! defined(cnSplitSearchThresholdInt)
// Default is cnSplitSearchThresholdShort = 64.
#if ! defined(cnSplitSearchThresholdShort)
#define cnSplitSearchThresholdShort  64
#endif // ! defined(cnSplitSearchThresholdShort)
#undef  SORT_LISTS
#define SORT_LISTS
#endif // defined(SPLIT_SEARCH)

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
// Default is cnBitsPerDigit = cnLogBitsPerWord.
#if ! defined(cnBitsPerDigit)
    #define cnBitsPerDigit cnLogBitsPerWord
#else // ! defined(cnBitsPerDigit)
    #if (cnBitsPerDigit <= 0) || (cnBitsPerDigit > cnBitsPerWord)
        #undef  cnBitsPerDigit
        #define cnBitsPerDigit  cnBitsPerWord
    #endif // (cnBitsPerDigit <= 0) || (cnBitsPerDigit > cnBitsPerWord)
#endif // ! defined(cnBitsPerDigit)

// Choose bottom.
// Bottom is where Bitmap is created automatically.
// We count digits up from there.
// Default is cnBitsAtBottom = cnLogBitsPerWord.
// Old default is cnBitsAtBottom = MAX(16, minimum allowed by cnBitsPerDigit).
#if ! defined(cnBitsAtBottom)
#undef  cnBitsAtBottom
#define cnBitsAtBottom  cnLogBitsPerWord
#endif // ! defined(cnBitsAtBottom)

// Default is -DBITMAP_ANYWHERE.
#if ! defined(NO_BITMAP_ANYWHERE)
#undef  BITMAP_ANYWHERE
#define BITMAP_ANYWHERE
#endif // ! defined(NO_BITMAP_ANYWHERE)

#define cnDigitsPerWord \
    (DIV_UP(cnBitsPerWord - cnBitsAtBottom, cnBitsPerDigit) + 1)

// Default is -DEMBED_KEYS which implies T_ONE.
// EMBED_KEYS implies T_ONE
#if ! defined(NO_EMBED_KEYS)
#undef  EMBED_KEYS
#define EMBED_KEYS
#endif // ! defined(NO_EMBED_KEYS)

#if defined(EMBED_KEYS)
#undef  T_ONE
#define T_ONE
#endif // defined(EMBED_KEYS)

#define T_NULL    0
#if defined(T_ONE)
#undef T_ONE
#define T_ONE     1
#endif // defined(T_ONE)
#define T_BITMAP  2
#define T_LIST    3

// Default is -UBPD_TABLE, i.e. -DNO_BPD_TABLE.
#if defined(BPD_TABLE)

// Use lookup tables (which theoretically support depth-based bits per digit)
// instead of a constant bits-per-digit throughout the tree.

extern const unsigned anDL_to_nBL[];
extern const unsigned anDL_to_nBitsIndexSz[];

// this one is not used in the lookup performance path
#define nDL_to_nBitsIndexSz(_nDL)     anDL_to_nBitsIndexSz[_nDL]
#define nDL_to_nBitsIndexSzNAX(_nDL)  nDL_to_nBitsIndexSz(_nDL)
#define nDL_to_nBitsIndexSzNAB(_nDL)  nDL_to_nBitsIndexSz(_nDL)
#define nDL_to_nBitsIndexSzNAT(_nDL)  nDL_to_nBitsIndexSz(_nDL)

// this one is not used in the lookup performance path
#define cnBitsIndexSzAtTop  nDL_to_nBitsIndexSz(cnDigitsPerWord)

// this one is not used in the lookup performance path
#define nDL_to_nBL(_nDL) \
    (((_nDL) < cnDigitsPerWord) ? anDL_to_nBL[_nDL] : cnBitsPerWord)

#define nDL_to_nBL_NAT(_nDL)  nDL_to_nBL(_nDL)

#else // defined(BPD_TABLE)

#define cnBitsIndexSzAtTop \
    (cnBitsPerWord - cnBitsAtBottom - (cnDigitsPerWord - 2) * cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAX(_nDL)  (cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAB(_nDL) \
    (((_nDL) == cnDigitsPerWord) ? cnBitsIndexSzAtTop : cnBitsPerDigit)

#define nDL_to_nBitsIndexSzNAT(_nDL) \
    (((_nDL) == 1) ? cnBitsAtBottom : cnBitsPerDigit)

// this one is not used in the lookup performance path
#define nDL_to_nBitsIndexSz(_nDL) \
    (((_nDL) == cnDigitsPerWord) \
        ? cnBitsIndexSzAtTop : nDL_to_nBitsIndexSzNAT(_nDL))

#define nDL_to_nBL_NAT(_nDL) \
    (((_nDL) - 1) * cnBitsPerDigit + cnBitsAtBottom)

#define nDL_to_nBL(_nDL) \
    (((_nDL) == cnDigitsPerWord) ? cnBitsPerWord : nDL_to_nBL_NAT(_nDL))

#endif // defined(BPD_TABLE)

// this one is not used in the lookup performance path
#define nBL_to_nDL(_nBL) \
     (DIV_UP((_nBL) - cnBitsAtBottom, cnBitsPerDigit) + 1)

// this one is not used in the lookup performance path
#define nBL_to_nDL_NotAtTop(_nBL)  nBL_to_nDL(_nBL)

#if defined RAMMETRICS
#define METRICS(x)  (x)
#else // defined RAMMETRICS
#define METRICS(x)
#endif // defined RAMMETRICS

// Default is -USEARCH_METRICS.
#if defined SEARCH_METRICS
#define SMETRICS(x)  (x)
#else // defined SEARCH_METRICS
#define SMETRICS(x)
#endif // defined SEARCH_METRICS

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

#if defined(T_ONE)

#if (cnBitsPerWord == 64)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(88 / (_nBL))
#elif (cnBitsPerWord == 32)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(44 / (_nBL))
#else
#error "Invalid cnBitsPerWord."
#endif

// Pop cnt bits are just above the type field.
// A value of zero means a pop cnt of one. 

#define     wr_nPopCnt(_wr, _nBL) \
    ((((_wr) >> 4) & MSK(nBL_to_nBitsPopCntSz(_nBL))) + 1)

#define set_wr_nPopCnt(_wr, _nBL, _nPopCnt) \
    ((_wr) &= ~(MSK(nBL_to_nBitsPopCntSz(_nBL)) << cnBitsMallocMask), \
        (_wr) |= ((_nPopCnt) - 1) << cnBitsMallocMask)

#endif // defined(T_ONE)

// Default is -UDL_IN_TYPE_IS_ABSOLUTE, i.e. -DTYPE_IS_RELATIVE.
#if defined(DL_IN_TYPE_IS_ABSOLUTE)

#define tp_to_nDL(_tp)   ((_tp)  - T_LIST)
#define nDL_to_tp(_nDL)  ((_nDL) + T_LIST)

#define     wr_nDL(_wr)     (tp_to_nDL(wr_nType(_wr)))
#define set_wr_nDL(_wr, _nDL) \
    (set_wr_nType((_wr), nDL_to_tp(_nDL)))

#define     wr_bIsSwitchDL(_wr, _tp, _nDL) \
   ((_tp) = wr_nType(_wr), (_nDL) = tp_to_nDL(_tp), tp_bIsSwitch(_tp))

#else // defined(DL_IN_TYPE_IS_ABSOLUTE)

#undef  TYPE_IS_RELATIVE
#define TYPE_IS_RELATIVE

#define T_NO_SKIP_SWITCH  (T_LIST + 1)

#define tp_to_nDS(_tp)   ((_tp)  - T_NO_SKIP_SWITCH)
#define nDS_to_tp(_nDS)  ((_nDS) + T_NO_SKIP_SWITCH)

#define     wr_nDS(_wr)        (tp_to_nDS(wr_nType(_wr)))
#define set_wr_nDS(_wr, _nDS)  (set_wr_nType((_wr), nDS_to_tp(_nDS)))

#define     wr_bIsSwitchDS(_wr, _tp, _nDS) \
   ((_tp) = wr_nType(_wr), (_nDS) = tp_to_nDS(_tp), tp_bIsSwitch(_tp))

#endif // defined(DL_IN_TYPE_IS_ABSOLUTE)

#define     tp_bIsSwitch(_tp)          ((_tp) > T_LIST)
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
#define PWR_wPrefixPop(_pwRoot, _pwr)  (((Switch_t *)(_pwr))->sw_wPrefixPop)
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

#define     pwr_pLinks(_pwr)  (((Switch_t *)(_pwr))->sw_aLinks)

#if defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr) \
    (STRUCT_OF((_pwRoot), Link_t, ln_wRoot)->ln_awBm)
#else // defined(BM_IN_LINK)
#define     PWR_pwBm(_pwRoot, _pwr)  (((Switch_t *)(_pwr))->sw_awBm)
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
#define FIRST_KEY  0

#else // defined(PP_IN_LINK)

#define     ls_wPopCnt(_ls)        (((ListLeaf_t *)(_ls))->ll_asKeys[0])
#define set_ls_wPopCnt(_ls, _cnt)  (ls_wPopCnt(_ls) = (_cnt))

// Index of first key within leaf (for all cases).
#define FIRST_KEY  1

#endif // defined(PP_IN_LINK)

// For PP_IN_LINK ls_pxKeys macros are only valid not at top or for
// T_ONE - not T_LIST - at top.
#if defined(COMPRESSED_LISTS)
#define     ls_pcKeys(_ls)    (&((ListLeaf_t *)(_ls))->ll_acKeys[FIRST_KEY])
#define     ls_psKeys(_ls)    (&((ListLeaf_t *)(_ls))->ll_asKeys[FIRST_KEY])
#if (cnBitsPerWord > 32)
#define     ls_piKeys(_ls)    (&((ListLeaf_t *)(_ls))->ll_aiKeys[FIRST_KEY])
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
#define     ls_pwKeys(_ls)    (&((ListLeaf_t *)(_ls))->ll_awKeys[FIRST_KEY])

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
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
    Word_t sw_awBm[N_WORDS_SWITCH_BM];
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
#if (cnDummiesInSwitch != 0)
    Word_t sw_awDummies[cnDummiesInSwitch];
#endif // (cnDummiesInSwitch != 0)
    Link_t sw_aLinks[1]; // variable size
} Switch_t;

Status_t Insert(Word_t *pwRoot, Word_t wKey, unsigned nBL);
Status_t Remove(Word_t *pwRoot, Word_t wKey, unsigned nBL);

Status_t InsertGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDL, Word_t wRoot);

Status_t RemoveGuts(Word_t *pwRoot,
                    Word_t wKey, unsigned nDL, Word_t wRoot);

Word_t FreeArrayGuts(Word_t *pwRoot,
                     Word_t wPrefix, unsigned nBL, int bDump);

Word_t OldBitmap(Word_t *pwr);

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

#endif // ( ! defined(_B_H_INCLUDED) )

