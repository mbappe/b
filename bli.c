// This file is #included in multiple .c files.
// With exactly one of LOOKUP, INSERT, REMOVE, COUNT and NEXT defined.
// It is not compiled on its own, i.e. without a wrapper.

#if defined(NEW_NEXT) || !defined(NEXT)

#if defined(__amd64__) || defined(i386)
//#include <emmintrin.h>
//#include <smmintrin.h>
#include <immintrin.h> // __m128i
#endif // __amd64__ || i386

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
        if (!cbEmbeddedBitmap || (nBLLoop > cnLogBitsPerLink)) {
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
        if ((wIndex > (unsigned)nLinks / 2)) {
            ww = wIndex; wwLimit = nLinks;
        } else {
            ww = 0; wwLimit = wIndex;
        }
        DBGC(printf("ww " OWx" wwLimit " OWx"\n", ww, wwLimit));
        wPopCnt = CountSwLoop(qya, ww, wwLimit - ww);
//printf("# CountSwLoop returned %zd\n", wPopCnt);
        assert(((int)wIndex < nLinks));
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
      #if !defined(_CONSTANT_NBPD) || cnBitsPerDigit < nLogSwSubCnts(1)
            int nShift
                = (nBW > nLogSwSubCnts(1)) ? (nBW - nLogSwSubCnts(1)) : 0;
            // Would like to resolve this test at compile time if possible.
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)[ii];
                }
            } else
      #else // !_CONSTANT_NBPD || cnBitsPerDigit < nLogSwSubCnts(1)
            assert(nBW >= nLogSwSubCnts(1));
            int nShift = nBW - nLogSwSubCnts(1);
      #endif // !_CONSTANT_NBPD || cnBitsPerDigit < nLogSwSubCnts(1)
            {
                int nCntNum
                    = ((wIndex << nLogSwSubCnts(1)) + (1 << (nBW - 1))) >> nBW;
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
      #if cnBitsPerWord > 32
        } else if (nBLR <= 32) {
          #if !defined(_CONSTANT_NBPD) || cnBitsPerDigit < cnLogSwCnts + 1
            int nShift
                = (nBW > nLogSwSubCnts(2)) ? (nBW - nLogSwSubCnts(2)) : 0;
            // Would like to resolve this test at compile time if possible.
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((uint32_t*)((Switch_t*)pwr)->sw_awCnts)[ii];
                }
            } else
          #else // !_CONSTANT_NBPD || cnBitsPerDigit < cnLogSwCnts + 1
            assert(nBW >= nLogSwSubCnts(2));
            int nShift = nBW - nLogSwSubCnts(2);
          #endif // !_CONSTANT_NBPD || cnBitsPerDigit < cnLogSwCnts + 1 else
            {
                int nCntNum
                    = ((wIndex << nLogSwSubCnts(2)) + (1 << (nBW - 1))) >> nBW;
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
      #endif // cnBitsPerWord > 32
        } else {
            // One subexpanse count per word.
      #if !defined(_CONSTANT_NBPD) || cnBitsPerDigit < cnLogSwCnts
            // Would like to resolve this test at compile time if possible.
            int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
            if (nShift == 0) {
                for (int ii = 0; ii < (int)wIndex; ++ii) {
                    wPopCnt += ((Switch_t*)pwr)->sw_awCnts[ii];
                }
            } else
      #else // !_CONSTANT_NBPD || cnBitsPerDigit < cnLogSwCnts
            assert(nBW >= cnLogSwCnts);
            int nShift = nBW - cnLogSwCnts;
      #endif // !_CONSTANT_NBPD || cnBitsPerDigit < cnLogSwCnts else
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
  #endif // cnSwCnts != 0
    DBGC(printf("\nCountSw wPopCnt %" _fw"u\n", wPopCnt));
    return wPopCnt;
}
  #endif // defined(COUNT)

#if defined(SKIP_LINKS)

static Word_t
PrefixMismatch(qp,
               Word_t wKey,
#if defined(CODE_BM_SW)
               int bBmSw,
#endif // defined(CODE_BM_SW)
               int *pnBLR)
{
    qv; (void)wKey; (void)pnBLR;
    DBGX(printf("PM: nBL %d pLn %p wRoot " OWx" nType %d pwr %p\n", qyp));
  #if defined(CODE_BM_SW)
    (void)bBmSw;
  #endif // defined(CODE_BM_SW)

    Word_t wPrefixMismatch; (void)wPrefixMismatch;
    int nBLR = /*(nType == T_SKIP_TO_BM_SW)
                 ? ((Switch_t*)pwr)->sw_nBLR :*/ gnBLRSkip(qy);
    assert(nBLR < nBL); // reserved
    *pnBLR = nBLR;

    Word_t wPrefix =
        0 ? 0
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
    return wPrefixMismatch;
}

#endif // defined(SKIP_LINKS)

#if defined(CODE_BM_SW)
    #define IS_BM_SW_ARG(_nType)  tp_bIsBmSw(_nType),
#else // defined(CODE_BM_SW)
    #define IS_BM_SW_ARG(_nType)
#endif // defined(CODE_BM_SW)

// PREFIX_MISMATCH updates nBLR.
// PrefixMismatch requires a real pwRoot (as opposed to &wRoot) when it
// may need to save that value for later dereference by Lookup at the leaf.
#define PREFIX_MISMATCH(qp) \
    ( assert(tp_bIsSkip(nType)), \
      PrefixMismatch(qy, wKey, \
                     IS_BM_SW_ARG(nType) \
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

#ifdef _LNX
    #define swapynew  &pLn, pLnNew, pwLnXNew
#else // _LNX
    #define swapynew  &pLn, pLnNew
#endif // _LNX else

static inline void
SwAdvance(pqpa,
          Link_t** ppLn,
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
    *ppwRoot = &pLnNew->ln_wRoot;
    *pwRoot = **ppwRoot;
    DBGX(printf("# SwAdvance: sw nBL %d pwRoot %p wRoot 0x%zx\n",
                *pnBL, *ppwRoot, *pwRoot));
    // Why aren't we updating *pnType and *ppwr here?
}

#ifdef LOOKUP
#ifdef _AUG_TYPE_X_LOOKUP
    #define _AUG_TYPE_X
#endif // _AUG_TYPE_X_LOOKUP
#endif // LOOKUP

#if defined(AUGMENT_TYPE_8) && defined(LOOKUP)
    #define _AUG_TYPE_8_SW
#elif defined(_AUG_TYPE_8_NEXT_EMPTY)
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

#define WROOT_IS_NULL(_nType, _wRoot) \
      ((wr_nType(WROOT_NULL) == (_nType)) && ((_wRoot) == WROOT_NULL))

#ifndef LOOKUP
  #define _USE_SEARCH_LIST
#elif defined(B_JUDYL) // !LOOKUP
  #ifdef HASKEY_FOR_JUDYL_LOOKUP
    #define _USE_HAS_KEY
  #elif defined(SEARCH_FOR_JUDYL_LOOKUP)
    #define _USE_SEARCH_LIST
  #else // HASKEY_FOR_JUDYL_LOOKUP elif SEARCH_FOR_JUDYL_LOOKUP
    #define _USE_LOCATE_KEY
  #endif // HASKEY_FOR_JUDYL_LOOKUP elif SEARCH_FOR_JUDYL_LOOKUP else
#else // !LOOKUP elif B_JUDYL
  #ifdef SEARCH_FOR_JUDY1_LOOKUP
    #define _USE_SEARCH_LIST
  #elif defined(LOCATEKEY_FOR_JUDY1_LOOKUP)
    #define _USE_LOCATE_KEY
  #else // SEARCH_FOR_JUDY1_LOOKUP elif LOCATEKEY_FOR_JUDY1_LOOKUP
    #define _USE_HAS_KEY
  #endif // SEARCH_FOR_JUDY1_LOOKUP elif LOCATEKEY_FOR_JUDY1_LOOKUP else
#endif // !LOOKUP elif B_JUDYL else

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

#define LIST_HAS_KEY(_suffix, qya, _nBLR, _wKey) \
    ListHasKey##_suffix(qya, nBLR, wKey)

IF_LOOKUP(static inline BJL(Word_t*)BJ1(Status_t) Lookup(Word_t wRootArg,
                                                         Word_t wKey))
#ifdef NEXT_QPA
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(qpa, Word_t* pwKey))
#elif defined(NEXT_QP) // NEXT_QPA
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(qp, Word_t* pwKey))
#else // NEXT_QPA elif NEXT_QP
IF_NEXT(static inline BJL(Word_t*)BJ1(Status_t) Next(Word_t wRootArg,
                                                     Word_t* pwKey))
