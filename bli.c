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

  #if defined(COUNT)
// Return the number of keys in the subtrees rooted by links that
// precede the specified link in this switch.
static Word_t
CountSw(qp,
        int nBLR, // number of bits left to decode by switch and below
        int nBW, // number of bits decoded by the switch
        Word_t wIndex, // index of relevant link in switch
        int nLinks)
{
    qv; (void)nBLR; (void)nLinks;
    DBGC(printf("\nCountSw nType %d nBLR %d nBW %d wIndex " OWx"\n",
                nType, nBLR, nBW, wIndex));
    Word_t wPopCnt = 0;
    Word_t ww, wwLimit;
    if ((wIndex > (unsigned)nLinks / 2)
          #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // We don't have a whole link with a pop count at the top.
            && (nBL < cnBitsPerWord)
          #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        )
    {
        ww = wIndex; wwLimit = nLinks;
    } else {
        ww = 0; wwLimit = wIndex;
    }
    DBGC(printf("ww " OWx" wwLimit " OWx"\n", ww, wwLimit));
    Link_t *pLinks =
#if defined(CODE_BM_SW)
        tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
#if defined(CODE_LIST_SW)
        tp_bIsListSw(nType) ? gpListSwLinks(qy) :
#endif // defined(CODE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr);
    int nBLLoop = nBLR - nBW;
    for (; ww < wwLimit; ++ww) {
        Link_t *pLnLoop = &pLinks[ww];
        Word_t wRootLoop = pLnLoop->ln_wRoot;
        Word_t wPopCntLoop;
        DBGC(printf("ww " OWx" wRootLoop " Owx"\n", ww, wRootLoop));
      #if defined(ALLOW_EMBEDDED_BITMAP)
        if (nBLLoop <= (int)LOG(sizeof(Link_t) * 8)) {
            assert(nBLLoop <= cnLogBitsPerWord); // for now
            wPopCntLoop
                = __builtin_popcountll(wRootLoop
                    & ((nBLLoop >= cnLogBitsPerWord)
                        ? (Word_t)-1 : MSK(EXP(nBLLoop))));
            wPopCnt += wPopCntLoop;
            DBGC(printf("embedded bitmap wPopCntLoop " OWx" wPopCnt " OWx"\n",
                        wPopCntLoop, wPopCnt));
        } else
      #elif defined(BITMAP) // defined(ALLOW_EMBEDDED_BITMAP
        assert(nBLLoop > (int)LOG(sizeof(Link_t) * 8));
      #endif // defined(ALLOW_EMBEDDED_BITMAP) elif defined(BITMAP)
        {
            Word_t *pwrLoop = wr_pwr(wRootLoop);
            int nTypeLoop = wr_nType(wRootLoop);
            DBGC(printf("pwrLoop %p nTypeLoop %d\n",
                        (void *)pwrLoop, nTypeLoop));
            int nBLRLoop = gnBLR(qyLoop);
            if (tp_bIsSwitch(nTypeLoop)) {
                wPopCntLoop = gwPopCnt(qyLoop, nBLRLoop);
                if (wPopCntLoop == 0) {
                    wPopCntLoop = EXP(nBLRLoop);
                }
                DBGC(printf("ww %" _fw"d bIsSwitch pwr %p wPopCnt %" _fw"d\n",
                            ww, (void *)pwrLoop, wPopCntLoop));
                wPopCnt += wPopCntLoop;
                DBGC(printf("switch wPopCntLoop " OWx" wPopCnt " OWx"\n",
                            wPopCntLoop, wPopCnt));
            } else switch (nTypeLoop) {
          #if defined(EMBED_KEYS)
            case T_EMBEDDED_KEYS:
                wPopCntLoop = wr_nPopCnt(wRootLoop, nBLLoop);
                assert(wPopCntLoop != 0);
                DBGC(printf("ww %" _fw"d T_EMBED_KEYS wRoot " OWx
                            " wPopCnt %" _fw"d\n",
                            ww, wRootLoop, wPopCntLoop));
                wPopCnt += wPopCntLoop;
                DBGC(printf("embedded keys wPopCntLoop " OWx
                            " wPopCnt " OWx"\n", wPopCntLoop, wPopCnt));
                break;
          #endif // defined(EMBED_KEYS)
            case T_LIST:
          #if defined(UA_PARALLEL_128)
            case T_LIST_UA:
          #endif // defined(UA_PARALLEL_128)
                wPopCntLoop = 0;
          #if ! defined(SEPARATE_T_NULL)
                if (pwrLoop != NULL)
          #endif // ! defined(SEPARATE_T_NULL)
                {
                    wPopCntLoop = gnListPopCnt(qyLoop, nBLRLoop);
                    assert(wPopCntLoop != 0);
                    DBGC(printf("ww %" _fw"d T_LIST pwr %p wPopCnt %" _fw"d\n",
                                ww, (void *)pwrLoop, wPopCntLoop));
                    wPopCnt += wPopCntLoop;
                }
                DBGC(printf("list wPopCntLoop " OWx" wPopCnt " OWx"\n",
                     wPopCntLoop, wPopCnt));
                break;
          #ifdef BITMAP
              #if defined(SKIP_TO_BITMAP)
            case T_SKIP_TO_BITMAP:
                  #if defined(PP_IN_LINK)
                // From where should we get pop count for PP_IN_LINK?
                // It exists in the bitmap but also in the link.
                // But there is no link at the top. KISS.
                  #endif // defined(PP_IN_LINK)
              #endif // defined(SKIP_TO_BITMAP)
            case T_BITMAP:
                wPopCntLoop
                    = w_wPopCntBL(*(pwrLoop + EXP(nBLRLoop - cnLogBitsPerWord)),
                                  nBLRLoop);
                if (wPopCntLoop == 0) {
                    wPopCntLoop = EXP(nBLRLoop);
                }
                DBGC(printf("ww %" _fw"d T_BITMAp pwr %p wPopCnt %" _fw"d\n",
                            ww, (void *)pwrLoop, wPopCntLoop));
                wPopCnt += wPopCntLoop;
                DBGC(printf("bitmap wPopCntLoop " OWx" wPopCnt " OWx"\n",
                            wPopCntLoop, wPopCnt));
                break;
          #endif // BITMAP
            default:
                DBG(printf("\nww %" _fw"d wRootLoop " OWx" nTypeLoop %d\n",
                       ww, wRootLoop, nTypeLoop));
                if (wPopCntTotal < 0x1000) {
                    DBGC(Dump(pwRootLast, 0, cnBitsPerWord));
                }
                assert(0);
            }
        }
    }
    assert(((int)wIndex < nLinks)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
               || (nBL == cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
           );
    if ((ww == (unsigned)nLinks)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // It wouldn't hurt to include this test even when there is no pop in
            // link, but it's not necessary in that case. We're being pedantic.
            && ((int)wIndex != nLinks)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        )
    {
        Word_t wPopCntSw = gwPopCnt(qy, nBLR);
        if (wPopCntSw == 0) {
            wPopCntSw = EXP(nBLR);
        }
        wPopCnt = wPopCntSw - wPopCnt;
        DBGC(printf("wPopCntSw " OWx"\n", wPopCntSw));
    }
    DBGC(printf("\nCountSw wPopCnt %" _fw"u\n", wPopCnt));
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
static Word_t
PrefixMismatch(qp,
               Word_t wKey,
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
               Word_t *pwPrefixMismatch,
  #endif // defined(SAVE_PREFIX)
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
               int *pnBLR)
{
    qv; (void)wKey; (void)pnBLR;
    DBGX(printf("PM: nBL %d pLn %p wRoot " OWx" nType %d pwr %p\n", qyp));
  #if defined(CODE_BM_SW)
    (void)bBmSw;
  #endif // defined(CODE_BM_SW)

    Word_t wPrefixMismatch; (void)wPrefixMismatch;
    int nBLR = gnBLRSkip(qy);
    assert(nBLR < nBL); // reserved
    *pnBLR = nBLR;

  #if ! defined(LOOKUP) || ! defined(SKIP_PREFIX_CHECK) || defined(SAVE_PREFIX_TEST_RESULT)
    Word_t wPrefix =
        0 ? 0
      #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
        : (nBL == cnBitsPerWord) ? 0
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
      #if defined(SKIP_TO_BITMAP)
          #if defined(PP_IN_LINK)
#error SKIP_TO_BITMAP and PP_IN_LINK is not coded yet
          #else // defined(PP_IN_LINK)
        : (wr_nType(pLn->ln_wRoot) == T_SKIP_TO_BITMAP)
            ?  w_wPrefixBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR)
          #endif // defined(PP_IN_LINK)
      #endif // defined(SKIP_TO_BITMAP)
      #if defined(CODE_BM_SW)
        : bBmSw ? PWR_wPrefixNATBL(pwRoot, (BmSwitch_t *)pwr, nBLR)
      #endif // defined(CODE_BM_SW)
        :         PWR_wPrefixNATBL(pwRoot, (  Switch_t *)pwr, nBLR);

    wPrefixMismatch = (wKey - wPrefix) >> nBLR; // positive means key is big

    if (wPrefixMismatch != 0) {
        DBGX(printf("PM: wKey " OWx" wPrefix " OWx" nBL %d nBLR %d\n",
                    wKey, wPrefix, nBL, nBLR));
    }
      #if ! defined(LOOKUP) || ! defined(SKIP_PREFIX_CHECK)
    return wPrefixMismatch;
      #endif // ! defined(LOOKUP) || ! defined(SKIP_PREFIX_CHECK)
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
    *pwPrefixMismatch = wPrefixMismatch;
      #endif // defined(SAVE_PREFIX)
    //*pwPrefixMismatch = wPrefixMismatch; ???
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    // Record that there were prefix bits that were not checked.
    *pbNeedPrefixCheck |= 1;
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    return 0;
  #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
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
    #define SAVE_PREFIX_ARGS  &wPrefixMismatch,
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

// PREFIX_MISMATCH updates nBLR.
// PrefixMismatch requires a real pwRoot (as opposed to &wRoot) when it
// may need to save that value for later dereference by Lookup at the leaf.
#define PREFIX_MISMATCH(qp) \
    ( assert(tp_bIsSkip(nType)), \
      PrefixMismatch(qy, wKey, \
                     IS_BM_SW_ARG(nType) \
                     LOOKUP_SKIP_PREFIX_CHECK_ARGS \
                     &nBLR) )

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

// Handle bigger picture tree cleanup.
// E.g.
// - uncompress bm sw after insert
// - create 2-digit bitmap leaf after insert
// - compress switch after remove
// - break up 2-digit bitmap leaf after remove
static inline int
SwCleanup(qp, Word_t wKey, int nBLR, int bCleanup)
{
    qv; (void)wKey; (void)nBLR; (void)bCleanup;
  #if defined(INSERT) || defined(REMOVE)
    // Cleanup is for adjusting tree after successful insert or remove.
    // It is not for undoing counts after unsuccessful insert or remove.
    if (bCleanup) {
      #if defined(INSERT)
        if (0
          #if defined(CODE_BM_SW)
            || (tp_bIsBmSw(Get_nType(&wRoot))
                // bits of keys * convert > bits per switch * retain
                && (gwPopCnt(qy, nBLR) * nBLR * EXP(cnBmSwConvert)
                    > EXP(gnBW(qy, T_BM_SW, nBLR)) * 8
                        * sizeof(Link_t) * cnBmSwRetain))
          #endif // defined(CODE_BM_SW)
            || ((cn2dBmMaxWpkPercent != 0)
                && (nBLR == cnBitsLeftAtDl2)
                && (gwPopCnt(qy, nBLR) * cn2dBmMaxWpkPercent
                     > EXP(cnBitsLeftAtDl2 - cnLogBitsPerWord) * 100))
            )
        {
            InsertCleanup(qy, wKey);
        }
      #else // defined(INSERT)
        RemoveCleanup(wKey, nBL, nBLR, pwRoot, wRoot);
      #endif // defined(INSERT)
        if (pLn->ln_wRoot != wRoot) { return 1; /* not done; goto restart */ }
    }
  #endif // defined(INSERT) || defined(REMOVE)
    return 0;
}

// Adjust pop count.
// Increment for insert on the way down.
// Decrement for remove on the way down.
// Also used to undo itself after we discover insert or remove is redundant.
static inline Word_t
SwIncr(qp, int nBLR, int bCleanup, int nIncr)
{
    qv; (void)nBLR; (void)bCleanup; (void)nIncr;
    Word_t wPopCnt = 0;
  #if defined(INSERT) || defined(REMOVE)
    if (!bCleanup) {
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        if (nBL < cnBitsPerWord)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        {
            wPopCnt = gwPopCnt(qy, nBLR) + nIncr;
            // Increment or decrement population count on the way in.
            swPopCnt(qy, nBLR, wPopCnt);
        }
    }
  #endif // defined(INSERT) || defined(REMOVE)
    return wPopCnt;
}

static inline void
SwAdvance(pqp, Link_t *pLnNew, int nBW, int *pnBLR)
{
    pqv;

    *pnBL = *pnBLR - nBW;
    *ppLn = pLnNew;
    // Be very careful with pwRoot from pqp.
    // It might not mean what you think it means.
    *pwRoot = pLnNew->ln_wRoot;
    // Why aren't we updating *pnType and *ppwr here?
    *pnBLR = *pnBL;
    DBGX(printf("sw nBL %d pLn %p wRoot 0x%zx\n",
                *pnBL, (void*)*ppLn, *pwRoot));
}

#ifdef SKIP_PREFIX_CHECK
#ifdef LOOKUP
static inline Status_t
PrefixCheckAtLeaf(qp, Word_t wKey
  #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , int bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF
  #ifdef SAVE_PREFIX_TEST_RESULT
                , Word_t wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
                , Word_t *pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef SAVE_PREFIX
                , Link_t *pLnPrefix, Word_t *pwrPrefix, int nBLRPrefix
  #endif // SAVE_PREFIX
                  )
{
    qv; (void)wKey;
    if (!tp_bIsBitmap(nType)) {
        if (nBL > cnBitsPerWord / 2) {
            return Success; // list leaf has whole word key
        }
    }
    if (0
      #if ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
        || !bNeedPrefixCheck // we followed no skip links
      #endif // ! defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
      #if defined(SAVE_PREFIX_TEST_RESULT)
        || (wPrefixMismatch != 0)
      #elif defined(SAVE_PREFIX)
          #if defined(PP_IN_LINK)
        || ((nBL >= cnBitsPerWord) && (wKey < EXP(nBLRPrefix)))
          #endif // defined(PP_IN_LINK)
        || (1
          #if defined(PP_IN_LINK)
            && (nBL < cnBitsPerWord)
          #endif // defined(PP_IN_LINK)
            && ((int)LOG(1
                        | (PWR_wPrefixNATBL(&pLnPrefix->ln_wRoot,
                                            (Switch_t *)pwrPrefix, nBLRPrefix)
                            ^ wKey))
                    < nBLRPrefix))
      #else // defined(SAVE_PREFIX_TEST_REUSLT)
        || ( (int)LOG(1
                | (PWR_wPrefixNATBL(pwRoot, (Switch_t *)pwrUp, nBL)
                    ^ wKey))
          #if defined(PP_IN_LINK)
            < nBL
          #else // defined(PP_IN_LINK)
            // The +1 is necessary because the pwrUp
            // prefix does not contain any less significant bits.
            < nDL_to_nBL_NAX(nBL_to_nDL(nBL) + 1)
          #endif // defined(PP_IN_LINK)
                )
      #endif // defined(SAVE_PREFIX_TEST_RESULT)
        )
    {
        return Success;
    }
    return Failure;
}
#endif // LOOKUP
#endif // SKIP_PREFIX_CHECK

#if defined(LOOKUP)
#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static Status_t
#endif // B_JUDYL
Lookup(
      #if defined(PLN_PARAM_FOR_LOOKUP)
       Link_t *pLn;
      #else // defined(PLN_PARAM_FOR_LOOKUP)
       Word_t wRoot,
      #endif // defined(PLN_PARAM_FOR_LOOKUP)
       Word_t wKey
       )
#else // defined(LOOKUP)
  #if defined(COUNT)
Word_t
  #elif defined(INSERT) && defined(B_JUDYL)
Word_t *
  #else // defined(COUNT) elif defined(INSERT) && defined(JUDYL)
Status_t
  #endif // defined(COUNT)
  #ifdef B_JUDYL
InsertRemoveL(int nBL, Link_t *pLn, Word_t wKey)
  #else // B_JUDYL
InsertRemove1(int nBL, Link_t *pLn, Word_t wKey)
  #endif // B_JUDYL
#endif // defined(LOOKUP)
{
#if defined(LOOKUP) && ! defined(PLN_PARAM_FOR_LOOKUP)
    Link_t *pLn = STRUCT_OF(&wRoot, Link_t, ln_wRoot);
#else // defined(LOOKUP) && ! defined(PLN_PARAM_FOR_LOOKUP)
    Word_t wRoot = pLn->ln_wRoot;
#endif // defined(LOOKUP) && ! defined(PLN_PARAM_FOR_LOOKUP)
    DBGX(printf("\n# %s pLn %p wRoot 0x%zx wKey 0x%zx ",
                strLookupOrInsertOrRemove, (void*)pLn, wRoot, wKey));

#if defined(LOOKUP)
    int nBL = cnBitsPerWord;
#endif // defined(LOOKUP)

    DBGX(printf("# nBL %d pLn %p wRoot " OWx"\n", nBL, pLn, wRoot));

#if defined(INSERT) && defined(B_JUDYL)
    Word_t *pwValue;
#endif // defined(INSERT) && defined(B_JUDYL)

#if !defined(RECURSIVE)
    Link_t *pLnOrig = pLn; (void)pLnOrig;
  #if !defined(LOOKUP)
    int nBLOrig = nBL; (void)nBLOrig;
  #endif // !defined(LOOKUP)
#endif // !defined(RECURSIVE)

    int nBW;
    Link_t *pLnNew;

    // wDigit needs this broad scope only for COUNT.
    // I wonder if it would help performance if we were to define this
    // in the narrower scope for LOOKUP?
    // In the one quick test I did I saw no performance difference.
    // Maybe we should revisit occasionally.
    Word_t wDigit;

    int nIncr; (void)nIncr;
#if !defined(RECURSIVE)
      #if defined(INSERT)
    nIncr = 1;
      #elif defined(REMOVE)
    nIncr = -1;
      #else // INSERT, REMOVE
    nIncr = 0; // make gcc happy
      #endif // INSERT, REMOVE
#endif // !defined(RECURSIVE)

    // InsertGuts checks pLnUp != NULL to id XX_SW link.
    // We needed a way to handle NO_TYPE_IN_XX_SW.
    // Should we limit its existence to that case?
    // Do we ever depend on this initialization of pLnUp?
    Link_t *pLnUp = NULL; (void)pLnUp;

    // nBLUp is used only for SKIP_TO_XX_SW and INSERT.
    // I think it will eventually be used for REMOVE and for
    // CODE_XX_SW without SKIP_TO_XX_SW.
    int nBLUp /* = nBLUp*/; (void)nBLUp; // silence gcc

    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
#if defined(SAVE_PREFIX_TEST_RESULT)
    Word_t wPrefixMismatch = 0; (void)wPrefixMismatch;
#endif // defined(SAVE_PREFIX_TEST_RESULT)
#if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrUp = pwrUp; // suppress "uninitialized" compiler warning
#endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Link_t *pLnPrefix = NULL; (void)pLnPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    int nBLRPrefix = 0; (void)nBLRPrefix;

    Word_t wPopCntUp = 0; (void)wPopCntUp;
#if defined(COUNT)
    Word_t wPopCntSum = 0;
#endif // defined(COUNT)

    // Cleanup is expensive. So we only do it if it has been requested.
    int bCleanupRequested = 0; (void)bCleanupRequested;
    int bCleanup = 0; (void)bCleanup;

    int nType;
    Word_t *pwr;
    int nBLR;
  #if ! defined(LOOKUP) || defined(B_JUDYL)
    int nPos;
  #endif // ! defined(LOOKUP) || defined(B_JUDYL)

  #ifdef GOTO_AT_FIRST_IN_LOOKUP
  #ifdef SKIP_LINKS
  #ifdef LOOKUP
    // This shortcut made the code faster in my testing.
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot);
    if (nType > T_SWITCH) {
        DBGX(printf("# goto t_skip_to_switch\n"));
        goto t_skip_to_switch;
    }

    // This shortcut made the code faster in my testing.
    nBLR = nBL;
    DBGX(printf("# goto fastAgain\n"));
    goto fastAgain;

  #endif // LOOKUP
  #endif // SKIP_LINKS
  #endif // GOTO_AT_FIRST_IN_LOOKUP

  #if ! defined(LOOKUP) || defined(B_JUDYL)
    nPos = -1;
  #endif // ! defined(LOOKUP) || defined(B_JUDYL)

#if defined(COUNT)
    int bLinkPresent;
    int nLinks;
#endif // defined(COUNT)

#if defined(INSERT) || defined(REMOVE)
  #if !defined(RECURSIVE)
top:;
  #endif // !defined(RECURSIVE)
#endif // defined(INSERT) || defined(REMOVE)
    DBGX(printf("# top\n"));
    nBLR = nBL;

#if defined(LOOKUP) || !defined(RECURSIVE)
again:;
#endif // defined(LOOKUP) || !defined(RECURSIVE)
    DBGX(printf("# again\n"));

#if defined(SKIP_LINKS)
    assert(nBLR == nBL);
#endif // defined(SKIP_LINKS)
#if ( ! defined(LOOKUP) )
  #if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
  #endif // ! defined(USE_XX_SW)
#endif // ( ! defined(LOOKUP) )
    DBGX(printf("# nBL %d pLn %p wRoot " OWx" wKey " OWx"\n", nBL, pLn, wRoot, wKey));

    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot); // pwr isn't meaningful for all nType values

  #if defined(JUMP_TABLE)
    static void *pvJumpTable[] = {
      #if defined(SEPARATE_T_NULL)
        &&t_null,
      #endif // defined(SEPARATE_T_NULL)
      #if (cwListPopCntMax != 0)
        &&t_list,
          #if defined(SKIP_TO_LIST)
        &&t_skip_to_list,
          #endif // defined(SKIP_TO_LIST)
          #if defined(UA_PARALLEL_128)
        &&t_list_ua,
          #endif // defined(UA_PARALLEL_128)
      #endif // (cwListPopCntMax != 0)
        &&t_bitmap,
      #if defined(SKIP_TO_BITMAP)
        &&t_skip_to_bitmap,
      #endif // defined(SKIP_TO_BITMAP)
      #if defined(EMBED_KEYS)
        &&t_embedded_keys,
      #endif // defined(EMBED_KEYS)
      #if defined(CODE_LIST_SW)
        &&t_list_sw,
      #endif // defined(CODE_LIST_SW)
      #if defined(SKIP_TO_LIST_SW)
        &&t_skip_to_list_sw,
      #endif // defined(SKIP_TO_LIST_SW)
      #if defined(CODE_BM_SW)
        &&t_bm_sw,
      #endif // defined(CODE_BM_SW)
      #if defined(SKIP_TO_BM_SW)
        &&t_skip_to_bm_sw,
      #endif // defined(SKIP_TO_BM_SW)
      #if defined(CODE_XX_SW)
        &&t_xx_sw,
      #endif // defined(CODE_XX_SW)
      #if defined(SKIP_TO_XX_SW)
        &&t_skip_to_xx_sw,
      #endif // defined(SKIP_TO_XX_SW)
      #if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
        &&t_full_bm_sw,
          #if defined(SKIP_TO_BM_SW)
        &&t_skip_to_full_bm_sw,
          #endif // defined(SKIP_TO_BM_SW)
      #endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
        &&t_switch,
      #if defined(SKIP_LINKS)
        &&t_skip_to_switch,
          #if (T_SKIP_TO_SWITCH <= 14)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 14)
          #if (T_SKIP_TO_SWITCH <= 13)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 13)
          #if (T_SKIP_TO_SWITCH <= 12)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 12)
          #if (T_SKIP_TO_SWITCH <= 11)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 11)
          #if (T_SKIP_TO_SWITCH <= 10)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 10)
          #if (T_SKIP_TO_SWITCH <= 9)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 9)
          #if (T_SKIP_TO_SWITCH <= 8)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 8)
          #if (T_SKIP_TO_SWITCH <= 7)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 7)
          #if (T_SKIP_TO_SWITCH <= 6)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 6)
          #if (T_SKIP_TO_SWITCH <= 5)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 5)
          #if (T_SKIP_TO_SWITCH <= 4)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 4)
          #if (T_SKIP_TO_SWITCH <= 3)
        &&t_skip_to_switch,
          #endif // (T_SKIP_TO_SWITCH <= 3)
      #endif // defined(SKIP_LINKS)
    };
    goto *pvJumpTable[nType];
  #endif // defined(JUMP_TABLE)

    goto fastAgain;
fastAgain:;
    switch (nType)
    {

  // At most one of DEFAULT_SKIP_TO_SW, DEFAULT_SWITCH,
  // DEFAULT_LIST and DEFAULT_BITMAP may be defined.
  #ifdef DEBUG
  #ifndef DEFAULT_SKIP_TO_SW
  #ifndef DEFAULT_SWITCH
  #ifndef DEFAULT_LIST
  #ifndef DEFAULT_BITMAP
  #if !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
    default: DBG(printf("unknown type %d\n", nType)); assert(0); exit(0);
  #endif // !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
  #endif // DEFAULT_BITMAP
  #endif // DEFAULT_LIST
  #endif // DEFAULT_SWITCH
  #endif // DEFAULT_SKIP_TO_SW
  #endif // DEBUG

#if defined(SKIP_LINKS)

  #if !defined(LVL_IN_WR_HB) && !defined(LVL_IN_SW)
  #if !defined(DEFAULT_SKIP_TO_SW) && !defined(ALL_SKIP_TO_SW_CASES)
    // For level in type, i.e. (!LVL_IN_WR_HB && !LVL_IN_SW),
    // multiple type values all represent T_SKIP_TO_SWITCH, i.e.
    // level = nType - T_SKIP_TO_SWITCH + 2.
    #error Level in type requires DEFAULT_SKIP_TO_SW or ALL_SKIP_TO_SW_CASES.
  #endif // !defined(LVL_IN_WR_HB) && !defined(LVL_IN_SW)
  #endif // !defined(DEFAULT_SKIP_TO_SW) && !defined(ALL_SKIP_TO_SW_CASES)

  #if defined(DEFAULT_SKIP_TO_SW)
          #if defined(DEFAULT_SWITCH)
          #error DEFAULT_SWITCH with DEFAULT_SKIP_TO_SW
          #endif // defined(DEFAULT_SWITCH)
          #if defined(DEFAULT_BITMAP)
          #error DEFAULT_BITMAP with DEFAULT_SKIP_TO_SW
          #endif // defined(DEFAULT_BITMAP)
          #if defined(DEFAULT_LIST)
          #error DEFAULT_LIST with DEFAULT_SKIP_TO_SW
          #endif // defined(DEFAULT_LIST)
    default:
  #endif // defined(DEFAULT_SKIP_TO_SW)
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    case T_SKIP_TO_SWITCH: // skip link to uncompressed switch
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
  #ifdef ALL_SKIP_TO_SW_CASES
    // Extra cases so we have at least EXP(cnBitsMallocMask) cases so gcc
    // will create a jump table with no bounds check at the beginning after
    // extracting nType from wRoot.
    // We have not coded a no-bounds-check version without SKIP_LINKS yet.
    case T_SKIP_TO_SWITCH+1:
    case T_SKIP_TO_SWITCH+2:
    case T_SKIP_TO_SWITCH+3:
    case T_SKIP_TO_SWITCH+4:
    case T_SKIP_TO_SWITCH+5:
    case T_SKIP_TO_SWITCH+6:
    case T_SKIP_TO_SWITCH+7:
    case T_SKIP_TO_SWITCH+8:
    case T_SKIP_TO_SWITCH+9:
    case T_SKIP_TO_SWITCH+10:
    case T_SKIP_TO_SWITCH+11:
    case T_SKIP_TO_SWITCH+12:
    case T_SKIP_TO_SWITCH+13:
    case T_SKIP_TO_SWITCH+14:
  #endif // ALL_SKIP_TO_SW_CASES
    {
        goto t_skip_to_switch; // silence cc in case there are no other uses
t_skip_to_switch:
        // Skip to switch.
        // pwr points to a switch
  #if defined(NO_PREFIX_CHECK) && defined(LOOKUP)
        nBLR = gnBLRSkip(qy);
  #else // defined(NO_PREFIX_CHECK) && defined(LOOKUP)
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
        DBG((nType != T_SKIP_TO_SWITCH) ? printf("nType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
        DBGX(printf("SKIP_TO_SW\n"));

        // Looks to me like PrefixMismatch has no performance issues with
        // not all digits being the same size.  It doesn't care.
        // But it does use nBL a couple of times.  Maybe it would help to
        // have bl tests here and call with a constant.  Possibly more
        // interestingly it does compare nBL to cnBitsPerWord for PP_IN_LINK.

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
      #if defined(COUNT)
            DBGC(printf("SKIP_TO_SW: COUNT PM " OWx"\n", wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt;
          #if ! defined(NO_SKIP_AT_TOP)
          #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                if (nBL >= cnBitsPerWord) {
                    //int nBitsIndexSz = nBL_to_nBitsIndexSzNAX(nBLR);
                    int nBitsIndexSz = nBL_to_nBitsIndexSzNAB(nBLR);
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW,
                                      EXP(nBitsIndexSz), EXP(nBitsIndexSz));
                } else
          #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
          #endif // ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
                    if (wPopCnt == 0) {
                        wPopCnt = EXP(nBLR);
                    }
                }
                DBGC(printf("SKIP_TO_SW: PM wPopCnt %" _fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("sksw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // defined(COUNT)
            break;
        }
  #endif // defined(NO_PREFIX_CHECK) && defined(LOOKUP)

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

#if defined(SKIP_TO_XX_SW) // Doesn't work yet.

    case T_SKIP_TO_XX_SW: // skip link to narrow/wide switch
    {
        goto t_skip_to_xx_sw; // silence cc in case there are no other uses
t_skip_to_xx_sw:
        // pwr points to a variable-width aka doubling switch
        DBGX(printf("SKIP_TO_XX_SW\n"));

        // Looks to me like PrefixMismatch has no performance issues with
        // not all digits being the same size.  It doesn't care.
        // But it does use nBL a couple of times.  Maybe it would help to
        // have bl tests here and call with a constant.  Possibly more
        // interestingly it does compare nBL to cnBitsPerWord.

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_XX_SW: COUNT PM %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt;
      #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
          #error Not ready yet
                if (nBL >= cnBitsPerWord) {
                    int nBitsIndexSz = nBL_to_nBitsIndexSzNAB(nBLR);
                    //int nLinks = ??? __builtin_popcount
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW, nLinks, nLinks);
                } else
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
                }
                DBGC(printf("SKIP_TO_XX_SW: PM wPopCnt %" _fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skxxsw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_xx_sw;

    } // end of T_SKIP_TO_XX_SW case

#endif // defined(SKIP_TO_XX_SW)

#endif // defined(SKIP_LINKS)

  #if defined(DEFAULT_SWITCH)
      #if defined(DEFAULT_SKIP_TO_SW)
      #error DEFAULT_SKIP_TO_SW with DEFAULT_SWITCH
      #endif // defined(DEFAULT_SKIP_TO_SW)
      #if defined(DEFAULT_BITMAP)
      #error DEFAULT_BITMAP with DEFAULT_SWITCH
      #endif // defined(DEFAULT_BITMAP)
      #if defined(DEFAULT_LIST)
      #error DEFAULT_LIST with DEFAULT_SWITCH
      #endif // defined(DEFAULT_LIST)
    default:
  #endif // defined(DEFAULT_SWITCH)
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        // nBL is bits left after picking the link from the previous switch
        // nBL has not been reduced by any skip indicated in that link
        // nBLR is bits left after reducing nBL by any skip in that link
        // nBLR is bits left to decode by this switch and below
        goto t_switch; // silence cc in case other the gotos are ifdef'd out
t_switch:;
        nBW = gnBW(qy, T_SWITCH, nBLR); // num bits decoded by this switch
  #ifdef BITMAP
        // What if nBLR <= nBW + cnLogBitsPerLink?
        // Don't we have a two-digit bitmap?
        // Shouldn't we have already changed nType when we created it?
        // Should we goto the bitmap code here?
        // Could we be doing lazy conversion with cnBitsInD1 < cnLogBitsPerLink?
        assert((nBLR > nBW + cnLogBitsPerLink) || cbEmbeddedBitmap);
  #endif // BITMAP
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
        // ((uint8_t *)&wKey)[(cnBitsPerWord - nBL) >> 3];
        // ((uint8_t *)&wKey)[cnDigitsPerWord - nDL];
        // ((uint8_t *)&wSwappedKey)[nDL];
        // *(uint8_t *)&wSwappedAndShiftedKey;
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];

        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
        goto switchTail; // in case other uses go away by ifdef
switchTail:;
        // Handle big picture tree cleanup.
        if (SwCleanup(qy, wKey, nBLR, bCleanup)) { goto restart; }
        wPopCntUp = SwIncr(qy, nBLR, bCleanup, nIncr); // adjust pop count
        IF_COUNT(wPopCntSum += CountSw(qy, nBLR, nBW, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(pLnUp = pLn);
        IF_SKIP_TO_XX_SW(IF_INSERT(nBLUp = nBL));
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR);

#ifdef BITMAP
        // Is there any reason to have
        // EXP(cnBitsInD1) <= (sizeof(Link_t) * 8)? What about lazy conversion
        // of embedded keys at nBL > sizeof(Link_t) * 8 to
        // nBL == sizeof(Link_t) * 8?
        // Initialize warns if cnBitsInD1 is too small relative
        // to sizeof(Link_t).
        // if nBL == LOG(sizeof(Link_t) * 8) there is no room for a
        // type field containing T_BITMAP so we goto t_bitmap directly.
        // What happens when it's time to make a 2-digit bitmap?
        // What happens when nBL < LOG(sizeof(Link_t) * 8)?
        // Would NewSwitch just allocate way more memory than we need?
        // Can we skip to one of these sub link size bitmaps?
        // What happens when it's time to make a 2-digit bitmap?
        // Should we back up nBLR and nBL and goto t_bitmap that way?
        // The first test below is done at compile time and will make the rest
        // of the code block go away if it is not needed.
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // We're leaving qy in an iffy state without updating nType and pwr.
            goto t_bitmap;
        }
#else // BITMAP
        assert(!cbEmbeddedBitmap || (nBL > cnLogBitsPerLink));
#endif // BITMAP

  #if defined(INSERT) || defined(REMOVE)
      #if defined(CODE_XX_SW) && !defined(NO_TYPE_IN_XX_SW)
        if ( (nType != T_XX_SW)
          #if defined(SKIP_TO_XX_SW)
            && (nType != T_SKIP_TO_XX_SW)
          #endif // defined(SKIP_TO_XX_SW)
            )
      #endif // defined(CODE_XX_SW) && !defined(NO_TYPE_IN_XX_SW)
        { pLnUp = NULL; } // InsertGuts infers XX_SW if pLnUp != NULL
  #endif // defined(INSERT) || defined(REMOVE)

  #if defined(LOOKUP) || !defined(RECURSIVE)
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
  #else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(nBL, pLn, wKey);
  #endif // defined(LOOKUP) || !defined(RECURSIVE)

    } // end of case T_SWITCH

#if defined(CODE_XX_SW)

    case T_XX_SW: // no-skip (aka close) switch (vs. distant switch) w/o bm
    {
        goto t_xx_sw;
t_xx_sw:;
        nBW = gnBW(qy, T_XX_SW, nBLR);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];

        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
        // wDigit is already correct.
  #if defined(LOOKUP) || defined(NO_TYPE_IN_XX_SW)
        // Handle big picture tree cleanup.
        if (SwCleanup(qy, wKey, nBLR, bCleanup)) { goto restart; }
        wPopCntUp = SwIncr(qy, nBLR, bCleanup, nIncr); // adjust pop count
        IF_COUNT(wPopCntSum += CountSw(qy, nBLR, nBW, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(pLnUp = pLn);
        IF_SKIP_TO_XX_SW(IF_INSERT(nBLUp = nBL));
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR);
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }

        // Handle XX_SW-specific special cases that don't go back to the top.
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
      #if defined(NO_TYPE_IN_XX_SW) || (defined(LOOKUP) && defined(HANDLE_DL2_IN_EMBEDDED_KEYS))
        // Blow-ups are handled in t_embedded_keys.
        goto t_embedded_keys;
      #else // defined(NO_TYPE_IN_XX_SW) || handle dl2 in t_embedded_keys
          #if defined(LOOKUP) || !defined(RECURSIVE)
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
          #else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(nBL, pLn, wKey);
          #endif // defined(LOOKUP) || !defined(RECURSIVE)
      #endif // defined(NO_TYPE_IN_XX_SW) || handle dl2 in t_embedded_keys

  #else // defined(LOOKUP) || defined(NO_TYPE_IN_XX_SW)
        goto switchTail;
  #endif // defined(LOOKUP) || defined(NO_TYPE_IN_XX_SW)

    } // end of case T_XX_SW

#endif // defined(CODE_XX_SW)

#if defined(CODE_BM_SW)

  #if defined(SKIP_TO_BM_SW)

    case T_SKIP_TO_BM_SW:
    {
        goto t_skip_to_bm_sw; // silence cc in case there are no other uses
t_skip_to_bm_sw:
        // pwr points to a bitmap switch
        DBGX(printf("SKIP_TO_BM_SW\n"));

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_BM_SW: COUNT PM %zd\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt;
      #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
          #error Not ready yet
                if (nBL >= cnBitsPerWord) {
                    int nBW = gnBW(qy, T_BM_SW, nBL);
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW, EXP(nBW), EXP(nBW));
                } else
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
                }
                DBGC(printf("SKIP_TO_BM_SW: PM wPopCnt %zd\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skbmsw wPopCnt 0x%zx wPopCntSum 0x%zx\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_bm_sw;

    } // end of T_SKIP_TO_BM_SW case

  #endif // defined(SKIP_TO_BM_SW)

  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_FULL_BM_SW:
    {
        goto t_full_bm_sw; // silence cc in case there are no other uses
t_full_bm_sw:
      #if defined(LOOKUP)
// Skip over sw_awBm.  The rest of BmSwitch_t must be same as Switch_t.
        pwr = (Word_t *)&((BmSwitch_t *)pwr)->sw_wPrefixPop;
        goto t_switch;
      #endif // defined(LOOKUP)
        goto t_bm_sw;

    } // end of case T_FULL_BM_SW

  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

    case T_BM_SW:
    {
        goto t_bm_sw; // silence cc in case other the gotos are ifdef'd out
t_bm_sw:;
        nBW = gnBW(qy, T_BM_SW, nBLR);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);

        Word_t wSwIndex;
  #if defined(BM_IN_LINK)
        // Have not coded for skip link at top here and elsewhere.
        assert( ! tp_bIsSkip(nType) || (nBL != cnBitsPerWord) );
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nBL == cnBitsPerWord and pLn not at the top.
        // What is the possible ambiguity?
        // How could we ever have nBL == cnBitsPerWord with pLn not at top?
        // Are we trying to distinguish an original call starting
        // somewhere in the middle of the tree from a normal original call
        // starting at the top?
        // Because the recursive implementation wants to do something different
        // for the final return?
        // Why do we set pLn to NULL sometimes?
        // Why do we set pLnUp to NULL sometimes?
        // We do actually call InsertOrRemove with nBL < cnBitsPerWord.
        // We need to know if there is a whole link.
        // InsertGuts always calls back into Insert with the same pLn
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nBL == cnBitsPerWord and pLn not at the top.
        // How could we ever have nBL with pLn not at the top? Huh?
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nBL == cnBitsPerWord)
      #else // defined(RECURSIVE)
                && (pLn == pLnOrig)
          #if !defined(LOOKUP)
                && (nBLOrig == cnBitsPerWord)
          #endif // !defined(LOOKUP)
      #endif // defined(RECURSIVE)
            ) )
  #endif // defined(BM_IN_LINK)
        {
  #if ! defined(COUNT)
            int bLinkPresent; // need less local bLinkPresent for COUNT
  #endif // ! defined(COUNT)
  #if defined(BM_SW_FOR_REAL)
      #ifdef ONE_BM_SW_INDEX_CALL
            BmSwIndex(qy, wDigit, &wSwIndex, &bLinkPresent);
      #else // ONE_BM_SW_INDEX_CALL
            BmSwIndex(qy, wDigit, /* pwSwIndex */ NULL, &bLinkPresent);
      #endif // ONE_BM_SW_INDEX_CALL
  #else // defined(BM_SW_FOR_REAL)
      #ifdef LOOKUP
            // Here are shortcuts that we use for performance experiments.
            // The code is not correct. It assumes we only ever test for keys
            // that are actually present.
            Word_t wAddr = (Word_t)&pwr_pLinks((BmSwitch_t *)pwr)[wDigit];
            (void)wAddr;
            wSwIndex = wDigit;
          #if defined(SW_BM_NO_DEREF)
            bLinkPresent = 1;
          #elif defined(SW_BM_DEREF_ADJACENT_WORD)
            // Test adjacent word in the same cache line for link presence.
            // We don't know what will be there so we xor with a constant
            // to see if we can get through the test.
            // Is there any difference in performance between reading the
            // first two words in a cache line in order vs any other two
            // words in any order?
            wAddr ^= (1 << cnLogBytesPerWord);
            bLinkPresent = *(Word_t *)wAddr ^ ~(Word_t)0x12484210;
          #elif defined(SW_BM_USE_ADJACENT_WORD)
            // Use adjacent word in link index calculation.
            wAddr ^= (1 << cnLogBytesPerWord);
            bLinkPresent = *(Word_t *)wAddr ^ ~(Word_t)0x12484210;
            wSwIndex += !bLinkPresent;
          #elif defined(SW_BM_DEREF_NEXT_LINE)
            // Test word in next cache line for link presence.
            // slow
            bLinkPresent = *(Word_t *)(wAddr + 64) ^ ~(Word_t)0x12484210;
          #elif defined(SW_BM_DEREF_PREV_LINE)
            // Does cache line read ahead help? Not in first test.
            // Test word in previous cache line for link presence.
            // We don't know what will be there so we xor with a constant
            // to see if we can get through the test.
            // slow
            bLinkPresent = *(Word_t *)(wAddr - 64) ^ ~(Word_t)0x12484210;
          #elif defined(SW_BM_DEREF_END_OF_LINE)
            // fast
            bLinkPresent = *(Word_t *)(wAddr | 0x38) ^ ~(Word_t)0x12484210;
          #elif defined(SW_BM_DEREF_BEGIN_OF_LINE)
            // fast
            wAddr &= ~(Word_t)63;
            bLinkPresent = *(Word_t *)wAddr ^ ~(Word_t)0x12484210;
          #elif defined(SW_BM_USE_BEGIN_OF_LINE)
            // Use word at beginning of cache line in link index calculation.
            // !!! fast !!!
            wAddr &= ~(Word_t)63;
            bLinkPresent = *(Word_t *)wAddr ^ ~(Word_t)0x12484210;
            wSwIndex = wDigit + !bLinkPresent;
          #endif // defined(BM_SW_FOR_REAL) && defined(SW_BM_DEREF_ONLY)
      #else // LOOKUP
            BmSwIndex(qy, wDigit, &wSwIndex, &bLinkPresent);
      #endif // LOOKUP
  #endif // defined(BM_SW_FOR_REAL)
  #if ! defined(COUNT)
            // Test to see if link exists before figuring out where it is.
            if ( ! bLinkPresent ) {
                DBGX(printf("missing link\n"));
                break; // not found
            }
  #endif // ! defined(COUNT)
  #if defined(BM_SW_FOR_REAL) && !defined(ONE_BM_SW_INDEX_CALL)
            BmSwIndex(qy, wDigit, &wSwIndex, /* pbPresent */ NULL);
  #endif // defined(BM_SW_FOR_REAL) && !defined(ONE_BM_SW_INDEX_CALL)
        }

        pLnNew = &pwr_pLinks((BmSwitch_t *)pwr)[wSwIndex];

        // Update wDigit before bmSwTail because we have to do it
        // in t_list_sw before goto bmSwTail.
        IF_COUNT(wDigit = wSwIndex);
        goto bmSwTail;
bmSwTail:;
        // bLinkPresent has already been initialized.
        IF_COUNT(nLinks = INT_MAX);
  #if defined(LOOKUP)
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR);
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
        goto again;
  #else // defined(LOOKUP)
      #ifdef INSERT
        // This test should be the same as the one in InsertCleanup.
        if (tp_bIsBmSw(nType)) {
            if (gwPopCnt(qy, nBLR) * nBLR * cnBmSwConvert
                > EXP(nBW) * 8 * sizeof(Link_t) * cnBmSwRetain)
            {
                bCleanupRequested = 1; // on success
            }
        }
      #endif // INSERT
        goto switchTail;
  #endif // defined(LOOKUP)

    } // end of case T_BM_SW

