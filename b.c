
// @(#) $Id: b.c,v 1.498 2015/02/07 21:01:58 mike Exp $
// @(#) $Source: /Users/mike/b/RCS/b.c,v $

#include "b.h"

#define nBytesKeySz(_nBL) \
     (((_nBL) <=  8) ? 1 : ((_nBL) <= 16) ? 2 \
    : ((_nBL) <= 32) ? 4 : sizeof(Word_t))

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

// Unfortunately, it is not sufficient to change these two tables to
// effect arbitrary switch sizes.
// It is necessary to tweak cnDigitsPerWord in b.h also.

#if defined(BPD_TABLE_RUNTIME_INIT)

unsigned anDL_to_nBitsIndexSz[ cnBitsPerWord + 1 ];
unsigned anDL_to_nBL[ cnBitsPerWord + 1 ];
unsigned anBL_to_nDL[ cnBitsPerWord * 2 ];

#else // defined(BPD_TABLE_RUNTIME_INIT)

const unsigned anDL_to_nBitsIndexSz[] = {
#define V(_nDL) (nBitsIndexSz_from_nDL(_nDL))
    V(  0),V(  1),V(  2),V(  3),V(  4),V(  5),V(  6),V(  7),V(  8),V(  9),
    V( 10),V( 11),V( 12),V( 13),V( 14),V( 15),V( 16),V( 17),V( 18),V( 19),
    V( 20),V( 21),V( 22),V( 23),V( 24),V( 25),V( 26),V( 27),V( 28),V( 29),
    V( 30),V( 31),V( 32),V( 33),V( 34),V( 35),V( 36),V( 37),V( 38),V( 39),
    V( 40),V( 41),V( 42),V( 43),V( 44),V( 45),V( 46),V( 47),V( 48),V( 49),
    V( 50),V( 51),V( 52),V( 53),V( 54),V( 55),V( 56),V( 57),V( 58),V( 59),
    V( 60),V( 61),V( 62),V( 63),V( 64)
#undef V
};

const unsigned anDL_to_nBL[] = {
#define V(_nDL) (nBL_from_nDL(_nDL))
    V(  0),V(  1),V(  2),V(  3),V(  4),V(  5),V(  6),V(  7),V(  8),V(  9),
    V( 10),V( 11),V( 12),V( 13),V( 14),V( 15),V( 16),V( 17),V( 18),V( 19),
    V( 20),V( 21),V( 22),V( 23),V( 24),V( 25),V( 26),V( 27),V( 28),V( 29),
    V( 30),V( 31),V( 32),V( 33),V( 34),V( 35),V( 36),V( 37),V( 38),V( 39),
    V( 40),V( 41),V( 42),V( 43),V( 44),V( 45),V( 46),V( 47),V( 48),V( 49),
    V( 50),V( 51),V( 52),V( 53),V( 54),V( 55),V( 56),V( 57),V( 58),V( 59),
    V( 60),V( 61),V( 62),V( 63),V( 64)
#undef V
};

const unsigned anBL_to_nDL[] = {
#define V(_nBL) nDL_from_nBL(_nBL)
    V(  0),V(  1),V(  2),V(  3),V(  4),V(  5),V(  6),V(  7),V(  8),V(  9),
    V( 10),V( 11),V( 12),V( 13),V( 14),V( 15),V( 16),V( 17),V( 18),V( 19),
    V( 20),V( 21),V( 22),V( 23),V( 24),V( 25),V( 26),V( 27),V( 28),V( 29),
    V( 30),V( 31),V( 32),V( 33),V( 34),V( 35),V( 36),V( 37),V( 38),V( 39),
    V( 40),V( 41),V( 42),V( 43),V( 44),V( 45),V( 46),V( 47),V( 48),V( 49),
    V( 50),V( 51),V( 52),V( 53),V( 54),V( 55),V( 56),V( 57),V( 58),V( 59),
    V( 60),V( 61),V( 62),V( 63),V( 64),V( 65),V( 66),V( 67),V( 68),V( 69),
    V( 70),V( 71),V( 72),V( 73),V( 74),V( 75),V( 76),V( 77),V( 78),V( 79),
    V( 80),V( 81),V( 82),V( 83),V( 84),V( 85),V( 86),V( 87),V( 88),V( 89),
    V( 90),V( 31),V( 32),V( 33),V( 34),V( 35),V( 36),V( 37),V( 38),V( 39),
    V(100),V(101),V(102),V(103),V(104),V(105),V(106),V(107),V(108),V(109),
    V(110),V(111),V(112),V(113),V(114),V(115),V(116),V(117),V(118),V(119),
    V(120),V(121),V(122),V(123),V(124),V(125),V(126),V(127)
#undef V
};

#endif // defined(BPD_TABLE_RUNTIME_INIT)

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

#if defined(DEBUG)
int bHitDebugThreshold;
#endif // defined(DEBUG)

#if (cnDigitsPerWord != 1)

Word_t wWordsAllocated; // number of words allocated but not freed
Word_t wMallocs; // number of unfreed mallocs
Word_t wEvenMallocs; // number of unfreed mallocs of an even number of words

#if ! defined(cnMallocExtraWords)
#define cnMallocExtraWords  0
#endif // ! defined(cnMallocExtraWords)

#if defined(GUARDBAND)
  #if ! defined(cnGuardWords)
      #define cnGuardWords 1
  #endif // ! defined(cnGuardWords)
#else // defined(GUARDBAND)
  #undef  cnGuardWords
  #define cnGuardWords 0
#endif // defined(cnGuardWords)

static Word_t
MyMalloc(Word_t wWords)
{
    Word_t ww = JudyMalloc(wWords + cnMallocExtraWords);
    DBGM(printf("\nM: %p %"_fw"d words *%p "OWx" %"_fw"d\n",
                (void *)ww, wWords, (void *)&((Word_t *)ww)[-1],
                ((Word_t *)ww)[-1], ((Word_t *)ww)[-1]));
    // Validate our assumptions about dlmalloc as we prepare to use
    // some of the otherwise wasted bits.
    if (wWords < EXP(16)) {
#if defined(DEBUG)
        if ((((((Word_t *)ww)[-1] >> 4) << 1)
                != ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            && (((((Word_t *)ww)[-1] >> 4) << 1) - 2
                != ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            && (((((Word_t *)ww)[-1] >> 4) << 1) - 4
                != ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2)))
        {
            printf("\nM: Oops ww %p (wWords + cnMallocExtraWords"
                   " + cnGuardWords)"
                   " %"_fw"d 0x%lx &ww[-1] %p ww[-1] "OWx" ww[-1] %"_fw"d"
                   " ((ww[-1] >> 4) << 1) %"_fw"d 0x%lx\n\n",
                   (void *)ww,
                   wWords + cnMallocExtraWords + cnGuardWords,
                   wWords + cnMallocExtraWords + cnGuardWords,
                   (void *)&((Word_t *)ww)[-1],
                   ((Word_t *)ww)[-1], ((Word_t *)ww)[-1],
                   ((((Word_t *)ww)[-1] >> 4) << 1),
                   ((((Word_t *)ww)[-1] >> 4) << 1));
        }
#endif // defined(DEBUG)
        assert((((((Word_t *)ww)[-1] >> 4) << 1)
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((((Word_t *)ww)[-1] >> 4) << 1) - 2
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((((Word_t *)ww)[-1] >> 4) << 1) - 4
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2)));
        // Save ww[-1] to make sure we can use some of the bits in the word.
        // We are saving enough to handle mallocs up to nearly 1MB.
        DBG(((Word_t *)ww)[-1] |= (((Word_t *)ww)[-1] >> 4) << 32);
        assert((((Word_t *)ww)[-1] >> 32)
           == ((((Word_t *)ww)[-1] & MSK(32)) >> 4));
        // The following does not always hold on free.
        assert((((Word_t *)ww)[-1] & 0x0f) == 3);
    }
    assert(ww != 0);
    assert((ww & 0xffff000000000000UL) == 0);
    assert((ww & cnMallocMask) == 0);
    ++wMallocs; wWordsAllocated += wWords;
    if ( ! (wWords & 1) ) { ++wEvenMallocs; }
    return ww;
}

static void
MyFree(Word_t *pw, Word_t wWords)
{
    if ( ! (wWords & 1) ) { --wEvenMallocs; }
    --wMallocs; wWordsAllocated -= wWords;
    DBGM(printf("F: "OWx" %"_fw"d words pw[-1] %p\n",
                (Word_t)pw, wWords, (void *)pw[-1]));
    // make sure it is ok for us to use some of the bits in the word
    if (wWords < EXP(16)) {
#if defined(DEBUG)
        if ((pw[-1] >> 32) != ((pw[-1] & MSK(32)) >> 4)) {
            printf("pw %p pw[0] "OWx"\n", pw, pw[0]);
            printf("wWords %ld pw[-1] "OWx"\n", wWords, pw[-1]);
        }
#endif // defined(DEBUG)
        assert((pw[-1] >> 32) == ((pw[-1] & MSK(32)) >> 4));
        DBG(pw[-1] &= MSK(32)); // restore the value expected by dlmalloc
        if (!((((pw[-1] >> 4) << 1)
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((pw[-1] >> 4) << 1) - 2
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((pw[-1] >> 4) << 1) - 4
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))))
        {
            printf("F: Oops (wWords + cnMallocExtraWords + cnGuardWords) 0x%lx"
                   " (pw[-1] >> 3) 0x%lx\n",
                   wWords + cnMallocExtraWords + cnGuardWords, pw[-1] >> 3);
        }
        assert((((pw[-1] >> 4) << 1)
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((pw[-1] >> 4) << 1) - 2
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2))
            || (((pw[-1] >> 4) << 1) - 4
                == ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords, 2)));
    }
    JudyFree(pw, wWords + cnMallocExtraWords);
}

#if (cwListPopCntMax != 0)

// How many words are needed for a T_LIST leaf?
// The layout of a list leaf depends on ifdefs and context.
// One thing all T_LIST leaves have in common (presently) is an array of keys.
// There may or may not be:
// - some dummy words at the beginning: cnDummiesInList
// - followed by a pop count: ! PP_IN_LINK or (list hangs from root word
//   and cnDummiesInList == 0)
// - or followed by a dummy pop count: PP_IN_LINK and DUMMY_POP_CNT_IN_LIST
//   and list doesn't hang from root word
//   - PP_IN_LINK root pop count goes in a dummy word: cnDummiesInList
// - followed by a marker key 0 at pxKeys[-1]
// - followed by some padding to align beginning of list: PSPLIT_PARALLEL
// - followed by the array of keys at pxKeys[0] - pxKeys[nPopCnt - 1]
// - followed by replicas of last key to align end of list: PSPLIT_PARALLEL
// - followed by padding to an odd word boundary for malloc optimization
// - followed by a marker key -1
static int
ListWordsTypeList(Word_t wPopCntArg, unsigned nBL)
{
    (void)nBL;

    if (wPopCntArg == 0) { return 0; }

    int nBytesKeySz =
#if defined(COMPRESSED_LISTS)
        (nBL <=  8) ? 1 : (nBL <= 16) ? 2 :
  #if (cnBitsPerWord > 32)
        (nBL <= 32) ? 4 :
  #endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        sizeof(Word_t);

#if defined(OLD_LISTS)
    int nBytes = (N_LIST_HDR_KEYS + POP_SLOT(nBL)) * nBytesKeySz;
#if defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)
  #if ! defined(PSPLIT_SEARCH_WORD) \
      && ! ( defined(ALIGN_LISTS) && ! defined(PSPLIT_PARALLEL) )
    if (nBytesKeySz < (int)sizeof(Word_t))
  #endif // ! defined(PSPLIT_SEARCH_WORD) && ...
    { nBytes = ALIGN_UP(nBytes, sizeof(Bucket_t)); }
#endif // defined(ALIGN_LISTS) || defined(PSPLIT_PARALLEL)

    nBytes += wPopCntArg * nBytesKeySz; // add list of real keys
#if defined(ALIGN_LIST_ENDS) || defined(PSPLIT_PARALLEL)
    // Pad array of keys so the end is aligned.
    // We'll eventually fill the padding with a replica of the last real key
    // so parallel searching yields no false positives.
#if ! defined(PSPLIT_SEARCH_WORD) \
  && ! ( defined(ALIGN_LIST_ENDS) && ! defined(PSPLIT_PARALLEL) )
    // Ignore ALIGN_LIST_ENDS for (nBL >= cnBitsPerWord) unless
    // PSPLIT_SEARCH_WORD in order to improve memory usage at the top.
    if (nBytesKeySz < (int)sizeof(Word_t))
#endif // ! defined(PSPLIT_SEARCH_WORD) && ...
    { nBytes = DIV_UP(nBytes, sizeof(Bucket_t)) * sizeof(Bucket_t); }
#endif // defined(ALIGN_LIST_ENDS) || defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    // Make room for -1 at the end to help make search faster.
    // How should we handle LIST_END_MARKERS for parallel searches?
    nBytes += nBytesKeySz;
#endif // defined(LIST_END_MARKERS)
    return DIV_UP(nBytes, sizeof(Word_t)) | 1;
#else // defined(OLD_LISTS)
    return ls_nSlotsInList(wPopCntArg, nBL, nBytesKeySz)
                 * nBytesKeySz / sizeof(Word_t);
#endif // defined(OLD_LISTS)
}

// How many words needed for leaf?  Use T_ONE instead of T_LIST if possible.
// But do not embed.
static unsigned
ListWordsExternal(Word_t wPopCnt, unsigned nBL)
{
#if defined(USE_T_ONE)
    if (wPopCnt == 1) { return 1; }
#endif // defined(USE_T_ONE)

    return ListWordsTypeList(wPopCnt, nBL);
}

// How many words are needed for the specified list leaf?
// Use T_ONE instead of T_LIST if possible.
// Use embedded keys instead of T_ONE if possible.
int
ListWords(int nPopCnt, int nBL)
{
#if defined(EMBED_KEYS)
    // We need space for the keys, the pop count and the type.
    // What about PP_IN_LINK?  Do we need space for pop count if not at top?
    // What difference would it make?
    // One more embedded 30, 20, 15, 12 and 10-bit key?  Assuming we don't use
    // the extra word in the link for embedded values?
    if (nPopCnt <= EmbeddedListPopCntMax(nBL)) {
        return 0; // Embed the keys, if any, in wRoot.
    }
#endif // defined(EMBED_KEYS)

    return ListWordsExternal(nPopCnt, nBL);
}

#if 0
#define PSPLIT(_nWords, _nBL, _xKeyMin, _KeyMax, _xKey) \
    (_xKey - xKeyMin) * _nWords + _nWords / 2 / (_xKeyMax - _xKeyMin);

unsigned
HolyListWords(Word_t *pwKeys, unsigned nPopCnt, unsigned nBL)
{
    if (nBL != sizeof(uint16_t) * 8) { return 0; }
    unsigned nKeysPerWord = sizeof(Word_t) / sizeof(uint16_t);
    if (nPopCnt <= nKeysPerWord) { return 1; }
    uint16_t psKeys = pwKeys;
    sKeyMin = psKeys[0];
    sKeyMax = psKeys[nPopCnt - 1];
    //unsigned nWords = nPopCnt + (nKeysPerWord - 1) / nKeysPerWord;
}
#endif

static void
NewListCommon(Word_t *pwList, Word_t wPopCnt, unsigned nBL, unsigned nWords)
{
    (void)pwList; (void)wPopCnt; (void)nBL; (void)nWords;
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
#if defined(LIST_END_MARKERS)
#if defined(USE_T_ONE)
        if (wPopCnt != 1)
#endif // defined(USE_T_ONE)
        { ls_pcKeysNAT(pwList)[-1] = 0; }
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL1 += nWords); // JUDYA
        METRICS(j__AllocWordsJL12 += nWords); // JUDYB -- overloaded
    } else if (nBL <= 16) {
#if defined(LIST_END_MARKERS)
#if defined(USE_T_ONE)
        if (wPopCnt != 1)
#endif // defined(USE_T_ONE)
        { ls_psKeysNAT(pwList)[-1] = 0; }
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL2 += nWords); // JUDYA
        METRICS(j__AllocWordsJL16 += nWords); // JUDYB
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
#if defined(LIST_END_MARKERS)
#if defined(USE_T_ONE)
        if (wPopCnt != 1)
#endif // defined(USE_T_ONE)
        { ls_piKeysNAT(pwList)[-1] = 0; }
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL4 += nWords); // JUDYA
        METRICS(j__AllocWordsJL32 += nWords); // JUDYB
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
#if defined(LIST_END_MARKERS)
#if defined(USE_T_ONE)
        if (wPopCnt != 1)
#endif // defined(USE_T_ONE)
#if defined(PP_IN_LINK) && (cnDummiesInList == 0)
        // ls_pwKeys is for T_LIST not at top (it incorporates dummies
        // and markers, but not pop count)
        { ls_pwKeysNAT(pwList)[-1 + (nBL == cnBitsPerWord)] = 0; }
#else // defined(PP_IN_LINK) && (cnDummiesInList == 0)
        { ls_pwKeysNAT(pwList)[-1] = 0; }
#endif // defined(PP_IN_LINK) && (cnDummiesInList == 0)
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLLW += nWords); // JUDYA and JUDYB
    }

    // Should we be setting wPrefix here for PP_IN_LINK?

    DBGM(printf("NewList pwList %p wPopCnt "OWx" nBL %d nWords %d\n",
        (void *)pwList, wPopCnt, nBL, nWords));
}

// Allocate a new T_LIST leaf (even if the leaf could be embedded).
// If we want to do a one-word parallel search when doing PSPLIT_SEARCH
// then we have to make sure every word in the list is full of keys, e.g.
// no pop count.
static Word_t *
NewListTypeList(Word_t wPopCnt, unsigned nBL)
{
    assert(wPopCnt != 0);

    unsigned nWords = ListWordsTypeList(wPopCnt, nBL);

    Word_t *pwList;
#if defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    // this is overkill since we don't care if lists are aligned;
    // only that we don't cross a cache line boundary unnecessarily
    if ((nBL <= 16) && (nWords > 2)) {
        posix_memalign((void **)&pwList, 64, nWords * sizeof(Word_t));
    } else
#endif // defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    {
        pwList = (Word_t *)MyMalloc(nWords);
    }

#if ! defined(OLD_LISTS)
    pwList += nWords - 1;
#endif // ! defined(OLD_LISTS)

#if defined(PP_IN_LINK)
    if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK)
    {
        set_ls_xPopCnt(pwList, nBL, wPopCnt);
    }

#if defined(DL_IN_LL)
    set_ll_nDL(pwList, nBL_to_nDL(nBL));
#endif // defined(DL_IN_LL)

    NewListCommon(pwList, wPopCnt, nBL, nWords);

    return pwList;
}

static Word_t *
NewListExternal(Word_t wPopCnt, unsigned nBL)
{
#if defined(USE_T_ONE)
    if (wPopCnt == 1) {
        assert(wPopCnt != 0);
        Word_t *pwList = (Word_t *)MyMalloc(1);
        NewListCommon(pwList, wPopCnt, nBL, /* nWords */ 1);
        return pwList;
    }
#endif // defined(USE_T_ONE)

    assert(wPopCnt != 0);
    return NewListTypeList(wPopCnt, nBL);
}

// Allocate memory for a new list for the given wPopCnt.
// Use an embedded list if possible.
// If an embedded list is not possible,
// then use an external T_ONE if possible.
// Otherwise use T_LIST.
// Return NULL if no memory is allocated, i.e. wPopCnt == 0 or
// embedded list is possible.
Word_t *
NewList(int nPopCnt, int nBL)
{
#if defined(EMBED_KEYS)
    // We need space for the keys, the pop count and the type.
    // What about PP_IN_LINK?  See ListWords for more comments.
    if (nPopCnt * nBL + nBL_to_nBitsPopCntSz(nBL) + cnBitsMallocMask
            <= cnBitsPerWord)
    {
        return NULL;
    }
#endif // defined(EMBED_KEYS)

    return NewListExternal(nPopCnt, nBL);
}

int
OldList(Word_t *pwList, int nPopCnt, int nBL, int nType)
{
#if defined(NO_TYPE_IN_XX_SW)
    // How is OldList supposed to know whether it was called with an inflated
    // list that has a type and needs to be freed or an embedded list with no
    // type that does not need to be freed?
    // Wait.  The caller passes the type separate from pwList.  Lucky.
#endif // defined(NO_TYPE_IN_XX_SW)
    if (nType == T_EMBEDDED_KEYS) { DBGR(printf("OL: 0.\n")); return 0; }

    assert(((nType == T_ONE) && (nPopCnt == 1)) || (nType == T_LIST));
    int nWords = ((nType == T_LIST) ? ListWordsTypeList(nPopCnt, nBL)
                                    : ListWords(nPopCnt, nBL));

    DBGM(printf("Old pwList %p wLen %d nBL %d nPopCnt %d nType %d\n",
        (void *)pwList, nWords, nBL, nPopCnt, nType));

    if (nWords == 0) { return 0; }

#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        METRICS(j__AllocWordsJLL1 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL12 -= nWords); // JUDYB -- overloaded
    } else if (nBL <= 16) {
        METRICS(j__AllocWordsJLL2 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL16 -= nWords); // JUDYB
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        METRICS(j__AllocWordsJLL4 -= nWords); // JUDYA
        METRICS(j__AllocWordsJL32 -= nWords); // JUDYB
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
        METRICS(j__AllocWordsJLLW -= nWords); // JUDYA and JUDYB
    }

