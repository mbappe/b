
#if ( ! defined(_BDEFINES_H_INCLUDED) )
#define _BDEFINES_H_INCLUDED

// If B_JUDYL is defined then we are building JudyL.
// If B_JUDYL is not defined then we are building Judy1.
// JudyL and Judy1 have different constraints and different defaults.
// We may build both Judy1 and JudyL with a single make, hence with
// a single set of -D options. Maybe we need to enhance the Makefile
// to have JUDY1_DEFINES and JUDYL_DEFINES.
// We don't build both 64-bit and 32-bit with a single make, but maybe
// we should. And maybe with DEFINES_32 and DEFINES_64.
// And maybe JUDY[1L]_DEFINES_(32|64).

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

// Apply JudyL-specific constraints to ifdefs from the build process.
#ifdef B_JUDYL

  #undef  NO_UA_PARALLEL_128
  #define NO_UA_PARALLEL_128

  #ifdef ALLOW_EMBEDDED_BITMAP
    #error No ALLOW_EMBEDDED_BITMAP for B_JUDYL.
  #endif // ALLOW_EMBEDDED_BITMAP
  #undef  NO_ALLOW_EMBEDDED_BITMAP
  #define NO_ALLOW_EMBEDDED_BITMAP

  // Allow DEFINES=-D<blah> on shared make command line for JUDY1.
  #undef CODE_XX_SW
  #undef NO_TYPE_IN_XX_SW
  #undef SKIP_TO_XX_SW
  #undef USE_XX_SW
  #undef USE_XX_SW_ONLY_AT_DL2
  #undef USE_XX_SW_AT_DLX

  // Disabling PARALLEL_SEARCH_WORD helps with worst case memory usage.
  #ifndef PARALLEL_SEARCH_WORD
    #undef NO_PARALLEL_SEARCH_WORD
    #define NO_PARALLEL_SEARCH_WORD
  #endif // PARALLEL_SEARCH_WORD

  // Default is to use a packed value area for a bitmap leaf.
  #ifndef   NO_PACK_BM_VALUES
  #undef       PACK_BM_VALUES
  #define      PACK_BM_VALUES
  #endif // NO_PACK_BM_VALUES

  // Default is to use a packed value area for a list leaf.
  #ifndef   NO_PACK_L1_VALUES
  #undef       PACK_L1_VALUES
  #define      PACK_L1_VALUES
  #endif // NO_PACK_L1_VALUES

#endif // B_JUDYL

// Default is POP_WORD unless NO_POP_WORD.
// POP_WORD makes gwPopCnt faster.
// gwPopCnt showed up as a heavy hitter in the profile.
#ifndef   NO_POP_WORD
  #undef     POP_WORD
  #define    POP_WORD
#endif // NO_POP_WORD

// Default is -UCODE_XX_SW.

// Default is -UNO_TYPE_IN_XX_SW.
// NO_TYPE_IN_XX_SW implies CODE_XX_SW.
// Should it imply USE_XX_SW?
#ifdef NO_TYPE_IN_XX_SW
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // NO_TYPE_IN_XX_SW

// SKIP_TO_XX_SW implies CODE_XX_SW.
// Should it imply USE_XX_SW?
#ifdef SKIP_TO_XX_SW
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // SKIP_TO_XX_SW

// USE_XX_SW_ONLY_AT_DL2 started life as a flag to preserve legacy USE_XX_SW
// behavior of creating XX_SW only at DL2 and doubling it as necessary to
// have only embedded keys and avoid blow-outs into external lists or branches
// or bitmaps. Keep doubling all the way through 8-bit and 7-bit embedded keys
// until we end up with one big bitmap at DL2.
// The t_bitmap code in Lookup assumes there are no bitmaps at DL1.
#ifdef    USE_XX_SW_ONLY_AT_DL2
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // USE_XX_SW_ONLY_AT_DL2

#ifdef    USE_XX_SW_AT_DLX
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // USE_XX_SW_AT_DLX

// Default is -UUSE_XX_SW.
// USE_XX_SW implies CODE_XX_SW.
// Time runs with USE_XX_SW take a long time because of insert times.
// USE_XX_SW doesn't work on JudyL or 32-bit yet.
#if defined(USE_XX_SW)
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // defined(USE_XX_SW)

