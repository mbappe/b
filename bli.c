
// @(#) $Id: bli.c,v 1.1 2016/12/19 00:10:32 mike Exp mike $
// @(#) $Source: /Users/mike/Judy/b/bli.c,v $

// This file is #included in other .c files four times.
// Once with #define LOOKUP and #undef INSERT, REMOVE and COUNT .
// Once with #define INSERT and #undef LOOKUP, REMOVE and COUNT .
// Once with #define REMOVE and #undef LOOKUP, INSERT and COUNT .
// Once with #define COUNT  and #undef LOOKUP, INSERT and REMOVE.

//#include <emmintrin.h>
//#include <smmintrin.h>
#include <immintrin.h> // __m128i

#if (cnDigitsPerWord <= 1)
//#if (cnBitsPerDigit < cnBitsPerWord)
//#if (cnBitsInD1 < cnBitsPerWord)
// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#else // (cnDigitsPerWord <= 1)

// COUNT is a work-in-progress.
// We're in the process of implementing Judy1Count.
// First we have to be able to find the number of keys
// that precede the specified key.
#if defined(COUNT)
static Word_t
CountSw(Word_t *pwRoot, int nBLR, Switch_t *pwr, int nBL, Word_t wIndex, int nLinks)
{
    (void)pwRoot; (void)nBLR; (void)nLinks;
    DBGC(printf("\nCountSw nBL %d wIndex "Owx"\n", nBL, wIndex));
    Word_t wPopCnt = 0;
    Word_t ww, wwLimit;
    if (wIndex <= (unsigned)nLinks / 2) {
        ww = 0; wwLimit = wIndex;
    } else {
        ww = wIndex; wwLimit = nLinks;
    }
    for (; ww < wwLimit; ++ww) {
        Word_t *pwRootLoop = &pwr_pLinks((Switch_t *)pwr)[ww].ln_wRoot;
        Word_t *pwrLoop = wr_pwr(*pwRootLoop);
        Word_t wPopCntLoop;
        int nTypeLoop = Get_nType(pwRootLoop);
        if (tp_bIsSwitch(nTypeLoop)) {
            wPopCntLoop = PWR_wPopCntBL(pwRootLoop, (Switch_t *)pwrLoop, nBL);
            DBGC(printf("ww %"_fw"d bIsSwitch pwr %p wPopCnt %"_fw"d\n",
                        ww, (void *)pwrLoop, wPopCntLoop));
            wPopCnt += wPopCntLoop;
        } else switch (nTypeLoop) {
        case T_EMBEDDED_KEYS:
            wPopCntLoop = wr_nPopCnt(*pwRootLoop, nBL);
            DBGC(printf("ww %"_fw"d T_EMBED_KEYS wRoot "Owx" wPopCnt %"_fw
                        "d\n", ww, *pwRootLoop, wPopCntLoop));
            wPopCnt += wPopCntLoop;
            break;
        case T_LIST:
      #if ! defined(SEPARATE_T_NULL)
            if (pwrLoop != NULL)
      #endif // ! defined(SEPARATE_T_NULL)
            {
                wPopCntLoop = PWR_xListPopCnt(pwRootLoop, pwrLoop, nBL);
                DBGC(printf("ww %"_fw"d T_LIST pwr %p wPopCnt %"_fw"d\n",
                            ww, (void *)pwr, wPopCntLoop));
                wPopCnt += wPopCntLoop;
            }
            break;
        case T_BITMAP:
            wPopCntLoop = w_wPopCntBL(*(pwrLoop + EXP(nBL - cnLogBitsPerWord)),
                                      nBL);
            DBGC(printf("ww %"_fw"d T_BITMAp pwr %p wPopCnt %"_fw"d\n",
                        ww, (void *)pwr, wPopCntLoop));
            wPopCnt += wPopCntLoop;
            break;
        default:
            printf("\nww %"_fw"d *pwRootLoop "Owx" nTypeLoop %d\n",
                   ww, *pwRootLoop, nTypeLoop);
            assert(0);
        }
    }
    if (ww == (unsigned)nLinks) {
        wPopCnt = PWR_wPopCntBL(pwRoot, pwr, nBLR) - wPopCnt;
  #if defined(INSERT)
        // We're piggybacking on Insert for the time being and
        // PWR_wPopCntBL has already been incremented so we
        // subtract it out here.
        --wPopCnt;
  #endif // defined(INSERT)
    }
    DBGC(printf("\nCountSw wPopCnt %"_fw"d\n", wPopCnt));
    return wPopCnt;
}
#endif // defined(COUNT)

#if defined(SKIP_LINKS)

// SKIP_PREFIX_CHECK applies to LOOKUP only and indicates that we are going
// to defer completing any prefix check until we reach the leaf.
// If SKIP_PREFIX_CHECK is not defined, then we do a prefix check immediately
// when we encounter any skip link and return lookup failure immediately if
// there is a prefix mismatch.
//
// ALWAYS_CHECK_PREFIX_AT_LEAF is relevant only if SKIP_PREFIX_CHECK.  It
// means we don't bother keeping track of whether or not any skip link was
// encountered along the way to the leaf and we always do a whole prefix
// check at the leaf.
// If ALWAYS_CHECK_PREFIX_AT_LEAF is not defined, then we keep keep track
// of whether or not a skip link was encountered on the way down and do
// a prefix check at the leaf if and only if a skip link was encounted.
//
// SAVE_PREFIX is relevant only if SKIP_PREFIX_CHECK.  It means we save a
// pointer to the lowest skip link encountered on the way down and we get
// the prefix from that location once we get to the leaf for doing the
// prefix mismatch check.
// SAVE_PREFIX_TEST_RESULT means we do a prefix check at every skip link
// encountered, but we don't act on the result by failing the lookup on
// mismatch until we reach the leaf.
// If neither SAVE_PREFIX nor SAVE_PREFIX_TEST_RESULT is defined we
// get the whole prefix from the lowest switch and use that for the
// prefix check at the leaf.
static intptr_t
PrefixMismatch(Word_t *pwRoot, Word_t *pwr, Word_t wKey,
               int nBL,
#if defined(CODE_BM_SW)
               int bBmSw,
#endif // defined(CODE_BM_SW)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
               int *pbNeedPrefixCheck,
  #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #if defined(SAVE_PREFIX)
      #if defined(PP_IN_LINK)
               Word_t **ppwRootPrefix,
      #else // defined(PP_IN_LINK)
               Word_t **ppwrPrefix,
      #endif // defined(PP_IN_LINK)
              int *pnBLRPrefix,
  #elif defined(SAVE_PREFIX_TEST_RESULT)
               int *pbPrefixMismatch,
  #endif // defined(SAVE_PREFIX)
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
               int *pnBLR)
{
    (void)pwRoot; (void)pwr; (void)wKey; (void)nBL; (void)pnBLR;

  #if defined(TYPE_IS_RELATIVE)
    int nBLR = nDL_to_nBL_NAT(nBL_to_nDL(nBL) - wr_nDS(*pwRoot));
  #else // defined(TYPE_IS_RELATIVE)
    int nBLR = wr_nBL(*pwRoot);
  #endif // defined(TYPE_IS_RELATIVE)
    assert(nBLR < nBL); // reserved

  #if ! defined(LOOKUP) || ! defined(SKIP_PREFIX_CHECK) \
            || defined(SAVE_PREFIX_TEST_RESULT)

    Word_t wPrefix;
      #if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
        wPrefix = w_wPrefixBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
    } else
      #endif // defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    {
        wPrefix =
      #if defined(CODE_BM_SW)
            bBmSw ? PWR_wPrefixNATBL(pwRoot, (BmSwitch_t *)pwr, nBLR) :
      #endif // defined(CODE_BM_SW)
                      PWR_wPrefixNATBL(pwRoot, (  Switch_t *)pwr, nBLR) ;
    }

    int bPrefixMismatch;
          #if defined(PP_IN_LINK)
    if (nBL == cnBitsPerWord) {
        // prefix is 0
        bPrefixMismatch = (wKey >= EXP(nBLR));
    } else
          #endif // defined(PP_IN_LINK)
    {
      #if defined(COUNT)
        *pnBLR = nBLR; // ? do this unconditionally at top ?
        DBGC(printf("PM: wKey "OWx" wPrefix "OWx" nBLR %d\n",
                    wKey, wPrefix, nBLR));
        return (wKey - wPrefix) >> nBLR; // positive means key is big
      #else // defined(COUNT)
        bPrefixMismatch = ((int)LOG(1 | (wPrefix ^ wKey)) >= nBLR);
      #endif // defined(COUNT)
    }
  #endif // ! defined(LOOKUP) || ! defined(SKIP_PREFIX_CHECK) || ...

  #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #if defined(SAVE_PREFIX)
    // Save info needed for prefix check at leaf.
    // Does this obviate the need for requiring a branch above the
    // bitmap as a place holder for the prefix check at the leaf?
    // It just might.
    // Maybe it's faster to use a word that is shared by all
    // than one that is shared by fewer.
    // Only one of pwRoot or pwr is needed by PWR_wPrefix.  If PP_IN_LINK,
    // then it uses pwRoot.  If not, then it uses pwr.
          #if defined(PP_IN_LINK)
    // If PP_IN_LINK and nBL == cnBitsPerWord there is no link.
    // Saving pwRoot for the purpose of looking at the prefix in the link
    // later makes no sense.  Use *ppwRootPrefix == NULL to indicate
    // that the prefix is 0.
    // Does this work if the caller passes in &wRoot rather than pwRoot?
    // I don't think it does.
    *ppwRootPrefix = (nBL != cnBitsPerWord) ? pwRoot : NULL;
          #else // defined(PP_IN_LINK)
    *ppwrPrefix = pwr;
          #endif // defined(PP_IN_LINK)
    *pnBLRPrefix = nBLR; // nBLR at which saved prefix applies
      #elif defined(SAVE_PREFIX_TEST_RESULT)
    *pbPrefixMismatch = bPrefixMismatch;
      #endif // defined(SAVE_PREFIX)
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        // Record that there were prefix bits that were not checked.
    *pbNeedPrefixCheck |= 1;
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
  #else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    if (bPrefixMismatch)
    {
        DBGX(printf("Mismatch wPrefix "Owx" nBL %d nBLR %d pwRoot %p\n",
                    wPrefix, nBL, nBLR, (void *)pwRoot));
        // Caller doesn't need/get an updated *pnBLR in this case.
        return 1; // prefix mismatch
    }
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)

    *pnBLR = nBLR;

    return 0; // no prefix mismatch
}

