
// @(#) $Id: b.c,v 1.8 2017/09/22 22:43:40 mike Exp mike $
// @(#) $Source: /home/mike/b/b.c,v $

#include "b.h"

// Check and/or Time depend on Judy1MallocSizes but this version
// of Judy does not use it.
const char *Judy1MallocSizes = "Judy1MallocSizes = 3, 5, 7, ...";

#define nBytesKeySz(_nBL) \
     (((_nBL) <=  8) ? 1 : ((_nBL) <= 16) ? 2 \
    : ((_nBL) <= 32) ? 4 : sizeof(Word_t))

Word_t wPopCntTotal;
#if defined(DEBUG)
Word_t *pwRootLast; // allow dumping of tree when root is not known
#endif // defined(DEBUG)

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

#if defined(JUNK)
static void
HexDump16(char *str, uint16_t *pus, int n)
{
    printf("%s:", str);
    for (int i = 0; i < n; i++) {
        printf(" %04x", pus[i]);
    }
    printf("\n");
}
#endif // defined(JUNK)

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

// Can we use some of the bits in the word at the address immediately
// preceeding the address returned by malloc?
//
// Can we assume it's safe to modify any bits that never change while we
// own a buffer? No. We need more info than that.
// It might be ok with dlmalloc, but, in theory, knowing only that the bits
// don't change while we own a buffer is not sufficient to guarantee that
// we can modify them safely.
//
// It looks like dlmalloc may do a read-modify-write of the word
// while we own the buffer.
// Do we have any synchronization with dlmalloc?
// If not, then it makes things quite difficult even if we know which
// bit or bits dlmalloc might modify in this way.
// Can we assume that because we don't allow concurrent array-
// changing operations (and we only use malloc during array-changing
// operations) that we are safe w.r.t. read-modify-write by dlmalloc
// and by us? I think we can -- if there is only one array.
// But if there are multiple threads and multiple arrays sharing a single
// heap then we could have a problem. Solving it might be as simple as
// acquiring a mutex around each read-modify-write and around each call to
// malloc and free.
static Word_t
MyMalloc(Word_t wWords)
{
    Word_t ww = JudyMalloc(wWords + cnMallocExtraWords);
    DBGM(printf("\nM(%zd): %p *%p 0x%zx\n",
                wWords, (void *)ww, (void *)&((Word_t *)ww)[-1],
                ((Word_t *)ww)[-1]));
#if defined(RAMMETRICS)
    j__AllocWordsJLL7 += wWords; // words requested
#endif // defined(RAMMETRICS)

    // Calculate the minimum number of units required to satisfy the request
    // assuming malloc must allocate at least one word for itself.
    size_t zUnitsRequired
        = ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords + 1,
                   EXP(cnBitsMallocMask - cnLogBytesPerWord))
            >> (cnBitsMallocMask - cnLogBytesPerWord);

    // Validate our assumptions about dlmalloc as we prepare to use
    // some of the otherwise unused bits in ww[-1].
    // Our assumption is that the word contains the number of bytes
    // actually allocated or'd with the least significant two bits set.
    // And that the number of bytes allocated is a multiple of
    // EXP(cnBitsMallocMask) no greater than cnExtraUnitsMax units more
    // than the number required.
    // A unit is EXP(cnBitsMallocMask) bytes.
    size_t zUnitsAllocated = ((size_t *)ww)[-1] >> cnBitsMallocMask;

// We don't have a good characterization for cnExtraUnitsMax.
// cnExtraUnitsMax values are based on our observations.
// Are our observations limited by our own malloc request behavior?
#if (cnBitsMallocMask == 3) && (cnBitsPerWord == 32)
  #define cnExtraUnitsMax 2 // 3
#elif (cnBitsMallocMask == 4) && (cnBitsPerWord == 64)
  #define cnExtraUnitsMax 2 // 2
#else
  #define cnExtraUnitsMax 0
#endif // cnBitsMallocMask && cnBitsPerWord

#define cnExtraUnitsBits 2 // number of bits used for saving alloc size
    assert(cnExtraUnitsMax <= (int)MSK(cnExtraUnitsBits));
    // zExtraUnits is the number of extra EXP(cnBitsMallocMask)-byte units over
    // and above the minimum amount that could be allocated by malloc.
    size_t zExtraUnits = zUnitsAllocated - zUnitsRequired; (void)zExtraUnits;
    assert(zExtraUnits <= cnExtraUnitsMax);
#if defined(LIBCMALLOC)
  // We can't use the preamble word with LIBCMALLOC because it monitors the
  // preamble word for changes and kills the process if it detects a change.
  #if defined(LIST_POP_IN_PREAMBLE)
    #error LIST_POP_IN_PREAMBLE with LIBCMALLOC is not supported
  #endif // defined(LIST_POP_IN_PREAMBLE)
#else // defined(LIBCMALLOC)

#define cnBitsUsed 2 // low bits used by malloc for bookkeeping
    assert((((Word_t *)ww)[-1] & MSK(cnBitsMallocMask)) == MSK(cnBitsUsed));
    // Save the bits of ww[-1] that we need at free time and make sure
    // none of the bits we want to use are changed by malloc while we
    // own the buffer.
    ((Word_t *)ww)[-1] &= ~(MSK(cnExtraUnitsBits) << cnBitsUsed);
    ((Word_t *)ww)[-1] |= zExtraUnits << cnBitsUsed;
#if defined(LIST_POP_IN_PREAMBLE)
    // Zero the high bits for our use.
    ((Word_t *)ww)[-1] &= MSK(cnExtraUnitsBits + cnBitsUsed);
#else // defined(LIST_POP_IN_PREAMBLE)
    // Twiddle the bits to illustrate that we can use them.
    ((Word_t *)ww)[-1] ^= (Word_t)-1 << (cnExtraUnitsBits + cnBitsUsed);
#endif // defined(LIST_POP_IN_PREAMBLE)
    DBGM(printf("ww[-1] 0x%zx\n", ((Word_t *)ww)[-1]));
#endif // defined(LIBCMALLOC)
    DBGM(printf("required %zd alloc %zd extra %zd\n",
                zUnitsRequired, zUnitsAllocated, zExtraUnits));
    assert(ww != 0);
    assert((ww & 0xffff000000000000UL) == 0);
    assert((ww & cnMallocMask) == 0);
    ++wMallocs; wWordsAllocated += wWords;
    // ? should we keep track of sub-optimal-size requests ?
    return ww;
}

static void
MyFree(Word_t *pw, Word_t wWords)
{
    DBGM(printf("\nF(pw %p, wWords %zd): pw[-1] 0x%zx\n",
                (void *)pw, wWords, pw[-1]));
    size_t zUnitsRequired
        = ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords + 1,
                   EXP(cnBitsMallocMask - cnLogBytesPerWord))
            >> (cnBitsMallocMask - cnLogBytesPerWord);
    (void)zUnitsRequired;
#ifdef LIBCMALLOC
    size_t zUnitsAllocated = pw[-1] >> cnBitsMallocMask;
    size_t zExtraUnits = zUnitsAllocated - zUnitsRequired;
    (void)zUnitsAllocated; (void)zExtraUnits;
#else // LIBCMALLOC
    assert((pw)[-1] & 2); // lock down what we think we know
    // Restore the value expected by dlmalloc.
    size_t zExtraUnits = (pw[-1] >> cnBitsUsed) & MSK(cnExtraUnitsBits);
    size_t zUnitsAllocated = zUnitsRequired + zExtraUnits;
    pw[-1] &= MSK(cnBitsUsed); // clear high bits
    pw[-1] |= zUnitsAllocated << cnBitsMallocMask;
    DBGM(printf("pw[-1] 0x%zx\n", pw[-1]));
#endif // LIBCMALLOC
    DBGM(printf("required %zd alloc %zd extra %zd\n",
                zUnitsRequired, zUnitsAllocated, zExtraUnits));
    --wMallocs; wWordsAllocated -= wWords;
#if defined(RAMMETRICS)
    j__AllocWordsJLL7 -= wWords; // words requested
#endif // defined(RAMMETRICS)
    JudyFree((RawP_t)pw, wWords + cnMallocExtraWords);
}

#if (cwListPopCntMax != 0)

// How many words are needed for a T_LIST leaf?
// The layout of a list leaf depends on ifdefs and possibly nBL.
// One thing all T_LIST leaves have in common (presently) is an array of keys.
// For OLD_LISTS there may or may not be:
// - a list pop count in the malloc preamble word: LIST_POP_IN_PREAMBLE
// - followed by some dummy words: cnDummiesInList
//   - root list pop count goes in a dummy word: PP_IN_LINK
// - followed by a pop count: ! POP_IN_WR_HB && ! LIST_POP_IN_PREAMBLE
//   && ( ! PP_IN_LINK or list hangs from root word and cnDummiesInList == 0 )
// - followed by a marker key 0 at pxKeys[-1]: LIST_END_MARKERS
// - followed by some padding to align beginning of list: PSPLIT_PARALLEL
// - followed by the array of keys at pxKeys[0] - pxKeys[nPopCnt - 1]
// - followed by replicas of last key to align end of list: PSPLIT_PARALLEL
// - followed by a marker key -1
// - followed by space for more keys: ?
// "New lists" were created when we added parallel search because the
// beginning of the key array had to be aligned and we didn't want to
// waste a whole bucket at the beginning for just a list pop count. So
// we put the list pop count at the end of the malloc buffer.
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

    // Why am I not seeing cnDummiesInList here?
    // Is this related to why PP_IN_LINK doesn't work?
    // I should test if dummies works.

    int nBytesHdr = (N_LIST_HDR_KEYS + POP_SLOT(nBL)) * nBytesKeySz;
    if (ALIGN_LIST(nBytesKeySz))
    {
        if ((cnMallocMask + 1) < sizeof(Bucket_t)) {
            // We don't know what address we are going to get from malloc.
            // We have to allocate enough memory to ensure that we will be able
            // to align the beginning of the array of real keys.
            nBytesHdr += sizeof(Bucket_t) - (cnMallocMask + 1);
            nBytesHdr = ALIGN_UP(nBytesHdr, sizeof(Bucket_t) - (cnMallocMask + 1));
        } else {
            nBytesHdr = ALIGN_UP(nBytesHdr, sizeof(Bucket_t));
        }
    }

    int nBytesKeys = wPopCntArg * nBytesKeySz; // add list of real keys

    // Pad array of keys so the end is aligned.
    // We'll eventually fill the padding with replicas of the last real key
    // so parallel searching yields no false positives.
    if (ALIGN_LIST_LEN(nBytesKeySz))
    {
#if defined(UA_PARALLEL_128)
        if ((nBL == 16) && (wPopCntArg <= 6)) {
            // UA_PARALLEL_128 makes a lot of assumptions.
            // It uses a 96-bit (12 byte) parallel search.
            // E.g. six keys fit in a three 32-bit-word leaf.
            assert(nBytesHdr == 0);
            nBytesKeys = 12;
        } else
#endif // defined(UA_PARALLEL_128)
        { nBytesKeys = ALIGN_UP(nBytesKeys, sizeof(Bucket_t)); }
    }

#if defined(LIST_END_MARKERS)
    // Make room for -1 at the end to help make search faster.
    // The marker at the beginning is accounted for in N_LIST_HDR_KEYS.
    // How should we handle LIST_END_MARKERS for parallel searches?
    nBytesKeys += nBytesKeySz;
#endif // defined(LIST_END_MARKERS)

    int nBytes = nBytesHdr + nBytesKeys;

#if defined(LIST_REQ_MIN_WORDS)
    return DIV_UP(nBytes, sizeof(Word_t));
#else // defined(LIST_REQ_MIN_WORDS)
    // Round up to full malloc chunk which is some odd number of words.
    // Malloc always allocates an integral number of MALLOC_ALIGNMENT-size
    // units with the second word aligned and uses the first word for itself.
    // Malloc never allocates less than four words.
    nBytes += sizeof(Word_t); // add a word for malloc
    return MAX(4, ALIGN_UP(nBytes, cnMallocMask + 1) >> cnLogBytesPerWord) - 1;
#endif // defined(LIST_REQ_MIN_WORDS)

#else // defined(OLD_LISTS)
    return MAX(ls_nSlotsInList(wPopCntArg, nBL, nBytesKeySz)
                 * nBytesKeySz / sizeof(Word_t) | 1, 3);
#endif // defined(OLD_LISTS)
}

// How many words needed for external leaf?
// Do not embed.
static unsigned
ListWordsExternal(Word_t wPopCnt, unsigned nBL)
{
    return ListWordsTypeList(wPopCnt, nBL);
}

// How many words are needed for the specified list leaf?
// Use embedded keys instead of T_LIST if possible.
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

#if JUNK
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
        ls_pcKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL1 += nWords); // 1 byte/key list leaf
    } else if (nBL <= 16) {
#if defined(LIST_END_MARKERS)
        ls_psKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL2 += nWords); // 2 bytes/key list leaf
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
#if defined(LIST_END_MARKERS)
        ls_piKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLL4 += nWords); // 4 bytes/key list leaf
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
#if defined(LIST_END_MARKERS)
#if defined(PP_IN_LINK) && (cnDummiesInList == 0)
        // ls_pwKeys is for T_LIST not at top (it incorporates dummies
        // and markers, but not pop count)
        ls_pwKeysNAT(pwList)[-1 + (nBL == cnBitsPerWord)] = 0;
#else // defined(PP_IN_LINK) && (cnDummiesInList == 0)
        ls_pwKeysNAT(pwList)[-1] = 0;
#endif // defined(PP_IN_LINK) && (cnDummiesInList == 0)
#endif // defined(LIST_END_MARKERS)
        METRICS(j__AllocWordsJLLW += nWords); // 1 word/key list leaf
    }

    // Should we be setting wPrefix here for PP_IN_LINK?

    DBGM(printf("NewListCommon pwList %p wPopCnt " OWx" nBL %d nWords %d\n",
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

    NewListCommon(pwList, wPopCnt, nBL, nWords);

    return pwList;
}

static Word_t *
NewListExternal(Word_t wPopCnt, unsigned nBL)
{
    assert(wPopCnt != 0);
    return NewListTypeList(wPopCnt, nBL);
}

// Allocate memory for a new list for the given wPopCnt.
// Use an embedded list if possible.
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
#if defined(EMBED_KEYS)
    if (nType == T_EMBEDDED_KEYS) { DBGR(printf("OL: 0.\n")); return 0; }
#endif // defined(EMBED_KEYS)

    int nWords = ( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
            || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
            ) ? ListWordsTypeList(nPopCnt, nBL) : ListWords(nPopCnt, nBL);

    DBGM(printf("Old pwList %p wLen %d nBL %d nPopCnt %d nType %d\n",
        (void *)pwList, nWords, nBL, nPopCnt, nType));

    if (nWords == 0) { return 0; }

#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        METRICS(j__AllocWordsJLL1 -= nWords); // 1 byte/key list leaf
    } else if (nBL <= 16) {
        METRICS(j__AllocWordsJLL2 -= nWords); // 2 bytes/key list leaf
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        METRICS(j__AllocWordsJLL4 -= nWords); // 4 bytes/key list leaf
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
        METRICS(j__AllocWordsJLLW -= nWords); // 1 word/key list leaf
    }

#if ! defined(OLD_LISTS)
    assert(nType == T_LIST);
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
        // Use LL3 column for B2 big bitmap leaf.
        METRICS(j__AllocWordsJLL3 += wWords);
    } else {
        METRICS(j__AllocWordsJLB1 += wWords); // bitmap leaf
    }

    DBGM(printf("NewBitmap nBL %d nBits " OWx
      " nBytes " OWx" wWords " OWx" pwBitmap " OWx"\n",
        nBL, EXP(nBL), EXP(nBL - cnLogBitsPerByte), wWords,
        (Word_t)pwBitmap));

    memset((void *)pwBitmap, 0, wWords * sizeof(Word_t));

    // Init wRoot before calling set_wr because set_wr may try to preserve
    // the high bits, e.g. if LVL_IN_WR_HB, so we want them to
    // be initialized.
    Word_t wRoot = 0; set_wr(wRoot, pwBitmap, T_BITMAP);

#if defined(SKIP_TO_BITMAP)
    Set_nBLR(&wRoot, nBL);
    if (nBLUp != nBL) {
//printf("\nNewBitmap: wKey " OWx" pwBitmap %p nBL %d &wPrefixPop %p wPrefixPop (before) " OWx"\n", wKey, (void *)pwBitmap, nBL, (void *)(pwBitmap + EXP(nBL - cnLogBitsPerWord)), *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));
    }
    set_w_wPrefixBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, wKey);
    if (nBLUp != nBL) {
//printf("After set_w_wPrefixBL: wPrefixPop " OWx"\n", *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));
        set_wr_nType(wRoot, T_SKIP_TO_BITMAP);
    }
#else // defined(SKIP_TO_BITMAP)
    assert(nBLUp == nBL);
#endif // defined(SKIP_TO_BITMAP)
    set_w_wPopCntBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, 0);
//printf("After set_w_wPopCntBL: wPrefixPop " OWx"\n", *(pwBitmap + EXP(nBL - cnLogBitsPerWord)));

    *pwRoot = wRoot;

    return pwBitmap;
}

static int
GetBLR(Word_t *pwRoot, int nBL)
{
    (void)pwRoot;

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) { return nBL; }
#endif // defined(NO_TYPE_IN_XX_SW)

    return
  #if defined(SKIP_LINKS)
        ((tp_bIsSwitch(wr_nType(*pwRoot)) && tp_bIsSkip(wr_nType(*pwRoot)))
      #if defined(SKIP_TO_BITMAP)
                || (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP)
      #endif // defined(SKIP_TO_BITMAP)
                || 0)
            ? wr_nBL(*pwRoot) :
  #endif // defined(SKIP_LINKS)
              nBL ;
}

static Word_t
OldBitmap(Word_t *pwRoot, Word_t *pwr, int nBL)
{
    int nBLR = GetBLR(pwRoot, nBL);

    Word_t wWords = EXP(nBLR - cnLogBitsPerWord) + 1;

    MyFree(pwr, wWords);

    if (nBLR == nDL_to_nBL(2)) {
        // Use LL3 column for B2 big bitmap leaf.
        METRICS(j__AllocWordsJLL3 -= wWords); // B2 big bitmap leaf
    } else {
        METRICS(j__AllocWordsJLB1 -= wWords); // bitmap leaf
    }

#if defined(PP_IN_LINK) && defined(SKIP_TO_BITMAP)
    if ((nBL != cnBitsPerWord)
        && (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP))
    {
        // We have to zero some higher bits so cleanup knows what to do.
        //printf("here pwRoot %p nBL %d\n", (void *)pwRoot, nBL);
        set_PWR_wPopCntBL(pwRoot, NULL, nBL, 0);
    }
#endif // defined(PP_IN_LINK) && defined(SKIP_TO_BITMAP)

    *pwRoot = 0; // Do we need to clear the rest of the link, e.g. PP_IN_LINK?

    return wWords * sizeof(Word_t);
}

// Allocate a new switch.
// Zero its links.
// Initialize its prefix if there is one.  Need to know nDLUp for
// PP_IN_LINK to figure out if the prefix field exists.
// Initialize its bitmap if there is one.  Need to know nDLUp for
// BM_IN_LINK to figure out if the bitmap field exists.
// Need to know nDLUp to know if we need a skip link.
// Install wRoot at pwRoot.  Need to know nDL.
// Account for the memory in Judy1LHTime.
// Need to know if we are at the bottom so we can count the memory as a
// bitmap leaf instead of a switch.
static Word_t *
NewSwitch(Word_t *pwRoot, Word_t wKey, int nBL,
#if defined(CODE_XX_SW)
          int nBitsIndexSzX,
#endif // defined(CODE_XX_SW)
          int nType,
          int nBLUp, Word_t wPopCnt)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot; (void)wRoot;
    (void)nType; // Used only if CODE_BM_SW?
    DBGI(printf("NewSwitch: pwRoot %p wKey " OWx" nBL %d nBLUp %d"
         " wPopCnt %" _fw"d.\n", (void *)pwRoot, wKey, nBL, nBLUp, wPopCnt));

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

#if ! defined(ALLOW_EMBEDDED_BITMAP)
    // Should we check here to see if the new switch would be equivalant to a
    // bitmap leaf and create a bitmap leaf instead?
  #if defined(CODE_BM_SW)
    assert((nType != T_SWITCH)
        || (nBL - nBitsIndexSz > (int)LOG(sizeof(Link_t) * 8)));
  #else // defined(CODE_BM_SW)
    assert(nBL - nBitsIndexSz > (int)LOG(sizeof(Link_t) * 8));
  #endif // defined(CODE_BM_SW)
#endif // ! defined(ALLOW_EMBEDDED_BITMAP)

#if defined(CODE_BM_SW)
    if (nType == T_BM_SW)
#if defined(BM_IN_LINK)
    { Link_t ln; assert(wIndexCnt <= sizeof(ln.ln_awBm) * cnBitsPerByte); }
#else // defined(BM_IN_LINK)
    {
         BmSwitch_t sw; (void)sw;
         assert(wIndexCnt <= sizeof(sw.sw_awBm) * cnBitsPerByte);
    }
#endif // defined(BM_IN_LINK)
#endif // defined(CODE_BM_SW)

    Word_t wLinks = wIndexCnt;

#if defined(BM_SW_FOR_REAL)
    if (nType == T_BM_SW)
    {
  #if defined(BM_IN_LINK)
        if (nBLUp != cnBitsPerWord)
  #endif // defined(BM_IN_LINK)
        {
            wLinks = 1; // number of links in switch
        }
    }
#endif // defined(BM_SW_FOR_REAL)

// cnMaskLsSwDL is for testing.
#if defined(USE_LIST_SW)
  #if !defined(cnMaskLsSwDL)
    #define cnMaskLsSwDL  0
  #endif // !defined(cnMaskLsSwDL)
    int bLsSw = ((1 << nBL_to_nDL(nBL)) & cnMaskLsSwDL);
#endif // defined(USE_LIST_SW)

    Word_t wBytes =
#if defined(CODE_BM_SW)
        (nType == T_BM_SW) ? sizeof(BmSwitch_t) :
#endif // defined(CODE_BM_SW)
#if defined(USE_LIST_SW)
  #if defined(CODE_XX_SW)
    #error CODE_XX_SW with USE_LIST_SW is not allowed yet
  #endif // defined(CODE_XX_SW)
  #if defined(CODE_BM_SW)
    #error CODE_BM_SW with USE_LIST_SW is not allowed yet
  #endif // defined(CODE_BM_SW)
        bLsSw ? sizeof(ListSw_t) :
#endif // defined(USE_LIST_SW)
        sizeof(Switch_t);

    // sizeof([Bm]Switch_t) includes one link; add the others
    wBytes += (wLinks - 1) * sizeof(Link_t);
    Word_t wWords = wBytes / sizeof(Word_t);

    Word_t *pwr = (Word_t *)MyMalloc(wWords);
    set_wr_pwr(wRoot, pwr);
    *pwRoot = wRoot;

#if defined(USE_LIST_SW)
    if (bLsSw) {
        snListSwPop(qy, wLinks);
        wRoot = *pwRoot; // preserve qy invariants
    }
#endif // defined(USE_LIST_SW)

    Link_t *pLinks =
#if defined(CODE_BM_SW)
        (nType == T_BM_SW) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
#if defined(USE_LIST_SW)
        bLsSw ? gpListSwLinks(qy) :
#endif // defined(USE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr);
    memset(pLinks, 0, wLinks * sizeof(Link_t));
#if defined(NO_TYPE_IN_XX_SW)
  #if !defined(USE_XX_SW)
    #error NO_TYPE_IN_XX_SW without USE_XX_SW is a problem
  #endif // !defined(USE_XX_SW)
    // The links in a switch at nDL_to_nBL(2) have nBL < nDL_to_nBL(2).
    // Hence the '=' in the '<=' here.
    if (nBL <= nDL_to_nBL(2)) {
        DBGI(printf("NS: Init ln_wRoots.\n"));
        for (int nn = 0; nn < (int)wLinks; ++nn) {
            pLinks[nn].ln_wRoot = ZERO_POP_MAGIC;
        }
        DBGI(printf("NS: Done init ln_wRoots.\n"));
    }
#endif // defined(NO_TYPE_IN_XX_SW)

    // Is a branch with embedded bitmaps a branch?
    // Or is it a bitmap?  Let's use bitmap since we get more info that way.
    // No need for ifdef RAMMETRICS. Code will go away if not.
    if (nBL <= (int)LOG(sizeof(Link_t) * 8)) {
        // Embedded bitmaps.
        // What if we have bits in the links that are not used as
        // bits in the bitmap?
        METRICS(j__AllocWordsJLL3 += wWords); // B2 big bitmap leaf
    } else
#if defined(CODE_BM_SW)
    if (nType == T_BM_SW) {
        METRICS(j__AllocWordsJBB += wWords); // bitmap branch
    } else
#endif // defined(CODE_BM_SW)
    { METRICS(j__AllocWordsJBU += wWords); } // uncompressed branch

#if defined(CODE_BM_SW)
    DBGM(printf("NewSwitch(pwRoot %p wKey " OWx
                    " nBL %d nType %d nBLU %d wPopCnt %ld)"
                    " pwr %p\n",
                (void *)pwRoot, wKey,
                nBL, nType, nBLUp, (long)wPopCnt, (void *)pwr));
#endif // defined(CODE_BM_SW)
    DBGI(printf("\nNewSwitch nBL %d nDL %d nBLUp %d\n",
                nBL, nBL_to_nDL(nBL), nBLUp));