#ifdef USE_XX_SW
  #undef  USE_XX_SW_ONLY_AT_DL2
  #ifndef USE_XX_SW_AT_DLX
    #define USE_XX_SW_ONLY_AT_DL2
  #endif // #ifndef USE_XX_SW_AT_DLX
#endif // USE_XX_SW

#if (cnBitsPerWord > 32)
  #if       !defined(NO_LVL_IN_WR_HB) && !defined(LVL_IN_SW)
    #undef              LVL_IN_WR_HB
    #define             LVL_IN_WR_HB
  //                 NO_LVL_IN_SW is not necessary.
  #endif // !defined(NO_LVL_IN_WR_HB) && !defined(LVL_IN_SW)
#endif // (cnBitsPerWord > 32)

// Define USE_BM_SW by default.
#ifndef   NO_USE_BM_SW
  #undef     USE_BM_SW
  #define    USE_BM_SW
#endif // NO_USE_BM_SW

// Default is -DPSPLIT_SEARCH_WORD -UPSPLIT_SEARCH_XOR_WORD.
// Default is -UNO_BINARY_SEARCH_WORD -UBACKWARD_SEARCH_WORD.
// PSPLIT_SEARCH_WORD does not apply at nBL == cnBitsPerDigit.
// If -DNO_PSPLIT_SEARCH_WORD, then binary search for nBL != cnBitsPerDigit
// unless -DNO_BINARY_SEARCH_WORD.
#ifndef   NO_PSPLIT_SEARCH_WORD
    #undef   PSPLIT_SEARCH_WORD
    #define  PSPLIT_SEARCH_WORD
#endif // NO_PSPLIT_SEARCH_WORD

// Default is -DPARALLEL_SEARCH_WORD.
#ifndef NO_PARALLEL_SEARCH_WORD
    #undef   PARALLEL_SEARCH_WORD
    #define  PARALLEL_SEARCH_WORD
#endif // NO_PARALLEL_SEARCH_WORD

// Choose conditional features and tuning parameters by #if, #define
// and #undef.
// E.g. DEBUG, RAMMETRICS, GUARDBAND.
// E.g. SKIP_LINKS, SKIP_PREFIX_CHECK, SORT_LISTS.
// E.g. cnListPopCntMax.
// Does it make sense for this file to be the only place where we use #define
// and #undef for conditional features and tuning parameters?

// NO_SKIP_LINKS means no skip links of any kind.
// SKIP_LINKS allows the type-specific SKIP_TO_<BLAH> to be defined.
// Default is -DSKIP_LINKS.
#ifndef   NO_SKIP_LINKS
  #undef     SKIP_LINKS
  #define    SKIP_LINKS
#endif // NO_SKIP_LINKS

// Default is BITMAP.
// BITMAP is required for one-digit and two-digit T_BITMAP bitmaps
// and for embedded bitmaps.
#ifdef   NO_BITMAP
  #ifdef BITMAP
    #error Cannot have BITMAP and NO_BITMAP
  #endif // BITMAP
  #ifdef SKIP_TO_BITMAP
    #error Cannot have SKIP_TO_BITMAP and NO_BITMAP.
  #endif // SKIP_TO_BITMAP
#else // NO_BITMAP
  #undef     BITMAP
  #define    BITMAP
  // Default is SKIP_TO_BITMAP.
  #ifndef NO_SKIP_TO_BITMAP
    #undef  SKIP_TO_BITMAP
    #define SKIP_TO_BITMAP
  #endif // #ifndef NO_SKIP_TO_BITMAP
  // Default is ALLOW_EMBEDDED_BITMAP for Judy1.
  #ifndef NO_ALLOW_EMBEDDED_BITMAP
    // What are the consequences of ALLOW_EMBEDDED_BITMAP
    // if (cnBitsInD1 > cnLogBitsPerLink)?
    // What about USE_XX_SW_ONLY_AT_DL2 which can yield (nBLR < cnBitsInD1)?
    #define ALLOW_EMBEDDED_BITMAP
  #endif // #ifndef NO_ALLOW_EMBEDDED_BITMAP
#endif // NO_BITMAP