#endif // defined(SKIP_LINKS)

#if defined(CODE_BM_SW)
    #define IS_BM_SW_ARG(_nType)  tp_bIsBmSw(_nType),
#else // defined(CODE_BM_SW)
    #define IS_BM_SW_ARG(_nType)
#endif // defined(CODE_BM_SW)

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    #define P_B_NEED_PREFIX_CHECK_ARG
#else // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    #define P_B_NEED_PREFIX_CHECK_ARG  &bNeedPrefixCheck,
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)

#if defined(PP_IN_LINK)
    #define P_PWR_PREFIX_ARG  &pwRootPrefix,
#else // defined(PP_IN_LINK)
    #define P_PWR_PREFIX_ARG  &pwrPrefix,
#endif // defined(PP_IN_LINK)

#if defined(SAVE_PREFIX)
    #define SAVE_PREFIX_ARGS \
        P_PWR_PREFIX_ARG \
        &nBLRPrefix,
#elif defined(SAVE_PREFIX_TEST_RESULT)
    #define SAVE_PREFIX_ARGS  &bPrefixMismatch,
#else // defined(SAVE_PREFIX_TEST_RESULT)
    #define SAVE_PREFIX_ARGS
#endif // defined(SAVE_PREFIX)

#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    #define LOOKUP_SKIP_PREFIX_CHECK_ARGS \
        P_B_NEED_PREFIX_CHECK_ARG \
        SAVE_PREFIX_ARGS
#else // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    #define LOOKUP_SKIP_PREFIX_CHECK_ARGS
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)

// PrefixMismatch requires a real pwRoot (as opposed to &wRoot) when it
// may need to save that value for later dereference by Lookup at the leaf.
#if defined(PP_IN_LINK)

  #if ! defined(PWROOT_ARG_FOR_LOOKUP) && ! defined(PWROOT_AT_TOP_FOR_LOOKUP)
      #error PWROOT_[ARG|AT_TOP]_FOR_LOOKUP is required for PP_IN_LINK ...
  #endif // ! defined(PWROOT_ARG_FOR_LOOKUP) && ! PWROOT_AT_TOP_FOR_LOOKUP

    #define PWROOT_ARG  pwRoot,

#else // defined(PP_IN_LINK)

  #if (defined(PWROOT_ARG_FOR_LOOKUP) || defined(PWROOT_AT_TOP_FOR_LOOKUP)) \
          && defined(USE_PWROOT_FOR_LOOKUP)

    #define PWROOT_ARG  pwRoot,

  #else // defined(USE_PWROOT_FOR_LOOKUP)

    #define PWROOT_ARG  &wRoot,

  #endif // defined(USE_PWROOT_FOR_LOOKUP)

#endif // defined(PP_IN_LINK)

#define PREFIX_MISMATCH(_nBL, _nType) \
    (tp_bIsSkip(_nType) \
        ? PrefixMismatch(PWROOT_ARG \
                         pwr, wKey, (_nBL), IS_BM_SW_ARG(_nType) \
                         LOOKUP_SKIP_PREFIX_CHECK_ARGS \
                         &nBLR) \
        : 0)

// nBL is bits left after finding pwRoot (not after decoding *pwRoot).
// nBL == 0 means cnBitsPerWord? (would make it less general).
// Or nBL == BLBits(pwRoot) + 1?  Or use cnLogBitsPerWord bits for nBL field?
// pwRoot is a pointer to the root of a subtree.
// nBS is number of bits to skip (common bits for all present keys below).
// nBW is number of bits decoded by the switch pointed to.
// nBW == 0 means all bits left, i.e. nBL - nBS; keys or list or bitmap.
// nBW != 0 means switch at nBL - nBS decoding nBW bits.
// If nBL - nBS <= cnBitsPerWord / 2, then use magic to extract type for nBL
// that don't have room for type.  Magic only works if Word_t can hold more
// than one key.
// If P, e.g. nBL - nBL <= some threshold, then sw has two-word links/buckets.
// What should P be?
static inline int
CaseGuts(int nBL, Word_t *pwRoot, int nBS, int nBW, int nType, Word_t *pwr)
{
    (void)nBL; (void)pwRoot; (void)nBS; (void)nBW; (void)nType; (void)pwr;

    return 1;
}

#if defined(LOOKUP)
static Status_t
Lookup(
      #if defined(PWROOT_ARG_FOR_LOOKUP)
       Word_t* pwRoot,
      #else // defined(PWROOT_ARG_FOR_LOOKUP)
       Word_t wRoot,
      #endif // defined(PWROOT_ARG_FOR_LOOKUP)
       Word_t wKey
       )
#else // defined(LOOKUP)
  #if defined(COUNT)
Word_t
  #else // defined(COUNT)
Status_t
  #endif // defined(COUNT)