#if defined(CODE_BM_SW)
    if (nType == T_BM_SW) {
  #if defined(SKIP_TO_BM_SW)
        if (nBL != nBLUp) {
            DBGI(printf("\nCreating T_SKIP_TO_BM_SW!\n"));
            set_wr_pwr(*pwRoot, pwr);
            set_wr_nBL(*pwRoot, nBL); // set nBL
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
        Set_nBLR(pwRoot, nBL);
  #endif // defined(SKIP_TO_XX_SW)
        set_pwr_nBW(pwRoot, nBitsIndexSz);
#endif // defined(CODE_XX_SW)
#if defined(NO_SKIP_AT_TOP)
        assert((nBLUp < cnBitsPerWord) || (nBL == nBLUp));
#endif // defined(NO_SKIP_AT_TOP)
        assert(nBL <= nBLUp);
#if defined(SKIP_LINKS)
        if (nBL < nBLUp) {
            set_wr_nBL(*pwRoot, nBL); // also sets nType == T_SKIP_TO_SWITCH
  #if defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
            if (nBL <= nDL_to_nBL(2)) {
                set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
            }
  #endif // defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
  #if defined(USE_LIST_SW)
      #if !defined(SKIP_TO_LIST_SW)
        #error No SKIP_TO_LIST_SW with USE_LIST_SW is not supported yet.
      #endif // !defined(SKIP_TO_LIST_SW)
            if (bLsSw) { set_wr_nType(*pwRoot, T_SKIP_TO_LIST_SW); }
  #endif // defined(USE_LIST_SW)
        } else
#endif // defined(SKIP_LINKS)
        {
  #if defined(USE_XX_SW)
            if (nBL <= nDL_to_nBL(2)) {
                set_wr_nType(*pwRoot, T_XX_SW);
            } else
  #endif // defined(USE_XX_SW)
            {
                set_wr_nType(*pwRoot,
  #if defined(USE_LIST_SW)
                             bLsSw ? T_LIST_SW :
  #endif // defined(USE_LIST_SW)
                             T_SWITCH);
            }
        }
    }

    wRoot = *pwRoot; // for qy; ? is this assignment backwards ?
#if defined(USE_LIST_SW)
    if (bLsSw) {
        // initialize the list of keys
        for (int nn = 0; nn < (int)wLinks; nn++) {
            ((ListSw_t *)pwr)->sw_aKeys[nn] = nn;
        }
    }
#endif // defined(USE_LIST_SW)
#if defined(CODE_BM_SW)
    if (nType == T_BM_SW)
    {
        // initialize the bitmap
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
            DBGI(printf("Not installing prefix left"
                            " nBL %d nBLUp %d wKey " OWx"\n",
                        nBL, nBLUp, wKey));

            set_PWR_wPrefixBL(pwRoot, pwr, nBL, 0);
        }
        else
#endif // defined(NO_UNNECESSARY_PREFIX)
        {
#if defined(CODE_BM_SW)
            if (nType == T_BM_SW) {
                set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL, wKey);
            } else
#endif // defined(CODE_BM_SW)
            {
                set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL, wKey);
            }
        }
#else // defined(SKIP_LINKS)

#if 0
        // Why do we bother with this?  Should we make it debug only?
#if defined(CODE_BM_SW)
        if (nType == T_BM_SW) {
            set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL, 0);
        } else
#endif // defined(CODE_BM_SW)
        {
            set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL, 0);
        }
#endif // defined(SKIP_LINKS)
#endif

#if defined(CODE_BM_SW)
        if (nType == T_BM_SW) {
            set_PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBL, wPopCnt);
        } else
#endif // defined(CODE_BM_SW)
        {
            set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBL, wPopCnt);
        }

#if defined(CODE_BM_SW)
        DBGM(printf("NewSwitch PWR_wPrefixPop " OWx"\n",
            (nType == T_BM_SW) ? PWR_wPrefixPop(pwRoot, (BmSwitch_t *)pwr)
                  : PWR_wPrefixPop(pwRoot, (Switch_t *)pwr)));
#endif // defined(CODE_BM_SW)
    }

    //DBGI(printf("After NewSwitch"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
DBGI(printf("NS: prefix " OWx"\n", PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL)));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nBL; // nBL is not used for all ifdef combos
    (void)nBLUp; // nBLUp is not used for all ifdef combos
}

#if defined(CODE_BM_SW)
#if defined(BM_SW_FOR_REAL)
static Word_t
OldSwitch(Word_t *pwRoot, int nBL, int bBmSw, int nLinks, int nBLUp);

// Uncompress a bitmap switch.
void
InflateBmSw(Word_t *pwRoot, Word_t wKey, int nBLR, int nBLUp)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);

    DBGI(printf("# InflateBmSw wKey " Owx" nBLR %d\n", wKey, nBLR));

    int nBW = nBL_to_nBW(nBLR);

    Word_t *pwrNew = NewSwitch(pwRoot, wKey, nBLR,
  #if defined(CODE_XX_SW)
                               nBW,
  #endif // defined(CODE_XX_SW)
                               T_SWITCH, nBLUp,
                               PWR_wPopCntBL(pwRoot,
                                             (BmSwitch_t *)pwr, nBLR));
    // NewSwitch installed the new wRoot at pwRoot.
    // What about PWR_pwBm(pwRoot, pwr) which we use below?
    // If BM_IN_LINK it will use pwRoot to find the bitmap.
    // How do we know NewSwitch hasn't overwritten the bitmap?
    // I guess we should make a copy of the link before calling NewSwitch.

    int nLinkCnt = 0; // link number in bm sw
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
        if (BitIsSet(PWR_pwBm(pwRoot, pwr), nn)) {
            pwr_pLinks((Switch_t *)pwrNew)[nn]
                = pwr_pLinks((BmSwitch_t *)pwr)[nLinkCnt];
            ++nLinkCnt;
        }
    }

    OldSwitch(&wRoot, nBLR, /* bBmSw */ 1, nLinkCnt, nBLUp);
}

static void
NewLink(Word_t *pwRoot, Word_t wKey, int nDLR, int nDLUp)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);
    int nBLR = nDL_to_nBL(nDLR);
    int nBLUp = nDL_to_nBL(nDLUp);

    DBGI(printf("NewLink(pwRoot %p wKey " OWx" nBLR %d)\n",
        (void *)pwRoot, wKey, nBLR));
    DBGI(printf("PWR_wPopCnt %" _fw"d\n",
         PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));

#if defined(BM_IN_LINK)
    assert(nBLR != cnBitsPerWord);
#endif // defined(BM_IN_LINK)

    // What is the index of the new link?
    unsigned nBitsIndexSz = nBL_to_nBitsIndexSz(nBLR);
    Word_t wIndex
        = ((wKey >> (nBLR - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1));
    DBGI(printf("wKey " OWx" nBLR %d nBitsIndexSz %d wIndex (virtual) " OWx"\n",
                wKey, nBLR, nBitsIndexSz, wIndex));

    // How many links are there in the old switch?
    int nLinkCnt = 0;
    for (unsigned nn = 0; nn < DIV_UP(EXP(nBitsIndexSz), cnBitsPerWord); nn++)
    {
        nLinkCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]);
    }
    // Now we know how many links were in the old switch.

    // sizeof(BmSwitch_t) includes one link; add the others
    unsigned nWordsOld
         = (sizeof(BmSwitch_t) + (nLinkCnt - 1) * sizeof(Link_t))
            / sizeof(Word_t);
    DBGI(printf("nLinkCnt %d nWordsOld %d\n", nLinkCnt, nWordsOld));

    // What criteria should we use to decide when to uncompress a bitmap
    // switch?
    //
    // An uncompressed switch should not use more memory than would be used
    // by all of the keys below it if all of the keys below it were in a
    // leaf just below the switch? This is a local criteria.
    // A switch uses about EXP(nBW) * sizeof(Link_t) bytes.
    // The keys below would use about nKeys * EXP(nBL) / 8 bytes.
    // Uncompress when nKeys * EXP(nBL) / 8 > EXP(nBW) * sizeof(Link_t).
    // For a switch with 256 64-bit links we'd need about 2K one-byte keys.
    // We'd need about 1K two-byte keys.
    // We'd need about 512 four-byte keys.
    // We'd need about 256 eight-byte keys.
    // With -E we get 16 keys * 16 links in the bottom switch. Not enough.
    // We get 16 keys * 16 links * 16 links in the next-to-bottom switch.
    // Plenty.

    // Does this include the key were inserting now?  I think it does.
    Word_t wPopCntKeys = PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
    (void)wPopCntKeys;

    // Should we consider uncompressing here? Or rely on InsertCleanup?
    // It's probably more efficient to do it here, but InsertCleanup will
    // probably end up doing it before we have a chance to do it here
    // the vast majority of the time.
    if (wPopCntKeys * nBLR * cnBmSwConvert
            > EXP(nBitsIndexSz) * 8 * sizeof(Link_t) * cnBmSwRetain) {
        InflateBmSw(pwRoot, wKey, nBLR, nBLUp);
    } else {
        // We replicate a bunch of newswitch here since
        // newswitch can create only empty bm sw.

        // Allocate memory for a new switch with one more link than the
        // old one.
        unsigned nWordsNew = nWordsOld + sizeof(Link_t) / sizeof(Word_t);
        *pwRoot = MyMalloc(nWordsNew);
        DBGI(printf("After malloc *pwRoot " OWx"\n", *pwRoot));

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
        DBGI(printf("wIndex (physical) " OWx"\n", wIndex));

        // Copy the old switch to the new switch and insert the new link.
        memcpy(wr_pwr(*pwRoot), pwr,
            sizeof(BmSwitch_t) + (wIndex - 1) * sizeof(Link_t));
        DBGI(printf("PWR_wPopCnt %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // Initialize the new link.
        DBGI(printf("pLinks %p\n",
                    (void *)pwr_pLinks((BmSwitch_t *)*pwRoot)));
        DBGI(printf("memset %p\n",
                    (void *)&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex]));
        memset(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex], 0, sizeof(Link_t));
        DBGI(printf("PWR_wPopCnt A %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        memcpy(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex + 1],
               &pwr_pLinks((BmSwitch_t *)pwr)[wIndex],
            (nLinkCnt - wIndex) * sizeof(Link_t));

        DBGI(printf("PWR_wPopCnt B %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // Set the bit in the bitmap indicating that the new link exists.
        SetBit(PWR_pwBm(pwRoot, *pwRoot),
            ((wKey >> (nBLR - nBitsIndexSz)) & (EXP(nBitsIndexSz) - 1)));
        DBGI(printf("PWR_wPopCnt %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));

        if (nBLR <= (int)LOG(sizeof(Link_t) * 8)) {
            METRICS(j__AllocWordsJLB1 += nWordsNew); // bitmap leaf
        } else
#if defined(RETYPE_FULL_BM_SW)
        if (nLinkCnt == EXP(nBitsIndexSz) - 1) {
  #if defined(DEBUG_INSERT)
            if ( ! (EXP(nBLR) & sBitsReportedMask) )
            {
                sBitsReportedMask |= EXP(nBLR);
                printf("# Retyping full BM_SW nKeys %ld nLinks %ld nBLR %d",
                       wPopCntKeys, nLinkCnt, nBLR);
                printf(" wPopCntTotal %ld wWordsAllocated %ld",
                       wPopCntTotal, wWordsAllocated);
                printf(" wMallocs %ld nWordsNull %d\n",
                       wMallocs, nWordsNull);
            }
  #endif // defined(DEBUG_INSERT)
            METRICS(j__AllocWordsJBU  += nWordsNew); // uncompressed branch
        } else
#endif // defined(RETYPE_FULL_BM_SW)
        {
            METRICS(j__AllocWordsJBB  += nWordsNew); // bitmap branch
        }

        // Update the type field in *pwRoot if necessary.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #if defined(RETYPE_FULL_BM_SW)
        if (nLinkCnt == EXP(nBitsIndexSz) - 1) {
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
                set_wr_nBL(*pwRoot, nBLR);
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
                set_wr_nBL(*pwRoot, nBLR);
                assert(wr_nBL(*pwRoot) == wr_nBL(wRoot));
            }
      #endif // defined(SKIP_TO_BM_SW)
  #endif // defined(LVL_IN_WR_HB)
            set_wr_nType(*pwRoot, nType);

        }
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

    // &wRoot won't cut it for BM_IN_LINK.
    OldSwitch(&wRoot, nBLR, /* bBmSw */ 1, nLinkCnt, nBLUp);
    }

    //DBGI(printf("After NewLink"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

static Word_t
OldSwitch(Word_t *pwRoot, int nBL,
#if defined(CODE_BM_SW)
          int bBmSw,
          int nLinks, // 0 means calculate
#endif // defined(CODE_BM_SW)
          int nBLUp)
{
#if defined(CODE_BM_SW) && ! defined(BM_SW_FOR_REAL)
    (void)nLinks;
#endif // defined(CODE_BM_SW) && ! defined(BM_SW_FOR_REAL)
    int nType = wr_nType(*pwRoot); (void)nType;
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
        if (nLinks == 0) {
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
        } else {
            wLinks = nLinks;
        }
    }
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

    Word_t wBytes =
#if defined(CODE_BM_SW)
        bBmSw ? sizeof(BmSwitch_t) :
#endif // defined(CODE_BM_SW)
#if defined(USE_LIST_SW)
        ((nType == T_LIST_SW) || (nType == T_SKIP_TO_LIST_SW))
            ? sizeof(ListSw_t) :
#endif // defined(USE_LIST_SW)
        sizeof(Switch_t);
    // sizeof([Bm]Switch_t) includes one link; add the others
    wBytes += (wLinks - 1) * sizeof(Link_t);
    Word_t wWords = wBytes / sizeof(Word_t);

    // No need for ifdef RAMMETRICS. Code will go away if not.
    if (nBL <= (int)LOG(sizeof(Link_t) * 8)) {
        // Embedded bitmaps.
        METRICS(j__AllocWordsJLL3 -= wWords); // B2 big bitmap leaf
    } else
#if defined(CODE_BM_SW)
    if (tp_bIsBmSw(wr_nType(*pwRoot))
  #if defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        && (wr_nType(*pwRoot) != T_FULL_BM_SW)
  #endif // defined(RETYPE_FULL_BM_SW) && ! defined(BM_IN_NON_BM_SW)
        )
    {
        METRICS(j__AllocWordsJBB  -= wWords); // bitmap branch
    } else
#endif // defined(CODE_BM_SW)
    { METRICS(j__AllocWordsJBU  -= wWords); } // uncompressed branch

    DBGR(printf("\nOldSwitch nBL %d nBLU %d wWords %" _fw"d " OWx"\n",
         nBL, nBLUp, wWords, wWords));

    MyFree(pwr, wWords);

    return wWords * sizeof(Word_t);

    (void)nBL; // silence compiler
    (void)nBLUp; // silence compiler
}

// Get the pop count of the tree/subtree represented by (*pwRoot, nBL).
static Word_t
GetPopCnt(Word_t *pwRoot, int nBL)
{
    int nBLR = GetBLR(pwRoot, nBL); // handles skip -- or not

#if defined(NO_TYPE_IN_XX_SW)
    if (nBLR < nDL_to_nBL(2)) { return wr_nPopCnt(*pwRoot, nBL); }
#endif // defined(NO_TYPE_IN_XX_SW)

#if defined(PP_IN_LINK)
    if (nBL == cnBitsPerWord)
#endif // ! defined(PP_IN_LINK)
    {
        // PWR_wPopCntBL handles only switches if ! PP_IN_LINK.
        // Would be nice to fix it.
        // But we have to handle the other types at the top anyway.
        if (*pwRoot == 0) { return 0; }
        int nType = wr_nType(*pwRoot);
#if defined(EMBED_KEYS)
        if (nType == T_EMBEDDED_KEYS) { return wr_nPopCnt(*pwRoot, nBL); }
#endif // defined(EMBED_KEYS)
        if ( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
            || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
            )
        {
            return PWR_xListPopCnt(pwRoot, wr_pwr(*pwRoot), nBL);
        }
        if ((nType == T_BITMAP)
  #if defined(SKIP_TO_BITMAP)
            || (nType == T_SKIP_TO_BITMAP)
  #endif // defined(SKIP_TO_BITMAP)
            || 0)
        {
            Word_t wPP = *(wr_pwr(*pwRoot) + EXP(nBLR - cnLogBitsPerWord));
            Word_t wPopCnt = w_wPopCntBL(wPP, nBLR);
            return (wPopCnt == 0) ? EXP(nBLR) : wPopCnt ;
        }
        assert(tp_bIsSwitch(nType));
    }

    Word_t wPopCnt =
      #if defined(CODE_BM_SW)
        tp_bIsBmSw(wr_nType(*pwRoot))
            ? PWR_wPopCntBL(pwRoot, (BmSwitch_t *)wr_pwr(*pwRoot), nBLR) :
      #endif // defined(CODE_BM_SW)
              PWR_wPopCntBL(pwRoot, (  Switch_t *)wr_pwr(*pwRoot), nBLR) ;

    // PWR_wPopCnt will be zero if the subtree is full or empty.
    // We use wRoot != 0 to disambiguate PWR_wPopCnt == 0.
    // Hence we cannot allow Remove to leave
    // wRoot != 0 unless the actual pop count is not zero.
    if ((wPopCnt == 0) && (*pwRoot != 0)) {
         return EXP(nBLR);
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

    int nBitsIndexSz;
#if defined(CODE_XX_SW)
    if (tp_bIsXxSw(wr_nType(*pwRoot))) {
        nBitsIndexSz = pwr_nBW(pwRoot);
    } else
#endif // defined(CODE_XX_SW)
    { nBitsIndexSz = nBL_to_nBitsIndexSz(nBL); }

    Word_t wPopCnt = 0;
    Word_t xx = 0;
    for (int nn = 0; nn < (int)EXP(nBitsIndexSz); nn++)
    {
#if defined(CODE_BM_SW)
        if ( ! tp_bIsBmSw(wr_nType(*pwRoot))
                    || BitIsSet(PWR_pwBm(pwRoot, wr_pwr(*pwRoot)), nn) )
#endif // defined(CODE_BM_SW)
        {
            wPopCnt += GetPopCnt(&pLinks[xx++].ln_wRoot, nBL - nBitsIndexSz);
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
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
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
        DBGR(printf("FreeArrayGuts pwR " OWx" wPrefix " OWx" nBL %d bDump %d\n",
             (Word_t)pwRoot, wPrefix, nBL, bDump));
        DBGR(printf("wRoot " OWx"\n", wRoot));
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
            printf(" wPrefix " OWx, wPrefix);
            printf(" nBL %2d", nBL);
            printf(" pwRoot " OWx, (Word_t)pwRoot);
            printf(" wr " OWx, wRoot);
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
            printf(" wPrefix " OWx, wPrefix);
            printf(" nBL %2d", nBL);
            printf(" pwRoot " OWx, (Word_t)pwRoot);
            printf(" wr " OWx, wRoot);
            printf("\n");
        }
#endif // defined(BM_SW_FOR_REAL)
        return 0;
    }

    if (bDump)
    {
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        printf(" wPrefix " OWx, wPrefix);
        printf(" nBL %2d", nBL);
        printf(" pwRoot " OWx, (Word_t)pwRoot);
        printf(" wr " OWx, wRoot);
    }

#if defined(SKIP_TO_BITMAP)
    if (nType == T_SKIP_TO_BITMAP) {
        if (bDump) {
            int nBLR = nBL;
            assert(tp_bIsSkip(nType));
            if (tp_bIsSkip(nType)) {
                nBLR = GetBLR(pwRoot, nBL);
            }
            printf(" wPrefixPop " OWx, *(pwr + EXP(nBLR - cnLogBitsPerWord)));
            Word_t wPopCnt = w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR);
            if (wPopCnt == 0) {
                wPopCnt = EXP(nBLR);
            }
            printf(" w_wPopCnt %" _fw"d", wPopCnt);
            printf(" skip to bitmap\n");
            printf(" nWords %4" _fw"d", EXP(nBLR - cnLogBitsPerWord));
            for (Word_t ww = 0; (ww < EXP(nBLR - cnLogBitsPerWord)); ww++) {
                if ((ww % 8) == 0) {
                    printf("\n");
                }
                printf(" " OWx, pwr[ww]);
            }
            printf("\n");
            return 0;
        }

        assert((nBL != cnBitsPerWord) || (wBytes == 0)); // wPopCntTotal, zeroLink
        assert((nBL != cnBitsPerWord) || (wr_pwr(*pwRootArg) == pwr));
        wBytes += OldBitmap(pwRoot, pwr, nBL);
        // OldBitmap zeros *pwRoot but not the rest of the link.
        return wBytes;
    }
#endif // defined(SKIP_TO_BITMAP)

    if ((nType == T_BITMAP)
        || ((nBL < cnBitsPerWord) && (EXP(nBL) <= sizeof(Link_t) * 8)))
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
                Word_t wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
                printf(" wr_wPopCnt %3" _fw"u",
                       wPopCnt != 0 ? wPopCnt : EXP(nBL));
                printf(" wr_wPrefix " OWx,
                       PWR_wPrefixBL(pwRoot, (Switch_t *)NULL, nBL));
            }
        }
#endif // defined(PP_IN_LINK)

        // If the bitmap is not embedded, then we have more work to do.
        // The test can be done at compile time and will make one the
        // other clauses go away.
        if (EXP(nBL) > sizeof(Link_t) * 8)
        {
            if ( ! bDump )
            {
                assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
                return OldBitmap(pwRoot, pwr, nBL);
            }

            printf(" nWords %4" _fw"d", EXP(nBL - cnLogBitsPerWord));
            for (Word_t ww = 0; (ww < EXP(nBL - cnLogBitsPerWord)); ww++) {
                if ((ww % 8) == 0) {
                    printf("\n");
                }
                printf(" " OWx, pwr[ww]);
            }
        }
        else
        {
            if (bDump) {
                if (cnBitsInD1 > cnLogBitsPerWord) {
                    printf(" nWords %4" _fw"d", EXP(nBL - cnLogBitsPerWord));
                    for (Word_t ww = 0;
                         (ww < EXP(nBL - cnLogBitsPerWord)); ww++)
                    {
                        if ((ww % 8) == 0) { printf("\n"); }
                        printf(" " OWx,
                               ((Word_t *)
                                   STRUCT_OF(pwRoot, Link_t, ln_wRoot))[ww]);
                    }
                } else {
                    printf(" wr " OWx, wRoot);
                }
            }
        }

        if (bDump)
        {
            printf("\n");
        }

        assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
        return 0;
    }

#if defined(SKIP_LINKS) // || (cwListPopCntMax != 0)
    assert( ! tp_bIsSkip(nType) || ((int)wr_nBL(wRoot) < nBL) );
    assert( ! tp_bIsSkip(nType) || (wr_nBL(wRoot) >= cnBitsInD1) );
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(EMBED_KEYS)

        if (nType == T_EMBEDDED_KEYS)
        {
            int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
            assert(nPopCntMax != 0);
            goto embeddedKeys;
embeddedKeys:;
            wPopCnt = wr_nPopCnt(wRoot, nBL);
            assert(wPopCnt != 0);

            if (!bDump) {
#if defined(NO_TYPE_IN_XX_SW)
                assert(nBL >= nDL_to_nBL(2));
#endif // defined(NO_TYPE_IN_XX_SW)
                // This OldList is a no-op and will return zero if
                // the key(s) is(are) embedded.
                assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
                return OldList(pwr, /* wPopCnt */ 1, nBL, nType);
            }

            printf(" tp_wPopCnt %3d", (int)wPopCnt);

#if defined(PP_IN_LINK)
            assert(nBL == nBLArg);
            if (nBLArg == cnBitsPerWord) {
                printf(" wr_wPrefix        N/A");
            } else {
                printf(" wr_wPrefix " OWx,
                       PWR_wPrefixBL(pwRoot, NULL, nBL));
            }
#endif // defined(PP_IN_LINK)

            if (EmbeddedListPopCntMax(nBL) != 0) {
                for (unsigned nn = 0; nn < wPopCnt; nn++) {
                    //char fmt[5]; sprintf(fmt, " %%0%d" _fw"x", (int)LOG((nBL<<1)-1));
                    printf(" %" _fw"x",
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot >> (cnBitsPerWord - ((nn + nPopCntMax - wPopCnt + 1) * nBL)))
#else // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot >> (cnBitsPerWord - ((nn + 1) * nBL)))
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                            & MSK(nBL));
                }
                printf("\n");
            } else {
                printf(" " OWx"\n", *pwr);
            }
        }
        else
#endif // defined(EMBED_KEYS)
        {
#if defined(DEBUG)
            if ( (nType != T_LIST)
  #if defined(UA_PARALLEL_128)
                && (nType != T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                )
            {
                printf("nType %d\n", nType);
            }
            assert( (nType == T_LIST)
  #if defined(UA_PARALLEL_128)
                   || (nType == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                   );
#endif // defined(DEBUG)

#if defined(PP_IN_LINK)
            if (nBL != cnBitsPerWord)
            {
                wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
            }
            else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
            }
            assert(wPopCnt != 0);

            if (!bDump)
            {
                wBytes = OldList(pwr, wPopCnt, nBL, nType);
                assert(wr_pwr(*pwRootArg) == pwr);
                goto zeroLink;
            }

#if defined(PP_IN_LINK)
            if (nBLArg == cnBitsPerWord)
            {
                printf(" ls_wPopCnt %3" _fw"u", wPopCnt);
                printf(" wr_wPrefix        N/A");
            }
            else
            {
                printf(" wr_wPopCnt %3" _fw"u", wPopCnt);
                printf(" wr_wPrefix " OWx,
                       PWR_wPrefixBL(pwRoot, NULL, nBL));
            }
#endif // defined(PP_IN_LINK)

            printf(" ls_wPopCnt %3" _fw"u", wPopCnt);

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
                { printf(" " OWx, ls_pwKeysNAT(pwr)[xx]); }
            }
            printf("\n");
        }

        if (!bDump) {
            if (nBL != cnBitsPerWord) { printf("FAG: nType %d\n", nType); }
            assert(nBL != cnBitsPerWord);
        }
        return 0;
    }
#endif // (cwListPopCntMax != 0)

    // Switch

    { // make C++ happy

    int nBLPrev = nBL;

#if defined(CODE_BM_SW)
    int bBmSw = tp_bIsBmSw(nType);
#endif // defined(CODE_BM_SW)
#if defined(SKIP_LINKS)
  #if defined(CODE_BM_SW)
      #if ! defined(SKIP_TO_BM_SW)
    if ( ! bBmSw )
      #endif // defined(SKIP_TO_BM_SW)
  #endif // defined(CODE_BM_SW)
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
#if defined(USE_LIST_SW)
        ((nType == T_LIST_SW) || (nType == T_SKIP_TO_LIST_SW))
            ? gpListSwLinks(qy) :
#endif // defined(USE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr) ;

    if (bDump)
    {
#if defined(PP_IN_LINK)
        if (nBLArg >= cnBitsPerWord)
        {
            // Add 'em up.
            Word_t wPopCnt = Sum(pwRoot, cnBitsPerWord);

            // Is wPopCnt == 0 ambiguous here?
            // If Dump is called in the middle of an insert,
            // after a new switch is created, and before any
            // keys are inserted into it we could have a
            // legitimate zero population.
            // But couldn't it also mean full pop?
            if ((wPopCnt == 0) && (pLinks[0].ln_wRoot != 0)) {
                printf(" sm_wPopCnt full");
            } else {
                printf(" sm_wPopCnt %3" _fw"u", wPopCnt);
            }

            printf(" wr_wPrefix        N/A");
        }
        else
#endif // defined(PP_IN_LINK)
        {
            Word_t wPopCnt =
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBL) :
#endif // defined(CODE_BM_SW)
                           PWR_wPopCntBL(pwRoot, (  Switch_t *)pwr, nBL)  ;
            if (wPopCnt == 0) {
                // Full-pop or in-transition sub-tree.
                // wPopCnt = EXP(nBL);
            }
            printf(" wr_wPopCnt %3" _fw"u", wPopCnt);
            printf(" wr_wPrefix " OWx,
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL) :
#endif // defined(CODE_BM_SW)
                           PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBL) );
        }

        printf(" wr_nBL %2d", nBL);
