
// @(#) $Id: b.c,v 1.392 2014/12/11 02:25:16 mike Exp mike $
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

static Word_t
MyMalloc(Word_t wWords)
{
    Word_t ww = JudyMalloc(wWords);
    DBGM(printf("\nM: "OWx" %"_fw"d words\n", ww, wWords));
    assert(ww != 0);
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
    DBGM(printf("F: "OWx" %"_fw"d words\n", (Word_t)pw, wWords));
    JudyFree(pw, wWords);
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
static unsigned
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

// How many words needed for leaf?  Use T_ONE instead of T_LIST if possible.
// Use embedded T_ONE instead of external T_ONE if possible.
unsigned
ListWords(Word_t wPopCnt, unsigned nDL)
{
    unsigned nBL = nDL_to_nBL(nDL);

#if defined(EMBED_KEYS)
    // We need space for the keys, the pop count and the type.
    // What about PP_IN_LINK?  Do we need space for pop count if not at top?
    // What difference would it make?
    // One more embedded 30, 20, 15, 12 and 10-bit key?  Assuming we don't use
    // the extra word in the link for embedded values?
    if (wPopCnt * nBL
            <= (cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL)))
    {
        return 0; // Embed the keys, if any, in wRoot.
    }
#endif // defined(EMBED_KEYS)

    return ListWordsExternal(wPopCnt, nBL);
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
NewList(Word_t wPopCnt, unsigned nDL)
{
    unsigned nBL = nDL_to_nBL(nDL); (void)nBL;

#if defined(EMBED_KEYS)
    // We need space for the keys, the pop count and the type.
    // What about PP_IN_LINK?  See ListWords for more comments.
    if (wPopCnt * nBL + nBL_to_nBitsPopCntSz(nBL) + cnBitsMallocMask
            <= cnBitsPerWord)
    {
        return NULL;
    }
#endif // defined(EMBED_KEYS)

    return NewListExternal(wPopCnt, nBL);
}

Word_t
OldList(Word_t *pwList, Word_t wPopCnt, unsigned nDL, unsigned nType)
{
    unsigned nBL = nDL_to_nBL(nDL);
    unsigned nWords = ((nType == T_LIST) ? ListWordsTypeList(wPopCnt, nBL)
                                         : ListWords(wPopCnt, nDL));

    DBGM(printf("Old pwList %p wLen %d nBL %d wPopCnt "OWx" nType %d\n",
        (void *)pwList, nWords, nDL_to_nBL(nDL), (Word_t)wPopCnt, nType));

    if (nWords == 0) { return 0; }

#if defined(DL_IN_LL)
    assert(nDL == ll_nDL(pwList));
#endif // defined(DL_IN_LL)

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
NewBitmap(Word_t *pwRoot, unsigned nBL)
{
    Word_t wWords = EXP(nBL - cnLogBitsPerWord);

    Word_t *pwBitmap = (Word_t *)MyMalloc(wWords);

    METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
    METRICS(j__AllocWordsJL12 += wWords); // JUDYB -- overloaded

    DBGM(printf("NewBitmap nBL %u nBits "OWx
      " nBytes "OWx" wWords "OWx" pwBitmap "OWx"\n",
        nBL, EXP(nBL), EXP(nBL - cnLogBitsPerByte), wWords,
        (Word_t)pwBitmap));

    memset((void *)pwBitmap, 0, wWords * sizeof(Word_t));

    Word_t wRoot; set_wr(wRoot, pwBitmap, T_BITMAP);

    *pwRoot = wRoot;

    return pwBitmap;
}

static Word_t
OldBitmap(Word_t *pwRoot, Word_t *pwr, unsigned nBL)
{
    Word_t wWords = EXP(nBL - cnLogBitsPerWord);

    MyFree(pwr, wWords);

    METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
    METRICS(j__AllocWordsJL12 -= wWords); // JUDYB -- overloaded

    *pwRoot = 0; // Do we need to clear the rest of the link, e.g. PP_IN_LINK?

    return wWords * sizeof(Word_t);
}

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
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
          int bBmSw,
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
          unsigned nDLUp, Word_t wPopCnt)
{
    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    assert((sizeof(BmSwitch_t) % sizeof(Word_t)) == 0);
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
    Word_t wIndexCnt = EXP(nBitsIndexSz);

#if ! defined(NDEBUG)
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (bBmSw)
#if defined(BM_IN_LINK)
    { Link_t ln; assert(wIndexCnt <= sizeof(ln.ln_awBm) * cnBitsPerByte); }
#else // defined(BM_IN_LINK)
    { BmSwitch_t sw; assert(wIndexCnt <= sizeof(sw.sw_awBm) * cnBitsPerByte); }
#endif // defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#endif // ! defined(NDEBUG)

    Word_t wLinks = wIndexCnt;

#if defined(BM_SW_FOR_REAL)
    if (bBmSw)
    {
  #if defined(BM_IN_LINK)
        if (nDLUp != cnDigitsPerWord)
  #endif // defined(BM_IN_LINK)
        {
            wLinks = 1; // number of links in switch
        }
    }
#endif // defined(BM_SW_FOR_REAL)

    Word_t wWords =
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        bBmSw ? sizeof(BmSwitch_t) :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            sizeof(Switch_t);

    // sizeof([Bm]Switch_t) includes one link; add the others
    wWords += (wLinks - 1) * sizeof(Link_t);
    wWords /= sizeof(Word_t);

    Word_t *pwr = (Word_t *)MyMalloc(wWords);

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (bBmSw) {
        memset(pwr_pLinks((BmSwitch_t *)pwr), 0, wLinks * sizeof(Link_t));
    } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    {
        memset(pwr_pLinks((Switch_t *)pwr), 0, wLinks * sizeof(Link_t));
    }

#if defined(RAMMETRICS)
    // Is a branch with embedded bitmaps a branch?
    // Or is it a bitmap?  Let's use bitmap since we get more info that way.
    if ((cnBitsInD1 <= cnLogBitsPerWord)
        && (nDL_to_nBL(nDL) - nBitsIndexSz <= cnBitsInD1))
    {
        // Embedded bitmaps.
        // What if we have bits in the links that are not used as
        // bits in the bitmap?
        METRICS(j__AllocWordsJLB1 += wWords); // JUDYA
    } else
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (bBmSw) {
        METRICS(j__AllocWordsJBB  += wWords); // JUDYA
    } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    {
        METRICS(j__AllocWordsJBU  += wWords); // JUDYA
    }
#endif // defined(RAMMETRICS)

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    DBGM(printf("NewSwitch(pwRoot %p wKey "OWx
                " nDL %d bBmSw %d nDLU %d wPopCnt %ld)"
                " pwr %p\n",
                (void *)pwRoot, wKey,
                nDL, bBmSw, nDLUp, (long)wPopCnt, (void *)pwr));
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    DBGI(printf("\nNewSwitch nDL %d nBL %d nDLU %d\n",
                nDL, nDL_to_nBL(nDL), nDLUp));
#if 0
    for (int ii = 1; ii < cnBitsPerWord; ii++) {
        int jj = nBL_to_nDL(ii);
        DBGI(printf("nBL %d nDL %d nDL_to_nBL %d nBitsIndexSz %d\n",
                    ii, jj, nDL_to_nBL(jj), nDL_to_nBitsIndexSz(jj)));
    }
#endif

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (bBmSw) {
        set_wr(*pwRoot, pwr, T_BM_SW);
    } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    {
        set_wr_pwr(*pwRoot, pwr);
#if defined(TYPE_IS_RELATIVE)
        set_wr_nDS(*pwRoot, nDLUp - nDL);
#else // defined(TYPE_IS_RELATIVE)
        if (nDL == nDLUp) {
            set_wr_nType(*pwRoot, T_SWITCH);
        } else {
            set_wr_nDL(*pwRoot, nDL);
        }
#endif // defined(TYPE_IS_RELATIVE)
    }

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (bBmSw)
    {
#if defined(BM_IN_LINK)
        if (nDLUp < cnDigitsPerWord)
#endif // defined(BM_IN_LINK)
        {
#if defined(BM_SW_FOR_REAL)

            memset(PWR_pwBm(pwRoot, pwr), 0, sizeof(PWR_pwBm(pwRoot, pwr)));

            unsigned nBL = nDL_to_nBL_NAT(nDL);
            // leave nBL greater than cnBitsPerWord intentionally for now

            Word_t wIndex
                = (wKey >> (nBL - nBitsIndexSz)) & (wIndexCnt - 1);

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
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

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
            && ((nDL - 1) > nBL_to_nDL(cnBitsInD1))
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
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            if (bBmSw) {
                set_PWR_wPrefix(pwRoot, (BmSwitch_t *)pwr, nDL, wKey);
            } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            {
                set_PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDL, wKey);
            }
        }
#else // defined(SKIP_LINKS)
        // Why do we bother with this?  Should we make it debug only?
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        if (bBmSw) {
            set_PWR_wPrefix(pwRoot, (BmSwitch_t *)pwr, nDL, 0);
        } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        {
            set_PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDL, 0);
        }
#endif // defined(SKIP_LINKS)

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        if (bBmSw) {
            set_PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDL, wPopCnt);
        } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        {
            set_PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDL, wPopCnt);
        }

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        DBGM(printf("NewSwitch PWR_wPrefixPop "OWx"\n",
            bBmSw ? PWR_wPrefixPop(pwRoot, (BmSwitch_t *)pwr)
                  : PWR_wPrefixPop(pwRoot, (Switch_t *)pwr)));
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    }

    //DBGI(printf("After NewSwitch"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nDL; // nDL is not used for all ifdef combos
    (void)nDLUp; // nDLUp is not used for all ifdef combos
}

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(BM_SW_FOR_REAL)
static void
NewLink(Word_t *pwRoot, Word_t wKey, unsigned nDL)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    DBGI(printf("NewLink(pwRoot %p wKey "OWx" nDL %d)\n",
        (void *)pwRoot, wKey, nDL));
    DBGI(printf("PWR_wPopCnt %"_fw"d\n",
         PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDL)));