#if ! defined(OLD_LISTS)
    // Could be T_ONE?
    if (nType == T_LIST) { pwList -= nWords - 1; }
#endif // ! defined(OLD_LISTS)

#if defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    // this is overkill since we don't care if lists are aligned;
    // only that we don't cross a cache line boundary unnecessarily
    if ((nBL <= 16) && (nWords > 2)) {
        free(pwList);
    } else
#endif // defined(COMPRESSED_LISTS) && defined(PLACE_LISTS)
    {
        MyFree(pwList, nWords);
    }

    return nWords * sizeof(Word_t);
}

#endif // (cwListPopCntMax != 0)

// We don't need NewBitmap unless cnBitsLeftAtD1 > LOG(sizeof(Link_t) * 8).
// Hopefully, the compiler will figure it out and not emit it.
static Word_t *
NewBitmap(Word_t *pwRoot, int nBL, int nBLUp, Word_t wKey)
{
    (void)nBLUp; (void)wKey;
    Word_t wWords = EXP(nBL - cnLogBitsPerWord) + 1;

    Word_t *pwBitmap = (Word_t *)MyMalloc(wWords);

    if (nBL == nDL_to_nBL(2)) {
        // Use Branch Bitmap column for 2-digit bitmap.
        METRICS(j__AllocWordsJBB += wWords); // JUDYA
    } else {
        METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
    }
    METRICS(j__AllocWordsJL12 += wWords); // JUDYB -- overloaded

    DBGM(printf("NewBitmap nBL %d nBits "OWx
      " nBytes "OWx" wWords "OWx" pwBitmap "OWx"\n",
        nBL, EXP(nBL), EXP(nBL - cnLogBitsPerByte), wWords,
        (Word_t)pwBitmap));

    memset((void *)pwBitmap, 0, wWords * sizeof(Word_t));

    // Init wRoot before calling set_wr because set_wr may try to preserve
    // the high bits, e.g. if LVL_IN_WR_HB, so we want them to
    // be initialized.
    Word_t wRoot = 0; set_wr(wRoot, pwBitmap, T_BITMAP);

#if defined(SKIP_TO_BITMAP)
  #if defined(TYPE_IS_RELATIVE)
    set_wr_nDS(wRoot, nBL_to_nDL(nBLUp) - nBL_to_nDL(nBL));
    Set_nType(&wRoot, T_BITMAP);
  #else // defined(TYPE_IS_RELATIVE)
    set_pwr_nBL(&wRoot, nBL);
  #endif // defined(TYPE_IS_RELATIVE)
    if (nBLUp != nBL) {
//printf("\nNewBitmap: wKey "OWx" pwBitmap %p nBL %d &wPrefixPop %p wPrefixPop (before) "OWx"\n", wKey, (void *)pwBitmap, nBL, (void *)(pwBitmap + EXP(nBL - cnLogBitsPerWord)), *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));
    }
    set_w_wPrefixBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, wKey);
    if (nBLUp != nBL) {
//printf("After set_w_wPrefixBL: wPrefixPop "OWx"\n", *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));
        set_wr_nType(wRoot, T_SKIP_TO_BITMAP);
    }
#else // defined(SKIP_TO_BITMAP)
    assert(nBLUp == nBL);
#endif // defined(SKIP_TO_BITMAP)
    set_w_wPopCntBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, 0);
//printf("After set_w_wPopCntBL: wPrefixPop "OWx"\n", *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));

    *pwRoot = wRoot;

    return pwBitmap;
}

static int
GetBLR(Word_t *pwRoot, int nBL)
{
    (void)pwRoot;

#if defined(TYPE_IS_RELATIVE)
    assert(nDL_to_nBL(nBL_to_nDL(nBL)) == nBL);
#endif // defined(TYPE_IS_RELATIVE)

    return
  #if defined(SKIP_LINKS)
        ((tp_bIsSwitch(wr_nType(*pwRoot)) && tp_bIsSkip(wr_nType(*pwRoot)))
      #if defined(SKIP_TO_BITMAP)
            || (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP)
      #endif // defined(SKIP_TO_BITMAP)
            || 0)
      #if defined(TYPE_IS_RELATIVE)
            ? nDL_to_nBL(nBL_to_nDL(nBL) - wr_nDS(*pwRoot)) :
      #else // defined(TYPE_IS_RELATIVE)
            ? wr_nBL(*pwRoot) :
      #endif // defined(TYPE_IS_RELATIVE)
  #endif // defined(SKIP_LINKS)
              nBL ;
}

static Word_t
OldBitmap(Word_t *pwRoot, Word_t *pwr, int nBL)
{
    nBL = GetBLR(pwRoot, nBL);

    Word_t wWords = EXP(nBL - cnLogBitsPerWord) + 1;

    MyFree(pwr, wWords);

    if (nBL == nDL_to_nBL(2)) {
        // Use Branch Bitmap column for 2-digit bitmap.
        METRICS(j__AllocWordsJBB -= wWords); // JUDYA
    } else {
        METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
    }
    METRICS(j__AllocWordsJL12 -= wWords); // JUDYB -- overloaded

    *pwRoot = 0; // Do we need to clear the rest of the link, e.g. PP_IN_LINK?

    return wWords * sizeof(Word_t);
}

// Allocate a new switch.
// Zero its links.
// Initialize its prefix if there is one.  Need to know nDLUp for
// PP_IN_LINK to figure out if the prefix field exists.
// Initialize its bitmap if there is one.  Need to know nDLUp for
// BM_IN_LINK to figure out if the bitmap field exists.
// Need to know nDLUp to know if we need a skip link (and to figure nDS
// if TYPE_IS_RELATIVE).
// Install wRoot at pwRoot.  Need to know nDL.
// Account for the memory (for both JUDYA and JUDYB columns in Judy1LHTime).
// Need to know if we are at the bottom so we can count the memory as a
// bitmap leaf instead of a switch.
static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, int nBL,
#if defined(CODE_XX_SW)
          int nBitsIndexSzX,
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
          int bBmSw,
#endif // defined(CODE_BM_SW)
          int nBLUp, Word_t wPopCnt)
{
#if defined(CODE_XX_SW)
    if ((nBitsIndexSzX != nBL_to_nBitsIndexSz(nBL))
        /*&& (nBitsIndexSzX != cnBW)*/)
    {
        DBGI(printf("# NewSwitch(nBitsIndexSzX %d)\n", nBitsIndexSzX));
    }
#endif // defined(CODE_XX_SW)
    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);
#if defined(CODE_BM_SW)
    assert((sizeof(BmSwitch_t) % sizeof(Word_t)) == 0);
#endif // defined(CODE_BM_SW)

#if defined(CODE_XX_SW)
    int nBitsIndexSz = nBitsIndexSzX;
#else // defined(CODE_XX_SW)
    int nBitsIndexSz = nBL_to_nBitsIndexSz(nBL);
#endif // defined(CODE_XX_SW)
    Word_t wIndexCnt = EXP(nBitsIndexSz);

#if ! defined(NDEBUG)
#if defined(CODE_BM_SW)
    if (bBmSw)
#if defined(BM_IN_LINK)
    { Link_t ln; assert(wIndexCnt <= sizeof(ln.ln_awBm) * cnBitsPerByte); }
#else // defined(BM_IN_LINK)
    {
         BmSwitch_t sw;
         assert(wIndexCnt <= sizeof(sw.sw_awBm) * cnBitsPerByte);
    }
#endif // defined(BM_IN_LINK)
#endif // defined(CODE_BM_SW)
#endif // ! defined(NDEBUG)

    Word_t wLinks = wIndexCnt;

#if defined(BM_SW_FOR_REAL)
    if (bBmSw)
    {
  #if defined(BM_IN_LINK)
        if (nBLUp != cnBitsPerWord)
  #endif // defined(BM_IN_LINK)
        {
            wLinks = 1; // number of links in switch
        }
    }
#endif // defined(BM_SW_FOR_REAL)

    Word_t wWords =
#if defined(CODE_BM_SW)
        bBmSw ? sizeof(BmSwitch_t) :
#endif // defined(CODE_BM_SW)
            sizeof(Switch_t);

    // sizeof([Bm]Switch_t) includes one link; add the others
    wWords += (wLinks - 1) * sizeof(Link_t);
    wWords /= sizeof(Word_t);

    Word_t *pwr = (Word_t *)MyMalloc(wWords);

#if defined(CODE_BM_SW)
    if (bBmSw) {
        memset(pwr_pLinks((BmSwitch_t *)pwr), 0, wLinks * sizeof(Link_t));
    } else
#endif // defined(CODE_BM_SW)
    {
        memset(pwr_pLinks((Switch_t *)pwr), 0, wLinks * sizeof(Link_t));
#if defined(NO_TYPE_IN_XX_SW)
        // The links in a switch at nDL_to_nBL(2) have nBL < nDL_to_nBL(2).
        // Hence the '=' in the '<=' here.
        if (nBL <= nDL_to_nBL(2)) {
            DBGR(printf("NS: Init ln_wRoots.\n"));
            for (int nn = 0; nn < (int)wLinks; ++nn) {
                pwr_pLinks((Switch_t *)pwr)[nn].ln_wRoot = ZERO_POP_MAGIC;
            }
            DBGR(printf("NS: Done init ln_wRoots.\n"));
        }
#endif // defined(NO_TYPE_IN_XX_SW)
    }

#if defined(RAMMETRICS)
    // Is a branch with embedded bitmaps a branch?
    // Or is it a bitmap?  Let's use bitmap since we get more info that way.
    if (nBL <= (int)LOG(sizeof(Link_t) * 8)) {
        // Embedded bitmaps.
        // What if we have bits in the links that are not used as
        // bits in the bitmap?
        METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
    } else
#if defined(CODE_BM_SW)
    if (bBmSw) {
        METRICS(j__AllocWordsJBB  += wWords); // JUDYA
    } else
#endif // defined(CODE_BM_SW)
    { METRICS(j__AllocWordsJBU  += wWords); } // JUDYA
#endif // defined(RAMMETRICS)

#if defined(CODE_BM_SW)
    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx
                " nBL %d bBmSw %d nBLU %d wPopCnt %ld)"
                " pwr %p\n",
                (void *)pwRoot, wKey,
                nBL, bBmSw, nBLUp, (long)wPopCnt, (void *)pwr));
#endif // defined(CODE_BM_SW)
    DBGI(printf("\nNewSwitch nBL %d nDL %d nBLUp %d\n",
                nBL, nBL_to_nDL(nBL), nBLUp));

#if defined(CODE_BM_SW)
    if (bBmSw) {
  #if defined(SKIP_TO_BM_SW)
        if (nBL != nBLUp) {
            DBGI(printf("\nCreating T_SKIP_TO_BM_SW!\n"));
            set_wr_pwr(*pwRoot, pwr);
  #if defined(TYPE_IS_RELATIVE)
            set_wr_nDS(*pwRoot, nBL_to_nDL(nBLUp) - nBL_to_nDL(nBL));
  #else // defined(TYPE_IS_RELATIVE)
            set_wr_nBL(*pwRoot, nBL); // set nBL
  #endif // defined(TYPE_IS_RELATIVE)
            // set_wr_nDS and set_wr_nBL overwrite
            // the type field.  So we have to set
            // T_SKIP_TO_BM_SW after that.
            set_wr(*pwRoot, pwr, T_SKIP_TO_BM_SW); // set type
        } else
  #endif // defined(SKIP_TO_BM_SW)
        { set_wr(*pwRoot, pwr, T_BM_SW); }
    } else
#endif // defined(CODE_BM_SW)
    {
        set_wr_pwr(*pwRoot, pwr);
#if defined(CODE_XX_SW)
  #if defined(SKIP_TO_XX_SW)
      #if defined(TYPE_IS_RELATIVE)
        set_wr_nDS(*pwRoot, nBL_to_nDL(nBLUp) - nBL_to_nDL(nBL));
      #else // defined(TYPE_IS_RELATIVE)
        set_pwr_nBL(pwRoot, nBL);
      #endif // defined(TYPE_IS_RELATIVE)
  #endif // defined(SKIP_TO_XX_SW)
        set_pwr_nBW(pwRoot, nBitsIndexSz);
#endif // defined(CODE_XX_SW)
#if defined(NO_SKIP_AT_TOP)
        assert((nBLUp < cnBitsPerWord) || (nBL == nBLUp));
#endif // defined(NO_SKIP_AT_TOP)
        if (nBL == nBLUp) {
  #if defined(USE_XX_SW)
            if (nBL <= nDL_to_nBL(2)) {
                set_wr_nType(*pwRoot, T_XX_SW);
            } else
  #endif // defined(USE_XX_SW)
            { set_wr_nType(*pwRoot, T_SWITCH); }
        } else {
  #if defined(TYPE_IS_RELATIVE)
            // set_wr_nDS sets nType to T_SKIP_TO_SWITCH.
            set_wr_nDS(*pwRoot, nBL_to_nDL(nBLUp) - nBL_to_nDL(nBL));
  #else // defined(TYPE_IS_RELATIVE)
            set_wr_nBL(*pwRoot, nBL); // also sets nType == T_SKIP_TO_SWITCH
  #endif // defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
            if (nBL <= nDL_to_nBL(2)) {
                set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
            }
  #endif // defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
        }
    }

#if defined(CODE_BM_SW)
    if (bBmSw)
    {
#if defined(BM_IN_LINK)
        if (nBLUp < cnBitsPerWord)
#endif // defined(BM_IN_LINK)
        {
#if defined(BM_SW_FOR_REAL)

            memset(PWR_pwBm(pwRoot, pwr), 0, sizeof(PWR_pwBm(pwRoot, pwr)));

            Word_t wIndex = (wKey >> (nBL - nBitsIndexSz)) & (wIndexCnt - 1);

            SetBit(PWR_pwBm(pwRoot, pwr), wIndex);

#else // defined(BM_SW_FOR_REAL)

            // Mind the high-order bits of the bitmap word if/when the bitmap
            // is smaller than a whole word.
            // Mind endianness.
            if (nBitsIndexSz < cnLogBitsPerWord)
            {
                *PWR_pwBm(pwRoot, pwr) = EXP(wIndexCnt) - 1;
            }
            else
            {
                memset(PWR_pwBm(pwRoot, pwr), -1,
                       sizeof(PWR_pwBm(pwRoot, pwr)));
            }

#endif // defined(BM_SW_FOR_REAL)
        }
    }
#endif // defined(CODE_BM_SW)

#if defined(PP_IN_LINK)
    if (nBLUp < cnBitsPerWord)
#endif // defined(PP_IN_LINK)
    {
#if defined(SKIP_LINKS)
        assert(nBL <= nBLUp);

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
        if ((nBL == nBLUp)
#if defined(SKIP_PREFIX_CHECK)
#if ! defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            && ((nBL - nBitsIndexSz) > 32)
#else // (cnBitsPerWord > 32)
            && ((nBL - nBitsIndexSz) > 16)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            && ((nBL - nBitsIndexSz) > cnBitsInD1)
#endif // ! defined(PP_IN_LINK)
#endif // defined(SKIP_PREFIX_CHECK)
            && 1)
        {
            DBGI(printf(
              "Not installing prefix left nBL %d nBLUp %d wKey "OWx"\n",
                nBL, nBLUp, wKey));

            set_PWR_wPrefixBL(pwRoot, pwr, nBL, 0);
        }
        else
#endif // defined(NO_UNNECESSARY_PREFIX)
        {
#if defined(CODE_BM_SW)
            if (bBmSw) {
                set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL, wKey);
            } else
#endif // defined(CODE_BM_SW)
            {
                set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL, wKey);
            }
        }
#else // defined(SKIP_LINKS)
        // Why do we bother with this?  Should we make it debug only?
#if defined(CODE_BM_SW)
        if (bBmSw) {
            set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL, 0);
        } else
#endif // defined(CODE_BM_SW)
        {
            set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL, 0);
        }
#endif // defined(SKIP_LINKS)

#if defined(CODE_BM_SW)
        if (bBmSw) {
            set_PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBL, wPopCnt);
        } else
#endif // defined(CODE_BM_SW)
        {
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBL, wPopCnt);
        }

#if defined(CODE_BM_SW)
        DBGM(printf("NewSwitch PWR_wPrefixPop "OWx"\n",
            bBmSw ? PWR_wPrefixPop(pwRoot, (BmSwitch_t *)pwr)
                  : PWR_wPrefixPop(pwRoot, (Switch_t *)pwr)));
#endif // defined(CODE_BM_SW)
    }

    //DBGI(printf("After NewSwitch"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nBL; // nBL is not used for all ifdef combos
    (void)nBLUp; // nBLUp is not used for all ifdef combos
}

#if defined(CODE_BM_SW)
#if defined(BM_SW_FOR_REAL)
static Word_t OldSwitch(Word_t *pwRoot, int nBL, int bBmSw, int nBLUp);
static void
NewLink(Word_t *pwRoot, Word_t wKey, int nDLR, int nDLUp)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);
    int nBLR = nDL_to_nBL(nDLR);
    int nBLUp = nDL_to_nBL(nDLUp);

    DBGI(printf("NewLink(pwRoot %p wKey "OWx" nBLR %d)\n",
        (void *)pwRoot, wKey, nBLR));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));

#if defined(BM_IN_LINK)
    assert(nBLR != cnBitsPerWord);
