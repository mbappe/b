
#if ( ! defined(_BDEFINES_H_INCLUDED) )
#define _BDEFINES_H_INCLUDED

#ifndef    NO_NEW_NEXT
  #undef      NEW_NEXT
  #define     NEW_NEXT
#endif // !NO_NEW_NEXT

// If B_JUDYL is defined then we are building JudyL.
// Otherwise we are building Judy1.
// JudyL and Judy1 have different constraints and different defaults.
// We can build both Judy1 and JudyL with a single make, hence with
// a single set of command line -D options.
// We probably ought to enhance Makefile
// to have JUDY1_DEFINES and JUDYL_DEFINES.
// We don't build both 64-bit and 32-bit with a single make, but maybe
// we should since they can be combined into a single library on Mac.
// So maybe we should have DEFINES_32 and DEFINES_64.
// And maybe JUDY[1L]_DEFINES_(32|64).

#ifdef B_JUDYL
  #define BJL(_OnlyForJudyL)  _OnlyForJudyL
  #define BJ1(_OnlyForJudy1)
#else // B_JUDYL
  #define BJL(_OnlyForJudyL)
  #define BJ1(_OnlyForJudy1)  _OnlyForJudy1
#endif // #else B_JUDYL

#define cnBitsPerByte  8

// Default cnBitsPerWord is determined by __LP64__ and _WIN64.
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

// Default is cnBitsPerDigit = 8.
#ifndef cnBitsPerDigit
  #define cnBitsPerDigit  8
#endif // !cnBitsPerDigit

// Number of bits in the least significant digit of the key.
// Default cnBitsInD1 is cnBitsPerDigit. We count digits up from there.
#ifndef cnBitsInD1
  #define cnBitsInD1  cnBitsPerDigit
#endif // !cnBitsInD1

#define cnBitsLeftAtDl1  cnBitsInD1

// Bits in the second least significant digit of the key. Not bits left.
#ifndef cnBitsInD2
  #define cnBitsInD2 \
      (((cnBitsLeftAtDl1) + (cnBitsPerDigit) <= (cnBitsPerWord)) \
          ? (cnBitsPerDigit) : (cnBitsPerWord) - (cnBitsLeftAtDl1))
#endif // !cnBitsInD2

#define cnBitsLeftAtDl2  (cnBitsLeftAtDl1 + cnBitsInD2)

// Bits in the third least significant digit of the key. Not bits left.
#ifndef cnBitsInD3
  #define cnBitsInD3 \
      (((cnBitsLeftAtDl2) + (cnBitsPerDigit) <= (cnBitsPerWord)) \
          ? (cnBitsPerDigit) : (cnBitsPerWord) - (cnBitsLeftAtDl2))
#endif // !cnBitsInD3

#define cnBitsLeftAtDl3     (cnBitsLeftAtDl2 + cnBitsInD3)

// Default is -DCOMPRESSED_LISTS.
#ifndef NO_COMPRESSED_LISTS
  #undef  COMPRESSED_LISTS
  #define COMPRESSED_LISTS
#endif // #ifndef NO_COMPRESSED_LISTS

// Default is LVL_IN_WR_HB for 64-bit and level in nType for 32-bit.
// The absence of LVL_IN_WR_HB and LVL_IN_PP is level in nType.
#if (cnBitsPerWord > 32)
  #if       !defined(NO_LVL_IN_WR_HB) && !defined(LVL_IN_PP)
    #undef              LVL_IN_WR_HB
    #define             LVL_IN_WR_HB
  //                 NO_LVL_IN_PP is not necessary.
  #endif // !defined(NO_LVL_IN_WR_HB) && !defined(LVL_IN_PP)
#endif // (cnBitsPerWord > 32)

// NO_SKIP_LINKS means no skip links of any kind.
// SKIP_LINKS allows the type-specific SKIP_TO_<BLAH> to be defined.
// Default is -DSKIP_LINKS.
#ifndef   NO_SKIP_LINKS
  #undef     SKIP_LINKS
  #define    SKIP_LINKS
#endif // NO_SKIP_LINKS

#ifdef SKIP_LINKS
#if !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
  // For level in type, i.e. (!LVL_IN_WR_HB && !LVL_IN_PP),
  // multiple type values all represent T_SKIP_TO_SWITCH, i.e.
  // level = nType - T_SKIP_TO_SWITCH + 2.
  // Macro names that begin with '_' are derived from other macros
  // and are not inteded to be set explicitly on the build command line.
  #define _LVL_IN_TYPE
  #if !defined(DEFAULT_SKIP_TO_SW) && !defined(ALL_SKIP_TO_SW_CASES)
      #error Level in type requires DEFAULT_SKIP_TO_SW or ALL_SKIP_TO_SW_CASES.
  #endif // !DEFAULT_SKIP_TO_SW && !ALL_SKIP_TO_SW_CASES
