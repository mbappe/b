
// @(#) $Id: b.c,v 1.188 2014/04/23 14:52:46 mike Exp mike $
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

static Word_t
NewBitmap(void)
{
    unsigned nWords = EXP(cnBitsAtBottom) / cnBitsPerWord;

    Word_t w = MyMalloc(nWords);

    METRICS(j__AllocWordsJLB1 += nWords); // JUDYA
    METRICS(j__AllocWordsJL12 += nWords); // JUDYB -- overloaded

    DBGM(printf("NewBitmap nBitsAtBottom %u nBits "OWx
      " nBytes "OWx" nWords %d w "OWx"\n",
        cnBitsAtBottom, EXP(cnBitsAtBottom),
        EXP(cnBitsAtBottom) / cnBitsPerByte, nWords, w));

    memset((void *)w, 0, nWords * sizeof(Word_t));

    return w;
}

Word_t
OldBitmap(Word_t wRoot)
{
    unsigned nWords = EXP(cnBitsAtBottom) / cnBitsPerWord;

    MyFree((Word_t *)wRoot, nWords);

    METRICS(j__AllocWordsJLB1 -= nWords); // JUDYA
    METRICS(j__AllocWordsJL12 -= nWords); // JUDYB -- overloaded

    return nWords * sizeof(Word_t);
}

#endif // (cnBitsAtBottom > cnLogBitsPerWord)

// Allocate a new switch.
// Zero its links.
// Initialize its prefix if there is one.  Need to know nDigitsLeftUp for
// PP_IN_LINK to figure out if the prefix exists.
// Initialize its bitmap if there is one.  Need to know nDigitsLeftUp for
// BM_IN_LINK to figure out if the bitmap exists.
// Install wRoot at pwRoot.  Need to know nDigitsLeft.
// Account for the memory (for both JUDYA and JUDYB columns in Judy1LHTime).
// Need to know if we are at the bottom so we should count it as a bitmap.
static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft,
          unsigned nDigitsLeftUp, Word_t wPopCnt)
{
    Word_t *pwr;
    unsigned nWords;
    unsigned nBytesOfLinks;

    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);

#if defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    if (nDigitsLeftUp == cnDigitsPerWord)
    {
        unsigned nLinks;

        nLinks = EXP(nDL_to_nBitsIndexSz(nDigitsLeft));

        // sizeof(Switch_t) includes one link; add the others
        nWords = (sizeof(Switch_t) + (nLinks - 1) * sizeof(Link_t))
            / sizeof(Word_t);

        DBGI(printf("nLinks %d sizeof(Switch_t) %zd sizeof(Link_t) %zd",
             nLinks, sizeof(Switch_t), sizeof(Link_t)));

        nBytesOfLinks = nLinks * sizeof(Link_t);
    }
    else
#endif // defined(BM_SWITCH_FOR_REAL) && defined(BM_IN_LINK)
    {
        nWords = sizeof(Switch_t) / sizeof(Word_t);

        nBytesOfLinks = sizeof(pwr_pLinks(pwr));
    }

    pwr = (Word_t *)MyMalloc(nWords);

    memset(pwr_pLinks(pwr), 0, nBytesOfLinks);

#if defined(RAM_METRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        // embedded bitmap
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += nWords); // JUDYA
        METRICS(j__AllocWordsJL12 += nWords); // JUDYB -- overloaded
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  += nWords); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  += nWords); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 += nWords); // JUDYB
    }
#endif // defined(RAM_METRICS)

    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx" nDL %d nDLU %d) pwr %p\n",
        (void *)pwRoot, wKey, nDigitsLeft, nDigitsLeftUp, (void *)pwr));

    set_wr(*pwRoot, pwr, nDigitsLeft_to_tp(nDigitsLeft));

#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
    if (nDigitsLeftUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
        unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDigitsLeft);
