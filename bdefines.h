
#if ( ! defined(_BDEFINES_H_INCLUDED) )
#define _BDEFINES_H_INCLUDED

// Add one word in Switch_t for subexpanse pop count(s) to improve performance
// of Count by default.
#ifndef cnSwCnts
  #define cnSwCnts  16
#endif // cnSwCnts
#if cnSwCnts == 1
  #define cnLogSwCnts  0
#elif cnSwCnts == 2
  #define cnLogSwCnts  1
#elif cnSwCnts == 4
  #define cnLogSwCnts  2
#elif cnSwCnts == 8
  #define cnLogSwCnts  3
#elif cnSwCnts == 16
  #define cnLogSwCnts  4
#elif cnSwCnts == 32
  #define cnLogSwCnts  5
#elif cnSwCnts == 64
  #define cnLogSwCnts  6
#elif cnSwCnts == 0
  #undef cnSwCnts
  #undef cnLogSwCnts
#else
  #error cnSwCnts must be a power of 2
#endif

#undef LVL_IN_PP // getting rid of this

// BM_SW_BM_IN_WR_OR_LNX, as is, requires us to give up the following:
// - SW_POP_IN_LNX for JudyL
//   - consider separating BM_SW_POP_IN_LNX from SWITCH_POP_IN_LNX
// - OFFSET_IN_SW_BM_WORD (don't care much) for JudyL
// - SKIP_TO_BM_SW for Judy1
//   - consider NO_SKIP_TO_BM_SW_LVL_IN_WR
//   - consider SKIP_TO_BM_SW_LVL_IN_TYPE
// - BM_SW_CNT_IN_WR (link count - don't care) for Judy1

#ifdef USE_BM_SW_BM_IN_WR_OR_LNX
  #ifdef B_JUDYL
    #undef  USE_BM_SW_BM_IN_LNX
    #define USE_BM_SW_BM_IN_LNX
  #else // B_JUDYL
    #undef  USE_BM_SW_BM_IN_WR_HB
    #define USE_BM_SW_BM_IN_WR_HB
  #endif // B_JUDYL
#endif // USE_BM_SW_BM_IN_WR_OR_LNX

#ifdef    USE_BM_SW_BM_IN_LNX
  #undef      BM_SW_BM_IN_LNX
  #define     BM_SW_BM_IN_LNX
#endif // USE_BM_SW_BM_IN_LNX

#ifdef    USE_BM_SW_BM_IN_WR_HB
  #undef      BM_SW_BM_IN_WR_HB
  #define     BM_SW_BM_IN_WR_HB
#endif // USE_BM_SW_BM_IN_WR_HB

// Not sure of NO_BM_SW_AT_TOP is the way to go for BM_SW_BM_IN_LNX.
// Maybe just test nBL and leave/use the bitmap in the switch.
#ifdef BM_SW_BM_IN_LNX
  #undef  NO_BM_SW_AT_TOP
  #define NO_BM_SW_AT_TOP
#endif // BM_SW_BM_IN_LNX

// Enable SW_POP_IN_WR_HB by default.
// Enable SW_POP_IN_WR_HB by default.
#if cnBitsPerWord > 32
#ifndef    NO_SW_POP_IN_WR_HB
#undef        SW_POP_IN_WR_HB
#define       SW_POP_IN_WR_HB
#endif // !NO_SW_POP_IN_WR_HB
#endif // cnBitsPerWord > 32

// Enable GPC_ALL_SKIP_TO_SW_CASES by default.
#ifndef    NO_GPC_ALL_SKIP_TO_SW_CASES
#undef        GPC_ALL_SKIP_TO_SW_CASES
#define       GPC_ALL_SKIP_TO_SW_CASES
#endif // !NO_GPC_ALL_SKIP_TO_SW_CASES

#ifdef NEW_NEXT_IS_EXCLUSIVE
  #undef  NEW_NEXT
  #define NEW_NEXT
