
// Todo:
//
// "pwr" is a confusing name.  We use it for pointer extracted from wRoot.
// Do not confuse it with pointer to wRoot.  Even though they will be the
// same when nType is zero.

#include "b.h"

#if defined(RAM_METRICS)
Word_t j__AllocWordsJLLW; // JUDYA  JUDYB
Word_t j__AllocWordsJBU4; //        JUDYB
Word_t j__AllocWordsJV12; //        JUDYB
Word_t j__AllocWordsJL12; //        JUDYB
Word_t j__AllocWordsJL16; //        JUDYB
Word_t j__AllocWordsJL32; //        JUDYB
Word_t j__AllocWordsJBU;  // JUDYA
Word_t j__AllocWordsJLB1; // JUDYA
Word_t j__AllocWordsJLL1; // JUDYA
Word_t j__AllocWordsJLL2; // JUDYA
Word_t j__AllocWordsJLL4; // JUDYA
#endif // defined(RAM_METRICS)

// From Judy1LHTime.c for convenience.

#if 0

Word_t    j__SearchCompares;            // number times LGet/1Test called
Word_t    j__SearchPopulation;          // Population of Searched object
Word_t    j__TreeDepth;                 // number time Branch_U called

#ifdef  JUDYA
Word_t    j__AllocWordsJBB;
Word_t    j__AllocWordsJBU;
Word_t    j__AllocWordsJBL;
Word_t    j__AllocWordsJLB1;
Word_t    j__AllocWordsJLL1;
Word_t    j__AllocWordsJLL2;
Word_t    j__AllocWordsJLL3;

Word_t    j__AllocWordsJLL4;
Word_t    j__AllocWordsJLL5;
Word_t    j__AllocWordsJLL6;
Word_t    j__AllocWordsJLL7;
#endif  // JUDYA  

#ifdef  JUDYB
Word_t    j__AllocWordsJBU4;
Word_t    j__AllocWordsJBU8;
Word_t    j__AllocWordsJBU16;
Word_t    j__AllocWordsJV12;
Word_t    j__AllocWordsJL12;
Word_t    j__AllocWordsJL16;
Word_t    j__AllocWordsJL32;
#endif  // JUDYB   

Word_t    j__AllocWordsJLLW;

#ifdef  JUDYA
Word_t    j__AllocWordsJV;
#endif  // JUDYA 

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

#if cnBitsPerDigit != 0

static Word_t *
NewList(Word_t wPopCnt, unsigned nDigitsLeft, Word_t wKey)
{
    DBGM(printf("NewList wPopCnt "OWx"\n", wPopCnt));

#if defined(COMPRESSED_LISTS)

    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;

    if (nBitsLeft > cnBitsPerWord)
    {
        nBitsLeft = cnBitsPerWord;
    }

    unsigned nBytesKeySz = (nBitsLeft <= 8) ? 1 : (nBitsLeft <= 16) ? 2
#if (cnBitsPerWord > 32)
        : (nBitsLeft <= 32) ? 4
#endif // (cnBitsPerWord > 32)
        : sizeof(Word_t);

    unsigned nWords = (DIV_UP(wPopCnt * nBytesKeySz, cnBytesPerWord)
        + OFFSET_OF(ListLeaf_t, ll_awKeys) / sizeof(Word_t)) | 1;

    if (nBytesKeySz == 1) {
        METRICS(j__AllocWordsJL12 += nWords); // JUDYB
        METRICS(j__AllocWordsJLL1 += nWords); // JUDYA
    } else if (nBytesKeySz == 2) {
        METRICS(j__AllocWordsJL16 += nWords); // JUDYB
        METRICS(j__AllocWordsJLL2 += nWords); // JUDYA
#if (cnBitsPerWord > 32)
    } else if (nBytesKeySz == 4) {
        METRICS(j__AllocWordsJL32 += nWords); // JUDYB
        METRICS(j__AllocWordsJLL4 += nWords); // JUDYA
#endif // (cnBitsPerWord > 32)
    } else {
        METRICS(j__AllocWordsJLLW += nWords); // BOTH
    }

#else // defined(COMPRESSED_LISTS)

    unsigned nWords = (wPopCnt
        + OFFSET_OF(ListLeaf_t, ll_awKeys) / sizeof(Word_t)) | 1;

    METRICS(j__AllocWordsJLLW += nWords);

#endif // defined(COMPRESSED_LISTS)

    DBGM(printf("NewList nWords %d\n", nWords));

    Word_t *pwList = (Word_t *)JudyMalloc(nWords);
    assert(pwList != NULL);
    assert(((Word_t)pwList & cnMallocMask) == 0);

    (void)nDigitsLeft;
    (void)wKey;

    DBGM(printf("NewList pwList %p wPopCnt "OWx" nWords %d\n",
        pwList, wPopCnt, nWords));

    set_ls_wPopCnt(pwList, wPopCnt);
    set_ls_wLen(pwList, nWords);

    set_ll_nDigitsLeft(pwList, nDigitsLeft);

// Should we be setting wPrefix here for PP_IN_LINK?
// What about wPopCnt?

    return pwList;
}