#endif // !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
#endif // SKIP_LINKS

#ifdef _LVL_IN_TYPE
  #define NO_SKIP_TO_XX_SW
  #define NO_SKIP_TO_BM_SW
  #define NO_SKIP_TO_LIST_SW
  #define NO_SKIP_TO_BITMAP
  #define NO_SKIP_TO_LIST
#endif // _LVL_IN_TYPE

// Apply JudyL-specific constraints to ifdefs from the build process.
#ifdef B_JUDYL

  #undef  NO_UA_PARALLEL_128
  #define NO_UA_PARALLEL_128

  // Allow DEFINES=-D<blah> on shared make command line for JUDY1.
  #undef USE_XX_SW_ONLY_AT_DL2

  // Disabling PARALLEL_SEARCH_WORD helps with worst case memory usage.
  #ifndef PARALLEL_SEARCH_WORD
    #undef NO_PARALLEL_SEARCH_WORD
    #define NO_PARALLEL_SEARCH_WORD
  #endif // PARALLEL_SEARCH_WORD

  // Default is to use a packed value area for a list leaf.
  // If NO_PACK_L1_VALUES && (cnBitsInD1 <= 8) then don't
  // pack the L1 value area.
  #ifdef   NO_PACK_L1_VALUES
    #define NO_EMBED_KEYS
    #undef     EMBED_KEYS
  #else // NO_PACK_L1_VALUES
    #undef     PACK_L1_VALUES
    #define    PACK_L1_VALUES
  #endif // NO_PACK_L1_VALUES

  #ifndef   NO_PREFETCH_LOCATEKEY_PSPLIT_VAL
    #undef     PREFETCH_LOCATEKEY_PSPLIT_VAL
    #define    PREFETCH_LOCATEKEY_PSPLIT_VAL
  #endif // NO_PREFETCH_LOCATEKEY_PSPLIT_VAL

  #ifndef   NO_PREFETCH_LOCATEKEY_NEXT_VAL
    #undef     PREFETCH_LOCATEKEY_NEXT_VAL
    #define    PREFETCH_LOCATEKEY_NEXT_VAL
  #endif // NO_PREFETCH_LOCATEKEY_NEXT_VAL

  #ifndef   NO_PREFETCH_LOCATEKEY_PREV_VAL
    #undef     PREFETCH_LOCATEKEY_PREV_VAL
    #define    PREFETCH_LOCATEKEY_PREV_VAL
  #endif // NO_PREFETCH_LOCATEKEY_PREV_VAL

  #ifndef NO_CACHE_ALIGN_L1
    #undef   CACHE_ALIGN_L1
    #define  CACHE_ALIGN_L1
  #endif // #ifndef NO_CACHE_ALIGN_L1

  // PREFETCH_LOCATE_KEY_8_[BEG|END]_VAL are not necessary with
  // default cnListPopCntMaxDl1=4 and CACHE_ALIGN_L1.

  // Default is no PREFETCH_BM_[PSPLIT|NEXT|PREV]_VAL

#else // B_JUDYL

  #undef EK_XV

#endif // #else B_JUDYL

// Default is -DEMBED_KEYS.
#if ! defined(NO_EMBED_KEYS)
#undef  EMBED_KEYS
#define EMBED_KEYS
#endif // ! defined(NO_EMBED_KEYS)

// Default is EK_XV.
#ifdef EMBED_KEYS
#ifdef B_JUDYL
#ifndef SPLAY_WITH_INSERT // Need InsertEmbedded for EX_XV.
#ifndef NO_EK_XV
  #undef  EK_XV
  #define EK_XV
#endif // #ifndef NO_EK_XV
#endif // #ifndef SPLAY_WITH_INSERT
#endif // B_JUDYL
#endif // EMBED_KEYS

#if cnBitsPerWord < 64
  #undef EK_XV
#endif // cnBitsPerWord < 64

#ifndef COMPRESSED_LISTS
  #undef EK_XV
#endif // #ifndef COMPRESSED_LISTS

#ifdef EK_XV
  #ifndef EMBED_KEYS
    #error EK_XV without EMBED_KEYS
  #endif // EMBED_KEYS
  #undef  NO_EK_CALC_POP
  #define NO_EK_CALC_POP

  // Default is prefetch 1st cache line of value area for EK_XV.
  #ifndef NO_PF_EK_XV
    #undef  PF_EK_XV
    #define PF_EK_XV
  #endif // #ifndef NO_PF_EK_XV

  // Default is prefetch 2nd cache line of value area for EK_XV.
  #ifndef NO_PF_EK_XV_2
    #undef  PF_EK_XV_2
    #define PF_EK_XV_2
  #endif // #ifndef NO_PF_EK_XV_2