InsertRemove(Word_t *pwRoot, Word_t wKey, int nBL)
#endif // defined(LOOKUP)
{
    int nBLUp; (void)nBLUp; // silence gcc
    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
#if defined(SAVE_PREFIX_TEST_RESULT)
    int bPrefixMismatch = 0; (void)bPrefixMismatch;
#endif // defined(SAVE_PREFIX_TEST_RESULT)
#if defined(LOOKUP)
    int nBL = cnBitsPerWord;
  #if defined(PWROOT_ARG_FOR_LOOKUP)
    Word_t wRoot = *pwRoot;
  #else // defined(PWROOT_ARG_FOR_LOOKUP)
      #if defined(BM_IN_LINK)
    // Is it a problem that we appear to be ignoring PWROOT_AT_TOP_FOR_LOOKUP
    // here? Should we be ensuring that it is not defined?
    Word_t *pwRoot = NULL; // used for top detection
      #else // defined(BM_IN_LINK)
          #if defined(PWROOT_AT_TOP_FOR_LOOKUP) \
              || defined(POP_IN_WR_HB) && ! defined(SEARCH_FROM_WRAPPER)
    Word_t *pwRoot = &wRoot;
          #else // defined(PWROOT_AT_TOP_FOR_LOOKUP) || ...
    // Silence unwarranted gcc used before initialized warning.
    // pwRoot is only uninitialized on the first time through the loop.
    // And we only use it if nBL != cnBitsPerWord
    // or if bNeedPrefixCheck is true.
    // And both of those imply it's not the first time through the loop.
    Word_t *pwRoot = pwRoot;
          #endif // defined(PWROOT_AT_TOP_FOR_LOOKUP) || ...
      #endif // defined(BM_IN_LINK)
  #endif // defined(PWROOT_ARG_FOR_LOOKUP)
#else // defined(LOOKUP)
  #if defined(CODE_XX_SW)
    Word_t *pwRootPrev = NULL; (void)pwRootPrev;
      #if defined(SKIP_TO_XX_SW)
    int nBLPrev = 0; (void)nBLPrev;
      #endif // defined(SKIP_TO_XX_SW)
  #endif // defined(CODE_XX_SW)
    Word_t wRoot;
  #if !defined(RECURSIVE)
          #if defined(INSERT)
    int nIncr = 1;
          #endif // defined(INSERT)
          #if defined(REMOVE)
    int nIncr = -1;
          #endif // defined(REMOVE)
  #endif // !defined(RECURSIVE)
#endif // defined(LOOKUP)
#if !defined(RECURSIVE)
  #if !defined(LOOKUP)
    Word_t *pwRootOrig = pwRoot; (void)pwRootOrig;
  #elif defined(BM_IN_LINK)
    Word_t *pwRootOrig = pwRoot; (void)pwRootOrig;
  #endif // !defined(LOOKUP)
#endif // !defined(RECURSIVE)
    int nBLR;
    Word_t wPopCnt; (void)wPopCnt;
#if ! defined(LOOKUP)
    int bCleanup = 0;
#endif // ! defined(LOOKUP)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrPrev = pwrPrev; // suppress "uninitialized" compiler warning
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    int nBLOrig = nBL; (void)nBLOrig;
    Word_t *pwRootPrefix = NULL; (void)pwRootPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    int nBLRPrefix = 0; (void)nBLRPrefix;
#if ! defined(LOOKUP)
    int nPos = nPos; // suppress "uninitialized" compiler warning
#endif // ! defined(LOOKUP)

    DBGX(printf("\n# %s ", strLookupOrInsertOrRemove));

#if defined(COUNT)
    Word_t wPopCntSum = 0;
#endif // defined(COUNT)

#if ! defined(LOOKUP)
#if defined(INSERT) || defined(REMOVE)
  #if !defined(RECURSIVE)
top:;
  #endif // !defined(RECURSIVE)
#endif // defined(INSERT) || defined(REMOVE)
    wRoot = *pwRoot;
#endif // ! defined(LOOKUP)
    nBLR = nBL;

#if defined(LOOKUP) || !defined(RECURSIVE)
again:;
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if defined(SKIP_LINKS)
    assert(nBLR == nBL);
#endif // defined(SKIP_LINKS)
#if ( ! defined(LOOKUP) )
  #if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
  #endif // ! defined(USE_XX_SW)
    DBGX(printf("# pwRoot %p ", (void *)pwRoot));
#else // ( ! defined(LOOKUP) )
    SMETRICS(j__TreeDepth++);
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# wRoot "OWx" wKey "OWx" nBL %d\n", wRoot, wKey, nBL));

#if ! defined(LOOKUP) /* don't care about performance */ \
      || (defined(USE_PWROOT_FOR_LOOKUP) \
              && (defined(PWROOT_ARG_FOR_LOOKUP) \
                  || defined(PWROOT_AT_TOP_FOR_LOOKUP)))
    int nType = Get_nType(pwRoot);
#else // ! defined(LOOKUP) || defined(USE_PWROOT_FOR_LOOKUP) && it's ok
    int nType = Get_nType(&wRoot);
#endif // ! defined(LOOKUP) || defined(USE_PWROOT_FOR_LOOKUP) && it's ok
    goto again2;
again2:;
    Word_t *pwr = wr_pwr(wRoot);
    goto again3;
again3:;
  #if defined(EXTRA_TYPES)
    switch (wRoot & MSK(cnBitsMallocMask + 1))
  #else // defined(EXTRA_TYPES)
    switch (nType)
  #endif // defined(EXTRA_TYPES)
    {

#if defined(SKIP_LINKS)

    default: // printf("unknown type %d\n", nType); assert(0); exit(0);
    // case T_SKIP_TO_SWITCH: // skip link to uncompressed switch
    {
        // pwr points to a switch
        DBGX(printf("SKIP_TO_SW\n"));

        // Looks to me like PrefixMismatch has no performance issues with
        // not all digits being the same size.  It doesn't care.
        // But it does use nBL a couple of times.  Maybe it would help to
        // have bl tests here and call with a constant.  Possibly more
        // interestingly it does compare nBL to cnBitsPerWord.

        // PREFIX_MISMATCH doesn't update nBLR if there is no match
        // unless defined(COUNT).
        intptr_t nPrefixMismatch = PREFIX_MISMATCH(nBL, nType);
        if (nPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_SW: COUNT PM %"_fw"d\n", nPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (nPrefixMismatch > 0) {
                Word_t wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
                DBGC(printf("SKIP_TO_SW: PM wPopCnt %"_fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
            }
  #endif // defined(COUNT)
            break;
        }

        // Logically, if we could arrange the source code accordingly,
        // we could just fall through to T_SWITCH.
        // But, with so many ifdefs, it is easier to use goto.
        // Is falling through to the next case faster than a goto?
        // Does the compiler turn a fall-through into a goto?
        // Does the compiler rearrange code to make gotos (in and/or out
        // of switch statements) be fall-throughs when possible?
        // Is it a waste of energy for us to try to figure out how
        // to ifdef the code to prefer fall-through to goto?
        // I have a limited amount of empirical evidence suggesting that
        // a goto is equivalent to a fall-through in this particular case.
        goto t_switch;

    } // end of default case

#if defined(SKIP_TO_BM_SW)

    case T_SKIP_TO_BM_SW:
    {
        // pwr points to a bitmap switch
        DBGX(printf("SKIP_TO_BM_SW\n"));

        // PREFIX_MISMATCH doesn't update nBLR if there is no match
        // unless defined(COUNT).
        intptr_t nPrefixMismatch = PREFIX_MISMATCH(nBL, nType);
        if (nPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_BM_SW: COUNT PM %"_fw"d\n",
                        nPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (nPrefixMismatch > 0) {
                Word_t wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
                DBGC(printf("SKIP_TO_BM_SW: PM wPopCnt %"_fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_bm_sw;

    } // end of T_SKIP_TO_BM_SW case

#endif // defined(SKIP_TO_BM_SW)

#if defined(SKIP_TO_XX_SW) // Doesn't work yet.

    case T_SKIP_TO_XX_SW: // skip link to narrow/wide switch
    {
        // pwr points to a variable-width aka doubling switch
        DBGX(printf("SKIP_TO_XX_SW\n"));

        // Looks to me like PrefixMismatch has no performance issues with
        // not all digits being the same size.  It doesn't care.
        // But it does use nBL a couple of times.  Maybe it would help to
        // have bl tests here and call with a constant.  Possibly more
        // interestingly it does compare nBL to cnBitsPerWord.

        // PREFIX_MISMATCH doesn't update nBLR if there is no match
        // unless defined(COUNT).
        intptr_t nPrefixMismatch = PREFIX_MISMATCH(nBL, nType);
        if (nPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_BM_SW: COUNT PM %"_fw"d\n",
                        nPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (nPrefixMismatch > 0) {
                Word_t wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
                DBGC(printf("SKIP_TO_BM_SW: PM wPopCnt %"_fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_xx_sw;

    } // end of T_SKIP_TO_XX_SW case

#endif // defined(SKIP_TO_XX_SW)

#endif // defined(SKIP_LINKS)

    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
#if defined(EXTRA_TYPES)
    case T_SWITCH | EXP(cnBitsMallocMask): // close switch w/o bm
#endif // defined(EXTRA_TYPES)
    {
        goto t_switch; // silence cc in case other the gotos are ifdef'd out
t_switch:;
        // nBL is bits left after picking the link from the previous switch
        // nBL is not reduced by any skip indicated in that link
        // nBLR is nBL reduced by any skip indicated in that link
        // nBLR is bits left at the top of this switch

        DBGX(printf("T_SWITCH nBL %d nBLR %d wPopCnt %"_fw"d pLinks %p\n",
                    nBL, nBLR, PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR),
                    (void *)pwr_pLinks((Switch_t *)pwr)));

  #if defined(INSERT) || defined(REMOVE)
        // Cleanup is for adjusting tree after successful insert or remove.
        // It is not for undoing counts after unsuccessful insert or remove.
        if (unlikely(bCleanup)) {
      #if defined(INSERT)
          #if (cn2dBmWpkPercent != 0)
            if (nBLR <= cnBitsLeftAtDl2) {
                InsertCleanup(wKey, nBL, pwRoot, wRoot);
            }
          #endif // (cn2dBmWpkPercent != 0)
      #else // defined(INSERT)
            RemoveCleanup(wKey, nBL, nBLR, pwRoot, wRoot);
      #endif // defined(INSERT)
            if (*pwRoot != wRoot) { goto restart; }
        } else {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR, wPopCnt + nIncr);
        }
      #if defined(INSERT)
        //pwRootPrev = pwRoot; // save pwRoot for T_XX_SW for InsertGuts
      #endif // defined(INSERT)
  #endif // defined(INSERT) || defined(REMOVE)

  #if defined(SKIP_TO_XX_SW)
      #if defined(TYPE_IS_RELATIVE)
        assert( ! tp_bIsSkip(wRoot)
            || (wr_nDS(wRoot) == nBL_to_nDL(nBL) - nBL_to_nDL(nBLR)) );
      #else // defined(TYPE_IS_RELATIVE)
        assert((pwr_nBL(&wRoot) == nBLR) /* || ! tp_bIsSkip(wRoot) */ || 0);
      #endif // defined(TYPE_IS_RELATIVE)
  #endif // defined(SKIP_TO_XX_SW)

        //int nBitsIndexSz = nBL_to_nBitsIndexSzNAX(nBLR);
        int nBitsIndexSz = nBL_to_nBitsIndexSzNAB(nBLR);
        nBL = nBLR - nBitsIndexSz;

  #if defined(SKIP_TO_XX_SW)
        // Hmm. Are we sure nBL_to_nBitsIndexSzNAB is faster than pwr_nBw?
        assert(pwr_nBW(&wRoot) == nBitsIndexSz);
  #endif // defined(SKIP_TO_XX_SW)

        // Is it possible that MSK(nBLR - nBL) would be faster here in some
        // cases, e.g. BitsInD2 != BitsPerDigit and BitsInD3 != BitsPerDigit?
        // We're banking on the compilier noticing that nB
        Word_t wIndex = (wKey >> nBL) & MSK(nBitsIndexSz);

        DBGX(printf("T_SWITCH wIndex %d 0x%x\n", (int)wIndex, (int)wIndex));

#if defined(COUNT)
        wPopCnt = CountSw(pwRoot, nBLR, (Switch_t *)pwr, nBL, wIndex,
                          1 << nBitsIndexSz);
        DBGC(printf("T_SWITCH wPopCnt %"_fw"d\n", wPopCnt));
        wPopCntSum += wPopCnt;
#endif // defined(COUNT)

        pwRoot = &pwr_pLinks((Switch_t *)pwr)[wIndex].ln_wRoot;

        goto switchTail;
switchTail:;

        wRoot = *pwRoot;
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just visited in
        // the next iteration of the loop if we've reached a leaf so we
        // preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // Is there any reason to have
        // EXP(cnBitsInD1) <= (sizeof(Link_t) * 8)? What about lazy conversion
        // of embedded keys at nBL > sizeof(Link_t) * 8 to
        // nBL == sizeof(Link_t) * 8?
        // Initialize warns if cnBitsInD1 is too small relative
        // to sizeof(Link_t).
        assert(EXP(nBL) > (sizeof(Link_t) * 8));
        // We've left some code here, in the comment, for reference only.
        // The first test is done at compile time and will make the rest
        // go away.
        //
        // if ((EXP(cnBitsInD1) <= (sizeof(Link_t) * 8))
        //     && (nBL <= (int)LOG(sizeof(Link_t) * 8)))
        // {
        //     goto t_bitmap;
        // }

        DBGX(printf("switchTail: pwRoot %p wRoot "OWx" nBL %d\n",
                    (void *)pwRoot, wRoot, nBL));

        nBLR = nBL; // Advance nBLR to the bottom of this switch now.
#if defined(LOOKUP) || !defined(RECURSIVE)
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
#else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nBL);
#endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of case T_SWITCH

#if defined(CODE_XX_SW)

    case T_XX_SW: // no-skip (aka close) switch (vs. distant switch) w/o bm
#if defined(EXTRA_TYPES)
    case T_XX_SW | EXP(cnBitsMallocMask): // close switch w/o bm
#endif // defined(EXTRA_TYPES)
    {
        goto t_xx_sw;
t_xx_sw:;
        // nBL is bits left after picking the link from the previous switch
        // nBL is not reduced by any skip indicated in that link
        // nBLR is nBL reduced by any skip indicated in that link
        // nBLR is bits left at the top of this switch

        DBGX(printf("T_XX_SW nBL %d nBLR %d wPopCnt %"_fw"d pLinks %p\n",
                    nBL, nBLR, PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR),
                    (void *)pwr_pLinks((Switch_t *)pwr)));

  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
      #if defined(INSERT)
          #if (cn2dBmWpkPercent != 0)
            assert(nBLR <= cnBitsLeftAtDl2);
            InsertCleanup(wKey, nBL, pwRoot, wRoot);
          #endif // (cn2dBmWpkPercent != 0)
      #else // defined(INSERT)
            RemoveCleanup(wKey, nBL, nBLR, pwRoot, wRoot);
      #endif // defined(INSERT)
            if (*pwRoot != wRoot) { goto restart; }
        } else {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR, wPopCnt + nIncr);
        }
      #if defined(INSERT)
        pwRootPrev = pwRoot; // save pwRoot for T_XX_SW for InsertGuts
          #if defined(SKIP_TO_XX_SW)
        nBLPrev = nBL;
          #endif // defined(SKIP_TO_XX_SW)
      #endif // defined(INSERT)
  #endif // defined(INSERT) || defined(REMOVE)

  #if defined(SKIP_TO_XX_SW)
      #if defined(TYPE_IS_RELATIVE)
        assert( ! tp_bIsSkip(wRoot)
            || (wr_nDS(wRoot) == nBL_to_nDL(nBL) - nBL_to_nDL(nBLR)) );
      #else // defined(TYPE_IS_RELATIVE)
          #if defined(DEBUG)
        if (pwr_nBL(&wRoot) != nBLR) {
            printf("T_XX_SW: pwr_nBL %d nBLR %d\n", pwr_nBL(&wRoot), nBLR);
        }
          #endif // defined(DEBUG)
        assert(pwr_nBL(&wRoot) == nBLR);
      #endif // defined(TYPE_IS_RELATIVE)
  #endif // defined(SKIP_TO_XX_SW)
  #if ! defined(LOOKUP) /* don't care about performance */ \
      || (defined(USE_PWROOT_FOR_LOOKUP) \
              && (defined(PWROOT_ARG_FOR_LOOKUP) \
                      || defined(PWROOT_AT_TOP_FOR_LOOKUP)))
        int nBW = Get_nBW(pwRoot);
  #else // ! defined(LOOKUP) || defined(USE_PWROOT_FOR_LOOKUP) && it's ok
        int nBW = Get_nBW(&wRoot);
  #endif // ! defined(LOOKUP) || defined(USE_PWROOT_FOR_LOOKUP) && it's ok
        nBL = nBLR - nBW;
        int nIndex = (wKey >> nBL) & MSK(nBW);

        DBGX(printf("T_XX_SW nBW %d nIndex %d 0x%x\n",
                    nBW, nIndex, nIndex));

#if defined(COUNT)
        wPopCnt = CountSw(pwRoot, nBLR, (Switch_t *)pwr, nBL, nIndex, 1<<nBW);
        DBGC(printf("T_XX_SW wPopCnt %"_fw"d\n", wPopCnt));
        wPopCntSum += wPopCnt;
#endif // defined(COUNT)

        pwRoot = &pwr_pLinks((Switch_t *)pwr)[nIndex].ln_wRoot;

        wRoot = *pwRoot;
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We may need to check the prefix of the switch we just visited in
        // the next iteration of the loop if we've reached a leaf so we
        // preserve the value of pwr.
        pwrPrev = pwr;
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        assert(EXP(nBL) > (sizeof(Link_t) * 8));

        DBGX(printf("T_XX_SW pwRoot %p wRoot "OWx" nBL %d\n",
                    (void *)pwRoot, wRoot, nBL));

  #if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
      #if defined(NO_TYPE_IN_XX_SW)
        // ZERO_POP_MAGIC is valid only if a word can hold at least two keys.
        assert(EmbeddedListPopCntMax(nBL) >= 2);
        if (wRoot == ZERO_POP_MAGIC)
      #else // defined(NO_TYPE_IN_XX_SW)
        if (wRoot == 0)
      #endif // defined(NO_TYPE_IN_XX_SW)
        { return Failure; }
  #else // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
        // Zero pop check is done in t_embedded_keys.
        // I don't know why yet, but it measures faster to defer
        // the zero check until then.
  #endif // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
  #if defined(NO_TYPE_IN_XX_SW) \
           || (defined(LOOKUP) && defined(HANDLE_DL2_IN_EMBEDDED_KEYS))
        // Blow-ups are handled in t_embedded_keys.
        goto t_embedded_keys;
  #else // defined(NO_TYPE_IN_XX_SW) || handle dl2 in t_embedded_keys
        // The only thing we do at "again" before switching on nType
        // is extract nType and pwr from wRoot.
        // We don't do any updating of nBL or nBLR.
        nBLR = nBL;
      #if defined(LOOKUP) || !defined(RECURSIVE)
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
      #else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(pwRoot, wKey, nBL);
      #endif // defined(LOOKUP) || !defined(RECURSIVE)
  #endif // defined(NO_TYPE_IN_XX_SW) || handle dl2 in t_embedded_keys

    } // end of case T_XX_SW

#endif // defined(CODE_XX_SW)

#if defined(CODE_BM_SW)

  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_FULL_BM_SW:
      #if defined(EXTRA_TYPES)
    case T_FULL_BM_SW | EXP(cnBitsMallocMask): // no skip switch
      #endif // defined(EXTRA_TYPES)
    {
      #if defined(LOOKUP)
// Skip over sw_awBm.  The rest of BmSwitch_t must be same as Switch_t.
        pwr = (Word_t *)&((BmSwitch_t *)pwr)->sw_wPrefixPop;
        goto t_switch;
      #endif // defined(LOOKUP)
        goto t_bm_sw;

    } // end of case T_FULL_BM_SW

  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_BM_SW:
  #if defined(EXTRA_TYPES)
    case T_BM_SW | EXP(cnBitsMallocMask): // no skip switch
  #endif // defined(EXTRA_TYPES)
    {
        goto t_bm_sw; // silence cc in case other the gotos are ifdef'd out
t_bm_sw:;
  #if defined(BM_SW_FOR_REAL) || ! defined(LOOKUP) || defined(DEBUG)
        nBLUp = nBL;
  #endif // defined(BM_SW_FOR_REAL) || ! defined(LOOKUP) || defined(DEBUG)
        // This assertion is a reminder that the NAX in the line below and
        // possibly later in this case are cheating.
        // The NAX assumes our test program doesn't generate any keys
        // that have bits set in the top digit.
        // It's really only legitimate to use NAB.
        assert(nBLR != cnBitsPerWord);
        //nBL = nBLR - nBL_to_nBitsIndexSzNAX(nBL);
        nBL = nBLR - nBW_from_nBL_NAB3(nBLR);

        Word_t wIndex = ((wKey >> nBL)
            // It is ok to use NAX here even though we might be at top because
            // we don't care if it returns an index size that is too big.
            // Of course, this assumes that NAX will yield nBitsIndexSz
            // greater than or equal to the actual value and won't cause
            // a crash.
            & (MSK(nBL_to_nBitsIndexSzNAX(nBLR))));

        DBGX(printf("T_BM_SW nBLR %d pLinks %p wIndex %d 0x%x\n", nBLR,
             (void *)pwr_pLinks((Switch_t *)pwr), (int)wIndex, (int)wIndex));

  #if defined(BM_IN_LINK)
        // Have not coded for skip link at top here and elsewhere.
        assert( ! tp_bIsSkip(nType) || (nBLUp != cnBitsPerWord) );
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nBL == cnBitsPerWord and pwRoot not at the top.
        // We need to know if there is a link surrounding *pwRoot.
        // InsertGuts always calls back into Insert with the same pwRoot
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nBL == cnBitsPerWord and pwRoot not at the top.
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nBL == cnBitsPerWord)
      #else // defined(RECURSIVE)
                && (pwRoot == pwRootOrig)
          #if !defined(LOOKUP)
                && (nBLOrig == cnBitsPerWord)
          #endif // !defined(LOOKUP)
      #endif // defined(RECURSIVE)
            ) )
  #endif // defined(BM_IN_LINK)
        {
  // Is this ifdef necessary?  Or will the compiler figure it out?
  #if (cnBitsPerDigit > cnLogBitsPerWord)
            unsigned nBmOffset = wIndex >> cnLogBitsPerWord;
  #else // (cnBitsPerDigit > cnLogBitsPerWord)
            unsigned nBmOffset = 0;
  #endif // (cnBitsPerDigit > cnLogBitsPerWord)
           Word_t wBm = PWR_pwBm(pwRoot, pwr)[nBmOffset];
           Word_t wBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
           // Test to see if link exists before figuring out where it is.
           if ( ! (wBm & wBit) )
           {
  #if defined(BM_SW_FOR_REAL)
                DBGX(printf("missing link\n"));
                nBL = nBLUp; // back up for InsertGuts
                goto notFound; // why can't we just "break;"?
  #else // defined(BM_SW_FOR_REAL)
                assert(0); // only for now
  #endif // defined(BM_SW_FOR_REAL)
            }
            Word_t wBmMask = wBit - 1;
            wIndex = 0;
  #if (cnBitsPerDigit > cnLogBitsPerWord)
            for (unsigned nn = 0; nn < nBmOffset; nn++)
            {
                wIndex += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
            }
  #endif // (cnBitsPerDigit > cnLogBitsPerWord)
            DBGX(printf("\npwRoot %p PWR_pwBm %p\n",
                        (void *)pwRoot, (void *)PWR_pwBm(pwRoot, pwr)));
            wIndex += __builtin_popcountll(wBm & wBmMask);
        }

#if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
  #if defined(INSERT)
          #if (cn2dBmWpkPercent != 0)
            if (nBLR <= cnBitsLeftAtDl2) {
                InsertCleanup(wKey, nBLUp, pwRoot, wRoot);
            }
          #endif // (cn2dBmWpkPercent != 0)
  #else // defined(INSERT)
            RemoveCleanup(wKey, nBLUp, nBLR, pwRoot, wRoot);
  #endif // defined(INSERT)
            if (*pwRoot != wRoot) { goto restart; }
        } else {
            // Increment or decrement population count on the way in.
            wPopCnt = PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
            set_PWR_wPopCntBL(pwRoot,
                              (BmSwitch_t *)pwr, nBLR, wPopCnt + nIncr);
            DBGX(printf("wPopCnt %zd\n",
                (size_t)PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));
        }
#endif // defined(INSERT) || defined(REMOVE)

#if defined(COUNT)
        wPopCntSum += CountSw(pwRoot, nBLR, (Switch_t *)pwr, nBL, wIndex, MAXUINT);
#endif // defined(COUNT)

        pwRoot = &pwr_pLinks((BmSwitch_t *)pwr)[wIndex].ln_wRoot;

        goto switchTail;

    } // end of case T_BM_SW

#endif // defined(CODE_BM_SW)

#if (cwListPopCntMax != 0)

    case T_LIST:
#if defined(EXTRA_TYPES)
    case T_LIST | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        goto t_list;
t_list:;
        DBGX(printf("T_LIST nBL %d\n", nBL));
  #if defined(INSERT) || defined(REMOVE)
        DBGX(printf("T_LIST bCleanup %d nIncr %d\n", bCleanup, nIncr));
  #endif // defined(INSERT) || defined(REMOVE)
        DBGX(printf("wKeyPopMask "OWx"\n", wPrefixPopMaskBL(nBL)));

  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

      #if defined(PP_IN_LINK)
          #if defined(INSERT)
        if (nIncr == -1) { return Failure; }
          #endif // defined(INSERT)
          #if defined(REMOVE)
        if (nIncr == 1) { return Failure; }
          #endif // defined(REMOVE)
      #endif // defined(PP_IN_LINK)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COUNT) && ! defined(SEPARATE_T_NULL)
        if (pwr == NULL) { nPos = ~0; }
      #endif // defined(COUNT) && ! defined(SEPARATE_T_NULL)

      #if defined(COMPRESSED_LISTS)
          #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
        // nBL is relative to the bottom of the switch
        // containing the pointer to the leaf.
        // Can we use NAT here since bNeedPrefixCheck will never
        // be true if we are at the top?
        // If the top digit is smaller than the rest, then NAT will
        // return nBL > cnBitsPerWord which works out perfectly.
          #endif // !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
          #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We don't support skip links directly to leaves -- yet.
        // Even with defined(PP_IN_LINK).
        // It is sufficient to check the prefix at the switch just
        // above the leaf.
        // pwrPrev is left from the previous iteration of the goto again
        // loop.
        // Would like to combine the source code for this prefix
        // check and the one done in the bitmap section if possible.
        if ( 0
              #if (cnBitsPerWord > 32)
            || (nBL > 32) // leaf has whole key
              #else // (cnBitsPerWord > 32)
            || (nBL > 16) // leaf has whole key
              #endif // (cnBitsPerWord > 32)
          // can't skip nBL check above or we might be at top
          // hmm; check nBL or check at top; which is better?
              #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            // leaf does not have whole key
            // What if there were no skips in the part that is missing?
            || ( ! bNeedPrefixCheck ) // we followed no skip links
              #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            // If we need a prefix check, then we're not at the top.
            // And pwRoot is initialized despite what gcc might think.
              #if defined(SAVE_PREFIX_TEST_RESULT)
            || ( ! bPrefixMismatch )
              #elif defined(SAVE_PREFIX)
                  #if defined(PP_IN_LINK)
            || ((pwRootPrefix == NULL) && (wKey < EXP(nBLRPrefix)))
                  #endif // defined(PP_IN_LINK)
            || (1
                  #if defined(PP_IN_LINK)
                && (pwRootPrefix != NULL)
                  #endif // defined(PP_IN_LINK)
                && ((int)LOG(1
                        | (PWR_wPrefixNATBL(pwRootPrefix,
                                            (Switch_t *)pwrPrefix, nBLRPrefix)
                            ^ wKey))
                    < nBLRPrefix))
              #else // defined(SAVE_PREFIX_TEST_REUSLT)
            || ((int)LOG(1
                    | (PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwrPrev, nBL)
                        ^ wKey))
                < (nBL
                  #if ! defined(PP_IN_LINK)
                    // prefix in parent switch doesn't contain last digit
                    // for ! defined(PP_IN_LINK) case
                    + nDL_to_nBitsIndexSzNAX(nBL_to_nDL(nBL) + 1)
                  #endif // ! defined(PP_IN_LINK)
                ))
              #endif // defined(SAVE_PREFIX_TEST_RESULT)
            )
          #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
      #endif // defined(COMPRESSED_LISTS)
        {

      // LOOKUP_NO_LIST_SEARCH is for analysis only.  We have retrieved the
      // pop count and prefix but we have not dereferenced the list itself.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
        #if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
        #endif // ! defined(SEPARATE_T_NULL)
        #if defined(LOOKUP)
                && ListHasKey(pwr, wKey, nBL, &wRoot)
        #else // defined(LOOKUP)
                && ((nPos = SearchList(pwr, wKey, nBL, &wRoot)) >= 0)
        #endif // defined(LOOKUP)
                )
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
          #if defined(REMOVE)
              #if defined(PP_IN_LINK)
                // Adjust wPopCnt in link to leaf for PP_IN_LINK.
                // wPopCnt in link to switch is adjusted elsewhere,
                // i.e. in the same place as wPopCnt in switch is
                // adjusted for pp-in-switch.
                assert(nIncr == -1);
                set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                                  PWR_wPopCntBL(pwRoot,
                                                (Switch_t *)NULL, nBL) - 1);
              #endif // defined(PP_IN_LINK)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(INSERT)
              #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
              #endif // ! defined(RECURSIVE)
          #endif // defined(INSERT)
          #if ! defined(COUNT)
                return KeyFound;
          #endif // ! defined(COUNT)
            }
          #if defined(COUNT)
            else
          #endif // defined(COUNT)
          #if ! defined(LOOKUP)
            {
                nPos ^= -1;
            }
          #endif // ! defined(LOOKUP)
        }
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) \
          && defined(COMPRESSED_LISTS)
        else
        {
            // Shouldn't this be using the previous nBL for pwrPrev?
            DBGX(printf("Mismatch at list wPrefix "OWx" nBL %d\n",
                 PWR_wPrefixNATBL(pwRoot, pwrPrev, nBL), nBL));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && ...

      #if defined(COUNT)
        DBGC(printf("T_LIST: nPos %d\n", nPos));
        return wPopCntSum + nPos;
      #endif // defined(COUNT)

      #if defined(PP_IN_LINK) && defined(INSERT)
        // Adjust wPopCnt in link to leaf for PP_IN_LINK.
        // wPopCnt in link to switch is adjusted elsewhere,
        // i.e. in the same place as wPopCnt in switch is
        // adjusted for pp-in-switch.
        // pwRoot is initialized despite what gcc might think.
        // Would be nice to be able to get the current pop count from
        // SearchList because chances are it will have read it.
        // But it is more important to avoid getting it when not necessary
        // during lookup.
        assert((nBL == cnBitsPerWord) // there is no link with pop count
            || (pwr != NULL) // non-NULL implies non-zero pop count
            || (PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) == 0));
        assert(nIncr == 1);
        DBGI(printf("did not find key\n"));
        set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                        PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + 1);
      #endif // defined(PP_IN_LINK) && defined(INSERT)

        break;

    } // end of case T_LIST

