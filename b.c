
// @(#) $Id: b.c,v 1.216 2014/06/04 02:45:19 mike Exp mike $
// @(#) $Source: /Users/mike/b/RCS/b.c,v $

#include "b.h"

#if defined(RAM_METRICS)
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
#endif // defined(RAM_METRICS)

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
ListWords(Word_t wPopCnt, unsigned nDigitsLeft)
{
    unsigned nWords;

#if defined(COMPRESSED_LISTS)

    unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);

    unsigned nBytesKeySz = (nBitsLeft <=  8) ? 1
                         : (nBitsLeft <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBitsLeft <= 32) ? 4
#endif // (cnBitsPerWord > 32)
                         : sizeof(Word_t);

    nWords = DIV_UP(wPopCnt * nBytesKeySz, cnBytesPerWord);

#else // defined(COMPRESSED_LISTS)

    nWords = wPopCnt;

    (void)nDigitsLeft;

#endif // defined(COMPRESSED_LISTS)

#if defined(PP_IN_LINK)
    if (nDigitsLeft == cnDigitsPerWord)
    {
        nWords += 1; // make room for pop count in the list at top
    }
#endif // defined(PP_IN_LINK)

    // Would be nice to do a better job of packing keys and pop count
    // if COMPRESSED_LISTS and not PP_IN_LINK or at top.

    nWords += OFFSET_OF(ListLeaf_t, ll_awKeys) / sizeof(Word_t);
    nWords |= 1; // mallocs of an even number of words waste a word

    return nWords;
}

Word_t *
NewList(Word_t wPopCnt, unsigned nDigitsLeft, Word_t wKey)
{
    (void)wKey;

    unsigned nWords = ListWords(wPopCnt, nDigitsLeft);

#if defined(COMPRESSED_LISTS)
    unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);

    unsigned nBytesKeySz = (nBitsLeft <=  8) ? 1
                         : (nBitsLeft <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBitsLeft <= 32) ? 4
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

    Word_t *pwList = (Word_t *)MyMalloc(nWords);

    DBGM(printf("NewList pwList %p wPopCnt "OWx" nWords %d\n",
        (void *)pwList, wPopCnt, nWords));

#if defined(PP_IN_LINK)
    if (nDigitsLeft == cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
    {
        set_ls_wPopCnt(pwList, wPopCnt);
    }

#if defined(DL_IN_LL)
    set_ll_nDigitsLeft(pwList, nDigitsLeft);
#endif // defined(DL_IN_LL)

// Should we be setting wPrefix here for PP_IN_LINK?

    return pwList;
}

Word_t
OldList(Word_t *pwList, Word_t wPopCnt, unsigned nDigitsLeft)
{
    unsigned nWords = ListWords(wPopCnt, nDigitsLeft);

    DBGM(printf("Old pwList %p wLen %d wPopCnt "OWx"\n",
        (void *)pwList, nWords, (Word_t)wPopCnt));

#if defined(DL_IN_LL)
    assert(nDigitsLeft == ll_nDigitsLeft(pwList));
#endif // defined(DL_IN_LL)

#if defined(COMPRESSED_LISTS)

    unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);

    unsigned nBytesKeySz = (nBitsLeft <=  8) ? 1
                         : (nBitsLeft <= 16) ? 2
#if (cnBitsPerWord > 32)
                         : (nBitsLeft <= 32) ? 4
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

    MyFree(pwList, nWords);

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
// Initialize its prefix if there is one.  Need to know nDigitsLeftUp for
// PP_IN_LINK to figure out if the prefix exists.
// Initialize its bitmap if there is one.  Need to know nDigitsLeftUp for
// BM_IN_LINK to figure out if the bitmap exists.
// Need to know nDigitsLeftUp if TYPE_IS_RELATIVE to figure nDS.
// Install wRoot at pwRoot.  Need to know nDigitsLeft.
// Account for the memory (for both JUDYA and JUDYB columns in Judy1LHTime).
// Need to know if we are at the bottom so we should count it as a bitmap.
static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft,
          unsigned nDigitsLeftUp, Word_t wPopCnt)
{
    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);

    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDigitsLeft);
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
    if (nDigitsLeftUp != cnDigitsPerWord)
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

#if defined(RAM_METRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        // embedded bitmap
        assert(nDigitsLeft == nBL_to_nDL(cnBitsAtBottom) + 1); // later
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
#endif // defined(RAM_METRICS)

    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx" nDL %d nDLU %d) pwr %p\n",
        (void *)pwRoot, wKey, nDigitsLeft, nDigitsLeftUp, (void *)pwr));

#if defined(TYPE_IS_RELATIVE)
    set_wr(*pwRoot, pwr, nDS_to_tp(nDigitsLeftUp - nDigitsLeft));
#else // defined(TYPE_IS_RELATIVE)
    set_wr(*pwRoot, pwr, nDigitsLeft_to_tp(nDigitsLeft));
#endif // defined(TYPE_IS_RELATIVE)