static Word_t
OldList(Word_t *pwList)
{
    unsigned nWords = ls_wLen(pwList);

    DBGM(printf("Old pwList %p wLen %d wPopCnt "OWx"\n",
        pwList, nWords, (Word_t)ls_wPopCnt(pwList)));

#if defined(COMPRESSED_LISTS)

    unsigned nDigitsLeft = ll_nDigitsLeft(pwList);

    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;

    if (nBitsLeft > cnBitsPerWord)
    {
        nBitsLeft = cnBitsPerWord;
    }

    unsigned nBytesKeySz = (nBitsLeft <= 8) ? 1 : (nBitsLeft <= 16) ? 2
#if (cnBitsPerWord > 32)
        : (nBitsLeft <= 32) ? 4 : 8;
#else // (cnBitsPerWord > 32)
        : 4;
#endif // (cnBitsPerWord > 32)

    if (nBytesKeySz == 1) {
        METRICS(j__AllocWordsJL12 -= nWords); // JUDYB
        METRICS(j__AllocWordsJLL1 -= nWords); // JUDYA
    } else if (nBytesKeySz == 2) {
        METRICS(j__AllocWordsJL16 -= nWords); // JUDYB
        METRICS(j__AllocWordsJLL2 -= nWords); // JUDYA
#if (cnBitsPerWord > 32)
    } else if (nBytesKeySz == 4) {
        METRICS(j__AllocWordsJL32 -= nWords); // JUDYB
        METRICS(j__AllocWordsJLL4 -= nWords); // JUDYA
#endif // (cnBitsPerWord > 32)
    } else {
        METRICS(j__AllocWordsJLLW -= nWords); // BOTH
    }

#else // defined(COMPRESSED_LISTS)

    METRICS(j__AllocWordsJLLW -= nWords);

#endif // defined(COMPRESSED_LISTS)

    JudyFree(pwList, nWords);

    return nWords * sizeof(Word_t);
}

static Word_t
NewBitmap(void)
{
    Word_t w = JudyMalloc(EXP(cnBitsAtBottom) / cnBitsPerWord);
    assert(w != 0);
    assert((w & cnMallocMask) == 0);

    METRICS(j__AllocWordsJLB1 += (EXP(cnBitsAtBottom) / cnBitsPerWord));

    DBGM(printf("NewBitmap nBitsAtBottom %u nBits "OWx
      " nBytes "OWx" nWords "OWx" w "OWx"\n",
        cnBitsAtBottom, EXP(cnBitsAtBottom),
        EXP(cnBitsAtBottom) / cnBitsPerByte,
        EXP(cnBitsAtBottom) / cnBitsPerWord, w));

    if (w == 0) { fprintf(stderr, "NewBitmap malloc.\n"); exit(1); }

    memset((void *)w, 0, EXP(cnBitsAtBottom) / cnBitsPerByte);

    return w;
}

Word_t
OldBitmap(Word_t wRoot)
{
    JudyFree((Word_t *)wRoot, EXP(cnBitsAtBottom) / cnBitsPerWord);

    METRICS(j__AllocWordsJLB1 -= (EXP(cnBitsAtBottom) / cnBitsPerWord));

    return EXP(cnBitsAtBottom) / cnBitsPerWord * sizeof(Word_t);
}

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

    // wKey and nDigitsLeft are provided in case we decide to initialize
    // prefix here.
    // But we don't have enough info to implement NO_UNNECESSARY_PREFIX here.
    (void)wKey; // fix "unused parameter" compiler warning
    (void)nDigitsLeft; // nDigitsLeft is not used for all ifdef combos
    (void)nDigitsLeftUp; // nDigitsLeftUp is not used for all ifdef combos

    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);

    pwr = (Word_t *)JudyMalloc(sizeof(Switch_t) / sizeof(Word_t));
    assert(pwr != NULL);
    assert(((Word_t)pwr & cnMallocMask) == 0);

#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(Switch_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 += sizeof(Switch_t) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  += sizeof(Switch_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 += sizeof(Switch_t) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx" nDigitsLeft %d) pwr %p\n",
        pwRoot, wKey, nDigitsLeft, pwr));

    set_wr(*pwRoot, pwr, nDigitsLeft_to_tp(nDigitsLeft));

    memset(pwr_pLinks(pwr), 0, sizeof(pwr_pLinks(pwr)));