#endif // (cwListPopCntMax != 0)

#if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP:
        DBGX(printf("T_SKIP_TO_BITMAP\n"));
        // PREFIX_MISMATCH may update nBLR only if there is a match.
        intptr_t nPrefixMismatch = PREFIX_MISMATCH(nBL, nType);
        if (nPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("T_SKIP_TO_BITMAP: COUNT PREFIX_MISMATCH %"_fw"d\n", nPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (nPrefixMismatch > 0) {
                Word_t wPopCnt = w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
                DBGC(printf("T_SKIP_TO_BITMAP: PREFIX_MISMATCH wPopCnt %"_fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_bitmap;
#endif // defined(SKIP_TO_BITMAP)
    case T_BITMAP:
#if defined(EXTRA_TYPES)
    case T_BITMAP | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        goto t_bitmap;
t_bitmap:;
  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

#if defined(PP_IN_LINK) && (defined(INSERT) || defined(REMOVE))
        if (EXP(cnBitsInD1) > sizeof(Link_t) * 8) {
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBLR);
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBLR,
                              wPopCnt + nIncr);
        }
#endif // defined(PP_IN_LINK) && (defined(INSERT) || defined(REMOVE))

#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

  #if defined(SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        // We have to do the prefix check here.
        if ( 0
          #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
            || ! bNeedPrefixCheck
          #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
          #if defined(SAVE_PREFIX)
              #if defined(PP_IN_LINK)
            || ((pwRootPrefix == NULL) && (wKey < EXP(nBLRPrefix)))
              #endif // defined(PP_IN_LINK)
            || (1
              #if defined(PP_IN_LINK)
                && (pwRootPrefix != NULL)
              #endif // defined(PP_IN_LINK)
                && ((int)LOG(1
                        | (PWR_wPrefixNATBL(pwRootPrefix,
                                            (Switch_t *)pwrPrefix, nBLRPrefix)
                            ^ wKey))
                    < nBLRPrefix))
          #else // defined(SAVE_PREFIX)
            // Notice that we're using pwr which was extracted from the
            // previous wRoot -- not the current wRoot -- to find the prefix,
            // if not PP_IN_LINK.  If PP_IN_LINK, then we are using the
            // current pwRoot to find the prefix.
            // nBL is different for the two cases.
            || ((int)LOG(1
                | (PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwrPrev, cnBitsInD1)
                            ^ wKey))
                // The +1 is necessary because the pwrPrev
                // prefix does not contain any less significant bits.
              #if defined(PP_IN_LINK)
                    < nBL
              #else // defined(PP_IN_LINK)
                    < nDL_to_nBL_NAX(nBL_to_nDL(nBLR) + 1)
              #endif // defined(PP_IN_LINK)
                )
          #endif // defined(SAVE_PREFIX)
            )
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
        {
  #if defined(COUNT)
            // Count bits.
            Word_t wCount = 0;
            unsigned nWordOffset = (wKey & MSK(nBLR)) >> cnLogBitsPerWord;
            unsigned nn;
            for (nn = 0; nn < nWordOffset; nn++) {
                wCount += __builtin_popcountll(pwr[nn]);
            }
            Word_t wBit = ((Word_t)1 << (wKey & (cnBitsPerWord - 1)));
            Word_t wBmMask = wBit - 1;
            //Word_t wBmMask = (wBit | wBit - 1);
            wCount += __builtin_popcountll(pwr[nn] & wBmMask);
            DBGC(printf("T_BITMAP: nWordOffset 0x%x wBit "OWx" wBmMask "OWx
                        " wCount %"_fw"d wPopCntSum + wCount %"_fw"d\n",
                        nWordOffset, wBit, wBmMask, wCount,
                        wPopCntSum + wCount));
            return wPopCntSum + wCount;
  #else // defined(COUNT)
  #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            // BUG?: Is pwrPrev valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
              #if defined(PP_IN_LINK)
            assert(PWR_wPopCnt(pwRoot, pwrPrev, cnBitsInD1) != 0);
              #else // defined(PP_IN_LINK)
            assert(PWR_wPopCnt(pwRoot, pwrPrev, cnBitsInD2) != 0);
              #endif // defined(PP_IN_LINK)
            return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
      #if defined(USE_XX_SW)
            // We assume we never blow-out into a bitmap.
            // But we don't really enforce it.
          #if defined(DEBUG)
            if (nBLR != cnBitsLeftAtDl2) { printf("nBLR %d\n", nBLR); }
          #endif // defined(DEBUG)
            assert(nBLR == cnBitsLeftAtDl2);
            assert(pwr == wr_pwr(wRoot));
            int bBitIsSet = BitIsSet(wr_pwr(wRoot),
                                     wKey & MSK(cnBitsLeftAtDl2));
      #else // defined(USE_XX_SW)
            // Might be able to speed this up with bl-specific code.
            // It looks like this code is assuming nBL == cnBitsInD1.
            // I'm not sure why this is ok now that we're installing a
            // bitmap at dl2.
            // I guess it is only assuming nBL == cnBitsInD1 if
            // EXP(cnBitsInD1) <= sizeof(Link_t) * 8.
            assert((nBLR == cnBitsInD1)
                || (EXP(cnBitsInD1) > sizeof(Link_t) * 8));
            int bBitIsSet
                = (cnBitsInD1 <= cnLogBitsPerWord)
                    ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsInD1))
                : (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)
                    ? BitIsSet(STRUCT_OF(pwRoot, Link_t, ln_wRoot),
                               wKey & MSK(cnBitsInD1))
                // Isn't pwr == wr_pwr(wRoot)
                // : BitIsSet(wr_pwr(wRoot), wKey & MSK(nBL));
                : BitIsSet(pwr, wKey & MSK(nBLR));
      #endif // defined(USE_XX_SW)
            if (bBitIsSet)
            {
      #if defined(REMOVE)
                goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT)
          #if !defined(RECURSIVE)
                if (nIncr > 0)
                {
                    DBGX(printf("Bit is set!\n"));
                    goto undo; // undo counting 
                }
          #endif // !defined(RECURSIVE)
      #endif // defined(INSERT)
                return KeyFound;
            }
            DBGX(printf("Bit is not set.\n"));
  #endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
  #endif // defined(COUNT)
        }
  #if defined(SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        else
        {
            // Shouldn't this be using the previous nBL for the pwrPrev case?
          #if defined(SKIP_TO_BITMAP)
            // But now that we have prefix in the bitmap can't we use that?
          #endif // defined(SKIP_TO_BITMAP)
            DBGX(printf("Mismatch at bitmap wPrefix "OWx" nBLR %d nBL %d\n",
                        PWR_wPrefixNATBL(pwRoot, pwrPrev, nBLR), nBLR, nBL));
          #if defined(COUNT)
            return wPopCntSum;
          #endif // defined(COUNT)
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
  #endif // defined(SKIP_LINKS)
#endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(INSERT)
  #if defined(SKIP_TO_BITMAP)
        // I guess we're getting away with leaving nBL
        // as the post-skip value.  Go figure.
  #endif // defined(SKIP_TO_BITMAP)
#endif // defined(INSERT)

#if defined(INSERT)
  #if 0
        // We removed the code that was in InsertGuts to handle bitmaps.
        // So we can no longer just 'break' here like we used to do.
        // We also removed the code to do cleanup after the call to InsertGuts.
        InsertAtBitmap(pwRoot, wKey, nBL_to_nDL(nBL), wRoot);
  #else
        InsertGuts(pwRoot, wKey, nBL, wRoot, nPos
      #if defined(CODE_XX_SW)
                   , pwRootPrev
        #if defined(SKIP_TO_XX_SW)
                   , nBLPrev
        #endif // defined(SKIP_TO_XX_SW)
      #endif // defined(CODE_XX_SW)
                   );
  #endif
  #if (cn2dBmWpkPercent != 0)
        if ((nBLR == cnBitsInD1)
            && (nBL == nBLR)
  #if 0
            && (PWR_wPopCntBL(pwRootPrev, (Switch_t *)wr_pwr(*pwRootPrev),
                              cnBitsLeftAtDl2)
                >= (EXP(cnBitsLeftAtDl2) * 100 / cnBitsPerWord
                       / cn2dBmWpkPercent))
  #endif
            && (wPopCnt >= (EXP(cnBitsLeftAtDl2) * 100 / cnBitsPerWord
                    / cn2dBmWpkPercent))
            )
        {
            goto cleanup;
        }
  #endif // (cn2dBmWpkPercent != 0)
        return Success;
#else // defined(INSERT)
        break;
#endif // defined(INSERT)

    } // end of case T_BITMAP

