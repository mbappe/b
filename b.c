
// @(#) $Id: b.c,v 1.263 2014/06/18 12:18:24 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/b.c,v $

#include "b.h"

#if defined(RAMMETRICS)
Word_t j__AllocWordsJBB;  // JUDYA         Branch Bitmap
Word_t j__AllocWordsJBU;  // JUDYA         Branch Uncompressed
Word_t j__AllocWordsJLB1; // JUDYA         Leaf Bitmap 1-Byte/Digit
Word_t j__AllocWordsJLL1; // JUDYA         Leaf Linear 1-Byte/Digit
Word_t j__AllocWordsJLL2; // JUDYA         Leaf Linear 2-Byte/Digit
Word_t j__AllocWordsJLL4; // JUDYA         Leaf Linear 4-Byte/Digit
Word_t j__AllocWordsJLLW; // JUDYA  JUDYB  Leaf Linear Word
Word_t j__AllocWordsJBU4; //        JUDYB  Branch Uncompressed 4-bit Digit
Word_t j__AllocWordsJL12; //        JUDYB  Leaf 12-bit Decode/Key
Word_t j__AllocWordsJL16; //        JUDYB  Leaf 16-bit Decode/Key
Word_t j__AllocWordsJL32; //        JUDYB  Leaf 32-bit Decode/Key
#endif // defined(RAMMETRICS)

// From Judy1LHTime.c for convenience.

#if 0

#ifdef JUDYA
Word_t j__AllocWordsJBL;  // Branch Linear
Word_t j__AllocWordsJLL3; // Leaf Linear 3-Byte/Digit
Word_t j__AllocWordsJLL5; // Leaf Linear 5-Byte/Digit
Word_t j__AllocWordsJLL6; // Leaf Linear 6-Byte/Digit
Word_t j__AllocWordsJLL7; // Leaf Linear 7-Byte/Digit
Word_t j__AllocWordsJV;   // Value Area
#endif // JUDYA  

#ifdef JUDYB
Word_t j__AllocWordsJBU8;  // Branch Uncompressed 8-bit Decode
Word_t j__AllocWordsJBU16; // Branch Uncompressed 16-bit Decode
Word_t j__AllocWordsJV12;  // Value Area 12-bit Decode
#endif // JUDYB   

#endif // 0

#if defined(BPD_TABLE)

const unsigned anDL_to_nBitsIndexSz[] = {
                    0, cnBitsAtBottom, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit, cnBitsPerDigit,
       cnBitsPerDigit
};

const unsigned anDL_to_nBL[] = {
    0,
    cnBitsAtBottom +  0 * cnBitsPerDigit,
    cnBitsAtBottom +  1 * cnBitsPerDigit,
    cnBitsAtBottom +  2 * cnBitsPerDigit,
    cnBitsAtBottom +  3 * cnBitsPerDigit,
    cnBitsAtBottom +  4 * cnBitsPerDigit,
    cnBitsAtBottom +  5 * cnBitsPerDigit,
    cnBitsAtBottom +  6 * cnBitsPerDigit,
    cnBitsAtBottom +  7 * cnBitsPerDigit,
    cnBitsAtBottom +  8 * cnBitsPerDigit,
    cnBitsAtBottom +  9 * cnBitsPerDigit,
    cnBitsAtBottom + 10 * cnBitsPerDigit,
    cnBitsAtBottom + 11 * cnBitsPerDigit,
    cnBitsAtBottom + 12 * cnBitsPerDigit,
    cnBitsAtBottom + 13 * cnBitsPerDigit,
    cnBitsAtBottom + 14 * cnBitsPerDigit,
    cnBitsAtBottom + 15 * cnBitsPerDigit,
    cnBitsAtBottom + 16 * cnBitsPerDigit,
    cnBitsAtBottom + 17 * cnBitsPerDigit,
    cnBitsAtBottom + 18 * cnBitsPerDigit,
    cnBitsAtBottom + 19 * cnBitsPerDigit,
    cnBitsAtBottom + 20 * cnBitsPerDigit,
    cnBitsAtBottom + 21 * cnBitsPerDigit,
    cnBitsAtBottom + 22 * cnBitsPerDigit,
    cnBitsAtBottom + 23 * cnBitsPerDigit,
    cnBitsAtBottom + 24 * cnBitsPerDigit,
    cnBitsAtBottom + 25 * cnBitsPerDigit,
    cnBitsAtBottom + 26 * cnBitsPerDigit,
    cnBitsAtBottom + 27 * cnBitsPerDigit,
    cnBitsAtBottom + 28 * cnBitsPerDigit,
    cnBitsAtBottom + 29 * cnBitsPerDigit,
    cnBitsAtBottom + 30 * cnBitsPerDigit,
    cnBitsAtBottom + 31 * cnBitsPerDigit,
    cnBitsAtBottom + 32 * cnBitsPerDigit,
    cnBitsAtBottom + 33 * cnBitsPerDigit,
    cnBitsAtBottom + 34 * cnBitsPerDigit,
    cnBitsAtBottom + 35 * cnBitsPerDigit,
    cnBitsAtBottom + 36 * cnBitsPerDigit,
    cnBitsAtBottom + 37 * cnBitsPerDigit,
    cnBitsAtBottom + 38 * cnBitsPerDigit,
    cnBitsAtBottom + 39 * cnBitsPerDigit,
    cnBitsAtBottom + 40 * cnBitsPerDigit,
    cnBitsAtBottom + 41 * cnBitsPerDigit,
    cnBitsAtBottom + 42 * cnBitsPerDigit,
    cnBitsAtBottom + 43 * cnBitsPerDigit,
    cnBitsAtBottom + 44 * cnBitsPerDigit,
    cnBitsAtBottom + 45 * cnBitsPerDigit,
    cnBitsAtBottom + 46 * cnBitsPerDigit,
    cnBitsAtBottom + 47 * cnBitsPerDigit,
    cnBitsAtBottom + 48 * cnBitsPerDigit,
    cnBitsAtBottom + 49 * cnBitsPerDigit,
    cnBitsAtBottom + 50 * cnBitsPerDigit,
    cnBitsAtBottom + 51 * cnBitsPerDigit,
    cnBitsAtBottom + 52 * cnBitsPerDigit,
    cnBitsAtBottom + 53 * cnBitsPerDigit,
    cnBitsAtBottom + 54 * cnBitsPerDigit,
    cnBitsAtBottom + 55 * cnBitsPerDigit,
    cnBitsAtBottom + 56 * cnBitsPerDigit,
    cnBitsAtBottom + 57 * cnBitsPerDigit,
    cnBitsAtBottom + 58 * cnBitsPerDigit,
    cnBitsAtBottom + 59 * cnBitsPerDigit,
    cnBitsAtBottom + 60 * cnBitsPerDigit,
    cnBitsAtBottom + 61 * cnBitsPerDigit,
    cnBitsAtBottom + 62 * cnBitsPerDigit,
    cnBitsAtBottom + 63 * cnBitsPerDigit,
};

#endif // defined(BPD_TABLE)

// Proposal for more generic names for the metrics.
//
// LB1 -- one-digit leaf bitmap
// LB2 -- two-digit leaf bitmap
// LB3 -- three-digit leaf bitmap
//
// LL1 -- one-byte leaf list
// LL2 -- two-byte leaf list
// LL3 -- three-byte leaf list
// LL4
// LL5
// LL6
// LL7
// LLW -- one-word leaf list
//
// BU1 -- one-digit uncompressed branch
// BU2 -- two-digit uncompressed branch
// BU3 -- three-digit uncompressed branch
//
// BB1 -- one-digit bitmap branch
// BB2 -- two-digit bitmap branch
// BL  -- linear/list branch
//
// V   -- JudyL value area (when separate from leaf)

#if (cnDigitsPerWord != 1)

static Word_t
MyMalloc(Word_t wWords)
{
    Word_t ww = JudyMalloc(wWords);
    DBGM(printf("\nM: "OWx" %"_fw"d words\n", ww, wWords));
    assert(ww != 0);
    assert((ww & cnMallocMask) == 0);
    return ww;
}

static void
MyFree(Word_t *pw, Word_t wWords)
{
    DBGM(printf("F: "OWx" %"_fw"d words\n", (Word_t)pw, wWords));
    JudyFree(pw, wWords);
}

#if (cwListPopCntMax != 0)