#endif // EK_XV

#ifdef AUGMENT_TYPE_8_PLUS_4
  #if cnBitsPerDigit == 8
  #if cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
    #pragma message("AUGMENT_TYPE_8_PLUS_4 is not needed.")
  #endif // cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
  #endif // cnBitsPerDigit == 8
  #undef  AUGMENT_TYPE_8
  #define AUGMENT_TYPE_8
#endif // AUGMENT_TYPE_8_PLUS_4

// Default is AUGMENT_TYPE_8.
// It seems to shine for Time -LmeB31.
#if cnBitsPerDigit == 8
#if cnBitsPerWord > 32
#ifndef USE_XX_SW
#ifndef DOUBLE_DOWN
#ifndef USE_LOWER_XX_SW
#ifndef USE_XX_SW_ONLY_AT_DL2
#ifndef XX_LISTS
#ifdef COMPRESSED_LISTS
  #if cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
  #ifndef    NO_AUGMENT_TYPE
    #undef      AUGMENT_TYPE_8
    #define     AUGMENT_TYPE_8
  #endif // !NO_AUGMENT_TYPE
  #endif // cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
  #ifndef AUGMENT_TYPE_8
  #ifndef NO_AUGMENT_TYPE
#pragma message("Warning: no default AUGMENT_TYPE_8 without all 8-bit digits.")
  #endif // !NO_AUGMENT_TYPE
  #endif // !AUGMENT_TYPE_8
#endif // COMPRESSED_LISTS
#endif // !XX_LISTS
#endif // !USE_XX_SW_ONLY_AT_DL2
#endif // !USE_LOWER_XX_SW
#endif // !DOUBLE_DOWN
#endif // !USE_XX_SW
#endif // cnBitsPerWord > 32
#endif // cnBitsPerDigit == 8

#ifdef AUG_TYPE_8_SW_NEXT
#ifndef AUGMENT_TYPE_8
  #error Cannot have AUG_TYPE_8_SW_NEXT without AUGMENT_TYPE_8.
#endif // !AUGMENT_TYPE_8
#endif // AUG_TYPE_8_SW_NEXT

#ifdef B_JUDYL
#ifndef    NO_SW_POP_IN_LNX
  #undef      SW_POP_IN_LNX
  #define     SW_POP_IN_LNX
#endif // !NO_SW_POP_IN_LNX
#endif // B_JUDYL

#ifdef SW_POP_IN_LNX
  #undef  _LNX
  #define _LNX
#endif // SW_POP_IN_LNX else

// _LNX will be defined automatically if needed by other ifdefs.
#ifdef B_JUDYL
#ifdef EMBED_KEYS
  #undef  _LNX // in case it's already been defined for another reason
  #define _LNX
#endif // EMBED_KEYS
#endif // B_JUDYL

#ifdef DOUBLE_DOWN
  #undef  XX_LISTS
  #define XX_LISTS
#endif // DOUBLE_DOWN

#ifdef XX_LISTS
  #undef  SKIP_TO_XX_SW
  #define SKIP_TO_XX_SW
#endif // XX_LISTS

// USE_LOWER_XX_SW turns on the use of narrow switches that decode the
// least significant bits of a digit.
// Default is -UUSE_LOWER_XX_SW
#ifdef USE_LOWER_XX_SW
  #undef  USE_XX_SW
  #define USE_XX_SW
  #undef  SKIP_TO_XX_SW
  #define SKIP_TO_XX_SW
#endif // USE_LOWER_XX_SW

// Default is -UCODE_XX_SW.

// Default is -UNO_TYPE_IN_XX_SW.
// NO_TYPE_IN_XX_SW means no type in XX_SW links with nBL < DL2.
// NO_TYPE_IN_XX_SW implies CODE_XX_SW.
// Should it imply USE_XX_SW?
// I don't know if NO_TYPE_IN_XX_SW applies only if USE_XX_SW_ONLY_AT_DL2.
#ifdef NO_TYPE_IN_XX_SW
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // NO_TYPE_IN_XX_SW

// SKIP_TO_XX_SW implies USE_XX_SW.
#ifdef SKIP_TO_XX_SW
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // SKIP_TO_XX_SW