#if defined(BM_IN_LINK)
    assert(nDL != cnDigitsPerWord);
#endif // defined(BM_IN_LINK)

    // What is the index of the new link?
    unsigned nBL = nDL_to_nBL(nDL);
    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
    Word_t wIndex
        = ((wKey >> (nBL - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1));
    DBGI(printf("wKey "OWx" nBL %d nBitsIndexSz %d wIndex (virtual) "OWx"\n",
                wKey, nBL, nBitsIndexSz, wIndex));

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
    if ((cnBitsInD1 <= cnLogBitsPerWord)
        && (nDL_to_nBL(nDL) - nBitsIndexSz <= cnBitsInD1))
    {
        METRICS(j__AllocWordsJLB1 -= nWordsOld); // JUDYA
    } else {
        METRICS(j__AllocWordsJBB  -= nWordsOld); // JUDYA
    }

    // What rule should we use to decide when to uncompress a bitmap switch?

    // 5/8 is close to the golden ratio
    // if (wPopCnt >= EXP(nBitsIndexSz) * 5 / 8)

    // Does this include the key were inserting now?  I think it does.
    Word_t wPopCntKeys = PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDL);
    (void)wPopCntKeys;

    // If the number of null words we'd add by uncompressing the switch
    // is insignificant w.r.t. the population, then do it.
    int nWordsNull = sizeof(Switch_t) - nWordsOld
           + (EXP(nBitsIndexSz) - 2) * sizeof(Link_t) / sizeof(Word_t);
    (void)nWordsNull;
    // if (wPopCntKeys / nWordsNull / nDL > 10)

#if defined(DEBUG_INSERT)
    static int16_t sDigitsReportedMask = 0;
    (void)sDigitsReportedMask;
#endif // defined(DEBUG_INSERT)
#if 0
    if (0)
#else
    // Threshold for converting bm sw into uncompressed switch.
    // Words-per-Key Numerator / Words-per-Key Denominator.
    if (wPopCntTotal * cnBmSwNum / cnBmSwDenom
        >= (wWordsAllocated /* + wMallocs + wEvenMallocs */ + nWordsNull))