#if defined(CODE_XX_SW)
        if (tp_bIsXxSw(nType)) {
            printf(" wr_nBW %2d", nBitsIndexSz);
        }
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask " OWx, wPrefixPopMask(nDL));
        //printf(" pLinks " OWx, (Word_t)pLinks);
        if (bBmSw) {
#if defined(BM_IN_LINK)
            if (nBLArg != cnBitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                printf(" pwBm " OWx" pLinks " OWx" Bm",
                       (Word_t)PWR_pwBm(pwRoot, pwr), (Word_t)pLinks);
                // Bitmaps are an integral number of words.
                for (int nn = 0;
                     nn < DIV_UP((int)EXP(nBL_to_nBitsIndexSz(nBL)),
                                 cnBitsPerWord);
                     nn ++)
                {
                    if ((nn % 8) == 0) {
                        printf("\n");
                    }
                    printf(" " OWx, PWR_pwBm(pwRoot, pwr)[nn]);
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
#if defined(USE_LIST_SW)
                    PWR_wPrefixBL(pwRoot, (ListSw_t   *)pwr, nBL) ;
#else // defined(USE_LIST_SW)
                    PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBL) ;
#endif // defined(USE_LIST_SW)
    }

    nBL -= nBitsIndexSz;

    for (Word_t ww = 0, nn = 0; nn < EXP(nBitsIndexSz); nn++)
    {
#if defined(CODE_BM_SW)
#if defined(BM_IN_LINK)
        assert( ! bBmSw || (nBLArg != cnBitsPerWord));
#endif // defined(BM_IN_LINK)
        if ( ! bBmSw || BitIsSet(PWR_pwBm(pwRoot, pwr), nn))
#endif // defined(CODE_BM_SW)
        {
            if (pLinks[ww].ln_wRoot != 0)
            {
                //printf("nn %" _fw"x\n", nn);
                wBytes += FreeArrayGuts(&pLinks[ww].ln_wRoot,
                        wPrefix | (nn << nBL), nBL, bDump);
            }

            ww++;
        }
    }

    if (bDump) return 0;

    // Someone has to clear PP and BM if PP_IN_LINK and BM_IN_LINK.
    // OldSwitch looks at BM.

    wBytes += OldSwitch(pwRootArg, nBL + nBitsIndexSz,
#if defined(CODE_BM_SW)
                        bBmSw, /* nLinks */ 0,
#endif // defined(CODE_BM_SW)
                        nBLPrev);
    }

zeroLink:

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

// Dump the path to the subtree rooted at (nBL, wPrefix)
// and the entire subtree below.
void
Judy1Dump(Word_t wRoot, int nBL, Word_t wPrefix)
{
#if defined(DEBUG)
    assert(nBL == cnBitsPerWord); // for now
    Dump(&wRoot, wPrefix, nBL);
#else // defined(DEBUG)
    (void)wRoot; (void)nBL; (void)wPrefix;
#endif // defined(DEBUG)
}

#if defined(EMBED_KEYS)
#if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
  #if ! defined(PACK_KEYS_RIGHT)
static void
InsertEmbedded(Word_t *pwRoot, int nBL, Word_t wKey)
{
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
    int nPopCnt = wr_nPopCnt(*pwRoot, nBL);
    assert(nPopCnt < nPopCntMax);
    DBGI(printf("\nInsertEmbedded: wRoot " OWx" nBL %d wKey " OWx
                    " nPopCnt %d Max %d\n",
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
    DBGI(printf("Insert: wKey " OWx" nSlot %d", wKey, nSlot));
#if defined(FILL_W_BIG_KEY)
#error
#endif // defined(FILL_W_BIG_KEY)
    if (nSlot < nPopCnt) {
        Word_t wLowBits
            = GetBits(*pwRoot, (nPopCnt - nSlot) * nBL,
                      cnBitsPerWord - (nPopCnt * nBL));
        DBGI(printf(" wLowBits " OWx, wLowBits));
        SetBits(pwRoot, (nPopCnt - nSlot) * nBL,
                cnBitsPerWord - ((nPopCnt + 1) * nBL), wLowBits);
    }
    SetBits(pwRoot, nBL, cnBitsPerWord - (nSlot + 1) * nBL, wKey);
    set_wr_nPopCnt(*pwRoot, nBL, nPopCnt + 1);
    DBGI(printf(" wRoot " OWx" nPopCnt %d\n",
                *pwRoot, wr_nPopCnt(*pwRoot, nBL)));
}
  #endif // ! defined(PACK_KEYS_RIGHT)
#endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // defined(EMBED_KEYS)

#if (cwListPopCntMax != 0)

#if defined(SORT_LISTS)

// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
static void
CopyWithInsertWord(Word_t *pTgt, Word_t *pSrc, unsigned nKeys, Word_t wKey)
{
    DBGI(printf("\nCopyWithInsertWord(pTgt %p pSrc %p nKeys %d wKey " OWx")\n",
                (void *)pTgt, (void *)pSrc, nKeys, wKey));
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++) {
        if (pSrc[n] >= wKey) {
            //if (pSrc[n] == wKey) Dump(pwRootLast, 0, cnBitsPerWord);
            assert(pSrc[n] != wKey);
            break;
        }
    }

    if (pTgt != pSrc) {
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // save the tail
    } else {
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = wKey; // insert the key

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL_WORD)
    // Pad the list with copies of the last real key in the list so the
    // length of the list from the first key through the last copy of the
    // last real key is an integral multiple of cnBytesListLenAlign.
    // cnBytesListLenAlign is set to the size of a parallel search bucket.
    // This way we don't need any special handling in the parallel search
    // code to handle a partial final bucket.
    for (; (n * sizeof(wKey)) % sizeof(Bucket_t); ++n) {
        pTgt[n] = pTgt[n-1];
    }
#endif // defined(PSPLIT_PARALLEL_WORD)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)
}

#if defined(COMPRESSED_LISTS)

#if (cnBitsPerWord > 32)
static void
CopyWithInsertInt(uint32_t *pTgt, uint32_t *pSrc, unsigned nKeys,
                  uint32_t iKey)
{
    DBGI(printf("\nCopyWithInsertInt(pTgt %p pSrc %p nKeys %d iKey 0x%x)\n",
                (void *)pTgt, (void *)pSrc, nKeys, iKey));
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++) {
        if (pSrc[n] >= iKey) {
            assert(pSrc[n] != iKey);
            break;
        }
    }

    if (pTgt != pSrc) {
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    } else {
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = iKey; // insert the key

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    // See CopyWithInsertWord for comment.
    for (; (n * sizeof(iKey)) % sizeof(Bucket_t); ++n) {
        pTgt[n] = pTgt[n-1];
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)
}
#endif // (cnBitsPerWord > 32)

static void
CopyWithInsertShort(uint16_t *pTgt, uint16_t *pSrc,
                    int nKeys, // number of keys excluding the new one
                    uint16_t sKey, int nPos)
{
    int n;

    if ((nPos == -1) // inflated embedded list
#if ! defined(EMBED_KEYS)
            && (nKeys != 0)
#else // ! defined(EMBED_KEYS)
            && 1 // avoid extraneous parens error
#endif // ! defined(EMBED_KEYS)
        )
    {
        // find the insertion point
        n = ~PsplitSearchByKey16(pSrc, nKeys, sKey, 0);
    } else { n = nPos; }

    if (pTgt != pSrc) {
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    } else {
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = sKey; // insert the key

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    // See CopyWithInsertWord for comment.
  #if defined(UA_PARALLEL_128)
    if (n <= 6) {
        for (; (n * sizeof(sKey)) % 12; ++n) {
            pTgt[n] = pTgt[n-1];
        }
    } else
  #endif // defined(UA_PARALLEL_128)
    {
        for (; (n * sizeof(sKey)) % sizeof(Bucket_t); ++n) {
            pTgt[n] = pTgt[n-1];
        }
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)
}

static void
CopyWithInsertChar(uint8_t *pTgt, uint8_t *pSrc, unsigned nKeys, uint8_t cKey)
{
    unsigned n;

    // find the insertion point
    for (n = 0; n < nKeys; n++) {
        if (pSrc[n] >= cKey) {
            assert(pSrc[n] != cKey);
            break;
        }
    }

    if (pTgt != pSrc) {
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    }
    else
    {
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = cKey; // insert the key

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    for (; (n * sizeof(cKey)) % sizeof(Bucket_t); ++n) {
        pTgt[n] = pTgt[n-1];
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
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
  #if (cnBitsPerWord >= 64)
    cnListPopCntMax64, // 32 < nBL <= 64
  #endif // (cnBitsPerWord >= 64)
    };

#endif // (cwListPopCntMax != 0)

void
HexDump(char *str, Word_t *pw, unsigned nWords)
{
    printf("\n%s (pw %p nWords %d):\n", str, (void *)pw, nWords);
    if (nWords % 8 == 0) {
        for (unsigned ii = 0; ii < nWords; ii += 8) {
            printf(" " OWx" " OWx" " OWx" " OWx,
                   pw[ii], pw[ii+1], pw[ii+2], pw[ii+3]);
            printf(" " OWx" " OWx" " OWx" " OWx"\n",
                   pw[ii+4], pw[ii+5], pw[ii+6], pw[ii+7]);
        }
    } else if (nWords % 4 == 0) {
        for (unsigned ii = 0; ii < nWords; ii += 4) {
            printf(" " OWx" " OWx" " OWx" " OWx"\n",
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

// Looks like the main/sole purpose of InsertCleanup at this point is to
// replace a 2-digit switch and whatever is hanging off of it with a
// a 2-digit bitmap once the population supports it as defined by
// cn2dBmMaxWpkPercent.
// nBL describes the level of the root word passed in. It has not been
// advanced by any skip in the containing link.
void
InsertCleanup(Word_t wKey, int nBL, Word_t *pwRoot, Word_t wRoot)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot);
    Word_t *pwr = wr_pwr(wRoot);
    int nType = wr_nType(wRoot);
    qv, (void)wKey; (void)nType;

#if defined(CODE_BM_SW)
    if (tp_bIsBmSw(nType)) {
        // We should have a one-liner for this.
  #if defined(SKIP_LINKS)
        int nBLR = tp_bIsSkip(nType) ? gnBLR(qy) : nBL;
  #else // defined(SKIP_LINKS)
        int nBLR = nBL;
  #endif // defined(SKIP_LINKS)
        Word_t wPopCnt = PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
        int nBW = gnBW(qy, T_BM_SW, nBLR);
        // -E: 256*16=4096 > 256*8=2048
        if (wPopCnt * nBLR * cnBmSwConvert
                > EXP(nBW) * 8 * sizeof(Link_t) * cnBmSwRetain) {
            InflateBmSw(pwRoot, wKey, nBLR, /* nBLUp */ nBL);
        }
    }
#endif // defined(CODE_BM_SW)

#if (cn2dBmMaxWpkPercent != 0) // conversion to big bitmap enabled
    int nDL = nBL_to_nDL(nBL);

    // Can't disable this one by ALLOW_EMBEDDED_BITMAP.
    assert(cnBitsInD1 > LOG(sizeof(Link_t) * 8)); // else doesn't work yet

    (void)nDL;
    Word_t wPopCnt;
    if ((nBL == nDL_to_nBL(2))
        && tp_bIsSwitch(nType)
#if defined(SKIP_LINKS)
        && ! tp_bIsSkip(nType)
#endif // defined(SKIP_LINKS)
#if defined(CODE_BM_SW)
        && ! tp_bIsBmSw(nType)
#endif // defined(CODE_BM_SW)
        // if wpk > maxwpk, don't convert
        // if wpk < maxwpk, convert
        // if words / keys < maxwpk, convert
        // if words < keys * maxwpk, convert
        // maxwpk = cn2dBmMaxWpkPercent / 100
        // if words < keys * cn2dBmMaxWpkPercent / 100, convert
        // if keys * cn2dBmMaxWpkPercent / 100 > words, convert
        // if keys * cn2dBmMaxWpkPercent > words * 100, convert
        // words = EXP(nBL - cnLogBitsPerWord)
        // keys = wPopCnt
        // if wPopCnt * cn2dBmMaxWpkPercent > EXP(nBL-cnLogBitsPerWord) * 100
        // Disable with cn2dBmMaxWpkPercent = 0.
        //-E: 256 * cn2dBmMaxPercent > 1024*100
        && ((wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBL))
                    * cn2dBmMaxWpkPercent
                > (EXP(nBL - cnLogBitsPerWord) * 100)))
    {
        DBGI(printf("Converting BM leaf.\n"));
        //Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord);
#if defined(SKIP_LINKS)
        assert( ! tp_bIsSkip(nType) );
#endif // defined(SKIP_LINKS)
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
        DBGI(printf("# IC: NewBitmap nBL %d nBW %d wPopCnt %" _fw"d"
                        " wWordsAllocated %" _fw"d wPopCntTotal %" _fw"d.\n",
                    nBL, nBW, wPopCnt, wWordsAllocated, wPopCntTotal));
        Word_t *pwBitmap = NewBitmap(pwRoot, nBL, nBL, wKey);
        set_w_wPopCntBL(*(pwBitmap + EXP(nBL - cnLogBitsPerWord)), nBL, wPopCnt);

        // Why are we not using InsertAll here to insert the keys?
        // It doesn't handle switches yet.

        int nBLLn = nBL - nBW;
        Word_t wBLM = MSK(nBLLn); // Bits left mask.

        for (Word_t ww = 0; ww < EXP(nBW); ww++)
        {
            Word_t *pwRootLn =
#if defined(USE_LIST_SW)
                (nType == T_LIST_SW) ? &gpListSwLinks(qy)[ww].ln_wRoot :
#endif // defined(USE_LIST_SW)
                &pwr_pLinks((Switch_t *)pwr)[ww].ln_wRoot;
            Word_t wRootLn = *pwRootLn;
#if defined(NO_TYPE_IN_XX_SW)
            if (nBLLn < nDL_to_nBL(2)) {
                goto embeddedKeys;
            }
#endif // defined(NO_TYPE_IN_XX_SW)
            int nTypeLn = wr_nType(wRootLn);
            Word_t *pwrLn = wr_pwr(wRootLn);

#if defined(EMBED_KEYS)
            if (nTypeLn == T_EMBEDDED_KEYS) {
                goto embeddedKeys;
embeddedKeys:;
                int nPopCntLn = wr_nPopCnt(wRootLn, nBLLn);
                for (int nn = 1; nn <= nPopCntLn; nn++) {
                    SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                           ((wRootLn >> (cnBitsPerWord - (nn * nBLLn)))
                               & wBLM));
                }
            } else
#endif // defined(EMBED_KEYS)
            if (nTypeLn == T_BITMAP) {
                memcpy(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                       pwrLn, EXP(nBLLn - 3));
                OldBitmap(pwRootLn, pwrLn, nBLLn);
            }
#if (cwListPopCntMax != 0)
            else if (wRootLn != 0) {
#if defined(DEBUG)
                if (nTypeLn != T_LIST)
  #if defined(UA_PARALLEL_128)
                if (nTypeLn != T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                { printf("nTypeLn %d\n", nTypeLn); }
                assert( (nTypeLn == T_LIST)
  #if defined(UA_PARALLEL_128)
                       || (nTypeLn == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                       );
#endif // defined(DEBUG)
#if defined(PP_IN_LINK)
                int nPopCntLn
                      = PWR_wPopCnt(pwRootLn, (Switch_t *)pwrLn, nBLLn);
#else // defined(PP_IN_LINK)
                int nPopCntLn = PWR_xListPopCnt(pwRootLn, pwrLn, nBLLn);
#endif // defined(PP_IN_LINK)
                if (nBLLn <= 8) {
                    uint8_t *pcKeysLn = ls_pcKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (pcKeysLn[nn] & wBLM));
                    }
                } else
#if (cnBitsPerWord == 64)
                if (nBLLn <= 16)
#endif // (cnBitsPerWord == 64)
                {
#if (cnBitsPerWord == 32)
                    assert(nBLLn <= 16);
#endif // (cnBitsPerWord == 32)
                    uint16_t *psKeysLn = ls_psKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (psKeysLn[nn] & wBLM));
                    }
                }
#if (cnBitsPerWord == 64)
                else {
                    assert(nBLLn <= 32);
                    uint32_t *piKeysLn = ls_piKeysNAT(pwrLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (piKeysLn[nn] & wBLM));
                    }
                }
#endif // (cnBitsPerWord == 64)
                assert(nPopCntLn != 0);
                OldList(pwrLn, nPopCntLn, nBLLn, T_LIST);
            }
#endif // (cwListPopCntMax != 0)
            else {
                // I guess remove can result in a NULL *pwRootLn in a bitmap
                // switch since we don't clean them up at the time.
                DBGI(printf("Null link in bm switch ww %" _fw"d.\n", ww));
            }
        }

#if defined(SKIP_LINKS)
        assert( ! tp_bIsSkip(nType) ); // How do we ensure this?
#endif // defined(SKIP_LINKS)
        OldSwitch(&wRoot, nBL,
#if defined(CODE_BM_SW)
                  /* bBmSw */ 0, /* nLinks */ 0,
#endif // defined(CODE_BM_SW)
                  /* nBLUp */ nBL);

#if defined(DEBUG)
        int count = 0;
        for (int jj = 0; jj < (int)EXP(nBL - cnLogBitsPerWord); jj++)
        {
            if (pwBitmap[jj] != 0) {
                DBGI(printf("jj %d pwBitmap[jj] " OWx" popcount %d\n",
                            jj, pwBitmap[jj],
                            __builtin_popcountll(pwBitmap[jj])));
            }
            count += __builtin_popcountll(pwBitmap[jj]);
        }
        if (count != (int)wPopCnt) {
            printf("count %d wPopCnt %" _fw"d\n", count, wPopCnt);
                    Dump(pwRootLast,
                              /* wPrefix */ (Word_t)0, cnBitsPerWord);
        }
        assert(count == (int)wPopCnt);
#endif // defined(DEBUG)
    }
#endif // (cn2dBmMaxWpkPercent != 0)
}

#if (cwListPopCntMax != 0)

// Insert each key from pwRootOld into pwRoot.  Then free pwRootOld.
// wKey contains the common prefix.
static void
InsertAll(Word_t *pwRootOld, int nBLOld, Word_t wKey, Word_t *pwRoot, int nBL)
{
    DBGI(printf("InsertAll(pwRootOld %p nBLOld %d wKey " OWx
                    " pwRoot %p nBL %d\n",
                (void *)pwRootOld, nBLOld, wKey, (void *)pwRoot, nBL));
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
  #if defined(EMBED_KEYS)
    if (nType == T_EMBEDDED_KEYS) {
        goto embeddedKeys;
embeddedKeys:;
        // How inefficient can we be?
        DBGI(printf("IA: Calling IEL nBLOld %d wKey " OWx" nBL %d\n",
                    nBLOld, wKey, nBL));
        wRootOld = InflateEmbeddedList(pwRootOld, wKey, nBLOld, wRootOld);
        DBGI(printf("After IEL\n"));
// If (nBLOld < nDL_to_nBL) Dump is going to think wRootOld is embeddded keys.
        //DBGI(Dump(&wRootOld, wKey & ~MSK(nBLOld), nBLOld));
        nType = wr_nType(wRootOld); // changed by IEL
        assert( (nType == T_LIST)
  #if defined(UA_PARALLEL_128)
               || (nType == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
               );
    }
  #endif // defined(EMBED_KEYS)
#endif // defined(CODE_XX_SW)

    Word_t *pwrOld = wr_pwr(wRootOld);

    if (nType != T_LIST)
  #if defined(UA_PARALLEL_128)
    if (nType != T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
    {
        printf("nType %d wRootOld " OWx" pwRootOld %p\n",
               nType, wRootOld, (void *)pwRootOld);
        DBGR(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    }
    assert( (nType == T_LIST)
  #if defined(UA_PARALLEL_128)
           || (nType == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
           );
#if defined(PP_IN_LINK) // ? what about POP_IN_WR_HB ?
    if (nBLOld < cnBitsPerWord) {
        // Adjust the count to compensate for pre-increment during insert.
        nPopCnt = PWR_wPopCntBL(pwRootOld, NULL, nBLOld) - 1;
    } else
#endif // defined(PP_IN_LINK)
    {
        // wRootOld might be newer than *pwRootOld
        nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLOld);
    }

    int status = 0; // for debug
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
        Word_t *pwKeys = ls_pwKeys(pwrOld, nBL);
        for (int nn = 0; nn < nPopCnt; nn++) {
            status = Insert(pwRoot, pwKeys[nn], nBL);
        }
    }
    assert(status == 1);
    (void)status;

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
    {
        //printf("pwRoot %p\n", (void *)pwRoot);
        wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
    }

    // Figure nBL for the new switch.
    int nDL = nBL_to_nDL(LOG(1 | (wPrefix ^ wKey)) + 1);
    int nBL = nDL_to_nBL(nDL);
    // nDL includes the highest order digit that is different.

    if (nDL <= nDLR) { printf("nDL %d nDLR %d\n", nDL, nDLR); }
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
                     bBmSwNew ? T_BM_SW :
#endif // defined(CODE_BM_SW)
                     T_SWITCH,
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
  #if defined(SKIP_LINKS)
    if (nDL - nDLR - 1 == 0) {
        Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
    }
  #endif // defined(SKIP_LINKS)
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
// determined that the key from an insert request is not present in the array.
// It is provided with a starting point (pwRoot, wRoot, nBL) for the insert
// and some additional information (nPos, pwRootPrev, nBLPrev) that may be
// necessary or helpful in some cases.
// InsertGuts does whatever is necessary to insert the key into the array
// and returns back to Insert.
// InsertGuts is where the main runtime policy decisions are made.
// Some are made in RemoveGuts, but those are closely aligned with
// the decisions made here.
// Do we create a list as high as possible or as low as possible?
// When do we create a new switch instead of adding to a list?
// At what level do we create a new switch?
// What type of switch do we create?
// When do we create a bitmap?
// When do we uncompress a switch?
// When do we double a switch?
// When do we coalesce switches?
Status_t
InsertGuts(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot, int nPos
#if defined(CODE_XX_SW)
           , Word_t *pwRootPrev
  #if defined(SKIP_TO_XX_SW)
           , int nBLPrev
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)
           )
{
#if defined(CODE_XX_SW)
    (void)pwRootPrev;
  #if defined(SKIP_TO_XX_SW)
    (void)nBLPrev;
  #endif // defined(SKIP_TO_XX_SW)
    int nBW; (void)nBW;
#endif // defined(CODE_XX_SW)
    int nDL = nBL_to_nDL(nBL);
    assert(nDL_to_nBL(nDL) >= nBL);
    DBGI(printf("InsertGuts pwRoot %p wKey " OWx" nBL %d wRoot " OWx"\n",
                (void *)pwRoot, wKey, nBL, wRoot));
    Link_t link; (void)link;

#if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1);
#endif // ! defined(USE_XX_SW)

    // Handle cases where wRoot has no type field before looking at
    // the type field in wRoot.

#if defined(NO_TYPE_IN_XX_SW)
    if (pwRootPrev != NULL) { // non-NULL only for XX_SW
        DBGR(printf("IG: goto embeddedKeys.\n"));
        assert(tp_bIsXxSw(wr_nType(*pwRootPrev)));
        goto embeddedKeys; // no type field is handled by embeddedKeys
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
    if (nType == T_EMBEDDED_KEYS) {
        goto embeddedKeys;
embeddedKeys:;
          #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
            #if ! defined(PACK_KEYS_RIGHT)
        // This is a performance shortcut that is not necessary.
        if (wr_nPopCnt(*pwRoot, nBL) < EmbeddedListPopCntMax(nBL)) {
            InsertEmbedded(pwRoot, nBL, wKey); return Success;
        }
            #endif // ! defined(PACK_KEYS_RIGHT)
          #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)

        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        // Is there any reason to delay the install of wRoot?
        *pwRoot = wRoot;
        nPos = -1; // Tell copy that we have no nPos.

        // BUG: The list may not be sorted at this point.  Does it matter?
        // Update: I'm not sure why I wrote that the list may not be sorted
        // at this point.  I can't think of why it would not be sorted.
        // Is it related to SEARCH_FROM_WRAPPER?
        nType = wr_nType(wRoot);
      #if ! defined(PP_IN_LINK)
        DBGI(printf("IG: wRoot " OWx" nType %d PWR_xListPopCnt %d\n",
                    wRoot, nType,
                    (int)PWR_xListPopCnt(&wRoot, wr_pwr(wRoot), nBL)));
      #endif // ! defined(PP_IN_LINK)
    }
  #endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

    Word_t *pwr = wr_pwr(wRoot);

// This first clause handles wRoot == 0 by treating it like a list leaf
// with zero population (and no allocated memory).
// But why is it ok to skip the test for a switch if ! defined(SKIP_LINKS)
// and !defined(BM_SW_FOR_REAL)? Because InsertGuts is called with a wRoot
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
        Word_t wPopCnt = 0;
        Word_t *pwKeys = NULL;
#if defined(COMPRESSED_LISTS)
  #if (cnBitsPerWord > 32)
        unsigned int *piKeys = NULL;
  #endif // (cnBitsPerWord > 32)
        unsigned short *psKeys = NULL;
        unsigned char *pcKeys = NULL;
#endif // defined(COMPRESSED_LISTS)

        DBGI(printf("InsertGuts List\n"));

        // Initialize wPopCnt, pwKeys, piKeys, psKeys and pcKeys for copy.
        // And set prefix in link if PP_IN_LINK and the list is empty and
        // we're not at the top.
        if (wRoot != 0) // pointer to old List
        {
#if defined(EMBED_KEYS)
            assert(nType != T_EMBEDDED_KEYS);
#endif // defined(EMBED_KEYS)
#if defined(PP_IN_LINK)
            // this test is no good unless we disallow skip from top
            if (nBL != cnBitsPerWord) {
                // Get pop from ln_wPrefixPop.
                // Why are we subracting one here? Is it because Insert
                // bumps pop count before calling InsertGuts?
                // How is this handled with InflateEmbeddedList?
                // ln_wPrefixPop is not affected by InflateEmbeddedList?
                // Can't we make PWR_xListPopCnt handle this case?
                wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL) - 1;
            } else
#endif // defined(PP_IN_LINK)
            {
                wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
            }
            pwKeys = ls_pwKeys(pwr, nBL); // list of keys in old List
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
            piKeys = ls_piKeys(pwr, nBL);
#endif // (cnBitsPerWord > 32)
            psKeys = ls_psKeys(pwr, nBL);
            pcKeys = ls_pcKeys(pwr, nBL);
#endif // defined(COMPRESSED_LISTS)
            // prefix is already set
        }
        else
        {
#if defined(PP_IN_LINK)
            if (nBL != cnBitsPerWord)
            {
                // What about no_unnecessary_prefix?
                set_PWR_wPrefixBL(pwRoot, pwr, nBL, wKey);
            }
#endif // defined(PP_IN_LINK)
        }

// We don't support skip links to lists yet.
// And we don't have any criteria yet for converting from a list to a switch
// other than the list is full.
// So we just add to an existing list or create a new one here.
// But this is where we might make a decision to use a skip link
// to a list or a bitmap or convert a list to a switch or convert the
// other way or create a bitmap switch or ...
// We need a new type field the destination of a link with type zero.
//
//  - bitmap leaf -- level (width), prefix and pop, bits
//  - list leaf -- level (key size), prefix, pop (capacity), keys
//  - bitmap switch -- level, prefix, pop, width, bitmap, links
//  - list switch -- level, prefix, pop, width, link indexes, links

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
#undef cnXxSwWpkPercent
#define cnXxSwWpkPercent  150
#endif // ! defined(cnXxSwWpkPercent)
            // This block is a performance/efficiency optimization.
            // It is not necessary for "correct" behavior.
            // It is only relevant if ifdefs allow for a blow-out.
            // This is possible if ! defined(NO_TYPE_IN_XX_SW).
            // We haven't written the blow-up creation code for
            // NO_TYPE_IN_XX_SW yet.
            if (nBL == nDL_to_nBL(2)) {
  #if defined(EMBED_KEYS)
                if ((int)wPopCnt >= nEmbeddedListPopCntMax)
  #endif // defined(EMBED_KEYS)
                {
                    if ((wWordsAllocated * 100 / wPopCntTotal)
                            < cnXxSwWpkPercent)
                    {
                        goto newSwitch;
                    }
                }
            }
            if (nBL < nDL_to_nBL(2)) {
  #if defined(EMBED_KEYS)
                if ((int)wPopCnt >= nEmbeddedListPopCntMax)
  #endif // defined(EMBED_KEYS)
                {
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
                // Inflate uses LWTL.
                || (ListWordsTypeList(wPopCnt + 1, nBL)
                        != ListWordsTypeList(wPopCnt, nBL)))
            {
                DBGI(printf("pwr %p wPopCnt %" _fw"d nBL %d\n",
                            (void *)pwr, wPopCnt, nBL));
                DBGI(printf("nType %d\n", nType));
                DBGI(printf("nBL %d LWE(pop %d) %d LWE(pop %d) %d\n",
                            nBL, (int)wPopCnt + 1,
                            ListWordsExternal(wPopCnt + 1, nBL),
                            (int)wPopCnt,
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
#if defined(UA_PARALLEL_128)
                if ((nBL == 16) && (wPopCnt < 6)) {
                    set_wr(wRoot, pwList, T_LIST_UA);
                } else
#endif // defined(UA_PARALLEL_128)
                { set_wr(wRoot, pwList, T_LIST); }
            }
            else
            {
                pwList = pwr;

#if defined(PP_IN_LINK)
                if (nDL != cnDigitsPerWord) {
                    assert(PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                           == wPopCnt + 1);
                }
#endif // defined(PP_IN_LINK)
            }

#if defined(PP_IN_LINK)
            if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK)
            {
                Set_xListPopCnt(&wRoot, nBL, wPopCnt + 1);
            }

            if (wPopCnt != 0)
#if defined(SORT_LISTS)
            {
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
  #if defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL)
copyWithInsert8:
  #endif // defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL)
                    CopyWithInsertChar(ls_pcKeysNAT(pwList),
                        pcKeys, wPopCnt, (unsigned char)wKey);
                } else if (nBL <= 16) {
  #if defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL)
copyWithInsert16:
  #endif // defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL)
                    CopyWithInsertShort(ls_psKeysNAT(pwList),
                        psKeys, wPopCnt, (unsigned short)wKey, nPos);
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
            }
            else
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
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
  #if defined(SORT_LISTS) \
      && defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL)
                    //printf("goto copyWithInsert8\n");
                    goto copyWithInsert8;
  #else // defined(SORT_LISTS) && ...
                    ls_pcKeysNAT(pwList)[wPopCnt] = wKey;
  #endif // defined(SORT_LISTS) && ...
                } else if (nBL <= 16) {
  #if defined(SORT_LISTS) \
      && defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL)
                    nPos = 0;
                    //printf("goto copyWithInsert16\n");
                    goto copyWithInsert16;
  #else // defined(SORT_LISTS) && ...
                    ls_psKeysNAT(pwList)[wPopCnt] = wKey;
  #endif // defined(SORT_LISTS) && ...
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
// Don't we need goto copyWithInsert32 for
// PSPLIT_SEARCH_32 && PSPLIT_PARALLEL?
// Maybe we're being protected by a Deflate below?
// Not sure why we're doing goto only for SORT_LISTS.
                    { ls_piKeysNAT(pwList)[wPopCnt] = wKey; }
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
// Don't we need goto copyWithInsertWord for
// PSPLIT_SEARCH_WORD && PSPLIT_PARALLEL?
// Maybe we're being protected by default !PSPLIT_SEARCH_WORD?
                    ls_pwKeys(pwList, nBL)[wPopCnt] = wKey;
                }
                // Shouldn't we be padding the extra key slots
                // for parallel search? Is the unsorted list
                // code so dead that we should abandon it?
                // Is the uncompressed lists so dead that we
                // should abandon it?
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
            if ( ! ( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
                   || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
                   ) )
            {
                printf("wRoot " Owx"\n", wRoot);
            }
#endif // defined(EMBED_KEYS)
#if defined(EMBED_KEYS)
            // Embed the list if it fits.
            assert( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
                   || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
                   );
            if ((int)wPopCnt < EmbeddedListPopCntMax(nBL))
            {
                DeflateExternalList(pwRoot, wPopCnt + 1, nBL, pwList);
#if defined(NO_TYPE_IN_XX_SW)
                if (!((nBL < nDL_to_nBL(2))
                    || (wr_nType(*pwRoot) == T_EMBEDDED_KEYS)))
                {
                    printf("nBL %d wPopCnt %ld nType %d\n",
                           nBL, wPopCnt, wr_nType(*pwRoot));
                }
                assert((nBL < nDL_to_nBL(2))
                    || (wr_nType(*pwRoot) == T_EMBEDDED_KEYS));
#else // defined(NO_TYPE_IN_XX_SW)
  #if defined(DEBUG)
                if ( ! (wr_nType(*pwRoot) == T_EMBEDDED_KEYS) )
                {
                    printf("\nnType 0x%x wPopCnt " OWx"\n",
                           wr_nType(*pwRoot), wPopCnt);
                }
  #endif // defined(DEBUG)
                assert(wr_nType(*pwRoot) == T_EMBEDDED_KEYS);
#endif // defined(NO_TYPE_IN_XX_SW)
                DBGR(printf("IG: after DEL *pwRoot " OWx"\n", *pwRoot));
            }
#endif // defined(EMBED_KEYS)
        }
        else
#endif // (cwListPopCntMax != 0)
        {
            // List is full. What do we do now?
            // - Partition the keys between two or more links?
            //   - Widen the switch with the link to the list?
            //   - Create and insert a new switch below the link to the list?
            // - Replace the list with a bitmap?
            // - Speed up the full path to this list somehow so its okay to increase
            //   the length of this list?
            //   - Uncompress an existing compressed switch?
            //   - Merge switches?
            //   - Replace a compressed switch with a faster variant? Possible?
            // How should we decide a list is full?
            // Is a fixed max length at each depth the best way or should it depend
            // on attributes of the actual path to the list?
            DBGI(printf("List is full nBL %d.\n", nBL));
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
                DBGI(printf("goto newSwitch\n"));
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
// Why do I get a wSufix may be uninitialized warning only with DEBUG_INSERT?
// Compiler doesn't know wSuffix is only used if nBL <= cnBitsPerWord.
// Why is it different for DEBUG_INSERT?
                { wMin = pwKeys[0]; wMax = pwKeys[wPopCnt - 1]; wSuffix = wKey; }
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
                DBGI(printf("wMin " OWx" wMax " OWx"\n", wMin, wMax));

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
            if (nBL > nBLOld) { nBL = nBLOld; } // blowout
            assert(nBL <= nBLOld);
#endif // defined(CODE_XX_SW)

#endif // defined(SKIP_LINKS)
#if ((cwListPopCntMax != 0) \
          && ((cnListPopCntMax64 == 0) || (cnListPopCntMax32 == 0) \
                                       || (cnListPopCntMax16 == 0))) \
      || defined(CODE_XX_SW)
            goto newSwitch;
newSwitch:
#endif // ((cwListPopCntMax != 0) && ... ) || ...
#if defined(SKIP_LINKS)
            DBGI(printf("InsertGuts newSwitch 0 nDL %d nBL %d nDLOld %d nBLOld %d\n",
                        nDL, nBL, nDLOld, nBLOld));

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
                nBL = cnBitsLeftAtDl3;
                nDL = 3;
            }
#else // defined(USE_XX_SW) && ! defined(SKIP_TO_XX_SW)
            if ((nBL < cnBitsLeftAtDl2) && (nBLOld >= cnBitsLeftAtDl2)) {
                nBL = cnBitsLeftAtDl2;
                nDL = 2;
            }
#endif // defined(USE_XX_SW) && ! defined(SKIP_TO_XX_SW)
            DBGI(printf("InsertGuts newSwitch 1 nDL %d nBL %d nDLOld %d nBLOld %d\n",
                        nDL, nBL, nDLOld, nBLOld));
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
#if ! defined(LVL_IN_SW) && ! defined(LVL_IN_WR_HB)
// Depth is in type.
            if (nDL != nDLOld) {
                if (nDL_to_tp(nDL) > (int)cnMallocMask) {
                    printf("# Oops. Can't encode absolute level for skip.\n");
                    printf("nDL %d nDLOld %d\n", nDL, nDLOld);
                    nDL = nDLOld - 1;
                    nBL = nDL_to_nBL(nDL);
                    assert(0);
                }
            }
#endif // ! defined(LVL_IN_SW) && ! defined(LVL_IN_WR_HB)
#else // defined(SKIP_LINKS)
            // I don't remember why this assertion was here.
            // But it blows and the code seems to do ok with it
            // commented out.
            // assert(nDL > 1);
#endif // defined(SKIP_LINKS)

#if ! defined(USE_XX_SW)
            if ((EXP(cnBitsInD1) > sizeof(Link_t) * 8) && (nDL == 1)) {
                assert(nBLOld == nBL);
#if defined(PP_IN_LINK)
                // NewBitmap changes *pwRoot and we change the Link_t
                // containing it on return from NewBitmap.
                // We need to preserve the Link_t for subsequent InsertAll.
                link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
#endif // defined(PP_IN_LINK)
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
                if (nBL < nDL_to_nBL(2)) {
                    DBGI(printf("\n# Blow up nBL %d nPopCnt %d\n",
                                nBL,
                                // How do I get pop count here?
                                // wr_nPopCnt(*pwRoot, nBL)
                                // Get_xListPopCnt(pwRoot, nBL)
                                (int)wPopCnt
                                ));
                }

  #if defined(USE_XX_SW)
                if (1 && (nBL == nDL_to_nBL(2)) // Use XX_SW at DL2.
      #if ! defined(SKIP_TO_XX_SW)
                      && (nBL == nBLOld)
      #endif // ! defined(SKIP_TO_XX_SW)
                    )
                {
                    DBGI(printf("# Creating T_XX_SW wKey " OWx" nBL %d\n",
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
#endif // defined(USE_XX_SW)
                    goto doubleIt;
doubleIt:;
#if defined(USE_XX_SW)
                    assert(nBL < nDL_to_nBL(2));
// Hmm.  *pwRoot has not been updated with the inflated list.
// What should we do?  Call OldList or install the inflated list?
// I think we are going to just inflate it again if we don't just leave it.
// So let's try installing it.
#if defined(NO_TYPE_IN_XX_SW)
                    DBGR(printf("IG: free inflated list.\n"));
                    assert( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
                           || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
                           );
                    OldList(wr_pwr(wRoot), wPopCnt, nBL, T_LIST);
#else // defined(NO_TYPE_IN_XX_SW)
#if defined(EMBED_KEYS)
                    if (wr_nType(*pwRoot) == T_EMBEDDED_KEYS) {
                        assert( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
                               || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
                               );
                        *pwRoot = wRoot;
                    }
#endif // defined(EMBED_KEYS)
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
                        nBLOld = nBLPrev;
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
                                      " wWordsAllocated %" _fw"d"
                                      " wPopCntTotal %" _fw"d.\n",
                                    nBL, nBLOld, wWordsAllocated, wPopCntTotal));
                        DBGI(printf("# IG: NewBitmap wPopCnt %" _fw"d.\n",
                                    wPopCnt));
                        DBGI(printf("# IG: NewBitmap nBL %d.\n", nBL));
#if ! defined(SKIP_TO_BITMAP)
                        assert(nBL == nBLOld);
#endif // ! defined(SKIP_TO_BITMAP)
                        NewBitmap(pwRoot, nBL, nBLOld, wKey);
#if defined(PP_IN_LINK)
                        set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL, 0);
#endif // defined(PP_IN_LINK)
                        DBGI(printf("# After NewBitmap; before insertAll.\n"));
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
                    printf("IG: pwRoot %p wKey " OWx" nBL %d wRoot " OWx"\n",
                           (void *)pwRoot, wKey, nBL, wRoot);
                    printf("nBLOld %d\n", nBLOld);
                }
#endif // defined(DEBUG)
                assert(nBL <= nBLOld);

#if defined(PP_IN_LINK)
                // NewSwitch changes *pwRoot and the Link_t containing it.
                // We need to preserve the Link_t for subsequent InsertAll.
                // We don't have a whole link at the top.
                if (nBLOld < cnBitsPerWord) {
                    link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
                }
#endif // defined(PP_IN_LINK)
                NewSwitch(pwRoot, wKey, nBL,
#if defined(CODE_XX_SW)
                          nBW,
#endif // defined(CODE_XX_SW)
  #if defined(USE_BM_SW)
      #if defined(USE_XX_SW)
                          (nBL <= nDL_to_nBL(2))
                              ? T_SWITCH :
      #endif // defined(USE_XX_SW)
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                          nBLOld != cnBitsPerWord ? T_BM_SW : T_SWITCH,
          #else // defined(BM_IN_LINK)
                          T_BM_SW,
          #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                          (nBLOld != cnBitsPerWord) && (nBL == nBLOld) ? T_BM_SW : T_SWITCH,
          #else // defined(BM_IN_LINK)
                          (nBL == nBLOld) ? T_BM_SW : T_SWITCH,
          #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
                          T_SWITCH,
  #endif // defined(USE_BM_SW)
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
                          /* bBmSw */ 0, /* nLinks */ 0,
#endif // defined(CODE_BM_SW)
                          /* nBLUp */ nBLOld);

                //printf("# New tree after InsertAll done looping:\n");
                //DBG(Dump(pwRoot, wKey, nBLOld));

            } else