#if defined(BM_SWITCH_FOR_REAL)

        memset(PWR_pwBm(pwRoot, pwr), 0, sizeof(PWR_pwBm(pwRoot, pwr)));

        unsigned nBitsLeft = nDL_to_nBL_NotAtTop(nDigitsLeft);
        // leave nBitsLeft greater than cnBitsPerWord intentionally for now

        Word_t wIndex
            = (wKey >> (nBitsLeft - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1);

        SetBit(PWR_pwBm(pwRoot, pwr), wIndex);

#else // defined(BM_SWITCH_FOR_REAL)

// Mind the high-order bits of the bitmap word if/when the bitmap is smaller
// than a whole word.  See OldSwitch.
        if (nBitsIndexSz < cnLogBitsPerWord)
        {
            *PWR_pwBm(pwRoot, pwr) = (Word_t)-1;
            //*PWR_pwBm(pwRoot, pwr) = EXP(nBitsIndexSz) - 1;
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
            && ((nDigitsLeft - 1) > cnDigitsAtBottom)
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
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        // embedded bitmap
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(Link_t) / sizeof(Word_t)); // JUDYA
        METRICS(j__AllocWordsJL12 += sizeof(Link_t) / sizeof(Word_t)); // JUDYB
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  += sizeof(Link_t) / sizeof(Word_t)); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  += sizeof(Link_t) / sizeof(Word_t)); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 += sizeof(Link_t) / sizeof(Word_t)); // JUDYB
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

    // Remember to finish updating *pwRoot.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    set_wr_nType(*pwRoot, nDigitsLeft_to_tp(nDigitsLeft));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    DBGI(printf("After NewLink"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SWITCH_FOR_REAL)

static Word_t
OldSwitch(Word_t *pwRoot, unsigned nDigitsLeft, unsigned nDigitsLeftUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);
    unsigned nWords = sizeof(Switch_t) / sizeof(Word_t);

#if defined(BM_SWITCH_FOR_REAL)
    Word_t wPopCnt;
#if defined(BM_IN_LINK)
    if (nDigitsLeftUp == cnDigitsPerWord)
    {
        wPopCnt = EXP(nDL_to_nBitsIndexSz(nDigitsLeft));
    }
    else
#endif // defined(BM_IN_LINK)
    {
        // How many links are there in the old switch?
        wPopCnt = 0;
        for (unsigned nn = 0;
            nn < DIV_UP(EXP(nDL_to_nBitsIndexSz(nDigitsLeft)), cnBitsPerWord);
            nn++)
        {
            wPopCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
        }
        // trim the count if it is too big due to extra one bits in the bitmap
        if (wPopCnt > EXP(nDL_to_nBitsIndexSz(nDigitsLeft)))
        {
            assert(wPopCnt == cnBitsPerWord);
            assert(PWR_pwBm(pwRoot, pwr)[0] == (Word_t)-1);
            wPopCnt = EXP(nDL_to_nBitsIndexSz(nDigitsLeft));
        }
        // Now we know how many links were in the old switch.
    }

    // sizeof(Switch_t) includes one link; add the others
    nWords += (wPopCnt - 1) * sizeof(Link_t) / sizeof(Word_t);

#endif // defined(BM_SWITCH_FOR_REAL)

#if defined(RAM_METRICS)
    if ((cnBitsAtBottom <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL12 -= nWords); // JUDYB -- overloaded
    }
    else
    {
#if defined(BM_SWITCH)
        METRICS(j__AllocWordsJBB  -= nWords); // JUDYA
#else // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU  -= nWords); // JUDYA
#endif // defined(BM_SWITCH)
        METRICS(j__AllocWordsJBU4 -= nWords); // JUDYB
    }