#endif
    {
#if defined(DEBUG_INSERT)
        if ( ! (EXP(nDL) & sDigitsReportedMask) )
        {
            sDigitsReportedMask |= EXP(nDL);
            printf("# Converting nKeys %ld nLinks %ld nDL %d",
                   wPopCntKeys, wPopCnt, nDL);
            printf(" wPopCntTotal %ld wWordsAllocated %ld",
               wPopCntTotal, wWordsAllocated);
            printf(" wMallocs %ld wEvenMallocs %ld nWordsNull %d\n",
               wMallocs, wEvenMallocs, nWordsNull);
        }
#endif // defined(DEBUG_INSERT)

        // We don't currently support skip links to bitmap switches.
        // As such, nDLUp always equals nDL.  But we should be
        // supporting skip links to bitmap switches so we'll have
        // to do something different when that happens.
        Word_t *pwrNew = NewSwitch(pwRoot, wKey,
                                   nDL, /* bBmSw */ 0, nDL, wPopCntKeys);
        unsigned mm = 0;
        for (unsigned nn = 0; nn < EXP(nBitsIndexSz); nn++) {
            if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn)) {
                pwr_pLinks((Switch_t *)pwrNew)[nn]
                    = pwr_pLinks((BmSwitch_t *)pwr)[mm];
                ++mm;
            }
        }
        // NewSwitch installs a proper wRoot at *pwRoot.
    } else {
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
             PWR_wPopCnt(pwRoot, (BmSwitch_t *)*pwRoot, nDL)));
        // Initialize the new link.
        DBGI(printf("pLinks %p\n",
                    (void *)pwr_pLinks((BmSwitch_t *)*pwRoot)));
        DBGI(printf("memset %p\n",
                    (void *)&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex]));
        memset(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex], 0, sizeof(Link_t));
        DBGI(printf("PWR_wPopCnt A %"_fw"d\n",
             PWR_wPopCnt(pwRoot, (BmSwitch_t *)*pwRoot, nDL)));
        memcpy(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex + 1],
               &pwr_pLinks((BmSwitch_t *)pwr)[wIndex],
            (wPopCnt - wIndex) * sizeof(Link_t));

        DBGI(printf("PWR_wPopCnt B %"_fw"d\n",
             PWR_wPopCnt(pwRoot, (BmSwitch_t *)*pwRoot, nDL)));
        // Set the bit in the bitmap indicating that the new link exists.
        SetBit(PWR_pwBm(pwRoot, *pwRoot),
            ((wKey >> (nBL - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1)));
        DBGI(printf("PWR_wPopCnt %"_fw"d\n",
             PWR_wPopCnt(pwRoot, (BmSwitch_t *)*pwRoot, nDL)));

        if ((cnBitsInD1 <= cnLogBitsPerWord)
            && (nDL_to_nBL(nDL) - nBitsIndexSz <= cnBitsInD1))
        {
            METRICS(j__AllocWordsJLB1 += nWordsNew); // JUDYA
        } else
#if defined(RETYPE_FULL_BM_SW)
        if (wPopCnt == EXP(nBitsIndexSz) - 1) {
  #if defined(DEBUG_INSERT)
            if ( ! (EXP(nDL) & sDigitsReportedMask) )
            {
                sDigitsReportedMask |= EXP(nDL);
                printf("# Retyping full BM_SW nKeys %ld nLinks %ld nDL %d",
                       wPopCntKeys, wPopCnt, nDL);
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
#if defined(BM_IN_NON_BM_SW)
            // Conserve precious type values if possible.
            set_wr_nType(*pwRoot, T_SWITCH);
#else // defined(BM_IN_NON_BM_SW)
            set_wr_nType(*pwRoot, T_FULL_BM_SW);
#endif // defined(BM_IN_NON_BM_SW)
        } else
#endif // defined(RETYPE_FULL_BM_SW)
        {
            set_wr_nType(*pwRoot, T_BM_SW);
        }
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    }

    MyFree(pwr, nWordsOld);

    //DBGI(printf("After NewLink"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

static Word_t
OldSwitch(Word_t *pwRoot, unsigned nDL,
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
          int bBmSw,
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
          unsigned nDLUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);

    Word_t wLinks = EXP(nDL_to_nBitsIndexSz(nDL));

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(BM_SW_FOR_REAL)
    if (bBmSw)
    {
  #if defined(BM_IN_LINK)
        if (nDLUp != cnDigitsPerWord)
  #endif // defined(BM_IN_LINK)
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
    }
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    Word_t wWords = bBmSw ? sizeof(BmSwitch_t) : sizeof(Switch_t);
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    Word_t wWords =  sizeof(Switch_t);
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    // sizeof([Bm]Switch_t) includes one link; add the others
    wWords += (wLinks - 1) * sizeof(Link_t);
    wWords /= sizeof(Word_t);

#if defined(RAMMETRICS)
    unsigned nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if (wr_nType(*pwRoot) == T_BM_SW) {
        METRICS(j__AllocWordsJBB  -= wWords); // JUDYA
    } else
#if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
    if (wr_nType(*pwRoot) == T_FULL_BM_SW) {
        METRICS(j__AllocWordsJBU  -= wWords); // JUDYA
    } else
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    if ((cnBitsInD1 <= cnLogBitsPerWord)
        && (nDL_to_nBL(nDL) - nBitsIndexSz <= cnBitsInD1))
    {
        // Embedded bitmaps.
        METRICS(j__AllocWordsJLB1 -= wWords); // JUDYA
    } else {
        METRICS(j__AllocWordsJBU  -= wWords); // JUDYA
    }
#endif // defined(RAMMETRICS)

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    DBGR(printf("\nOldSwitch nDL %d bBmSw %d nDLU %d wWords %"_fw"d "OWx"\n",
         nDL, bBmSw, nDLUp, wWords, wWords));
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

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

    assert(nBL >= cnBitsInD1);
    assert(nDL >= 1);

    if ( ! bDump )
    {
        DBGR(printf("FreeArrayGuts pwR "OWx" wPrefix "OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBL, bDump));
        DBGR(printf("wRoot "OWx"\n", wRoot));
    }

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

    if (((EXP(cnBitsInD1) <= sizeof(Link_t) * 8) && (nDL == 1))
        || (nType == T_BITMAP))
    {
#if defined(PP_IN_LINK)
        if (bDump)
        {
            assert(nBLArg != cnBitsPerWord);
 
            if (EXP(cnBitsInD1) > sizeof(Link_t) * 8)
            {
                printf(" wr_wPopCnt %3"_fw"u",
                       PWR_wPopCnt(pwRoot, NULL, nDL));
                printf(" wr_wPrefix "OWx,
                       PWR_wPrefix(pwRoot, NULL, nDL));
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
    assert((nType < T_SW_BASE)
        || (nDL - wr_nDS(wRoot) >= nBL_to_nDL(cnBitsInD1)));
#else // defined(TYPE_IS_RELATIVE)
    assert((nType < T_SW_BASE) || (wr_nDL(wRoot) <= nBL_to_nDL(nBL)));
#endif // defined(TYPE_IS_RELATIVE)
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(USE_T_ONE)

        if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS))
        {
#if defined(EMBED_KEYS)
            if (nBL <= cnBitsPerWord - cnBitsMallocMask) {
                wPopCnt = wr_nPopCnt(wRoot, nBL);
            } else
#endif // defined(EMBED_KEYS)
            {
                wPopCnt = 1;
            }

            if (!bDump)
            {
                // This OldList is a no-op and will return zero if
                // the key(s) is(are) embedded.
                return OldList(pwr, /* wPopCnt */ 1, nDL, nType);
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

#if defined(EMBED_KEYS)
            if (nBL <= cnBitsPerWord - cnBitsMallocMask) {
                for (unsigned nn = 1; nn <= wPopCnt; nn++) {
                    printf(" %08"_fw"x",
                        (wRoot >> (cnBitsPerWord - (nn * nBL)))
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
            if (nDL != cnDigitsPerWord)
            {
                wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL);
            }
            else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = ls_xPopCnt(pwr, nBL);
            }

            if (!bDump)
            {
                return OldList(pwr, wPopCnt, nDL, nType);
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

    unsigned nDLPrev = nDL;

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    int bBmSw = 0;
    if ( 0 || (nType == T_BM_SW)
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
           || (nType == T_FULL_BM_SW)
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #if defined(EXTRA_TYPES)
           || (nType == T_BM_SW + EXP(cnBitsMallocMask))
      #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
           || (nType == T_FULL_BM_SW + EXP(cnBitsMallocMask))
      #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #endif // defined(EXTRA_TYPES)
        )
    {
        bBmSw = 1;
    }
    else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    {
#if defined(TYPE_IS_RELATIVE)
        nDL = nDL - wr_nDS(wRoot);
#else // defined(TYPE_IS_RELATIVE)
        if (nType >= T_SW_BASE) {
            nDL = wr_nDL(wRoot);
        }
#endif // defined(TYPE_IS_RELATIVE)
    }
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    nBL = nDL_to_nBL(nDL);

    nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    pLinks = bBmSw ? pwr_pLinks((BmSwitch_t *)pwr)
                   : pwr_pLinks((Switch_t *)pwr);
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
    pLinks = pwr_pLinks((Switch_t *)pwr);
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBLArg >= cnBitsPerWord)
        {
            // Add 'em up.
            Word_t xx = 0; (void)xx;
            Word_t wPopCnt = 0;
            for (unsigned nn = 0; nn < EXP(cnBitsIndexSzAtTop); nn++)
            {
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if ! defined(BM_IN_LINK)
        if ( ! bBmSw || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // ! defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        {
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                Word_t *pwRootLn
                            = bBmSw
                                ? &pwr_pLinks((BmSwitch_t *)pwr)[nn].ln_wRoot
                                : &pwr_pLinks((  Switch_t *)pwr)[nn].ln_wRoot;
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                Word_t *pwRootLn = &pwr_pLinks((Switch_t *)pwr)[nn].ln_wRoot;
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if ! defined(BM_IN_LINK)
                if (bBmSw) {
                    pwRootLn = &pwr_pLinks((BmSwitch_t *)pwr)[xx].ln_wRoot;
                    xx++;
                }
#endif // ! defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

// *pwRootLn may not be a pointer to a switch
// It may be a pointer to a list leaf.
// And if nBL_to_nDL(cnBitsInD1) == cnDigitsPerWord - 1, then it could be
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
                        = (nTypeLn == T_SWITCH)
                            ? PWR_wPopCnt(pwRootLn, NULL, nDL)
                            : PWR_wPopCnt(pwRootLn, NULL, wr_nDL(*pwRootLn));
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
                    wPopCnt += 1
                        + ((nTypeLn == T_SWITCH)
                            ? wPrefixPopMask(nDL)
                            : wPrefixPopMask(wr_nDL(*pwRootLn)));
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
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                   bBmSw ? PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDL) :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                         PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDL));
            printf(" wr_wPrefix "OWx,
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                   bBmSw ? PWR_wPrefix(pwRoot, (BmSwitch_t *)pwr, nDL) :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                         PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDL));
        }

        printf(" wr_nDL %2d", nDL);
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask "OWx, wPrefixPopMask(nDL));
        //printf(" pLinks "OWx, (Word_t)pLinks);
        if (bBmSw)
        {
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
        }
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        printf("\n");
    }

    nBL = nDL_to_nBL(nDL - 1);
    nBitsIndexSz = nDL_to_nBitsIndexSz(nDL);

    // skip link has extra prefix bits
    if (nDLPrev > nDL)
    {
        wPrefix =
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            bBmSw ? PWR_wPrefix(pwRoot, (BmSwitch_t *)pwr, nDL) :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                PWR_wPrefix(pwRoot, (  Switch_t *)pwr, nDL);
    }

    Word_t xx = 0;
    for (Word_t nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(BM_IN_LINK)
        if ( ! bBmSw
            || ((nBLArg == cnBitsPerWord)
                || BitIsSet(PWR_pwBm(pwRoot, pwr), nn)))
#else // defined(BM_IN_LINK)
        if ( ! bBmSw || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
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

    wBytes += OldSwitch(pwRootArg, nDL,
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                        bBmSw,
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                        nDLPrev);

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
    if (bHitDebugThreshold) {
        printf("\nDump\n");
        FreeArrayGuts(pwRoot, wPrefix, nBL, /* bDump */ 1);
        printf("End Dump\n");
    }
}
#endif // defined(DEBUG)

#if (cwListPopCntMax != 0)

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
    DBGI(printf("\nCopyWithInsertInt(pTgt %p pSrc %p nKeys %d iKey 0x%x\n",
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
InsertAtDl1(Word_t *pwRoot, Word_t wKey, unsigned nDL,
            unsigned nBL, Word_t wRoot);

Status_t
InsertAtBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot);

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)

Word_t
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, unsigned nBL, Word_t wRoot);

Word_t
DeflateExternalList(Word_t *pwRoot,
                    unsigned nPopCnt, unsigned nBL, Word_t *pwr);

#endif // defined(EMBED_KEYS)

// Max list length as a function of nBL.
// Array is indexed by LOG(nBL-1).
const unsigned anListPopCntMax[] = {
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
    for (unsigned ii = 0; ii < nWords; ii++) {
        printf(OWx"\n", pw[ii]);
    }
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
InsertGuts(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
    (void)nDL;
    unsigned nBL = nDL_to_nBL(nDL); (void)nBL;
    DBGI(printf("InsertGuts pwRoot %p wKey "OWx" nDL %d wRoot "OWx"\n",
               (void *)pwRoot, wKey, nDL, wRoot));

    assert(nDL >= 1);

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

    // Check to see if we're at the bottom before checking nType since
    // nType may be invalid if wRoot is an embedded bitmap.
    // The first test can be done at compile time and might make the
    // InsertAtDl1 go away.
    if ((EXP(cnBitsInD1) <= sizeof(Link_t) * 8) && (nDL == 1)) {
        return InsertAtDl1(pwRoot, wKey, nDL, nBL, wRoot);
    }

    unsigned nType = wr_nType(wRoot); (void)nType; // silence gcc

    if (nType == T_BITMAP) {
        return InsertAtBitmap(pwRoot, wKey, nDL, wRoot);
    }

#if (cwListPopCntMax != 0)
#if defined(EMBED_KEYS)
    // Change an embedded list into an external list to make things
    // easier for Insert.  We'll change it back later if it makes sense.
    // We used to use T_ONE for a single embedded key.  But not anymore.
    if (nType == T_EMBEDDED_KEYS) {
        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        // BUG: The list may not be sorted at this point.  Does it matter?
        // Update: I'm not sure why I wrote that the list may not be sorted
        // at this point.  I can't think of why it would not be sorted.
        nType = wr_nType(wRoot);
    }
#endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

    Word_t *pwr = wr_tp_pwr(wRoot, nType);

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
                assert((nDL == cnDigitsPerWord)
                    || (PWR_wPopCnt(pwRoot, NULL, nDL) == wPopCnt + 1));
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
                    wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL) - 1;
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

        unsigned nDLOld = nDL;

#if defined(EMBED_KEYS) && ! defined(POP_CNT_MAX_IS_KING)
        // It makes no sense to impose a pop limit that is less than what
        // will fit as embedded keys.  If we want to be able to do that for
        // running experiments, then we can use POP_CNT_MAX_IS_KING.
        int nEmbeddedListPopCntMax
            = (cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL))
                / nBL;
#endif // defined(EMBED_KEYS)

#if (cwListPopCntMax != 0) // true if we are using lists; embedded or external
        if (0
#if defined(EMBED_KEYS) && ! defined(POP_CNT_MAX_IS_KING)
            || (wPopCnt < (Word_t)nEmbeddedListPopCntMax)
#endif // defined(EMBED_KEYS) && ! defined(POP_CNT_MAX_IS_KING)
            || ((nDL == 1) && (wPopCnt < cnListPopCntMaxDl1))
#if defined(cnListPopCntMaxDl2)
            || ((nDL == 2) && (wPopCnt < cnListPopCntMaxDl2))
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
            || ((nDL == 3) && (wPopCnt < cnListPopCntMaxDl3))
#endif // defined(cnListPopCntMaxDl3)
            || ((nDL != 1)
#if defined(cnListPopCntMaxDl2)
                && (nDL != 2)
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
                && (nDL != 3)
#endif // defined(cnListPopCntMaxDl3)
                && (wPopCnt < anListPopCntMax[LOG(nBL - 1)])))
        {
            Word_t *pwList;

            // Allocate memory for a new list if necessary.
            // Init or update pop count if necessary.
            if ((pwr == NULL)
                || (ListWordsExternal(wPopCnt + 1, nBL)
                        != ListWordsExternal(wPopCnt, nBL)))
            {
                // Allocate a new list and init pop count if pop count is
                // in the list.  Also init the beginning of the list marker
                // if LIST_END_MARKERS.
                pwList = NewListTypeList(wPopCnt + 1, nBL);
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
                set_ls_xPopCnt(pwList, nBL, wPopCnt + 1);
#endif // defined(PP_IN_LINK)
            }

            set_wr(wRoot, pwList, T_LIST);

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

            *pwRoot = wRoot; // install new

            if ((wPopCnt != 0) && (pwr != pwList))
            {
                OldList(pwr, wPopCnt, nDL, nType);
            }

#if defined(EMBED_KEYS)
            // Embed the list if it fits.
            assert(wr_nType(wRoot) == T_LIST);
            if ((nBL * (wPopCnt + 1)
                    <= cnBitsPerWord - cnBitsMallocMask
                        - nBL_to_nBitsPopCntSz(nBL))
                || (wPopCnt == 0))
            {
                DeflateExternalList(pwRoot, wPopCnt + 1, nBL, pwList);
            }
#endif // defined(EMBED_KEYS)
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            Word_t w;

            // List is full; insert a switch or create a bitmap.
            DBGI(printf("List is full.\n"));
#if defined(SKIP_LINKS)
#if (cwListPopCntMax != 0)
#if    (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) \
    || (cnListPopCntMax16 == 0)
            if (wPopCnt == 0) {
                // Can't dereference list if there isn't one.
                // Go directly to bitmap.  Going to bitmap is a two-step
                // process.  First create the switch above the bitmap.
                // Can't skip directly to bitmap since bitmap has no
                // prefix.  Then create the bitmap below the switch.
                if (nDLOld >= 2) {
                    nDL = 2; // first step
                }
                goto newSwitch;
            }
#endif // (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) || ...
#endif // (cwListPopCntMax != 0)
#endif // defined(SKIP_LINKS)

#if defined(PP_IN_LINK) || defined(NO_SKIP_AT_TOP)
            if (nDL < cnDigitsPerWord)
#endif // defined(PP_IN_LINK) || defined(NO_SKIP_AT_TOP)
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
                // Go directly to bitmap.  Going to bitmap is a two-step
                // process.  First create the switch above the bitmap.
                // Can't skip directly to bitmap since bitmap has no
                // prefix.  Then create the bitmap below the switch.
                if (nDLOld >= 2) {
                    nDL = 2;
                }
#endif // (cwListPopCntMax != 0)
            }
#if (cwListPopCntMax != 0)
#if    (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) \
    || (cnListPopCntMax16 == 0)
newSwitch:
#endif // (cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) || ...
#endif // (cwListPopCntMax != 0)

            // We don't create a switch below nDL == 2.
            // Nor do we create a switch below nBL == cnLogBitsPerWord.
            // Nor do we support a skip link directly to a bitmap -- yet.
            if (nDL < 2)
            {
                DBGI(printf("InsertGuts nDL %d nBL %d", nDL, nBL));
                if (nDLOld >= 2) {
                    nDL = 2;
                }
            }
            assert(nDL_to_nBL(nDL) > LOG(sizeof(Link_t) * 8));

#if ! defined(DEPTH_IN_SW)
#if defined(TYPE_IS_RELATIVE)
            // cnMallocMask is the largest nType value
            // if we're storing it in wRoot.
            // How do we handle this for ! defined(TYPE_IS_RELATIVE)?
            // We can't unless defined(DEPTH_IN_SW).  So we have an
            // assertion in Initialize().
            if (nDS_to_tp(nDLOld - nDL) > cnMallocMask) {
                nDL = nDLOld - tp_to_nDS(cnMallocMask);
            }
#endif // defined(TYPE_IS_RELATIVE)
#endif // ! defined(DEPTH_IN_SW)
#else // defined(SKIP_LINKS)
            // I'm don't remember why this assertion was here.
            // But it blows and the code seems to do ok with it
            // commented out.
            // assert(nDL > 1);
#endif // defined(SKIP_LINKS)

            if ((EXP(cnBitsInD1) > sizeof(Link_t) * 8) && (nDL == 1))
            {
#if defined(SKIP_LINKS)
                assert(nDLOld == 1); // Handled above, right?
#if 0
                // no skip link to bitmap
                if (nDL == 1) {
                    if (nDLOld > 2) { nDL = 2; }
                }
#endif
#endif // defined(SKIP_LINKS)
                assert(nDLOld == nDL);
                NewBitmap(pwRoot, cnBitsInD1);
#if defined(PP_IN_LINK)
                set_PWR_wPopCnt(pwRoot, NULL, /* nDL */ 1, 0);
#endif // defined(PP_IN_LINK)
            }
            else
            {
                // NewSwitch overwrites *pwRoot which is a problem for
                // T_ONE with embedded keys.

#if defined(BM_SW_AT_DL2)
                // We want a bm switch at dl2, but we don't currently support
                // skipping to a bm switch.
                if ((nDL == 2) && (nDLOld != nDL)) {
                    assert(nDLOld >= 3);
                    nDL = 3;
                }
#endif // defined(BM_SW_AT_DL2)

                NewSwitch(pwRoot, wKey, nDL,
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(USE_BM_SW)
                          /* bBmSw */ nDL == nDLOld,
#elif defined(BM_SW_AT_DL2)
                          /* bBmSw */ (nDL == 2),
#else // defined(BM_SW_AT_DL2)
                          /* bBmSw */ 0,
#endif // defined(USE_BM_SW)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                          nDLOld, /* wPopCnt */ 0);

                if (nDL == nDLOld) {
                    DBGI(printf("\n# InsertGuts After NewSwitch Dump\n"));
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                    DBGI(printf("\n"));
                }
            }

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
                    if (nDL == nDLOld) {
                    DBGI(printf(
                         "\n# InsertGuts After Insert(wKey 0x%x) Dump\n",
                         piKeys[w]));
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                    DBGI(printf("\n"));
                    }
                }
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                for (w = 0; w < wPopCnt; w++)
                {
                    Insert(pwRoot, pwKeys[w], nDLOld);

                    if (nDL == nDLOld) {
                    DBGI(printf(
                         "\n# InsertGuts After Insert(wKey "OWx") Dump\n",
                         pwKeys[w]));
                    DBGI(Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord));
                    DBGI(printf("\n"));
                    }
                }
            }

            if (nDL == nDLOld) {
                DBGI(printf("Just Before InsertGuts calls final Insert"));
                DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
            }
            Insert(pwRoot, wKey, nDLOld);