// USE_XX_SW_ONLY_AT_DL2 started life as a flag to preserve legacy USE_XX_SW
// behavior of creating XX_SW only at DL2 and doubling it as necessary to
// have only embedded keys and avoid blow-outs into external lists or branches
// or bitmaps. Keep doubling all the way through 8-bit and 7-bit embedded keys
// until we end up with one big bitmap at DL2.
// The t_bitmap code in Lookup assumes there are no bitmaps at DL1.
// I don't know if/when embedded list overflow results in the creation of
// an external list. I don't know if an external list hanging from a DL2
// XX_SW can be splayed into another switch.
#ifdef    USE_XX_SW_ONLY_AT_DL2
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // USE_XX_SW_ONLY_AT_DL2

#ifdef    USE_XX_SW_ALWAYS
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // USE_XX_SW_ALWAYS

// Default is -UUSE_XX_SW.
// USE_XX_SW implies CODE_XX_SW.
// Time runs with USE_XX_SW take a long time because of insert times.
// USE_XX_SW doesn't work on JudyL or 32-bit yet.
#if defined(USE_XX_SW)
  #undef  CODE_XX_SW
  #define CODE_XX_SW
#endif // defined(USE_XX_SW)

// Define USE_BM_SW by default.
#ifndef   NO_USE_BM_SW
  #undef     USE_BM_SW
  #define    USE_BM_SW
#endif // NO_USE_BM_SW

// Default is -DPSPLIT_SEARCH_WORD -UPSPLIT_SEARCH_XOR_WORD.
// Default is -UNO_BINARY_SEARCH_WORD -UBACKWARD_SEARCH_WORD.
// PSPLIT_SEARCH_WORD does not apply at nBL == cnBitsPerWord.
// If -DNO_PSPLIT_SEARCH_WORD, then binary search for nBL != cnBitsPerWord
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
// E.g. SKIP_LINKS, SKIP_PREFIX_CHECK.
// E.g. cnListPopCntMax.
// Does it make sense for this file to be the only place where we use #define
// and #undef for conditional features and tuning parameters?

#ifdef NO_SKIP_LINKS
  #undef  NO_SKIP_TO_BITMAP
  #define NO_SKIP_TO_BITMAP
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
  // Default is SKIP_TO_BITMAP for Judy1.
  #ifndef B_JUDYL
  #ifndef NO_SKIP_TO_BITMAP
    #undef  SKIP_TO_BITMAP
    #define SKIP_TO_BITMAP
  #endif // #ifndef NO_SKIP_TO_BITMAP
  #endif // #ifndef B_JUDYL
  // Default is ALLOW_EMBEDDED_BITMAP.
  #ifndef NO_ALLOW_EMBEDDED_BITMAP
// Not sure why we are placing this condition on ALLOW_EMBEDDED_BITMAP.
      #if (cnBitsPerDigit * 2 > cnLogBitsPerWord)
    // What are the consequences of ALLOW_EMBEDDED_BITMAP
    // if (cnBitsInD1 > cnLogBitsPerLink)?
    // What about USE_XX_SW_ONLY_AT_DL2 which can yield (nBLR < cnBitsInD1)?
    #define ALLOW_EMBEDDED_BITMAP
      #endif // (cnBitsPerDigit * 2 > cnLogBitsPerWord)
  #endif // #ifndef NO_ALLOW_EMBEDDED_BITMAP
#endif // else NO_BITMAP