#endif // defined(USE_XX_SW)
            {
#if defined(PP_IN_LINK)
                // InsertAll may look in the link containing wRoot for
                // pop count. That's why we preserved the contents of
                // the link before overwriting it above.
                if (nBLOld < cnBitsPerWord) {
                    InsertAll(&link.ln_wRoot, nBLOld, wKey, pwRoot, nBLOld);
                } else
#endif // defined(PP_IN_LINK)
                {
                    // *pwRoot now points to a switch
                    InsertAll(&wRoot, nBLOld, wKey, pwRoot, nBLOld);
                }
            }

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
  #if defined(EMBED_KEYS)
        assert(wr_nType(*pwRoot) != T_EMBEDDED_KEYS);
  #endif // defined(EMBED_KEYS)
        int nDLR = ! tp_bIsSkip(nType) ? nDL : wr_nDL(wRoot);
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
  #if defined(SKIP_TO_BM_SW)
            assert((nType == T_SKIP_TO_BM_SW) || (nType == T_BM_SW));
  #else // defined(SKIP_TO_BM_SW)
            assert(nType == T_BM_SW);
  #endif // defined(SKIP_TO_BM_SW)
#if defined(SKIP_LINKS)
            DBGI(printf("wPrefix " OWx" w_wPrefix " OWx" nDLR %d\n",
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
                             bBmSwNew ? T_BM_SW : T_SWITCH,
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
            if (nDL - nDLR - 1 == 0) {
                Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
            }
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
// It assumes the input is an embedded list.
Word_t
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot)
{
    (void)pwRoot;
    DBGI(printf(
         "InflateEmbeddedList pwRoot %p wKey " OWx" nBL %d wRoot " OWx"\n",
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
        printf("IEL: wRoot " OWx" nBL %d nPopCnt %d Max %d nBitsPopCntSz %d\n",
               wRoot, nBL, nPopCnt, nPopCntMax, nBL_to_nBitsPopCntSz(nBL));
    }
#endif // defined(DEBUG)
    assert(nPopCnt <= nPopCntMax);

    if (nPopCnt == 0) {
        printf("IEL: wRoot " OWx" nBL %d\n", wRoot, nBL);
    }
    assert(nPopCnt != 0);
    // Why not allocate a big enough list to hold the new key that we're about
    // to insert? Maybe we already do for most cases but why not make sure by
    // adding one to the nPopCnt argument here? Would it make some things more
    // complicated later?
    Word_t *pwList = NewListTypeList(nPopCnt, nBL);
    Word_t wRootNew = 0;
#if defined(UA_PARALLEL_128)
    if ((nBL == 16) && (nPopCnt <= 6)) {
        set_wr(wRootNew, pwList, T_LIST_UA);
    } else
#endif // defined(UA_PARALLEL_128)
    { set_wr(wRootNew, pwList, T_LIST); }
#if defined(PP_IN_LINK)
    if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK)
    {
        Set_xListPopCnt(&wRootNew, nBL, nPopCnt);
    }

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
  #if defined(DEBUG_REMOVE)
//            printf("nn %d nSlot %d pcKeys[?] 0x%x\n", nn, nSlot, pcKeys[nn]);
  #endif // defined(DEBUG_REMOVE)
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
            if (nBL == 24) {
                /*printf("pwList %p piKeys %p piKeys[%d] %x\n",
                       (void *)pwList, (void *)piKeys, nn, piKeys[nn]);*/
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
            pwKeys[nn] = (wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM);
        }
    }

    // What about padding the bucket and/or malloc buffer?

    return wRootNew; // wRootNew is not installed; *pwRoot is intact
}