#endif // defined(BM_IN_LINK)

    // What is the index of the new link?
    unsigned nBitsIndexSz = nBL_to_nBitsIndexSz(nBLR);
    Word_t wIndex
        = ((wKey >> (nBLR - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1));
    DBGI(printf("wKey "OWx" nBLR %d nBitsIndexSz %d wIndex (virtual) "OWx"\n",
                wKey, nBLR, nBitsIndexSz, wIndex));

    // How many links are there in the old switch?
    Word_t wPopCnt = 0;
    for (unsigned nn = 0; nn < DIV_UP(EXP(nBitsIndexSz), cnBitsPerWord); nn++)
    {
        wPopCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
    // Now we know how many links were in the old switch.

    // sizeof(BmSwitch_t) includes one link; add the others
    unsigned nWordsOld
         = (sizeof(BmSwitch_t) + (wPopCnt - 1) * sizeof(Link_t))
            / sizeof(Word_t);
    DBGI(printf("link count %"_fw"d nWordsOld %d\n", wPopCnt, nWordsOld));
    // What rule should we use to decide when to uncompress a bitmap switch?

    // 5/8 is close to the golden ratio
    // if (wPopCnt >= EXP(nBitsIndexSz) * 5 / 8)

    // Does this include the key were inserting now?  I think it does.
    Word_t wPopCntKeys = PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
    (void)wPopCntKeys;

    // If the number of null words we'd add by uncompressing the switch
    // is insignificant w.r.t. the population, then do it.
    int nWordsNull = sizeof(Switch_t) - nWordsOld
           + (EXP(nBitsIndexSz) - 2) * sizeof(Link_t) / sizeof(Word_t);
    (void)nWordsNull;

#if defined(DEBUG_INSERT)
    static Word_t sBitsReportedMask = 0;
    (void)sBitsReportedMask;
#endif // defined(DEBUG_INSERT)
#if 0
    if (0)
#else
    // Threshold for converting bm sw into uncompressed switch.
    // Words-per-Key Numerator / Words-per-Key Denominator.
    // Shouldn't we be checking to see if conversion is appropriate on
    // insert even if/when we're not adding a new link?
    if ((wPopCntTotal * cnBmSwWpkPercent / 100
            >= (wWordsAllocated /* + wMallocs + wEvenMallocs */ + nWordsNull))
        && (wPopCnt > EXP(nBitsIndexSz) * cnBmSwLinksPercent / 100))
#endif
    {
#if defined(DEBUG_INSERT)
        if ( ! (EXP(nBLR) & sBitsReportedMask) )
        {
            sBitsReportedMask |= EXP(nBLR);
            DBGI(printf("# Converting nKeys %ld nLinks %ld nBLR %d",
                   wPopCntKeys, wPopCnt, nBLR));
            DBGI(printf(" wPopCntTotal %ld wWordsAllocated %ld",
               wPopCntTotal, wWordsAllocated));
            DBGI(printf(" wMallocs %ld wEvenMallocs %ld nWordsNull %d\n",
               wMallocs, wEvenMallocs, nWordsNull));
        }
#endif // defined(DEBUG_INSERT)

#if 0
        printf("A PWR_pwBm %p *PWR_pwBm %p\n",
               (void *)PWR_pwBm(pwRoot, pwr), (void *)*PWR_pwBm(pwRoot, pwr));
        Dump(pwRootLast, 0, cnBitsPerWord);
        HexDump("Before NewSwitch", pwr, 200);
#endif
        Word_t *pwrNew
            = NewSwitch(pwRoot, wKey, nBLR,
#if defined(CODE_XX_SW)
                        nBL_to_nBitsIndexSz(nBLR),
#endif // defined(CODE_XX_SW)
                        /*bBmSw*/ 0, nBLUp, wPopCntKeys);
#if 0
        printf("B PWR_pwBm %p *PWR_pwBm %p\n",
               (void *)PWR_pwBm(pwRoot, pwr), (void *)*PWR_pwBm(pwRoot, pwr));
#endif
        unsigned mm = 0;
        for (unsigned nn = 0; nn < EXP(nBitsIndexSz); nn++) {
            if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn)) {
                pwr_pLinks((Switch_t *)pwrNew)[nn]
                    = pwr_pLinks((BmSwitch_t *)pwr)[mm];
                ++mm;
#if 0
        printf("nn %d ln_wRoot %p\n",
               nn, (void *)pwr_pLinks((Switch_t *)pwrNew)[nn].ln_wRoot);
        printf("%p\n", (void *)&pwr_pLinks((BmSwitch_t *)pwr)[mm]);
#endif
            }
        }
#if 0
        printf("C PWR_pwBm %p *PWR_pwBm %p\n",
               (void *)PWR_pwBm(pwRoot, pwr), (void *)*PWR_pwBm(pwRoot, pwr));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
#endif
        // NewSwitch installs a proper wRoot at *pwRoot.
    } else {
        // We replicate a bunch of newswitch here since
        // newswitch can create only empty bm sw.

        // Allocate memory for a new switch with one more link than the
        // old one.
        unsigned nWordsNew = nWordsOld + sizeof(Link_t) / sizeof(Word_t);
        *pwRoot = MyMalloc(nWordsNew);
        DBGI(printf("After malloc *pwRoot "OWx"\n", *pwRoot));

        // Where does the new link go?
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
        DBGI(printf("wIndex (physical) "OWx"\n", wIndex));

        // Copy the old switch to the new switch and insert the new link.
        memcpy(wr_pwr(*pwRoot), pwr,
            sizeof(BmSwitch_t) + (wIndex - 1) * sizeof(Link_t));
        DBGI(printf("PWR_wPopCnt %"_fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // Initialize the new link.
        DBGI(printf("pLinks %p\n",
                    (void *)pwr_pLinks((BmSwitch_t *)*pwRoot)));
        DBGI(printf("memset %p\n",
                    (void *)&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex]));
        memset(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex], 0, sizeof(Link_t));
        DBGI(printf("PWR_wPopCnt A %"_fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        memcpy(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex + 1],
               &pwr_pLinks((BmSwitch_t *)pwr)[wIndex],
            (wPopCnt - wIndex) * sizeof(Link_t));

        DBGI(printf("PWR_wPopCnt B %"_fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // Set the bit in the bitmap indicating that the new link exists.
        SetBit(PWR_pwBm(pwRoot, *pwRoot),
            ((wKey >> (nBLR - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1)));
        DBGI(printf("PWR_wPopCnt %"_fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));

        if (nBLR <= (int)LOG(sizeof(Link_t) * 8)) {
            METRICS(j__AllocWordsJLB1 += nWordsNew); // JUDYA
        } else
#if defined(RETYPE_FULL_BM_SW)
        if (wPopCnt == EXP(nBitsIndexSz) - 1) {
  #if defined(DEBUG_INSERT)
            if ( ! (EXP(nBLR) & sBitsReportedMask) )
            {
                sBitsReportedMask |= EXP(nBLR);
                printf("# Retyping full BM_SW nKeys %ld nLinks %ld nBLR %d",
                       wPopCntKeys, wPopCnt, nBLR);
                printf(" wPopCntTotal %ld wWordsAllocated %ld",
                       wPopCntTotal, wWordsAllocated);
                printf(" wMallocs %ld wEvenMallocs %ld nWordsNull %d\n",
                       wMallocs, wEvenMallocs, nWordsNull);
            }
  #endif // defined(DEBUG_INSERT)
            METRICS(j__AllocWordsJBU  += nWordsNew); // JUDYA
        } else
#endif // defined(RETYPE_FULL_BM_SW)
        {
            METRICS(j__AllocWordsJBB  += nWordsNew); // JUDYA
        }

        // Update the type field in *pwRoot if necessary.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #if defined(RETYPE_FULL_BM_SW)
        if (wPopCnt == EXP(nBitsIndexSz) - 1) {
            // Bitmap switch is fully populated.
            // Let's change the type so Lookup is faster.
            // Hopefully we never get here because we convert to a true
            // uncompressed switch first.
            // I wonder if there is a clever way to reuse the space
            // used by the bitmaps here.
            // I wonder about converting to a skip link here if we're
            // configured with no skip link to bm switch.
      #if defined(SKIP_TO_BM_SW)
            //assert(BM_IN_NON_BM_SW);
          #if defined(LVL_IN_WR_HB)
            if (tp_bIsSkip(wr_nType(wRoot))) {
              #if defined(TYPE_IS_RELATIVE)
                set_wr_nDS(*pwRoot, wr_nDS(wRoot));
              #else // defined(TYPE_IS_RELATIVE)
                set_wr_nBL(*pwRoot, nBLR);
              #endif // defined(TYPE_IS_RELATIVE)
            }
          #endif // defined(LVL_IN_WR_HB)
            set_wr_nType(*pwRoot,
                         (wr_nType(wRoot) == T_SKIP_TO_BM_SW)
                             ? T_SKIP_TO_SWITCH : T_SWITCH);
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_NON_BM_SW)
            // Conserve precious type values if possible.
            set_wr_nType(*pwRoot, T_SWITCH);
          #else // defined(BM_IN_NON_BM_SW)
            set_wr_nType(*pwRoot, T_FULL_BM_SW);
          #endif // defined(BM_IN_NON_BM_SW)
      #endif // defined(SKIP_TO_BM_SW)
        } else
  #endif // defined(RETYPE_FULL_BM_SW)
        {
            int nType = wr_nType(wRoot);
  #if ! defined(LVL_IN_WR_HB)
            set_wr_nType(*pwRoot, nType);
  #endif // ! defined(LVL_IN_WR_HB)
            // depth is preserved if the beginning of the switch is copied
            // in some cases
  #if defined(LVL_IN_WR_HB)
      #if defined(SKIP_TO_BM_SW)
            if (nType == T_SKIP_TO_BM_SW) {
          #if defined(TYPE_IS_RELATIVE)
                set_wr_nDS(*pwRoot, wr_nDS(wRoot));
                assert(wr_nDS(*pwRoot) == wr_nDS(wRoot));
          #else // defined(TYPE_IS_RELATIVE)
                set_wr_nBL(*pwRoot, nBLR);
                assert(wr_nBL(*pwRoot) == wr_nBL(wRoot));
          #endif // defined(TYPE_IS_RELATIVE)
            }
      #endif // defined(SKIP_TO_BM_SW)
  #endif // defined(LVL_IN_WR_HB)
            set_wr_nType(*pwRoot, nType);

        }
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    }

    OldSwitch(&wRoot, nBLR, /* bBmSw */ 1, nBLUp);

    //DBGI(printf("After NewLink"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

static Word_t
OldSwitch(Word_t *pwRoot, int nBL,
#if defined(CODE_BM_SW)
          int bBmSw,
#endif // defined(CODE_BM_SW)
          int nBLUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    int nBitsIndexSz;
#if defined(USE_XX_SW)
    if (tp_bIsXxSw(wr_nType(*pwRoot))) {
        nBitsIndexSz = pwr_nBW(pwRoot);
    } else
#endif // defined(USE_XX_SW)
    { nBitsIndexSz = nBL_to_nBitsIndexSz(nBL); }

    Word_t wLinks = EXP(nBitsIndexSz);

#if defined(CODE_BM_SW)
#if defined(BM_SW_FOR_REAL)
    if (bBmSw)
    {
  #if defined(BM_IN_LINK)
        if (nBLUp != cnBitsPerWord)
  #endif // defined(BM_IN_LINK)
        {
            // How many links are there in the old switch?
            wLinks = 0;
            for (int nn = 0;
                     nn < (int)DIV_UP(EXP(nBL_to_nBitsIndexSz(nBL)),
                                  cnBitsPerWord);
                     nn++)
            {
                wLinks += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
            }
            assert(wLinks <= EXP(nBL_to_nBitsIndexSz(nBL)));
            // Now we know how many links were in the old switch.
        }
    }
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

#if defined(CODE_BM_SW)
    Word_t wWords = bBmSw ? sizeof(BmSwitch_t) : sizeof(Switch_t);
#else // defined(CODE_BM_SW)
    Word_t wWords = sizeof(Switch_t);
#endif // defined(CODE_BM_SW)
    // sizeof([Bm]Switch_t) includes one link; add the others
    wWords += (wLinks - 1) * sizeof(Link_t);
    wWords /= sizeof(Word_t);

#if defined(RAMMETRICS)
    if (nBL <= (int)LOG(sizeof(Link_t) * 8)) {
        // Embedded bitmaps.
        METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
    } else
#if defined(CODE_BM_SW)
    if (tp_bIsBmSw(wr_nType(*pwRoot))
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        && (wr_nType(*pwRoot) != T_FULL_BM_SW)
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        ) 
    {
        METRICS(j__AllocWordsJBB  -= wWords); // JUDYA
    } else
#endif // defined(CODE_BM_SW)
    { METRICS(j__AllocWordsJBU  -= wWords); } // JUDYA
#endif // defined(RAMMETRICS)

    DBGR(printf("\nOldSwitch nBL %d nBLU %d wWords %"_fw"d "OWx"\n",
         nBL, nBLUp, wWords, wWords));

    MyFree(pwr, wWords);

    return wWords * sizeof(Word_t);

    (void)nBL; // silence compiler
    (void)nBLUp; // silence compiler
}

static Word_t
GetPopCnt(Word_t *pwRoot, int nDL)
{
    int nBLR = GetBLR(pwRoot, nDL_to_nBL(nDL));
    int nDLR = nBL_to_nDL(nBLR);
    Word_t wPopCnt;

#if defined(SKIP_TO_BITMAP)
    if (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP) {
#if defined(TYPE_IS_RELATIVE)
        int nBL = nDL_to_nBL(nDLR); // nDL_to_nBL(nDL - wr_nDS(*pwRoot));
#else // defined(TYPE_IS_RELATIVE)
        int nBL = wr_nBL(*pwRoot);
#endif // defined(TYPE_IS_RELATIVE)
#if 0
        printf("\nhere nBL %d\n", nBL);
        printf("wPrefixPop "OWx"\n",
               *(wr_pwr(*pwRoot) + EXP(nBL - cnLogBitsPerWord)));
#endif // 0
        wPopCnt = w_wPopCntBL(*(wr_pwr(*pwRoot) + EXP(nBL - cnLogBitsPerWord)), nBL);
        if (wPopCnt == 0) {
            wPopCnt += wPrefixPopMask(nDLR) + 1 ; // ? full pop at top ?
        }
        return wPopCnt;
    }
#endif // defined(SKIP_TO_BITMAP)

    wPopCnt =
      #if defined(CODE_BM_SW)
        tp_bIsBmSw(wr_nType(*pwRoot))
            ? PWR_wPopCnt(pwRoot, (BmSwitch_t *)wr_pwr(*pwRoot), nDLR) :
      #endif // defined(CODE_BM_SW)
              PWR_wPopCnt(pwRoot, (  Switch_t *)wr_pwr(*pwRoot), nDLR) ;

    // We use wRoot != 0 to disambiguate PWR_wPopCnt == 0.
    // Hence we cannot allow Remove to leave
    // wRoot != 0 unless the actual pop count is not zero.
    if ((wPopCnt == 0) && (*pwRoot != 0)) {
         wPopCnt += wPrefixPopMask(nDLR) + 1 ; // ? full pop at top ?
    }

    return wPopCnt;
}

#if defined(PP_IN_LINK)

// Sum up the pop count.
// It assumes *pwRoot is in a link to a switch.
// nBLUp is the bits left to decode after getting to pwRoot.
// if *pwRoot is in a skip link, then nBLUp must be adjusted by the skip
// amount to get the digits left at the next node.
static Word_t
Sum(Word_t *pwRoot, int nBLUp)
{
    assert(tp_bIsSwitch(wr_nType(*pwRoot)));

#if defined(CODE_BM_SW) && defined(BM_IN_LINK)
    assert( ! tp_bIsBmSw(wr_nType(*pwRoot)) || (nBLUp != cnBitsPerWord) );
#endif // defined(CODE_BM_SW) && defined(BM_IN_LINK)

    int nBL = GetBLR(pwRoot, nBLUp);

    Link_t *pLinks =
#if defined(CODE_BM_SW)
        tp_bIsBmSw(wr_nType(*pwRoot))
            ? pwr_pLinks((BmSwitch_t *)wr_pwr(*pwRoot)) :
#endif // defined(CODE_BM_SW)
              pwr_pLinks((  Switch_t *)wr_pwr(*pwRoot)) ;

    Word_t wPopCnt = 0;
    Word_t xx = 0;
    for (int nn = 0; nn < (int)EXP(nBL_to_nBitsIndexSz(nBL)); nn++)
    {
#if defined(CODE_BM_SW)
        if ( ! tp_bIsBmSw(wr_nType(*pwRoot))
                    || BitIsSet(PWR_pwBm(pwRoot, wr_pwr(*pwRoot)), nn) )
#endif // defined(CODE_BM_SW)
        {
            wPopCnt += GetPopCnt(&pLinks[xx++].ln_wRoot, nDL - 1);
        }
    }

    return wPopCnt;
}

#endif // defined(PP_IN_LINK)

Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, int nBL, int bDump)
{
    Word_t *pwRootArg = pwRoot;
#if defined(BM_IN_LINK) || defined(PP_IN_LINK)
    int nBLArg = nBL;
#endif // defined(BM_IN_LINK) || defined(PP_IN_LINK)
    Word_t wRoot = *pwRoot;
    // nType is not valid for NO_TYPE_IN_XX_SW if nBL >= nDL_to_nBL(2)
    int nType = wr_nType(wRoot); (void)nType; // silence gcc
    // pwr is not valid for NO_TYPE_IN_XX_SW if nBL >= nDL_to_nBL(2)
    Word_t *pwr = wr_pwr(wRoot);
    int nBitsIndexSz;
    Link_t *pLinks;
    Word_t wBytes = 0;

#if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1);
#endif // ! defined(USE_XX_SW)

    if ( ! bDump )
    {
        DBGR(printf("FreeArrayGuts pwR "OWx" wPrefix "OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBL, bDump));
        DBGR(printf("wRoot "OWx"\n", wRoot));
    }

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        if (wRoot == ZERO_POP_MAGIC) { return 0; }
  #if defined(HANDLE_BLOWOUTS)
        if ((wRoot & BLOWOUT_MASK(nBL)) == ZERO_POP_MAGIC) {
            if (bDump) { printf(" blowout\n"); return 0; }
            assert(0); // not yet
        }
  #endif // defined(HANDLE_BLOWOUTS)

        //DBG(printf("FAG: goto embeddedKeys.\n"));
        if (bDump) {
            printf(" wPrefix "OWx, wPrefix);
            printf(" nBL %2d", nBL);
            printf(" pwRoot "OWx, (Word_t)pwRoot);
            printf(" wr "OWx, wRoot);
            goto embeddedKeys;
        }
        return 0;
    }
#endif // defined(NO_TYPE_IN_XX_SW)

    if (wRoot == 0)
    {
#if defined(BM_SW_FOR_REAL)
        if (bDump)
        {
            printf(" wPrefix "OWx, wPrefix);
            printf(" nBL %2d", nBL);
            printf(" pwRoot "OWx, (Word_t)pwRoot);
            printf(" wr "OWx, wRoot);
            printf("\n");
        }
#endif // defined(BM_SW_FOR_REAL)
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

#if defined(SKIP_TO_BITMAP)
    if (nType == T_SKIP_TO_BITMAP) {
        if (bDump) {
            int nBLR = nBL;
            if (tp_bIsSkip(nType)) {
                nBLR = GetBLR(pwRoot, nBL);
            }
            printf(" wPrefixPop "OWx, *(pwr + EXP(nBLR - cnLogBitsPerWord)));
            printf(" w_wPopCnt %ld",
                   w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR));
            printf(" skip to bitmap\n");
            return 0;
        }

        return OldBitmap(pwRoot, pwr, nBL);
    }
#endif // defined(SKIP_TO_BITMAP)

    if ((nType == T_BITMAP)
        || (1
#if ! defined(USE_XX_SW)
            && (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)
#endif // ! defined(USE_XX_SW)
            && (EXP(nBL) <= sizeof(Link_t) * 8)))
    {
#if ! defined(USE_XX_SW)
        assert((nType == T_BITMAP) || (nBL == cnBitsInD1));
#endif // ! defined(USE_XX_SW)
#if defined(PP_IN_LINK)
        if (bDump)
        {
            assert(nBLArg != cnBitsPerWord);
 
            if (EXP(cnBitsInD1) > sizeof(Link_t) * 8)
            {
                printf(" wr_wPopCnt %3"_fw"u",
                       PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL));
                printf(" wr_wPrefix "OWx,
                       PWR_wPrefixBL(pwRoot, (Switch_t *)NULL, nBL));
            }
        }
#endif // defined(PP_IN_LINK)

        // If the bitmap is not embedded, then we have more work to do.
        // The test can be done at compile time and will make one the
        // other clauses go away.
        if (EXP(cnBitsInD1) > sizeof(Link_t) * 8)
        {
            if ( ! bDump )
            {
                return OldBitmap(pwRoot, pwr, nBL);
            }

            printf(" nWords %4"_fw"d", EXP(nBL - cnLogBitsPerWord));
            for (Word_t ww = 0; (ww < EXP(nBL - cnLogBitsPerWord)); ww++) {
                if ((ww % 8) == 0) {
                    printf("\n");
                }
                printf(" "Owx, pwr[ww]);
            }
        }
        else
        {
            if (bDump) {
                if (cnBitsInD1 > cnLogBitsPerWord) {
                    printf(" nWords %4"_fw"d", EXP(nBL - cnLogBitsPerWord));
                    for (Word_t ww = 0;
                         (ww < EXP(nBL - cnLogBitsPerWord)); ww++)
                    {
                        if ((ww % 8) == 0) { printf("\n"); }
                        printf(" "Owx,
                               ((Word_t *)
                                   STRUCT_OF(pwRoot, Link_t, ln_wRoot))[ww]);
                    }
                } else {
                    printf(" wr "OWx, wRoot);
                }
            }
        }

        if (bDump)
        {
            printf("\n");
        }

        return 0;
    }

#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #if defined(TYPE_IS_RELATIVE)
    assert( ! tp_bIsSkip(nType) || (wr_nDS(wRoot) >= 1) );
  #else // defined(TYPE_IS_RELATIVE)
    assert( ! tp_bIsSkip(nType) || (wr_nBL(wRoot) < nBL) );
    assert( ! tp_bIsSkip(nType) || (wr_nBL(wRoot) >= cnBitsInD1) );
  #endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(USE_T_ONE)

        if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS))
        {
            int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
#if defined(EMBED_KEYS)
            if (EmbeddedListPopCntMax(nBL) != 0) {
                goto embeddedKeys;
embeddedKeys:;
                wPopCnt = wr_nPopCnt(wRoot, nBL);
            } else
#endif // defined(EMBED_KEYS)
            { wPopCnt = 1; }

            if (!bDump) {
#if defined(NO_TYPE_IN_XX_SW)
                assert(nBL >= nDL_to_nBL(2));
#endif // defined(NO_TYPE_IN_XX_SW)
                // This OldList is a no-op and will return zero if
                // the key(s) is(are) embedded.
                return OldList(pwr, /* wPopCnt */ 1, nBL, nType);
            }

            printf(" tp_wPopCnt %3d", (int)wPopCnt);

#if defined(PP_IN_LINK)
            assert(nBL == nBLArg);
            if (nBLArg == cnBitsPerWord) {
                printf(" wr_wPrefix        N/A");
            } else {
                printf(" wr_wPrefix "OWx,
                       PWR_wPrefixBL(pwRoot, NULL, nBL));
            }
#endif // defined(PP_IN_LINK)

#if defined(EMBED_KEYS)
            if (EmbeddedListPopCntMax(nBL) != 0) {
                for (unsigned nn = 0; nn < wPopCnt; nn++) {
                    printf(" %08"_fw"x",
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot >> (cnBitsPerWord - ((nn + nPopCntMax - wPopCnt + 1) * nBL)))
#else // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot >> (cnBitsPerWord - ((nn + 1) * nBL)))
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                            & MSK(nBL));
                }
                printf("\n");
            } else