#endif // NEXT_QPA elif NEXT_QP else
IF_NEXT_EMPTY(static inline Status_t NextEmpty(Word_t wRootArg, Word_t* pwKey))
IF_INSERT(       BJL(Word_t*)BJ1(Status_t) Insert(qpa, Word_t wKey))
IF_REMOVE(                       Status_t  Remove(qpa, Word_t wKey))
#ifdef COUNT_2
IF_COUNT(static inline Word_t Count(qpa, Word_t wKey0, Word_t wKey))
#else // COUNT_2
IF_COUNT(static inline Word_t Count(qpa, Word_t wKey))
#endif // COUNT_2 else
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
    IF_NOT_LOOKUP(IF_NOT_NEXT(IF_NOT_NEXT_EMPTY(qva)));
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
  #if defined(LOOKUP) || defined(_NEXT_WROOT) || defined(NEXT_EMPTY)
    int nBL = cnBitsPerWord;
    // Only wRoot word of pLn at top.
    Link_t* pLn = STRUCT_OF(&wRootArg, Link_t, ln_wRoot); (void)pLn;
    Word_t* pwRoot = &wRootArg; (void)pwRoot;
      #ifdef _LNX
    Word_t* pwLnX = NULL; (void)pwLnX;
      #endif // _LNX
    Word_t wRoot = wRootArg; (void)wRoot;
    // nBL, pLn and wRoot of qy are set up
    int nType;
    DBGX(nType = -1); // for compiler for qv in Checkpoint
    Word_t *pwr;
    DBGX(pwr = NULL); // for compiler for qv in Checkpoint
  #endif // LOOKUP || _NEXT_WROOT elif NEXT_EMPTY else
  #if defined(NEXT) || defined(NEXT_EMPTY)
    Word_t wKey = *pwKey;
  #endif // NEXT || NEXT_EMPTY
  #ifdef NEXT
      #ifdef NEW_NEXT_IS_EXCLUSIVE
    if (++wKey == 0) {
        return 0;
    }
      #endif // NEW_NEXT_IS_EXCLUSIVE
  #endif // NEXT
  #ifndef LOOKUP
  #ifndef COUNT
    int nBLOrig = nBL; (void)nBLOrig;
    Link_t *pLnOrig = pLn; (void)pLnOrig;
  #endif // !COUNT
  #endif // !LOOKUP
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
      #ifdef INSERT
    nIncr = 1;
      #elif defined(REMOVE)
    nIncr = -1;
      #else // INSERT elif REMOVE
    nIncr = 0; // make gcc happy
      #endif // INSERT elif REMOVE else
    // nBLUp was used only for CODE_XX_SW and INSERT.
    // I think it will eventually be used for REMOVE.
    int nBLUp = 0; (void)nBLUp; // silence gcc
    Link_t *pLnUp = NULL; (void)pLnUp;
  #if defined(NEXT) && defined(RESTART_UP_FOR_NEXT)
    #define _RESTART_UP_FOR_NEXT_OR_EMPTY
  #elif defined(NEXT_EMPTY) && defined(RESTART_UP_FOR_NEXT_EMPTY)
    #define _RESTART_UP_FOR_NEXT_OR_EMPTY
  #endif // NEXT && RESTART_UP_FOR_NEXT elif NEXT_EMPTY && RESTART_UP_... else
  #ifdef _RESTART_UP_FOR_NEXT_OR_EMPTY
      #define _RESTART_UP
  #elif defined(INSERT) && defined(_RETURN_NULL_TO_INSERT_AGAIN)
      #define _RESTART_UP
  #endif // _RESTART_UP_FOR_NEXT_OR_EMPTY elif ...
  #if defined(NEXT) && defined(_NEXT_SHORTCUT)
    //int nTypeUp = -1; (void)nTypeUp;
  #endif // NEXT && _NEXT_SHORTCUT
  #ifdef _RESTART_UP_FOR_NEXT_OR_EMPTY
    // gcc thinks we use nBLRUp uninitialized unless we initialize it here.
    int nBLRUp = 0; // valid only if nBLUp != 0
  #endif // _RESTART_UP_FOR_NEXT_OR_EMPTY
      #ifdef _LNX
    Word_t* pwLnXUp = NULL; (void)pwLnXUp;
      #endif // _LNX
    // gcc complains that nBLUp may be used uninitialized with CODE_XX_SW.
    int bNeedPrefixCheck = 0; (void)bNeedPrefixCheck;
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
      #ifndef LOOKUP
      #if !defined(COUNT)
    Word_t* pwLnXOrig = pwLnX; (void)pwLnXOrig;
      #endif // !COUNT
      #endif // !LOOKUP
  #endif // _LNX
    Link_t *pLnPrefix = NULL; (void)pLnPrefix;
    Word_t *pwrPrefix = NULL; (void)pwrPrefix;
    int nBLRPrefix = 0; (void)nBLRPrefix;
    Word_t wPopCntUp = 0; (void)wPopCntUp;
  #ifdef COUNT
    Word_t wPopCntSum = 0;
      #ifdef COUNT_2
    Word_t wPopCntSum1 = wPopCntSum1;
    int nPhaseCount = 0;
      #endif // COUNT_2
      #ifdef COUNT_2_PREFIX
    struct { Link_t* q_pLn; int q_nBL;
              #ifdef _LNX
             Word_t* q_pwLnX;
              #endif // _LNX
    } aqyaCount[2] = { { pLn, nBL
              #ifdef _LNX
                       , pwLnX
              #endif // _LNX
    } };
    int nBitsCount = LOG((((wKey0 == 0) ? 0 : (wKey0 - 1)) ^ wKey) | 1) + 1;
      #elif defined(COUNT_2)
    Link_t *pLnCount = pLn;
      #endif // COUNT_2_PREFIX elif COUNT_2
  #endif // COUNT
    // Cleanup is expensive. So we only do it if it has been requested.
    int bCleanupRequested = 0; (void)bCleanupRequested;
    int bCleanup = 0; (void)bCleanup;
    int nBLR;
    int nPos; (void)nPos; // (void) is for Judy1 LOOKKUP turn-on
  #ifdef _AUG_TYPE
    int nAugTypeBits;
  #endif // _AUG_TYPE
  #ifdef GOTO_AT_FIRST_IN_LOOKUP
  #ifdef SKIP_LINKS
  #if defined(LOOKUP) || defined(NEXT_EMPTY)
    // This shortcut made LOOKUP and NEXT_EMPTY faster in my original testing.
    // It made NEXT slower.
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot);
    // nBL, pLn, wRoot, nType and pwr of qy are set up
  #ifdef SKIP_TO_SW_SHORTCUT
    // I've seen this shortcut wreak havoc on packed bitmap performance
    // with Alder Lake. Even though it inexplicably
    // improves performance of the top-level leaf with no switch, i.e.
    // populations up to the first splay.
    if (nType >= T_SKIP_TO_SWITCH) { // What about T_SKIP_TO_??_SW?
        DBGX(Checkpoint(qya, "goto skip_to_sw"));
        goto t_skip_to_switch;
    }
  #endif // SKIP_TO_SW_SHORTCUT
    // This shortcut made the code faster in my testing.
    nBLR = nBL;
      #ifdef _AUG_TYPE
    nAugTypeBits = AugTypeBits(nBL);
      #endif // _AUG_TYPE
    goto fastAgain;
  #endif // LOOKUP || NEXT_EMPTY
  #endif // SKIP_LINKS
  #endif // GOTO_AT_FIRST_IN_LOOKUP
  #if !defined(LOOKUP) || defined(B_JUDYL)
    nPos = -1;
  #endif // !LOOKUP || B_JUDYL
  #ifdef COUNT
    int bLinkPresent;
    int nLinks;
  #endif // COUNT
  #ifndef LOOKUP
  #if !defined(COUNT) || defined(COUNT_2)
    goto top;
    DBGX(Checkpoint(qya, "top"));
top:;
  #endif // !COUNT || COUNT_2
  #endif // !LOOKUP
    nBLR = nBL;
    goto again;
    DBGX(Checkpoint(qya, "again"));
again:;
  #ifdef _AUG_TYPE
    nAugTypeBits = AugTypeBits(nBL);
    goto againAugType;
againAugType:;
  #endif // _AUG_TYPE
  #ifdef SKIP_LINKS
    assert(nBLR == nBL);
  #endif // SKIP_LINKS
  #ifndef LOOKUP
  #ifndef CODE_XX_SW
    assert(nBL >= cnBitsInD1); // valid for LOOKUP too
  #endif // !CODE_XX_SW
  #endif // !LOOKUP
  #ifdef _RESTART_UP
insertAgain:
  #endif // _RESTART_UP
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot); // pwr isn't meaningful for all nType values
    // nBL, pLn, wRoot, nType and pwr of qy are set up
    DBGX(Checkpoint(qya, "enter switch stmt"));
    DBGX(printf("# wKey 0x%zx\n", wKey));
    goto fastAgain;
fastAgain:;
  #ifdef _AUG_TYPE