// Replace an external T_LIST leaf with a wRoot with embedded keys.
// The function assumes it is possible.
Word_t
DeflateExternalList(Word_t *pwRoot,
                    int nPopCnt, int nBL, Word_t *pwr)
{
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(EK_CALC_POP)
    assert(0); // not yet
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(EK_CALC_POP)
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
//printf("DEL: nBL %d nPopCntMax %d\n", nBL, nPopCntMax);

    assert( (wr_nType(*pwRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
           || (wr_nType(*pwRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );

    DBGI(printf("DeflateExternalList pwRoot %p nPopCnt %d nBL %d pwr %p\n",
               (void *)pwRoot, nPopCnt, nBL, (void *)pwr));
#if defined(DEBUG_INSERT)
    //HexDump("External List", pwr, nPopCnt + 1);
    Dump(pwRoot, 0, nBL);
#endif // defined(DEBUG_INSERT)

    assert((nPopCnt <= nPopCntMax) || (nPopCnt == 1));

    Word_t wRoot = 0;

    assert(nPopCnt <= nPopCntMax);

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
//printf("nBL %d nPopCnt %d wRoot " OWx"\n", nBL, nPopCnt, wRoot);

    for (int nn = 0;
#if defined(FILL_W_KEY)
             nn < nPopCntMax;
#else // defined(FILL_W_KEY)
             nn < nPopCnt;
#endif // defined(FILL_W_KEY)
             nn++)
    {
        // Slots are numbered from msb to lsb starting at 1.
#if defined(REVERSE_SORT_EMBEDDED_KEYS)
  #if defined(PACK_KEYS_RIGHT)
        int nSlot = (nPopCntMax - nn);
  #else // defined(PACK_KEYS_RIGHT)
        int nSlot = (nPopCnt - nn);
  #if defined(FILL_W_KEY)
      #error Sorry, REVERSE_SORT && FILL_W_KEY && ! PACK_KEYS_RIGHT.
  #endif // defined(FILL_W_KEY)
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
                    pcKeys[(nn < nPopCnt) ? nn :
  #if defined(FILL_W_BIG_KEY)
                            nPopCnt - 1
  #else // defined(FILL_W_BIG_KEY)
                            0
  #endif // defined(FILL_W_BIG_KEY)
                        ]);
            DBGR(printf("nn %d nSlot %d pcKeys[?] 0x%x\n", nn, nSlot,
                    pcKeys[(nn < nPopCnt) ? nn : 0]));
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNAT(pwr);
            DBGI(printf("nn %d nSlot %d psKeys[?] 0x%x\n", nn, nSlot,
                    psKeys[(nn < nPopCnt) ? nn : 0]));
            SetBits(&wRoot, nBL, /* lsb */ cnBitsPerWord - (nSlot * nBL),
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

    OldList(pwr, nPopCnt, nBL, wr_nType(*pwRoot));

    goto done;
done:;

    *pwRoot = wRoot;

    DBGI(printf("DEL wRoot " OWx"\n", wRoot));

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

    DBGI(printf("SetBit(pwRoot " OWx" wKey " OWx")\n",
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

    DBGI(printf("SetBit(pwr " OWx" wKey " OWx") pwRoot %p\n",
                (Word_t)pwr, wKey & MSK(nBL), (void *)pwRoot));

    SetBit(pwr, wKey & MSK(nBL));

    // population is in the word following the bitmap
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
DBGR(printf("RC: pwRoot %p wRoot " OWx" nBL %d nBLR %d\n", (void *)pwRoot, wRoot, nBL, nBLR));
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

        int nBitsIndexSz;
      #if defined(CODE_XX_SW)
        if (tp_bIsXxSw(nType)) {
            nBitsIndexSz = pwr_nBW(pwRoot);
        } else
      #endif // defined(CODE_XX_SW)
        { nBitsIndexSz = nBL_to_nBitsIndexSz(nBL); }

        Word_t wIndex = (wKey >> (nBLR - nBitsIndexSz)) & MSK(nBitsIndexSz);
        for (Word_t ww = 0; ww < EXP(nBitsIndexSz); ww++)
        {
            Word_t *pwRootLn = &((
#if defined(CODE_BM_SW)
                tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
                                      pwr_pLinks((  Switch_t *)pwr))
                    [ww].ln_wRoot);

            int nBLX = wr_bIsSwitch(*pwRootLn)
                            && tp_bIsSkip(wr_nType(*pwRootLn)) ?
                         wr_nBL(*pwRootLn)
                      : nDL_to_nBL(nDLR - 1);

            //--nDLX;
            if (((*pwRootLn != 0) && (ww != wIndex))
                    || (PWR_wPopCntBL(pwRootLn, (Switch_t *)NULL, nBLX) != 0))
            {
                DBGR(printf("wr_nBLX %d", nBLX));
                DBGR(printf(" PWR_wPopCntBL %" _fw"d " OWx"\n",
                            PWR_wPopCntBL(pwRootLn, NULL, nBLX),
                            PWR_wPopCntBL(pwRootLn, NULL, nBLX)));
                DBGR(printf(" PWR_wPrefixPop %" _fw"d " OWx"\n",
                            PWR_wPrefixPop(pwRootLn, NULL),
                            PWR_wPrefixPop(pwRootLn, NULL)));
                DBGR(printf("Not empty ww %zd wIndex %zd *pwRootLn " OWx"\n",
                     (size_t)ww, (size_t)wIndex, *pwRootLn));
                return; // may need cleanup lower; caller checks *pwRoot
            }
        }
        // whole array pop is zero
        FreeArrayGuts(pwRoot, wKey, nBL, /* bDump */ 0);
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

    DBGR(printf("RemoveGuts(pwRoot %p wKey " OWx" nBL %d wRoot " OWx")\n",
                (void *)pwRoot, wKey, nBL, wRoot));

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
    if (((nType == T_EMBEDDED_KEYS))
// Why is nBL_to_nBitsPopCntSz irrelevant here?
        && (nBL <= cnBitsPerWord - cnBitsMallocMask))
    {
        goto embeddedKeys;
embeddedKeys:;
        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot);
        *pwRoot = wRoot;
        // Is there any reason to preserve *pwRoot?
        // Is it a problem to have an external list that could
        // be embedded?
        nType = wr_nType(wRoot);
        pwr = wr_pwr(wRoot);
        assert(wr_nType(wRoot) == nType);
    }
#endif // defined(EMBED_KEYS)

    assert( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
           || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );
    assert( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
           || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );
    assert(nType == wr_nType(wRoot));

    Word_t wPopCnt;

#if defined(PP_IN_LINK)
    // this test only works if we disallow skip links from the top
    // for PP_IN_LINK.
    if (nBL < cnBitsPerWord) {
        wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL) + 1;
    } else
#endif // defined(PP_IN_LINK)
    {
        wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
    }

    // wPopCnt is the count before the remove.

    if (wPopCnt == 1) {
        assert( ! tp_bIsSwitch(nType) );
        OldList(pwr, wPopCnt, nBL, nType);
#if defined(NO_TYPE_IN_XX_SW)
        if (nBL < nDL_to_nBL(2)) { *pwRoot = ZERO_POP_MAGIC; } else
#endif // defined(NO_TYPE_IN_XX_SW)
        { *pwRoot = 0; }
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }

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
    }
    else
    {
        pwList = pwr;
    }

    if (pwList != pwr) {
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

#if defined(UA_PARALLEL_128)
        if ((nBL == 16) && (wPopCnt - 1 <= 6)) {
            set_wr(wRoot, pwList, T_LIST_UA);
        } else
#endif // defined(UA_PARALLEL_128)
        { set_wr(wRoot, pwList, T_LIST); }
    }

#if defined(PP_IN_LINK)
    if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK)
    {
        Set_xListPopCnt(&wRoot, nBL, wPopCnt - 1);
    }

#if defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
        unsigned nKeys = wPopCnt - 1; (void)nKeys;
#endif // defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        MOVE(&ls_pcKeysNAT(pwList)[nIndex],
             &ls_pcKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n;
#if defined(PSPLIT_PARALLEL)
        // pad list to an integral number of parallel search buckets in length
        for (; (n * 1) % sizeof(Bucket_t); ++n) {
            ls_pcKeysNAT(pwr)[n] = ls_pcKeysNAT(pwr)[n-1];
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_pcKeysNAT(pwList)[n] = -1;
#endif // defined(LIST_END_MARKERS)
    } else if (nBL <= 16) {
        MOVE(&ls_psKeysNAT(pwList)[nIndex],
             &ls_psKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n; // first empty slot
#if defined(PSPLIT_PARALLEL)
  #if defined(UA_PARALLEL_128)
        if (n <= 6) {
            for (; (n * 2) % 12; ++n) {
                ls_psKeysNAT(pwr)[n] = ls_psKeysNAT(pwr)[n-1];
            }
        } else
  #endif // defined(UA_PARALLEL_128)
        {
            for (; (n * 2) % sizeof(Bucket_t); ++n) {
                ls_psKeysNAT(pwr)[n] = ls_psKeysNAT(pwr)[n-1];
            }
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_psKeysNAT(pwList)[n] = -1;
#endif // defined(LIST_END_MARKERS)
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        MOVE(&ls_piKeysNAT(pwList)[nIndex],
             &ls_piKeysNAT(pwr)[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n;
#if defined(PSPLIT_PARALLEL)
        // pad list to an integral number of parallel search buckets in length
        for (; (n * 4) % sizeof(Bucket_t); ++n) {
            ls_piKeysNAT(pwr)[n] = ls_piKeysNAT(pwr)[n-1];
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_piKeysNAT(pwList)[n] = -1;
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
        int n = wPopCnt - 1; (void)n;
#if defined(PSPLIT_PARALLEL) && defined(PSPLIT_SEARCH_WORD)
        // pad list to an integral number of parallel search buckets in length
        for (; (n * sizeof(Word_t)) % sizeof(Bucket_t); ++n) {
            ls_pwKeysNAT(pwr)[n] = ls_pwKeysNAT(pwr)[n-1];
        }
#endif // defined(PSPLIT_PARALLEL) && defined(PSPLIT_SEARCH_WORD)
#if defined(LIST_END_MARKERS)
        ls_pwKeys(pwList, nBL)[n] = -1;
#endif // defined(LIST_END_MARKERS)
    }

    if (pwList != pwr)
    {
        OldList(pwr, wPopCnt, nBL, nType);
    }

    *pwRoot = wRoot;

#if defined(EMBED_KEYS)
    // Embed the list if it fits.
    assert( (wr_nType(wRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
           || (wr_nType(wRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );
    assert( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
           || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );
    if ((int)wPopCnt <= EmbeddedListPopCntMax(nBL) + 1) {
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

    // EXP(nBL) is risky because nBL could be cnBitsPerWord
    if (nBL <= (int)LOG(sizeof(Link_t) * 8)) {
        ClrBit(STRUCT_OF(pwRoot, Link_t, ln_wRoot), wKey & MSK(nBL));
    } else {
        int nBLR = nBL;
  #if defined(SKIP_TO_BITMAP)
        if (wr_nType(*pwRoot) == T_SKIP_TO_BITMAP) {
            nBLR = Get_nBLR(pwRoot);
        }
  #endif // defined(SKIP_TO_BITMAP)
        Word_t *pwr = wr_pwr(wRoot);

        ClrBit(pwr, wKey & MSK(nBLR));

        set_w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR,
            w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR) - 1);

#if defined(DEBUG_COUNT)
        Word_t wPopCnt = 0;
        for (Word_t ww = 0; ww < EXP(nBLR - cnLogBitsPerWord); ww++) {
            wPopCnt += __builtin_popcountll(pwr[ww]);
        }
        assert(wPopCnt == w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR));
#endif // defined(DEBUG_COUNT)

#if defined(PP_IN_LINK)
        assert(PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBLR)
            == w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR));
#endif // defined(PP_IN_LINK)

        // Free the bitmap if it is empty.
        if (w_wPopCntBL(*(pwr + EXP(nBLR - cnLogBitsPerWord)), nBLR) == 0) {
            OldBitmap(pwRoot, pwr, nBL);
        }
    }

    return Success;
}

#endif // (cnDigitsPerWord != 1)

static void
Initialize(void)
{
    // There may be an issue with dlmalloc and greater than 2MB (size of huge
    // page) requests. Dlmalloc may mmap something other than an integral
    // multiple of 2MB. Since our bitmaps contain an extra word at the end
    // we need to be careful about bitmaps that 2MB plus one word and bigger.
    assert((cnBitsLeftAtDl2 < 24)
        || ((cn2dBmMaxWpkPercent == 0) && (cnBitsInD1 < 24)));
#if defined(UA_PARALLEL_128)
    assert(cnBitsMallocMask >= 4);
    for (int i = 1; i <= 6; i++) {
        if (ListWordsTypeList(i, 16) != 3) {
            printf("ListWordsTypeList(%d, 16) %d\n",
                   i, ListWordsTypeList(i, 16));
        }
        assert(ListWordsTypeList(i, 16) == 3);
    }
    assert(ListWordsTypeList(7, 16) > 3);
#endif // defined(UA_PARALLEL_128)

#if defined(CODE_BM_SW) && ! defined(PP_IN_LINK)
    assert(&((BmSwitch_t *)0)->sw_wPrefixPop == &((Switch_t *)0)->sw_wPrefixPop);
#endif // defined(CODE_BM_SW) && ! defined(PP_IN_LINK)
#if defined(NO_TYPE_IN_XX_SW)
  #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
    assert(T_EMBEDDED_KEYS != 0); // see b.h
  #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // defined(NO_TYPE_IN_XX_SW)
#if ! defined(EMBED_KEYS)
    // We don't support NO_EMBED_KEYS with cnListPopCntMax<X> == 0.
    assert(cnListPopCntMaxDl1 != 0);
  #if defined(cnListPopCntMaxDl2)
    assert(cnListPopCntMaxDl2 != 0);
  #endif // defined(cnListPopCntMaxDl2)
  #if defined(cnListPopCntMaxDl3)
    assert(cnListPopCntMaxDl3 != 0);
  #endif // defined(cnListPopCntMaxDl3)
    assert(cnListPopCntMax8   != 0);
    assert(cnListPopCntMax16  != 0);
    assert(cnListPopCntMax32  != 0);
  #if cnBitsPerWord > 32
    assert(cnListPopCntMax64  != 0);
  #endif // cnBitsPerWord > 32
#endif // ! defined(EMBED_KEYS)

    // Search assumes lists are sorted if LIST_END_MARKERS is defined.
#if defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)
    assert(0);
#endif // defined(LIST_END_MARKERS) && ! defined(SORT_LISTS)

#if ! defined(SKIP_TO_BITMAP) && defined(SKIP_TO_XX_SW)
    #error SKIP_TO_XX_SW without SKIP_TO_BITMAP
#endif // ! defined(SKIP_TO_BITMAP) && defined(SKIP_TO_XX_SW)

    // Why would we want to be able to fit more than one digits' worth of
    // keys into a Link_t as an embedded bitmap?
    // An uncompressed switch of such links would be bigger than the
    // corresponding bitmap and it would be slower.
    // But a compressed switch might provide some value?
    // And a skip link to such a bitmap might provide some value?
    // And, unlike a link that is completely filled with an embedded
    // bitmap, there would be room for a type field.
    // For experimentation?
    if (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8) {
        printf("# Warning: (EXP(cnBitsLeftAtDl2) <= sizeof(Link_t) * 8)"
                 " makes no sense.\n");
        printf("# Maybe increase cnBitsInD[12] or decrease sizeof(Link_t).\n");
        printf("# Or increase cnBitsPerDigit.\n");
    }
    // Why would we want to be able to fit a whole digit's worth of
    // keys into a Link_t as an embedded bitmap?
    // A real bitmap encompassing a whole uncompressed switch of such bitmaps
    // would use no more memory and it would be faster.
    // But a compressed switch might provide some value?
    // And a skip link to such a bitmap might provide some value?
    // For experimentation?
    // Does JudyL change the situation?
    else if (EXP(cnBitsInD1) <= sizeof(Link_t) * 8) {
        printf("# Warning: (EXP(cnBitsInD1) <= sizeof(Link_t) * 8)"
                 " makes no sense.\n");
        printf("# Mabye increase cnBitsInD1 or decrease sizeof(Link_t).\n");
    }
#if ! defined(ALLOW_EMBEDDED_BITMAP)
    assert(EXP(cnBitsInD1) > sizeof(Link_t) * 8);
#endif // ! defined(ALLOW_EMBEDDED_BITMAP)
    assert(EXP(cnBitsLeftAtDl2) > sizeof(Link_t) * 8);

// SAVE_PREFIX should be called SAVE_PREFIX_PTR?
#if defined(SAVE_PREFIX)
  #if ! defined(SKIP_PREFIX_CHECK)
    // SKIP_PREFIX_CHECK should be called SKIP_OR_DEFER_PREFIX_CHECK?
    // Maybe SKIP_OR_DEFER should't exist?
    // Maybe it should be SAVE (implies DEFER) or SKIP?
    #error SAVE_PREFIX without SKIP_PREFIX_CHECK
  #endif // ! defined(SKIP_PREFIX_CHECK)
#endif // defined(SAVE_PREFIX)

#if defined(SKIP_LINKS)
  #if ! defined(LVL_IN_WR_HB)
      #if ! defined(LVL_IN_SW)
    // We could be a lot more creative here w.r.t. mapping our scarce type
    // values to absolute depths.  But why?  We have to look at the prefix
    // in a different word anyway.  See comments at tp_to_nDL in b.h.
    if (nDL_to_tp(cnDigitsPerWord - 1) > cnMallocMask) {
        printf("\n");
        printf("nDL_to_tp(cnDigitsPerWord   %2d) 0x%02x\n",
               cnDigitsPerWord, nDL_to_tp(cnDigitsPerWord));
        printf("tp_to_nDL(cnMallocMask    0x%02x)   %2d\n",
               (int)cnMallocMask, (int)tp_to_nDL(cnMallocMask));
    }
      #endif // ! defined(LVL_IN_SW)
  #endif // ! defined(LVL_IN_WR_HB)
#else // defined(SKIP_LINKS)
  #if defined(SKIP_TO_BITMAP)
      #error SKIP_TO_BITMAP without SKIP_LINKS
  #endif // defined(SKIP_TO_BITMAP)
  #if defined(SKIP_TO_LIST)
      #error SKIP_TO_LIST without SKIP_LINKS
  #endif // defined(SKIP_TO_LIST)
  #if defined(SKIP_TO_SWITCH)
      #error SKIP_TO_SWITCH without SKIP_LINKS
  #endif // defined(SKIP_TO_SWITCH)
  #if defined(SKIP_TO_BM_SW)
      #error SKIP_TO_BM_SW without SKIP_LINKS
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(SKIP_TO_XX_SW)
      #error SKIP_TO_XX_SW without SKIP_LINKS
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(SKIP_LINKS)


  #if defined(NO_TYPE_IN_XX_SW)
      #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
    // Not sure if/why this matters.
    assert (wr_nType(ZERO_POP_MAGIC) == T_EMBEDDED_KEYS);
      #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)
  #endif // defined(NO_TYPE_IN_XX_SW)

  #if defined(CODE_XX_SW)
    // Make sure nBW field is big enough.
    assert((cnBitsLeftAtDl2 - cnBW - (cnLogBitsPerWord + 1))
        <= (int)MSK(cnBitsXxSwWidth));
  #endif // defined(CODE_XX_SW)

#if defined(BPD_TABLE_RUNTIME_INIT)
    for (unsigned nDL = 0;
         nDL < sizeof(anDL_to_nBitsIndexSz) / sizeof(anDL_to_nBitsIndexSz[0]);
         nDL++)
    {
        anDL_to_nBitsIndexSz[nDL] = nBitsIndexSz_from_nDL(nDL);
    }
    for (unsigned nDL = 0;
         nDL < sizeof(anDL_to_nBL) / sizeof(anDL_to_nBL[0]); ++nDL)
    {
        anDL_to_nBL[nDL] = nBL_from_nDL(nDL);
    }
    for (unsigned nBL = 0;
         nBL < sizeof(anBL_to_nDL) / sizeof(anBL_to_nDL[0]); ++nBL)
    {
        anBL_to_nDL[nBL] = nDL_from_nBL(nBL);
    }
#endif // defined(BPD_TABLE_RUNTIME_INIT)

    printf("\n");

#if defined(POP_IN_WR_HB)
    printf("#    POP_IN_WR_HB\n");
#else // defined(POP_IN_WR_HB)
    printf("# No POP_IN_WR_HB\n");
#endif // defined(POP_IN_WR_HB)

#if defined(LIST_POP_IN_PREAMBLE)
    printf("#    LIST_POP_IN_PREAMBLE\n");
#else // defined(LIST_POP_IN_PREAMBLE)
    printf("# No LIST_POP_IN_PREAMBLE\n");
#endif // defined(LIST_POP_IN_PREAMBLE)

#if defined(LIST_REQ_MIN_WORDS)
    printf("#    LIST_REQ_MIN_WORDS\n");
#else // defined(LIST_REQ_MIN_WORDS)
    printf("# No LIST_REQ_MIN_WORDS\n");
#endif // defined(LIST_REQ_MIN_WORDS)

#if defined(LVL_IN_WR_HB)
    printf("#    LVL_IN_WR_HB\n");
#else // defined(LVL_IN_WR_HB)
    printf("# No LVL_IN_WR_HB\n");
#endif // defined(LVL_IN_WR_HB)

#if defined(ALLOW_EMBEDDED_BITMAP)
    printf("#    ALLOW_EMBEDDED_BITMAP\n");
#else // defined(ALLOW_EMBEDDED_BITMAP)
    printf("# No ALLOW_EMBEDDED_BITMAP\n");
#endif // defined(ALLOW_EMBEDDED_BITMAP)

#if defined(COMPRESSED_LISTS)
    printf("#    COMPRESSED_LISTS\n");
#else // defined(COMPRESSED_LISTS)
    printf("# No COMPRESSED_LISTS\n");
#endif // defined(COMPRESSED_LISTS)

#if defined(SORT_LISTS)
    printf("#    SORT_LISTS\n");
#else // defined(SORT_LISTS)
    printf("# No SORT_LISTS\n");
#endif // defined(SORT_LISTS)

#if defined(ALIGN_LISTS)
    printf("#    ALIGN_LISTS\n");
#else // defined(ALIGN_LISTS)
    printf("# No ALIGN_LISTS\n");
#endif // defined(ALIGN_LISTS)

#if defined(ALIGN_LIST_LENS)
    printf("#    ALIGN_LIST_LENS\n");
#else // defined(ALIGN_LIST_LENS)
    printf("# No ALIGN_LIST_LENS\n");
#endif // defined(ALIGN_LIST_LENS)

#if defined(SKIP_LINKS)
    printf("#    SKIP_LINKS\n");
#else // defined(SKIP_LINKS)
    printf("# No SKIP_LINKS\n");
#endif // defined(SKIP_LINKS)

#if defined(SKIP_TO_BITMAP)
    printf("#    SKIP_TO_BITMAP\n");
#else // defined(SKIP_TO_BITMAP)
    printf("# No SKIP_TO_BITMAP\n");
#endif // defined(SKIP_TO_BITMAP)

#if defined(USE_XX_SW)
    printf("#    USE_XX_SW\n");
#else // defined(USE_XX_SW)
    printf("# No USE_XX_SW\n");
#endif // defined(USE_XX_SW)

#if defined(CODE_XX_SW)
    printf("#    CODE_XX_SW\n");
#else // defined(CODE_XX_SW)
    printf("# No CODE_XX_SW\n");
#endif // defined(CODE_XX_SW)

#if defined(SKIP_TO_XX_SW)
    printf("#    SKIP_TO_XX_SW\n");
#else // defined(SKIP_TO_XX_SW)
    printf("# No SKIP_TO_XX_SW\n");
#endif // defined(SKIP_TO_XX_SW)

#if defined(XX_SHORTCUT)
    printf("#    XX_SHORTCUT\n");
#else // defined(XX_SHORTCUT)
    printf("# No XX_SHORTCUT\n");
#endif // defined(XX_SHORTCUT)

#if defined(XX_SHORTCUT_GOTO)
    printf("#    XX_SHORTCUT_GOTO\n");
#else // defined(XX_SHORTCUT_GOTO)
    printf("# No XX_SHORTCUT_GOTO\n");
#endif // defined(XX_SHORTCUT_GOTO)

#if defined(EMBED_KEYS)
    printf("#    EMBED_KEYS\n");
#else // defined(EMBED_KEYS)
    printf("# No EMBED_KEYS\n");
#endif // defined(EMBED_KEYS)

#if defined(EK_CALC_POP)
    printf("#    EK_CALC_POP\n");
#else // defined(EK_CALC_POP)
    printf("# No EK_CALC_POP\n");
#endif // defined(EK_CALC_POP)

#if defined(BL_SPECIFIC_PSPLIT_SEARCH)
    printf("#    BL_SPECIFIC_PSPLIT_SEARCH\n");
#else // defined(BL_SPECIFIC_PSPLIT_SEARCH)
    printf("# No BL_SPECIFIC_PSPLIT_SEARCH\n");
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH)

#if defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
    printf("#    BL_SPECIFIC_PSPLIT_SEARCH_WORD\n");
#else // defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)
    printf("# No BL_SPECIFIC_PSPLIT_SEARCH_WORD\n");
#endif // defined(BL_SPECIFIC_PSPLIT_SEARCH_WORD)

#if defined(PSPLIT_SEARCH_8)
    printf("#    PSPLIT_SEARCH_8\n");
#else // defined(PSPLIT_SEARCH_8)
    printf("# No PSPLIT_SEARCH_8\n");
#endif // defined(PSPLIT_SEARCH_8)

#if defined(PSPLIT_SEARCH_16)
    printf("#    PSPLIT_SEARCH_16\n");
#else // defined(PSPLIT_SEARCH_16)
    printf("# No PSPLIT_SEARCH_16\n");
#endif // defined(PSPLIT_SEARCH_16)

#if defined(PSPLIT_SEARCH_32)
    printf("#    PSPLIT_SEARCH_32\n");
#else // defined(PSPLIT_SEARCH_32)
    printf("# No PSPLIT_SEARCH_32\n");
#endif // defined(PSPLIT_SEARCH_32)

#if defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)
    printf("# No EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_PARALLEL_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)
    printf("# No EMBEDDED_KEYS_PARALLEL_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_PARALLEL_FOR_INSERT)

#if defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)
    printf("# No EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)
    printf("# No EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_PSPLIT_BY_KEY_FOR_INSERT)

#if defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)
    printf("#    EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP\n");
#else // defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)
    printf("# No EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP\n");
#endif // defined(EMBEDDED_KEYS_UNROLLED_FOR_LOOKUP)

#if defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)
    printf("#    EMBEDDED_KEYS_UNROLLED_FOR_INSERT\n");
#else // defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)
    printf("# No EMBEDDED_KEYS_UNROLLED_FOR_INSERT\n");
#endif // defined(EMBEDDED_KEYS_UNROLLED_FOR_INSERT)

#if defined(PSPLIT_PARALLEL)
    printf("#    PSPLIT_PARALLEL\n");
#else // defined(PSPLIT_PARALLEL)
    printf("# No PSPLIT_PARALLEL\n");
#endif // defined(PSPLIT_PARALLEL)

#if defined(PSPLIT_PARALLEL_WORD)
    printf("#    PSPLIT_PARALLEL_WORD\n");
#else // defined(PSPLIT_PARALLEL_WORD)
    printf("# No PSPLIT_PARALLEL_WORD\n");
#endif // defined(PSPLIT_PARALLEL_WORD)

#if defined(UA_PARALLEL_128)
    printf("#    UA_PARALLEL_128\n");
#else // defined(UA_PARALLEL_128)
    printf("# No UA_PARALLEL_128\n");
#endif // defined(UA_PARALLEL_128)

#if defined(PARALLEL_128)
    printf("#    PARALLEL_128\n");
#else // defined(PARALLEL_128)
    printf("# No PARALLEL_128\n");
#endif // defined(PARALLEL_128)

#if defined(PARALLEL_64)
    printf("#    PARALLEL_64\n");
#else // defined(PARALLEL_64)
    printf("# No PARALLEL_64\n");
#endif // defined(PARALLEL_64)

#if defined(OLD_HK_128)
    printf("#    OLD_HK_128\n");
#else // defined(OLD_HK_128)
    printf("# No OLD_HK_128\n");
#endif // defined(OLD_HK_128)

#if defined(HK_MOVEMASK)
    printf("#    HK_MOVEMASK\n");
#else // defined(HK_MOVEMASK)
    printf("# No HK_MOVEMASK\n");
#endif // defined(HK_MOVEMASK)

#if defined(HK_EXPERIMENT)
    printf("#    HK_EXPERIMENT\n");
#else // defined(HK_EXPERIMENT)
    printf("# No HK_EXPERIMENT\n");
#endif // defined(HK_EXPERIMENT)

#if defined(PSPLIT_EARLY_OUT)
    printf("#    PSPLIT_EARLY_OUT\n");
#else // defined(PSPLIT_EARLY_OUT)
    printf("# No PSPLIT_EARLY_OUT\n");
#endif // defined(PSPLIT_EARLY_OUT)

#if defined(BINARY_SEARCH_WORD)
    printf("#    BINARY_SEARCH_WORD\n");
#else // defined(BINARY_SEARCH_WORD)
    printf("# No BINARY_SEARCH_WORD\n");
#endif // defined(BINARY_SEARCH_WORD)

#if defined(RAMMETRICS)
    printf("#    RAMMETRICS\n");
#else // defined(RAMMETRICS)
    printf("# No RAMMETRICS\n");
#endif // defined(RAMMETRICS)

#if defined(NDEBUG)
    printf("#    NDEBUG\n");
#else // defined(NDEBUG)
    printf("# No NDEBUG\n");
#endif // defined(NDEBUG)

#if defined(SPLIT_SEARCH_BINARY)
    printf("#    SPLIT_SEARCH_BINARY\n");
#else // defined(SPLIT_SEARCH_BINARY)
    printf("# No SPLIT_SEARCH_BINARY\n");
#endif // defined(SPLIT_SEARCH_BINARY)

#if defined(PSPLIT_HYBRID)
    printf("#    PSPLIT_HYBRID\n");
#else // defined(PSPLIT_HYBRID)
    printf("# No PSPLIT_HYBRID\n");
#endif // defined(PSPLIT_HYBRID)

#if defined(PSPLIT_SEARCH_WORD)
    printf("#    PSPLIT_SEARCH_WORD\n");
#else // defined(PSPLIT_SEARCH_WORD)
    printf("# No PSPLIT_SEARCH_WORD\n");
#endif // defined(PSPLIT_SEARCH_WORD)

#if defined(PSPLIT_SEARCH_XOR_WORD)
    printf("#    PSPLIT_SEARCH_XOR_WORD\n");
#else // defined(PSPLIT_SEARCH_XOR_WORD)
    printf("# No PSPLIT_SEARCH_XOR_WORD\n");
#endif // defined(PSPLIT_SEARCH_XOR_WORD)

#if defined(LIST_END_MARKERS)
    printf("#    LIST_END_MARKERS\n");
#else // defined(LIST_END_MARKERS)
    printf("# No LIST_END_MARKERS\n");
#endif // defined(LIST_END_MARKERS)

#if defined(OLD_LISTS)
    printf("#    OLD_LISTS\n");
#else // defined(OLD_LISTS)
    printf("# No OLD_LISTS\n");
#endif // defined(OLD_LISTS)

#if defined(JUMP_TABLE)
    printf("#    JUMP_TABLE\n");
#else // defined(JUMP_TABLE)
    printf("# No JUMP_TABLE\n");
#endif // defined(JUMP_TABLE)

#if defined(BACKWARD_SEARCH_WORD)
    printf("#    BACKWARD_SEARCH_WORD\n");
#else // defined(BACKWARD_SEARCH_WORD)
    printf("# No BACKWARD_SEARCH_WORD\n");
#endif // defined(BACKWARD_SEARCH_WORD)

#if defined(TRY_MEMCHR)
    printf("#    TRY_MEMCHR\n");
#else // defined(TRY_MEMCHR)
    printf("# No TRY_MEMCHR\n");
#endif // defined(TRY_MEMCHR)

#if defined(RECURSIVE)
    printf("#    RECURSIVE\n");
#else // defined(RECURSIVE)
    printf("# No RECURSIVE\n");
#endif // defined(RECURSIVE)

#if defined(RECURSIVE_INSERT)
    printf("#    RECURSIVE_INSERT\n");
#else // defined(RECURSIVE_INSERT)
    printf("# No RECURSIVE_INSERT\n");
#endif // defined(RECURSIVE_INSERT)

#if defined(RECURSIVE_REMOVE)
    printf("#    RECURSIVE_REMOVE\n");
#else // defined(RECURSIVE_REMOVE)
    printf("# No RECURSIVE_REMOVE\n");
#endif // defined(RECURSIVE_REMOVE)

#if defined(LOOKUP_NO_LIST_DEREF)
    printf("#    LOOKUP_NO_LIST_DEREF\n");
#else // defined(LOOKUP_NO_LIST_DEREF)
    printf("# No LOOKUP_NO_LIST_DEREF\n");
#endif // defined(LOOKUP_NO_LIST_DEREF)

#if defined(LOOKUP_NO_LIST_SEARCH)
    printf("#    LOOKUP_NO_LIST_SEARCH\n");
#else // defined(LOOKUP_NO_LIST_SEARCH)
    printf("# No LOOKUP_NO_LIST_SEARCH\n");
#endif // defined(LOOKUP_NO_LIST_SEARCH)

#if defined(ONE_DEREF_AT_LIST)
    printf("#    ONE_DEREF_AT_LIST\n");
#else // defined(ONE_DEREF_AT_LIST)
    printf("# No ONE_DEREF_AT_LIST\n");
#endif // defined(ONE_DEREF_AT_LIST)

#if defined(LOOKUP_NO_BITMAP_DEREF)
    printf("#    LOOKUP_NO_BITMAP_DEREF\n");
#else // defined(LOOKUP_NO_BITMAP_DEREF)
    printf("# No LOOKUP_NO_BITMAP_DEREF\n");
#endif // defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(LOOKUP_NO_BITMAP_SEARCH)
    printf("#    LOOKUP_NO_BITMAP_SEARCH\n");
#else // defined(LOOKUP_NO_BITMAP_SEARCH)
    printf("# No LOOKUP_NO_BITMAP_SEARCH\n");
#endif // defined(LOOKUP_NO_BITMAP_SEARCH)

#if defined(ZERO_POP_CHECK_BEFORE_GOTO)
    printf("#    ZERO_POP_CHECK_BEFORE_GOTO\n");
#else // defined(ZERO_POP_CHECK_BEFORE_GOTO)
    printf("# No ZERO_POP_CHECK_BEFORE_GOTO\n");
#endif // defined(ZERO_POP_CHECK_BEFORE_GOTO)

#if defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    printf("#    HANDLE_DL2_IN_EMBEDDED_KEYS\n");
#else // defined(HANDLE_DL2_IN_EMBEDDED_KEYS)
    printf("# No HANDLE_DL2_IN_EMBEDDED_KEYS\n");
#endif // defined(HANDLE_DL2_IN_EMBEDDED_KEYS)

#if defined(HANDLE_BLOWOUTS)
    printf("#    HANDLE_BLOWOUTS\n");
#else // defined(HANDLE_BLOWOUTS)
    printf("# No HANDLE_BLOWOUTS\n");
#endif // defined(HANDLE_BLOWOUTS)

#if defined(NO_TYPE_IN_XX_SW)
    printf("#    NO_TYPE_IN_XX_SW\n");
#else // defined(NO_TYPE_IN_XX_SW)
    printf("# No NO_TYPE_IN_XX_SW\n");
#endif // defined(NO_TYPE_IN_XX_SW)

#if defined(FILL_W_KEY)
    printf("#    FILL_W_KEY\n");
#else // defined(FILL_W_KEY)
    printf("# No FILL_W_KEY\n");
#endif // defined(FILL_W_KEY)

#if defined(FILL_W_BIG_KEY)
    printf("#    FILL_W_BIG_KEY\n");
#else // defined(FILL_W_BIG_KEY)
    printf("# No FILL_W_BIG_KEY\n");
#endif // defined(FILL_W_BIG_KEY)

#if defined(FILL_WITH_ONES)
    printf("#    FILL_WITH_ONES\n");
#else // defined(FILL_WITH_ONES)
    printf("# No FILL_WITH_ONES\n");
#endif // defined(FILL_WITH_ONES)

#if defined(MASK_EMPTIES)
    printf("#    MASK_EMPTIES\n");
#else // defined(MASK_EMPTIES)
    printf("# No MASK_EMPTIES\n");
#endif // defined(MASK_EMPTIES)

#if defined(EMBEDDED_LIST_FIXED_POP)
    printf("#    EMBEDDED_LIST_FIXED_POP\n");
#else // defined(EMBEDDED_LIST_FIXED_POP)
    printf("# No EMBEDDED_LIST_FIXED_POP\n");
#endif // defined(EMBEDDED_LIST_FIXED_POP)

#if defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)
    printf("#    USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL\n");
#else // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)
    printf("# No USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL\n");
#endif // defined(USE_WORD_ARRAY_EMBEDDED_KEYS_PARALLEL)

#if defined(REVERSE_SORT_EMBEDDED_KEYS)
    printf("#    REVERSE_SORT_EMBEDDED_KEYS\n");
#else // defined(REVERSE_SORT_EMBEDDED_KEYS)
    printf("# No REVERSE_SORT_EMBEDDED_KEYS\n");
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS)

    printf("# No EXTRA_TYPES\n");

#if defined(POP_WORD)
    printf("#    POP_WORD\n");
#else // defined(POP_WORD)
    printf("# No POP_WORD\n");
#endif // defined(POP_WORD)

#if defined(POP_WORD_IN_LINK)
    printf("#    POP_WORD_IN_LINK\n");
#else // defined(POP_WORD_IN_LINK)
    printf("# No POP_WORD_IN_LINK\n");
#endif // defined(POP_WORD_IN_LINK)

#if defined(SEARCHMETRICS)
    printf("#    SEARCHMETRICS\n");
#else // defined(SEARCHMETRICS)
    printf("# No SEARCHMETRICS\n");
#endif // defined(SEARCHMETRICS)

#if defined(PWROOT_PARAMETER_FOR_LOOKUP)
    printf("#    PWROOT_PARAMETER_FOR_LOOKUP\n");
#else // defined(PWROOT_PARAMETER_FOR_LOOKUP)
    printf("# No PWROOT_PARAMETER_FOR_LOOKUP\n");
#endif // defined(PWROOT_PARAMETER_FOR_LOOKUP)

#if defined(PWROOT_AT_TOP_FOR_LOOKUP)
    printf("#    PWROOT_AT_TOP_FOR_LOOKUP\n");
#else // defined(PWROOT_AT_TOP_FOR_LOOKUP)
    printf("# No PWROOT_AT_TOP_FOR_LOOKUP\n");
#endif // defined(PWROOT_AT_TOP_FOR_LOOKUP)

#if defined(USE_PWROOT_FOR_LOOKUP)
    printf("#    USE_PWROOT_FOR_LOOKUP\n");
#else // defined(USE_PWROOT_FOR_LOOKUP)
    printf("# No USE_PWROOT_FOR_LOOKUP\n");
#endif // defined(USE_PWROOT_FOR_LOOKUP)

#if defined(LVL_IN_SW)
    printf("#    LVL_IN_SW\n");
#else // defined(LVL_IN_SW)
    printf("# No LVL_IN_SW\n");
#endif // defined(LVL_IN_SW)

#if defined(SEPARATE_T_NULL)
    printf("#    SEPARATE_T_NULL\n");
#else // defined(SEPARATE_T_NULL)
    printf("# No SEPARATE_T_NULL\n");
#endif // defined(SEPARATE_T_NULL)

#if defined(SKIP_TO_LIST)
    printf("#    SKIP_TO_LIST\n");
#else // defined(SKIP_TO_LIST)
    printf("# No SKIP_TO_LIST\n");
#endif // defined(SKIP_TO_LIST)

#if defined(BPD_TABLE)
    printf("#    BPD_TABLE\n");
#else // defined(BPD_TABLE)
    printf("# No BPD_TABLE\n");
#endif // defined(BPD_TABLE)

#if defined(BPD_TABLE_RUNTIME_INIT)
    printf("#    BPD_TABLE_RUNTIME_INIT\n");
#else // defined(BPD_TABLE_RUNTIME_INIT)
    printf("# No BPD_TABLE_RUNTIME_INIT\n");
#endif // defined(BPD_TABLE_RUNTIME_INIT)

#if defined(USE_BM_SW)
    printf("#    USE_BM_SW\n");
#else // defined(USE_BM_SW)
    printf("# No USE_BM_SW\n");
#endif // defined(USE_BM_SW)

#if defined(CODE_BM_SW)
    printf("#    CODE_BM_SW\n");
#else // defined(CODE_BM_SW)
    printf("# No CODE_BM_SW\n");
#endif // defined(CODE_BM_SW)

#if defined(BM_SW_FOR_REAL)
    printf("#    BM_SW_FOR_REAL\n");
#else // defined(BM_SW_FOR_REAL)
    printf("# No BM_SW_FOR_REAL\n");
#endif // defined(BM_SW_FOR_REAL)

#if defined(BITMAP_BY_BYTE)
    printf("#    BITMAP_BY_BYTE\n");
#else // defined(BITMAP_BY_BYTE)
    printf("# No BITMAP_BY_BYTE\n");
#endif // defined(BITMAP_BY_BYTE)

#if defined(SKIP_TO_BM_SW)
    printf("#    SKIP_TO_BM_SW\n");
#else // defined(SKIP_TO_BM_SW)
    printf("# No SKIP_TO_BM_SW\n");
#endif // defined(SKIP_TO_BM_SW)

#if defined(NO_OLD_LISTS)
    printf("#    NO_OLD_LISTS\n");
#else // defined(NO_OLD_LISTS)
    printf("# No NO_OLD_LISTS\n");
#endif // defined(NO_OLD_LISTS)

#if defined(NO_SKIP_AT_TOP)
    printf("#    NO_SKIP_AT_TOP\n");
#else // defined(NO_SKIP_AT_TOP)
    printf("# No NO_SKIP_AT_TOP\n");
#endif // defined(NO_SKIP_AT_TOP)

#if defined(PACK_KEYS_RIGHT)
    printf("#    PACK_KEYS_RIGHT\n");
#else // defined(PACK_KEYS_RIGHT)
    printf("# No PACK_KEYS_RIGHT\n");
#endif // defined(PACK_KEYS_RIGHT)

#if defined(RETYPE_FULL_BM_SW)
    printf("#    RETYPE_FULL_BM_SW\n");
#else // defined(RETYPE_FULL_BM_SW)
    printf("# No RETYPE_FULL_BM_SW\n");
#endif // defined(SKIP_TO_BM_SW)

#if defined(BM_IN_NON_BM_SW)
    printf("#    BM_IN_NON_BM_SW\n");
#else // defined(BM_IN_NON_BM_SW)
    printf("# No BM_IN_NON_BM_SW\n");
#endif // defined(BM_IN_NON_BM_SW)

#if defined(BM_IN_LINK)
    printf("#    BM_IN_LINK\n");
#else // defined(BM_IN_LINK)
    printf("# No BM_IN_LINK\n");
#endif // defined(BM_IN_LINK)

#if defined(PP_IN_LINK)
    printf("#    PP_IN_LINK\n");
#else // defined(PP_IN_LINK)
    printf("# No PP_IN_LINK\n");
#endif // defined(PP_IN_LINK)

#if defined(SKIP_PREFIX_CHECK)
    printf("#    SKIP_PREFIX_CHECK\n");
#else // defined(SKIP_PREFIX_CHECK)
    printf("# No SKIP_PREFIX_CHECK\n");
#endif // defined(SKIP_PREFIX_CHECK)

#if defined(NO_UNNECESSARY_PREFIX)
    printf("#    NO_UNNECESSARY_PREFIX\n");
#else // defined(NO_UNNECESSARY_PREFIX)
    printf("# No NO_UNNECESSARY_PREFIX\n");
#endif // defined(NO_UNNECESSARY_PREFIX)

#if defined(SAVE_PREFIX)
    printf("#    SAVE_PREFIX\n");
#else // defined(SAVE_PREFIX)
    printf("# No SAVE_PREFIX\n");
#endif // defined(SAVE_PREFIX)

#if defined(SAVE_PREFIX_TEST_RESULT)
    printf("#    SAVE_PREFIX_TEST_RESULT\n");
#else // defined(SAVE_PREFIX_TEST_RESULT)
    printf("# No SAVE_PREFIX_TEST_RESULT\n");
#endif // defined(SAVE_PREFIX_TEST_RESULT)

#if defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    printf("#    ALWAYS_CHECK_PREFIX_AT_LEAF\n");
#else // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)
    printf("# No ALWAYS_CHECK_PREFIX_AT_LEAF\n");
#endif // defined(ALWAYS_CHECK_PREFIX_AT_LEAF)

#if defined(SEARCH_FROM_WRAPPER)
    printf("#    SEARCH_FROM_WRAPPER\n");
#else // defined(SEARCH_FROM_WRAPPER)
    printf("# No SEARCH_FROM_WRAPPER\n");
#endif // defined(SEARCH_FROM_WRAPPER)

#if defined(DEBUG)
    printf("#    DEBUG\n");
#else // defined(DEBUG)
    printf("# No DEBUG\n");
#endif // defined(DEBUG)

#if defined(DEBUG_ALL)
    printf("#    DEBUG_ALL\n");
#else // defined(DEBUG_ALL)
    printf("# No DEBUG_ALL\n");
#endif // defined(DEBUG_ALL)

#if defined(DEBUG_INSERT)
    printf("#    DEBUG_INSERT\n");
#else // defined(DEBUG_INSERT)
    printf("# No DEBUG_INSERT\n");
#endif // defined(DEBUG_INSERT)

#if defined(DEBUG_REMOVE)
    printf("#    DEBUG_REMOVE\n");
#else // defined(DEBUG_REMOVE)
    printf("# No DEBUG_REMOVE\n");
#endif // defined(DEBUG_REMOVE)

#if defined(DEBUG_LOOKUP)
    printf("#    DEBUG_LOOKUP\n");
#else // defined(DEBUG_LOOKUP)
    printf("# No DEBUG_LOOKUP\n");
#endif // defined(DEBUG_LOOKUP)

#if defined(DEBUG_MALLOC)
    printf("#    DEBUG_MALLOC\n");
#else // defined(DEBUG_MALLOC)
    printf("# No DEBUG_MALLOC\n");
#endif // defined(DEBUG_MALLOC)

#if defined(GUARDBAND)
    printf("#    GUARDBAND\n");
#else // defined(GUARDBAND)
    printf("# No GUARDBAND\n");
#endif // defined(GUARDBAND)

#if defined(DEBUG_COUNT)
    printf("#    DEBUG_COUNT\n");
#else // defined(DEBUG_COUNT)
    printf("# No DEBUG_COUNT\n");
#endif // defined(DEBUG_COUNT)

#if defined(IF_SVALUE) // in RandomNumb.h
    printf("#    IF_SVALUE\n");
#else // defined(IF_SVALUE)
    printf("# No IF_SVALUE\n");
#endif // defined(IF_SVALUE)

#if defined(GAUSS) // in RandomNumb.h
    printf("#    GAUSS\n");
#else // defined(GAUSS)
    printf("# No GAUSS\n");
#endif // defined(GAUSS)

#if defined(USE_PDEP_INTRINSIC) // in Time.c
    printf("#    USE_PDEP_INTRINSIC\n");
#else // defined(USE_PDEP_INTRINSIC)
    printf("# No USE_PDEP_INTRINSIC\n");
#endif // defined(USE_PDEP_INTRINSIC)

#if defined(SLOW_PDEP) // in Time.c
    printf("#    SLOW_PDEP\n");
#else // defined(SLOW_PDEP)
    printf("# No SLOW_PDEP\n");
#endif // defined(SLOW_PDEP)

#if defined(EXTRA_SLOW_PDEP) // in Time.c
    printf("#    EXTRA_SLOW_PDEP\n");
#else // defined(EXTRA_SLOW_PDEP)
    printf("# No EXTRA_SLOW_PDEP\n");
#endif // defined(EXTRA_SLOW_PDEP)

#if defined(CALL_GET_NEXT_KEY) // in Time.c
    printf("#    CALL_GET_NEXT_KEY\n");
#else // defined(CALL_GET_NEXT_KEY)
    printf("# No CALL_GET_NEXT_KEY\n");
#endif // defined(CALL_GET_NEXT_KEY)

#if defined(SYNC_SYNC) // in Time.c
    printf("#    SYNC_SYNC\n");
#else // defined(SYNC_SYNC)
    printf("# No SYNC_SYNC\n");
#endif // defined(SYNC_SYNC)

#if defined(KFLAG) // in Time.c
    printf("#    KFLAG\n");
#else // defined(KFLAG)
    printf("# No KFLAG\n");
#endif // defined(KFLAG)

#if defined(NO_USE_XX_SW)
    printf("#    NO_USE_XX_SW\n");
#else // defined(NO_USE_XX_SW)
    printf("# No NO_USE_XX_SW\n");
#endif // defined(NO_USE_XX_SW)

#if defined(NO_XX_SHORTCUT)
    printf("#    NO_XX_SHORTCUT\n");
#else // defined(NO_XX_SHORTCUT)
    printf("# No NO_XX_SHORTCUT\n");
#endif // defined(NO_XX_SHORTCUT)

#if defined(NO_XX_SHORTCUT_GOTO)
    printf("#    NO_XX_SHORTCUT_GOTO\n");
#else // defined(NO_XX_SHORTCUT_GOTO)
    printf("# No NO_XX_SHORTCUT_GOTO\n");
#endif // defined(NO_XX_SHORTCUT_GOTO)

#if defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("#    NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#else // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("# No NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#endif // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)

#if defined(NO_UA_PARALLEL_128)
    printf("#    NO_UA_PARALLEL_128\n");
#else // defined(NO_UA_PARALLEL_128)
    printf("# No NO_UA_PARALLEL_128\n");
#endif // defined(NO_UA_PARALLEL_128)

#if defined(NO_PARALLEL_128)
    printf("#    NO_PARALLEL_128\n");
#else // defined(NO_PARALLEL_128)
    printf("# No NO_PARALLEL_128\n");
#endif // defined(NO_PARALLEL_128)

#if defined(NO_SORT_LISTS)
    printf("#    NO_SORT_LISTS\n");
#else // defined(NO_SORT_LISTS)
    printf("# No NO_SORT_LISTS\n");
#endif // defined(NO_SORT_LISTS)

#if defined(NO_PSPLIT_PARALLEL)
    printf("#    NO_PSPLIT_PARALLEL\n");
#else // defined(NO_PSPLIT_PARALLEL)
    printf("# No NO_PSPLIT_PARALLEL\n");
#endif // defined(NO_PSPLIT_PARALLEL)

#if defined(NO_PSPLIT_EARLY_OUT)
    printf("#    NO_PSPLIT_EARLY_OUT\n");
#else // defined(NO_PSPLIT_EARLY_OUT)
    printf("# No NO_PSPLIT_EARLY_OUT\n");
#endif // defined(NO_PSPLIT_EARLY_OUT)

#if defined(NO_LVL_IN_WR_HB)
    printf("#    NO_LVL_IN_WR_HB\n");
#else // defined(NO_LVL_IN_WR_HB)
    printf("# No NO_LVL_IN_WR_HB\n");
#endif // defined(NO_LVL_IN_WR_HB)

#if defined(NO_SKIP_TO_XX_SW)
    printf("#    NO_SKIP_TO_XX_SW\n");
#else // defined(NO_SKIP_TO_XX_SW)
    printf("# No NO_SKIP_TO_XX_SW\n");
#endif // defined(NO_SKIP_TO_XX_SW)

#if defined(NO_USE_BM_SW)
    printf("#    NO_USE_BM_SW\n");
#else // defined(NO_USE_BM_SW)
    printf("# No NO_USE_BM_SW\n");
#endif // defined(NO_USE_BM_SW)

#if defined(NO_SKIP_TO_BM_SW)
    printf("#    NO_SKIP_TO_BM_SW\n");
#else // defined(NO_SKIP_TO_BM_SW)
    printf("# No NO_SKIP_TO_BM_SW\n");
#endif // defined(NO_SKIP_TO_BM_SW)

#if defined(NO_POP_IN_WR_HB)
    printf("#    NO_POP_IN_WR_HB\n");
#else // defined(NO_POP_IN_WR_HB)
    printf("# No NO_POP_IN_WR_HB\n");
#endif // defined(NO_POP_IN_WR_HB)

#if defined(NO_EMBED_KEYS)
    printf("#    NO_EMBED_KEYS\n");
#else // defined(NO_EMBED_KEYS)
    printf("# No NO_EMBED_KEYS\n");
#endif // defined(NO_EMBED_KEYS)

#if defined(NO_EK_CALC_POP)
    printf("#    NO_EK_CALC_POP\n");
#else // defined(NO_EK_CALC_POP)
    printf("# No NO_EK_CALC_POP\n");
#endif // defined(NO_EK_CALC_POP)

#if defined(NO_PSPLIT_SEARCH_8)
    printf("#    NO_PSPLIT_SEARCH_8\n");
#else // defined(NO_PSPLIT_SEARCH_8)
    printf("# No NO_PSPLIT_SEARCH_8\n");
#endif // defined(NO_PSPLIT_SEARCH_8)

#if defined(NO_PSPLIT_SEARCH_16)
    printf("#    NO_PSPLIT_SEARCH_16\n");
#else // defined(NO_PSPLIT_SEARCH_16)
    printf("# No NO_PSPLIT_SEARCH_16\n");
#endif // defined(NO_PSPLIT_SEARCH_16)

#if defined(NO_PSPLIT_SEARCH_32)
    printf("#    NO_PSPLIT_SEARCH_32\n");
#else // defined(NO_PSPLIT_SEARCH_32)
    printf("# No NO_PSPLIT_SEARCH_32\n");
#endif // defined(NO_PSPLIT_SEARCH_32)

#if defined(NO_BINARY_SEARCH_WORD)
    printf("#    NO_BINARY_SEARCH_WORD\n");
#else // defined(NO_BINARY_SEARCH_WORD)
    printf("# No NO_BINARY_SEARCH_WORD\n");
#endif // defined(NO_BINARY_SEARCH_WORD)

#if defined(NO_SVALUE) // in RandomNumb.h
    printf("#    NO_SVALUE\n");
#else // defined(NO_SVALUE)
    printf("# No NO_SVALUE\n");
#endif // defined(NO_SVALUE)

#if defined(NO_IF_SVALUE) // in RandomNumb.h
    printf("#    NO_IF_SVALUE\n");
#else // defined(NO_IF_SVALUE)
    printf("# No NO_IF_SVALUE\n");
#endif // defined(NO_IF_SVALUE)

#if defined(NO_LFSR) // in RandomNumb.h
    printf("#    NO_LFSR\n");
#else // defined(NO_LFSR)
    printf("# No NO_LFSR\n");
#endif // defined(NO_LFSR)

#if defined(NO_GAUSS) // in RandomNumb.h
    printf("#    NO_GAUSS\n");
#else // defined(NO_GAUSS)
    printf("# No NO_GAUSS\n");
#endif // defined(NO_GAUSS)

#if defined(NO_KFLAG) // in Time.c
    printf("#    NO_KFLAG\n");
#else // defined(NO_KFLAG)
    printf("# No NO_KFLAG\n");
#endif // defined(NO_KFLAG)

#if defined(NO_FVALUE) // in Time.c
    printf("#    NO_FVALUE\n");
#else // defined(NO_FVALUE)
    printf("# No NO_FVALUE\n");
#endif // defined(NO_FVALUE)

#if defined(NO_TRIM_EXPANSE) // in Time.c
    printf("#    NO_TRIM_EXPANSE\n");
#else // defined(NO_TRIM_EXPANSE)
    printf("# No NO_TRIM_EXPANSE\n");
#endif // defined(NO_TRIM_EXPANSE)

#if defined(NO_DFLAG) // in Time.c
    printf("#    NO_DFLAG\n");
#else // defined(NO_DFLAG)
    printf("# No NO_DFLAG\n");
#endif // defined(NO_DFLAG)

#if defined(NO_OFFSET) // in Time.c
    printf("#    NO_OFFSET\n");
#else // defined(NO_OFFSET)
    printf("# No NO_OFFSET\n");
#endif // defined(NO_OFFSET)

#if defined(BM_IN_LINK)
    printf("\n");
    printf("# BM_IN_LINK is buggy.\n");
    printf("# With two-digit bitmap leaf conversion.\n");
    printf("# With bmsw to uncompressed conversion.\n");
    printf("# With retype full bmsw if BM_IN_NON_BM_SW.\n");
    printf("# With skip at top?\n");
    printf("# With PP_IN_LINK unless NO_SKIP_AT_TOP?\n");
#endif // defined(BM_IN_LINK)

    printf("\n");
#ifdef N_LIST_HDR_KEYS
    printf("# N_LIST_HDR_KEYS %d\n", N_LIST_HDR_KEYS);
#endif // N_LIST_HDR_KEYS

    printf("\n");
#if defined(MALLOC_ALIGNMENT)
    printf("# MALLOC_ALIGNMENT %d\n", MALLOC_ALIGNMENT);
#else // defined(MALLOC_ALIGNMENT)
    printf("# No MALLOC_ALIGNMENT\n");
#endif // defined(MALLOC_ALIGNMENT)
    printf("# cnBitsMallocMask %d\n", cnBitsMallocMask);
    printf("# sizeof(Bucket_t) %zd\n", sizeof(Bucket_t));
    printf("\n");
    printf("# cnBitsInD1 %d\n", cnBitsInD1);
    printf("# cnBitsInD2 %d\n", cnBitsInD2);
    printf("# cnBitsInD3 %d\n", cnBitsInD3);
    printf("\n");
    printf("# cnBitsPerWord %d\n", cnBitsPerWord);
    printf("# cnBitsPerDigit %d\n", cnBitsPerDigit);
    printf("# cnDigitsPerWord %d\n", cnDigitsPerWord);
    printf("\n");
    printf("# cnListPopCntMax8  %d\n", cnListPopCntMax8);
    printf("# cnListPopCntMax16 %d\n", cnListPopCntMax16);
    printf("# cnListPopCntMax32 %d\n", cnListPopCntMax32);
#if defined(cnListPopCntMax64)
    printf("# cnListPopCntMax64 %d\n", cnListPopCntMax64);
#endif // defined(cnListPopCntMax64)
    printf("\n");
    printf("# cnListPopCntMaxDl1 %d\n", cnListPopCntMaxDl1);
#if defined(cnListPopCntMaxDl2)
    printf("# cnListPopCntMaxDl2 %d\n", cnListPopCntMaxDl2);
#else // defined(cnListPopCntMaxDl2)
    printf("# cnListPopCntMaxDl2 n/a\n");
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
    printf("# cnListPopCntMaxDl3 %d\n", cnListPopCntMaxDl3);
#else // defined(cnListPopCntMaxDl3)
    printf("# cnListPopCntMaxDl3 n/a\n");
#endif // defined(cnListPopCntMaxDl3)
    printf("\n");
    printf("# cwListPopCntMax %d\n", cwListPopCntMax);
    printf("# cn2dBmMaxWpkPercent %d\n", cn2dBmMaxWpkPercent);
    printf("# cnBmSwConvert %d\n", cnBmSwConvert);
    printf("# cnBmSwRetain %d\n", cnBmSwRetain);

#if defined(CODE_XX_SW)
    printf("\n");
    printf("# cnBW %d\n", cnBW);
    printf("# cnBWIncr %d\n", cnBWIncr);
#endif // defined(CODE_XX_SW)

    printf("\n");
    printf("# cnBinarySearchThresholdWord %d\n", cnBinarySearchThresholdWord);

#if defined(EMBED_KEYS)
    printf("\n");
    int nPopCntMaxPrev = -1;
    for (int nBL = cnBitsPerWord; nBL > 0; --nBL) {
        int nPopCntMax = EmbeddedListPopCntMax(nBL);
        if (nPopCntMax != nPopCntMaxPrev) {
            if (nPopCntMaxPrev != -1) {
                printf("# EmbeddedListPopCntMax(%2d)  %2d\n",
                       nBL+1, nPopCntMaxPrev);
            }
            printf("# EmbeddedListPopCntMax(%2d)  %2d\n", nBL, nPopCntMax);
            nPopCntMaxPrev = nPopCntMax;
        }
    }
#endif // defined(EMBED_KEYS)

    // How big are T_LIST leaves.
    for (int nBL = cnBitsPerWord; nBL >= 8; nBL >>= 1) {
        printf("\n");
        int nWordsPrev = 0, nBoundaries = 0, nWords;
        for (int nPopCnt = 1; nBoundaries <= 3; nPopCnt++) {
            if ((nWords = ListWordsTypeList(nPopCnt, nBL)) != nWordsPrev) {
                ++nBoundaries;
                if (nWordsPrev != 0) {
                    printf("# ListWordsTypeList(nBL %2d, nPopCnt %3d) %3d\n",
                           nBL, nPopCnt - 1,
                           ListWordsTypeList(nPopCnt - 1, nBL));
                }
                printf("# ListWordsTypeList(nBL %2d, nPopCnt %3d) %3d\n",
                       nBL, nPopCnt, nWords);
                nWordsPrev = nWords;
            }
        }
    }

    printf("\n");
    printf("# cnDummiesInList %d\n", cnDummiesInList);
    printf("# cnDummiesInSwitch %d\n", cnDummiesInSwitch);
    printf("# cnDummiesInLink %d\n", cnDummiesInLink);
#if defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords %d\n", cnMallocExtraWords);
#else // defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords n/a\n");
#endif // defined(cnMallocExtraWords)

    printf("\n# cnBitsIndexSzAtTop %d\n", cnBitsIndexSzAtTop);
    for (int dd = 1; dd <= cnDigitsPerWord; dd++) {
        printf("# nDL_to_nBitsIndexSz(%d) %d\n", dd, nDL_to_nBitsIndexSz(dd));
    }

    printf("\n");

    // Print the type values.

    printf("# Link types:\n\n");
#if defined(SEPARATE_T_NULL)
    printf("# 0x%x %-20s\n", T_NULL, "T_NULL");
#endif // defined(SEPARATE_T_NULL)
#if (cwListPopCntMax != 0)
    printf("# 0x%x %-20s\n", T_LIST, "T_LIST");
#if defined(SKIP_TO_LIST)
    printf("# 0x%x %-20s\n", T_SKIP_TO_LIST, "T_SKIP_TO_LIST");
#endif // defined(SKIP_TO_LIST)
#if defined(UA_PARALLEL_128)
    printf("# 0x%x %-20s\n", T_LIST_UA, "T_LIST_UA");
#endif // defined(UA_PARALLEL_128)
#endif // (cwListPopCntMax != 0)
    printf("# 0x%x %-20s\n", T_BITMAP, "T_BITMAP");
#if defined(SKIP_TO_BITMAP)
    printf("# 0x%x %-20s\n", T_SKIP_TO_BITMAP, "T_SKIP_TO_BITMAP");
#endif // defined(SKIP_TO_BITMAP)
#if defined(EMBED_KEYS)
    printf("# 0x%x %-20s\n", T_EMBEDDED_KEYS, "T_EMBEDDED_KEYS");
#endif // defined(EMBED_KEYS)
#if defined(CODE_LIST_SW)
    printf("# 0x%x %-20s\n", T_LIST_SW, "T_LIST_SW");
#endif // defined(CODE_LIST_SW)
#if defined(SKIP_TO_LIST_SW)
    printf("# 0x%x %-20s\n", T_SKIP_TO_LIST_SW, "T_SKIP_TO_LIST_SW");
#endif // defined(SKIP_TO_LIST_SW)
#if defined(CODE_BM_SW)
    printf("# 0x%x %-20s\n", T_BM_SW, "T_BM_SW");
#endif // defined(CODE_BM_SW)
#if defined(SKIP_TO_BM_SW)
    printf("# 0x%x %-20s\n", T_SKIP_TO_BM_SW, "T_SKIP_TO_BM_SW");
#endif // defined(SKIP_TO_BM_SW)
#if defined(CODE_XX_SW)
    printf("# 0x%x %-20s\n", T_XX_SW, "T_XX_SW");
#endif // defined(CODE_XX_SW)
#if defined(SKIP_TO_XX_SW) // doesn't work yet
    printf("# 0x%x %-20s\n", T_SKIP_TO_XX_SW, "T_SKIP_TO_XX_SW");
#endif // defined(SKIP_TO_XX_SW) // doesn't work yet
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    printf("# 0x%x %-20s\n", T_FULL_BM_SW, "T_FULL_BM_SW");
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    printf("# 0x%x %-20s\n", T_SKIP_TO_FULL_BM_SW, "T_SKIP_TO_FULL_BM_SW");
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    printf("# 0x%x %-20s\n", T_SWITCH, "T_SWITCH");
#if defined(SKIP_LINKS)
    printf("# 0x%x %-20s\n", T_SKIP_TO_SWITCH, "T_SKIP_TO_SWITCH");
#endif // defined(SKIP_LINKS)

    printf("\n");
}

// ***************************************************************************
// JUDY1 FUNCTIONS:

Word_t
Judy1FreeArray(PPvoid_t PPArray, PJError_t PJError)
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("Judy1FreeArray\n"));

    // A real user shouldn't pass NULL to Judy1FreeArray.
    // Judy1LHTime uses NULL to give us an opportunity to print
    // configuration info into the log file before we start testing.
    if (PPArray == NULL) { Initialize(); return 0; }

#if (cnDigitsPerWord != 1)

  #if defined(DEBUG)
    Word_t wMallocsBefore = wMallocs; (void)wMallocsBefore;
    Word_t wWordsAllocatedBefore = wWordsAllocated;
      #if defined(RAMMETRICS)
    Word_t j__AllocWordsTOTBefore = j__AllocWordsTOT;
    (void)j__AllocWordsTOTBefore;
    Word_t j__TotalBytesAllocatedBefore = j__TotalBytesAllocated;
    (void)j__TotalBytesAllocatedBefore;
      #endif // defined(RAMMETRICS)
  #endif // defined(DEBUG)

    Word_t wBytes = FreeArrayGuts((Word_t *)PPArray, /* wPrefix */ 0,
                                   cnBitsPerWord, /* bDump */ 0);

    DBGR(printf("# wPopCntTotal %" _fw"u 0x%" _fw"x\n",
               wPopCntTotal, wPopCntTotal));
    DBGR(printf("# Judy1FreeArray wBytes %" _fw"u words %" _fw"u\n",
               wBytes, wBytes/sizeof(Word_t)));
    DBGR(printf("# Judy1FreeArray wBytes 0x%" _fw"x words 0x%" _fw"x\n",
               wBytes, wBytes/sizeof(Word_t)));
    DBGR(printf("# wWordsAllocatedBefore %" _fw"u 0x%" _fw"x\n",
               wWordsAllocatedBefore, wWordsAllocatedBefore));
#if defined(RAMMETRICS)
    DBGR(printf("# j__AllocWordsTOTBefore %" _fw"u 0x%" _fw"x\n",
               j__AllocWordsTOTBefore, j__AllocWordsTOTBefore));
    DBGR(printf("# j__TotalBytesAllocatedBefore %" _fw"u words %" _fw"u\n",
               j__TotalBytesAllocatedBefore,
               j__TotalBytesAllocatedBefore/sizeof(Word_t)));
    DBGR(printf("# j__TotalBytesAllocatedBefore 0x%" _fw"x words 0x%" _fw"x\n",
               j__TotalBytesAllocatedBefore,
               j__TotalBytesAllocatedBefore/sizeof(Word_t)));
    DBGR(printf("# Total MiB Before %" _fw"u rem %" _fw"u\n",
               j__TotalBytesAllocatedBefore / (1024 * 1024),
               j__TotalBytesAllocatedBefore % (1024 * 1024)));
    DBGR(printf("# Total MiB Before 0x%" _fw"x rem 0x%" _fw"x\n",
               j__TotalBytesAllocatedBefore / (1024 * 1024),
               j__TotalBytesAllocatedBefore % (1024 * 1024)));
#endif // defined(RAMMETRICS)
    DBGR(printf("# wMallocsBefore %" _fw"u 0x%" _fw"x\n",
               wMallocsBefore, wMallocsBefore));

    DBGR(printf("After Judy1FreeArray:\n"));
    DBGR(printf("# wWordsAllocated %" _fw"u\n", wWordsAllocated));
#if defined(RAMMETRICS)
    DBGR(printf("# j__AllocWordsTOT %" _fw"u\n", j__AllocWordsTOT));
    DBGR(printf("# j__TotalBytesAllocated 0x%" _fw"x\n",
               j__TotalBytesAllocated));
    DBGR(printf("# Total MiB 0x%" _fw"x rem 0x%" _fw"x\n",
               j__TotalBytesAllocated / (1024 * 1024),
               j__TotalBytesAllocated % (1024 * 1024)));
#endif // defined(RAMMETRICS)
    DBGR(printf("# wMallocs %" _fw"u\n", wMallocs));
    DBGR(printf("\n"));
    assert((wWordsAllocatedBefore - wWordsAllocated)
               == (wBytes / sizeof(Word_t)));
    // Assuming wWordsAllocated is zero is presumptuous.
    // What if the application has more than one Judy1 array?
    assert(wWordsAllocated == 0);
#if defined(RAMMETRICS)
    // Assuming j__AllocWordsTOT is zero is presumptuous.
    // What if the application has more than one Judy1 or JudyL array, e.g.
    // Judy1LHTime with -1L or Judy1LHCheck?
    assert(j__AllocWordsTOT == 0);
    // Dlmalloc doesn't necessarily unmap everything even if we free it.
    //assert(j__TotalBytesAllocated == 0);
#endif // defined(RAMMETRICS)
    // Assuming wMallocs is zero is presumptuous.
    // What if the application has more than one Judy1 array?
    assert(wMallocs == 0);

    // Should have FreeArrayGuts adjust wPopCntTotal this as it goes.
    assert(Judy1Count(*PPArray, 0, (Word_t)-1, NULL) == 0);
    wPopCntTotal = 0; // What if there is more than one Judy1 array?

    return wBytes;

#else // (cnDigitsPerWord != 1)
    JudyFree((RawP_t)*PPArray, EXP(cnBitsPerWord - cnLogBitsPerWord));
    return EXP(cnBitsPerWord - cnLogBitsPerByte);
#endif // (cnDigitsPerWord != 1)
}

// Return the number of keys that are present from wKey0 through wKey1.
// Include wKey0 and wKey1 in the count if they are present.
// Return zero for full pop and identify this case by:
// (wKey0 == 0) && (wKey1 == -1) && (PArray != NULL).
//
// 'typedef const void * Pcvoid_t' aka 'typedef void * const Pcvoid_t'
// Pcvoid_t is a pointer to a constant.
// The value of *PArray cannot be changed.
Word_t
Judy1Count(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, JError_t *pJError)
{
    DBGC(printf("Judy1Count\n"));
    //DBGC(Dump(pwRootLast, 0, cnBitsPerWord));

#if (cnDigitsPerWord != 1)

    // There is really no need for us to disambiguate since the caller
    // can do it just fine without our help.
    // The interesting case is when we return zero for full pop.
    // But the caller can identify this case by:
    // (wKey0 == 0) && (wKey1 == -1) && (PArray != NULL).
    // This is no more onerous than the Judy way of checking
    // pJError->je_Errno.
    // If we supported exceptions there might be an argument for us
    // making the distinction.

    // Return 0 if the array is empty or wKey0 > wKey1.
    // JudyCommon/JudyCount.c defines C_JERR for this case.
    // The 'C_' is an abbreviation for count.
    if ((PArray == (Pvoid_t)NULL) || (wKey0 > wKey1))
    {
        if (pJError != NULL)
        {
            pJError->je_Errno = JU_ERRNO_NONE; // zero pop
            pJError->je_ErrID = __LINE__;
        }

        return 0; // C_JERR
    }

    // Set je_Errno just in case the array is full and we return 0?
    // So we always set je_Errno to something? Unlike Unix which
    // sets errno only in the case of an error?
    if (pJError != NULL)
    {
        pJError->je_Errno = JU_ERRNO_FULL; // full pop
        pJError->je_ErrID = __LINE__;
    }

    Word_t wRoot = (Word_t)PArray;
    // Count returns the number of keys before the specified key.
    // It does not include the specified key.
    Word_t wCount0 = (wKey0 == 0) ? 0 : Count(&wRoot, wKey0, cnBitsPerWord);
    DBGC(printf("Count wKey0 " OWx" Count0 %" _fw"d\n", wKey0, wCount0));
    Word_t wCount1 = Count(&wRoot, wKey1, cnBitsPerWord);
    DBGC(printf("Count wKey1 " OWx" Count1 %" _fw"d\n", wKey1, wCount1));
    Word_t wCount = wCount1 - wCount0;
    wCount += Judy1Test(PArray, wKey1, NULL);

    if ((wKey0 == 0) && (wKey1 == (Word_t)-1))
    {
        unsigned nType = wr_nType(wRoot);
        Word_t *pwr = wr_pwr(wRoot);
        Word_t wPopCnt; (void)wPopCnt;

  #if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        if ( ! tp_bIsSwitch(nType) )
        {
      #if defined(EMBED_KEYS)
            if (nType == T_EMBEDDED_KEYS) {
                wPopCnt = 1; // Always a full word to top; never embedded.
            } else
      #endif // defined(EMBED_KEYS)
            if (pwr == NULL) {
                wPopCnt = 0;
      #if defined(SKIP_TO_BITMAP)
            } else if (nType == T_SKIP_TO_BITMAP) {
                wPopCnt = GetPopCnt(&wRoot, cnBitsPerWord);
      #endif // defined(SKIP_TO_BITMAP)
            } else {
                assert( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
                       || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
                       );
                // ls_wPopCnt is valid at top for PP_IN_LINK.
                wPopCnt = PWR_xListPopCnt(&wRoot, pwr, cnBitsPerWord);
            }
        }
        else // ! tp_bIsSwitch(nType)
  #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        { // tp_bIsSwitch(nType)
  #if defined(PP_IN_LINK)
            wPopCnt = Sum(&wRoot, cnBitsPerWord);
  #else // defined(PP_IN_LINK)
            wPopCnt = GetPopCnt(&wRoot, cnBitsPerWord);
  #endif // defined(PP_IN_LINK)
        }

  #if defined(DEBUG)
        if (wPopCnt != wPopCntTotal)
        {
            printf("\nAssertion error debug:\n");
            printf("\nwPopCnt %" _fw"u wPopCntTotal %" _fw"u\n",
                   wPopCnt, wPopCntTotal);
            if (wPopCntTotal < 0x1000) {
                Dump(pwRootLast, 0, cnBitsPerWord);
            }
        }
        assert(wPopCnt == wPopCntTotal);

        if (wPopCnt != wCount)
        {
            printf("\nAssertion error debug:\n");
            printf("\nwPopCnt %" _fw"u wCount %" _fw"u\n",
                   wPopCnt, wCount);
            if (wPopCntTotal < 0x1000) {
                Dump(pwRootLast, 0, cnBitsPerWord);
            }
        }
        assert(wPopCnt == wCount);
  #endif // defined(DEBUG)
    }

    return wCount;

#else // (cnDigitsPerWord != 1)

    (void)PArray; (void)wKey0; (void)wKey1, (void)PJError;
    return wPopCntTotal;

#endif // (cnDigitsPerWord != 1)
}

// NextGuts(wRoot, nBL, pwKey, wSkip, bPrev, bEmpty)
//
// Find the (wSkip+1)'th present key/index greater than or equal to wKey.
// Return 0 if such a key/index exists.
// Otherwise return the amount of the population deficiency.
//
// If wSkip=0, then find the 1st present key/index equal to or after wKey.
// If wSkip=1, then find the 2nd present key/index equal to or after wKey.
// If wSkip=2, then find the 3rd present key/index equal to or after wKey.
//
// To find all keys:
//    for (wKey = 0; Next(&wKey, /* wSkip */ 0) == 0; wKey++) ;
// A Judy way:
//    for (wKey = 0; JudyFirst(&wKey); wKey++) ;
// Another Judy way:
//    if (wKey = 0, JudyFirst(&wKey)) do ; while (JudyNext(&wKey)) ;
//
// To find every wN'th key starting with the wN'th:
//    for (wKey = 0; Next(&wKey, /* wSkip */ wN-1) == 0; wKey++) ;
// To find every wN'th key starting with the 1st:
//    if (wKey = 0, Next(&wKey, /* wSkip */ 0) == 0)
//        do ; while (wKey++, Next(&wKey, /* wSkip */ wN-1) == 0) ;
static Word_t
NextGuts(Word_t *pwRoot, int nBL,
         Word_t *pwKey, Word_t wSkip, int bPrev, int bEmpty)
{
#define A(_zero) assert(_zero)
    (void)bEmpty;
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);
    DBGN(printf("NextGuts(wRoot " OWx" nBL %d *pwKey " OWx
                    " wSkip %" _fw"d bPrev %d bEmpty %d) pwr %p\n",
                wRoot, nBL, *pwKey, wSkip, bPrev, bEmpty, (void *)pwr));
    int nType = wr_nType(wRoot);
    switch (nType) {
#if defined(UA_PARALLEL_128)
    case T_LIST_UA:
        DBGN(printf("T_LIST_UA\n"));
        goto t_list;
#endif // defined(UA_PARALLEL_128)
    case T_LIST: {
        goto t_list;
t_list:;
        DBGN(printf("T_LIST\n"));
        //A(0);
        if (pwr == NULL) {
            //A(0); // startup
            return wSkip + 1;
        }
        //A(0);
        int nPos = SearchList(pwr, *pwKey, nBL, &wRoot);
        if (bPrev) {
            //A(0);
            if (nPos < 0) {
                //A(0);
                nPos ^= -1;
                if (nPos == 0) { /*A(0);*/ return wSkip + 1; }
                //A(0);
                --nPos;
            }
            //A(0);
            if ((Word_t)nPos < wSkip) {
                A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
                return wSkip - nPos;
            }
            //A(0);
            *pwKey = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
            *pwKey |= ls_pxKey(pwr, nBL, nPos - wSkip);
        } else {
            //A(0);
            if (nPos < 0) { /*A(0);*/ nPos ^= -1; }
            //A(0);
            int nPopCnt = PWR_xListPopCnt(&wRoot, pwr, nBL);
            if (nPos + wSkip >= (Word_t)nPopCnt) {
                //A(0);
                return nPos + wSkip - ((Word_t)nPopCnt - 1);
            }
            //A(0);
            *pwKey = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
            *pwKey |= ls_pxKey(pwr, nBL, nPos + wSkip);
        }
        //A(0);
        return 0;
    }
  #if defined(EMBED_KEYS)
    case T_EMBEDDED_KEYS: {
        DBGN(printf("T_EMBEDDED_KEYS: *pwKey " OWx" wSkip %" _fw"u\n",
                    *pwKey, wSkip));
        //A(0); // check -B10 -DS1
        assert(nBL != cnBitsPerWord);
        int nPos = SearchEmbeddedX(&wRoot, *pwKey, nBL);
        if (bPrev) {
            //A(0); // check -B10 -DS1
            if (nPos < 0) {
                //A(0); // check -B10 -DS1
                nPos ^= -1;
                if (nPos == 0) {
                    //A(0); // check -B10 -DS1
                    return wSkip + 1;
                }
                //A(0); // check -B10 -DS1
                --nPos;
            }
            //A(0); // check -B10 -DS1
            if ((Word_t)nPos < wSkip) {
                A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
                return wSkip - nPos;
            }
            //A(0); // check -B10 -DS1
            *pwKey &= ~MSK(nBL);
            *pwKey |= GetBits(wRoot, nBL,
                              cnBitsPerWord - (nPos - wSkip + 1) * nBL);
        } else {
            //A(0); // check -B10 -DS1
            if (nPos < 0) { /*A(0);*/ nPos ^= -1; }
            //A(0); // check -B10 -DS1
            int nPopCnt = wr_nPopCnt(wRoot, nBL);
            if (nPos + wSkip >= (Word_t)nPopCnt) {
                //A(0); // check -B10 -DS1
                return nPos + wSkip - ((Word_t)nPopCnt - 1);
            }
            //A(0); // check -B10 -DS1
            *pwKey &= ~MSK(nBL);
            *pwKey |= GetBits(wRoot, nBL,
                              cnBitsPerWord - (nPos + wSkip + 1) * nBL);
        }
        //A(0); // check -B10 -DS1
        return 0;
    }
  #endif // defined(EMBED_KEYS)
  #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: {
        DBGN(printf("T_SKIP_TO_BITMAP\n"));
        //A(0);
        nBL = wr_nBL(wRoot);
        Word_t wPrefix
            = w_wPrefixBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL);
        if (wPrefix > (*pwKey & ~MSK(nBL))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                A(0); // UNTESTED - Our test skip links have wPrefix == 0?
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBL))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBL);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
        }
        //A(0);
    }
  #endif // defined(SKIP_TO_BITMAP)
    case T_BITMAP: {
        DBGN(printf("T_BITMAP *pwKey " OWx" wSkip %" _fw"u\n", *pwKey, wSkip));
        assert(nBL != cnBitsPerWord);
        int nWordNum = (*pwKey & MSK(nBL)) >> cnLogBitsPerWord;
        int nBitNum = *pwKey & MSK(cnLogBitsPerWord);
        if (bPrev) {
            //A(0);
            Word_t wBm = pwr[nWordNum];
            if (nBitNum < cnBitsPerWord - 1) {
                //A(0);
                wBm &= MSK(nBitNum + 1);
            }
            //A(0);
            for (;;) {
                //A(0);
                int nPopCnt = __builtin_popcountll(wBm);
                if ((Word_t)nPopCnt > wSkip) { /*A(0);*/ break; }
                //A(0);
                wSkip -= nPopCnt;
                if (--nWordNum < 0) { /*A(0);*/ return wSkip + 1; }
                //A(0);
                wBm = pwr[nWordNum];
            }
            //A(0);
            while (wSkip--) {
               A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
               // ask doug how to find nth set msb
            }
            //A(0);
            nBitNum = LOG(wBm);
        } else {
            //A(0);
            Word_t wBm = pwr[nWordNum] & ~MSK(nBitNum);
            for (;;) {
                //A(0);
                int nPopCnt = __builtin_popcountll(wBm);
                if ((Word_t)nPopCnt > wSkip) { /*A(0);*/ break; }
                //A(0);
                wSkip -= nPopCnt;
                if (++nWordNum >= (int)EXP(nBL - cnLogBitsPerWord)) {
                    //A(0);
                    return wSkip + 1;
                }
                //A(0);
                wBm = pwr[nWordNum];
            }
            //A(0);
            while (wSkip--) {
                //A(0); // startup
                Word_t wLsb = wBm & -wBm; // least bit
                wBm ^= wLsb; // have cleared one bit
            }
            //A(0);
            nBitNum = __builtin_ctzll(wBm);
        }
        //A(0);
        *pwKey &= ~MSK(nBL);
        *pwKey |= (nWordNum << cnLogBitsPerWord) + nBitNum;
        return 0;
    }
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
#if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
        DBG((nType != T_SKIP_TO_SWITCH)
            ? printf("NextGuts: Unhandled nType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
#endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
        DBGN(printf("SKIP_TO_SW\n"));
        //A(0);
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(&wRoot, (Switch_t *)pwr, nBL);
        if (wPrefix > (*pwKey & ~MSK(nBL))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                //A(0); b-32 -1ICvdgi --splay-key-bits=0xff00ffff
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBL))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBL);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
        }
        //A(0);
    }
  #endif // defined(SKIP_LINKS)
    case T_SWITCH: {
        //A(0);
        DBGN(printf("T_SW wSkip %" _fw"u\n", wSkip));
        int nBits = nBL_to_nBitsIndexSz(nBL); // bits decoded by switch
        Word_t wIndex = (*pwKey >> (nBL-nBits)) & MSK(nBits);
        if (bPrev) {
            //A(0);
            for (; wIndex != (Word_t)-1; wIndex--) {
                //A(0);
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x"
                                    " pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    // if we can't be sure we're skipping the whole thing, then
                    // recurse else skip it and move to the next link
                    if (wPopCnt > wSkip) {
                        // prev might be in here
                        //A(0);
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits,
                                               pwKey, wSkip, bPrev, bEmpty))
                            == 0)
                        {
                            //A(0);
                            return 0;
                        }
                        //A(0);
                        wSkip = wCount - 1;
                    } else {
                        // prev is not in here
                        A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
                        assert(*pwKey & MSK(nBL - nBits));
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey |= MSK(nBL - nBits); // set low bits below index
                *pwKey -= EXP(nBL - nBits); // subtract one from index
            }
            //A(0);
            *pwKey += EXP(nBL); // add one to prefix; why ?
            DBGN(printf("T_SW: Failure\n"));
            return wSkip + 1;
        } else {
            //A(0);
            for (; wIndex < EXP(nBits); wIndex++) {
                //A(0);
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x"
                                    " pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if ((wPopCnt > wSkip) /* || *pwKey & MSK(nBL - nBits) */) {
                        //A(0);
                        // next might be in here
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits,
                                               pwKey, wSkip, bPrev, bEmpty))
                            == 0)
                        {
                            //A(0);
                            return 0;
                        }
                        //A(0);
                        wSkip = wCount - 1;
                    } else {
                        // next is not in here
                        //A(0); // startup
                        assert((*pwKey & MSK(nBL - nBits)) == 0);
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey &= ~MSK(nBL - nBits); // clear low bits below index
                *pwKey += EXP(nBL - nBits); // add one to index
            }
            //A(0);
            *pwKey -= EXP(nBL); // subtract one from prefix; why ?
                                // are we just undoing the add from above
                                // and low bits are already zero?
                                // restoring prefix to what it was
                                // originally?
            DBGN(printf("T_SW: Failure *pwKey 0x%016" _fw"x\n", *pwKey));
            return wSkip + 1;
        }
        assert(0); // not expected to get here
    }
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: {
        DBGN(printf("T_SKIP_TO_BM_SW\n"));
        //A(0);
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(&wRoot, (BmSwitch_t *)pwr, nBL);
        if (wPrefix > (*pwKey & ~MSK(nBL))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                A(0); // UNTESTED - Our test skip links have wPrefix == 0?
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBL))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBL);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
        }
        //A(0);
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(CODE_BM_SW)
    case T_BM_SW: {
        //A(0); // check -B17
        DBGN(printf("T_BM_SW wSkip %" _fw"u\n", wSkip));
        int nBits = nBL_to_nBitsIndexSz(nBL); // bits decoded by switch
        Word_t *pwBmWords = PWR_pwBm(&wRoot, pwr);
        int nLinks = 0;
        for (int nn = 0;
             nn < (int)DIV_UP(EXP(nBits), cnBitsPerWord); nn++) {
            nLinks += __builtin_popcountll(pwBmWords[nn]);
        }
        Link_t *pLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t wIndex = (*pwKey >> (nBL-nBits)) & MSK(nBits);
        int nBmWordNum = wIndex >> cnLogBitsPerWord;
        int nBmBitNum = wIndex & (cnBitsPerWord - 1);
        Word_t wBmWord = pwBmWords[nBmWordNum];

        // find starting link
        Word_t wBmSwIndex = 0; // avoid bogus gcc may be uninitialized warning
        int wBmSwBit;
        BmSwIndex(qy, wIndex, &wBmSwIndex, &wBmSwBit);
        DBGN(printf("T_BM_SW wIndex 0x%zx wBmSwIndex %" _fw"u bLnPresent %d\n",
                    wIndex, wBmSwIndex, wBmSwBit));

        if (bPrev) {
            //A(0); // check -B17
            wBmWord &= MSK(nBmBitNum); // mask off high bits
            DBGN(printf("T_BM_SW masked wBmWord 0x%016" _fw"x\n", wBmWord));

            Link_t *pLn = &pLinks[wBmSwIndex];
            DBGN(printf("T_BM_SW pLn %p\n", (void *)pLn));
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                goto BmSwGetPrevIndex;
            }
            Word_t wPopCnt; // declare outside while for goto above
            //A(0); // check -B17
            while (pLn >= pLinks) {
                //A(0); // check -B17
                wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                assert(wPopCnt != 0);
                if (wPopCnt != 0) {
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wIndex 0x%" _fw"x pLn->ln_wRoot "
                                OWx"\n", wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_BM_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        // prev might be in here
                        //A(0); // check -B17
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits,
                                               pwKey, wSkip, bPrev, bEmpty))
                            == 0)
                        {
                            //A(0); // check -B17
                            return 0;
                        }
                        //A(0); // check -B17
                        DBGN(printf("not found *pwKey " OWx"\n", *pwKey));
                        wSkip = wCount - 1;
                    } else {
                        // prev is not in here
                        A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
                        assert(*pwKey & MSK(nBL - nBits));
                        wSkip -= wPopCnt;
                    }
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0); // check -B17
                wBmWord &= ~EXP(nBmBitNum); // clear current wIndex bit