#endif // NEW_NEXT_IS_EXCLUSIVE

#ifndef    NO_NEW_NEXT
  #undef      NEW_NEXT
  #define     NEW_NEXT
#endif // !NO_NEW_NEXT

#ifndef    NO_FULL_SW
  #undef      FULL_SW
  #define     FULL_SW
#endif // !NO_FULL_SW

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

#if cnBitsPerDigit == 8
#if cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
  #define _ALL_DIGITS_ARE_8_BITS
#endif // cnBitsInD1 == 8 && cnBitsInD2 == 8 && cnBitsInD3 == 8
#endif // cnBitsPerDigit == 8

// Default is -DCOMPRESSED_LISTS.
#ifndef    NO_COMPRESSED_LISTS
  #undef      COMPRESSED_LISTS
  #define     COMPRESSED_LISTS
#endif // !NO_COMPRESSED_LISTS

#ifndef   NO_COUNT_2_PREFIX
  #undef     COUNT_2_PREFIX
  #define    COUNT_2_PREFIX
#endif // NO_COUNT_2_PREFIX

#ifdef COUNT_2_PREFIX
  #undef  COUNT_2
  #define COUNT_2
#endif // COUNT_2_PREFIX

// Default is -DPARALLEL_128.
// Determines type hence size of Bucket_t.
#ifndef PARALLEL_256
#if !defined(PARALLEL_64) && !defined(NO_PARALLEL_128)
  #undef  PARALLEL_128
  #define PARALLEL_128
#endif // !PARALLEL_64 && !NO_PARALLEL_128
#endif // !PARALLEL_256

// Default is LVL_IN_WR_HB for 64-bit and level in nType for 32-bit.
// The absence of LVL_IN_WR_HB and LVL_IN_PP is level in nType.
#if (cnBitsPerWord > 32)
#ifndef    NO_LVL_IN_WR_HB
  #undef      LVL_IN_WR_HB
  #define     LVL_IN_WR_HB
#endif // !NO_LVL_IN_WR_HB
#endif // (cnBitsPerWord > 32)

#ifdef BM_SW_BM_IN_WR_HB
#ifdef LVL_IN_WR_HB
  #undef  NO_SKIP_TO_BM_SW
  #define NO_SKIP_TO_BM_SW
#endif // LVL_IN_WR_HB
#endif // BM_SW_BM_IN_WR_HB

// NO_SKIP_LINKS means no skip links of any kind.
// SKIP_LINKS allows the type-specific SKIP_TO_<BLAH> to be defined.
// Default is -DSKIP_LINKS.
#ifndef   NO_SKIP_LINKS
  #undef     SKIP_LINKS
  #define    SKIP_LINKS
#endif // NO_SKIP_LINKS

#ifdef SKIP_LINKS
#ifndef LVL_IN_WR_HB
  // For level in type, i.e. !LVL_IN_WR_HB,
  // multiple type values all represent T_SKIP_TO_SWITCH, i.e.
  // level = nType - T_SKIP_TO_SWITCH + 2.
  // Macro names that begin with '_' are derived from other macros
  // and are not inteded to be set explicitly on the build command line.
  #define _LVL_IN_TYPE
  #ifndef ALL_SKIP_TO_SW_CASES
    #undef  DEFAULT_SKIP_TO_SW
    #define DEFAULT_SKIP_TO_SW
  #endif // !ALL_SKIP_TO_SW_CASES
  #if !defined(DEFAULT_SKIP_TO_SW) && !defined(ALL_SKIP_TO_SW_CASES)
      #error Level in type requires DEFAULT_SKIP_TO_SW or ALL_SKIP_TO_SW_CASES.
  #endif // !DEFAULT_SKIP_TO_SW && !ALL_SKIP_TO_SW_CASES
#endif // !LVL_IN_WR_HB
#endif // SKIP_LINKS

