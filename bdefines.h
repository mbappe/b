
#if ( ! defined(_BCONF_H_INCLUDED) )
#define _BCONF_H_INCLUDED

// Makefile must define B_JUDYL in order to build libL, i.e. -DB_JUDYL.

// Set initial defines based on whether we are building Judy1 or JudyL.
// If B_JUDYL is not defined then we are building Judy1.
#ifdef B_JUDYL
  #undef NO_EMBED_KEYS
  #define NO_EMBED_KEYS
  #define NO_UA_PARALLEL_128
#endif // B_JUDYL

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
    #undef   DEBUG_LOOKUP
    #define  DEBUG_LOOKUP
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

#endif // ( ! defined(_BCONF_H_INCLUDED) )