BmSwGetPrevIndex:
                --pLn;
                DBGN(printf("T_BM_SW pLn %p\n", (void *)pLn));
                DBGN(printf("T_BM_SW pLinks %p\n", (void *)pLinks));
                // Find prev set bit in bitmap.
                DBGN(printf("T_BM_SW wBmWord 0x%016" _fw"x\n", wBmWord));
                if (wBmWord != 0) {
                    assert(pLn >= pLinks);
                    //A(0); // check -B17
                    // My LOG works for 64-bit and 32-bit Linux and Windows.
                    // No variant of __builtin_clz[l][l] does.
                    nBmBitNum = LOG(wBmWord);
                    DBGN(printf("T_BM_SW prev link nBmBitNum %d\n",
                                nBmBitNum));
                } else {
                    //A(0); // check -B17
                    nBmBitNum = 0;
                    while (--nBmWordNum >= 0) {
                        //A(0); // check -B17
                        if ((wBmWord = pwBmWords[nBmWordNum]) != 0) {
                            //A(0); // check -B17
                            assert(pLn >= pLinks);
                            nBmBitNum = LOG(wBmWord);
                            DBGN(printf("T_BM_SW prev link nBmWordNum %d"
                                            " nBmBitNum %d\n",
                                        nBmWordNum, nBmBitNum));
                            break;
                        }
                        //A(0); // check -B17 -S1
                    }
                    //A(0); // check -B17
// ???
                    DBGN(printf("bPrev T_BM_SW no more links nBmWordNum %d"
                                    " wIndex " OWx" *pwKey " OWx"\n",
                                nBmWordNum, wIndex, *pwKey));
if ((nBmWordNum == 0) && (wIndex == 0xff)) {
                //Dump(pwRootLast, 0, cnBitsPerWord);
}
                    // restore *pwKey
                    if (nBmWordNum == -1) {
                        if (*pwKey & ~MSK(nBL))
                        //if (wIndex == 0)
                        {
                            //*pwKey += EXP(nBL); // ? necessary ? not sure
                            DBGN(printf("T_BM_SW: Failure *pwKey 0x%016" _fw"x"
                                            " ret wSkip+1 %zd\n",
                                        *pwKey, wSkip+1));
                            return wSkip + 1;
                        }
                        wIndex = (nBmWordNum << cnLogBitsPerWord) + nBmBitNum;
                        //wIndex &= MSK(nBL);
                        DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                        *pwKey &= ~MSK(nBL);
                        *pwKey |= MSK(nBL - nBits);
                        *pwKey |= wIndex << (nBL - nBits);
                        DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
                        continue;
                    }
                }
                //A(0); // check -B17
                wIndex = (nBmWordNum << cnLogBitsPerWord) + nBmBitNum;
                //wIndex &= MSK(nBL);
                DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                if (nBL == cnBitsPerWord) {
                    *pwKey = 0;
                } else {
                    *pwKey &= ~MSK(nBL);
                }
                *pwKey |= MSK(nBL - nBits);
                *pwKey |= wIndex << (nBL - nBits);
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
            }
            //A(0); // check -B17
            *pwKey += EXP(nBL); // ? does this matter ? guess it does
            DBGN(printf("T_BM_SW: Failure *pwKey 0x%016" _fw"x\n", *pwKey));
            return wSkip + 1;
        } else {
            //A(0); // check -B17
            if (*pwKey == 0x1f986) {
                //Dump(pwRootLast, 0, cnBitsPerWord);
            }
            wBmWord &= ~MSK(nBmBitNum); // mask off low bits
            DBGN(printf("T_BM_SW masked wBmWord 0x%016" _fw"x\n", wBmWord));
            Link_t *pLn = &pLinks[wBmSwIndex];
            DBGN(printf("T_BM_SW pLn %p\n", (void *)pLn));
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                goto BmSwGetNextIndex;
            }
            Word_t wPopCnt; // declare outside while for goto above
            //A(0); // check -B17
            while (pLn < &pLinks[nLinks]) {
                //A(0); // check -B17
                wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                assert(wPopCnt != 0);
                if (wPopCnt != 0) {
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wIndex 0x%" _fw"x wBmSwIndex 0x%"
                                _fw"x pLn->ln_wRoot " OWx"\n",
                                wIndex, wBmSwIndex, pLn->ln_wRoot));
                    DBGN(printf("T_BM_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if ((wPopCnt > wSkip) /* || (*pwKey & MSK(nBL - nBits)) */ ) {
                        //A(0); // check -B17
                        DBGN(printf("T_BM_SW: wSkip %" _fw"d *pwKey " OWx"\n",
                                    wSkip, *pwKey));
                        // next might be in here
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits,
                                               pwKey, wSkip, bPrev, bEmpty))
                            == 0)
                        {
                            //A(0); // check -B17
                            DBGN(printf("T_BM_SW NextGuts returned 0\n"));
                            return 0;
                        }
                        //A(0); // check
                        wSkip = wCount - 1;
                    } else {
                        // next is not in here
                        A(0); // startup
                        assert(*pwKey & MSK(nBL - nBits));
                        wSkip -= wPopCnt;
                    }
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0); // check -B17
                wBmWord &= ~EXP(nBmBitNum); // clear current wIndex bit
                ++pLn;
                DBGN(printf("T_BM_SW pLn %p\n", (void *)pLn));