#endif // defined(CODE_BM_SW)

#if defined(CODE_LIST_SW)

  #if defined(SKIP_TO_LIST_SW)

    case T_SKIP_TO_LIST_SW:
    {
        goto t_skip_to_list_sw; // silence cc in case there are no other uses
t_skip_to_list_sw:
        DBGX(printf("SKIP_TO_LIST_SW\n"));

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("SKIP_TO_LIST_SW: COUNT PM %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt;
      #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
          #error Not ready yet
                if (nBL >= cnBitsPerWord) {
                    int nBW = gnBW(qy, T_LIST_SW, nBLR);
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW, EXP(nBW), EXP(nBW));
                } else
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
                }
                DBGC(printf("SKIP_TO_LIST_SW: PM wPopCnt %" _fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("sklssw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_list_sw;

    } // end of T_SKIP_TO_LIST_SW case

  #endif // defined(SKIP_TO_LIST_SW)

    // Compressed switch with a list header indicating which links are
    // present and implying their offsets.
    case T_LIST_SW:
    {
        goto t_list_sw; // silence cc in case other the gotos are ifdef'd out
t_list_sw:;

        nBW = gnBW(qy, T_LIST_SW, nBLR);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);

        Word_t wSwIndex;
  #if defined(SW_LIST_IN_LINK)
        // Have not coded for skip link at top here and elsewhere.
        assert( ! tp_bIsSkip(nType) || (nBL != cnBitsPerWord) );
        // We avoid ambiguity by disallowing calls to Insert/Remove with
        // nBL == cnBitsPerWord and pLn not at the top.
        // We need to know if there is a whole link.
        // InsertGuts always calls back into Insert with the same pLn
        // it was called with.  So it means Insert cannot call InsertGuts
        // with nBL == cnBitsPerWord and pLn not at the top.
        // What about defined(RECURSIVE)?
        // What about Remove and RemoveGuts?
        if ( ! (1
      #if defined(RECURSIVE)
                && (nBL == cnBitsPerWord)
      #else // defined(RECURSIVE)
                && (pLn == pLnOrig)
          #if !defined(LOOKUP)
                && (nBLOrig == cnBitsPerWord)
          #endif // !defined(LOOKUP)
      #endif // defined(RECURSIVE)
            ) )
  #endif // defined(SW_LIST_IN_LINK)
        {
  #if ! defined(COUNT)
            int bLinkPresent;
  #endif // ! defined(COUNT)
            ListSwIndex(qy, nBW, wDigit, /* pwIndex */ NULL, &bLinkPresent);
  #if ! defined(COUNT)
            // Test to see if link exists before figuring out where it is.
            if ( ! bLinkPresent )
            {
                DBGX(printf("missing link\n"));
                break; // not found
            }
  #endif // ! defined(COUNT)
            ListSwIndex(qy, nBW, wDigit, &wSwIndex, /* pbPresent */ NULL);
        }

        pLnNew = &gpListSwLinks(qy)[wSwIndex];
        // Update wDigit before goto bmSwTail because we're about to
        // leave the scope of the local wSwIndex and the destination
        // has its own wSwIndex.
        IF_COUNT(wDigit = wSwIndex);
        // We've initialized bLinkPresent prior to goto bmSwTail.
#if defined(CODE_BM_SW)
        goto bmSwTail;
#else // defined(CODE_BM_SW)
        // bLinkPresent has already been initialized.
        IF_COUNT(nLinks = INT_MAX);
  #if defined(LOOKUP)
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR);
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
        goto again;
  #else // defined(LOOKUP)
        goto switchTail;
  #endif // defined(LOOKUP)
#endif // defined(CODE_BM_SW)

    } // end of case T_LIST_SW