#endif // defined(EMBED_KEYS)
            {
                printf(" "Owx"\n", *pwr);
            }
        }
        else
#endif // defined(USE_T_ONE)
        {
            assert(nType == T_LIST);

#if defined(PP_IN_LINK)
            if (nBL != cnBitsPerWord)
            {
                wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
            }
            else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = ls_xPopCnt(pwr, nBL);
            }

            if (!bDump)
            {
                return OldList(pwr, wPopCnt, nBL, nType);
            }

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
                       PWR_wPrefixBL(pwRoot, NULL, nBL));
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
                xx += ((nBLArg == cnBitsPerWord) && (cnDummiesInList == 0));
#endif // defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    printf(" %02x", ls_pcKeysNAT(pwr)[xx]);
                } else if (nBL <= 16) {
                    printf(" %04x", ls_psKeysNAT(pwr)[xx]);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    printf(" %08x", ls_piKeysNAT(pwr)[xx]);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                { printf(" "Owx, ls_pwKeysNAT(pwr)[xx]); }
            }
            printf("\n");
        }

        return 0;
    }
#endif // (cwListPopCntMax != 0)

    // Switch

    int nBLPrev = nBL;

#if defined(CODE_BM_SW)
    int bBmSw = tp_bIsBmSw(nType);
  #if ! defined(SKIP_TO_BM_SW)
    if ( ! bBmSw )
  #endif // defined(SKIP_TO_BM_SW)
#endif // defined(CODE_BM_SW)
#if defined(SKIP_LINKS)
    {
        if (tp_bIsSkip(nType)) {
            nBL = GetBLR(pwRoot, nBL);
        }
    }
#endif // defined(SKIP_LINKS)

#if defined(CODE_XX_SW)
    if (tp_bIsXxSw(nType)) {
        nBitsIndexSz = pwr_nBW(pwRoot);
    } else
#endif // defined(CODE_XX_SW)
    { nBitsIndexSz = nBL_to_nBitsIndexSz(nBL); }

    pLinks = 
#if defined(CODE_BM_SW)
             bBmSw ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
                     pwr_pLinks((  Switch_t *)pwr) ;

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBLArg >= cnBitsPerWord)
        {
            // Add 'em up.
            Word_t wPopCnt = Sum(pwRoot, cnBitsPerWord);

            printf(" sm_wPopCnt %3"_fw"u", wPopCnt);
            printf(" wr_wPrefix        N/A");
        }
        else
#endif // defined(PP_IN_LINK)
        {
            printf(" wr_wPopCnt %3"_fw"u",
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBL) :
#endif // defined(CODE_BM_SW)
                           PWR_wPopCntBL(pwRoot, (  Switch_t *)pwr, nBL) );
            printf(" wr_wPrefix "OWx,
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL) :
#endif // defined(CODE_BM_SW)
                           PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBL) );
        }

        printf(" wr_nBL %2d", nBL);
#if defined(CODE_BM_SW)
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask "OWx, wPrefixPopMask(nDL));
        //printf(" pLinks "OWx, (Word_t)pLinks);
        if (bBmSw) {
#if defined(BM_IN_LINK)
            if (nBLArg != cnBitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                printf(" Bm");
                // Bitmaps are an integral number of words.
                for (int nn = 0;
                     nn < DIV_UP((int)EXP(nBL_to_nBitsIndexSz(nBL)),
                                 cnBitsPerWord);
                     nn ++)
                {
                    if ((nn % 8) == 0) {
                        printf("\n");
                    }
                    printf(" "Owx, PWR_pwBm(pwRoot, pwr)[nn]);
                }
            }
        }
#endif // defined(CODE_BM_SW)
        printf("\n");
    }

    // skip link has extra prefix bits
    if (nBLPrev > nBL)
    {
        wPrefix =
#if defined(CODE_BM_SW)
            bBmSw ? PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL) :
#endif // defined(CODE_BM_SW)
                    PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBL) ;
    }

    nBL -= nBitsIndexSz;

    Word_t xx = 0;
    for (Word_t nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(CODE_BM_SW)
#if defined(BM_IN_LINK)
        assert(nBLArg != cnBitsPerWord);
#endif // defined(BM_IN_LINK)
        if ( ! bBmSw || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(CODE_BM_SW)
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

    wBytes += OldSwitch(pwRootArg, nBL + nBitsIndexSz,
#if defined(CODE_BM_SW)
                        bBmSw,
#endif // defined(CODE_BM_SW)
                        nBLPrev);

    DBGR(printf("memset(%p, 0, %zd)\n",
         (void *)STRUCT_OF(pwRootArg, Link_t, ln_wRoot), sizeof(Link_t)));

#if defined(PP_IN_LINK) || defined(BM_IN_LINK)
    if (nBLArg == cnBitsPerWord) {
        *pwRootArg = 0;
    } else
#endif // defined(PP_IN_LINK) || defined(BM_IN_LINK)
    {
        memset(STRUCT_OF(pwRootArg, Link_t, ln_wRoot), 0, sizeof(Link_t));
    }

    return wBytes;
}

#if defined(DEBUG)
void
Dump(Word_t *pwRoot, Word_t wPrefix, int nBL)
{
    if (bHitDebugThreshold) {
        printf("\nDump\n");
        FreeArrayGuts(pwRoot, wPrefix, nBL, /* bDump */ 1);
        printf("End Dump\n");
    }
}
#endif // defined(DEBUG)

#if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
  #if ! defined(PACK_KEYS_RIGHT)
      //#if ! defined(FILL_WITH_ONES)
static void
InsertEmbedded(Word_t *pwRoot, int nBL, Word_t wKey)
{
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
    int nPopCnt = wr_nPopCnt(*pwRoot, nBL);
    assert(nPopCnt < nPopCntMax);
    DBGI(printf("\nInsert: wRoot "OWx" nBL %d wKey "OWx" nPopCnt %d Max %d\n",
                *pwRoot, nBL, wKey, nPopCnt, nPopCntMax));
#if defined(NO_TYPE_IN_XX_SW)
    // clear out ZERO_POP_MAGIC
    if ((nPopCnt == 0) && (nBL < nDL_to_nBL(2))) { *pwRoot = 0; }
#endif // defined(NO_TYPE_IN_XX_SW)
    // find the slot
    wKey &= MSK(nBL);
    int nSlot = 0;
    for (; nSlot < nPopCnt; ++nSlot) {
        if (GetBits(*pwRoot, nBL, cnBitsPerWord - (nSlot + 1) * nBL) > wKey) {
            break;
        }
    }
    DBGI(printf("Insert: wKey "OWx" nSlot %d", wKey, nSlot));
    if (nSlot < nPopCnt) {
        Word_t wLowBits
            = GetBits(*pwRoot, (nPopCnt - nSlot) * nBL,
                      cnBitsPerWord - (nPopCnt * nBL));
        DBGI(printf(" wLowBits "OWx, wLowBits));
        SetBits(pwRoot, (nPopCnt - nSlot) * nBL, 
                cnBitsPerWord - ((nPopCnt + 1) * nBL), wLowBits);
    }
    SetBits(pwRoot, nBL, cnBitsPerWord - (nSlot + 1) * nBL, wKey); 
    set_wr_nPopCnt(*pwRoot, nBL, nPopCnt + 1);
    DBGI(printf(" wRoot "OWx" nPopCnt %d\n",
                *pwRoot, wr_nPopCnt(*pwRoot, nBL)));
}
      //#endif // ! defined(FILL_WITH_ONES)
  #endif // ! defined(PACK_KEYS_RIGHT)
#endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)

#if (cwListPopCntMax != 0)

#if defined(SORT_LISTS)

// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsertWord(Word_t *pTgt, Word_t *pSrc, unsigned nKeys, Word_t wKey)
{
    DBGI(printf("\nCopyWithInsertWord(pTgt %p pSrc %p nKeys %d wKey "OWx")\n",
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
            //if (pSrc[n] == wKey) Dump(pwRootLast, 0, cnBitsPerWord);
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

#if defined(LIST_END_MARKERS)
    pTgt[nKeys+1] = -1;
#endif // defined(LIST_END_MARKERS)
}

#if defined(COMPRESSED_LISTS)

#if (cnBitsPerWord > 32)
static void
CopyWithInsertInt(uint32_t *pTgt, uint32_t *pSrc, unsigned nKeys,
                  uint32_t iKey)
{
    DBGI(printf("\nCopyWithInsertInt(pTgt %p pSrc %p nKeys %d iKey 0x%x)\n",
                pTgt, pSrc, nKeys, iKey));
#if (cwListPopCntMax != 0)
    unsigned int ai[cwListPopCntMax]; // buffer for move if pSrc == pTgt
#else // (cwListPopCntMax != 0)
    unsigned int ai[1]; // buffer for move if pSrc == pTgt
#endif // (cwListPopCntMax != 0)
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++)
    {
        if (pSrc[n] >= iKey)
        {
            assert(pSrc[n] != iKey);
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

    pTgt[n] = iKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail

#if defined(PSPLIT_PARALLEL)
    // pad to a word boundary with the last key in the list so
    // parallel search won't give a false positive and the last key
    // in the last word is the maximum key in the list
#if defined(PARALLEL_128) // requires ALIGN_LIST_ENDS
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(__m128i)))) != 0)
#else // defined(PARALLEL_128)
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(Word_t)))) != 0)
#endif // defined(PARALLEL_128)
    {
        pTgt[nKeys+1] = pTgt[nKeys]; // or pTgt[0] or iKey
        ++nKeys;
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[nKeys+1] = -1;
#endif // defined(LIST_END_MARKERS)
}
#endif // (cnBitsPerWord > 32)

static void
CopyWithInsertShort(uint16_t *pTgt, uint16_t *pSrc, unsigned nKeys,
                    uint16_t sKey)
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
        if (pSrc[n] >= sKey)
        {
            assert(pSrc[n] != sKey);
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

    pTgt[n] = sKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail

#if defined(PSPLIT_PARALLEL)
    // pad to a word boundary with a key that exists in the list so
    // parallel search won't give a false positive
#if defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(__m128i)))) != 0)
#else // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(Word_t)))) != 0)
#endif // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    {
        pTgt[nKeys+1] = pTgt[nKeys]; // or pTgt[0] or sKey
        ++nKeys;
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[nKeys+1] = -1;
#endif // defined(LIST_END_MARKERS)
}

static void
CopyWithInsertChar(uint8_t *pTgt, uint8_t *pSrc, unsigned nKeys, uint8_t cKey)
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
        if (pSrc[n] >= cKey)
        {
            assert(pSrc[n] != cKey);
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

    pTgt[n] = cKey; // insert the key

    COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail

#if defined(PSPLIT_PARALLEL)
    // pad to a word boundary with a key that exists in the list so
    // parallel search won't give a false positive
#if defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(__m128i)))) != 0)
#else // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    while (((Word_t)&pTgt[nKeys+1] & MSK(LOG(sizeof(Word_t)))) != 0)
#endif // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
    {
        pTgt[nKeys+1] = pTgt[nKeys]; // or pTgt[0] or cKey
        ++nKeys;
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[nKeys+1] = -1;
#endif // defined(LIST_END_MARKERS)
}

#endif // defined(COMPRESSED_LISTS)
#endif // defined(SORT_LISTS)
#endif // (cwListPopCntMax != 0)

Status_t
InsertAtDl1(Word_t *pwRoot, Word_t wKey, int nDL,
            int nBL, Word_t wRoot);

Status_t
InsertAtBitmap(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot);

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)

Word_t
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot);

Word_t
DeflateExternalList(Word_t *pwRoot,
                    int nPopCnt, int nBL, Word_t *pwr);

#endif // defined(EMBED_KEYS)

// Max list length as a function of nBL.
// Array is indexed by LOG(nBL-1).
const int anListPopCntMax[] = {
                    0, //  1 < nBL <=  2
                    0, //  2 < nBL <=  4
    cnListPopCntMax8 , //  4 < nBL <=  8
    cnListPopCntMax16, //  8 < nBL <= 16
    cnListPopCntMax32, // 16 < nBL <= 32
    cnListPopCntMax64, // 32 < nBL <= 64
    };

#endif // (cwListPopCntMax != 0)

void
HexDump(char *str, Word_t *pw, unsigned nWords)
{
    printf("\n%s (pw %p nWords %d):\n", str, (void *)pw, nWords);
    if (nWords % 8 == 0) {
        for (unsigned ii = 0; ii < nWords; ii += 8) {
            printf(" %016lx %016lx %016lx %016lx",
                   pw[ii], pw[ii+1], pw[ii+2], pw[ii+3]);
            printf(" %016lx %016lx %016lx %016lx\n",
                   pw[ii+4], pw[ii+5], pw[ii+6], pw[ii+7]);
        }
    } else if (nWords % 4 == 0) {
        for (unsigned ii = 0; ii < nWords; ii += 4) {
            printf(" "OWx" "OWx" "OWx" "OWx"\n",
                   pw[ii], pw[ii+1], pw[ii+2], pw[ii+3]);
        }
    } else {
        for (unsigned ii = 0; ii < nWords; ii++) {
            printf(OWx"\n", pw[ii]);
        }
    }
}

// Adjust the tree if necessary following Insert.
// nDL does not include any skip in *pwRoot/wRoot.
static void InsertAll(Word_t *pwRootOld,
                      int nBLOld, Word_t wKey, Word_t *pwRoot, int nBL);
void
InsertCleanup(Word_t wKey, int nBL, Word_t *pwRoot, Word_t wRoot)
{
    int nDL = nBL_to_nDL(nBL);

// Default cnNonBmLeafPopCntMax is 1280.  Keep W/K <= 1.
#if ! defined(cnNonBmLeafPopCntMax)
    #define cnNonBmLeafPopCntMax  0
#endif // ! defined(cnNonBmLeafPopCntMax)

    (void)wKey; (void)nDL; (void)pwRoot; (void)wRoot;
    int nType = wr_nType(wRoot);
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;
    Word_t wPopCnt;
// Default cnBmWpkPercent is 80, create bm at 80% wpk.
#if ! defined(cnBmWpkPercent)
#define cnBmWpkPercent  80
#endif // ! defined(cnBmWpkPercent)
    if ((nBL == nDL_to_nBL(2))
        && tp_bIsSwitch(nType)
        && ! tp_bIsSkip(nType)
#if defined(CODE_BM_SW)
        && ! tp_bIsBmSw(nType)
#endif // defined(CODE_BM_SW)
        && (((wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBL))
                >= (EXP(nBL) * 100 / cnBitsPerWord / cnBmWpkPercent))
            || ((cnNonBmLeafPopCntMax != 0)
                && (wPopCnt > cnNonBmLeafPopCntMax))))
    {
        DBGI(printf("Converting BM leaf.\n"));
        //Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        assert( ! tp_bIsSkip(nType) );
        //printf("wRoot %p wPopCnt %ld\n", (void *)wRoot, wPopCnt);
        DBGI(printf("\n# IC: Creating a bitmap at nBL %d.\n", nBL));

        int nBW;
#if defined(USE_XX_SW)
        assert(tp_bIsXxSw(nType));
        if (tp_bIsXxSw(nType)) { nBW = pwr_nBW(&wRoot); } else
#endif // defined(USE_XX_SW)
        {
            assert(nBL == nDL_to_nBL(nBL_to_nDL(nBL)));
            nBW = nBL_to_nBitsIndexSz(nBL);
        }

        // Allocate a new bitmap.
        DBGI(printf("# IC: NewBitmap nBL %d nBW %d wPopCnt %ld"
                    " wWordsAllocated %ld wPopCntTotal %ld.\n",
                    nBL, nBW, wPopCnt, wWordsAllocated, wPopCntTotal));
        Word_t *pwBitmap = NewBitmap(pwRoot, nBL, nBL, wKey);
        set_w_wPopCntBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, wPopCnt);

        // Why are we not using InsertAll here to insert the keys?
        // It doesn't handle switches yet.

        int nBLLn = nBL - nBW;
        Word_t wBLM = MSK(nBLLn); // Bits left mask.

        for (Word_t ww = 0; ww < EXP(nBW); ww++)
        {
            Word_t *pwRootLn = &pwr_pLinks((Switch_t *)pwr)[ww].ln_wRoot;
            Word_t wRootLn = *pwRootLn;
#if defined(NO_TYPE_IN_XX_SW)
            if (nBLLn < nDL_to_nBL(2)) {
                goto embeddedKeys;
            }
#endif // defined(NO_TYPE_IN_XX_SW)
            int nTypeLn = wr_nType(wRootLn);
            Word_t *pwrLn = wr_pwr(wRootLn);

            if (nTypeLn == T_EMBEDDED_KEYS) {
                goto embeddedKeys;
embeddedKeys:;
                int nPopCntLn = wr_nPopCnt(wRootLn, nBLLn);
                for (int nn = 1; nn <= nPopCntLn; nn++) {
                    SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                           ((wRootLn >> (cnBitsPerWord - (nn * nBLLn)))
                               & wBLM));
                }
            } else if (nTypeLn == T_BITMAP) {
                memcpy(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                       pwrLn, EXP(nBLLn - 3));
                OldBitmap(pwRootLn, pwrLn, nBLLn);
            }
#if (cwListPopCntMax != 0)
            else if (wRootLn != 0) {
                assert(nTypeLn == T_LIST);
#if defined(PP_IN_LINK)
                int nPopCntLn
                      = PWR_wPopCnt(pwRootLn, (Switch_t *)pwrLn, nBLLn);
#else // defined(PP_IN_LINK)
                int nPopCntLn = ls_xPopCnt(pwrLn, nBLLn);
#endif // defined(PP_IN_LINK)
                if (nBLLn <= 8) {
                    uint8_t *pcKeysLn = ls_pcKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (pcKeysLn[nn] & wBLM));
                    }
                } else if (nBLLn <= 16) {
                    uint16_t *psKeysLn = ls_psKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (psKeysLn[nn] & wBLM));
                    }
                } else {
                    assert(nBLLn <= 32);
                    uint32_t *piKeysLn = ls_piKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (piKeysLn[nn] & wBLM));
                    }
                }
            }
#endif // (cwListPopCntMax != 0)
            else {
                // I guess remove can result in a NULL *pwRootLn in a bitmap
                // switch since we don't clean them up at the time.
                DBGI(printf("Null link in bm switch ww %ld.\n", ww));
            }
        }

        assert( ! tp_bIsSkip(nType) ); // How do we ensure this?
        OldSwitch(&wRoot, nBL,
#if defined(CODE_BM_SW)
                  /* bBmSw */ 0,
#endif // defined(CODE_BM_SW)
                  /* nBLUp */ nBL);

#if defined(DEBUG)
        int count = 0;
        for (int jj = 0; jj < (int)EXP(nBL - cnLogBitsPerWord); jj++)
        {
            if (pwBitmap[jj] != 0) {
                DBGI(printf("jj %d pwBitmap[jj] 0x%016lx popcount %d\n",
                            jj, pwBitmap[jj],
                            __builtin_popcountll(pwBitmap[jj])));
            }
            count += __builtin_popcountll(pwBitmap[jj]);
        }
        if (count != (int)wPopCnt) {
            printf("count %d wPopCnt %ld\n", count, wPopCnt);
                    Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord);
        }
        assert(count == (int)wPopCnt);
#endif // defined(DEBUG)
    }
}

#if (cwListPopCntMax != 0)