#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
    if (nDigitsLeftUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
#if defined(BM_SWITCH_FOR_REAL)
        unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
        if (nBitsLeft > cnBitsPerWord) nBitsLeft = cnBitsPerWord;
        unsigned nBitsIndexSz = pwr_nBitsIndexSz(pwr);
        Word_t wIndex
            = (wKey >> (nBitsLeft - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1);
        SetBit(PWR_pwBm(pwRoot, pwr), wIndex);
#else // defined(BM_SWITCH_FOR_REAL)
#if 0
        printf("nBytes %"_fw"d\n",
            DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord) * cnBytesPerWord);
#endif
        memset(PWR_pwBm(pwRoot, pwr), -1,
               DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord)
                   * cnBytesPerWord);
      //printf("PWR_pwBm(pwRoot, pwr)[0] "OWx"\n", PWR_pwBm(pwRoot, pwr)[0]);
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
        // We need the prefix at the leaf even if there is no skip if
        // defined(SKIP_PREFIX_CHECK).
        // List leaf only needs it if the keys in the list are less than
        // whole words.
        // Bits in a bitmap are always less than whole words and always
        // need the prefix.
        // Does it mean we'd have to add the prefix when transitioning
        // from list to bitmap?
        // We could get rid of the leaf check if we enhance Insert
        // to keep track of any prefix checks done along the way and
        // pass that info to InsertGuts.
        if ((nDigitsLeft == nDigitsLeftUp)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            && (nDigitsLeft * cnBitsPerDigit > 32)
#else // (cnBitsPerWord > 32)
            && (nDigitsLeft * cnBitsPerDigit > 16)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            && (nDigitsLeft > cnDigitsAtBottom + 1))
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
#endif // defined(SKIP_LINKS)

        set_PWR_wPopCnt(pwRoot, pwr, nDigitsLeft, wPopCnt);

        DBGM(printf("NewSwitch PWR_wPrefixPop "OWx"\n",
            PWR_wPrefixPop(pwRoot, pwr)));
    }

    return pwr;
}