#endif // defined(RAM_METRICS)

    DBGR(printf("\nOldSwitch nDL %d nDLU %d nWords %d 0x%x\n",
         nDigitsLeft, nDigitsLeftUp, nWords, nWords));

    MyFree(pwr, nWords);

    return nWords * sizeof(Word_t);

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
    unsigned nDigitsLeft = nBL_to_nDL(nBitsLeft);
    Word_t *pwr;
    unsigned nBitsIndexSz;
    Link_t *pLinks;
    unsigned nType;
    Word_t wBytes = 0;

    if ( ! bDump )
    {
        DBGR(printf("FreeArrayGuts pwR "OWx" wPrefix "OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBitsLeft, bDump));
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
            return OldBitmap(wRoot);
        }

        printf(" nWords %2"_fw"d", EXP(cnBitsAtBottom) / cnBitsPerWord);
        for (unsigned nn = 0;
            //(nn < EXP(cnBitsAtBottom) / cnBitsPerWord) && (nn < 8);
            (nn < EXP(cnBitsAtBottom) / cnBitsPerWord);
             nn++)
        {
            if ((nn % 8) == 0) {
                printf("\n");
            }
            printf(" "Owx, ((Word_t *)wRoot)[nn]);
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

    nType = wr_nType(wRoot); (void)nType; // silence gcc

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if (tp_to_nDigitsLeft(nType) > nBL_to_nDL(nBitsLeft))
    {
        DBGI(printf("\nnType %d\n", nType));
    }
    assert(tp_to_nDigitsLeft(nType) <= nBL_to_nDL(nBitsLeft));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    pwr = wr_tp_pwr(wRoot, nType);

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

        assert(nType == 0);

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
                printf(" %02x", ls_pcKeys(wRoot)[xx]);
            } else if (nBitsLeft <= 16) {
                printf(" %04x", ls_psKeys(wRoot)[xx]);
#if (cnBitsPerWord > 32)
            } else if (nBitsLeft <= 32) {
                printf(" %08x", ls_piKeys(wRoot)[xx]);
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
    nDigitsLeft = tp_to_nDigitsLeft(nType);
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
// And if cnDigitsAtBottom == cnDigitsPerWord - 1, then it could be a
// pointer to a bitmap?
                Word_t wPopCntLn;
#if defined(SKIP_LINKS)
                unsigned nTypeLn = wr_nType(*pwRootLn);
                if (tp_bIsSwitch(nTypeLn))
                {
                    wPopCntLn
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      wr_nDigitsLeft(*pwRootLn));
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
                    wPopCnt += wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1;
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

    nBitsLeft = nDL_to_nBL(nDigitsLeft - 1);
    //DBGR(printf("nBitsLeft %d\n", nBitsLeft));

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
    Word_t *pwr;
    unsigned nType;

    // Validate global constant parameters set up in the header file.
#if 0
    assert(cnDigitsAtBottom > 0); // can't get to full pop
#endif
    assert(cnDigitsAtBottom + 1 <= cnDigitsPerWord);
#if defined(SKIP_LINKS)
    // type field must have enough values
    assert(cnDigitsAtBottom + cnMallocMask >= cnDigitsPerWord + 1);
#endif // defined(SKIP_LINKS)

    DBGI(printf("InsertGuts pwRoot %p ", (void *)pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
#if (cnBitsAtBottom <= cnLogBitsPerWord)

        assert(!BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
            *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

#else // (cnBitsAtBottom <= cnLogBitsPerWord)

        if (wRoot == 0)
        {
            *pwRoot = wRoot = NewBitmap();
        }

        assert(!BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

        DBGI(printf("SetBit(wRoot "OWx" wKey "OWx") pwRoot %p\n",
            wRoot, wKey & (EXP(cnBitsAtBottom) - 1), (void *)pwRoot));

        SetBit(wRoot, wKey & (EXP(cnBitsAtBottom) - 1));

#endif // (cnBitsAtBottom <= cnLogBitsPerWord)

#if defined(PP_IN_LINK)
        // What about no_unnecessary_prefix?
        set_PWR_wPrefix(pwRoot, NULL, nDigitsLeft, wKey);
#endif // defined(PP_IN_LINK)

        return Success;
    }

    nType = wr_nType(wRoot); (void)nType; // silence gcc

    pwr = wr_tp_pwr(wRoot, nType);

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

            set_wr(wRoot, pwList, /* nType */ 0); // !tp_bIsSwitch

            *pwRoot = wRoot; // install new
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            unsigned nDigitsLeftOld = nDigitsLeft;
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
                        wMin = ls_pcKeys(wRoot)[0];
                        wMax = ls_pcKeys(wRoot)[wPopCnt - 1];
                        wSuffix = wKey & 0xff;
                    } else if (nBitsLeft <= 16) {
                        wMin = ls_psKeys(wRoot)[0];
                        wMax = ls_psKeys(wRoot)[wPopCnt - 1];
                        wSuffix = wKey & 0xffff;
#if (cnBitsPerWord > 32)
                    } else if (nBitsLeft <= 32) {
                        wMin = ls_piKeys(wRoot)[0];
                        wMax = ls_piKeys(wRoot)[wPopCnt - 1];
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
                            if (ls_pcKeys(wRoot)[w] < wMin)
                                wMin = ls_pcKeys(wRoot)[w];
                            if (ls_pcKeys(wRoot)[w] > wMax)
                                wMax = ls_pcKeys(wRoot)[w];
                        } else if (nBitsLeft <= 16) {
                            if (ls_psKeys(wRoot)[w] < wMin)
                                wMin = ls_psKeys(wRoot)[w];
                            if (ls_psKeys(wRoot)[w] > wMax)
                                wMax = ls_psKeys(wRoot)[w];
#if (cnBitsPerWord > 32)
                        } else if (nBitsLeft <= 32) {
                            if (ls_piKeys(wRoot)[w] < wMin)
                                wMin = ls_piKeys(wRoot)[w];
                            if (ls_piKeys(wRoot)[w] > wMax)
                                wMax = ls_piKeys(wRoot)[w];
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
                                LOG(1 | ((wSuffix ^ wMin) | (wSuffix ^ wMax)))
                                + 1);
                    }
                    else
#endif // defined(COMPRESSED_LISTS)
                    {
                        nDigitsLeft
                            = nBL_to_nDL(
                                LOG(1 | ((wKey ^ wMin) | (wKey ^ wMax))) + 1);
                    }
                }
                else
                {
                    // can't dereference list if there isn't one
                    // go directly to bitmap
                    nDigitsLeft = cnDigitsAtBottom + 1;
                }
            }

            // We don't create a switch below cnDigitsAtBottom + 1.
            // Why?  Because we've defined cnDigitsAtBottom as automatic
            // bitmap (no switch) and we may need a prefix at
            // cnDigitsAtBottom + 1 since we don't have one in the bitmap.
            if (nDigitsLeft <= cnDigitsAtBottom)
            {
                DBGI(printf("InsertGuts nDigitsLeft <= cnDigitsAtBottom\n"));

                nDigitsLeft = cnDigitsAtBottom + 1;
            }