#if (cwListPopCntMax != 0)
            // Hmm.  Should this be nDLOld?
            if (wPopCnt != 0) { OldList(pwr, wPopCnt, nDLOld, nType); }
#endif // (cwListPopCntMax != 0)
        }
    }
#if defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)
    else
    {
#if defined(BM_SW_FOR_REAL)
  #if defined(TYPE_IS_RELATIVE)
        unsigned nDLR = nDL - wr_nDS(wRoot);
  #else // defined(TYPE_IS_RELATIVE)
        unsigned nDLR = (nType == T_SWITCH) ? nDL : wr_nDL(wRoot);
  #endif // defined(TYPE_IS_RELATIVE)
        if ( 0 || (nType == T_BM_SW)
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
               || (nType == T_FULL_BM_SW)
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #if defined(EXTRA_TYPES)
               || (nType == T_BM_SW + EXP(cnBitsMallocMask))
      #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
               || (nType == T_FULL_BM_SW + EXP(cnBitsMallocMask))
      #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #endif // defined(EXTRA_TYPES)
            )
        {
            nDLR = nDL;
        }
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
            || ((wPrefix = PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR))
                == w_wPrefixNotAtTop(wKey, nDLR)))
        // If nDS != 0 then we're not at the top or PP_IN_LINK is not defined.