#if defined(EMBED_KEYS)

    case T_EMBEDDED_KEYS:
#if defined(EXTRA_TYPES)
    case T_EMBEDDED_KEYS | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        goto t_embedded_keys; // suppress compiler unused-label warnings
t_embedded_keys:; // the semi-colon allows for a declaration next; go figure
        assert(EmbeddedListPopCntMax(nBL) != 0);
  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

  #if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord)
        {
            // Adjust pop count in the link on the way in.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
  #endif // ! defined(LOOKUP) && defined(PP_IN_LINK)

  #if defined(COUNT)
        {
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            Word_t wBLM = MSK(nBL); // Bits left mask.

            int nn;
            for (nn = 0; nn < nPopCnt; nn++) {
                int nSlot = (nn + 1);
                Word_t wKeyInSlot = (wKey & ~wBLM)
                               | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM);
                if (wKeyInSlot >= wKey) {
                    break;
                }
            }
            DBGC(printf("EK: wPopCntSum(before) %"_fw"d nn %d\n", wPopCntSum, nn));
            return wPopCntSum + nn;
        }
  #endif // defined(COUNT)

  #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_DEREF)

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        DBGX(printf("EMBEDDED_KEYS\n")); 

      #if    (defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)) \
          || (defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP))

#if defined(HANDLE_BLOWOUTS)
    // We haven't written the insert code to create blow-outs for
    // NO_TYPE_IN_XX_SW yet.
  #if defined(NO_TYPE_IN_XX_SW)
    #define BLOWOUT_CHECK(_nBL) \
         ((wRoot & BLOWOUT_MASK(_nBL)) == ZERO_POP_MAGIC)
  #else // defined(NO_TYPE_IN_XX_SW)
    #define BLOWOUT_CHECK(_nBL)  (wr_nType(wRoot) != T_EMBEDDED_KEYS)
  #endif // defined(NO_TYPE_IN_XX_SW)