unsigned
ListWords(Word_t wPopCnt, unsigned nDL)
{
    unsigned nWords;

#if defined(COMPRESSED_LISTS)

    unsigned nBL = nDL_to_nBL(nDL);

    unsigned nBytesKeySz = (nBL <=  8) ? 1
                         : (nBL <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBL <= 32) ? 4
#endif // (cnBitsPerWord > 32)
                         : sizeof(Word_t);

#else // defined(COMPRESSED_LISTS)

    unsigned nBytesKeySz = sizeof(Word_t);

    (void)nDL;

#endif // defined(COMPRESSED_LISTS)

    if ((0 == wPopCnt)
#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
        || ((wPopCnt == 1)
            && (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1))
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
        )
    {
        return 0; // Use wRoot for the key.
    }

    // make room for pop count in the list
#if defined(T_ONE)
    if (wPopCnt > 1)
#endif // defined(T_ONE)
    {
#if defined(PP_IN_LINK)
	if (nDL == cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
        {
            ++wPopCnt;
        }
    }

    nWords = DIV_UP(wPopCnt * nBytesKeySz, sizeof(Word_t));

    nWords |= 1; // mallocs of an even number of words waste a word

    return nWords;
}

Word_t *
NewList(Word_t wPopCnt, unsigned nDL, Word_t wKey)
{
    (void)wKey;

    unsigned nWords = ListWords(wPopCnt, nDL);

    if (nWords == 0) { return NULL; }

#if defined(COMPRESSED_LISTS)
    unsigned nBL = nDL_to_nBL(nDL);

    unsigned nBytesKeySz = (nBL <=  8) ? 1
                         : (nBL <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBL <= 32) ? 4
#endif // (cnBitsPerWord > 32)
                         : sizeof(Word_t);
    if (nBytesKeySz == 1) {
        METRICS(j__AllocWordsJLL1 += nWords); // JUDYA
        METRICS(j__AllocWordsJL12 += nWords); // JUDYB -- overloaded
    } else if (nBytesKeySz == 2) {
        METRICS(j__AllocWordsJLL2 += nWords); // JUDYA
        METRICS(j__AllocWordsJL16 += nWords); // JUDYB
#if (cnBitsPerWord > 32)
    } else if (nBytesKeySz == 4) {
        METRICS(j__AllocWordsJLL4 += nWords); // JUDYA
        METRICS(j__AllocWordsJL32 += nWords); // JUDYB
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
        METRICS(j__AllocWordsJLLW += nWords); // JUDYA and JUDYB
    }

    Word_t *pwList;
#if defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    // this is overkill since we don't care if lists are aligned;
    // only that we don't cross a cache line boundary unnecessarily
    if (nBL <= 16) {
        posix_memalign((void **)&pwList, 64, nWords * sizeof(Word_t));
    } else
#endif // defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    {
        pwList = (Word_t *)MyMalloc(nWords);
    }

    DBGM(printf("NewList pwList %p wPopCnt "OWx" nWords %d\n",
        (void *)pwList, wPopCnt, nWords));

#if defined(T_ONE)
    if (wPopCnt > 1)
#endif // defined(T_ONE)
    {
#if defined(PP_IN_LINK)
        if (nDL == cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
        {
            set_ls_wPopCnt(pwList, wPopCnt);
        }
#if defined(DL_IN_LL)
        set_ll_nDL(pwList, nDL);
#endif // defined(DL_IN_LL)
    }

// Should we be setting wPrefix here for PP_IN_LINK?

    return pwList;
}

Word_t
OldList(Word_t *pwList, Word_t wPopCnt, unsigned nDL)
{
    unsigned nWords = ListWords(wPopCnt, nDL);

    DBGM(printf("Old pwList %p wLen %d wPopCnt "OWx"\n",
        (void *)pwList, nWords, (Word_t)wPopCnt));

    if (nWords == 0) { return 0; }

#if defined(DL_IN_LL)
    assert(nDL == ll_nDL(pwList));
#endif // defined(DL_IN_LL)

#if defined(COMPRESSED_LISTS)

    unsigned nBL = nDL_to_nBL(nDL);

    unsigned nBytesKeySz = (nBL <=  8) ? 1
                         : (nBL <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBL <= 32) ? 4
#endif // (cnBitsPerWord > 32)
                         : sizeof(Word_t);

    if (nBytesKeySz == 1) {
        METRICS(j__AllocWordsJLL1 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL12 -= nWords); // JUDYB -- overloaded
    } else if (nBytesKeySz == 2) {
        METRICS(j__AllocWordsJLL2 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL16 -= nWords); // JUDYB
#if (cnBitsPerWord > 32)
    } else if (nBytesKeySz == 4) {
        METRICS(j__AllocWordsJLL4 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL32 -= nWords); // JUDYB
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
        METRICS(j__AllocWordsJLLW -= nWords); // JUDYA and JUDYB
    }

#if defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    // this is overkill since we don't care if lists are aligned;
    // only that we don't cross a cache line boundary unnecessarily
    if (nBL <= 16) {
        free(pwList);
    } else
#endif // defined(COMPRESSED_LISTS) && defined(ALIGN_LISTS)
    {
        MyFree(pwList, nWords);
    }

    return nWords * sizeof(Word_t);
}

#endif // (cwListPopCntMax != 0)

#if (cnBitsAtBottom > cnLogBitsPerWord)

static Word_t *
NewBitmap(void)
{
    Word_t wWords = EXP(cnBitsAtBottom - cnLogBitsPerWord);

    Word_t w = MyMalloc(wWords);

    METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
    METRICS(j__AllocWordsJL12 += wWords); // JUDYB -- overloaded

    DBGM(printf("NewBitmap nBitsAtBottom %u nBits "OWx
      " nBytes "OWx" wWords "OWx" w "OWx"\n",
        cnBitsAtBottom, EXP(cnBitsAtBottom),
        EXP(cnBitsAtBottom - cnLogBitsPerByte), wWords, w));

    memset((void *)w, 0, wWords * sizeof(Word_t));

    return (Word_t *)w;
}

Word_t
OldBitmap(Word_t *pwr)
{
    Word_t wWords = EXP(cnBitsAtBottom - cnLogBitsPerWord);

    MyFree(pwr, wWords);

    METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
    METRICS(j__AllocWordsJL12 -= wWords); // JUDYB -- overloaded

    return wWords * sizeof(Word_t);
}

#endif // (cnBitsAtBottom > cnLogBitsPerWord)

// Allocate a new switch.
// Zero its links.
// Initialize its prefix if there is one.  Need to know nDLUp for
// PP_IN_LINK to figure out if the prefix exists.
// Initialize its bitmap if there is one.  Need to know nDLUp for
// BM_IN_LINK to figure out if the bitmap exists.
// Need to know nDLUp if TYPE_IS_RELATIVE to figure nDS.
// Install wRoot at pwRoot.  Need to know nDL.
// Account for the memory (for both JUDYA and JUDYB columns in Judy1LHTime).
// Need to know if we are at the bottom so we should count it as a bitmap.
static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, unsigned nDL,
          unsigned nDLUp, Word_t wPopCnt)
{
    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);

    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
    Word_t wIndexCnt = EXP(nBitsIndexSz);

#if defined(BM_SWITCH)
#if !defined(NDEBUG)
#if defined(BM_IN_LINK)
    { Link_t ln; assert(wIndexCnt <= sizeof(ln.ln_awBm) * cnBitsPerByte); }
#else // defined(BM_IN_LINK)
    { Switch_t sw; assert(wIndexCnt <= sizeof(sw.sw_awBm) * cnBitsPerByte); }
#endif // defined(BM_IN_LINK)
#endif // !defined(NDEBUG)
#endif // defined(BM_SWITCH)

    Word_t wLinks;
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    if (nDLUp != cnDigitsPerWord)
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if defined(BM_SWITCH_FOR_REAL)
    {
        wLinks = 1;
    }
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    else
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)
    {
        wLinks = wIndexCnt;
    }
#endif // ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)

    // sizeof(Switch_t) includes one link; add the others
    Word_t wWords
        = (sizeof(Switch_t) + (wLinks - 1) * sizeof(Link_t)) / sizeof(Word_t);

    Word_t *pwr = (Word_t *)MyMalloc(wWords);

    memset(pwr_pLinks(pwr), 0, wLinks * sizeof(Link_t));

#if defined(DUMMY_IN_SW)
    ((Switch_t *)pwr)->sw_dummy = 0;
#endif // defined(DUMMY_IN_SW)

#if defined(RAMMETRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDL <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        // embedded bitmap
        assert(nDL == nBL_to_nDL(cnBitsAtBottom) + 1); // later
        METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
        METRICS(j__AllocWordsJL12 += wWords); // JUDYB -- overloaded
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  += wWords); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  += wWords); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 += wWords); // JUDYB
    }
#endif // defined(RAMMETRICS)

    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx" nDL %d nDLU %d) pwr %p\n",
        (void *)pwRoot, wKey, nDL, nDLUp, (void *)pwr));

#if defined(TYPE_IS_RELATIVE)
    set_wr(*pwRoot, pwr, nDS_to_tp(nDLUp - nDL));
#else // defined(TYPE_IS_RELATIVE)
    set_wr(*pwRoot, pwr, nDL_to_tp(nDL));
#endif // defined(TYPE_IS_RELATIVE)

#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
    if (nDLUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
#if defined(BM_SWITCH_FOR_REAL)

        memset(PWR_pwBm(pwRoot, pwr), 0, sizeof(PWR_pwBm(pwRoot, pwr)));

        unsigned nBL = nDL_to_nBL_NAT(nDL);
        // leave nBL greater than cnBitsPerWord intentionally for now

        Word_t wIndex
            = (wKey >> (nBL - nBitsIndexSz)) & (wIndexCnt - 1);

        SetBit(PWR_pwBm(pwRoot, pwr), wIndex);

#else // defined(BM_SWITCH_FOR_REAL)

        // Mind the high-order bits of the bitmap word if/when the bitmap
        // is smaller than a whole word.
        // Mind endianness.
        if (nBitsIndexSz < cnLogBitsPerWord)
        {
            *PWR_pwBm(pwRoot, pwr) = EXP(wIndexCnt) - 1;
        }
        else
        {
            memset(PWR_pwBm(pwRoot, pwr), -1, sizeof(PWR_pwBm(pwRoot, pwr)));
        }

#endif // defined(BM_SWITCH_FOR_REAL)
    }
#endif // defined(BM_SWITCH)

#if defined(PP_IN_LINK)
    if (nDLUp < cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
    {
#if defined(SKIP_LINKS)
        assert(nDL <= nDLUp);

#if defined(NO_UNNECESSARY_PREFIX)
        // If defined(SKIP_PREFIX_CHECK) then we may need the prefix at the
        // leaf even if there is no skip directly to the leaf.  Why?  Because
        // there may have been a skip somewhere above and we do the prefix
        // check at the leaf.
        // If defined(SAVE_PREFIX) then we save a pointer to the prefix at
        // the place where the skip occurred so there is no need to have the
        // prefix at the leaf unless the skip goes directly to the leaf.
        // List leaves only need a prefix check if there is a skip link if
        // the keys in the list are less than whole words.  For now we set
        // prefix if we're at any compressed list level even if it isn't
        // necessary.
        // Bits in a bitmap leaf are always less than whole words and always
        // need the prefix if there is a skip link and we skip the check
        // at the point of skip and we don't save a pointer to the prefix.
        // Does it mean we'd have to add the prefix when transitioning
        // from full word list directly to bitmap?  Do we ever do this?
        if ((nDL == nDLUp)
#if defined(SKIP_PREFIX_CHECK)
#if ! defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            && (nDL_to_nBL_NAT(nDL - 1) > 32)
#else // (cnBitsPerWord > 32)
            && (nDL_to_nBL_NAT(nDL - 1) > 16)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            && ((nDL - 1) > nBL_to_nDL(cnBitsAtBottom))
#endif // ! defined(PP_IN_LINK)
#endif // defined(SKIP_PREFIX_CHECK)
            && 1)
        {
            DBGI(printf(
              "Not installing prefix left %d up %d wKey "OWx"\n",
                nDL, nDLUp, wKey));

            set_PWR_wPrefix(pwRoot, pwr, nDL, 0);
        }
        else
#endif // defined(NO_UNNECESSARY_PREFIX)
        {
            set_PWR_wPrefix(pwRoot, pwr, nDL, wKey);
        }
#else // defined(SKIP_LINKS)
        // Why do we bother with this?  Should we make it debug only?
        set_PWR_wPrefix(pwRoot, pwr, nDL, 0);
#endif // defined(SKIP_LINKS)

        set_PWR_wPopCnt(pwRoot, pwr, nDL, wPopCnt);

        DBGM(printf("NewSwitch PWR_wPrefixPop "OWx"\n",
            PWR_wPrefixPop(pwRoot, pwr)));
    }

    DBGI(printf("After NewSwitch"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nDL; // nDL is not used for all ifdef combos
    (void)nDLUp; // nDLUp is not used for all ifdef combos
}

#if defined(BM_SWITCH_FOR_REAL)
static void
NewLink(Word_t *pwRoot, Word_t wKey, unsigned nDL)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    DBGI(printf("NewLink(pwRoot %p wKey "OWx" nDL %d)\n",
        (void *)pwRoot, wKey, nDL));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, pwr, nDL)));

#if defined(BM_IN_LINK)
    assert(nDL != cnDigitsPerWord);
#endif // defined(BM_IN_LINK)

    // How many links are there in the old switch?
    Word_t wPopCnt = 0;
    for (unsigned nn = 0;
         nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDL)), cnBitsPerWord);
         nn++)
    {
        wPopCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
    // Now we know how many links were in the old switch.

    // Allocate memory for a new switch with one more link than the old one.
    // Update *pwRoot.

    // sizeof(Switch_t) includes one link; add the others
    unsigned nWords
        = (sizeof(Switch_t) + wPopCnt * sizeof(Link_t)) / sizeof(Word_t);
    DBGI(printf("link count %"_fw"d nWords %d\n", wPopCnt, nWords));
    *pwRoot = MyMalloc(nWords);
    DBGI(printf("After malloc *pwRoot "OWx"\n", *pwRoot));

#if defined(RAMMETRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDL <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        // embedded bitmap
        assert(nDL == nBL_to_nDL(cnBitsAtBottom) + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(Link_t)/sizeof(Word_t)); // JUDYA
        METRICS(j__AllocWordsJL12 += sizeof(Link_t)/sizeof(Word_t)); // JUDYB
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  += sizeof(Link_t)/sizeof(Word_t)); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  += sizeof(Link_t)/sizeof(Word_t)); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 += sizeof(Link_t)/sizeof(Word_t)); // JUDYB
    }