#ifdef BITMAP
#ifdef B_JUDYL
  #ifndef cnListPopCntMaxDl1
    #define cnListPopCntMaxDl1  4
  #endif // #ifndef cnListPopCntMaxDl1
  // PACK_BM_VALUES means use a packed value area for a bitmap leaf if/when
  // the values area is less than max and/or when UNPACK_BM_VALUES is not
  // defined.
  // Default is PACK_BM_VALUES.
  #ifndef NO_PACK_BM_VALUES
    #undef   PACK_BM_VALUES
    #define  PACK_BM_VALUES
  #endif // #ifndef NO_PACK_BM_VALUES
  #ifdef PACK_BM_VALUES
    // Default is BMLF_CNTS only for default digit size of eight.
    // It hasn't been tested with anything else.
    #if (cnBitsPerWord > 32) // Why?
    #if cnBitsPerDigit == 8
    #if cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
    #ifndef NO_BMLF_CNTS
      #undef  BMLF_CNTS
      #define BMLF_CNTS
    #endif // !NO_BMLF_CNTS
    #endif // cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
    #endif // cnBitsPerDigit == 8
    #endif // (cnBitsPerWord > 32)
    #ifdef BMLF_CNTS
      // Default is BMLF_CNTS_CUM for BMLF_CNTS unless NO_BMLF_CNTS_CUM.
      // BMLF_CNTS_CUM is ignored if BMLF_POP_COUNT_[1|8].
      // BMLF_POP_COUNT_[1|8] always use cumulative cnts.
      #ifndef NO_BMLF_CNTS_CUM
        #undef  BMLF_CNTS_CUM
        #define BMLF_CNTS_CUM
      #endif // !NO_BMLF_CNTS_CUM
      #ifndef NO_BMLF_CNTS_IN_LNX
        #undef  BMLF_CNTS_IN_LNX
        #define BMLF_CNTS_IN_LNX
      #endif // !NO_BMLF_CNTS_IN_LNX
    #else // BMLF_CNTS
      #ifndef NO_BMLF_CNTS
        #pragma message("Warning: not defining BMLF_CNTS")
      #endif // #ifndef NO_BMLF_CNTS
      #undef BMLF_CNTS_CUM
      #undef BMLF_CNTS_IN_LNX
    #endif // BMLF_CNTS else
    #ifndef NO_PF_BM_PREV_HALF_VAL
      #undef   PF_BM_PREV_HALF_VAL
      #define  PF_BM_PREV_HALF_VAL
    #endif // #ifndef NO_PF_BM_PREV_HALF_VAL
    #ifndef NO_PF_BM_NEXT_HALF_VAL
      #undef   PF_BM_NEXT_HALF_VAL
      #define  PF_BM_NEXT_HALF_VAL
    #endif // #ifndef NO_PF_BM_NEXT_HALF_VAL
  #endif // PACK_BM_VALUES

  // UNPACK_BM_VALUES means use an unpacked value area for a bitmap leaf
  // if/when the value area is max and/or when PACK_BM_VALUES is not defined.
  // Default is UNPACK_BM_VALUES.
  #ifndef   NO_UNPACK_BM_VALUES
    #undef     UNPACK_BM_VALUES
    #define    UNPACK_BM_VALUES
  #endif // NO_UNPACK_BM_VALUES

  #ifndef PACK_BM_VALUES
  #ifndef UNPACK_BM_VALUES
    #error Must have at least one of PACK_BM_VALUES and UNPACK_BM_VALUES.
  #endif // #ifndef UNPACK_BM_VALUES
  #endif // #ifndef PACK_BM_VALUES

  #ifdef BMLF_CNTS_IN_LNX
    #undef  _LNX
    #define _LNX
  #endif // BMLF_CNTS_IN_LNX
#else // B_JUDYL
    #undef BMLF_CNTS
    #undef BMLF_CNTS_CUM
    #ifdef ALLOW_EMBEDDED_BITMAP
        // below doesn't catch problems for Link_t bigger than one word
        #if (cnBitsInD1 <= cnLogBitsPerWord)
            #define _D1BmFitsInLink
        #elif (cnBitsPerDigit <= cnLogBitsPerWord)
            #define _D1BmFitsInLink
        #endif // cnBitsInD1 <= ... elif cnBitsPerDigit <= ... else
        #ifdef _D1BmFitsInLink
            #if cn2dBmMaxWpkPercent
                // 2-digit bitmap code doesn't work with embedded bitmap
                #error cn2dBmMaxWpkPercent and embedded bitmap
            #else // cn2dBmMaxWpkPercent
                #undef  cn2dBmMaxWpkPercent // in case defined as 0
                #define cn2dBmMaxWpkPercent  0
            #endif // #else cn2dBmMaxWpkPercent
        #endif // _D1BmFitsInLink
    #endif // ALLOW_EMBEDDED_BITMAP
#endif // #else B_JUDYL
#endif // BITMAP

// How should we handle the relationship between USE_XX_SW_ONLY_AT_DL2,
// ALLOW_EMBEDDED_BITMAP, cbEmbeddedBitmap, cnBitsInD1, and cnLogBitsPerLink?
// Would be nice to simplify.
#ifndef BITMAP
  #ifdef ALLOW_EMBEDDED_BITMAP
    #error ALLOW_EMBEDDED_BITMAP requires BITMAP
  #endif // ALLOW_EMBEDDED_BITMAP
#endif // #ifndef BITMAP

// Default is PREFIX_WORD_IN_BITMAP_LEAF for B_JUDYL.
// We have to be careful for Judy1 because we don't want to bump our bitmap
// struct from six to eight words.
#if       !defined(NO_PREFIX_WORD_IN_BITMAP_LEAF) && defined(B_JUDYL)
  #undef              PREFIX_WORD_IN_BITMAP_LEAF
  #define             PREFIX_WORD_IN_BITMAP_LEAF
#endif // !defined(NO_PREFIX_WORD_IN_BITMAP_LEAF) && defined(B_JUDYL)

// No sense requesting extra memory that we don't have a use for by default.
#define LIST_REQ_MIN_WORDS