#else // defined(HANDLE_BLOWOUTS)
    #define BLOWOUT_CHECK(_nBL) (0)
#endif // defined(HANDLE_BLOWOUTS)

#if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
    #define ZERO_CHECK  (0)
#else // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
  #if defined(NO_TYPE_IN_XX_SW)
    #define ZERO_CHECK  (wRoot == ZERO_POP_MAGIC)
  #else // defined(NO_TYPE_IN_XX_SW)
    #define ZERO_CHECK  (wRoot == 0)
  #endif // defined(NO_TYPE_IN_XX_SW)
#endif // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)

#if defined(NO_TYPE_IN_XX_SW) || defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    #define HANDLE_DL2(_nBL) \
        if ((_nBL) < nDL_to_nBL(2)) { \
            if (ZERO_CHECK) { goto break2; } \
            if (BLOWOUT_CHECK(_nBL)) { goto again; } \
        }
#else // defined(NO_TYPE_IN_XX_SW) || defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    #define HANDLE_DL2(_nBL)
#endif // defined(NO_TYPE_IN_XX_SW) || defined(HANDLE_DL2_IN_EMBEDDED_KEYS)

#define CASE_BLX(_nBL) \
        case (_nBL): \
            HANDLE_DL2(_nBL); \
            if (EmbeddedListHasKey(wRoot, wKey, (_nBL))) { goto foundIt; } \
            goto break2

        switch (nBL) {
        CASE_BLX( 0); CASE_BLX( 1); CASE_BLX( 2); CASE_BLX( 3); CASE_BLX( 4);
        CASE_BLX( 5); CASE_BLX( 6); CASE_BLX( 7); CASE_BLX( 8); CASE_BLX( 9);
        CASE_BLX(10); CASE_BLX(11); CASE_BLX(12); CASE_BLX(13); CASE_BLX(14);
        CASE_BLX(15);
        default: DBG(printf("nBL %d\n", nBL)); assert(0);
                      CASE_BLX(16);
                                    CASE_BLX(17); CASE_BLX(18); CASE_BLX(19);
        CASE_BLX(20); CASE_BLX(21); CASE_BLX(22); CASE_BLX(23); CASE_BLX(24);
        CASE_BLX(25); CASE_BLX(26); CASE_BLX(27); CASE_BLX(28); CASE_BLX(29);
        CASE_BLX(30); CASE_BLX(31); CASE_BLX(32); CASE_BLX(33); CASE_BLX(34);
        CASE_BLX(35); CASE_BLX(36); CASE_BLX(37); CASE_BLX(38); CASE_BLX(39);
        CASE_BLX(40); CASE_BLX(41); CASE_BLX(42); CASE_BLX(43); CASE_BLX(44);
        CASE_BLX(45); CASE_BLX(46); CASE_BLX(47); CASE_BLX(48); CASE_BLX(49);
        CASE_BLX(50); CASE_BLX(51); CASE_BLX(52); CASE_BLX(53); CASE_BLX(54);
        CASE_BLX(55); CASE_BLX(56); CASE_BLX(57); CASE_BLX(58); CASE_BLX(59);
        CASE_BLX(60); CASE_BLX(61); CASE_BLX(62); CASE_BLX(63); CASE_BLX(64);
        }