#endif // defined(RAMMETRICS)

    // Where does the new link go?
    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
    unsigned nBL = nDL_to_nBL(nDL);
    Word_t wIndex
        = ((wKey >> (nBL - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1));
    DBGI(printf("wIndex "OWx"\n", wIndex));
    unsigned nBmOffset = wIndex >> cnLogBitsPerWord;
    Word_t wBm = PWR_pwBm(pwRoot, pwr)[nBmOffset];
    Word_t wBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
    assert( ! (wBm & wBit) );
    Word_t wBmMask = wBit - 1;
    // recalculate index as link number in sparse array of links
    wIndex = 0;
    for (unsigned nn = 0; nn < nBmOffset; nn++)
    {
        wIndex += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
    wIndex += __builtin_popcountll(wBm & wBmMask);
    // Now we know where the new link goes.
    DBGI(printf("wIndex "OWx"\n", wIndex));

    // Copy the old switch to the new switch and insert the new link.
    memcpy(wr_pwr(*pwRoot), pwr,
        sizeof(Switch_t) + (wIndex - 1) * sizeof(Link_t));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDL)));
    // Initialize the new link.
    DBGI(printf("pLinks %p\n", (void *)pwr_pLinks(*pwRoot)));
    DBGI(printf("memset %p\n", (void *)&pwr_pLinks(*pwRoot)[wIndex]));
    memset(&pwr_pLinks(*pwRoot)[wIndex], 0, sizeof(Link_t));
    DBGI(printf("PWR_wPopCnt A %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDL)));
    memcpy(&pwr_pLinks(*pwRoot)[wIndex + 1], &pwr_pLinks(pwr)[wIndex],
        (wPopCnt - wIndex) * sizeof(Link_t));

    DBGI(printf("PWR_wPopCnt B %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDL)));
    // Set the bit in the bitmap indicating that the new link exists.
    SetBit(PWR_pwBm(pwRoot, *pwRoot),
        ((wKey >> (nBL - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1)));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDL)));

    MyFree(pwr, nWords - sizeof(Link_t) / sizeof(Word_t));

    // Caller updates type field in *pwRoot if necessary.

    DBGI(printf("After NewLink"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SWITCH_FOR_REAL)

static Word_t
OldSwitch(Word_t *pwRoot, unsigned nDL, unsigned nDLUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    Word_t wLinks;
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    if (nDLUp != cnDigitsPerWord)
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if defined(BM_SWITCH_FOR_REAL)
    {
        // How many links are there in the old switch?
        wLinks = 0;
        for (unsigned nn = 0;
            nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDL)), cnBitsPerWord);
            nn++)
        {
            wLinks += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
        }
        assert(wLinks <= EXP(nDL_to_nBitsIndexSz(nDL)));
        // Now we know how many links were in the old switch.
    }
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    else
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)
    {
        wLinks = EXP(nDL_to_nBitsIndexSz(nDL));
    }
#endif // ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)

    // sizeof(Switch_t) includes one link; add the others
    Word_t wWords
        = (sizeof(Switch_t) + (wLinks - 1) * sizeof(Link_t)) / sizeof(Word_t);

#if defined(RAMMETRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDL <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        assert(nDL == nBL_to_nDL(cnBitsAtBottom) + 1); // later
        METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
        METRICS(j__AllocWordsJL12 -= wWords); // JUDYB -- overloaded
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  -= wWords); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  -= wWords); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 -= wWords); // JUDYB
    }
#endif // defined(RAMMETRICS)

    DBGR(printf("\nOldSwitch nDL %d nDLU %d wWords %"_fw"d "OWx"\n",
         nDL, nDLUp, wWords, wWords));

    MyFree(pwr, wWords);

    return wWords * sizeof(Word_t);

    (void)nDL; // silence compiler
    (void)nDLUp; // silence compiler
}

Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, unsigned nBL, int bDump)
{
    Word_t *pwRootArg = pwRoot;
#if defined(BM_IN_LINK) || defined(PP_IN_LINK)
    unsigned nBLArg = nBL;
#endif // defined(BM_IN_LINK) || defined(PP_IN_LINK)
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    unsigned nDL = nBL_to_nDL(nBL);
    unsigned nBitsIndexSz;
    Link_t *pLinks;
    Word_t wBytes = 0;

    assert(nBL >= cnBitsAtBottom);

    if ( ! bDump )
    {
        DBGR(printf("FreeArrayGuts pwR "OWx" wPrefix "OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBL, bDump));
        DBGR(printf("wRoot "OWx"\n", wRoot));
    }

    if (wRoot == 0)
    {
#if defined(BM_SWITCH_FOR_REAL)
        if (bDump)
        {
            printf(" wPrefix "OWx, wPrefix);
            printf(" nBL %2d", nBL);
            printf(" pwRoot "OWx, (Word_t)pwRoot);
            printf(" wr "OWx, wRoot);
            printf("\n");
        }
#endif // defined(BM_SWITCH_FOR_REAL)
        return 0;
    }

    if (bDump)
    {
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefix "OWx, wPrefix);
        printf(" nBL %2d", nBL);
        printf(" pwRoot "OWx, (Word_t)pwRoot);
        printf(" wr "OWx, wRoot);
    }

    if (nBL <= cnBitsAtBottom)
    {
#if defined(PP_IN_LINK)
        if (bDump)
        {
            assert(nBLArg != cnBitsPerWord);
 
            printf(" wr_wPopCnt %3"_fw"u",
                   PWR_wPopCnt(pwRoot, NULL, nDL));
            printf(" wr_wPrefix "OWx,
                   PWR_wPrefix(pwRoot, NULL, nDL));
        }
#endif // defined(PP_IN_LINK)

        // If the bitmap is not embedded, then we have more work to do.
#if (cnBitsAtBottom > cnLogBitsPerWord)

        if (!bDump)
        {
            return OldBitmap(pwr);
        }

        printf(" nWords %2"_fw"d", EXP(cnBitsAtBottom) / cnBitsPerWord);
        for (Word_t ww = 0;
            (ww < EXP(cnBitsAtBottom - cnLogBitsPerWord));
             ww++)
        {
            if ((ww % 8) == 0) {
                printf("\n");
            }
            printf(" "Owx, pwr[ww]);
        }

#else // (cnBitsAtBottom > cnLogBitsPerWord)

        if (bDump) printf(" wr "OWx, wRoot);

#endif // (cnBitsAtBottom > cnLogBitsPerWord)

        if (bDump)
        {
            printf("\n");
        }

        return 0;
    }

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
    assert(nDL - tp_to_nDS(nType)
        >= nBL_to_nDL(cnBitsAtBottom));
#else // defined(TYPE_IS_RELATIVE)
    if (tp_to_nDL(nType) > nBL_to_nDL(nBL))
printf("nType %d nBL %d tpnDL %d nBLnDL %d\n",
    nType, nBL, tp_to_nDL(nType), nBL_to_nDL(nBL));

    assert( ! tp_bIsSwitch(nType) || (tp_to_nDL(nType) <= nBL_to_nDL(nBL)));
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(T_ONE)

        if (nType == T_ONE)
        {
#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
            if (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1) {
                wPopCnt = (wRoot >> 4) & (EXP(LOG(119 / nBL)) - 1);
                if (wPopCnt == 0) { wPopCnt = 1; }
            } else
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
            {
                wPopCnt = 1;
            }

            if (!bDump)
            {
                // This OldList is a no-op and will return zero if
                // the key(s) is(are) embedded.
                return OldList(pwr, /* wPopCnt */ 1, nDL);
            }

            printf(" tp_wPopCnt %3d", (int)wPopCnt);

#if defined(PP_IN_LINK)
            assert(nBL == nBLArg);
            if (nBLArg == cnBitsPerWord) {
                printf(" wr_wPrefix        N/A");
            } else {
                printf(" wr_wPrefix "OWx,
                       PWR_wPrefix(pwRoot, NULL, nDL));
            }
#endif // defined(PP_IN_LINK)

#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
            if (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1) {
                for (unsigned nn = 1; nn <= wPopCnt; nn++) {
                    printf(" %"_fw"x\n",
                        (wRoot >> (cnBitsPerWord - (nn * nBL)))
                            & (EXP(nBL) - 1));
                }
            } else
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
            {
                printf(" "Owx"\n", *pwr);
            }
        }
        else
#endif // defined(T_ONE)
        {
            assert(nType == T_LIST);

#if defined(PP_IN_LINK)
            if (nDL != cnDigitsPerWord)
            {
                wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL);
            }
            else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = ls_wPopCnt(pwr);
            }

            if (!bDump)
            {
                return OldList(pwr, wPopCnt, nDL);
            }

            Word_t *pwKeys = pwr_pwKeys(pwr);

#if defined(PP_IN_LINK)
            if (nBLArg == cnBitsPerWord)
            {
                printf(" ls_wPopCnt %3"_fw"u", wPopCnt);
                printf(" wr_wPrefix        N/A");
            }
            else
            {
                printf(" wr_wPopCnt %3"_fw"u", wPopCnt);
                printf(" wr_wPrefix "OWx,
                       PWR_wPrefix(pwRoot, NULL, nDL));
            }
#endif // defined(PP_IN_LINK)

            printf(" ls_wPopCnt %3llu", (unsigned long long)wPopCnt);

            for (unsigned nn = 0;
                //(nn < wPopCnt) && (nn < 8);
                (nn < wPopCnt);
                 nn++)
            {
                unsigned xx = nn;
#if defined(PP_IN_LINK)
                xx += (nBLArg == cnBitsPerWord);
#endif // defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    printf(" %02x", ls_pcKeys(pwr)[xx]);
                } else if (nBL <= 16) {
                    printf(" %04x", ls_psKeys(pwr)[xx]);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    printf(" %08x", ls_piKeys(pwr)[xx]);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                { printf(" "Owx, pwKeys[xx]); }
            }
            printf("\n");
        }

        return 0;
    }
#endif // (cwListPopCntMax != 0)

    // Switch

    unsigned nDLPrev = nDL;
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
    nDL = nDL - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
    nDL = tp_to_nDL(nType);
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    nBL = nDL_to_nBL(nDL);

    nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
    pLinks = pwr_pLinks(pwr);

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBLArg == cnBitsPerWord)
        {
// Add 'em up.
#if defined(BM_SWITCH) && ! defined(BM_IN_LINK)
            Word_t xx = 0;
#endif // defined(BM_SWITCH) && ! defined(BM_IN_LINK)
            Word_t wPopCnt = 0;
            for (unsigned nn = 0; nn < EXP(cnBitsIndexSzAtTop); nn++)
            {
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
        if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
        {
#if defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[xx].ln_wRoot;
                xx++;
#else // defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[nn].ln_wRoot;
#endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)

// *pwRootLn may not be a pointer to a switch
// It may be a pointer to a list leaf.
// And if nBL_to_nDL(cnBitsAtBottom) == cnDigitsPerWord - 1, then it could be
// a pointer to a bitmap?
                Word_t wPopCntLn;
#if defined(SKIP_LINKS)
                unsigned nTypeLn = wr_nType(*pwRootLn);
                if (tp_bIsSwitch(nTypeLn))
                {
                    wPopCntLn
#if defined(TYPE_IS_RELATIVE)
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      nDL - wr_nDS(*pwRootLn));
#else // defined(TYPE_IS_RELATIVE)
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      wr_nDL(*pwRootLn));
#endif // defined(TYPE_IS_RELATIVE)
                }
                else
#endif // defined(SKIP_LINKS)
                {
                    wPopCntLn
                        = PWR_wPopCnt(pwRootLn, NULL, cnDigitsPerWord - 1);
                }

                wPopCnt += wPopCntLn;

                // We use pwr_pLinks(pwr)[nn].ln_wRoot != 0 to disambiguate
                // wPopCnt == 0.  Hence we cannot allow Remove to leave
                // pwr_pLinks(pwr)[nn].ln_wRoot != 0 unless the actual
                // population count is not zero.
                if ((wPopCntLn == 0) && (*pwRootLn != 0))
                {
// Why are we checking cwListPopCntMax here?
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
                    wPopCnt += 1
                            + wPrefixPopMask(nDL - wr_nDS(*pwRootLn));
#else // defined(TYPE_IS_RELATIVE)
                    wPopCnt += wPrefixPopMask(wr_nDL(*pwRootLn)) + 1;
#endif // defined(TYPE_IS_RELATIVE)
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    wPopCnt += wPrefixPopMask(cnDigitsPerWord - 1) + 1;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                }
        }
            }

            printf(" sm_wPopCnt %3"_fw"u", wPopCnt);
            printf(" wr_wPrefix        N/A");
        }
        else