#endif // defined(CODE_LIST_SW)

#if (cwListPopCntMax != 0)

  #if defined(SKIP_TO_LIST)
    case T_SKIP_TO_LIST: {
        DBGX(printf("T_SKIP_TO_LIST\n"));
        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("T_SKIP_TO_LIST: COUNT PREFIX_MISMATCH %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt = gnListPopCnt(qy, nBLR);
                assert(wPopCnt != 0);
                DBGC(printf("T_SKIP_TO_LIST: PREFIX_MISMATCH wPopCnt %" _fw
                                "d\n", wPopCnt));
                wPopCntSum += wPopCnt; // break below to return wPopCntSum
                DBGC(printf("skls wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_list;
    }
  #endif // defined(SKIP_TO_LIST)

  #if defined(DEFAULT_LIST)
      #if defined(DEFAULT_SKIP_TO_SW)
      #error DEFAULT_SKIP_TO_SW with DEFAULT_LIST
      #endif // defined(DEFAULT_SKIP_TO_SW)
      #if defined(DEFAULT_BITMAP)
      #error DEFAULT_BITMAP with DEFAULT_LIST
      #endif // defined(DEFAULT_BITMAP)
      #if defined(DEFAULT_SWITCH)
      #error DEFAULT_SWITCH with DEFAULT_LIST
      #endif // defined(DEFAULT_SWITCH)
    default:
  #endif // defined(DEFAULT_LIST)
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    case T_LIST:
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_list;
t_list:;
        DBGX(printf("T_LIST nBL %d nBLR %d\n", nBL, nBLR));
        DBGX(printf("wKeyPopMask " OWx"\n", wPrefixPopMaskBL(nBLR)));

  #if defined(INSERT) || defined(REMOVE)
        DBGX(printf("T_LIST bCleanup %d nIncr %d\n", bCleanup, nIncr));
        if (bCleanup) {
    #if defined(INSERT) && defined(B_JUDYL)
            return NULL;
    #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
    #endif // defined(INSERT)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
          #if defined(INSERT)
        if (nIncr == -1) { return Failure; } // undo
          #endif // defined(INSERT)
          #if defined(REMOVE)
        if (nIncr == 1) { return Failure; } // undo
          #endif // defined(REMOVE)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COUNT) && ! defined(SEPARATE_T_NULL)
        if (pwr == NULL) { nPos = ~0; }
      #endif // defined(COUNT) && ! defined(SEPARATE_T_NULL)

      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
      #ifdef LOOKUP
        if (PrefixCheckAtLeaf(qy, wKey
  #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF
  #ifdef SAVE_PREFIX_TEST_RESULT
                , wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
                , pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef SAVE_PREFIX
                , pLnPrefix, pwrPrefix, nBLRPrefix
  #endif // SAVE_PREFIX
                  ) // end call to PrefixCheckAtLeaf
            == Success)
      #endif // LOOKUP
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
            SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
            SMETRICS(++j__GetCalls);
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
          #if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
          #endif // ! defined(SEPARATE_T_NULL)
          #if defined(LOOKUP)
              #if defined(B_JUDYL)
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInList(qy, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // defined(B_JUDYL)
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qy, nBLR, wKey) >= 0)
                  #else // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
                && ListHasKey(qy, nBLR, wKey)
                  #endif // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
              #endif // defined(B_JUDYL)
          #else // defined(LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
          #endif // defined(LOOKUP)
                )
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
          #if defined(INSERT)
              #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
              #endif // ! defined(RECURSIVE)
          #endif // defined(INSERT)
          #if defined(REMOVE)
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                // Adjust wPopCnt in link to leaf if pop cnt is in the link.
                // wPopCnt in link to switch is adjusted elsewhere, i.e. in
                // the same place as wPopCnt in switch is adjusted when pop
                // cnt is in the switch.
                assert(nIncr == -1);
                if (nBL < cnBitsPerWord) {
                    swPopCnt(qy, nBLR, gwPopCnt(qy, nBLR) - 1);
                }
              #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
              #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                DBGX(printf("Lookup (or Insert) returning nPos %d %p," OWx"\n",
                             nPos, (void *)&pwr[~nPos], pwr[~nPos]));
                return &pwr[~nPos];
              #else // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
          #endif // defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
            }
          #if defined(COUNT)
            else
          #endif // defined(COUNT)
          #if defined(INSERT) || defined(REMOVE) || defined(COUNT)
            {
                nPos ^= -1;
            }
          #endif // defined(INSERT) || defined(REMOVE) || defined(COUNT)
        }
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
        else
        {
            // Shouldn't this be using the previous nBL for pwrUp?
            DBGX(printf("Mismatch at list wPrefix " OWx" nBL %d\n",
          #ifdef PP_IN_LINK
                        gwPrefix(qy),
          #else // PP_IN_LINK
                        PWR_wPrefixNATBL(NULL, pwrUp, nBL),
          #endif // PP_IN_LINK
                        nBL));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && ...

      #if defined(COUNT)
        DBGC(printf("T_LIST: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
        return wPopCntSum;
      #endif // defined(COUNT)

      #if defined(INSERT)
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Adjust wPopCnt in link to leaf if pop cnt is in the link.
        // wPopCnt in link to switch is adjusted elsewhere, i.e. in
        // the same place as wPopCnt in switch is adjusted when pop
        // cnt is in the switch.
        // Would be nice to be able to get the current pop count from
        // SearchList because chances are it will have read it.
        // But it is more important to avoid getting it when not necessary
        // during lookup.
        assert((nBL == cnBitsPerWord) // there is no link with pop count
            || (pwr != NULL) // non-NULL implies non-zero pop count
            || (gwPopCnt(qy, nBLR) == 0));
        assert(nIncr == 1);
        DBGI(printf("did not find key\n"));
        if (nBL < cnBitsPerWord) {
            swPopCnt(qy, nBLR, gwPopCnt(qy, nBLR) + 1);
        }
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
      #endif // defined(INSERT)
      #ifdef INSERT
          // Tried putting test for (cn2dBmMaxWpkPercent != 0) in the
          // condition below but g++ complained about the unsigned comparison
          // to less than zero at the end of the condition always being false.
          #if (cn2dBmMaxWpkPercent != 0) // g++ warns always
        if ((EXP(cnBitsInD1) > sizeof(Link_t) * 8) // compiled out
            && (nBLR == cnBitsInD1) // check that conversion is not done
            && (nBL == nBLR) // converting skip makes no sense
            // this should agree with the test in InsertCleanup
            && (wPopCntUp * cn2dBmMaxWpkPercent
                > EXP(cnBitsLeftAtDl2 - cnLogBitsPerWord) * 100)
            )
        {
            bCleanupRequested = 1; // goto cleanup when done
        }
          #endif // (cn2dBmMaxWpkPercent != 0)
      #endif // INSERT

        break;

    } // end of case T_LIST

#if defined(UA_PARALLEL_128)
    case T_LIST_UA:
    {
        goto t_list_ua;
t_list_ua:;
        DBGX(printf("T_LIST_UA nBL %d nBLR %d\n", nBL, nBLR));
        DBGX(printf("wKeyPopMask " OWx"\n", wPrefixPopMaskBL(nBLR)));

  #if defined(INSERT) || defined(REMOVE)
        DBGX(printf("T_LIST_UA bCleanup %d nIncr %d\n", bCleanup, nIncr));
        if (bCleanup) {
            return Success;
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
          #if defined(INSERT)
        if (nIncr == -1) { return Failure; } // undo
          #endif // defined(INSERT)
          #if defined(REMOVE)
        if (nIncr == 1) { return Failure; } // undo
          #endif // defined(REMOVE)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COUNT) && ! defined(SEPARATE_T_NULL)
        if (pwr == NULL) { nPos = ~0; }
      #endif // defined(COUNT) && ! defined(SEPARATE_T_NULL)

      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
      #ifdef LOOKUP
        if (PrefixCheckAtLeaf(qy, wKey
  #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF
  #ifdef SAVE_PREFIX_TEST_RESULT
                , wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
                , pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef SAVE_PREFIX
                , pLnPrefix, pwrPrefix, nBLRPrefix
  #endif // SAVE_PREFIX
                  ) // end call to PrefixCheckAtLeaf
            == Success)
      #endif // LOOKUP
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
            SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
            SMETRICS(++j__GetCalls);
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
        #if ! defined(SEPARATE_T_NULL)
                && (pwr != NULL)
        #endif // ! defined(SEPARATE_T_NULL)
        #if defined(LOOKUP) && !defined(B_JUDYL)
                && ListHasKey(qy, nBLR, wKey)
        #else // defined(LOOKUP) && !defined(B_JUDYL)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
        #endif // defined(LOOKUP) && !defined(B_JUDYL)
                )
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
          #if defined(INSERT)
              #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
              #endif // ! defined(RECURSIVE)
          #endif // defined(INSERT)
          #if defined(REMOVE)
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                // Adjust wPopCnt in link to leaf if pop cnt is in the link.
                // wPopCnt in link to switch is adjusted elsewhere, i.e. in
                // the same place as wPopCnt in switch is adjusted when pop
                // cnt is in the switch.
                assert(nIncr == -1);
                if (nBL < cnBitsPerWord) {
                    swPopCnt(qy, nBLR, gwPopCnt(qy, nBLR) - 1);
                }
              #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
                return KeyFound;
          #endif // defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
            }
          #if defined(COUNT)
            else
          #endif // defined(COUNT)
          #if defined(INSERT) || defined(REMOVE) || defined(COUNT)
            {
                nPos ^= -1;
            }
          #endif // defined(INSERT) || defined(REMOVE) || defined(COUNT)
        }
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
        else
        {
            // Shouldn't this be using the previous nBL for pwrUp?
            DBGX(printf("Mismatch at list wPrefix " OWx" nBL %d\n",
          #ifdef PP_IN_LINK
                        gwPrefix(qy),
          #else // PP_IN_LINK
                        PWR_wPrefixNATBL(NULL, pwrUp, nBL),
          #endif // PP_IN_LINK
                        nBL));
        }
      #endif // defined(LOOKUP) && defined(SKIP_PREFIX_CHECK) && ...

      #if defined(COUNT)
        DBGC(printf("T_LIST_UA: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
        return wPopCntSum;
      #endif // defined(COUNT)

      #if defined(INSERT)
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Adjust wPopCnt in link to leaf if pop cnt is in the link.
        // wPopCnt in link to switch is adjusted elsewhere, i.e. in
        // the same place as wPopCnt in switch is adjusted when pop
        // cnt is in the switch.
        // Would be nice to be able to get the current pop count from
        // SearchList because chances are it will have read it.
        // But it is more important to avoid getting it when not necessary
        // during lookup.
        assert((nBL == cnBitsPerWord) // there is no link with pop count
            || (pwr != NULL) // non-NULL implies non-zero pop count
            || (gwPopCnt(qy, nBLR) == 0));
        assert(nIncr == 1);
        DBGI(printf("did not find key\n"));
        if (nBL < cnBitsPerWord) {
            swPopCnt(qy, nBLR, gwPopCnt(qy, nBLR) + 1);
        }
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
      #endif // defined(INSERT)

        break;

    } // end of case T_LIST_UA
#endif // defined(UA_PARALLEL_128)

#endif // (cwListPopCntMax != 0)

#ifdef BITMAP
  #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: {
        goto t_skip_to_bitmap;
t_skip_to_bitmap:;
        DBGX(printf("T_SKIP_TO_BITMAP\n"));
        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
  #if defined(COUNT)
            DBGC(printf("T_SKIP_TO_BITMAP: COUNT PREFIX_MISMATCH %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wPrefixMismatch > 0) {
                Word_t wPopCnt
                    = w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
                DBGC(printf("T_SKIP_TO_BITMAP: PREFIX_MISMATCH wPopCnt %" _fw
                                "d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skbm wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        goto t_bitmap;
    }
  #endif // defined(SKIP_TO_BITMAP)
  #if defined(DEFAULT_BITMAP)
      #if defined(DEFAULT_SKIP_TO_SW)
      #error DEFAULT_SKIP_TO_SW with DEFAULT_BITMAP
      #endif // defined(DEFAULT_SKIP_TO_SW)
      #if defined(DEFAULT_LIST)
      #error DEFAULT_BITMAP with DEFAULT_BITMAP
      #endif // defined(DEFAULT_LIST)
      #if defined(DEFAULT_SWITCH)
      #error DEFAULT_SWITCH with DEFAULT_BITMAP
      #endif // defined(DEFAULT_SWITCH)
    default:
  #endif // defined(DEFAULT_BITMAP)
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    case T_BITMAP:
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_bitmap;
t_bitmap:;
  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
      #if defined(INSERT) && defined(B_JUDYL)
            return NULL;
      #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
      #endif // defined(INSERT) && defined(B_JUDYL)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(INSERT) || defined(REMOVE)
        if (EXP(cnBitsInD1) > sizeof(Link_t) * 8) {
            Word_t wPopCnt = gwPopCnt(qy, nBLR);
            swPopCnt(qy, nBLR, wPopCnt + nIncr);
        }
#endif // defined(INSERT) || defined(REMOVE)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

#if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_DEREF)

      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
      #ifdef LOOKUP
        if (PrefixCheckAtLeaf(qy, wKey
  #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF
  #ifdef SAVE_PREFIX_TEST_RESULT
                , wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
                , pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef SAVE_PREFIX
                , pLnPrefix, pwrPrefix, nBLRPrefix
  #endif // SAVE_PREFIX
                  ) // end call to PrefixCheckAtLeaf
            == Success)
      #endif // LOOKUP
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
  #if defined(COUNT)
            // Count bits.
            Word_t wPopCnt;
      #if defined(ALLOW_EMBEDDED_BITMAP)
            if ((cnBitsInD1 <= LOG(sizeof(Link_t) * 8))
                && (nBLR == cnBitsInD1))
            {
                //assert(nBL == nBLR); // skip to sub-link-size bm
                assert(cnBitsInD1 <= cnLogBitsPerWord); // for now
                Word_t wBit = EXP(wKey & MSK(nBL));
                Word_t wBmMask = wBit - 1;
                wPopCnt = __builtin_popcountll(wRoot & wBmMask);
            } else
      #else // defined(ALLOW_EMBEDDED_BITMAP)
            assert(cnBitsInD1 > LOG(sizeof(Link_t) * 8));
      #endif // defined(ALLOW_EMBEDDED_BITMAP)
            {
                int nWordOffset = (wKey & MSK(nBLR)) >> cnLogBitsPerWord;
                // Do we count from the front or the back? If nBLR is small
                // enough it's faster to just always count from the same
                // end rather than going to the trouble of figuring out
                // which end is closer.
                if ((nBLR > 8) && wKey & EXP(nBLR - 1)) {
                    wPopCnt
                        = w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)),
                                      nBLR);
                    if (wPopCnt == 0) {
                        wPopCnt = EXP(nBLR);
                    }
                    for (int nn = nWordOffset + 1;
                             nn < (int)EXP(nBLR - cnLogBitsPerWord); nn++) {
                        wPopCnt -= __builtin_popcountll(pwr[nn]);
                    }
                    Word_t wBmMask
                        = ((Word_t)-1 << (wKey & (cnBitsPerWord - 1)));
                    wPopCnt
                        -= __builtin_popcountll(pwr[nWordOffset] & wBmMask);
                }
                else
                {
                    wPopCnt = 0;
                    for (int nn = 0; nn < nWordOffset; nn++) {
                        wPopCnt += __builtin_popcountll(pwr[nn]);
                    }
                    Word_t wBmMask
                        = ((Word_t)1 << (wKey & (cnBitsPerWord - 1))) - 1;
                    wPopCnt
                        += __builtin_popcountll(pwr[nWordOffset] & wBmMask);
                }
            }
            wPopCntSum += wPopCnt;
            DBGC(printf("bm nBLR %d wPopCnt " OWx" wPopCntSum " OWx"\n",
                        nBLR, wPopCnt, wPopCntSum));
            return wPopCntSum;
  #else // defined(COUNT)
    #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            // BUG?: Is pwrUp valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            assert(gwPopCnt(qy, cnBitsInD1) != 0);
              #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // ? cnBitsLeftAtDl2 ?
            assert(gwPopCnt(qy, cnBitsInD2) != 0);
              #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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
            int bBitIsSet = BitIsSet(pwr, wKey & MSK(cnBitsLeftAtDl2));
      #else // defined(USE_XX_SW)
            // Might be able to speed this up with bl-specific code.
            int bBitIsSet
                = (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)
                        && (nBLR == cnBitsInD1)
                ? (cnBitsInD1 <= cnLogBitsPerWord)
                    ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsInD1))
                    : BitIsSet(pLn, wKey & MSK(cnBitsInD1))
                // It's faster with this pre-test of nBL then the literal
                // argument of cnBitsInD1 or cnBitsInD2 then using nBL
                // directly. Unfortunately.
          #if 0
                : BitIsSet(pwr, wKey & MSK(nBL))
          #else
                : (cn2dBmMaxWpkPercent != 0) && (nBL == cnBitsLeftAtDl2)
                    ? BitIsSet(pwr, wKey & MSK(cnBitsLeftAtDl2))
                    : BitIsSet(pwr, wKey & MSK(cnBitsLeftAtDl1))
          #endif
                ;
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
      #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                return NULL;
      #else // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                return KeyFound;
      #endif // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
            }
            DBGX(printf("Bit is not set.\n"));
    #endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
  #endif // defined(COUNT)
        }
  #if defined(SKIP_LINKS)
      #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
        else
        {
            // Shouldn't this be using the previous nBL for the pwrUp case?
          #if defined(SKIP_TO_BITMAP)
            // But now that we have prefix in the bitmap can't we use that?
          #endif // defined(SKIP_TO_BITMAP)
            DBGX(printf("Mismatch at bitmap wPrefix " OWx" nBLR %d nBL %d\n",
          #ifdef PP_IN_LINK
                        gwPrefix(qy),
          #else // PP_IN_LINK
                        PWR_wPrefixNATBL(NULL, pwrUp, nBL),
          #endif // PP_IN_LINK
                        nBLR, nBL));
          #if defined(COUNT)
            DBGC(printf("bm wPopCntSum " OWx"\n", wPopCntSum));
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
        // Should we be using nBLR here?
        InsertAtBitmap(qy, wKey);
  #else
        if ((cn2dBmMaxWpkPercent // check that conversion is enabled
            && (EXP(cnBitsInD1) > sizeof(Link_t) * 8) // compiled out
            && (nBLR == cnBitsInD1) // check that conversion is not done
            && (nBL == nBLR) // converting skip to b1 makes no sense
            // this should agree with the test in InsertCleanup
            && (wPopCntUp * cn2dBmMaxWpkPercent
                > EXP(cnBitsLeftAtDl2 - cnLogBitsPerWord) * 100)))
        {
            bCleanupRequested = 1; // goto cleanup when done
        }
  #endif
#endif // defined(INSERT)
        break;

    } // end of case T_BITMAP