break2:
            
      #endif // (defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) ... )

      #if (!defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)) \
          || (!defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP))

        int nPopCnt = wr_nPopCnt(wRoot, nBL);

          #if (defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP) \
                  && defined(LOOKUP)) \
              || (defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT) \
                  && !defined(LOOKUP))

        // PSPLIT_SEARCH_BY_KEY expects smallest key in xKeys[0] and
        // largest key in xKeys[nPopCnt-1].
              #if 0
        int nPos = 0;
        if (nBL == 8) {
           PSPLIT_SEARCH_BY_KEY(uint8_t, 8, &wRoot, nPopCnt, wKey, nPos);
        } else if (nBL == 16) {
           PSPLIT_SEARCH_BY_KEY(uint16_t, 16, &wRoot, nPopCnt, wKey, nPos);
        } else if (nBL == 32) {
           PSPLIT_SEARCH_BY_KEY(uint32_t, 32, &wRoot, nPopCnt, wKey, nPos);
        } else {
            goto unrolled;
        }
        if (nPos >= 0) { goto foundIt; }
        break;
unrolled:;
              #endif
          #endif // (defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP) ... )

        Word_t wKeyRoot;

        switch (nPopCnt) {
          #if (cnBitsPerWord == 64)
        case 8: // max for 7-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (8 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 7: // max for 8-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (7 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 6: // max for 9-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (6 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 5: // max for 10 to 11-bit keys and 64 bits;
            wKeyRoot = wRoot >> (cnBitsPerWord - (5 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
          #endif // (cnBitsPerWord == 64)
        case 4: // max for 12 to 14-bit keys and 64 bits; 6 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (4 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 3: // max for 15 to 19-bit keys and 64 bits; 7-9 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (3 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        case 2: // max for 20 to 29-bit keys and 64 bits; 10-14 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (2 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        default: // max for 30 to 60-bit keys and 64 bits; 15-29 for 32
            wKeyRoot = wRoot >> (cnBitsPerWord - (1 * nBL));
            if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
        }

      #endif // (!defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) ... )

        break;

foundIt:;

      #if defined(REMOVE)
        goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT)
          #if !defined(RECURSIVE)
        if (nIncr > 0) { goto undo; } // undo counting
          #endif // !defined(RECURSIVE)
      #endif // defined(INSERT)

  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        return KeyFound;

    } // end of case T_EMBEDDED_KEYS

#endif // defined(EMBED_KEYS)

#if defined(USE_T_ONE)

    case T_ONE:
#if defined(EXTRA_TYPES)
    case T_ONE | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
#if defined(EMBED_KEYS)
        assert(nBL
            > cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL));
#else // defined(EMBED_KEYS)
        assert(nBL > cnBitsPerWord - cnBitsMallocMask);
#endif // defined(EMBED_KEYS)

  #if ! defined(LOOKUP)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // ! defined(LOOKUP)

  #if defined(COUNT)
        return wPopCntSum;
  #endif // defined(COUNT)

  #if ! defined(LOOKUP) && defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord)
        {
            // Adjust pop count in the link on the way in.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
  #endif // ! defined(LOOKUP) && defined(PP_IN_LINK)

  #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)
        return KeyFound;
  #else // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        if (*pwr == wKey)
        {
      #if defined(REMOVE)
            goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT)
          #if !defined(RECURSIVE)
            if (nIncr > 0) { goto undo; } // undo counting
          #endif // !defined(RECURSIVE)
      #endif // defined(INSERT)
            return KeyFound;
        }

  #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_DEREF)

        break;

    } // end of case T_ONE

#endif // defined(USE_T_ONE)

#if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)

    case T_NULL:
#if defined(EXTRA_TYPES)
    case T_NULL | EXP(cnBitsMallocMask):
#endif // defined(EXTRA_TYPES)
    {
        assert(wRoot == 0);

      #if ! defined(LOOKUP)
        if ( bCleanup ) { return Success; }
      #endif // ! defined(LOOKUP)

  // Adjust wPopCnt in link to leaf for PP_IN_LINK.
  // wPopCnt in link to switch is adjusted elsewhere, i.e. in the same place
  // as wPopCnt in switch is adjusted for pp-in-switch.
  #if defined(PP_IN_LINK)
      #if ! defined(LOOKUP)
        // What about defined(RECURSIVE)?
        if (nBL != cnBitsPerWord)
        {
            // If nBL != cnBitsPerWord then we're not at top.
            // And pwRoot is initialized despite what gcc might think.
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL,
                PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) + nIncr);
        }
      #endif // ! defined(LOOKUP)
  #endif // defined(PP_IN_LINK)

        break;

    } // end of case T_NULL

#endif // defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)

    } // end of switch

#if defined(BM_SW_FOR_REAL)
notFound:; // why don't we just "break;" above?
#endif // defined(BM_SW_FOR_REAL)
#if defined(COUNT)
    return wPopCntSum;
#endif // defined(COUNT)
#if defined(INSERT)
  #if defined(BM_IN_LINK)
    // If InsertGuts calls Insert, then it is always with the same
    // pwRoot and nBL that Insert passed to InsertGuts.
      #if !defined(RECURSIVE)
    assert((nBL != cnBitsPerWord) || (pwRoot == pwRootOrig));
      #endif // !defined(RECURSIVE)
  #endif // defined(BM_IN_LINK)
    // InsertGuts is called with a pwRoot and nBL indicates the
    // bits that were not decoded in identifying pwRoot.  nBL
    // does not include any skip indicated in the type field of *pwRoot.
    InsertGuts(pwRoot, wKey, nBL, wRoot, nPos
  #if defined(CODE_XX_SW)
               , pwRootPrev
      #if defined(SKIP_TO_XX_SW)
               , nBLPrev
      #endif // defined(SKIP_TO_XX_SW)
  #endif // defined(CODE_XX_SW)
               );
    // Skip cleanup. Rely on T_BITMAP for that.
    return Success;
undo:;
    DBGX(printf("undo\n"));
#endif // defined(INSERT)
#if defined(REMOVE)
  #if !defined(RECURSIVE)
    if (nIncr < 0)
  #endif // !defined(RECURSIVE)
#endif // defined(REMOVE)
  #if defined(INSERT) || defined(REMOVE)
      #if !defined(RECURSIVE)
    {
        // Undo the counting we did on the way in.
        nIncr *= -1;
restart:;
        pwRoot = pwRootOrig;
        nBL = nBLOrig;
        goto top;
    }
      #endif // !defined(RECURSIVE)
  #endif // defined(INSERT) || defined(REMOVE)
    return Failure;
  #if defined(INSERT) || defined(REMOVE)
      #if defined(REMOVE)
removeGutsAndCleanup:;
    RemoveGuts(pwRoot, wKey, nBL, wRoot);
      #else // defined(REMOVE)
          #if (cn2dBmWpkPercent != 0)
cleanup:;
          #endif // (cn2dBmWpkPercent != 0)
      #endif // defined(REMOVE)
    // Walk the tree again to see if we need to make any adjustments.
    // For insert our new pop may justify a bigger bitmap.
    // For remove we may need to pull back.
    bCleanup = 1; // ?? nIncr == 0 ??
    DBGX(printf("Cleanup pwRO "OWx" nBLO %d\n",
                (Word_t)pwRootOrig, nBLOrig));
    // Tree may not be valid yet.
    // It may still have a non-NULL pointer to a switch that has
    // been emptied.
    // Dump may blow an assertion.
    // DBGX(Dump(pwRootOrig, /* wPrefix */ (Word_t)0, nBLOrig));
    goto restart;
  #endif // defined(INSERT) || defined(REMOVE)
}

#undef RECURSIVE
#undef InsertGuts
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

#endif // (cnDigitsPerWord <= 1)

#if defined(LOOKUP)