#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
    if (nDigitsLeftUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
#if defined(BM_SWITCH_FOR_REAL)

        memset(PWR_pwBm(pwRoot, pwr), 0, sizeof(PWR_pwBm(pwRoot, pwr)));

        unsigned nBitsLeft = nDL_to_nBL_NotAtTop(nDigitsLeft);
        // leave nBitsLeft greater than cnBitsPerWord intentionally for now

        Word_t wIndex
            = (wKey >> (nBitsLeft - nBitsIndexSz)) & (wIndexCnt - 1);

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
    if (nDigitsLeftUp < cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
    {
#if defined(SKIP_LINKS)
        assert(nDigitsLeft <= nDigitsLeftUp);

#if defined(NO_UNNECESSARY_PREFIX)
        // If defined(SKIP_PREFIX_CHECK) then we need the prefix at the leaf
        // even if there is no skip to the leaf.  Why?  Because there may
        // have been a skip somewhere above and we do the prefix check at the
        // leaf.  List leaves only need it if the keys in the list are less than
        // whole words.  For now we set prefix if we're at any compressed list
        // level even if it isn't necessary.
        // Bits in a bitmap leaf are always less than whole words and always
        // need the prefix for this case.
        // Does it mean we'd have to add the prefix when transitioning
        // from full word list directly to bitmap?  Do we ever do this?
        // We could get rid of the leaf check in some cases if we enhance Insert
        // to keep track of any prefix checks done along the way and
        // pass that info to InsertGuts.
        // We could go back up to where there is a skip (hence a prefix) and
        // do the check.
        if ((nDigitsLeft == nDigitsLeftUp)
#if defined(SKIP_PREFIX_CHECK)
#if ! defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            && (nDL_to_nBL_NotAtTop(nDigitsLeft - 1) > 32)
#else // (cnBitsPerWord > 32)
            && (nDL_to_nBL_NotAtTop(nDigitsLeft - 1) > 16)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            && ((nDigitsLeft - 1) > nBL_to_nDL(cnBitsAtBottom))
#endif // ! defined(PP_IN_LINK)
#endif // defined(SKIP_PREFIX_CHECK)
            && 1)
        {
            DBGI(printf(
              "Not installing prefix left %d up %d wKey "OWx"\n",
                nDigitsLeft, nDigitsLeftUp, wKey));

            set_PWR_wPrefix(pwRoot, pwr, nDigitsLeft, 0);
        }
        else
#endif // defined(NO_UNNECESSARY_PREFIX)
        {
            set_PWR_wPrefix(pwRoot, pwr, nDigitsLeft, wKey);
        }
#else // defined(SKIP_LINKS)
        set_PWR_wPrefix(pwRoot, pwr, nDigitsLeft, 0);
#endif // defined(SKIP_LINKS)

        set_PWR_wPopCnt(pwRoot, pwr, nDigitsLeft, wPopCnt);

        DBGM(printf("NewSwitch PWR_wPrefixPop "OWx"\n",
            PWR_wPrefixPop(pwRoot, pwr)));
    }

    DBGI(printf("After NewSwitch"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nDigitsLeft; // nDigitsLeft is not used for all ifdef combos
    (void)nDigitsLeftUp; // nDigitsLeftUp is not used for all ifdef combos
}

#if defined(BM_SWITCH_FOR_REAL)
static void
NewLink(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    DBGI(printf("NewLink(pwRoot %p wKey "OWx" nDigitsLeft %d)\n",
        (void *)pwRoot, wKey, nDigitsLeft));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, pwr, nDigitsLeft)));

#if defined(BM_IN_LINK)
    assert(nDigitsLeft != cnDigitsPerWord);
#endif // defined(BM_IN_LINK)

    // How many links are there in the old switch?
    Word_t wPopCnt = 0;
    for (unsigned nn = 0;
         nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDigitsLeft)), cnBitsPerWord);
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