#ifdef _LVL_IN_TYPE
  #define NO_SKIP_TO_XX_SW
  #undef  NO_SKIP_TO_BM_SW
  #define NO_SKIP_TO_BM_SW
  #define NO_SKIP_TO_LIST_SW
  #define NO_SKIP_TO_BITMAP
  #define NO_SKIP_TO_LIST
#endif // _LVL_IN_TYPE

#ifdef RIGID_XX_SW
  #ifndef _ALL_DIGITS_ARE_8_BITS
    #error
  #endif // _ALL_DIGITS_ARE_8_BITS
  #ifdef USE_XX_SW_ONLY_AT_DL2
    #error
  #endif // USE_XX_SW_ONLY_AT_DL2
  #undef  USE_XX_SW
  #define USE_XX_SW
#endif // RIGID_XX_SW

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

  #ifndef NO_PF_BM_SUBEX_PSPLIT
  #ifndef NO_HYPERTUNE_PF_BM
    #undef  HYPERTUNE_PF_BM
    #define HYPERTUNE_PF_BM
  #endif // NO_HYPERTUNE_PF_BM
  #ifdef HYPERTUNE_PF_BM
      #undef  PF_BM_SUBEX_PSPLIT
      #define PF_BM_SUBEX_PSPLIT
  #endif // HYPERTUNE_PF_BM
  #endif // !NO_PF_BM_SUBEX_PSPLIT

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

#ifdef DOUBLE_DOWN
  // Does XX_LISTS without DOUBLE_DOWN make sense?
  // I don't think we ever get a shared list unless DOUBLE_DOWN.
  // But USE_XX_SW can create XX_SW even without DOUBLE_DOWN.
  #undef  XX_LISTS
  #define XX_LISTS
#endif // DOUBLE_DOWN

#ifdef AUG_TYPE_64_LOOKUP
  #define _AUG_TYPE_X_LOOKUP
#elif defined(AUG_TYPE_32_LOOKUP)
  #define _AUG_TYPE_X_LOOKUP
#elif defined(AUG_TYPE_16_LOOKUP)
  #define _AUG_TYPE_X_LOOKUP
#elif defined(AUG_TYPE_8_LOOKUP)
  #define _AUG_TYPE_X_LOOKUP
#endif // AUG_TYPE_64_LOOKUP elif ...  AUG_TYPE_16_LOOKUP else

#ifdef _AUG_TYPE_X_LOOKUP
  #undef  AUGMENT_TYPE
  #define AUGMENT_TYPE
#endif // _AUG_TYPE_X_LOOKUP

#ifdef AUGMENT_TYPE_8_PLUS_4
  #ifdef _ALL_DIGITS_ARE_8_BITS
    #pragma message("AUGMENT_TYPE_8_PLUS_4 is not needed.")
  #endif // _ALL_DIGITS_ARE_8_BITS
  #undef  AUGMENT_TYPE_8
  #define AUGMENT_TYPE_8 // only until we have _AUG_TYPE_8
#endif // AUGMENT_TYPE_8_PLUS_4

#ifdef AUG_TYPE_8_SW_NEXT
  #undef  AUGMENT_TYPE_8
  #define AUGMENT_TYPE_8 // only until we have _AUG_TYPE_8
#endif // AUG_TYPE_8_SW_NEXT

#ifndef B_JUDYL
  #undef AUG_TYPE_8_NEXT_EK_XV
#endif // B_JUDYL

#ifdef AUG_TYPE_8_NEXT_EK_XV
  #ifdef AUGMENT_TYPE_8_PLUS_4
    #error
  #endif // AUGMENT_TYPE_8_PLUS_4
  #undef  AUGMENT_TYPE_8
  #define AUGMENT_TYPE_8 // only until we have _AUG_TYPE_8
#endif // AUG_TYPE_8_NEXT_EK_XV