#endif // BITMAP

#if defined(EMBED_KEYS)

    case T_EMBEDDED_KEYS:
    {
        goto t_embedded_keys; // suppress compiler unused-label warnings
t_embedded_keys:; // the semi-colon allows for a declaration next; go figure
        assert(EmbeddedListPopCntMax(nBL) != 0);
  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
            return Success;
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

// The pop field in a link with type T_EMBEDDED_KEYS is not the same as
// with other link types. We may be attempting to squeeze every last key
// we can from the memory in the link.
// Hence we may not be able to simply adjust the pop count
// independently. Instead we do no adjustment here and we rely on
// InsertGuts and RemoveGuts to handle it properly despite it being a
// little different.
// We don't support skip to embedded keys yet.

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
            DBGC(printf("EK: wPopCntSum(before) %" _fw"d nn %d\n", wPopCntSum,
                        nn));
            wPopCntSum += nn;
            DBGC(printf("ek nn %d wPopCntSum " OWx"\n", nn, wPopCntSum));
            return wPopCntSum;
        }
  #endif // defined(COUNT)

  #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_DEREF)

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        DBGX(printf("EMBEDDED_KEYS\n"));

      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
      #ifdef LOOKUP
        if (PrefixCheckAtLeaf(qy, wKey
  #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF
  #ifdef SAVE_PREFIX_TEST_RESULT
                , wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
                , pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef SAVE_PREFIX
                , pLnPrefix, pwrPrefix, nBLRPrefix
  #endif // SAVE_PREFIX
                  ) // end call to PrefixCheckAtLeaf
            == Success)
      #endif // LOOKUP
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
      #if    (defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)) || (defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP))

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
break2:;

      #endif // (defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) ... )

      #if (!defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)) || (!defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP))

        int nPopCnt = wr_nPopCnt(wRoot, nBL);

          #if (defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP) && defined(LOOKUP)) || (defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT) && !defined(LOOKUP))

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
        }

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