BmSwGetNextIndex:
                // Find next set bit in bitmap.
                DBGN(printf("T_BM_SW wBmWord 0x%016" _fw"x\n", wBmWord));
                if (wBmWord != 0) {
                    //A(0); // check -B17
                    nBmBitNum = __builtin_ctzll(wBmWord);
                    DBGN(printf("T_BM_SW next link nBmWordNum %d nBmBitNum %d\n", nBmWordNum, nBmBitNum));
                } else {
                    //A(0); // check -B17
                    nBmBitNum = 0;
                    while (++nBmWordNum < (int)EXP(nBits - cnLogBitsPerWord)) {
                        //A(0); // check -B17
                        if ((wBmWord = pwBmWords[nBmWordNum]) != 0) {
                            //A(0); // check -B17
                            nBmBitNum = __builtin_ctzll(wBmWord);
                            DBGN(printf("T_BM_SW next link nBmWordNum %d nBmBitNum %d\n", nBmWordNum, nBmBitNum));
                            break;
                        }
                        //A(0); // check -B33 -S1
                    }
                    //A(0); // check -B17
                    DBGN(printf("bNext T_BM_SW no more links nBmWordNum %d wIndex " OWx" *pwKey " OWx"\n", nBmWordNum, wIndex, *pwKey));
                    if (nBmWordNum == (int)EXP(nBits - cnLogBitsPerWord)) {
                        if (*pwKey & ~MSK(nBL)) {
                            return wSkip + 1;
                        }
                    }
                }
                //A(0); // check -B17
                wIndex = (nBmWordNum << cnLogBitsPerWord) + nBmBitNum;
                DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                if (nBL == cnBitsPerWord) {
                    *pwKey = 0;
                } else {
                    *pwKey &= ~MSK(nBL); // clear low bits
                }
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
                *pwKey |= wIndex << (nBL - nBits); // set index bits
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
            }
            //A(0);
            // What is wKey supposed to contain on return if/when count > 0?
            // Undefined? The last key found? Unchanged from entry?
            // Look at the recursive call.
            *pwKey -= EXP(nBL);
            DBGN(printf("T_BM_SW: Failure\n"));
            return wSkip + 1;
        }
        assert(0); // not expected to get here
    }
  #endif // defined(CODE_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: {
        DBGN(printf("T_SKIP_TO_XX_SW\n"));
        //A(0);
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(&wRoot, (Switch_t *)pwr, nBL);
        if (wPrefix > (*pwKey & ~MSK(nBL))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                A(0); // UNTESTED - Our test skip links have wPrefix == 0?
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBL))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBL);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
        }
        //A(0);
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        //A(0);
        DBGN(printf("T_SW wSkip %" _fw"u\n", wSkip));
        int nBits = pwr_nBW(&wRoot); // bits decoded by switch
        Word_t wIndex = (*pwKey >> (nBL-nBits)) & MSK(nBits);
        if (bPrev) {
            //A(0);
            for (; wIndex != (Word_t)-1; wIndex--) {
                //A(0);
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex " OWx" pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        //A(0);
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits, pwKey,
                                              wSkip, bPrev, bEmpty)) == 0) {
                            //A(0);
                            return 0;
                        }
                        //A(0);
                        wSkip = wCount - 1;
                    } else {
                        A(0); // UNTESTED - No tests do wSkip > 0 with bPrev.
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey |= MSK(nBL - nBits);
                *pwKey -= EXP(nBL - nBits);
            }
            //A(0);
            *pwKey += EXP(nBL);
            DBGN(printf("T_SW: Failure\n"));
            return wSkip + 1;
        } else {
            //A(0);
            for (; wIndex < EXP(nBits); wIndex++) {
                //A(0);
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        //A(0);
                        Word_t wCount;
                        if ((wCount = NextGuts(&pLn->ln_wRoot, nBL - nBits, pwKey,
                                              wSkip, bPrev, bEmpty)) == 0) {
                            //A(0);
                            return 0;
                        }
                        //A(0);
                        wSkip = wCount - 1;
                    } else {
                        //A(0); // startup
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey &= ~MSK(nBL - nBits);
                *pwKey += EXP(nBL - nBits);
            }
            //A(0);
            *pwKey -= EXP(nBL);
            DBGN(printf("T_SW: Failure\n"));
            return wSkip + 1;
        }
        assert(0); // not expected to get here
    }
  #endif // defined(USE_XX_SW)
    assert(0); // not expected to get here
    }
    assert(0); // not expected to get here
    return -1; // address compiler complaint
}