// Default is no AUGMENT_TYPE.
// Unequivocal performance tests with AUGMENT_TYPE_8 have been elusive.
// AUGMENT_TYPE_8 doesn't seem to cost too much but I'm also not sure it
// helps much with _ALL_DIGITS_ARE_8_BITS.
// I have seen some small differences, mostly at high populations, show up with
// -e and other splay masks that cause a deep tree.
// Maybe when gnBW is more costly we will see bigger differences.
// Not sure about JudyL vs Judy1.
#ifdef AUGMENT_TYPE_8
  #if cnBitsPerDigit != 8 || cnBitsPerWord <= 32
    #error
  #elif defined(USE_XX_SW) || defined(USE_XX_SW_ONLY_AT_DL2)
    #error
  #elif defined(DOUBLE_DOWN) || defined(USE_LOWER_XX_SW)
    #error
  #elif defined(XX_LISTS) || !defined(COMPRESSED_LISTS)
    #error
  #endif // cnBitsPerDigit != 8 || cnBitsPerWord <= 32 elif ... else
  #ifndef AUGMENT_TYPE_8_PLUS_4
    #if cnBitsInD1 != 8 || cnBitsInD2 != 8 || cnBitsInD3 != 8
      #error
    #endif // cnBitsInD1 != 8 || cnBitsInD2 != 8 || cnBitsInD3 != 8
  #endif // !AUGMENT_TYPE_8_PLUS_4
#endif // AUGMENT_TYPE_8

#ifdef AUGMENT_TYPE_8
  #undef  AUGMENT_TYPE
  #define AUGMENT_TYPE
#endif // AUGMENT_TYPE_8

#ifdef AUGMENT_TYPE_NOT
  #undef  AUGMENT_TYPE
  #define AUGMENT_TYPE
#endif // AUGMENT_TYPE_NOT

#ifdef AUG_TYPE_8_NEXT_EMPTY
#ifdef AUGMENT_TYPE_8
#ifdef NEXT_EMPTY
  #define _AUG_TYPE_8_NEXT_EMPTY
#endif // NEXT_EMPTY
#endif // AUGMENT_TYPE_8
#endif // AUG_TYPE_8_NEXT_EMPTY

#ifdef B_JUDYL
#ifndef    NO_SW_POP_IN_LNX
  #undef      SW_POP_IN_LNX
  #define     SW_POP_IN_LNX
#endif // !NO_SW_POP_IN_LNX
#endif // B_JUDYL

#ifdef BM_SW_BM_IN_WR_HB
#ifdef       BM_SW_CNT_IN_WR
  #error
#endif //    BM_SW_CNT_IN_WR
  #undef  NO_BM_SW_CNT_IN_WR
  #define NO_BM_SW_CNT_IN_WR
#endif // BM_SW_BM_IN_WR_HB

#ifdef BM_SW_BM_IN_LNX
  #undef  _LNX
  #define _LNX
#endif // BM_SW_BM_IN_LNX

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

#ifndef B_JUDYL
#undef DSPLIT_16
#undef DS_4_WAY
#undef DS_4_WAY_A
#undef DS_8_WAY
#undef DS_8_WAY_A
#undef DS_16_WAY
#undef DS_16_WAY_A
#undef DS_AVG
#undef DS_ONE_DIV
#undef DS_SAVE_DIV
#undef DS_NO_CHECK
#undef DS_EARLY_OUT_CHECK
#endif // B_JUDYL

// DSPLIT_16 without DS_16_WAY* or DS_8_WAY* or DS_4_WAY* yields 64-way.
// Without DS_SAVE_[DIV|POP] yields magic method.
// There is no way to get ONE_DIV or 2-divide methods with a calculated pop.
#ifdef DS_4_WAY
  #define _DSPLIT_16
#endif // DS_4_WAY
#ifdef DS_4_WAY_A
  #define _DSPLIT_16
#endif // DS_4_WAY_A
#ifdef DS_8_WAY
  #define _DSPLIT_16
  #undef  DS_EARLY_OUT_CHECK
  #define DS_EARLY_OUT_CHECK