// XX_SW doesn't work without LVL_IN_WR_HB yet.
#ifdef SKIP_TO_XX_SW
  #if !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
    #error SKIP_TO_XX_SW requires LVL_IN_WR_HB or LVL_IN_PP
  #endif // !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
#endif // SKIP_TO_XX_SW

// Default is -DGOTO_AT_FIRST_IN_LOOKUP.
#ifndef   NO_GOTO_AT_FIRST_IN_LOOKUP
  #undef     GOTO_AT_FIRST_IN_LOOKUP
  #define    GOTO_AT_FIRST_IN_LOOKUP
#endif // NO_GOTO_AT_FIRST_IN_LOOKUP

// At most one of DEFAULT_SKIP_TO_SW, DEFAULT_SWITCH,
// DEFAULT_LIST and DEFAULT_BITMAP may be defined.
#ifdef DEFAULT_SKIP_TO_SW
  #ifdef DEFAULT_SWITCH
    #error DEFAULT_SWITCH with DEFAULT_SKIP_TO_SW
  #endif // DEFAULT_SWITCH
  #ifdef DEFAULT_BITMAP
    #error DEFAULT_BITMAP with DEFAULT_SKIP_TO_SW
  #endif // DEFAULT_BITMAP
  #ifdef DEFAULT_LIST
    #error DEFAULT_LIST with DEFAULT_SKIP_TO_SW
  #endif // DEFAULT_LIST
#elif defined(DEFAULT_SWITCH)
  #ifdef DEFAULT_BITMAP
    #error DEFAULT_BITMAP with DEFAULT_SWITCH
  #endif // DEFAULT_BITMAP
  #ifdef DEFAULT_LIST
    #error DEFAULT_LIST with DEFAULT_SWITCH
  #endif // DEFAULT_LIST
#elif defined(DEFAULT_BITMAP)
  #ifdef DEFAULT_LIST
    #error DEFAULT_LIST with DEFAULT_BITMAP
  #endif // DEFAULT_LIST
#endif // defined(DEFAULT_BITMAP)

// DEFAULT_AND_CASE means include the explicit case statement even for the
// default case defined by DEFAULT_<BLAH>.
// Default is -DDEFAULT_AND_CASE.
#ifndef   NO_DEFAULT_AND_CASE
  #undef     DEFAULT_AND_CASE
  #define    DEFAULT_AND_CASE
#endif // NO_DEFAULT_AND_CASE

// Default is no -DALL_SKIP_TO_SW_CASES.

// Default is -DNDEBUG -UDEBUG_ALL -UDEBUG
// -UDEBUG_INSERT -UDEBUG_REMOVE -UDEBUG_LOOKUP -UDEBUG_MALLOC
// -UDEBUG_COUNT -UDEBUG_NEXT
#ifdef DEBUG_ALL
    #undef   DEBUG_ASSERT
    #define  DEBUG_ASSERT
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
#elif !defined(DEBUG) // DEBUG_ALL
  #if defined(DEBUG_COUNT) || defined(DEBUG_NEXT) || defined(DEBUG_ASSERT)
    #define  DEBUG
  #elif defined(DEBUG_INSERT) || defined(DEBUG_REMOVE)
    #define  DEBUG
  #elif defined(DEBUG_LOOKUP) || defined(DEBUG_MALLOC)
    #define  DEBUG
  #else // COUNT||NEXT||ASSERT||INSERT||REMOVE||LOOKUP||MALLOC
    #undef  NDEBUG
    #define NDEBUG // turn off assertions for performance
  #endif // COUNT||NEXT||ASSERT||INSERT||REMOVE||LOOKUP||MALLOC else
#endif // DEBUG_ALL elif !DEBUG else

#ifndef PP_IN_LINK
  // Default is POP_WORD ifndef PP_IN_LINK unless NO_POP_WORD.
  // POP_WORD makes gwPopCnt faster.
  // gwPopCnt showed up as a heavy hitter in the profile.
  #ifndef   NO_POP_WORD
    #undef     POP_WORD
    #define    POP_WORD
  #endif // NO_POP_WORD
#endif // #ifndef PP_IN_LINK

#ifdef FAST_MALLOC_2
  #undef  FAST_MALLOC_1
  #define FAST_MALLOC_1
#endif // FAST_MALLOC_2

#ifdef FAST_MALLOC_1
  #undef  FAST_MALLOC
  #define FAST_MALLOC
#endif // FAST_MALLOC_1

// Default is -DPSPLIT_SEARCH_8
#if ! defined(NO_PSPLIT_SEARCH_8)
#undef  PSPLIT_SEARCH_8
#define PSPLIT_SEARCH_8
#endif // ! defined(NO_PSPLIT_SEARCH_8)

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