#endif // defined(SKIP_LINKS)
        {
            // Missing link.
#if defined(EXTRA_TYPES)
            assert((nType == T_BM_SW)
                || (nType == T_BM_SW + EXP(cnBitsMallocMask)));
#else // defined(EXTRA_TYPES)
            assert(nType == T_BM_SW);
#endif // defined(EXTRA_TYPES)
#if defined(SKIP_LINKS)
            DBGI(printf("wPrefix "OWx" w_wPrefix "OWx" nDLR %d\n",
                        PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLR),
                        w_wPrefix(wKey, nDLR), nDLR));
#endif // defined(SKIP_LINKS)
            NewLink(pwRoot, wKey, nDLR);
            Insert(pwRoot, wKey, nDL);
        }
#endif // defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
        else
#endif // defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
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
            nDLRoot = nDL - wr_nDS(wRoot);
#else // defined(TYPE_IS_RELATIVE)
            nDLRoot = (nType == T_SWITCH) ? nDL : wr_nDL(wRoot);
#endif // defined(TYPE_IS_RELATIVE)

            // Can't have a prefix mismatch if there is no skip.
            assert(nDLRoot < nDL);

            unsigned nDLUp = nDL;

            // figure new nDL for old parent link
            Word_t wPrefix = PWR_wPrefix(pwRoot, (Switch_t *)pwr, nDLRoot);
            nDL = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
            // nDL includes the highest order digit that is different.

            assert(nDL > nDLRoot);
            assert(nDL <= nDLUp);

            if ((wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)pwr, nDLRoot)) == 0)
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

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#if defined(USE_BM_SW)
            int bBmSwNew = (nDL == nDLUp); // new switch type