#endif // DS_8_WAY
#ifdef DS_8_WAY_A
  #define _DSPLIT_16
#endif // DS_8_WAY_A
#ifdef DS_16_WAY
  #define _DSPLIT_16
#endif // DS_16_WAY
#ifdef DS_16_WAY_A
  #define _DSPLIT_16
#endif // DS_16_WAY_A

#ifdef DS_EARLY_OUT_CHECK
  #define _DSPLIT_16
  #ifdef DS_AVG
    #error DS_EARLY_OUT_CHECK is not compatible with DS_AVG
  #endif // DS_AVG
#endif // DS_EARLY_OUT_CHECK

#ifdef DS_AVG
  #define _DSPLIT_16
#endif // DS_AVG
#ifdef DS_SAVE_DIV
  #define _DSPLIT_16
#endif // DS_SAVE_DIV
#ifdef DS_ONE_DIV
  #define _DSPLIT_16
#endif // DS_ONE_DIV

#ifdef _DSPLIT_16
  #undef  DSPLIT_16
  #define DSPLIT_16
#endif // _DSPLIT_16

#ifdef DSPLIT_16
  #undef _LNX
  #define _LNX
#endif // DSPLIT_16

#ifdef XX_LISTS
#ifdef NO_SKIP_TO_XX_SW
  #undef  USE_XX_SW
  #define USE_XX_SW
#else // NO_SKIP_TO_XX_SW
  #undef  SKIP_TO_XX_SW
  #define SKIP_TO_XX_SW
#endif // NO_SKIP_TO_XX_SW else
#endif // XX_LISTS

// USE_LOWER_XX_SW turns on the use of narrow switches that decode the
// least significant bits of a digit.
// Default is -UUSE_LOWER_XX_SW
#ifdef USE_LOWER_XX_SW
  #ifdef NO_SKIP_TO_XX_SW
    #error
  #endif // NO_SKIP_TO_XX_SW
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

#ifndef CODE_XX_SW
#if cnBitsPerDigit == cnBitsInD3 && cnBitsInD3 == cnBitsInD2
#if cnBitsInD2 == cnBitsInD1 && (cnBitsPerWord % cnBitsPerDigit) == 0
  #define _CONSTANT_NBPD
#endif // BitsInD2 == BitsInD1 && (BitsPerWord % BitsPerDigit) == 0
#endif // cnBitsPerDigit == cnBitsInD3 && cnBitsInD3 == cnBitsInD2
#endif // CODE_XX_SW

#ifndef cnLogBmSwLinksPerBit
  #ifdef BM_SW_BM_IN_WR_HB
    #define cnLogBmSwLinksPerBit 4 // A lot of assumptions here.
  #elif defined(BM_SW_BM_IN_LNX) // BM_SW_BM_IN_WR_HB
    #define cnLogBmSwLinksPerBit 2 // A lot of assumptions here.
  #else // BM_SW_BM_IN_WR_HB elif BM_SW_BM_IN_LNX
    #define cnLogBmSwLinksPerBit 0
  #endif // BM_SW_BM_IN_WR_HB elif BM_SW_BM_IN_LNX else
#endif // !cnLogBmSwLinksPerBit

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
// E.g. SKIP_LINKS.
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
  #ifndef LVL_IN_WR_HB
    #error SKIP_TO_XX_SW requires LVL_IN_WR_HB
  #endif // !LVL_IN_WR_HB
#endif // SKIP_TO_XX_SW

// Default is -DGOTO_AT_FIRST_IN_LOOKUP.
#ifndef   NO_GOTO_AT_FIRST_IN_LOOKUP
  #undef     GOTO_AT_FIRST_IN_LOOKUP
  #define    GOTO_AT_FIRST_IN_LOOKUP
#endif // NO_GOTO_AT_FIRST_IN_LOOKUP

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