#if defined(BM_SWITCH_FOR_REAL)
static void
NewLink(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft)
{
    Word_t *pwr = wr_pwr(*pwRoot);

#if defined(BM_IN_LINK)
    assert(nDigitsLeft != cnDigitsPerWord);
#endif // defined(BM_IN_LINK)

    // How many links are there in the old switch?
    Word_t wPopCnt = 0;
    for (unsigned nn = 0; nn < EXP(cnBitsPerDigit) >> cnLogBitsPerWord; nn++)
    {
        wPopCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
    // Now we know how many links were in the old switch.

    // Allocate memory for a new switch with one more link than the old one.
    // Update *pwRoot.

    // sizeof(Switch_t) includes one link; add the others
    unsigned nWords
        = (sizeof(Switch_t) + wPopCnt * sizeof(Link_t)) / sizeof(Word_t);
    *pwRoot = JudyMalloc(nWords);

#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 += sizeof(Link_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 += sizeof(Link_t) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  += sizeof(Link_t) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 += sizeof(Link_t) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    // Where does the new link go?
    unsigned nBitsIndexSz = pwr_nBitsIndexSz(pwr);
    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit - nBitsIndexSz;
    Word_t wIndex = ((wKey >> nBitsLeft) & (EXP(nBitsIndexSz) - 1));
    unsigned nBmOffset = wIndex >> cnLogBitsPerWord;
    Word_t wBm = PWR_pwBm(pwRoot, pwr)[nBmOffset];
    Word_t wBit = ((Word_t)1 << (wIndex & (cnBitsPerWord - 1)));
    assert( ! (wBm & wBit) );
    Word_t wBmMask = wBit - 1;
    // recalculate index as link number in sparse array of links
    wIndex = 0;
#if (cnBitsPerDigit > cnLogBitsPerWord)
    for (unsigned nn = 0; nn < nBmOffset; nn++)
    {
        wIndex += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
#endif // (cnBitsPerDigit > cnLogBitsPerWord)
    wIndex += __builtin_popcountll(wBm & wBmMask);
    // Now we know where the new link goes.

    // Copy the old switch to the new switch and insert the new link.
    memcpy(wr_pwr(*pwRoot), pwr,
        sizeof(Switch_t) + (wIndex - 1) * sizeof(Link_t));
    // Initialize the new link.
    memset(&pwr_pLinks(*pwRoot)[wIndex], 0, sizeof(Link_t));
    memcpy(&pwr_pLinks(*pwRoot)[wIndex + 1], &pwr_pLinks(pwr)[wIndex],
        (wPopCnt - wIndex) * sizeof(Link_t));

    // Remember to finish updating *pwRoot.
    set_wr_nType(*pwRoot, nDigitsLeft_to_tp(nDigitsLeft));
}
#endif // defined(BM_SWITCH_FOR_REAL)

static Word_t
#if defined(RAM_METRICS)
OldSwitch(Switch_t *pSw, unsigned nDigitsLeft)
#else // defined(RAM_METRICS)
OldSwitch(Switch_t *pSw)
#endif // defined(RAM_METRICS)
{
#if defined(RAM_METRICS)
    if (((cnBitsPerDigit * cnDigitsAtBottom) <= cnLogBitsPerWord)
        && (nDigitsLeft <= cnDigitsAtBottom + 1))
    {
        assert(nDigitsLeft == cnDigitsAtBottom + 1); // later
        METRICS(j__AllocWordsJLB1 -= sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJV12 -= sizeof(*pSw) / sizeof(Word_t));
    }
    else
    {
        METRICS(j__AllocWordsJBU  -= sizeof(*pSw) / sizeof(Word_t));
        METRICS(j__AllocWordsJBU4 -= sizeof(*pSw) / sizeof(Word_t));
    }
#endif // defined(RAM_METRICS)

    JudyFree((Word_t *)pSw, sizeof(*pSw) / sizeof(Word_t));

    return sizeof(*pSw);
}

Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, unsigned nBitsLeft, int bDump)
{
#if defined(BM_IN_LINK) || defined(PP_IN_LINK)
    unsigned nBitsLeftArg = nBitsLeft;
#endif // defined(BM_IN_LINK) || defined(PP_IN_LINK)
    Word_t wRoot = *pwRoot;
    unsigned nDigitsLeft = DIV_UP(nBitsLeft, cnBitsPerDigit);
    Word_t *pwr;
    unsigned nBitsIndexSz;
    Link_t *pLinks;
    unsigned nType;
    Word_t wBytes = 0;

    if (wRoot == 0)
    {
        return 0;
    }

    if (bDump)
    {
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefix "OWx, wPrefix);
        printf(" nBitsLeft %2d", nBitsLeft);
        printf(" pwRoot "OWx, (Word_t)pwRoot);
        //printf(" wr "OWx, wRoot);
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
        if (cnBitsAtBottom > cnLogBitsPerWord)
        {
            if (!bDump)
            {
                return OldBitmap(wRoot);
            }

            printf("nWords %"_fw"d\n", EXP(cnBitsAtBottom) / cnBitsPerWord);
            for (unsigned nn = 0;
                //(nn < EXP(cnBitsAtBottom) / cnBitsPerWord) && (nn < 8);
                (nn < EXP(cnBitsAtBottom) / cnBitsPerWord);
                 nn++)
            {
                printf(" "Owx, ((Word_t *)wRoot)[nn]);
            }
        }
        else
        {
            if (bDump) printf(" wr "OWx, wRoot);
        }

        if (bDump)
        {
            printf("\n");
        }

        return 0;
    }

    nType = wr_nType(wRoot);

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    assert((tp_to_nDigitsLeft(nType) * cnBitsPerDigit)
        <= ALIGN_UP(nBitsLeft, cnBitsPerDigit));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    pwr = wr_tp_pwr(wRoot, nType);

#if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt = ls_wPopCnt(pwr);
        Word_t *pwKeys = pwr_pwKeys(pwr);

        assert(nType == 0);

        if (!bDump)
        {
            return OldList(pwr);
        }
#if defined(PP_IN_LINK)
        if (nBitsLeftArg == cnBitsPerWord)
        {
            printf(" ls_wPopCnt %3"_fw"u", wPopCnt);
            printf(" wr_wPrefix        N/A");
        }
        else
        {
            printf(" wr_wPopCnt %3"_fw"u",
                   PWR_wPopCnt(pwRoot, NULL, nDigitsLeft));
            printf(" wr_wPrefix "OWx, PWR_wPrefix(pwRoot, NULL, nDigitsLeft));
        }
#endif // defined(PP_IN_LINK)

        printf(" ls_wLen %3llu", (unsigned long long)ls_wLen(wRoot));
        printf(" ls_wPopCnt %3llu", (unsigned long long)wPopCnt);

        for (unsigned nn = 0;
            //(nn < wPopCnt) && (nn < 8);
            (nn < wPopCnt);
             nn++)
        {
#if defined(COMPRESSED_LISTS)
            if (nBitsLeft <= 8) {
                printf(" %02x", ls_pcKeys(wRoot)[nn]);
            } else if (nBitsLeft <= 16) {
                printf(" %04x", ls_psKeys(wRoot)[nn]);
#if (cnBitsPerWord > 32)
            } else if (nBitsLeft <= 32) {
                printf(" %08x", ls_piKeys(wRoot)[nn]);
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            { printf(" "Owx, pwKeys[nn]); }
        }
        printf("\n");

        return 0;
    }
#endif // (cwListPopCntMax != 0)

    // Switch

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    nDigitsLeft = tp_to_nDigitsLeft(nType);
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    if ((nBitsLeft = nDigitsLeft * cnBitsPerDigit) > cnBitsPerWord)
    {
        nBitsLeft = cnBitsPerWord;
    }

    wPrefix = PWR_wPrefix(pwRoot, pwr, nDigitsLeft);
    nBitsIndexSz = pwr_nBitsIndexSz(pwr);
    pLinks = pwr_pLinks(pwr);

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBitsLeftArg == cnBitsPerWord)
        {
// Add 'em up.
            Word_t wPopCnt = 0;
            for (unsigned nn = 0; nn < EXP(cnBitsPerDigit); nn++)
            {
                Word_t *pwRootLn = &pwr_pLinks(pwr)[nn].ln_wRoot;
// *pwRootLn may not be a pointer to a switch
// It may be a pointer to a list leaf.
// And if cnDigitsAtBottom == cnDigitsPerWord - 1, then it could be a
// pointer to a bitmap?
                Word_t wPopCntLn;
#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
                unsigned nTypeLn = wr_nType(*pwRootLn);
                if (tp_bIsSwitch(nTypeLn))
                {
                    wPopCntLn
                        = PWR_wPopCnt(pwRootLn, NULL,
                                      wr_nDigitsLeft(*pwRootLn));
                }
                else
#endif // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
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

            printf(" wr_wPopCnt %3"_fw"u", wPopCnt);
            printf(" wr_wPrefix        N/A");
        }
        else
#endif // defined(PP_IN_LINK)
        {
            printf(" wr_wPopCnt %3"_fw"u",
                PWR_wPopCnt(pwRoot, pwr, nDigitsLeft));
            printf(" wr_wPrefix "OWx, wPrefix);
        }

        printf(" wr_nDigitsLeft %2d", nDigitsLeft);
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
            for (unsigned nn = 0;
                          nn < DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord);
                          nn ++)
            {
                printf(" "OWx, PWR_pwBm(pwRoot, pwr)[nn]);
            }
        }