#else // defined(USE_BM_SW)
            int bBmSwNew = 0; // new switch type
#endif // defined(USE_BM_SW)
            int bBmSwOld = 0; // no skip link to bm switch
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

#if defined(BM_IN_LINK)
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            Link_t ln;
            Word_t wIndexCnt = EXP(nDL_to_nBitsIndexSzNAT(nDLRoot));
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
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#endif // defined(BM_IN_LINK)

            Word_t *pwSw;
            // initialize prefix/pop for new switch
            // Make sure to pass the right key for BM_SW_FOR_REAL.
            DBGI(printf("IG: nDL %d nDLUp %d\n", nDL, nDLUp));
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            pwSw = NewSwitch(pwRoot, wPrefix, nDL, bBmSwNew, nDLUp, wPopCnt);
            DBGI(HexDump("After NewSwitch",
                         pwSw, bBmSwNew ? 3 : (EXP(cnBitsPerDigit) + 1)));
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            pwSw = NewSwitch(pwRoot, wPrefix, nDL, nDLUp, wPopCnt);
            DBGI(HexDump("After NewSwitch", pwSw, EXP(cnBitsPerDigit) + 1));
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            DBGI(printf("Just after InsertGuts calls NewSwitch"
                        " for prefix mismatch.\n"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
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

            if (bBmSwOld)
            {
#if defined(BM_IN_LINK)
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
#endif // defined(BM_IN_LINK)
            }
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

#if defined(TYPE_IS_RELATIVE)
            // Update type field in wRoot that points to old switch since
            // it is not skipping as many digits now.
            DBGI(printf("nDL %d nDLR %d nDLU %d\n",
                   nDL, nDLRoot, nDLUp));
            set_wr_nDS(wRoot, nDL - nDLRoot - 1);
#else // defined(TYPE_IS_RELATIVE)
            if (nDL - nDLRoot - 1 == 0) {
               set_wr_nType(wRoot, T_SWITCH);
            }
#endif // defined(TYPE_IS_RELATIVE)
            // Copy wRoot from old link (after being updated) to new link.
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            if (bBmSwNew) {
                pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
            } else
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            {
                pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot = wRoot;
            }

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
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                set_PWR_wPrefix(
                    bBmSwNew
                        ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                          &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLRoot, wPrefix);
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                set_PWR_wPrefix(
                    &pwr_pLinks((Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLRoot, wPrefix);
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            }

#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            set_PWR_wPopCnt(
                    bBmSwNew
                        ? &pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_wRoot :
                          &pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLRoot, wPopCnt);
#else // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            set_PWR_wPopCnt(&pwr_pLinks((  Switch_t *)pwSw)[nIndex].ln_wRoot,
                    NULL, nDLRoot, wPopCnt);
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
#else // defined(PP_IN_LINK)
#if defined(NO_UNNECESSARY_PREFIX)
           // We could go to the trouble of zeroing the no-longer necessary
           // prefix in the old switch. 
#endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)
            DBGI(printf("Just before InsertGuts calls Insert"
                        " for prefix mismatch.\n"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

            Insert(pwRoot, wKey, nDLUp);
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
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, unsigned nBL, Word_t wRoot)
{
    DBGI(printf(
         "InflateEmbeddedList pwRoot %p wKey "OWx" nBL %d wRoot "OWx"\n",
         (void *)pwRoot, wKey, nBL, wRoot));

    assert(wr_nType(wRoot) == T_EMBEDDED_KEYS);

    Word_t *pwKeys;
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
    uint32_t *piKeys;
#endif // (cnBitsPerWord > 32)
    uint16_t *psKeys;
    uint8_t  *pcKeys;
#endif // defined(COMPRESSED_LISTS)

    unsigned nPopCnt = wr_nPopCnt(wRoot, nBL);

    assert(nBL * nPopCnt
        <= cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL));

    Word_t *pwList = NewListTypeList(nPopCnt, nBL);

    Word_t wBLM = MSK(nBL); // Bits left mask.
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        pcKeys = ls_pcKeysNAT(pwList);
        for (unsigned nn = 1; nn <= nPopCnt; nn++) {
            pcKeys[nn-1] = (uint8_t)((wKey & ~wBLM)
                | ((wRoot >> (cnBitsPerWord - (nn * nBL))) & wBLM));
        }
    } else
    if (nBL <= 16) {
        psKeys = ls_psKeysNAT(pwList);
        for (unsigned nn = 1; nn <= nPopCnt; nn++) {
            psKeys[nn-1] = (uint16_t)((wKey & ~wBLM)
                | ((wRoot >> (cnBitsPerWord - (nn * nBL))) & wBLM));
        }
    } else
#if (cnBitsPerWord > 32)
    if (nBL <= 32) {
        piKeys = ls_piKeysNAT(pwList);
        for (unsigned nn = 1; nn <= nPopCnt; nn++) {
            piKeys[nn-1] = (uint32_t)((wKey & ~wBLM)
                | ((wRoot >> (cnBitsPerWord - (nn * nBL))) & wBLM));
        }
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
        for (unsigned nn = 1; nn <= nPopCnt; nn++) {
            pwKeys[nn-1] = (wKey & ~wBLM)
                | ((wRoot >> (cnBitsPerWord - (nn * nBL))) & wBLM);
        }
    }

    set_wr(wRoot, pwList, T_LIST);
    *pwRoot = wRoot;

    return wRoot;
}