// Default is -DNEXT_FROM_WRAPPER.
#ifndef           NO_NEXT_FROM_WRAPPER
  #undef             NEXT_FROM_WRAPPER
  #define            NEXT_FROM_WRAPPER
#endif // #ifndef NO_NEXT_FROM_WRAPPER

#if cnBitsPerWord > 32
#ifndef NO_BM_POP_IN_WR_HB
  #undef  BM_POP_IN_WR_HB
  #define BM_POP_IN_WR_HB
#endif // #ifndef NO_BM_POP_IN_WR_HB
#endif // cnBitsPerWord > 32

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

#ifdef BM_SW_BM_IN_LNX
#if cnLogBmSwLinksPerBit < 3
  #undef  NO_OFFSET_IN_SW_BM_WORD
  #define NO_OFFSET_IN_SW_BM_WORD
#endif // cnLogBmSwLinksPerBit < 3
#endif // BM_SW_BM_IN_LNX

// Default is OFFSET_IN_SW_BM_WORD for B_JUDYL unless NO_OFFSET_IN_SW_BM_WORD.
// OFFSET_IN_SW_BM_WORD is not used for Judy1.
#undef              OFFSET_IN_SW_BM_WORD
#ifdef B_JUDYL
#ifndef          NO_OFFSET_IN_SW_BM_WORD
  #define           OFFSET_IN_SW_BM_WORD
#endif // ifndef NO_OFFSET_IN_SW_BM_WORD
#endif // B_JUDYL

#ifndef USE_BM_SW_BM_IN_WR_HB
#ifndef USE_BM_SW_BM_IN_LNX
#ifndef   NO_PF_BM_SW_LN
    #undef   PF_BM_SW_LN
    #define  PF_BM_SW_LN
#endif // NO_PF_BM_SW_LN
#endif // !USE_BM_SW_BM_IN_LNX
#endif // !USE_BM_SW_BM_IN_WR_HB

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

// Fix NUM_TYPES on command line based on other ifdefs if
// ALL_SKIP_TO_SW_CASES && AUGMENT_TYPE and the default 9 is not correct.
// It is used to avoid defining duplicate cases in the main switch statement
// for LOOKUP.
#ifdef ALL_SKIP_TO_SW_CASES
#ifdef AUGMENT_TYPE
#ifndef NUM_TYPES
  #define NUM_TYPES  11
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

// Embedded keys were originally sorted with the smallest key in the most
// significant bits of the word -- opposite of an external list.
// Hence REVERSE_SORT_EMBEDDED_KEYS means the biggest key is put in the most
// significant bits of the word -- same as an external list.
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

// We don't have NEW_NEXT_EMPTY code in Judy1 embedded keys for 32-bit
// or for NO_REVERSE_SORT_EMBEDDED_KEYS.
#ifdef EMBED_KEYS
#if !defined(REVERSE_SORT_EMBEDDED_KEYS) || cnBitsPerWord <= 32
  #define _NO_NEW_NEXT_EMPTY_FOR_JUDY1
#endif // !REVERSE_SORT_EMBEDDED_KEYS || cnBitsPerWord <= 32
#endif // EMBED_KEYS
#if defined(B_JUDYL) || !defined(_NO_NEW_NEXT_EMPTY_FOR_JUDY1)
  #ifndef    NO_NEW_NEXT_EMPTY
    #undef      NEW_NEXT_EMPTY
    #define     NEW_NEXT_EMPTY
  #endif // !NO_NEW_NEXT_EMPTY
#else // B_JUDYL || !_NO_NEW_NEXT_EMPTY_FOR_JUDY1
  #ifdef      NEW_NEXT_EMPTY
    #error
  #endif //   NEW_NEXT_EMPTY
#endif // B_JUDYL || !_NO_NEW_NEXT_EMPTY_FOR_JUDY1 else

// We can't handle T_SKIP_TO_FULL_SW with _LVL_IN_TYPE.
#ifdef FULL_SW
#ifdef SKIP_LINKS
#ifndef _LVL_IN_TYPE
  #define _SKIP_TO_FULL_SW