int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
{
#if (cnDigitsPerWord > 1)

    Word_t wRoot = (Word_t)pcvRoot;

  #if (cwListPopCntMax != 0)
      #if defined(SEARCH_FROM_WRAPPER)
    // Handle a top level T_LIST leaf here -- without calling Lookup.
    // For PP_IN_LINK a T_LIST leaf at the top has a pop count field in
    // the list, but T_LIST leaves that are not at the top do not. And,
    // for PP_IN_LINK there is no complete Link_t at the top -- only wRoot.
    // SEARCH_FROM_WRAPPER allows us avoid making the mainline PP_IN_LINK
    // T_LIST leaf handling code have to know or test if it is at the top.
    // The list search function we use here can't assume the list is sorted
    // if we're not sorting lists on insert.
    // Is T_LIST the only node type that is different at the top for
    // PP_IN_LINK? Doesn't the incomplete Link_t complicate Lookup for
    // the other node types?
    int nType = Get_nType(&wRoot);
    if (nType == T_LIST)
    {
        Word_t *pwr = wr_pwr(wRoot);

        // ls_wPopCount is valid only at the top for PP_IN_LINK
        // the first word in the list is used for pop count at the top
        return (1
#if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
#endif // ! defined(SEPARATE_T_NULL)
                && (SearchListWord(ls_pwKeys(pwr, cnBitsPerWord),
                                   wKey, cnBitsPerWord,
                                   PWR_xListPopCnt(&wRoot, pwr, cnBitsPerWord))
                    >= 0))
            ? Success : Failure;
    }
      #endif // defined(SEARCH_FROM_WRAPPER)
  #endif // (cwListPopCntMax != 0)

    return Lookup(
  #if defined(PWROOT_ARG_FOR_LOOKUP)
                  &wRoot,
  #else // defined(PWROOT_ARG_FOR_LOOKUP)
                  wRoot,
  #endif // defined(PWROOT_ARG_FOR_LOOKUP)
                  wKey
                  );

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    DBGL(printf("\nJudy1Test(pcvRoot %p)\n", (void *)pcvRoot));

    if (pcvRoot == NULL)
    {
        return Failure;
    }

  #if defined(BITMAP_BY_BYTE) // vs. bitmap by word

    Word_t wByteNum = BitmapByteNum(wKey);
    Word_t wByteMask = BitmapByteMask(wKey);     

    DBGL(printf("Judy1Test num "OWx" mask "OWx"\n", wByteNum, wByteMask));
    DBGL(printf("val %x\n", (int)(((char *)pcvRoot)[wByteNum] & wByteMask)));

      #if defined(LOOKUP_NO_BITMAP_DEREF)
        return Success;
      #else // defined(LOOKUP_NO_BITMAP_DEREF)
        return (((char *)pcvRoot)[wByteNum] & wByteMask) ? Success : Failure;
      #endif // defined(LOOKUP_NO_BITMAP_DEREF)

  #else // defined(BITMAP_BY_BYTE)
    // bitmap by word

    Word_t wWordNum = BitmapWordNum(wKey);
    Word_t wWordMask = BitmapWordMask(wKey);     

    DBGL(printf("Judy1Test num "OWx" mask "OWx"\n", wWordNum, wWordMask));
    DBGL(printf("val %x\n",
        (int)(((Word_t *)pcvRoot)[wWordNum] & wWordMask)));

      #if defined(LOOKUP_NO_BITMAP_DEREF)
        return Success;
      #else // defined(LOOKUP_NO_BITMAP_DEREF)
        return (((Word_t *)pcvRoot)[wWordNum] & wWordMask)
            ? Success : Failure;
      #endif // defined(LOOKUP_NO_BITMAP_DEREF)

  #endif // defined(BITMAP_BY_BYTE)

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(LOOKUP)

#if defined(INSERT)

int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
{
    Word_t *pwRoot = (Word_t *)ppvRoot;

#if (cnDigitsPerWord > 1)

    int status;

    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if defined(DEBUG)

    pwRootLast = pwRoot;

  #endif // defined(DEBUG)

  #if (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_I)
    // Handle the top level list leaf before calling Insert.  Why?
    // To simplify Insert for PP_IN_LINK.  Does it still apply?
    // Do not assume the list is sorted.
    // But if we do the insert here, and the list is sorted, then leave it
    // sorted -- so we don't have to worry about ifdefs in this code.
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);

    if (((T_LIST == nType)
#if ! defined(SEPARATE_T_NULL)
            && (wr_pwr(wRoot) != NULL)
#endif // ! defined(SEPARATE_T_NULL)
            && 1)
      #if defined(USE_T_ONE)
        || (nType == T_ONE)
      #endif // defined(USE_T_ONE)
      #if defined(SEPARATE_T_NULL)
        || (nType == T_NULL)
      #endif // defined(SEPARATE_T_NULL)
        )
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Success)
        {
            status = Failure;
        }
        else
        {
            Word_t *pwr = wr_pwr(wRoot);
            Word_t wPopCnt;

      #if defined(USE_T_ONE)
            if (nType == T_ONE) {
                wPopCnt = 1;
            } else
      #endif // defined(USE_T_ONE)
      #if defined(SEPARATE_T_NULL)
            if (nType == T_NULL) {
                assert(pwr == NULL);
                wPopCnt = 0;
            } else
      #endif // defined(SEPARATE_T_NULL)
            {
                wPopCnt = PWR_xListPopCnt(pwRoot, pwr, cnBitsPerWord);
            }

#if (cnBitsPerWord == 64)
            if (wPopCnt == cnListPopCntMax64)
#else // (cnBitsPerWord == 64)
            if (wPopCnt == cnListPopCntMax32)
#endif // (cnBitsPerWord == 64)
            {
                status = InsertGuts(pwRoot, wKey, cnBitsPerWord, wRoot, -1
#if defined(CODE_XX_SW)
                                  , NULL
  #if defined(SKIP_TO_XX_SW)
                                  , 0
  #endif // defined(SKIP_TO_XX_SW)
                                    );
#endif // defined(CODE_XX_SW)
            }
            else
            {
                Word_t *pwListNew = NewList(wPopCnt + 1, cnDigitsPerWord);
                Word_t *pwKeysNew = ls_pwKeys(pwListNew, cnBitsPerWord);
                set_wr(wRoot, pwListNew, T_LIST);
                Word_t *pwKeys;
      #if defined(USE_T_ONE)
                if (nType == T_ONE) {
                    pwKeys = pwr;
                } else
      #endif // defined(USE_T_ONE)
                { pwKeys = ls_pwKeys(pwr, cnBitsPerWord); }

 // Isn't this chunk of code already in InsertGuts?
                unsigned nn;
                for (nn = 0; (nn < wPopCnt) && (pwKeys[nn] < wKey); nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                pwKeysNew[nn] = wKey;
                COPY(&pwKeysNew[nn + 1], &pwKeys[nn], wPopCnt - nn);
      #if defined(LIST_END_MARKERS)
                // pwKeysNew incorporates top pop count and markers
                pwKeysNew[wPopCnt + 1] = -1;
      #endif // defined(LIST_END_MARKERS)

                OldList(pwr, wPopCnt, cnBitsPerWord, nType);
                *pwRoot = wRoot;

                status = Success;
            }
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_I)
    {
        status = Insert(pwRoot, wKey, cnBitsPerWord);
    }

    if (status == Success) {
        // count successful inserts minus successful removes
        wPopCntTotal++;
  #if defined(DEBUG)
        if (!bHitDebugThreshold && (wPopCntTotal > cwDebugThreshold)) {
            bHitDebugThreshold = 1;
            if (cwDebugThreshold != 0) {
                printf("\nHit debug threshold.\n");
            }
        }
  #endif // defined(DEBUG)
    }

    DBGI(printf("\n# After Insert(wKey "OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGI(printf("\n"));

  #if defined(DEBUG_COUNT) || ! defined(PP_IN_LINK)
    // Judy1Count really slows down testing for PP_IN_LINK.
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
  #endif // defined(DEBUG_COUNT) || ! defined(PP_IN_LINK)

    return status;

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wRoot = *pwRoot;
    Word_t wByteNum, wByteMask;
    char c;

    DBGI(printf("\nJudy1Set(ppvRoot %p wKey "OWx") wRoot "OWx"\n",
        (void *)ppvRoot, wKey, wRoot));

    if (wRoot == 0)
    {
        wRoot = JudyMalloc(EXP(cnBitsPerWord - cnLogBitsPerWord));
        assert(wRoot != 0);
        assert((wRoot & cnMallocMask) == 0);

        DBGI(printf("Malloc wRoot "OWx"\n", wRoot));

        *ppvRoot = (PPvoid_t)wRoot;
    }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);     

    DBGI(printf("Judy1Set num "OWx" mask "OWx"\n", wByteNum, wByteMask));

    if ((c = ((char *)wRoot)[wByteNum]) & wByteMask)
    {
        return Failure; // dup
    }

    ((char *)wRoot)[wByteNum] = c | wByteMask;

    return Success;

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warning
}

#endif // defined(INSERT)

#if defined(REMOVE)

int
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, P_JE)
{
    Word_t *pwRoot = (Word_t *)ppvRoot;

#if (cnDigitsPerWord > 1)

    int status;

    DBGR(printf("\n\n# Judy1Unset ppvRoot %p wKey "OWx"\n",
                (void *)ppvRoot, wKey));

  #if (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_R)
    // Handle the top level list leaf.
    // Do not assume the list is sorted, but maintain the current order so
    // we don't have to bother with ifdefs in this code.
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);
    if (nType == T_LIST)
    {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Failure)
        {
            status = Failure;
        }
        else
        {
            Word_t *pwr = wr_pwr(wRoot);
            Word_t wPopCnt = PWR_xListPopCnt(pwRoot, pwr, cnBitsPerWord);
            Word_t *pwListNew;
            if (wPopCnt != 1)
            {
                pwListNew = NewList(wPopCnt - 1, cnDigitsPerWord);
                Word_t *pwKeysNew;
      #if defined(USE_T_ONE)
                if (wPopCnt == 2) {
                    set_wr(wRoot, pwListNew, T_ONE);
                    pwKeysNew = pwListNew;
                } else
      #endif // defined(USE_T_ONE)
                {
                    set_wr(wRoot, pwListNew, T_LIST);
                    pwKeysNew = ls_pwKeys(pwListNew, cnBitsPerWord);
                }

                Word_t *pwKeys = ls_pwKeys(pwr, cnBitsPerWord);

 // Isn't this chunk of code already in RemoveGuts?
                unsigned nn;
                for (nn = 0; pwKeys[nn] != wKey; nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                COPY(&pwKeysNew[nn], &pwKeys[nn + 1], wPopCnt - nn - 1);
      #if defined(LIST_END_MARKERS)
          #if defined(USE_T_ONE)
                if (wPopCnt != 2)
          #endif // defined(USE_T_ONE)
                // pwKeysNew incorporates top pop count and markers
                { pwKeysNew[wPopCnt - 1] = -1; }
      #endif // defined(LIST_END_MARKERS)
            }
            else
            {
                wRoot = 0; // set_wr(wRoot, NULL, 0)
            }
            OldList(pwr, wPopCnt, cnBitsPerWord, nType);
            *pwRoot = wRoot;
            status = Success;
        }
    }
    else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_R)
    {
        status = Remove(pwRoot, wKey, cnBitsPerWord);
    }

    if (status == Success) { wPopCntTotal--; }

  #if defined(DEBUG_REMOVE)
    DBGR(printf("\n# After Remove(wKey "OWx") %s Dump\n", wKey,
            status == Success ? "Success" : "Failure"));
    DBGR(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGR(printf("\n"));
  #endif // defined(DEBUG_REMOVE)

  #if defined(DEBUG_COUNT) || ! defined(PP_IN_LINK)
    // Judy1Count really slows down testing for PP_IN_LINK.
    assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
  #endif // defined(DEBUG_COUNT) || ! defined(PP_IN_LINK)

    return status;

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wRoot = *pwRoot;
    Word_t wByteNum, wByteMask;
    char c;

    if (wRoot == 0) { return Failure; }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);     

    if ( ! ((c = ((char *)wRoot)[wByteNum]) & wByteMask) )
    {
        return Failure; // not present
    }

    ((char *)wRoot)[wByteNum] = c & ~wByteMask;

    return Success;

#endif // (cnDigitsPerWord > 1)

    (void)PJError; // suppress "unused parameter" compiler warnings
}

#endif // defined(REMOVE)