#if defined(RAM_METRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        // embedded bitmap
        assert(nDigitsLeft == nBL_to_nDL(cnBitsAtBottom) + 1); // later
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
#endif // defined(RAM_METRICS)

    // Where does the new link go?
    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDigitsLeft);
    unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
    Word_t wIndex
        = ((wKey >> (nBitsLeft - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1));
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
         PWR_wPopCnt(pwRoot, *pwRoot, nDigitsLeft)));
    // Initialize the new link.
    DBGI(printf("pLinks %p\n", (void *)pwr_pLinks(*pwRoot)));
    DBGI(printf("memset %p\n", (void *)&pwr_pLinks(*pwRoot)[wIndex]));
    memset(&pwr_pLinks(*pwRoot)[wIndex], 0, sizeof(Link_t));
    DBGI(printf("PWR_wPopCnt A %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDigitsLeft)));
    memcpy(&pwr_pLinks(*pwRoot)[wIndex + 1], &pwr_pLinks(pwr)[wIndex],
        (wPopCnt - wIndex) * sizeof(Link_t));

    DBGI(printf("PWR_wPopCnt B %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDigitsLeft)));
    // Set the bit in the bitmap indicating that the new link exists.
    SetBit(PWR_pwBm(pwRoot, *pwRoot),
        ((wKey >> (nBitsLeft - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1)));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, *pwRoot, nDigitsLeft)));

    MyFree(pwr, nWords - sizeof(Link_t) / sizeof(Word_t));

    // Caller updates type field in *pwRoot if necessary.

    DBGI(printf("After NewLink"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SWITCH_FOR_REAL)

static Word_t
OldSwitch(Word_t *pwRoot, unsigned nDigitsLeft, unsigned nDigitsLeftUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    Word_t wLinks;
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    if (nDigitsLeftUp != cnDigitsPerWord)
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if defined(BM_SWITCH_FOR_REAL)
    {
        // How many links are there in the old switch?
        wLinks = 0;
        for (unsigned nn = 0;
            nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDigitsLeft)), cnBitsPerWord);
            nn++)
        {
            wLinks += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
        }
        assert(wLinks <= EXP(nDL_to_nBitsIndexSz(nDigitsLeft)));
        // Now we know how many links were in the old switch.
    }
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    else
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
#if ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)
    {
        wLinks = EXP(nDL_to_nBitsIndexSz(nDigitsLeft));
    }
#endif // ( ! defined(BM_SWITCH_FOR_REAL) ) || defined(BM_IN_LINK)

    // sizeof(Switch_t) includes one link; add the others
    Word_t wWords
        = (sizeof(Switch_t) + (wLinks - 1) * sizeof(Link_t)) / sizeof(Word_t);

#if defined(RAM_METRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom) + 1))
    {
        assert(nDigitsLeft == nBL_to_nDL(cnBitsAtBottom) + 1); // later
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
#endif // defined(RAM_METRICS)

    DBGR(printf("\nOldSwitch nDL %d nDLU %d wWords %"_fw"d "OWx"\n",
         nDigitsLeft, nDigitsLeftUp, wWords, wWords));

    MyFree(pwr, wWords);

    return wWords * sizeof(Word_t);

    (void)nDigitsLeft; // silence compiler
    (void)nDigitsLeftUp; // silence compiler
}

Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, unsigned nBitsLeft, int bDump)
{
    Word_t *pwRootArg = pwRoot;
#if defined(BM_IN_LINK) || defined(PP_IN_LINK)
    unsigned nBitsLeftArg = nBitsLeft;
#endif // defined(BM_IN_LINK) || defined(PP_IN_LINK)
    Word_t wRoot = *pwRoot;
    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    unsigned nDigitsLeft = nBL_to_nDL(nBitsLeft);
    unsigned nBitsIndexSz;
    Link_t *pLinks;
    Word_t wBytes = 0;

    assert(nBitsLeft >= cnBitsAtBottom);

    if ( ! bDump )
    {
        DBGR(printf("FreeArrayGuts pwR "OWx" wPrefix "OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBitsLeft, bDump));
        DBGR(printf("wRoot "OWx"\n", wRoot));
    }

    if (wRoot == 0)
    {
#if defined(BM_SWITCH_FOR_REAL)
        if (bDump)
        {
            printf(" wPrefix "OWx, wPrefix);
            printf(" nBL %2d", nBitsLeft);
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
        printf(" nBL %2d", nBitsLeft);
        printf(" pwRoot "OWx, (Word_t)pwRoot);
        printf(" wr "OWx, wRoot);
    }

    if (nBitsLeft <= cnBitsAtBottom)
    {
#if defined(PP_IN_LINK)
        if (bDump)
        {
            assert(nBitsLeftArg != cnBitsPerWord);
 
            printf(" wr_wPopCnt %3"_fw"u",
                   PWR_wPopCnt(pwRoot, NULL, nDigitsLeft));
            printf(" wr_wPrefix "OWx,
                   PWR_wPrefix(pwRoot, NULL, nDigitsLeft));
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
    assert(nDigitsLeft - tp_to_nDS(nType)
        >= nBL_to_nDL(cnBitsAtBottom));
#else // defined(TYPE_IS_RELATIVE)
    assert(tp_to_nDigitsLeft(nType) <= nBL_to_nDL(nBitsLeft));
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(PP_IN_LINK)
        if (nDigitsLeft != cnDigitsPerWord)
        {
            wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft);
        }
        else
#endif // defined(PP_IN_LINK)
        {
            wPopCnt = ls_wPopCnt(pwr);
        }

        Word_t *pwKeys = pwr_pwKeys(pwr);

        assert(nType == T_OTHER);

        if (!bDump)
        {
            return OldList(pwr, wPopCnt, nDigitsLeft);
        }
#if defined(PP_IN_LINK)
        if (nBitsLeftArg == cnBitsPerWord)
        {
            printf(" ln_wPopCnt %3"_fw"u", wPopCnt);
            printf(" wr_wPrefix        N/A");
        }
        else
        {
            printf(" wr_wPopCnt %3"_fw"u",
                   PWR_wPopCnt(pwRoot, NULL, nDigitsLeft));
            printf(" wr_wPrefix "OWx, PWR_wPrefix(pwRoot, NULL, nDigitsLeft));
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
            xx += (nBitsLeftArg == cnBitsPerWord);
#endif // defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
            if (nBitsLeft <= 8) {
                printf(" %02x", ls_pcKeys(pwr)[xx]);
            } else if (nBitsLeft <= 16) {
                printf(" %04x", ls_psKeys(pwr)[xx]);
#if (cnBitsPerWord > 32)
            } else if (nBitsLeft <= 32) {
                printf(" %08x", ls_piKeys(pwr)[xx]);
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            { printf(" "Owx, pwKeys[xx]); }
        }
        printf("\n");

        return 0;
    }
#endif // (cwListPopCntMax != 0)

    // Switch

    unsigned nDigitsLeftPrev = nDigitsLeft;
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
    nDigitsLeft = nDigitsLeft - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
    nDigitsLeft = tp_to_nDigitsLeft(nType);
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    nBitsLeft = nDL_to_nBL(nDigitsLeft);

    nBitsIndexSz = nDL_to_nBitsIndexSz(nDigitsLeft);
    pLinks = pwr_pLinks(pwr);

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBitsLeftArg == cnBitsPerWord)
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
                                      nDigitsLeft - wr_nDS(*pwRootLn));
#else // defined(TYPE_IS_RELATIVE)
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      wr_nDigitsLeft(*pwRootLn));
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
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(TYPE_IS_RELATIVE)
                    wPopCnt += 1
                            + wPrefixPopMask(nDigitsLeft - wr_nDS(*pwRootLn));
#else // defined(TYPE_IS_RELATIVE)
                    wPopCnt += wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1;
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
                PWR_wPopCnt(pwRoot, pwr, nDigitsLeft));
            printf(" wr_wPrefix "OWx, PWR_wPrefix(pwRoot, pwr, nDigitsLeft));
        }

        printf(" wr_nDL %2d", nDigitsLeft);
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask "OWx, wPrefixPopMask(nDigitsLeft));
        //printf(" pLinks "OWx, (Word_t)pLinks);
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if (nBitsLeftArg != cnBitsPerWord)
#endif // defined(BM_IN_LINK)
        {
            printf(" Bm");
            // Bitmaps are an integral number of words.
            for (unsigned nn = 0;
                 nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDigitsLeft)),
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

    DBGR(printf("nDigitsLeftPrev %d\n", nDigitsLeftPrev));
    DBGR(printf("nDigitsLeft %d\n", nDigitsLeft));
    nBitsLeft = nDL_to_nBL(nDigitsLeft - 1);
    DBGR(printf("nBitsLeft %d\n", nBitsLeft));

    nBitsIndexSz = nDL_to_nBitsIndexSz(nDigitsLeft);

    // skip link has extra prefix bits
    if (nDigitsLeftPrev > nDigitsLeft)
    {
        wPrefix = PWR_wPrefix(pwRoot, pwr, nDigitsLeft);
    }

    Word_t xx = 0;
    for (Word_t nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if ((nBitsLeftArg == cnBitsPerWord)
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
                        wPrefix | (nn << nBitsLeft), nBitsLeft, bDump);
            }

            xx++;
        }
    }

    if (bDump) return 0;

    // Someone has to clear PP and BM if PP_IN_LINK and BM_IN_LINK.
    // OldSwitch looks at BM.

    wBytes += OldSwitch(pwRootArg, nDigitsLeft, nDigitsLeftPrev);

    DBGR(printf("memset(%p, 0, %zd)\n",
         (void *)STRUCT_OF(pwRootArg, Link_t, ln_wRoot), sizeof(Link_t)));

#if defined(PP_IN_LINK) || defined(BM_IN_LINK)
    if (nBitsLeftArg == cnBitsPerWord)
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
Dump(Word_t *pwRoot, Word_t wPrefix, unsigned nBitsLeft)
{
    printf("\nDump\n");
    FreeArrayGuts(pwRoot, wPrefix, nBitsLeft, /* bDump */ 1);
    printf("End Dump\n");
}
#endif // defined(DEBUG)