//printf("# fastAgain nAugTypeBits 0x%x nType %d\n", nAugTypeBits, nType);
  #endif // _AUG_TYPE
  // AUGMENT_TYPE_8 with AUGMENT_TYPE assumes the low three bits of nBL are
  // always zero and uses the high three bits of nBL to augment the type.
  // for eight different legal nBL values: 8, 16, 24, 32, 40, 48, 56, 64.
  // How expensive would it be to maintain nDL as well as nBL?
  // AUGMENT_TYPE without AUGMENT_TYPE_8 has four different nBL groups:
  // 5-8, 9-16, 17-32, 33-64. 0-4 does not work.

  // __builtin_prefetch(0, 0); // Uncomment and find prefetcht0 in bl[L].s.
  #ifdef _AUG_TYPE
    switch (nAugTypeBits | nType)
  #else // _AUG_TYPE
    switch (nType)
  #endif // _AUG_TYPE
    {
  #ifdef DEBUG
  #ifndef DEFAULT_SKIP_TO_SW
  #if !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
    default:
        DBG(printf("\n# Unknown type 0x%02x nBL %d wRoot 0x%zx\n",
                   nType, nBL, wRoot));
      #ifdef _AUG_TYPE
        DBG(printf("\n# nAugTypeBits 0x%02x\n", nAugTypeBits));
      #endif // _AUG_TYPE
        exit(1); // We lose debug output piped to a file when assert blows.
        //assert(0);
  #endif // !defined(SKIP_LINKS) || !defined(ALL_SKIP_TO_SW_CASES)
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
      #ifndef DEFAULT_SKIP_TO_SW
          #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
              #ifdef _SKIP_TO_FULL_SW
    case T_SKIP_TO_FULL_SW + ((64 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((56 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((48 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((40 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((32 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((24 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((16 - 1) << cnBitsTypeMask):
              #endif // _SKIP_TO_FULL_SW
    case T_SKIP_TO_SWITCH + ((64 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
              #ifdef _SKIP_TO_FULL_SW
    case T_SKIP_TO_FULL_SW + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((16 / 2 - 1) << cnBitsTypeMask):
              #endif // _SKIP_TO_FULL_SW
    case T_SKIP_TO_SWITCH + ((64 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 2 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 2 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
              #ifdef _SKIP_TO_FULL_SW
    case T_SKIP_TO_FULL_SW + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((16 / 4 - 1) << cnBitsTypeMask):
              #endif // _SKIP_TO_FULL_SW
    case T_SKIP_TO_SWITCH + ((64 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 4 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 4 - 1) << cnBitsTypeMask):
          #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
              #ifdef _SKIP_TO_FULL_SW
    case T_SKIP_TO_FULL_SW + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_FULL_SW + ((16 / 8 - 1) << cnBitsTypeMask):
              #endif // _SKIP_TO_FULL_SW
    case T_SKIP_TO_SWITCH + ((64 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((56 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((48 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((40 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((32 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((24 / 8 - 1) << cnBitsTypeMask):
    case T_SKIP_TO_SWITCH + ((16 / 8 - 1) << cnBitsTypeMask):
          #else // AUG_TYPE_64_LOOKUP && LOOKUP
  #ifdef _SKIP_TO_FULL_SW
    CASES_AUG_TYPE(T_SKIP_TO_FULL_SW)
  #endif // _SKIP_TO_FULL_SW
    CASES_AUG_TYPE(T_SKIP_TO_SWITCH)
              #ifdef _SKIP_TO_FULL_SW
    case T_SKIP_TO_FULL_SW:
                  #ifdef NEXT_EMPTY
        goto t_skip_to_full_sw;
                  #endif // NEXT_EMPTY
              #endif // _SKIP_TO_FULL_SW
    case T_SKIP_TO_SWITCH: // skip link to uncompressed switch
          #endif // else AUG_TYPE_64_LOOKUP && LOOKUP
      #endif // !DEFAULT_SKIP_TO_SW
        goto t_skip_to_switch; // silence cc in case there are no other uses
  #endif // SKIP_LINKS

  #ifdef SKIP_TO_XX_SW // Doesn't work yet.
    CASES_AUG_TYPE(T_SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: // skip link to narrow/wide switch
        goto t_skip_to_xx_sw;
  #endif // SKIP_TO_XX_SW

  // #ifdef SWITCH
      #if defined(AUG_TYPE_64_LOOKUP) && defined(LOOKUP)
          #ifdef FULL_SW
    case T_FULL_SW + ((64 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_FULL_SW + ((56 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_FULL_SW + ((48 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_FULL_SW + ((40 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_FULL_SW + ((32 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_FULL_SW + ((24 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_FULL_SW + ((16 - 1) << cnBitsTypeMask): goto t_sw_16;
          #endif // FULL_SW
    case T_SWITCH + ((64 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_32_LOOKUP) && defined(LOOKUP)
          #ifdef FULL_SW
    case T_FULL_SW + ((64 / 2 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_FULL_SW + ((56 / 2 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_FULL_SW + ((48 / 2 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_FULL_SW + ((40 / 2 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_FULL_SW + ((32 / 2 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_FULL_SW + ((24 / 2 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_FULL_SW + ((16 / 2 - 1) << cnBitsTypeMask): goto t_sw_16;
          #endif // FULL_SW
    case T_SWITCH + ((64 / 2 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 2 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 2 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 2 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 2 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 2 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 2 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_16_LOOKUP) && defined(LOOKUP)
          #ifdef FULL_SW
    case T_FULL_SW + ((64 / 4 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_FULL_SW + ((56 / 4 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_FULL_SW + ((48 / 4 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_FULL_SW + ((40 / 4 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_FULL_SW + ((32 / 4 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_FULL_SW + ((24 / 4 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_FULL_SW + ((16 / 4 - 1) << cnBitsTypeMask): goto t_sw_16;
          #endif // FULL_SW
    case T_SWITCH + ((64 / 4 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 4 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 4 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 4 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 4 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 4 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 4 - 1) << cnBitsTypeMask): goto t_sw_16;
      #elif defined(AUG_TYPE_8_LOOKUP) && defined(LOOKUP)
          #ifdef FULL_SW
    case T_FULL_SW + ((64 / 8 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_FULL_SW + ((56 / 8 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_FULL_SW + ((48 / 8 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_FULL_SW + ((40 / 8 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_FULL_SW + ((32 / 8 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_FULL_SW + ((24 / 8 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_FULL_SW + ((16 / 8 - 1) << cnBitsTypeMask): goto t_sw_16;
          #endif // FULL_SW
    case T_SWITCH + ((64 / 8 - 1) << cnBitsTypeMask): goto t_sw_64;
    case T_SWITCH + ((56 / 8 - 1) << cnBitsTypeMask): goto t_sw_56;
    case T_SWITCH + ((48 / 8 - 1) << cnBitsTypeMask): goto t_sw_48;
    case T_SWITCH + ((40 / 8 - 1) << cnBitsTypeMask): goto t_sw_40;
    case T_SWITCH + ((32 / 8 - 1) << cnBitsTypeMask): goto t_sw_32;
    case T_SWITCH + ((24 / 8 - 1) << cnBitsTypeMask): goto t_sw_24;
    case T_SWITCH + ((16 / 8 - 1) << cnBitsTypeMask): goto t_sw_16;
      #else // AUG_TYPE_64_LOOKUP && LOOKUP
          #ifdef _AUG_TYPE_8_SW
              #ifdef FULL_SW
    case T_FULL_SW + 7 * (1 << cnBitsTypeMask): goto t_sw_plus_112;
    case T_FULL_SW + 6 * (1 << cnBitsTypeMask): goto t_sw_plus_96;
    case T_FULL_SW + 5 * (1 << cnBitsTypeMask): goto t_sw_plus_80;
    case T_FULL_SW + 4 * (1 << cnBitsTypeMask): goto t_sw_plus_64;
              #endif // FULL_SW
    case T_SWITCH + 7 * (1 << cnBitsTypeMask): goto t_sw_plus_112;
    case T_SWITCH + 6 * (1 << cnBitsTypeMask): goto t_sw_plus_96;
    case T_SWITCH + 5 * (1 << cnBitsTypeMask): goto t_sw_plus_80;
    case T_SWITCH + 4 * (1 << cnBitsTypeMask): goto t_sw_plus_64;
          #endif // _AUG_TYPE_8_SW
        #if defined(AUGMENT_TYPE) && defined(LOOKUP) || defined(_AUG_TYPE_8_SW)
              #ifdef FULL_SW
    case T_FULL_SW + 3 * (1 << cnBitsTypeMask): goto t_sw_plus_48;
    case T_FULL_SW + 2 * (1 << cnBitsTypeMask): goto t_sw_plus_32;
    case T_FULL_SW + 1 * (1 << cnBitsTypeMask): goto t_sw_plus_16;
                #endif // FULL_SW
    case T_SWITCH + 3 * (1 << cnBitsTypeMask): goto t_sw_plus_48;
    case T_SWITCH + 2 * (1 << cnBitsTypeMask): goto t_sw_plus_32;
    case T_SWITCH + 1 * (1 << cnBitsTypeMask): goto t_sw_plus_16;
        #endif // AUGMENT_TYPE && LOOKUP || _AUG_TYPE_8_SW
          #ifdef NEXT
          #if defined(AUG_TYPE_8_NEXT_EK_XV) && !defined(AUG_TYPE_8_SW_NEXT)
              #ifdef FULL_SW
    CASES_AUG_TYPE(T_FULL_SW)
              #endif // FULL_SW
    CASES_AUG_TYPE(T_SWITCH)
          #endif // AUG_TYPE_8_NEXT_EK_XV && !AUG_TYPE_8_SW_NEXT
          #endif // NEXT
          #ifndef _AUG_TYPE_8_SW
          #if !defined(AUGMENT_TYPE) || !defined(LOOKUP)
          #ifdef FULL_SW
    case T_FULL_SW:
              #ifdef NEXT_EMPTY
        goto t_full_sw;
              #endif // NEXT_EMPTY
          #endif // FULL_SW
    case T_SWITCH: // no-skip (aka close) switch (vs. distant switch) w/o bm
              #ifdef NEXT_EMPTY
              #ifdef FULL_SW
        assert(gwPopCnt(qya, nBLR) != BPW_EXP(nBLR));
              #endif // FULL_SW
              #endif // NEXT_EMPTY
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
          #ifdef NEXT
              #if defined(AUG_TYPE_8_SW_NEXT) || defined(AUG_TYPE_8_NEXT_EK_XV)
    CASES_AUG_TYPE(T_LIST)
              #endif // AUG_TYPE_8_SW_NEXT || AUG_TYPE_8_NEXT_EK_XV
          #else // NEXT
              #if defined(AUGMENT_TYPE_8) && defined(_AUG_TYPE)
    case T_LIST + 7 * (1 << cnBitsTypeMask): goto t_list112;
    case T_LIST + 6 * (1 << cnBitsTypeMask): goto t_list96;
    case T_LIST + 5 * (1 << cnBitsTypeMask): goto t_list80;
    case T_LIST + 4 * (1 << cnBitsTypeMask): goto t_list64;
              #endif // AUGMENT_TYPE_8 && _AUG_TYPE
              #if defined(AUGMENT_TYPE) && defined(_AUG_TYPE)
    case T_LIST + 3 * (1 << cnBitsTypeMask): goto t_list48;
    case T_LIST + 2 * (1 << cnBitsTypeMask): goto t_list32;
    case T_LIST + 1 * (1 << cnBitsTypeMask): goto t_list16;
              #endif // AUGMENT_TYPE && _AUG_TYPE
          #endif // NEXT else
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP
  #endif // (cwListPopCntMax != 0)
  #ifndef _AUG_TYPE_X
  #if (cwListPopCntMax != 0)
  #ifdef _T_LIST
    case T_LIST:
        goto t_list;
  #endif // _T_LIST
  #endif // (cwListPopCntMax != 0)
  #endif // !_AUG_TYPE_X

  #ifdef UA_PARALLEL_128
    CASES_AUG_TYPE(T_LIST_UA)
    case T_LIST_UA:
        goto t_list_ua;
  #endif // UA_PARALLEL_128

  #ifdef XX_LISTS
    CASES_AUG_TYPE(T_XX_LIST)
    case T_XX_LIST:
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
  #ifdef BITMAP
  #if defined(PACK_BM_VALUES) || !defined(LOOKUP) || !defined(B_JUDYL)
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

  #ifdef SKIP_LINKS
t_skip_to_switch:
    {
        // Skip to switch.
        // pwr points to a switch
          #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
          #ifndef FULL_SW
        DBG((nType != T_SKIP_TO_SWITCH) ? printf("\nnType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
          #endif // FULL_SW
          #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
        DBGX(printf("SKIP_TO_SW\n"));

        // Looks to me like PrefixMismatch has no performance issues with
        // not all digits being the same size.  It doesn't care.
        // But it does use nBL a couple of times.  Maybe it would help to
        // have bl tests here and call with a constant.

        // PREFIX_MISMATCH updates nBLR.
        Word_t wPrefixMismatch = PREFIX_MISMATCH(qy);
        if (wPrefixMismatch != 0) {
          #if defined(COUNT)
            DBGC(printf("SKIP_TO_SW: COUNT PM 0x%016zx\n", wPrefixMismatch));
            // If key is bigger than prefix we have to count the keys here.
            // Othwerwise we don't.
            if (wKey > (wKey - (wPrefixMismatch << nBLR))) {
                Word_t wPopCnt;
                wPopCnt = gwPopCnt(qya, nBLR);
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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
            goto break_from_main_switch;
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
      #if defined(NEXT_EMPTY) && defined(FULL_SW) && !defined(_SKIP_TO_FULL_SW)
        goto t_full_sw;
      #else // NEXT_EMPTY && FULL_SW && !_SKIP_TO_FULL_SW
        goto t_switch;
      #endif // NEXT_EMPTY && FULL_SW && !_SKIP_TO_FULL_SW else
  #endif // else AUGMENT_TYPE_8 && LOOKUP && BL_SPECIFIC_SKIP
    } // end of t_skip_to_sw
  #endif // SKIP_LINKS

  #ifdef _SKIP_TO_FULL_SW
  #ifdef NEXT_EMPTY
  #ifdef FULL_SW
        goto t_skip_to_full_sw;
t_skip_to_full_sw:;
    {
        if (PREFIX_MISMATCH(qy)) {
            *pwKey = wKey;
            return Success;
        }
        goto t_full_sw;
    }
  #endif // FULL_SW
  #endif // NEXT_EMPTY
  #endif // _SKIP_TO_FULL_SW

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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
          #ifdef PF_EK_VAL
        PREFETCH(pwLnXNew);
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
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
       assert((nBLR != nBL));
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
            goto break_from_main_switch;
        }
        DBGX(Checkpoint(qya, "t_switch"));
        nBW = gnBWGuts(T_SWITCH, wRoot, nBLR); // gnBLR may be slower
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
        // ((uint8_t *)&wKey)[(cnBitsPerWord - nBL) >> 3];
        // ((uint8_t *)&wKey)[cnDigitsPerWord - nDL];
        // ((uint8_t *)&wSwappedKey)[nDL];
        // *(uint8_t *)&wSwappedAndShiftedKey;
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
        DBGX(printf("pwLnXNew %p\n", pwLnXNew));
          #ifdef PF_EK_VAL
          #ifdef LOOKUP
        PREFETCH(pwLnXNew);
          #endif // LOOKUP
          #endif // PF_EK_VAL
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef PF_PWR
        PREFETCH(wr_pwr(pLnNew->ln_wRoot));
      #endif // PF_PWR
        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
  //#endif // !AUGMENT_TYPE || !LOOKUP || SKIP_LINKS
      #ifdef NEXT_EMPTY
          #ifdef FULL_SW
              #if cnSwCnts == 0
        assert(nType != T_FULL_SW);
              #endif // cnSwCnts == 0
        assert(gwPopCnt(qya, nBLR) != BPW_EXP(nBLR));
          #else // FULL_SW
        goto t_full_sw_guts;
          #endif // FULL_SW else
      #endif // NEXT_EMPTY
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
      #ifdef COUNT
          #ifdef COUNT_2_PREFIX
        if (nBLR - nBW < nBitsCount) {
          #endif // COUNT_2_PREFIX
            wPopCntSum += CountSw(qya, wDigit, nLinks);
          #ifdef COUNT_2_PREFIX
            aqyaCount[nPhaseCount & 1].q_nBL = nBL;
            aqyaCount[nPhaseCount & 1].q_pLn = pLn;
              #ifdef _LNX
            aqyaCount[nPhaseCount & 1].q_pwLnX = pwLnX;
              #endif // _LNX
            nPhaseCount |= 1;
        }
          #endif // COUNT_2_PREFIX
        if (!bLinkPresent) { goto break_from_main_switch; }
      #endif // COUNT
        // Save the previous link and advance to the next.
        IF_NOT_LOOKUP(nBLUp = nBL);
        IF_NOT_LOOKUP(pLnUp = pLn);
      #ifdef _NEXT_SHORTCUT
        //IF_NEXT(nTypeUp = nType);
      #endif // _NEXT_SHORTCUT
      #ifdef _RESTART_UP_FOR_NEXT_OR_EMPTY
        nBLRUp = nBLR;
      #endif // _RESTART_UP_FOR_NEXT_OR_EMPTY
      #ifdef _LNX
        IF_NOT_LOOKUP(IF_NOT_NEXT(pwLnXUp = pwLnX));
      #endif // _LNX
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
        goto again; // nType = wr_nType(wRoot); pwr = wr_pwr(wRoot); switch
    } // end of t_switch
  #endif // _T_SWITCH_TAIL || !AUGMENT_TYPE || !LOOKUP
  // #endif SWITCH

  #ifdef NEXT_EMPTY
    goto t_full_sw;
t_full_sw:; // check for full sub-expanse if cnSwCnts else for full sw
    {
        nBW = gnBW(qy, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW); // extract bits from key
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #endif // _LNX
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
        goto t_full_sw_guts;
t_full_sw_guts:;
      #if cnSwCnts != 0
          #ifdef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
        if (gwPopCnt(qya, nBLR) == BPW_EXP(nBLR)) {
            if (nBLR == cnBitsPerWord) {
                return Failure;
            }
            wKey = (wKey & ~NZ_MSK(nBLR)) + EXP(nBLR);
            goto break_from_main_switch;
        }
          #endif // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
        Word_t* pwCnts = ((Switch_t*)pwr)->sw_awCnts;
        if (nBLR <= 16) {
            int nShift
                = (nBW > nLogSwSubCnts(1)) ? (nBW - nLogSwSubCnts(1)) : 0;
            int nCntNum = wDigit >> nShift;
          #ifdef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            while (((uint16_t*)pwCnts)[nCntNum++] == EXP(nBLR - nBW + nShift))
          #else // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            if (((uint16_t*)pwCnts)[nCntNum++] == EXP(nBLR - nBW + nShift))
          #endif // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART else
            {
          #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                if (gwPopCnt(qya, nBLR) == EXP(nBLR)) {
                    wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                    goto break_from_main_switch;
                }
                do
          #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                {
                    if (nCntNum == (int)EXP(nBW - nShift)) {
                        wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                        goto break_from_main_switch;
                    }
                }
          #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                while (((uint16_t*)pwCnts)[nCntNum++]
                           == EXP(nBLR - nBW + nShift));
          #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            }
            if (nCntNum - 1 > (int)(wDigit >> nShift)) {
                assert(((nCntNum - 1) << nShift) > (int)wDigit);
                wDigit = (nCntNum - 1) << nShift;
                pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
          #ifdef _LNX
                pwLnXNew = gpwLnX(qy, EXP(nBW), wDigit);
          #endif // _LNX
                wKey = (wKey & ~MSK(nBLR)) + (wDigit << (nBLR - nBW));
            }
          #if cnBitsPerWord > 32
        } else if (nBLR <= 32) {
            int nShift
                = (nBW > nLogSwSubCnts(2)) ? (nBW - nLogSwSubCnts(2)) : 0;
            int nCntNum = wDigit >> nShift;
              #ifdef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            while (((uint32_t*)pwCnts)[nCntNum++] == EXP(nBLR - nBW + nShift))
              #else // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            if (((uint32_t*)pwCnts)[nCntNum++] == EXP(nBLR - nBW + nShift))
              #endif // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART else
            {
              #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                if (gwPopCnt(qya, nBLR) == EXP(nBLR)) {
                    wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                    goto break_from_main_switch;
                }
                do
              #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                {
                    if (nCntNum == (int)EXP(nBW - nShift)) {
                        wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                        goto break_from_main_switch;
                    }
                }
              #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                while (((uint32_t*)pwCnts)[nCntNum++]
                           == EXP(nBLR - nBW + nShift));
              #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            }
            if (nCntNum - 1 > (int)(wDigit >> nShift)) {
                assert(((nCntNum - 1) << nShift) > (int)wDigit);
                wDigit = (nCntNum - 1) << nShift;
                pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
              #ifdef _LNX
                pwLnXNew = gpwLnX(qy, EXP(nBW), wDigit);
              #endif // _LNX
                wKey = (wKey & ~MSK(nBLR)) + (wDigit << (nBLR - nBW));
            }
          #endif // cnBitsPerWord > 32
        } else {
          #if cnSwCnts > 1
            int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
            int nCntNum = wDigit >> nShift;
              #ifdef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            while (pwCnts[nCntNum++] == EXP(nBLR - nBW + nShift))
              #else // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            if (pwCnts[nCntNum++] == EXP(nBLR - nBW + nShift))
              #endif // NEXT_EMPTY_TEST_WHOLE_BEFORE_PART else
            {
              #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                if (gwPopCnt(qya, nBLR) == BPW_EXP(nBLR)) {
                    if (nBLR == cnBitsPerWord) {
                        return Failure;
                    }
                    wKey = (wKey & ~NZ_MSK(nBLR)) + EXP(nBLR);
                    goto break_from_main_switch;
                }
                do
              #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                {
                    if (nCntNum == (int)EXP(nBW - nShift)) {
                        if (nBLR == cnBitsPerWord) {
                            return Failure;
                        }
                        wKey = (wKey & ~NZ_MSK(nBLR)) + EXP(nBLR);
                        goto break_from_main_switch;
                    }
                }
              #ifndef NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
                while (pwCnts[nCntNum++] == EXP(nBLR - nBW + nShift));
              #endif // !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
            }
            if (nCntNum - 1 > (int)(wDigit >> nShift)) {
                assert(((nCntNum - 1) << nShift) > (int)wDigit);
                wDigit = (nCntNum - 1) << nShift;
                pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
                  #ifdef _LNX
                pwLnXNew = gpwLnX(qy, EXP(nBW), wDigit);
                  #endif // _LNX
                wKey = (wKey & ~NZ_MSK(nBLR)) + (wDigit << (nBLR - nBW));
            }
          #elif !defined(NEXT_EMPTY_TEST_WHOLE_BEFORE_PART) // cnSwCnts > 1
            if (gwPopCnt(qya, nBLR) == BPW_EXP(nBLR)) {
                if (nBLR == cnBitsPerWord) {
                    return Failure;
                }
                wKey = (wKey & ~NZ_MSK(nBLR)) + EXP(nBLR);
                goto break_from_main_switch;
            }
          #endif // cnSwCnts > 1 elif !NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
        }
        goto switchTail;
      #else // cnSwCnts != 0
          #ifdef FULL_SW
        assert(gwPopCnt(qya, nBLR) == BPW_EXP(nBLR));
          #else // FULL_SW
        if (gwPopCnt(qya, nBLR) != BPW_EXP(nBLR)) {
            goto switchTail;
        }
          #endif // FULL_SW else
        wKey = (wKey & ~NZ_MSK(nBLR)) + BPW_EXP(nBLR);
        goto break_from_main_switch;
      #endif // cnSwCnts != 0 else
    }
  #endif // NEXT_EMPTY

  #ifdef CODE_XX_SW
t_xx_sw:
    {
        DBGX(Checkpoint(qya, "t_xx_sw"));
        nBW = gnBWGuts(T_XX_SW, wRoot, nBLR); // num bits decoded
        wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
        pLnNew = &pwr_pLinks((Switch_t *)pwr)[wDigit];
      #ifdef _LNX
        pwLnXNew = gpwLnX(qy, /* wLinks */ EXP(nBW), /* wIndex */ wDigit);
      #endif // _LNX
        IF_COUNT(bLinkPresent = 1);
        IF_COUNT(nLinks = 1 << nBW);
      #ifdef NEXT_EMPTY
          #if cnSwCnts != 0
        goto t_full_sw_guts; // check for full sub-expanses
          #else // cnSwCnts != 0
        if (gwPopCnt(qya, nBLR) != BPW_EXP(nBLR)) {
            goto switchTail;
        }
        wKey = (wKey & ~NZ_MSK(nBLR)) + BPW_EXP(nBLR);
        goto break_from_main_switch;
          #endif // cnSwCnts != 0 else
      #endif // NEXT_EMPTY
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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
        nBW = gnBWGuts(T_BM_SW, wRoot, nBLR); // num bits decoded
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
          #ifdef PF_SW_BM_WORDS
          //#ifndef OFFSET_IN_SW_BM_WORD
// Why are we prefetching the bitmap at all?
// We first go after the word with the bit.
// If the bit is set then we go after all of the words before it.
// But I think the cpu can figure that out.
            pcPrefetch = (void*)PWR_pwBm(&pLn->ln_wRoot, pwr, nBW);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
          //#endif // #ifndef OFFSET_IN_SW_BM_WORD
          #endif // PF_SW_BM_WORDS
          #ifdef B_JUDYL
          #if (cnBitsPerWord > 32)
            int nLinkCnt = BmSwLinkCnt(qya);
            wSwIndex = Psplit(nLinkCnt, nBW, /*nShift*/ 0, wDigit);
            (void)wSwIndex;
              #ifdef PF_BM_SW_LN
            pcPrefetch = (void*)&pwr_pLinks((BmSwitch_t*)pwr)[wSwIndex];
            PREFETCH(pcPrefetch - 64);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
              #endif // PF_BM_SW_LN
              #ifdef REMOTE_LNX
              #ifdef PF_BM_SW_LNX
            pcPrefetch = (void*)gpwLnX(qy, nLinkCnt, wSwIndex);
            PREFETCH(pcPrefetch - 64);
            PREFETCH(pcPrefetch);
            PREFETCH(pcPrefetch + 64);
              #endif // PF_BM_SW_LNX
              #endif // REMOTE_LNX
          #endif // (cnBitsPerWord > 32)
          #endif // B_JUDYL
//printf("# wDigit %zd\n", wDigit);
          #ifdef ONE_BM_SW_INDEX_CALL
            BmSwIndex(qya, wDigit, &wSwIndex, &bLinkPresent);
//printf("# wSwIndex %zd\n", wSwIndex);
          #else // ONE_BM_SW_INDEX_CALL
            BmSwIndex(qya, wDigit, /* pwSwIndex */ NULL, &bLinkPresent);
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
            BmSwIndex(qya, wDigit, &wSwIndex, &bLinkPresent);
              #endif // defined(BM_SW_FOR_REAL) && defined(SW_BM_DEREF_ONLY)
          #else // LOOKUP
            BmSwIndex(qya, wDigit, &wSwIndex, &bLinkPresent);
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
                int nWordNum
                    = (wDigit >> cnLogBmSwLinksPerBit) >> cnLogBmBitsPerBmWord;
                int nBitNum = (wDigit >> cnLogBmSwLinksPerBit)
                                & MSK(cnLogBmBitsPerBmWord);
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
                        wDigit <<= cnLogBmSwLinksPerBit;
                        // Might be WROOT_NULL, but link will be present.
                        //printf("wDigit 0x%zx\n", wDigit);
                        wKey &= ~NZ_MSK(nBLR);
                        wKey |= (wDigit << (nBLR - nBW));
                        //printf("# restart wKey 0x%02zx\n", wKey);
                        goto restart;
                    }
                    if ((Word_t)++nWordNum
                        >= (((nBW - cnLogBmSwLinksPerBit)
                                <= cnLogBmBitsPerBmWord)
                            ? 0
                            : EXP((nBW - cnLogBmSwLinksPerBit)
                                      - cnLogBmBitsPerBmWord)))
                    {
                        if (nBL >= cnBitsPerWord) {
                            //printf("# break\n");
                            break;
                        }
                        //printf("# tryNext *pwKey 0x%02zx\n", *pwKey);
              #ifdef RESTART_UP_FOR_NEXT
                        goto tryNextDigit;
              #else // RESTART_UP_FOR_NEXT
                        goto break_from_main_switch;
              #endif // RESTART_UP_FOR_NEXT else
                    }
                    wBm = pwBmWords[nWordNum];
                }
          #endif // NEXT
                IF_NEXT_EMPTY(*pwKey = wKey; return Success);
                goto break_from_main_switch;
            }
      #endif // ! defined(COUNT)
      #if defined(BM_SW_FOR_REAL) && !defined(ONE_BM_SW_INDEX_CALL)
            BmSwIndex(qya, wDigit, &wSwIndex, /* pbPresent */ NULL);
      #endif // defined(BM_SW_FOR_REAL) && !defined(ONE_BM_SW_INDEX_CALL)
        }

        pLnNew = &pwr_pLinks((BmSwitch_t *)pwr)[wSwIndex];
      #ifdef _LNX
        int nLinkCnt = BmSwLinkCnt(qya);
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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
                && (pLn == pLnOrig)
              #ifndef LOOKUP
                && (nBLOrig == cnBitsPerWord)
              #endif // !LOOKUP
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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
            goto break_from_main_switch;
        }
        goto t_list;
    } // end of t_skip_to_list
  #endif // SKIP_TO_LIST

  #if (cwListPopCntMax != 0)
  #ifdef NEXT_EMPTY
#define T_LIST_LOOKUP_GUTS(_nBL, pLn, pwRoot, pwLnX, _wKey, _wRoot, _suffix) \
{ \
    nBLR = nBL = _nBL; \
    int nPos; /* nPos is not used after break_from_main_switch for LOOKUP */ \
    if (WROOT_IS_NULL(T_LIST, _wRoot) \
        || (PREFIX_CHECK_AT_LEAF(qy, _wKey) != Success) \
        || ((nPos = -!LIST_HAS_KEY(_suffix, qya, nBLR, _wKey)) < 0)) \
    { \
        *pwKey = wKey; return Success; /* return from NextEmpty */ \
    } \
    goto t_list_tail; \
}
  #else // NEXT_EMPTY
#define T_LIST_LOOKUP_GUTS(_nBL, pLn, pwRoot, pwLnX, _wKey, _wRoot, _suffix) \
{ \
    nBLR = nBL = _nBL; \
    int nPos; /* nPos is not used after break_from_main_switch for LOOKUP */ \
    if (WROOT_IS_NULL(T_LIST, _wRoot) \
        || ((nPos = SEARCH_LIST(_suffix, qya, nBLR, _wKey)) < 0)) \
    { \
        goto break_from_main_switch; \
    } \
    SMETRICS_POP(j__SearchPopulation += gnListPopCnt(qy, nBLR)); \
    SMETRICS_GET(++j__GetCalls); \
    return BJL(&gpwValues(qy)[~nPos]) BJ1(KeyFound); \
}
  #endif // NEXT_EMPTY
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

  #ifndef NEXT
  #ifndef _AUG_TYPE_X
  #if (cwListPopCntMax != 0)
  #if defined(AUGMENT_TYPE_8) && defined(_AUG_TYPE)
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
  #if defined(AUGMENT_TYPE) && defined(_AUG_TYPE)
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
  #if defined(AUGMENT_TYPE) && defined(_AUG_TYPE)
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
  #if defined(AUGMENT_TYPE_8) && defined(_AUG_TYPE)
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
  #if defined(AUGMENT_TYPE) && defined(_AUG_TYPE)
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
  #if defined(AUGMENT_TYPE) && defined(_AUG_TYPE)
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
  #endif // !NEXT

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

        if (WROOT_IS_NULL(T_LIST, wRoot)) {
            IF_INS_OR_REM(nPos = 0);
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
            IF_NOT_INSERT(goto break_from_main_switch);
            IF_INSERT(goto t_list_request_cleanup_if_needed);
        }

        IF_COUNT(Word_t wKeyGe = wKey);
        // Search the list.
        // AUGMENT_TYPE_8_PLUS_4 and PACK_L1_VALUES both complicate this.
        // I wonder if we could incorporate them into the SEARCH_LIST macro.
      #ifdef LOOKUP
          #ifdef AUGMENT_TYPE
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
          #else // AUGMENT_TYPE
        if ((nPos =
              #if defined(B_JUDYL) && !defined(PACK_L1_VALUES)
                ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1))
                        ? -!ListHasKey8(qya, nBLR, wKey) :
              #endif // B_JUDYL && !PACK_L1_VALUES
                    SEARCH_LIST(, qya, nBLR, wKey)) >= 0)
          #endif // AUGMENT_TYPE else
      #elif defined(NEXT) || defined(COUNT) // LOOKUP
        if ((nPos = LocateGeKeyInList(qya, nBLR, &wKey)) >= 0)
      #elif defined(NEXT_EMPTY) // LOOKUP elif NEXT || COUNT elif
        if ((nPos = -!ListHasKey(qya, nBLR, wKey)) >= 0)
      #else // LOOKUP elif NEXT || COUNT elif NEXT_EMPTY
        if ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
      #endif // LOOKUP elif NEXT || COUNT elif NEXT_EMPTY else
        {
            //SMETRICS_POP(j__SearchPopulation += gnListPopCnt(qy, nBLR));
            SMETRICS_GET(++j__GetCalls);
            IF_INSERT(if (nIncr > 0) goto undo); // undo counting
            IF_REMOVE(goto removeGutsAndCleanup);
            IF_COUNT(wPopCntSum += (nPos + (wKeyGe == wKey)));
            IF_COUNT(goto break_from_main_switch);
      #ifdef NEXT_EMPTY
            goto t_list_tail;
t_list_tail:;
            int nPopCnt = gnListPopCnt(qy, nBLR);
            Word_t wIncr = BPW_EXP(nBLR);
            Word_t wKeyX = (wKey & ~(wIncr - 1)) + wIncr;
            nPos = LocateKeyInList(qya, nBLR, wKey);
            assert((Word_t)nPopCnt - nPos <= wKeyX - wKey);
            if (nPopCnt - nPos == (int)(wKeyX - wKey)) {
                wKey = wKeyX;
                goto break_from_main_switch;
            }
          #ifdef COMPRESSED_LISTS
            if (nBLR <= 8) {
                uint8_t* pcKeys = ls_pcKeysX(pwr, nBLR, nPopCnt);
                while (++wKey,
                       (++nPos < nPopCnt) && (pcKeys[nPos] == (uint8_t)wKey));
            } else if (nBLR <= 16) {
                uint16_t* psKeys = ls_psKeysX(pwr, nBLR, nPopCnt);
                while (++wKey,
                       (++nPos < nPopCnt) && (psKeys[nPos] == (uint16_t)wKey));
            } else
              #if cnBitsPerWord > 32
            if (nBLR <= 32) {
                uint32_t* piKeys = ls_piKeysX(pwr, nBLR, nPopCnt);
                while (++wKey,
                       (++nPos < nPopCnt) && (piKeys[nPos] == (uint32_t)wKey));
            } else
              #endif // cnBitsPerWord > 32
          #endif // COMPRESSED_LISTS
            {
                Word_t* pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
                while (++wKey, (++nPos < nPopCnt) && (pwKeys[nPos] == wKey));
            }
            *pwKey = wKey;
            return Success;
      #endif // NEXT_EMPTY
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
        IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
        {
      // LOOKUP_NO_LIST_SEARCH is for analysis only.
      #if !defined(LOOKUP) || !defined(LOOKUP_NO_LIST_SEARCH)
            IF_COUNT(Word_t wKeyGe = wKey);
            if (1
          #if defined(LOOKUP)
                && ListHasKey(qya, nBLR, wKey)
          #elif defined(NEXT) || defined(COUNT) // LOOKUP
                && ((nPos = LocateGeKeyInList(qya, nBLR, &wKey)) >= 0)
          #else // LOOKUP elif NEXT || COUNT
                && ((nPos = SearchList(qya, nBLR, wKey)) >= 0)
          #endif // LOOKUP elif NEXT || COUNT else
                )
      #endif // !LOOKUP || !LOOKUP_NO_LIST_SEARCH
            {
                IF_INSERT(if (nIncr > 0) { goto undo; }) // undo counting
                IF_REMOVE(goto removeGutsAndCleanup);
                IF_COUNT(wPopCntSum += (nPos + (wKeyGe == wKey)));
                IF_COUNT(goto break_from_main_switch);
                IF_NEXT(*pwKey = wKey);
                IF_NOT_REMOVE(return KeyFound);
            }
      #if defined(INSERT) || defined(REMOVE) || defined(COUNT)
            { nPos ^= -1; }
      #endif // INSERT || REMOVE || COUNT
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

        {
            nBLR = gnListBLR(qy);
            assert(nBLR > nBL); // skip up instead of down
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
            {
                SMETRICS_GET(++j__GetCalls);
      #if defined(INSERT)
                if (nIncr > 0) { goto undo; } // undo counting
      #endif // defined(INSERT)
      #if defined(REMOVE)
                goto removeGutsAndCleanup;
      #endif // defined(REMOVE)
                IF_COUNT(++nPos);
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
      #ifdef NEXT_EMPTY
                int nPopCnt = gnListPopCnt(qy, nBLR);
                Word_t wIncr = BPW_EXP(nBLR);
                Word_t wKeyX = (wKey & ~(wIncr - 1)) + wIncr;
                assert((Word_t)nPopCnt - nPos <= wKeyX - wKey);
                if (nPopCnt - nPos == (int)(wKeyX - wKey)) {
                    wKey = wKeyX;
                    goto break_from_main_switch;
                }
          #ifdef COMPRESSED_LISTS
                if (nBLR <= 8) {
                    uint8_t* pcKeys = ls_pcKeysX(pwr, nBLR, nPopCnt);
                    while (++wKey,
                       (++nPos < nPopCnt) && (pcKeys[nPos] == (uint8_t)wKey));
                } else if (nBLR <= 16) {
                    uint16_t* psKeys = ls_psKeysX(pwr, nBLR, nPopCnt);
                    while (++wKey,
                       (++nPos < nPopCnt) && (psKeys[nPos] == (uint16_t)wKey));
                } else
              #if cnBitsPerWord > 32
                if (nBLR <= 32) {
                    uint32_t* piKeys = ls_piKeysX(pwr, nBLR, nPopCnt);
                    while (++wKey,
                       (++nPos < nPopCnt) && (piKeys[nPos] == (uint32_t)wKey));
                } else
              #endif // cnBitsPerWord > 32
          #endif // COMPRESSED_LISTS
                {
                    Word_t* pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
                    while (++wKey,
                           (++nPos < nPopCnt) && (pwKeys[nPos] == wKey));
                }
                *pwKey = wKey;
                return Success;
      #endif // NEXT_EMPTY
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
      #if defined(COUNT)
        DBGC(printf("T_XX_LIST: nPos %d\n", nPos));
        wPopCntSum += nPos;
        DBGC(printf("list nPos 0x%x wPopCntSum " OWx"\n", nPos, wPopCntSum));
      #endif // defined(COUNT)
        IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
        {
              #ifdef USE_XX_SW_ONLY_AT_DL2
            // We assume we never blow-out into a one-digit bitmap.
// But that doesn't mean we don't want to support skip directly to a
// one-digit bitmap bypassing DL2.
            // We just double until we end up with one big bitmap at DL2.
            // But what about when we create XX_SW at (nBLR == nBitsLeftAtDl2)
            // and cbEmbeddedBitmap and nBLR - cnBWMin == cnBitsInD1? We did
            // not explicitly double.
            // Or nBLR - cnBWMin <= cnLogBitsPerLink?
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
        }
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
    #if defined(HYPERTUNE_PF_BM) && defined(LOOKUP) && defined(B_JUDYL)
        // What else are we assuming here in our overzealous hand-tuning of
        // this case necessitated by the untuned code being too slow?
      #ifndef BM_DSPLIT
        #error HYPERTUNE_PF_BM without BM_DSPLIT
      #endif // BM_DSPLIT
      #ifdef BMLF_POP_COUNT_32
        #error HYPERTUNE_PF_BM with BMLF_POP_COUNT_32 is TBD
      #endif // BMLF_POP_COUNT_32
      #if cnBitsInD1 != 8
        #error cnBitsInD1 != 8 with BM_DSPLIT is TBD
      #endif // cnBitsInD1 != 8
        // wKey already contains the key on entry here; wKey is a Word_t
        // pwLnX already contains a pointer to the second word of the JP on
        // entry here; pwLnX is a Word_t*
        // pwr already contains the pointer from the first word of the JP on
        // entry here; pwr is a Word_t*
        Word_t* pwBitmapValues
                    = gpwBitmapValues(cnBitsInD1, pwRoot, cnBitsInD1);
        Word_t wDigit = wKey & 0xff;
        // second word of the JP with cnts in high four bytes
        // and cumulative cnts in low four bytes
        Word_t wCnts = *pwLnX;
        // which word in the four-word bitmap contains the bit for wKey
        int nBmWordNum = wDigit >> 6;
        // how many bits set in previous words
        int nIndex = ((uint8_t*)&wCnts)[nBmWordNum];
        // how many bits set in the word with the bit for the key
        int subcnt = ((uint8_t*)&wCnts)[4 + nBmWordNum];
        int nBitNum = wKey & 0x3f;
        char* pcPrefetch
                = (char*)&pwBitmapValues[nIndex + (nBitNum * subcnt >> 6)];
        __builtin_prefetch(pcPrefetch - 32, 0, 0);
        __builtin_prefetch(pcPrefetch + 32, 0, 0);
        Word_t wBit = (Word_t)1 << nBitNum;
        Word_t *pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
        if (pwBitmap[nBmWordNum] & wBit) {
            // count bits for keys < wKey
            nIndex += __builtin_popcountll(pwBitmap[nBmWordNum] & (wBit - 1));
            return &pwBitmapValues[nIndex];
        }
        goto break_from_main_switch;
    #else // HYPERTUNE_PF_BM && LOOKUP && B_JUDYL
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
      #ifdef NEXT_EMPTY
        if ((!cbEmbeddedBitmap || (nBLR > cnLogBitsPerLink))
            && (gwBitmapPopCnt(qya, nBLR) == EXP(nBLR)))
        {
            // Why is gwBitmapPopCnt wrong for embedded bitmap?
            // Even after we fix it we might have to disambiguate
            // full pop here by checking a bit in the bitmap.
            wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
            goto break_from_main_switch;
        }
      #endif // NEXT_EMPTY
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
                        ? 0 : EXP(MAX(nBLR, cnLogBitsPerWord) - cnLogBitsPerWord)))
                {
          #ifdef SKIP_TO_BITMAP
          #ifndef NO_SKIP_AT_TOP
                    if (nBL >= cnBitsPerWord) {
                        break;
                    }
          #endif // NO_SKIP_AT_TOP
          #endif // SKIP_TO_BITMAP
          #ifdef RESTART_UP_FOR_NEXT
                    { goto tryNextDigit; }
          #else // RESTART_UP_FOR_NEXT
                    { goto break_from_main_switch; }
          #endif // RESTART_UP_FOR_NEXT else
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
                goto break_from_main_switch;
            }
            // Count bits.
            Word_t wPopCnt;
            if (cbEmbeddedBitmap && (nBLR <= cnLogBitsPerLink)) {
                assert(nBL == nBLR); // no skip to sub-link-size bm
                if (nBLR <= cnLogBitsPerWord) {
                    Word_t wBmMask = NZ_MSK((wKey & NZ_MSK(nBLR)) + 1);
                    Word_t wBits =
                        ((cnLogBitsPerLink == cnLogBitsPerWord)
                                && ((Word_t*)pLn == &pLn->ln_wRoot))
                            ? wRoot : *(Word_t*)pLn;
                    wPopCnt = __builtin_popcountll(wBits & wBmMask);
                } else {
                    Word_t wBitNum = wKey & MSK(nBLR);
                    Word_t wBmWordNum = wBitNum >> cnLogBitsPerWord;
                    Word_t wBmMask
                        = NZ_MSK((wBitNum & MSK(cnLogBitsPerWord)) + 1);
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
          #if cnWordsBm2Cnts != 0
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
                            = ~NZ_MSK((wKey & (cnBitsPerWord - 1)) + 1);
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
                            = NZ_MSK((wKey & (cnBitsPerWord - 1)) + 1);
                        wPopCnt += __builtin_popcountll(pwBitmap[nWordOffset]
                                                            & wBmMask);
                    }
                } else
          #endif // cnWordsBm2Cnts != 0
          #endif // cn2dBmMaxWpkPercent != 0
                if ((nBLR > 8) && (wKey & EXP(nBLR - 1))) {
                    wPopCnt = gwBitmapPopCnt(qya, nBLR);
                    if (wPopCnt == 0) {
                        wPopCnt = EXP(nBLR);
                    }
                    for (int nn = nWordOffset + 1;
                             nn < (int)EXP(MAX(nBLR, cnLogBitsPerWord) - cnLogBitsPerWord); nn++) {
                        wPopCnt -= __builtin_popcountll(pwBitmap[nn]);
                    }
                    Word_t wBmMask = ~NZ_MSK((wKey & MSK(nBLR)
                                       & (cnBitsPerWord - 1)) + 1);
                    wPopCnt -= __builtin_popcountll(pwBitmap[nWordOffset]
                                                        & wBmMask);
                } else {
                    wPopCnt = 0;
                    for (int nn = 0; nn < nWordOffset; nn++) {
                        wPopCnt += __builtin_popcountll(pwBitmap[nn]);
                    }
                    Word_t wBmMask
                        = NZ_MSK((wKey & MSK(nBLR) & (cnBitsPerWord - 1)) + 1);
                    wPopCnt += __builtin_popcountll(pwBitmap[nWordOffset]
                                                        & wBmMask);
                }
            }
            wPopCntSum += wPopCnt;
            DBGC(printf("bm nBLR %d wPopCnt " OWx" wPopCntSum " OWx"\n",
                        nBLR, wPopCnt, wPopCntSum));
            goto break_from_main_switch;
      #endif // COUNT
          #if defined(LOOKUP) && defined(LOOKUP_NO_BITMAP_SEARCH)
            // BUG?: Is pwrUp valid here, i.e. does it mean what this code
            // thinks it means?  Since SKIP_PREFIX_CHECK may not be #defined?
            // ? cnBitsLeftAtDl2 ?
            assert(gwPopCnt(qya, cnBitsInD2) != 0);
            return KeyFound;
          #else // LOOKUP && LOOKUP_NO_BITMAP_SEARCH
              #ifdef USE_XX_SW_ONLY_AT_DL2
            // We assume we never blow-out into a one-digit bitmap.
// But that doesn't mean we don't want to support skip directly to a
// one-digit bitmap bypassing DL2.
            // We just double until we end up with one big bitmap at DL2.
            // But what about when we create XX_SW at (nBLR == nBitsLeftAtDl2)
            // and cbEmbeddedBitmap and nBLR - cnBWMin == cnBitsInD1? We did
            // not explicitly double.
            // Or nBLR - cnBWMin <= cnLogBitsPerLink?
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
                              #ifdef BM_DSPLIT
                                  #if cnBitsInD1 != 8
              #error cnBitsInD1 != 8 with BM_DSPLIT is TBD
                                  #endif // cnBitsInD1 != 8
            (void)wPopCnt;
            Word_t wDigit = wKey & MSK(nBLR);
            Word_t wCnts = *pwLnX;
                                  #ifdef BMLF_POP_COUNT_32
            // BMLF_POP_COUNT_32 keeps a count of each 32-bit sub-bitmap.
            int nBmNum = wDigit >> 5;
            Word_t wSums = wCnts * 0x0101010101010100;
            int nIndex = ((uint8_t*)&wSums)[nBmNum];
            int subcnt = ((uint8_t*)&wCnts)[nBmNum];
            char* pcPrefetch
                = (char*)&pwBitmapValues[
                    nIndex + Psplit(subcnt, 5, /*nShift*/ 0, wKey & 0x1f)];
                                  #else // BMLF_POP_COUNT_32
            int nBmWordNum = wDigit >> cnLogBitsPerWord; (void)nBmWordNum;
            int nIndex = ((uint8_t*)&wCnts)[nBmWordNum];
            int subcnt = ((uint8_t*)&wCnts)[4+nBmWordNum];
            char* pcPrefetch
                = (char*)&pwBitmapValues[
                    nIndex + Psplit(subcnt, 6, /*nShift*/ 0, wKey & 0x3f)];
                                  #endif // BMLF_POP_COUNT_32 else
                              #else // BM_DSPLIT
            char* pcPrefetch
                = (char*)&pwBitmapValues[
                    Psplit(wPopCnt, cnBitsInD1, /*nShift*/ 0, wKey)];
                              #endif // BM_DSPLIT else
            (void)pcPrefetch;
                              #ifdef PF_BM_PREV_HALF_VAL
            PREFETCH(pcPrefetch - 32);
                              #endif // PF_BM_PREV_HALF_VAL
                              #ifdef PF_BM_PSPLIT_VAL
            PREFETCH(pcPrefetch);
                              #endif // PF_BM_PSPLIT_VAL
                              #ifdef PF_BM_PREV_VAL
            PREFETCH(pcPrefetch - 64);
                              #endif // PF_BM_PREV_VAL
                              #ifdef PF_BM_NEXT_HALF_VAL
            PREFETCH(pcPrefetch + 32);
                              #endif // PF_BM_NEXT_HALF_VAL
                              #ifdef PF_BM_NEXT_VAL
            PREFETCH(pcPrefetch + 64);
                              #endif // PF_BM_NEXT_VAL
                          #endif // PACK_BM_VALUES
                          #ifdef UNPACK_BM_VALUES
                          #ifdef PF_UNPACKED_BM_VAL
            PREFETCH(&pwBitmapValues[wKey & MSK(cnBitsInD1)]);
                          #endif // PF_UNPACKED_BM_VAL
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
                if (nIncr > 0) {
                    DBGX(printf("Bit is set!\n"));
                    goto undo; // undo counting
                }
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
  #if defined(LOOKUP) && defined(PACK_BM_VALUES) && defined(BM_DSPLIT)
                {
                                  #ifdef BMLF_POP_COUNT_32
                                      #ifdef BMLF_CNTS_CUM
    #error BMLF_CNTS_CUM with BMLF_POP_COUNT_32 with BM_DSPLIT is TBD
                                      #endif // BMLF_CNTS_CUM
                    uint32_t *pu32Bms
                                = (uint32_t*)((BmLeaf_t*)pwr)->bmlf_awBitmap;
                    uint32_t u32Bm = pu32Bms[nBmNum]; // uint32_t we want
                    uint32_t u32BmBitMask = EXP(wDigit & (32 - 1));
                    nIndex += PopCount32(u32Bm & (u32BmBitMask - 1));
                                  #else // BMLF_POP_COUNT_32
  #ifdef BMLF_POP_COUNT_8
    #error BMLF_POP_COUNT_8 with BM_DSPLIT
  #elif defined(BMLF_POP_COUNT_1) // BMLF_POP_COUNT_8
    #error BMLF_POP_COUNT_1 with BM_DSPLIT
  #else // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1
    // The name "BMLF_POP_COUNT_64" doesn't exist. But that is where we are.
    // Subexpanse counts are for 64-bit bitmaps.
    #ifndef BMLF_CNTS_CUM
      #error BM_DSPLIT w/o BMLF_CNTS_CUM (and not BMLF_POP_COUNT_32)
    #endif // !BMLF_CNTS_CUM
  #endif // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1 else
                    Word_t *pwBmWords = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
                    Word_t wBmWord = pwBmWords[nBmWordNum]; // word we want
                    Word_t wBmBitMask = EXP(wDigit & (cnBitsPerWord - 1));
                    nIndex += PopCount64(wBmWord & (wBmBitMask - 1));
                                  #endif // BMLF_POP_COUNT_32 else
                }
  #else // LOOKUP && PACK_BM_VALUES && BM_DSPLIT
                int nIndex =
                          #ifndef LOOKUP
                    BM_UNPACKED(wRoot) ? (int)(wKey & MSK(cnBitsInD1)) :
                          #endif // !LOOKUP
                        BmIndex(qya, cnBitsInD1, wKey);
  #endif // LOOKUP && PACK_BM_VALUES && BM_DSPLIT
                Word_t* pwValue = &pwBitmapValues[nIndex];
                      #ifdef LOOKUP
                      #ifdef PACK_BM_VALUES
                          #ifndef PF_BM_PREV_VAL
                          #ifndef PF_BM_PREV_HALF_VAL
                          #ifndef PF_BM_PSPLIT_VAL
                          #ifndef PF_BM_NEXT_HALF_VAL
                          #ifndef PF_BM_NEXT_VAL
                //Word_t wDiff = 0;
                if (1)
                          #endif // !PF_BM_NEXT_VAL
                          #endif // !PF_BM_NEXT_HALF_VAL
                          #endif // !PF_BM_PSPLIT_VAL
                          #endif // !PF_BM_PREV_HALF_VAL
                          #endif // !PF_BM_PREV_VAL
                          #ifndef PF_BM_PREV_VAL
                          #ifndef PF_BM_PREV_HALF_VAL
                          #ifdef PF_BM_PSPLIT_VAL
                          #ifndef PF_BM_NEXT_HALF_VAL
                          #ifndef PF_BM_NEXT_VAL
                Word_t wDiff
                    = (Word_t)pwValue - ((Word_t)pcPrefetch & ~MSK(6));
                if (wDiff < 64)
                          #endif // #ifndef PF_BM_NEXT_VAL
                          #endif // #ifndef PF_BM_NEXT_HALF_VAL
                          #endif // PF_BM_PSPLIT_VAL
                          #endif // #ifndef PF_BM_PREV_HALF_VAL
                          #endif // #ifndef PF_BM_PREV_VAL
                          #ifndef PF_BM_PREV_VAL
                          #ifdef PF_BM_PREV_HALF_VAL
                          #ifndef PF_BM_PSPLIT_VAL
                          #ifdef PF_BM_NEXT_HALF_VAL
                          #ifndef PF_BM_NEXT_VAL
                Word_t wDiff
                    = (Word_t)pwValue - (((Word_t)pcPrefetch - 32) & ~MSK(6));
                if (wDiff < 128)
                          #endif // #ifndef PF_BM_NEXT_VAL
                          #endif // PF_BM_NEXT_HALF_VAL
                          #endif // #ifndef PF_BM_PSPLIT_VAL
                          #endif // PF_BM_PREV_HALF_VAL
                          #endif // #ifndef PF_BM_PREV_VAL
                          #ifdef PF_BM_PREV_VAL
                          #ifndef PF_BM_PREV_HALF_VAL
                          #ifdef PF_BM_PSPLIT_VAL
                          #ifndef PF_BM_NEXT_HALF_VAL
                          #ifdef PF_BM_NEXT_VAL
                Word_t wDiff
                    = (Word_t)pwValue - (((Word_t)pcPrefetch - 64) & ~MSK(6));
                if (wDiff < 192)
                          #endif // PF_BM_NEXT_VAL
                          #endif // #ifndef PF_BM_NEXT_HALF_VAL
                          #endif // PF_BM_PSPLIT_VAL
                          #endif // #ifndef PF_BM_PREV_HALF_VAL
                          #endif // PF_BM_PREV_VAL
                {
                    SMETRICS_HIT(++j__DirectHits);
                } else {
                    SMETRICS_NHIT(++j__NotDirectHits);
                }
                      #endif // PACK_BM_VALUES
                      #endif // LOOKUP
                return pwValue;
                  #endif // else BMLF_INTERLEAVE && !LOOKUP
              #else // (LOOKUP || INSERT) && B_JUDYL
                // Next doesn't get here. Count doesn't get here.
                // Remove doesn't get here.
                // Lookup and Insert don't get here for JudyL.
                  #ifdef NEXT_EMPTY
                      #ifdef _BMLF_BM_IN_LNX
                Word_t* pwBitmap = pwLnX;
                      #else // _BMLF_BM_IN_LNX
                Word_t* pwBitmap =
                    (cbEmbeddedBitmap && (nBLR <= cnLogBitsPerLink))
                        ? (Word_t*)pLn // Judy1 embedded bitmap.
                        : ((BmLeaf_t*)pwr)->bmlf_awBitmap;
                      #endif // else _BMLF_BM_IN_LNX
                int nBitNum = wKey & MSK(nBLR) & MSK(cnLogBitsPerWord);
                int nWordNum = (wKey & MSK(nBLR)) >> cnLogBitsPerWord;
                Word_t wBm = ~pwBitmap[nWordNum] & ~MSK(nBitNum);
                if ((cnBitsInD1 < cnLogBitsPerWord)
                    && (nBLR < cnLogBitsPerWord))
                {
                    wBm &= MSK(EXP(nBLR));
                }
                for (;;) {
                    if (wBm != 0) {
                        nBitNum = __builtin_ctzll(wBm);
                        *pwKey = (wKey & ~MSK(nBLR))
                               | (nWordNum << cnLogBitsPerWord) | nBitNum;
                        return Success;
                    }
                      #ifdef USE_BM2CNTS_FOR_NEXT_EMPTY
                      #if cn2dBmMaxWpkPercent != 0
                      #if cnWordsBm2Cnts != 0
                    // Check bitmap sub counts.
                    if (nBLR == cnBitsLeftAtDl2) {
                        for (int ii = (wKey & MSK(cnBitsLeftAtDl2))
                                        >> cnLogBmlfBitsPerCnt;
                                 ii < (int)EXP(cnBitsLeftAtDl2)
                                        >> cnLogBmlfBitsPerCnt;
                                 ++ii)
                        {
                            if (gpxBitmapCnts(qya, nBLR)[ii]
                                != EXP(cnLogBmlfBitsPerCnt))
                            {
                                goto notFull;
                            }
                        }
                        wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                        goto break_from_main_switch;
                    notFull:;
                    }
                      #endif // cnWordsBm2Cnts != 0
                      #endif // cn2dBmMaxWpkPercent != 0
                      #endif // USE_BM2CNTS_FOR_NEXT_EMPTY
                    if (++nWordNum
                        >= (int)((cbEmbeddedBitmap
                                     && (nBLR <= cnLogBitsPerWord))
                           ? 0 : EXP(MAX(nBLR, cnLogBitsPerWord) - cnLogBitsPerWord)))
                    {
                        wKey = (wKey & ~MSK(nBLR)) + EXP(nBLR);
                        goto break_from_main_switch;
                    }
                    wBm = ~pwBitmap[nWordNum];
                }
                  #elif !defined(B_JUDYL) // NEXT_EMPTY
                return KeyFound; // LOOKUP || INSERT
                  #endif // NEXT_EMPTY else
              #endif // #else (LOOKUP || INSERT) && B_JUDYL
            }
            DBGX(printf("Bit is not set.\n"));
            IF_NEXT_EMPTY(*pwKey = wKey; return Success);
          #endif // LOOKUP && LOOKUP_NO_BITMAP_SEARCH else
        }
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
    #endif // HYPERTUNE_PF_BM && LOOKUP && B_JUDYL else
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
                  #ifdef PF_UNPACKED_BM_VAL
            PREFETCH(&pwBitmapValues[wKey & MSK(cnBitsInD1)]);
                  #endif // PF_UNPACKED_BM_VAL
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

    #define ZERO_CHECK  (wRoot == WROOT_NULL)

      #ifdef _AUG_TYPE_X_EK
        #define _AUG_TYPE_EK
      #elif defined(_AUG_TYPE_8_EK) // AUG_TYPE_64_LOOKUP && LOOKUP
        #define _AUG_TYPE_EK
      #endif // AUG_TYPE_64_LOOKUP && LOOKUP elif _AUG_TYPE_8_EK

      #ifdef _AUG_TYPE_EK

          #ifdef LOOKUP_NO_LIST_SEARCH
    #error
          #endif // LOOKUP_NO_LIST_SEARCH
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

    #define EK_WROOT_IS_NULL(_wRoot)  WROOT_IS_NULL(T_EMBEDDED_KEYS, (_wRoot))

          #ifdef NEXT_EMPTY
    #define T_EK_X(_nBL, _wRoot, _pwLnX, _wKey) goto t_embedded_keys;
          #else // NEXT_EMPTY
    #define T_EK_X(_nBL, _wRoot, _pwLnX, _wKey) \
        SMETRICS_EK_GUTS((_nBL), (_wRoot)); \
        if (EK_WROOT_IS_NULL((_wRoot))) { goto break_from_main_switch; } \
        if (EmbeddedListHasKey((_wRoot), (_wKey), (_nBL))) { \
            return BJL(pwLnX)BJ1(KeyFound); \
        } \
        goto break_from_main_switch
          #endif // NEXT_EMPTY

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
        if (wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS) {
            if (wRoot == WROOT_NULL) {
                IF_NEXT_EMPTY(*pwKey = wKey; return Success);
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
            if ((wRoot >> (cnBitsPerWord - nBL)) <= (wKey & MSK(nBL))) {
                ++wPopCntSum;
            }
          #else // B_JUDYL
            int nPos = SearchEmbeddedX(qya, wKey);
            wPopCntSum += nPos < 0 ? ~nPos : nPos + 1;
          #endif // B_JUDYL else
            DBGC(printf("EK: nPos %d wPopCntSum %zd\n", nPos, wPopCntSum));
            goto break_from_main_switch;
        }
      #endif // defined(COUNT)

      #ifdef LOOKUP
          #ifdef EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
        #define _PARALLEL_EK
          #endif // EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
      #elif defined(NEXT_EMPTY)
          #ifdef EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP
          #ifdef _ALL_DIGITS_ARE_8_BITS
          #ifndef USE_XX_SW
        #define _PARALLEL_EK
          #endif // !USE_XX_SW
          #endif // _ALL_DIGITS_ARE_8_BITS
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

      #ifdef NEXT_EMPTY
      #ifdef _PARALLEL_EK
        if (0)
      #endif // _PARALLEL_EK
        {
            goto findEmpty;
findEmpty:;
          #ifdef B_JUDYL
            if ((wRoot >> (cnBitsPerWord - nBL)) == (wKey & MSK(nBL))) {
                if ((++wKey & MSK(nBLR)) == 0) {
                    goto break_from_main_switch;
                }
            }
          #else // B_JUDYL
              #ifdef _PARALLEL_EK
            // A bogus match with the bits in the least significant slot might
            // cause problems.
            // 1. It might cause a bogus match with the next least significant
            // slot if HasKey uses the magic method.
            // 2. It might obfuscate a real match.
            // Looks like we might have a problem with this.
            // I think it assumes that there is an aligned slot at the least
            // significant end.
            // I wonder if _PARALLEL_EK makes the same assumption.
            Word_t wRootLoop = (wRoot & ~(Word_t)1) | ((wKey ^ 1) & 1);
            int nPos = LocateKey64((uint64_t*)&wRootLoop, wKey, nBL) - 1;
            // Already did EmbeddedListHasKey. We know wKey is present.
            assert(nPos >= 0);
              #else // _PARALLEL_EK
            int nPos = SearchEmbeddedX(qya, wKey);
            Word_t wRootLoop = wRoot;
            if (nPos >= 0)
              #endif // _PARALLEL_EK else
            {
                int nPopCntMax = EmbeddedListPopCntMax(nBL);
                wRootLoop >>= cnBitsPerWord - (nPopCntMax * nBL);
                do {
                    if ((++wKey & MSK(nBLR)) == 0) {
                        goto break_from_main_switch;
                    }
                } while ((++nPos != wr_nPopCnt(wRoot, nBL))
                    && ((((wRootLoop >> nPos * nBL) ^ wKey) & MSK(nBL)) == 0));
            }
          #endif // B_JUDYL else
            *pwKey = wKey;
            return Success;
        }
      #endif // NEXT_EMPTY

      #if defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)
        return wRoot ? Success : Failure;
      #endif // defined(LOOKUP) && defined(LOOKUP_NO_LIST_SEARCH)

        {

          #ifdef _PARALLEL_EK

  #ifdef NEXT_EMPTY
#define CASE_BLX(_nBL) \
        case (_nBL): \
            if (EmbeddedListHasKey(wRoot, wKey, (_nBL))) { goto findEmpty; } \
            *pwKey = wKey; \
            return Success
  #else // NEXT_EMPTY
#define CASE_BLX(_nBL) \
        case (_nBL): \
            if (EmbeddedListHasKey(wRoot, wKey, (_nBL))) { goto foundIt; } \
            goto break2
  #endif // NEXT_EMPTY else

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
        goto break2;
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
                // fall through
            case 7: // max for 8-bit keys and 64 bits;
                wKeyRoot = wRoot >> (cnBitsPerWord - (7 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                // fall through
            case 6: // max for 9-bit keys and 64 bits;
                wKeyRoot = wRoot >> (cnBitsPerWord - (6 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                // fall through
            case 5: // max for 10 to 11-bit keys and 64 bits;
                wKeyRoot = wRoot >> (cnBitsPerWord - (5 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                  #endif // (cnBitsPerWord == 64)
                // gcc doesn't like "fall through" before endif
                // fall through
            case 4: // max for 12 to 14-bit keys and 64 bits; 6 for 32
                wKeyRoot = wRoot >> (cnBitsPerWord - (4 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                // fall through
            case 3: // max for 15 to 19-bit keys and 64 bits; 7-9 for 32
                wKeyRoot = wRoot >> (cnBitsPerWord - (3 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                // fall through
            case 2: // max for 20 to 29-bit keys and 64 bits; 10-14 for 32
                wKeyRoot = wRoot >> (cnBitsPerWord - (2 * nBL));
                if (((wKeyRoot ^ wKey) & MSK(nBL)) == 0) goto foundIt;
                // fall through
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
        goto foundIt;
foundIt:
              #ifdef REMOVE
        goto removeGutsAndCleanup;
              #endif // REMOVE
              #ifdef INSERT
        if (nIncr > 0) { goto undo; } // undo counting
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
      #ifdef SMETRICS_EK_XV
        SMETRICS_GET(++j__GetCalls);
      #else // SMETRICS_EK_XV
        SMETRICS_GETN(++j__GetCallsNot);
      #endif // SMETRICS_EK_XV else
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
                Word_t wK = (*pwLnX >> (nn * nBits)) & MSK(nBits);
                if (wK >= (wKey & MSK(nBits))) {
                    nn += (wK == (wKey & MSK(nBits)));
                    break;
                }
            }
            DBGC(printf("EK_XV: wPopCntSum(before) %" _fw"d nn %d\n",
                        wPopCntSum, nn));
            wPopCntSum += nn;
            DBGC(printf("ek_xv nn %d wPopCntSum " OWx"\n", nn, wPopCntSum));
            goto break_from_main_switch;
        }
      #endif // defined(COUNT)
      #ifdef NEXT_EMPTY
        {
            while (SearchEmbeddedX(qya, wKey) >= 0) {
                if ((++wKey & MSK(nBLR)) == 0) {
                    goto break_from_main_switch;
                }
            }
            *pwKey = wKey;
            return Success;
        }
      #endif // NEXT_EMPTY
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

  #ifdef SMETRICS_EK_XV
#define SMETRICS_EK_XV_POP(x)  SMETRICS_POP(x)
  #else // SMETRICS_EK_XV
#define SMETRICS_EK_XV_POP(x)
  #endif // SMETRICS_EK_XV else

  #if (cnBitsInD1 < cnLogBitsPerByte)
#define XV_BLX(_nBL) \
        case (_nBL): \
            PF_2(_nBL); \
    SMETRICS_EK_XV_POP(j__SearchPopulation \
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
    SMETRICS_EK_XV_POP(j__SearchPopulation \
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
        if (nIncr > 0) { goto undo; } // undo counting
      #endif // defined(INSERT)
      #if defined(INSERT) || defined(LOOKUP)
          #ifdef SMETRICS_EK_XV
        // Was the prefetch of the value a hit?
        Word_t wEnd;
              #ifdef PF_EK_XV_2
        // We're doing a lot of work here even though we've set ourselves
        // up for a 100% hit ratio by default.
        // This test must be the same as in PF_2.
        if (EmbeddedListPopCntMax(nBL) > 2) {
            wEnd = (Word_t)(pwr + 16) & ~(Word_t)0x3f;
        } else
              #endif // PF_EK_XV_2
        { wEnd = (Word_t)(pwr + 8) & ~(Word_t)0x3f; }
        if ((Word_t)&pwr[nPos] < wEnd) {
            SMETRICS_HIT(++j__DirectHits);
        }
          #endif // SMETRICS_EK_XV
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
      #if defined(NEXT_EMPTY) || defined(PREV_EMPTY)
            *pwKey = wKey; return Success;
      #endif // NEXT_EMPTY || PREV_EMPTY

        goto break_from_main_switch;

    } // end of t_separate_t_null
  #endif // SEPARATE_T_NULL || (cwListPopCntMax == 0)

break_from_main_switch:;
    // Key is not present.
    DBGX(Checkpoint(qya, "break_from_main_switch"));
  #ifdef _RESTART_UP_FOR_NEXT_OR_EMPTY
    if (nBL < cnBitsPerWord) {
        goto tryNextDigit;
tryNextDigit:;
        DBGX(Checkpoint(qya, "tryNextDigit"));
        assert(nBL < cnBitsPerWord);
      #ifdef NEXT_EMPTY
        assert((wKey & MSK(nBL)) == 0);
      #else // NEXT_EMPTY
        Word_t wIncr = EXP(nBL);
        wKey &= ~(wIncr - 1);
        wKey += wIncr;
      #endif // NEXT_EMPTY
        DBGX(printf("# next digit wKey " OWx"\n", wKey));
        if ((nBLUp & MSK(cnLogBitsPerWord)) != 0) {
            // here nBLUp != cnBitsPerWord && nBLUp != 0
            if ((wKey & MSK(nBLRUp)) != 0) {
      #if defined(NEXT) && defined(_NEXT_SHORTCUT)
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
      #endif // NEXT && _NEXT_SHORTCUT
                goto restartUp; // restart at (nBLUp, pLnUp) with this new wKey
            }
        }
        if (wKey != 0) {
            goto restart; // restart at top with this new wKey
        }
    }
    IF_NEXT(return BJL(NULL)BJ1(Failure));
    IF_NEXT_EMPTY(return Failure);
  #elif defined(NEXT)
    if (nBL < cnBitsPerWord) {
        Word_t wIncr = EXP(nBL);
        wKey &= ~(wIncr - 1);
        wKey += wIncr;
        if (wKey != 0) {
            goto restart;
        }
    }
    return BJL(NULL)BJ1(Failure);
  #endif // _RESTART_UP_FOR_NEXT_OR_EMPTY
  #ifdef COUNT
      #ifdef COUNT_2
          #ifdef COUNT_2_PREFIX
    if (nPhaseCount < 2)
          #else // COUNT_2_PREFIX
    if (nPhaseCount == 0)
          #endif // COUNT_2_PREFIX else
    {
        DBGC(printf("Count wKey 0x%zx wKey0 0x%zx nPhase %d wPopCntSum %zd\n",
                    wKey, wKey0, nPhaseCount, wPopCntSum));
        if (wKey0 == 0) { return wPopCntSum; }
        wPopCntSum1 = wPopCntSum;
        wPopCntSum = 0;
        wKey = wKey0 - 1;
          #ifdef COUNT_2_PREFIX
        nBL = aqyaCount[0].q_nBL;
        pLn = aqyaCount[0].q_pLn;
              #ifdef _LNX
        pwLnX = aqyaCount[0].q_pwLnX;
              #endif // _LNX
          #else // COUNT_2_PREFIX
        nBL = cnBitsPerWord;
        pLn = pLnCount;
              #ifdef _LNX
        pwLnX = NULL;
              #endif // _LNX
          #endif // COUNT_2_PREFIX
        pwRoot = &pLn->ln_wRoot;
        wRoot = pLn->ln_wRoot;
          #ifdef COUNT_2_PREFIX
        nPhaseCount = 2;
          #else // COUNT_2_PREFIX
        nPhaseCount = 1;
          #endif // COUNT_2_PREFIX else
        goto top;
    }
    DBGC(printf("Count wKey 0x%zx wKey0 0x%zx nPhaseCount %d wPopCntSum %zd\n",
                wKey, wKey0, nPhaseCount, wPopCntSum));
    return wPopCntSum1 - wPopCntSum;
      #else // COUNT_2
    DBGC(printf("done wPopCntSum " OWx"\n", wPopCntSum));
    return wPopCntSum;
      #endif // COUNT_2
  #endif // COUNT
  #ifdef INSERT
      #ifdef BM_IN_LINK
    // If InsertGuts calls Insert, then it is always with the same
    // pLn and nBL that Insert passed to InsertGuts.
    assert((nBL != cnBitsPerWord) || (pLn == pLnOrig));
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
  #ifdef _RESTART_UP
    {
        goto restartUp;
restartUp:;
        DBGX(Checkpoint(qya, "restartUp"));
        DBGX(printf("nBLUp %d pLnUp %p\n", nBLUp, pLnUp));
        nBL = nBLUp;
        pLn = pLnUp;
        pwRoot = &pLnUp->ln_wRoot;
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
  #endif // _RESTART_UP
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
    if (nIncr < 0)
#endif // defined(REMOVE)
    {
  #if defined(INSERT) || defined(REMOVE)
        // Undo the counting we did on the way in.
        nIncr *= -1;
  #endif // defined(INSERT) || defined(REMOVE)
    goto restart;
restart:;
        DBGX(Checkpoint(qya, "restart"));
  #ifndef LOOKUP
  #ifndef COUNT
        nBL = nBLOrig;
        pLn = pLnOrig; // should we set pLnUp = NULL
        pwRoot = &pLnOrig->ln_wRoot;
          #ifdef _LNX
        pwLnX = pwLnXOrig;
          #endif // _LNX
// Work around bogus warning from gcc 11.4.0 in Ubuntu 22.04 with
// -DBM_IN_LINK -DDEBUG.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        wRoot = pLn->ln_wRoot;
#pragma GCC diagnostic pop
        goto top;
  #endif // !COUNT
  #endif // !LOOKUP
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
#undef InsertRemove
#undef DBGX
#undef strLookupOrInsertOrRemove
#undef KeyFound

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
    (void)PJError; // suppress "unused parameter" compiler warning
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
    // Doesn't the incomplete Link_t, i.e. only wRoot and no link extension,
    // complicate Lookup for other node types?
    int nType = wr_nType(wRoot);
    if ((nType == T_LIST) && ((WROOT_NULL != T_LIST) || (wRoot != 0)))
    {
        Word_t* pwRoot = (Word_t*)&pcvRoot;
        int nBL = cnBitsPerWord;
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
    (void)PJError; // suppress "unused parameter" compiler warning
    int nBL = cnBitsPerWord;
    Word_t* pwRoot = (Word_t*)ppvRoot;
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

    if (BJL(pwValue != 0)BJ1(status == Success)) {
  #ifdef B_JUDYL
        DBGI(printf("\n# After InsertL(wKey " OWx") Dump\n", wKey));
  #else // B_JUDYL
        DBGI(printf("\n# After Insert1(wKey " OWx") Dump\n", wKey));
  #endif // B_JUDYL else
        DBGI(Dump((Word_t *)ppvRoot, wKey, cnBitsPerWord));
        DBGI(printf("\n"));
    }

      #ifdef B_JUDYL
    //assert((JudyLCount(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal)
    //    || bPopCntTotalIsInvalid);
      #else // B_JUDYL
    //assert(Judy1Count(*ppvRoot, 0, (Word_t)-1, NULL) == wPopCntTotal);
      #endif // B_JUDYL

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
    (void)PJError; // suppress "unused parameter" compiler warnings
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
}

#endif // defined(REMOVE)

