// This file is #included in multiple .c files.
// With exactly one of LOOKUP, INSERT, REMOVE, COUNT and NEXT defined.
// It is not compiled on its own, i.e. without a wrapper.

#if (cnDigitsPerWord <= 1)

//#if (cnBitsPerDigit < cnBitsPerWord)
//#if (cnBitsInD1 < cnBitsPerWord)
// One big bitmap is implemented completely in Judy1Test, Judy1Set
// and Judy1Unset.  There is no need for Lookup, Insert and Remove.
#else // (cnDigitsPerWord <= 1)

#if defined(NEW_NEXT) || !defined(NEXT)

//#include <emmintrin.h>
//#include <smmintrin.h>
#include <immintrin.h> // __m128i

  #if defined(COUNT)
// Return the total number of keys in the subtrees rooted by links that
// precede the specified link in this switch.
static Word_t
CountSw(qpa,
        Word_t wIndex, // index of relevant link in switch
        int nLinks)
{
    qva; (void)nLinks;
    int nBLR = gnBLR(qy);
    assert(wRoot != WROOT_NULL);
    DBGC(printf("\n# CountSw nBL %d nType %d nBLR %d wIndex %zd nLinks %d\n",
                nBL, nType, nBLR, wIndex, nLinks));
    Word_t wPopCnt = 0;
    Link_t *pLinks =
#if defined(CODE_BM_SW)
        tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
#if defined(CODE_LIST_SW)
        tp_bIsListSw(nType) ? gpListSwLinks(qy) :
#endif // defined(CODE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr);
    (void)pLinks;
  #ifdef XX_LISTS
    // nBL == cnBitsPerWord is the only time wIndex will be after the last
    // link in the switch?
    // What are we doing here?
    if (!tp_bIsBmSw(nType)) { // don't have to look for T_XX_SW in BmSw.
        int nBW = gnBW(qy, nBLR);
        assert((Word_t)nLinks == EXP(nBW));
        assert(wIndex < (Word_t)nLinks);
        int nBLLoop = nBLR - nBW;
        Link_t *pLnLoop = &pLinks[wIndex]; // not to be counted
        Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
        Word_t wRootLoop = *pwRootLoop;
        int nTypeLoop = wr_nType(wRootLoop);
        if (nTypeLoop == T_XX_LIST) {
            // What are we doing here?
            int nBLRLoop = gnListBLR(qyx(Loop)); // not to be counted
            assert(nBLRLoop > nBLLoop);
            assert(nBLRLoop <= cnBitsPerWord);
            // Why are we changing wIndex?
            // Back up to the beginning of this shared list.
            wIndex &= ~MSK(nBLRLoop - nBLLoop);
            DBGC(printf("# CountSw wIndex " OWx"\n", wIndex));
        }
    }
  #endif // XX_LISTS
  #if cnSwCnts == 0
    #define _NO_SW_CNTS // Some switches must be counted w/o sw_awCnts.
  #elif defined(CODE_BM_SW)
    #define _NO_SW_CNTS
  #elif defined(CODE_XX_SW)
    #define _NO_SW_CNTS
  // To do: other types of switches.
  #endif // cnSwCnts == 0 elif CODE_BM_SW elif CODE_XX_SW
  #if cnSwCnts != 0 && defined(_NO_SW_CNTS)
    if (tp_bIsBmSw(nType))
  #endif // cnSwCnts != 0 && _NO_SW_CNTS
  #ifdef _NO_SW_CNTS
    {
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
        wPopCnt = CountSwLoop(qya, ww, wwLimit - ww);
//printf("# CountSwLoop returned %zd\n", wPopCnt);
        assert(((int)wIndex < nLinks)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                   || (nBL == cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
               );
        if (ww != 0) {
            assert(wRoot != WROOT_NULL);
            Word_t wPopCntSw = gwPopCnt(qya, nBLR);
            DBGC(printf("wPopCntSw 0x%zx wPopCnt 0x%zx\n",
                        wPopCntSw, wPopCnt));
            wPopCnt = wPopCntSw - wPopCnt;
            DBGC(printf("wPopCntSw " OWx"\n", wPopCntSw));
        }
    }
  #endif // _NO_SW_CNTS
  #if cnSwCnts != 0 && defined(_NO_SW_CNTS)
    else
  #endif // cnSwCnts != 0 && _NO_SW_CNTS
  #if cnSwCnts != 0
    {
        int nBW = gnBW(qy, nBLR);
        assert(nLinks == (int)EXP(nBW));
        DBGC(printf("CountSw nBW %d\n", nBW));
        if (nBLR <= 16) {
            // Four subexpanse counts per word.
      #if defined(CODE_XX_SW) || !defined(_CONSTANT_NBW)
            int nShift = (nBW > cnLogSwCnts + 2) ? (nBW - cnLogSwCnts - 2) : 0;
            // Would like to resolve this test at compile time if possible.
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)[ii];
                }
            } else
      #else // CODE_XX_SW || !_CONSTANT_NBW
            assert(nBW >= cnLogSwCnts + 2);
            int nShift = nBW - cnLogSwCnts - 2;
      #endif // CODE_XX_SW || !_CONSTANT_NBW else
            {
                int nCntNum = (((wIndex << cnLogSwCnts) + (1 << (nBW - 3)))
                                   >> (nBW - 2));
                int nCum = 0;
                for (int ii = 0; ii < nCntNum; ++ii) {
                    nCum += ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)[ii];
                }
                int xx = nCntNum << nShift;
                // If nShift is close to nBW then it may not be worth the
                // trouble of figuring out if the 2nd half is shorter than
                // the first half.
                if ((wIndex >> (nShift - 1)) & 1) { // subtract
                    wPopCnt = nCum - CountSwLoop(qya, wIndex, xx - wIndex);
                } else {
                    wPopCnt = nCum + CountSwLoop(qya, xx, wIndex - xx);
                }
            }
        } else if (nBLR <= 32) {
            // Two subexpanse counts per word.
      #if defined(CODE_XX_SW) || !defined(_CONSTANT_NBW)
            // Would like to resolve this test at compile time if possible.
            int nShift = (nBW > cnLogSwCnts + 1) ? (nBW - cnLogSwCnts - 1) : 0;
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((uint32_t*)((Switch_t*)pwr)->sw_awCnts)[ii];
                }
            } else
      #else // CODE_XX_SW || !_CONSTANT_NBW
            assert(nBW >= cnLogSwCnts + 1);
            int nShift = nBW - cnLogSwCnts - 1;
      #endif // CODE_XX_SW || !_CONSTANT_NBW else
            {
                int nCntNum = (((wIndex << cnLogSwCnts) + (1 << (nBW - 2)))
                                   >> (nBW - 1));
                Word_t wCum = 0;
                for (int i = 0; i < nCntNum; ++i) {
                    wCum += ((uint32_t*)((Switch_t*)pwr)->sw_awCnts)[i];
                }
                int xx = nCntNum << nShift;
                if ((wIndex >> (nShift - 1)) & 1) { // subtract from
                    wPopCnt = wCum - CountSwLoop(qya, wIndex, xx - wIndex);
                } else {
                    wPopCnt = wCum + CountSwLoop(qya, xx, wIndex - xx);
                }
            }
        } else {
            // One subexpanse count per word.
      #if defined(CODE_XX_SW) || !defined(_CONSTANT_NBW)
            // Would like to resolve this test at compile time if possible.
            int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((Switch_t*)pwr)->sw_awCnts[ii];
                }
            } else
      #else // CODE_XX_SW || !_CONSTANT_NBW
            assert(nBW >= cnLogSwCnts);
            int nShift = nBW - cnLogSwCnts;
      #endif // CODE_XX_SW || !_CONSTANT_NBW else
            {
                int nCntNum = (((wIndex << cnLogSwCnts) + (1 << (nBW - 1)))
                                   >> (nBW - 0));
                Word_t wCum = 0;
                for (int i = 0; i < nCntNum; ++i) {
                    wCum += ((Switch_t*)pwr)->sw_awCnts[i];
                }
      #if cnSwCnts == 1
                if (nCntNum) {
                    wPopCnt = wCum + CountSwLoop(qya,
                                                 nLinks/2, wIndex - nLinks/2);
                } else
      #endif // cnSwCnts == 1
                {
                    int xx = nCntNum << nShift;
                    if ((wIndex >> (nShift - 1)) & 1) { // subtract
                        wPopCnt = wCum - CountSwLoop(qya, wIndex, xx - wIndex);
                    } else {
                        wPopCnt = wCum + CountSwLoop(qya, xx, wIndex - xx);
                    }
                }
            }
        }
    }
  #endif // cnSwCnts == 0
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

    // Non-zero wPrefixMismatch means prefix mismatch.
    // If wPrefixMismatch is zero it means no prefix mismatch.
    // Caller can derive wPrefix from wKey and wPrefixMismatch.
    // Caller can figure out how wPrefix compares to (wKey & ~MSK(nBLR)).
    wPrefixMismatch = (wKey - wPrefix) >> nBLR;

    //printf("wKey 0x%zx wPrefix 0x%zx nBLR %d\n", wKey, wPrefix, nBLR);
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
    DBGX(Checkpoint(qya, "SwCleanup"));
    DBGX(printf("wKey 0x%016zx nBLR %d\n", wKey, nBLR));
    // Cleanup is for adjusting tree after successful insert or remove.
    // It is not for undoing counts after unsuccessful insert or remove.
  #if defined(INSERT)
    if (0
      #if defined(CODE_BM_SW)
        || (tp_bIsBmSw(Get_nType(&wRoot)) && InflateBmSwTest(qya))
      #endif // defined(CODE_BM_SW)
      #ifdef BITMAP
        || ((cn2dBmMaxWpkPercent != 0)
            && (nBLR == cnBitsLeftAtDl2)
            && (GetPopCnt(qya) * cn2dBmMaxWpkPercent * cnBitsPerWord
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

// Adjust pop count in a switch.
// Increment for insert on the way down.
// Decrement for remove on the way down.
// Also used to undo itself after we discover insert or remove is redundant.
static inline Word_t
SwIncr(qpa, int nBLR, int nDigit, int nBW, int nIncr)
{
    qva; (void)nBLR; (void)nDigit; (void)nBW; (void)nIncr;
  #if defined(INSERT) || defined(REMOVE)
      #if cnSwCnts != 0
        Word_t* pwCnts = ((Switch_t*)pwr)->sw_awCnts;
        if (nBLR <= 16) {
            int nShift = (nBW > cnLogSwCnts + 2) ? (nBW - cnLogSwCnts - 2) : 0;
            ((uint16_t*)pwCnts)[nDigit >> nShift] += nIncr;
        } else if (nBLR <= 32) {
            int nShift = (nBW > cnLogSwCnts + 1) ? (nBW - cnLogSwCnts - 1) : 0;
            ((uint32_t*)pwCnts)[nDigit >> nShift] += nIncr;
        } else {
          #if cnSwCnts == 1
            if (!(nDigit >> (nBW - 1)))
          #endif // cnSwCnts == 1
            {
                int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
                pwCnts[nDigit >> nShift] += nIncr;
            }
        }
      #endif // cnSwCnts != 0
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBL < cnBitsPerWord)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    {
        // We may temporarily increment above EXP(nBLR) when trying to insert
        // a key into a full-pop subtree.
        // If we could figure out how to abort the insert early that would
        // probably be better.
        Word_t wPopCnt = gwPopCnt(qya, nBLR) + nIncr;
        swPopCnt(qya, nBLR, wPopCnt);
        return wPopCnt;
    }
  #endif // defined(INSERT) || defined(REMOVE)
    return 0;
}

#ifdef _LNX
  #ifdef QP_PLN
    #define swapynew  pLnNew, pwLnXNew
  #else // QP_PLN
    #define swapynew  &pLn, pLnNew, pwLnXNew
  #endif // QP_PLN else
#else // _LNX
  #ifdef QP_PLN
    #define swapynew  pLnNew
  #else // QP_PLN
    #define swapynew  &pLn, pLnNew
  #endif // QP_PLN else
#endif // _LNX else

static inline void
SwAdvance(pqpa,
  #ifndef QP_PLN
          Link_t** ppLn,
  #endif // !QP_PLN
          Link_t *pLnNew,
  #ifdef _LNX
          Word_t* pwLnXNew,
  #endif // _LNX
          int nBW, int *pnBLR)
{
    pqva;
    *pnBL = *pnBLR - nBW;
    *pnBLR = *pnBL;
    // Be very careful with pwRoot from pqp.
    // It might not mean what you think it means.
  #ifdef _LNX
    *ppwLnX = pwLnXNew;
  #endif // _LNX
    *ppLn = pLnNew;
  #ifdef QP_PLN
    *pwRoot = pLnNew->ln_wRoot;
    DBGX(printf("# SwAdvance: sw nBL %d pLn %p wRoot 0x%zx\n",
                *pnBL, (void*)*ppLn, *pwRoot));
  #else // QP_PLN
    *ppwRoot = &pLnNew->ln_wRoot;
    *pwRoot = **ppwRoot;
    DBGX(printf("# SwAdvance: sw nBL %d pwRoot %p wRoot 0x%zx\n",
                *pnBL, *ppwRoot, *pwRoot));
  #endif // QP_PLN else
    // Why aren't we updating *pnType and *ppwr here?
}

#ifdef SKIP_PREFIX_CHECK
#if defined(COMPRESSED_LISTS) || defined(BITMAP) // should be implied
#ifdef LOOKUP // should be implied

    #define _DO_PREFIX_CHECK_AT_LEAF

  #ifdef ALWAYS_CHECK_PREFIX_AT_LEAF
#define _B_NEED_PREFIX_CHECK_ARG
  #else // ALWAYS_CHECK_PREFIX_AT_LEAF
#define _B_NEED_PREFIX_CHECK_ARG  , bNeedPrefixCheck
  #endif // ALWAYS_CHECK_PREFIX_AT_LEAF else

  #ifdef SAVE_PREFIX_TEST_RESULT
#define _SAVED_PREFIX_TEST_RESULT_ARG  , wPrefixMismatch
  #else // SAVE_PREFIX_TEST_RESULT
#define _SAVED_PREFIX_TEST_RESULT_ARG  , pwrUp
  #endif // SAVE_PREFIX_TEST_RESULT else

  #ifdef SAVE_PREFIX
#define _SAVED_PREFIX_ARGS  , pLnPrefix, pwrPrefix, nBLRPrefix
  #else // SAVE_PREFIX
#define _SAVED_PREFIX_ARGS
  #endif // SAVE_PREFIX else

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
#endif // COMPRESSED_LISTS || BITMAP
#endif // SKIP_PREFIX_CHECK

#ifdef _DO_PREFIX_CHECK_AT_LEAF
    #define PREFIX_CHECK_AT_LEAF(qy, _wKey) \
        PrefixCheckAtLeaf(qy, _wKey \
                          _B_NEED_PREFIX_CHECK_ARG \
                          _SAVED_PREFIX_TEST_RESULT_ARG \
                          _SAVED_PREFIX_ARGS \
                          )
#else // _DO_PREFIX_CHECK_AT_LEAF
    #define PREFIX_CHECK_AT_LEAF(qy, _wKey)  Success
#endif // _DO_PREFIX_CHECK_AT_LEAF else

#ifdef LOOKUP
#ifdef _AUG_TYPE_X_LOOKUP
    #define _AUG_TYPE_X
#endif // _AUG_TYPE_X_LOOKUP
#endif // LOOKUP

#if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    #define _AUG_TYPE_8_SW
#elif defined(AUG_TYPE_8_SW_NEXT) && defined(NEXT)
    #define _AUG_TYPE_8_SW
#endif // AUGMENT_TYPE_8 && LOOKUP elif AUG_TYPE_8_SW_NEXT && NEXT

#if defined(AUGMENT_TYPE) && defined(LOOKUP) || defined(_AUG_TYPE_8_SW)
    #define _AUG_TYPE
#elif defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
    #define _AUG_TYPE
#endif

#ifdef _AUG_TYPE

static int
AugTypeBits(int nBL)
{
  #ifdef AUG_TYPE_64_LOOKUP
    return (nBL << (cnBitsTypeMask - 0)) - (1 << cnBitsTypeMask);
  #elif defined(AUG_TYPE_32_LOOKUP)
    return (nBL << (cnBitsTypeMask - 1)) - (1 << cnBitsTypeMask);
  #elif defined(AUG_TYPE_16_LOOKUP)
    return (nBL << (cnBitsTypeMask - 2)) - (1 << cnBitsTypeMask);
  #elif defined(AUG_TYPE_8_LOOKUP)
    return (nBL << (cnBitsTypeMask - 3)) - (1 << cnBitsTypeMask);
  #elif defined(AUGMENT_TYPE_8_PLUS_4)
    // cnBitsPerDigit == 8
    //  4 <= cnBitsLeftAtDl1 <  12
    // 12 <= cnBitsLeftAtDl2 <  20
    // 20 <= cnBitsLeftAtDl3 <  28
      #if 1
    return
        (((nBL + 4) << (cnBitsTypeMask - 3)) & ~MSK(cnBitsTypeMask))
            - (1 << cnBitsTypeMask);
      #else
    uint64_t x = 0x7060504030201000;
    return ((uint8_t*)&x)[(nBL+4)/8-1];
      #endif
  #elif defined(AUGMENT_TYPE_8)
      #if 1
    return (nBL << (cnBitsTypeMask - 3)) - (1 << cnBitsTypeMask);
      #else
    uint64_t x = 0x7060504030201000;
    return ((uint8_t*)&x)[nBL/8-1];
      #endif
  #else // AUGMENT_TYPE_8
      #if 1
    return (LOG(nBL - 1) - 2) << cnBitsTypeMask; // nLogBL << cnBitsTypeMask
      #else
    uint64_t x = 0x3030303020201000;
    return ((uint8_t*)&x)[nBL/8-1];
      #endif
  #endif // else AUGMENT_TYPE_8
}

#ifndef _AUG_TYPE_X
static int
AugTypeBitsInv(int nAugTypeBits)
{
    ASSERT((nAugTypeBits & cnTypeMask) == 0);
  #ifdef AUG_TYPE_64_LOOKUP
    return (nAugTypeBits + (1 << cnBitsTypeMask)) >> (cnBitsTypeMask - 0);
  #elif defined(AUG_TYPE_32_LOOKUP)
    return (nAugTypeBits + (1 << cnBitsTypeMask)) >> (cnBitsTypeMask - 1);
  #elif defined(AUG_TYPE_16_LOOKUP)
    return (nAugTypeBits + (1 << cnBitsTypeMask)) >> (cnBitsTypeMask - 2);
  #elif defined(AUG_TYPE_8_LOOKUP)
    return (nAugTypeBits + (1 << cnBitsTypeMask)) >> (cnBitsTypeMask - 3);
  #elif defined(AUGMENT_TYPE_8_PLUS_4)
    // If cnBitsLeftAtDl3 + cnBitsPerDigit * 5 < cnBitsPerWord, then
    // ((nAugTypeBits + 16) >> 1) == 64 is ambiguous.
    assert((cnBitsLeftAtDl3 + cnBitsPerDigit * 5 >= cnBitsPerWord)
        || (((nAugTypeBits + 16) >> 1) < 64));
      #if 1
    uint64_t x = ( (Word_t)cnBitsPerWord                         << 56)
               + (((Word_t)cnBitsLeftAtDl3 + cnBitsPerDigit * 4) << 48)
               + (((Word_t)cnBitsLeftAtDl3 + cnBitsPerDigit * 3) << 40)
               + (((Word_t)cnBitsLeftAtDl3 + cnBitsPerDigit * 2) << 32)
               + ((        cnBitsLeftAtDl3 + cnBitsPerDigit    ) << 24)
               + (         cnBitsLeftAtDl3                       << 16)
               + (         cnBitsLeftAtDl2                       <<  8)
               + (         cnBitsLeftAtDl1                            );
    return ((uint8_t*)&x)[nAugTypeBits/(1 << cnBitsTypeMask)];
      #else
    if (nAugTypeBits == 0) {
        return cnBitsInD1;
    }
    if (nAugTypeBits == 16) {
        return cnBitsLeftAtDl2;
    }
    if (cnBitsLeftAtDl3 > 24) { // compile time test
        if (nAugTypeBits == 112) {
            return cnBitsPerWord;
        }
    }
    return cnBitsLeftAtDl3 + (nAugTypeBits >> 1) - 16;
      #endif
  #elif defined(AUGMENT_TYPE_8)
      #if 1
    return (nAugTypeBits + (1 << cnBitsTypeMask)) >> (cnBitsTypeMask - 3);
      #else
    uint64_t x = 0x4038302820181008;
    return ((uint8_t*)&x)[nAugTypeBits/16];
      #endif
  #else // AUGMENT_TYPE_8
    assert(!(nAugTypeBits & ~(1 << cnBitsTypeMask)));
      #if 1
    return 8 << (nAugTypeBits >> cnBitsTypeMask);
      #else
    uint64_t x = 0x4040404020201008;
    return ((uint8_t*)&x)[nAugTypeBits/16];
      #endif
  #endif // else AUGMENT_TYPE_8
}
#endif // !_AUG_TYPE_X

#endif // _AUG_TYPE

#if (cwListPopCntMax != 0)
  // _T_LIST indicates t_list label is needed.
  // Use _T_LIST to allow less than 80 character lines.
  #if defined(_AUG_TYPE_X) && !defined(BL_SPECIFIC_LIST)
     #define _T_LIST
  #elif cnBitsInD1 <= 8 || !defined(LOOKUP)
     #define _T_LIST
  #elif !defined(AUGMENT_TYPE) || defined(AUGMENT_TYPE_8_PLUS_4)
     #define _T_LIST
  #elif defined(AUG_TYPE_8_SW_NEXT) && defined(NEXT)
     #define _T_LIST
  #endif // cnBitsInD1 <= 8 || !AUG_TYPE || AUG_TYPE_8_PLUS_4 || !LOOKUP
#endif // (cwListPopCntMax != 0)

#ifdef EMBED_KEYS
#ifndef B_JUDYL
#ifdef AUGMENT_TYPE_8
#ifndef AUGMENT_TYPE_8_PLUS_4
#ifdef LOOKUP
    #define _AUG_TYPE_8_EK
#endif // LOOKUP
#endif // !AUGMENT_TYPE_8_PLUS_4
#endif // AUGMENT_TYPE_8
#ifdef _AUG_TYPE_X
    #define _AUG_TYPE_X_EK
#endif // _AUG_TYPE_X
#endif // !B_JUDYL
#endif // EMBED_KEYS

#ifdef JUMP_TABLE

  #ifdef SEPARATE_T_NULL
    #define SEPARATE_T_NULL_COMMA  &&t_null,
  #else // SEPARATE_T_NULL
    #define SEPARATE_T_NULL_COMMA
  #endif // else SEPARATE_T_NULL

  #ifndef _T_LIST
    #define t_list  NULL
  #endif // _T_LIST

  #if (cwListPopCntMax != 0)
    #define LIST_COMMA(_label)  (_label),
  #else // (cwListPopCntMax != 0)
    #define LIST_COMMA(_label)
  #endif // else (cwListPopCntMax != 0)

  #ifdef XX_LISTS
    #define XX_LISTS_COMMA(_label)  (_label),
  #else // XX_LISTS
    #define XX_LISTS_COMMA(_label)
  #endif // else XX_LISTS

  #ifdef SKIP_TO_LIST
    #define SKIP_TO_LIST_COMMA(_label)  (_label),
  #else // SKIP_TO_LIST
    #define SKIP_TO_LIST_COMMA(_label)
  #endif // else SKIP_TO_LIST

  #ifdef UA_PARALLEL_128
    #define UA_PARALLEL_128_COMMA(_label)  (_label),
  #else // UA_PARALLEL_128
    #define UA_PARALLEL_128_COMMA(_label)
  #endif // else UA_PARALLEL_128

  #ifdef BITMAP
    #define BITMAP_COMMA(_label)  (_label),
  #else // BITMAP
    #define BITMAP_COMMA(_label)
  #endif // else BITMAP

  #ifdef SKIP_TO_BITMAP
    #define SKIP_TO_BM_COMMA(_label)  (_label),
  #else // SKIP_TO_BITMAP
    #define SKIP_TO_BM_COMMA(_label)
  #endif // else SKIP_TO_BITMAP

  #ifdef UNPACK_BM_VALUES
    #define UNPACKED_BM_COMMA(_label)  (_label),
  #else // UNPACK_BM_VALUES
    #define UNPACKED_BM_COMMA(_label)
  #endif // UNPACK_BM_VALUES

  #ifdef CODE_LIST_SW
    #define LIST_SW_COMMA(_label)  (_label),
  #else // CODE_LIST_SW
    #define LIST_SW_COMMA(_label)
  #endif // else CODE_LIST_SW

  #ifdef SKIP_TO_LIST_SW
    #define SKIP_TO_LIST_SW_COMMA(_label)  (_label),
  #else // SKIP_TO_LIST_SW
    #define SKIP_TO_LIST_SW_COMMA(_label)
  #endif // else SKIP_TO_LIST_SW

  #ifdef CODE_BM_SW
    #define BM_SW_COMMA(_label)  (_label),
  #else // CODE_BM_SW
    #define BM_SW_COMMA(_label)
  #endif // else CODE_BM_SW

  #ifdef SKIP_TO_BM_SW
    #define SKIP_TO_BM_SW_COMMA(_label)  (_label),
  #else // SKIP_TO_BM_SW
    #define SKIP_TO_BM_SW_COMMA(_label)
  #endif // else SKIP_TO_BM_SW

  #ifdef CODE_XX_SW
    #define XX_SW_COMMA(_label)  (_label),
  #else // CODE_XX_SW
    #define XX_SW_COMMA(_label)
  #endif // else CODE_XX_SW

  #ifdef SKIP_TO_XX_SW
    #define SKIP_TO_XX_SW_COMMA(_label)  (_label),
  #else // SKIP_TO_XX_SW
    #define SKIP_TO_XX_SW_COMMA(_label)
  #endif // else SKIP_TO_XX_SW

  #define SKIP_TO_FULL_BM_SW_COMMA(_label)
  #if defined(RETYPE_FULL_BM_SW) && !defined(USE_BM_IN_NON_BM_SW)
    #define FULL_BM_SW_COMMA(_label)  (_label),
      #ifdef SKIP_TO_BM_SW
    #undef  SKIP_TO_FULL_BM_SW_COMMA
    #define SKIP_TO_FULL_BM_SW_COMMA(_label)  (_label),
      #endif // else SKIP_TO_BM_SW
  #else // RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW
    #define FULL_BM_SW_COMMA(_label)
  #endif // else RETYPE_FULL_BM_SW && !USE_BM_IN_NON_BM_SW

  #ifdef EMBED_KEYS
      #ifdef _AUG_TYPE_8_EK
    #define t_embedded_keys  t_ek_0
      #endif // _AUG_TYPE_8_EK
    #define EK_COMMA(_label)  (_label),
  #else // EMBED_KEYS
    #define EK_COMMA(_label)
  #endif // else EMBED_KEYS

  #ifdef EK_XV
    #define EK_XV_COMMA(_label)  (_label),
  #else // EK_XV
    #define EK_XV_COMMA(_label)
  #endif // else EK_XV

  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    #define SW_COMMA(_label)  NULL, // place holder for missing &&t_switch
  #else // AUGMENT_TYPE && LOOKUP
    #define SW_COMMA(_label)  (_label),
  #endif // else AUGMENT_TYPE && LOOKUP

  #ifdef SKIP_LINKS
    #define SKIP_TO_SW_COMMA(_label)  (_label),
  #else // SKIP_LINKS
    #define SKIP_TO_SW_COMMA(_label)
  #endif // else SKIP_LINKS

  // T_SKIP_TO_SWITCH is an enumeration constant; not a macro.
  // It's value is treated as zero by the preprocessor.
  #ifdef _LVL_IN_TYPE
    #define JT_LVL_IN_TYPE \
        &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch, \
        &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch, \
        &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch, \
        &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch, \
        &&t_skip_to_switch, &&t_skip_to_switch, &&t_skip_to_switch
  #else // _LVL_IN_TYPE
    #define JT_LVL_IN_TYPE  NULL
  #endif // else _LVL_IN_TYPE

  #define JT_ENTRIES \
        SEPARATE_T_NULL_COMMA \
        LIST_COMMA(&&t_list) \
        XX_LISTS_COMMA(&&t_xx_list) \
        SKIP_TO_LIST_COMMA(&&t_skip_to_list) \
        UA_PARALLEL_128_COMMA(&&t_list_ua) \
        BITMAP_COMMA(&&t_bitmap) \
        SKIP_TO_BM_COMMA(&&t_skip_to_bitmap) \
        UNPACKED_BM_COMMA(&&t_unpacked_bm) \
        LIST_SW_COMMA(&&t_list_sw) \
        SKIP_TO_LIST_SW_COMMA(&&t_skip_to_list_sw) \
        BM_SW_COMMA(&&t_bm_sw) \
        SKIP_TO_BM_SW_COMMA(&&t_skip_to_bm_sw) \
        XX_SW_COMMA(&&t_xx_sw) \
        SKIP_TO_XX_SW_COMMA(&&t_skip_to_xx_sw) \
        FULL_BM_SW_COMMA(&&t_full_bm_sw) \
        SKIP_TO_FULL_BM_SW_COMMA(&&t_skip_to_full_bm_sw) \
        EK_COMMA(&&t_embedded_keys) \
        EK_XV_COMMA(&&t_ek_xv) \
        SW_COMMA(&&t_switch) \
        SKIP_TO_SW_COMMA(&&t_skip_to_switch) \
        JT_LVL_IN_TYPE

#endif // JUMP_TABLE

#define WROOT_IS_NULL(_nType, _wRoot) \
      ((wr_nType(WROOT_NULL) == (_nType)) && ((_wRoot) == WROOT_NULL))

#ifndef LOOKUP
  #define _USE_SEARCH_LIST
#elif defined(B_JUDYL)
  #ifdef HASKEY_FOR_JUDYL_LOOKUP
#define _USE_HAS_KEY
  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
#define _USE_SEARCH_LIST
  #else // HASKEY_FOR_JUDYL_LOOKUP elif SEARCH_FOR_JUDYL_LOOKUP
#define _USE_LOCATE_KEY
  #endif // HASKEY_FOR_JUDYL_LOOKUP elif SEARCH_FOR_JUDYL_LOOKUP else
#else // LOOKUP elif B_JUDYL
  #ifdef SEARCH_FOR_JUDY1_LOOKUP
#define _USE_SEARCH_LIST
  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
#define _USE_LOCATE_KEY
  #else // SEARCH_FOR_JUDY1_LOOKUP elif LOCATEKEY_FOR_JUDY1_LOOKUP
#define _USE_HAS_KEY
  #endif // SEARCH_FOR_JUDY1_LOOKUP elif LOCATEKEY_FOR_JUDY1_LOOKUP else
#endif // LOOKUP elif B_JUDYL else

#if defined(LOOKUP_NO_LIST_SEARCH) && defined(LOOKUP)
  #define SEARCH_LIST(_suffix, qya, _nBLR, _wKey)  0
#elif defined(_USE_HAS_KEY) // LOOKUP_NO_LIST_SEARCH && LOOKUP
  #define SEARCH_LIST(_suffix, qya, _nBLR, _wKey) \
    -!ListHasKey##_suffix(qya, nBLR, wKey)
#elif defined(_USE_LOCATE_KEY) // no-search elif _USE_HAS_KEY
  #define SEARCH_LIST(_suffix, qya, _nBLR, _wKey) \
    LocateKeyInList##_suffix(qya, nBLR, wKey)
#else // no-search elif _USE_HAS_KEY elif _USE_LOCATE_KEY
  #define SEARCH_LIST(_suffix, qya, _nBLR, _wKey) \
    SearchList##_suffix(qya, nBLR, wKey)
#endif // no-search elif _USE_HAS_KEY elif _USE_LOCATE_KEY else

IF_LOOKUP(static BJL(Word_t*)BJ1(Status_t) Lookup(Word_t wRootArg,
                                                   Word_t wKey))
#ifdef NEXT_QPA
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(qpa, Word_t* pwKey))
#elif defined(NEXT_QP) // NEXT_QPA
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(qp, Word_t* pwKey))
#else // NEXT_QPA elif NEXT_QP
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(Word_t wRootArg,
                                                     Word_t* pwKey))