// Replace an external T_LIST leaf with a wRoot with embedded keys or
// an external T_ONE leaf.
// The function assumes it is possible.
Word_t
DeflateExternalList(Word_t *pwRoot,
                    unsigned nPopCnt, unsigned nBL, Word_t *pwr)
{
    unsigned nPopCntMax
      = (cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL)) / nBL;

    DBGI(printf("DeflateExternalList pwRoot %p nPopCnt %d nBL %d pwr %p\n",
               (void *)pwRoot, nPopCnt, nBL, (void *)pwr));
    //HexDump("External List", pwr, nPopCnt + 1);

    assert((nPopCnt <= nPopCntMax) || (nPopCnt == 1));

    Word_t wRoot;

    if (nPopCnt <= nPopCntMax)
    {
        wRoot = 0;

#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
        uint32_t *piKeys;
#endif // (cnBitsPerWord > 32)
        uint16_t *psKeys;
        uint8_t  *pcKeys;
#endif // defined(COMPRESSED_LISTS)

        set_wr_nType(wRoot, T_EMBEDDED_KEYS);
        set_wr_nPopCnt(wRoot, nBL, nPopCnt);

        Word_t wBLM = MSK(nBL);
#if defined(COMPRESSED_LISTS)
        if (nBL <= 8) {
            pcKeys = ls_pcKeysNAT(pwr);
            unsigned nn = 1;
            for (; nn <= nPopCnt; nn++) {
               wRoot |= (pcKeys[nn-1] & wBLM) << (cnBitsPerWord - (nn * nBL));
            }
#if defined(PAD_T_ONE)
            while (nn <= nPopCntMax) {
               wRoot |= (pcKeys[0] & wBLM) << (cnBitsPerWord - (nn * nBL));
               ++nn;
            }
#endif // defined(PAD_T_ONE)
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNAT(pwr);
            unsigned nn = 1;
            for (; nn <= nPopCnt; nn++) {
               wRoot |= (psKeys[nn-1] & wBLM) << (cnBitsPerWord - (nn * nBL));
            }
#if defined(PAD_T_ONE)
            while (nn <= nPopCntMax) {
               wRoot |= (psKeys[0] & wBLM) << (cnBitsPerWord - (nn * nBL));
               ++nn;
            }
#endif // defined(PAD_T_ONE)
        } else
#if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNAT(pwr);
            unsigned nn = 1;
            for (; nn <= wr_nPopCnt(wRoot, nBL); nn++) {
               wRoot |= (piKeys[nn-1] & wBLM) << (cnBitsPerWord - (nn * nBL));
            }
#if defined(PAD_T_ONE)
            while (nn <= nPopCntMax) {
               wRoot |= (piKeys[0] & wBLM) << (cnBitsPerWord - (nn * nBL));
               ++nn;
            }
#endif // defined(PAD_T_ONE)
        } else
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        {
            // I don't think we have to worry about adjusting ls_pwKeys
            // for PP_IN_LINK here since we will not be at the top.
            assert(nBL != cnBitsPerWord);
#if defined(COMPRESSED_LISTS)
            assert(nPopCnt == 1);
            wRoot |= (ls_pwKeysNAT(pwr)[0] & wBLM) << (cnBitsPerWord - nBL);
#else // defined(COMPRESSED_LISTS)
            Word_t *pwKeys = ls_pwKeysNAT(pwr);
            unsigned nn = 1;
            for (; nn <= wr_nPopCnt(wRoot, nBL); nn++) {
               wRoot |= (pwKeys[nn-1] & wBLM) << (cnBitsPerWord - (nn * nBL));
            }
#if defined(PAD_T_ONE)
            while (nn <= nPopCntMax) {
               wRoot |= (pwKeys[0] & wBLM) << (cnBitsPerWord - (nn * nBL));
               ++nn;
            }
#endif // defined(PAD_T_ONE)
#endif // defined(COMPRESSED_LISTS)
        }
    }
    else
    {
        assert(nPopCnt == 1);
        assert(nPopCntMax == 0);
        Word_t *pwList = NewList(1, nBL_to_nDL(nBL));
        set_wr(wRoot, pwList, T_ONE); // external T_ONE list
        Word_t *pwKeys = ls_pwKeys(pwr, nBL);
        *pwList = pwKeys[0];
    }

    OldList(pwr, nPopCnt, nBL_to_nDL(nBL), T_LIST);

    *pwRoot = wRoot;

    DBGI(printf("DEL wRoot "OWx"\n", wRoot));

    return wRoot;
}

#endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

Status_t
InsertAtDl1(Word_t *pwRoot, Word_t wKey, unsigned nDL,
            unsigned nBL, Word_t wRoot)
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
InsertAtBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
        (void)pwRoot;
        unsigned nBL = nDL_to_nBL(nDL);

        Word_t *pwr = wr_pwr(wRoot);

        assert(pwr != NULL);

        assert( ! BitIsSet(pwr, wKey & MSK(nBL)) );

        DBGI(printf("SetBit(pwr "OWx" wKey "OWx") pwRoot %p\n",
                    (Word_t)pwr, wKey & MSK(nBL), (void *)pwRoot));

        SetBit(pwr, wKey & MSK(nBL));

#if defined(PP_IN_LINK)

// Shouldn't we do this when we create the switch with the link that
// points to this bitmap rather than on every insert into the bitmap?

        // What about no_unnecessary_prefix?
        set_PWR_wPrefix(pwRoot, NULL, nDL, wKey);

#endif // defined(PP_IN_LINK)

        return Success;
}

Status_t
RemoveBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL,
             unsigned nBL, Word_t wRoot);

Status_t
RemoveGuts(Word_t *pwRoot, Word_t wKey, unsigned nDL, Word_t wRoot)
{
    unsigned nType = wr_nType(wRoot); (void)nType;
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;
    unsigned nBL = nDL_to_nBL(nDL); (void)nBL;

    DBGR(printf("RemoveGuts\n"));

// Could we be more specific in this ifdef, e.g. cnListPopCntMax16?
#if (cwListPopCntMax != 0)
    if ((nBL <= LOG(sizeof(Link_t) * 8)) || (nType == T_BITMAP))
#else // (cwListPopCntMax != 0)
    assert((nBL <= LOG(sizeof(Link_t) * 8)) || (nType == T_BITMAP));
#endif // (cwListPopCntMax != 0)
    {
        return RemoveBitmap(pwRoot, wKey, nDL, nBL, wRoot);
    }

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)
    if (((nType == T_ONE) || (nType == T_EMBEDDED_KEYS))
        && (nBL <= cnBitsPerWord - cnBitsMallocMask))
    {
        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        nType = T_LIST;
        pwr = wr_pwr(wRoot);
        assert(wr_nType(wRoot) == nType);
    }
#endif // defined(EMBED_KEYS)

#if defined(USE_T_ONE)
    if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS)) {
        assert(nBL > cnBitsPerWord - cnBitsMallocMask);
        //OldList(pwr, /* wPopCnt */ 1, nDL, T_ONE);
        OldList(pwr, /* wPopCnt */ 1, nDL, T_EMBEDDED_KEYS);
        *pwRoot = 0; // Do we need to clear the rest of the link also?
        return Success;
    }
#endif // defined(USE_T_ONE)

    assert(wr_nType(wRoot) == T_LIST);
    assert(nType == T_LIST);

    Word_t wPopCnt;

#if defined(PP_IN_LINK)
    if (nDL != cnDigitsPerWord) {
        wPopCnt = PWR_wPopCnt(pwRoot, NULL, nDL) + 1;
    } else
#endif // defined(PP_IN_LINK)
    {
        wPopCnt = ls_xPopCnt(pwr, nBL);
    }

// Why was this #if defined(USE_T_ONE) ever here?
//#if ! defined(USE_T_ONE)
    if (wPopCnt == 1) {
        OldList(pwr, wPopCnt, nDL, nType);
        *pwRoot = 0;
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }
//#endif // ! defined(USE_T_ONE)

    Word_t *pwKeys = ls_pwKeysNAT(pwr);

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
        pwList = NewListTypeList(wPopCnt - 1, nBL);
        // Why are we copying the old list to the new one?
        // Because the beginning will be the same.
        // Except for the the pop count.

        switch (nBytesKeySz(nBL)) {
        case sizeof(Word_t):
             COPY(ls_pwKeysNAT(pwList), ls_pwKeysNAT(pwr), wPopCnt - 1);
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

#if defined(PP_IN_LINK)
    assert(nDL != cnDigitsPerWord); // T_LIST at top is handled by Judy1Unset?
#else // defined(PP_IN_LINK)
    set_ls_xPopCnt(pwList, nBL, wPopCnt - 1);
#endif // defined(PP_IN_LINK)

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
        MOVE(&ls_pwKeysNAT(pwList)[nIndex], &pwKeys[nIndex + 1],
             wPopCnt - nIndex - 1);
#if defined(LIST_END_MARKERS)
        ls_pwKeysNAT(pwList)[nKeys] = -1;
#endif // defined(LIST_END_MARKERS)
    }

    if (pwList != pwr)
    {
        OldList(pwr, wPopCnt, nDL, nType);
        *pwRoot = wRoot;
    }