// Insert each key from pwRootOld into pwRoot.  Then free pwRootOld.
// wKey contains the common prefix.
static void
InsertAll(Word_t *pwRootOld, int nBLOld, Word_t wKey, Word_t *pwRoot, int nBL)
{
    Word_t wRootOld = *pwRootOld;
#if defined(NO_TYPE_IN_XX_SW)
    if (nBLOld < nDL_to_nBL(2)) {
        if (wRootOld == ZERO_POP_MAGIC) { return; }
  #if defined(HANDLE_BLOWOUTS)
        if ((wRootOld & BLOWOUT_MASK(nBLOld)) == ZERO_POP_MAGIC) { return; }
  #endif // defined(HANDLE_BLOWOUTS)
        goto embeddedKeys;
    }
#endif // defined(NO_TYPE_IN_XX_SW)
    if (wRootOld == 0) { return; }
    int nType = wr_nType(wRootOld);
    int nPopCnt;
#if defined(CODE_XX_SW)
    if (nType == T_EMBEDDED_KEYS) {
        goto embeddedKeys;
embeddedKeys:;
        // How inefficient can we be?
        DBGI(printf("IA: Calling IEL nBLOld %d wKey "OWx" nBL %d\n",
                    nBLOld, wKey, nBL));
        wRootOld = InflateEmbeddedList(pwRootOld, wKey, nBLOld, wRootOld);
        DBGI(printf("After IEL\n"));
// If (nBLOld < nDL_to_nBL) Dump is going to think wRootOld is embeddded keys.
        //DBGI(Dump(&wRootOld, wKey & ~MSK(nBLOld), nBLOld));
        nType = wr_nType(wRootOld); // changed by IEL
        assert(nType == T_LIST);
    }
#endif // defined(CODE_XX_SW)

    Word_t *pwrOld = wr_pwr(wRootOld);

#if defined(CODE_XX_SW)
    if (nType == T_ONE) {
        nPopCnt = 1;
        Insert(pwRoot, *pwrOld, nBL);
    } else
#endif // defined(CODE_XX_SW)
    {
        if (nType != T_LIST) {
            printf("nType %d wRootOld "OWx" pwRootOld %p\n",
                   nType, wRootOld, (void *)pwRootOld);
            DBGR(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
        }
        assert(nType == T_LIST); // What about T_ONE?
        nPopCnt = PWR_xListPopCnt(&wRootOld, nBLOld);
        
        int status;
#if defined(COMPRESSED_LISTS)
        if (nBLOld <= (int)sizeof(uint8_t) * 8) {
            uint8_t *pcKeys = ls_pcKeysNAT(pwrOld);
            for (int nn = 0; nn < nPopCnt; nn++) {
                status = Insert(pwRoot, pcKeys[nn] | (wKey & ~MSK(8)), nBL);
            }
        } else if (nBLOld <= (int)sizeof(uint16_t) * 8) {
            uint16_t *psKeys = ls_psKeysNAT(pwrOld);
            for (int nn = 0; nn < nPopCnt; nn++) {
                status = Insert(pwRoot, psKeys[nn] | (wKey & ~MSK(16)), nBL);
            }
#if (cnBitsPerWord > 32)
        } else if (nBLOld <= (int)sizeof(uint32_t) * 8) {
            uint32_t *piKeys = ls_piKeysNAT(pwrOld);
            for (int nn = 0; nn < nPopCnt; nn++) {
                status = Insert(pwRoot, piKeys[nn] | (wKey & ~MSK(32)), nBL);
            }
#endif // (cnBitsPerWord > 32)
        } else
#endif // defined(COMPRESSED_LISTS)
        {
            Word_t *pwKeys = ls_pwKeysNAT(pwrOld);
            for (int nn = 0; nn < nPopCnt; nn++) {
                status = Insert(pwRoot, pwKeys[nn], nBL);
            }
        }
        assert(status == 1);
    }

#if defined(NO_TYPE_IN_XX_SW)
    // OldList uses nType even if (nBL < nDL_to_nBL(2)) implies an
    // embedded list.
#endif // defined(NO_TYPE_IN_XX_SW)
    if (nPopCnt != 0) { OldList(pwrOld, nPopCnt, nBLOld, nType); }
}

#endif // (cwListPopCntMax != 0)

// Handle a prefix mismatch by inserting a switch above and demoting
// the current *pwRoot.
static void
PrefixMismatch(Word_t *pwRoot, int nBLUp, Word_t wKey, int nBLR)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwr = Get_pwr(pwRoot); (void)pwr;
    int nDLUp = nBL_to_nDL(nBLUp);
    int nDLR = nBL_to_nDL(nBLR);

    // Can't have a prefix mismatch if there is no skip.
    assert(nBLR < nBLUp);

    // For PP_IN_LINK, we'll use wPrefix to install the prefix for the
    // old *pwRoot in it's new location.
    Word_t wPrefix;
#if defined(PP_IN_LINK)
    if (nBLUp == cnBitsPerWord) { wPrefix = 0; /* limitation */ } else
#endif // defined(PP_IN_LINK)
#if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
        wPrefix = w_wPrefixBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
    } else
#endif // defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    { wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR); }

    // Figure nBL for the new switch.
    int nDL = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
    int nBL = nDL_to_nBL(nDL);
    // nDL includes the highest order digit that is different.

    assert(nDL > nDLR);
    assert(nBL <= nBLUp);

    Word_t wPopCnt;
#if defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
    if (nBLUp == cnBitsPerWord) { wPopCnt = wPopCntTotal; /* hack */ } else
#endif // defined(PP_IN_LINK) && ! defined(NO_SKIP_AT_TOP)
    {
#if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
        if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
            wPopCnt = w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
        } else
#endif // defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
        { wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR); }
        if (wPopCnt == 0) {
            // full pop
            wPopCnt = wPrefixPopMaskBL(nBLR) + 1;
        }
    }

#if defined(NO_SKIP_AT_TOP)
    // no skip link at top => no prefix mismatch at top
    assert(nBLUp < cnBitsPerWord);
#endif // defined(NO_SKIP_AT_TOP)

    // todo nBitsIndexSz; wide switch
    int nIndex = (wPrefix >> nDL_to_nBL_NAT(nDL - 1))
               & (EXP(nDL_to_nBitsIndexSz(nDL)) - 1);
    // nIndex is the logical index in new switch.
    // It may not be the same as the index in the old switch.

#if defined(CODE_BM_SW)

    // set bBmSwNew
  #if defined(USE_BM_SW)
      #if defined(SKIP_TO_BM_SW)
  #if defined(BM_IN_LINK)
    int bBmSwNew = (nDLUp != cnDigitsPerWord);
  #else // defined(BM_IN_LINK)
    int bBmSwNew = 1;
  #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
  #if defined(BM_IN_LINK)
    int bBmSwNew = ((nDLUp != cnDigitsPerWord) && (nDL == nDLUp));
  #else // defined(BM_IN_LINK)
    int bBmSwNew = (nDL == nDLUp);
  #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
    int bBmSwNew = 0;
  #endif // defined(USE_BM_SW)

    // set bBmSwOld
  #if defined(SKIP_TO_BM_SW)
    int bBmSwOld = tp_bIsBmSw(wr_nType(wRoot)); (void)bBmSwOld;
  #elif defined(BM_IN_LINK)
    int bBmSwOld = 0; // no skip link to bm switch
  #endif // defined(SKIP_TO_BM_SW)

#endif // defined(CODE_BM_SW)

#if defined(BM_IN_LINK)
#if defined(CODE_BM_SW)
    Link_t ln;
    Word_t wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDLR));
    if (bBmSwOld)
    {
    // Save the old bitmap before it is trashed by NewSwitch.
    // Is it possible that nDLUp != cnDigitsPerWord and
    // we are at the top?
    if (nDLUp != cnDigitsPerWord)
    {
        memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL),
               DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
#if ! defined(BM_SW_FOR_REAL)
        assert((wIndexCnt < cnBitsPerWord)
            || (ln.ln_awBm[0] == (Word_t)-1));
#endif // ! defined(BM_SW_FOR_REAL)
    }
    }
#endif // defined(CODE_BM_SW)
#endif // defined(BM_IN_LINK)

    Word_t *pwSw;
    // initialize prefix/pop for new switch
    // Make sure to pass the right key for BM_SW_FOR_REAL.
    DBGI(printf("IG: nDL %d nDLUp %d\n", nDL, nDLUp));
    assert(nBL <= nBLUp);
    // NewSwitch changes *pwRoot (and the link containing it).
    // It does not change our local wRoot and pwr (or pwRoot).
    pwSw = NewSwitch(pwRoot, wPrefix, nBL,
#if defined(CODE_XX_SW)
                     nBL_to_nBitsIndexSz(nBL),
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
                     bBmSwNew,
#endif // defined(CODE_BM_SW)
                     nBLUp, wPopCnt);
    //DBGI(HexDump("After NewSwitch", pwSw, EXP(cnBitsPerDigit) + 1));
    DBGI(printf("Just after InsertGuts calls NewSwitch"
                " for prefix mismatch.\n"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

#if defined(CODE_BM_SW)
    if (bBmSwNew)
    {
#if defined(BM_SW_FOR_REAL)
#if defined(BM_IN_LINK)
    if (nDLUp != cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
    {
        // Switch was created with only one link based on wKey
        // passed in.  Unless BM_IN_LINK && switch is at top.
        nIndex = 0;
    }
#endif // defined(BM_SW_FOR_REAL)
    }

#if defined(BM_IN_LINK)
    if (bBmSwOld)
    {
    if (nDLUp != cnDigitsPerWord)
    {
        // Copy bitmap from old link to new link.
        memcpy(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm,
               ln.ln_awBm,
               DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
    }
    else
    {
        // Initialize bitmap in new link.
        // Mind the high-order bits of the bitmap word if/when the
        // bitmap is smaller than a whole word.
        // Mind endianness.
        if (wIndexCnt < cnBitsPerWord)
        {
            *pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm
                = EXP(wIndexCnt) - 1;
        }
        else
        {
            memset(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm, -1,
                   DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
        }
    }
    }
#endif // defined(BM_IN_LINK)

#endif // defined(CODE_BM_SW)

    // Initialize the link in the new switch that points to the
    // old *pwRoot.
#if defined(TYPE_IS_RELATIVE)
    // Update type field in wRoot that points to old switch since
    // it is not skipping as many digits now.
    DBGI(printf("nDL %d nDLR %d nDLU %d\n",
           nDL, nDLR, nDLUp));
    if (nDL - nDLR - 1 == 0) {
        Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
    } else {
// set_wr_nDS should preserve the type instead of overwriting it?
        set_wr_nDS(wRoot, nDL - nDLR - 1); // type = T_SKIP_TO_SWITCH
  #if defined(SKIP_TO_BM_SW)
        if (bBmSwOld) { set_wr_nType(wRoot, T_SKIP_TO_BM_SW); }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(CODE_XX_SW) && defined(SKIP_TO_XX_SW)
        if (tp_bIsXxSw(wr_nType(wRoot))) { Set_nType(&wRoot, T_SKIP_TO_XX_SW); }
  #endif // defined(CODE_XX_SW) && defined(SKIP_TO_XX_SW)
    }
#else // defined(TYPE_IS_RELATIVE)
    if (nDL - nDLR - 1 == 0) {
        Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
    }
#endif // defined(TYPE_IS_RELATIVE)
    // Copy wRoot from old link (after being updated) to new link.
#if defined(CODE_BM_SW)
    if (bBmSwNew) {
        pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
    } else
#endif // defined(CODE_BM_SW)
    {
        pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
    }

    // Set prefix and pop count for the old switch if necessary.
    // It's only necessary for PP_IN_LINK because in that case
    // they have a new location now.  Their old location is used
    // by the new switch and has been overwritten.
#if defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
    if (nDLR == nDL - 1)
    {
        // The previously necessary prefix in the old switch
        // is now unnecessary.
        DBGI(printf("nDLR %d nDL %d\n",
                    nDLR, nDL));
    }
    else
#endif // defined(NO_UNNECESSARY_PREFIX)
    {
#if defined(CODE_BM_SW)
        set_PWR_wPrefix(
            bBmSwNew
                ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                  &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
            NULL, nDLR, wPrefix);
#else // defined(CODE_BM_SW)
        set_PWR_wPrefix(
            &pwr_pLinks((Switch_t *)pwSw)[nIndex].ln_wRoot,
            NULL, nDLR, wPrefix);
#endif // defined(CODE_BM_SW)
    }

#if defined(CODE_BM_SW)
    set_PWR_wPopCnt(
            bBmSwNew
                ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                  &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
            (Switch_t *)NULL, nDLR, wPopCnt);
#else // defined(CODE_BM_SW)
    set_PWR_wPopCnt(&pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
            (Switch_t *)NULL, nDLR, wPopCnt);
#endif // defined(CODE_BM_SW)
#else // defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
   // We could go to the trouble of zeroing the no-longer necessary
   // prefix in the old switch. 
#endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)
    DBGI(printf("Just before InsertGuts calls Insert"
                " for prefix mismatch.\n"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    Insert(pwRoot, wKey, nDL_to_nBL(nDLUp));
}

// InsertGuts
// This function is called from the iterative Insert function once Insert has
// determined that the key from an insert request is not present.
// It is provided with a starting pwRoot for the insert.
// InsertGuts does whatever is necessary to insert the key into the array
// and returns back to Insert.
// InsertGuts is where the main runtime policy decisions are made.
// Some are made in RemoveGuts, but those are closely aligned with
// the decisions made here.
// Do we create a list as high as possible or as low as possible?
// When do we create a new switch instead of adding to a list?
// When do we create a bitmap?
// When do we uncompress switches?
// When do we coalesce switches?
Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot
#if defined(CODE_XX_SW)
           , Word_t *pwRootPrev
#endif // defined(CODE_XX_SW)
           )
{
#if defined(CODE_XX_SW)
    (void)pwRootPrev;
    int nBW; (void)nBW;
#endif // defined(CODE_XX_SW)
    int nDL = nBL_to_nDL(nBL);
    assert(nDL_to_nBL(nDL) >= nBL);
    DBGI(printf("InsertGuts pwRoot %p wKey "OWx" nBL %d wRoot "OWx"\n",
                (void *)pwRoot, wKey, nBL, wRoot));

#if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1);
#endif // ! defined(USE_XX_SW)

#if defined(USE_T_ONE)
#if defined(COMPRESSED_LISTS)
    uint8_t  cKey;
    uint16_t sKey;
#if (cnBitsPerWord > 32)
    uint32_t iKey;
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
#endif // defined(USE_T_ONE)

    // Would be nice to validate sanity of ifdefs here.  Or somewhere.
    // assert(cnBitsInD1 >= cnLogBitsPerWord);
#if defined(EMBED_KEYS) && ! defined(USE_T_ONE)
    assert(0); // EMBED_KEYS requires USE_T_ONE
#endif // defined(EMBED_KEYS) && ! defined(USE_T_ONE)

#if defined(NO_TYPE_IN_XX_SW)
    if (pwRootPrev != NULL) {
        DBGR(printf("IG: goto embeddedKeys.\n"));
        assert(tp_bIsXxSw(wr_nType(*pwRootPrev)));
        goto embeddedKeys;
    }
#endif // defined(NO_TYPE_IN_XX_SW)

    // Check to see if we're at the bottom before checking nType since
    // nType may be invalid if wRoot is an embedded bitmap.
    // The first test can be done at compile time and might make the
    // InsertAtDl1 go away.
    if ((EXP(cnBitsInD1) <= sizeof(Link_t) * 8) && (nBL == cnBitsInD1)) {
        return InsertAtDl1(pwRoot, wKey, nDL, nBL, wRoot);
    }

    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc

    if ((nType == T_BITMAP)
#if defined(SKIP_TO_BITMAP)
        || (nType == T_SKIP_TO_BITMAP)
#endif // defined(SKIP_TO_BITMAP)
        || 0)
    {
        return InsertAtBitmap(pwRoot, wKey, nDL, wRoot);
    }

    // Can the following be moved into the if ! switch block?
#if (cwListPopCntMax != 0)
#if defined(EMBED_KEYS)
    // Change an embedded list into an external list to make things
    // easier for Insert.  We'll change it back later if it makes sense.
    // We used to use T_ONE for a single embedded key.  But not anymore.
    if (nType == T_EMBEDDED_KEYS) {
        goto embeddedKeys;
embeddedKeys:;

          #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
            #if ! defined(PACK_KEYS_RIGHT)
                //#if ! defined(FILL_WITH_ONES)
        if (wr_nPopCnt(*pwRoot, nBL) < EmbeddedListPopCntMax(nBL)) {
            InsertEmbedded(pwRoot, nBL, wKey); return Success;
        }
                //#endif // ! defined(FILL_WITH_ONES)
            #endif // ! defined(PACK_KEYS_RIGHT)
          #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)

        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        // BUG: The list may not be sorted at this point.  Does it matter?
        // Update: I'm not sure why I wrote that the list may not be sorted
        // at this point.  I can't think of why it would not be sorted.
        // Is it related to SEARCH_FROM_WRAPPER?
        nType = wr_nType(wRoot);
        DBGI(printf("IG: wRoot "OWx" nType %d\n", wRoot, nType));
    }
#endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

    Word_t *pwr = wr_pwr(wRoot);

// This first clause handles wRoot == 0 by treating it like a list leaf
// with zero population (and no allocated memory).
// But why is it ok to skip the test for a switch if ! defined(SKIP_LINKS)
// and !defined(BM_SW_FOR_REAL)?
// InsertGuts is called with a wRoot
// that points to a switch only for prefix mismatch or missing link cases.
#if defined(SKIP_LINKS) 
    if (!tp_bIsSwitch(nType))
#else // defined(SKIP_LINKS)
#if defined(BM_SW_FOR_REAL)
#if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType))
#else // (cwListPopCntMax == 0)
    if (pwr == NULL)
#endif // (cwListPopCntMax == 0)
#endif // defined(BM_SW_FOR_REAL)
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

        // Initialize wPopCnt, pwKeys, piKeys, psKeys and pcKeys for copy.
        // And set prefix in link if PP_IN_LINK and the list is empty and
        // we're not at the top.
        if (wRoot != 0) // pointer to old List
        {
            assert(nType != T_EMBEDDED_KEYS);
#if defined(USE_T_ONE)
            if (nType == T_ONE)
            {
                wPopCnt = 1;
  #if defined(PP_IN_LINK)
                // pop count in link should have been bumped by now
                // if we're not at the top
                // IEL does not affect it.
                assert((nDL == cnDigitsPerWord)
                    || (PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                        == wPopCnt + 1));
  #endif // defined(PP_IN_LINK)
                pwKeys = pwr;
                // can we really not just do pxKeys = pwr?
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                iKey = (uint32_t)*pwr; piKeys = &iKey;
#endif // (cnBitsPerWord > 32)
                sKey = (uint16_t)*pwr; psKeys = &sKey;
                cKey = (uint8_t)*pwr; pcKeys = &cKey;
#endif // defined(COMPRESSED_LISTS)
            }
            else
#endif // defined(USE_T_ONE)
            {
#if defined(PP_IN_LINK)
                if (nDL != cnDigitsPerWord) {
// Why are we subracting one here?
// Is it because Insert bumps pop count before calling InsertGuts?
// How is this handled with InflateEmbeddedList?
// ln_wPrefixPop is not affected by InflateEmbeddedList?
                    wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL) - 1;
                    pwKeys = ls_pwKeysNAT(pwr); // list of keys in old List
                } else {
                    wPopCnt = ls_xPopCnt(pwr, nBL);
                    pwKeys = ls_pwKeys(pwr, cnBitsPerWord);
                }
#else // defined(PP_IN_LINK)
                wPopCnt = ls_xPopCnt(pwr, nBL);
                pwKeys = ls_pwKeysNAT(pwr); // list of keys in old List
#endif // defined(PP_IN_LINK)
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
                piKeys = ls_piKeysNAT(pwr);
#endif // (cnBitsPerWord > 32)
                psKeys = ls_psKeysNAT(pwr);
                pcKeys = ls_pcKeysNAT(pwr);
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

        int nDLOld = nDL; (void)nDLOld;
        int nBLOld = nBL; (void)nBLOld;

#if (cwListPopCntMax != 0) // true if we are using lists; embedded or external

  #if defined(EMBED_KEYS)
      #if ! defined(POP_CNT_MAX_IS_KING) || defined(CODE_XX_SW)
        // It makes no sense to impose a pop limit that is less than what
        // will fit as embedded keys.  If we want to be able to do that for
        // running experiments, then we can use POP_CNT_MAX_IS_KING.
        int nEmbeddedListPopCntMax = EmbeddedListPopCntMax(nBL);
      #endif // ! defined(POP_CNT_MAX_IS_KING) || defined(CODE_XX_SW)
  #endif // defined(EMBED_KEYS)
  #if defined(NO_TYPE_IN_XX_SW)
        if ((nBL < nDL_to_nBL(2))
            && (wPopCnt == (Word_t)nEmbeddedListPopCntMax))
        {
            DBGR(printf("IG: goto doubleIt nBL %d cnt %d max %d.\n",
                        nBL, (int)wPopCnt, nEmbeddedListPopCntMax));
            goto doubleIt;
        }
  #endif // defined(NO_TYPE_IN_XX_SW)

        if (0
#if defined(EMBED_KEYS) && ! defined(POP_CNT_MAX_IS_KING)
            || (wPopCnt < (Word_t)nEmbeddedListPopCntMax)
#endif // defined(EMBED_KEYS) && ! defined(POP_CNT_MAX_IS_KING)
            || ((nBL == cnBitsInD1)
                && ((int)wPopCnt < (int)cnListPopCntMaxDl1))
#if defined(cnListPopCntMaxDl2)
            || ((nBL == cnBitsLeftAtDl2)
                && ((int)wPopCnt < (int)cnListPopCntMaxDl2))
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
            || ((nBL == cnBitsLeftAtDl3) && (wPopCnt < cnListPopCntMaxDl3))
#endif // defined(cnListPopCntMaxDl3)
            || ((nBL != cnBitsInD1)
#if defined(cnListPopCntMaxDl2)
                && (nBL != cnBitsLeftAtDl2)
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
                && (nBL != cnBitsLeftAtDl3)
#endif // defined(cnListPopCntMaxDl3)
                && ((int)wPopCnt < anListPopCntMax[LOG(nBL - 1)])))
        {
#if defined(CODE_XX_SW)
#if ! defined(cnXxSwWpkPercent)
#define cnXxSwWpkPercent  150
#endif // ! defined(cnXxSwWpkPercent)
            // This block is a performance/efficiency optimization.
            // It is not necessary for "correct" behavior.
            // It is only relevant if ifdefs allow for a blow-out.
            // This is possible if ! defined(NO_TYPE_IN_XX_SW).
            // We haven't written the blow-up creation code for
            // NO_TYPE_IN_XX_SW yet.
            if (nBL == nDL_to_nBL(2)) {
                if ((int)wPopCnt >= nEmbeddedListPopCntMax) {
                    if ((wWordsAllocated * 100 / wPopCntTotal)
                            < cnXxSwWpkPercent)
                    {
                        goto newSwitch;
                    }
                }
            }
            if (nBL < nDL_to_nBL(2)) {
                if ((int)wPopCnt >= nEmbeddedListPopCntMax) {
                    if ((wWordsAllocated * 100 / wPopCntTotal)
                            < cnXxSwWpkPercent)
                    {
                        goto doubleIt;
                    }
                }
            }
#endif // defined(CODE_XX_SW)

            Word_t *pwList;

            // Allocate memory for a new list if necessary.
            // Init or update pop count if necessary.
            if ((pwr == NULL)
                // Inflate uses LWTL, but we don't call Inflate for T_ONE.
                || (nType == T_ONE)
                || (ListWordsTypeList(wPopCnt + 1, nBL)
                        != ListWordsTypeList(wPopCnt, nBL)))
            {
                DBGI(printf("pwr %p wPopCnt %ld nBL %d\n", pwr, wPopCnt, nBL));
                DBGI(printf("nType %d\n", nType));
                DBGI(printf("LWE %d LWE %d\n",
                            ListWordsExternal(wPopCnt + 1, nBL),
                            ListWordsExternal(wPopCnt    , nBL)));
                // Allocate a new list and init pop count if pop count is
                // in the list.  Also init the beginning of the list marker
                // if LIST_END_MARKERS.
                assert(wPopCnt + 1 != 0);
                pwList = NewListTypeList(wPopCnt + 1, nBL);
#if defined(PP_IN_LINK)
                assert((nDL == cnDigitsPerWord)
                    || (PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                            == wPopCnt + 1));
#endif // defined(PP_IN_LINK)
                set_wr(wRoot, pwList, T_LIST);
            }
            else
            {
                pwList = pwr;

#if defined(PP_IN_LINK)
                if (nDL != cnDigitsPerWord) {
                    assert(PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                           == wPopCnt + 1);
                } else
#endif // defined(PP_IN_LINK)
                { set_ls_xPopCnt(pwList, nBL, wPopCnt + 1); }
            }

            set_PWR_xListPopCnt(&wRoot, nBL, wPopCnt + 1);

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    CopyWithInsertChar(ls_pcKeysNAT(pwList),
                        pcKeys, wPopCnt, (unsigned char)wKey);
                } else if (nBL <= 16) {
                    CopyWithInsertShort(ls_psKeysNAT(pwList),
                        psKeys, wPopCnt, (unsigned short)wKey);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    CopyWithInsertInt(ls_piKeysNAT(pwList),
                        piKeys, wPopCnt, (unsigned int)wKey);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    CopyWithInsertWord(ls_pwKeys(pwList, nBL),
                                       pwKeys, wPopCnt, wKey);
                }
            } else
#else // defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    COPY(ls_pcKeysNAT(pwList), pcKeys, wPopCnt);
                } else if (nBL <= 16) {
                    COPY(ls_psKeysNAT(pwList), psKeys, wPopCnt);
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    COPY(ls_piKeysNAT(pwList), piKeys, wPopCnt);
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    COPY(ls_pwKeys(pwList, nBL), pwKeys, wPopCnt);
                }
            }
#endif // defined(SORT_LISTS)
            {
// shared code for (SORT && wPopCnt == 0) and ! SORT
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    ls_pcKeysNAT(pwList)[wPopCnt] = wKey;
                } else if (nBL <= 16) {
                    ls_psKeysNAT(pwList)[wPopCnt] = wKey;
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    { ls_piKeysNAT(pwList)[wPopCnt] = wKey; }
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    ls_pwKeys(pwList, nBL)[wPopCnt] = wKey;
                }
            }

            if ((wPopCnt != 0) && (pwr != pwList))
            {
                OldList(pwr, wPopCnt, nBL, nType);
            }

            // It's a bit ugly that we are installing this T_LIST for
            // if NO_TYPE_IN_XX_SW.  But we know that DEL is going to
            // fix it.  We could give deflate &wRoot, then install the new
            // wRoot ourselves (if DEL returned it).
            *pwRoot = wRoot; // install new

#if defined(EMBED_KEYS)
            // Embed the list if it fits.
            assert(wr_nType(wRoot) == T_LIST);
            if (((int)wPopCnt < EmbeddedListPopCntMax(nBL)) || (wPopCnt == 0))
            {
                DeflateExternalList(pwRoot, wPopCnt + 1, nBL, pwList);
#if defined(NO_TYPE_IN_XX_SW)
                if (!((nBL < nDL_to_nBL(2))
                    || (wr_nType(*pwRoot) == T_EMBEDDED_KEYS)
                    || ((wr_nType(*pwRoot) == T_ONE) && (wPopCnt == 0))))
                {
                    printf("nBL %d wPopCnt %ld nType %d\n",
                           nBL, wPopCnt, wr_nType(*pwRoot));
                }
                assert((nBL < nDL_to_nBL(2))
                    || (wr_nType(*pwRoot) == T_EMBEDDED_KEYS)
                    || ((wr_nType(*pwRoot) == T_ONE) && (wPopCnt == 0)));
#else // defined(NO_TYPE_IN_XX_SW)
                assert((wr_nType(*pwRoot) == T_EMBEDDED_KEYS)
                    || ((wr_nType(*pwRoot) == T_ONE) && (wPopCnt == 0)));
#endif // defined(NO_TYPE_IN_XX_SW)
                DBGR(printf("IG: after DEL *pwRoot "OWx"\n", *pwRoot));
            }
#endif // defined(EMBED_KEYS)
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            // List is full; insert a switch or create a bitmap.
            DBGI(printf("List is full.\n"));
#if defined(SKIP_LINKS)
#if (cwListPopCntMax != 0)
#if    (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) \
    || (cnListPopCntMax16 == 0)
            // Figure out the length of the common prefix of
            // the keys that are in the list and the key that
            // we are inserting.
            if (wPopCnt == 0) {
                // If max list length is zero there are
                // no keys in the list.   We need to jump over
                // dereferencing of the list and skip as far
                // down as possible, i.e. go directly to dl2.
                // We can't skip directly to dl1 since neither
                // bitmap nor list leaf have a prefix.
                if (nDLOld >= 2) {
  #if defined(NO_SKIP_AT_TOP)
                    if (nBLOld != cnBitsPerWord)
  #endif // defined(NO_SKIP_AT_TOP)
                    {
                        nBL = cnBitsLeftAtDl2;
                        nDL = 2;
                    }
                }
                goto newSwitch;
            }
#endif // (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) || ...
#endif // (cwListPopCntMax != 0)
#endif // defined(SKIP_LINKS)

#if defined(CODE_XX_SW)
            // If we are already at dl2, then there
            // is no need to check for a common prefix
            // since we can't skip anyway.  And the
            // skip code causes problems for T_XX_SW.
            if (nBL <= cnBitsLeftAtDl2) {
                goto newSwitch;
            }
#endif // defined(CODE_XX_SW)

#if defined(NO_SKIP_AT_TOP)
            if (nDL < cnDigitsPerWord)
#endif // defined(NO_SKIP_AT_TOP)
#if defined(SKIP_LINKS)
            {
#if defined(COMPRESSED_LISTS)
                Word_t wSuffix; (void)wSuffix;
#endif // defined(COMPRESSED_LISTS)
#if (cwListPopCntMax != 0)
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
                        if (pcKeys[w] < wMin) wMin = pcKeys[w];
                        if (pcKeys[w] > wMax) wMax = pcKeys[w];
                    } else if (nBL <= 16) {
                        if (psKeys[w] < wMin) wMin = psKeys[w];
                        if (psKeys[w] > wMax) wMax = psKeys[w];
#if (cnBitsPerWord > 32)
                    } else if (nBL <= 32) {
                        if (piKeys[w] < wMin) wMin = piKeys[w];
                        if (piKeys[w] > wMax) wMax = piKeys[w];
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
                            LOG((EXP(cnBitsInD1) - 1)
                                    | ((wSuffix ^ wMin)
                                    |  (wSuffix ^ wMax)))
                                + 1);
                }
                else
#endif // defined(COMPRESSED_LISTS)
                {
                    nDL
                        = nBL_to_nDL(
                            LOG((EXP(cnBitsInD1) - 1)
                                    | ((wKey ^ wMin) | (wKey ^ wMax)))
                                + 1);
                }
#else // (cwListPopCntMax != 0)
                // Can't dereference list if there isn't one.
                // Go directly to dl2.
                // Can't skip directly to dl1 since neither bitmap nor
                // list leaf have a prefix.
                if (nDLOld >= 2) {
  #if defined(NO_SKIP_AT_TOP)
                    if (nBLOld != cnBitsPerWord)
  #endif // defined(NO_SKIP_AT_TOP)
                    { nDL = 2; }
                }
#endif // (cwListPopCntMax != 0)
            }
            nBL = nDL_to_nBL(nDL);
#if defined(CODE_XX_SW)
            if (nBL > nBLOld) { nBL = nBLOld; }
            assert(nBL <= nBLOld);
#endif // defined(CODE_XX_SW)

#if ((cwListPopCntMax != 0) \
          && (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) \
                                      || (cnListPopCntMax16 == 0)) \
      || defined(CODE_XX_SW)