#if (cwListPopCntMax != 0)
#if defined(SORT_LISTS)

// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsertWord(Word_t *pTgt, Word_t *pSrc, unsigned nKeys, Word_t wKey)
{
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
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc
    Word_t *pwr = wr_tp_pwr(wRoot, nType);

    // Validate global constant parameters set up in the header file.
#if 0
    assert(nBL_to_nDL(cnBitsAtBottom) > 0); // can't get to full pop
#endif
    assert(nBL_to_nDL(cnBitsAtBottom) + 1 <= cnDigitsPerWord);
#if defined(SKIP_LINKS)
    // type field must have enough values
    assert(nBL_to_nDL(cnBitsAtBottom) + cnMallocMask >= cnDigitsPerWord + 1);
#endif // defined(SKIP_LINKS)

    DBGI(printf("InsertGuts pwRoot %p ", (void *)pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom))
    {
#if (cnBitsAtBottom <= cnLogBitsPerWord)

        assert(!BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
            *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

#else // (cnBitsAtBottom <= cnLogBitsPerWord)

        if (pwr == NULL)
        {
            pwr = NewBitmap();
            set_wr(wRoot, pwr, T_OTHER);
            *pwRoot = wRoot;
        }

        assert(!BitIsSet(pwr, wKey & (EXP(cnBitsAtBottom) - 1)));

        DBGI(printf("SetBit(wRoot "OWx" wKey "OWx") pwRoot %p\n",
            wRoot, wKey & (EXP(cnBitsAtBottom) - 1), (void *)pwRoot));

        SetBit(pwr, wKey & (EXP(cnBitsAtBottom) - 1));

#endif // (cnBitsAtBottom <= cnLogBitsPerWord)

#if defined(PP_IN_LINK)
        // What about no_unnecessary_prefix?
        set_PWR_wPrefix(pwRoot, NULL, nDigitsLeft, wKey);
#endif // defined(PP_IN_LINK)

        return Success;
    }

// This first clause handles wRoot == 0 by treating it like a list leaf
// with zero population (and no allocated memory).
// But why is it ok to skip the test for a switch if !defined(SKIP_LINKS)
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

        if (pwr != NULL) // pointer to old List
        {
#if defined(PP_IN_LINK)
            if (nDigitsLeft != cnDigitsPerWord) {
                wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) - 1;
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
            if (nDigitsLeft != cnDigitsPerWord)
            {
                // What about no_unnecessary_prefix?
                set_PWR_wPrefix(pwRoot, NULL, nDigitsLeft, wKey);
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

        unsigned nDigitsLeftOld = nDigitsLeft;
#if (cwListPopCntMax != 0)
        if (wPopCnt < cwListPopCntMax)
        {
            // allocate a new list and init pop count in the first word
            Word_t *pwList = NewList(wPopCnt + 1, nDigitsLeft, wKey);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
                if (nBitsLeft <= 8) {
                    CopyWithInsertChar(ls_pcKeys(pwList),
                        (unsigned char *)pwKeys, wPopCnt,
                        (unsigned char)wKey);
                } else if (nBitsLeft <= 16) {
                    CopyWithInsertShort(ls_psKeys(pwList),
                        (unsigned short *)pwKeys, wPopCnt,
                        (unsigned short)wKey);
#if (cnBitsPerWord > 32)
                } else if (nBitsLeft <= 32) {
                    CopyWithInsertInt(ls_piKeys(pwList),
                        (unsigned int *)pwKeys, wPopCnt,
                        (unsigned int)wKey);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    CopyWithInsertWord(ls_pwKeys(pwList),
                        pwKeys, wPopCnt, wKey);
                }
            } else
#else // defined(SORT_LISTS)
#if defined(MIN_MAX_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);

                if (nBitsLeft <= 8)
                {
                    unsigned char cKey = wKey;

                    COPY(ls_pcKeys(pwList), pcKeys, wPopCnt);

                    if (cKey < pcKeys[wPopCnt - 1])
                    {
                        ls_pcKeys(pwList)[wPopCnt] = pcKeys[wPopCnt - 1];

                        if (cKey < pcKeys[0])
                        {
                            ls_pcKeys(pwList)[wPopCnt - 1] = pcKeys[0];
                            ls_pcKeys(pwList)[0] = wKey;
                        }
                        else
                        {
                            ls_pcKeys(pwList)[wPopCnt - 1] = wKey;
                        }
                    }
                    else
                    {
                        ls_pcKeys(pwList)[wPopCnt] = wKey;
                    }
                }
                else if (nBitsLeft <= 16)
                {
                    unsigned short sKey = wKey;

                    COPY(ls_psKeys(pwList), psKeys, wPopCnt);

                    if (sKey < psKeys[wPopCnt - 1])
                    {
                        ls_psKeys(pwList)[wPopCnt] = psKeys[wPopCnt - 1];

                        if (sKey < psKeys[0])
                        {
                            ls_psKeys(pwList)[wPopCnt - 1] = psKeys[0];
                            ls_psKeys(pwList)[0] = wKey;
                        }
                        else
                        {
                            ls_psKeys(pwList)[wPopCnt - 1] = wKey;
                        }
                    }
                    else
                    {
                        ls_psKeys(pwList)[wPopCnt] = wKey;
                    }
                }
#if (cnBitsPerWord > 32)
                else if (nBitsLeft <= 32)
                {
                    unsigned int iKey = wKey;

                    COPY(ls_piKeys(pwList), piKeys, wPopCnt);

                    if (iKey < piKeys[wPopCnt - 1])
                    {
                        ls_piKeys(pwList)[wPopCnt] = piKeys[wPopCnt - 1];

                        if (iKey < piKeys[0])
                        {
                            ls_piKeys(pwList)[wPopCnt - 1] = piKeys[0];
                            ls_piKeys(pwList)[0] = wKey;
                        }
                        else
                        {
                            ls_piKeys(pwList)[wPopCnt - 1] = wKey;
                        }
                    }
                    else
                    {
                        ls_piKeys(pwList)[wPopCnt] = wKey;
                    }
                }
#endif // (cnBitsPerWord > 32)
                else
#endif // defined(COMPRESSED_LISTS)
                {
                    COPY(ls_pwKeys(pwList), pwKeys, wPopCnt);

                    if (wKey < pwKeys[wPopCnt - 1])
                    {
                        ls_pwKeys(pwList)[wPopCnt] = pwKeys[wPopCnt - 1];

                        if (wKey < pwKeys[0])
                        {
                            ls_pwKeys(pwList)[wPopCnt - 1] = pwKeys[0];
                            ls_pwKeys(pwList)[0] = wKey;
                        }
                        else
                        {
                            ls_pwKeys(pwList)[wPopCnt - 1] = wKey;
                        }
                    }
                    else
                    {
                        ls_pwKeys(pwList)[wPopCnt] = wKey;
                    }
                }
            } else
#else // defined(MIN_MAX_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
                if (nBitsLeft <= 8) {
                    COPY(ls_pcKeys(pwList), pcKeys, wPopCnt);
                } else if (nBitsLeft <= 16) {
                    COPY(ls_psKeys(pwList), psKeys, wPopCnt);
#if (cnBitsPerWord > 32)
                } else if (nBitsLeft <= 32) {
                    COPY(ls_piKeys(pwList), piKeys, wPopCnt);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                { COPY(ls_pwKeys(pwList), pwKeys, wPopCnt); }
            }
#endif // defined(MIN_MAX_LISTS)
#endif // defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
                if (nBitsLeft <= 8) {
                    ls_pcKeys(pwList)[wPopCnt] = wKey;
                } else if (nBitsLeft <= 16) {
                    ls_psKeys(pwList)[wPopCnt] = wKey;
#if (cnBitsPerWord > 32)
                } else if (nBitsLeft <= 32) {
                    ls_piKeys(pwList)[wPopCnt] = wKey;
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                { ls_pwKeys(pwList)[wPopCnt] = wKey; }
            }

            set_wr(wRoot, pwList, T_OTHER);
            *pwRoot = wRoot; // install new
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            Word_t w;

            // List is full; insert a switch

#if defined(PP_IN_LINK)
            if (nDigitsLeft < cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
#if defined(SKIP_LINKS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);
                Word_t wSuffix;
#endif // defined(COMPRESSED_LISTS)
                if (cwListPopCntMax != 0) // use const for compile time check
                {
                    Word_t wMax, wMin;
#if defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
#if defined(COMPRESSED_LISTS)
                    if (nBitsLeft <= 8) {
                        wMin = ls_pcKeys(pwr)[0];
                        wMax = ls_pcKeys(pwr)[wPopCnt - 1];
                        wSuffix = wKey & 0xff;
                    } else if (nBitsLeft <= 16) {
                        wMin = ls_psKeys(pwr)[0];
                        wMax = ls_psKeys(pwr)[wPopCnt - 1];
                        wSuffix = wKey & 0xffff;
#if (cnBitsPerWord > 32)
                    } else if (nBitsLeft <= 32) {
                        wMin = ls_piKeys(pwr)[0];
                        wMax = ls_piKeys(pwr)[wPopCnt - 1];
                        wSuffix = wKey & 0xffffffff;
#endif // (cnBitsPerWord > 32)
                    } else 
#endif // defined(COMPRESSED_LISTS)
                    { wMin = pwKeys[0]; wMax = pwKeys[wPopCnt - 1]; }
#else // defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
                    // walk the list to find max and min
                    wMin = (Word_t)-1;
                    wMax = 0;

#if defined(COMPRESSED_LISTS)
                        wSuffix = (nBitsLeft <= 8) ? (wKey & 0xff)
#if (cnBitsPerWord > 32)
                                : (nBitsLeft > 16) ? (wKey & 0xffffffff)
#endif // (cnBitsPerWord > 32)
                                : (wKey & 0xffff);
#endif // defined(COMPRESSED_LISTS)

                    for (w = 0; w < wPopCnt; w++)
                    {
#if defined(COMPRESSED_LISTS)
                        if (nBitsLeft <= 8) {
                            if (ls_pcKeys(pwr)[w] < wMin)
                                wMin = ls_pcKeys(pwr)[w];
                            if (ls_pcKeys(pwr)[w] > wMax)
                                wMax = ls_pcKeys(pwr)[w];
                        } else if (nBitsLeft <= 16) {
                            if (ls_psKeys(pwr)[w] < wMin)
                                wMin = ls_psKeys(pwr)[w];
                            if (ls_psKeys(pwr)[w] > wMax)
                                wMax = ls_psKeys(pwr)[w];
#if (cnBitsPerWord > 32)
                        } else if (nBitsLeft <= 32) {
                            if (ls_piKeys(pwr)[w] < wMin)
                                wMin = ls_piKeys(pwr)[w];
                            if (ls_piKeys(pwr)[w] > wMax)
                                wMax = ls_piKeys(pwr)[w];
#endif // (cnBitsPerWord > 32)
                        } else 
#endif // defined(COMPRESSED_LISTS)
                        {
                            if (pwKeys[w] < wMin) wMin = pwKeys[w];
                            if (pwKeys[w] > wMax) wMax = pwKeys[w];
                        }
                    }
#endif // defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
                    DBGI(printf("wMin "OWx" wMax "OWx"\n", wMin, wMax));

#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                    if (nBitsLeft <= 32)
#else // (cnBitsPerWord > 32)
                    if (nBitsLeft <= 16)
#endif // (cnBitsPerWord > 32)
                    {
                        nDigitsLeft
                            = nBL_to_nDL(
                                LOG((EXP(cnBitsAtBottom) - 1)
                                        | ((wSuffix ^ wMin)
                                        | (wSuffix ^ wMax)))
                                    + 1);
                    }
                    else
#endif // defined(COMPRESSED_LISTS)
                    {
                        nDigitsLeft
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
                    nDigitsLeft = nBL_to_nDL(cnBitsAtBottom) + 1;
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
            if (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom))
            {
                DBGI(printf("InsertGuts nDigitsLeft"
                            " <= nBL_to_nDL(cnBitsAtBottom)\n"));

                nDigitsLeft = nBL_to_nDL(cnBitsAtBottom) + 1;
            }
#else // defined(SKIP_LINKS)
            assert(nDigitsLeft > nBL_to_nDL(cnBitsAtBottom));
#endif // defined(SKIP_LINKS)
            NewSwitch(pwRoot, wKey, nDigitsLeft, nDigitsLeftOld,
                      /* wPopCnt */ 0);

#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
            unsigned nBitsLeftOld = nDL_to_nBL(nDigitsLeftOld);
#else // defined(SKIP_LINKS)
// Revisit the use of "Old" here.
            unsigned nBitsLeftOld = nDL_to_nBL(nDigitsLeft);
#endif // defined(SKIP_LINKS)
            if (nBitsLeftOld <= 8) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pcKeys[w] | (wKey & ~0xff),
                           nDigitsLeftOld);
                }
            } else if (nBitsLeftOld <= 16) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, psKeys[w] | (wKey & ~0xffff),
                           nDigitsLeftOld);
                }