#else // defined(SKIP_LINKS)
            assert(nDigitsLeft > cnDigitsAtBottom);
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
        if (wPopCnt != 0) OldList(pwr, wPopCnt, nDigitsLeft);
#endif // (cwListPopCntMax != 0)
    }
#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    else
    {
#if defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
        unsigned nDLR = tp_to_nDigitsLeft(nType);
        Word_t wPrefix = PWR_wPrefix(pwRoot, pwr, nDLR);
        if ((nDLR == nDigitsLeft) || (wPrefix == w_wPrefix(wKey, nDLR)))
#endif // defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL)
        {
#if defined(SKIP_LINKS)
            DBGI(printf("wPrefix "OWx" w_wPrefix "OWx" nDLR %d\n",
                 wPrefix, w_wPrefix(wKey, nDLR), nDLR));
#endif // defined(SKIP_LINKS)
            // no link -- for now -- will eventually have to check
            NewLink(pwRoot, wKey, nDigitsLeft);
            Insert( pwRoot, wKey, nDigitsLeft);
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

            nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

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
            unsigned wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDigitsLeftRoot));
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
// Mind the high-order bits of the bitmap word if/when the bitmap is smaller
// than a whole word.  See OldSwitch.
                if (wIndexCnt < cnBitsPerWord)
                {
                    *pwr_pLinks(pwSw)[nIndex].ln_awBm = (Word_t)-1;
                    //*pwr_pLinks(pwSw)[nIndex].ln_awBm = wIndexCnt - 1;
                }
                else
                {
                    memset(pwr_pLinks(pwSw)[nIndex].ln_awBm, -1,
                           DIV_UP(wIndexCnt, cnBitsPerWord) * cnBytesPerWord);
                }
            }