#endif // defined(PP_IN_LINK)
        {
            printf(" wr_wPopCnt %3"_fw"u",
                PWR_wPopCnt(pwRoot, pwr, nDL));
            printf(" wr_wPrefix "OWx, PWR_wPrefix(pwRoot, pwr, nDL));
        }

        printf(" wr_nDL %2d", nDL);
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask "OWx, wPrefixPopMask(nDL));
        //printf(" pLinks "OWx, (Word_t)pLinks);
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if (nBLArg != cnBitsPerWord)
#endif // defined(BM_IN_LINK)
        {
            printf(" Bm");
            // Bitmaps are an integral number of words.
            for (unsigned nn = 0;
                 nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDL)),
                             cnBitsPerWord);
                 nn ++)
            {
                if ((nn % 8) == 0) {
                    printf("\n");
                }
                printf(" "Owx, PWR_pwBm(pwRoot, pwr)[nn]);
            }
        }
#endif // defined(BM_SWITCH)
        printf("\n");
    }

    nBL = nDL_to_nBL(nDL - 1);
    nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);

    // skip link has extra prefix bits
    if (nDLPrev > nDL)
    {
        wPrefix = PWR_wPrefix(pwRoot, pwr, nDL);
    }

    Word_t xx = 0;
    for (Word_t nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if ((nBLArg == cnBitsPerWord)
            || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#else // defined(BM_IN_LINK)
        if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(BM_IN_LINK)
#endif // defined(BM_SWITCH)
        {
            if (pLinks[xx].ln_wRoot != 0)
            {
                //printf("nn %"_fw"x\n", nn);
                wBytes += FreeArrayGuts(&pLinks[xx].ln_wRoot,
                        wPrefix | (nn << nBL), nBL, bDump);
            }

            xx++;
        }
    }

    if (bDump) return 0;

    // Someone has to clear PP and BM if PP_IN_LINK and BM_IN_LINK.
    // OldSwitch looks at BM.

    wBytes += OldSwitch(pwRootArg, nDL, nDLPrev);

    DBGR(printf("memset(%p, 0, %zd)\n",
         (void *)STRUCT_OF(pwRootArg, Link_t, ln_wRoot), sizeof(Link_t)));

#if defined(PP_IN_LINK) || defined(BM_IN_LINK)
    if (nBLArg == cnBitsPerWord)
    {
        *pwRootArg = 0;
    }
    else
#endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)
    {
        memset(STRUCT_OF(pwRootArg, Link_t, ln_wRoot), 0, sizeof(Link_t));
    }

    return wBytes;
}

#if defined(DEBUG)
void
Dump(Word_t *pwRoot, Word_t wPrefix, unsigned nBL)
{
    printf("\nDump\n");
    FreeArrayGuts(pwRoot, wPrefix, nBL, /* bDump */ 1);
    printf("End Dump\n");
}
#endif // defined(DEBUG)

#if (cwListPopCntMax != 0)

#if defined(COMPRESSED_LISTS) && ((cnBitsAtBottom + cnBitsPerDigit) <= 16)
static Status_t
SearchList16(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
    unsigned short *psKeys = pwr_psKeys(pwr);
    unsigned short sKey = wKey;
    unsigned short sKeyLoop;
      #if defined(SORT_LISTS)
        #if defined(LINEAR_SEARCH)
    sKeyLoop = *psKeys;
    if (nPopCnt != 1)
    {
        unsigned short *psLastKey = &psKeys[nPopCnt-1];
        while (sKeyLoop < sKey) {
            sKeyLoop = *++psKeys;
            if (psKeys == psLastKey) {
                break;
            }
        }
    }
        #else // defined(LINEAR_SEARCH)
          #if defined(SPLIT_SEARCH) \
                  && (cnSplitSearchThresholdShort > 1)
              #if defined(SPLIT_SEARCH_LOOP)
    while
              #else // defined(SPLIT_SEARCH_LOOP)
    if
              #endif // defined(SPLIT_SEARCH_LOOP)
       (nPopCnt >= cnSplitSearchThresholdShort)
    {
// To do: Try to minimize the number of cache lines we hit.
// If ! PP_IN_LINK then we already hit the first one to get the pop count.
// Let's try aligning these lists.
        // pick a starting point
              #if defined(BINARY_SEARCH) \
                  || defined(SPLIT_SEARCH_LOOP)
        unsigned nSplit = nPopCnt / 2;
              #else // defined(BINARY_SEARCH) || ...
        unsigned nSplit
            = wKey % EXP(nBL) * nPopCnt / EXP(nBL);
              #endif // defined(BINARY_SEARCH) || ...
        if (psKeys[nSplit] <= sKey) {
            psKeys = &psKeys[nSplit];
            nPopCnt -= nSplit;
// To do: Shouldn't we go backwards if we exit the loop after this step?
// It might be very important.
// What about cache line alignment?
        } else {
            nPopCnt = nSplit;
            goto loop;
        }
    }
          #endif // defined(SPLIT_SEARCH) && ...
    if ((sKeyLoop = psKeys[nPopCnt - 1]) > sKey)
    {
          #if defined(SPLIT_SEARCH) \
                  && (cnSplitSearchThresholdShort > 1)
loop:
          #endif // defined(SPLIT_SEARCH) && ...
        while ((sKeyLoop = *psKeys++) < sKey);
    }
        #endif // defined(LINEAR_SEARCH)
      #else // defined(SORT_LISTS)
    unsigned short *psKeysEnd = &psKeys[nPopCnt];
    while (sKeyLoop = *psKeys, psKeys++ < psKeysEnd)
      #endif // defined(SORT_LISTS)
    {
        if (sKeyLoop == sKey)
        {
            return Success;
        }
    }

    return Failure;
}
#endif // defined(COMPRESSED_LISTS) && ...

#if defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) \
    && ((cnBitsAtBottom + cnBitsPerDigit) <= 32)
static Status_t
SearchList32(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
    unsigned int *piKeys = pwr_piKeys(pwr);
    unsigned int iKey = wKey;
    unsigned int iKeyLoop;
      #if defined(SORT_LISTS)
          #if defined(SPLIT_SEARCH)
              #if defined(SPLIT_SEARCH_LOOP)
    while
              #else // defined(SPLIT_SEARCH_LOOP)
    if
              #endif // defined(SPLIT_SEARCH_LOOP)
       (nPopCnt >= cnSplitSearchThresholdInt)
    {
        if (piKeys[nPopCnt / 2] <= iKey) {
            piKeys = &piKeys[nPopCnt / 2];
            nPopCnt -= nPopCnt / 2;
        } else {
            nPopCnt /= 2;
        }
    }
          #endif // defined(SPLIT_SEARCH)
    if ((iKeyLoop = piKeys[nPopCnt - 1]) > iKey)
    {
        while ((iKeyLoop = *piKeys++) < iKey);
    }
      #else // defined(SORT_LISTS)
    unsigned int *piKeysEnd = &piKeys[nPopCnt];
    while (iKeyLoop = *piKeys, piKeys++ < piKeysEnd)
      #endif // defined(SORT_LISTS)
    {
        if (iKeyLoop == iKey)
        {
            return Success;
        }
    }

    return Failure;
}
#endif // defined(COMPRESSED_LISTS) && (cnBitsPerWord > 32) && ...

static Status_t
SearchListWord(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
    (void)nBL;
        // Looks like we might want a loop threshold of 8 for
        // 64-bit keys at the top level.
        // And there's not much difference with threshold of
        // 16 or 32.
        // Not sure about 64-bit
        // keys at a lower level or 32-bit keys at the top level.
    Word_t *pwKeys = pwr_pwKeys(pwr);
    Word_t wKeyLoop;
  #if defined(SORT_LISTS)
  #if defined(SPLIT_SEARCH)
      #if defined(SPLIT_SEARCH_LOOP)
    while
      #else // defined(SPLIT_SEARCH_LOOP)
    if
      #endif // defined(SPLIT_SEARCH_LOOP)
       (nPopCnt >= cnSplitSearchThresholdWord)
    {
        if (pwKeys[nPopCnt / 2] <= wKey) {
            pwKeys = &pwKeys[nPopCnt / 2];
            nPopCnt -= nPopCnt / 2;
        } else {
            nPopCnt /= 2;
        }
    }
  #endif // defined(SPLIT_SEARCH)
  #if ! defined(SPLIT_SEARCH_LOOP) \
      || (cnSplitSearchThresholdWord > 2)
    if ((wKeyLoop = pwKeys[nPopCnt - 1]) > wKey) {
        while ((wKeyLoop = *pwKeys++) < wKey);
    }
  #else // ! defined(SPLIT_SEARCH_LOOP) || ...
    wKeyLoop = *pwKeys;
  #endif // ! defined(SPLIT_SEARCH_LOOP) || ...
  #else // defined(SORT_LISTS)
    Word_t *pwKeysEnd = &pwKeys[nPopCnt];
    while (wKeyLoop = *pwKeys, pwKeys++ < pwKeysEnd)
  #endif // defined(SORT_LISTS)
    {
        if (wKeyLoop == wKey)
        {
            return Success;
        }
    }

    return Failure;
}

#ifdef  TBD

#define repbyte1(s) (((-((Word_t)1))/255) * (s))
#define haszero1(v) (((v) - repbyte1(0x1)) & ~(v) & repbyte1(0x80))
#define hasvalue1(x,n) haszero1((x) ^ (n))

// For finding a zero byte in a Word_t may be usefull someday
// Note: untested
// #define haszero(v)        (((v) - 0x01010101UL) & ~(v) & 0x80808080UL)

// Does Word x have a byte == v
// #define hasvalue(x,n)     (haszero((x) ^ (~0UL/255 * (n))))

// For finding if Word_t has a byte == VALUE
#define haszero(VALUE)                                                  \
    ((VALUE) ((-((Word_t)1)) * 255) & ~(v) & (-((((Word_t)1) * 255) * 8)))

#define hasvalue(x,n) (haszero((x) ^ (-((Word_t)1))/255 * (n)))
#endif  // TBD