#if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)

    case T_NULL:
    {
        assert(wRoot == 0);

  // COUNT never gets here so we could probably just use !defined(LOOKUP).
  #if defined(INSERT) || defined(REMOVE)
        if ( bCleanup ) {
      #if defined(INSERT) && defined(B_JUDYL)
            return NULL;
      #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
      #endif // defined(INSERT) && defined(B_JUDYL)
        }
  #endif // defined(INSERT) || defined(REMOVE)

  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #if defined(INSERT) || defined(REMOVE)
        // Adjust wPopCnt in link to leaf if pop cnt is in the link.
        // wPopCnt in link to switch is adjusted elsewhere, i.e. in
        // the same place as wPopCnt in switch is adjusted when pop
        // cnt is in the switch.
        // What about defined(RECURSIVE)?
        if (nBL < cnBitsPerWord) {
            // If nBL < cnBitsPerWord then we're not at top.
            swPopCnt(qy, nBL, gwPopCnt(qy, nBL) + nIncr);
        }
  #endif // defined(INSERT) || defined(REMOVE)
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

        break;

    } // end of case T_NULL

#endif // defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)

    } // end of switch

    // Key is not present.
#if defined(COUNT)
    DBGC(printf("done wPopCntSum " OWx"\n", wPopCntSum));
    return wPopCntSum;