#ifdef PSPLIT_SEARCH_8
#ifdef PSPLIT_PARALLEL
#ifndef NO_PARALLEL_LOCATEKEY_8
  #undef  PARALLEL_LOCATEKEY_8
  #define PARALLEL_LOCATEKEY_8
#endif // #ifndef NO_PARALLEL_LOCATEKEY_8
#endif // PSPLIT_PARALLEL
#endif // PSPLIT_SEARCH_8

// Default is -DSEARCH_FROM_WRAPPER.
#ifndef           NO_SEARCH_FROM_WRAPPER
  #undef             SEARCH_FROM_WRAPPER
  #define            SEARCH_FROM_WRAPPER
#endif // #ifndef NO_SEARCH_FROM_WRAPPER

#ifndef NO_BM_POP_IN_WR_HB
  #undef  BM_POP_IN_WR_HB
  #define BM_POP_IN_WR_HB
#endif // #ifndef NO_BM_POP_IN_WR_HB

#ifndef NO_LKIL8_ONE_BUCKET
    #define LKIL8_ONE_BUCKET
#endif // #ifndef NO_LKIL8_ONE_BUCKET

// BM_SW_CNT_IN_WR (bmsw link count) is default only for default digit size
// of eight.  assert(cnBitsCnt >= nBW)
#ifdef USE_BM_SW
#ifdef B_JUDYL
#if cnBitsPerWord > 32
  #if cnBitsPerDigit == 8
  #if cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
    #ifndef NO_BM_SW_CNT_IN_WR
        #undef  BM_SW_CNT_IN_WR
        #define BM_SW_CNT_IN_WR
    #endif // #ifndef NO_BM_SW_CNT_IN_WR
  #endif // cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
  #endif // cnBitsPerDigit == 8
  #ifndef BM_SW_CNT_IN_WR
  #ifndef NO_BM_SW_CNT_IN_WR
    #pragma message("Warning: not defining BM_SW_CNT_IN_WR")
  #endif // #ifndef NO_BM_SW_CNT_IN_WR
  #endif // #ifndef BM_SW_CNT_IN_WR
#endif // cnBitsPerWord > 32
#endif // B_JUDYL
#endif // USE_BM_SW

// Default is OFFSET_IN_SW_BM_WORD for B_JUDYL unless NO_OFFSET_IN_SW_BM_WORD.
// OFFSET_IN_SW_BM_WORD is not used for Judy1.
#undef              OFFSET_IN_SW_BM_WORD
#ifdef B_JUDYL
#ifndef          NO_OFFSET_IN_SW_BM_WORD
  #define           OFFSET_IN_SW_BM_WORD
#endif // ifndef NO_OFFSET_IN_SW_BM_WORD
#endif // B_JUDYL

#ifndef   NO_PREFETCH_BM_LN
    #undef   PREFETCH_BM_LN
    #define  PREFETCH_BM_LN
#endif // NO_PREFETCH_BM_LN

// Default is REMOTE_LNX if _LNX.
// Ultimately, I think upper level switches will have no or a remote
// link and leaf level switches with have a two-word link.
#ifdef _LNX
  #ifndef NO_REMOTE_LNX
    #undef   REMOTE_LNX
    #define  REMOTE_LNX
  #endif // ifndef NO_REMOTE_LNX
#else // _LNX
  #undef    REMOTE_LNX
#endif // else _LNX

// _RETURN_NULL_TO_INSERT_AGAIN was created to deal with the issue of
// InsertGuts calling back into Insert for the final insert but Insert not
// having a pwLnX parameter and not being able to easily figure it out
// for the pLn for which it is called.
// Instead of calling back into Insert we simply return NULL to cause
// Insert to loop back to the top with the pwLnX that it already knows.
#ifdef REMOTE_LNX
  #undef  _RETURN_NULL_TO_INSERT_AGAIN
  #define _RETURN_NULL_TO_INSERT_AGAIN
#endif // REMOTE_LNX

// gcc does a crappy job with 64-bit vectors.
#ifndef NO_OLD_HK_64
  #undef   OLD_HK_64
  #define  OLD_HK_64
#endif // #ifndef NO_OLD_HK_64

#ifdef AUGMENT_TYPE_8
  #undef  AUGMENT_TYPE
  #define AUGMENT_TYPE
#endif // AUGMENT_TYPE_8
#ifdef AUGMENT_TYPE_NOT
  #undef  AUGMENT_TYPE
  #define AUGMENT_TYPE
#endif // AUGMENT_TYPE_NOT