#if (cnBitsPerWord > 32)
            } else if (nBitsLeftOld <= 32) {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, piKeys[w] | (wKey & ~(Word_t)0xffffffff),
                           nDigitsLeftOld);
                }
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pwKeys[w], nDigitsLeftOld);
                }
            }

            DBGI(printf("Just Before InsertGuts calls Insert"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
            Insert(pwRoot, wKey, nDigitsLeftOld);
        }

#if (cwListPopCntMax != 0)
        // Hmm.  Should this be nDigitsLeftOld?
        if (wPopCnt != 0) OldList(pwr, wPopCnt, nDigitsLeftOld);
#endif // (cwListPopCntMax != 0)
    }
#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    else
    {
#if defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
#if defined(TYPE_IS_RELATIVE)
        unsigned nDLR = nDigitsLeft - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
        unsigned nDLR = tp_to_nDigitsLeft(nType);
#endif // defined(TYPE_IS_RELATIVE)
        Word_t wPrefix;
        // Test to see if this is a missing link case.
        // If not, then it is a prefix mismatch case.
        // nDigitsLeft does not include any skip indicated in nType.
        // If nDS == 0 or nDigitsLeft == tp_to_nDigitsLeft, then we know
        // it is a missing link because it can't be a prefix mismatch.
        // Unfortunately, nDS != 0 (or the other) does not imply a prefix
        // mismatch.
        // if (wPrefix == w_wPrefix(wKey, nDLR))
        // It's a bit of a bummer that we are doing the prefix check again.
        // Can we avoid it as follows:
        if ((nDLR == nDigitsLeft)
            || ((wPrefix = PWR_wPrefix(pwRoot, pwr, nDLR))
                == w_wPrefixNotAtTop(wKey, nDLR)))
        // If nDS != 0 then we're not at the top or PP_IN_LINK is not defined.
#endif // defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
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
            set_wr_nType(*pwRoot, nDS_to_tp(nDigitsLeft - nDLR));
#else // defined(TYPE_IS_RELATIVE)
            set_wr_nType(*pwRoot, nDigitsLeft_to_tp(nDLR));
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            Insert(pwRoot, wKey, nDigitsLeft);
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
            unsigned nDigitsLeftRoot;
            Word_t wPopCnt;

#if defined(TYPE_IS_RELATIVE)
            nDigitsLeftRoot = nDigitsLeft - tp_to_nDS(nType);
#else // defined(TYPE_IS_RELATIVE)
            nDigitsLeftRoot = tp_to_nDigitsLeft(nType);
#endif // defined(TYPE_IS_RELATIVE)

            // Can't have a prefix mismatch if there is no skip.
            assert(nDigitsLeftRoot < nDigitsLeft);

            unsigned nDigitsLeftUp = nDigitsLeft;

            // figure new nDigitsLeft for old parent link
            Word_t wPrefix = PWR_wPrefix(pwRoot, pwr, nDigitsLeftRoot);
            nDigitsLeft = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
            // nDigitsLeft includes the highest order digit that is different.

            assert(nDigitsLeft > nDigitsLeftRoot);

            if ((wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDigitsLeftRoot)) == 0)
            {
                // full pop overflow
                wPopCnt = wPrefixPopMask(nDigitsLeftRoot) + 1;
            }

            // Have to get old prefix before inserting the new switch because
            // NewSwitch copies to *pwRoot.
            // Also deal with switch at top with no link if PP_IN_LINK.

            unsigned nIndex;