#endif // defined(COUNT)
#if defined(INSERT)
  #if defined(BM_IN_LINK)
    // If InsertGuts calls Insert, then it is always with the same
    // pLn and nBL that Insert passed to InsertGuts.
      #if !defined(RECURSIVE)
    assert((nBL != cnBitsPerWord) || (pLn == pLnOrig));
      #endif // !defined(RECURSIVE)
  #endif // defined(BM_IN_LINK)
    // InsertGuts is called with a pLn and nBL indicates the
    // bits that were not decoded in identifying pLn.  nBL
    // does not include any skip indicated in the type field of *pLn.
  #ifdef B_JUDYL
    pwValue =
  #endif // B_JUDYL
        InsertGuts(qy, wKey, nPos
  #if defined(CODE_XX_SW)
                     , pLnUp
      #if defined(SKIP_TO_XX_SW)
                     , nBLUp
      #endif // defined(SKIP_TO_XX_SW)
  #endif // defined(CODE_XX_SW)
                   );
  #ifdef B_JUDYL
    *pwValue = 0;
  #endif // B_JUDYL
    if (bCleanupRequested) {
        goto cleanup;
    }
  #ifdef B_JUDYL
    DBGX(printf("InsertGuts returned %p\n", (void*)pwValue));
    DBGX(printf("Insert returning %p\n", (void*)pwValue));
    return pwValue;
  #else // B_JUDYL
    return Success;
  #endif // B_JUDYL
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
      #endif // !defined(RECURSIVE)
  #endif // defined(INSERT) || defined(REMOVE)