#endif // defined(BM_IN_LINK)

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
    DBGR(printf("RemoveGuts\n"));

#if (cwListPopCntMax != 0)
    if (nDigitsLeft <= cnDigitsAtBottom)
#else // (cwListPopCntMax != 0)
    assert(nDigitsLeft <= cnDigitsAtBottom);
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

        ClrBit(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
#if defined(PP_IN_LINK)
        if (PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) == 0)
        {
            DBGL(printf("RemoveGuts OldBitmap nDigitsLeft %d\n",
                 nDigitsLeft));
            OldBitmap(wRoot); *pwRoot = 0;
            // Do we need to clear the rest of the link also?
        }
#else // defined(PP_IN_LINK)
        //printf("RemoveGuts not checking for empty bitmap.\n");
        // BUG: We should check if the bitmap is empty and free it if so.
        // Count bits?
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
            wPopCnt = ls_wPopCnt(wRoot);
        }

        if (wPopCnt == 1)
        {
            OldList((Word_t *)wRoot, wPopCnt, nDigitsLeft);
            *pwRoot = 0;
            // Do we need to clear the rest of the link also?
            // BUG:  We should check if the switch is empty and free it
            // (and on up the tree as necessary).
        }
        else
        {
            Word_t *pwKeys = wr_pwKeys(wRoot);

            unsigned nIndex;
            for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
                (nBitsLeft <= 8)
                    ? (wr_pcKeys(wRoot)[nIndex] != (unsigned char )wKey)
                : (nBitsLeft <= 16)
                    ? (wr_psKeys(wRoot)[nIndex] != (unsigned short)wKey)
#if (cnBitsPerWord > 32)
                : (nBitsLeft <= 32)
                    ? (wr_piKeys(wRoot)[nIndex] != (unsigned int)wKey)
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
                COPY(pwList, (Word_t *)wRoot,
                     ListWords(wPopCnt - 1, nDigitsLeft));
            }
            else
            {
                pwList = (Word_t *)wRoot;
            }

#if defined(COMPRESSED_LISTS)
            if (nBitsLeft <= 8) {
                MOVE(&wr_pcKeys(pwList)[nIndex],
                     &wr_pcKeys(wRoot)[nIndex + 1], wPopCnt - nIndex - 1);
            } else if (nBitsLeft <= 16) {
                MOVE(&wr_psKeys(pwList)[nIndex],
                     &wr_psKeys(wRoot)[nIndex + 1], wPopCnt - nIndex - 1);
#if (cnBitsPerWord > 32)
            } else if (nBitsLeft <= 32) {
                MOVE(&wr_piKeys(pwList)[nIndex],
                     &wr_piKeys(wRoot)[nIndex + 1], wPopCnt - nIndex - 1);
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                MOVE(&wr_pwKeys(pwList)[nIndex], &pwKeys[nIndex + 1],
                     wPopCnt - nIndex - 1);
            }

            if (pwList != (Word_t *)wRoot)
            {
                OldList((Word_t *)wRoot, wPopCnt, nDigitsLeft);
                *pwRoot = wRoot = (Word_t)pwList;
                pwKeys = wr_pwKeys(wRoot);
            }