newSwitch:
#endif // ((cwListPopCntMax != 0) && ... ) || ...

            // Apply constraints that cause us to create the new switch
            // at a higher level than would be required if only the common
            // prefix were considered.

            // We don't create a switch below nDL == 2.
            // Nor do we create a switch at or below nBL == cnLogBitsPerWord.
            // The latter is enforced by disallowing
            // cnBitsAtDl2 <= cnLogBitsPerWord no later than Initialize time.
            // Nor do we support a skip link directly to a bitmap -- yet.
#if defined(USE_XX_SW) && ! defined(SKIP_TO_XX_SW)
            // We don't skip to a switch below DL3.  Because we don't support
            // skip to T_XX_SW yet and T_XX_SW is critically important at
            // DL2 and below.
            if ((nBL < cnBitsLeftAtDl3) && (nBLOld >= cnBitsLeftAtDl3)) {
                DBGI(printf("InsertGuts nDL %d nBL %d nDLOld %d nBLOld %d\n",
                           nDL, nBL, nDLOld, nBLOld));
                nBL = cnBitsLeftAtDl3;
                nDL = 3;
            }
#else // defined(USE_XX_SW) && ! defined(SKIP_TO_XX_SW)
            if ((nBL < cnBitsLeftAtDl2) && (nBLOld >= cnBitsLeftAtDl2)) {
                DBGI(printf("InsertGuts nDL %d nBL %d nDLOld %d nBLOld %d\n",
                           nDL, nBL, nDLOld, nBLOld));
                nBL = cnBitsLeftAtDl2;
                nDL = 2;
            }
#endif // defined(USE_XX_SW) && ! defined(SKIP_TO_XX_SW)
            assert(nBL > (int)LOG(sizeof(Link_t) * 8));

#if defined(PP_IN_LINK)
            // PP_IN_LINK can only support skip from top for wPrefix == 0.
            if (nBLOld == cnBitsPerWord) {
                while ((nBL != nBLOld) && ((wKey >> nBL) != 0)) {
                    nDL += 1;
                    nBL = nDL_to_nBL(nDL);
                    DBGI(printf("Bumping PP_IN_LINK skip at top nDLR"
                                    " from %d to %d.\n",
                                nDL - 1, nDL));
                }
            }
#endif // defined(PP_IN_LINK)
#if ! defined(DEPTH_IN_SW) && ! defined(LVL_IN_WR_HB)
// Depth is in type.
            if (nDL != nDLOld) {
#if defined(TYPE_IS_RELATIVE)
                // cnMallocMask is the largest nType value
                // if we're storing it in wRoot.
                // How do we handle this for ! defined(TYPE_IS_RELATIVE)?
                // We can't unless defined(DEPTH_IN_SW).  So we have an
                // assertion in Initialize().
                if (nDS_to_tp(nDLOld - nDL) > (int)cnMallocMask) {
                    DBGI(printf("# Oops.  Trimming nDS to cnMallocMask\n"));
                    nDL = nDLOld - tp_to_nDS(cnMallocMask);
                    nBL = nDL_to_nBL(nDL);
                    //assert(0);
                }
#else // defined(TYPE_IS_RELATIVE)
                if (nDL_to_tp(nDL) > (int)cnMallocMask) {
                    printf("# Oops. Can't encode absolute level for skip.\n");
                    printf("nDL %d nDLOld %d\n", nDL, nDLOld);
                    nDL = nDLOld - 1;
                    nBL = nDL_to_nBL(nDL);
                    assert(0);
                }
#endif // defined(TYPE_IS_RELATIVE)
            }
#endif // ! defined(DEPTH_IN_SW) && ! defined(LVL_IN_WR_HB)
#else // defined(SKIP_LINKS)
            // I don't remember why this assertion was here.
            // But it blows and the code seems to do ok with it
            // commented out.
            // assert(nDL > 1);
#endif // defined(SKIP_LINKS)

#if ! defined(USE_XX_SW)
            if ((EXP(cnBitsInD1) > sizeof(Link_t) * 8) && (nDL == 1)) {
                assert(nBLOld == nBL);
                NewBitmap(pwRoot, nBL, nBLOld, wKey);
#if defined(PP_IN_LINK)
                set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL, 0);
#endif // defined(PP_IN_LINK)
            }
            else
#endif // ! defined(USE_XX_SW)
            {
                // NewSwitch overwrites *pwRoot which would be a problem for
                // embedded keys.
                // Unless we've inflated them out.  Which we have.

#if defined(CODE_XX_SW)
                if (nBL != nDL_to_nBL(nBL_to_nDL(nBL))) {
                    DBGI(printf("\n# Blow up nBL %d wPopCnt %ld\n",
                                nBL,
                                PWR_wPopCntBL(NULL,
                                              (Switch_t *)wr_pwr(*pwRootPrev),
                                              16)));
                }

  #if defined(USE_XX_SW)
                if (1 && (nBL == nDL_to_nBL(2)) // Use XX_SW at DL2.
      #if ! defined(SKIP_TO_XX_SW)
                      && (nBL == nBLOld)
      #endif // ! defined(SKIP_TO_XX_SW)
                    )
                {
                    DBGI(printf("# Creating T_XX_SW wKey "OWx" nBL %d\n",
                                wKey, nBL));
      #if defined(SKIP_TO_XX_SW)
                    if (nBLOld != nBL) {
                        DBGR(printf("Skip to T_XX_SW nBLOld %d\n", nBLOld));
                    }
      #endif // defined(SKIP_TO_XX_SW)
                    nBW = cnBW;
                } else if (pwRootPrev != NULL) {
// Shouldn't we think about some other option here?
// What about a small bitmap?
// Or another switch?
                    goto doubleIt;
doubleIt:;
                    assert(nBL < nDL_to_nBL(2));
// Hmm.  *pwRoot has not been updated with the inflated list.
// What should we do?  Call OldList or install the inflated list?
// I think we are going to just inflate it again if we don't just leave it.
// So let's try installing it.
#if defined(NO_TYPE_IN_XX_SW)
                    DBGR(printf("IG: free inflated list.\n"));
                    assert(wr_nType(wRoot) == T_LIST);
                    OldList(wr_pwr(wRoot), wPopCnt, nBL, T_LIST);
#else // defined(NO_TYPE_IN_XX_SW)
                    assert(wr_nType(*pwRoot) != T_ONE);
                    if (wr_nType(*pwRoot) == T_EMBEDDED_KEYS) {
                        assert(wr_nType(wRoot) == T_LIST);
                        *pwRoot = wRoot;
                    }
#endif // defined(NO_TYPE_IN_XX_SW)
                    pwRoot = pwRootPrev;
                    wRoot = *pwRoot;
                    nType = wr_nType(wRoot);
                    assert(tp_bIsXxSw(nType));
                    pwr = wr_pwr(wRoot);
                    // parent is XX_SW; back up and replace it
                    nDL = 2;
                    nBL = nDL_to_nBL(nDL);
      #if defined(SKIP_TO_XX_SW)
                    if (tp_bIsSkip(nType)) {
                        nBLOld = cnBitsPerWord; // Only for skip detection?
                        assert(nBLOld > nBL);
                        nDLOld = nBL_to_nDL(nBLOld);
                    } else
      #endif // defined(SKIP_TO_XX_SW)
                    {
                        nBLOld = nBL;
                        nDLOld = nDL;
                    }
                    nBW = pwr_nBW(&wRoot);
                    DBGI(printf("# Double nBL %d from nBW %d.\n", nBL, nBW));
                    assert(nBL > (int)LOG(sizeof(Link_t) * 8));
                    nBW += cnBWIncr;
                    if (nBL - nBW <= (int)LOG(sizeof(Link_t) * 8)) {
// Doubling here would use at least as much memory as a big bitmap.
// Are we here because the list is full?
// Is it possible we are here because our words/key is good?
                        DBGI(printf("# IG: NewBitmap nBL %d"
                                    " nBLOld %d"
                                    " wWordsAllocated %ld"
                                    " wPopCntTotal %ld.\n",
                                    nBL, nBLOld, wWordsAllocated, wPopCntTotal));
                        DBGI(printf("# IG: NewBitmap wPopCnt %ld.\n",
                                    wPopCnt));
                        DBGI(printf("# IG: NewBitmap nBL %d.\n", nBL));
                        NewBitmap(pwRoot, nBL, nBLOld, wKey);
#if defined(PP_IN_LINK)
                        set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL, 0);
#endif // defined(PP_IN_LINK)
                        DBGI(printf("After NewBitmap; before insertAll.\n"));
                        DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                        goto insertAll;
                    }
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                } else
  #endif // defined(USE_XX_SW)
                { nBW = nBL_to_nBitsIndexSz(nBL); }
#endif // defined(CODE_XX_SW)

#if defined(DEBUG)
                if (nBL > nBLOld) {
                    printf("IG: pwRoot %p wKey "OWx" nBL %d wRoot "OWx"\n",
                           (void *)pwRoot, wKey, nBL, wRoot);
                    printf("nBLOld %d\n", nBLOld);
                }
#endif // defined(DEBUG)
                assert(nBL <= nBLOld);

                NewSwitch(pwRoot, wKey, nBL,
#if defined(CODE_XX_SW)
                          nBW,
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
  #if defined(USE_BM_SW)
      #if defined(USE_XX_SW)
                          (nBL <= nDL_to_nBL(2))
                              ? 0 :
      #endif // defined(USE_XX_SW)
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                          /* bBmSw */ nBLOld != cnBitsPerWord,
          #else // defined(BM_IN_LINK)
                          /* bBmSw */ 1,
          #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                          (nBLOld != cnBitsPerWord) && (nBL == nBLOld),
          #else // defined(BM_IN_LINK)
                          /* bBmSw */ nBL == nBLOld,
          #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
                          /* bBmSw */ 0,
  #endif // defined(USE_BM_SW)
#endif // defined(CODE_BM_SW)
                          nBLOld, /* wPopCnt */ 0);

#if defined(CODE_XX_SW)
               if (nBL <= nDL_to_nBL(2)) {
  #if defined(SKIP_TO_XX_SW)
                   if (nBL != nBLOld) {
                       assert(nBL == nDL_to_nBL(nBL_to_nDL(nBL)));
                       assert(GetBLR(pwRoot, nBLOld) == nBL);
                       set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
                       assert(tp_bIsXxSw(wr_nType(*pwRoot)));
                       assert(GetBLR(pwRoot, nBLOld) == nBL);
                   } else
  #endif // defined(SKIP_TO_XX_SW)
                   {
                       if (nBW >= 7) {
                           DBGI(printf("# Setting T_XX_SW nBW %d nBL %d.\n",
                                       nBW, nBL));
                       }
                       set_wr_nType(*pwRoot, T_XX_SW);
                   }
               }
#endif // defined(CODE_XX_SW)

                if (nBL == nBLOld) {
                    DBGI(printf("\n# InsertGuts After NewSwitch Dump\n"));
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                    DBGI(printf("\n"));
                }
            }

            // Now we need to move the keys from the old subtree to the new
            // subtree.

            // NewSwitch changed *pwRoot.
            // But wRoot, nType, pwr, nBL and nBLOld still all apply
            // to the tree whose keys must be reinserted.