#endif // defined(BM_SWITCH)
        printf("\n");
    }

    nBitsLeft = ALIGN_UP(nBitsLeft - nBitsIndexSz, cnBitsPerDigit);

#if defined(BM_SWITCH)
    Word_t xx = 0;
#endif // defined(BM_SWITCH)
    for (Word_t nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(BM_SWITCH)
#if defined(BM_IN_LINK)
        if ((nBitsLeftArg == cnBitsPerWord)
            || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#else // defined(BM_IN_LINK)
        if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(BM_IN_LINK)
        {
            //printf("nn %"_fw"d\n", nn);
            wBytes += FreeArrayGuts(&pLinks[xx].ln_wRoot,
                    wPrefix | (nn << nBitsLeft), nBitsLeft, bDump);
            xx++;
        }
        else
        {
            //printf("nn %"_fw"d no bit\n", nn);
        }
#else // defined(BM_SWITCH)
        pwRoot = &pLinks[nn].ln_wRoot;
#endif // defined(BM_SWITCH)
    }

#if defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr, nDigitsLeft) + wBytes);
#else // defined(RAM_METRICS)
    return bDump ? 0 : (OldSwitch((Switch_t *)pwr) + wBytes);
#endif // defined(RAM_METRICS)
}

#if defined(DEBUG)
void
Dump(Word_t wRoot, Word_t wPrefix, unsigned nBitsLeft)
{
    FreeArrayGuts(&wRoot, wPrefix, nBitsLeft, /* bDump */ 1);
}
#endif // defined(DEBUG)

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
// When do we create a new switch for a list being too long?
// When do we create a bitmap?
// When do we uncompress switches?
// When do we coalesce switches?
Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    Word_t *pwr;
    Word_t *pwSw;
    unsigned nType;

    // Validate global constant parameters set up in the header file.
#if 0
    assert(cnDigitsAtBottom > 0); // can't get to full pop
#endif
    assert(cnDigitsAtBottom + 1 <= cnDigitsPerWord);
#if defined(SKIP_LINKS)
    assert(cnDigitsAtBottom + cnMallocMask >= cnDigitsPerWord + 1);