#if defined(PP_IN_LINK)
            // PP_IN_LINK => no skip link at top => no prefix mismatch at top
            assert(nDigitsLeftUp < cnDigitsPerWord);
#endif // defined(PP_IN_LINK)

            // todo nBitsIndexSz; wide switch
            nIndex = (wPrefix >> nDL_to_nBL_NotAtTop(nDigitsLeft - 1))
                & (EXP(nDL_to_nBitsIndexSz(nDigitsLeft)) - 1);
            // nIndex is the logical index in new switch.
            // It may not be the same as the index in the old switch.

#if defined(BM_IN_LINK)
            // Save the old bitmap before it is trashed by NewSwitch.
            // Is it possible that nDigitsLeftUp != cnDigitsPerWord and
            // we are at the top?
            Link_t ln;
            Word_t wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDigitsLeftRoot));
            if (nDigitsLeftUp != cnDigitsPerWord)
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
                             wPrefix, nDigitsLeft, nDigitsLeftUp, wPopCnt);

#if defined(BM_SWITCH_FOR_REAL)
#if defined(BM_IN_LINK)
            if (nDigitsLeftUp != cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                // Switch was created with only one link based on wKey
                // passed in.  Unless BM_IN_LINK && switch is at top.
                nIndex = 0;
            }
#endif // defined(BM_SWITCH_FOR_REAL)

#if defined(BM_IN_LINK)
            if (nDigitsLeftUp != cnDigitsPerWord)
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
                   nDigitsLeft, nDigitsLeftRoot, nDigitsLeftUp));
            set_wr_nDS(wRoot, nDigitsLeft - nDigitsLeftRoot - 1);
#endif // defined(TYPE_IS_RELATIVE)
            // Copy wRoot from old link to new link.
            pwr_pLinks(pwSw)[nIndex].ln_wRoot = wRoot;

#if defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
            if (nDigitsLeftRoot == nDigitsLeft - 1)
            {
                // The previously necessary prefix in the old switch
                // is now unnecessary.
                DBGI(printf("nDLR %d nDL %d\n",
                            nDigitsLeftRoot, nDigitsLeft));
            }
            else