restart:;
  #if defined(INSERT) || defined(REMOVE)
      #if !defined(RECURSIVE)
        nBL = nBLOrig;
        pLn = pLnOrig;
        wRoot = pLn->ln_wRoot;
        goto top;
    }
      #endif // !defined(RECURSIVE)
  #endif // defined(INSERT) || defined(REMOVE)
  #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
    return NULL;
  #else // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
    return Failure;
  #endif // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
  #if defined(INSERT) || defined(REMOVE)
      #if defined(REMOVE)
removeGutsAndCleanup:;
    RemoveGuts(qy, wKey);
      #endif // defined(REMOVE)
    goto cleanup;
cleanup:;
    // Walk the tree again to see if we need to make any adjustments.
    // For insert our new pop may justify a bigger bitmap.
    // For remove we may need to pull back.
    bCleanup = 1; // ?? nIncr == 0 ??
    DBGX(printf("Cleanup nBLO %d pLnO %p\n", nBLOrig, (void*)pLnOrig));
    // Tree may not be valid yet.
    // It may still have a non-NULL pointer to a switch that has
    // been emptied.
    // Dump may blow an assertion.
    // DBGX(Dump(&pLnOrig->ln_wRoot, /* wPrefix */ (Word_t)0, nBLOrig));
    goto restart;
  #endif // defined(INSERT) || defined(REMOVE)
}

#undef RECURSIVE
#undef InsertGuts
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

#ifdef B_JUDYL
#define Insert  InsertL
#define Remove  RemoveL
#else // B_JUDYL
#define Insert  Insert1
#define Remove  Remove1
#endif // B_JUDYL