Status_t
SearchList(Word_t *pwr, Word_t wKey, unsigned nBL, unsigned nPopCnt)
{
  #if defined(COMPRESSED_LISTS)
      #if ((cnBitsAtBottom + cnBitsPerDigit) <= 16)
    if (nBL <= 16) {
        return SearchList16(pwr, wKey, nBL, nPopCnt);
    } else
      #endif // ((cnBitsAtBottom + cnBitsPerDigit) <= 16)
      #if ((cnBitsAtBottom + cnBitsPerDigit) <= 32) && (cnBitsPerWord > 32)
    if (nBL <= 32) {
        return SearchList32(pwr, wKey, nBL, nPopCnt);
    } else
      #endif // ((cnBitsAtBottom + cnBitsPerDigit) <= 32) && ...
  #endif // defined(COMPRESSED_LISTS)
    {
        return SearchListWord(pwr, wKey, nBL, nPopCnt);
    }
}

#if defined(SORT_LISTS)

// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsertWord(Word_t *pTgt, Word_t *pSrc, unsigned nKeys, Word_t wKey)
{
    DBGI(printf("\nCopyWithInsertWord(pTgt %p pSrc %p nKeys %d wKey "OWx"\n",
                pTgt, pSrc, nKeys, wKey));
#if (cwListPopCntMax != 0)
    Word_t aw[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    Word_t aw[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&aw[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = aw;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}

#if defined(COMPRESSED_LISTS)

#if (cnBitsPerWord > 32)
static void
CopyWithInsertInt(unsigned int *pTgt, unsigned int *pSrc,
    unsigned nKeys, unsigned int wKey)
{
    DBGI(printf("\nCopyWithInsertInt(pTgt %p pSrc %p nKeys %d wKey 0x%x\n",
                pTgt, pSrc, nKeys, wKey));
#if (cwListPopCntMax != 0)
    unsigned int ai[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned int ai[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&ai[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = ai;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}
#endif // (cnBitsPerWord > 32)

static void
CopyWithInsertShort(unsigned short *pTgt, unsigned short *pSrc,
    unsigned nKeys, unsigned short wKey)
{
#if (cwListPopCntMax != 0)
    unsigned short as[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned short as[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&as[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = as;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}

static void
CopyWithInsertChar(unsigned char *pTgt, unsigned char *pSrc,
    unsigned nKeys, unsigned short wKey)
{
#if (cwListPopCntMax != 0)
    unsigned char ac[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned char ac[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= wKey)
        {
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc)
    {
        COPY(pTgt, pSrc, n); // copy the head
    }
    else
    {
        COPY(&ac[n], &pSrc[n], nKeys - n); // save the tail
        pSrc = ac;
    }

    pTgt[n] = wKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
}

#endif // defined(COMPRESSED_LISTS)
#endif // defined(SORT_LISTS)
#endif // (cwListPopCntMax != 0)

Status_t
InsertAtBottom(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot);

// InsertGuts
// This function is called from the iterative Insert function once Insert has
// determined that the key from an insert request is not present.
// It is provided with a starting location for the insert.
// InsertGuts does whatever is necessary to insert the key into the array
// and returns back to Insert.
// This is where the main runtime policy decisions are made.
// Some are made in RemoveGuts, but those are closely aligned with
// the decisions made here.
// Do we create a list as high as possible or as low as possible?
// When do we create a new switch instead of adding to a list?
// When do we create a bitmap?
// When do we uncompress switches?
// When do we coalesce switches?
Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
    DBGI(printf("InsertGuts pwRoot %p wKey "OWx" nDL %d wRoot "OWx"\n",
               (void *)pwRoot, wKey, nDL, wRoot));

    assert(nDL >= 1); // for now; may use these values later

#if defined(T_ONE)
#if defined(COMPRESSED_LISTS)
    uint16_t asKeys[16], sKey; (void)asKeys;
    uint32_t aiKeys[4], iKey; (void)aiKeys;
#endif // defined(COMPRESSED_LISTS)
#endif // defined(T_ONE)

    // Would be nice to validate sanity of ifdefs here.  Or somewhere.
    assert(cnBitsAtBottom >= cnLogBitsPerWord);
#if defined(EMBED_KEYS) && ! defined(T_ONE)
    assert(0); // EMBED_KEYS requires T_ONE
#endif // defined(EMBED_KEYS) && ! defined(T_ONE)

    // Check to see if we're at the bottom before checking nType since
    // nType may be invalid if wRoot is an embedded bitmap.
    if (nDL == 1) {
        return InsertAtBottom(pwRoot, wKey, nDL, wRoot);
    }

    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc
    Word_t *pwr = wr_tp_pwr(wRoot, nType);

// This first clause handles wRoot == 0 by treating it like a list leaf
// with zero population (and no allocated memory).
// But why is it ok to skip the test for a switch if ! defined(SKIP_LINKS)
// and !defined(BM_SWITCH_FOR_REAL)?
// InsertGuts is called with a wRoot
// that points to a switch only for prefix mismatch or missing link cases.
#if defined(SKIP_LINKS) 
    if (!tp_bIsSwitch(nType))
#else // defined(SKIP_LINKS)
#if defined(BM_SWITCH_FOR_REAL)
#if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
#else // (cwListPopCntMax == 0)
    if (pwr == NULL)
#endif // (cwListPopCntMax == 0)
#endif // defined(BM_SWITCH_FOR_REAL)
#endif // defined(SKIP_LINKS)
    {
        Word_t wPopCnt;
        Word_t *pwKeys;
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
        unsigned int *piKeys;
#endif // (cnBitsPerWord > 32)
        unsigned short *psKeys;
        unsigned char *pcKeys;
#endif // defined(COMPRESSED_LISTS)

        DBGI(printf("InsertGuts List\n"));

        if (wRoot != 0) // pointer to old List
        {
#if defined(T_ONE)
            if (nType == T_ONE)
            {
  #if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                unsigned nBL = nDL_to_nBL(nDL);
                if (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1) {
                    wPopCnt = wr_nPopCnt(wRoot, nBL);
                } else
  #endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                {
                    wPopCnt = 1;
                }
  #if defined(PP_IN_LINK)
                // pop count in link should have been bumped by now
                // if we're not at the top
                assert((nDL == cnDigitsPerWord)
                    || (PWR_wPopCnt(pwRoot, NULL, nDL) == wPopCnt + 1));
  #endif // defined(PP_IN_LINK)
  #if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                // copy the embedded list into a temporary external list
                if (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1)
                {
                    Word_t ww = wRoot >> (cnBitsPerWord - nBL);
                    ww |= wKey & ~(EXP(nBL) - 1);

                    pwKeys = &ww;
      #if defined(COMPRESSED_LISTS)
          #if (cnBitsPerWord > 32)
                    iKey = (uint32_t)ww; piKeys = &iKey;
          #endif // (cnBitsPerWord > 32)
                    sKey = (uint16_t)ww; psKeys = &sKey;
      #endif // defined(COMPRESSED_LISTS)
                }
                else
  #endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                {
                    pwKeys = pwr;
                    // can we really not just to pxKeys = pwr?
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                    iKey = (uint32_t)*pwr; piKeys = &iKey;
#endif // (cnBitsPerWord > 32)
                    sKey = (uint16_t)*pwr; psKeys = &sKey;
#endif // defined(COMPRESSED_LISTS)
                }
            }
            else
#endif // defined(T_ONE)
            {
#if defined(PP_IN_LINK)
                if (nDL != cnDigitsPerWord) {
                    wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL) - 1;
                    pwKeys = ls_pwKeys(pwr); // list of keys in old List
                } else {
                    wPopCnt = ls_wPopCnt(pwr);
                    pwKeys = ls_pwKeys(pwr) + 1; // skip over pop count
                }
#else // defined(PP_IN_LINK)
                wPopCnt = ls_wPopCnt(pwr);
                pwKeys = ls_pwKeys(pwr); // list of keys in old List
#endif // defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                piKeys = ls_piKeys(pwr);
#endif // (cnBitsPerWord > 32)
                psKeys = ls_psKeys(pwr);
                pcKeys = ls_pcKeys(pwr);
#endif // defined(COMPRESSED_LISTS)
            }
            // prefix is already set
        }
        else
        {
            wPopCnt = 0;
            // make compiler happy about uninitialized variable
            // it doesn't recognize that (wPopCnt == 0) ==> pwKeys will not
            // be examined
            pwKeys = NULL;
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            piKeys = NULL;
#endif // (cnBitsPerWord > 32)
            psKeys = NULL;
            pcKeys = NULL;
#endif // defined(COMPRESSED_LISTS)
#if defined(PP_IN_LINK)
            if (nDL != cnDigitsPerWord)
            {
                // What about no_unnecessary_prefix?
                set_PWR_wPrefix(pwRoot, NULL, nDL, wKey);
            }
#endif // defined(PP_IN_LINK)
        }

// We don't support skip links to lists or bitmaps yet.  And don't have
// any criteria yet for converting from a list to a switch other than the
// list is full.  So we just add to an existing list or create a new one
// here.  But this is where we might make a decision to use a skip link
// to a list or a bitmap or convert a list to a switch or convert the
// other way or create a bitmap switch or ...
// We need a new type field the destination of a link with type zero.
//
//  - bitmap leaf -- depth, prefix and pop (depth implies width), bits
//  - list leaf -- depth, prefix, pop, capacity, key size, keys
//  - bitmap switch -- depth, prefix, pop, capacity, bitmap, links
//  - list switch -- depth, prefix, pop, capacity, (key, link) pairs

        unsigned nDLOld = nDL;
#if (cwListPopCntMax != 0)
        if (wPopCnt < cwListPopCntMax)
        {
            Word_t *pwList;

            if ((pwr == NULL)
                || (ListWords(wPopCnt + 1, nDL)
                    != ListWords(wPopCnt, nDL)))
            {
                // allocate a new list and init pop count in the first word
                pwList = NewList(wPopCnt + 1, nDL, wKey);
#if defined(PP_IN_LINK)
                assert((nDL == cnDigitsPerWord)
                    || (PWR_wPopCnt(pwRoot, NULL, nDL)
                            == wPopCnt + 1));
#endif // defined(PP_IN_LINK)
            }
            else
            {
                pwList = pwr;

#if defined(PP_IN_LINK)
                assert(nDL != cnDigitsPerWord);
                assert(PWR_wPopCnt(pwRoot, NULL, nDL) == wPopCnt + 1);
#else // defined(PP_IN_LINK)
                set_ls_wPopCnt(pwList, wPopCnt + 1);
#endif // defined(PP_IN_LINK)
            }

            set_wr(wRoot, pwList, T_LIST);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBL = nDL_to_nBL(nDL);
                if (nBL <= 8) {
                    CopyWithInsertChar(ls_pcKeys(pwList),
                        pcKeys, wPopCnt, (unsigned char)wKey);
                } else if (nBL <= 16) {
                    CopyWithInsertShort(ls_psKeys(pwList),
                        psKeys, wPopCnt, (unsigned short)wKey);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    CopyWithInsertInt(ls_piKeys(pwList),
                        piKeys, wPopCnt, (unsigned int)wKey);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    CopyWithInsertWord(
#if defined(PP_IN_LINK)
                        (nDL == cnDigitsPerWord) +
#endif // defined(PP_IN_LINK)
                            ls_pwKeys(pwList),
                        pwKeys, wPopCnt, wKey);
                }
            } else
#else // defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBL = nDL_to_nBL(nDL);
                if (nBL <= 8) {
                    COPY(ls_pcKeys(pwList), pcKeys, wPopCnt);
                } else if (nBL <= 16) {
                    COPY(ls_psKeys(pwList), psKeys, wPopCnt);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    COPY(ls_piKeys(pwList), piKeys, wPopCnt);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
#if defined(PP_IN_LINK)
                    COPY(ls_pwKeys(pwList) + (nDL == cnDigitsPerWord),
                         pwKeys, wPopCnt);
#else // defined(PP_IN_LINK)
                    COPY(ls_pwKeys(pwList), pwKeys, wPopCnt);
#endif // defined(PP_IN_LINK)
                }
            }
#endif // defined(SORT_LISTS)
            {
// shared code for (SORT && wPopCnt == 0) and ! SORT
                unsigned nBL = nDL_to_nBL(nDL); (void)nBL;
#if defined(T_ONE)
                if (wPopCnt == 0) {
#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                    if (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1) {
                        set_wr(wRoot, (wKey << (cnBitsPerWord - nBL)), T_ONE);
                    }
                    else
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
                    {
                        *pwList = wKey; set_wr_nType(wRoot, T_ONE);
                    }
                } else
#endif // defined(T_ONE)
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    ls_pcKeys(pwList)[wPopCnt] = wKey;
                } else if (nBL <= 16) {
                    ls_psKeys(pwList)[wPopCnt] = wKey;
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    { ls_piKeys(pwList)[wPopCnt] = wKey; }
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
#if defined(PP_IN_LINK)
                    ls_pwKeys(pwList)[wPopCnt + (nDL == cnDigitsPerWord)]
#else // defined(PP_IN_LINK)
                    ls_pwKeys(pwList)[wPopCnt]
#endif // defined(PP_IN_LINK)
                        = wKey;
                }
            }

            *pwRoot = wRoot; // install new

            if ((wPopCnt != 0) && (pwr != pwList))
            {
                OldList(pwr, wPopCnt, nDL);
            }
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            Word_t w;

            // List is full; insert a switch

#if defined(PP_IN_LINK)
            if (nDL < cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
#if defined(SKIP_LINKS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBL = nDL_to_nBL(nDL);
                Word_t wSuffix;
#endif // defined(COMPRESSED_LISTS)
                if (cwListPopCntMax != 0) // use const for compile time check
                {
                    Word_t wMax, wMin;
#if defined(SORT_LISTS)
#if defined(COMPRESSED_LISTS)
                    if (nBL <= 8) {
                        wMin = pcKeys[0];
                        wMax = pcKeys[wPopCnt - 1];
                        wSuffix = wKey & 0xff;
                    } else if (nBL <= 16) {
                        wMin = psKeys[0];
                        wMax = psKeys[wPopCnt - 1];
                        wSuffix = wKey & 0xffff;
#if (cnBitsPerWord > 32)
                    } else if (nBL <= 32) {
                        wMin = piKeys[0];
                        wMax = piKeys[wPopCnt - 1];
                        wSuffix = wKey & 0xffffffff;
#endif // (cnBitsPerWord > 32)
                    } else 
#endif // defined(COMPRESSED_LISTS)
                    { wMin = pwKeys[0]; wMax = pwKeys[wPopCnt - 1]; }
#else // defined(SORT_LISTS)
                    // walk the list to find max and min
                    wMin = (Word_t)-1;
                    wMax = 0;

#if defined(COMPRESSED_LISTS)
                        wSuffix = (nBL <= 8) ? (wKey & 0xff)
#if (cnBitsPerWord > 32)
                                : (nBL > 16) ? (wKey & 0xffffffff)
#endif // (cnBitsPerWord > 32)
                                : (wKey & 0xffff);
#endif // defined(COMPRESSED_LISTS)

                    for (w = 0; w < wPopCnt; w++)
                    {
#if defined(COMPRESSED_LISTS)
                        if (nBL <= 8) {
                            if (pcKeys[w] < wMin)
                                wMin = pcKeys[w];
                            if (pcKeys[w] > wMax)
                                wMax = pcKeys[w];
                        } else if (nBL <= 16) {
                            if (psKeys[w] < wMin)
                                wMin = psKeys[w];
                            if (psKeys[w] > wMax)
                                wMax = psKeys[w];
#if (cnBitsPerWord > 32)
                        } else if (nBL <= 32) {
                            if (piKeys[w] < wMin)
                                wMin = piKeys[w];
                            if (piKeys[w] > wMax)
                                wMax = piKeys[w];
#endif // (cnBitsPerWord > 32)
                        } else 
#endif // defined(COMPRESSED_LISTS)
                        {
                            if (pwKeys[w] < wMin) wMin = pwKeys[w];
                            if (pwKeys[w] > wMax) wMax = pwKeys[w];
                        }
                    }
#endif // defined(SORT_LISTS)
                    DBGI(printf("wMin "OWx" wMax "OWx"\n", wMin, wMax));

#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                    if (nBL <= 32)
#else // (cnBitsPerWord > 32)
                    if (nBL <= 16)
#endif // (cnBitsPerWord > 32)
                    {
                        nDL
                            = nBL_to_nDL(
                                LOG((EXP(cnBitsAtBottom) - 1)
                                        | ((wSuffix ^ wMin)
                                        |  (wSuffix ^ wMax)))
                                    + 1);
                    }
                    else
#endif // defined(COMPRESSED_LISTS)
                    {
                        nDL
                            = nBL_to_nDL(
                                LOG((EXP(cnBitsAtBottom) - 1)
                                        | ((wKey ^ wMin) | (wKey ^ wMax)))
                                    + 1);
                    }
                }
                else
                {
                    // can't dereference list if there isn't one
                    // go directly to bitmap
                    nDL = nBL_to_nDL(cnBitsAtBottom) + 1;
                }
            }

            // We don't create a switch below nBL_to_nDL(cnBitsAtBottom) + 1.
            // Why?  Because we've defined nBL_to_nDL(cnBitsAtBottom) as
            // automatic bitmap (no switch) and we may need a prefix at
            // nBL_to_nDL(cnBitsAtBottom) + 1 since we don't have one in the
            // bitmap.
            // Creating a switch just above the bottom can be a problem, e.g.
            // (bitsPerWord=64, bitsPerDigit=32) will try to create a very
            // large switch.  We'd be much better off with a skip link to
            // a bitmap or a bitmap switch.
            if (nDL <= nBL_to_nDL(cnBitsAtBottom))
            {
                DBGI(printf("InsertGuts nDL"
                            " <= nBL_to_nDL(cnBitsAtBottom)\n"));

                nDL = nBL_to_nDL(cnBitsAtBottom) + 1;
            }
#else // defined(SKIP_LINKS)
            assert(nDL > nBL_to_nDL(cnBitsAtBottom));
#endif // defined(SKIP_LINKS)

            // NewSwitch overwrites *pwRoot which is a problem for
            // T_ONE with embedded keys.

            NewSwitch(pwRoot, wKey, nDL, nDLOld,
                      /* wPopCnt */ 0);

#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
            unsigned nBLOld = nDL_to_nBL(nDLOld);
#else // defined(SKIP_LINKS)
// Revisit the use of "Old" here.
            unsigned nBLOld = nDL_to_nBL(nDL);
#endif // defined(SKIP_LINKS)
            if (nBLOld <= 8) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pcKeys[w] | (wKey & ~(Word_t)0xff),
                           nDLOld);
                }
            } else if (nBLOld <= 16) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, psKeys[w] | (wKey & ~(Word_t)0xffff),
                           nDLOld);
                }