#if defined(EMBED_KEYS)
    // Embed the list if it fits.
    assert(wr_nType(wRoot) == T_LIST);
    assert(nType == T_LIST);
    if ((nBL * (wPopCnt - 1)
            <= cnBitsPerWord - cnBitsMallocMask - nBL_to_nBitsPopCntSz(nBL))
        || (wPopCnt == 2))
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
RemoveBitmap(Word_t *pwRoot, Word_t wKey, unsigned nDL,
             unsigned nBL, Word_t wRoot)
{
    (void)nDL;

    if (EXP(nBL) <= sizeof(Link_t) * 8) {
        ClrBit(STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey & MSK(nBL));
    } else {
        Word_t *pwr = wr_pwr(wRoot);

        ClrBit(pwr, wKey & MSK(nBL));

#if defined(PP_IN_LINK)

#if defined(DEBUG)
        Word_t wPopCnt = 0;
        for (Word_t ww = 0; ww < EXP(nBL - cnLogBitsPerWord); ww++) {
            wPopCnt += __builtin_popcountll(pwr[ww]);
        }
        if (wPopCnt != PWR_wPopCnt(pwRoot, NULL, nDL)) {
            printf("\nwPopCnt "OWx" PWR_wPopCnt "OWx"\n",
                   wPopCnt, PWR_wPopCnt(pwRoot, NULL, nDL));
            HexDump("Bitmap", pwr, EXP(nBL - cnLogBitsPerWord));
        }
        assert(wPopCnt == PWR_wPopCnt(pwRoot, NULL, nDL));
#endif // defined(DEBUG)

        if (PWR_wPopCnt(pwRoot, NULL, nDL) != 0) {
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
      #if defined(USE_T_ONE)
        if ((nType == T_ONE) || (nType == T_EMBEDDED_KEYS)) {
            wPopCnt = 1; // Always a full word to top; never embedded.
        } else
      #endif // defined(USE_T_ONE)
        {
            // ls_wPopCnt is valid at top for PP_IN_LINK if ! USE_T_ONE
            wPopCnt = ls_xPopCnt(pwr, cnBitsPerWord);
        }
    }
    else
  #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
    {
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        int bBmSw = 0;
        if ( 0 || (nType == T_BM_SW)
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
               || (nType == T_FULL_BM_SW)
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #if defined(EXTRA_TYPES)
               || (nType == T_BM_SW + EXP(cnBitsMallocMask))
      #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
               || (nType == T_FULL_BM_SW + EXP(cnBitsMallocMask))
      #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
  #endif // defined(EXTRA_TYPES)
            )
        {
            bBmSw = 1;
        }
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

  #if defined(PP_IN_LINK)
        // no skip links at root for PP_IN_LINK -- no place for prefix
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
          #if defined(TYPE_IS_RELATIVE)
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        assert((wr_nDS(wRoot) == 0)
            || (nType == T_FULL_BM_SW)
            || (nType == T_BM_SW));
  #else // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        assert((wr_nDS(wRoot) == 0)
            || (nType == T_BM_SW));
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
          #else // defined(TYPE_IS_RELATIVE)
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        assert((nType == T_SWITCH)
            || (nType == T_FULL_BM_SW)
            || (nType == T_BM_SW));
  #else // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        assert((nType == T_SWITCH)
            || (nType == T_BM_SW));
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
          #endif // defined(TYPE_IS_RELATIVE)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        // add up the pops in the links
        Word_t xx = 0; (void)xx;
        wPopCnt = 0;
        for (unsigned nn = 0; nn < EXP(cnBitsIndexSzAtTop); nn++)
        {
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
      #if ! defined(BM_IN_LINK)
            if ( ! bBmSw || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
      #endif // ! defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            {
                Word_t *pwRootLn =
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                         bBmSw ? &pwr_pLinks((BmSwitch_t *)pwr)[nn].ln_wRoot :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                                 &pwr_pLinks((  Switch_t *)pwr)[nn].ln_wRoot;
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
      #if ! defined(BM_IN_LINK)
                if (bBmSw) {
                    pwRootLn = &pwr_pLinks((BmSwitch_t *)pwr)[xx].ln_wRoot;
                    xx++;
                }
      #endif // ! defined(BM_IN_LINK)
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)

                // *pwRootLn may not be a pointer to a switch
                // It may be a pointer to a list leaf.
                // And if nBL_to_nDL(cnBitsInD1) == cnDigitsPerWord - 1,
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
                                      NULL, 
                                      (nTypeLn == T_SWITCH)
                                          ? cnDigitsPerWord - 1
                                          : wr_nDL(wRoot));
          #endif // defined(TYPE_IS_RELATIVE)
                }
                else
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                {
                    wPopCntLn = PWR_wPopCnt(pwRootLn,
                                            NULL, cnDigitsPerWord - 1);
                }

      #if defined(DEBUG_INSERT)
                if (bHitDebugThreshold && (wPopCntLn != 0))
                {
                    printf("Pop sum");
          #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    unsigned nDL
              #if defined(TYPE_IS_RELATIVE)
                        = cnDigitsPerWord - 1 - wr_nDS(*pwRootLn);
              #else // defined(TYPE_IS_RELATIVE)
                        = (nTypeLn == T_SWITCH)
                            ? cnDigitsPerWord - 1 : wr_nDL(*pwRootLn);
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
                        (nTypeLn == T_SWITCH)
                            ? cnDigitsPerWord - 1 : wr_nDL(*pwRootLn)
          #endif // defined(TYPE_IS_RELATIVE)
                        ;
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

      #if defined(DEBUG_INSERT)
                if (bHitDebugThreshold) {
                    printf("Pop sum (full)");
          #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                    printf(" mask "Owx" %"_fw"d\n",
                           wPrefixPopMask(nDL),
                           wPrefixPopMask(nDL));
                    printf("nn %d wPopCntLn %"_fw"d "OWx"\n",
                           nn, wPrefixPopMask(nDL) + 1,
                           wPrefixPopMask(nDL) + 1);
          #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
                }
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
            cnDigitsPerWord - wr_nDS(wRoot)
              #else // defined(TYPE_IS_RELATIVE)
            (nType == T_SWITCH) ? cnDigitsPerWord : wr_nDL(*pwRoot)
              #endif // defined(TYPE_IS_RELATIVE)
            ;
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        if (bBmSw) {
            nDL = cnDigitsPerWord;
        }
        if (!(wPopCnt - 1 <= wPrefixPopMask(nDL))) {
            printf("bBmSw %d\n", bBmSw);
        }
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
        assert(wPopCnt - 1 <= wPrefixPopMask(nDL));
          #endif // ! defined(NDEBUG)
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #else // defined(PP_IN_LINK)
      #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        int nDL =
          #if defined(TYPE_IS_RELATIVE)
            cnDigitsPerWord - wr_nDS(wRoot)
          #else // defined(TYPE_IS_RELATIVE)
            (nType == T_SWITCH) ? cnDigitsPerWord : wr_nDL(wRoot)
          #endif // defined(TYPE_IS_RELATIVE)
            ;
        wPopCnt =
#if defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
            bBmSw ? PWR_wPopCnt(NULL, (BmSwitch_t *)pwr, nDL) :
#endif // defined(USE_BM_SW) || defined(BM_SW_AT_DL2)
                    PWR_wPopCnt(NULL, (  Switch_t *)pwr, nDL);
        if (wPopCnt == 0) {
            wPopCnt = wPrefixPopMask(nDL) + 1;
        }
      #else // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        wPopCnt = PWR_wPopCnt(NULL, pwr, cnDigitsPerWord);
      #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
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