#if defined(MIN_MAX_LISTS) && !defined(SORT_LISTS)
            // if we removed min or max, then we need to find a new one
            if ((nIndex == 0) || (nIndex == wPopCnt - 1))
            {
                for (unsigned nn = 1; nn < wPopCnt - 2; nn++)
                {
#if defined(COMPRESSED_LISTS)
                    if (nBitsLeft <= 8) {
                        unsigned char knn = wr_pcKeys(wRoot)[nn];
                        if (knn < wr_pcKeys(wRoot)[0])
                        {
                            wr_pcKeys(wRoot)[nn] = wr_pcKeys(wRoot)[0];
                            wr_pcKeys(wRoot)[0] = knn;
                        }
                        if (knn > wr_pcKeys(wRoot)[wPopCnt - 2])
                        {
                            wr_pcKeys(wRoot)[nn]
                                = wr_pcKeys(wRoot)[wPopCnt - 2];
                            wr_pcKeys(wRoot)[wPopCnt - 2] = knn;
                        }
                    } else if (nBitsLeft <= 16) {
                        unsigned short knn = wr_psKeys(wRoot)[nn];
                        if (knn < wr_psKeys(wRoot)[0])
                        {
                            wr_psKeys(wRoot)[nn] = wr_psKeys(wRoot)[0];
                            wr_psKeys(wRoot)[0] = knn;
                        }
                        if (knn > wr_psKeys(wRoot)[wPopCnt - 2])
                        {
                            wr_psKeys(wRoot)[nn]
                                = wr_psKeys(wRoot)[wPopCnt - 2];
                            wr_psKeys(wRoot)[wPopCnt - 2] = knn;
                        }
#if (cnBitsPerWord > 32)
                    } else if (nBitsLeft <= 32) {
                        unsigned int knn = wr_piKeys(wRoot)[nn];
                        if (knn < wr_piKeys(wRoot)[0])
                        {
                            wr_piKeys(wRoot)[nn] = wr_piKeys(wRoot)[0];
                            wr_piKeys(wRoot)[0] = knn;
                        }
                        if (knn > wr_piKeys(wRoot)[wPopCnt - 2])
                        {
                            wr_piKeys(wRoot)[nn]
                                = wr_piKeys(wRoot)[wPopCnt - 2];
                            wr_piKeys(wRoot)[wPopCnt - 2] = knn;
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
                set_ls_wPopCnt(wRoot, wPopCnt - 1);
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
    MyFree(*PPArray,
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
        wPopCnt = wr_ls_wPopCnt(wRoot);
    }
    else
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    {
#if defined(PP_IN_LINK)
        // no skip links at root for PP_IN_LINK -- no place for prefix
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        assert(tp_to_nDigitsLeft(nType) == cnDigitsPerWord);
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
// And if cnDigitsAtBottom == cnDigitsPerWord - 1, then it could be a
// pointer to a bitmap?
            Word_t wPopCntLn;
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            unsigned nTypeLn = wr_nType(*pwRootLn);
            if (tp_bIsSwitch(nTypeLn))
            {
                wPopCntLn
                    = PWR_wPopCnt(pwRootLn, NULL,
                                  wr_nDigitsLeft(*pwRootLn));
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
                printf(" mask "OWx" %"_fw"d",
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
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
#if defined(DEBUG_INSERT)
                printf("Pop sum (full)");
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                printf(" mask "Owx" %"_fw"d\n",
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
                printf("nn %d wPopCntLn %"_fw"d "OWx"\n",
                    nn, wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1,
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#endif // defined(DEBUG_INSERT)

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                wPopCnt += wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)) + 1;
#else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                wPopCnt += wPrefixPopMask(cnDigitsPerWord - 1) + 1;
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
            }
        }
        }
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        assert(wPopCnt - 1 <= wPrefixPopMask(tp_to_nDigitsLeft(nType)));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#else // defined(PP_IN_LINK)
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        wPopCnt = PWR_wPopCnt(NULL, pwr, tp_to_nDigitsLeft(nType));
        if (wPopCnt == 0)
        {
            wPopCnt = wPrefixPopMask(tp_to_nDigitsLeft(nType)) + 1;
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
    }
    assert(wPopCnt == wInserts);
#endif // defined(DEBUG)

    return wPopCnt;
}