#endif // _LVL_IN_TYPE
#endif // SKIP_LINKS
#endif // FULL_SW

#ifndef _SKIP_TO_FULL_SW
  #undef  LOOKUP_BEFORE_NEXT_EMPTY
  #define LOOKUP_BEFORE_NEXT_EMPTY
#endif // !SKIP_TO_FULL_SW

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

#ifdef NEXT_SHORTCUT
  #undef  NEXT_SHORTCUT_NULL
  #define NEXT_SHORTCUT_NULL
  #undef  NEXT_SHORTCUT_SWITCH
  #define NEXT_SHORTCUT_SWITCH
#endif // defined(NEXT_SHORTCUT)

#ifdef NEXT_SHORTCUT_NULL
  #define _NEXT_SHORTCUT
#elif defined(NEXT_SHORTCUT_SWITCH) // NEXT_SHORTCUT_NULL
  #define _NEXT_SHORTCUT
#endif // NEXT_SHORTCUT_NULL elif NEXT_SHORTCUT_SWITCH

// RESTART_UP_FOR_NEXT[_EMPTY] is not defined by default.
//#define    RESTART_UP_FOR_NEXT
//#define    RESTART_UP_FOR_NEXT_EMPTY

// Our internal NextX acts like JudyXFirst(wKey) if !NEW_NEXT_IS_EXCLUSIVE
// which is the default.
// Hence JudyXNext(wKey) calls internal NextX(wKey+1).
// And JudyXNext(wKey) is usually done on a key that exists.
// LOCATE_GE_USING_EQ_M1 (for nBL without LOCATE_GE_KEY_<nBL>) causes
// LocateGeKeyInList(wKey) to use LocateKey(wKey-1) for nBL.
// Then LocateKeyInList resorts to SearchList only if wKey-1 does not exist.
// NO_LOCATE_GE_USING_EQ_M1 doesn't bother with LocateKey(wKey) and just
// starts with Search.
// NO_LOCATE_GE_USING_EQ_M1 applies to  NEXT
//    LOCATE_GE_USING_EQ_M1 applies to !NEXT

#ifdef PARALLEL_64
  #undef  NO_LOCATE_GE_KEY_X
  #define NO_LOCATE_GE_KEY_X
  #undef  NO_LOCATE_GE_AFTER_LOCATE_EQ
  #define NO_LOCATE_GE_AFTER_LOCATE_EQ
#endif // PARALLEL_64

#ifndef    NO_LOCATE_GE_AFTER_LOCATE_EQ
  #undef      LOCATE_GE_AFTER_LOCATE_EQ
  #define     LOCATE_GE_AFTER_LOCATE_EQ
#endif // !NO_LOCATE_GE_AFTER_LOCATE_EQ

// Default is LOCATE_GE_KEY_<8|16|24|32> which uses LOCATE_GE_KEY for
// LocateGeKeyInList.
#ifdef NO_LOCATE_GE_KEY_X
  #define _LOCATE_GE_KEY_X_NOT_OK
#elif !defined(COMPRESSED_LISTS) || !defined(PSPLIT_PARALLEL)
  #define _LOCATE_GE_KEY_X_NOT_OK
#elif !defined(PARALLEL_128) && !defined(PARALLEL_256)
  #define _LOCATE_GE_KEY_X_NOT_OK
#endif // !defined(PARALLEL_128) && !defined(PARALLEL_256)
#ifndef  _LOCATE_GE_KEY_X_NOT_OK
  #undef  LOCATE_GE_KEY_8
  #define LOCATE_GE_KEY_8
  #undef  LOCATE_GE_KEY_16
  #define LOCATE_GE_KEY_16
    #if cnBitsPerWord > 32 || defined(PARALLEL_SEARCH_WORD)
  #undef  LOCATE_GE_KEY_24
  #define LOCATE_GE_KEY_24
  #undef  LOCATE_GE_KEY_32
  #define LOCATE_GE_KEY_32
    #endif // cnBitsPerWord > 32 || defined(PARALLEL_SEARCH_WORD)