#endif // NEXT_QPA elif NEXT_QP else
IF_INSERT(       BJL(Word_t*)BJ1(Status_t) Insert(qpa, Word_t wKey))
IF_REMOVE(                       Status_t  Remove(qpa, Word_t wKey))
IF_COUNT (                       Word_t    Count (qpa, Word_t wKey))
#ifdef NEW_NEXT_IS_EXCLUSIVE
// Find the first present key greater than *pwKey.
#else // NEW_NEXT_IS_EXCLUSIVE
// Find the first present key greater than or equal to *pwKey.
#endif // NEW_NEXT_IS_EXCLUSIVE else
// If such a key exists, then set *pwKey and return Success.
// Otherwise return Failure.
// NextL should return pwValue.
// "inline" makes a difference.
{
    IF_NOT_LOOKUP(IF_NOT_NEXT(qva));
  #if defined(NEXT_QPA) && defined(NEXT)
    qva;
  #elif defined(NEXT_QP) && defined(NEXT) // NEXT_QPA && NEXT
    qv;
      #ifdef _LNX
    Word_t* pwLnX = NULL; // (void)pwLnX;
      #endif // _LNX
  #elif defined(NEXT) // NEXT_QPA && NEXT elif NEXT_QP && NEXT
    #define _NEXT_WROOT
  #endif // NEXT_QPA && NEXT elif NEXT_QP && NEXT elif NEXT
  #if defined(LOOKUP) || defined(_NEXT_WROOT)
    int nBL = cnBitsPerWord;
    Word_t wRoot = wRootArg;
    // Only wRoot word of pLn at top.
    Link_t* pLn = STRUCT_OF(&wRootArg, Link_t, ln_wRoot); // (void)pLn;
      #ifndef QP_PLN
    Word_t* pwRoot = &wRootArg; // (void)pwRoot;
      #endif // !QP_PLN
      #ifdef _LNX
    Word_t* pwLnX = NULL; // (void)pwLnX;
      #endif // _LNX
    // nBL, pLn and wRoot of qy are set up
    int nType;
    DBGX(nType = -1); // for compiler for qv in Checkpoint
    Word_t *pwr;
    DBGX(pwr = NULL); // for compiler for qv in Checkpoint
  #endif // LOOKUP || _NEXT_WROOT
  #ifdef NEXT
    Word_t wKey = *pwKey;
      #ifdef NEW_NEXT_IS_EXCLUSIVE
    if (++wKey == 0) {
        return 0;
    }
      #endif // NEW_NEXT_IS_EXCLUSIVE
  #endif // NEXT
  #ifndef RECURSIVE
  #ifndef LOOKUP
  #ifndef COUNT
    int nBLOrig = nBL; (void)nBLOrig;
    Link_t *pLnOrig = pLn; (void)pLnOrig;
  #endif // !LOOKUP
  #endif // !COUNT
  #endif // !RECURSIVE
    // nBL, pLn, and wRoot are set up
    DBGX(printf("# %s nBL %d pLn %p wRoot 0x%zx wKey 0x%zx\n",
                strLookupOrInsertOrRemove, nBL, (void*)pLn, wRoot, wKey));
  #if defined(INSERT) && defined(B_JUDYL)
    Word_t *pwValue = NULL;
  #endif // INSERT && B_JUDYL
    // gcc thinks nBW may be used uninitialized in tryNextDigit
    // for USE_XX_SW if we don't initialize it here. I can't figure out why.
    int nBW = 0;
    Link_t *pLnNew;
    // wDigit needs this broad scope only for COUNT.
    // I wonder if it would help performance if we were to define this
    // in the narrower scope for LOOKUP?
    // In the one quick test I did I saw no performance difference.
    // Maybe we should revisit occasionally.
    Word_t wDigit;
    int nIncr; (void)nIncr;
  #ifndef RECURSIVE
      #ifdef INSERT
    nIncr = 1;
      #elif defined(REMOVE)
    nIncr = -1;
      #else // INSERT elif REMOVE
    nIncr = 0; // make gcc happy
      #endif // INSERT elif REMOVE else
  #endif // !RECURSIVE
    // nBLUp was used only for CODE_XX_SW and INSERT.
    // I think it will eventually be used for REMOVE.
    int nBLUp = 0; (void)nBLUp; // silence gcc
    Link_t *pLnUp = NULL; (void)pLnUp;
  #ifdef _NEXT_SHORTCUT
    //IF_NEXT(int nTypeUp = -1; (void)nTypeUp);
  #endif // _NEXT_SHORTCUT
    // gcc thinks we use nBLRUp uninitialized unless we initialize it here.
    IF_NEXT(int nBLRUp = 0); // valid only if nBLUp != 0
      #ifdef _LNX
    Word_t* pwLnXUp = NULL; (void)pwLnXUp;
      #endif // _LNX
    // gcc complains that nBLUp may be used uninitialized with CODE_XX_SW.
    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
  #ifdef SAVE_PREFIX_TEST_RESULT
    Word_t wPrefixMismatch = 0; (void)wPrefixMismatch;
  #endif // SAVE_PREFIX_TEST_RESULT
  #ifdef _LNX
    Word_t* pwLnXNew;
      #ifdef REMOTE_LNX
    // DoubleDown calls Insert not at the top.
    // But high enough to get a valid pwLnX before we need it.
    // Or not? SwIncr is going to fault if SW_POP_IN_LNX?
    //assert(nBL >= cnBitsPerWord);
    if (nBL < cnBitsPerWord) {
        DBGX(Checkpoint(qya, "Insert Not-at-Top"));
    }
      #else // REMOTE_LNX
    if (nBL < cnBitsPerWord) {
         // Can we do this unconditionally for LOOKUP to save the test?
         pwLnX = &pLn->ln_wX;
    }
      #endif // REMOTE_LNX else
      #if defined(INSERT) || defined(REMOVE) || defined(NEXT)
    Word_t* pwLnXOrig = pwLnX; (void)pwLnXOrig;
      #endif // INSERT || REMOVE || NEXT
  #endif // _LNX
  #if defined(LOOKUP) && defined(SKIP_PREFIX_CHECK)
    Word_t *pwrUp = pwrUp; // suppress "uninitialized" compiler warning
  #endif // LOOKUP && SKIP_PREFIX_CHECK
    Link_t *pLnPrefix = NULL; (void)pLnPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    int nBLRPrefix = 0; (void)nBLRPrefix;
    Word_t wPopCntUp = 0; (void)wPopCntUp;
  #ifdef COUNT
    Word_t wPopCntSum = 0;
  #endif // COUNT
    // Cleanup is expensive. So we only do it if it has been requested.
    int bCleanupRequested = 0; (void)bCleanupRequested;
    int bCleanup = 0; (void)bCleanup;
    int nBLR;
    int nPos; (void)nPos; // (void) is for Judy1 LOOKKUP turn-on
  #ifdef _AUG_TYPE
  #ifndef AUGMENT_TYPE_NOT
    int nAugTypeBits;
  #endif // !AUGMENT_TYPE_NOT
  #endif // _AUG_TYPE
  #ifdef GOTO_AT_FIRST_IN_LOOKUP
  #ifdef SKIP_LINKS
  #if defined(LOOKUP) // || defined(NEXT)
    // This shortcut made LOOKUP faster in my original testing.
    // It make NEXT a lot slower just after splay.
    // I don't know why.
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot);
    // nBL, pLn, wRoot, nType and pwr of qy are set up
    if (nType >= T_SKIP_TO_SWITCH) { // What about T_SKIP_TO_??_SW?
        DBGX(Checkpoint(qya, "goto skip_to_sw"));
        goto t_skip_to_switch;
    }
    // This shortcut made the code faster in my testing.
    nBLR = nBL;
      #ifdef _AUG_TYPE
      #ifndef AUGMENT_TYPE_NOT
    nAugTypeBits = AugTypeBits(nBL);
      #endif // !AUGMENT_TYPE_NOT
      #endif // _AUG_TYPE
    goto fastAgain;
  #endif // LOOKUP || NEXT
  #endif // SKIP_LINKS
  #endif // GOTO_AT_FIRST_IN_LOOKUP
  #if !defined(LOOKUP) || defined(B_JUDYL)
    nPos = -1;
  #endif // !LOOKUP || B_JUDYL
  #ifdef COUNT
    int bLinkPresent;
    int nLinks;
  #endif // COUNT
  #if defined(INSERT) || defined(REMOVE) || defined(NEXT)
  #ifndef RECURSIVE
    goto top;
    DBGX(Checkpoint(qya, "top"));
top:;
  #endif // !RECURSIVE
  #endif // INSERT || REMOVE || NEXT
    nBLR = nBL;
  #if defined(LOOKUP) || !defined(RECURSIVE)
    goto again;
    DBGX(Checkpoint(qya, "again"));
again:;
  #endif // LOOKUP || !RECURSIVE
  #ifdef _AUG_TYPE
  #ifndef AUGMENT_TYPE_NOT
    nAugTypeBits = AugTypeBits(nBL);
    goto againAugType;
againAugType:;
  #endif // !AUGMENT_TYPE_NOT
  #endif // _AUG_TYPE
  #ifdef SKIP_LINKS
    assert(nBLR == nBL);
  #endif // SKIP_LINKS
  #ifndef LOOKUP
  #ifndef CODE_XX_SW
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
  #endif // !CODE_XX_SW
  #endif // !LOOKUP
  #if defined(INSERT) && defined(_RETURN_NULL_TO_INSERT_AGAIN) || defined(NEXT)
insertAgain:
  #endif // INSERT && _RETURN_NULL_TO_INSERT_AGAIN || NEXT
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot); // pwr isn't meaningful for all nType values
    // nBL, pLn, wRoot, nType and pwr of qy are set up
    DBGX(Checkpoint(qya, "enter switch stmt"));
    DBGX(printf("# wKey 0x%zx\n", wKey));
    goto fastAgain;
fastAgain:;
  #ifdef _AUG_TYPE
  #ifndef AUGMENT_TYPE_NOT
//printf("# fastAgain nAugTypeBits 0x%x nType %d\n", nAugTypeBits, nType);
  #endif // _AUG_TYPE
  #endif // !AUGMENT_TYPE_NOT
  #ifdef JUMP_TABLE
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
                    JT_ENTRIES,
            [16 ] = JT_ENTRIES,
            [32 ] = JT_ENTRIES,
            [48 ] = JT_ENTRIES,
            [64 ] = JT_ENTRIES,
            [80 ] = JT_ENTRIES,
            [96 ] = JT_ENTRIES,
            [112] = JT_ENTRIES,
      #ifdef AUGMENT_TYPE
      #ifdef LOOKUP
            [16 + T_LIST] = LIST_COMMA(&&t_list16)
          #if cn2dBmMaxWpkPercent != 0
            [16 + T_BITMAP] = &&t_bm_plus_16,
          #endif // cn2dBmMaxWpkPercent != 0
            [16 + T_SWITCH] = &&t_sw_plus_16,
            [32 + T_LIST] = LIST_COMMA(&&t_list32)
            [32 + T_SWITCH] = &&t_sw_plus_32,
            [48 + T_LIST] = LIST_COMMA(&&t_list48)
            [48 + T_SWITCH] = &&t_sw_plus_48,
          #ifdef AUGMENT_TYPE_8
            [64 + T_LIST] = LIST_COMMA(&&t_list64)
            [64 + T_SWITCH] = &&t_sw_plus_64,
            [80 + T_LIST] = LIST_COMMA(&&t_list80)
            [80 + T_SWITCH] = &&t_sw_plus_80,
            [96 + T_LIST] = LIST_COMMA(&&t_list96)
            [96 + T_SWITCH] = &&t_sw_plus_96,
            [112 + T_LIST] = LIST_COMMA(&&t_list112)
            [112 + T_SWITCH] = &&t_sw_plus_112,
          #endif // AUGMENT_TYPE_8
      #elif defined(NEXT)
          #ifdef AUG_TYPE_8_SW_NEXT
            [16 + T_SWITCH] = &&t_sw_plus_16,
            [32 + T_SWITCH] = &&t_sw_plus_32,
            [48 + T_SWITCH] = &&t_sw_plus_48,
            [64 + T_SWITCH] = &&t_sw_plus_64,
            [80 + T_SWITCH] = &&t_sw_plus_80,
            [96 + T_SWITCH] = &&t_sw_plus_96,
            [112 + T_SWITCH] = &&t_sw_plus_112,
          #endif // AUG_TYPE_8_SW_NEXT
      #endif // LOOKUP elif NEXT
      #ifdef _AUG_TYPE_8_EK
            [T_EMBEDDED_KEYS +  16] = &&t_ek_16,
            [T_EMBEDDED_KEYS +  32] = &&t_ek_32,
            [T_EMBEDDED_KEYS +  48] = &&t_ek_48,
            [T_EMBEDDED_KEYS +  64] = &&t_ek_64,
            [T_EMBEDDED_KEYS +  80] = &&t_ek_80,
            [T_EMBEDDED_KEYS +  96] = &&t_ek_96,
            [T_EMBEDDED_KEYS + 112] = &&t_ek_112,
      #endif // _AUG_TYPE_8_EK
      #ifdef EK_XV
      #if defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
            [T_EK_XV +   0] = &&t_ek_xv_plus_0,
            [T_EK_XV +  16] = &&t_ek_xv_plus_16,
            [T_EK_XV +  32] = &&t_ek_xv_plus_32,
            [T_EK_XV +  48] = &&t_ek_xv_plus_48,
            [T_EK_XV +  64] = &&t_ek_xv_plus_64,
            [T_EK_XV +  80] = &&t_ek_xv_plus_80,
            [T_EK_XV +  96] = &&t_ek_xv_plus_96,
            [T_EK_XV + 112] = &&t_ek_xv_plus_112,
      #endif // defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
      #endif // EK_XV
      #endif // AUGMENT_TYPE
    };
  #endif // JUMP_TABLE
  // AUGMENT_TYPE_NOT means create jump table entries or switch table cases
  // according to AUGMENT_TYPE and AUGMENT_TYPE_8 but don't do the work of
  // augmenting the type or jump or switch based on an augmented type.
  // AUGMENT_TYPE_8 with AUGMENT_TYPE assumes the low three bits of nBL are
  // always zero and uses the high three bits of nBL to augment the type.
  // for eight different legal nBL values: 8, 16, 24, 32, 40, 48, 56, 64.
  // How expensive would it be to maintain nDL as well as nBL?
  // AUGMENT_TYPE without AUGMENT_TYPE_8 has four different nBL groups:
  // 5-8, 9-16, 17-32, 33-64. 0-4 does not work.
    // AUGMENTED_TYPE
  #ifdef _AUG_TYPE
    #define AUGMENTED_TYPE  (nAugTypeBits | nType)
  #else // _AUG_TYPE
    #define AUGMENTED_TYPE  nType
  #endif // _AUG_TYPE
    // MASKED_AUGMENTED_TYPE
  // MASK_TYPE serves no purpose for JUMP_TABLE.
  // We have it to help gauge cost when doing it for !JUMP_TABLE.
  #ifdef MASK_TYPE
    // AUGMENTED_TYPE_MASK
      #ifdef AUG_TYPE_64_LOOKUP
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 6))
      #elif defined(AUG_TYPE_32_LOOKUP)
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 5))
      #elif defined(AUG_TYPE_16_LOOKUP)
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 4))
      #elif defined(AUG_TYPE_8_LOOKUP)
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 3))
      #elif defined(AUGMENT_TYPE_8)
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 3))
      #elif defined(AUGMENT_TYPE)
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask + 2))
      #else // AUGMENT_TYPE_8 elif AUGMENT_TYPE
    #define AUGMENTED_TYPE_MASK  ((int)MSK(cnBitsTypeMask))
      #endif // AUGMENT_TYPE_8 elif AUGMENT_TYPE else
    #define MASKED_AUGMENTED_TYPE  (AUGMENTED_TYPE & AUGMENTED_TYPE_MASK)
  #else // MASK_TYPE
    #define MASKED_AUGMENTED_TYPE  AUGMENTED_TYPE
  #endif // MASK_TYPE else
  // __builtin_prefetch(0, 0); // Uncomment and find prefetcht0 in bl[L].s.
  #ifdef JUMP_TABLE
    goto *pvJumpTable[MASKED_AUGMENTED_TYPE];
  #else // JUMP_TABLE