#if (cnBitsPerWord > 32)
            } else if (nBLOld <= 32) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot,
                           piKeys[w] | (wKey & ~(Word_t)0xffffffff),
                           nDLOld);
                }
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pwKeys[w], nDLOld);

                    DBGI(printf(
                        "\n# InsertGuts After Insert(wKey "OWx") Dump\n",
                        pwKeys[w]));
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                    DBGI(printf("\n"));
                }
            }

            DBGI(printf("Just Before InsertGuts calls final Insert"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
            Insert(pwRoot, wKey, nDLOld);

#if (cwListPopCntMax != 0)
            // Hmm.  Should this be nDLOld?
            if (wPopCnt != 0) { OldList(pwr, wPopCnt, nDLOld); }
#endif // (cwListPopCntMax != 0)
        }
    }
#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    else
    {
#if defined(BM_SWITCH_FOR_REAL)
#if defined(TYPE_IS_RELATIVE)
        unsigned nDLR = nDL - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
        unsigned nDLR = tp_to_nDL(nType);
#endif // defined(TYPE_IS_RELATIVE)
#if defined(SKIP_LINKS)
        Word_t wPrefix;
        // Test to see if this is a missing link case.
        // If not, then it is a prefix mismatch case.
        // nDL does not include any skip indicated in nType.
        // If nDS == 0 or nDL == tp_to_nDL, then we know
        // it is a missing link because it can't be a prefix mismatch.
        // Unfortunately, nDS != 0 (or the other) does not imply a prefix
        // mismatch.
        // if (wPrefix == w_wPrefix(wKey, nDLR))
        // It's a bit of a bummer that we are doing the prefix check again.
        // Can we avoid it as follows:
        if ((nDLR == nDL)
            || ((wPrefix = PWR_wPrefix(pwRoot, pwr, nDLR))
                == w_wPrefixNotAtTop(wKey, nDLR)))
        // If nDS != 0 then we're not at the top or PP_IN_LINK is not defined.
#endif // defined(SKIP_LINKS)
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL)
        {
#if defined(SKIP_LINKS)
            DBGI(printf("wPrefix "OWx" w_wPrefix "OWx" nDLR %d\n",
                        PWR_wPrefix(pwRoot, pwr, nDLR),
                        w_wPrefix(wKey, nDLR), nDLR));
#endif // defined(SKIP_LINKS)
            // no link -- for now -- will eventually have to check
            NewLink(pwRoot, wKey, nDLR);
            // Remember to update type field in *pwRoot if necessary.
            // Would need to add a parameter to NewLink to do it there.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
            set_wr_nType(*pwRoot, nDS_to_tp(nDL - nDLR));
#else // defined(TYPE_IS_RELATIVE)
            set_wr_nType(*pwRoot, nDL_to_tp(nDLR));
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            Insert(pwRoot, wKey, nDL);
        }
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
        else
#endif // defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
#if defined(SKIP_LINKS)
        {
            // prefix mismatch
            // insert a switch so we can add just one key; seems like a waste
            // A bitmap switch would be great; no reason to consider
            // converting the existing bitmap to a list if a bitmap switch is
            // short.  Huh?
            unsigned nDLRoot;
            Word_t wPopCnt;

#if defined(TYPE_IS_RELATIVE)
            nDLRoot = nDL - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
            nDLRoot = tp_to_nDL(nType);
#endif // defined(TYPE_IS_RELATIVE)

            // Can't have a prefix mismatch if there is no skip.
            assert(nDLRoot < nDL);

            unsigned nDLUp = nDL;

            // figure new nDL for old parent link
            Word_t wPrefix = PWR_wPrefix(pwRoot, pwr, nDLRoot);
            nDL = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
            // nDL includes the highest order digit that is different.

            assert(nDL > nDLRoot);

            if ((wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDLRoot)) == 0)
            {
                // full pop overflow
                wPopCnt = wPrefixPopMask(nDLRoot) + 1;
            }

            // Have to get old prefix before inserting the new switch because
            // NewSwitch copies to *pwRoot.
            // Also deal with switch at top with no link if PP_IN_LINK.

            unsigned nIndex;