#endif // defined(SKIP_LINKS)

    DBGI(printf("InsertGuts pwRoot %p ", pwRoot));
    DBGI(printf(" wRoot "OWx" wKey "OWx" nDigitsLeft %d\n",
            wRoot, wKey, nDigitsLeft));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        if (cnBitsAtBottom <= cnLogBitsPerWord) // compile time
        {
            assert(!BitIsSetInWord(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSetInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSet(&wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(!BitIsSet(pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            DBGI(printf("SetBitInWord(*pwRoot "OWx" wKey "OWx")\n",
                *pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            SetBitInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1));

            assert(BitIsSet(pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
            assert(BitIsSetInWord(*pwRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
        }
        else
        {
            if (wRoot == 0)
            {
                *pwRoot = wRoot = NewBitmap();
            }

            assert(!BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));

            DBGI(printf("SetBit(wRoot "OWx" wKey "OWx") pwRoot %p\n",
                wRoot, wKey & (EXP(cnBitsAtBottom) - 1), pwRoot));

            SetBit(wRoot, wKey & (EXP(cnBitsAtBottom) - 1));

            assert(BitIsSet(wRoot, wKey & (EXP(cnBitsAtBottom) - 1)));
        }

#if defined(PP_IN_LINK)
        // What about no_unnecessary_prefix?
        set_PWR_wPrefix(pwRoot, NULL, nDigitsLeft, wKey);
#endif // defined(PP_IN_LINK)

        return Success;
    }

    nType = wr_nType(wRoot);

    pwr = wr_tp_pwr(wRoot, nType);

#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    if (!tp_bIsSwitch(nType))
#else // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    assert(nType == 0);
#endif // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
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

        if (pwr != NULL) // pointer to old List
        {
            wPopCnt = ls_wPopCnt(pwr);
            pwKeys = ls_pwKeys(pwr); // list of keys in old List
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            piKeys = ls_piKeys(pwr);
#endif // (cnBitsPerWord > 32)
            psKeys = ls_psKeys(pwr);
            pcKeys = ls_pcKeys(pwr);
#endif // defined(COMPRESSED_LISTS)
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
            // What about no_unnecessary_prefix?
            set_PWR_wPrefix(pwRoot, NULL, nDigitsLeft, wKey);
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

        if (wPopCnt < cwListPopCntMax)
        {
            // allocate a new list and init pop count in the first word
            Word_t *pwList = NewList(wPopCnt + 1, nDigitsLeft, wKey);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
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
#elif defined(MIN_MAX_LISTS)
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
            } else
#else // defined(MIN_MAX_LISTS)
            { COPY(ls_pwKeys(pwList), pwKeys, wPopCnt); }
#endif // defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
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
        {
            unsigned nDigitsLeftOld = nDigitsLeft;
            Word_t w;

            // List is full; insert a switch

#if defined(PP_IN_LINK)
            if (nDigitsLeft < cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
#if defined(SKIP_LINKS)
            {
                if (cwListPopCntMax != 0) // use const for compile time check
                {
                    Word_t wMax, wMin;
#if defined(SORT_LISTS) || defined(MIN_MAX_LISTS)
#if defined(COMPRESSED_LISTS)
                    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;
                    Word_t wSuffix;
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

                    for (w = 0; w < wPopCnt; w++)
                    {
                        if (pwKeys[w] < wMin) wMin = pwKeys[w];
                        if (pwKeys[w] > wMax) wMax = pwKeys[w];
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
                            = LOG(1 | ((wSuffix ^ wMin) | (wSuffix ^ wMax)))
                                / cnBitsPerDigit + 1;
                    }
                    else
#endif // defined(COMPRESSED_LISTS)
                    {
                        nDigitsLeft
                            = LOG(1 | ((wKey ^ wMin) | (wKey ^ wMax)))
                                / cnBitsPerDigit + 1;
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
            pwSw = NewSwitch(pwRoot, wKey, nDigitsLeft, nDigitsLeftOld,
                             /* wPopCnt */ 0);

#if defined(COMPRESSED_LISTS)
#if defined(SKIP_LINKS)
            unsigned nBitsLeftOld = nDigitsLeftOld * cnBitsPerDigit;
#else // defined(SKIP_LINKS)
// Revisit the use of "Old" here.
            unsigned nBitsLeftOld = nDigitsLeft * cnBitsPerDigit;
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

            Insert(pwRoot, wKey, nDigitsLeftOld);
        }

        if (wPopCnt != 0) OldList(pwr); // free old
    }
#if defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)
    else
    {
//#if defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
//#endif // defined(SKIP_LINKS) && defined(BM_SWITCH_FOR_REAL)
#if defined(BM_SWITCH_FOR_REAL)
        // no link
        NewLink(pwRoot, wKey, nDigitsLeft);
        Insert(pwRoot, wKey, nDigitsLeft);
#endif // defined(BM_SWITCH_FOR_REAL)
#if defined(SKIP_LINKS)
        // prefix mismatch
        // insert a switch so we can add just one key; seems like a waste
// A bitmap switch would be great; no reason to consider converting the
// existing bitmap to a list if a bitmap switch is short.
        unsigned nDigitsLeftRoot;
        Word_t wPopCnt;

        nDigitsLeftRoot = tp_to_nDigitsLeft(nType);

        assert(nDigitsLeftRoot < nDigitsLeft);

        unsigned nDigitsLeftUp = nDigitsLeft;

        // figure new nDigitsLeft for old parent link
        Word_t wPrefix = PWR_wPrefix(pwRoot, pwr, nDigitsLeftRoot);
        nDigitsLeft = LOG(1 | (wPrefix ^ wKey)) / cnBitsPerDigit + 1;
        // nDigitsLeft includes the digit that is different.

        assert(nDigitsLeft > nDigitsLeftRoot);

        if ((wPopCnt = PWR_wPopCnt(pwRoot, pwr, nDigitsLeftRoot)) == 0)
        {
            // full pop overflow
            wPopCnt = wPrefixPopMask(nDigitsLeftRoot) + 1;
        }

// Have to get old prefix before inserting the new switch because
// NewSwitch copies to *pwRoot.
// But also have to deal with switch at top with no link if PP_IN_LINK.

        unsigned nIndex;

#if defined(PP_IN_LINK)
        // PP_IN_LINK ==> no skip link at top ==> no prefix mismatch at top
        assert(nDigitsLeftUp < cnDigitsPerWord);
#endif // defined(PP_IN_LINK)

        // todo nBitsIndexSz; wide switch
        assert(pwr_nBitsIndexSz(pwr) == cnBitsPerDigit);
        nIndex = (wPrefix >> ((nDigitsLeft - 1) * cnBitsPerDigit))
            & (EXP(cnBitsPerDigit) - 1);
        // nIndex is the index in new switch.
        // It may not be the same as the index in the old switch.

#if defined(BM_IN_LINK)
        // Save the old bitmap before it is trashed by NewSwitch.
        // is it possible that nDigitsLeftUp != cnDigitsPerWord and
        // we are at the top?
        Link_t ln;
        if (nDigitsLeftUp != cnDigitsPerWord)
        {
            memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL),
                DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord) * cnBytesPerWord);
            assert(ln.ln_awBm[0] == (Word_t)-1);
        }
#endif // defined(BM_IN_LINK)

        // initialize prefix/pop for new switch
        pwSw = NewSwitch(pwRoot, wKey, nDigitsLeft, nDigitsLeftUp, wPopCnt);

#if defined(BM_IN_LINK)
        if (nDigitsLeftUp != cnDigitsPerWord)
        {
            // Copy bitmap from old link to new link.
            memcpy(pwr_pLinks(pwSw)[nIndex].ln_awBm, ln.ln_awBm,
                DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord) * cnBytesPerWord);
        }
        else
        {
            // Initialize bitmap in new link.
            memset(pwr_pLinks(pwSw)[nIndex].ln_awBm, (Word_t)-1,
                DIV_UP(EXP(cnBitsPerDigit), cnBitsPerWord) * cnBytesPerWord);
        }
#endif // defined(BM_IN_LINK)

        // Copy wRoot from old link to new link.
        pwr_pLinks(pwSw)[nIndex].ln_wRoot = wRoot;

#if defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
        if (nDigitsLeftRoot != nDigitsLeft - pwr_nBitsIndexSz(pwSw))
#endif // defined(NO_UNNECESSARY_PREFIX)
        {
            set_PWR_wPrefix(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                            nDigitsLeftRoot, wPrefix);
        }

        set_PWR_wPopCnt(&pwr_pLinks(pwSw)[nIndex].ln_wRoot, NULL,
                        nDigitsLeftRoot, wPopCnt);
#endif // defined(PP_IN_LINK)

        Insert(pwRoot, wKey, nDigitsLeftUp);
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SWITCH_FOR_REAL)

    return Success;
}

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nDigitsLeft, Word_t wRoot)
{
    DBGR(printf("RemoveGuts\n"));

    if (nDigitsLeft <= cnDigitsAtBottom)
    {
        if (cnBitsAtBottom <= cnLogBitsPerWord)
        {
            // What if link has more space than just *pwRoot due
            // to BM_IN_LINK and/or PP_IN_LINK?
            ClrBitInWord(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
            *pwRoot = wRoot;
        }
        else
        {
            ClrBit(wRoot, wKey & ((EXP(cnBitsAtBottom)) - 1UL));
#if defined(PP_IN_LINK)
            if (PWR_wPopCnt(pwRoot, NULL, nDigitsLeft) == 0)
            {
                DBGL(printf("RemoveGuts OldBitmap nDigitsLeft %d\n",
                     nDigitsLeft));
                OldBitmap(wRoot); *pwRoot = 0;
            }
#else // defined(PP_IN_LINK)
            //printf("RemoveGuts not checking for empty bitmap.\n");
            // BUG: We should check if the bitmap is empty and free it if so.
            // Count bits?
#endif // defined(PP_IN_LINK)
        }

        if (*pwRoot == 0)
        {
            // We return to Remove which will clean up ancestors.
            DBGR(printf("RemoveGuts *pwRoot is now 0\n"));
        }
    }
    else
    {

#if defined(COMPRESSED_LISTS)
    unsigned nBitsLeft = nDigitsLeft * cnBitsPerDigit;

    if (nBitsLeft > cnBitsPerWord)
    {
        nBitsLeft = cnBitsPerWord;
    }
#endif // defined(COMPRESSED_LISTS)

    Word_t wPopCnt = ls_wPopCnt(wRoot);
    unsigned n;

    if (wPopCnt == 1)
    {
        OldList((Word_t *)wRoot); *pwRoot = 0;
        // BUG:  We should check if the switch is empty and free it
        // (and on up the tree as necessary).
    }
    else
    {
        Word_t *pwKeys = wr_pwKeys(wRoot);

        for (n = 0;
#if defined(COMPRESSED_LISTS)
            (nBitsLeft <= 8)
                ? (wr_pcKeys(wRoot)[n] != (unsigned char )wKey)
            : (nBitsLeft <= 16)
                ? (wr_psKeys(wRoot)[n] != (unsigned short)wKey)
#if (cnBitsPerWord > 32)
            : (nBitsLeft <= 32)
                ? (wr_piKeys(wRoot)[n] != (unsigned int)wKey)
#endif // (cnBitsPerWord > 32)
                : (pwKeys[n] != wKey);
#else // defined(COMPRESSED_LISTS)
            pwKeys[n] != wKey;
#endif // defined(COMPRESSED_LISTS)
            n++)
            ; // semicolon on separate line to silence compiler warning

#if defined(MAX_MIN_LISTS)
        assert(0); // later
#else // defined(MAX_MIN_LISTS)
        // BUG:  We should shrink the list.
#if defined(COMPRESSED_LISTS)
        if (nBitsLeft <= 8) {
            MOVE(&wr_pcKeys(wRoot)[n],
                 &wr_pcKeys(wRoot)[n + 1], wPopCnt - n - 1);
        } else if (nBitsLeft <= 16) {
            MOVE(&wr_psKeys(wRoot)[n],
                 &wr_psKeys(wRoot)[n + 1], wPopCnt - n - 1);
#if (cnBitsPerWord > 32)
        } else if (nBitsLeft <= 32) {
            MOVE(&wr_piKeys(wRoot)[n],
                 &wr_piKeys(wRoot)[n + 1], wPopCnt - n - 1);
#endif // (cnBitsPerWord > 32)
        } else
#endif // defined(COMPRESSED_LISTS)
        { MOVE(&pwKeys[n], &pwKeys[n + 1], wPopCnt - n - 1); }
#endif // defined(MAX_MIN_LISTS)

        set_ls_wPopCnt(wRoot, wPopCnt - 1);
    }

    }

    (void)pwRoot; (void)wKey; (void)nDigitsLeft; (void)wRoot;

    return Success;
}

#endif // cnBitsPerDigit != 0

// ****************************************************************************
// JUDY1 FUNCTIONS:

Word_t
Judy1FreeArray(PPvoid_t PPArray, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("Judy1FreeArray\n"));

#if (cnBitsPerDigit != 0)
    return FreeArrayGuts((Word_t *)PPArray,
        /* wPrefix */ 0, cnBitsPerWord, /* bDump */ 0);
#else // (cnBitsPerDigit != 0)
    JudyFree(*PPArray,
       EXP(cnBitsPerWord - cnLogBitsPerByte - cnLogBytesPerWord));
    return EXP(cnBitsPerWord - cnLogBitsPerByte);
#endif // (cnBitsPerDigit != 0)
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
    if (tp_bIsSwitch(nType))
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
        // BUG: Remember to check the switch bitmap.
        // BUG: nBitsPerIndex > cnBitsPerDigit.
        wPopCnt = 0;
        for (unsigned nn = 0; nn < EXP(cnBitsPerDigit); nn++)
        {
            Word_t *pwRootLn = &pwr_pLinks(pwr)[nn].ln_wRoot;
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
                printf(" mask "OWx" %zd",
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                printf(" nn %d wPopCntLn %zd "OWx"\n",
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
                printf(" mask "Owx" %zd\n",
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)),
                    wPrefixPopMask(wr_nDigitsLeft(*pwRootLn)));
                printf("nn %d wPopCntLn %zd "OWx"\n",
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