// How should we handle the relationship between USE_XX_SW_ONLY_AT_DL2,
// ALLOW_EMBEDDED_BITMAP, cbEmbeddedBitmap, cnBitsInD1, and cnLogBitsPerLink?
// Would be nice to simplify.

#ifndef BITMAP
  #ifdef ALLOW_EMBEDDED_BITMAP
    #error ALLOW_EMBEDDED_BITMAP requires BITMAP
  #endif // ALLOW_EMBEDDED_BITMAP
#endif // BITMAP

// Default is PREFIX_WORD_IN_BITMAP_LEAF for B_JUDYL.
// The only reason it is not the default for Judy1 is that it would make our
// bitmap struct be eight words instead of six words. Maybe we should use
// the preamble word.
#ifdef SKIP_TO_BITMAP
  #ifdef B_JUDYL
    #undef  PREFIX_WORD_IN_BITMAP_LEAF
    #define PREFIX_WORD_IN_BITMAP_LEAF
  #endif // B_JUDYL
#endif // SKIP_TO_BITMAP

// No sense requesting extra memory that we don't have a use for by default.
#define LIST_REQ_MIN_WORDS

// XX_SW doesn't work without LVL_IN_WR_HB yet.
#ifdef USE_XX_SW
  #ifndef LVL_IN_WR_HB
    #error USE_XX_SW requires LVL_IN_WR_HB
  #endif // LVL_IN_WR_HB
#endif // USE_XX_SW

// Default is -DGOTO_AT_FIRST_IN_LOOKUP.
#ifndef   NO_GOTO_AT_FIRST_IN_LOOKUP
  #undef     GOTO_AT_FIRST_IN_LOOKUP
  #define    GOTO_AT_FIRST_IN_LOOKUP
#endif // NO_GOTO_AT_FIRST_IN_LOOKUP

// DEFAULT_AND_CASE means include the explicit case statement even for the
// default case defined by DEFAULT_<BLAH>.
// Default is -DDEFAULT_AND_CASE.
#ifndef   NO_DEFAULT_AND_CASE
  #undef     DEFAULT_AND_CASE
  #define    DEFAULT_AND_CASE
#endif // NO_DEFAULT_AND_CASE

// Default is -DALL_SKIP_TO_SW_CASES.
#ifdef SKIP_LINKS
  #ifndef   NO_ALL_SKIP_TO_SW_CASES
      #undef   ALL_SKIP_TO_SW_CASES
      #define  ALL_SKIP_TO_SW_CASES
  #endif // NO_ALL_SKIP_TO_SW_CASES
#endif // SKIP_LINKS

// Default is -DNDEBUG -UDEBUG_ALL -UDEBUG
// -UDEBUG_INSERT -UDEBUG_REMOVE -UDEBUG_LOOKUP -UDEBUG_MALLOC
// -UDEBUG_COUNT -UDEBUG_NEXT
#if defined(DEBUG_ALL)
//  #undef   DEBUG_LOOKUP
//  #define  DEBUG_LOOKUP
    #undef   DEBUG_INSERT
    #define  DEBUG_INSERT
    #undef   DEBUG_REMOVE
    #define  DEBUG_REMOVE
    #undef   DEBUG_MALLOC
    #define  DEBUG_MALLOC
    #undef   DEBUG_COUNT
    #define  DEBUG_COUNT
    #undef   DEBUG_NEXT
    #define  DEBUG_NEXT
    #undef   DEBUG
    #define  DEBUG
#else // defined(DEBUG_ALL)
  #if defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || defined(DEBUG_REMOVE)
    #undef   DEBUG
    #define  DEBUG
  #endif // defined(DEBUG_INSERT) || defined(DEBUG_LOOKUP) || ...
  #if defined(DEBUG_MALLOC) || defined(DEBUG_COUNT) || defined(DEBUG_NEXT)
    #undef   DEBUG
    #define  DEBUG
  #endif // defined(DEBUG_MALLOC) || defined(DEBUG_COUNT) || ...
#endif // defined(DEBUG_ALL)

#if ! defined(DEBUG)
    #undef  NDEBUG
    #define NDEBUG
#endif // ! defined(DEBUG)

#endif // ( ! defined(_BDEFINES_H_INCLUDED) )

