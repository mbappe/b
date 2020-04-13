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

static void
Checkpoint(qp, const char *str)
{
    qv;
    printf("# %20s: " qfmt "\n", str, qyp);
}

  #if defined(COUNT)
// Return the total number of keys in the subtrees rooted by links that
// precede the specified link in this switch.
static Word_t
CountSw(qp,
        int nBLR, // number of bits left to decode by switch and below
        int nBW, // number of bits decoded by the switch
        Word_t wIndex, // index of relevant link in switch
        int nLinks)
{
    qv; (void)nBLR; (void)nLinks;
    assert(wRoot != WROOT_NULL);
    DBGC(printf("\n# CountSw nType %d nBLR %d nBW %d wIndex " OWx"\n",
                nType, nBLR, nBW, wIndex));
    Word_t wPopCnt = 0;
    Word_t ww, wwLimit;
    Link_t *pLinks =
#if defined(CODE_BM_SW)
        tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
#if defined(CODE_LIST_SW)
        tp_bIsListSw(nType) ? gpListSwLinks(qy) :
#endif // defined(CODE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr);
  #ifdef XX_LISTS
    int nBLLoop = nBLR - nBW;
    Link_t *pLnLoop = &pLinks[wIndex];
    Word_t* pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
    Word_t wRootLoop = *pwRootLoop;
    int nTypeLoop = wr_nType(wRootLoop);
    if (nTypeLoop == T_XX_LIST) {
        int nBLRLoop = gnListBLR(qyx(Loop));
        wIndex &= ~MSK(nBLRLoop - nBLLoop);
DBGC(printf("# CountSw wIndex " OWx"\n", wIndex));
    }
  #endif // XX_LISTS
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
    wPopCnt = CountSwLoop(nBLR - nBW, &pLinks[ww], wwLimit - ww);
//printf("# CountSwLoop returned %zd\n", wPopCnt);
    assert(((int)wIndex < nLinks)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
               || (nBL == cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
           );
    if (ww != 0) {
        assert(wRoot != WROOT_NULL);
        Word_t wPopCntSw = gwPopCnt(qy, nBLR);
        DBGC(printf("wPopCntSw 0x%zx wPopCnt 0x%zx\n", wPopCntSw, wPopCnt));
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

  #if !defined(LOOKUP)
    #define DO_PREFIX_TEST
  #endif // !defined(LOOKUP)
  #if !defined(SKIP_PREFIX_CHECK)
    #define DO_PREFIX_TEST
  #endif // !defined(SKIP_PREFIX_CHECK)
  #if defined(SAVE_PREFIX_TEST_RESULT)
    #define DO_PREFIX_TEST
  #endif // defined(SAVE_PREFIX_TEST_RESULT)

  #ifdef DO_PREFIX_TEST
    Word_t wPrefix =
        0 ? 0
      #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
        : (nBL == cnBitsPerWord) ? 0
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
      #if defined(SKIP_TO_BITMAP)
        : (wr_nType(pLn->ln_wRoot) == T_SKIP_TO_BITMAP)
            ? gwBitmapPrefix(qy, nBLR)
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
  #endif // DO_PREFIX_TEST
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

#if defined(INSERT) || defined(REMOVE)
// Handle bigger picture tree cleanup.
// E.g.
// - uncompress bm sw after insert
// - create 2-digit bitmap leaf after insert
// - compress switch after remove
// - break up 2-digit bitmap leaf after remove
static inline int
SwCleanup(qpa, Word_t wKey, int nBLR
  #if defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
        , Word_t **ppwValue
  #endif // defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
          )
{
    qva; (void)wKey; (void)nBLR;
    DBGX(Checkpoint(qy, "SwCleanup"));
    DBGX(printf("wKey 0x%016zx nBLR %d\n", wKey, nBLR));
    // Cleanup is for adjusting tree after successful insert or remove.
    // It is not for undoing counts after unsuccessful insert or remove.
  #if defined(INSERT)
    if (0
      #if defined(CODE_BM_SW)
        || (tp_bIsBmSw(Get_nType(&wRoot)) && InflateBmSwTest(qy))
      #endif // defined(CODE_BM_SW)
      #ifdef BITMAP
        || ((cn2dBmMaxWpkPercent != 0)
            && (nBLR == cnBitsLeftAtDl2)
            && (GetPopCnt(pwRoot, nBL) * cn2dBmMaxWpkPercent * cnBitsPerWord
                 > EXP(cnBitsLeftAtDl2) * 100))
      #endif // BITMAP
        )
    {
      #if defined(B_JUDYL) && defined(EMBED_KEYS)
        // InsertCleanup may change pwValue of embedded keys.
        Word_t *pwValue;
        if ((pwValue = InsertCleanup(qya, wKey)) != NULL) {
            *ppwValue = pwValue;
        }
      #else // defined(B_JUDYL) && defined(EMBED_KEYS)
        InsertCleanup(qya, wKey);
      #endif // defined(B_JUDYL) && defined(EMBED_KEYS)
    }
  #else // defined(INSERT)
    RemoveCleanup(wKey, nBL, nBLR, pwRoot, wRoot);
  #endif // defined(INSERT)
    if (pLn->ln_wRoot != wRoot) { return 1; /* not done; goto restart */ }
    DBGX(printf("SwCleanup returning 0\n"));
    return 0;
}
#endif // defined(INSERT) || defined(REMOVE)

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
    *pnBLR = *pnBL;
    // Be very careful with pwRoot from pqp.
    // It might not mean what you think it means.
  #ifdef QP_PLN
    *ppLn = pLnNew;
    *pwRoot = pLnNew->ln_wRoot;
    DBGX(printf("sw nBL %d pLn %p wRoot 0x%zx\n",
                *pnBL, (void*)*ppLn, *pwRoot));
  #else // QP_PLN
    *ppwRoot = &pLnNew->ln_wRoot;
    *pwRoot = **ppwRoot;
    DBGX(printf("sw nBL %d pwRoot %p wRoot 0x%zx\n",
                *pnBL, *ppwRoot, *pwRoot));
  #endif // QP_PLN else
    // Why aren't we updating *pnType and *ppwr here?
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

#ifdef AUGMENT_TYPE
#ifdef LOOKUP

static int
AugTypeBits(int nBL)
{
  #ifdef AUGMENT_TYPE_8
    // atb = (nBL << 1) - 16
    return (nBL << 1) - 16;
  #else // AUGMENT_TYPE_8
    // nLogBytesLeft = LOG(nBL - 1) - 2
    // atb = nLogBytesLeft << 4
    return (LOG(nBL - 1) - 2) << 4; // nLogBytesLeft << 4
  #endif // else AUGMENT_TYPE_8
}

static int
AugTypeBitsInv(int nAugTypeBits)
{
    assert(!(nAugTypeBits & cnMallocMask));
  #ifdef AUGMENT_TYPE_8
    // nBL = (atb + 16) >> 1
    return (nAugTypeBits + 16) >> 1; // nBL
  #else // AUGMENT_TYPE_8
    // nLogBytesLeft = atb >> 4
    return nAugTypeBits >> 4; // nLogBytesLeft;
  #endif // else AUGMENT_TYPE_8
}

#endif // LOOKUP
#endif // AUGMENT_TYPE

  #if defined(LOOKUP)
      #ifdef B_JUDYL
static Word_t *
      #else // B_JUDYL
static Status_t
      #endif // B_JUDYL
Lookup(Word_t wRoot, Word_t wKey)
  #else // defined(LOOKUP)
      #if defined(COUNT)
Word_t
      #elif defined(INSERT) && defined(B_JUDYL)
Word_t *
      #else // defined(COUNT) elif defined(INSERT) && defined(JUDYL)
Status_t
      #endif // defined(COUNT)
      #ifdef B_JUDYL
InsertRemoveL(qp, Word_t wKey)
      #else // B_JUDYL
InsertRemove1(qp, Word_t wKey)
      #endif // B_JUDYL
  #endif // defined(LOOKUP)
{
  #ifdef LOOKUP
      #ifdef QP_PLN
    Link_t *pLn = STRUCT_OF(&wRoot, Link_t, ln_wRoot);
      #else // QP_PLN
    Word_t* pwRoot = &wRoot;
      #endif // QP_PLN else
  #else // LOOKUP
      #ifdef QP_PLN
    Word_t wRoot = pLn->ln_wRoot;
      #else // QP_PLN
    Word_t wRoot = *pwRoot;
      #endif // QP_PLN else
  #endif // #else LOOKUP
  #ifndef QP_PLN
    Link_t* pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot);
  #endif // !QP_PLN
    // pLn and wRoot of qy are set up
    DBGX(printf("\n# %s pLn %p wRoot 0x%zx wKey 0x%zx\n",
                strLookupOrInsertOrRemove, (void*)pLn, wRoot, wKey));

#if defined(LOOKUP)
    int nBL = cnBitsPerWord;
#endif // defined(LOOKUP)
    // nBL, pLn and wRoot of qy are set up

#if defined(INSERT) && defined(B_JUDYL)
    Word_t *pwValue = NULL;
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

    Link_t *pLnUp = NULL; (void)pLnUp;

    // nBLUp is used only for CODE_XX_SW and INSERT.
    // I think it will eventually be used for REMOVE.
    int nBLUp = cnBitsPerWord; (void)nBLUp; // silence gcc
    // gcc complains that nBLUp may be used uninitialized with CODE_XX_SW.

    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
#if defined(SAVE_PREFIX_TEST_RESULT)
    Word_t wPrefixMismatch = 0; (void)wPrefixMismatch;
#endif // defined(SAVE_PREFIX_TEST_RESULT)
  #ifdef _LNX
    Word_t* pwLnX = NULL; (void)pwLnX;
      #ifndef LOOKUP
      #ifndef REMOTE_LNX
    // We should leave pwLnX NULL for nBL == cnBitsPerWord, but the code
    // is not supposed to be using it in that case so we skip the test.
    // I guess it is not quite as important as it would be if we were in
    // LOOKUP here, but we're not.
    pwLnX = &STRUCT_OF(pwRoot, Link_t, ln_wRoot)->ln_wX;
      #endif // ifndef REMOTE_LNX
      #endif // ifndef LOOKUP
  #endif // _LNX
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

    // nBL, pLn and wRoot of qy are set up
    int nType;
    DBGX(nType = -1); // for compiler for qv in Log
    Word_t *pwr;
    DBGX(pwr = NULL); // for compiler for qv in Log
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
    // nBL, pLn, wRoot, nType and pwr of qy are set up
    if (nType > T_SWITCH) {
        DBGX(Checkpoint(qy, "goto skip_to_sw"));
        goto t_skip_to_switch;
    }

    // This shortcut made the code faster in my testing.
    nBLR = nBL;
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
    goto top;
    DBGX(Checkpoint(qy, "top"));
top:;
  #endif // !defined(RECURSIVE)
#endif // defined(INSERT) || defined(REMOVE)
    nBLR = nBL;

#if defined(LOOKUP) || !defined(RECURSIVE)
    goto again;
    DBGX(Checkpoint(qy, "again"));
again:;
#endif // defined(LOOKUP) || !defined(RECURSIVE)

#if defined(SKIP_LINKS)
    assert(nBLR == nBL);
#endif // defined(SKIP_LINKS)
#if ( ! defined(LOOKUP) )
  #if ! defined(CODE_XX_SW)
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
  #endif // ! defined(CODE_XX_SW)
#endif // ( ! defined(LOOKUP) )

   #ifdef INSERT
   #ifdef _RETURN_NULL_TO_INSERT_AGAIN
BJL(insertAgain:)
   #endif // _RETURN_NULL_TO_INSERT_AGAIN
   #endif // INSERT
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot); // pwr isn't meaningful for all nType values
    // nBL, pLn, wRoot, nType and pwr of qy are set up
    DBGX(Checkpoint(qy, "enter switch stmt"));

    goto fastAgain;
fastAgain:;
  #if defined(JUMP_TABLE)
    static
      #ifdef JUMP_TABLE_CONST
            const
      #endif // JUMP_TABLE_CONST
                void *pvJumpTable[]
      #ifdef JUMP_TABLE_TEXT
                    __attribute__ ((section (".text#")))
      #elif defined(JUMP_TABLE_DATA) // JUMP_TABLE_TEXT
                    __attribute__ ((section (".data#")))
      #endif // JUMP_TABLE_TEXT elif JUMP_TABLE_DATA
        = {
      #if defined(SEPARATE_T_NULL)
            &&t_null,
      #endif // defined(SEPARATE_T_NULL)
      #if (cwListPopCntMax != 0)
            &&t_list,
          #ifdef XX_LISTS
            &&t_xx_list,
          #endif // XX_LISTS
          #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
          #endif // defined(SKIP_TO_LIST)
          #if defined(UA_PARALLEL_128)
            &&t_list_ua,
          #endif // defined(UA_PARALLEL_128)
      #endif // (cwListPopCntMax != 0)
      #ifdef BITMAP
            &&t_bitmap,
          #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
          #endif // defined(SKIP_TO_BITMAP)
          #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
          #endif // UNPACK_BM_VALUES
      #endif // BITMAP
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
      #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
      #if defined(EMBED_KEYS)
            &&t_embedded_keys,
          #ifdef EK_XV
            &&t_ek_xv,
          #endif // EK_XV
      #endif // defined(EMBED_KEYS)
            &&t_switch,
      #if defined(SKIP_LINKS)
            // T_SKIP_TO_SWITCH is an enumeration constant; not a macro.
            // It's value is treated as zero by the preprocessor.
            &&t_skip_to_switch,
          #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
          #endif // _LVL_IN_TYPE
      #endif // defined(SKIP_LINKS)
      #ifdef LOOKUP
      #ifdef AUGMENT_TYPE
            [16] = // two eight-bit digits left
          #if defined(SEPARATE_T_NULL)
            &&t_null,
          #endif // defined(SEPARATE_T_NULL)
          #if (cwListPopCntMax != 0)
            &&t_list16,
              #ifdef XX_LISTS
            &&t_xx_list,
              #endif // XX_LISTS
              #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
              #endif // defined(SKIP_TO_LIST)
              #if defined(UA_PARALLEL_128)
            &&t_list_ua,
              #endif // defined(UA_PARALLEL_128)
          #endif // (cwListPopCntMax != 0)
          #ifdef BITMAP
            &&t_bitmap,
              #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
              #endif // defined(SKIP_TO_BITMAP)
              #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
              #endif // UNPACK_BM_VALUES
          #endif // BITMAP
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
          #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
          #if defined(EMBED_KEYS)
            &&t_embedded_keys,
              #ifdef EK_XV
            &&t_ek_xv,
              #endif // EK_XV
          #endif // defined(EMBED_KEYS)
            &&t_sw_plus_16,
          #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
              #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
              #endif // _LVL_IN_TYPE
          #endif // defined(SKIP_LINKS)
            // three eight-bit digits left (also four eight-bit digits left
            // for AUGMENT_TYPE_8
            [32] =
          #if defined(SEPARATE_T_NULL)
            &&t_null,
          #endif // defined(SEPARATE_T_NULL)
          #if (cwListPopCntMax != 0)
            &&t_list32,
              #ifdef XX_LISTS
            &&t_xx_list,
              #endif // XX_LISTS
              #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
              #endif // defined(SKIP_TO_LIST)
              #if defined(UA_PARALLEL_128)
            &&t_list_ua,
              #endif // defined(UA_PARALLEL_128)
          #endif // (cwListPopCntMax != 0)
          #ifdef BITMAP
            &&t_bitmap,
              #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
              #endif // defined(SKIP_TO_BITMAP)
              #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
              #endif // UNPACK_BM_VALUES
          #endif // BITMAP
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
          #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
          #if defined(EMBED_KEYS)
            &&t_embedded_keys,
              #ifdef EK_XV
            &&t_ek_xv,
              #endif // EK_XV
          #endif // defined(EMBED_KEYS)
            &&t_sw_plus_32,
          #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
              #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
              #endif // _LVL_IN_TYPE
          #endif // defined(SKIP_LINKS)
            [48] =
          #if defined(SEPARATE_T_NULL)
            &&t_null,
          #endif // defined(SEPARATE_T_NULL)
          #if (cwListPopCntMax != 0)
              #ifdef AUGMENT_TYPE_8
            &&t_list32,
              #else // AUGMENT_TYPE_8
            &&t_listWord,
              #endif // AUGMENT_TYPE_8 else
              #ifdef XX_LISTS
            &&t_xx_list,
              #endif // XX_LISTS
              #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
              #endif // defined(SKIP_TO_LIST)
              #if defined(UA_PARALLEL_128)
            &&t_list_ua,
              #endif // defined(UA_PARALLEL_128)
          #endif // (cwListPopCntMax != 0)
          #ifdef BITMAP
            &&t_bitmap,
              #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
              #endif // defined(SKIP_TO_BITMAP)
              #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
              #endif // UNPACK_BM_VALUES
          #endif // BITMAP
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
          #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
          #if defined(EMBED_KEYS)
            &&t_embedded_keys,
              #ifdef EK_XV
            &&t_ek_xv,
              #endif // EK_XV
          #endif // defined(EMBED_KEYS)
            &&t_sw_plus_48,
          #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
              #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
              #endif // _LVL_IN_TYPE
          #endif // defined(SKIP_LINKS)
          #ifdef AUGMENT_TYPE_8
            [64] =
              #if defined(SEPARATE_T_NULL)
            &&t_null,
              #endif // defined(SEPARATE_T_NULL)
              #if (cwListPopCntMax != 0)
            &&t_listWord,
                  #ifdef XX_LISTS
            &&t_xx_list,
                  #endif // XX_LISTS
                  #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
                  #endif // defined(SKIP_TO_LIST)
                  #if defined(UA_PARALLEL_128)
            &&t_list_ua,
                  #endif // defined(UA_PARALLEL_128)
              #endif // (cwListPopCntMax != 0)
              #ifdef BITMAP
            &&t_bitmap,
                  #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
                  #endif // defined(SKIP_TO_BITMAP)
                  #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
                  #endif // UNPACK_BM_VALUES
              #endif // BITMAP
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
              #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
              #if defined(EMBED_KEYS)
            &&t_embedded_keys,
                  #ifdef EK_XV
            &&t_ek_xv,
                  #endif // EK_XV
              #endif // defined(EMBED_KEYS)
            &&t_sw_plus_64,
              #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
                  #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
                  #endif // _LVL_IN_TYPE
              #endif // defined(SKIP_LINKS)
            [80] =
              #if defined(SEPARATE_T_NULL)
            &&t_null,
              #endif // defined(SEPARATE_T_NULL)
              #if (cwListPopCntMax != 0)
            &&t_listWord,
                  #ifdef XX_LISTS
            &&t_xx_list,
                  #endif // XX_LISTS
                  #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
                  #endif // defined(SKIP_TO_LIST)
                  #if defined(UA_PARALLEL_128)
            &&t_list_ua,
                  #endif // defined(UA_PARALLEL_128)
              #endif // (cwListPopCntMax != 0)
              #ifdef BITMAP
            &&t_bitmap,
                  #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
                  #endif // defined(SKIP_TO_BITMAP)
                  #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
                  #endif // UNPACK_BM_VALUES
              #endif // BITMAP
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
              #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
              #if defined(EMBED_KEYS)
            &&t_embedded_keys,
                  #ifdef EK_XV
            &&t_ek_xv,
                  #endif // EK_XV
              #endif // defined(EMBED_KEYS)
            &&t_sw_plus_80,
              #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
                  #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
                  #endif // _LVL_IN_TYPE
              #endif // defined(SKIP_LINKS)
            [96] =
              #if defined(SEPARATE_T_NULL)
            &&t_null,
              #endif // defined(SEPARATE_T_NULL)
              #if (cwListPopCntMax != 0)
            &&t_listWord,
                  #ifdef XX_LISTS
            &&t_xx_list,
                  #endif // XX_LISTS
                  #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
                  #endif // defined(SKIP_TO_LIST)
                  #if defined(UA_PARALLEL_128)
            &&t_list_ua,
                  #endif // defined(UA_PARALLEL_128)
              #endif // (cwListPopCntMax != 0)
              #ifdef BITMAP
            &&t_bitmap,
                  #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
                  #endif // defined(SKIP_TO_BITMAP)
                  #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
                  #endif // UNPACK_BM_VALUES
              #endif // BITMAP
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
              #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
              #if defined(EMBED_KEYS)
            &&t_embedded_keys,
                  #ifdef EK_XV
            &&t_ek_xv,
                  #endif // EK_XV
              #endif // defined(EMBED_KEYS)
            &&t_sw_plus_96,
              #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
                  #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
                  #endif // _LVL_IN_TYPE
              #endif // defined(SKIP_LINKS)
            [112] =
              #if defined(SEPARATE_T_NULL)
            &&t_null,
              #endif // defined(SEPARATE_T_NULL)
              #if (cwListPopCntMax != 0)
            &&t_listWord,
                  #ifdef XX_LISTS
            &&t_xx_list,
                  #endif // XX_LISTS
                  #if defined(SKIP_TO_LIST)
            &&t_skip_to_list,
                  #endif // defined(SKIP_TO_LIST)
                  #if defined(UA_PARALLEL_128)
            &&t_list_ua,
                  #endif // defined(UA_PARALLEL_128)
              #endif // (cwListPopCntMax != 0)
              #ifdef BITMAP
            &&t_bitmap,
                  #if defined(SKIP_TO_BITMAP)
            &&t_skip_to_bitmap,
                  #endif // defined(SKIP_TO_BITMAP)
                  #ifdef UNPACK_BM_VALUES
            &&t_unpacked_bm,
                  #endif // UNPACK_BM_VALUES
              #endif // BITMAP
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
              #endif // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
              #if defined(EMBED_KEYS)
            &&t_embedded_keys,
                  #ifdef EK_XV
            &&t_ek_xv,
                  #endif // EK_XV
              #endif // defined(EMBED_KEYS)
            &&t_sw_plus_112,
              #if defined(SKIP_LINKS)
            &&t_skip_to_switch,
                  #ifdef _LVL_IN_TYPE
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
            &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch,
                  #endif // _LVL_IN_TYPE
              #endif // defined(SKIP_LINKS)
          #endif // AUGMENT_TYPE_8
      #endif // AUGMENT_TYPE
      #endif // LOOKUP
    };
    //__builtin_prefetch(0, 1);
  // AUGMENT_TYPE_NOT means create jump table entries or switch table cases
  // according to AUGMENT_TYPE and AUGMENT_TYPE_8 but don't do the work of
  // augmenting the type or jump or switch based on an augmented type.
  // AUGMENT_TYPE_8 with AUGMENT_TYPE assumes the low three bits of nBL are
  // always zero and uses the high three bits of nBL to augment the type.
  // for eight different legal nBL values: 8, 16, 24, 32, 40, 48, 56, 64.
  // How expensive would it be to maintain nDL as well as nBL?
  // AUGMENT_TYPE without AUGMENT_TYPE_8 has four different nBL groups:
  // 5-8, 9-16, 17-32, 33-64. 0-4 does not work.
  #if defined(AUGMENT_TYPE_8) && !defined(AUGMENT_TYPE_NOT) && defined(LOOKUP)
    goto *pvJumpTable[(AugTypeBits(nBL) | nType)
      #ifdef MASK_TYPE
                          & 0x7f // help compiler
      #endif // MASK_TYPE
                      ];
  #elif defined(AUGMENT_TYPE) && !defined(AUGMENT_TYPE_NOT) && defined(LOOKUP)
    goto *pvJumpTable[(AugTypeBits(nBL) | nType)
      #ifdef MASK_TYPE
                          & 0x3f // help compiler
      #endif // MASK_TYPE
                      ];
  #else // AUG_8 && !AUG_NOT && LOOKUP elif AUG && !AUG_NOT && LOOKUP
    goto *pvJumpTable[nType];
  #endif // AUG_8 && !AUG_NOT && LOOKUP elif AUG && !AUG_NOT && LOOKUP else
  #endif // defined(JUMP_TABLE)

    //__builtin_prefetch(0, 1);
  #ifdef AUGMENT_TYPE_8
    #ifndef AUGMENT_TYPE
      #error AUGMENT_TYPE_8 without AUGMENT_TYPE
    #endif // !AUGMENT_TYPE
  #endif // AUGMENT_TYPE_8
  #if defined(AUGMENT_TYPE_8) && !defined(AUGMENT_TYPE_NOT) && defined(LOOKUP)
    switch ((AugTypeBits(nBL) | nType)
      #ifdef MASK_TYPE
                & 0x7f // help compiler
      #endif // MASK_TYPE
            )
  #elif defined(AUGMENT_TYPE) && !defined(AUGMENT_TYPE_NOT) && defined(LOOKUP)
    switch ((AugTypeBits(nBL) | nType)
      #ifdef MASK_TYPE
                & 0x3f // help compiler
      #endif // MASK_TYPE
            )
  #else // AUG_8 && !AUG_NOT && LOOKUP elif AUG && !AUG_NOT && LOOKUP
    switch (nType)
  #endif // AUG_8 && !AUG_NOT && LOOKUP elif AUG && !AUG_NOT && LOOKUP else
    {

  // At most one of DEFAULT_SKIP_TO_SW, DEFAULT_SWITCH,
  // DEFAULT_LIST and DEFAULT_BITMAP may be defined.
  #ifdef DEBUG
  #ifndef DEFAULT_SKIP_TO_SW
  #ifndef DEFAULT_SWITCH
  #ifndef DEFAULT_LIST
  #ifndef DEFAULT_BITMAP
  #if !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
    default: DBG(printf("unknown type %d nBL %d\n", nType, nBL)); assert(0);
  #endif // !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
  #endif // DEFAULT_BITMAP
  #endif // DEFAULT_LIST
  #endif // DEFAULT_SWITCH
  #endif // DEFAULT_SKIP_TO_SW
  #endif // DEBUG

#if defined(SKIP_LINKS)

  #if !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
  #if !defined(DEFAULT_SKIP_TO_SW) && !defined(ALL_SKIP_TO_SW_CASES)
    // For level in type, i.e. (!LVL_IN_WR_HB && !LVL_IN_PP),
    // multiple type values all represent T_SKIP_TO_SWITCH, i.e.
    // level = nType - T_SKIP_TO_SWITCH + 2.
    #error Level in type requires DEFAULT_SKIP_TO_SW or ALL_SKIP_TO_SW_CASES.
  #endif // !defined(LVL_IN_WR_HB) && !defined(LVL_IN_PP)
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
  #ifdef ALL_SKIP_TO_SW_CASES
    // Extra cases so we have at least EXP(cnBitsMallocMask) cases so gcc
    // will create a jump table with no bounds check at the beginning after
    // extracting nType from wRoot (for !AUGMENT_TYPE).
    // We have not coded a no-bounds-check version without SKIP_LINKS yet.
    // It's ok to create extra cases if !AUGMENT_TYPE, but we can't get by
    // so easily with AUGMENT_TYPE so we use a clunky method of having to
    // define NUM_TYPES.
      #ifdef AUGMENT_TYPE
      #if NUM_TYPES <= 15
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH+1:
    case  96 + T_SKIP_TO_SWITCH+1:
    case  80 + T_SKIP_TO_SWITCH+1:
    case  64 + T_SKIP_TO_SWITCH+1:
          #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH+1:
    case 32 + T_SKIP_TO_SWITCH+1:
    case 16 + T_SKIP_TO_SWITCH+1:
          #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
      #endif // NUM_TYPES <= 15
      #endif // AUGMENT_TYPE
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 15
    case T_SKIP_TO_SWITCH+1:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 15
      #ifdef AUGMENT_TYPE
      #if NUM_TYPES <= 14
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH+2:
    case  96 + T_SKIP_TO_SWITCH+2:
    case  80 + T_SKIP_TO_SWITCH+2:
    case  64 + T_SKIP_TO_SWITCH+2:
          #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH+2:
    case 32 + T_SKIP_TO_SWITCH+2:
    case 16 + T_SKIP_TO_SWITCH+2:
          #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
      #endif // NUM_TYPES <= 14
      #endif // AUGMENT_TYPE
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 14
    case T_SKIP_TO_SWITCH+2:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 14
      #ifdef AUGMENT_TYPE
      #if NUM_TYPES <= 13
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH+3:
    case  96 + T_SKIP_TO_SWITCH+3:
    case  80 + T_SKIP_TO_SWITCH+3:
    case  64 + T_SKIP_TO_SWITCH+3:
          #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH+3:
    case 32 + T_SKIP_TO_SWITCH+3:
    case 16 + T_SKIP_TO_SWITCH+3:
          #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
      #endif // NUM_TYPES <= 13
      #endif // AUGMENT_TYPE
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 13
    case T_SKIP_TO_SWITCH+3:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 13
      #ifdef AUGMENT_TYPE
      #if NUM_TYPES <= 12
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH+4:
    case  96 + T_SKIP_TO_SWITCH+4:
    case  80 + T_SKIP_TO_SWITCH+4:
    case  64 + T_SKIP_TO_SWITCH+4:
          #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH+4:
    case 32 + T_SKIP_TO_SWITCH+4:
    case 16 + T_SKIP_TO_SWITCH+4:
          #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
      #endif // NUM_TYPES <= 12
      #endif // AUGMENT_TYPE
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 12
    case T_SKIP_TO_SWITCH+4:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 12
      #ifdef AUGMENT_TYPE
      #if NUM_TYPES <= 11
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH+5:
    case  96 + T_SKIP_TO_SWITCH+5:
    case  80 + T_SKIP_TO_SWITCH+5:
    case  64 + T_SKIP_TO_SWITCH+5:
          #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH+5:
    case 32 + T_SKIP_TO_SWITCH+5:
    case 16 + T_SKIP_TO_SWITCH+5:
          #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
      #endif // NUM_TYPES <= 11
      #endif // AUGMENT_TYPE
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 11
    case T_SKIP_TO_SWITCH+5:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 11
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 10
    case T_SKIP_TO_SWITCH+6:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 10
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 9
    case T_SKIP_TO_SWITCH+7:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 9
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 8
    case T_SKIP_TO_SWITCH+8:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 8
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 7
    case T_SKIP_TO_SWITCH+9:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 7
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 6
    case T_SKIP_TO_SWITCH+10:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 6
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 5
    case T_SKIP_TO_SWITCH+11:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 5
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 4
    case T_SKIP_TO_SWITCH+12:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 4
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 3
    case T_SKIP_TO_SWITCH+13:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 3
  #endif // ALL_SKIP_TO_SW_CASES
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
      #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_SWITCH:
    case  96 + T_SKIP_TO_SWITCH:
    case  80 + T_SKIP_TO_SWITCH:
    case  64 + T_SKIP_TO_SWITCH:
      #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
      #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_SWITCH:
    case 32 + T_SKIP_TO_SWITCH:
    case 16 + T_SKIP_TO_SWITCH:
      #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_SKIP_TO_SWITCH: // skip link to uncompressed switch
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_skip_to_switch; // silence cc in case there are no other uses
t_skip_to_switch:
        // Skip to switch.
        // pwr points to a switch
  #if defined(NO_PREFIX_CHECK) && defined(LOOKUP)
        nBLR = gnBLRSkip(qy);
  #else // defined(NO_PREFIX_CHECK) && defined(LOOKUP)
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
        DBG((nType != T_SKIP_TO_SWITCH) ? printf("\nnType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
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
                    //int nBW = nBL_to_nBWNAX(nBLR);
                    int nBW = nBL_to_nBWNAB(nBLR);
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW,
                                      EXP(nBW), EXP(nBW));
                } else
          #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
          #endif // ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
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

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_XX_SW:
    case  96 + T_SKIP_TO_XX_SW:
    case  80 + T_SKIP_TO_XX_SW:
    case  64 + T_SKIP_TO_XX_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_XX_SW:
    case 32 + T_SKIP_TO_XX_SW:
    case 16 + T_SKIP_TO_XX_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
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
                    int nBW = nBL_to_nBWNAB(nBLR);
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
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SWITCH:
    {
        goto t_sw_plus_112;
t_sw_plus_112:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        nBW = gnBW(qy, AugTypeBitsInv(112));
        assert(nBLR == nBL);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
    case  96 + T_SWITCH:
    {
        goto t_sw_plus_96;
t_sw_plus_96:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        nBW = gnBW(qy, AugTypeBitsInv(96));
        assert(nBLR == nBL);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
    case  80 + T_SWITCH:
    {
        goto t_sw_plus_80;
t_sw_plus_80:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        nBW = gnBW(qy, AugTypeBitsInv(80));
        assert(nBLR == nBL);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
    case  64 + T_SWITCH:
    {
        goto t_sw_plus_64;
t_sw_plus_64:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        nBW = gnBW(qy, AugTypeBitsInv(64));
        assert(nBLR == nBL);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SWITCH:
    {
        goto t_sw_plus_48;
t_sw_plus_48:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        assert(nBLR == nBL);
      #ifdef AUGMENT_TYPE_8
        nBW = gnBW(qy, AugTypeBitsInv(48));
        assert(gnBW(qy, nBLR) == nBW);
      #else // AUGMENT_TYPE_8
        nBW = gnBW(qy, nBLR); // num bits decoded
      #endif // else AUGMENT_TYPE_8
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
    case 32 + T_SWITCH:
    {
        goto t_sw_plus_32;
t_sw_plus_32:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        assert(nBLR == nBL);
      #ifdef AUGMENT_TYPE_8
        nBW = gnBW(qy, AugTypeBitsInv(32));
        assert(gnBW(qy, nBLR) == nBW);
      #else // AUGMENT_TYPE_8
        nBW = gnBW(qy, nBLR); // num bits decoded
      #endif // else AUGMENT_TYPE_8
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
    case 16 + T_SWITCH:
    {
        goto t_sw_plus_16;
t_sw_plus_16:
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            break;
        }
        assert(nBLR == nBL);
      #ifdef AUGMENT_TYPE_8
        nBW = gnBW(qy, AugTypeBitsInv(16));
        assert(gnBW(qy, nBLR) == nBW);
      #else // AUGMENT_TYPE_8
        nBW = gnBW(qy, nBLR); // num bits decoded
      #endif // else AUGMENT_TYPE_8
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnX);
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
  #endif // BITMAP
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
    } // end of case T_SWITCH
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
  #if !defined(AUGMENT_TYPE) || !defined(LOOKUP)
    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
  #endif // !defined(AUGMENT_TYPE) || !defined(LOOKUP)
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
  #if !defined(AUGMENT_TYPE) || !defined(LOOKUP) || defined(SKIP_LINKS)
        // nBL is bits left after picking the link we're handling now
        // nBL has not been reduced by any skip indicated in that link
        // nBLR is bits left after reducing nBL by any skip in the link
        // nBLR is bits left to decode by this switch and below
        goto t_switch; // silence cc in case other the gotos are ifdef'd out
t_switch:;
  #ifdef AUGMENT_TYPE
  #ifdef LOOKUP
       assert(nBLR != nBL);
  #endif // LOOKUP
  #endif // AUGMENT_TYPE
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
  #if defined(INSERT) || defined(REMOVE)
            if (bCleanup) {
      #if defined(INSERT) && defined(B_JUDYL)
                return pwValue;
      #else // defined(INSERT) && defined(B_JUDYL)
                return Success;
      #endif // defined(INSERT)
            } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)
            IF_COUNT(return wPopCntSum);
            break;
        }
        DBGX(Checkpoint(qy, "t_switch"));
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
        // ((uint8_t *)&wKey)[(cnBitsPerWord - nBL) >> 3];
        // ((uint8_t *)&wKey)[cnDigitsPerWord - nDL];
        // ((uint8_t *)&wSwappedKey)[nDL];
        // *(uint8_t *)&wSwappedAndShiftedKey;
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
        DBGX(printf("updated pwLnX %p\n", pwLnX));
      #ifdef PREFETCH_EK_VAL
      #ifdef LOOKUP
        PREFETCH(pwLnX);
      #endif // LOOKUP
      #endif // PREFETCH_EK_VAL
  #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
        goto switchTail; // in case other uses go away by ifdef
switchTail:;
  #if defined(INSERT) || defined(REMOVE)
        // Handle big picture tree cleanup.
        if (bCleanup
            && SwCleanup(qya, wKey, nBLR
    #if defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                    , &pwValue
    #endif // defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                      ) != 0)
        {
            goto restart;
        }
  #endif // defined(INSERT) || defined(REMOVE)
        // adjust pop count
        IF_NOT_COUNT(IF_NOT_LOOKUP(wPopCntUp = SwIncr(qy, nBLR, bCleanup,
                                                      nIncr)));
        IF_COUNT(wPopCntSum += CountSw(qy, nBLR, nBW, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(pLnUp = pLn);
        IF_CODE_XX_SW(IF_INSERT(nBLUp = nBL));
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR); // updates wRoot
  #ifndef QP_PLN
        pLn = pLnNew;
  #endif // !QP_PLN
  #ifdef BITMAP
        // Is there any reason to have
        // (cnBitsInD1 <= cnLogBitsPerLink)? What about lazy conversion
        // of embedded keys at (nBL > cnLogBitsPerLink) to
        // (nBL == cnLogBitsPerLink)?
        // Initialize warns if cnBitsInD1 is too small relative
        // to sizeof(Link_t).
        // if (nBL == cnLogBitsPerLink) there is no room for a
        // type field containing T_BITMAP so we goto t_bitmap directly.
        // What happens when it's time to make a 2-digit bitmap?
        // What happens when (nBL < cnLogBitsPerword)?
        // Would NewSwitch just allocate way more memory than we need?
        // Can we skip to one of these sub link size bitmaps?
        // What happens when it's time to make a 2-digit bitmap?
        // Should we back up nBLR and nBL and goto t_bitmap that way?
        // The first test below is done at compile time and will make the rest
        // of the code block go away if it is not needed.
      #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // qy is in an iffy state without updating nType and pwr.
            goto t_bitmap;
        }
      #endif // PACK_BM_VALUES || !B_JUDYL
  #else // BITMAP
        assert(!cbEmbeddedBitmap || (nBL > cnLogBitsPerLink));
  #endif // BITMAP
  #if defined(LOOKUP) || !defined(RECURSIVE)
        goto again; // nType = wr_nType(wRoot); *pwr = wr_pwr(wRoot); switch
  #else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(nBL, pLn, wKey);
  #endif // defined(LOOKUP) || !defined(RECURSIVE)
  #endif // !defined(AUGMENT_TYPE) || !defined(LOOKUP) || defined(SKIP_LINKS)

    } // end of case T_SWITCH

#if defined(CODE_XX_SW)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_XX_SW:
    case  96 + T_XX_SW:
    case  80 + T_XX_SW:
    case  64 + T_XX_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_XX_SW:
    case 32 + T_XX_SW:
    case 16 + T_XX_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_XX_SW: // no-skip (aka close) switch (vs. distant switch) w/o bm
    {
        goto t_xx_sw;
t_xx_sw:;
        DBGX(Checkpoint(qy, "t_xx_sw"));
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
  #ifdef _LNX
        pwLnX = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
  #endif // _LNX
        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
  // _XX_SW_TAIL is for cases where we don't go back to the top switch but
  // rather go directly to embedded keys.
  // NO_TYPE_IN_XX_SW needs it.
  // LOOKUP with ZERO_POP_CHECK_BEFORE_GOTO and
  // LOOKUP with HANDLE_DL2_IN_EMBEDDED_KEYS use it in an attempt to improve
  // performance.
  #ifdef NO_TYPE_IN_XX_SW // implies EMBED_KEYS
    #define _XX_SW_TAIL // the '_' prefix means internal and not for user
  #endif // NO_TYPE_IN_XX_SW
  #ifdef EMBED_KEYS
  #ifdef LOOKUP
    #ifdef ZERO_POP_CHECK_BEFORE_GOTO
      #define _XX_SW_TAIL
    #endif // ZERO_POP_CHECK_BEFORE_GOTO
    #ifdef HANDLE_DL2_IN_EMBEDDED_KEYS
      #define _XX_SW_TAIL
    #endif // HANDLE_DL2_IN_EMBEDDED_KEYS
  #endif // LOOKUP
  #endif // EMBED_KEYS
  #ifdef _XX_SW_TAIL
// Would be nice to be able to extract this chunk of code into a function
// because it is a replica of what is in t_switch. There would be a lot of
// parameters: qp, pqp, nBLUp, pLnUp, pwrUp, pLnNew, wKey, pnBLR, nBW, wDigit,
// nLinks, nCleanup, nIncr, bLinkPresent, wPopCntSum,
// We can't simply goto because the code diverges. But we might be able to
// handle the divergence by testing for XX_SW.
// Beginning of SwTailCommon:
      #if defined(INSERT) || defined(REMOVE)
        // Handle big picture tree cleanup.
        if (bCleanup
            && SwCleanup(qya, wKey, nBLR
          #if defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                    , &pwValue
          #endif // defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                      ) != 0)
        {
            goto restart;
        }
      #endif // defined(INSERT) || defined(REMOVE)
        wPopCntUp = SwIncr(qy, nBLR, bCleanup, nIncr); // adjust pop count
        IF_COUNT(wPopCntSum += CountSw(qy, nBLR, nBW, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(pLnUp = pLn);
        IF_CODE_XX_SW(IF_INSERT(nBLUp = nBL));
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqy, pLnNew, nBW, &nBLR);
      #ifndef QP_PLN
        pLn = pLnNew;
      #endif // !QP_PLN
      #ifdef BITMAP
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
      #endif // BITMAP
// End of SwTailCommon.
        // Handle XX_SW-specific special cases that don't go back to the top.
      #if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
          #if defined(NO_TYPE_IN_XX_SW)
        // ZERO_POP_MAGIC is valid only if a word can hold at least two keys.
        assert(EmbeddedListPopCntMax(nBL) >= 2);
          #endif // defined(NO_TYPE_IN_XX_SW)
        if (wRoot == WROOT_NULL) {
            return Failure;
        }
      #endif // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
        // Blow-ups are handled in t_embedded_keys.
        goto t_embedded_keys;
  #endif // _XX_SW_TAIL
        goto switchTail;

    } // end of case T_XX_SW

#endif // defined(CODE_XX_SW)

#if defined(CODE_BM_SW)

  #if defined(SKIP_TO_BM_SW)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_BM_SW:
    case  96 + T_SKIP_TO_BM_SW:
    case  80 + T_SKIP_TO_BM_SW:
    case  64 + T_SKIP_TO_BM_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_BM_SW:
    case 32 + T_SKIP_TO_BM_SW:
    case 16 + T_SKIP_TO_BM_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
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
                if (nBL >= cnBitsPerWord) {
                    int nBW = gnBW(qy, nBLR); // num bits decoded
                    // Abuse CountSw into counting whole switch.
                    int nLinkCnt = BmSwLinkCnt(qy);
                    wPopCnt = CountSw(qy, nBLR, nBW,
                                      /*wIndex*/ nLinkCnt,
                                      /*nLinks*/ nLinkCnt);
                } else
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                { wPopCnt = gwPopCnt(qy, nBLR); }
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

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_FULL_BM_SW:
    case  96 + T_FULL_BM_SW:
    case  80 + T_FULL_BM_SW:
    case  64 + T_FULL_BM_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_FULL_BM_SW:
    case 32 + T_FULL_BM_SW:
    case 16 + T_FULL_BM_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_FULL_BM_SW:
    {
        goto t_full_bm_sw; // silence cc in case there are no other uses
t_full_bm_sw:
      #if defined(LOOKUP)
        goto t_switch;
      #endif // defined(LOOKUP)
        goto t_bm_sw;

    } // end of case T_FULL_BM_SW

  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_BM_SW:
    case  96 + T_BM_SW:
    case  80 + T_BM_SW:
    case  64 + T_BM_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_BM_SW:
    case 32 + T_BM_SW:
    case 16 + T_BM_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_BM_SW:
    {
        goto t_bm_sw; // silence cc in case other the gotos are ifdef'd out
t_bm_sw:;
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);

        Word_t wSwIndex;
  #if defined(BM_IN_LINK) && defined(SKIP_TO_BM_SW)
        wSwIndex = wDigit; // in case the condition below is false
        IF_COUNT(bLinkPresent = 1); // in case the condition below is false
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
  #endif // defined(BM_IN_LINK) && defined(SKIP_TO_BM_SW)
        {
  #if ! defined(COUNT)
            int bLinkPresent; // need less local bLinkPresent for COUNT
  #endif // ! defined(COUNT)
  #if defined(BM_SW_FOR_REAL)
            char* pcPrefetch; (void)pcPrefetch;
      #ifndef OFFSET_IN_SW_BM_WORD
            pcPrefetch = (void*)PWR_pwBm(&pLn->ln_wRoot, pwr, nBW);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
      #endif // #ifndef OFFSET_IN_SW_BM_WORD
      #ifdef B_JUDYL
      #if (cnBitsPerWord > 32)
            int nLinkCnt = BmSwLinkCnt(qy);
            wSwIndex = Psplit(nLinkCnt, nBW, /*nShift*/ 0, wDigit);
            (void)wSwIndex;
          #ifdef PREFETCH_BM_LN
            pcPrefetch = (void*)&pwr_pLinks((BmSwitch_t*)pwr)[wSwIndex];
            PREFETCH(pcPrefetch - 64);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
          #endif // PREFETCH_BM_LN
          #ifdef _LNX
          #ifdef PREFETCH_BM_EK
            pcPrefetch = (void*)gpwLnX(qy, nLinkCnt, wSwIndex);
            PREFETCH(pcPrefetch - 64);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
          #endif // PREFETCH_BM_EK
          #endif // _LNX
      #endif // (cnBitsPerWord > 32)
      #endif // B_JUDYL
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
          #else
            BmSwIndex(qy, wDigit, &wSwIndex, &bLinkPresent);
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
  #ifdef _LNX
        int nLinkCnt = BmSwLinkCnt(qy);
      #ifndef BM_SW_FOR_REAL
        assert(nLinkCnt == (1<<nBW));
      #endif // BM_SW_FOR_REAL
        pwLnX = gpwLnX(qy, nLinkCnt, wSwIndex);
  #endif // _LNX

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
      #ifndef QP_PLN
        pLn = pLnNew;
      #endif // !QP_PLN
      #ifdef BITMAP
      #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
        // compiler complains ifndef BITMAP even if cbEmbeddedBitmap==0
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
      #endif // PACK_BM_VALUES || !B_JUDYL
      #endif // BITMAP
        goto again;
  #else // defined(LOOKUP)
      #ifdef INSERT
        // This test should be the same as the one in InsertCleanup.
        if (tp_bIsBmSw(nType) && InflateBmSwTest(qy)) {
            DBGX(Checkpoint(qy, "T_BM_SW req cleanup"));
            bCleanupRequested = 1; // on success
        }
      #endif // INSERT
        goto switchTail;
  #endif // defined(LOOKUP)

    } // end of case T_BM_SW

#endif // defined(CODE_BM_SW)

#if defined(CODE_LIST_SW)

  #if defined(SKIP_TO_LIST_SW)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_LIST_SW:
    case  96 + T_SKIP_TO_LIST_SW:
    case  80 + T_SKIP_TO_LIST_SW:
    case  64 + T_SKIP_TO_LIST_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_LIST_SW:
    case 32 + T_SKIP_TO_LIST_SW:
    case 16 + T_SKIP_TO_LIST_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
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
                    int nBW = gnBW(qy, nBLR); // num bits decoded
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qy, nBLR, nBW, EXP(nBW), EXP(nBW));
                } else
      #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qy, nBLR);
                }
                DBGC(printf("SKIP_TO_LIST_SW: PM wPopCnt %" _fw"d\n",
                            wPopCnt));
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
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_LIST_SW:
    case  96 + T_LIST_SW:
    case  80 + T_LIST_SW:
    case  64 + T_LIST_SW:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_LIST_SW:
    case 32 + T_LIST_SW:
    case 16 + T_LIST_SW:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_LIST_SW:
    {
        goto t_list_sw; // silence cc in case other the gotos are ifdef'd out
t_list_sw:;

        nBW = gnBW(qy, nBLR); // num bits decoded
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
      #ifndef QP_PLN
        pLn = pLnNew;
      #endif // !QP_PLN
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
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_LIST:
    case  96 + T_SKIP_TO_LIST:
    case  80 + T_SKIP_TO_LIST:
    case  64 + T_SKIP_TO_LIST:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_LIST:
    case 32 + T_SKIP_TO_LIST:
    case 16 + T_SKIP_TO_LIST:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
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

  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
      #ifdef AUGMENT_TYPE_8
    case 112 + T_LIST:
    case  96 + T_LIST:
    case  80 + T_LIST:
    case  64 + T_LIST:
      #else // AUGMENT_TYPE_8
    case  48 + T_LIST:
      #endif // AUGMENT_TYPE_8 else
    {
        goto t_listWord;
t_listWord:;
      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
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
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #ifndef LOOKUP_NO_LIST_SEARCH
            if (1
                && ((wr_nType(WROOT_NULL) != T_LIST) || (wRoot != WROOT_NULL))
              #ifdef B_JUDYL
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInListWord(qy, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // B_JUDYL
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
                  #else // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
                && ListHasKeyWord(qy, nBLR, wKey)
                  #endif // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
              #endif // B_JUDYL
                )
      #endif // !LOOKUP_NO_LIST_SEARCH
            {
                SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
                SMETRICS(++j__GetCalls);
              #ifdef B_JUDYL
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                  #ifndef PACK_L1_VALUES
                if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
                    return &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
                  #endif // #ifndef PACK_L1_VALUES
                { return &gpwValues(qy)[~nPos]; }
              #else // B_JUDYL
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // B_JUDYL else
            }
        }
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
        break;
    } // end of case T_LIST
      #ifdef AUGMENT_TYPE_8
    case 48 + T_LIST:
      #endif // AUGMENT_TYPE_8
    case 32 + T_LIST:
    {
        goto t_list32;
t_list32:;
      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
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
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #ifndef LOOKUP_NO_LIST_SEARCH
            if (1
                && ((wr_nType(WROOT_NULL) != T_LIST) || (wRoot != WROOT_NULL))
              #ifdef B_JUDYL
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInList32(qy, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // B_JUDYL
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
                  #else // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
                && ListHasKey32(qy, nBLR, wKey)
                  #endif // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
              #endif // B_JUDYL
                )
      #endif // !LOOKUP_NO_LIST_SEARCH
            {
                SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
                SMETRICS(++j__GetCalls);
              #ifdef B_JUDYL
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                  #ifndef PACK_L1_VALUES
                if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
                    return &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
                  #endif // #ifndef PACK_L1_VALUES
                { return &gpwValues(qy)[~nPos]; }
              #else // B_JUDYL
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // B_JUDYL else
            }
        }
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
        break;
    } // end of case T_LIST
    case 16 + T_LIST:
    {
        goto t_list16;
t_list16:;
      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
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
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #ifndef LOOKUP_NO_LIST_SEARCH
            if (1
                && ((wr_nType(WROOT_NULL) != T_LIST) || (wRoot != WROOT_NULL))
              #ifdef B_JUDYL
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInList16(qya, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // B_JUDYL
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
                  #else // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
                && ListHasKey16(qy, nBLR, wKey)
                  #endif // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
              #endif // B_JUDYL
                )
      #endif // !LOOKUP_NO_LIST_SEARCH
            {
                SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
                SMETRICS(++j__GetCalls);
              #ifdef B_JUDYL
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                  #ifndef PACK_L1_VALUES
                if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
                    return &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
                  #endif // #ifndef PACK_L1_VALUES
                { return &gpwValues(qy)[~nPos]; }
              #else // B_JUDYL
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // B_JUDYL else
            }
        }
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
        break;
    } // end of case T_LIST
  #endif // AUGMENT_TYPE && LOOKUP
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
    case T_LIST:
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_list;
t_list:;
        DBGX(Checkpoint(qy, "t_list"));

  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
    #if defined(INSERT) && defined(B_JUDYL)
            return pwValue;
    #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
    #endif // defined(INSERT)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

        assert((pwr != NULL)
            || ((wr_nType(WROOT_NULL) == T_LIST) && (wRoot == WROOT_NULL)));
        assert((pwr == NULL) || (gnListBLR(qy) == nBL));

        // Search the list.  wPopCnt is the number of keys in the list.

      #if defined(COUNT)
        if ((wr_nType(WROOT_NULL) == T_LIST) && (wRoot == WROOT_NULL)) {
             nPos = ~0;
        }
      #endif // defined(COUNT)

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
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
                && ((wr_nType(WROOT_NULL) != T_LIST) || (wRoot != WROOT_NULL))
          #if defined(LOOKUP)
              #if defined(B_JUDYL)
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                      #if defined(AUGMENT_TYPE) && !defined(AUGMENT_TYPE_NOT)
                && ((nPos = LocateKeyInList8(qy, nBLR, wKey)) >= 0)
                      #else // AUGMENT_TYPE && !AUGMENT_TYPE_NOT
                && ((nPos = LocateKeyInList(qya, nBLR, wKey)) >= 0)
                      #endif // AUGMENT_TYPE && !AUGMENT_TYPE_NOT else
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // defined(B_JUDYL)
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
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
                SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
                SMETRICS(++j__GetCalls);
          #if defined(INSERT)
              #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
              #endif // ! defined(RECURSIVE)
          #endif // defined(INSERT)
          #if defined(REMOVE)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
              #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                  #ifndef PACK_L1_VALUES
                if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
                    return &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
                  #endif // #ifndef PACK_L1_VALUES
                { return &gpwValues(qy)[~nPos]; }
              #else // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // (defined(LOOKUP) || defined(INSERT)) && ...
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
      #if defined(LOOKUP)
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
      #endif // defined(LOOKUP)

      #if defined(COUNT)
        DBGC(printf("T_LIST: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
        return wPopCntSum;
      #endif // defined(COUNT)

      #ifdef INSERT
          // Tried putting test for (cn2dBmMaxWpkPercent != 0) in the
          // condition below but g++ complained about the unsigned comparison
          // to less than zero at the end of the condition always being false.
          #if (cn2dBmMaxWpkPercent != 0) // g++ warns always
              #ifdef BITMAP
        if ((cnBitsInD1 > cnLogBitsPerLink) // compiled out
            && (nBLR == cnBitsInD1) // check that conversion is not done
            && (nBL == nBLR) // converting skip makes no sense
            // this should agree with the test in InsertCleanup
            && (wPopCntUp * cn2dBmMaxWpkPercent * cnBitsPerWord
                > EXP(cnBitsLeftAtDl2) * 100)
            )
        {
            DBGX(Checkpoint(qy, "T_LIST req cleanup"));
            bCleanupRequested = 1; // goto cleanup when done
        }
              #endif // BITMAP
          #endif // (cn2dBmMaxWpkPercent != 0)
      #endif // INSERT

        break;

    } // end of case T_LIST

#if defined(UA_PARALLEL_128)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_LIST_UA:
    case  96 + T_LIST_UA:
    case  80 + T_LIST_UA:
    case  64 + T_LIST_UA:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_LIST_UA:
    case 32 + T_LIST_UA:
    case 16 + T_LIST_UA:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
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
      #if defined(LOOKUP)
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
      #endif // defined(LOOKUP)

      #if defined(COUNT)
        DBGC(printf("T_LIST_UA: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
        return wPopCntSum;
      #endif // defined(COUNT)

        break;

    } // end of case T_LIST_UA
#endif // defined(UA_PARALLEL_128)

#ifdef XX_LISTS
  #ifdef DEFAULT_XX_LIST
    default:
  #endif // DEFAULT_XX_LIST
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_XX_LIST:
    case  96 + T_XX_LIST:
    case  80 + T_XX_LIST:
    case  64 + T_XX_LIST:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_XX_LIST:
    case 32 + T_XX_LIST:
    case 16 + T_XX_LIST:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_XX_LIST:
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_xx_list;
t_xx_list:;
        DBGX(Checkpoint(qy, "t_xx_list"));

  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
    #if defined(INSERT) && defined(B_JUDYL)
            return pwValue;
    #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
    #endif // defined(INSERT)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

        assert((pwr != NULL) || (wr_nType(WROOT_NULL) == T_XX_LIST));

        // Search the list. wPopCnt is the number of keys in the list.

      #if defined(COUNT)
        if ((wr_nType(WROOT_NULL) == T_XX_LIST) && (wRoot == WROOT_NULL)) {
             nPos = ~0;
        }
      #endif // defined(COUNT)

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
            nBLR = gnListBLR(qy);
            assert(nBLR > nBL); // skip up instead of down
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #if ! defined(LOOKUP) || ! defined(LOOKUP_NO_LIST_SEARCH)
            if (1
                && ((wr_nType(WROOT_NULL) != T_XX_LIST)
                    || (wRoot != WROOT_NULL))
          #if defined(LOOKUP)
              #if defined(B_JUDYL)
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qy, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qy, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInList(qya, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // defined(B_JUDYL)
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qy, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
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
                SMETRICS(j__SearchPopulation += gnListPopCnt(qy, nBLR));
                SMETRICS(++j__GetCalls);
          #if defined(INSERT)
              #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
              #endif // ! defined(RECURSIVE)
          #endif // defined(INSERT)
          #if defined(REMOVE)
                goto removeGutsAndCleanup;
          #endif // defined(REMOVE)
          #if defined(LOOKUP) || defined(INSERT) || defined(REMOVE)
              #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                  #ifndef PACK_L1_VALUES
                if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
                    return &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
                  #endif // #ifndef PACK_L1_VALUES
                { return &gpwValues(qy)[~nPos]; }
              #else // (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
              #endif // (defined(LOOKUP) || defined(INSERT)) && ...
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
      #if defined(LOOKUP)
          #if defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
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
          #endif // defined(SKIP_PREFIX_CHECK) && defined(COMPRESSED_LISTS)
      #endif // defined(LOOKUP)

      #if defined(COUNT)
        DBGC(printf("T_XX_LIST: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
        return wPopCntSum;
      #endif // defined(COUNT)

        break;

    } // end of case T_XX_LIST
#endif // XX_LISTS

#endif // (cwListPopCntMax != 0)

#ifdef BITMAP
  #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
  #if defined(SKIP_TO_BITMAP)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_SKIP_TO_BITMAP:
    case  96 + T_SKIP_TO_BITMAP:
    case  80 + T_SKIP_TO_BITMAP:
    case  64 + T_SKIP_TO_BITMAP:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_SKIP_TO_BITMAP:
    case 32 + T_SKIP_TO_BITMAP:
    case 16 + T_SKIP_TO_BITMAP:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_SKIP_TO_BITMAP:
    {
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
                Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR);
                DBGC(printf("T_SKIP_TO_BITMAP: PREFIX_MISMATCH wPopCnt %" _fw
                                "d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skbm wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
  #endif // defined(COUNT)
            break;
        }
        // We don't support skip to unpacked bitmap yet.
        goto t_bitmap;
    }
  #endif // PACK_BM_VALUES || !B_JUDYL
  #endif // defined(SKIP_TO_BITMAP)
      #if defined(UNPACK_BM_VALUES) || defined(CODE_UNPACK_BM_VALUES)
      #ifndef LOOKUP
    case T_UNPACKED_BM:
        goto t_unpacked_bm;
t_unpacked_bm:;
      #endif // #ifndef LOOKUP
      #endif // defined(UNPACK_BM_VALUES) || defined(CODE_UNPACK_BM_VALUES)
  #if defined(PACK_BM_VALUES) || !defined(LOOKUP) || !defined(B_JUDYL)
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
  // Should this be ifndef(DEFAULT_UNPACKED_BM) || def(DEFAULT_AND_CASE)?
  #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_BITMAP:
    case  96 + T_BITMAP:
    case  80 + T_BITMAP:
    case  64 + T_BITMAP:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_BITMAP:
    case 32 + T_BITMAP:
    case 16 + T_BITMAP:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_BITMAP:
  #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
    {
        goto t_bitmap;
t_bitmap:;
  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
      #if defined(INSERT) && defined(B_JUDYL)
            return pwValue;
      #else // defined(INSERT) && defined(B_JUDYL)
            return Success;
      #endif // defined(INSERT) && defined(B_JUDYL)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)

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
            if ((wr_nType(WROOT_NULL) == T_BITMAP)
                && (!cbEmbeddedBitmap || (nBLR > cnLogBitsPerLink))
                && (wRoot == WROOT_NULL))
            {
                return wPopCntSum;
            }
            // Count bits.
            Word_t wPopCnt;
            if (cbEmbeddedBitmap && (nBLR <= cnLogBitsPerLink)) {
                assert(nBL == nBLR); // no skip to sub-link-size bm
                if (nBLR <= cnLogBitsPerWord) {
                    Word_t wBit = EXP(wKey & MSK(nBLR));
                    Word_t wBmMask = wBit - 1;
                    wPopCnt = __builtin_popcountll(wRoot & wBmMask);
                } else {
                    Word_t wBitNum = wKey & MSK(nBLR);
                    Word_t wBmWordNum = wBitNum >> cnLogBitsPerWord;
                    Word_t wBit = EXP(wBitNum & MSK(cnLogBitsPerWord));
                    Word_t wBmMask = wBit - 1;
                    Word_t *pwLn = (Word_t*)pLn;
                    wPopCnt = 0;
                    for (int i = 0; i < (int)wBmWordNum; ++i) {
                        wPopCnt += __builtin_popcountll(pwLn[i]);
                    }
                    wPopCnt
                        += __builtin_popcountll(pwLn[wBmWordNum] & wBmMask);
                }
            } else {
                int nWordOffset = (wKey & MSK(nBLR)) >> cnLogBitsPerWord;
                // Do we count from the front or the back? If nBLR is small
                // enough it's faster to just always count from the same
                // end rather than going to the trouble of figuring out
                // which end is closer.
          #ifdef _BMLF_BM_IN_LNX
                Word_t *pwBitmap = pwLnX;
          #else // _BMLF_BM_IN_LNX
                Word_t *pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
          #endif // else _BMLF_BM_IN_LNX
                if ((nBLR > 8) && (wKey & EXP(nBLR - 1))) {
                    wPopCnt = gwBitmapPopCnt(qy, nBLR);
                    if (wPopCnt == 0) {
                        wPopCnt = EXP(nBLR);
                    }
                    for (int nn = nWordOffset + 1;
                             nn < (int)EXP(nBLR - cnLogBitsPerWord); nn++) {
                        wPopCnt -= __builtin_popcountll(pwBitmap[nn]);
                    }
                    Word_t wBmMask
                        = ((Word_t)-1
                            << (wKey & MSK(nBLR) & (cnBitsPerWord - 1)));
                    wPopCnt -= __builtin_popcountll(pwBitmap[nWordOffset]
                                                        & wBmMask);
                } else {
                    wPopCnt = 0;
                    for (int nn = 0; nn < nWordOffset; nn++) {
                        wPopCnt += __builtin_popcountll(pwBitmap[nn]);
                    }
                    Word_t wBmMask
                        = ((Word_t)1
                            << (wKey & MSK(nBLR) & (cnBitsPerWord - 1))) - 1;
                    wPopCnt += __builtin_popcountll(pwBitmap[nWordOffset]
                                                        & wBmMask);
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
      #ifdef USE_XX_SW_ONLY_AT_DL2
            // We assume we never blow-out into a one-digit bitmap.
// But that doesn't mean we don't want to support skip directly to a
// one-digit bitmap bypassing DL2.
            // We just double until we end up with one big bitmap at DL2.
            // But what about when we create XX_SW at (nBLR == nBitsLeftAtDl2)
            // and cbEmbeddedBitmap and nBLR - cnBW == cnBitsInD1? We did not
            // explicitly double.
            // Or nBLR - cnBW <= cnLogBitsPerLink?
            // We end up with one big bitmap at DL2 but it is
            // represented by an XX_SW and embedded bitmaps?
            // What if cnBitsInD1 > cnLogBitsPerLink?
            // cbEmbeddedBitmap won't be true.
            // Do we have code to make the whole thing T_BITMAP?
            // We end up here with (nBL <= cnBitsInD1)?
            // We have to be sure to handle that case below
            // for USE_XX_SW_ONLY_AT_DL2?
      #endif // USE_XX_SW_ONLY_AT_DL2
          #if defined(BMLF_INTERLEAVE) && !defined(LOOKUP)
            Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t);
            wWordsHdr += EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
            Word_t wDigit = wKey & MSK(cnBitsInD1);
            int nLogBmPartBmBits = cnBitsInD1 - cnLogBmlfParts;
            int nBmPartBmWords = (nLogBmPartBmBits < cnLogBitsPerWord)
                ? 1 : EXP(nLogBmPartBmBits - cnLogBitsPerWord);
            int nBmPartSz = nBmPartBmWords + EXP(cnBitsInD1 - cnLogBmlfParts);
            int nBmPartNum = wDigit >> (cnBitsInD1 - cnLogBmlfParts);
            Word_t wKeyLeft = wDigit & MSK(cnBitsInD1 - cnLogBmlfParts);
            int nBmBitPartNum = nBmPartNum;
              #ifdef BMLFI_SPLIT_BM
            nBmBitPartNum += !!(wDigit & EXP(cnBitsInD1 - cnLogBmlfParts - 1));
              #endif // BMLFI_SPLIT_BM
            Word_t* pwBmBitPart = &pwr[wWordsHdr] + nBmPartSz * nBmBitPartNum;
            int bBitIsSet = BitIsSet(pwBmBitPart
              #ifdef BMLFI_BM_AT_END
                                     + nBmPartSz - nBmPartBmWords
              #endif // BMLFI_BM_AT_END
                                   , wKeyLeft);
          #else // defined(BMLF_INTERLEAVE) && !defined(LOOKUP)
      #ifdef B_JUDYL
          #if defined(LOOKUP) || defined(INSERT)
            Word_t* pwBitmapValues = gpwBitmapValues(qy, cnBitsInD1);
          #endif // defined(LOOKUP) || defined(INSERT)
      #ifdef LOOKUP
          #ifdef PACK_BM_VALUES
                Word_t wPopCnt = gwBitmapPopCnt(qy, cnBitsInD1);
// Note: I wonder if the penalty for a branch misprediction is
// exacerbated when prefetching is done on both forks of the branch.
// For example, T_LIST vs T_BITMAP(packed) at nBLR==cnBitsInD1.
// We could structure the leaves so that both sides would want to prefetch
// the exact same cache lines relative to pwr and be able to initiate the
// prefetch before the switch;
// pop in wRoot and value area starting at pwr[-1].
                char* pcPrefetch
                    = (char*)&pwBitmapValues[
                        Psplit(wPopCnt, cnBitsInD1, /*nShift*/ 0, wKey)];
                (void)pcPrefetch;
              #ifdef PF_BM_PREV_HALF_VAL
                PREFETCH(pcPrefetch - 32);
              #endif // PF_BM_PREV_HALF_VAL
              #ifdef PREFETCH_BM_PSPLIT_VAL
                PREFETCH(pcPrefetch);
              #endif // PREFETCH_BM_PSPLIT_VAL
              #ifdef PREFETCH_BM_PREV_VAL
                PREFETCH(pcPrefetch - 64);
              #endif // PREFETCH_BM_PREV_VAL
              #ifdef PF_BM_NEXT_HALF_VAL
                PREFETCH(pcPrefetch + 32);
              #endif // PF_BM_NEXT_HALF_VAL
              #ifdef PREFETCH_BM_NEXT_VAL
                PREFETCH(pcPrefetch + 64);
              #endif // PREFETCH_BM_NEXT_VAL
          #endif // PACK_BM_VALUES
          #ifdef UNPACK_BM_VALUES
          #ifdef PREFETCH_BM_VAL
                PREFETCH(&pwBitmapValues[wKey & MSK(cnBitsInD1)]);
          #endif // PREFETCH_BM_VAL
          #endif // UNPACK_BM_VALUES
      #endif // LOOKUP
      #endif // B_JUDYL
            // Use compile-time tests to speed this up. Hopefully.
  #if defined(B_JUDYL) && defined(LOOKUP) && defined(BMLF_POP_COUNT_1_NO_TEST)
            int bBitIsSet = 1;
  #else // BMLF_POP_COUNT_1_NO_TEST
            int bBitIsSet =
          #ifdef _BMLF_BM_IN_LNX
                BitIsSet(pwLnX, wKey & MSK(cnBitsInD1));
          #else // _BMLF_BM_IN_LNX
     // We don't need/want to check for WROOT_NULL for embedded bitmap.
                ((wr_nType(WROOT_NULL) == T_BITMAP)
                        && (!cbEmbeddedBitmap || (nBLR > cnLogBitsPerLink))
                        && (wRoot == WROOT_NULL))
                    ? 0 :
              #ifdef USE_XX_SW_ONLY_AT_DL2
                BitIsSet((nBLR <= cnLogBitsPerLink)
                             ? (Word_t*)pLn : ((BmLeaf_t*)pwr)->bmlf_awBitmap,
                         wKey & MSK(nBLR));
              #else // USE_XX_SW_ONLY_AT_DL2
                ((cn2dBmMaxWpkPercent == 0) || (nBLR == cnBitsInD1))
                    ? (cbEmbeddedBitmap && (cnBitsInD1 <= cnLogBitsPerWord))
                        ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsInD1))
                        : BitIsSet(cbEmbeddedBitmap
                                       ? (Word_t*)pLn
                                       : ((BmLeaf_t*)pwr)->bmlf_awBitmap,
                                   wKey & MSK(cnBitsInD1))
                    :
                      (cbEmbeddedBitmap
                            && (cnBitsLeftAtDl2 <= cnLogBitsPerWord))
                        ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsLeftAtDl2))
                        : BitIsSet((cbEmbeddedBitmap
                                           && (cnBitsLeftAtDl2
                                               <= cnLogBitsPerLink))
                                       ? (Word_t*)pLn
                                       : ((BmLeaf_t*)pwr)->bmlf_awBitmap,
                                   wKey & MSK(cnBitsLeftAtDl2));
              #endif // #else USE_XX_SW_ONLY_AT_DL2
          #endif // else _BMLF_BM_IN_LNX
  #endif // BMLF_POP_COUNT_1_NO_TEST
          #endif // defined(BMLF_INTERLEAVE) && !defined(LOOKUP) else
            if (bBitIsSet) {
      #if defined(REMOVE)
                goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT)
          #if !defined(RECURSIVE)
                if (nIncr > 0) {
                    DBGX(printf("Bit is set!\n"));
                    goto undo; // undo counting
                }
          #endif // !defined(RECURSIVE)
      #endif // defined(INSERT)
      #if (defined(LOOKUP) || defined(INSERT)) && defined(B_JUDYL)
              #if defined(BMLF_INTERLEAVE) && !defined(LOOKUP)
                Word_t* pwBmPart = &pwr[wWordsHdr] + nBmPartSz * nBmPartNum;
                return &pwBmPart[wKeyLeft
                  #ifndef BMLFI_BM_AT_END
                                 + nBmPartBmWords
                  #endif // !BMLFI_BM_AT_END
                                 ];
              #else // defined(BMLF_INTERLEAVE) && !defined(LOOKUP)
                int nIndex =
                  #ifndef LOOKUP
                    BM_UNPACKED(wRoot) ? (int)(wKey & MSK(cnBitsInD1)) :
                  #endif // !LOOKUP
                        BmIndex(qya, cnBitsInD1, wKey);
                Word_t* pwValue = &pwBitmapValues[nIndex];
              #ifdef LOOKUP
              #ifdef PACK_BM_VALUES
                SMETRICS(j__SearchPopulation += wPopCnt);
                SMETRICS(++j__GetCalls);
                  #ifndef PREFETCH_BM_PREV_VAL
                  #ifndef PF_BM_PREV_HALF_VAL
                  #ifdef PREFETCH_BM_PSPLIT_VAL
                  #ifndef PF_BM_NEXT_HALF_VAL
                  #ifndef PREFETCH_BM_NEXT_VAL
                SMETRICS(j__DirectHits
                    += (Word_t)pwValue - ((Word_t)pcPrefetch & ~MSK(6))
                        < 64);
                  #endif // #ifndef PREFETCH_BM_NEXT_VAL
                  #endif // #ifndef PF_BM_NEXT_HALF_VAL
                  #endif // PREFETCH_BM_PSPLIT_VAL
                  #endif // #ifndef PF_BM_PREV_HALF_VAL
                  #endif // #ifndef PREFETCH_BM_PREV_VAL
                  #ifndef PREFETCH_BM_PREV_VAL
                  #ifdef PF_BM_PREV_HALF_VAL
                  #ifndef PREFETCH_BM_PSPLIT_VAL
                  #ifdef PF_BM_NEXT_HALF_VAL
                  #ifndef PREFETCH_BM_NEXT_VAL
                SMETRICS(j__DirectHits
                    += (Word_t)pwValue - (((Word_t)pcPrefetch - 32) & ~MSK(6))
                        < 128);
                  #endif // #ifndef PREFETCH_BM_NEXT_VAL
                  #endif // PF_BM_NEXT_HALF_VAL
                  #endif // #ifndef PREFETCH_BM_PSPLIT_VAL
                  #endif // PF_BM_PREV_HALF_VAL
                  #endif // #ifndef PREFETCH_BM_PREV_VAL
                  #ifdef PREFETCH_BM_PREV_VAL
                  #ifndef PF_BM_PREV_HALF_VAL
                  #ifdef PREFETCH_BM_PSPLIT_VAL
                  #ifndef PF_BM_NEXT_HALF_VAL
                  #ifdef PREFETCH_BM_NEXT_VAL
                SMETRICS(j__DirectHits
                    += (Word_t)pwValue - (((Word_t)pcPrefetch - 64) & ~MSK(6))
                        < 192);
                  #endif // PREFETCH_BM_NEXT_VAL
                  #endif // #ifndef PF_BM_NEXT_HALF_VAL
                  #endif // PREFETCH_BM_PSPLIT_VAL
                  #endif // #ifndef PF_BM_PREV_HALF_VAL
                  #endif // PREFETCH_BM_PREV_VAL
              #endif // PACK_BM_VALUES
              #endif // LOOKUP
                return pwValue;
              #endif // defined(BMLF_INTERLEAVE) && !defined(LOOKUP) else
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
            && (cnBitsInD1 > cnLogBitsPerLink) // compiled out
            && (nBLR == cnBitsInD1) // check that conversion is not done
            && (nBL == nBLR) // converting skip to b1 makes no sense
            // this should agree with the test in InsertCleanup
            && (wPopCntUp * cn2dBmMaxWpkPercent * cnBitsPerWord
                > EXP(cnBitsLeftAtDl2) * 100)))
        {
            bCleanupRequested = 1; // goto cleanup when done
        }
  #endif
#endif // defined(INSERT)
        break;

    } // end of case T_BITMAP
  #endif // PACK_BM_VALUES || !LOOKUP || !B_JUDYL
#ifdef LOOKUP
#if defined(UNPACK_BM_VALUES) || defined(CODE_UNPACK_BM_VALUES)
  #ifdef AUGMENT_TYPE_8
    case 112 + T_UNPACKED_BM:
    case  96 + T_UNPACKED_BM:
    case  80 + T_UNPACKED_BM:
    case  64 + T_UNPACKED_BM:
  #endif // AUGMENT_TYPE_8
  #ifdef AUGMENT_TYPE
    case 48 + T_UNPACKED_BM:
    case 32 + T_UNPACKED_BM:
    case 16 + T_UNPACKED_BM:
  #endif // AUGMENT_TYPE
    case T_UNPACKED_BM: // never exists for B_JUDY1
    {
        goto t_unpacked_bm;
t_unpacked_bm:;
#if defined(LOOKUP_NO_BITMAP_DEREF)
        return KeyFound;
#else // defined(LOOKUP_NO_BITMAP_DEREF)
      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
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
      #endif // SKIP_PREFIX_CHECK
      #endif // COMPRESSED_LISTS
        {
      #ifdef BMLF_INTERLEAVE
          #ifdef _BMLF_BM_IN_LNX
            #error _BMLF_BM_LNX with BMLF_INTERLEAVE
          #endif // _BMLF_BM_IN_LNX
            Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t);
            wWordsHdr += EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
            Word_t wDigit = wKey & MSK(cnBitsInD1);
            int nLogBmPartBmBits = cnBitsInD1 - cnLogBmlfParts;
            int nBmPartBmWords = (nLogBmPartBmBits < cnLogBitsPerWord)
                ? 1 : EXP(nLogBmPartBmBits - cnLogBitsPerWord);
            int nBmPartSz = nBmPartBmWords + EXP(cnBitsInD1 - cnLogBmlfParts);
            // Does the compiler figure out that nBmPartNum will always be
            // zero if cnLogBmlfParts is zero? And avoid the shift here and
            // the multiply and add below?
            int nBmPartNum = wDigit >> (cnBitsInD1 - cnLogBmlfParts);
            // Does the compiler figure out that wKeyLeft will be equal to
            // wDigit if cnLogBmlfParts is zero? And avoid the mask?
            Word_t wKeyLeft = wDigit & MSK(cnBitsInD1 - cnLogBmlfParts);
            Word_t* pwBmPart = &pwr[wWordsHdr] + nBmPartSz * nBmPartNum;
          #ifdef BMLFI_LNX
            pwBmPart += *pwLnX;
          #endif // BMLFI_LNX
            //__builtin_prefetch(pwBmPart + 16, 0, 0);
            int nBmBitPartNum = nBmPartNum;
              #ifdef BMLFI_SPLIT_BM
            nBmBitPartNum += !!(wDigit & EXP(cnBitsInD1 - cnLogBmlfParts - 1));
#if cnLogBmlfParts <= cnBitsInD1
  #error
#endif // cnLogBmlfParts <= cnBitsInD1
              #endif // BMLFI_SPLIT_BM
            Word_t* pwBmBitPart = &pwr[wWordsHdr] + nBmPartSz * nBmBitPartNum;
            if (BitIsSet(pwBmBitPart
              #ifdef BMLFI_BM_AT_END
                         + nBmPartSz - nBmPartBmWords
              #endif // BMLFI_BM_AT_END
                       , wKeyLeft))
            {
#if cnLogBmlfParts <= cnBitsInD1 - cnLogBitsPerWord
                assert(*pwBmBitPart != 0);
  #error
#endif // cnLogBmlfParts <= cnBitsInD1 - cnLogBitsPerWord
                return &pwBmPart[wKeyLeft
          #ifndef BMLFI_BM_AT_END
                                 + nBmPartBmWords
          #endif // !BMLFI_BM_AT_END
          #ifdef BMLFI_BM_HB
                                 + (*pwBmBitPart == 0)
          #endif // BMLFI_BM_HB
                                 ];
            }
      #else // BMLF_INTERLEAVE
            Word_t* pwBitmapValues = gpwBitmapValues(qy, cnBitsInD1);
          #ifdef PREFETCH_BM_VAL
            PREFETCH(&pwBitmapValues[wKey & MSK(cnBitsInD1)]);
          #endif // PREFETCH_BM_VAL
            if (BitIsSet(
          #ifdef _BMLF_BM_IN_LNX
                         pwLnX,
          #else // _BMLF_BM_IN_LNX
                         ((BmLeaf_t*)pwr)->bmlf_awBitmap,
          #endif // else _BMLF_BM_IN_LNX
                         wKey & MSK(cnBitsInD1)))
            {
                return &pwBitmapValues[wKey & MSK(cnBitsInD1)];
            }
      #endif // BMLF_INTERLEAVE else
            DBGX(printf("Bit is not set.\n"));
        }
#endif // defined(LOOKUP_NO_BITMAP_DEREF)
        break;

    } // end of case T_UNPACKED_BM
#endif // defined(UNPACK_BM_VALUES) || defined(CODE_UNPACK_BM_VALUES)
#endif // LOOKUP
#endif // BITMAP

#if defined(EMBED_KEYS)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_EMBEDDED_KEYS:
    case  96 + T_EMBEDDED_KEYS:
    case  80 + T_EMBEDDED_KEYS:
    case  64 + T_EMBEDDED_KEYS:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_EMBEDDED_KEYS:
    case 32 + T_EMBEDDED_KEYS:
    case 16 + T_EMBEDDED_KEYS:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_EMBEDDED_KEYS:
    {
        goto t_embedded_keys; // suppress compiler unused-label warnings
t_embedded_keys:; // the semi-colon allows for a declaration next; go figure
        DBGX(printf("T_EMBEDDED_KEYS %d nBL %d\n", T_EMBEDDED_KEYS, nBL));

  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
//assert(0); // Just checking; uh oh; do we need better testing?
      #if defined(B_JUDYL) && defined(INSERT)
            return pwValue;
      #else // defined(B_JUDYL) && defined(INSERT)
            return Success;
      #endif // defined(B_JUDYL) && defined(INSERT)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)
  #if defined(B_JUDYL)
    assert(pwLnX != NULL);
  #endif // defined(B_JUDYL)

        // Have to or in cnMallocAlignment unless nBL allows for at least
        // two embedded keys plus a type field. It doesn't buy us anything
        // unless we want to support an embedded key with more than
        // cnBitsPerWord - cnBitsMallocMask - 1 bits.
  #ifndef NO_TYPE_IN_XX_SW
        if (wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
  #endif // #ifndef NO_TYPE_IN_XX_SW
        {
            if (wRoot == WROOT_NULL) {
                break; // switch (nType) case T_EMBEDDED_KEYS
            }
        }
        assert(EmbeddedListPopCntMax(nBL) != 0);

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
                Word_t wKeyInSlot
                    = (wKey & ~wBLM)
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

      #if defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)
          #define _PARALLEL_EK
      #endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP) && defined(LOOKUP)
      #if defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP)
          #define _PARALLEL_EK
      #endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT) && !defined(LOOKUP)

      #ifdef _PARALLEL_EK

#if defined(HANDLE_BLOWOUTS)
    // We haven't written the insert code to create blow-outs for
    // NO_TYPE_IN_XX_SW yet.
  #if defined(NO_TYPE_IN_XX_SW)
    // A blowout has the high bit set and the high bit of the next key
    // slot clear. Type field can be anything other than T_EMBEDED_KEYS.
    // 44 pointer bits can be anything.
    // High bit must be set.
    // nBL is never less than 7 so next six high bits are always usable.
    // And 8 of the next 9 high bits are usable, but exactly which ones
    // depends on nBL.
    #define BLOWOUT_CHECK(_nBL) \
        ((wRoot & BLOWOUT_MASK(_nBL)) == (ZERO_POP_MAGIC & ~cnMallocMask))
  #else // defined(NO_TYPE_IN_XX_SW)
    #define BLOWOUT_CHECK(_nBL)  (wr_nType(wRoot) != T_EMBEDDED_KEYS)
  #endif // defined(NO_TYPE_IN_XX_SW)
#else // defined(HANDLE_BLOWOUTS)
    #define BLOWOUT_CHECK(_nBL) (0)
#endif // defined(HANDLE_BLOWOUTS)

#if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
    #define ZERO_CHECK  (0)
#else // defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
    #define ZERO_CHECK  (wRoot == WROOT_NULL)
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

      #else // _PARALLEL_EK

        Word_t wKeyRoot;

          #ifdef B_JUDYL
        wKeyRoot = wRoot >> (cnBitsPerWord - nBL);
        if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
          #else // B_JUDYL
        int nPopCnt = wr_nPopCnt(wRoot, nBL);
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
          #endif // #else B_JUDYL

      #endif // #else _PARALLEL_EK
        }

        break; // switch (nType) case T_EMBEDDED_KEYS

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

  #if defined(B_JUDYL) && (defined(INSERT) || defined(LOOKUP))
        return pwLnX;
  #else // defined(B_JUDYL) && (defined(INSERT) || defined(LOOKUP))
        return KeyFound;
  #endif // defined(B_JUDYL) && (defined(INSERT) || defined(LOOKUP))

    } // end of case T_EMBEDDED_KEYS

  #ifdef EK_XV
      #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_EK_XV:
    case  96 + T_EK_XV:
    case  80 + T_EK_XV:
    case  64 + T_EK_XV:
      #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
      #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_EK_XV:
    case 32 + T_EK_XV:
    case 16 + T_EK_XV:
      #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_EK_XV:
    {
        goto t_ek_xv; // suppress compiler unused-label warnings
t_ek_xv:; // the semi-colon allows for a declaration next; go figure
        DBGX(printf("T_EK_XV\n"));
  #if defined(INSERT) || defined(REMOVE)
        if (bCleanup) {
      #if defined(B_JUDYL) && defined(INSERT)
            return pwValue;
      #else // defined(B_JUDYL) && defined(INSERT)
            return Success;
      #endif // defined(B_JUDYL) && defined(INSERT)
        } // cleanup is complete
  #endif // defined(INSERT) || defined(REMOVE)
        assert(pwLnX != NULL);
  #if defined(COUNT)
        {
            int nBits = 1 << (LOG(nBL - 1) + 1);
      #if (cnBitsInD1 < cnLogBitsPerByte)
            if (nBits < 8) { nBits = 8; }
      #endif // (cnBitsInD1 < cnLogBitsPerByte)
            int nn;
            for (nn = 0; nn < wr_nPopCnt(wRoot, nBL); nn++) {
                if (((*pwLnX >> (nn * nBits)) & MSK(nBits))
                        >= (wKey & MSK(nBits)))
                {
                    break;
                }
            }
            DBGC(printf("EK_XV: wPopCntSum(before) %" _fw"d nn %d\n",
                        wPopCntSum, nn));
            wPopCntSum += nn;
            DBGC(printf("ek_xv nn %d wPopCntSum " OWx"\n", nn, wPopCntSum));
            return wPopCntSum;
        }
  #endif // defined(COUNT)
      #ifdef COMPRESSED_LISTS
      #ifdef SKIP_PREFIX_CHECK
      #ifdef LOOKUP
        #error not coded yet
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
      #ifdef _PARALLEL_EK

  #ifdef PF_EK_XV_2
#define PF_2(_nBL) \
            if (EmbeddedListPopCntMax(_nBL) > 2) { \
                PREFETCH(pwr + 8); \
            }
  #else // PF_EK_XV_2
#define PF_2(pwr)
  #endif // #else PF_EK_XV_2

  #if (cnBitsInD1 < cnLogBitsPerByte)
#define XV_BLX(_nBL) \
        case (_nBL): \
            PF_2(_nBL); \
            if ((nPos = LocateKey64((uint64_t*)pwLnX, wKey, \
                    MAX(8, (Word_t)2 << LOG((_nBL) - 1)))) >= 0) { \
                assert(nPos < wr_nPopCnt(wRoot, (_nBL))); \
                goto xv_foundIt; \
            } \
            goto xv_break2
  #else // (cnBitsInD1 < cnLogBitsPerByte)
#define XV_BLX(_nBL) \
        case (_nBL): \
            PF_2(_nBL); \
            if ((nPos = LocateKey64((uint64_t*)pwLnX, wKey, \
                    (Word_t)2 << LOG((_nBL) - 1))) >= 0) { \
                assert(nPos < wr_nPopCnt(wRoot, (_nBL))); \
                goto xv_foundIt; \
            } \
            goto xv_break2
  #endif // else (cnBitsInD1 < cnLogBitsPerByte)

        DBGX(printf("\n# pwLnX %p *pwLnX 0x%zx wKey 0x%zx nBL %d\n",
                    pwLnX, *pwLnX, wKey, nBL));
  #ifdef LOOKUP
        //PREFETCH(pwr + wr_nPopCnt(wRoot, nBL) / 2);
      #ifdef PF_EK_XV
        PREFETCH(pwr);
      #endif // PF_EK_XV
  #endif // LOOKUP
        switch (nBL) {
                                XV_BLX( 2); XV_BLX( 3); XV_BLX( 4);
        XV_BLX( 5); XV_BLX( 6); XV_BLX( 7); XV_BLX( 8); XV_BLX( 9);
        XV_BLX(10); XV_BLX(11); XV_BLX(12); XV_BLX(13); XV_BLX(14);
        XV_BLX(15);
        default: DBG(printf("nBL %d\n", nBL)); assert(0);
                      XV_BLX(16);
                                    XV_BLX(17); XV_BLX(18); XV_BLX(19);
        XV_BLX(20); XV_BLX(21); XV_BLX(22); XV_BLX(23); XV_BLX(24);
        XV_BLX(25); XV_BLX(26); XV_BLX(27); XV_BLX(28); XV_BLX(29);
        XV_BLX(30); XV_BLX(31); XV_BLX(32); XV_BLX(33); XV_BLX(34);
        XV_BLX(35); XV_BLX(36); XV_BLX(37); XV_BLX(38); XV_BLX(39);
        XV_BLX(40); XV_BLX(41); XV_BLX(42); XV_BLX(43); XV_BLX(44);
        XV_BLX(45); XV_BLX(46); XV_BLX(47); XV_BLX(48); XV_BLX(49);
        XV_BLX(50); XV_BLX(51); XV_BLX(52); XV_BLX(53); XV_BLX(54);
        XV_BLX(55); XV_BLX(56); XV_BLX(57); XV_BLX(58); XV_BLX(59);
        XV_BLX(60); XV_BLX(61); XV_BLX(62); XV_BLX(63); XV_BLX(64);
        }
xv_break2:;

      #else // _PARALLEL_EK
        #error not coded yet

        Word_t wKeyRoot;

        int nPopCnt = wr_nPopCnt(wRoot, nBL);
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

      #endif // #else _PARALLEL_EK
        }

        break; // switch (nType) case T_EK_XV

xv_foundIt:;

      #if defined(REMOVE)
        goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(INSERT)
          #if !defined(RECURSIVE)
        if (nIncr > 0) { goto undo; } // undo counting
          #endif // !defined(RECURSIVE)
      #endif // defined(INSERT)

  #if defined(INSERT) || defined(LOOKUP)
        return &pwr[nPos];
  #else // defined(INSERT) || defined(LOOKUP)
        return KeyFound;
  #endif // defined(INSERT) || defined(LOOKUP)

    } // end of case T_EK_XV
  #endif // EK_XV

#endif // defined(EMBED_KEYS)

#if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)

  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case 112 + T_NULL:
    case  96 + T_NULL:
    case  80 + T_NULL:
    case  64 + T_NULL:
  #endif // defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case 48 + T_NULL:
    case 32 + T_NULL:
    case 16 + T_NULL:
  #endif // defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_NULL:
    {
        assert(wRoot == WROOT_NULL);

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

    } // end of switch (nType)

    DBGX(Checkpoint(qy, "exit switch stmt"));

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
      #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    assert((pwLnX != NULL) || (nBL == cnBitsPerWord));
      #endif // _RETURN_NULL_TO_INSERT_AGAIN
  #endif // B_JUDYL
    BJL(pwValue =)
        InsertGuts(qya, wKey, nPos
      #if defined(CODE_XX_SW)
                 , pLnUp, nBLUp
      #endif // defined(CODE_XX_SW)
                   );
  #ifdef B_JUDYL
      #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    if (pwValue == NULL) {
        assert((pwLnX != NULL) || (nBL == cnBitsPerWord));
        // How do we make sure we're not reinserting into a switch or leaf
        // which has already counted the insert before InsertGuts was called?
        assert(pLn->ln_wRoot != wRoot);
        wRoot = pLn->ln_wRoot;
        nBLR = nBL;
        goto insertAgain;
    }
      #endif // _RETURN_NULL_TO_INSERT_AGAIN
    DBGI(printf("Initializing pwValue %p for wKey 0x%zx\n", pwValue, wKey));
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
    DBGX(Checkpoint(qy, "undo"));
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
    goto restart;
restart:;
        DBGX(Checkpoint(qy, "restart"));
  #if defined(INSERT) || defined(REMOVE)
      #if !defined(RECURSIVE)
        nBL = nBLOrig;
        pLn = pLnOrig; // should we set pLnUp = NULL
          #ifndef QP_PLN
        pwRoot = &pLnOrig->ln_wRoot;
          #endif // !QP_PLN
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
    DBGX(Checkpoint(qy, "removeGutsAndCleanup"));
    RemoveGuts(qya, wKey);
      #endif // defined(REMOVE)
    goto cleanup;
cleanup:;
    DBGX(Checkpoint(qy, "cleanup"));
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
//#undef InsertGuts
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

    DBGL(printf("\n\n# JudyLGet pcvRoot %p wKey " OWx"\n",
                (void *)pcvRoot, wKey));

    Word_t wRoot = (Word_t)pcvRoot;
    if ((WROOT_NULL != 0) && (wRoot == 0)) {
        return 0;
    }

  #if (cwListPopCntMax != 0)
  // Use SEARCH_FROM_WRAPPER to handle nBL == cnBitsPerWord situations that
  // would otherwise complicate, i.e. slow down, Lookup.
  #if defined(SEARCH_FROM_WRAPPER)
    // Handle a top level T_LIST leaf here -- without calling Lookup.
    // For PP_IN_LINK a T_LIST leaf at the top has a pop count field in
    // the list, but T_LIST leaves that are not at the top do not. And,
    // for PP_IN_LINK there is no complete Link_t at the top -- only wRoot.
    // SEARCH_FROM_WRAPPER allows us avoid making the mainline PP_IN_LINK
    // T_LIST leaf handling code have to know or test if it is at the top.
    // Is T_LIST the only node type that is different at the top for
    // PP_IN_LINK? Doesn't the incomplete Link_t complicate Lookup for
    // the other node types?
    int nType = wr_nType(wRoot);
    if ((nType == T_LIST) && ((WROOT_NULL != T_LIST) || (wRoot != 0)))
    {
      #ifdef QP_PLN
        Link_t *pLn = STRUCT_OF(&pcvRoot, Link_t, ln_wRoot);
      #else // QP_PLN
        Word_t* pwRoot = (Word_t*)&pcvRoot;
      #endif // QP_PLN else
        int nBL = cnBitsPerWord;
      // PWR_xListPopCount is valid only at the top for PP_IN_LINK.
      // The first word in the list is used for pop count at the top.
      #if defined(B_JUDYL)
        int nPopCnt = gnListPopCnt(qy, nBL);
        int nPos = SearchListWord(ls_pwKeysX(wr_pwr(wRoot), nBL, nPopCnt),
                                  wKey, nBL, nPopCnt);
        return (nPos >= 0) ? (PPvoid_t)&gpwValues(qy)[~nPos] : NULL;
      #else // defined(B_JUDYL)
        return ListHasKeyWord(qy, nBL, wKey);
      #endif // #else defined(B_JUDYL)
    }
  #endif // defined(SEARCH_FROM_WRAPPER)
  #endif // (cwListPopCntMax != 0)

    return
  #ifdef B_JUDYL
        (PPvoid_t)
  #endif // B_JUDYL
            Lookup(wRoot, wKey);

#else // (cnDigitsPerWord > 1)

    // one big Bitmap

    DBGL(printf("\nJudy1Test(pcvRoot %p)\n", (void *)pcvRoot));

    if (pcvRoot == NULL) {
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
#if (cnDigitsPerWord > 1)

    int nBL = cnBitsPerWord;
      #ifdef QP_PLN
    Link_t *pLn = STRUCT_OF(ppvRoot, Link_t, ln_wRoot);
      #else // QP_PLN
    Word_t* pwRoot = (Word_t*)ppvRoot;
      #endif // QP_PLN else
    qv;

    // We use WROOT_NULL internally to represent an empty expanse.
    // But the user initializes the root word to 0.
    // If WROOT_NULL != 0 we change it before calling any internal
    // functions.
    if ((WROOT_NULL != 0) && (wRoot == 0)) {
        *(Word_t*)ppvRoot = wRoot = WROOT_NULL;
    }

  // Judy1LHTime and Judy1LHCheck put a -1 word before and after the root
  // word of the array solely so we can make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(((void**)ppvRoot)[-1] == (void*)-1);
    assert(((void**)ppvRoot)[ 1] == (void*)-1);
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
    // To make insert faster?
    int nPopCnt;
    //assert(cnListPopCntMax64 == auListPopCntMax[64]);
    //assert(cnListPopCntMax32 == auListPopCntMax[32]);
    if ((T_LIST == nType) && (pwr != NULL)
        && ((nPopCnt = gnListPopCnt(qy, cnBitsPerWord))
            !=
      #if (cnBitsPerWord == 64)
                cnListPopCntMax64
      #else // (cnBitsPerWord == 64)
                cnListPopCntMax32
      #endif // (cnBitsPerWord == 64)
            ))
    {
        int nPos = ~SearchListWord(ls_pwKeysX(pwr, cnBitsPerWord, nPopCnt),
                                   wKey, cnBitsPerWord, nPopCnt);
        if (nPos >= 0) {
            Word_t *pwKeys = ls_pwKeys(pwr, cnBitsPerWord);
            assert(ls_pwKeysX(pwr, cnBitsPerWord, nPopCnt) == pwKeys);
            BJL(Word_t *pwValues = gpwValues(qy));
            Word_t *pwrNew; (void)pwrNew;
            Word_t *pwKeysNew; (void)pwKeysNew;
            BJL(Word_t *pwValuesNew);
            if (ListSlotCnt(nPopCnt, cnBitsPerWord) < (nPopCnt + 1)) {
                // need bigger buffer
      #ifdef SFWI_INSERT_AT_LIST
          #ifdef REMOTE_LNX
                Word_t* pwLnX = NULL;
          #endif // REMOTE_LNX
                BJL(pwValue =) InsertAtList(qya, wKey, nPos
          #if defined(CODE_XX_SW)
                                          , /* pLnUp */ NULL
                                          , /* nBLUp */ 0
          #endif // defined(CODE_XX_SW)
                                            );
      #else // SWFI_INSERT_AT_LIST
                pwrNew = NewList(nPopCnt + 1, cnBitsPerWord);
                set_wr(wRoot, pwrNew, T_LIST);
                *pwRoot = wRoot;
                pwKeysNew = ls_pwKeys(pwrNew, cnBitsPerWord);
                assert(ls_pwKeysX(pwrNew, cnBitsPerWord, nPopCnt + 1)
                       == pwKeysNew);
                COPY(pwKeysNew, pwKeys, nPos);
                COPY(&pwKeysNew[nPos + 1], &pwKeys[nPos], nPopCnt - nPos);
          #ifdef B_JUDYL
                pwValuesNew = gpwValues(qy); // (pLn, nBLR)
                COPY(&pwValuesNew[~nPos + 1], &pwValues[~nPos + 1], nPos);
                COPY(&pwValuesNew[~nPopCnt], &pwValues[~nPopCnt + 1],
                     nPopCnt - nPos);
          #endif // B_JUDYL
                if (pwr != NULL) {
                    OldList(pwr, nPopCnt, cnBitsPerWord, T_LIST);
                }
                pwKeysNew[nPos] = wKey;
          #if defined(LIST_END_MARKERS)
                // pwKeysNew incorporates top pop count and markers
                pwKeysNew[nPopCnt + 1] = -1;
          #endif // defined(LIST_END_MARKERS)
                Set_xListPopCnt(pwRoot, cnBitsPerWord, nPopCnt + 1);
                BJL(pwValue = &pwValuesNew[~nPos]);
                DBGI(printf("PAD(pwKeysNew %p nPopCnt + 1 %d)\n",
                            pwKeysNew, nPopCnt + 1));
                PAD(pwKeysNew, nPopCnt + 1);
      #endif // SWFI_INSERT_AT_LIST
                BJL(*pwValue = 0);
                BJ1(status = Success);
            } else {
                MOVE(&pwKeys[nPos + 1], &pwKeys[nPos], nPopCnt - nPos);
                pwKeysNew = pwKeys;
      #ifdef B_JUDYL
                MOVE(&pwValues[~nPopCnt], &pwValues[~nPopCnt + 1],
                     nPopCnt - nPos);
                pwValuesNew = pwValues;
      #endif // B_JUDYL
                pwKeysNew[nPos] = wKey;
      #if defined(LIST_END_MARKERS)
                // pwKeysNew incorporates top pop count and markers
                pwKeysNew[nPopCnt + 1] = -1;
      #endif // defined(LIST_END_MARKERS)
                Set_xListPopCnt(pwRoot, cnBitsPerWord, nPopCnt + 1);
                BJL(pwValue = &pwValuesNew[~nPos]);
                PAD(pwKeysNew, nPopCnt + 1);
                BJL(*pwValue = 0);
                BJ1(status = Success);
            }
        } else {
            BJL(pwValue = &gpwValues(qy)[nPos]);
            BJ1(status = Failure);
        }
    } else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_I)
    {
        int nBL = cnBitsPerWord;
        BJL(pwValue) BJ1(status) = Insert(qy, wKey);
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
        if (!bHitDebugThreshold) {
            if ((wPopCntTotal > cwDebugThreshold)
                && ((cwDebugThresholdMax == 0)
                    || (wPopCntTotal <= cwDebugThresholdMax)))
            {
                bHitDebugThreshold = 1;
                if (cwDebugThreshold != 0) {
                    BJ1(printf("\n# Hit debug threshold 1.\n"));
                    BJL(printf("\n# Hit debug threshold L.\n"));
                }
            }
        } else if ((cwDebugThresholdMax != 0)
            && (wPopCntTotal > cwDebugThresholdMax)) {
                bHitDebugThreshold = 0;
                BJ1(printf("\n# Beyond debug threshold 1.\n"));
                BJL(printf("\n# Beyond debug threshold L.\n"));
        }
  #endif // defined(DEBUG)
    }

    DBGI(printf("\n# After Insert(wKey " OWx") Dump\n", wKey));
    DBGI(Dump((Word_t *)ppvRoot, wKey, cnBitsPerWord));
    DBGI(printf("\n"));

  #if ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)
  #if ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
    // Judy1Count really slows down testing for PP_IN_LINK.
      #ifdef B_JUDYL
    //assert((JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal)
    //    || bPopCntTotalIsInvalid);
      #else // B_JUDYL
    //assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
      #endif // B_JUDYL
  #endif // ! defined(POP_WORD_IN_LINK) || defined(DEBUG_COUNT)
  #endif // ! defined(PP_IN_LINK) || defined(DEBUG_COUNT)

  // Judy1LHTime and Judy1LHCheck put a -1 word before and after the root
  // word of the array solely so we can make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    if (ppvRoot[-1] != (void*)-1) {
        printf("ppvRoot %p\n", ppvRoot);
        printf("ppvRoot[-1] %p\n", ppvRoot[-1]);
    }
    assert(((void**)ppvRoot)[-1] == (void*)-1);
    assert(((void**)ppvRoot)[ 1] == (void*)-1);
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

    Word_t wRoot = *(Word_t*)ppvRoot;

    Word_t wByteNum, wByteMask;
    char c;

    DBGI(printf("\n# Judy1Set(ppvRoot %p wKey " OWx") wRoot " OWx"\n",
        (void *)ppvRoot, wKey, wRoot));

    if (wRoot == 0) {
        wRoot = JudyMalloc(EXP(cnBitsPerWord - cnLogBitsPerWord));
        assert(wRoot != 0);
        assert((wRoot & cnMallocMask) == 0);

        DBGI(printf("Malloc wRoot " OWx"\n", wRoot));

        *ppvRoot = (PPvoid_t)wRoot;
    }

    wByteNum = BitmapByteNum(wKey);
    wByteMask = BitmapByteMask(wKey);

    DBGI(printf("Judy1Set num " OWx" mask " OWx"\n", wByteNum, wByteMask));

    if ((c = ((char *)wRoot)[wByteNum]) & wByteMask) {
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
    assert(((void**)&pLn->ln_wRoot)[-1] == (void*)-1);
    assert(((void**)&pLn->ln_wRoot)[ 1] == (void*)-1);
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
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot);
    if (nType == T_LIST) {
        if (Judy1Test((Pcvoid_t)wRoot, wKey, PJError) == Failure) {
            status = Failure;
        } else {
            Word_t *pwr = wr_pwr(wRoot);
            Word_t wPopCnt = PWR_xListPopCnt(pwRoot, pwr, cnBitsPerWord);
            Word_t *pwListNew;
            if (wPopCnt != 1) {
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
            } else {
                wRoot = 0; // set_wr(wRoot, NULL, 0)
            }
            OldList(pwr, wPopCnt, cnBitsPerWord, nType);
            *pwRoot = wRoot;
            status = Success;
        }
    } else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_R)
    {
  #ifdef QP_PLN
        status = Remove(cnBitsPerWord, pLn, wKey);
  #else // QP_PLN
        status = Remove(cnBitsPerWord, (Word_t*)ppvRoot, wKey);
  #endif // QP_PLN else
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
                    JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL)
      #else // B_JUDYL
                    Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL)
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
    assert(((void**)&pLn->ln_wRoot)[-1] == (void*)-1);
    assert(((void**)&pLn->ln_wRoot)[ 1] == (void*)-1);
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