// Return the wCount'th key in the array.
// Return 1 if a key is found.
// Return 0 if wCount is bigger than the population of the array.
// Return -1 if pwKey is NULL.
int
Judy1ByCount(Pcvoid_t PArray, Word_t wCount, Word_t *pwKey, PJError_t PJError)
{
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("J1BC: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("J1BC: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    *pwKey = 0;
    DBGN(printf("J1BC: wCount %" _fw"d *pwKey " OWx"\n", wCount, *pwKey));
    Word_t wKey = *pwKey;
    // The Judy1 man page specifies that wCount == 0 is reserved for
    // specifying the last key in a fully populated array.
    --wCount; // Judy API spec is off-by-one IMHO
    wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                      wCount, /* bPrev */ 0, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("J1BC: *pwKey " OWx"\n", *pwKey));
    }
    return wCount == 0;
}

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1First(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("J1F: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("J1F: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("J1F: *pwKey " OWx"\n", *pwKey));
    Word_t wKey = *pwKey;
    Word_t wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                             /* wCount */ 0, /* bPrev */ 0, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("J1F: *pwKey " OWx"\n", *pwKey));
    }
    DBGN(printf("J1F: returning %d\n", wCount == 0));
    return wCount == 0;
}

// Find the next bigger key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1Next(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    Word_t wKeyLocal, *pwKeyLocal;
    if (pwKey != NULL) {
        wKeyLocal = *pwKey + 1;
        if (wKeyLocal == 0) {
            return 0; // What about PJError?
        }
        pwKeyLocal = &wKeyLocal;
    } else {
        pwKeyLocal = NULL;
    }
    int ret = Judy1First(PArray, pwKeyLocal, PJError);
    if (ret == 1) {
        *pwKey = wKeyLocal;
    }
    return ret;
}

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1Last(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("J1L: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("J1L: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("J1L: *pwKey " OWx"\n", *pwKey));
    Word_t wKey = *pwKey;
    Word_t wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                             /* wCount */ 0, /* bPrev */ 1, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("J1L done: *pwKey " OWx"\n", *pwKey));
    }
    DBGN(printf("J1L: returning %d\n", wCount == 0));
    return wCount == 0;
}

// Find the next smaller key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1Prev(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    Word_t wKeyLocal, *pwKeyLocal;
    if (pwKey != NULL) {
        if (*pwKey == 0) {
            return 0; // What about PJError?
        }
        wKeyLocal = *pwKey - 1;
        pwKeyLocal = &wKeyLocal;
    } else {
        pwKeyLocal = NULL;
    }
    int ret = Judy1Last(PArray, pwKeyLocal, PJError);
    if (ret == 1) {
        *pwKey = wKeyLocal;
    }
    return ret;
}

// If *pwKey is not in the array then return Success and leave *pwKey unchanged.
// Otherwise find the next bigger (or smaller if bPrev) key than *pwKey
// which is not in the array.
// Put the resulting key in *pwKey on return.
// Return Success if a key is found.
// Return Failure if no key is found.
// *pwKey is undefined if Failure is returned.
static Status_t
NextEmptyGuts(Word_t *pwRoot, Word_t *pwKey, int nBL, int bPrev)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);
    DBGN(printf("NextEmptyGuts(pwRoot %p *pwKey %p nBL %d bPrev %d)"
                    " wRoot %p pwr %p\n",
                (void *)pwRoot, (void *)*pwKey, nBL, bPrev,
                (void *)wRoot, (void *)pwr));
    int nIncr;
    switch (wr_nType(wRoot)) {
#if defined(UA_PARALLEL_128)
    case T_LIST_UA:
        goto t_list;
#endif // defined(UA_PARALLEL_128)
    case T_LIST:; {
        goto t_list;
t_list:;
        int nPos;
        if ((pwr == NULL)
                || ((nPos = SearchList(pwr, *pwKey, nBL, pwRoot)) < 0)) {
            return Success;
        }
        Word_t wKeyLoop = *pwKey;
        wKeyLoop = (nBL == cnBitsPerWord) ? wKeyLoop : (wKeyLoop & MSK(nBL));
        if (bPrev) {
            if (nPos == 0) {
                if (wKeyLoop == 0) { // suffix
                    return Failure;
                }
                --(*pwKey);
                return Success;
            }
        }
        // save prefix for return
        *pwKey = (nBL == cnBitsPerWord) ? 0 : (*pwKey & ~MSK(nBL));
        int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
        nIncr = bPrev ? -1 : 1;
        for (;;)
        {
            wKeyLoop += nIncr;
            nPos += nIncr;
            if (bPrev) {
                if (nPos < 0) {
                    if (wKeyLoop != (Word_t)-1) { break; }
                    return Failure;
                }
            } else {
                if (nPos >= nPopCnt) {
                    if (wKeyLoop != ((Word_t)1 << (nBL - 1)) * 2) { break; }
                    return Failure;
                }
            }
            Word_t wKeyList = ls_pxKey(pwr, nBL, nPos);
            if (nBL < cnBitsPerWord) { wKeyList &= MSK(nBL); }
            if (wKeyList == wKeyLoop) { continue; }
            *pwKey |= wKeyLoop;
            return Success;
        }
        *pwKey |= wKeyLoop;
        return Success;
    }
  #if defined(EMBED_KEYS)
    case T_EMBEDDED_KEYS:; {
        Word_t wRootNew = InflateEmbeddedList(NULL, *pwKey, nBL, wRoot);
        Word_t wKeyLast = *pwKey & ~MSK(nBL); // prefix
        if (bPrev) {
            nIncr = -1;
        } else {
            wKeyLast |= MSK(nBL);
            nIncr = 1;
        }
        while (SearchList(wr_pwr(wRootNew), *pwKey, nBL, &wRootNew) >= 0) {
            if (*pwKey == wKeyLast) {
                OldList(wr_pwr(wRootNew), wr_nPopCnt(wRoot, nBL), nBL, wr_nType(wRootNew));
                return Failure;
            }
            *pwKey += nIncr;
        }
        OldList(wr_pwr(wRootNew), wr_nPopCnt(wRoot, nBL), nBL, wr_nType(wRootNew));
        return Success;
    }
  #endif // defined(EMBED_KEYS)
  #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: {
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = w_wPrefixBL(*(pwr + EXP(nBL - cnLogBitsPerWord)), nBL);
        if (wPrefix != (*pwKey & ~MSK(nBL))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
    }
  #endif // defined(SKIP_TO_BITMAP)
    case T_BITMAP:; {
        int nWordNum = (*pwKey & MSK(nBL)) >> cnLogBitsPerWord;
        int nBitNum = *pwKey & MSK(cnLogBitsPerWord);
        if (bPrev) {
            Word_t wBm = ~pwr[nWordNum];
            if (nBitNum < cnBitsPerWord - 1) {
                wBm &= MSK(nBitNum + 1);
            }
            for (;;) {
                if (wBm != 0) {
                    nBitNum = LOG(wBm);
                    *pwKey = (*pwKey & ~MSK(nBL))
                           | (nWordNum << cnLogBitsPerWord) | nBitNum;
                    return Success;
                }
                if (nWordNum-- <= 0) {
                    return Failure;
                }
                wBm = ~pwr[nWordNum];
            }
        } else {
            // invert bits so empty looks full then keep high bits
            Word_t wBm = ~pwr[nWordNum] & ~MSK(nBitNum);
            for (;;) {
                if (wBm != 0) {
                    nBitNum = __builtin_ctzll(wBm);
                    *pwKey = (*pwKey & ~MSK(nBL))
                           | (nWordNum << cnLogBitsPerWord) | nBitNum;
                    return Success;
                }
                if (++nWordNum >= (int)EXP(nBL - cnLogBitsPerWord)) {
                    return Failure;
                }
                wBm = ~pwr[nWordNum];
            }
        }
    }
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_SW)
        int nType = wr_nType(wRoot); (void)nType;
        DBG((nType != T_SKIP_TO_SWITCH)
            ? printf("NextGuts: Unhandled nType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_SW)
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL);
        if (wPrefix != (*pwKey & ~MSK(nBL))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
    }
  #endif // defined(SKIP_LINKS)
    case T_SWITCH: {
        int nBits = nBL_to_nBitsIndexSz(nBL); // bits decoded by switch
        Word_t wPrefix = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
        Word_t wIndex = (*pwKey >> (nBL - nBits)) & MSK(nBits);
        for (;;) {
            Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            if (NextEmptyGuts(&pLn->ln_wRoot, pwKey, nBL - nBits, bPrev)
                    == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) { return Failure; }
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                if (++wIndex >= EXP(nBits)) { return Failure; }
                *pwKey = 0; // suffix
            }
            *pwKey |= wPrefix + (wIndex << (nBL - nBits));
        }
    }
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: {
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBL);
        if (wPrefix != (*pwKey & ~MSK(nBL))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(USE_BM_SW)
    case T_BM_SW: {
        //A(0); // check -B17
        //Word_t wKey = *pwKey;
        int nBits = nBL_to_nBitsIndexSz(nBL); // bits decoded by switch
        Link_t *pLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t wPrefix = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
        Word_t wIndex = (*pwKey >> (nBL - nBits)) & MSK(nBits);
        DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
        for (;;) {
            //A(0); // check -B17
            Word_t wBmSwIndex;
            int wBmSwBit;
            BmSwIndex(qy, wIndex, &wBmSwIndex, &wBmSwBit);
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                return Success;
            }
            //A(0); // check -B17
            // link is present
            Link_t *pLn = &pLinks[wBmSwIndex];
            if (NextEmptyGuts(&pLn->ln_wRoot, pwKey, nBL - nBits, bPrev)
                    == Success) {
                //A(0); // check -B17
                return Success;
            }
            //A(0); // check -B25
            if (bPrev) {
                //A(0); // check -B17 -S1
                if (wIndex-- <= 0) {
                    //A(0); // check -B17 -S1
                    //printf("wKey %zx\n", wKey);
                    //*pwKey = wKey;
                    return Failure;
                }
                //A(0); // check -B17 -S1
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                //A(0); // check -B17 -S1
                if (++wIndex >= EXP(nBits)) {
                    //A(0); // check -B17 (with SKIP_TO_BM_SW)
                    //*pwKey = wKey;
                    return Failure;
                }
                //A(0); // check -B17 -S1
                *pwKey = 0; // suffix
            }
            //A(0); // check -B17 -S1
            *pwKey |= wPrefix + (wIndex << (nBL - nBits));
        }
        A(0);
    }
  #endif // defined(USE_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: {
        nBL = wr_nBL(wRoot);
        Word_t wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL);
        if (wPrefix != (*pwKey & ~MSK(nBL))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBL))));
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        int nBits = pwr_nBW(&wRoot); // bits decoded by switch
        Word_t wPrefix = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
        Word_t wIndex = (*pwKey >> (nBL - nBits)) & MSK(nBits);
        for (;;) {
            Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            if (NextEmptyGuts(&pLn->ln_wRoot, pwKey, nBL - nBits, bPrev)
                    == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) { return Failure; }
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                if (++wIndex >= EXP(nBits)) { return Failure; }
                *pwKey = 0; // suffix
            }
            *pwKey |= wPrefix + (wIndex << (nBL - nBits));
        }
    }
  #endif // defined(USE_XX_SW)
    }
    assert(0); // not handled yet
    exit(1);
}

// If *pwKey is not in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger key than *pwKey which is not in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1FirstEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    DBGN(printf("J1FE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("J1FE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("J1FE: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("J1FE: *pwKey " OWx"\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    Status_t status = NextEmptyGuts((Word_t *)&PArray,
                                    &wKeyLocal, cnBitsPerWord, /* bPrev */ 0);
    if (status == Success) {
        *pwKey = wKeyLocal;
        DBGN(printf("J1FE: *pwKey " OWx"\n", *pwKey));
    }
    return status == Success;
}

// Find the next bigger key than *pwKey which is not in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1NextEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    DBGN(printf("J1NE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        return Judy1FirstEmpty(PArray, pwKey, PJError);
    }
    DBGN(printf("J1NE: *pwKey %zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    int ret = 0;
    if (++wKeyLocal != 0) {
        if ((ret = Judy1FirstEmpty(PArray, &wKeyLocal, PJError)) == 1) {
            *pwKey = wKeyLocal;
        }
    }
    DBGN(printf("J1NE: ret %d\n", ret));
    return ret;
}

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller key than *pwKey which is not in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
int
Judy1LastEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    DBGN(printf("J1LE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("J1LE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("J1LE: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("J1LE: *pwKey " OWx"\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    Status_t status = NextEmptyGuts((Word_t *)&PArray,
                                     &wKeyLocal, cnBitsPerWord, /* bPrev */ 1);
    if (status == Success) {
        *pwKey = wKeyLocal;
        DBGN(printf("J1LE: *pwKey " OWx"\n", *pwKey));
    }
    return status == Success;
}

// Find the next smaller key than *pwKey which is not in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error so we
// compare with JudyA.
int
Judy1PrevEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
{
    DBGN(printf("J1PE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        return Judy1LastEmpty(PArray, pwKey, PJError);
    }
    DBGN(printf("J1PE: *pwKey %zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    int ret = 0;
    if (wKeyLocal-- != 0) {
        if ((ret = Judy1LastEmpty(PArray, &wKeyLocal, PJError)) == 1) {
            *pwKey = wKeyLocal;
        }
    }
    DBGN(printf("J1PE: ret %d\n", ret));
    return ret;
}

Word_t
Judy1MemUsed(Pcvoid_t PArray)
{
    (void)PArray;
    return 0;
}

Word_t
Judy1MemActive(Pcvoid_t PArray)
{
    (void)PArray;
    return 0;
}