#endif // (cnDigitsPerWord <= 1)

#if defined(LOOKUP)

// 'const void * PArray' is not the same as 'void * const PArray'.
// But 'typedef const void * Pcvoid_t' is the same as
//     'typedef void * const Pcvoid_t'.
// 'Pcvoid_t PArray' is the same as 'const void * PArray'.
// *pcvRoot cannot be modified.
#ifdef B_JUDYL
PPvoid_t // typedef void **
JudyLGet(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
#else // B_JUDYL
int // Status_t
Judy1Test(Pcvoid_t pcvRoot, Word_t wKey, PJError_t PJError)
#endif // B_JUDYL
{
#if (cnDigitsPerWord > 1)

    int nBL = cnBitsPerWord;
    Link_t *pLn = STRUCT_OF(&pcvRoot, Link_t, ln_wRoot);
    Word_t wRoot = (Word_t)pcvRoot;
    int nType = wr_nType(wRoot);
    Word_t *pwr = wr_pwr(wRoot);
    qv;

    DBGL(printf("\n\n# JudyLGet pcvRoot %p wKey " OWx"\n",
                (void *)pcvRoot, wKey));

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
    if ((nType == T_LIST)
      #if ! defined(SEPARATE_T_NULL)
        && (pwr != NULL)
      #endif // ! defined(SEPARATE_T_NULL)
        && 1)
    {
      #if defined(B_JUDYL) && !defined(HASKEY_FROM_JUDYL_LOOKUP)
        // PWR_xListPopCount is valid only at the top for PP_IN_LINK.
        // The first word in the list is used for pop count at the top.
        int nPos = SearchListWord(ls_pwKeys(pwr, cnBitsPerWord),
                                  wKey, cnBitsPerWord,
                                  gnListPopCnt(qy, /* nBLR */ nBL));
        return (nPos >= 0) ? (PPvoid_t)&pwr[~nPos] : NULL;
      #else // // defined(B_JUDYL) && !defined(HASKEY_FROM_JUDYL_LOOKUP)
        return ListHasKeyWord(qy, cnBitsPerWord, wKey);
      #endif // defined(B_JUDYL) && !defined(HASKEY_FROM_JUDYL_LOOKUP)
    }
  #endif // defined(SEARCH_FROM_WRAPPER)
  #endif // (cwListPopCntMax != 0)

    return
  #ifdef B_JUDYL
        (PPvoid_t)
  #endif // B_JUDYL
            Lookup(
  #if defined(PLN_PARAM_FOR_LOOKUP)
                   pLn,
  #else // defined(PLN_PARAM_FOR_LOOKUP)
                   wRoot,
  #endif // defined(PLN_PARAM_FOR_LOOKUP)
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

    DBGL(printf("Judy1Test num " OWx" mask " OWx"\n", wByteNum, wByteMask));
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

    DBGL(printf("Judy1Test num " OWx" mask " OWx"\n", wWordNum, wWordMask));
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

// Judy1Set returns Success if the key was not already present.
// Judy1Set returns Failure if the key was already present.
// JudyLIns returns a pointer to the value area if the key was
// not already present, and it initializes the value area to zero.
// JudyLIns returns a pointer to the value area if the key was
// already present, but it leaves the content of the value area alone.
#ifdef B_JUDYL
PPvoid_t // 'typedef void ** PPvoid_t'
JudyLIns(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
#else // B_JUDYL
int // Status_t
Judy1Set(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
#endif // B_JUDYL
{
    Word_t wRoot = *(Word_t*)ppvRoot;

#if (cnDigitsPerWord > 1)

    int nBL = cnBitsPerWord;
    Link_t *pLn = STRUCT_OF(ppvRoot, Link_t, ln_wRoot);
    int nType = wr_nType(wRoot);
    Word_t *pwr = wr_pwr(wRoot);
    qv;

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(((Word_t*)&pLn->ln_wRoot)[-1] == 0);
    assert(((Word_t*)&pLn->ln_wRoot)[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

#ifdef B_JUDYL
    Word_t *pwValue;
#else // B_JUDYL
    int status;
#endif // B_JUDYL

    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey " OWx"\n",
                (void *)ppvRoot, wKey));

#if 0
    // This pre-test causes problems if we are running an experiment
    // that involves returning 1 unconditionally from Lookup.
    if (Judy1Test((Pcvoid_t)pLn->ln_wRoot, wKey, NULL) == Success) {
        return Failure;
    }
#endif

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
                status = InsertGuts(qy, wKey, /* nPos */ -1
#if defined(CODE_XX_SW)
                                  , /* pLnUp */ NULL
  #if defined(SKIP_TO_XX_SW)
                                  , /* nBLUp */ 0
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
                pwKeys = ls_pwKeys(pwr, cnBitsPerWord);

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
  #ifdef B_JUDYL
        pwValue
  #else // B_JUDYL
        status
  #endif // B_JUDYL
            = Insert(cnBitsPerWord,
                     STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey);
    }

  #ifdef B_JUDYL
    // Cannot distinguish between new insert of wKey and
    // previously inserted wKey if *pwValue == 0.
    // If we are running Judy1LHTime we know that *pwValue will never be
    // zero for a previously inserted key (unless wKey == 0).
    bPopCntTotalIsInvalid = 1;
  #endif // B_JUDYL

  #ifdef B_JUDYL
    if (*pwValue == 0)
  #else // B_JUDYL
    if (status == Success)
  #endif // B_JUDYL
    {
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

    DBGI(printf("\n# After Insert(wKey " OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGI(printf("\n"));

  #if ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)
  #if ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
    // Judy1Count really slows down testing for PP_IN_LINK.
      #ifdef B_JUDYL
    assert((JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal)
        || bPopCntTotalIsInvalid);
      #else // B_JUDYL
    //assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
      #endif // B_JUDYL
  #endif // ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
  #endif // ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(((Word_t*)&pLn->ln_wRoot)[-1] == 0);
    assert(((Word_t*)&pLn->ln_wRoot)[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

  #ifdef B_JUDYL
    DBGI(printf("# JudyLIns ppvRoot %p wKey 0x%zx returning %p,0x%zx\n",
                (void*)ppvRoot, wKey, (void*)pwValue, *pwValue));
    return (PPvoid_t)pwValue;
  #else // B_JUDYL
    return status;
  #endif // B_JUDYL

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    Word_t wByteNum, wByteMask;
    char c;

    DBGI(printf("\n# Judy1Set(ppvRoot %p wKey " OWx") wRoot " OWx"\n",
        (void *)ppvRoot, wKey, wRoot));

    if (wRoot == 0)
    {
        wRoot = JudyMalloc(EXP(cnBitsPerWord - cnLogBitsPerWord));
        assert(wRoot != 0);
        assert((wRoot & cnMallocMask) == 0);

        DBGI(printf("Malloc wRoot " OWx"\n", wRoot));

        *ppvRoot = (PPvoid_t)wRoot;
    }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);

    DBGI(printf("Judy1Set num " OWx" mask " OWx"\n", wByteNum, wByteMask));

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
#ifdef B_JUDYL
JudyLDel(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
#else // B_JUDYL
Judy1Unset(PPvoid_t ppvRoot, Word_t wKey, PJError_t PJError)
#endif // B_JUDYL
{
    //Word_t wRoot = *(Word_t*)ppvRoot;

#if (cnDigitsPerWord > 1)

    Link_t *pLn = STRUCT_OF(ppvRoot, Link_t, ln_wRoot);

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(((Word_t*)&pLn->ln_wRoot)[-1] == 0);
    assert(((Word_t*)&pLn->ln_wRoot)[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    DBGR(printf("\n# Judy1Unset ppvRoot %p  wKey 0x%zx (before):"
                  " wPopCntTotal %zd\n",
                (void*)ppvRoot, wKey, wPopCntTotal));

    int status;

    DBGR(printf("\n\n# Judy1Unset ppvRoot %p wKey " OWx"\n",
                (void *)ppvRoot, wKey));

  #ifdef B_JUDYL
    if (JudyLGet((Pcvoid_t)pLn->ln_wRoot, wKey, NULL) == NULL)
  #else // B_JUDYL
    if (Judy1Test((Pcvoid_t)pLn->ln_wRoot, wKey, NULL) != Success)
  #endif // B_JUDYL
    {
        return Failure;
    }

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
                set_wr(wRoot, pwListNew, T_LIST);
                pwKeysNew = ls_pwKeys(pwListNew, cnBitsPerWord);

                Word_t *pwKeys = ls_pwKeys(pwr, cnBitsPerWord);

 // Isn't this chunk of code already in RemoveGuts?
                unsigned nn;
                for (nn = 0; pwKeys[nn] != wKey; nn++) { }
                COPY(pwKeysNew, pwKeys, nn);
                COPY(&pwKeysNew[nn], &pwKeys[nn + 1], wPopCnt - nn - 1);
      #if defined(LIST_END_MARKERS)
                // pwKeysNew incorporates top pop count and markers
                pwKeysNew[wPopCnt - 1] = -1;
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
        status = Remove(cnBitsPerWord, pLn, wKey);
    }

    if (status == Success) { wPopCntTotal--; }

  #if defined(DEBUG_REMOVE)
    DBGR(printf("\n# After Remove(ppvRoot %p, wKey " OWx") %s Dump\n",
                (void *)ppvRoot, wKey,
                status == Success ? "Success" : "Failure"));
    DBGR(Dump((Word_t *)ppvRoot, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    DBGR(printf("\n"));
  #endif // defined(DEBUG_REMOVE)

  #if ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)
  #if ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
    // Judy1Count really slows down testing for PP_IN_LINK.
      #ifdef B_JUDYL
    if (JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL) != wPopCntTotal)
      #else // B_JUDYL
    if (Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) != wPopCntTotal)
      #endif // B_JUDYL
    {
        DBGR(printf("wPopCntTotal %zd Judy1Count %zd\n",
                    wPopCntTotal,
      #ifdef B_JUDYL
                    Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL)
      #else // B_JUDYL
                    JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL)
      #endif // B_JUDYL
                    ));
    }
      #ifdef B_JUDYL
    assert(JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL)
        || bPopCntTotalIsInvalid);
      #else // B_JUDYL
    //assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
      #endif // B_JUDYL
  #endif // ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
  #endif // ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)
    DBGR(printf("# Judy1Unset (after ): wPopCntTotal %zd\n\n", wPopCntTotal));

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(((Word_t*)&pLn->ln_wRoot)[-1] == 0);
    assert(((Word_t*)&pLn->ln_wRoot)[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

    DBGR(printf("JudyLDel ppvRoot %p wKey 0x%zx status %d\n",
                (void*)ppvRoot, wKey, status));
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