//printf("# MASKED_AUGMENTED_TYPE 0x%x\n", MASKED_AUGMENTED_TYPE);
    switch (MASKED_AUGMENTED_TYPE)
    {
  // At most one of DEFAULT_SKIP_TO_SW, DEFAULT_SWITCH,
  // DEFAULT_LIST and DEFAULT_BITMAP may be defined.
  // If none of DEFAULT_SKIP_TO_SW, DEFAULT_SWITCH, DEFAULT_LIST and
  // DEFAULT_BITMAP is defined, then we don't need a default case.
  #ifdef DEBUG
  #ifndef DEFAULT_SKIP_TO_SW
  #ifndef DEFAULT_SWITCH
  #ifndef DEFAULT_LIST
  #ifndef DEFAULT_BITMAP
  #if !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
    default:
        DBG(printf("\n# Unknown augmented type 0x%02x nBL %d wRoot 0x%zx\n",
                   MASKED_AUGMENTED_TYPE, nBL, wRoot));
        exit(1); // We lose debug output piped to a file when assert blows.
        //assert(0);
  #endif // !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
  #endif // DEFAULT_BITMAP
  #endif // DEFAULT_LIST
  #endif // DEFAULT_SWITCH
  #endif // DEFAULT_SKIP_TO_SW
  #endif // DEBUG
  #if defined(AUGMENT_TYPE_8) && defined(_AUG_TYPE)
    #define CASES_AUG_TYPE_8(_t) \
        case (_t) + 7 * (1 << cnBitsTypeMask): \
        case (_t) + 6 * (1 << cnBitsTypeMask): \
        case (_t) + 5 * (1 << cnBitsTypeMask): \
        case (_t) + 4 * (1 << cnBitsTypeMask):
  #else // AUGMENT_TYPE_8 && _AUG_TYPE
    #define CASES_AUG_TYPE_8(_t)
  #endif // AUGMENT_TYPE_8 && _AUG_TYPE else
  #ifdef _AUG_TYPE
    #define CASES_AUG_TYPE(_t) \
        CASES_AUG_TYPE_8(_t) \
        case (_t) + 3 * (1 << cnBitsTypeMask): \
        case (_t) + 2 * (1 << cnBitsTypeMask): \
        case (_t) + 1 * (1 << cnBitsTypeMask):
  #else // _AUG_TYPE
    #define CASES_AUG_TYPE(_t)
  #endif // _AUG_TYPE else
  #if defined(DEFAULT_SKIP_TO_SW)
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
      #if NUM_TYPES <= 15
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH + 1)
      #endif // NUM_TYPES <= 15
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 15
    case T_SKIP_TO_SWITCH+1:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 15
      #if NUM_TYPES <= 14
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH + 2)
      #endif // NUM_TYPES <= 14
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 14
    case T_SKIP_TO_SWITCH+2:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 14
      #if NUM_TYPES <= 13
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH + 3)
      #endif // NUM_TYPES <= 13
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 13
    case T_SKIP_TO_SWITCH+3:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 13
      #if NUM_TYPES <= 12
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH + 4)
      #endif // NUM_TYPES <= 12
      #if !defined(AUGMENT_TYPE) || NUM_TYPES <= 12
    case T_SKIP_TO_SWITCH+4:
      #endif // !AUGMENT_TYPE || NUM_TYPES <= 12
      #if NUM_TYPES <= 11
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH + 5)
      #endif // NUM_TYPES <= 11
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
  #ifdef SKIP_LINKS
      #if !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
          #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_SWITCH + ((64 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_SWITCH + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 2 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_SWITCH + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 4 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_SWITCH + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 8 - 1) << cnBitsTypeMask):
          #else // AUG_TYPE_64_LOOKUP && LOOKUP
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH)
    case T_SKIP_TO_SWITCH: // skip link to uncompressed switch
          #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
      #endif // !defined(DEFAULT_SKIP_TO_SW) || defined(DEFAULT_AND_CASE)
        goto t_skip_to_switch; // silence cc in case there are no other uses
  #endif // SKIP_LINKS

  #ifdef SKIP_TO_XX_SW // Doesn't work yet.
    CASES_AUG_TYPE(T_SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: // skip link to narrow/wide switch
        goto t_skip_to_xx_sw;
  #endif // SKIP_TO_XX_SW

  // #ifdef SWITCH
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_SWITCH + ((64 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_SWITCH + ((64 / 2 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 2 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 2 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 2 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 2 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 2 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 2 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_SWITCH + ((64 / 4 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 4 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 4 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 4 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 4 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 4 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 4 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_SWITCH + ((64 / 8 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 8 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 8 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 8 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 8 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 8 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 8 - 1) << cnBitsTypeMask): goto t_sw_16;
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
          #if defined(DEFAULT_SWITCH)
    default:
          #endif // defined(DEFAULT_SWITCH)
          #ifdef _AUG_TYPE_8_SW
    case T_SWITCH + 7 * (1 << cnBitsTypeMask): goto t_sw_plus_112;
    case T_SWITCH + 6 * (1 << cnBitsTypeMask): goto t_sw_plus_96;
    case T_SWITCH + 5 * (1 << cnBitsTypeMask): goto t_sw_plus_80;
    case T_SWITCH + 4 * (1 << cnBitsTypeMask): goto t_sw_plus_64;
          #endif // _AUG_TYPE_8_SW
        #if defined(AUGMENT_TYPE) && defined(LOOKUP) || defined(_AUG_TYPE_8_SW)
    case T_SWITCH + 3 * (1 << cnBitsTypeMask): goto t_sw_plus_48;
    case T_SWITCH + 2 * (1 << cnBitsTypeMask): goto t_sw_plus_32;
    case T_SWITCH + 1 * (1 << cnBitsTypeMask): goto t_sw_plus_16;
        #endif // AUGMENT_TYPE && LOOKUP || _AUG_TYPE_8_SW
          #ifdef NEXT
          #if defined(AUG_TYPE_8_NEXT_EK_XV) && !defined(AUG_TYPE_8_SW_NEXT)
    CASES_AUG_TYPE(T_SWITCH)
          #endif // AUG_TYPE_8_NEXT_EK_XV && !AUG_TYPE_8_SW_NEXT
          #endif // NEXT
          #ifndef _AUG_TYPE_8_SW
          #if !defined(AUGMENT_TYPE) || !defined(LOOKUP)
          #if !defined(DEFAULT_SWITCH) || defined(DEFAULT_AND_CASE)
    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
          #endif // !DEFAULT_SWITCH || DEFAULT_AND_CASE
          #endif // !AUGMENT_TYPE || !LOOKUP
          #endif // !_AUG_TYPE_8_SW
          #if !defined(AUGMENT_TYPE) || !defined(LOOKUP) || defined(SKIP_LINKS)
        goto t_switch;
          #endif // !AUGMENT_TYPE || !LOOKUP || SKIP_LINKS
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP
  // #endif SWITCH

  #ifdef CODE_XX_SW
    CASES_AUG_TYPE(T_XX_SW)
    case T_XX_SW: // no-skip (aka close) switch (vs. distant switch) w/o bm
        goto t_xx_sw;
  #endif // CODE_XX_SW

  #ifdef SKIP_TO_BM_SW
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BM_SW + ((64 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((56 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((48 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((40 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((32 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((24 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((16 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BM_SW + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((16 / 2 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BM_SW + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((16 / 4 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BM_SW + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BM_SW + ((16 / 8 - 1) << cnBitsTypeMask):
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
    CASES_AUG_TYPE(T_SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW:
      #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
        goto t_skip_to_bm_sw;
  #endif // SKIP_TO_BM_SW

  #if defined(RETYPE_FULL_BM_SW) && !defined(BM_IN_NON_BM_SW)
    CASES_AUG_TYPE(T_FULL_BM_SW)
    case T_FULL_BM_SW:
        goto t_full_bm_sw;
  #endif // RETYPE_FULL_BM_SW && !BM_IN_NON_BM_SW

  #ifdef CODE_BM_SW
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_BM_SW + ((64 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((56 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((48 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((40 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((32 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((24 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((16 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_BM_SW + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((16 / 2 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_BM_SW + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((16 / 4 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_BM_SW + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_BM_SW + ((16 / 8 - 1) << cnBitsTypeMask):
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
    CASES_AUG_TYPE(T_BM_SW)
    case T_BM_SW:
      #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
        goto t_bm_sw;
  #endif // CODE_BM_SW

  #ifdef SKIP_TO_LIST_SW
    CASES_AUG_TYPE(T_SKIP_TO_LIST_SW)
    case T_SKIP_TO_LIST_SW:
        goto t_skip_to_list_sw;
  #endif // SKIP_TO_LIST_SW

  #ifdef CODE_LIST_SW
    // Compressed switch with a list header indicating which links are
    // present and implying their offsets.
    CASES_AUG_TYPE(T_LIST_SW)
    case T_LIST_SW:
        goto t_list_sw;
  #endif // CODE_LIST_SW

  #ifdef SKIP_TO_LIST
    CASES_AUG_TYPE(T_SKIP_TO_LIST)
    case T_SKIP_TO_LIST:
        goto t_skip_to_list;
  #endif // SKIP_TO_LIST

  #if (cwListPopCntMax != 0)
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_LIST + ((64 - 1) << cnBitsTypeMask): goto t_list_64;
    case T_LIST + ((56 - 1) << cnBitsTypeMask): goto t_list_56;
    case T_LIST + ((48 - 1) << cnBitsTypeMask): goto t_list_48;
    case T_LIST + ((40 - 1) << cnBitsTypeMask): goto t_list_40;
    case T_LIST + ((32 - 1) << cnBitsTypeMask): goto t_list_32;
    case T_LIST + ((24 - 1) << cnBitsTypeMask): goto t_list_24;
    case T_LIST + ((16 - 1) << cnBitsTypeMask): goto t_list_16;
    case T_LIST + (( 8 - 1) << cnBitsTypeMask): goto t_list_8;
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_LIST + ((64 / 2 - 1) << cnBitsTypeMask): goto t_list_64;
    case T_LIST + ((56 / 2 - 1) << cnBitsTypeMask): goto t_list_56;
    case T_LIST + ((48 / 2 - 1) << cnBitsTypeMask): goto t_list_48;
    case T_LIST + ((40 / 2 - 1) << cnBitsTypeMask): goto t_list_40;
    case T_LIST + ((32 / 2 - 1) << cnBitsTypeMask): goto t_list_32;
    case T_LIST + ((24 / 2 - 1) << cnBitsTypeMask): goto t_list_24;
    case T_LIST + ((16 / 2 - 1) << cnBitsTypeMask): goto t_list_16;
    case T_LIST + (( 8 / 2 - 1) << cnBitsTypeMask): goto t_list_8;
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_LIST + ((64 / 4 - 1) << cnBitsTypeMask): goto t_list_64;
    case T_LIST + ((56 / 4 - 1) << cnBitsTypeMask): goto t_list_56;
    case T_LIST + ((48 / 4 - 1) << cnBitsTypeMask): goto t_list_48;
    case T_LIST + ((40 / 4 - 1) << cnBitsTypeMask): goto t_list_40;
    case T_LIST + ((32 / 4 - 1) << cnBitsTypeMask): goto t_list_32;
    case T_LIST + ((24 / 4 - 1) << cnBitsTypeMask): goto t_list_24;
    case T_LIST + ((16 / 4 - 1) << cnBitsTypeMask): goto t_list_16;
    case T_LIST + (( 8 / 4 - 1) << cnBitsTypeMask): goto t_list_8;
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_LIST + ((64 / 8 - 1) << cnBitsTypeMask): goto t_list_64;
    case T_LIST + ((56 / 8 - 1) << cnBitsTypeMask): goto t_list_56;
    case T_LIST + ((48 / 8 - 1) << cnBitsTypeMask): goto t_list_48;
    case T_LIST + ((40 / 8 - 1) << cnBitsTypeMask): goto t_list_40;
    case T_LIST + ((32 / 8 - 1) << cnBitsTypeMask): goto t_list_32;
    case T_LIST + ((24 / 8 - 1) << cnBitsTypeMask): goto t_list_24;
    case T_LIST + ((16 / 8 - 1) << cnBitsTypeMask): goto t_list_16;
    case T_LIST + (( 8 / 8 - 1) << cnBitsTypeMask): goto t_list_8;
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
          #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    case T_LIST + 7 * (1 << cnBitsTypeMask): goto t_list112;
    case T_LIST + 6 * (1 << cnBitsTypeMask): goto t_list96;
    case T_LIST + 5 * (1 << cnBitsTypeMask): goto t_list80;
    case T_LIST + 4 * (1 << cnBitsTypeMask): goto t_list64;
          #endif // AUGMENT_TYPE_8 && LOOKUP
          #if defined(AUGMENT_TYPE) && defined(LOOKUP)
    case T_LIST + 3 * (1 << cnBitsTypeMask): goto t_list48;
    case T_LIST + 2 * (1 << cnBitsTypeMask): goto t_list32;
    case T_LIST + 1 * (1 << cnBitsTypeMask): goto t_list16;
          #endif // AUGMENT_TYPE && LOOKUP
          #ifdef NEXT
          #if defined(AUG_TYPE_8_SW_NEXT) || defined(AUG_TYPE_8_NEXT_EK_XV)
    CASES_AUG_TYPE(T_LIST)
          #endif // AUG_TYPE_8_SW_NEXT || AUG_TYPE_8_NEXT_EK_XV
          #endif // NEXT
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP
  #endif // (cwListPopCntMax != 0)
  #ifndef _AUG_TYPE_X
  #if defined(DEFAULT_LIST)
    default:
  #endif // DEFAULT_LIST
  #if (cwListPopCntMax != 0)
  #ifdef _T_LIST
      #if !defined(DEFAULT_LIST) || defined(DEFAULT_AND_CASE)
    case T_LIST:
      #endif // !DEFAULT_LIST || DEFAULT_AND_CASE
        goto t_list;
  #endif // _T_LIST
  #endif // (cwListPopCntMax != 0)
  #endif // !_AUG_TYPE_X

  #ifdef UA_PARALLEL_128
    CASES_AUG_TYPE(T_LIST_UA)
    case T_LIST_UA:
        goto t_list_ua;
  #endif // UA_PARALLEL_128

  #ifdef DEFAULT_XX_LIST
    default:
  #endif // DEFAULT_XX_LIST
  #ifdef XX_LISTS
    CASES_AUG_TYPE(T_XX_LIST)
      #if !defined(DEFAULT_XX_SW) || defined(DEFAULT_AND_CASE)
    case T_XX_LIST:
      #endif // !DEFAULT_XX_SW || DEFAULT_AND_CASE
        goto t_xx_list;
  #endif // XX_LISTS

  #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
  #ifdef SKIP_TO_BITMAP
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BITMAP + ((64 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((56 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((48 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((40 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((32 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((24 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((16 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BITMAP + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((16 / 2 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BITMAP + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((16 / 4 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_SKIP_TO_BITMAP + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_BITMAP + ((16 / 8 - 1) << cnBitsTypeMask):
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
    CASES_AUG_TYPE(T_SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP:
      #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
        goto t_skip_to_bitmap;
  #endif // SKIP_TO_BITMAP
  #endif // PACK_BM_VALUES || !B_JUDYL

  #ifdef BITMAP
  #ifdef _AUG_TYPE
  #if cn2dBmMaxWpkPercent != 0
          #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((16 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((16 / 2 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((16 / 4 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((16 / 8 - 1) << cnBitsTypeMask):
          #else // AUG_TYPE_64_LOOKUP && LOOKUP
    case T_BITMAP + ((16 / 8 - 1)  << cnBitsTypeMask):
          #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
      #ifdef LOOKUP
        goto t_bm_plus_16;
      #endif // LOOKUP
  #endif // cn2dBmMaxWpkPercent != 0
  #endif // _AUG_TYPE
  #endif // BITMAP
  #if defined(DEFAULT_BITMAP)
    default:
  #endif // defined(DEFAULT_BITMAP)
  #ifdef BITMAP
  #if defined(PACK_BM_VALUES) || !defined(LOOKUP) || !defined(B_JUDYL)
      #if !defined(DEFAULT_BITMAP) || defined(DEFAULT_AND_CASE)
          #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((8 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((8 / 2 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((8 / 4 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_BITMAP + ((8 / 8 - 1) << cnBitsTypeMask):
          #else // AUG_TYPE_64_LOOKUP && LOOKUP
    case T_BITMAP:
          #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
      #endif // !DEFAULT_BITMAP || DEFAULT_AND_CASE
        goto t_bitmap;
  #endif // PACK_BM_VALUES || !LOOKUP || !B_JUDYL
  #endif // BITMAP

  #ifdef UNPACK_BM_VALUES
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_UNPACKED_BM + ((8 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_UNPACKED_BM + ((8 / 2 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_UNPACKED_BM + ((8 / 4 - 1) << cnBitsTypeMask):
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_UNPACKED_BM + ((8 / 8 - 1) << cnBitsTypeMask):
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
    case T_UNPACKED_BM: // never exists for B_JUDY1
      #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
        goto t_unpacked_bm;
  #endif // UNPACK_BM_VALUES

  #if defined(EMBED_KEYS)
      #ifdef _AUG_TYPE_X_EK
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 - 1) << cnBitsTypeMask): goto t_ek_56;
    case T_EMBEDDED_KEYS + ((48 - 1) << cnBitsTypeMask): goto t_ek_48;
    case T_EMBEDDED_KEYS + ((40 - 1) << cnBitsTypeMask): goto t_ek_40;
    case T_EMBEDDED_KEYS + ((32 - 1) << cnBitsTypeMask): goto t_ek_32;
    case T_EMBEDDED_KEYS + ((24 - 1) << cnBitsTypeMask): goto t_ek_24;
    case T_EMBEDDED_KEYS + ((16 - 1) << cnBitsTypeMask): goto t_ek_16;
    case T_EMBEDDED_KEYS + (( 8 - 1) << cnBitsTypeMask): goto t_ek_8;
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 2 - 1) << cnBitsTypeMask): goto t_ek_56;
    case T_EMBEDDED_KEYS + ((48 / 2 - 1) << cnBitsTypeMask): goto t_ek_48;
    case T_EMBEDDED_KEYS + ((40 / 2 - 1) << cnBitsTypeMask): goto t_ek_40;
    case T_EMBEDDED_KEYS + ((32 / 2 - 1) << cnBitsTypeMask): goto t_ek_32;
    case T_EMBEDDED_KEYS + ((24 / 2 - 1) << cnBitsTypeMask): goto t_ek_24;
    case T_EMBEDDED_KEYS + ((16 / 2 - 1) << cnBitsTypeMask): goto t_ek_16;
    case T_EMBEDDED_KEYS + (( 8 / 2 - 1) << cnBitsTypeMask): goto t_ek_8;
      #endif // AUG_TYPE_32_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 4 - 1) << cnBitsTypeMask): goto t_ek_56;
    case T_EMBEDDED_KEYS + ((48 / 4 - 1) << cnBitsTypeMask): goto t_ek_48;
    case T_EMBEDDED_KEYS + ((40 / 4 - 1) << cnBitsTypeMask): goto t_ek_40;
    case T_EMBEDDED_KEYS + ((32 / 4 - 1) << cnBitsTypeMask): goto t_ek_32;
    case T_EMBEDDED_KEYS + ((24 / 4 - 1) << cnBitsTypeMask): goto t_ek_24;
    case T_EMBEDDED_KEYS + ((16 / 4 - 1) << cnBitsTypeMask): goto t_ek_16;
    case T_EMBEDDED_KEYS + (( 8 / 4 - 1) << cnBitsTypeMask): goto t_ek_8;
      #endif // AUG_TYPE_16_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 8 - 1) << cnBitsTypeMask): goto t_ek_56;
    case T_EMBEDDED_KEYS + ((48 / 8 - 1) << cnBitsTypeMask): goto t_ek_48;
    case T_EMBEDDED_KEYS + ((40 / 8 - 1) << cnBitsTypeMask): goto t_ek_40;
    case T_EMBEDDED_KEYS + ((32 / 8 - 1) << cnBitsTypeMask): goto t_ek_32;
    case T_EMBEDDED_KEYS + ((24 / 8 - 1) << cnBitsTypeMask): goto t_ek_24;
    case T_EMBEDDED_KEYS + ((16 / 8 - 1) << cnBitsTypeMask): goto t_ek_16;
    case T_EMBEDDED_KEYS + (( 8 / 8 - 1) << cnBitsTypeMask): goto t_ek_8;
      #endif // AUG_TYPE_8_LOOKUP && LOOKUP
      #elif defined(_AUG_TYPE_X) // _AUG_TYPE_X_EK
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((48 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((40 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((32 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((24 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((16 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + (( 8 - 1) << cnBitsTypeMask):
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((48 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((40 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((32 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((24 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((16 / 2 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + (( 8 / 2 - 1) << cnBitsTypeMask):
      #endif // AUG_TYPE_32_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((48 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((40 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((32 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((24 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((16 / 4 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + (( 8 / 4 - 1) << cnBitsTypeMask):
      #endif // AUG_TYPE_16_LOOKUP && LOOKUP
      #if defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_EMBEDDED_KEYS + ((56 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((48 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((40 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((32 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((24 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + ((16 / 8 - 1) << cnBitsTypeMask): \
    case T_EMBEDDED_KEYS + (( 8 / 8 - 1) << cnBitsTypeMask):
      #endif // AUG_TYPE_8_LOOKUP && LOOKUP
        goto t_embedded_keys;
      #endif // _AUG_TYPE_X_EK elif _AUG_TYPE_X
      #ifdef _AUG_TYPE_8_EK
    case T_EMBEDDED_KEYS + 7 * (1 << cnBitsTypeMask): goto t_ek_112;
    case T_EMBEDDED_KEYS + 6 * (1 << cnBitsTypeMask): goto t_ek_96;
    case T_EMBEDDED_KEYS + 5 * (1 << cnBitsTypeMask): goto t_ek_80;
    case T_EMBEDDED_KEYS + 4 * (1 << cnBitsTypeMask): goto t_ek_64;
    case T_EMBEDDED_KEYS + 3 * (1 << cnBitsTypeMask): goto t_ek_48;
    case T_EMBEDDED_KEYS + 2 * (1 << cnBitsTypeMask): goto t_ek_32;
    case T_EMBEDDED_KEYS + 1 * (1 << cnBitsTypeMask): goto t_ek_16;
    case T_EMBEDDED_KEYS + 0 * (1 << cnBitsTypeMask): goto t_ek_0;
      #else // _AUG_TYPE_8_EK
          #ifndef _AUG_TYPE_X
    CASES_AUG_TYPE(T_EMBEDDED_KEYS)
    case T_EMBEDDED_KEYS:
        goto t_embedded_keys;
          #endif // !_AUG_TYPE_X
      #endif // _AUG_TYPE_8_EK else
  #endif // EMBED_KEYS

  #ifdef EK_XV
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
    case T_EK_XV + ((56 - 1) << cnBitsTypeMask): goto t_ek_xv_56;
    case T_EK_XV + ((48 - 1) << cnBitsTypeMask): goto t_ek_xv_48;
    case T_EK_XV + ((40 - 1) << cnBitsTypeMask): goto t_ek_xv_40;
    case T_EK_XV + ((32 - 1) << cnBitsTypeMask): goto t_ek_xv_32;
    case T_EK_XV + ((24 - 1) << cnBitsTypeMask): goto t_ek_xv_24;
    case T_EK_XV + ((16 - 1) << cnBitsTypeMask): goto t_ek_xv_16;
    case T_EK_XV + (( 8 - 1) << cnBitsTypeMask): goto t_ek_xv_8;
      #endif // defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
      #if defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
    case T_EK_XV + ((56 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_56;
    case T_EK_XV + ((48 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_48;
    case T_EK_XV + ((40 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_40;
    case T_EK_XV + ((32 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_32;
    case T_EK_XV + ((24 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_24;
    case T_EK_XV + ((16 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_16;
    case T_EK_XV + (( 8 / 2 - 1) << cnBitsTypeMask): goto t_ek_xv_8;
      #endif // defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
      #if defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
    case T_EK_XV + ((56 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_56;
    case T_EK_XV + ((48 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_48;
    case T_EK_XV + ((40 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_40;
    case T_EK_XV + ((32 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_32;
    case T_EK_XV + ((24 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_24;
    case T_EK_XV + ((16 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_16;
    case T_EK_XV + (( 8 / 4 - 1) << cnBitsTypeMask): goto t_ek_xv_8;
      #endif // defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
      #if defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
    case T_EK_XV + ((56 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_56;
    case T_EK_XV + ((48 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_48;
    case T_EK_XV + ((40 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_40;
    case T_EK_XV + ((32 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_32;
    case T_EK_XV + ((24 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_24;
    case T_EK_XV + ((16 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_16;
    case T_EK_XV + (( 8 / 8 - 1) << cnBitsTypeMask): goto t_ek_xv_8;
      #endif // defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
      #if defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
    case T_EK_XV + 7 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_112;
    case T_EK_XV + 6 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_96;
    case T_EK_XV + 5 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_80;
    case T_EK_XV + 4 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_64;
    case T_EK_XV + 3 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_48;
    case T_EK_XV + 2 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_32;
    case T_EK_XV + 1 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_16;
    case T_EK_XV + 0 * (1 << cnBitsTypeMask): goto t_ek_xv_plus_0;
      #else // AUG_TYPE_8_NEXT_EK_XV && NEXT
          #ifndef _AUG_TYPE_X
    CASES_AUG_TYPE(T_EK_XV)
    case T_EK_XV:
        goto t_ek_xv;
          #endif // !_AUG_TYPE_X
      #endif // AUG_TYPE_8_NEXT_EK_XV && NEXT else
  #endif // EK_XV

  #if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)
    CASES_AUG_TYPE(T_NULL)
    case T_NULL:
        goto t_separate_t_null;
  #endif // SEPARATE_T_NULL || (cwListPopCntMax == 0)

    } // end of switch (nType)
    goto break_from_main_switch;
  #endif // else JUMP_TABLE

  #ifdef SKIP_LINKS
t_skip_to_switch:
    {
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
            DBGC(printf("SKIP_TO_SW: COUNT PM 0x%016zx\n", wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt;
              #if ! defined(NO_SKIP_AT_TOP)
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                if (nBL >= cnBitsPerWord) {
                    //int nBW = nBL_to_nBWNAX(nBLR);
                    int nBW = nBL_to_nBWNAB(nBLR);
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qya, EXP(nBW), EXP(nBW));
                } else
              #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
              #endif // ! defined(NO_SKIP_AT_TOP)
                {
                    wPopCnt = gwPopCnt(qya, nBLR);
                }
                DBGC(printf("SKIP_TO_SW: PM wPopCnt %" _fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("sksw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
          #endif // defined(COUNT)
          #ifdef NEXT
            // If we're not past the prefix then advance to it.
            // If we are past the prefix then advance to the next digit.
            if (wKey < (wKey - (wPrefixMismatch << nBLR))) {
                // wPrefixMismatch = (wKey - wPrefix) >> nBLR;
                wKey &= ~MSK(nBLR);
                wKey -= (wPrefixMismatch << nBLR);
                goto restart;
            }
          #endif // NEXT
            goto break_from_main_switch;
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
  // BL_SPECIFIC_SKIP applies to AUGMENT_TYPE_8 and causes us to goto a
  // bl-specific switch case instead of the generic one.
  // How do we do this for AUGMENT_TYPE without AUGMENT_TYPE_8?
  // It appears the additional switch statement is more costly than
  // using code that can handle any nBLR.
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP) && defined(BL_SPECIFIC_SKIP)
      // This jump table is significantly slower than the switch.
      #ifdef BL_SPECIFIC_SKIP_JT
        static void *pvJT[] = {
            &&t_switch,
            &&t_switch,
            &&t_sw_plus_16,
            &&t_sw_plus_32,
            &&t_sw_plus_48,
            &&t_sw_plus_64,
            &&t_sw_plus_80,
            &&t_sw_plus_96,
        };
        goto *pvJT[((nBLR + 4) / 8)
          #ifdef MASK_NBLR
                    & 7 // To compare with switch.
          #endif // MASK_NBLR
                          ];
      #else // BL_SPECIFIC_SKIP_JT
        switch (((nBLR + 4) / 8)
          #ifdef MASK_NBLR
                    & 7 // I think this gets rid of bounds check and helps.
          #endif // MASK_NBLR
                )
        {
        case  2:
            assert(nBLR == AugTypeBitsInv(1 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_16;
        case  3:
            assert(nBLR == AugTypeBitsInv(2 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_32;
        default:
            assert(nBLR == AugTypeBitsInv(3 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_48;
        case  5:
            assert(nBLR == AugTypeBitsInv(4 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_64;
        case  6:
            assert(nBLR == AugTypeBitsInv(5 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_80;
        case  7:
            assert(nBLR == AugTypeBitsInv(6 * (1 << cnBitsTypeMask)));
            goto t_sw_plus_96;
          #if defined(AUGMENT_TYPE_8_PLUS_4) && cnBitsLeftAtDl3 < 24
        // t_sw_plus_112 is not bl-specific in this case.
        // It would be possible have a bl-specific case, but I don't
        // think it's worth the trouble.
        case  8:
            assert(AugTypeBits(nBLR) == 7 * (1 << cnBitsTypeMask));
            goto t_sw_plus_112;
          #endif // AUGMENT_TYPE_8_PLUS_4 && cnBitsLeftAtDl3 < 24
        }
      #endif // BL_SPECIFIC_SKIP_JT
  #else // AUGMENT_TYPE_8 && LOOKUP && BL_SPECIFIC_SKIP
        goto t_switch;
  #endif // else AUGMENT_TYPE_8 && LOOKUP && BL_SPECIFIC_SKIP
    } // end of t_skip_to_sw
  #endif // SKIP_LINKS

  #ifdef SKIP_TO_XX_SW // Doesn't work yet.
t_skip_to_xx_sw:
    {
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
      #ifdef COUNT
            DBGC(printf("SKIP_TO_XX_SW: COUNT PM %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt;
          #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
              #error Not ready yet
                if (nBL >= cnBitsPerWord) {
                    int nBW = nBL_to_nBWNAB(nBLR);
                    //int nLinks = ??? __builtin_popcount
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qya, nLinks, nLinks);
                } else
          #endif // PP_IN_LINK && !NO_SKIP_AT_TOP
                {
                    wPopCnt = gwPopCnt(qya, nBLR);
                }
                DBGC(printf("SKIP_TO_XX_SW: PM wPopCnt %" _fw"d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skxxsw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // COUNT
      #ifdef NEXT
            // If we're not past the prefix then advance to it.
            // If we are past the prefix then advance to the next digit.
            if (wKey < (wKey - (wPrefixMismatch << nBLR))) {
                // wPrefixMismatch = (wKey - wPrefix) >> nBLR;
                wKey &= ~MSK(nBLR);
                wKey -= (wPrefixMismatch << nBLR);
                goto restart;
            }
      #endif // NEXT
            goto break_from_main_switch;
        }
        goto t_xx_sw;
    } // end of t_skip_to_xx_sw
  #endif // SKIP_TO_XX_SW

  #ifdef _AUG_TYPE_X
t_sw_64:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 64;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_64
t_sw_56:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 56;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_56
t_sw_48:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 48;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_48
t_sw_40:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 40;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_40
t_sw_32:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 32;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_32
t_sw_24:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 24;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_24
t_sw_16:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        nBL = nBLR = 16;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_16
  #endif // _AUG_TYPE_X

  #ifndef _AUG_TYPE_X_LOOKUP
  #ifdef _AUG_TYPE_8_SW

t_sw_plus_112:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
      #if cnBitsLeftAtDl3 >= 24
        // Help compiler know nBLR is a constant; does it help?
        nBLR = AugTypeBitsInv(7 * (1 << cnBitsTypeMask));
      #endif // cnBitsLeftAtDl3 >= 24
        nBL = nBLR;
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_plus_112

t_sw_plus_96:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        // Help compiler know nBLR is a constant; does it help?
        nBLR = AugTypeBitsInv(6 * (1 << cnBitsTypeMask));
      #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
      #endif // !BL_SPECIFIC_SKIP
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_plus_96

t_sw_plus_80:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        // Help compiler know nBLR is a constant; does it help?
        nBLR = AugTypeBitsInv(5 * (1 << cnBitsTypeMask));
      #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
      #endif // !BL_SPECIFIC_SKIP
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of t_sw_plus_80

t_sw_plus_64:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
        // Help compiler know nBLR is a constant; does it help?
        nBLR = AugTypeBitsInv(4 * (1 << cnBitsTypeMask));
      #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
      #endif // !BL_SPECIFIC_SKIP
        nBW = gnBW(qy, nBLR);
        assert(gnBW(qy, nBLR) == nBW);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
        goto againAugType;
    } // end of goto t_sw_plus_64

  #endif // _AUG_TYPE_8_SW

  #if defined(AUGMENT_TYPE) && defined(LOOKUP) || defined(_AUG_TYPE_8_SW)

t_sw_plus_48:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
      #ifdef AUGMENT_TYPE_8
        // Help compiler know nBLR is a constant; does it help?
        assert(nBLR == AugTypeBitsInv(3 * (1 << cnBitsTypeMask)));
        nBLR = AugTypeBitsInv(3 * (1 << cnBitsTypeMask));
          #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
          #endif // !BL_SPECIFIC_SKIP
        nBW = gnBW(qy, nBLR);
      #else // AUGMENT_TYPE_8
        // Would be nice if nBW were a constant and the compiler knew it.
        // We know 32 < nBL <= 64 or we wouldn't be here.
        // If 64 - cnBitsLeftAtDl3 is a multiple of cnBitsPerDigit and
        // cnBitsLeftAtDl3 <= 32 then nBW is cnBitsPerDigit.
          #if ((cnBitsPerWord - cnBitsLeftAtDl3) % cnBitsPerDigit) == 0
        nBW = cnBitsPerDigit;
        assert(nBW == gnBW(qy, nBLR));
          #else // (cnBitsPerWord-cnBitsLeftAtDl3) % cnBitsPerDigit == 0
        #pragma message("Don't know nBW at t_sw_plus_48.")
        nBW = gnBW(qy, nBLR); // num bits decoded
          #endif // else (cnBitsPerWord-cnBitsLeftAtDl3) % cnBitsPerDigit == 0
      #endif // else AUGMENT_TYPE_8
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
      #ifdef AUGMENT_TYPE_8
        // Calculate nAugTypeBits while we know nBL is a constant.
        nAugTypeBits = AugTypeBits(nBL);
      #else // AUGMENT_TYPE_8
        // Is there a faster way to calculate nAugTypeBits here?
        // If nBW is constant above and less than or equal to 16
        // then nAugTypeBits will either not change or be reduced
        // by 16, right?
        // The assertion will blow if we have a single digit that
        // spans > 32 to <= 16 bits left.
        assert((nBL - 1) & 0x30);
        // 48 or 32
         #if cnBitsTypeMask == 4
        nAugTypeBits = 32 + (((nBL - 1) & 32) >> 1);
        //nAugTypeBits -= ((~(nBL - 1) & 32) >> 1);
        assert(nAugTypeBits == AugTypeBits(nBL));
         #else // cnBitsTypeMask == 4
        nAugTypeBits = AugTypeBits(nBL);
         #endif // cnBitsTypeMask == 4
      #endif // AUGMENT_TYPE_8
        goto againAugType;
    } // end of t_sw_plus_48

t_sw_plus_32:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
      #ifdef AUGMENT_TYPE_8
        // Help compiler know nBLR is a constant; does it help?
        assert(nBLR == AugTypeBitsInv(2 * (1 << cnBitsTypeMask)));
        nBLR = AugTypeBitsInv(2 * (1 << cnBitsTypeMask)); // nBLR is a constant
          #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
          #endif // !BL_SPECIFIC_SKIP
      #endif // AUGMENT_TYPE_8
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
      #ifdef AUGMENT_TYPE_8
        nAugTypeBits = AugTypeBits(nBL);
      #else // AUGMENT_TYPE_8
          #if cnBitsTypeMask == 4
        nAugTypeBits = 16 + ((nBL - 1) & 16);
        assert(nAugTypeBits == AugTypeBits(nBL));
          #else // cnBitsTypeMask == 4
        nAugTypeBits = AugTypeBits(nBL);
          #endif // cnBitsTypeMask == 4
      #endif // AUGMENT_TYPE_8
        goto againAugType;
    } // end of t_sw_plus_32

t_sw_plus_16:
    {
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) { goto break_from_main_switch; }
      // If we can, set nBLR, nBL and nBW to a constant to help compiler.
      // I wonder how much of this the compiler can figure out on its own.
      #ifdef AUGMENT_TYPE_8
        assert(nBLR == AugTypeBitsInv(1 << cnBitsTypeMask));
        nBLR = AugTypeBitsInv(1 << cnBitsTypeMask);
          #ifndef BL_SPECIFIC_SKIP
        nBL = nBLR;
          #endif // !BL_SPECIFIC_SKIP
        assert(nBLR == cnBitsLeftAtDl2);
      #elif cnBitsLeftAtDl2 > 16
        // We never have a switch at nBLR == cnBitsInD1.
        assert(0); // could ifdef this whole chunk of code
      #elif cnBitsLeftAtDl3 > 16 && cnBitsInD1 <= 8
        nBLR = nBL = cnBitsLeftAtDl2;
      #elif cnBitsLeftAtDl3 + cnBitsPerDigit > 16 && cnBitsLeftAtDl2 <= 8
        nBLR = nBL = cnBitsLeftAtDl3;
      #else
        #pragma message("Don't know nBLR at t_sw_plus_16.")
        // Not a bug. Just perf issue to flag.
        // nBLR is already set correctly. Just not a constant.
      #endif
        nBW = gnBW(qy, nBLR);
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
          #ifdef PREFETCH_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_SKIP_PREFIX_CHECK(pwrUp = pwr);
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
      #ifdef BITMAP
        // Can't assume embedded bitmap for JudyL. Might be T_EMBEDDED_KEYS,
        // T_EK_XV or T_UNPACKED_BM.
          #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
            // nType and pwr have not been updated.
            goto t_bitmap;
        }
          #endif // PACK_BM_VALUES || !B_JUDYL
          #ifdef CHECK_TYPE_FOR_EBM
        if ((nType = wr_nType(wRoot)) == T_BITMAP) {
            goto t_bitmap;
        }
          #else // CHECK_TYPE_FOR_EBM
              #ifdef _BMLF_BM_IN_LNX
              #if defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
              #ifndef UNPACK_BM_VALUES
        if (cnListPopCntMaxDl1 == 0) {
            // nType and pwr have not been updated.
            if ((nType = wr_nType(wRoot)) != T_BITMAP) {
                DBGX(printf("\n# nBL %d nType %d\n", nBL, nType));
                if ((wRoot != WROOT_NULL) || (*pwLnX != 0)) {
                    DBGX(printf("\n# goto t_bitmap wKey 0x%zx\n", wKey));
                    DBGX(Dump(pwRootLast, wKey, cnBitsPerWord));
                    DBGX(printf("\n"));
                }
                assert(wRoot == WROOT_NULL);
                assert(*pwLnX == 0);
            }
            goto t_bitmap;
        }
              #endif // !UNPACK_BM_VALUES
              #endif // defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
              #endif // _BMLF_BM_IN_LNX
          #endif // CHECK_TYPE_FOR_EBM else
      #endif // BITMAP
        // Does compiler know nBL is a constant after we call SwAdvance if
        // it knew that nBLR and nBW were constants going in?
        // If so then we can simply do nAugTypeBits = AugTypeBits(nBL).
        // It looks like it does.
      #ifdef AUGMENT_TYPE_8
        nAugTypeBits = AugTypeBits(nBL);
      #elif cnBitsLeftAtDl3 > 16 && cnBitsInD1 <= 8
        nAugTypeBits = AugTypeBits(nBL);
      #elif cnBitsLeftAtDl3 + cnBitsPerDigit > 16 && cnBitsLeftAtDl2 <= 8
        nAugTypeBits = AugTypeBits(nBL);
      #else
        nAugTypeBits = ((nBL - 1) & 8) << 1;
        assert(nAugTypeBits == AugTypeBits(nBL));
      #endif
        goto againAugType;
    } // end of t_sw_plus_16

  #endif // AUGMENT_TYPE && LOOKUP || _AUG_TYPE_8_SW
  #endif // !AUG_TYPE_X_LOOKUP

  // #ifdef SWITCH
  #if defined(SKIP_LINKS) || defined(CODE_XX_SW)
    #define _T_SWITCH_TAIL
  #endif // defined(SKIP_LINKS) || defined(CODE_XX_SW)
  #if defined(_T_SWITCH_TAIL) || !defined(AUGMENT_TYPE) || !defined(LOOKUP)
    // nBL is bits left after picking the link we're handling now
    // nBL has not been reduced by any skip indicated in that link
    // nBLR is bits left after reducing nBL by any skip in the link
    // nBLR is bits left to decode by this switch and below
    goto t_switch;
t_switch:
    {
      #if defined(AUGMENT_TYPE) && defined(LOOKUP) || defined(_AUG_TYPE_8_SW)
      #ifndef AUGMENT_TYPE_NOT
       assert(nBLR != nBL);
      #endif // !AUGMENT_TYPE_NOT
      #endif // AUGMENT_TYPE && LOOKUP || _AUG_TYPE_8_SW
        if (WROOT_IS_NULL(T_SWITCH, wRoot)) {
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
            goto break_from_main_switch;
        }
        DBGX(Checkpoint(qya, "t_switch"));
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
        // ((uint8_t *)&wKey)[(cnBitsPerWord - nBL) >> 3];
        // ((uint8_t *)&wKey)[cnDigitsPerWord - nDL];
        // ((uint8_t *)&wSwappedKey)[nDL];
        // *(uint8_t *)&wSwappedAndShiftedKey;
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
        DBGX(printf("pwLnXNew %p\n", pwLnXNew));
          #ifdef PREFETCH_EK_VAL
          #ifdef LOOKUP
        PREFETCH(pwLnXNew);
          #endif // LOOKUP
          #endif // PREFETCH_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PREFETCH_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PREFETCH_PWR
        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
  //#endif // !AUGMENT_TYPE || !LOOKUP || SKIP_LINKS
        goto switchTail; // in case other uses go away by ifdef
switchTail:;
//printf("# switchTail\n");
      #if defined(INSERT) || defined(REMOVE)
        // Handle big picture tree cleanup.
        if (bCleanup) {
            if (SwCleanup(qya, wKey, nBLR
          #if defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                    , &pwValue
          #endif // defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                      ) != 0)
            {
                goto restart;
            }
        } else {
            BJ1(IF_INSERT(wPopCntUp =))
                SwIncr(qya, nBLR, wDigit, nBW, nIncr); // adjust pop count
            // *pwRoot may have changed so wRoot is out of date
        }
      #endif // defined(INSERT) || defined(REMOVE)
        IF_COUNT(wPopCntSum += CountSw(qya, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(nBLUp = nBL);
        IF_NOT_LOOKUP(pLnUp = pLn);
      #ifdef _NEXT_SHORTCUT
        //IF_NEXT(nTypeUp = nType);
      #endif // _NEXT_SHORTCUT
        IF_NEXT(nBLRUp = nBLR);
      #ifdef _LNX
        IF_NOT_LOOKUP(IF_NOT_NEXT(pwLnXUp = pwLnX));
      #endif // _LNX
        IF_LOOKUP(IF_SKIP_PREFIX_CHECK(pwrUp = pwr));
        SwAdvance(pqya, swapynew, nBW, &nBLR); // updates wRoot
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
        goto again; // nType = wr_nType(wRoot); pwr = wr_pwr(wRoot); switch
      #else // defined(LOOKUP) || !defined(RECURSIVE)
        return InsertRemove(qya, wKey);
      #endif // defined(LOOKUP) || !defined(RECURSIVE)
    } // end of t_switch
  #endif // _T_SWITCH_TAIL || !AUGMENT_TYPE || !LOOKUP
  // #endif SWITCH

  #ifdef CODE_XX_SW
t_xx_sw:
    {
        DBGX(Checkpoint(qya, "t_xx_sw"));
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
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
        if (bCleanup) {
            if (SwCleanup(qya, wKey, nBLR
              #if defined(B_JUDYL) && defined(EMBED_KEYS) && defined(INSERT)
                    , &pwValue
              #endif // B_JUDYL && EMBED_KEYS && INSERT
                      ) != 0)
            {
                goto restart;
            }
        } else {
            BJ1(IF_INSERT(wPopCntUp =))
                SwIncr(qya, nBLR, wDigit, nBW, nIncr); // adjust pop count
        }
          #endif // INSERT || REMOVE
        IF_COUNT(wPopCntSum += CountSw(qya, wDigit, nLinks));
        IF_COUNT(if (!bLinkPresent) return wPopCntSum);
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(nBLUp = nBL);
        IF_NOT_LOOKUP(pLnUp = pLn);
        IF_NEXT(nBLRUp = nBLR);
      #ifdef _LNX
        IF_NOT_LOOKUP(pwLnXUp = pwLnX);
      #endif // _LNX
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqya, swapynew, nBW, &nBLR);
          #ifdef BITMAP
          #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
          #endif // PACK_BM_VALUES || !B_JUDYL
          #endif // BITMAP
// End of SwTailCommon.
        // Handle XX_SW-specific special cases that don't go back to the top.
          #if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
              #if defined(NO_TYPE_IN_XX_SW)
        // ZERO_POP_MAGIC is valid only if a word can hold at least two keys.
        assert(EmbeddedListPopCntMax(nBL) >= 2);
              #endif // NO_TYPE_IN_XX_SW
        if (wRoot == WROOT_NULL) {
            return Failure;
        }
          #endif // LOOKUP && ZERO_POP_CHECK_BEFORE_GOTO
        // Blow-ups are handled in t_embedded_keys.
        goto t_embedded_keys;
      #endif // _XX_SW_TAIL
        goto switchTail;
    } // end of t_xx_sw
  #endif // CODE_XX_SW

  #ifdef SKIP_TO_BM_SW
t_skip_to_bm_sw:
    {
        // pwr points to a bitmap switch
        DBGX(printf("SKIP_TO_BM_SW\n"));

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
      #if defined(COUNT)
            DBGC(printf("SKIP_TO_BM_SW: COUNT PM %zd\n", wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt;
          #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                if (nBL >= cnBitsPerWord) {
                    int nBW = gnBW(qy, nBLR); // num bits decoded
                    (void)nBW;
                    // Abuse CountSw into counting whole switch.
                    int nLinkCnt = BmSwLinkCnt(qy);
                    wPopCnt = CountSw(qya,
                                      /*wIndex*/ nLinkCnt,
                                      /*nLinks*/ nLinkCnt);
                } else
          #endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
                { wPopCnt = gwPopCnt(qya, nBLR); }
                DBGC(printf("SKIP_TO_BM_SW: PM wPopCnt %zd\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skbmsw wPopCnt 0x%zx wPopCntSum 0x%zx\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // defined(COUNT)
      #ifdef NEXT
            DBGN(printf("SKIP_TO_BM_SW: NEXT PM %zd\n", wPrefixMismatch));
            // If we're not past the prefix then advance to it.
            // If we are past the prefix then advance to the next digit.
            if (wKey < (wKey - (wPrefixMismatch << nBLR))) {
                // Test this with cnListPopCntMax=64 and
                // btime -W0 -1 -lv -s-2560 -S5 -n1024 -B-21.
                //assert(0);
                // wPrefixMismatch = (wKey - wPrefix) >> nBLR;
                wKey &= ~MSK(nBLR);
                wKey -= (wPrefixMismatch << nBLR);
                goto restart;
            }
      #endif // NEXT
            goto break_from_main_switch;
        }
        goto t_bm_sw;
    } // end of t_skip_to_bm_sw
  #endif // SKIP_TO_BM_SW

  #if defined(RETYPE_FULL_BM_SW) && !defined(BM_IN_NON_BM_SW)
t_full_bm_sw:
    {
          #ifdef LOOKUP
        goto t_switch;
          #endif // LOOKUP
        goto t_bm_sw;
    } // end of t_full_bm_sw
  #endif // RETYPE_FULL_BM_SW && !BM_IN_NON_BM_SW

  #ifdef CODE_BM_SW
t_bm_sw:
    {
//printf("# t_bm_sw\n");
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
        if (nBL < cnBitsPerWord)
      #endif // defined(BM_IN_LINK) && defined(SKIP_TO_BM_SW)
        {
      #if ! defined(COUNT)
            int bLinkPresent; // need less local bLinkPresent for COUNT
      #endif // ! defined(COUNT)
      #if defined(BM_SW_FOR_REAL)
            char* pcPrefetch; (void)pcPrefetch;
// Why are we prefetching here if !LOOKUP?
// Why are we prefetching here if !B_JUDYL?
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
//printf("# wDigit %zd\n", wDigit);
          #ifdef ONE_BM_SW_INDEX_CALL
            BmSwIndex(qy, wDigit, &wSwIndex, &bLinkPresent);
//printf("# wSwIndex %zd\n", wSwIndex);
          #else // ONE_BM_SW_INDEX_CALL
            BmSwIndex(qy, wDigit, /* pwSwIndex */ NULL, &bLinkPresent);
          #endif // ONE_BM_SW_INDEX_CALL
//printf("# bLinkPresent %d\n", bLinkPresent);
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
          #ifdef NEXT
                // missing link
                // find the next present link
                Word_t *pwBmWords = PWR_pwBm(pwRoot, pwr, nBW);
              #if defined(OFFSET_IN_SW_BM_WORD) || defined(X_SW_BM_HALF_WORDS)
                #define cnLogBmBitsPerBmWord  (cnLogBitsPerWord - 1)
              #else // OFFSET_IN_SW_BM_WORD || X_SW_BM_HALF_WORDS
                #define cnLogBmBitsPerBmWord  cnLogBitsPerWord
              #endif // OFFSET_IN_SW_BM_WORD || X_SW_BM_HALF_WORDS else
                int nWordNum = wDigit >> cnLogBmBitsPerBmWord;
                int nBitNum = wDigit & MSK(cnLogBmBitsPerBmWord);
                Word_t wBm = pwBmWords[nWordNum] & ~MSK(nBitNum);
                //printf("wDigit 0x%zx nBitNum %d\n", wDigit, nBitNum);
                for (;;) {
                    wBm &= NZ_MSK(EXP(cnLogBmBitsPerBmWord));
                    //printf("nWordNum %d wBm 0x%zx\n", nWordNum, wBm);
                    if (wBm) {
                        nBitNum = __builtin_ctzll(wBm);
                        //printf("nBitNum %d\n", nBitNum);
                        wDigit = nWordNum << cnLogBmBitsPerBmWord;
                        wDigit |= nBitNum;
                        //printf("wDigit 0x%zx\n", wDigit);
                        wKey &= ~NZ_MSK(nBLR);
                        wKey |= (wDigit << (nBLR - nBW));
                        //printf("# restart wKey 0x%02zx\n", wKey);
                        goto restart;
                    }
                    if ((Word_t)++nWordNum
                        >= ((nBW <= cnLogBmBitsPerBmWord)
                            ? 0 : EXP(nBW - cnLogBmBitsPerBmWord)))
                    {
                        if (nBL >= cnBitsPerWord) {
                            //printf("# break\n");
                            break;
                        }
                        //printf("# tryNext *pwKey 0x%02zx\n", *pwKey);
                        goto tryNextDigit;
                    }
                    wBm = pwBmWords[nWordNum];
                }
          #endif // NEXT
                goto break_from_main_switch;
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
        pwLnXNew = gpwLnX(qy, nLinkCnt, wSwIndex);
      #endif // _LNX

        // Update wDigit before bmSwTail because we have to do it
        // in t_list_sw before goto bmSwTail.
        IF_COUNT(wDigit = wSwIndex);
        goto bmSwTail;
bmSwTail:;
//printf("# bmSwTail\n");
        // bLinkPresent has already been initialized.
        IF_COUNT(nLinks = INT_MAX);
      #if defined(LOOKUP)
        IF_SKIP_PREFIX_CHECK(IF_LOOKUP(pwrUp = pwr));
        SwAdvance(pqya, swapynew, nBW, &nBLR);
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
        if (tp_bIsBmSw(nType) && InflateBmSwTest(qya)) {
            DBGX(Checkpoint(qya, "T_BM_SW req cleanup"));
            bCleanupRequested = 1; // on success
        }
          #endif // INSERT
        goto switchTail;
      #endif // defined(LOOKUP)
    } // end of t_bm_sw
  #endif // CODE_BM_SW

  #ifdef SKIP_TO_LIST_SW
t_skip_to_list_sw:
    {
        DBGX(printf("SKIP_TO_LIST_SW\n"));

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
      #ifdef COUNT
            DBGC(printf("SKIP_TO_LIST_SW: COUNT PM %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt;
          #if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
              #error Not ready yet
                if (nBL >= cnBitsPerWord) {
                    int nBW = gnBW(qy, nBLR); // num bits decoded
                    // Abuse CountSw into counting whole switch.
                    wPopCnt = CountSw(qya, EXP(nBW), EXP(nBW));
                } else
          #endif // PP_IN_LINK && !NO_SKIP_AT_TOP
                {
                    wPopCnt = gwPopCnt(qya, nBLR);
                }
                DBGC(printf("SKIP_TO_LIST_SW: PM wPopCnt %" _fw"d\n",
                            wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("sklssw wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // COUNT
            goto break_from_main_switch;
        }
        goto t_list_sw;

    } // end of t_skip_to_list_sw
  #endif // SKIP_TO_LIST_SW

  #ifdef CODE_LIST_SW
    // Compressed switch with a list header indicating which links are
    // present and implying their offsets.
t_list_sw:
    {
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);

        Word_t wSwIndex;
      #ifdef SW_LIST_IN_LINK
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
          #ifdef RECURSIVE
                && (nBL == cnBitsPerWord)
          #else // RECURSIVE
                && (pLn == pLnOrig)
              #ifndef LOOKUP
                && (nBLOrig == cnBitsPerWord)
              #endif // !LOOKUP
          #endif // else RECURSIVE
            ) )
      #endif // defined(SW_LIST_IN_LINK)
        {
      #ifndef COUNT
            int bLinkPresent;
      #endif // !COUNT
            ListSwIndex(qy, nBW, wDigit, /* pwIndex */ NULL, &bLinkPresent);
      #ifndef COUNT
            // Test to see if link exists before figuring out where it is.
            if ( ! bLinkPresent )
            {
                DBGX(printf("missing link\n"));
                goto break_from_main_switch;
            }
      #endif // !COUNT
            ListSwIndex(qy, nBW, wDigit, &wSwIndex, /* pbPresent */ NULL);
        }

        pLnNew = &gpListSwLinks(qy)[wSwIndex];
        // Update wDigit before goto bmSwTail because we're about to
        // leave the scope of the local wSwIndex and the destination
        // has its own wSwIndex.
        IF_COUNT(wDigit = wSwIndex);
        // We've initialized bLinkPresent prior to goto bmSwTail.
      #ifdef CODE_BM_SW
        goto bmSwTail;
      #else // CODE_BM_SW
        // bLinkPresent has already been initialized.
        IF_COUNT(nLinks = INT_MAX);
          #ifdef LOOKUP
        IF_LOOKUP(IF_SKIP_PREFIX_CHECK(pwrUp = pwr));
        SwAdvance(pqya, swapynew, nBW, &nBLR);
          #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
        if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) { goto t_bitmap; }
          #endif // PACK_BM_VALUES || !B_JUDYL
        goto again;
          #else // LOOKUP
        goto switchTail;
          #endif // else LOOKUP
      #endif // else CODE_BM_SW

    } // end of t_list_sw
  #endif // CODE_LIST_SW

  #ifdef SKIP_TO_LIST
t_skip_to_list:
    {
        DBGX(printf("T_SKIP_TO_LIST\n"));
        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
      #ifdef COUNT
            DBGC(printf("T_SKIP_TO_LIST: COUNT PREFIX_MISMATCH %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt = gnListPopCnt(qy, nBLR);
                assert(wPopCnt != 0);
                DBGC(printf("T_SKIP_TO_LIST: PREFIX_MISMATCH wPopCnt %" _fw
                                "d\n", wPopCnt));
                wPopCntSum += wPopCnt; // break below to return wPopCntSum
                DBGC(printf("skls wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // COUNT
            goto break_from_main_switch;
        }
        goto t_list;
    } // end of t_skip_to_list
  #endif // SKIP_TO_LIST

  #if (cwListPopCntMax != 0)
#define T_LIST_LOOKUP_GUTS(_nBL, pLn, pwRoot, pwLnX, _wKey, _wRoot, _suffix) \
{ \
    nBLR = nBL = _nBL; \
    int nPos; /* nPos is not used after break_from_main_switch for LOOKUP */ \
    if (WROOT_IS_NULL(T_LIST, _wRoot) \
        || (PREFIX_CHECK_AT_LEAF(qy, _wKey) != Success) \
        || ((nPos = SEARCH_LIST(_suffix, qya, nBLR, _wKey)) < 0)) \
    { \
        goto break_from_main_switch; \
    } \
    SMETRICS_POP(j__SearchPopulation += gnListPopCnt(qy, nBLR)); \
    SMETRICS_GET(++j__GetCalls); \
    return BJL(&gpwValues(qy)[~nPos]) BJ1(KeyFound); \
}
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #ifdef _AUG_TYPE_X
  #ifdef BL_SPECIFIC_LIST
t_list_64: T_LIST_LOOKUP_GUTS(64, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
t_list_56: T_LIST_LOOKUP_GUTS(56, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
t_list_48: T_LIST_LOOKUP_GUTS(48, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
  #else // BL_SPECIFIC_LIST
t_list_64:
t_list_56:
t_list_48:
  #endif // BL_SPECIFIC_LIST else
t_list_40: // nBL == 40
    {
  #ifdef BL_SPECIFIC_LIST
        nBLR = nBL = 40;
  #endif // BL_SPECIFIC_LIST
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
    }
  #ifdef BL_SPECIFIC_LIST
t_list_32: T_LIST_LOOKUP_GUTS(32, pLn, pwRoot, pwLnX, wKey, wRoot, 32);
  #else // BL_SPECIFIC_LIST
t_list_32: // nBL == 32
  #endif // BL_SPECIFIC_LIST else
t_list_24: // nBL == 24
    {
  #ifdef BL_SPECIFIC_LIST
        nBLR = nBL = 24;
  #endif // BL_SPECIFIC_LIST else
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, 32);
    }
t_list_16: T_LIST_LOOKUP_GUTS(16, pLn, pwRoot, pwLnX, wKey, wRoot, 16);
t_list_8: T_LIST_LOOKUP_GUTS(8, pLn, pwRoot, pwLnX, wKey, wRoot, /*suffix*/ 8);
  #endif // _AUG_TYPE_X
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #ifdef AUGMENT_TYPE_8
  #ifdef LOOKUP
  #ifdef BL_SPECIFIC_LIST
t_list112: // nDL == 8
    {
      #if cnBitsLeftAtDl3 >= 24 // For AUGMENT_TYPE_8_PLUS_4?
        assert(nBL == AugTypeBitsInv(7 * (1 << cnBitsTypeMask)));
        assert(nBLR == nBL);
        nBLR = nBL = AugTypeBitsInv(7 * (1 << cnBitsTypeMask));
      #endif // cnBitsLeftAtDl3 >= 24
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
    } // end of t_list112
  #endif // BL_SPECIFIC_LIST
  #endif // LOOKUP
  #endif // AUGMENT_TYPE_8
  #endif // (cwListPopCntMax != 0)

  #ifndef _AUG_TYPE_X
  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #ifdef BL_SPECIFIC_LIST
t_list96: // nDL == 7
    {
        nBLR = nBL = AugTypeBitsInv(6 * (1 << cnBitsTypeMask));
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
    } // end of t_list96
  #endif // BL_SPECIFIC_LIST
  #endif // AUGMENT_TYPE_8 && LOOKUP
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
  #ifdef BL_SPECIFIC_LIST
t_list80: // nDL == 6
    {
        nBLR = nBL = AugTypeBitsInv(5 * (1 << cnBitsTypeMask));
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
    } // end of t_list80
  #endif // BL_SPECIFIC_LIST
  #endif // AUGMENT_TYPE_8 && LOOKUP
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
      #ifdef AUGMENT_TYPE_8
          #ifndef BL_SPECIFIC_LIST
t_list112: // nDL == 8
t_list96:  // nDL == 7
t_list80:  // nDL == 6
          #endif // !BL_SPECIFIC_LIST
t_list64: // nDL == 5
          #if !defined(BL_SPECIFIC_LIST) && cnBitsLeftAtDl3 > 24
t_list48: // nDL == 2
          #endif // !BL_SPECIFIC_LIST && cnBitsLeftAtDl3 > 24
      #else // AUGMENT_TYPE_8
t_list48: // nBL > 32
      #endif // else AUGMENT_TYPE_8
    {
      #ifdef BL_SPECIFIC_LIST
        nBLR = nBL = AugTypeBitsInv(4 * (1 << cnBitsTypeMask));
      #endif // BL_SPECIFIC_LIST
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
    } // end of t_list64
  #endif // AUGMENT_TYPE && LOOKUP
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
  #ifdef BL_SPECIFIC_LIST
t_list48: // nDL == 4
    {
        nBLR = nBL = AugTypeBitsInv(3 * (1 << cnBitsTypeMask));
      #if defined(AUGMENT_TYPE_8) && cnBitsLeftAtDl3 > 24
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, Word);
      #else // AUGMENT_TYPE_8 && cnBitsLeftAtDl3 > 24
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, 32);
      #endif // else AUGMENT_TYPE_8 && cnBitsLeftAtDl3 > 24
    } // end of t_list48
  #endif // BL_SPECIFIC_LIST
  #endif // AUGMENT_TYPE_8 && LOOKUP
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
      #if defined(AUGMENT_TYPE_8) && !defined(BL_SPECIFIC_LIST)
      #if cnBitsLeftAtDl3 <= 24
t_list48: // nDL == 4
      #endif // cnBitsLeftAtDl3 <= 24
      #endif // AUGMENT_TYPE_8 && !BL_SPECIFIC_LIST
t_list32:
    //      nDL ==  3 for  AUGMENT_TYPE_8
    // 16 < nBL <= 32 for !AUGMENT_TYPE_8
      #if defined(AUGMENT_TYPE_8) && !defined(BL_SPECIFIC_LIST)
      #if cnBitsLeftAtDl2 > 16
t_list16: // nDL == 2
      #endif // cnBitsLeftAtDl2 > 16
      #endif // AUGMENT_TYPE_8 && !BL_SPECIFIC_LIST
    {
      #ifdef BL_SPECIFIC_LIST
        assert(nBLR == AugTypeBitsInv(2 * (1 << cnBitsTypeMask)));
        nBLR = nBL = AugTypeBitsInv(2 * (1 << cnBitsTypeMask));
      #endif // BL_SPECIFIC_LIST
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, 32);
    } // end of case t_list32
  #endif // AUGMENT_TYPE && LOOKUP
  #endif // (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE) && defined(LOOKUP)
      #ifndef AUGMENT_TYPE_8
t_list16:
      #elif defined(BL_SPECIFIC_LIST) || cnBitsLeftAtDl2 <= 16
t_list16:
      #endif // !AUGMENT_TYPE_8 elif BL_SPECIFIC_LIST || cnBitsLeftAtDl2 <= 16
    //     nDL ==  2 for  AUGMENT_TYPE_8
    // 8 < nBL <= 16 for !AUGMENT_TYPE_8
  #if defined(AUGMENT_TYPE_8) && !defined(BL_SPECIFIC_LIST) && cnBitsInD1 > 8
t_list: // nDL == 1
  #endif // AUGMENT_TYPE_8 && !BL_SPECIFIC_LIST && cnBitsInD1 > 8
    {   // 8 < nBL <= 16 for AUGMENT_TYPE && !AUGMENT_TYPE_8
      #if defined(AUGMENT_TYPE_8)
          #if defined(BL_SPECIFIC_LIST) || !defined(AUGMENT_TYPE_8_PLUS_4)
        assert(nBLR == AugTypeBitsInv(1 << cnBitsTypeMask));
        nBLR = nBL = AugTypeBitsInv(1 << cnBitsTypeMask);
          #endif // BL_SPECIFIC_LIST || !AUGMENT_TYPE_8_PLUS_4
      #elif cnBitsLeftAtDl2 > 16
        assert(nBLR == cnBitsInD1);
        nBLR = nBL = cnBitsInD1;
      #elif cnBitsLeftAtDl3 > 16 && cnBitsInD1 <= 8
        assert(nBLR == cnBitsLeftAtDl2);
        nBLR = nBL = cnBitsLeftAtDl2;
      #elif cnBitsLeftAtDl3 + cnBitsPerDigit > 16 && cnBitsLeftAtDl2 <= 8
        assert(nBLR == cnBitsLeftAtDl3);
        nBLR = nBL = cnBitsLeftAtDl3;
      #else
        #pragma message("t_list16 cannot use a constant nBL.")
        // Not a bug. Just perf issue to flag.
        // nBLR is already set correctly. Just not a constant.
      #endif
      #if defined(BL_SPECIFIC_LIST) && cnBitsLeftAtDl2 > 16
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, 32);
      #else // BL_SPECIFIC_LIST && cnBitsLeftAtDl2 > 16
        T_LIST_LOOKUP_GUTS(nBL, pLn, pwRoot, pwLnX, wKey, wRoot, 16);
      #endif // BL_SPECIFIC_LIST && cnBitsLeftAtDl2 > 16 else
    } // end of t_list16
  #endif // AUGMENT_TYPE && LOOKUP
  #endif // (cwListPopCntMax != 0)
  #endif // !_AUG_TYPE_X

// Notes on LOOKUP for AUGMENT_TYPE_8_PLUS_4 without BL_SPECIFIC_LIST.
// We want at most 4 code blocks. nBL <= 8, 16, 32, 64.
// We will use the AUGMENT_TYPE w/o AUGMENT_TYPE_8 code blocks.
// If cnBitsInD1 > 8, then goto 2-byte code block. Label it with t_list.
// - Delete 1-byte code block.
// - Do not set nBLR in 2-byte code block -- unless we can.
// - Disable use of (LocateKey|HasKey)32 in 2-byte code block.
// If cnBitsLeftAtDl2 > 16, then goto 4-byte code block.
// Label it with t_list16.
// - Delete 2-byte code block -- if appropriate.
// - Do not set nBLR in 4-byte code block.
// If cnBitsLeftAtDl3 > 24, i.e. nBL at Dl4 > 32, then goto 8-byte code block.
// Label it with t_list48.
// - Do not set nBLR in 8-byte code block.

  #if !defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)
// t_list is not needed for AUGMENT_TYPE and LOOKUP if cnBitsInD1 > 8.
  #ifdef _AUG_TYPE_X_LOOKUP
      #ifndef LOOKUP
    #define _ONE_BYTE_T_LIST
      #endif // !LOOKUP
  #elif !defined(AUGMENT_TYPE_8) || defined(BL_SPECIFIC_LIST)
    #define _ONE_BYTE_T_LIST
  #elif cnBitsInD1 <= 8 || !defined(LOOKUP)
    #define _ONE_BYTE_T_LIST
  #elif defined(AUG_TYPE_8_SW_NEXT) && defined(NEXT)
    #define _ONE_BYTE_T_LIST
  #endif // _AUG_TYPE_X_LOOKUP elif ...
  #ifdef _ONE_BYTE_T_LIST
  #ifdef _T_LIST
t_list:
    // nDL == 1  for                   AUGMENT_TYPE_8
    // nBL <= 8  for  AUGMENT_TYPE && !AUGMENT_TYPE_8
    // all cases for !AUGMENT_TYPE
    {
        DBGX(Checkpoint(qya, "t_list"));
        // Establish a constant nBL and nBLR for the compiler, if possible.
      #ifdef AUGMENT_TYPE
      #ifdef LOOKUP
          #if defined(_AUG_TYPE_X) && !defined(BL_SPECIFIC_LIST)
          #elif defined(AUGMENT_TYPE_8) || (cnBitsInD1 == 8)
        assert(nBLR == AugTypeBitsInv(0));
        nBLR = nBL = AugTypeBitsInv(0);
          #elif cnBitsInD1 <= 8 && cnBitsLeftAtDl2 > 8
        assert(nBLR == cnBitsInD1);
        nBLR = nBL = cnBitsInD1;
          #elif cnBitsInD1 <= 16 && cnBitsLeftAtDl2 > 16
        assert(nBLR == cnBitsInD2);
        nBLR = nBL = cnBitsInD2;
          #else
        #pragma message("Can't use a constant nBL in t_list.")
        // nBLR is already set correctly. Just not a constant.
          #endif // _AUG_TYPE_X && BL_SPECIFIC_LIST elif ...
      #endif // LOOKUP
      #endif // AUGMENT_TYPE

        IF_INSERT(if (bCleanup) return BJL(pwValue)BJ1(Success));
        IF_REMOVE(if (bCleanup) return Success);

        // Ifdef out valid assertions to speed regression testing.
        DBGX(assert((pwr != NULL) || WROOT_IS_NULL(T_LIST, wRoot)));
        DBGX(assert((pwr == NULL) || (gnListBLR(qy) == nBL)));

        if (WROOT_IS_NULL(T_LIST, wRoot)
            || (PREFIX_CHECK_AT_LEAF(qy, wKey) != Success))
        {
            IF_INS_OR_REM(nPos = 0);
            IF_NOT_INSERT(goto break_from_main_switch);
            IF_INSERT(goto t_list_request_cleanup_if_needed);
        }

        // Search the list.
        // AUGMENT_TYPE_8_PLUS_4 and PACK_L1_VALUES both complicate this.
        // I wonder if we could incorporate them into the SEARCH_LIST macro.
      #ifdef LOOKUP
          #if defined(AUGMENT_TYPE) && !defined(AUGMENT_TYPE_NOT)
              #ifdef _AUG_TYPE_X
        if ((nPos = SEARCH_LIST(, qya, nBLR, wKey)) >= 0)
              #elif defined(AUGMENT_TYPE_8) && cnBitsInD1 > 8
        if ((nPos = SEARCH_LIST(16, qya, nBLR, wKey)) >= 0)
              #else // AUGMENT_TYPE_8 && cnBitsInD1 > 8
                  #if defined(B_JUDYL) && !defined(PACK_L1_VALUES)
        if ((nPos = -!ListHasKey8(qya, nBLR, wKey)) >= 0)
                  #else // B_JUDYL && !PACK_L1_VALUES
        if ((nPos = SEARCH_LIST(8, qya, nBLR, wKey)) >= 0)
                  #endif // B_JUDYL && !PACK_L1_VALUES else
              #endif // AUGMENT_TYPE_8 && cnBitsInD1 > 8 else
          #else // AUGMENT_TYPE && !AUGMENT_TYPE_NOT
        if ((nPos =
              #if defined(B_JUDYL) && !defined(PACK_L1_VALUES)
                ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1))
                        ? -!ListHasKey8(qya, nBLR, wKey) :
              #endif // B_JUDYL && !PACK_L1_VALUES
                    SEARCH_LIST(, qya, nBLR, wKey)) >= 0)
          #endif // AUGMENT_TYPE && !AUGMENT_TYPE_NOT else
      #elif defined(NEXT)
        if ((nPos = LocateGeKeyInList(qya, nBLR, &wKey)) >= 0)
      #elif defined(COUNT)
        if ((nPos = nBLR <= 32
                  ? LocateGeKeyInList(qya, nBLR, &wKey)
                  : SearchList(qya, nBLR, wKey))
            >= 0)
      #else // LOOKUP elif NEXT
        if ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
      #endif // LOOKUP elif NEXT else
        {
            SMETRICS_POP(j__SearchPopulation += gnListPopCnt(qy, nBLR));
            SMETRICS_GET(++j__GetCalls);
      #ifndef RECURSIVE
            IF_INSERT(if (nIncr > 0) goto undo); // undo counting
      #endif // !RECURSIVE
            IF_REMOVE(goto removeGutsAndCleanup);
            IF_COUNT(nPos ^= -1); // could/should count be returning here?
      #if defined(LOOKUP) || defined(INSERT) || defined(NEXT)
            IF_NEXT(*pwKey = wKey);
          #if B_JUDYL
            return
              #ifndef PACK_L1_VALUES
                ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1))
                    ? &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))] :
              #endif // #ifndef PACK_L1_VALUES
                      &gpwValues(qy)[~nPos];
          #else // B_JUDYL
            // Success for Lookup and Remove; Failure for Insert
            return KeyFound;
          #endif // B_JUDYL else
      #endif // LOOKUP || INSERT || NEXT
        }
      #if defined(INSERT) || defined(REMOVE) || defined(COUNT)
        nPos ^= -1;
      #endif // INSERT || REMOVE || COUNT
        IF_COUNT(wPopCntSum += nPos);
      #ifdef INSERT
t_list_request_cleanup_if_needed:
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
            DBGX(Checkpoint(qya, "T_LIST req cleanup"));
            bCleanupRequested = 1; // goto cleanup when done
        }
              #endif // BITMAP
          #endif // (cn2dBmMaxWpkPercent != 0)
      #endif // INSERT
        goto break_from_main_switch;
    } // end of t_list
  #endif // _T_LIST
  #endif // _ONE_BYTE_T_LIST
  #endif // !SEPARATE_T_NULL || (cwListPopCntMax == 0)

  #ifdef UA_PARALLEL_128
t_list_ua:
    {
        nBLR = nBL = 16;
        DBGX(Checkpoint(qya, "t_list_ua"));
        IF_INS_OR_REM(if (bCleanup) { return Success; }) // cleanup is complete
        // Search the list.  wPopCnt is the number of keys in the list.
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
      #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
            if (1
          #if defined(LOOKUP) && !defined(B_JUDYL)
                && ListHasKey(qya, nBLR, wKey)
          #else // LOOKUP && !B_JUDYL
                && ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
          #endif // LOOKUP && !B_JUDYL else
                )
      #endif // !LOOKUP || !LOOKUP_NO_LIST_SEARCH
            {
      #ifndef RECURSIVE
                IF_INSERT(if (nIncr > 0) { goto undo; }) // undo counting
      #endif // !RECURSIVE
                IF_REMOVE(goto removeGutsAndCleanup);
                IF_COUNT(wPopCntSum += nPos);
                IF_NOT_REMOVE(return KeyFound);
            }
      #ifdef NEXT
            else {
                int nPopCnt = gnListPopCnt(qy, nBLR);
                if (~nPos < nPopCnt) {
                    uint16_t* psKeys = ls_psKeysX(pwr, nBLR, nPopCnt);
                    *pwKey = (wKey & ~NZ_MSK(nBLR)) | psKeys[~nPos];
                }
                return Success;
            }
      #elif !defined(LOOKUP)
            { nPos ^= -1; }
      #endif // NEXT elif !LOOKUP
        }
        IF_COUNT(wPopCntSum += nPos);
        goto break_from_main_switch;
    } // end of t_list_ua
  #endif // UA_PARALLEL_128

  #ifdef XX_LISTS
t_xx_list:
    {
        DBGX(Checkpoint(qya, "t_xx_list"));

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
        if (WROOT_IS_NULL(T_XX_LIST, wRoot)) {
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
                && ((wr_nType(WROOT_NULL) != T_LIST) || (wRoot != WROOT_NULL))
          #if defined(LOOKUP)
              #if defined(B_JUDYL)
                  #if defined(HASKEY_FOR_JUDYL_LOOKUP)
                // HASKEY_FOR_JUDYL_LOOKUP is for analysis only.
                && ((nPos = -!ListHasKey(qya, nBLR, wKey)) >= 0)
                  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
                && ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
                  #else // defined(HASKEY_FOR_JUDYL_LOOKUP) elif ...
                && ((nPos = LocateKeyInList(qya, nBLR, wKey)) >= 0)
                  #endif // defined(HASKEY_FOR_JUDYL_LOOKUP)
              #else // defined(B_JUDYL)
                  #if defined(SEARCH_FOR_JUDY1_LOOKUP)
                && (SearchList(qya, nBLR, wKey) >= 0)
                  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
                && (LocateKeyInList(qya, nBLR, wKey) >= 0)
                  #else // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
                && ListHasKey(qya, nBLR, wKey)
                  #endif // defined(SEARCH_FOR_JUDY1_LOOKUP) elif ...
              #endif // defined(B_JUDYL)
          #else // defined(LOOKUP)
                && ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
          #endif // defined(LOOKUP)
                )
      #endif // ! defined(LOOKUP) !! ! defined(LOOKUP_NO_LIST_SEARCH)
            {
                SMETRICS_GET(++j__GetCalls);
      #if defined(INSERT)
          #if ! defined(RECURSIVE)
                if (nIncr > 0) { goto undo; } // undo counting
          #endif // ! defined(RECURSIVE)
      #endif // defined(INSERT)
      #if defined(REMOVE)
                goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
      #if defined(LOOKUP) || defined(INSERT) || defined(NEXT)
                IF_NEXT(*pwKey = wKey);
          #ifdef B_JUDYL
                DBGX(printf("Lookup (or Insert) returning nPos %d %p 0x%zx\n",
                             nPos,
                             &gpwValues(qy)[~nPos], gpwValues(qy)[~nPos]));
                return
              #ifndef PACK_L1_VALUES
                    ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1))
                        ? &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))] :
              #endif // #ifndef PACK_L1_VALUES
                          &gpwValues(qy)[~nPos];
          #else // B_JUDYL
                // Success for Lookup and Remove; Failure for Insert
                return KeyFound;
          #endif // B_JUDYL else
      #endif // LOOKUP || INSERT || NEXT
            }
      #ifdef NEXT
            else {
                if (!WROOT_IS_NULL(T_LIST, wRoot)) {
                    int nPopCnt = gnListPopCnt(qy, nBLR);
                    if (~nPos < nPopCnt) {
          #ifdef COMPRESSED_LISTS
                        if (nBLR <= 8) {
                            uint8_t* pcKeys = ls_pcKeysX(pwr, nBLR, nPopCnt);
                            *pwKey = (wKey & ~NZ_MSK(nBLR)) | pcKeys[~nPos];
                        } else if (nBLR <= 16) {
                            uint16_t* psKeys = ls_psKeysX(pwr, nBLR, nPopCnt);
                            *pwKey = (wKey & ~NZ_MSK(nBLR)) | psKeys[~nPos];
              #if cnBitsPerWord > 32
                        } else if (nBLR <= 32) {
                            uint32_t* piKeys = ls_piKeysX(pwr, nBLR, nPopCnt);
                            *pwKey = (wKey & ~NZ_MSK(nBLR)) | piKeys[~nPos];
              #endif // cnBitsPerWord > 32
                        } else
          #endif // COMPRESSED_LISTS
                        {
                            Word_t* pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
                            *pwKey = pwKeys[~nPos];
                        }
          #ifdef B_JUDYL
                        return
              #ifndef PACK_L1_VALUES
                            ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1))
                                ? &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))] :
              #endif // #ifndef PACK_L1_VALUES
                                  &gpwValues(qy)[nPos];
          #else // B_JUDYL
                        return Success;
          #endif // B_JUDYL else
                    }
                }
                // Get tryNextDigit to skip over aliases?
                nBW -= nBLR - nBL;
                nBL = nBLR;
            }
      #endif // NEXT
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

        goto break_from_main_switch;

    } // end of t_xx_list
  #endif // XX_LISTS

  #if defined(PACK_BM_VALUES) || !defined(B_JUDYL)
  #ifdef SKIP_TO_BITMAP
t_skip_to_bitmap:
    {
        DBGX(printf("T_SKIP_TO_BITMAP\n"));
        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
      #ifdef COUNT
            DBGC(printf("T_SKIP_TO_BITMAP: COUNT PREFIX_MISMATCH %" _fw"d\n",
                        wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt = gwBitmapPopCnt(qya, nBLR);
                DBGC(printf("T_SKIP_TO_BITMAP: PREFIX_MISMATCH wPopCnt %" _fw
                                "d\n", wPopCnt));
                wPopCntSum += wPopCnt; // fall through to return wPopCntSum
                DBGC(printf("skbm wPopCnt " OWx" wPopCntSum " OWx"\n",
                            wPopCnt, wPopCntSum));
            }
      #endif // COUNT
      #ifdef NEXT
            // If we're not past the prefix then advance to it.
            // If we are past the prefix then advance to the next digit.
            if (wKey < (wKey - (wPrefixMismatch << nBLR))) {
                // wPrefixMismatch = (wKey - wPrefix) >> nBLR;
                wKey &= ~MSK(nBLR);
                wKey -= (wPrefixMismatch << nBLR);
                goto restart;
            }
      #endif // NEXT
            goto break_from_main_switch;
        }
        // We don't support skip to unpacked bitmap yet.
  #ifdef AUGMENT_TYPE
  #ifdef LOOKUP
  #if cn2dBmMaxWpkPercent != 0
        if (nBLR > cnBitsInD1) {
            goto t_bm_plus_16;
        }
  #endif // cn2dBmMaxWpkPercent != 0
  #endif // LOOKUP
  #endif // AUGMENT_TYPE
        goto t_bitmap;
    } // end of t_skip_to_bitmap
  #endif // SKIP_TO_BITMAP
  #endif // PACK_BM_VALUES || !B_JUDYL

  #ifdef BITMAP
  #ifdef AUGMENT_TYPE
  #ifdef LOOKUP
  #if cn2dBmMaxWpkPercent != 0
t_bm_plus_16:
    {
        assert(nBLR == cnBitsLeftAtDl2);
        nBLR = cnBitsLeftAtDl2;
      #ifndef SKIP_TO_BITMAP
        nBL = nBLR; // We don't use nBL for LOOKUP except for DEBUG.
      #endif // !SKIP_TO_BITMAP
      #ifdef LOOKUP_NO_BITMAP_DEREF
        return KeyFound;
      #else // LOOKUP_NO_BITMAP_DEREF
          #ifdef SKIP_PREFIX_CHECK
        if (PrefixCheckAtLeaf(qy, wKey
              #ifndef ALWAYS_CHECK_PREFIX_AT_LEAF
                , bNeedPrefixCheck
              #endif // !ALWAYS_CHECK_PREFIX_AT_LEAF
              #ifdef SAVE_PREFIX_TEST_RESULT
                , wPrefixMismatch
              #else // SAVE_PREFIX_TEST_RESULT
                , pwrUp
              #endif // SAVE_PREFIX_TEST_RESULT else
              #ifdef SAVE_PREFIX
                , pLnPrefix, pwrPrefix, nBLRPrefix
              #endif // SAVE_PREFIX
                  ) // end call to PrefixCheckAtLeaf
            == Success)
          #endif // SKIP_PREFIX_CHECK
        {
          #ifdef LOOKUP_NO_BITMAP_SEARCH
            // BUG?: Is pwrUp valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            assert(gwPopCnt(qya, cnBitsInD1) != 0);
              #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // ? cnBitsLeftAtDl2 ?
            assert(gwPopCnt(qya, cnBitsInD2) != 0);
              #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            return KeyFound;
          #else // LOOKUP_NO_BITMAP_SEARCH
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
            // We end up here with (nBLR <= cnBitsInD1)?
            // We have to be sure to handle that case below
            // for USE_XX_SW_ONLY_AT_DL2?
              #endif // USE_XX_SW_ONLY_AT_DL2
            // Use compile-time tests to speed this up. Hopefully.
            if ((wr_nType(WROOT_NULL) == T_BITMAP) && (wRoot == WROOT_NULL)) {
                goto break_from_main_switch;
            }
            SMETRICS_GETN(++j__GetCallsNot);
            if (BitIsSet(((BmLeaf_t*)pwr)->bmlf_awBitmap, wKey & MSK(nBLR))) {
                return KeyFound;
            }
            DBGX(printf("Bit is not set.\n"));
          #endif // LOOKUP_NO_BITMAP_SEARCH
        }
          #ifdef SKIP_LINKS
          #ifdef SKIP_PREFIX_CHECK
        else
        {
            // Shouldn't this be using the previous nBL for the pwrUp case?
                  #ifdef SKIP_TO_BITMAP
            // But now that we have prefix in the bitmap can't we use that?
                  #endif // SKIP_TO_BITMAP
            DBGX(printf("Mismatch at bitmap wPrefix " OWx" nBLR %d nBL %d\n",
                  #ifdef PP_IN_LINK
                        gwPrefix(qy),
                  #else // PP_IN_LINK
                        PWR_wPrefixNATBL(NULL, pwrUp, nBL),
                  #endif // PP_IN_LINK
                        nBLR, nBL));
        }
          #endif // SKIP_PREFIX_CHECK
          #endif // SKIP_LINKS
      #endif // LOOKUP_NO_BITMAP_DEREF else
        goto break_from_main_switch;
    } // end of t_bm_plus_16
  #endif // cn2dBmMaxWpkPercent != 0
  #endif // LOOKUP
  #endif // AUGMENT_TYPE
  #endif // BITMAP

  #ifdef UNPACK_BM_VALUES
      #ifndef LOOKUP
t_unpacked_bm:
      #endif // !LOOKUP
  #endif // UNPACK_BM_VALUES
  #ifdef BITMAP
  #if defined(PACK_BM_VALUES) || !defined(LOOKUP) || !defined(B_JUDYL)
t_bitmap:
    {
      #if defined(AUGMENT_TYPE) && !defined(AUGMENT_TYPE_8) && cnBitsInD1 > 8
        #error AUGMENT_TYPE (not _8) with cnBitsInD1 > 8.
      #endif // AUGMENT_TYPE && !AUGMENT_TYPE_8 && cnBitsInD1 > 8
      #ifdef B_JUDYL
        // nBLR may not be updated for goto embedded t_bitmap from digit 2
          #ifndef CHECK_TYPE_FOR_EBM
          #ifdef _BMLF_BM_IN_LNX
          #if defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
          #ifndef UNPACK_BM_VALUES
        if (cnListPopCntMaxDl1 == 0) {
        } else
          #endif // !UNPACK_BM_VALUES
          #endif // defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
          #endif // _BMLF_BM_IN_LNX
          #endif // !CHECK_TYPE_FOR_EBM
        { assert(nBLR == cnBitsInD1); }
        nBLR = cnBitsInD1;
          #ifndef SKIP_TO_BITMAP
        nBL = nBLR; // We don't use nBL for LOOKUP except for DEBUG.
          #endif // !SKIP_TO_BITMAP
      #else // B_JUDYL
          #if cn2dBmMaxWpkPercent == 0
        assert(nBLR == cnBitsInD1);
        nBLR = cnBitsInD1;
              #ifndef SKIP_TO_BITMAP
        nBL = nBLR; // We don't use nBL for LOOKUP except for DEBUG.
              #endif // !SKIP_TO_BITMAP
          #endif // cn2dBmMaxWpkPercent == 0
          #ifdef AUGMENT_TYPE
          #ifdef LOOKUP
        assert(nBLR == cnBitsInD1);
        nBLR = cnBitsInD1;
              #ifndef SKIP_TO_BITMAP
        nBL = nBLR; // We don't use nBL for LOOKUP except for DEBUG.
              #endif // !SKIP_TO_BITMAP
          #endif // LOOKUP
          #endif // AUGMENT_TYPE
      #endif // B_JUDYL else
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
        {
      #ifdef NEXT
// To do: figure out if nBLR <= cnLogBitsPerWord at compile time.
            int nBitNum = wKey & MSK(nBLR) & MSK(cnLogBitsPerWord);
          #ifdef _BMLF_BM_IN_LNX
            Word_t* pwBitmap = pwLnX;
          #else // _BMLF_BM_IN_LNX
            Word_t* pwBitmap =
                (cbEmbeddedBitmap && (nBLR <= cnLogBitsPerLink))
                    ? (Word_t*)pLn // Judy1 embedded bitmap.
                    : ((BmLeaf_t*)pwr)->bmlf_awBitmap;
          #endif // else _BMLF_BM_IN_LNX
            int nWordNum = (wKey & MSK(nBLR)) >> cnLogBitsPerWord;
            Word_t wBm = pwBitmap[nWordNum] & ~MSK(nBitNum);
            for (;;) {
                if (wBm) {
                    nBitNum = __builtin_ctzll(wBm);
                    wKey &= ~MSK(nBLR);
                    wKey |= ((nWordNum << cnLogBitsPerWord) + nBitNum);
                    *pwKey = wKey;
          #ifdef B_JUDYL
              #ifdef BMLF_INTERLEAVE
              #else // BMLF_INTERLEAVE
                    Word_t* pwBitmapValues = gpwBitmapValues(qy, cnBitsInD1);
                    int nIndex =
                        BM_UNPACKED(wRoot) ? (int)(wKey & MSK(cnBitsInD1)) :
                            BmIndex(qya, cnBitsInD1, wKey);
                    return &pwBitmapValues[nIndex];
              #endif // BMLF_INTERLEAVE else
          #endif // B_JUDYL
                    BJ1(return Success);
                }
                if (++nWordNum
                    >= (int)((cbEmbeddedBitmap && (nBLR <= cnLogBitsPerWord))
                        ? 0 : EXP(nBLR - cnLogBitsPerWord)))
                {
          #ifdef SKIP_TO_BITMAP
          #ifndef NO_SKIP_AT_TOP
                    if (nBL >= cnBitsPerWord) {
                        break;
                    }
          #endif // NO_SKIP_AT_TOP
          #endif // SKIP_TO_BITMAP
                    { goto tryNextDigit; }
                }
                wBm = pwBitmap[nWordNum];
            }
            goto break_from_main_switch;
      #endif // NEXT
      #ifdef COUNT
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
                    Word_t wBit = EXP(wKey & NZ_MSK(nBLR));
                    Word_t wBmMask = wBit - 1;
                    Word_t wBits =
                        ((cnLogBitsPerLink == cnLogBitsPerWord)
                                && ((Word_t*)pLn == &pLn->ln_wRoot))
                            ? wRoot : *(Word_t*)pLn;
                    wPopCnt = __builtin_popcountll(wBits & wBmMask);
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
          #if cn2dBmMaxWpkPercent != 0
                if (nBLR == cnBitsLeftAtDl2) {
                    int nCntNum = (wKey & MSK(cnBitsLeftAtDl2))
                                    >> cnLogBmlfBitsPerCnt;
                    wPopCnt = 0;
              #ifdef BMLF_COUNT_CNTS_BACKWARD
                    if (wKey & EXP(nBLR - 1)) {
                        wPopCnt = gwBitmapPopCnt(qya, nBLR);
                        for (int ii = nCntNum;
                             ii < (int)EXP(cnBitsLeftAtDl2)
                                    >> cnLogBmlfBitsPerCnt;
                             ++ii)
                        {
                            wPopCnt -= gpxBitmapCnts(qya, nBLR)[ii];
                        }
                    } else
              #endif // BMLF_COUNT_CNTS_BACKWARD
                    {
                        for (int ii = 0; ii < nCntNum; ++ii) {
                            wPopCnt += gpxBitmapCnts(qya, nBLR)[ii];
                        }
                    }
                    int nWordNum = nWordOffset
                        & MSK(cnLogBmlfBitsPerCnt - cnLogBitsPerWord);
              #ifdef BMLF_COUNT_BITS_BACKWARD
                    if (wKey & EXP(cnLogBmlfBitsPerCnt - cnLogBitsPerWord - 1))
                    {
                        wPopCnt += gpxBitmapCnts(qya, nBLR)[nCntNum];
                        for (int jj = nWordNum + 1;
                             jj < (int)EXP(cnLogBmlfBitsPerCnt)
                                    / cnBitsPerWord; ++jj)
                        {
                            wPopCnt -= __builtin_popcountll(pwBitmap
                                [(nCntNum << (cnLogBmlfBitsPerCnt
                                                  - cnLogBitsPerWord)) + jj]);
                        }
                        Word_t wBmMask
                            = ~(((Word_t)1
                                << (wKey & (cnBitsPerWord - 1))) - 1);
                        wPopCnt -= __builtin_popcountll(pwBitmap[nWordOffset]
                                                            & wBmMask);
                    } else
              #endif // BMLF_COUNT_BITS_BACKWARD
                    {
                        for (int jj = 0; jj < nWordNum; ++jj) {
                            wPopCnt += __builtin_popcountll(pwBitmap
                                [(nCntNum << (cnLogBmlfBitsPerCnt
                                                  - cnLogBitsPerWord)) + jj]);
                        }
                        Word_t wBmMask
                            = ((Word_t)1 << (wKey & (cnBitsPerWord - 1))) - 1;
                        wPopCnt += __builtin_popcountll(pwBitmap[nWordOffset]
                                                            & wBmMask);
                    }
                } else
          #endif // cn2dBmMaxWpkPercent != 0
                if ((nBLR > 8) && (wKey & EXP(nBLR - 1))) {
                    wPopCnt = gwBitmapPopCnt(qya, nBLR);
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
      #endif // COUNT
          #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            // BUG?: Is pwrUp valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
              #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            assert(gwPopCnt(qya, cnBitsInD1) != 0);
              #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // ? cnBitsLeftAtDl2 ?
            assert(gwPopCnt(qya, cnBitsInD2) != 0);
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
            // We end up here with (nBLR <= cnBitsInD1)?
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
            // T_UNPACKED_BM doesn't come here for Lookup.
            SMETRICS_GET(++j__GetCalls);
            Word_t wPopCnt = gwBitmapPopCnt(qya, cnBitsInD1);
            SMETRICS_POP(j__SearchPopulation += wPopCnt); // fast and slow
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
                  #else // B_JUDYL
            SMETRICS_GETN(++j__GetCallsNot);
                  #endif // B_JUDYL else
            // Use compile-time tests to speed this up. Hopefully.
  #if defined(B_JUDYL) && defined(LOOKUP) && defined(BMLF_POP_COUNT_1_NO_TEST)
            int bBitIsSet = 1;
  #else // B_JUDYL && LOOKUP && BMLF_POP_COUNT_1_NO_TEST
            int bBitIsSet =
                      #ifdef _BMLF_BM_IN_LNX
                BitIsSetInWord(*pwLnX, wKey & MSK(cnBitsInD1));
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
                        ? BitIsSetInWord(
                            ((cnLogBitsPerLink == cnLogBitsPerWord)
                                    && ((Word_t*)pLn == &pLn->ln_wRoot))
                                ? wRoot : *(Word_t*)pLn,
                            wKey & NZ_MSK(cnBitsInD1)
                                         )
                        : BitIsSet(cbEmbeddedBitmap
                                       ? (Word_t*)pLn
                                       : ((BmLeaf_t*)pwr)->bmlf_awBitmap,
                                   wKey & MSK(cnBitsInD1))
                    :
                      (cbEmbeddedBitmap
                            && (cnBitsLeftAtDl2 <= cnLogBitsPerWord))
// BUG: using wRoot for 2-digit embedded bitmap even if wRoot is not
// the first word in Link_t.
                        ? BitIsSetInWord(wRoot, wKey & MSK(cnBitsLeftAtDl2))
                        : BitIsSet((cbEmbeddedBitmap
                                           && (cnBitsLeftAtDl2
                                               <= cnLogBitsPerLink))
                                       ? (Word_t*)pLn
                                       : ((BmLeaf_t*)pwr)->bmlf_awBitmap,
                                   wKey & MSK(cnBitsLeftAtDl2));
                          #endif // #else USE_XX_SW_ONLY_AT_DL2
                      #endif // else _BMLF_BM_IN_LNX
  #endif // B_JUDYL && LOOKUP && BMLF_POP_COUNT_1_NO_TEST else
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
                          #ifndef PREFETCH_BM_PREV_VAL
                          #ifndef PF_BM_PREV_HALF_VAL
                          #ifndef PREFETCH_BM_PSPLIT_VAL
                          #ifndef PF_BM_NEXT_HALF_VAL
                          #ifndef PREFETCH_BM_NEXT_VAL
                //Word_t wDiff = 0;
                if (1)
                          #endif // !PREFETCH_BM_NEXT_VAL
                          #endif // !PF_BM_NEXT_HALF_VAL
                          #endif // !PREFETCH_BM_PSPLIT_VAL
                          #endif // !PF_BM_PREV_HALF_VAL
                          #endif // !PREFETCH_BM_PREV_VAL
                          #ifndef PREFETCH_BM_PREV_VAL
                          #ifndef PF_BM_PREV_HALF_VAL
                          #ifdef PREFETCH_BM_PSPLIT_VAL
                          #ifndef PF_BM_NEXT_HALF_VAL
                          #ifndef PREFETCH_BM_NEXT_VAL
                Word_t wDiff
                    = (Word_t)pwValue - ((Word_t)pcPrefetch & ~MSK(6));
                if (wDiff < 64)
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
                Word_t wDiff
                    = (Word_t)pwValue - (((Word_t)pcPrefetch - 32) & ~MSK(6));
                if (wDiff < 128)
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
                Word_t wDiff
                    = (Word_t)pwValue - (((Word_t)pcPrefetch - 64) & ~MSK(6));
                if (wDiff < 192)
                          #endif // PREFETCH_BM_NEXT_VAL
                          #endif // #ifndef PF_BM_NEXT_HALF_VAL
                          #endif // PREFETCH_BM_PSPLIT_VAL
                          #endif // #ifndef PF_BM_PREV_HALF_VAL
                          #endif // PREFETCH_BM_PREV_VAL
                {
                    SMETRICS_HIT(++j__DirectHits);
                } else {
                    // Strangely, I think I've seen numbers that show it is
                    // faster to:
                    // if (wDiff < (Word_t)pwValue) {
                    //     SMETRICS_NHIT(++j__GetCallsP);
                    // } else {
                    //     SMETRICS_NHIT(++j__GetCallsM);
                    // }
                    // But it is just too hard to believe to leave it in.
                    SMETRICS_NHIT(++j__NotDirectHits);
                }
                      #endif // PACK_BM_VALUES
                      #endif // LOOKUP
                return pwValue;
                  #endif // else BMLF_INTERLEAVE && !LOOKUP
              #else // (LOOKUP || INSERT) && B_JUDYL
                return IF_NEXT(BJL(NULL) BJ1(Success)) IF_NOT_NEXT(KeyFound);
              #endif // #else (LOOKUP || INSERT) && B_JUDYL
            }
            DBGX(printf("Bit is not set.\n"));
          #endif // defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
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
      #endif // else LOOKUP && LOOKUP_NO_BITMAP_DEREF

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
        goto break_from_main_switch;
    } // end of t_bitmap
  #endif // PACK_BM_VALUES || !LOOKUP || !B_JUDYL
  #endif // BITMAP

  #ifdef UNPACK_BM_VALUES
      #ifdef LOOKUP
t_unpacked_bm:
    {
          #ifdef SMETRICS_UNPACKED_BM
        SMETRICS_GET(++j__GetCalls);
        SMETRICS_HIT(++j__DirectHits);
        SMETRICS_POP(j__SearchPopulation += gwBitmapPopCnt(qya, cnBitsInD1));
          #else // SMETRICS_UNPACKED_BM
        SMETRICS_GETN(++j__GetCallsNot);
          #endif // SMETRICS_UNPACKED_BM else
        // We don't use nBLR or nBL in this case.
        // This case is for JudyL only and JudyL doesn't support a bitmap
        // above cnBitsInD1.
        assert(nBLR == cnBitsInD1);
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
          #endif // LOOKUP_NO_BITMAP_DEREF
        goto break_from_main_switch;

    } // end of t_unpacked_bm
      #endif // LOOKUP
  #endif // UNPACK_BM_VALUES

  #ifdef EMBED_KEYS

      #ifdef HANDLE_BLOWOUTS
    // We haven't written the insert code to create blow-outs for
    // NO_TYPE_IN_XX_SW yet.
          #ifdef NO_TYPE_IN_XX_SW
    // A blowout has the high bit set and the high bit of the next key
    // slot clear. Type field can be anything other than T_EMBEDED_KEYS.
    // 44 pointer bits can be anything.
    // High bit must be set.
    // nBL is never less than 7 so next six high bits are always usable.
    // And 8 of the next 9 high bits are usable, but exactly which ones
    // depends on nBL.
    #define BLOWOUT_CHECK(_nBL) \
        ((wRoot & BLOWOUT_MASK(_nBL)) == (ZERO_POP_MAGIC & ~cnMallocMask))
          #else // NO_TYPE_IN_XX_SW
    #define BLOWOUT_CHECK(_nBL)  (wr_nType(wRoot) != T_EMBEDDED_KEYS)
          #endif // NO_TYPE_IN_XX_SW
      #else // HANDLE_BLOWOUTS
    #define BLOWOUT_CHECK(_nBL) (0)
      #endif // HANDLE_BLOWOUTS else

      #if defined(LOOKUP) && defined(ZERO_POP_CHECK_BEFORE_GOTO)
    #define ZERO_CHECK  (0)
      #else // LOOKUP && ZERO_POP_CHECK_BEFORE_GOTO
    #define ZERO_CHECK  (wRoot == WROOT_NULL)
      #endif // LOOKUP && ZERO_POP_CHECK_BEFORE_GOTO else

      #if defined(NO_TYPE_IN_XX_SW) || defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    #define HANDLE_DL2(_nBL) \
        if ((_nBL) < nDL_to_nBL(2)) { \
            if (ZERO_CHECK) { goto break_from_main_switch; } \
            if (BLOWOUT_CHECK(_nBL)) { goto again; } \
        }
      #else // NO_TYPE_IN_XX_SW || HANDLE_DL2_IN_EMBEDDED_KEYS
    #define HANDLE_DL2(_nBL)
      #endif // NO_TYPE_IN_XX_SW || HANDLE_DL2_IN_EMBEDDED_KEYS

      #ifdef _AUG_TYPE_X_EK
        #define _AUG_TYPE_EK
      #elif defined(_AUG_TYPE_8_EK) // AUG_TYPE_64_LOOKUP && LOOKUP
        #define _AUG_TYPE_EK
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP elif _AUG_TYPE_8_EK

      #ifdef _AUG_TYPE_EK

          #ifdef LOOKUP_NO_LIST_SEARCH
    #error
          #endif // LOOKUP_NO_LIST_SEARCH
          #ifdef SKIP_PREFIX_CHECK
    #error
          #endif // SKIP_PREFIX_CHECK
          #ifndef EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
    #error
          #endif // !EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP

          #ifdef SMETRICS_EK
    #define SMETRICS_EK_GUTS(_nBL, _wRoot) \
        SMETRICS_GET(++j__GetCalls); SMETRICS_HIT(++j__DirectHits); \
        SMETRICS_POP(j__SearchPopulation \
                         += BJL(1)BJ1(wr_nPopCnt((_wRoot), (_nBL))))
          #else // SMETRICS_EK
    #define SMETRICS_EK_GUTS(_nBL, _wRoot)  SMETRICS_GETN(++j__GetCallsNot)
          #endif // SMETRICS_EK else

          #ifdef NO_TYPE_IN_XX_SW
    #define EK_WROOT_IS_NULL(_wRoot)  ((_wRoot) == WROOT_NULL)
          #else // NO_TYPE_IN_XX_SW
    #define EK_WROOT_IS_NULL(_wRoot)  WROOT_IS_NULL(T_EMBEDDED_KEYS, (_wRoot))
          #endif // NO_TYPE_IN_XX_SW else

    #define T_EK_X(_nBL, _wRoot, _pwLnX, _wKey) \
        SMETRICS_EK_GUTS((_nBL), (_wRoot)); \
        if (EK_WROOT_IS_NULL((_wRoot))) { goto break_from_main_switch; } \
        HANDLE_DL2((_nBL)); \
        if (EmbeddedListHasKey((_wRoot), (_wKey), (_nBL))) { \
            return BJL(pwLnX)BJ1(KeyFound); \
        } \
        goto break_from_main_switch

          #ifdef _AUG_TYPE_X_EK
t_ek_56:  T_EK_X(56, wRoot, pwLnX, wKey);
t_ek_48:  T_EK_X(48, wRoot, pwLnX, wKey);
t_ek_40:  T_EK_X(40, wRoot, pwLnX, wKey);
t_ek_32:  T_EK_X(32, wRoot, pwLnX, wKey);
t_ek_24:  T_EK_X(24, wRoot, pwLnX, wKey);
t_ek_16:  T_EK_X(16, wRoot, pwLnX, wKey);
t_ek_8:   T_EK_X( 8, wRoot, pwLnX, wKey);
          #endif // _AUG_TYPE_X_EK
          #ifdef _AUG_TYPE_8_EK
t_ek_112: assert(0);
t_ek_96:  T_EK_X(56, wRoot, pwLnX, wKey);
t_ek_80:  T_EK_X(48, wRoot, pwLnX, wKey);
t_ek_64:  T_EK_X(40, wRoot, pwLnX, wKey);
t_ek_48:  T_EK_X(32, wRoot, pwLnX, wKey);
t_ek_32:  T_EK_X(24, wRoot, pwLnX, wKey);
t_ek_16:  T_EK_X(16, wRoot, pwLnX, wKey);
t_ek_0:   T_EK_X( 8, wRoot, pwLnX, wKey);
          #endif // _AUG_TYPE_8_EK

      #else // _AUG_TYPE_EK

t_embedded_keys:
    {
        DBGX(printf("T_EMBEDDED_KEYS %d nBL %d\n", T_EMBEDDED_KEYS, nBL));
      #ifdef SMETRICS_EK
        SMETRICS_GET(++j__GetCalls);
        SMETRICS_HIT(++j__DirectHits);
        SMETRICS_POP(j__SearchPopulation += BJL(1)BJ1(wr_nPopCnt(wRoot, nBL)));
      #else // SMETRICS_EK
        SMETRICS_GETN(++j__GetCallsNot);
      #endif // SMETRICS_EK else
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
                goto break_from_main_switch;
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
          #ifdef B_JUDYL
            if ((wRoot >> (cnBitsPerWord - nBL)) < (wKey & MSK(nBL))) {
                ++wPopCntSum;
            }
          #else // B_JUDYL
            int nPos = SearchEmbeddedX(qya, wKey);
            wPopCntSum += nPos < 0 ? ~nPos : nPos;
          #endif // B_JUDYL else
            DBGC(printf("EK: nPos %d wPopCntSum %zd\n", nPos, wPopCntSum));
            return wPopCntSum;
        }
      #endif // defined(COUNT)

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

      #ifdef LOOKUP
          #ifdef EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
        #define _PARALLEL_EK
          #endif // EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
      #elif defined(NEXT) && !defined(B_JUDYL) // LOOKUP
          #ifdef PARALLEL_EK_FOR_NEXT
        #define _PARALLEL_EK // doesn't work yet
          #endif // PARALLEL_EK_FOR_NEXT
      #elif !defined(NEXT) // LOOKUP elif NEXT && !B_JUDYL
          #ifdef EMBEDDED_KEYS_PARALLEL_FOR_INSERT
        #define _PARALLEL_EK
          #endif // EMBEDDED_KEYS_PARALLEL_FOR_INSERT
      #endif // LOOKUP elif NEXT && !B_JUDYL elif !NEXT

          #ifdef _PARALLEL_EK

#define CASE_BLX(_nBL) \
        case (_nBL): \
            HANDLE_DL2(_nBL); \
            if (EmbeddedListHasKey(wRoot, wKey, (_nBL))) { goto foundIt; } \
            goto break2

        switch (nBL) {
        CASE_BLX( 0); CASE_BLX( 1); CASE_BLX( 2); CASE_BLX( 3); CASE_BLX( 4);
        CASE_BLX( 5); CASE_BLX( 6); CASE_BLX( 7); CASE_BLX( 8); CASE_BLX( 9);
        CASE_BLX(10); CASE_BLX(11); CASE_BLX(12); CASE_BLX(13); CASE_BLX(14);
        CASE_BLX(15); default: DBG(printf("nBL %d\n", nBL)); assert(0);
                      CASE_BLX(16); CASE_BLX(17); CASE_BLX(18); CASE_BLX(19);
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
          #elif 0
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            Word_t wBLM = MSK(nBL);
            Word_t wKeyRoot
                = (wRoot >> (cnBitsPerWord - (nPopCnt * nBL))) & wBLM;
            Word_t wKeyX = wKey & wBLM;
            if (wKeyX > wKeyRoot) goto break_from_main_switch;
            nPos = nPopCnt - 1;
            switch (nPopCnt) {
                      #if (cnBitsPerWord == 64)
            case 8: // max for 7-bit keys and 64 bits;
                wKeyRoot = (wRoot >> (cnBitsPerWord - (7 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 7; goto foundIt; }
            case 7: // max for 8-bit keys and 64 bits;
                wKeyRoot = (wRoot >> (cnBitsPerWord - (6 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 6; goto foundIt; }
            case 6: // max for 9-bit keys and 64 bits;
                wKeyRoot = (wRoot >> (cnBitsPerWord - (5 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 5; goto foundIt; }
            case 5: // max for 10 to 11-bit keys and 64 bits;
                wKeyRoot = (wRoot >> (cnBitsPerWord - (4 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 4; goto foundIt; }
                      #endif // (cnBitsPerWord == 64)
            case 4: // max for 12 to 14-bit keys and 64 bits; 6 for 32
                wKeyRoot = (wRoot >> (cnBitsPerWord - (3 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 3; goto foundIt; }
            case 3: // max for 15 to 19-bit keys and 64 bits; 7-9 for 32
                wKeyRoot = (wRoot >> (cnBitsPerWord - (2 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 2; goto foundIt; }
            case 2: // max for 20 to 29-bit keys and 64 bits; 10-14 for 32
                wKeyRoot = (wRoot >> (cnBitsPerWord - (1 * nBL))) & wBLM;
                if (wKeyX > wKeyRoot) { nPos = 1; goto foundIt; }
            case 1: // max for 30 to 60-bit keys and 64 bits; 15-29 for 32
                nPos = 0; goto foundIt;
            }
          #elif !defined(NEXT) // _PARALLEL_EK
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
          #elif defined(B_JUDYL) // _PARALLEL_EK elif !NEXT
            Word_t wSuffix = wRoot >> (cnBitsPerWord - nBL);
            if ((wKey & NBPW_MSK(nBL)) <= wSuffix) {
                *pwKey = (wKey & ~NBPW_MSK(nBL)) | wSuffix;
                return pwLnX;
            }
          #elif defined(REVERSE_SORT_EMBEDDED_KEYS) \
             && defined(FILL_W_BIG_KEY) && cnBitsPerWord > 32
  #define CASE_BLX(_nBL) \
    case _nBL: \
        nPos = LocateGeKeyInEk64(qya, wKey); \
        nPopCntMax = EmbeddedListPopCntMax(_nBL); \
        if (nPos < wr_nPopCnt(wRoot, _nBL)) { \
            Word_t wBLM = NZ_MSK(_nBL); \
            *pwKey = ((wRoot >> (cnBitsPerWord - (nPopCntMax - nPos) * _nBL)) \
                           & wBLM) \
                       | (wKey & ~wBLM); \
            goto foundIt; \
        } \
        break;

        int nPopCntMax;
  #if 0
        switch (nBL & MSK(cnLogBitsPerWord))
  #else
        switch (nBL)
  #endif
        {
  #if cnBitsInD1 < 8 || defined(USE_XX_SW)
                      CASE_BLX( 1); CASE_BLX( 2); CASE_BLX( 3); CASE_BLX( 4);
        CASE_BLX( 5); CASE_BLX( 6); CASE_BLX( 7);
  #endif // cnBitsInD1 < 8 || USE_XX_SW
                                                  CASE_BLX( 8); CASE_BLX( 9);
        CASE_BLX(10); CASE_BLX(11); CASE_BLX(12); CASE_BLX(13); CASE_BLX(14);
        CASE_BLX(15); //default:
                      CASE_BLX(16); CASE_BLX(17); CASE_BLX(18); CASE_BLX(19);
        CASE_BLX(20); CASE_BLX(21); CASE_BLX(22); CASE_BLX(23); CASE_BLX(24);
        CASE_BLX(25); CASE_BLX(26); CASE_BLX(27); CASE_BLX(28); CASE_BLX(29);
        CASE_BLX(30); CASE_BLX(31); CASE_BLX(32); CASE_BLX(33); CASE_BLX(34);
        CASE_BLX(35); CASE_BLX(36); CASE_BLX(37); CASE_BLX(38); CASE_BLX(39);
        CASE_BLX(40); CASE_BLX(41); CASE_BLX(42); CASE_BLX(43); CASE_BLX(44);
        CASE_BLX(45); CASE_BLX(46); CASE_BLX(47); CASE_BLX(48); CASE_BLX(49);
        CASE_BLX(50); CASE_BLX(51); CASE_BLX(52); CASE_BLX(53); CASE_BLX(54);
        // Some of these cases may not be necessary depending on ifdefs, e.g.
        // do we have an explicity pop cnt for T_EMBEDDED_KEYS.
        CASE_BLX(55); CASE_BLX(56); CASE_BLX(57); CASE_BLX(58); CASE_BLX(59);
        CASE_BLX(60); CASE_BLX(61); CASE_BLX(62); CASE_BLX(63); CASE_BLX(64);
        }
          #else // _PARALLEL_EK elif !NEXT elif B_JUDYL
              #if 1
                  #ifdef REVERSE_SORT_EMBEDDED_KEYS
            int nPopCntMax = EmbeddedListPopCntMax(nBL);
                  #endif // REVERSE_SORT_EMBEDDED_KEYS
            if ((nPos = SearchEmbeddedX(qya, wKey)) >= 0) {
                Word_t wBLM = MSK(nBL);
                *pwKey = ((wRoot >> (cnBitsPerWord - nBL *
                  #ifdef REVERSE_SORT_EMBEDDED_KEYS
                                   (nPopCntMax - nPos)
                  #else // REVERSE_SORT_EMBEDDED_KEYS
                                   (nPos + 1)
                  #endif // REVERSE_SORT_EMBEDDED_KEYS else
                                     ))
                               & wBLM)
                           | (wKey & ~wBLM);
                goto foundIt;
            }
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            if (~nPos < nPopCnt) {
                Word_t wBLM = MSK(nBL);
                *pwKey = ((wRoot >> (cnBitsPerWord - nBL *
                  #ifdef REVERSE_SORT_EMBEDDED_KEYS
                                   (nPopCntMax - ~nPos)
                  #else // REVERSE_SORT_EMBEDDED_KEYS
                                   (~nPos + 1)
                  #endif // REVERSE_SORT_EMBEDDED_KEYS else
                                                            ))
                               & wBLM)
                           | (wKey & ~wBLM);
                goto foundIt;
            }
              #else // 1
    #define CASE_BLX(_nBL) \
        case _nBL: \
        if ((nPos = SearchEmbeddedX(qya, wKey)) < 0) { \
            nPopCnt = wr_nPopCnt(wRoot, _nBL); \
            if (~nPos < nPopCnt) { \
                Word_t wBLM = MSK(_nBL); \
                *pwKey = ((wRoot >> (cnBitsPerWord - (~nPos+1) * _nBL)) \
                               & wBLM) \
                           | (wKey & ~wBLM); \
                goto foundIt; \
            } \
            Word_t wIncr = EXP(_nBL); \
            wKey &= ~(wIncr - 1); \
            wKey += wIncr; \
            if ((wKey & NZ_MSK(_nBL + 8)) != 0) { \
                pwRoot = &((++pLn)->ln_wRoot); \
                if (wr_nType(wRoot = *pwRoot) == T_EMBEDDED_KEYS) { \
                    goto t_ek_next; \
                } \
                goto restartUp; \
            } \
            if (wKey != 0) goto restart; \
        } \
        { \
            Word_t wBLM = MSK(_nBL); \
            *pwKey = ((wRoot >> (cnBitsPerWord - (nPos+1) * _nBL)) & wBLM) \
                       | (wKey & ~wBLM); \
            goto foundIt; \
        }

        int nPopCnt;
        goto t_ek_next;
t_ek_next:
  #if 0
        switch (nBL & MSK(cnLogBitsPerWord))
  #else
        switch (nBL)
  #endif
        {
                                                  CASE_BLX( 8); CASE_BLX( 9);
        CASE_BLX(10); CASE_BLX(11); CASE_BLX(12); CASE_BLX(13); CASE_BLX(14);
        CASE_BLX(15); //default:
                      CASE_BLX(16); CASE_BLX(17); CASE_BLX(18); CASE_BLX(19);
        CASE_BLX(20); CASE_BLX(21); CASE_BLX(22); CASE_BLX(23); CASE_BLX(24);
        CASE_BLX(25); CASE_BLX(26); CASE_BLX(27); CASE_BLX(28); CASE_BLX(29);
        CASE_BLX(30); CASE_BLX(31); CASE_BLX(32); CASE_BLX(33); CASE_BLX(34);
        CASE_BLX(35); CASE_BLX(36); CASE_BLX(37); CASE_BLX(38); CASE_BLX(39);
        CASE_BLX(40); CASE_BLX(41); CASE_BLX(42); CASE_BLX(43); CASE_BLX(44);
        CASE_BLX(45); CASE_BLX(46); CASE_BLX(47); CASE_BLX(48); CASE_BLX(49);
        CASE_BLX(50); CASE_BLX(51); CASE_BLX(52); CASE_BLX(53); CASE_BLX(54);
        CASE_BLX(55); CASE_BLX(56);
        }
              #endif // 1 else
          #endif // _PARALLEL_EK elif !NEXT elif B_JUDYL else
        }
        goto break_from_main_switch;

          #if !defined(NEXT) || !defined(B_JUDYL)
foundIt:
              #ifdef REMOVE
        goto removeGutsAndCleanup;
              #endif // REMOVE
              #ifdef INSERT
                  #ifndef RECURSIVE
        if (nIncr > 0) { goto undo; } // undo counting
                  #endif // !RECURSIVE
              #endif // INSERT
          #endif // !NEXT || !B_JUDYL

      #if !defined(NEXT) || !defined(B_JUDYL)
          #if defined(B_JUDYL) && (defined(INSERT) || defined(LOOKUP))
        return pwLnX;
          #else // B_JUDYL && (INSERT || LOOKUP)
        return KeyFound;
          #endif // B_JUDYL && (INSERT || LOOKUP)
      #endif // !NEXT || !B_JUDYL
    } // end of t_embedded_keys

      #endif // _AUG_TYPE_EK else
  #endif // EMBED_KEYS

  #ifdef EK_XV

      #ifdef _AUG_TYPE_X
t_ek_xv_56:
t_ek_xv_48:
t_ek_xv_40:
t_ek_xv_32:
t_ek_xv_24:
t_ek_xv_16:
t_ek_xv_8:
      #endif // _AUG_TYPE_X
      #if defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
t_ek_xv_plus_112: assert(nBL == 64); goto t_ek_xv;
t_ek_xv_plus_96: assert(nBL == 56); goto t_ek_xv;
t_ek_xv_plus_80: assert(nBL == 48); goto t_ek_xv;
t_ek_xv_plus_64: assert(nBL == 40); goto t_ek_xv;
t_ek_xv_plus_48: assert(nBL == 32); goto t_ek_xv;
t_ek_xv_plus_32: assert(nBL == 24); goto t_ek_xv;
t_ek_xv_plus_16: assert(nBL == 16);
    {
        DBGX(printf("t_ek_xv_plus_16\n"));
        nBL = 16;
        assert(pwLnX != NULL);
        {
            if ((nPos = SearchEmbeddedX(qya, wKey)) >= 0) {
                goto xv2foundIt;
            }
            assert(wRoot != WROOT_NULL);
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            if (~nPos < nPopCnt) {
                int nBits = 1 << (LOG(nBL - 1) + 1);
              #if (cnBitsInD1 < cnLogBitsPerByte)
                if (nBits < 8) { nBits = 8; }
              #endif // (cnBitsInD1 < cnLogBitsPerByte)
                *pwKey = ((*pwLnX >> (~nPos * nBits)) & MSK(nBits))
                           | (wKey & ~MSK(nBits));
                return &pwr[~nPos];
            }
        }
        goto break_from_main_switch;
xv2foundIt:;
        *pwKey = wKey;
        return &pwr[nPos];
    } // end of t_ek_xv_plus_16
t_ek_xv_plus_0: assert(nBL == 8);
    {
        DBGX(printf("t_ek_xv_plus_0\n"));
        if ((nPos = LocateGeKeyInWord8(*pwLnX, wKey)) >= 0) {
            *pwKey = (wKey & ~MSK(nBL)) | ((uint8_t*)pwLnX)[nPos];
            return &pwr[nPos];
        }
        goto break_from_main_switch;
    } // end of t_ek_xv_plus_0
      #endif // defined(AUG_TYPE_8_NEXT_EK_XV) && defined(NEXT)
    goto t_ek_xv;
t_ek_xv:
    {
        DBGX(printf("T_EK_XV\n"));
      #ifdef SMETRICS_EK
        SMETRICS_GET(++j__GetCalls);
        SMETRICS_HIT(++j__DirectHits);
      #else // SMETRICS_EK
        SMETRICS_GETN(++j__GetCallsNot);
      #endif // SMETRICS_EK else
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
        //#error not coded yet
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
      #if defined(_PARALLEL_EK) && !defined(NEXT)

  #ifdef PF_EK_XV_2
#define PF_2(_nBL) \
            if (EmbeddedListPopCntMax(_nBL) > 2) { \
                PREFETCH(pwr + 8); \
            }
  #else // PF_EK_XV_2
#define PF_2(pwr)
  #endif // #else PF_EK_XV_2

  #ifdef SMETRICS_EK
#define SMETRICS_EK_POP(x)  SMETRICS_POP(x)
  #else // SMETRICS_EK
#define SMETRICS_EK_POP(x)
  #endif // SMETRICS_EK else

  #if (cnBitsInD1 < cnLogBitsPerByte)
#define XV_BLX(_nBL) \
        case (_nBL): \
            PF_2(_nBL); \
    SMETRICS_EK_POP(j__SearchPopulation \
                        += GetBits(wRoot, cnBitsCnt, cnLsbCnt)); \
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
    SMETRICS_EK_POP(j__SearchPopulation \
                        += GetBits(wRoot, cnBitsCnt, cnLsbCnt)); \
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
            XV_BLX(15); default: DBG(printf("nBL %d\n", nBL)); assert(0);
                        XV_BLX(16); XV_BLX(17); XV_BLX(18); XV_BLX(19);
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

      #else // _PARALLEL_EK && !NEXT
          #ifdef NEXT
            if (nBL == 8) {
                if ((nPos = LocateGeKeyInWord8(*pwLnX, wKey)) >= 0) {
                    *pwKey = (wKey & ~MSK(nBL)) | ((uint8_t*)pwLnX)[nPos];
                    return &pwr[nPos];
                }
                goto break_from_main_switch;
            }
          #endif // NEXT
            if ((nPos = SearchEmbeddedX(qya, wKey)) >= 0) {
                goto xv_foundIt;
            }
          #ifdef NEXT
            assert(wRoot != WROOT_NULL);
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            if (~nPos < nPopCnt) {
                int nBits = 1 << (LOG(nBL - 1) + 1);
              #if (cnBitsInD1 < cnLogBitsPerByte)
                if (nBits < 8) { nBits = 8; }
              #endif // (cnBitsInD1 < cnLogBitsPerByte)
                *pwKey = ((*pwLnX >> (~nPos * nBits)) & MSK(nBits))
                           | (wKey & ~MSK(nBits));
                return &pwr[~nPos];
            }
          #endif // NEXT
      #endif // _PARALLEL_EK && !NEXT else
        }

        goto break_from_main_switch;

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
      #elif defined(NEXT)
        *pwKey = wKey;
        return &pwr[nPos];
      #else // INSERT || LOOKUP elif NEXT
        return KeyFound;
      #endif // INSERT || LOOKUP elif NEXT else
    } // end of t_ek_xv
  #endif // EK_XV

  #if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)
t_separate_t_null:
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
            swPopCnt(qya, nBL, gwPopCnt(qya, nBL) + nIncr);
        }
      #endif // defined(INSERT) || defined(REMOVE)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

        goto break_from_main_switch;

    } // end of t_separate_t_null
  #endif // SEPARATE_T_NULL || (cwListPopCntMax == 0)

break_from_main_switch:;
    // Key is not present.
    DBGX(Checkpoint(qya, "break_from_main_switch"));
  #ifdef NEXT
    if (nBL < cnBitsPerWord) {
        goto tryNextDigit;
tryNextDigit:;
        DBGX(Checkpoint(qya, "tryNextDigit"));
        assert(nBL < cnBitsPerWord);
        Word_t wIncr = EXP(nBL);
        wKey &= ~(wIncr - 1);
        wKey += wIncr;
        DBGX(printf("# next digit wKey " OWx"\n", wKey));
        if ((nBLUp & MSK(cnLogBitsPerWord)) != 0) {
            if ((wKey & MSK(nBLRUp)) != 0) {
      #ifdef _NEXT_SHORTCUT // skip over WROOT_NULLs and/or bypass switch(Up)
                BJL(assert(cnBitsInD1 > cnLogBitsPerWord)); // No thought yet.
                BJ1(assert(cnBitsInD1 > cnLogBitsPerLink)); // No thought yet.
                // A compressed switch may have no link for incremented wKey.
                // Make sure we aren't in a compressed switch.
                //if (nTypeUp >= T_SWITCH)
                if (wr_nType(pLnUp->ln_wRoot) >= T_SWITCH)
                {
                    wRoot = (++pLn)->ln_wRoot;
          #ifdef NEXT_SHORTCUT_SWITCH
          #ifdef REMOTE_LNX
                    ++pwLnX;
          #endif // REMOTE_LNX
          #endif // NEXT_SHORTCUT_SWITCH
          #ifdef NEXT_SHORTCUT_NULL
                    while (wRoot == WROOT_NULL) {
                        wKey += wIncr;
                        if ((wKey & MSK(nBLRUp)) == 0) {
                            if (wKey == 0) {
                                return BJL(NULL)BJ1(Failure);
                            }
                            goto restart;
                        }
                        wRoot = (++pLn)->ln_wRoot;
              #ifdef NEXT_SHORTCUT_SWITCH
              #ifdef REMOTE_LNX
                        ++pwLnX;
              #endif // REMOTE_LNX
              #endif // NEXT_SHORTCUT_SWITCH
                    }
          #endif // NEXT_SHORTCUT_NULL
          #ifdef NEXT_SHORTCUT_SWITCH
                    pwRoot = &pLn->ln_wRoot;
              #ifdef _LNX
              #ifndef REMOTE_LNX
                    pwLnX = &pLn->ln_wX;
              #endif // !REMOTE_LNX
              #endif // _LNX
                    goto insertAgain;
          #endif // NEXT_SHORTCUT_SWITCH
                } else {
                   // T_BM_SW (T_SKIP_TO_BM_SW) would come here.
                }
      #endif // _NEXT_SHORTCUT
                goto restartUp; // restart at (nBLUp, pLnUp) with this new wKey
            }
        }
        if (wKey != 0) {
            goto restart; // restart at top with this new wKey
        }
    }
    return BJL(NULL)BJ1(Failure);
  #endif // NEXT
  #ifdef COUNT
    DBGC(printf("done wPopCntSum " OWx"\n", wPopCntSum));
    return wPopCntSum;
  #endif // COUNT
  #ifdef INSERT
      #ifdef BM_IN_LINK
    // If InsertGuts calls Insert, then it is always with the same
    // pLn and nBL that Insert passed to InsertGuts.
          #ifndef RECURSIVE
    assert((nBL != cnBitsPerWord) || (pLn == pLnOrig));
          #endif // !RECURSIVE
      #endif // BM_IN_LINK
    // InsertGuts is called with a pLn and nBL indicates the
    // bits that were not decoded in identifying pLn.  nBL
    // does not include any skip indicated in the type field of *pLn.
      #ifdef _LNX
    assert((pwLnX != NULL) || (nBL == cnBitsPerWord));
      #endif // _LNX
      #ifdef REMOTE_LNX
    assert((nBLUp < cnBitsPerWord) || (pwLnXUp == NULL));
      #endif // REMOTE_LNX
      #ifdef CODE_XX_SW
    if (nBL >= cnBitsPerWord) {
        assert(nBLUp == 0);
        assert(pLnUp == NULL);
          #ifdef _LNX
        assert(pwLnXUp == NULL);
          #endif // _LNX
    } else {
          #ifdef _LNX
          #if 0
        // This assertion doesn't work for Insert not-at-top.
        if ((nBLUp >= cnBitsPerWord) != (pwLnXUp == NULL)) {
            printf("\n# nBLUp %d pwLnXUp %p\n", nBLUp, pwLnXUp);
        }
        assert((nBLUp >= cnBitsPerWord) == (pwLnXUp == NULL));
          #endif
          #endif // _LNX
    }
      #endif // CODE_XX_SW
    BJL(pwValue =)
    BJ1(Status_t status =)
        InsertGuts(qya, wKey, nPos
      #if defined(CODE_XX_SW)
                 , pLnUp, nBLUp
          #ifdef REMOTE_LNX
                 , pwLnXUp
          #endif // REMOTE_LNX
      #endif // defined(CODE_XX_SW)
                   );
    BJ1((void)status);
      #ifndef _RETURN_NULL_TO_INSERT_AGAIN
    BJL(assert(pwValue != NULL));
    BJL(assert(((Word_t)pwValue & (sizeof(Word_t) - 1)) == 0));
    BJ1(assert(status == Success));
      #endif // _RETURN_NULL_TO_INSERT_AGAIN
      #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    if (BJL(pwValue == NULL) BJ1(status == 0)) {
            #ifdef _LNX
        assert((pwLnX != NULL) || (nBL == cnBitsPerWord));
            #endif // _LNX
// How do we know if InsertGuts wants us to reinsert at
// (nBL, pLn/pwRoot) or at its parent? Or higher?
// Because it has modified the parent. Or higher.

// I wonder if we could just return -1, -2, ...
// I would have said 1, 2, ... but Success == 1 for Judy1.
// There are a lot of possible return values that are not valid pointers
// to JudyL values.

// I wonder if we could undo and then reinsert at the top?
// Do we have a valid enough tree to do the undo?
// Would the undo leave the place holder in place?

// I wonder if InsertGuts could call Remove (which would fail, but would
// undo the counts) before calling Insert?
// Could it initiate Remove in the middle of the tree?
// Then do Insert from the same spot?
// If InsertGuts calls Remove, then Remove will fail and all the count
// decrementing it does will be undone in an undo phase.
// No good.

// How about having InsertGuts pass information back using pLn
// by not freeing pLn and having Insert free it?

// How about having InsertGuts call Insert using (nBLUp, pLnUp)?

        // How do we make sure we're not reinserting into a switch or leaf
        // which has already counted the insert before InsertGuts was called?
        assert(pLn->ln_wRoot != wRoot);
        wRoot = pLn->ln_wRoot;
        nBLR = nBL;
        DBGX(Checkpoint(qya, "goto insertAgain"));
        goto insertAgain;
    } else if (BJL(pwValue == (Word_t*)-1) BJ1((int)status == -1))
      #endif // _RETURN_NULL_TO_INSERT_AGAIN
  #endif // INSERT
  #if defined(INSERT) && defined(_RETURN_NULL_TO_INSERT_AGAIN) || defined(NEXT)
    {
        goto restartUp;
restartUp:;
        DBGX(Checkpoint(qya, "restartUp"));
        DBGX(printf("nBLUp %d pLnUp %p\n", nBLUp, pLnUp));
        nBL = nBLUp;
        pLn = pLnUp;
      #ifndef QP_PLN
        pwRoot = &pLnUp->ln_wRoot;
      #endif // !QP_PLN
      #ifdef _LNX
        pwLnX = pwLnXUp;
      #endif // LNX
        wRoot = pLnUp->ln_wRoot;
        nBLR = nBL;
        // How do we reset nBLUp, pLnUp, pLnXUp?
        nBLUp = 0;
        pLnUp = NULL;
      #ifdef _LNX
        pwLnXUp = NULL;
      #endif // LNX
        goto insertAgain;
    }
  #endif // INSERT && _RETURN_NULL_TO_INSERT_AGAIN || NEXT
  #ifdef INSERT
      #ifdef B_JUDYL
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
    DBGX(Checkpoint(qya, "undo"));
  #endif // INSERT
#if defined(REMOVE)
  #if !defined(RECURSIVE)
    if (nIncr < 0)
  #endif // !defined(RECURSIVE)
#endif // defined(REMOVE)
    {
  #if defined(INSERT) || defined(REMOVE)
      #if !defined(RECURSIVE)
        // Undo the counting we did on the way in.
        nIncr *= -1;
      #endif // !defined(RECURSIVE)
  #endif // defined(INSERT) || defined(REMOVE)
    goto restart;
restart:;
        DBGX(Checkpoint(qya, "restart"));
  #if defined(INSERT) || defined(REMOVE) || defined(NEXT)
      #if !defined(RECURSIVE)
        nBL = nBLOrig;
        pLn = pLnOrig; // should we set pLnUp = NULL
          #ifndef QP_PLN
        pwRoot = &pLnOrig->ln_wRoot;
          #endif // !QP_PLN
          #ifdef _LNX
        pwLnX = pwLnXOrig;
          #endif // _LNX
        wRoot = pLn->ln_wRoot;
        goto top;
      #endif // !defined(RECURSIVE)
  #endif // INSERT || REMOVE || NEXT
    }
  #if (defined(LOOKUP) || defined(INSERT) || defined(NEXT)) && defined(B_JUDYL)
    return NULL;
  #else // (LOOKUP || INSERT || NEXT) && B_JUDYL
    return Failure;
  #endif // (LOOKUP || INSERT || NEXT) && B_JUDYL else
  #if defined(INSERT) || defined(REMOVE)
      #if defined(REMOVE)
removeGutsAndCleanup:;
    DBGX(Checkpoint(qya, "removeGutsAndCleanup"));
    RemoveGuts(qya, wKey);
      #endif // defined(REMOVE)
    goto cleanup;
cleanup:;
    DBGX(Checkpoint(qya, "cleanup"));
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

#ifdef B_JUDYL
#define Insert  InsertL
#define Remove  RemoveL
#else // B_JUDYL
#define Insert  Insert1
#define Remove  Remove1
#endif // B_JUDYL

#endif // defined(NEW_NEXT) || !defined(NEXT)

// Just to make sure I don't use these below.
#undef RECURSIVE
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

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

  #ifdef B_JUDYL
    DBGL(printf("\n\n# JudyLGet pcvRoot aka wRoot %p wKey " OWx"\n",
                (void*)pcvRoot, wKey));
  #else // B_JUDYL
    DBGL(printf("\n\n# Judy1Test pcvRoot aka wRoot %p wKey " OWx"\n",
                (void*)pcvRoot, wKey));
  #endif // B_JUDYL else

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
        SMETRICS_GET(++j__GetCalls);
      #ifdef REMOTE_LNX
        Word_t* pwLnX = NULL;
      #endif // REMOTE_LNX
      #if defined(B_JUDYL)
        int nPos = SearchListWord(qya, nBL, wKey);
        return (nPos >= 0) ? (PPvoid_t)&gpwValues(qy)[~nPos] : NULL;
      #else // defined(B_JUDYL)
        return ListHasKeyWord(qya, nBL, wKey);
      #endif // #else defined(B_JUDYL)
    }
  #endif // defined(SEARCH_FROM_WRAPPER)
  #endif // (cwListPopCntMax != 0)

    return BJL((PPvoid_t)) Lookup(wRoot, wKey);

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
    DBGI(printf("\n\n# JudyLIns ppvRoot %p wKey " OWx"\n",
                (void*)ppvRoot, wKey));
  #else // B_JUDYL
    int status;
    DBGI(printf("\n\n# Judy1Set ppvRoot %p wKey " OWx"\n",
                (void*)ppvRoot, wKey));
  #endif // B_JUDYL else

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
        int nPos = ~SearchListWord(qya, nBLR, wKey);
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
  #ifdef REMOTE_LNX
        Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
        BJL(pwValue) BJ1(status) = Insert(qya, wKey);
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

  #ifdef B_JUDYL
    DBGI(printf("\n# After InsertL(wKey " OWx") Dump\n", wKey));
  #else // B_JUDYL
    DBGI(printf("\n# After Insert1(wKey " OWx") Dump\n", wKey));
  #endif // B_JUDYL else
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

    int nBL = cnBitsPerWord;
    Link_t *pLn = STRUCT_OF(ppvRoot, Link_t, ln_wRoot);
    Word_t* pwRoot = &pLn->ln_wRoot; (void)pwRoot;

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
            Word_t wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
            Word_t *pwListNew;
            if (wPopCnt != 1) {
                pwListNew = NewList(wPopCnt - 1, cnDigitsPerWord);
                Word_t *pwKeysNew;
                set_wr(wRoot, pwListNew, T_LIST);
                pwKeysNew = ls_pwKeys(pwListNew, nBL);

                Word_t *pwKeys = ls_pwKeys(pwr, nBL);

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
            OldList(pwr, wPopCnt, nBL, nType);
            *pwRoot = wRoot;
            status = Success;
        }
    } else
  #endif // (cwListPopCntMax != 0) && defined(SEARCH_FROM_WRAPPER_R)
    {
  #ifdef REMOTE_LNX
        Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
        status = Remove(qya, wKey);
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