// Fix NUM_TYPES on command line based on other ifdefs if
// ALL_SKIP_TO_SW_CASES && AUGMENT_TYPE and the default 9 is not correct.
// It is used to avoid defining duplicate cases in the main switch statement
// for LOOKUP.
#ifdef ALL_SKIP_TO_SW_CASES
#ifdef AUGMENT_TYPE
#ifndef NUM_TYPES
  #define NUM_TYPES  9
#endif // !NUM_TYPES
#endif // AUGMENT_TYPE
#endif // ALL_SKIP_TO_SW_CASES

#ifdef B_JUDYL
  #ifdef BMLFI_LNX
    #undef  BMLF_INTERLEAVE
    #define BMLF_INTERLEAVE
  #endif // BMLFI_LNX
  #ifdef BMLFI_BM_HB
    #undef  BMLF_INTERLEAVE
    #define BMLF_INTERLEAVE
  #endif // BMLFI_BM_HB
  #ifdef BMLFI_VARIABLE_SZ
    #undef  BMLF_INTERLEAVE
    #define BMLF_INTERLEAVE
  #endif // BMLFI_VARIABLE_SZ
  #ifdef BMLF_INTERLEAVE
    #ifndef UNPACK_BM_VALUES
      #error BMLF_INTERLEAVE without UNPACK_BM_VALUES
    #endif // !UNPACK_BM_VALUES
    #ifndef cnLogBmlfParts
      #define cnLogBmlfParts  6
    #endif // cnLogBmlfParts
  #endif // BMLF_INTERLEAVE
#else // B_JUDYL
  #undef BMLF_INTERLEAVE
  #undef BMLFI_LNX
  #undef BMLFI_BM_HB
  #undef BMLFI_VARIABLE_SZ
  #undef cnLogBmlfParts
#endif // B_JUDYL

#ifdef MASK_NBLR
  #undef  BL_SPECIFIC_SKIP
  #define BL_SPECIFIC_SKIP
#endif // MASK_NBLR
#ifdef BL_SPECIFIC_SKIP_JT
  #undef  BL_SPECIFIC_SKIP
  #define BL_SPECIFIC_SKIP
#endif // BL_SPECIFIC_SKIP_JT

#ifdef AUGMENT_TYPE_8_PLUS_4
  #if cnBitsLeftAtDl3 < 24
    // AUGMENT_TYPE_8_PLUS_4 with cnBitsLeftAtDl3 < 24 yields 9 digits
    // and makes AugTypeBitsInv(112) ambiguous. Hence some code slower.
    #pragma message "AUGMENT_TYPE_8_PLUS_4 with cnBitsLeftAtDl3 < 24."
  #endif // cnBitsLeftAtDl3 < 24
#endif // AUGMENT_TYPE_8_PLUS_4

#ifdef REMOTE_LNX
  #undef DUMMY_REMOTE_LNX
#endif // REMOTE_LNX

#ifndef   NO_REVERSE_SORT_EMBEDDED_KEYS
  #undef     REVERSE_SORT_EMBEDDED_KEYS
  #define    REVERSE_SORT_EMBEDDED_KEYS
#endif // NO_REVERSE_SORT_EMBEDDED_KEYS

#ifdef B_JUDYL
  #undef  REVERSE_SORT_EMBEDDED_KEYS
#endif // B_JUDYL

#ifdef REVERSE_SORT_EMBEDDED_KEYS
  #undef  FILL_W_BIG_KEY
  #define FILL_W_BIG_KEY
  #undef  NO_EK_CALC_POP
  #define NO_EK_CALC_POP
#endif // REVERSE_SORT_EMBEDDED_KEYS

#ifndef B_JUDYL
  #undef    PACK_BM_VALUES
  #undef NO_PACK_BM_VALUES
  #undef    UNPACK_BM_VALUES
  #undef NO_UNPACK_BM_VALUES
#endif // !B_JUDYL

#ifdef DSMETRICS_HITS
  #ifdef DSMETRICS_NHITS
    #error DSMETRICS_HITS and DSMETRICS_NHITS are mutually exclusive.
  #endif // DSMETRICS_NHITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_HITS

#ifdef DSMETRICS_NHITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_NHITS

#ifdef DSMETRICS_GETS
  #undef  SEARCHMETRICS
  #define SEARCHMETRICS
#endif // DSMETRICS_GETS

#ifdef SEARCHMETRICS
  #ifndef NO_SMETRICS_HITS
    #undef  SMETRICS_HITS
    #define SMETRICS_HITS
  #endif // !NO_SMETRICS_HITS
  // Default is no SMETRICS_SEARCH_POP
  // Default is no SMETRICS_MISCOMPARES
  // Default is no SMETRICS_EK
  // Default is no SMETRICS_UNPACKED_BM
#endif // SEARCHMETRICS

#endif // ( ! defined(_BDEFINES_H_INCLUDED) )