#if defined(USE_XX_SW)
            if (pwRoot == pwRootPrev) {
insertAll:;
                // nBW is for the new tree.
                //printf("Calling InsertAll for all links nBW %d\n", nBW);
                //printf("# Old tree:\n");
                //DBG(Dump(&wRoot, wKey & ~MSK(nBLOld), nBLOld));
                int nBLR = nBL - pwr_nBW(&wRoot);
                for (int nIndex = 0;
                         nIndex < (int)EXP(pwr_nBW(&wRoot));
                         nIndex++)
                {
                    //printf("# New tree before IA nIndex %d:\n", nIndex);
                    //DBG(Dump(pwRoot, wKey, nBLOld));
                    InsertAll(&pwr_pLinks((Switch_t *)pwr)[nIndex].ln_wRoot,
                              nBLR,
                              (wKey & ~MSK(nBL)) | (nIndex << nBLR),
                              pwRoot,
// How are we going to get nBLOld from pwRootPrev?
// Do we need it?  We need it for the call back into Insert.
                              nBLOld);
                }

#if ! defined(SKIP_TO_XX_SW)
                assert(nBL == nDL_to_nBL(2));
                assert(nBLOld == nBL);
#endif // ! defined(SKIP_TO_XX_SW)
                OldSwitch(&wRoot, /* nBL */ nBL,
#if defined(CODE_BM_SW)
                          /* bBmSw */ 0,
#endif // defined(CODE_BM_SW)
                          /* nBLUp */ nBLOld);

                //printf("# New tree after InsertAll done looping:\n");
                //DBG(Dump(pwRoot, wKey, nBLOld));

            } else
#endif // defined(USE_XX_SW)
            { InsertAll(&wRoot, nBLOld, wKey, pwRoot, nBLOld); }

            if (nBL == nBLOld) {
                DBGI(printf("Just Before InsertGuts calls final Insert"));
                DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
            }

            Insert(pwRoot, wKey, nBLOld);
        }
    }
#if defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)
    else
    {
        assert(wr_nType(*pwRoot) != T_EMBEDDED_KEYS);
  #if defined(TYPE_IS_RELATIVE)
        int nDLR = ! tp_bIsSkip(nType) ? nDL : nDL - wr_nDS(wRoot);
  #else // defined(TYPE_IS_RELATIVE)
        int nDLR = ! tp_bIsSkip(nType) ? nDL : wr_nDL(wRoot);
  #endif // defined(TYPE_IS_RELATIVE)
        (void)nDLR; // silence the compiler
  #if defined(BM_SW_FOR_REAL)
      #if defined(SKIP_LINKS)
        Word_t wPrefix;
        // Test to see if this is a missing link case.
        // If not, then it is a prefix mismatch case.
        // nDL does not include any skip indicated in nType.
        // If nDS == 0 or nDL == tp_to_nDL, then we know
        // it is a missing link because it can't be a prefix mismatch.
        // Unfortunately, nDS != 0 (or the other) does not imply a prefix
        // mismatch.
        // It's a bit of a bummer that we are doing the prefix check again.
        // But at least we only do it if there is a skip.
        if ((nDLR == nDL)
#if defined(PP_IN_LINK)
            || ((nDLR == cnDigitsPerWord)
                    && ((wPrefix = 0) == w_wPrefixNotAtTop(wKey, nDLR)))
            || ((nDLR != cnDigitsPerWord)
                && ((wPrefix = PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR))
                            == w_wPrefixNotAtTop(wKey, nDLR)))
#else // defined(PP_IN_LINK)
            || ((wPrefix = PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR))
                            == w_wPrefixNotAtTop(wKey, nDLR))
#endif // defined(PP_IN_LINK)
            )
      #endif // defined(SKIP_LINKS)
        {
            // Missing link.
#if defined(EXTRA_TYPES)
            assert((nType == T_BM_SW)
                || (nType == T_BM_SW + EXP(cnBitsMallocMask)));
#else // defined(EXTRA_TYPES)
  #if defined(SKIP_TO_BM_SW)
            assert((nType == T_SKIP_TO_BM_SW) || (nType == T_BM_SW));
  #else // defined(SKIP_TO_BM_SW)
            assert(nType == T_BM_SW);
  #endif // defined(SKIP_TO_BM_SW)
#endif // defined(EXTRA_TYPES)
#if defined(SKIP_LINKS)
            DBGI(printf("wPrefix "OWx" w_wPrefix "OWx" nDLR %d\n",
                        PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR),
                        w_wPrefix(wKey, nDLR), nDLR));
#endif // defined(SKIP_LINKS)
            NewLink(pwRoot, wKey, nDLR, /* nDLUp */ nDL);
            Insert(pwRoot, wKey, nDL_to_nBL(nDL));
        }
#endif // defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
        else
#endif // defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS)
        {
#if 1
            PrefixMismatch(pwRoot, nDL_to_nBL(nDL), wKey, nDL_to_nBL(nDLR));
#else
            // prefix mismatch
            // insert a switch so we can add just one key; seems like a waste
            // A bitmap switch would be great; no reason to consider
            // converting the existing bitmap to a list if a bitmap switch is
            // short.  Huh?

            // Can't have a prefix mismatch if there is no skip.
            assert(nDLR < nDL);

            int nDLUp = nDL;
            int nBLUp = nBL;

            Word_t wPrefix;
#if defined(PP_IN_LINK)
            // We'll use wPrefix to install the prefix for the old switch in
            // it's new location.
            if (nDLUp == cnDigitsPerWord) { wPrefix = 0; } else
#endif // defined(PP_IN_LINK)
            { wPrefix = PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR); }

            // Figure nBL for the new switch.
            nDL = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
            nBL = nDL_to_nBL(nDL);
            // nDL includes the highest order digit that is different.

            assert(nDL > nDLR);
            assert(nBL <= nBLUp);

            Word_t wPopCnt;
#if defined(PP_IN_LINK)
            if (nDLUp == cnDigitsPerWord) {
                wPopCnt = wPopCntTotal; // hack
            } else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDLR);
                if (wPopCnt == 0)
                {
                    // full pop overflow
                    wPopCnt = wPrefixPopMask(nDLR) + 1;
                }
            }

#if defined(NO_SKIP_AT_TOP)
            // no skip link at top => no prefix mismatch at top
            assert(nDLUp < cnDigitsPerWord);
#endif // defined(NO_SKIP_AT_TOP)

            // todo nBitsIndexSz; wide switch
            int nIndex = (wPrefix >> nDL_to_nBL_NAT(nDL - 1))
                       & (EXP(nDL_to_nBitsIndexSz(nDL)) - 1);
            // nIndex is the logical index in new switch.
            // It may not be the same as the index in the old switch.

#if defined(CODE_BM_SW)

            // set bBmSwNew
  #if defined(USE_BM_SW)
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
            int bBmSwNew = (nDLUp != cnDigitsPerWord);
          #else // defined(BM_IN_LINK)
            int bBmSwNew = 1;
          #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
            int bBmSwNew = ((nDLUp != cnDigitsPerWord) && (nDL == nDLUp));
          #else // defined(BM_IN_LINK)
            int bBmSwNew = (nDL == nDLUp);
          #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
            int bBmSwNew = 0;
  #endif // defined(USE_BM_SW)

            // set bBmSwOld
  #if defined(SKIP_TO_BM_SW)
            int bBmSwOld = tp_bIsBmSw(nType);
  #elif defined(BM_IN_LINK)
            int bBmSwOld = 0; // no skip link to bm switch
  #endif // defined(SKIP_TO_BM_SW)

#endif // defined(CODE_BM_SW)

#if defined(BM_IN_LINK)
#if defined(CODE_BM_SW)
            Link_t ln;
            Word_t wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDLR));
            if (bBmSwOld)
            {
            // Save the old bitmap before it is trashed by NewSwitch.
            // Is it possible that nDLUp != cnDigitsPerWord and
            // we are at the top?
            if (nDLUp != cnDigitsPerWord)
            {
                memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL),
                       DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
#if ! defined(BM_SW_FOR_REAL)
                assert((wIndexCnt < cnBitsPerWord)
                    || (ln.ln_awBm[0] == (Word_t)-1));
#endif // ! defined(BM_SW_FOR_REAL)
            }
            }
#endif // defined(CODE_BM_SW)
#endif // defined(BM_IN_LINK)

            Word_t *pwSw;
            // initialize prefix/pop for new switch
            // Make sure to pass the right key for BM_SW_FOR_REAL.
            DBGI(printf("IG: nDL %d nDLUp %d\n", nDL, nDLUp));
            assert(nBL <= nBLUp);
            // NewSwitch changes *pwRoot (and the link containing it).
            // It does not change our local wRoot and pwr (or pwRoot).
            pwSw = NewSwitch(pwRoot, wPrefix, nBL,
#if defined(CODE_XX_SW)
                             nBL_to_nBitsIndexSz(nBL),
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
                             bBmSwNew,
#endif // defined(CODE_BM_SW)
                             nBLUp, wPopCnt);
            //DBGI(HexDump("After NewSwitch", pwSw, EXP(cnBitsPerDigit) + 1));
            DBGI(printf("Just after InsertGuts calls NewSwitch"
                        " for prefix mismatch.\n"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

#if defined(CODE_BM_SW)
            if (bBmSwNew)
            {
#if defined(BM_SW_FOR_REAL)
#if defined(BM_IN_LINK)
            if (nDLUp != cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                // Switch was created with only one link based on wKey
                // passed in.  Unless BM_IN_LINK && switch is at top.
                nIndex = 0;
            }
#endif // defined(BM_SW_FOR_REAL)
            }

#if defined(BM_IN_LINK)
            if (bBmSwOld)
            {
            if (nDLUp != cnDigitsPerWord)
            {
                // Copy bitmap from old link to new link.
                memcpy(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm,
                       ln.ln_awBm,
                       DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
            }
            else
            {
                // Initialize bitmap in new link.
                // Mind the high-order bits of the bitmap word if/when the
                // bitmap is smaller than a whole word.
                // Mind endianness.
                if (wIndexCnt < cnBitsPerWord)
                {
                    *pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm
                        = EXP(wIndexCnt) - 1;
                }
                else
                {
                    memset(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm, -1,
                           DIV_UP(wIndexCnt, cnBitsPerWord) * sizeof(Word_t));
                }
            }
            }
#endif // defined(BM_IN_LINK)

#endif // defined(CODE_BM_SW)

            // Initialize the link in the new switch that points to the
            // old switch.
#if defined(TYPE_IS_RELATIVE)
            // Update type field in wRoot that points to old switch since
            // it is not skipping as many digits now.
            DBGI(printf("nDL %d nDLR %d nDLU %d\n",
                   nDL, nDLR, nDLUp));
            if (nDL - nDLR - 1 == 0) {
                Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
            } else {
// set_wr_nDS should preserve the type instead of overwriting it?
                set_wr_nDS(wRoot, nDL - nDLR - 1); // type = T_SKIP_TO_SWITCH
  #if defined(SKIP_TO_BM_SW)
                if (bBmSwOld) { set_wr_nType(wRoot, T_SKIP_TO_BM_SW); }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(CODE_XX_SW) && defined(SKIP_TO_XX_SW)
                if (tp_bIsXxSw(nType)) { Set_nType(&wRoot, T_SKIP_TO_XX_SW); }
  #endif // defined(CODE_XX_SW) && defined(SKIP_TO_XX_SW)
            }
#else // defined(TYPE_IS_RELATIVE)
            if (nDL - nDLR - 1 == 0) {
                Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
            }
#endif // defined(TYPE_IS_RELATIVE)
            // Copy wRoot from old link (after being updated) to new link.
#if defined(CODE_BM_SW)
            if (bBmSwNew) {
                pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
            } else
#endif // defined(CODE_BM_SW)
            {
                pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
            }

            // Set prefix and pop count for the old switch if necessary.
            // It's only necessary for PP_IN_LINK because in that case
            // they have a new location now.  Their old location is used
            // by the new switch and has been overwritten.
#if defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
            if (nDLR == nDL - 1)
            {
                // The previously necessary prefix in the old switch
                // is now unnecessary.
                DBGI(printf("nDLR %d nDL %d\n",
                            nDLR, nDL));
            }
            else
#endif // defined(NO_UNNECESSARY_PREFIX)
            {
#if defined(CODE_BM_SW)
                set_PWR_wPrefix(
                    bBmSwNew
                        ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                          &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLR, wPrefix);
#else // defined(CODE_BM_SW)
                set_PWR_wPrefix(
                    &pwr_pLinks((Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLR, wPrefix);
#endif // defined(CODE_BM_SW)
            }

#if defined(CODE_BM_SW)
            set_PWR_wPopCnt(
                    bBmSwNew
                        ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                          &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    (Switch_t *)NULL, nDLR, wPopCnt);
#else // defined(CODE_BM_SW)
            set_PWR_wPopCnt(&pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    (Switch_t *)NULL, nDLR, wPopCnt);
#endif // defined(CODE_BM_SW)
#else // defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
           // We could go to the trouble of zeroing the no-longer necessary
           // prefix in the old switch. 
#endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)
            DBGI(printf("Just before InsertGuts calls Insert"
                        " for prefix mismatch.\n"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

            Insert(pwRoot, wKey, nDL_to_nBL(nDLUp));
#endif // 1
        }
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)

    return Success;
}

#if (cwListPopCntMax != 0)
#if defined(EMBED_KEYS)

// Replace a wRoot that has embedded keys with an external T_LIST leaf.
// This function never creates a T_ONE.
// It assumes the input is an embedded list and not and external T_ONE.
Word_t
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot)
{
    (void)pwRoot;
    DBGI(printf(
         "InflateEmbeddedList pwRoot %p wKey "OWx" nBL %d wRoot "OWx"\n",
         (void *)pwRoot, wKey, nBL, wRoot));

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        if (wRoot == ZERO_POP_MAGIC) { return 0; }
  #if defined(HANDLE_BLOWOUTS)
        if ((wRoot & BLOWOUT_MASK(nBL)) == ZERO_POP_MAGIC) { return 0; }
  #endif // defined(HANDLE_BLOWOUTS)
    }
#else // defined(NO_TYPE_IN_XX_SW)
    assert(wr_nType(wRoot) == T_EMBEDDED_KEYS);
#endif // defined(NO_TYPE_IN_XX_SW)

    Word_t *pwKeys;
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
    uint32_t *piKeys;
#endif // (cnBitsPerWord > 32)
    uint16_t *psKeys;
    uint8_t  *pcKeys;
#endif // defined(COMPRESSED_LISTS)

    int nPopCnt = wr_nPopCnt(wRoot, nBL);
    DBGI(printf("IEL: nPopCnt %d\n", nPopCnt));
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
#if defined(DEBUG)
    if (nPopCnt > EmbeddedListPopCntMax(nBL))
    {
        printf("IEL: wRoot "OWx" nBL %d nPopCnt %d Max %d nBitsPopCntSz %d\n",
               wRoot, nBL, nPopCnt, nPopCntMax, nBL_to_nBitsPopCntSz(nBL));
    }
#endif // defined(DEBUG)
    assert(nPopCnt <= nPopCntMax);

    if (nPopCnt == 0) {
        printf("IEL: wRoot "OWx" nBL %d\n", wRoot, nBL);
    }
    assert(nPopCnt != 0);
    Word_t *pwList = NewListTypeList(nPopCnt, nBL);

    Word_t wBLM = MSK(nBL); // Bits left mask.

    for (int nn = 0; nn < nPopCnt; nn++) {
#if defined(REVERSE_SORT_EMBEDDED_KEYS)
  #if defined(PACK_KEYS_RIGHT)
        int nSlot = (nPopCntMax - nn);
  #else // defined(PACK_KEYS_RIGHT)
        int nSlot = (nPopCnt - nn);
  #endif // defined(PACK_KEYS_RIGHT)
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)
        int nSlot = (nn + 1);
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)
#if defined(COMPRESSED_LISTS)
        if (nBL <= 8) {
            pcKeys = ls_pcKeysNAT(pwList);
            pcKeys[nn] = (uint8_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNAT(pwList);
            psKeys[nn] = (uint16_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
        } else
#if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNAT(pwList);
            piKeys[nn] = (uint32_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
        } else
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        {
#if defined(COMPRESSED_LISTS)
            assert(nPopCnt == 1);
#endif // defined(COMPRESSED_LISTS)
            pwKeys = ls_pwKeysNAT(pwList);
#if defined(PP_IN_LINK)
            assert(nBL != cnBitsPerWord);
#endif // defined(PP_IN_LINK)
            pwKeys[nn] = (wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM);
        }
    }

    wRoot = 0;
    set_wr(wRoot, pwList, T_LIST);

    // Could this be problematic if wRoot is not the only word in the link?
    // We're not replacing pwRoot->ln_wRoot but what about the surroundings?
    set_PWR_xListPopCnt(&wRoot, nBL, nPopCnt);

    return wRoot;
}

// Replace an external T_LIST leaf with a wRoot with embedded keys or
// an external T_ONE leaf.
// The function assumes it is possible.
Word_t
DeflateExternalList(Word_t *pwRoot,
                    int nPopCnt, int nBL, Word_t *pwr)
{
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(T_ONE_CALC_POP)
    assert(0); // not yet
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(T_ONE_CALC_POP)
    int nPopCntMax = EmbeddedListPopCntMax(nBL);
//printf("DEL: nBL %d nPopCntMax %d\n", nBL, nPopCntMax);

    assert(wr_nType(*pwRoot) == T_LIST);

    DBGI(printf("DeflateExternalList pwRoot %p nPopCnt %d nBL %d pwr %p\n",
               (void *)pwRoot, nPopCnt, nBL, (void *)pwr));
    //HexDump("External List", pwr, nPopCnt + 1);

    assert((nPopCnt <= nPopCntMax) || (nPopCnt == 1));

    Word_t wRoot;

    if (nPopCnt <= nPopCntMax)
    {
        wRoot = 0;
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(FILL_WITH_ONES)
        --wRoot;
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(FILL_WITH_ONES)

#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
        uint32_t *piKeys;
#endif // (cnBitsPerWord > 32)
        uint16_t *psKeys;
        uint8_t  *pcKeys;
#endif // defined(COMPRESSED_LISTS)

        assert(nPopCnt != 0);
#if defined(NO_TYPE_IN_XX_SW)
        if (nBL >= nDL_to_nBL(2))
#endif // defined(NO_TYPE_IN_XX_SW)
        { set_wr_nType(wRoot, T_EMBEDDED_KEYS); }
        set_wr_nPopCnt(wRoot, nBL, nPopCnt); // no-op if NO_TYPE_IN_XX_SW
//printf("nBL %d nPopCnt %d wRoot "OWx"\n", nBL, nPopCnt, wRoot);

        for (int nn = 0;
#if defined(FILL_W_KEY)
                 nn < nPopCntMax;
#else // defined(FILL_W_KEY)
                 nn < nPopCnt;
#endif // defined(FILL_W_KEY)
                 nn++)
        {
#if defined(REVERSE_SORT_EMBEDDED_KEYS)
  #if defined(PACK_KEYS_RIGHT)
            int nSlot = (nPopCntMax - nn);
  #else // defined(PACK_KEYS_RIGHT)
            int nSlot = (nPopCnt - nn);
  #endif // defined(PACK_KEYS_RIGHT)
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)
            int nSlot = (nn + 1);
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)
#if defined(COMPRESSED_LISTS)
            if (nBL <= 8) {
                pcKeys = ls_pcKeysNAT(pwr);
                // Uh oh.  Are we really padding with the smallest key?
                // Isn't this contrary to some assumptions in the code?
                // Hmm.  I think we changed it to the smallest key so
                // we could calculate offset of found key using magic.
                SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                        pcKeys[(nn < nPopCnt) ? nn : 0]);
            } else
            if (nBL <= 16) {
                psKeys = ls_psKeysNAT(pwr);
                SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                        psKeys[(nn < nPopCnt) ? nn : 0]);
            } else
#if (cnBitsPerWord > 32)
            if (nBL <= 32) {
                piKeys = ls_piKeysNAT(pwr);
                SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                        piKeys[(nn < nPopCnt) ? nn : 0]);
            } else
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            {
                // I don't think we have to worry about adjusting ls_pwKeys
                // for PP_IN_LINK here since we will not be at the top.
                assert(nBL != cnBitsPerWord);
                Word_t *pwKeys = ls_pwKeys(pwr, nBL);
                SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                        pwKeys[(nn < nPopCnt) ? nn : 0]);
            }
        }
    }
    else
    {
        assert(nPopCnt == 1);
        assert(nPopCntMax == 0);
        Word_t *pwList = NewList(1, nBL);
        wRoot = 0; set_wr(wRoot, pwList, T_ONE); // external T_ONE list
        set_PWR_xListPopCnt(&wRoot, nBL, 1);
        Word_t *pwKeys = ls_pwKeys(pwr, nBL);
        *pwList = pwKeys[0];
    }

    OldList(pwr, nPopCnt, nBL, T_LIST);

    goto done;
done:;

    *pwRoot = wRoot;

    DBGI(printf("DEL wRoot "OWx"\n", wRoot));

    return wRoot;
}

#endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