#endif //  _LOCATE_GE_KEY_X_NOT_OK

// LOCATE_GE_KEY_<8|16|24|32> are only independent if
// _ALL_DIGITS_ARE_8_BITS and !USE_XX_SW.
#if defined(_ALL_DIGITS_ARE_8_BITS) && !defined(USE_XX_SW)
  #define _INDEPENDENT_LOCATE_GE_KEY_X
#endif // _ALL_DIGITS_ARE_8_BITS && !USE_XX_SW

#if defined(_INDEPENDENT_LOCATE_GE_KEY_X) && defined(NO_LOCATE_GE_KEY_8)
  #undef  LOCATE_GE_KEY_8
#endif // _INDEPENDENT_LOCATE_GE_KEY_X && NO_LOCATE_GE_KEY_8
#if defined(_INDEPENDENT_LOCATE_GE_KEY_X) && defined(NO_LOCATE_GE_KEY_16)
  #undef  LOCATE_GE_KEY_16
#endif // _INDEPENDENT_LOCATE_GE_KEY_X && NO_LOCATE_GE_KEY_16
#if defined(_INDEPENDENT_LOCATE_GE_KEY_X) && defined(NO_LOCATE_GE_KEY_24)
  #undef  LOCATE_GE_KEY_24
#endif // _INDEPENDENT_LOCATE_GE_KEY_X && NO_LOCATE_GE_KEY_24
#if defined(_INDEPENDENT_LOCATE_GE_KEY_X) && defined(NO_LOCATE_GE_KEY_32)
  #undef  LOCATE_GE_KEY_32
#endif // _INDEPENDENT_LOCATE_GE_KEY_X && NO_LOCATE_GE_KEY_32

#if defined(LOCATE_GE_KEY_8) && defined(NO_LOCATE_GE_KEY_8)
  #error
#endif // LOCATE_GE_KEY_8 && NO_LOCATE_GE_KEY_8
#if defined(LOCATE_GE_KEY_16) && defined(NO_LOCATE_GE_KEY_16)
  #error
#endif // LOCATE_GE_KEY_16 && NO_LOCATE_GE_KEY_16
#if defined(LOCATE_GE_KEY_24) && defined(NO_LOCATE_GE_KEY_24)
  #error
#endif // LOCATE_GE_KEY_16 && NO_LOCATE_GE_KEY_16
#if defined(LOCATE_GE_KEY_32) && defined(NO_LOCATE_GE_KEY_32)
  #error
#endif // LOCATE_GE_KEY_32 && NO_LOCATE_GE_KEY_32

#ifdef LOCATE_GE_KEY_8
  #define _LOCATE_GE_KEY_X
#elif defined(LOCATE_GE_KEY_16) // LOCATE_GE_KEY_8
  #define _LOCATE_GE_KEY_X
#elif defined(LOCATE_GE_KEY_24) // LOCATE_GE_KEY_8 elif LOCATE_GE_KEY_16
  #define _LOCATE_GE_KEY_X
#elif defined(LOCATE_GE_KEY_32) // LOCATE_GE_KEY_8 ... elif LOCATE_GE_KEY_24
  #define _LOCATE_GE_KEY_X
#endif // LOCATE_GE_KEY_8 elif ... elif LOCATE_GE_KEY_32 else

#if defined(_LOCATE_GE_KEY_X) && defined(_LOCATE_GE_KEY_X_NOT_OK)
  #error _LOCATE_GE_KEY_X && _LOCATE_GE_KEY_X_NOT_OK
#endif // defined(_LOCATE_GE_KEY_X) && defined(_LOCATE_GE_KEY_X_NOT_OK)

#endif // ( ! defined(_BDEFINES_H_INCLUDED) )