#endif // defined(NO_UNNECESSARY_PREFIX)
            {
                set_PWR_wPrefix(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                                nDigitsLeftRoot, wPrefix);
            }

            set_PWR_wPopCnt(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                            nDigitsLeftRoot, wPopCnt);
#else // defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
           // We could go to the trouble of zeroing the no-longer necessary
           // prefix in the old switch. 
#endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)

            Insert(pwRoot, wKey, nDigitsLeftUp);
        }
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)

    return Success;
}

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    Word_t *pwr = wr_pwr(wRoot);

    DBGR(printf("RemoveGuts\n"));

#if (cwListPopCntMax != 0)
    if (nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom))
#else // (cwListPopCntMax != 0)
    assert(nDigitsLeft <= nBL_to_nDL(cnBitsAtBottom));
#endif // (cwListPopCntMax != 0)
    {
#if (cnBitsAtBottom <= cnLogBitsPerWord)

        ClrBitInWord(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
        // What if link has more than just ln_wRoot due
        // to BM_IN_LINK and/or PP_IN_LINK?
        // What if population just went to 0?  Should we clear the rest
        // of the link?
        // Or can we rely on cleanup phase in Remove to do it?
        *pwRoot = wRoot;

#else // (cnBitsAtBottom <= cnLogBitsPerWord)

        ClrBit(pwr, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
#if defined(PP_IN_LINK)
        if (PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) == 0)
        {
            DBGL(printf("RemoveGuts OldBitmap nDigitsLeft %d\n",
                 nDigitsLeft));
            OldBitmap(pwr); *pwRoot = 0;
            // Do we need to clear the rest of the link also?
        }
#else // defined(PP_IN_LINK)

        // Free the bitmap if it is empty.
        for (Word_t ww = 0;
             ww < EXP(cnBitsAtBottom - cnLogBitsPerWord); ww++)
        {
            if (__builtin_popcountll(pwr[ww]))
            {
                goto done;
            }
        }

        OldBitmap(pwr); *pwRoot = 0;
done:

#endif // defined(PP_IN_LINK)

#endif // (cnBitsAtBottom <= cnLogBitsPerWord)

        if (*pwRoot == 0)
        {
            // We return to Remove which will clean up ancestors.
            DBGR(printf("RemoveGuts *pwRoot is now 0\n"));
        }
    }
#if (cwListPopCntMax != 0)
    else
    {
#if defined(COMPRESSED_LISTS)

        unsigned nBitsLeft = nDL_to_nBL(nDigitsLeft);

#endif // defined(COMPRESSED_LISTS)

        Word_t wPopCnt;

#if defined(PP_IN_LINK)
        if (nDigitsLeft != cnDigitsPerWord)
        {
            wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) + 1;
        }
        else
#endif // defined(PP_IN_LINK)
        {
            wPopCnt = ls_wPopCnt(pwr);
        }

        if (wPopCnt == 1)
        {
            OldList(pwr, wPopCnt, nDigitsLeft);
            *pwRoot = 0;
            // Do we need to clear the rest of the link also?
            // BUG:  We should check if the switch is empty and free it
            // (and on up the tree as necessary).
        }
        else
        {
            Word_t *pwKeys = pwr_pwKeys(pwr);

            unsigned nIndex;
            for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
                (nBitsLeft <= 8)
                    ? (pwr_pcKeys(pwr)[nIndex] != (unsigned char )wKey)
                : (nBitsLeft <= 16)
                    ? (pwr_psKeys(pwr)[nIndex] != (unsigned short)wKey)
#if (cnBitsPerWord > 32)
                : (nBitsLeft <= 32)
                    ? (pwr_piKeys(pwr)[nIndex] != (unsigned int)wKey)
#endif // (cnBitsPerWord > 32)
                    : (pwKeys[nIndex] != wKey);
#else // defined(COMPRESSED_LISTS)
                pwKeys[nIndex] != wKey;
#endif // defined(COMPRESSED_LISTS)
                nIndex++)
                ; // semicolon on separate line to silence compiler warning

            Word_t *pwList;
            if (ListWords(wPopCnt - 1, nDigitsLeft)
                    != ListWords(wPopCnt, nDigitsLeft))
            {
                // Malloc a new, smaller list.
                pwList = NewList(wPopCnt - 1, nDigitsLeft, wKey);
                COPY(pwList, pwr, ListWords(wPopCnt - 1, nDigitsLeft));
            }
            else
            {
                pwList = pwr;
            }

#if defined(COMPRESSED_LISTS)
            if (nBitsLeft <= 8) {
                MOVE(&pwr_pcKeys(pwList)[nIndex],
                     &pwr_pcKeys(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
            } else if (nBitsLeft <= 16) {
                MOVE(&pwr_psKeys(pwList)[nIndex],
                     &pwr_psKeys(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#if (cnBitsPerWord > 32)
            } else if (nBitsLeft <= 32) {
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
                OldList(pwr, wPopCnt, nDigitsLeft);
                set_wr(wRoot, pwList, T_OTHER);
                *pwRoot = wRoot;
                pwKeys = pwr_pwKeys(pwList);
            }

#if defined(MIN_MAX_LISTS) && !defined(SORT_LISTS)
            // if we removed min or max, then we need to find a new one
            if ((nIndex == 0) || (nIndex == wPopCnt - 1))
            {
                for (unsigned nn = 1; nn < wPopCnt - 2; nn++)
                {
#if defined(COMPRESSED_LISTS)
                    if (nBitsLeft <= 8) {
                        unsigned char knn = pwr_pcKeys(pwList)[nn];
                        if (knn < pwr_pcKeys(pwList)[0])
                        {
                            pwr_pcKeys(pwList)[nn] = pwr_pcKeys(pwList)[0];
                            pwr_pcKeys(pwList)[0] = knn;
                        }
                        if (knn > pwr_pcKeys(pwList)[wPopCnt - 2])
                        {
                            pwr_pcKeys(pwList)[nn]
                                = pwr_pcKeys(pwList)[wPopCnt - 2];
                            pwr_pcKeys(pwList)[wPopCnt - 2] = knn;
                        }
                    } else if (nBitsLeft <= 16) {
                        unsigned short knn = pwr_psKeys(pwList)[nn];
                        if (knn < pwr_psKeys(pwList)[0])
                        {
                            pwr_psKeys(pwList)[nn] = pwr_psKeys(pwList)[0];
                            pwr_psKeys(pwList)[0] = knn;
                        }
                        if (knn > pwr_psKeys(pwList)[wPopCnt - 2])
                        {
                            pwr_psKeys(pwList)[nn]
                                = pwr_psKeys(pwList)[wPopCnt - 2];
                            pwr_psKeys(pwList)[wPopCnt - 2] = knn;
                        }
#if (cnBitsPerWord > 32)
                    } else if (nBitsLeft <= 32) {
                        unsigned int knn = pwr_piKeys(pwList)[nn];
                        if (knn < pwr_piKeys(pwList)[0])
                        {
                            pwr_piKeys(pwList)[nn] = pwr_piKeys(pwList)[0];
                            pwr_piKeys(pwList)[0] = knn;
                        }
                        if (knn > pwr_piKeys(pwList)[wPopCnt - 2])
                        {
                            pwr_piKeys(pwList)[nn]
                                = pwr_piKeys(pwList)[wPopCnt - 2];
                            pwr_piKeys(pwList)[wPopCnt - 2] = knn;
                        }
#endif // (cnBitsPerWord > 32)
                    } else
#endif // defined(COMPRESSED_LISTS)
                    {
                        Word_t knn = pwKeys[nn];
                        if (knn < pwKeys[0])
                        {
                            pwKeys[nn] = pwKeys[0];
                            pwKeys[0] = knn;
                        }
                        if (knn > pwKeys[wPopCnt - 2])
                        {
                            pwKeys[nn] = pwKeys[wPopCnt - 2];
                            pwKeys[wPopCnt - 2] = knn;
                        }
                    }
                }
            }
#endif // defined(MIN_MAX_LISTS) && !defined(SORT_LISTS)

#if defined(PP_IN_LINK)
            if (nDigitsLeft == cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
            {
                set_ls_wPopCnt(pwList, wPopCnt - 1);
            }
        }
    }
#endif // (cwListPopCntMax != 0)

    (void)pwRoot; (void)wKey; (void)nDigitsLeft; (void)wRoot;

    return Success;
}

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
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    unsigned nType = wr_nType(wRoot);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    Word_t *pwr = wr_tp_pwr(wRoot, nType);
    Word_t wPopCnt;

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
    {
        wPopCnt = ls_wPopCnt(pwr);
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
        assert(tp_to_nDigitsLeft(nType) == cnDigitsPerWord);
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
// And if nBL_to_nDL(cnBitsAtBottom) == cnDigitsPerWord - 1, then it could be
// a pointer to a bitmap?
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
                    = PWR_wPopCnt(pwRootLn, NULL, wr_nDigitsLeft(*pwRootLn));
#endif // defined(TYPE_IS_RELATIVE)
            }
            else
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            {
                wPopCntLn
                    = PWR_wPopCnt(pwRootLn, NULL, cnDigitsPerWord - 1);
            }

#if defined(DEBUG_INSERT)
            if (wPopCntLn != 0)
            {
                printf("Pop sum");
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                unsigned nDigitsLeft
#if defined(TYPE_IS_RELATIVE)
                    = cnDigitsPerWord - 1 - wr_nDS(*pwRootLn);
#else // defined(TYPE_IS_RELATIVE)
                    = wr_nDigitsLeft(*pwRootLn);
#endif // defined(TYPE_IS_RELATIVE)
                printf(" mask "OWx" %"_fw"d",
                    wPrefixPopMask(nDigitsLeft),
                    wPrefixPopMask(nDigitsLeft));
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
                int nDigitsLeft =
#if defined(TYPE_IS_RELATIVE)
                    cnDigitsPerWord - wr_nDS(*pwRootLn)
#else // defined(TYPE_IS_RELATIVE)
                    wr_nDigitsLeft(*pwRootLn)
#endif // defined(TYPE_IS_RELATIVE)
                    ;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if defined(DEBUG_INSERT)
                printf("Pop sum (full)");
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                printf(" mask "Owx" %"_fw"d\n",
                    wPrefixPopMask(nDigitsLeft), wPrefixPopMask(nDigitsLeft));
                printf("nn %d wPopCntLn %"_fw"d "OWx"\n",
                    nn, wPrefixPopMask(nDigitsLeft) + 1,
                    wPrefixPopMask(nDigitsLeft) + 1);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#endif // defined(DEBUG_INSERT)

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                wPopCnt += wPrefixPopMask(nDigitsLeft) + 1;
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                wPopCnt += wPrefixPopMask(cnDigitsPerWord - 1) + 1;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            }
        }
        }
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if ! defined(NDEBUG)
        int nDigitsLeft =
#if defined(TYPE_IS_RELATIVE)
            cnDigitsPerWord - tp_to_nDS(nType)
#else // defined(TYPE_IS_RELATIVE)
            tp_to_nDigitsLeft(nType)
#endif // defined(TYPE_IS_RELATIVE)
            ;
        assert(wPopCnt - 1 <= wPrefixPopMask(nDigitsLeft));
#endif // ! defined(NDEBUG)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#else // defined(PP_IN_LINK)
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        int nDigitsLeft =
#if defined(TYPE_IS_RELATIVE)
            cnDigitsPerWord - tp_to_nDS(nType)
#else // defined(TYPE_IS_RELATIVE)
            tp_to_nDigitsLeft(nType)
#endif // defined(TYPE_IS_RELATIVE)
            ;
        wPopCnt = PWR_wPopCnt(NULL, pwr, nDigitsLeft);
        if (wPopCnt == 0)
        {
            wPopCnt = wPrefixPopMask(nDigitsLeft) + 1;
        }
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        wPopCnt = PWR_wPopCnt(NULL, pwr, cnDigitsPerWord);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#endif // defined(PP_IN_LINK)
    }

#if defined(DEBUG)
    if (wPopCnt != wInserts)
    {
        printf("\nwPopCnt %"_fw"d wInserts %"_fw"d\n", wPopCnt, wInserts);
        Dump(pwRootLast, 0, cnBitsPerWord);
    }
    assert(wPopCnt == wInserts);
#endif // defined(DEBUG)

    return wPopCnt;

#else // (cnDigitsPerWord != 1)

    (void)PArray; (void)wKey0; (void)wKey1, (void)PJError;
    return wInserts;

#endif // (cnDigitsPerWord != 1)
}