#if defined(PP_IN_LINK)
            // PP_IN_LINK => no skip link at top => no prefix mismatch at top
            assert(nDLUp < cnDigitsPerWord);
#endif // defined(PP_IN_LINK)

            // todo nBitsIndexSz; wide switch
            nIndex = (wPrefix >> nDL_to_nBL_NAT(nDL - 1))
                & (EXP(nDL_to_nBitsIndexSz(nDL)) - 1);
            // nIndex is the logical index in new switch.
            // It may not be the same as the index in the old switch.

#if defined(BM_IN_LINK)
            // Save the old bitmap before it is trashed by NewSwitch.
            // Is it possible that nDLUp != cnDigitsPerWord and
            // we are at the top?
            Link_t ln;
            Word_t wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDLRoot));
            if (nDLUp != cnDigitsPerWord)
            {
                memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL),
                       DIV_UP(wIndexCnt, cnBitsPerWord) * cnBytesPerWord);
#if ! defined(BM_SWITCH_FOR_REAL)
                assert((wIndexCnt < cnBitsPerWord)
                    || (ln.ln_awBm[0] == (Word_t)-1));
#endif // ! defined(BM_SWITCH_FOR_REAL)
            }
#endif // defined(BM_IN_LINK)

            Word_t *pwSw;
            // initialize prefix/pop for new switch
            // Make sure to pass the right key for BM_SWITCH_FOR_REAL.
            pwSw = NewSwitch(pwRoot,
                             wPrefix, nDL, nDLUp, wPopCnt);

#if defined(BM_SWITCH_FOR_REAL)
#if defined(BM_IN_LINK)
            if (nDLUp != cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                // Switch was created with only one link based on wKey
                // passed in.  Unless BM_IN_LINK && switch is at top.
                nIndex = 0;
            }
#endif // defined(BM_SWITCH_FOR_REAL)

#if defined(BM_IN_LINK)
            if (nDLUp != cnDigitsPerWord)
            {
                // Copy bitmap from old link to new link.
                memcpy(pwr_pLinks(pwSw)[nIndex].ln_awBm, ln.ln_awBm,
                       DIV_UP(wIndexCnt, cnBitsPerWord) * cnBytesPerWord);
            }
            else
            {
                // Initialize bitmap in new link.
                // Mind the high-order bits of the bitmap word if/when the
                // bitmap is smaller than a whole word.
                // Mind endianness.
                if (wIndexCnt < cnBitsPerWord)
                {
                    *pwr_pLinks(pwSw)[nIndex].ln_awBm = EXP(wIndexCnt) - 1;
                }
                else
                {
                    memset(pwr_pLinks(pwSw)[nIndex].ln_awBm, -1,
                           DIV_UP(wIndexCnt, cnBitsPerWord) * cnBytesPerWord);
                }
            }
#endif // defined(BM_IN_LINK)

#if defined(TYPE_IS_RELATIVE)
            DBGI(printf("nDL %d nDLR %d nDLU %d\n",
                   nDL, nDLRoot, nDLUp));
            set_wr_nDS(wRoot, nDL - nDLRoot - 1);
#endif // defined(TYPE_IS_RELATIVE)
            // Copy wRoot from old link to new link.
            pwr_pLinks(pwSw)[nIndex].ln_wRoot = wRoot;

#if defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
            if (nDLRoot == nDL - 1)
            {
                // The previously necessary prefix in the old switch
                // is now unnecessary.
                DBGI(printf("nDLR %d nDL %d\n",
                            nDLRoot, nDL));
            }
            else
#endif // defined(NO_UNNECESSARY_PREFIX)
            {
                set_PWR_wPrefix(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                                nDLRoot, wPrefix);
            }

            set_PWR_wPopCnt(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                            nDLRoot, wPopCnt);
#else // defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
           // We could go to the trouble of zeroing the no-longer necessary
           // prefix in the old switch. 
#endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)

            Insert(pwRoot, wKey, nDLUp);
        }
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)

    return Success;
}

Status_t
InsertAtBottom(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
        assert(nDL == 1); (void)nDL; // use depends on ifdefs
        (void)wRoot;

#if (cnBitsAtBottom <= cnLogBitsPerWord)

        assert( ! BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)) );

        DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
                    *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

#else // (cnBitsAtBottom <= cnLogBitsPerWord)

        Word_t *pwr = wr_pwr(wRoot);

        if (pwr == NULL)
        {
            pwr = NewBitmap();

            // If we set the type field to 0 in the *pwRoot to the bitmap,
            // then we wouldn't need to extract pwr before derefencing it
            // during lookup.
            // We could use nDL == 1 to indicate that T_NULL does
            // not imply pwr == NULL.
            set_wr(wRoot, pwr, T_BITMAP);

            *pwRoot = wRoot;
        }

        assert( ! BitIsSet(pwr, wKey & (EXP(cnBitsAtBottom) - 1)) );

        DBGI(printf("SetBit(wRoot "OWx" wKey "OWx") pwRoot %p\n",
                    wRoot, wKey & (EXP(cnBitsAtBottom) - 1), (void *)pwRoot));

        SetBit(pwr, wKey & (EXP(cnBitsAtBottom) - 1));

#endif // (cnBitsAtBottom <= cnLogBitsPerWord)

#if defined(PP_IN_LINK)

        // What about no_unnecessary_prefix?
        set_PWR_wPrefix(pwRoot, NULL, nDL, wKey);

#endif // defined(PP_IN_LINK)

        return Success;
}

Status_t
RemoveBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot);

#if defined(T_ONE) && (cwListPopCntMax != 0)
Status_t
RemoveTypeOne(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t *pwr);
#endif // defined(T_ONE) && (cwListPopCntMax != 0)

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;

    DBGR(printf("RemoveGuts\n"));

#if (cwListPopCntMax != 0)
    if (nDL <= 1)
#else // (cwListPopCntMax != 0)
    assert(nDL <= 1);
#endif // (cwListPopCntMax != 0)
    {
        return RemoveBitmap(pwRoot, wKey, nDL, wRoot);
    }

#if (cwListPopCntMax != 0)

#if defined(T_ONE)
    if (wr_nType(wRoot) == T_ONE) {
        return RemoveTypeOne(pwRoot, wKey, nDL, pwr);
    }
#endif // defined(T_ONE)

    assert(wr_nType(wRoot) == T_LIST);

    Word_t wPopCnt;

#if defined(PP_IN_LINK)
    if (nDL != cnDigitsPerWord) {
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL) + 1;
    } else
#endif // defined(PP_IN_LINK)
    {
        wPopCnt = ls_wPopCnt(pwr);
    }

#if ! defined(T_ONE)
    if (wPopCnt == 1) {
        OldList(pwr, wPopCnt, nDL);
        *pwRoot = 0;
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }
#endif // ! defined(T_ONE)

    unsigned nBL = nDL_to_nBL(nDL); (void)nBL;

    Word_t *pwKeys = pwr_pwKeys(pwr);

    unsigned nIndex;
    for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
        (nBL <= 16) ? (pwr_psKeys(pwr)[nIndex] != (uint16_t)wKey) :
#if (cnBitsPerWord > 32)
        (nBL <= 32) ? (pwr_piKeys(pwr)[nIndex] != (uint32_t)wKey) :
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        (pwKeys[nIndex] != wKey); nIndex++) { }

    // nIndex identifies the key being removed.

    Word_t *pwList;
    if (ListWords(wPopCnt - 1, nDL) != ListWords(wPopCnt, nDL))
    {
        // Malloc a new, smaller list.
        pwList = NewList(wPopCnt - 1, nDL, wKey);
        // Why are we copying the old list to the new one?
        // Because the beginning will be the same.
        // Except for the the pop count.
#if (cnBitsPerWord == 64) && defined(EMBED_KEYS)
        if ((wPopCnt == 2)
            && (nBL <= cnBitsPerWord - cnLogBitsPerWord - 1))
        {
            // nIndex is 0 or 1
            if (nBL <= 16) {
                set_wr(wRoot,
                      ((Word_t)(pwr_psKeys(pwr)[ ! nIndex ])
                          << (cnBitsPerWord - nBL)),
                       T_ONE);
            } else if (nBL <= 32) {
                set_wr(wRoot,
                      ((Word_t)(pwr_piKeys(pwr)[ ! nIndex ])
                          << (cnBitsPerWord - nBL)),
                       T_ONE);
            }
            goto cleanup; // stop pretending; it's painful
        }
#endif // (cnBitsPerWord == 64) && defined(EMBED_KEYS)

        COPY(pwList, pwr, ListWords(wPopCnt - 1, nDL));

        set_wr(wRoot, pwList, T_LIST);
    }
    else
    {
        pwList = pwr;
    }

#if defined(PP_IN_LINK)
    assert(nDL != cnDigitsPerWord);
#else // defined(PP_IN_LINK)
#if defined(T_ONE)
    if (wPopCnt != 2)
#endif // defined(T_ONE)
    {
        set_ls_wPopCnt(pwList, wPopCnt - 1);
    }
#endif // defined(PP_IN_LINK)

#if defined(T_ONE)
    if (wPopCnt == 2) {
#if defined(COMPRESSED_LISTS)
        Word_t wPrefix = wKey & ~(EXP(nBL) - 1);
        if (nBL <= 16) {
            *pwList = wPrefix | pwr_psKeys(pwr)[ ! nIndex ];
        } else if (nBL <= 32) {
            *pwList = wPrefix | pwr_piKeys(pwr)[ ! nIndex ];
        } else
#endif // defined(COMPRESSED_LISTS)
        {
            *pwList = pwKeys[ ! nIndex ];
        }
        set_wr_nType(wRoot, T_ONE);
        *pwRoot = wRoot;
    } else
