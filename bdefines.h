
#if ( ! defined(_BCONF_H_INCLUDED) )
#define _BCONF_H_INCLUDED

// Choose features and parameters.
// DEBUG, RAMMETRICS, GUARDBAND
// SKIP_LINKS, SKIP_PREFIX_CHECK, SORT_LISTS

// Default is -DNDEBUG -UDEBUG_ALL -UDEBUG
// -UDEBUG_INSERT -UDEBUG_REMOVE -UDEBUG_LOOKUP -UDEBUG_MALLOC
// -UDEBUG_COUNT -UDEBUG_NEXT
#if defined(DEBUG_ALL)
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