Status_t
InsertAtDl1(Word_t *pwRoot, Word_t wKey, int nDL,
            int nBL, Word_t wRoot)
{
    (void)nDL; (void)nBL; (void)wRoot;

    assert(EXP(nBL) <= sizeof(Link_t) * 8);
    assert( ! BitIsSet(STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey & MSK(nBL)));

    DBGI(printf("SetBit(pwRoot "OWx" wKey "OWx")\n",
                    (Word_t)pwRoot, wKey & MSK(nBL)));

    SetBit(STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey & MSK(nBL));

#if defined(PP_IN_LINK)

    // What about no_unnecessary_prefix?
    // And is this ever necessary since we don't support skip to bitmap?
    if (EXP(cnBitsInD1) > sizeof(Link_t) * 8) {
        set_PWR_wPrefix(pwRoot, NULL, nDL, wKey);
    }

#endif // defined(PP_IN_LINK)

    return Success;
}

// InsertAtBitmap is for a bitmap that is not at the bottom.
Status_t
InsertAtBitmap(Word_t *pwRoot, Word_t wKey, int nDL, Word_t wRoot)
{
    (void)pwRoot;
    int nBL = nDL_to_nBL(nDL);

#if defined(SKIP_TO_BITMAP)
    if (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP) {
        int nBLR = GetBLR(pwRoot, nBL);
        int nDLR = nBL_to_nDL(nBLR);
        Word_t *pwr = Get_pwr(pwRoot); (void)pwr;

        Word_t wPrefix;
#if defined(PP_IN_LINK)
        if (nBL == cnBitsPerWord) { wPrefix = 0; /* limitation */ } else
#endif // defined(PP_IN_LINK)
#if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
        if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
            wPrefix = w_wPrefixBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)),
                                 nBLR);
        } else
#endif // defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
        { wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR); }

        int bPrefixMismatch;
  #if defined(PP_IN_LINK)
        if (nBL == cnBitsPerWord) {
            // prefix is 0
            bPrefixMismatch = (wKey >= EXP(nBLR));
        } else
  #endif // defined(PP_IN_LINK)
        { bPrefixMismatch = ((int)LOG(1 | (wPrefix ^ wKey)) >= nBLR); }

        if (bPrefixMismatch) {
            PrefixMismatch(pwRoot, nBL, wKey, nBLR);
            return Success;
        }

        nBL = nBLR;
        nDL = nDLR;
    }
#endif // defined(SKIP_TO_BITMAP)

    Word_t *pwr = wr_pwr(wRoot);

    assert(pwr != NULL);

    assert( ! BitIsSet(pwr, wKey & MSK(nBL)) );

    DBGI(printf("SetBit(pwr "OWx" wKey "OWx") pwRoot %p\n",
                (Word_t)pwr, wKey & MSK(nBL), (void *)pwRoot));

    SetBit(pwr, wKey & MSK(nBL));

    set_w_wPopCntBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL,
        w_wPopCntBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL) + 1);

#if defined(PP_IN_LINK)

    // Shouldn't we do this when we create the switch with the link
    // that points to this bitmap rather than on every insert into
    // the bitmap?

    // What about no_unnecessary_prefix?
    set_PWR_wPrefix(pwRoot, NULL, nDL, wKey);

#endif // defined(PP_IN_LINK)

    return Success;
}

Status_t
RemoveBitmap(Word_t *pwRoot, Word_t wKey, int nDL,
             int nBL, Word_t wRoot);

void
RemoveCleanup(Word_t wKey, int nBL, int nBLR, Word_t *pwRoot, Word_t wRoot)
{
    (void)nBL; (void)nBLR;
    int nDL = nBL_to_nDL(nBL);
    int nDLR = nBL_to_nDL(nBLR);
    (void)wKey; (void)nDL; (void)nDLR; (void)pwRoot; (void)wRoot;

    int nType = wr_nType(wRoot); (void)nType;
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;

  #if defined(PP_IN_LINK)
    if (nDL == cnDigitsPerWord) {
        // We don't keep a total pop count for the whole array for
        // PP_IN_LINK.  So we have to accumulate the pop counts of
        // all of the links in the top switch to figure out if we
        // can free the array.  Yuck.
        // Don't we use zero to mean full pop?  How do we know if
        // zero means zero?  Since we just finished a remove we
        // know we can't be at full pop.  This is also why we can't
        // allow an empty node to persist after the remove that
        // made it empty.
        Word_t wIndex = wKey >> (cnBitsPerWord - cnBitsIndexSzAtTop);
        for (Word_t ww = 0; ww < EXP(cnBitsIndexSzAtTop); ww++)
        {
            Word_t *pwRootLn = &((
#if defined(CODE_BM_SW)
                tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
                                      pwr_pLinks((  Switch_t *)pwr))
                    [ww].ln_wRoot);

            int nDLX = wr_bIsSwitch(*pwRootLn)
                            && tp_bIsSkip(wr_nType(*pwRootLn)) ?
      #if defined(TYPE_IS_RELATIVE)
                         nDL - wr_nDS(*pwRootLn)
      #else // defined(TYPE_IS_RELATIVE)
                         wr_nDL(*pwRootLn)
      #endif // defined(TYPE_IS_RELATIVE)
                      : nDL - 1;

            //--nDLX;
            DBGR(printf("wr_nDLX %d", nDLX));
            DBGR(printf(" PWR_wPopCnt %"_fw"d\n",
                        PWR_wPopCnt(pwRootLn, NULL, nDLX)));
            if (((*pwRootLn != 0) && (ww != wIndex))
                    || (PWR_wPopCnt(pwRootLn, (Switch_t *)NULL, nDLX) != 0))
            {
                DBGR(printf("Not empty ww %zd wIndex %zd\n",
                     (size_t)ww, (size_t)wIndex));
                return; // may need cleanup lower; caller checks *pwRoot
            }
        }
        // whole array pop is zero
        FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDL), /* bDump */ 0);
        // caller checks *pwRoot == NULL to see if cleanup is done
    } else
  #endif // defined(PP_IN_LINK)
    {
        Word_t wPopCnt =
#if defined(CODE_BM_SW)
            tp_bIsBmSw(nType) ? PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDLR) :
#endif // defined(CODE_BM_SW)
                                PWR_wPopCnt(pwRoot, (  Switch_t *)pwr, nDLR);

        if (wPopCnt == 0) {
            FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDL), /* bDump */ 0);
        }
    }
}

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot)
{
    // nType is not valid for NO_TYPE_IN_XX_SW and nBL < nDL_to_nBL(2)
    int nType = wr_nType(wRoot); (void)nType;
    // pwr is not valid for NO_TYPE_IN_XX_SW and nBL < nDL_to_nBL(2)
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;
    int nDL = nBL_to_nDL(nBL); (void)nDL;

    DBGR(printf("RemoveGuts\n"));

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        goto embeddedKeys;
    }
#endif // defined(NO_TYPE_IN_XX_SW)

// Could we be more specific in this ifdef, e.g. cnListPopCntMax16?
#if (cwListPopCntMax != 0)
    if ((nBL <= (int)LOG(sizeof(Link_t) * 8))
  #if defined(SKIP_TO_BITMAP)
        || (nType == T_SKIP_TO_BITMAP)
  #endif // defined(SKIP_TO_BITMAP)
        || (nType == T_BITMAP))
#else // (cwListPopCntMax != 0)
    assert((nBL <= (int)LOG(sizeof(Link_t) * 8))
  #if defined(SKIP_TO_BITMAP)
        || (nType == T_SKIP_TO_BITMAP)
  #endif // defined(SKIP_TO_BITMAP)
        || (nType == T_BITMAP));
#endif // (cwListPopCntMax != 0)
    {
        return RemoveBitmap(pwRoot, wKey, nDL, nBL, wRoot);
    }

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)
    if (((nType == T_ONE) || (nType == T_EMBEDDED_KEYS))
// Why is nBL_to_nBitsPopCntSz irrelevant here?
        && (nBL <= cnBitsPerWord - cnBitsMallocMask))
    {
        goto embeddedKeys;
embeddedKeys:;
        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        //*pwRoot = wRoot;
        nType = T_LIST;
        pwr = wr_pwr(wRoot);
        assert(wr_nType(wRoot) == nType);
    }
#endif // defined(EMBED_KEYS)

#if defined(USE_T_ONE)
    if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS)) {
// When do we get here?  Removing last key?  Yes.
        //assert(0);
// Why is nBL_to_nBitsPopCntSz irrelevant here?
        assert(nBL > cnBitsPerWord - cnBitsMallocMask);
        assert(nType == T_ONE);
        OldList(pwr, /* wPopCnt */ 1, nBL, nType);
        *pwRoot = 0; // Do we need to clear the rest of the link also?
        return Success;
    }
#endif // defined(USE_T_ONE)

    assert(wr_nType(wRoot) == T_LIST);
    assert(nType == T_LIST);

    Word_t wPopCnt;

#if defined(PP_IN_LINK)
    if (nDL != cnDigitsPerWord) {
        wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL) + 1;
    } else
#endif // defined(PP_IN_LINK)
    {
        wPopCnt = ls_xPopCnt(pwr, nBL);
    }

// Why was this #if defined(USE_T_ONE) ever here?
//#if ! defined(USE_T_ONE)
    if (wPopCnt == 1) {
        OldList(pwr, wPopCnt, nBL, nType);
#if defined(NO_TYPE_IN_XX_SW)
        if (nBL < nDL_to_nBL(2)) { *pwRoot = ZERO_POP_MAGIC; } else
#endif // defined(NO_TYPE_IN_XX_SW)
        { *pwRoot = 0; }
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }
//#endif // ! defined(USE_T_ONE)

    Word_t *pwKeys = ls_pwKeys(pwr, nBL);

    unsigned nIndex;
    for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
        (nBL <=  8) ? (ls_pcKeysNAT(pwr)[nIndex] != (uint8_t) wKey) :
        (nBL <= 16) ? (ls_psKeysNAT(pwr)[nIndex] != (uint16_t)wKey) :
#if (cnBitsPerWord > 32)
        (nBL <= 32) ? (ls_piKeysNAT(pwr)[nIndex] != (uint32_t)wKey) :
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        (pwKeys[nIndex] != wKey); nIndex++) { }

    // nIndex identifies the key being removed.

    Word_t *pwList;
    if (ListWordsTypeList(wPopCnt - 1, nBL)
        != ListWordsTypeList(wPopCnt, nBL))
    {
        // Malloc a new, smaller list.
        assert(wPopCnt - 1 != 0);
        pwList = NewListTypeList(wPopCnt - 1, nBL);
        // Why are we copying the old list to the new one?
        // Because the beginning will be the same.
        // Except for the the pop count.

        switch (nBytesKeySz(nBL)) {
        case sizeof(Word_t):
             COPY(ls_pwKeys(pwList, nBL), ls_pwKeys(pwr, nBL), wPopCnt - 1);
             break;
#if (cnBitsPerWord > 32)
        case 4:
             COPY(ls_piKeysNAT(pwList), ls_piKeysNAT(pwr), wPopCnt - 1);
             break;
#endif // (cnBitsPerWord > 32)
        case 2:
             COPY(ls_psKeysNAT(pwList), ls_psKeysNAT(pwr), wPopCnt - 1);
             break;
        case 1:
             COPY(ls_psKeysNAT(pwList), ls_psKeysNAT(pwr), wPopCnt - 1);
             break;
        }

        set_wr(wRoot, pwList, T_LIST);
    }
    else
    {
        pwList = pwr;
    }

    set_PWR_xListPopCnt(&wRoot, nBL, wPopCnt - 1);

#if defined(PP_IN_LINK)
    if (nDL == cnDigitsPerWord)
#endif // defined(PP_IN_LINK)
    { set_ls_xPopCnt(pwList, nBL, wPopCnt - 1); }

#if defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
        unsigned nKeys = wPopCnt - 1; (void)nKeys;
#endif // defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        MOVE(&ls_pcKeysNAT(pwList)[nIndex],
             &ls_pcKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#if defined(PSPLIT_PARALLEL)
        // need to pad the list to a word boundary with a key that exists
        // so parallel search won't return a false positive
#if defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_pcKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(__m128i))))
#else // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_pcKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(Word_t))))
#endif // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        {
            ls_pcKeysNAT(pwr)[nKeys] = ls_pcKeysNAT(pwr)[nKeys-1];
            ++nKeys;
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_pcKeysNAT(pwList)[nKeys] = -1;
#endif // defined(LIST_END_MARKERS)
    } else if (nBL <= 16) {
        MOVE(&ls_psKeysNAT(pwList)[nIndex],
             &ls_psKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#if defined(PSPLIT_PARALLEL)
        // need to pad the list to a word boundary with a key that exists
        // so parallel search won't return a false positive
#if defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_psKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(__m128i))))
#else // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_psKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(Word_t))))
#endif // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        {
            ls_psKeysNAT(pwr)[nKeys] = ls_psKeysNAT(pwr)[nKeys-1];
            ++nKeys;
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_psKeysNAT(pwList)[nKeys] = -1;
#endif // defined(LIST_END_MARKERS)
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        MOVE(&ls_piKeysNAT(pwList)[nIndex],
             &ls_piKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
#if defined(PSPLIT_PARALLEL)
        // need to pad the list to a word boundary with a key that exists
        // so parallel search won't return a false positive
#if defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_piKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(__m128i))))
#else // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        while ((Word_t)&ls_piKeysNAT(pwr)[nKeys] & MSK(LOG(sizeof(Word_t))))
#endif // defined(ALIGN_LIST_ENDS) && defined(PARALLEL_128)
        {
            ls_piKeysNAT(pwr)[nKeys] = ls_piKeysNAT(pwr)[nKeys-1];
            ++nKeys;
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_piKeysNAT(pwList)[nKeys] = -1;
#endif // defined(LIST_END_MARKERS)
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
#if defined(LIST_END_MARKERS)
#if defined(PP_IN_LINK) && (cnDummiesInList == 0)
        assert(0);
#endif // defined(PP_IN_LINK) && (cnDummiesInList == 0)
#endif // defined(LIST_END_MARKERS)
        MOVE(&ls_pwKeys(pwList, nBL)[nIndex], &pwKeys[nIndex + 1],
             wPopCnt - nIndex - 1);
#if defined(LIST_END_MARKERS)
        ls_pwKeys(pwList, nBL)[nKeys] = -1;
#endif // defined(LIST_END_MARKERS)
    }

    if (pwList != pwr)
    {
        OldList(pwr, wPopCnt, nBL, nType);
    }

    *pwRoot = wRoot;

#if defined(EMBED_KEYS)
    // Embed the list if it fits.
    assert(wr_nType(wRoot) == T_LIST);
    assert(nType == T_LIST);
    if (((int)wPopCnt <= EmbeddedListPopCntMax(nBL)) || (wPopCnt == 2))
    {
        DeflateExternalList(pwRoot, wPopCnt - 1, nBL, pwList);
    }
#endif // defined(EMBED_KEYS)

    return Success;

#endif // (cwListPopCntMax != 0)

    (void)pwRoot; (void)wKey; (void)nDL; (void)wRoot;
}

// Clear the bit for wKey in the bitmap.
// And free the bitmap if it is empty and not embedded.
Status_t
RemoveBitmap(Word_t *pwRoot, Word_t wKey, int nDL,
             int nBL, Word_t wRoot)
{
    (void)nDL;

    if (EXP(nBL) <= sizeof(Link_t) * 8) {
        ClrBit(STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey & MSK(nBL));
    } else {
  #if defined(SKIP_TO_BITMAP)
        if (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP) {
      #if defined(TYPE_IS_RELATIVE)
            nBL = nDL_to_nBL(nDL - wr_nDS(*pwRoot));
      #else // defined(TYPE_IS_RELATIVE)
            nBL = pwr_nBL(pwRoot);
      #endif // defined(TYPE_IS_RELATIVE)
            nDL = nBL_to_nDL(nBL);
        }
  #endif // defined(SKIP_TO_BITMAP)
        Word_t *pwr = wr_pwr(wRoot);

        ClrBit(pwr, wKey & MSK(nBL));

        set_w_wPopCntBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL,
            w_wPopCntBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL) - 1);

#if defined(PP_IN_LINK)

#if defined(DEBUG)
        Word_t wPopCnt = 0;
        for (Word_t ww = 0; ww < EXP(nBL - cnLogBitsPerWord); ww++) {
            wPopCnt += __builtin_popcountll(pwr[ww]);
        }
        if (wPopCnt != PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)) {
            printf("\nwPopCnt "OWx" PWR_wPopCnt "OWx"\n",
                   wPopCnt, PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL));
            HexDump("Bitmap", pwr, EXP(nBL - cnLogBitsPerWord));
        }
        assert(wPopCnt == PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL));
#endif // defined(DEBUG)

        if (PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL) != 0) {
            return Success; // bitmap is not empty
        }

#else // defined(PP_IN_LINK)

        // Free the bitmap if it is empty.
        for (Word_t ww = 0; ww < EXP(nBL - cnLogBitsPerWord); ww++) {
            if (__builtin_popcountll(pwr[ww])) {
                return Success; // bitmap is not empty
            }
        }

#endif // defined(PP_IN_LINK)

        OldBitmap(pwRoot, pwr, nBL);
    }

    return Success;
}

#endif // (cnDigitsPerWord != 1)

// ***************************************************************************
// JUDY1 FUNCTIONS:

Word_t
Judy1FreeArray(PPvoid_t PPArray, P_JE)
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("Judy1FreeArray\n"));

#if (cnDigitsPerWord != 1)

  #if defined(DEBUG)
    Word_t wMallocsBefore = wMallocs;
    Word_t wEvenMallocsBefore = wEvenMallocs;
    Word_t wWordsAllocatedBefore = wWordsAllocated;
  #endif // defined(DEBUG)

    Word_t wBytes = FreeArrayGuts((Word_t *)PPArray, /* wPrefix */ 0,
                                   cnBitsPerWord, /* bDump */ 0);

    // Should enhance FreeArrayGuts to adjust wPopCntTotal for another
    // sanity check.  But it does not do this now.
    DBG(printf("# wPopCntTotal %ld\n", wPopCntTotal));
    DBG(printf("# wWordsAllocatedBefore %ld\n", wWordsAllocatedBefore));
    DBG(printf("# wMallocsBefore %ld\n", wMallocsBefore));
    DBG(printf("# wEvenMallocsBefore %ld\n", wEvenMallocsBefore));
    DBG(printf("# wWordsAllocated %ld\n", wWordsAllocated));
    DBG(printf("# wMallocs %ld\n", wMallocs));
    DBG(printf("# wEvenMallocs %ld\n", wEvenMallocs));
    DBG(printf("\n"));
    assert((wWordsAllocatedBefore - wWordsAllocated)
               == (wBytes / sizeof(Word_t)));
    assert(wWordsAllocated == 0);
    assert(wMallocs == 0);
    assert(wEvenMallocs == 0);

    return wBytes;

#else // (cnDigitsPerWord != 1)
    JudyFree(*PPArray, EXP(cnBitsPerWord - cnLogBitsPerWord));
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
    if ((PArray == (Pvoid_t)NULL) || (wKey0 > wKey1))
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
    Word_t *pwr = wr_pwr(wRoot);
    Word_t wPopCnt;

  #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    if ( ! tp_bIsSwitch(nType) )
    {
      #if defined(USE_T_ONE)
        if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS)) {
            wPopCnt = 1; // Always a full word to top; never embedded.
        } else
      #endif // defined(USE_T_ONE)
        if (pwr == NULL) {
            wPopCnt = 0;
#if defined(SKIP_TO_BITMAP)
        } else if (nType == T_SKIP_TO_BITMAP) {
            wPopCnt = GetPopCnt(&wRoot, cnDigitsPerWord);
#endif // defined(SKIP_TO_BITMAP)
        } else {
            assert(nType == T_LIST);
            // ls_wPopCnt is valid at top for PP_IN_LINK if ! USE_T_ONE
            wPopCnt = ls_xPopCnt(pwr, cnBitsPerWord);
        }
    }
    else // ! tp_bIsSwitch(nType)
  #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    { // tp_bIsSwitch(nType)
  #if defined(PP_IN_LINK)
        wPopCnt = Sum(&wRoot, cnBitsPerWord);
  #else // defined(PP_IN_LINK)
        wPopCnt = GetPopCnt(&wRoot, cnDigitsPerWord);
  #endif // defined(PP_IN_LINK)
    }

  #if defined(DEBUG)
    if (wPopCnt != wPopCntTotal)
    {
        printf("\nAssertion error debug:\n");
        printf("\nwPopCnt %"_fw"d wPopCntTotal %"_fw"d\n",
               wPopCnt, wPopCntTotal);
        Dump(pwRootLast, 0, cnBitsPerWord);
    }
    assert(wPopCnt == wPopCntTotal);
  #endif // defined(DEBUG)

    return wPopCnt;

#else // (cnDigitsPerWord != 1)

    (void)PArray; (void)wKey0; (void)wKey1, (void)PJError;
    return wPopCntTotal;

#endif // (cnDigitsPerWord != 1)
}