#endif // defined(T_ONE)
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        MOVE(&pwr_pcKeys(pwList)[nIndex],
             &pwr_pcKeys(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
    } else if (nBL <= 16) {
        MOVE(&pwr_psKeys(pwList)[nIndex],
             &pwr_psKeys(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        MOVE(&pwr_piKeys(pwList)[nIndex],
             &pwr_piKeys(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        MOVE(&pwr_pwKeys(pwList)[nIndex], &pwKeys[nIndex + 1],
             wPopCnt - nIndex - 1);
    }

    if (pwList != pwr)
    {
#if (cnBitsPerWord == 64) && defined(EMBED_KEYS)
cleanup:
#endif // (cnBitsPerWord == 64) && defined(EMBED_KEYS)
        OldList(pwr, wPopCnt, nDL);
        *pwRoot = wRoot;
    }

    return Success;

#endif // (cwListPopCntMax != 0)

    (void)pwRoot; (void)wKey; (void)nDL; (void)wRoot;
}

// Clear the bit for wKey in the bitmap.
// And free the bitmap if it is empty and not embedded.
Status_t
RemoveBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
    (void)nDL;

#if (cnBitsAtBottom <= cnLogBitsPerWord)

    ClrBitInWord(wRoot, wKey & MSK(cnBitsAtBottom));

    // What if link has more than just ln_wRoot due
    // to BM_IN_LINK and/or PP_IN_LINK?
    // What if population just went to 0?
    // Should we clear the rest of the link?
    // Or can we rely on bCleanup phase in Remove to do it if necessary?

    *pwRoot = wRoot;

#else // (cnBitsAtBottom <= cnLogBitsPerWord)

    Word_t *pwr = wr_pwr(wRoot);

    ClrBit(pwr, wKey & MSK(cnBitsAtBottom));

  #if defined(PP_IN_LINK)

    if (PWR_wPopCnt(pwRoot, NULL, nDL) != 0) {
        goto skipOldBitmap; // bitmap is not empty
    }

  #else // defined(PP_IN_LINK)

    // Free the bitmap if it is empty.
    for (Word_t ww = 0; ww < EXP(cnBitsAtBottom - cnLogBitsPerWord); ww++) {
        if (__builtin_popcountll(pwr[ww])) {
            goto skipOldBitmap; // bitmap is not empty
        }
    }

  #endif // defined(PP_IN_LINK)

    OldBitmap(pwr);

    *pwRoot = 0; // Do we need to clear the rest of the link, e.g. PP_IN_LINK?

skipOldBitmap:

#endif // (cnBitsAtBottom <= cnLogBitsPerWord)

    if (*pwRoot == 0)
    {
        // We return to Remove which will clean up ancestors.
        DBGR(printf("RemoveGuts *pwRoot is now 0\n"));
    }

    return Success;
}

#if defined(T_ONE) && (cwListPopCntMax != 0)

// Remove the key from a T_ONE leaf.
Status_t
RemoveTypeOne(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t *pwr)
{
    (void)wKey;

#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)

    unsigned nBL = nDL_to_nBL(nDL);
    unsigned nBitsPopCnt = LOG(119 / nBL);
    unsigned nPopCnt
             = ((Word_t)pwr >> (cnLogBitsPerWord + 1)) & MSK(nBitsPopCnt);

    Word_t wRoot = (Word_t)pwr;

    if (nPopCnt <= 1)
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)
    {
        OldList(pwr, 1, nDL); // OldList is a no-op if the list is embedded.
        *pwRoot = 0; // Do we need to clear the rest of the link also?
        return Success;
    }

#if defined(EMBED_KEYS) && (cnBitsPerWord == 64)

    assert(nPopCnt * nBL
           <= cnBitsPerWord - cnLogBitsPerWord - 1 - nBitsPopCnt);

    // Copy the last key in the embedded list to the slot currently
    // occupied by the key being removed.
    // Embedded lists are not sorted.

    unsigned nn;
    for (nn = 1;
        (((wRoot >> (cnBitsPerWord - (nn * nBL))) ^ wKey) & ~MSK(nBL));
         ++nn) { }

    Word_t wKeyLast = (wRoot >> (cnBitsPerWord - (nPopCnt * nBL))) & MSK(nBL);

    wRoot &= ~(MSK(nBL) << (cnBitsPerWord - (nn * nBL))); // clear slot
    wRoot |= wKeyLast << (cnBitsPerWord - (nn * nBL)); // or in key
    wRoot &= ~(MSK(nBitsPopCnt) << (cnLogBitsPerWord + 1)); // clear pop cnt
    wRoot |= (nPopCnt - 1) << (cnLogBitsPerWord + 1); // or in new pop cnt
    // Do we need to care about clearing the vacated slot?

    *pwRoot = wRoot;

    return Success;
 
#endif // defined(EMBED_KEYS) && (cnBitsPerWord == 64)

}

#endif // defined(T_ONE) && (cwListPopCntMax != 0)

#endif // (cnDigitsPerWord != 1)

// ****************************************************************************
// JUDY1 FUNCTIONS:

Word_t
Judy1FreeArray(PPvoid_t PPArray, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("Judy1FreeArray\n"));

#if (cnDigitsPerWord != 1)
    return FreeArrayGuts((Word_t *)PPArray,
        /* wPrefix */ 0, cnBitsPerWord, /* bDump */ 0);
#else // (cnDigitsPerWord != 1)
    JudyFree(*PPArray,
       EXP(cnBitsPerWord - cnLogBitsPerByte - cnLogBytesPerWord));
    return EXP(cnBitsPerWord - cnLogBitsPerByte);
#endif // (cnDigitsPerWord != 1)
}

// Return the number of keys that are present from wKey0 through wKey1.
// Include wKey0 and wKey1 if they are present.
Word_t
Judy1Count(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, P_JE)
{
    DBGR(printf("Judy1Count\n"));

#if (cnDigitsPerWord != 1)

    // Return C_JERR if the array is empty or wKey0 > wKey1.
    if ((PArray == (Pvoid_t) NULL) || (wKey0 > wKey1))
    {
        if (PJError != NULL)
        {
            JU_ERRNO(PJError) = JU_ERRNO_NONE; // zero pop
            JU_ERRID(PJError) = __LINE__;
        }

        return 0; // C_JERR
    }

    if (PJError != NULL)
    {
        JU_ERRNO(PJError) = JU_ERRNO_FULL; // full pop
        JU_ERRID(PJError) = __LINE__;
    }

    Word_t wRoot = (Word_t)PArray;
    unsigned nType = wr_nType(wRoot);
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    Word_t wPopCnt;

  #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if ( ! tp_bIsSwitch(nType) )
    {
      #if defined(T_ONE)
        if (nType == T_ONE) {
            wPopCnt = 1; // Always a full word to top; never embedded.
        } else
      #endif // defined(T_ONE)
        wPopCnt = ls_wPopCnt(pwr); // valid at top for PP_IN_LINK if ! T_ONE
    }
    else
  #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    {
  #if defined(PP_IN_LINK)
        // no skip links at root for PP_IN_LINK -- no place for prefix
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
          #if defined(TYPE_IS_RELATIVE)
        assert(tp_to_nDS(nType) == 0);
          #else // defined(TYPE_IS_RELATIVE)
        assert(tp_to_nDL(nType) == cnDigitsPerWord);
          #endif // defined(TYPE_IS_RELATIVE)
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        // add up the pops in the links
      #if defined(BM_SWITCH) && !defined(BM_IN_LINK)
        Word_t xx = 0;
      #endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
        wPopCnt = 0;
        for (unsigned nn = 0; nn < EXP(cnBitsIndexSzAtTop); nn++)
        {
      #if defined(BM_SWITCH) && !defined(BM_IN_LINK)
            if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
      #endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)
            {
      #if defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[xx].ln_wRoot;
                xx++;
      #else // defined(BM_SWITCH) && !defined(BM_IN_LINK)
                Word_t *pwRootLn = &pwr_pLinks(pwr)[nn].ln_wRoot;
      #endif // defined(BM_SWITCH) && !defined(BM_IN_LINK)

                // *pwRootLn may not be a pointer to a switch
                // It may be a pointer to a list leaf.
                // And if nBL_to_nDL(cnBitsAtBottom) == cnDigitsPerWord - 1,
                // then it could be a pointer to a bitmap?
                Word_t wPopCntLn;
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                unsigned nTypeLn = wr_nType(*pwRootLn);
                if (tp_bIsSwitch(nTypeLn))
                {
                    wPopCntLn
          #if defined(TYPE_IS_RELATIVE)
                        = PWR_wPopCnt(pwRootLn, NULL,
                                  cnDigitsPerWord - 1 - wr_nDS(*pwRootLn));
          #else // defined(TYPE_IS_RELATIVE)
                        = PWR_wPopCnt(pwRootLn,
                                      NULL, wr_nDL(*pwRootLn));
          #endif // defined(TYPE_IS_RELATIVE)
                }
                else
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                {
                    wPopCntLn = PWR_wPopCnt(pwRootLn,
                                            NULL, cnDigitsPerWord - 1);
                }

      #if defined(DEBUG_INSERT)
                if (wPopCntLn != 0)
                {
                    printf("Pop sum");
          #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    unsigned nDL
              #if defined(TYPE_IS_RELATIVE)
                        = cnDigitsPerWord - 1 - wr_nDS(*pwRootLn);
              #else // defined(TYPE_IS_RELATIVE)
                        = wr_nDL(*pwRootLn);
              #endif // defined(TYPE_IS_RELATIVE)
                    printf(" mask "OWx" %"_fw"d",
                           wPrefixPopMask(nDL),
                           wPrefixPopMask(nDL));
          #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" nn %d wPopCntLn %"_fw"d "OWx"\n",
                           nn, wPopCntLn, wPopCntLn);
                }
      #endif // defined(DEBUG_INSERT)

                wPopCnt += wPopCntLn;

                // We use pwr_pLinks(pwr)[nn].ln_wRoot != 0 to disambiguate
                // wPopCnt == 0.  Hence we cannot allow Remove to leave
                // pwr_pLinks(pwr)[nn].ln_wRoot != 0 unless the actual
                // population count is not zero.
                if ((wPopCntLn == 0) && (*pwRootLn != 0))
                {
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    // Isn't this assuming tp_bIsSwitch(nTypeLn)?
                    // T_LIST and T_ONE can't be full pop?
                    // And we don't skip directly to T_BITMAP?
                    // What if bitmap is direcly below top switch?
                    assert(tp_bIsSwitch(nTypeLn));
                    int nDL =
          #if defined(TYPE_IS_RELATIVE)
                        cnDigitsPerWord - 1 - wr_nDS(*pwRootLn)
          #else // defined(TYPE_IS_RELATIVE)
                        wr_nDL(*pwRootLn)
          #endif // defined(TYPE_IS_RELATIVE)
                        ;
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

      #if defined(DEBUG_INSERT)
                    printf("Pop sum (full)");
          #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" mask "Owx" %"_fw"d\n",
                           wPrefixPopMask(nDL),
                           wPrefixPopMask(nDL));
                    printf("nn %d wPopCntLn %"_fw"d "OWx"\n",
                           nn, wPrefixPopMask(nDL) + 1,
                           wPrefixPopMask(nDL) + 1);
          #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
      #endif // defined(DEBUG_INSERT)

      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    wPopCnt += wPrefixPopMask(nDL) + 1;
      #else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    wPopCnt += wPrefixPopMask(cnDigitsPerWord - 1) + 1;
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                }
            }
        }
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
          #if ! defined(NDEBUG)
        int nDL =
              #if defined(TYPE_IS_RELATIVE)
            cnDigitsPerWord - tp_to_nDS(nType)
              #else // defined(TYPE_IS_RELATIVE)
            tp_to_nDL(nType)
              #endif // defined(TYPE_IS_RELATIVE)
            ;
        assert(wPopCnt - 1 <= wPrefixPopMask(nDL));
          #endif // ! defined(NDEBUG)
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #else // defined(PP_IN_LINK)
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        int nDL =
          #if defined(TYPE_IS_RELATIVE)
            cnDigitsPerWord - tp_to_nDS(nType)
          #else // defined(TYPE_IS_RELATIVE)
            tp_to_nDL(nType)
          #endif // defined(TYPE_IS_RELATIVE)
            ;
        wPopCnt = PWR_wPopCnt(NULL, pwr, nDL);
        if (wPopCnt == 0) {
            wPopCnt = wPrefixPopMask(nDL) + 1;
        }
      #else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        wPopCnt = PWR_wPopCnt(NULL, pwr, cnDigitsPerWord);
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #endif // defined(PP_IN_LINK)
    }

  #if defined(DEBUG)
    if (wPopCnt != wInserts)
    {
        Dump(pwRootLast, 0, cnBitsPerWord);
        printf("\nwPopCnt %"_fw"d wInserts %"_fw"d\n", wPopCnt, wInserts);
    }
    assert(wPopCnt == wInserts);
  #endif // defined(DEBUG)

    return wPopCnt;

#else // (cnDigitsPerWord != 1)

    (void)PArray; (void)wKey0; (void)wKey1, (void)PJError;
    return wInserts;

#endif // (cnDigitsPerWord != 1)
}


