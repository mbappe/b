#include "b.h"

#ifdef B_JUDYL
#define Insert  InsertL
#define Count  CountL
#else // B_JUDYL
#define Insert  Insert1
#define Count  Count1
#endif // B_JUDYL

// Check and/or Time depend on Judy1MallocSizes but this version
// of Judy does not use it.
#ifdef B_JUDYL
const char *JudyLMallocSizes = "JudyL"
#else // B_JUDYL
const char *Judy1MallocSizes = "Judy1"
#endif // B_JUDYL
    "MallocSizes = 3, 5, 7, ...";

#define nBytesKeySz(_nBL) \
     (((_nBL) <=  8) ? 1 : ((_nBL) <= 16) ? 2 \
    : ((_nBL) <= 32) ? 4 : sizeof(Word_t))

Word_t wPopCntTotal;

#ifdef B_JUDYL
// bPopCntTotalIsInvalid is overloaded. We use cbPopCntTotalIsInvalid to
// disable cleanup assertions that assume there is only one array under test.
// The assumption is invalid for JudyL Check testing without NO_TEST_HS and
// with Check testing of both JudyL and Judy1.
// We also use NO_ROOT_WORD_CHECK for Check HS testing.
#ifndef cbPopCntTotalIsInvalid
  #define cbPopCntTotalIsInvalid  0
#endif // cbPopCntTotalIsInvalid
int bPopCntTotalIsInvalid = cbPopCntTotalIsInvalid;
#endif // B_JUDYL

#if defined(DEBUG)
Word_t *pwRootLast; // allow dumping of tree when root is not known
#endif // defined(DEBUG)

#if defined(BPD_TABLE)

// Unfortunately, it is not sufficient to change these two tables to
// effect arbitrary switch sizes.
// It is necessary to tweak cnDigitsPerWord in b.h also.

#if defined(BPD_TABLE_RUNTIME_INIT)

unsigned anDL_to_nBW[ cnBitsPerWord + 1 ];
unsigned anDL_to_nBL[ cnBitsPerWord + 1 ];
unsigned anBL_to_nDL[ cnBitsPerWord * 2 ];

#else // defined(BPD_TABLE_RUNTIME_INIT)

const unsigned anDL_to_nBW[] = {
#define V(_nDL) (nBW_from_nDL(_nDL))
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

#ifdef B_JUDYL
#define wMallocs  wMallocsL
#define wWordsAllocated  wWordsAllocatedL
#else // B_JUDYL
#define wMallocs  wMallocs1
#define wWordsAllocated  wWordsAllocated1
#endif // B_JUDYL

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

static void
Log(qp, const char *str)
{
    printf("# %20s: " qfmt "\n", str, qy);
    qv; // assertions after printf
}

#ifdef RAMMETRICS
static Word_t
AllocWords(Word_t *pw, int nWords)
{
    (void)pw; (void)nWords;
  #if !defined(LIBCMALLOC) || defined(__linux__)
    return (pw[-1] & 0xfffff8) / sizeof(Word_t); // dlmalloc head word
  #else // !defined(LIBCMALLOC) || defined(__linux__)
    return nWords;
  #endif //#else !defined(LIBCMALLOC) || defined(__linux__)
}
#endif // RAMMETRICS

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
MyMallocGutsRM(Word_t wWords, int nLogAlignment, Word_t *pwAllocWords)
{
    (void)pwAllocWords; // RAMMETRICS
    Word_t ww, wOff;

    if (nLogAlignment > cnBitsMallocMask) {
        wWords += 1 << (nLogAlignment - cnLogBytesPerWord);
        ww = JudyMalloc(wWords + cnMallocExtraWords);
        wOff = ALIGN_UP(ww + 1, /* power of 2 */ 1 << nLogAlignment) - ww;
        ((Word_t*)(ww + wOff))[-1] = wOff;
    } else {
        ww = JudyMalloc(wWords + cnMallocExtraWords);
        wOff = 0;
    }
    DBGM(printf("\nM(%zd): %p *%p 0x%zx\n",
                wWords, (void *)ww, (void *)&((Word_t *)ww)[-1],
                ((Word_t *)ww)[-1]));

#ifdef RAMMETRICS
    if (pwAllocWords != NULL) {
        (*pwAllocWords)
            += AllocWords((Word_t*)ww, wWords + cnMallocExtraWords);
    }
#endif // RAMMETRICS

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
  #define cnExtraUnitsMax 1
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
  // LIST_POP_IN_PREAMBLE means something different for B_JUDYL so the
  // LIBCMALLOC limitation does not apply. It means use the word before pwr.
  #if defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
    #error LIST_POP_IN_PREAMBLE with LIBCMALLOC is not supported
  #endif // defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
#else // defined(LIBCMALLOC)

#define cnBitsUsed 2 // low bits used by malloc for bookkeeping
    assert((((Word_t *)ww)[-1] & MSK(cnBitsMallocMask)) == MSK(cnBitsUsed));
    // Save the bits of ww[-1] that we need at free time and make sure
    // none of the bits we want to use are changed by malloc while we
    // own the buffer.
    ((Word_t *)ww)[-1] &= ~(MSK(cnExtraUnitsBits) << cnBitsUsed);
    ((Word_t *)ww)[-1] |= zExtraUnits << cnBitsUsed;
#if defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
    // Zero the high bits for our use.
    ((Word_t *)ww)[-1] &= MSK(cnExtraUnitsBits + cnBitsUsed);
#else // defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
    // Twiddle the bits to illustrate that we can use them.
    ((Word_t *)ww)[-1] ^= (Word_t)-1 << (cnExtraUnitsBits + cnBitsUsed);
#endif // #else defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
    DBGM(printf("ww[-1] 0x%zx\n", ((Word_t *)ww)[-1]));
#endif // defined(LIBCMALLOC)
    DBGM(printf("required %zd alloc %zd extra %zd\n",
                zUnitsRequired, zUnitsAllocated, zExtraUnits));
    assert(ww != 0);
    assert((ww & 0xffff000000000000UL) == 0);
    assert((ww & cnMallocMask) == 0);
    ++wMallocs; wWordsAllocated += wWords;
    // ? should we keep track of sub-optimal-size requests ?
    ww += wOff; // number of bytes
    return ww;
}

static Word_t
MyMallocRM(Word_t wWords, Word_t* pwAllocWords)
{
    return MyMallocGutsRM(wWords, /*LogAlign*/ cnBitsMallocMask, pwAllocWords);
}

static void
MyFreeGutsRM(Word_t *pw, Word_t wWords, int nLogAlignment,
             Word_t* pwAllocWords)
{
    (void)pwAllocWords; // RAMMETRICS

    if (nLogAlignment > cnBitsMallocMask) {
        Word_t wOff = pw[-1]; // number of bytes
        wWords += 1 << (nLogAlignment - cnLogBytesPerWord);
        pw = (Word_t*)((Word_t)pw - wOff);
    }
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
    assert(pw[-1] & 2); // lock down what we think we know
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

#ifdef RAMMETRICS
    if (pwAllocWords != NULL) {
        (*pwAllocWords) -= AllocWords(pw, wWords + cnMallocExtraWords);
    }
#endif // RAMMETRICS

    JudyFree((RawP_t)pw, wWords + cnMallocExtraWords);
}

static void
MyFreeRM(Word_t *pw, Word_t wWords, Word_t *pwAllocWords)
{
    MyFreeGutsRM(pw, wWords, /*nLogAlignment*/ cnBitsMallocMask, pwAllocWords);
}

#ifdef RAMMETRICS
  #define MyMallocGuts(a, b, c)  MyMallocGutsRM(a, b, c)
  #define MyMalloc(a, b)  MyMallocRM(a, b)
  #define MyFreeGuts(a, b, c, d)  MyFreeGutsRM(a, b, c, d)
  #define MyFree(a, b, c)  MyFreeRM(a, b, c)
#else // RAMMETRICS
  #define MyMallocGuts(a, b, c)  MyMallocGutsRM(a, b, NULL)
  #define MyMalloc(a, b)  MyMallocRM(a, NULL)
  #define MyFreeGuts(a, b, c, d)  MyFreeGutsRM(a, b, c, NULL)
  #define MyFree(a, b, c)  MyFreeRM(a, b, NULL)
#endif // RAMMETRICS

#if (cwListPopCntMax != 0)

#if (cwListPopCntMax < 256)
typedef uint8_t uListPopCntMax_t;
#else // (cwListPopCntMax < 256)
typedef uint16_t uListPopCntMax_t;
#endif // #else (cwListPopCntMax < 256)

// Max list length as a function of nBL.
// Array is indexed by nBL.
static uListPopCntMax_t auListPopCntMax[] = {
    0,
    //  0 < nBL <=  8
    cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 ,
    cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 ,
    //  8 < nBL <= 16
    cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16,
    cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16,
    // 16 < nBL <= 24
    cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24,
    cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24,
    // 24 < nBL <= 32
    cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32,
    cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32,
  #if (cnBitsPerWord >= 64)
    // 32 < nBL <= 40
    cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40,
    cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40,
    // 40 < nBL <= 48
    cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48,
    cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48,
    // 48 < nBL <= 56
    cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56,
    cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56,
    // 56 < nBL <= 64
    cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64,
    cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64,
  #endif // (cnBitsPerWord >= 64)
    };

static int16_t aauListWordCnt[cnBitsPerWord + 1][cwListPopCntMax + 1];
static uListPopCntMax_t aauListSlotCnt[cnBitsPerWord + 1][cwListPopCntMax + 1];

#ifdef OLD_LIST_WORD_CNT

// How many words are needed for a T_LIST leaf?
// The layout of a list leaf depends on ifdefs and possibly nBL.
// One thing all T_LIST leaves have in common (presently) is an array of keys.
// For OLD_LISTS there may or may not be:
// - a list pop count in the malloc preamble word: LIST_POP_IN_PREAMBLE
// - followed by some dummy words: cnDummiesInList
//   - root list pop count goes in a dummy word: PP_IN_LINK or POP_WORD_IN_LINK
// - followed by a pop count: ! POP_IN_WR_HB && ! LIST_POP_IN_PREAMBLE
//   && ( (!PP_IN_LINK && !POP_WORD_IN_LINK)
//        or list hangs from root word and cnDummiesInList == 0 )
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
CalcListWordCnt(Word_t wPopCntArg, unsigned nBL)
{
    (void)nBL;

    assert(wPopCntArg != 0);

#ifdef FAST_LIST_WORDS
  #ifndef COMPRESSED_LISTS
    #error FAST_LIST_WORDS requires COMPRESSED_LISTS
  #endif // COMPRESSED_LISTS
  #ifdef LIST_END_MARKERS
    #error FAST_LIST_WORDS requires no LIST_END_MARKERS
  #endif // LIST_END_MARKERS
  #ifndef OLD_LISTS
    #error FAST_LIST_WORDS requires OLD_LISTS
  #endif // OLD_LISTS
  #if (N_LIST_HDR_KEYS != 0)
    #error FAST_LIST_WORDS requires (N_LIST_HDR_KEYS != 0)
  #endif // (N_LIST_HDR_KEYS != 0)
    // FAST_LIST_WORDS also requires sizeof(Bucket_t) <= cnMallocMask + 1.
    (void)wPopCntArg;
  #ifdef B_JUDYL
    if (nBL == 8) {
        return 256 + 256 / 8; // cnListPopCntMax8 == 256
    } else {
        // log(56-1) = 5, log(40-1) = 5, exp(5+1) = 64
        // log(32-1) = 4, log(24-1) = 4, exp(4+1) = 32
        // log(16-1) = 3, exp(3+1) = 16
        // log(8-1) = 2, exp(2+1) = 8
        int nBytesPerListEntry = EXP(LOG(nBL-1)-2);
        return 32 + 32 * nBytesPerListEntry / 8; // cnListPopCntMax8 == 32
    }
  #else // B_JUDYL
    if (nBL == 8) {
        return 256 / 8; // cnListPopCntMax = 256
    } else {
        int nBytesPerListEntry = EXP(LOG(nBL-1)-2);
        return 32 * nBytesPerListEntry / 8; // cnListPopCntMax = 32
    }
  #endif // B_JUDYL
#else // FAST_LIST_WORDS
  #ifdef FULL_ALLOC
    wPopCntArg = 256;
  #endif // FULL_ALLOC

  #if defined(COMPRESSED_LISTS)
    // log(56-1) = 5, log(40-1) = 5, exp(5+1) = 64
    // log(32-1) = 4, log(24-1) = 4, exp(4+1) = 32
    // log(16-1) = 3, exp(3+1) = 16
    // log(8-1) = 2, exp(2+1) = 8
    assert(nBL >= 5);
    // Will the compiler get rid of LOG and EXP if nBL is a constant?
    // Or are we better off with the old way?
    //    int nBytesKeySz =
    //   #if defined(COMPRESSED_LISTS)
    //        (nBL <=  8) ? 1 : (nBL <= 16) ? 2 :
    //      #if (cnBitsPerWord > 32)
    //        (nBL <= 32) ? 4 :
    //      #endif // (cnBitsPerWord > 32)
    //   #endif // defined(COMPRESSED_LISTS)
    //        sizeof(Word_t);
    // Or should we just assume that nBL is a multiple of 8?
    int nBytesKeySz = EXP(LOG(nBL-1)-2);
  #else // defined(COMPRESSED_LISTS)
    int nBytesKeySz = sizeof(Word_t);
  #endif // defined(COMPRESSED_LISTS)

    wPopCntArg = ListSlotCnt(wPopCntArg, nBL);

#if defined(OLD_LISTS)

    int nBytesHdr = cnDummiesInList * sizeof(Word_t);
    nBytesHdr += (N_LIST_HDR_KEYS + POP_SLOT(nBL)) * nBytesKeySz;
    if (ALIGN_LIST(nBytesKeySz))
    {
        if ((cnMallocMask + 1) < sizeof(Bucket_t)) {
            // We don't know what address we are going to get from malloc.
            // We have to allocate enough memory to ensure that we will be able
            // to align the beginning of the array of real keys.
            nBytesHdr += sizeof(Bucket_t) - (cnMallocMask + 1);
            nBytesHdr
                = ALIGN_UP(nBytesHdr, sizeof(Bucket_t) - (cnMallocMask + 1));
        } else {
            nBytesHdr = ALIGN_UP(nBytesHdr, sizeof(Bucket_t));
        }
    }

    int nBytesKeys = wPopCntArg * nBytesKeySz; // add list of real keys

    // Pad array of keys so the end is aligned.
    // We'll eventually fill the padding with replicas of the last real key
    // so parallel searching yields no false positives.
//printf("nBytesKeySz %d\n", nBytesKeySz);
    if (ALIGN_LIST_LEN(nBytesKeySz))
    {
//printf("nBytesKeys b %d\n", nBytesKeys);
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
//printf("nBytesKeys a %d\n", nBytesKeys);
    }

#if defined(LIST_END_MARKERS)
    // Make room for -1 at the end to help make search faster.
    // The marker at the beginning is accounted for in N_LIST_HDR_KEYS.
    // How should we handle LIST_END_MARKERS for parallel searches?
    nBytesKeys += nBytesKeySz;
#endif // defined(LIST_END_MARKERS)

    int nBytes = nBytesHdr + nBytesKeys;

#if defined(LIST_REQ_MIN_WORDS)
    nBytes = ALIGN_UP(nBytes, sizeof(Word_t));
  #ifdef B_JUDYL
    nBytes += ALIGN_UP(wPopCntArg * sizeof(Word_t), sizeof(Bucket_t));
  #endif // B_JUDYL
    return nBytes / sizeof(Word_t);
#else // defined(LIST_REQ_MIN_WORDS)
    // Round up to full malloc chunk which is some odd number of words.
    // Malloc always allocates an integral number of MALLOC_ALIGNMENT-size
    // units with the second word aligned and uses the first word for itself.
    // Malloc never allocates less than four words.
    nBytes += sizeof(Word_t); // add a word for malloc
    int nWords
        = MAX(4, ALIGN_UP(nBytes, cnMallocMask + 1) >> cnLogBytesPerWord) - 1;
    return nWords;
#endif // defined(LIST_REQ_MIN_WORDS)

#else // defined(OLD_LISTS)
    return ls_nSlotsInList(wPopCntArg, nBL, nBytesKeySz)
        * nBytesKeySz / sizeof(Word_t);
#endif // defined(OLD_LISTS)
#endif // FAST_LIST_WORDS
}

#else // OLD_LIST_WORD_CNT

// ListWordsMin returns the minimum number of words that will hold a list.
// It respects all alignment constraints.
// The return value could be passed to MyMalloc and we'd get a buffer
// big enough for the list.
// Our current approach for 64-bit JudyL is to put the key area after the
// value area with pwr pointing at the key area. One benefit of this
// approach is locating the value after finding the key is really easy.
// One disadvantage is we have to align the key area on a malloc boundary.
// What cases can we not handle yet? We assume pop is not in key area.
#ifndef LIST_POP_IN_PREAMBLE
#ifndef POP_WORD_IN_LINK
#ifndef POP_IN_WR_HB
#ifndef PP_IN_LINK
  #error ListWordsMin requires LIST_POP_IN_PREAMBLE for 32-bit JudyL.
#endif // #ifndef PP_IN_LINK
#endif // #ifndef POP_IN_WR_HB
#endif // #ifndef POP_WORD_IN_LINK
#endif // #ifndef LIST_POP_IN_PREAMBLE
static int
ListWordsMin(int nPopCnt, int nBL)
{
    int nBytesPerKey = ExtListBytesPerKey(nBL);
#ifdef B_JUDYL
#ifndef PACK_L1_VALUES
    if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
        nPopCnt = EXP(cnBitsInD1);
    }
#endif // #ifndef PACK_L1_VALUES
#endif // B_JUDYL
#ifdef UA_PARALLEL_128 // implies !B_JUDYL && PARALLEL_128 && 32-bit
  #ifdef B_JUDYL
    #error UA_PARALLEL_128 and B_JUDYL are incompatible
  #endif // B_JUDYL
  #ifndef PARALLEL_128
    #error UA_PARALLEL_128 and no PARALLEL_128 are incompatible
  #endif // PARALLEL_128
  #if (cnBitsPerWord > 32)
    #error UA_PARALLEL_128 and 64-bit are incompatible
  #endif // (cnBitsPerWord > 32)
    // I wonder if we could calculate the population count for this case to
    // obviate the need for a pop count in memory.
    if ((nPopCnt <= 6) && (nBL == 16)) {
        return 3;
    }
#endif // UA_PARALLEL_128
    int nBytesPerBucket // key allocation unit
        = ALIGN_LIST_LEN(nBytesPerKey) ? sizeof(Bucket_t) : sizeof(Word_t);
    int nKeyBytes = ALIGN_UP(nPopCnt * nBytesPerKey, nBytesPerBucket);
    int nKeyWords = nKeyBytes / sizeof(Word_t); // accumulator
#ifdef B_JUDYL
    int nValueWords = nPopCnt;
  #ifdef LIST_POP_IN_PREAMBLE
    ++nValueWords; // space for list pop
  #else // LIST_POP_IN_PREAMBLE
      #if (cnBitsPerWord == 32)
          #error ListWordsMin requires LIST_POP_IN_PREAMBLE for 32-bit JudyL.
      #endif // (cnBitsPerWord == 32)
  #endif // #else LIST_POP_IN_PREAMBLE
    // Keys must begin on a malloc alignment boundary.
    nValueWords = ALIGN_UP(nValueWords,
                           cnMallocAlignment >> cnLogBytesPerWord);
    // Malloc returns a malloc aligned pointer. If our keys have to be more
    // strictly aligned then we have a problem because we have to wait until
    // runtime to figure out how much to offset pwr. Or we have to
    // overallocate.
    // The code isn't smart enough for either one yet.
    assert((cnMallocAlignment >= sizeof(Bucket_t))
        || !ALIGN_LIST(nBytesPerKey));
    nKeyWords += nValueWords;
#endif // B_JUDYL
    return nKeyWords; // accumulated list words
}

// CalcListWordCnt is for reducing the frequency of list mallocs and copies
// and the number of different sizes of malloc buffers that we would otherwise
// use if we had only ListWordsMin.
// We use sizes that are roughly the powers of the square root of two.
// CalcListWordCnt and CalcListSlotCnt have limitations.
// They may not have not been vetted with UA_PARALLEL_128.
// They have not been enhanced for header words
// LIST_END_MARKERS, !LIST_REQ_MIN_WORDS, !OLD_LISTS.
static int
CalcListWordCnt(int nPopCnt, int nBL)
{
    int nListWordsMin = ListWordsMin(nPopCnt, nBL);
    // What is the first power of two bigger than nListWordsMin ** 2?
    // nListWordsMin being a power of two does us no good because it won't
    // accommodate the malloc overhead word.
    int nNextPow = EXP(LOG(nListWordsMin) + 1); // first power of two bigger
    // See if the power of two divided by the square root of two is
    // big enough for our list.
    // Candidate number of malloc words including one word of malloc overhead.
    int nWords = MAX(4, (nNextPow * 46340 / (1<<16) + 1) & ~1);
    if (nListWordsMin > nWords - 1) { nWords = nNextPow; }
    --nWords; // Subtract malloc overhead word for request.
    assert((nPopCnt <= auListPopCntMax[nBL])
#ifdef EMBED_KEYS
  #ifndef POP_CNT_MAX_IS_KING
        || (nPopCnt <= EmbeddedListPopCntMax(nBL))
  #endif // #ifndef POP_CNT_MAX_IS_KING
#endif // EMBED_KEYS
            );
    // Trim to avoid wasted space.
    // No need to trim for temporarily inflated embedded list that
    // exceeds the maximum external list length.
    // Don't waste cpu calling EmbeddedListPopCntMax.
#ifdef EMBED_KEYS
  #ifndef POP_CNT_MAX_IS_KING
    if (nPopCnt <= auListPopCntMax[nBL])
  #endif // #ifndef POP_CNT_MAX_IS_KING
#endif // EMBED_KEYS
    {
        int nFullListWordsMin = ListWordsMin(auListPopCntMax[nBL], nBL);
        if (nFullListWordsMin < nWords) {
            nWords = nFullListWordsMin;
        }
    }
#ifdef LIST_REQ_MIN_WORDS // don't request words that have no benefit
    // We used a simple method of choosing the malloc request size.
    // Is it possible that nWords minus one malloc chunk holds
    // just as many keys so we're wasting a malloc chunk? Or wasting
    // more than one malloc chunk?
    // Requesting a single word that we won't use, even if it doesn't take
    // memory from the heap, is inconsistent with LIST_REQ_MIN_WORDS.
#endif // LIST_REQ_MIN_WORDS
    return nWords;
}

#endif // #else OLD_LIST_WORD_CNT

static int
ListWordCnt(int nPopCnt, int nBL)
{
    assert(nBL < (int)(sizeof(aauListWordCnt) / sizeof(aauListWordCnt[0])));
    assert(nPopCnt
           < (int)(sizeof(aauListWordCnt[0]) / sizeof(aauListWordCnt[0][0])));
    int nListWordCnt = aauListWordCnt[nBL][nPopCnt];
    if (nListWordCnt == 0) {
        nListWordCnt = CalcListWordCnt(nPopCnt, nBL);
        aauListWordCnt[nBL][nPopCnt] = nListWordCnt;
    }
    return nListWordCnt;
}

#ifdef OLD_LIST_WORD_CNT

// How many keys fit in a list buffer that must hold at least nPopCnt keys?
static int
CalcListSlotCnt(int nPopCnt, int nBL)
{
    int nBytesPerKey = ExtListBytesPerKey(nBL);
    int nSlots = ALIGN_UP(nPopCnt, sizeof(Bucket_t) / nBytesPerKey);
    return EXP(LOG(nSlots - 1) + 1);
}

#else // OLD_LIST_WORD_CNT

// We have to be able to figure out where to point pwr from nPopCnt and nBL.
// We can get list words from ListWordCnt.
// But we need to know the max capacity of that size.
// How do we do it? Successive approximation method. Yuck.
// How many keys fit in a list buffer that must hold at least nPopCnt keys?
static int
CalcListSlotCnt(int nPopCnt, int nBL)
{
#ifdef B_JUDYL
#ifndef PACK_L1_VALUES
    if ((cnBitsInD1 <= 8) && (nBL == cnBitsInD1)) {
        return EXP(cnBitsInD1);
    }
#endif // #ifndef PACK_L1_VALUES
#endif // B_JUDYL
    int nBytesPerKey = ExtListBytesPerKey(nBL);
    int nBytesPerBucket
        = ALIGN_LIST_LEN(nBytesPerKey)
#ifdef UA_PARALLEL_128 // implies B_JUDYL && PARALLEL_128
                && ((nPopCnt > 6) || (nBL != 2))
#endif // UA_PARALLEL_128
            ? sizeof(Bucket_t) : sizeof(Word_t);
    int nListWords = ListWordCnt(nPopCnt, nBL);
    int nWordsPerBucket = nBytesPerBucket >> cnLogBytesPerWord;
    int nWordsPerUnit = nWordsPerBucket; // accumulator
    int nKeysPerBucket = nBytesPerBucket / nBytesPerKey;
#ifdef B_JUDYL
    // A unit is a full bucket of keys plus the corresponding values with
    // no regard for alignment issues.
    nWordsPerUnit += nKeysPerBucket;
    int nWordsPerMallocChunk = cnMallocAlignment >> cnLogBytesPerWord;
    // nWordsPerChunk represents the alignment requirement of the start of
    // the key area.
    // It must be malloc aligned because we use the low bits of the pointer.
    int nWordsPerChunk = nWordsPerMallocChunk; // accumulator
    // It must be bucket aligned if we are aligning buckets.
    assert((nWordsPerBucket <= nWordsPerMallocChunk)
        || !ALIGN_LIST(nBytesPerKey));
#ifdef LIST_POP_IN_PREAMBLE
    // How do we incorporate the pop word?
    // I'm not sure about this method.
    // I wonder if we should go after key chunks first rather than
    // value chunks.
    int nListChunks = (nListWords - 1) / nWordsPerChunk;
#else // LIST_POP_IN_PREAMBLE
    int nListChunks = nListWords / nWordsPerChunk;
#endif // #endif LIST_POP_IN_PREAMBLE
    int nValueChunks = nListChunks * nKeysPerBucket / nWordsPerUnit;
    if (nValueChunks == 0) {
        nValueChunks = 1;
    }
    int nValues = nValueChunks * nWordsPerChunk;
#ifdef LIST_POP_IN_PREAMBLE
    --nValues;
#endif // LIST_POP_IN_PREAMBLE
    int nKeyBuckets
        = (nListWords - nValueChunks * nWordsPerChunk) / nWordsPerBucket;
    int nKeys = nKeyBuckets * nKeysPerBucket;
    if (nKeys > nValues) {
        nKeys = nValues;
        // Try one more value chunk.
        ++nValueChunks;
        // Will it always be enough for LIST_POP_IN_PREAMBLE?
        nKeyBuckets
            = (nListWords - nValueChunks * nWordsPerChunk) / nWordsPerBucket;
        if (nKeyBuckets * nKeysPerBucket > nKeys) {
            nKeys = nKeyBuckets * nKeysPerBucket; // accumulator
            if (nValues + nWordsPerChunk < nKeys) {
                nKeys = nValues + nWordsPerChunk;
            }
        }
    }
    int nListSlots = nKeys;
#else // B_JUDYL
    int nListUnits = nListWords / nWordsPerUnit;
    int nListSlots = nListUnits * nKeysPerBucket; // slots/unit = keys/bucket
#endif // #endif B_JUDYL
#ifdef EMBED_KEYS
  #ifndef POP_CNT_MAX_IS_KING
    if (nPopCnt <= auListPopCntMax[nBL])
  #endif // #ifndef POP_CNT_MAX_IS_KING
#endif // EMBED_KEYS
    {
        if (nListSlots > auListPopCntMax[nBL]) {
            nListSlots = auListPopCntMax[nBL];
        }
    }
#ifdef DEBUG
    int nListWordsX = CalcListWordCnt(nListSlots, nBL);
    if (nListWordsX != nListWords) {
        printf("\n");
        printf("ListWordCnt(nPopCnt %d, nBL %d) %d\n",
                nPopCnt, nBL, nListWords);
        printf("ListSlotCnt(nPopCnt %d, nBL %d) %d\n",
                nPopCnt, nBL, nListSlots);
        printf("ListWordCnt(nPopCnt %d, nBL %d) %d\n",
                nListSlots, nBL, nListWordsX);
        printf("auListPopCntMax[nBL %d] %d\n", nBL, auListPopCntMax[nBL]);
    }
    assert(nListWordsX == nListWords);
#endif // DEBUG
    return nListSlots;
}

// Successive approximation version of ListSlotCnt.
#ifdef LIST_SLOT_CNT_SUCCESSIVE_APPROXIMATION
static int
CalcListSlotCntX(int nPopCnt, int nBL)
{
    assert(nPopCnt <= auListPopCntMax[nBL]);
    int nBytesPerKey = ExtListBytesPerKey(nBL);
#ifdef UA_PARALLEL_128 // implies 32-bit Judy && PARALLEL_128
  #ifdef B_JUDYL
    #error B_JUDYL with UA_PARALLEL_128.
  #endif // B_JUDYL
  #if (cnBitsPerWord > 32)
    #error B_JUDYL with UA_PARALLEL_128.
  #endif // (cnBitsPerWord > 32)
    if ((nPopCnt <= 6) && (nBL == 2)) {
        return 6;
    }
#endif // UA_PARALLEL_128
    // How many words are we working with?
    int nListWords = ListWordCnt(nPopCnt, nBL);
    // How many whole units fit in nListWords?
    int nBytesPerBucket // bytes of keys per allocation unit
        = ALIGN_LIST_LEN(nBytesPerKey) ? sizeof(Bucket_t) : sizeof(Word_t);
    int nWordsPerBucket = nBytesPerBucket / cnBytesPerWord;
    int nWordsPerUnit = nWordsPerBucket; // accumulator
    int nKeysPerBucket = nBytesPerBucket / nBytesPerKey;
#ifdef B_JUDYL
    nWordsPerUnit += nKeysPerBucket; // values
#endif // B_JUDYL
    int nUnits = nListWords / nWordsPerUnit;
    // Do that many keys fit in our list?
    // Keys per unit == keys per bucket.
    int nSlots = nKeysPerBucket * nUnits;
    if (nSlots > auListPopCntMax[nBL]) {
        nSlots = auListPopCntMax[nBL];
    }
    if (ListWordCnt(nSlots, nBL) <= nListWords) {
        if (nSlots < auListPopCntMax[nBL]) {
            while (ListWordCnt(++nSlots, nBL) <= nListWords) {}
            --nSlots;
        }
    } else {
        while (ListWordCnt(--nSlots, nBL) > nListWords) {}
    }
    if (nSlots > auListPopCntMax[nBL]) {
        nSlots = auListPopCntMax[nBL];
    }
    assert(ListWordCnt(nSlots, nBL) == nListWords);
    if (ListSlotCnt(nPopCnt, nBL) != nSlots) {
        printf("\n");
        printf("nPopCnt %d\n", nPopCnt);
        printf("nBL %d\n", nBL);
        printf("nSlots %d\n", nSlots);
        printf("ListSlotCnt(nPopCnt, nBL) %d\n", ListSlotCnt(nPopCnt, nBL));
    }
    assert(ListSlotCnt(nPopCnt, nBL) == nSlots);
    if (ListSlotCnt(nSlots, nBL) != nSlots) {
        printf("\n");
        printf("nSlots %d\n", nSlots);
        printf("nBL %d\n", nBL);
        printf("ListSlotCnt(nSlots, nBL) %d\n", ListSlotCnt(nSlots, nBL));
    }
    assert(ListSlotCnt(nSlots, nBL) == nSlots);
    return nSlots;
}
#endif // LIST_SLOT_CNT_SUCCESSIVE_APPROXIMATION

#endif // #else OLD_LIST_WORD_CNT

static int
ListSlotCnt(int nPopCnt, int nBL)
{
    assert(nBL < (int)(sizeof(aauListSlotCnt) / sizeof(aauListSlotCnt[0])));
    assert(nPopCnt
           < (int)(sizeof(aauListSlotCnt[0]) / sizeof(aauListSlotCnt[0][0])));
    int nListSlotCnt = aauListSlotCnt[nBL][nPopCnt];
    if (nListSlotCnt == 0) {
        nListSlotCnt = CalcListSlotCnt(nPopCnt, nBL);
        assert(nListSlotCnt <= (int)MSK(sizeof(aauListSlotCnt[0][0]) * 8));
        aauListSlotCnt[nBL][nPopCnt] = nListSlotCnt;
    }
    return nListSlotCnt;
}

static void
NewListCommon(Word_t *pwList, Word_t wPopCnt, unsigned nBL, unsigned nWords)
{
    (void)pwList; (void)wPopCnt; (void)nBL; (void)nWords;
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
#if defined(LIST_END_MARKERS)
        ls_pcKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
    } else if (nBL <= 16) {
#if defined(LIST_END_MARKERS)
        ls_psKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
    } else if (nBL <= 24) {
#if defined(LIST_END_MARKERS)
  #if (cnBitsPerWord > 32)
        ls_piKeysNAT(pwList)[-1] = 0;
  #else // (cnBitsPerWord > 32)
        ls_pwKeysNAT(pwList)[-1] = 0;
  #endif // (cnBitsPerWord > 32)
#endif // defined(LIST_END_MARKERS)
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
#if defined(LIST_END_MARKERS)
        ls_piKeysNAT(pwList)[-1] = 0;
#endif // defined(LIST_END_MARKERS)
#endif // (cnBitsPerWord > 32)
    }
    else
#endif // defined(COMPRESSED_LISTS)
    {
#if defined(LIST_END_MARKERS)
        ls_pwKeysNAT(pwList)[-1
  #if (cnDummiesInList==0)
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // ls_pwKeys is for T_LIST not at top (it incorporates dummies
            // and markers, but not pop count)
                             + (nBL == cnBitsPerWord)
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
  #endif // (cnDummiesInList==0)
                             ] = 0;
#endif // defined(LIST_END_MARKERS)
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

    int nWords = ListWordCnt(wPopCnt, nBL);

    Word_t *pwList
        = (Word_t*)MyMalloc(nWords,
                            &j__AllocWordsJLL[(nBL + 7) / 8
                                                  & (sizeof(Word_t) - 1)]);

#ifdef B_JUDYL
  #ifdef FAST_LIST_WORDS
    if (nBL == 8) {
        pwList = &pwList[256]; // must agree with ListWordCnt
    } else {
        pwList = &pwList[32]; // must agree with ListWordCnt
    }
  #else // FAST_LIST_WORDS
      #ifdef FULL_ALLOC
    pwList = (Word_t *)ALIGN_UP((Word_t)&pwList[256], sizeof(Bucket_t));
      #else // FULL_ALLOC
    //pwList = (Word_t*)ALIGN_UP((Word_t)&pwList[nKeySlots], sizeof(Bucket_t));
    int nKeySlots = ListSlotCnt(wPopCnt, nBL);
#ifdef LIST_POP_IN_PREAMBLE
    ++nKeySlots; // make room for list pop count
#endif // LIST_POP_IN_PREAMBLE
    pwList = (Word_t*)ALIGN_UP((Word_t)&pwList[nKeySlots], cnMallocAlignment);
      #endif // FULL_ALLOC
  #endif // FAST_LIST_WORDS
#else // B_JUDYL
  #if ! defined(OLD_LISTS)
    pwList += nWords - 1;
  #endif // ! defined(OLD_LISTS)
#endif // B_JUDYL

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
static Word_t *
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

static int
OldList(Word_t *pwList, int nPopCnt, int nBL, int nType)
{
    (void)nType;
    int nWords = ListWordCnt(nPopCnt, nBL);

#ifdef B_JUDYL
  #ifdef FAST_LIST_WORDS
    if (nBL == 8) {
        pwList = &pwList[-256];
    } else {
        pwList = &pwList[-32];
    }
  #else // FAST_LIST_WORDS
      #ifdef FULL_ALLOC
    pwList = (Word_t *)((Word_t)&pwList[-256] & ~cnMallocMask);
      #else // FULL_ALLOC
    int nKeySlots = ListSlotCnt(nPopCnt, nBL);
          #ifdef LIST_POP_IN_PREAMBLE
    ++nKeySlots; // make room for list pop count
          #endif // LIST_POP_IN_PREAMBLE
    pwList = (Word_t*)((Word_t)&pwList[-nKeySlots] & ~cnMallocMask);
      #endif // FULL_ALLOC
  #endif // FAST_LIST_WORDS
#else // B_JUDYL
  #if ! defined(OLD_LISTS)
    assert(nType == T_LIST);
    if (nType == T_LIST) { pwList -= nWords - 1; }
  #endif // ! defined(OLD_LISTS)
#endif // B_JUDYL

    MyFree(pwList, nWords,
           &j__AllocWordsJLL[(nBL + 7) / 8 & (sizeof(Word_t) - 1)]);

    return nWords * sizeof(Word_t);
}

#endif // (cwListPopCntMax != 0)

// cnLogBmWordsX determines when a bitmap value area grows to uncompressed.
// Roughly, we transition when words per key won't be much more than
// EXP(cnLogBmWordsX + 1) / (EXP(cnLogBmWordsX) + 1)
// It's a number between one and two.
// A bigger cnLogBmWordsX means a bigger words per key is allowed.
#define cnLogBmWordsX  5

#ifdef BITMAP

static Word_t
BitmapWordCnt(int nBLR, Word_t wPopCnt)
{
    (void)nBLR; (void)wPopCnt;
    Word_t wWords;
  #ifdef B_JUDYL
    assert(nBLR == cnBitsInD1);
    // Number of words in BmLeaf_t plus the bitmap.
    // BmLeaf_t proper includes one word of the bitmap.
    Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t)
                         + EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
    // Go for an uncompressed value area as soon as possible while keeping
    // words per key reasonable.
    // If wpk <= threshhold, then go to uncompressed, where words is size of
    // the bitmap leaf (including an estimated 1 word of malloc overhead) plus
    // 1 word in the switch for wRoot and one word in the switch
    // for an embedded value.
    Word_t wFullPopWordsMin = wWordsHdr + EXP(nBLR);
      // PACK_BM_VALUES is an incomplete quick hack to see the performance of
      // a bitmap leaf with an uncompressed value area and no test of
      // bLsbBmUncompressed. The plot is perfectly flat.
      #ifdef PACK_BM_VALUES
    Word_t wFullPopWordsMinPlusMalloc = (wFullPopWordsMin | 1) + 1;
    Word_t wFullPopWordsMinPlusX
        = wFullPopWordsMinPlusMalloc + sizeof(Link_t) / sizeof(Word_t);
          #ifdef EMBED_KEYS
    ++wFullPopWordsMinPlusX; // Value word in switch.
          #endif // EMBED_KEYS
    // Max pop with compressed value area.
    Word_t wPopCntMax
        = wFullPopWordsMinPlusX
            * (EXP(cnLogBmWordsX) + 1) / EXP(cnLogBmWordsX + 1);
    // We want wPopCntMax to be efficient w.r.t. malloc.
    // That is, we want wWordsMin(wPopCntMax) to be an odd number.
    if (!((wWordsHdr + wPopCntMax) & 1)) {
        ++wPopCntMax;
    }
    if (wPopCnt > wPopCntMax) {
        return wFullPopWordsMin;
    }
    Word_t wWordsMin = wWordsHdr + wPopCnt; // space for hdr + values
    wWords = wWordsMin | 1; // make efficent for malloc
    assert(wWords >= 3); // minimum efficient for malloc
      #else // PACK_BM_VALUES
    return wWords = wFullPopWordsMin;
      #endif // PACK_BM_VALUES
  #else // B_JUDYL
    (void)wPopCnt;
      #if !defined(KISS_BM) && !defined(KISS)
    if (cbEmbeddedBitmap) {
        assert(nBLR == cnBitsLeftAtDl2);
        wWords = sizeof(BmLeaf_t) / sizeof(Word_t)
                     + EXP(MAX(1, cnBitsLeftAtDl2 - cnLogBitsPerWord));
    } else if (cn2dBmMaxWpkPercent == 0) {
        assert(nBLR == cnBitsInD1);
        wWords = sizeof(BmLeaf_t) / sizeof(Word_t)
                     + EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
    } else
      #endif // !defined(KISS_BM) && !defined(KISS)
    {
        // Number of words in the bitmap plus the rest of BmLeaf_t.
        // BmLeaf_t includes one word of bitmap.
        wWords = sizeof(BmLeaf_t) / sizeof(Word_t)
                     + EXP(MAX(1, nBLR - cnLogBitsPerWord));
    }
  #endif // #else B_JUDYL
    return wWords;
}

// We don't need NewBitmap unless (cnBitsLeftAtD1 > cnLogBitsPerLink).
// Hopefully, the compiler will figure it out and not emit it.
static Word_t *
NewBitmap(qp, int nBLR, Word_t wKey, Word_t wPopCnt)
{
    qv; (void)wKey;
    Word_t wWords = BitmapWordCnt(nBLR, wPopCnt);

    Word_t *pwBitmap
        = (Word_t *)MyMalloc(wWords,
  #if (cn2dBmMaxWpkPercent != 0)
                             (nBLR == nDL_to_nBL(2))
                                 ? &j__AllocWordsJLB2 :  // DL2 big bitmap leaf
  #endif // (cn2dBmMaxWpkPercent != 0)
                                   &j__AllocWordsJLB1    // bitmap leaf
                             );

    DBGM(printf("NewBitmap nBLR %d nBits " OWx
      " nBytes " OWx" wWords " OWx" pwBitmap " OWx"\n",
        nBLR, EXP(nBLR), EXP(nBLR - cnLogBitsPerByte), wWords,
        (Word_t)pwBitmap));

    memset((void *)pwBitmap, 0, wWords * sizeof(Word_t));

    // Init wRoot before calling set_wr because set_wr may try to preserve
    // the high bits, e.g. if LVL_IN_WR_HB, so we want them to
    // be initialized.
    wRoot = 0; set_wr(wRoot, pwBitmap, T_BITMAP);
  #ifdef B_JUDYL
      #if (cnBitsPerWord > 32)
    if (wWords == BitmapWordCnt(nBLR, EXP(nBLR))) {
        wRoot |= EXP(cnLsbBmUncompressed);
    }
      #endif // (cnBitsPerWord > 32)
    DBGM(printf("NewBitmap wRoot 0x%zx\n", wRoot));
  #endif // B_JUDYL

#if defined(SKIP_TO_BITMAP)
    if (nBL != nBLR) {
        Set_nBLR(&wRoot, nBLR);
        set_wr_nType(wRoot, T_SKIP_TO_BITMAP);
    }
#else // defined(SKIP_TO_BITMAP)
    assert(nBL == nBLR);
#endif // defined(SKIP_TO_BITMAP)
    *pwRoot = wRoot;
    pwr = wr_pwr(wRoot);
    nType = wr_nType(wRoot);
#if defined(SKIP_TO_BITMAP)
    swBitmapPrefix(qy, nBLR, wKey);
#endif // defined(SKIP_TO_BITMAP)
    swBitmapPopCnt(qy, /* nBLR */ nBLR, wPopCnt);

    return pwBitmap;
}

#endif // BITMAP

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
            ? (int)wr_nBL(*pwRoot) :
  #endif // defined(SKIP_LINKS)
              nBL ;
}

#ifdef BITMAP
static Word_t
OldBitmap(Word_t *pwr, int nBLR, Word_t wPopCnt)
{
    Word_t wWords = BitmapWordCnt(nBLR, wPopCnt);

    MyFree(pwr, wWords,
#if (cn2dBmMaxWpkPercent != 0)
           (nBLR == nDL_to_nBL(2))
               ? &j__AllocWordsJLB2 :  // DL1 big bitmap leaf
#endif // (cn2dBmMaxWpkPercent != 0)
                 &j__AllocWordsJLB1    // bitmap leaf
           );

    return wWords * sizeof(Word_t);
}
#endif // BITMAP

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
          int nBWX,
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
    if ((nBWX != nBL_to_nBW(nBL)) /*&& (nBWX != cnBW)*/) {
        DBGI(printf("# NewSwitch(nBWX %d)\n", nBWX));
    }
#endif // defined(CODE_XX_SW)
    assert((sizeof(Switch_t) % sizeof(Word_t)) == 0);
#if defined(CODE_BM_SW)
    assert((sizeof(BmSwitch_t) % sizeof(Word_t)) == 0);
#endif // defined(CODE_BM_SW)

#if defined(CODE_XX_SW)
    int nBW = nBWX;
#else // defined(CODE_XX_SW)
    int nBW = nBL_to_nBW(nBL);
#endif // defined(CODE_XX_SW)
    Word_t wIndexCnt = EXP(nBW);

#ifndef USE_XX_SW_ONLY_AT_DL2
    assert(nBL - nBW >= cnBitsInD1);
#endif // #ifndef USE_XX_SW_ONLY_AT_DL2

#if defined(CODE_BM_SW)
    if (nType == T_BM_SW) {
        assert(wIndexCnt <= N_WORDS_SWITCH_BM
               * cnBitsPerWord
  #ifdef OFFSET_IN_SW_BM_WORD
               / 2
  #endif // OFFSET_IN_SW_BM_WORD
               );
    }
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
        (nType == T_BM_SW)
            ? sizeof(BmSwitch_t)
  #ifndef BM_IN_LINK
                + ALIGN_UP(N_WORDS_SWITCH_BM * sizeof(Word_t),
                           cnMallocAlignment)
  #endif // BM_IN_LINK
            :
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

    wBytes += wLinks * sizeof(Link_t);
    Word_t wWords = wBytes / sizeof(Word_t);
#if defined(B_JUDYL) && defined(EMBED_KEYS)
    wWords += wLinks; // Embedded Values in Switch
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)

#ifdef RAMMETRICS
    // Is a branch with embedded bitmaps a branch?
    Word_t *pwAllocWords =  // RAMMETRICS
  #if defined(CODE_BM_SW)
        (nType == T_BM_SW) ? &j__AllocWordsJBB :  // bitmap branch
  #endif // defined(CODE_BM_SW)
                             &j__AllocWordsJBU ;  // uncompressed branch
#endif // RAMMETRICS
#if defined(CODE_BM_SW) && defined(CACHE_ALIGN_BM_SW)
    Word_t *pwr
        = (Word_t*)MyMallocGuts(wWords,
                                (nType == T_BM_SW) ? 6 : cnBitsMallocMask,
                                pwAllocWords);
#else // CACHE_ALIGN_BM_SW
    Word_t *pwr = (Word_t*)MyMalloc(wWords, pwAllocWords);
#endif // CACHE_ALIGN_BM_SW
#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
    if (nType == T_BM_SW) {
        pwr += ALIGN_UP(N_WORDS_SWITCH_BM,
                        cnMallocAlignment >> cnLogBytesPerWord);
    }
#endif // defined(CODE_BM_SW) && !defined(BM_IN_LINK)
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
    if (cbEmbeddedBitmap && ((nBL - nBW) <= cnLogBitsPerLink)) {
        memset(pLinks, 0, sizeof(Link_t) * wLinks);
    } else {
        for (int nn = 0; nn < (int)wLinks; ++nn) {
            pLinks[nn].ln_wRoot = WROOT_NULL;
        }
    }
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
        set_pwr_nBW(pwRoot, nBW);
#endif // defined(CODE_XX_SW)
#if defined(NO_SKIP_AT_TOP)
        assert((nBLUp < cnBitsPerWord) || (nBL == nBLUp));
#endif // defined(NO_SKIP_AT_TOP)
        assert(nBL <= nBLUp);
#if defined(SKIP_LINKS)
        if (nBL < nBLUp) { // Is "nBLUp" actually pre-skip "nBL"?
            set_wr_nBL(*pwRoot, nBL); // also sets nType == T_SKIP_TO_SWITCH
  #if defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
      #ifdef USE_XX_SW_ONLY_AT_DL2
            if (nBL <= nDL_to_nBL(2))
      #else // USE_XX_SW_ONLY_AT_DL2
            if (nBW != nDL_to_nBW(nDL))
      #endif // #else USE_XX_SW_ONLY_AT_DL2
            {
                set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
            }
  #else // defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
            assert(nBW == nBL_to_nBW(nBL));
  #endif // #else defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
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
            // I think "nBL" here is really "nBLR".
      #ifdef USE_XX_SW_ONLY_AT_DL2
            if (nBL <= nDL_to_nBL(2))
      #else // USE_XX_SW_ONLY_AT_DL2
            if (nBW != nDL_to_nBW(nDL))
      #endif // #else USE_XX_SW_ONLY_AT_DL2
            {
                set_wr_nType(*pwRoot, T_XX_SW);
            } else
  #else // defined(USE_XX_SW)
            assert(nBW == nBL_to_nBW(nBL));
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
            memset(PWR_pwBm(pwRoot, pwr), 0,
                   N_WORDS_SWITCH_BM * sizeof(Word_t));
            Word_t wIndex = (wKey >> (nBL - nBW)) & (wIndexCnt - 1);
            // Set the bit in the bitmap indicating that the new link exists.
            // SetBitInSwBmWord
            int nn = gnWordNumInSwBm(wIndex);
            PWR_pwBm(pwRoot, pwr)[nn] |= gwBitMaskInSwBmWord(wIndex);
  #ifdef OFFSET_IN_SW_BM_WORD
            // UpdateOffsetsInSwBmWords
            while (++nn < N_WORDS_SWITCH_BM) {
                PWR_pwBm(pwRoot, pwr)[nn] += (Word_t)1 << (cnBitsPerWord / 2);
            }
  #endif // OFFSET_IN_SW_BM_WORD
#else // defined(BM_SW_FOR_REAL)
            // Mind the high-order bits of the bitmap word if/when the bitmap
            // is smaller than a whole word.
            // Mind endianness.
            if (nBW < cnLogBitsPerWord) {
                *PWR_pwBm(pwRoot, pwr) = EXP(wIndexCnt) - 1;
            } else {
                memset(PWR_pwBm(pwRoot, pwr), -1,
                       N_WORDS_SWITCH_BM * sizeof(Word_t));
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
            && ((nBL - nBW) > 32)
#else // (cnBitsPerWord > 32)
            && ((nBL - nBW) > 16)
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
            && ((nBL - nBW) > cnBitsInD1)
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

#if defined(PP_IN_LINK)
        assert(nBLUp < cnBitsPerWord);
#endif // defined(PP_IN_LINK)
#if defined(POP_WORD_IN_LINK)
        if (nBLUp < cnBitsPerWord)
#endif // defined(POP_WORD_IN_LINK)
        {
#if defined(CODE_BM_SW)
            if (nType == T_BM_SW) {
                set_PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBL, wPopCnt);
            } else
#endif // defined(CODE_BM_SW)
            {
                set_PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBL, wPopCnt);
            }
        }
    }

    DBGI(printf("NS: prefix " OWx"\n",
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBL)));
    //DBGI(printf("\n# Just before returning from NewSwitch "));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nBL; // nBL is not used for all ifdef combos
    (void)nBLUp; // nBLUp is not used for all ifdef combos
}

#if defined(CODE_BM_SW)
static Word_t
OldSwitch(Word_t *pwRoot, int nBL, int bBmSw, int nLinks, int nBLUp);

#ifdef B_JUDYL
#define InflateBmSw  InflateBmSwL
#else // B_JUDYL
#define InflateBmSw  InflateBmSw1
#endif // #else B_JUDYL

// Uncompress a bitmap switch.
#if defined(B_JUDYL) && defined(EMBED_KEYS)
Word_t*
#else // defined(B_JUDYL) && defined(EMBED_KEYS)
void
#endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
InflateBmSw(Word_t *pwRoot, Word_t wKey, int nBLR, int nBLUp)
{
    Word_t wRoot = *pwRoot;
    Word_t *pwr = wr_pwr(wRoot);

    DBGI(printf("# InflateBmSw wKey " Owx" nBLR %d nBLUp %d\n",
                wKey, nBLR, nBLUp));

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

    Word_t *pwBm = PWR_pwBm(pwRoot, pwr);
    Link_t *pSwLinks = pwr_pLinks((Switch_t*)pwrNew);
    Link_t *pBmSwLinks = pwr_pLinks((BmSwitch_t *)pwr);
    int nLinkCnt = 0; // link number in bm sw
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
        if (pwBm[gnWordNumInSwBm(nn)] & gwBitMaskInSwBmWord(nn)) {
            pSwLinks[nn] = pBmSwLinks[nLinkCnt];
            ++nLinkCnt;
        }
    }
#if defined(B_JUDYL) && defined(EMBED_KEYS)
    Word_t *pSwValues = (Word_t*)&pSwLinks[1<<nBW];
    Word_t *pBmSwValues = (Word_t*)&pBmSwLinks[nLinkCnt];
    nLinkCnt = 0;
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
        if (pwBm[gnWordNumInSwBm(nn)] & gwBitMaskInSwBmWord(nn)) {
            pSwValues[nn] = pBmSwValues[nLinkCnt];
            ++nLinkCnt;
        }
    }
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)

    OldSwitch(&wRoot, nBLR, /* bBmSw */ 1, nLinkCnt, nBLUp);

#if defined(B_JUDYL) && defined(EMBED_KEYS)
    Word_t wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
    if ((pSwLinks[wDigit].ln_wRoot != WROOT_NULL)
        && (wr_nType(pSwLinks[wDigit].ln_wRoot) == T_EMBEDDED_KEYS))
    {
        Word_t *pwValue = &pSwValues[wDigit];
        DBGX(printf("InflateBmSw returning pwValue %p\n", pwValue));
        return pwValue;
    }
    // NULL means pwValue didn't change, but not that nothing changed.
    return NULL;
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
}

// NewLink uses OldSwitch but not NewSwitch. Could we change it?
#if defined(BM_SW_FOR_REAL)
static void
NewLink(qp, Word_t wKey, int nDLR, int nDLUp)
{
    qv;
    int nBLR = nDL_to_nBL(nDLR);
    int nBLUp = nDL_to_nBL(nDLUp);

    DBGI(printf("NewLink(pLn %p wKey " OWx" nBLR %d)\n", pLn, wKey, nBLR));
    DBGI(printf("PWR_wPopCnt %" _fw"d\n",
         PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));

#if defined(BM_IN_LINK)
    assert(nBLR != cnBitsPerWord);
#endif // defined(BM_IN_LINK)

    // What is the index of the new link?
    unsigned nBW = nBL_to_nBW(nBLR);
    Word_t wIndex = (wKey >> (nBLR - nBW)) & MSK(nBW);
    Word_t wDigit = wIndex;
    DBGI(printf("wKey " OWx" nBLR %d nBW %d wIndex (virtual) " OWx"\n",
                wKey, nBLR, nBW, wIndex));

    // How many links are there in the old switch?
    int nLinkCnt = 0;
    for (int nn = 0; nn < N_WORDS_SWITCH_BM; nn++)
    {
        nLinkCnt += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]
#if defined(OFFSET_IN_SW_BM_WORD)
                                   & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
#endif // defined(OFFSET_IN_SW_BM_WORD)
                                         );
    }
    // Now we know how many links were in the old switch.

    // sizeof(BmSwitch_t) includes one link; add the others
    unsigned nWordsOld
         = (sizeof(BmSwitch_t)
#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
                + ALIGN_UP(N_WORDS_SWITCH_BM * sizeof(Word_t),
                           cnMallocAlignment)
#endif // defined(CODE_BM_SW) && !defined(BM_IN_LINK)
                + nLinkCnt * sizeof(Link_t))
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
    if (InflateBmSwTest(qy)) {
        InflateBmSw(pwRoot, wKey, nBLR, nBLUp);
    } else {
        // We replicate a bunch of newswitch here since
        // newswitch can create only empty bm sw.

        // Allocate memory for a new switch with one more link than the
        // old one.
        unsigned nWordsNew = nWordsOld + sizeof(Link_t) / sizeof(Word_t);
#if defined(B_JUDYL) && defined(EMBED_KEYS)
        nWordsNew += nLinkCnt + 1; // Embedded Values in Switch
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
        Word_t *pwBm = PWR_pwBm(pwRoot, pwr);
#if defined(CODE_BM_SW) && defined(CACHE_ALIGN_BM_SW)
        *pwRoot = MyMallocGuts(nWordsNew, /* logAlign */ 6, &j__AllocWordsJBB);
#else // CACHE_ALIGN_BM_SW
        *pwRoot = MyMalloc(nWordsNew, &j__AllocWordsJBB);
#endif // CACHE_ALIGN_BM_SW

#ifndef BM_IN_LINK
        *pwRoot += ALIGN_UP(N_WORDS_SWITCH_BM * sizeof(Word_t),
                            cnMallocAlignment);
#endif // BM_IN_LINK
        DBGI(printf("After malloc *pwRoot " OWx"\n", *pwRoot));

        // Where does the new link go?
        int nBmWordNum = gnWordNumInSwBm(wIndex);
        Word_t wBmWord = PWR_pwBm(pwRoot, pwr)[nBmWordNum];
        Word_t wBmBitMask = gwBitMaskInSwBmWord(wIndex);
        Word_t wBmMask = wBmBitMask - 1;
        assert( ! (wBmWord & wBmBitMask) );
        // recalculate index as link number in sparse array of links
        Word_t wIndex = 0;
        for (int nn = 0; nn < nBmWordNum; nn++) {
            wIndex += __builtin_popcountll(PWR_pwBm(pwRoot, pwr)[nn]
#if defined(OFFSET_IN_SW_BM_WORD)
                                   & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
#endif // defined(OFFSET_IN_SW_BM_WORD)
                                           );
        }
        wIndex += __builtin_popcountll(wBmWord & wBmMask);
        // Now we know where the new link goes.
        DBGI(printf("wIndex (physical) " OWx"\n", wIndex));

        // Copy the old switch to the new switch and insert the new link.
        // copy header and leading links from old switch to new switch
        memcpy(wr_pwr(*pwRoot), pwr,
               sizeof(BmSwitch_t) + wIndex * sizeof(Link_t));
#ifndef BM_IN_LINK
        memcpy(PWR_pwBm(pwRoot, wr_pwr(*pwRoot)), pwBm,
               N_WORDS_SWITCH_BM * sizeof(Word_t));
#endif // BM_IN_LINK
        DBGI(printf("PWR_wPopCnt %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
#if defined(B_JUDYL) && defined(EMBED_KEYS)
        Link_t *pNewLinks = pwr_pLinks((BmSwitch_t *)*pwRoot);
        Word_t *pNewValues = (Word_t*)&pNewLinks[nLinkCnt + 1];
        Link_t *pOldLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t *pOldValues = (Word_t*)&pOldLinks[nLinkCnt];
        Word_t ww;
        for (ww = 0; ww < wIndex; ++ww) {
            pNewValues[ww] = pOldValues[ww];
        }
        for (++ww; ww <= (Word_t)nLinkCnt; ++ww) {
            pNewValues[ww] = pOldValues[ww - 1];
        }
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)

        // Initialize the new link.
        DBGI(printf("pLinks %p\n",
                    (void *)pwr_pLinks((BmSwitch_t *)*pwRoot)));
        // initialize new link in new switch
        if (cbEmbeddedBitmap && ((nBLR - nBW) <= cnLogBitsPerLink)) {
            memset(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex], 0,
                               sizeof(Link_t));
        } else {
            pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex].ln_wRoot = WROOT_NULL;
        }
        DBGI(printf("PWR_wPopCnt A %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // copy trailing links from old switch to new switch
        memcpy(&pwr_pLinks((BmSwitch_t *)*pwRoot)[wIndex + 1],
               &pwr_pLinks((BmSwitch_t *)pwr)[wIndex],
            (nLinkCnt - wIndex) * sizeof(Link_t));

        DBGI(printf("PWR_wPopCnt B %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        // Set the bit in the bitmap indicating that the new link exists.
        // SetBitInSwBmWord
        int nn = gnWordNumInSwBm(wDigit);
        PWR_pwBm(pwRoot, wr_pwr(*pwRoot))[nn] |= gwBitMaskInSwBmWord(wDigit);
  #ifdef OFFSET_IN_SW_BM_WORD
        // UpdateOffsetsInSwBmWords
        while (++nn < N_WORDS_SWITCH_BM) {
            PWR_pwBm(pwRoot, wr_pwr(*pwRoot))[nn]
                += (Word_t)1 << (cnBitsPerWord / 2);
        }
  #endif // OFFSET_IN_SW_BM_WORD
        DBGI(printf("PWR_wPopCnt %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));

        // Update the type field in *pwRoot if necessary.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #if defined(RETYPE_FULL_BM_SW)
        if ((Word_t)nLinkCnt == EXP(nBW) - 1) {
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
// BUG: How does OldSwitch know there is a bitmap before pwr?
printf("retyping full bmsw\n");
            set_wr_nType(*pwRoot,
                         (wr_nType(wRoot) == T_SKIP_TO_BM_SW)
                             ? T_SKIP_TO_SWITCH : T_SWITCH);
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_NON_BM_SW)
            // Conserve precious type values if possible.
// BUG: How does OldSwitch know there is a bitmap before pwr?
printf("retyping full bmsw\n");
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

        // &wRoot won't cut it for BM_IN_LINK. Really?
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

    int nBW;
#if defined(USE_XX_SW)
    if (tp_bIsXxSw(wr_nType(*pwRoot))) {
        nBW = pwr_nBW(pwRoot);
    } else
#endif // defined(USE_XX_SW)
    { nBW = nBL_to_nBW(nBL); }

    Word_t wLinks = EXP(nBW);

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
                for (int nn = 0; nn < N_WORDS_SWITCH_BM; nn++) {
                    wLinks
                        += __builtin_popcountll(
                            PWR_pwBm(pwRoot, pwr)[nn]
  #if defined(OFFSET_IN_SW_BM_WORD)
                                & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
  #endif // defined(OFFSET_IN_SW_BM_WORD)
                                                );
                }
                assert(wLinks <= EXP(nBL_to_nBW(nBL)));
                // Now we know how many links were in the old switch.
            }
        } else {
            wLinks = nLinks;
        }
    }
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

    Word_t wBytes =
#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
        bBmSw ? sizeof(BmSwitch_t)
                  + ALIGN_UP(N_WORDS_SWITCH_BM * sizeof(Word_t),
                             cnMallocAlignment)
              :
#endif // defined(CODE_BM_SW) && !defined(BM_IN_LINK)
#if defined(USE_LIST_SW)
        ((nType == T_LIST_SW) || (nType == T_SKIP_TO_LIST_SW))
            ? sizeof(ListSw_t) :
#endif // defined(USE_LIST_SW)
        sizeof(Switch_t);
    wBytes += wLinks * sizeof(Link_t);
    Word_t wWords = wBytes / sizeof(Word_t);
#if defined(B_JUDYL) && defined(EMBED_KEYS)
    wWords += wLinks; // Embedded Values in Switch
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)

#ifdef RAMMETRICS
    Word_t *pwAllocWords =  // RAMMETRICS
#if defined(CODE_BM_SW)
        bBmSw ? &j__AllocWordsJBB :  // bitmap branch
#endif // defined(CODE_BM_SW)
                &j__AllocWordsJBU ;  // uncompressed branch
#endif // RAMMETRICS

    DBGR(printf("\nOldSwitch nBL %d nBLU %d wWords %" _fw"d " OWx"\n",
         nBL, nBLUp, wWords, wWords));

#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
    if (bBmSw) {
        pwr -= ALIGN_UP(N_WORDS_SWITCH_BM,
                        cnMallocAlignment >> cnLogBytesPerWord);
    }
#endif // defined(CODE_BM_SW) && !defined(BM_IN_LINK)

#if defined(CODE_BM_SW) && defined(CACHE_ALIGN_BM_SW)
    MyFreeGuts(pwr, wWords, bBmSw ? 6 : cnBitsMallocMask, pwAllocWords);
#else // CACHE_ALIGN_BM_SW
    MyFree(pwr, wWords, pwAllocWords);
#endif // CACHE_ALIGN_BM_SW

    return wWords * sizeof(Word_t);

    (void)nBL; // silence compiler
    (void)nBLUp; // silence compiler
}

// Get the pop count of the tree/subtree represented by (*pwRoot, nBL).
// GetPopCnt requires nBL < cnBitsPerWord if pop in link.
static Word_t
GetPopCnt(Word_t *pwRoot, int nBL)
{
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
#ifdef USE_XX_SW_ONLY_AT_DL2
        assert(nBL == cnLogBitsPerLink);
#endif // USE_XX_SW_ONLY_AT_DL2
        if (nBL <= cnLogBitsPerWord) {
            return __builtin_popcountll(*pwRoot);
        }
        int wPopCnt = 0;
        for (int i = 0; i < (int)EXP(nBL - cnLogBitsPerWord); ++i) {
            wPopCnt += __builtin_popcountll(pwRoot[i]);
        }
        return wPopCnt;
    }

    int nBLR = GetBLR(pwRoot, nBL); // handles skip -- or not

#if defined(NO_TYPE_IN_XX_SW)
    if (nBLR < nDL_to_nBL(2)) { return wr_nPopCnt(*pwRoot, nBL); }
#endif // defined(NO_TYPE_IN_XX_SW)

    int nType = wr_nType(*pwRoot);

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if ((nBL == cnBitsPerWord) || (nType == T_EMBEDDED_KEYS))
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    {
        // PWR_wPopCntBL handles only switches if !PP_IN_LINK.
        // Would be nice to fix it.
        // But we have to handle the other types at the top anyway.
        if (*pwRoot == WROOT_NULL) { return 0; }
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
  #ifdef BITMAP
        if ((nType == T_BITMAP)
      #if defined(SKIP_TO_BITMAP)
            || (nType == T_SKIP_TO_BITMAP)
      #endif // defined(SKIP_TO_BITMAP)
            || 0)
        {
            Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
            Word_t wRoot = *pwRoot; (void)wRoot;
            Word_t *pwr = wr_pwr(wRoot);
            Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR);
            return (wPopCnt == 0) ? EXP(nBLR) : wPopCnt ;
        }
  #endif // BITMAP
        assert(tp_bIsSwitch(nType));
    }

  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    assert(nBL < cnBitsPerWord);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

// Sum up the pop count.
// It assumes *pwRoot is in a link to a switch.
// nBLUp is the bits left to decode after getting to pwRoot.
// if *pwRoot is in a skip link, then nBLUp must be adjusted by the skip
// amount to get the digits left at the next node.
static Word_t
Sum(Word_t *pwRoot, int nBLUp)
{
    Word_t *pwr = wr_pwr(*pwRoot);
    int nType = wr_nType(*pwRoot); (void)nType;
    assert(tp_bIsSwitch(nType));

#if defined(CODE_BM_SW) && defined(BM_IN_LINK)
    assert( ! tp_bIsBmSw(nType) || (nBLUp != cnBitsPerWord) );
#endif // defined(CODE_BM_SW) && defined(BM_IN_LINK)

    int nBL = GetBLR(pwRoot, nBLUp);

    Link_t *pLinks =
#if defined(CODE_BM_SW)
        tp_bIsBmSw(nType)
            ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
              pwr_pLinks((  Switch_t *)pwr) ;

    int nBW;
#if defined(CODE_XX_SW)
    if (tp_bIsXxSw(nType)) {
        nBW = pwr_nBW(pwRoot);
    } else
#endif // defined(CODE_XX_SW)
    { nBW = nBL_to_nBW(nBL); }

    Word_t wPopCnt = 0;
    Word_t xx = 0;
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
#if defined(CODE_BM_SW)
        if ( ! tp_bIsBmSw(nType)
                    || (PWR_pwBm(pwRoot, pwr)[gnWordNumInSwBm(nn)]
                        & gwBitMaskInSwBmWord(nn))
            )
#endif // defined(CODE_BM_SW)
        {
            wPopCnt += GetPopCnt(&pLinks[xx++].ln_wRoot, nBL - nBW);
        }
    }

    return wPopCnt;
}

#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

static Word_t
FreeArrayGuts(Word_t *pwRoot, Word_t wPrefix, int nBL, int bDump
#if defined(B_JUDYL) && defined(EMBED_KEYS)
            , Word_t *pwrUp, int nBWUp
  #ifdef CODE_BM_SW
            , int nTypeUp
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
              )
{
    Word_t *pwRootArg = pwRoot;
#if defined(BM_IN_LINK) || defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    int nBLArg = nBL;
#endif // defined(BM_IN_LINK) || defined(PP_IN_LINK) || ...
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot;
    // nType is not valid for NO_TYPE_IN_XX_SW if nBL <= nDL_to_nBL(2)
    int nType = wr_nType(wRoot); (void)nType; // silence gcc
    // pwr is not valid for NO_TYPE_IN_XX_SW if nBL <= nDL_to_nBL(2)
    Word_t *pwr = wr_pwr(wRoot);
    int nBW;
    Link_t *pLinks;
    Word_t wBytes = 0;

    assert((wRoot != 0) || (WROOT_NULL == 0));

#if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1);
#endif // ! defined(USE_XX_SW)

    if (!bDump) { DBGR(printf("FreeArrayGuts")); }

    if (bDump) {
        printf(" nBL %2d", nBL);
        // Check for zeros in suffix?  Print dots for suffix?
        // How would we represent a partially significant hex digit?
        printf(" wPrefix " OWx, wPrefix);
        printf(" pwRoot " OWx, (Word_t)pwRoot);
        printf(" wRoot " OWx, wRoot);
    }

    // Check for embedded bitmap before assuming nType is valid.
    if (cbEmbeddedBitmap && (nBL == cnBitsInD1)) {
        if (bDump) {
            if (nBL > cnLogBitsPerWord) {
                printf(" nWords %4" _fw"d", EXP(nBL - cnLogBitsPerWord));
                for (Word_t ww = 0; ww < EXP(nBL - cnLogBitsPerWord); ++ww) {
                    if ((ww % 8) == 0) { printf("\n"); }
                    printf(" " OWx,
                           ((Word_t*)STRUCT_OF(pwRoot, Link_t, ln_wRoot))[ww]);
                }
            }
            printf("\n");
        }
        return 0;
    }

    if (wRoot == WROOT_NULL) {
        if (bDump) { printf(" WROOT_NULL\n"); }
        return 0;
    }

#if defined(NO_TYPE_IN_XX_SW)
    assert(WROOT_NULL == ZERO_POP_MAGIC);
    if (nBL < nDL_to_nBL(2)) {
  #if defined(HANDLE_BLOWOUTS)
        if ((wRoot & BLOWOUT_MASK(nBL)) == ZERO_POP_MAGIC) {
            if (bDump) { printf(" BLOWOUT\n"); }
            assert(0); // not yet
        }
  #endif // defined(HANDLE_BLOWOUTS)
        goto embeddedKeys;
    }
#endif // defined(NO_TYPE_IN_XX_SW)

#if defined(SKIP_TO_BITMAP)
    if (nType == T_SKIP_TO_BITMAP) {
        if (bDump) { printf(" SKIP_TO_BITMAP"); }
        int nBLR = GetBLR(pwRoot, nBL);
        Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR);
        if (bDump) {
            printf(" nBLR %2d", nBLR);
            printf(" wPrefixPop " OWx, *(pwr + EXP(nBLR - cnLogBitsPerWord)));
            if (wPopCnt == 0) {
                wPopCnt = EXP(nBLR);
            }
            printf(" w_wPopCnt %" _fw"d", wPopCnt);
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

        // wPopCntTotal, zeroLink
        assert((nBL != cnBitsPerWord) || (wBytes == 0));
        assert((nBL != cnBitsPerWord) || (wr_pwr(*pwRootArg) == pwr));
        wBytes += OldBitmap(pwr, nBLR, wPopCnt);
        // Zero *pwRoot. What about the rest of the link?
        // nType == 0 && pwr == NULL should be good enough, no?
        *pwRoot = WROOT_NULL;

        return wBytes;
    }
#endif // defined(SKIP_TO_BITMAP)

#ifdef BITMAP
    if (nType == T_BITMAP) {
        if (bDump) { printf(" BITMAP"); }
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        if (bDump) {
            assert(nBLArg != cnBitsPerWord);

            if (cnBitsInD1 > cnLogBitsPerLink) {
                Word_t wPopCnt = gwBitmapPopCnt(qy, nBL);
                printf(" wr_wPopCnt %3" _fw"u", wPopCnt);
            }
        }
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(PP_IN_LINK)
        if (bDump) {
            assert(nBLArg != cnBitsPerWord);

            if (cnBitsInD1 > cnLogBitsPerLink) {
                printf(" wr_wPrefix " OWx,
                       PWR_wPrefixBL(pwRoot, (Switch_t *)NULL, nBL));
            }
        }
#endif // defined(PP_IN_LINK)

        // If the bitmap is not embedded, then we have more work to do.
        // The test can be done at compile time and will make one the
        // other clauses go away.
        if (!bDump) {
            assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
            Word_t wPopCnt = gwBitmapPopCnt(qy, nBL);
            wBytes = OldBitmap(pwr, nBL, wPopCnt);
            *pwRoot = WROOT_NULL;
            return wBytes;
        }

        int nWords
            = (nBL <= cnLogBitsPerWord) ? 1 : EXP(nBL - cnLogBitsPerWord);
        BmLeaf_t *pBmLeaf = (BmLeaf_t*)pwr;
        printf(" nWords %4d", nWords);
        printf(" wPopCnt %5zd", gwBitmapPopCnt(qy, nBL));
        Word_t wPopCntL = 0;
        for (Word_t ww = 0; (int)ww < nWords; ++ww) {
            wPopCntL += __builtin_popcountll(pBmLeaf->bmlf_awBitmap[ww]);
            if ((ww != 0) && (ww % 4) == 0) {
                printf(" %5zd", wPopCntL);
                wPopCntL = 0;
            }
            if ((ww % 8) == 0) {
                printf("\n");
            }
            printf(" 0x%016zx", pBmLeaf->bmlf_awBitmap[ww]);
        }
  #ifdef B_JUDYL
        printf("\n Values\n");
        for (int ww = 0;
             ww < (int)(
      #if (cnBitsPerWord > 32)
                        (wRoot & EXP(cnLsbBmUncompressed)) ? EXP(nBL) :
      #endif // (cnBitsPerWord > 32)
                            gwBitmapPopCnt(qy, nBL));
             ++ww)
        {
            if ((ww != 0) && (ww % 4) == 0) {
                printf("\n");
            }
            printf(" 0x%016zx", gpwBitmapValues(qy, nBL)[ww]);
        }
        printf("\n");
  #endif // B_JUDYL

        assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
        return 0;
    }
#endif // BITMAP

#if defined(SKIP_LINKS) // || (cwListPopCntMax != 0)
    assert( ! tp_bIsSkip(nType) || ((int)wr_nBL(wRoot) < nBL) );
    assert( ! tp_bIsSkip(nType) || (wr_nBL(wRoot) >= cnBitsInD1) );
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

#if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

#if defined(EMBED_KEYS)

        if (nType == T_EMBEDDED_KEYS) {
            goto embeddedKeys;
embeddedKeys:;
            if (!bDump) { return 0; }

            printf(" EMBEDDED_KEYS");
            wPopCnt = wr_nPopCnt(wRoot, nBL);
            assert(wPopCnt != 0);
            printf(" wr_nPopCnt %3d", (int)wPopCnt);

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
                    printf(" 0x%016" _fw"x",
#if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot
                            >> (cnBitsPerWord
                                - ((nn + nPopCntMax - wPopCnt + 1) * nBL)))
#else // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                        (wRoot >> (cnBitsPerWord - ((nn + 1) * nBL)))
#endif // defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(PACK_KEYS_RIGHT)
                            & MSK(nBL));
#ifdef B_JUDYL
                    int nDigitX = (wPrefix >> nBL) & MSK(nBWUp);
  #ifdef CODE_BM_SW
                    if (tp_bIsBmSw(nTypeUp)) {
      #ifdef BM_SW_FOR_REAL
                        printf(",n/a");
      #else // BM_SW_FOR_REAL
                        printf(",0x%zx",
                               ((Word_t*)&pwr_pLinks((BmSwitch_t*)pwrUp)
                                   [1<<nBWUp])[nDigitX]);
      #endif // else BM_SW_FOR_REAL
                    } else
  #endif // CODE_BM_SW
                    printf(",0x%zx",
                           ((Word_t*)&pwr_pLinks((Switch_t*)pwrUp)
                               [1<<nBWUp])[nDigitX]);
#endif // B_JUDYL
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
                printf("\nnType %d\n", nType);
            }
            assert( (nType == T_LIST)
  #if defined(UA_PARALLEL_128)
                   || (nType == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                   );
#endif // defined(DEBUG)

            assert((wRoot != 0) || (WROOT_NULL == 0));
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            if (nBL != cnBitsPerWord) {
                wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL);
            } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            { wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL); }
            assert(wPopCnt != 0);

            if (!bDump)
            {
                wBytes = OldList(pwr, wPopCnt, nBL, nType);
                assert(wr_pwr(*pwRootArg) == pwr);
                goto zeroLink;
            }

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            if (nBLArg < cnBitsPerWord) {
                printf(" ln_wPopCnt %3" _fw"u", wPopCnt);
            } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            { printf(" ls_wPopCnt %3" _fw"u", wPopCnt); }

            printf(" ln_wPrefix ");
#if defined(PP_IN_LINK)
            if (nBLArg < cnBitsPerWord) {
                printf( OWx, PWR_wPrefixBL(pwRoot, NULL, nBL));
            } else
#endif // defined(PP_IN_LINK)
            { printf("       N/A"); }

            for (int nn = 0;
                //(nn < wPopCnt) && (nn < 8);
                (nn < (int)wPopCnt);
                 nn++)
            {
                int xx = nn;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                xx += ((nBLArg == cnBitsPerWord) && (cnDummiesInList == 0));
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(COMPRESSED_LISTS)
                if (nBL <= 8) {
                    printf(" %02x", ls_pcKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                    printf("," OWx,
                           ((Word_t*)ls_pcKeysNATX(pwr, wPopCnt))[~xx]);
  #endif // B_JUDYL
                } else if (nBL <= 16) {
                    printf(" %04x", ls_psKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                    printf("," OWx,
                           ((Word_t*)ls_psKeysNATX(pwr, wPopCnt))[~xx]);

  #endif // B_JUDYL
#if (cnBitsPerWord > 32)
                } else if (nBL <= 32) {
                    printf(" %08x", ls_piKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                    printf("," OWx,
                           ((Word_t*)ls_piKeysNATX(pwr, wPopCnt))[~xx]);
  #endif // B_JUDYL
#endif // (cnBitsPerWord > 32)
                } else
#endif // defined(COMPRESSED_LISTS)
                {
                    printf(" " OWx, ls_pwKeysX(pwr, nBL, wPopCnt)[xx]);
  #ifdef B_JUDYL
                    printf("," OWx, gpwValues(qy)[~xx]);
  #endif // B_JUDYL
                }
            }
  #ifdef PSPLIT_PARALLEL
            if (ALIGN_LIST_LEN(ExtListBytesPerKey(nBL))) {
                for (int nn = (int)wPopCnt;
                     nn * ExtListBytesPerKey(nBL) % sizeof(Bucket_t);
                     ++nn)
                {
                    int xx = nn;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                    xx += ((nBLArg == cnBitsPerWord)
                        && (cnDummiesInList == 0));
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(COMPRESSED_LISTS)
                    if (nBL <= 8) {
                        printf(" %02x", ls_pcKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ((Word_t*)ls_pcKeysNATX(pwr,
                                                           wPopCnt))[~nn]);
                        }
  #endif // B_JUDYL
                    } else if (nBL <= 16) {
                        printf(" %04x", ls_psKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ((Word_t*)ls_psKeysNATX(pwr,
                                                           wPopCnt))[~nn]);
                        }

  #endif // B_JUDYL
#if (cnBitsPerWord > 32)
                    } else if (nBL <= 32) {
                        printf(" %08x", ls_piKeysNATX(pwr, wPopCnt)[xx]);
  #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ((Word_t*)ls_piKeysNATX(pwr,
                                                           wPopCnt))[~nn]);
                        }
  #endif // B_JUDYL
#endif // (cnBitsPerWord > 32)
                    } else
#endif // defined(COMPRESSED_LISTS)
                    {
                        printf(" " OWx, ls_pwKeysX(pwr, nBL, wPopCnt)[xx]);
  #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ls_pwKeysX(pwr, nBL, wPopCnt)[~nn]);
                        }
  #endif // B_JUDYL
                    }
                }
            }
  #endif // PSPLIT_PARALLEL
  #if defined(UA_PARALLEL_128)
            if (nType == T_LIST_UA) {
                assert(nBL == 16);
                assert(wPopCnt <= 6);
                assert(cnBitsPerWord == 32);
                assert(cnBitsMallocMask >= 4);
                assert(ListWordCnt(wPopCnt, nBL) == 3);
                //printf("\nT_LIST_UA pwr %p\n", (void*)pwr);
                //HexDump(/* str */ "", /* pw */ &pwr[-1], /* nWords */ 5);
            }
  #endif // defined(UA_PARALLEL_128)
            if (nBL == cnBitsPerWord) {
                //printf(" " OWx, ls_pwKeysX(pwr, nBL, wPopCnt)[wPopCnt]);
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
        nBW = pwr_nBW(pwRoot);
    } else
#endif // defined(CODE_XX_SW)
    { nBW = nBL_to_nBW(nBL); }

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
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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
            printf(" wr_nBW %2d", nBW);
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
                for (int nn = 0; nn < N_WORDS_SWITCH_BM; nn++) {
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

    nBL -= nBW;

    for (Word_t ww = 0, nn = 0; nn < EXP(nBW); nn++)
    {
#if defined(CODE_BM_SW)
  #if defined(BM_IN_LINK)
        assert( ! bBmSw || (nBLArg != cnBitsPerWord));
  #endif // defined(BM_IN_LINK)
      #if defined(B_JUDYL) && defined(EMBED_KEYS)
        int nLinks = 0;
        if (bBmSw) {
            nLinks = BmSwLinkCnt(qy);
        }
      #endif // defined(B_JUDYL) && defined(EMBED_KEYS)
        int nBmWordNum = gnWordNumInSwBm(nn);
        Word_t wBmBitMask = gwBitMaskInSwBmWord(nn);
        if ( ! bBmSw || (PWR_pwBm(pwRoot, pwr)[nBmWordNum] & wBmBitMask) )
#endif // defined(CODE_BM_SW)
        {
            if ((cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink))
                || (pLinks[ww].ln_wRoot != WROOT_NULL))
            {
#if defined(B_JUDYL) && defined(EMBED_KEYS)
  #if defined(CODE_BM_SW)
                // We don't pass enough info to FreeArrayGuts yet for it to
                // be able to handle printing the values in this case.
                // So we handle it here. Yuck.
                if (bDump
                      && bBmSw
                      && (wr_nType(pLinks[ww].ln_wRoot) == T_EMBEDDED_KEYS))
                {
                    printf(" nBL %2d", nBL);
                    printf(" wPrefix " OWx, wPrefix | (nn << nBL));
                    printf(" pwRoot " OWx, (Word_t)&pLinks[ww].ln_wRoot);
                    printf(" wr " OWx, pLinks[ww].ln_wRoot);
                    printf(" 0x%016" _fw"x",
                           (pLinks[ww].ln_wRoot
                                  >> (cnBitsPerWord - nBL))
                               & MSK(nBL));
                    printf(",0x%zx",
                           ((Word_t*)&pLinks[nLinks])[ww]);
                    printf("\n");
                } else
  #endif // defined(CODE_BM_SW)
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                wBytes += FreeArrayGuts(&pLinks[ww].ln_wRoot,
                                        wPrefix | (nn << nBL), nBL, bDump
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                                      , /*pwrUp*/ pwr, /*nBWUp*/ nBW
  #ifdef CODE_BM_SW
                                      , nType
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                                        );
            }

            ww++;
        }
    }

    if (bDump) return 0;

    // Someone has to clear PP and BM if PP_IN_LINK and BM_IN_LINK.
    // OldSwitch looks at BM.

    wBytes += OldSwitch(pwRootArg, nBL + nBW,
#if defined(CODE_BM_SW)
                        bBmSw, /* nLinks */ 0,
#endif // defined(CODE_BM_SW)
                        nBLPrev);
    }

zeroLink:

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK) || defined(BM_IN_LINK)
    if (nBLArg == cnBitsPerWord) {
        *pwRootArg = 0;
    } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK) || ...
    {
        *pwRootArg = (nBL != cnBitsPerWord) ? WROOT_NULL : 0;
    }

    return wBytes;
}

#if defined(DEBUG)
void
Dump(Word_t *pwRoot, Word_t wPrefix, int nBL)
{
    if (bHitDebugThreshold) {
        printf("# Dump\n");
        FreeArrayGuts(pwRoot, wPrefix, nBL, /* bDump */ 1
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                    , /*pwrUp*/ NULL, /*nBW*/ 0
  #ifdef CODE_BM_SW
                    , /* nTypeUp */ -1
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                      );
        printf("# End Dump\n");
    }
}
#endif // defined(DEBUG)

// Dump the path to the subtree rooted at (nBL, wPrefix)
// and the entire subtree below.
void
#ifdef B_JUDYL
JudyLDump(Word_t wRoot, int nBL, Word_t wPrefix)
#else // B_JUDYL
Judy1Dump(Word_t wRoot, int nBL, Word_t wPrefix)
#endif // B_JUDYL
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
    int nPopCnt
        = ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
                && (*pwRoot == WROOT_NULL))
            ? 0 : wr_nPopCnt(*pwRoot, nBL);
    assert(nPopCnt < nPopCntMax);
    DBGI(printf("\nInsertEmbedded: wRoot " OWx" nBL %d wKey " OWx
                    " nPopCnt %d Max %d\n",
                *pwRoot, nBL, wKey, nPopCnt, nPopCntMax));
#if defined(NO_TYPE_IN_XX_SW)
    // clear out ZERO_POP_MAGIC
    if ((nPopCnt == 0) && (nBL < nDL_to_nBL(2))) { *pwRoot = 0; }
#endif // defined(NO_TYPE_IN_XX_SW)
    if ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS) && (nPopCnt == 0)) {
        *pwRoot = 0;
        set_wr_nType(*pwRoot, T_EMBEDDED_KEYS);
    }
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

// CopyWithInsert may operate on a list that is installed and one that is not.
// What is our general rule w.r.t. qp?
// Should we add qpOld? What about qpUp?
// Since InsertGuts may want to use Insert to move things from old to new it
// makes sense to install the new and update qy even if it means using a new
// local variable, qyOld.
// CopyWithInsert can handle pTgt == pSrc, but cannot handle any other
// overlapping buffer scenarios.
#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
CopyWithInsertWord(qp, Word_t *pSrc,
                   int nKeys, // number of keys excluding the new one
                   Word_t wKey, int nPos)
{
    DBGI(Log(qy, "CopyWithInsertWord"));
    qv;
    DBGI(printf("\nCopyWithInsertWord(pSrc %p nKeys %d wKey " OWx")\n",
                (void *)pSrc, nKeys, wKey));
    Word_t *pTgt = ls_pwKeysX(pwr, nBL, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtValues = gpwValues(qy);
    Word_t *pwSrcValues = pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcValues -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
    int n;

    // Why don't we know nPos for inflated embedded list?
    // We should enhance Insert to use LocateKey for embedded keys so we
    // don't need to search here.
    if ((nPos == -1) // inflated embedded list
#if ! defined(EMBED_KEYS)
            && (nKeys != 0)
#else // ! defined(EMBED_KEYS)
            && 1 // avoid extraneous parens error
#endif // ! defined(EMBED_KEYS)
        )
    {
        // find the insertion point
        n = ~SearchListWord(pSrc, wKey, cnBitsPerWord, nKeys);
    } else { n = nPos; }
    assert(nPos <= nKeys);

    if (pTgt != pSrc) {
#ifdef B_JUDYL
        // copy the values tail
        COPY(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
        // copy the values head
        COPY(&pwTgtValues[-n    ], &pwSrcValues[-n    ], n        );
#endif // B_JUDYL
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    } else {
#ifdef B_JUDYL
        // move the values tail
        MOVE(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
#endif // B_JUDYL
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = wKey; // insert the key

#ifdef B_JUDYL
    // I think we should change this a bit.
    // A couple of options:
    // - This function returns n and caller figures out pwValue.
    // - Add qp parameters to this function so it can legitimately
    //   figure out pwValue.
    Word_t *pwValue = &pwTgtValues[~n];
#endif // B_JUDYL

    n = nKeys + 1;
#if defined(PARALLEL_SEARCH_WORD)
    // Pad the list with copies of the last real key in the list so the
    // length of the list from the first key through the last copy of the
    // last real key is an integral multiple of cnBytesListLenAlign.
    // cnBytesListLenAlign is set to the size of a parallel search bucket.
    // This way we don't need any special handling in the parallel search
    // code to handle a partial final bucket.
    for (; (n * sizeof(wKey)) % sizeof(Bucket_t); ++n) {
        pTgt[n] = pTgt[n-1];
    }
#endif // defined(PARALLEL_SEARCH_WORD)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)

#ifdef B_JUDYL
    DBGI(printf("CopyWithInsertWord returning %p\n", (void*)pwValue));
    return pwValue;
#endif // B_JUDYL
}

#if defined(COMPRESSED_LISTS)

#if (cnBitsPerWord > 32)
#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
CopyWithInsert32(qp, uint32_t *pSrc,
                 int nKeys, // number of keys excluding the new one
                 uint32_t iKey, int nPos)
{
    DBGI(Log(qy, "CopyWithInsert32"));
    qv;
    uint32_t *pTgt = ls_piKeysX(pwr, nBL, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtValues = gpwValues(qy);
    Word_t *pwSrcValues = (Word_t*)pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcValues -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
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
        n = ~PsplitSearchByKey32(pSrc, nKeys, iKey, 0);
    } else { n = nPos; }
    assert(nPos <= nKeys);

    if (pTgt != pSrc) {
#ifdef B_JUDYL
        // copy the values tail
        COPY(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
        // copy the values head
        COPY(&pwTgtValues[-n    ], &pwSrcValues[-n    ], n        );
#endif // B_JUDYL
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    } else {
#ifdef B_JUDYL
        // move the values tail
        MOVE(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
#endif // B_JUDYL
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = iKey; // insert the key

#ifdef B_JUDYL
    Word_t *pwValue = &pwTgtValues[~n];
#endif // B_JUDYL

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    // See CopyWithInsertWord and CopyWithInsert8 for comment.
    for (; (n * sizeof(iKey)) % sizeof(Bucket_t); ++n) {
        pTgt[n] = pTgt[n-1];
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)

#ifdef B_JUDYL
    return pwValue;
#endif // B_JUDYL
}
#endif // (cnBitsPerWord > 32)

#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
CopyWithInsert16(qp, uint16_t *pSrc,
                 int nKeys, // number of keys excluding the new one
                 uint16_t sKey, int nPos)
{
    DBGI(Log(qy, "CopyWithInsert16"));
    qv;
    uint16_t *pTgt = ls_psKeysX(pwr, nBL, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtValues = gpwValues(qy);
    Word_t *pwSrcValues = (Word_t*)pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcValues -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
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
    assert(nPos <= nKeys);

    if (pTgt != pSrc) {
#ifdef B_JUDYL
        // copy the values tail
        COPY(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
        // copy the values head
        COPY(&pwTgtValues[-n    ], &pwSrcValues[-n    ], n        );
#endif // B_JUDYL
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    } else {
#ifdef B_JUDYL
        // move the values tail
        MOVE(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
#endif // B_JUDYL
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = sKey; // insert the key

#ifdef B_JUDYL
    Word_t *pwValue = &pwTgtValues[~n];
#endif // B_JUDYL

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    // See CopyWithInsertWord for comment.
  #if defined(UA_PARALLEL_128)
    if ((nType == T_LIST_UA) && (n <= 6)) {
        for (; (n * sizeof(sKey)) % 12; ++n) {
            pTgt[n] = pTgt[n-1];
        }
    } else
  #endif // defined(UA_PARALLEL_128)
    {
        // See CopyWithInsertWord and CopyWithInsert8 for comment.
        for (; (n * sizeof(sKey)) % sizeof(Bucket_t); ++n) {
             pTgt[n] = pTgt[n-1];
        }
    }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)

#ifdef B_JUDYL
    return pwValue;
#endif // B_JUDYL
}

#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
CopyWithInsert8(qp, uint8_t *pSrc,
                int nKeys, // number of keys excluding the new one
                uint8_t cKey, int nPos)
{
    DBGI(Log(qy, "CopyWithInsert8"));
    qv;
    uint8_t *pTgt = ls_pcKeysX(pwr, nBL, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtValues = gpwValues(qy);
    Word_t *pwSrcValues = (Word_t*)pSrc; (void)pwSrcValues;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcValues -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
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
        n = ~PsplitSearchByKey8(pSrc, nKeys, cKey, 0);
    } else { n = nPos; }
    assert(nPos <= nKeys);

    if (pTgt != pSrc) {
#ifdef B_JUDYL
#if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBL != cnBitsInD1)
#endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // copy the values tail
            COPY(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
            // copy the values head
            COPY(&pwTgtValues[-n    ], &pwSrcValues[-n    ], n        );
        }
#endif // B_JUDYL
        COPY(pTgt, pSrc, n); // copy the head
        COPY(&pTgt[n+1], &pSrc[n], nKeys - n); // copy the tail
    }
    else
    {
#ifdef B_JUDYL
#if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBL != cnBitsInD1)
#endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // move the values tail
            MOVE(&pwTgtValues[~nKeys], &pwSrcValues[-nKeys], nKeys - n);
        }
#endif // B_JUDYL
        MOVE(&pTgt[n+1], &pSrc[n], nKeys - n); // move the tail
    }

    pTgt[n] = cKey; // insert the key

#ifdef B_JUDYL
    Word_t *pwValue;
#if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
    if (nBL == cnBitsInD1) {
        pwValue = &pwTgtValues[~(cKey & MSK(cnBitsInD1))];
    } else
#endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
    { pwValue = &pwTgtValues[~n]; }
#endif // B_JUDYL

    n = nKeys + 1;
#if defined(PSPLIT_PARALLEL)
    // Padding is redundant in some cases.
    // But avoiding it is probably more expensive than doing it.
    // This code assumes InflateEmbedded pads the list.
    //if ((pTgt != pSrc)
    //    || ((nKeys * sizeof(cKey) % sizeof(Bucket_t)) == 0)
    //    //|| 1
    //    || (nPos == nKeys))
    //{
        for (; (n * sizeof(cKey)) % sizeof(Bucket_t); ++n) {
            pTgt[n] = pTgt[n-1];
        }
    //}
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
    pTgt[n] = -1;
#endif // defined(LIST_END_MARKERS)

#ifdef B_JUDYL
    return pwValue;
#endif // B_JUDYL
}

#endif // defined(COMPRESSED_LISTS)
#endif // defined(SORT_LISTS)
#endif // (cwListPopCntMax != 0)

#ifdef B_JUDYL
Word_t *
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtBitmap(qp, Word_t wKey);

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)

static Word_t
DeflateExternalList(Word_t *pwRoot,
                    int nPopCnt, int nBL, Word_t *pwr
  #ifdef B_JUDYL
                  , Word_t *pwValueUp
  #endif // B_JUDYL
                    );

#endif // defined(EMBED_KEYS)

#endif // (cwListPopCntMax != 0)

static void
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
#if defined(B_JUDYL) && defined(EMBED_KEYS)
Word_t*
#else // defined(B_JUDYL) && defined(EMBED_KEYS)
void
#endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
InsertCleanup(qp, Word_t wKey)
{
    qv; (void)wKey;
#if defined(B_JUDYL) && defined(EMBED_KEYS)
    Word_t *pwValue = NULL;
#endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)

#if defined(CODE_BM_SW)
    if (tp_bIsBmSw(nType)) {
        // We should have a one-liner for this.
  #if defined(SKIP_LINKS)
        int nBLR = tp_bIsSkip(nType) ? gnBLR(qy) : nBL;
  #else // defined(SKIP_LINKS)
        int nBLR = nBL;
  #endif // defined(SKIP_LINKS)
        if (InflateBmSwTest(qy)) {
  #if defined(B_JUDYL) && defined(EMBED_KEYS)
            // InflateBmSw may change pwValue of all embedded keys.
            Word_t *pwValueRet
                = InflateBmSw(pwRoot, wKey, nBLR, /* nBLUp */ nBL);
            if (pwValueRet != NULL) {
                pwValue = pwValueRet;
            }
  #else // defined(B_JUDYL) && defined(EMBED_KEYS)
            InflateBmSw(pwRoot, wKey, nBLR, /* nBLUp */ nBL);
  #endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
        }
    }
#endif // defined(CODE_BM_SW)

#ifdef BITMAP
#if (cn2dBmMaxWpkPercent != 0) // conversion to big bitmap enabled
    int nDL = nBL_to_nDL(nBL);

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
        // Use PWR_wPopCntBL because we might get called with pop count zero
        // during InsertAll and we should be done converting before we ever
        // get called with full pop. See embedded assert.
        && ((wPopCnt = PWR_wPopCntBL(pwRoot, pwr, nBL)),
            assert(wPopCnt < MSK(nBL)),
            wPopCnt * cn2dBmMaxWpkPercent
                > (EXP(nBL - cnLogBitsPerWord) * 100)))
    {
        DBGI(printf("Converting BM leaf.\n"));

        // Can't disable this one by ALLOW_EMBEDDED_BITMAP.
        // The code doesn't work yet.
        assert((cnBitsInD1 > cnLogBitsPerLink) || !cbEmbeddedBitmap);
        //assert((cnBitsInD2 > cnLogBitsPerLink) || !cbEmbeddedBitmap);

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
            nBW = nBL_to_nBW(nBL);
        }

        // Allocate a new bitmap.
        DBGI(printf("# IC: NewBitmap nBL %d nBW %d wPopCnt %" _fw"d"
                        " wWordsAllocated %" _fw"d wPopCntTotal %" _fw"d.\n",
                    nBL, nBW, wPopCnt, wWordsAllocated, wPopCntTotal));
        Word_t *pwrNew = NewBitmap(qy, /* nBLNew */ nBL, wKey, wPopCnt);
// NewBitmap installs a new wRoot.
// nBL, pLn, pwRoot have not changed, but *pwRoot has.
        Word_t *pwBitmap = ((BmLeaf_t*)pwrNew)->bmlf_awBitmap;

        // Why are we not using InsertAll here to insert the keys?
        // It doesn't handle switches yet.

        int nBLLn = nBL - nBW;
        Word_t wBLM = MSK(nBLLn); // Bits left mask.

        for (Word_t ww = 0; ww < EXP(nBW); ww++)
        {
            Link_t *pLnLn =
#if defined(USE_LIST_SW)
                (nType == T_LIST_SW) ? &gpListSwLinks(qy)[ww] :
#endif // defined(USE_LIST_SW)
                &pwr_pLinks((Switch_t *)pwr)[ww];
            Word_t *pwRootLn = &pLnLn->ln_wRoot;
            Word_t wRootLn = *pwRootLn;
            // We have to pick WROOT_NULL so it cannot match a legal
            // wRoot of embedded keys for NO_TYPE_IN_XX_SW or else
            // we have to move this test and continue to after the
            // test for typeless embedded keys.
            if (wRootLn == WROOT_NULL) {
                continue;
            }
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
                continue;
            }
#endif // defined(EMBED_KEYS)
#ifdef BITMAP
            if (nTypeLn == T_BITMAP) {
                Word_t *pwBitmapLn = ((BmLeaf_t*)pwrLn)->bmlf_awBitmap;
                memcpy(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                       pwBitmapLn, EXP(nBLLn - 3));
                Word_t wPopCntLn = gwBitmapPopCnt(qyLn, nBLLn);
                OldBitmap(pwrLn, nBLLn, wPopCntLn);
                continue;
            }
#endif // BITMAP
#if (cwListPopCntMax != 0)
            {
#if defined(DEBUG)
                if (nTypeLn != T_LIST)
  #if defined(UA_PARALLEL_128)
                    if (nTypeLn != T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                    { printf("nTypeLn %d\n", nTypeLn); }
                assert((nTypeLn == T_LIST)
  #if defined(UA_PARALLEL_128)
                   || (nTypeLn == T_LIST_UA)
  #endif // defined(UA_PARALLEL_128)
                   );
#endif // defined(DEBUG)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                int nPopCntLn
                      = PWR_wPopCntBL(pwRootLn, (Switch_t *)pwrLn, nBLLn);
#else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                int nPopCntLn = PWR_xListPopCnt(pwRootLn, pwrLn, nBLLn);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#ifdef COMPRESSED_LISTS
                if (nBLLn <= 8) {
                    uint8_t *pcKeysLn = ls_pcKeysNATX(pwrLn, nPopCntLn);
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
                    uint16_t *psKeysLn = ls_psKeysNATX(pwrLn, nPopCntLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (psKeysLn[nn] & wBLM));
                    }
                }
  #if (cnBitsPerWord == 64)
                else {
                    assert(nBLLn <= 32);
                    uint32_t *piKeysLn = ls_piKeysNATX(pwrLn, nPopCntLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                               (piKeysLn[nn] & wBLM));
                    }
                }
  #endif // (cnBitsPerWord == 64)
#else // COMPRESSED_LISTS
                Word_t *pwKeysLn = ls_pwKeysNATX(pwrLn, nPopCntLn);
                for (int nn = 0; nn < nPopCntLn; nn++) {
                    SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                           (pwKeysLn[nn] & wBLM));
                }
#endif // COMPRESSED_LISTS
                assert(nPopCntLn != 0);
                OldList(pwrLn, nPopCntLn, nBLLn, T_LIST);
            }
#endif // (cwListPopCntMax != 0)
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
            printf("count %d wPopCnt %" _fw"d wKey %zx\n",
                   count, wPopCnt, wKey);
            Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        }
        assert(count == (int)wPopCnt);
#endif // defined(DEBUG)
    }
#endif // (cn2dBmMaxWpkPercent != 0)
#endif // BITMAP
#if defined(B_JUDYL) && defined(EMBED_KEYS)
    // InsertCleanup may or may not change pwValue.
    // It returns NULL for unchanged.
    DBGX(printf("InsertCleanup returning pwValue %p\n", pwValue));
    return pwValue;
#endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
}

#if (cwListPopCntMax != 0)

// Insert each key from pwRootOld into pwRoot.  Then free pwRootOld.
// wKey contains the common prefix.
static void
InsertAll(Word_t *pwRootOld, int nBLOld, Word_t wKey, Word_t *pwRoot, int nBL)
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot);
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
    if (wRootOld == WROOT_NULL) { return; }
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
        // wRootOld here, but new from IEL's perspective
#ifdef B_JUDYL
        assert(0); // No code for getting pwValueUp here yet.
#endif // B_JUDYL
        wRootOld = InflateEmbeddedList(pwRootOld, wKey, nBLOld, wRootOld
#ifdef B_JUDYL
                                     , /* pwValueUp */ NULL
#endif // B_JUDYL
                                       );
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
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBLOld < cnBitsPerWord) {
        // Adjust the count to compensate for pre-increment during insert.
        nPopCnt = PWR_wPopCntBL(pwRootOld, NULL, nBLOld) - 1;
    } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    {
        // wRootOld might be newer than *pwRootOld
        nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLOld);
    }

#ifdef B_JUDYL
    Word_t* pwVal;
#else // B_JUDYL
    int status = 0; // for debug
#endif // B_JUDYL
#if defined(COMPRESSED_LISTS)
    if (nBLOld <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
  #ifdef B_JUDYL
        Word_t* pwValues = (Word_t*)pcKeys;
      #ifdef LIST_POP_IN_PREAMBLE
        --pwValues;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
        for (int nn = 0; nn < nPopCnt; nn++) {
#ifdef B_JUDYL
            pwVal = Insert(nBL, pLn, pcKeys[nn] | (wKey & ~MSK(8)));
            *pwVal = pwValues[~nn];
#else // B_JUDYL
            status = Insert(nBL, pLn, pcKeys[nn] | (wKey & ~MSK(8)));
#endif // B_JUDYL
            DBGI(printf("Just after Insert in InsertAll(uint8_t)"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        }
    } else if (nBLOld <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
  #ifdef B_JUDYL
        Word_t* pwValues = (Word_t*)psKeys;
      #ifdef LIST_POP_IN_PREAMBLE
        --pwValues;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
        for (int nn = 0; nn < nPopCnt; nn++) {
#ifdef B_JUDYL
            pwVal = Insert(nBL, pLn, psKeys[nn] | (wKey & ~MSK(16)));
            *pwVal = pwValues[~nn];
#else // B_JUDYL
            status = Insert(nBL, pLn, psKeys[nn] | (wKey & ~MSK(16)));
#endif // B_JUDYL
            DBGI(printf("Just after Insert in InsertAll(uint16_t)"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        }
#if (cnBitsPerWord > 32)
    } else if (nBLOld <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
  #ifdef B_JUDYL
        Word_t* pwValues = (Word_t*)piKeys;
      #ifdef LIST_POP_IN_PREAMBLE
        --pwValues;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
        for (int nn = 0; nn < nPopCnt; nn++) {
#ifdef B_JUDYL
            pwVal = Insert(nBL, pLn, piKeys[nn] | (wKey & ~MSK(32)));
            *pwVal = pwValues[~nn];
#else // B_JUDYL
            status = Insert(nBL, pLn, piKeys[nn] | (wKey & ~MSK(32)));
#endif // B_JUDYL
            DBGI(printf("Just after Insert in InsertAll(uint32_t)"));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        }
#endif // (cnBitsPerWord > 32)
    } else
#else // defined(COMPRESSED_LISTS)
    (void)wKey;
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwrOld, nBL, nPopCnt);
  #ifdef B_JUDYL
        Word_t* pwValues = pwKeys;
      #ifdef LIST_POP_IN_PREAMBLE
        --pwValues;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
        for (int nn = 0; nn < nPopCnt; nn++) {
#ifdef B_JUDYL
            pwVal = Insert(nBL, pLn, pwKeys[nn]);
            *pwVal = pwValues[~nn];
#else // B_JUDYL
            status = Insert(nBL, pLn, pwKeys[nn]);
#endif // B_JUDYL
            DBGI(printf("\n# Just after Insert in InsertAll(Word_t) "));
            DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        }
    }
#ifndef B_JUDYL
    assert(status == 1);
    (void)status;
#endif // B_JUDYL

#if defined(NO_TYPE_IN_XX_SW)
    // OldList uses nType even if (nBL < nDL_to_nBL(2)) implies an
    // embedded list.
#endif // defined(NO_TYPE_IN_XX_SW)
    if (nPopCnt != 0) { OldList(pwrOld, nPopCnt, nBLOld, nType); }
}

#endif // (cwListPopCntMax != 0)

// Handle a prefix mismatch by inserting a switch above and demoting
// the current *pwRoot.
#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
InsertAtPrefixMismatch(qp, Word_t wKey, int nBLR)
{
    qv;
    int nDL = nBL_to_nDL(nBL); (void)nDL;
    int nDLR = nBL_to_nDL(nBLR); (void)nDLR;

    // Can't have a prefix mismatch if there is no skip.
    assert(nBLR < nBL);

    // Get wPrefix for qp.
    // For PP_IN_LINK, we'll use wPrefix to install the prefix for the
    // old *pwRoot in it's new location.
    Word_t wPrefix;
#if defined(PP_IN_LINK)
    if (nBL == cnBitsPerWord) { wPrefix = 0; /* limitation */ } else
#endif // defined(PP_IN_LINK)
#if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
        wPrefix = gwBitmapPrefix(qy, nBLR);
    } else
#endif // defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
    {
        //printf("pwRoot %p\n", (void *)pwRoot);
        wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
    }
    // Have wPrefix for qp.

    // Figure nBL for the new switch.

    // Determine the minimum nBL for the new switch, i.e. where the prefix
    // no longer matches.
    // After finding the minimum nBL we have to choose an (nBLNew, nBWNew)
    // such that nBLNew >= nBLMin >= nBLNew - nBWNew.
    assert(wPrefix != wKey);
    int nBLNew = LOG(wPrefix ^ wKey) + 1;
    // nBLNew is the highest order bit that is different changed into an nBL.
    int nDLNew = nBL_to_nDL(nBLNew);
    // nDLNew includes the highest order digit that is different.

    // Choose (nBLNew, nBWNew).
    nBLNew = nDL_to_nBL(nDLNew);
    int nBWNew = nDL_to_nBW(nDLNew);
    DBGX(printf("InsertAtPrefixMismatch choosing nBLNew %d nBwNew %d.\n",
                nBLNew, nBWNew));
    assert(nBLNew > nBLR);
    assert(nBLNew <= nBL);

    Word_t wPopCnt;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if !defined(NO_SKIP_AT_TOP)
    if (nBL == cnBitsPerWord) { wPopCnt = wPopCntTotal; /* hack */ } else
#endif // !defined(NO_SKIP_AT_TOP)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    {
#if defined(SKIP_TO_BITMAP)
#if !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
        if (Get_nType(pwRoot) == T_SKIP_TO_BITMAP) {
            wPopCnt = gwBitmapPopCnt(qy, nBLR);
        } else
#endif // !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
#endif // defined(SKIP_TO_BITMAP)
        {
            wPopCnt = PWR_wPopCntBL(pwRoot, (Switch_t *)pwr, nBLR);
            if (wPopCnt == 0) {
                // full pop
                wPopCnt = EXP(nBLR);
            }
        }
    }

#if defined(NO_SKIP_AT_TOP)
    // no skip link at top => no prefix mismatch at top
    assert(nBL < cnBitsPerWord);
#endif // defined(NO_SKIP_AT_TOP)

    // todo nBW; wide switch
    int nIndex = (wPrefix >> nDL_to_nBL_NAT(nDLNew - 1))
               & (EXP(nDL_to_nBW(nDLNew)) - 1);
    // nIndex is the logical index in new switch.
    // It may not be the same as the index in the old switch.

#if defined(CODE_BM_SW)

    // set bBmSwNew
  #if defined(USE_BM_SW)
      #if defined(SKIP_TO_BM_SW)
  #if defined(BM_IN_LINK)
    int bBmSwNew = (nDL != cnDigitsPerWord);
  #else // defined(BM_IN_LINK)
    int bBmSwNew = 1;
  #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
  #if defined(BM_IN_LINK)
    int bBmSwNew = ((nDL != cnDigitsPerWord) && (nDLNew == nDL));
  #else // defined(BM_IN_LINK)
    int bBmSwNew = (nDLNew == nDL);
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
    Word_t wIndexCnt = EXP(nDL_to_nBWNAT(nDLR));
    if (bBmSwOld)
    {
        // Save the old bitmap before it is trashed by NewSwitch.
        // Is it possible that nDL != cnDigitsPerWord and
        // we are at the top?
        if (nDL != cnDigitsPerWord)
        {
            memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL), sizeof(ln.ln_awBm));
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
    DBGI(printf("IG: nDLNew %d nDL %d\n", nDLNew, nDL));
    assert(nBLNew <= nBL);
    // NewSwitch changes *pwRoot (and the link containing it).
    // It does not change our local wRoot and pwr (or pwRoot).
    pwSw = NewSwitch(pwRoot, wPrefix, nBLNew,
#if defined(CODE_XX_SW)
                     nBWNew,
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
                     bBmSwNew ? T_BM_SW :
#endif // defined(CODE_BM_SW)
                     T_SWITCH,
                     nBL, wPopCnt);

#if defined(CODE_BM_SW)
    if (bBmSwNew)
    {
#if defined(BM_SW_FOR_REAL)
#if defined(BM_IN_LINK)
    if (nDL != cnDigitsPerWord)
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
        if (nDL != cnDigitsPerWord)
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
    if (nBLNew - nBLR == nBWNew) {
        Clr_bIsSkip(&wRoot); // Change type to the non-skip variant.
    }
  #endif // defined(SKIP_LINKS)
    // Copy wRoot from old link (after being updated) to new link.
    Link_t *pLinks =
  #if defined(CODE_BM_SW)
        bBmSwNew ? pwr_pLinks((BmSwitch_t *)pwSw) :
  #endif // defined(CODE_BM_SW)
                   pwr_pLinks((  Switch_t *)pwSw) ;
    pLinks[nIndex].ln_wRoot = wRoot;

    // Set the prefix and/or pop in the link in the new
    // switch if they are in the link.
#if defined(PP_IN_LINK)
  #if defined(NO_UNNECESSARY_PREFIX)
    if (nDLR < nDLNew - 1)
  #endif // defined(NO_UNNECESSARY_PREFIX)
    { set_PWR_wPrefix(&pLinks[nIndex].ln_wRoot, NULL, nDLR, wPrefix); }
#else // defined(PP_IN_LINK)
  #if defined(NO_UNNECESSARY_PREFIX)
    // We could go to the trouble of zeroing the no-longer necessary
    // prefix in the old switch. To what end?
  #endif // defined(NO_UNNECESSARY_PREFIX)
#endif // defined(PP_IN_LINK)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    set_PWR_wPopCnt(&pLinks[nIndex].ln_wRoot, NULL, nDLR, wPopCnt);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    DBGI(printf("Just before InsertAtPrefixMismatch calls Insert"
                    " for prefix mismatch.\n"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

#ifdef B_JUDYL
    return
#endif // B_JUDYL
        Insert(nBL, pLn, wKey);
}

#if defined(CODE_XX_SW)
  #ifdef B_JUDYL
static Word_t*
  #else // B_JUDYL
static void
  #endif // B_JUDYL
DoubleIt(qp,
         Word_t wKey
  #ifdef SKIP_TO_XX_SW
       , int nBLUp
  #endif // SKIP_TO_XX_SW
       , Link_t *pLnUp
  #ifdef CODE_XX_SW
       , Word_t wPopCnt
  #endif // CODE_XX_SW
         )
{
    qv; (void)pLnUp;
    int nDL = nBL_to_nDL(nBL);
    int nBLOld = nBL;
    int nDLOld = nDL; (void)nDLOld;
  #ifdef SKIP_TO_XX_SW
    (void)nBLUp;
  #endif // SKIP_TO_XX_SW
    (void)wPopCnt;
    int nBW;

    DBGI(printf("DoubleIt\n"));
    {
  #if defined(USE_XX_SW)
        {
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
            // parent is XX_SW; back up and replace it
            pLn = pLnUp;
            wRoot = pLn->ln_wRoot;
            pwRoot = &pLn->ln_wRoot;
            nType = wr_nType(wRoot);
            assert(tp_bIsXxSw(nType));
            pwr = wr_pwr(wRoot);
            // The only place we put XX_SW is nDL == 2.
            nDL = 2; // This is more accurately nDLR.
            nBL = nDL_to_nBL(nDL); // This is more accurately nBLR.
            // We now have a new qy. Partially.
            // nBL isn't quite right.
            // We should be updating nBL and nDL here.
            // But the code below isn't ready.
            // Except for the call to NewBitmap.
      #if defined(SKIP_TO_XX_SW)
            if (tp_bIsSkip(nType)) {
                nBLOld = nBLUp;
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
            assert(nBL > cnLogBitsPerLink);
            nBW += cnBWIncr;
            if (nBL - nBW <= cnLogBitsPerLink) {
// Doubling here would use at least as much memory as a big bitmap.
// Are we here because the list is full?
// Is it possible we are here because our words/key is good?
                DBGI(printf("# IG: NewBitmap nBL %d"
                              " nBLOld %d"
                              " wWordsAllocated %" _fw"d"
                              " wPopCntTotal %" _fw"d.\n",
                            nBL, nBLOld, wWordsAllocated, wPopCntTotal));
                DBGI(printf("# IG: NewBitmap nBL %d.\n", nBL));
#if ! defined(SKIP_TO_BITMAP)
                assert(nBL == nBLOld);
#endif // ! defined(SKIP_TO_BITMAP)
                // We have updated qy to qyUp. Except nBL is really nBLR.
                // And nBLOld has what should be in nBL.
                int nBLNew = nBL;
                nBL = nBLOld;
                NewBitmap(qy, nBLNew, wKey, /* wPopCnt */ 0);
                nBL = nBLNew;
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL, 0);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                DBGI(printf("# After NewBitmap; before insertAll.\n"));
                DBGI(Dump(pwRootLast,
                      /* wPrefix */ (Word_t)0, cnBitsPerWord));
                // We have updated qy to qyUp. Except nBL is really nBLR.
                // And nBLOld has what should be in nBL.
                goto insertAll;
            }
            DBGI(Dump(pwRootLast,
                      /* wPrefix */ (Word_t)0, cnBitsPerWord));
        }
  #else // defined(USE_XX_SW)
        { nBW = nBL_to_nBW(nBL); }
  #endif // defined(USE_XX_SW)

        // For XX_SW we have updated qy to qyUp. Except nBL is really nBLR.
        // And nBLOld has what should be in nBL.

#if defined(DEBUG)
        if (nBL > nBLOld) {
            printf("IG: pwRoot %p wKey " OWx" nBL %d wRoot " OWx"\n",
                   (void *)pwRoot, wKey, nBL, wRoot);
            printf("nBLOld %d\n", nBLOld);
        }
#endif // defined(DEBUG)
        assert(nBL <= nBLOld);

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // NewSwitch changes *pwRoot and the Link_t containing it.
        // We need to preserve the Link_t for subsequent InsertAll.
        // We don't have a whole link at the top.
        if (nBLOld < cnBitsPerWord) {
            link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
        }
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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
                  (nBLOld != cnBitsPerWord)
                      && (nBL == nBLOld) ? T_BM_SW : T_SWITCH,
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
            DBGI(printf("\n# DoubleIt After NewSwitch Dump\n"));
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
    if (pLn == pLnUp) {
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
// How are we going to get nBLOld from pLnUp?
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
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // InsertAll may look in the link containing wRoot for
        // pop count. That's why we preserved the contents of
        // the link before overwriting it above.
        if (nBLOld < cnBitsPerWord) {
            InsertAll(&link.ln_wRoot, nBLOld, wKey, pwRoot, nBLOld);
        } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        {
            // *pwRoot now points to a switch
            InsertAll(&wRoot, nBLOld, wKey, pwRoot, nBLOld);
        }
    }

    if (nBL == nBLOld) {
        DBGI(printf("Just Before DoubleIt calls final Insert"));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
    }

  #ifdef B_JUDYL
    Word_t *pwVal = Insert(nBLOld, pLn, wKey);
    *pwVal = 0; // How much does this cost?
    return pwVal;
  #else // B_JUDYL
    Insert(nBLOld, pLn, wKey);
  #endif // B_JUDYL
}
#endif // defined(CODE_XX_SW)

#ifdef B_JUDYL
static Word_t*
#else // B_JUDYL
static void
#endif // B_JUDYL
InsertSwitch(qp,
             Word_t wKey,
             int nBLNew
  #ifdef SKIP_TO_XX_SW
           , int nBLUp
  #endif // SKIP_TO_XX_SW
#ifdef CODE_XX_SW
           , Link_t *pLnUp
#endif // CODE_XX_SW
#ifdef CODE_XX_SW
           , Word_t wPopCnt
#endif // CODE_XX_SW
             )
{
    qv;
    int nDL = nBL_to_nDL(nBL); (void)nDL;
    int nDLNew = nBL_to_nDL(nBLNew); (void)nDLNew;
#if defined(SKIP_LINKS)
    DBGI(printf("InsertSwitch 0 nDLNew %d nBLNew %d nDL %d nBL %d\n",
                nDLNew, nBLNew, nDL, nBL));

    // Apply constraints that cause us to create the new switch
    // at a higher level than would be required if only the common
    // prefix, n[BD]LNew, were considered.

    // We don't create a switch below nDL == 2. Why? Policy?
    // Nor do we create a switch at or below nBL == cnLogBitsPerWord.
    // Why can't we?
    // The latter is enforced in one way by disallowing
    // cnBitsAtDl2 <= cnLogBitsPerWord no later than Initialize time.
#ifdef USE_XX_SW_ONLY_AT_DL2
#ifndef SKIP_TO_XX_SW
    // We don't skip to a switch below DL3.  Because we can't
    // skip to T_XX_SW and we want T_XX_SW at DL2 and below.
    if ((nBLNew < cnBitsLeftAtDl3) && (nBL >= cnBitsLeftAtDl3)) {
        nBLNew = cnBitsLeftAtDl3;
        nDLNew = 3;
    }
#endif // #ifndef SKIP_TO_XX_SW
#endif // USE_XX_SW_ONLY_AT_DL2
    if ((nBLNew < cnBitsLeftAtDl2) && (nBL >= cnBitsLeftAtDl2)) {
        nBLNew = cnBitsLeftAtDl2;
        nDLNew = 2;
    }
    DBGI(printf("InsertSwitch 1 nDLNew %d nBLNew %d nDL %d nBL %d\n",
                nDLNew, nBLNew, nDL, nBL));
#ifdef BITMAP
    // How did we get here?
    // I don't think we should get here if (nBLNew <= cnLogBitsPerLink).
    assert((nBLNew > cnLogBitsPerLink) || !cbEmbeddedBitmap);
#endif // BITMAP

    Link_t link; (void)link;
#if defined(PP_IN_LINK)
    // PP_IN_LINK can only support skip from top for wPrefix == 0.
    if (nBL == cnBitsPerWord) {
        while ((nBLNew != nBL) && ((wKey >> nBLNew) != 0)) {
            nDLNew += 1;
            nBLNew = nDL_to_nBL(nDLNew);
            DBGI(printf("Bumping PP_IN_LINK skip at top nDLR"
                            " from %d to %d.\n",
                        nDLNew - 1, nDLNew));
        }
    }
#endif // defined(PP_IN_LINK)
#if ! defined(LVL_IN_PP) && ! defined(LVL_IN_WR_HB)
// Depth is in type.
    if (nDLNew != nDL) {
        if (nDL_to_tp(nDLNew) > (int)cnMallocMask) {
            printf("\n# Oops. Can't encode absolute level for skip.\n");
            printf("nDLNew %d nDL %d\n", nDLNew, nDL);
            nDLNew = nDL - 1;
            nBLNew = nDL_to_nBL(nDLNew);
            assert(0);
        }
    }
#endif // ! defined(LVL_IN_PP) && ! defined(LVL_IN_WR_HB)
#endif // defined(SKIP_LINKS)

#ifdef BITMAP
  #ifndef B_JUDYL
    // If the proposed switch would be big enough hold a bitmap covering
    // nBLNew, then we should create a bitmap,
    // i.e. (nBLNew - nBW) <= cnLogBitsPerLink.
  #endif // B_JUDYL
      #ifdef USE_XX_SW_ONLY_AT_DL2
    // t_bitmap in Insert1 can't handle a bitmap at (nBL != cnBitsLeftAtD2)
    // for USE_XX_SW_ONLY_AT_DL2 because we don't want to have to test for it.
    // We expect that once (nBL == cnBitsLeftAtDl2) we just keep doubling
    // until (nBL == cnLogBitsPerLink).
      #else // USE_XX_SW_ONLY_AT_DL2
    if ((nDLNew == 1) && !cbEmbeddedBitmap)
    {
        assert(nBL == nBLNew);
          #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // NewBitmap changes *pwRoot and we change the Link_t
        // containing it on return from NewBitmap.
        // We need to preserve the Link_t for subsequent InsertAll.
        link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
          #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        NewBitmap(qy, nBLNew, wKey, /* wPopCnt */ 0);
          #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        set_PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBLNew, 0);
          #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    }
    else
      #endif // #else USE_XX_SW_ONLY_AT_DL2
#endif // BITMAP
    {
#if defined(USE_BM_SW) || defined(CODE_XX_SW)
        int nBW;
#endif // defined(USE_BM_SW) || defined(CODE_XX_SW)
#ifndef USE_XX_SW_ONLY_AT_DL2
        // When would a switch at nDLNew == 1 make any sense?
        // A bitmap switch for B_JUDYL with the link containing the value?
        assert(nDLNew > 1);
#endif // #ifndef USE_XX_SW_ONLY_AT_DL2

        // NewSwitch overwrites *pwRoot which would be a problem for
        // embedded keys.
        // Unless we've inflated them out.  Which we have.

#if defined(CODE_XX_SW)
        if (nBLNew < nDL_to_nBL(2)) {
            DBGI(printf("\n# Blow up nBLNew %d nPopCnt %d\n",
                        nBLNew, (int)wPopCnt));
        }

  #if defined(USE_XX_SW)
        if (1
      #ifdef USE_XX_SW_ONLY_AT_DL2
              && (nBLNew == nDL_to_nBL(2)) // Use XX_SW at DL2.
      #endif // USE_XX_SW_ONLY_AT_DL2
      #if ! defined(SKIP_TO_XX_SW)
              && (nBLNew == nBL)
      #endif // ! defined(SKIP_TO_XX_SW)
            )
        {
            DBGI(printf("# Creating T_XX_SW wKey " OWx" nBLNew %d\n",
                        wKey, nBLNew));
      #if defined(SKIP_TO_XX_SW)
            if (nBL != nBLNew) {
                DBGR(printf("Skip to T_XX_SW nBL %d\n", nBL));
            }
      #endif // defined(SKIP_TO_XX_SW)
            nBW = cnBW;
        } else if (pLnUp != NULL) {
// Shouldn't we think about some other option here?
// What about a small bitmap?
// Or another switch?
  #endif // defined(USE_XX_SW)
            return DoubleIt(qy, wKey
      #if defined(SKIP_TO_XX_SW)
                          , nBLUp
      #endif // defined(SKIP_TO_XX_SW)
                          , pLnUp
      #ifdef CODE_XX_SW
                          , wPopCnt
      #endif // CODE_XX_SW
                            );
  #if defined(USE_BM_SW) || defined(USE_XX_SW)
        } else
        { nBW = nBL_to_nBW(nBLNew); }
  #endif // defined(USE_BM_SW) || defined(USE_XX_SW)
#elif defined(USE_BM_SW) // defined(CODE_XX_SW)
        nBW = nBL_to_nBW(nBLNew);
#endif // #elif defined(USE_BM_SW)

#if defined(DEBUG)
        if (nBLNew > nBL) {
            printf("IG: pwRoot %p wKey " OWx" nBLNew %d wRoot " OWx"\n",
                   (void *)pwRoot, wKey, nBLNew, wRoot);
            printf("nBL %d\n", nBL);
        }
#endif // defined(DEBUG)
        assert(nBLNew <= nBL);

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // NewSwitch changes *pwRoot and the Link_t containing it.
        // We need to preserve the Link_t for subsequent InsertAll.
        // We don't have a whole link at the top.
        if (nBL < cnBitsPerWord) {
            link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
        }
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        NewSwitch(pwRoot, wKey, nBLNew,
#if defined(CODE_XX_SW)
                  nBW,
#endif // defined(CODE_XX_SW)
  #if defined(USE_BM_SW)
      #if defined(USE_XX_SW)
                  (nBLNew <= nDL_to_nBL(2)) ? T_SWITCH :
      #endif // defined(USE_XX_SW)
                  // Don't waste time with T_BM_SW if splaying a big list.
                  // Use T_BM_SW if splaying a small list or handling a prefix
                  // mismatch.
                  // This should be fine-tuned for Judy1 vs. JudyL with
                  // EMBED_KEYS vs. JudyL without EMBED_KEYS, etc.
                  ((GetPopCnt(pwRoot, nBL) + 1) >= EXP(nBW))
                          && (nBLNew <= nBL)
                      ? T_SWITCH :
                  // Here only if small subtree or nBLNew > nBL.
                  // Use uncompressed switch in prefix mismatch case if pop
                  // handles it.
                  // This should be fine-tuned for Judy1 vs. JudyL with
                  // EMBED_KEYS vs. JudyL without EMBED_KEYS, as well as
                  // variable bits per digit, etc.
                  (nBLNew > nBL)
                          && (GetPopCnt(pwRoot, nBL)
                              >= nDLNew * EXP(cnBitsPerDigit))
                      ? T_SWITCH :
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                  nBL != cnBitsPerWord ? T_BM_SW : T_SWITCH,
          #else // defined(BM_IN_LINK)
                  T_BM_SW,
          #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK)
                  (nBL != cnBitsPerWord)
                      && (nBLNew == nBL) ? T_BM_SW : T_SWITCH,
          #else // defined(BM_IN_LINK)
                  (nBLNew == nBL) ? T_BM_SW : T_SWITCH,
          #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
                  T_SWITCH,
  #endif // defined(USE_BM_SW)
                  nBL, /* wPopCnt */ 0);

#if defined(CODE_XX_SW)
        if (nBLNew <= nDL_to_nBL(2)) {
  #if defined(SKIP_TO_XX_SW)
            if (nBLNew != nBL) {
                assert(nBLNew == nDL_to_nBL(nBL_to_nDL(nBLNew)));
                assert(GetBLR(pwRoot, nBL) == nBLNew);
                set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
                assert(tp_bIsXxSw(wr_nType(*pwRoot)));
                assert(GetBLR(pwRoot, nBL) == nBLNew);
            } else
  #endif // defined(SKIP_TO_XX_SW)
            {
                if (nBW >= 7) {
                    DBGI(printf("# Setting T_XX_SW nBW %d nBLNew %d.\n",
                                nBW, nBLNew));
                }
                set_wr_nType(*pwRoot, T_XX_SW);
            }
        }
#endif // defined(CODE_XX_SW)

        if (nBLNew == nBL) {
            DBGI(printf("\n# InsertSwitch After NewSwitch Dump\n"));
            DBGI(Dump(pwRootLast,
                      /* wPrefix */ (Word_t)0, cnBitsPerWord));
            DBGI(printf("\n"));
        }
    }

    // Now we need to move the keys from the old subtree to the new
    // subtree.

    // NewSwitch changed *pwRoot.
    // But wRoot, nType, pwr, nBLNew and nBL still all apply
    // to the tree whose keys must be reinserted.
#if defined(USE_XX_SW)
// Hmm. pLnUp was created to handle NO_TYPE_IN_XX_SW.
// Why would we test it for a different case?
// If pLn == pLnUp does it mean we backed up?
// Do we use this during doubling?
    assert(pLn != pLnUp);
    if (pLn == pLnUp) {
//insertAll:;
        // nBW is for the new tree.
        //printf("Calling InsertAll for all links nBW %d\n", nBW);
        //printf("# Old tree:\n");
        //DBG(Dump(&wRoot, wKey & ~MSK(nBL), nBL));
        int nBLR = nBLNew - pwr_nBW(&wRoot);
        for (int nIndex = 0;
                 nIndex < (int)EXP(pwr_nBW(&wRoot));
                 nIndex++)
        {
            //printf("# New tree before IA nIndex %d:\n", nIndex);
            //DBG(Dump(pwRoot, wKey, nBL));
            InsertAll(&pwr_pLinks((Switch_t *)pwr)[nIndex].ln_wRoot,
                      nBLR,
                      (wKey & ~MSK(nBLNew)) | (nIndex << nBLR),
                      pwRoot,
// How are we going to get nBL from pLnUp?
// Do we need it?  We need it for the call back into Insert.
                      nBL);
        }

#if ! defined(SKIP_TO_XX_SW)
        assert(nBLNew == nDL_to_nBL(2));
        assert(nBL == nBLNew);
#endif // ! defined(SKIP_TO_XX_SW)
        OldSwitch(&wRoot, /* nBL */ nBLNew,
#if defined(CODE_BM_SW)
                  /* bBmSw */ 0, /* nLinks */ 0,
#endif // defined(CODE_BM_SW)
                  /* nBLUp */ nBL);

        //printf("# New tree after InsertAll done looping:\n");
        //DBG(Dump(pwRoot, wKey, nBL));

    } else
#endif // defined(USE_XX_SW)
    {
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // InsertAll may look in the link containing wRoot for
        // pop count. That's why we preserved the contents of
        // the link before overwriting it above.
        if (nBL < cnBitsPerWord) {
            InsertAll(&link.ln_wRoot, nBL, wKey, pwRoot, nBL);
        } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        {
            // *pwRoot now points to a switch
            InsertAll(/* old */ &wRoot, /* old */ nBL, wKey, pwRoot, nBL);
        }
    }

    if (nBLNew == nBL) {
        DBGI(printf("Just Before InsertGuts calls final Insert"));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
    }

#ifdef B_JUDYL
    Word_t *pwVal = Insert(nBL, pLn, wKey);
    *pwVal = 0; // How much does this cost?
    return pwVal;
#else // B_JUDYL
    Insert(nBL, pLn, wKey);
#endif // B_JUDYL
}

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
#ifdef B_JUDYL
static Word_t*
#else // B_JUDYL
static void
#endif // B_JUDYL
Splay(qp,
      Word_t wKey,
#ifdef CODE_XX_SW
      Link_t *pLnUp,
  #ifdef SKIP_TO_XX_SW
      int nBLUp,
  #endif // SKIP_TO_XX_SW
#endif // CODE_XX_SW
      Word_t wPopCnt,
      Word_t *pwKeys
#ifdef COMPRESSED_LISTS
#if (cnBitsPerWord > 32)
    , unsigned int *piKeys
#endif // (cnBitsPerWord > 32)
    , unsigned short *psKeys,
      unsigned char *pcKeys
#endif // COMPRESSED_LISTS
      )
{
      qv;
      int nDL = nBL_to_nDL(nBL); (void)nDL;
      (void)wPopCnt;
      (void)pwKeys;
#ifdef COMPRESSED_LISTS
#if (cnBitsPerWord > 32)
      (void)piKeys;
#endif // (cnBitsPerWord > 32)
      (void)psKeys;
      (void)pcKeys;
#endif // COMPRESSED_LISTS
      int nBLNew = nBL;

      DBGI(printf("Splay nBL %d.\n", nBL));

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
        if (nDL >= 2) {
  #if defined(NO_SKIP_AT_TOP)
            if (nBL != cnBitsPerWord)
  #endif // defined(NO_SKIP_AT_TOP)
            {
                nBLNew = cnBitsLeftAtDl2;
            }
        }
        return InsertSwitch(qy, wKey, nBLNew
      #ifdef CODE_XX_SW
          #ifdef SKIP_TO_XX_SW
                          , nBLUp
          #endif // SKIP_TO_XX_SW
                          , pLnUp
      #endif // CODE_XX_SW
      #ifdef CODE_XX_SW
                          , wPopCnt
      #endif // CODE_XX_SW
                            );
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
        return InsertSwitch(qy, wKey, nBLNew
      #ifdef SKIP_TO_XX_SW
                          , nBLUp
      #endif // SKIP_TO_XX_SW
                          , pLnUp
      #ifdef CODE_XX_SW
                          , wPopCnt
      #endif // CODE_XX_SW
                            );
    }
#endif // defined(CODE_XX_SW)

#if defined(NO_SKIP_AT_TOP)
    if (nDL < cnDigitsPerWord)
#endif // defined(NO_SKIP_AT_TOP)
#if defined(SKIP_LINKS)
    {
        Word_t wSuffix;
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
        {
            wMin = pwKeys[0];
            wMax = pwKeys[wPopCnt - 1];
            wSuffix = wKey;
        }
#else // defined(SORT_LISTS)
        // walk the list to find max and min
        wMin = (Word_t)-1;
        wMax = 0;

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
        nBLNew = LOG((wSuffix ^ wMin) | (wSuffix ^ wMax)) + 1;
#else // (cwListPopCntMax != 0)
        // Can't dereference list if there isn't one.
        // Go directly to dl2.
        // Can't skip directly to dl1 since neither bitmap nor
        // list leaf have a prefix.
        if (nDL >= 2) {
  #if defined(NO_SKIP_AT_TOP)
            if (nBL != cnBitsPerWord)
  #endif // defined(NO_SKIP_AT_TOP)
            {
                nBLNew = nDL_to_nBL(2);
            }
        }
#endif // #else (cwListPopCntMax != 0)
    }
    nBLNew = nDL_to_nBL(nBL_to_nDL(nBLNew)); // round up to digit boundary
  #ifdef USE_XX_SW_ONLY_AT_DL2
    if (nBLNew > nBL) { nBLNew = nBL; } // time to double the switch
  #endif // USE_XX_SW_ONLY_AT_DL2
    assert(nBLNew <= nBL);

#endif // defined(SKIP_LINKS)
    return InsertSwitch(qy, wKey, nBLNew
  #ifdef SKIP_TO_XX_SW
                      , nBLUp
  #endif // SKIP_TO_XX_SW
#if defined(CODE_XX_SW)
                      , pLnUp
#endif // defined(CODE_XX_SW)
      #ifdef CODE_XX_SW
                      , wPopCnt
      #endif // CODE_XX_SW
                        );
}

#ifdef B_JUDYL
static Word_t*
#else // B_JUDYL
static void
#endif // B_JUDYL
InsertAtList(qp,
             Word_t wKey,
             int nPos
#ifdef CODE_XX_SW
           , Link_t *pLnUp
  #ifdef SKIP_TO_XX_SW
           , int nBLUp
  #endif // SKIP_TO_XX_SW
#endif // CODE_XX_SW
#if defined(B_JUDYL) && defined(EMBED_KEYS)
           , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
             )
{
    qv;
    Word_t wPopCnt = 0;
    Word_t *pwKeys = NULL;
#if defined(COMPRESSED_LISTS)
  #if (cnBitsPerWord > 32)
    unsigned int *piKeys = NULL;
  #endif // (cnBitsPerWord > 32)
    unsigned short *psKeys = NULL;
    unsigned char *pcKeys = NULL;
#endif // defined(COMPRESSED_LISTS)
#ifdef B_JUDYL
    Word_t *pwValue;
#endif // B_JUDYL

    DBGI(printf("InsertAtList\n"));

    // Initialize wPopCnt, pwKeys, piKeys, psKeys and pcKeys for copy.
    // And set prefix in link if PP_IN_LINK and the list is empty and
    // we're not at the top.
    assert((wRoot != 0) || (WROOT_NULL == 0));
    // We get here no matter which type of WROOT_NULL we have.
    // When we insert into an empty expanse we call InsertAtList.
    if (wRoot != WROOT_NULL) { // pwr is pointer to old List
#if defined(EMBED_KEYS)
        assert(nType != T_EMBEDDED_KEYS);
#endif // defined(EMBED_KEYS)
        wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL);
        pwKeys = ls_pwKeys(pwr, nBL); // list of keys in old List
#if defined(COMPRESSED_LISTS)
#if (cnBitsPerWord > 32)
        piKeys = ls_piKeysNATX(pwr, wPopCnt);
#endif // (cnBitsPerWord > 32)
        psKeys = ls_psKeysNATX(pwr, wPopCnt);
        pcKeys = ls_pcKeysNATX(pwr, wPopCnt);
#endif // defined(COMPRESSED_LISTS)
        // prefix is already set
    } else {
#if defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord) {
            // What about no_unnecessary_prefix?
            set_PWR_wPrefixBL(pwRoot, pwr, nBL, wKey);
        }
#endif // defined(PP_IN_LINK)
        // I don't think it is necessary to set nPos here.
        // I wonder what is best for performance of CopyWithInsert.
        nPos = 0;
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

    int nDL = nBL_to_nDL(nBL);
    int nDLOld = nDL; (void)nDLOld;
    int nBLOld = nBL; (void)nBLOld;

#if (cwListPopCntMax != 0) // true if we are using lists; embedded or external

  #if defined(EMBED_KEYS)
  #if ! defined(POP_CNT_MAX_IS_KING) || defined(CODE_XX_SW)
    // It makes no sense to impose a pop limit that is less than what
    // will fit as embedded keys.  If we want to be able to do that for
    // running experiments, then we can use POP_CNT_MAX_IS_KING.
    int nEmbeddedListPopCntMax = EmbeddedListPopCntMax(nBL);
    (void)nEmbeddedListPopCntMax;
  #endif // ! defined(POP_CNT_MAX_IS_KING) || defined(CODE_XX_SW)
  #endif // defined(EMBED_KEYS)
  #if defined(NO_TYPE_IN_XX_SW)
    if ((nBL < nDL_to_nBL(2))
        && (wPopCnt == (Word_t)nEmbeddedListPopCntMax))
    {
        DBGR(printf("IG: goto doubleIt nBL %d cnt %d max %d.\n",
                    nBL, (int)wPopCnt, nEmbeddedListPopCntMax));
        return DoubleIt(qy, wKey
      #ifdef SKIP_TO_XX_SW
                      , nBLUp
      #endif // SKIP_TO_XX_SW
                      , pLnUp
      #ifdef CODE_XX_SW
                      , wPopCnt
      #endif // CODE_XX_SW
                        );
    }
  #endif // defined(NO_TYPE_IN_XX_SW)

    if (0
  #if defined(EMBED_KEYS) && !defined(POP_CNT_MAX_IS_KING)
            || (wPopCnt < (Word_t)nEmbeddedListPopCntMax)
  #endif // defined(EMBED_KEYS) && !defined(POP_CNT_MAX_IS_KING)
            || ((int)wPopCnt < auListPopCntMax[nBL])
        )
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
                    return InsertSwitch(qy, wKey, /* nBLNew */ nBL,
      #ifdef SKIP_TO_XX_SW
                                        nBLUp,
      #endif // SKIP_TO_XX_SW
                                        pLnUp
      #ifdef CODE_XX_SW
                                      , wPopCnt
      #endif // CODE_XX_SW
                                        );
                }
            }
        }
// We get here only if list is not full.
// And here we check if embedded list is full?
// The earlier requirement that the list is full inhibits us
// from taking this opportunity to double instead of splay?
// Do we need auListPopCntMax to be bigger for this case?
// Or ifdef USE_XX_SW_ONLY_AT_DL2 in the above list is full test?
// I'm not a fan of putting the list is full and needs to double
// case in with the code for handling the list is not full case.
        if (nBL < nDL_to_nBL(2)) {
  #if defined(EMBED_KEYS)
            if ((int)wPopCnt >= nEmbeddedListPopCntMax)
  #endif // defined(EMBED_KEYS)
            {
                if ((wWordsAllocated * 100 / wPopCntTotal)
                        < cnXxSwWpkPercent)
                {
                    return DoubleIt(qy, wKey
      #if defined(SKIP_TO_XX_SW)
                                  , nBLUp
      #endif // defined(SKIP_TO_XX_SW)
                                  , pLnUp
      #ifdef CODE_XX_SW
                                  , wPopCnt
      #endif // CODE_XX_SW
                                    );
                }
            }
        }
#endif // defined(CODE_XX_SW)

        Word_t *pwList;

        // Allocate memory for a new list if necessary.
        // Init or update pop count if necessary.
        if ((wPopCnt == 0) || (ListSlotCnt(wPopCnt, nBL) < (int)(wPopCnt + 1))) {
            DBGI(printf("pwr %p wPopCnt %" _fw"d nBL %d\n",
                        (void *)pwr, wPopCnt, nBL));
            DBGI(printf("nType %d\n", nType));
            DBGI(printf("nBL %d LWE(pop+1 %d) %d\n",
                        nBL, (int)wPopCnt + 1,
                        ListWordCnt(wPopCnt + 1, nBL)));
            // Allocate a new list and init pop count if pop count is
            // in the list.  Also init the beginning of the list marker
            // if LIST_END_MARKERS.
            assert(wPopCnt + 1 != 0);
            pwList = NewListTypeList(wPopCnt + 1, nBL);
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            assert((nDL == cnDigitsPerWord)
                || (PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                        == wPopCnt + 1));
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if defined(UA_PARALLEL_128)
            if ((nBL == 16) && (wPopCnt < 6)) {
                set_wr(wRoot, pwList, T_LIST_UA);
            } else
#endif // defined(UA_PARALLEL_128)
            { set_wr(wRoot, pwList, T_LIST); }

            // qy is no longer correct.
            // wRoot has been updated.
            // pLn->ln_wRoot has not been updated.
            // pwr, nType have not been updated.
            // Can we delay updating wRoot?
            // Can we update the others now?
        } else {
            pwList = pwr;

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            if (nDL != cnDigitsPerWord) {
                assert(PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL)
                       == wPopCnt + 1);
            }
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        }

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        { Set_xListPopCnt(&wRoot, nBL, wPopCnt + 1); }

        // It's a bit ugly that we are installing this T_LIST for
        // NO_TYPE_IN_XX_SW.  But we know that DEL is going to fix it.
        // We could give DeflateExternalList &wRoot, then install the new
        // wRoot ourselves (if DEL returned it).

        pLn->ln_wRoot = wRoot; // install new
        int nTypeOld = nType;
        nType = wr_nType(wRoot);
        Word_t *pwrOld = pwr;
        pwr = pwList;
        DBGX(Log(qy, "After NewList"));

        if (wPopCnt != 0)
#if defined(SORT_LISTS)
        {
#if defined(COMPRESSED_LISTS)
            if (nBL <= 8) {
                goto copyWithInsert8;
copyWithInsert8:
  #ifdef B_JUDYL
                pwValue =
  #endif // B_JUDYL
                    CopyWithInsert8(qy, pcKeys, wPopCnt,
                                    (unsigned char)wKey, nPos);
            } else if (nBL <= 16) {
                goto copyWithInsert16;
copyWithInsert16:
  #ifdef B_JUDYL
                pwValue =
  #endif // B_JUDYL
                    CopyWithInsert16(qy, psKeys, wPopCnt,
                                     (uint16_t)wKey, nPos);
#if (cnBitsPerWord > 32)
            } else if (nBL <= 32) {
                goto copyWithInsert32;
copyWithInsert32:
  #ifdef B_JUDYL
                pwValue =
  #endif // B_JUDYL
                    CopyWithInsert32(qy, piKeys, wPopCnt,
                                     (unsigned int)wKey, nPos);
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                goto copyWithInsertWord;
copyWithInsertWord:
  #ifdef B_JUDYL
                pwValue =
  #endif // B_JUDYL
                    CopyWithInsertWord(qy, pwKeys, wPopCnt, wKey, nPos);
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
  #if !defined(EMBED_KEYS) && defined(SORT_LISTS) \
  && defined(PSPLIT_SEARCH_8) && defined(PSPLIT_PARALLEL)
                //printf("goto copyWithInsert8\n");
                goto copyWithInsert8;
  #else // !defined(EMBED_KEYS) && ... d&& efined(PSPLIT_PARALLEL)
// Why is it ok here to skip the padding that would be done in
// CopyWithInsert8? Because we know wPopCnt == 0 so we know we are
// going to deflate into embedded keys later?
                ls_pcKeysNATX(pwList, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
          #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
                if (nBL == cnBitsInD1) {
                    pwValue = &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
          #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
                { pwValue = &gpwValues(qy)[~wPopCnt]; }
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && ... && defined(PSPLIT_PARALLEL)
            } else if (nBL <= 16) {
  #if !defined(EMBED_KEYS) && defined(SORT_LISTS) \
  && defined(PSPLIT_SEARCH_16) && defined(PSPLIT_PARALLEL)
                nPos = 0;
                //printf("goto copyWithInsert16\n");
                goto copyWithInsert16;
  #else // !defined(EMBED_KEYS) && ... && defined(PSPLIT_PARALLEL)
                ls_psKeysNATX(pwList, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
                pwValue = &gpwValues(qy)[~wPopCnt];
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && ... && defined(PSPLIT_PARALLEL)
#if (cnBitsPerWord > 32)
            } else if (nBL <= 32) {
  #if !defined(EMBED_KEYS) && defined(SORT_LISTS) \
      && defined(PSPLIT_SEARCH_32) && defined(PSPLIT_PARALLEL)
                //printf("goto copyWithInsert32\n");
                goto copyWithInsert32;
  #else // !defined(EMBED_KEYS) && ... defined(PSPLIT_PARALLEL)
                ls_piKeysNATX(pwList, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
                pwValue = &gpwValues(qy)[~wPopCnt];
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && ... defined(PSPLIT_PARALLEL)
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
  #if !defined(EMBED_KEYS) && defined(SORT_LISTS) \
      && defined(PARALLEL_SEARCH_WORD)
                //printf("goto copyWithInsertWord\n");
                goto copyWithInsertWord;
  #else // !defined(EMBED_KEYS) && ... && defined(PARALLEL_SEARCH_WORD)
                ls_pwKeysX(pwList, nBL, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
                pwValue = &gpwValues(qy)[~wPopCnt];
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && ... && defined(PARALLEL_SEARCH_WORD)
            }
            // Shouldn't we be padding the extra key slots
            // for parallel search? Is the unsorted list
            // code so dead that we should abandon it?
            // Is the uncompressed lists so dead that we
            // should abandon it?
        }

        if ((wPopCnt != 0) && (pwr != pwrOld)) {
            OldList(pwrOld, wPopCnt, nBL, nTypeOld);
        }

#if defined(EMBED_KEYS)
        // Embed the list if it fits.
        if ( ! ( (nType == T_LIST)
#if defined(UA_PARALLEL_128)
               || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
               ) )
        {
            printf("wRoot " Owx"\n", wRoot);
        }
#endif // defined(EMBED_KEYS)
#if defined(EMBED_KEYS)
        // Embed the list if it fits.
        assert((nType == T_LIST)
#if defined(UA_PARALLEL_128)
               || (nType == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
               );
        if ((int)wPopCnt < EmbeddedListPopCntMax(nBL))
        {
            DeflateExternalList(pwRoot, wPopCnt + 1, nBL, pwList
#ifdef B_JUDYL
                              , pwValueUp
#endif // B_JUDYL
                                );
            assert(*pwRoot != WROOT_NULL);
#ifdef B_JUDYL
            // Update pwValue for return.
            pwValue = pwValueUp;
#endif // B_JUDYL
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
        DBGI(printf("List is full nBL %d wPopCnt %zd.\n", nBL, wPopCnt));

  #ifdef B_JUDYL
        pwValue =
  #endif // B_JUDYL
            Splay(qy, wKey,
#ifdef CODE_XX_SW
                  pLnUp,
  #ifdef SKIP_TO_XX_SW
                  nBLUp,
  #endif // SKIP_TO_XX_SW
#endif // CODE_XX_SW
                  wPopCnt, pwKeys
#ifdef COMPRESSED_LISTS
#if (cnBitsPerWord > 32)
                , piKeys
#endif // (cnBitsPerWord > 32)
                , psKeys, pcKeys
#endif // COMPRESSED_LISTS
                  );
    }
#ifdef B_JUDYL
    DBGI(printf("InsertAtList returning %p\n", (void*)pwValue));
    return pwValue;
#endif // B_JUDYL
}

// InsertGuts
// This function is called from the iterative Insert function once Insert has
// determined that the key from an insert request is not present in the array.
// It is provided with a starting point (pwRoot, wRoot, nBL) for the insert
// and some additional information (nPos, pLnUp, nBLPrev) that may be
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
#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertGuts(qp, Word_t wKey, int nPos
#if defined(CODE_XX_SW)
         , Link_t *pLnUp
  #if defined(SKIP_TO_XX_SW)
         , int nBLUp
  #endif // defined(SKIP_TO_XX_SW)
#endif // defined(CODE_XX_SW)
#if defined(B_JUDYL) && defined(EMBED_KEYS)
         , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
           )
{
    qv;
#if defined(CODE_XX_SW)
  #if defined(SKIP_TO_XX_SW)
    (void)nBLUp;
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
    if (pLnUp != NULL) { // non-NULL only for XX_SW
        DBGR(printf("IG: goto embeddedKeys.\n"));
        assert(tp_bIsXxSw(wr_nType(pLnUp->ln_wRoot)));
        goto embeddedKeys; // no type field is handled by embeddedKeys
    }
#endif // defined(NO_TYPE_IN_XX_SW)

#ifdef BITMAP
  #ifndef B_JUDYL
    // Embedded bitmap is not implemented for B_JUDYL yet.
    // Check to see if we're at the bottom before checking nType since
    // nType may be invalid if wRoot is an embedded bitmap.
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        SetBit(pLn, wKey & MSK(nBL)); // Insert into embedded bitmap.
        return Success;
    }
  #endif // #ifndef B_JUDYL

    // Shouldn't we be checking for nDL <= 2 for USE_XX_SW_ONLY_AT_DL2
    // before testing any type values? Nope. USE_XX_SW_ONLY_AT_DL2 does
    // not imply NO_TYPE_IN_XX_SW.

    if ((wr_nType(WROOT_NULL) != T_BITMAP) || (wRoot != WROOT_NULL)) {
        if ((nType == T_BITMAP)
  #if defined(SKIP_TO_BITMAP)
            || (nType == T_SKIP_TO_BITMAP)
  #endif // defined(SKIP_TO_BITMAP)
            || 0)
        {
            return InsertAtBitmap(qy, wKey);
        }
    }
#endif // BITMAP

    // Can the following be moved into the if ! switch block?
#if (cwListPopCntMax != 0)
  #if defined(EMBED_KEYS)
    // Change an embedded list into an external list to make things
    // easier for Insert.  We'll change it back later if it makes sense.
    if (nType == T_EMBEDDED_KEYS) {
        goto embeddedKeys;
embeddedKeys:;
        int nPopCnt
            = ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
                    && (wRoot == WROOT_NULL))
                ? 0 : wr_nPopCnt(wRoot, nBL);
          #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
              #if ! defined(PACK_KEYS_RIGHT)
                  #ifndef B_JUDYL // for JudyL turn-on
        // This is a performance shortcut that is not necessary.
        if (nPopCnt < EmbeddedListPopCntMax(nBL)) {
            InsertEmbedded(pwRoot, nBL, wKey); return Success;
        }
                  #endif // B_JUDYL // for JudyL turn-on
              #endif // ! defined(PACK_KEYS_RIGHT)
          #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)

        if (nPopCnt != 0) {
            wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot
#ifdef B_JUDYL
                                      , pwValueUp
#endif // B_JUDYL
                                        );
        }
        // InflateEmbeddedList installs wRoot. It also initializes the
        // other words in the link if there are any.
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Insert would have incremented pop count in the link on the way in
        // if this had been a T_LIST at that time.
        // Do we use the prefix-pop field in the link for embedded keys?
        set_PWR_wPopCntBL(pwRoot, NULL, nBL, nPopCnt + 1);
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        nPos = -1; // Tell copy that we have no nPos.
        // Why don't we have an nPos?

        // BUG: The list may not be sorted at this point.  Does it matter?
        // Update: I'm not sure why I wrote that the list may not be sorted
        // at this point.  I can't think of why it would not be sorted.
        // Is it related to SEARCH_FROM_WRAPPER?
        // Is it related to possibly not sorting embedded keys since we
        // parallel search embedded keys?
        nType = wr_nType(wRoot);
      #if !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
        DBGI(printf("IG: wRoot " OWx" nType %d PWR_xListPopCnt %d\n",
                    wRoot, nType,
                    (int)PWR_xListPopCnt(&wRoot, wr_pwr(wRoot), nBL)));
      #endif // !defined(PP_IN_LINK) && !defined(POP_WORD_IN_LINK)
    }
  #endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

    pwr = wr_pwr(wRoot);
    DBGX(Log(qy, "InsertGuts"));

// Why is it ok to skip the test for a switch if !defined(SKIP_LINKS)
// and !defined(BM_SW_FOR_REAL)? Because InsertGuts is called with a wRoot
// that points to a switch only for prefix mismatch or missing link cases.
#if defined(SKIP_LINKS)
    if (!tp_bIsSwitch(nType)
        || ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)))
#else // defined(SKIP_LINKS)
  #if defined(BM_SW_FOR_REAL)
      #if (cwListPopCntMax != 0)
    if (!tp_bIsSwitch(nType)
        || ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)))
      #else // (cwListPopCntMax == 0)
    if (wRoot == WROOT_NULL)
      #endif // (cwListPopCntMax == 0)
  #endif // defined(BM_SW_FOR_REAL)
#endif // defined(SKIP_LINKS)
    {
#ifdef B_JUDYL
        return
#endif // B_JUDYL
            InsertAtList(qy, wKey, nPos
#ifdef CODE_XX_SW
                       , pLnUp
  #ifdef SKIP_TO_XX_SW
                       , nBLUp
  #endif // SKIP_TO_XX_SW
#endif // CODE_XX_SW
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                       , pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                         );
    }
#if defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)
    else
    {
        assert(*pwRoot != WROOT_NULL);
  #if defined(EMBED_KEYS)
        assert(wr_nType(*pwRoot) != T_EMBEDDED_KEYS);
  #endif // defined(EMBED_KEYS)
        int nDLR =
  #ifdef SKIP_LINKS
                 tp_bIsSkip(nType) ? (int)wr_nDL(wRoot) :
  #endif // SKIP_LINKS
                 nDL;
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
            NewLink(qy, wKey, nDLR, /* nDLUp */ nDL);
#ifdef B_JUDYL
            return
#endif // B_JUDYL
                Insert(nBL, pLn, wKey);
        }
#endif // defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
        else
#endif // defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS)
        {
  #ifdef B_JUDYL
            return
  #endif // B_JUDYL
                InsertAtPrefixMismatch(qy, wKey, nDL_to_nBL(nDLR));
        }
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)

#ifndef B_JUDYL
    return Success;
#endif // B_JUDYL
}

#if (cwListPopCntMax != 0)
#if defined(EMBED_KEYS)

// Replace a wRoot that has embedded keys with an external T_LIST leaf.
// It assumes the input is an embedded list.
Word_t
InflateEmbeddedList(Word_t *pwRoot, Word_t wKey, int nBL, Word_t wRoot
#ifdef B_JUDYL
                  , Word_t *pwValueUp
#endif // B_JUDYL
                    )
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
    if (nPopCnt > nPopCntMax) {
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
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    { Set_xListPopCnt(&wRootNew, nBL, nPopCnt); }

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
            pcKeys = ls_pcKeysNATX(pwList, nPopCnt);
            pcKeys[nn] = (uint8_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
  #if defined(DEBUG_REMOVE)
//            printf("nn %d nSlot %d pcKeys[?] 0x%x\n", nn, nSlot, pcKeys[nn]);
  #endif // defined(DEBUG_REMOVE)
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNATX(pwList, nPopCnt);
            psKeys[nn] = (uint16_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
        } else
#if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNATX(pwList, nPopCnt);
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
            pwKeys = ls_pwKeysNATX(pwList, nPopCnt);
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            assert(nBL != cnBitsPerWord);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            pwKeys[nn] = (wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM);
        }
#ifdef B_JUDYL
        // Copy the value.
        // BUG: Fix this to use gpwValues
        pwList[~(nn
  #ifdef LIST_POP_IN_PREAMBLE
                 + 1
  #endif // LIST_POP_IN_PREAMBLE
                 )] = *pwValueUp; // gpwValues(qy)[~nn] = *pwValueUp;
#endif // B_JUDYL
    }

    // What about padding the bucket and/or malloc buffer?

    *pwRoot = wRootNew;

    return wRootNew; // wRootNew is installed
}

// Replace an external T_LIST leaf with a wRoot with embedded keys.
// The function assumes it is possible.
static Word_t
DeflateExternalList(Word_t *pwRoot,
                    int nPopCnt, int nBL, Word_t *pwr
#ifdef B_JUDYL
                  , Word_t *pwValueUp
#endif // B_JUDYL
                    )
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
    set_wr_nPopCnt(wRoot, nBL, nPopCnt); // no-op if EK_CALC_POP
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
            pcKeys = ls_pcKeysNATX(pwr, nPopCnt);
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
            psKeys = ls_psKeysNATX(pwr, nPopCnt);
            DBGI(printf("nn %d nSlot %d psKeys[?] 0x%x\n", nn, nSlot,
                    psKeys[(nn < nPopCnt) ? nn : 0]));
            SetBits(&wRoot, nBL, /* lsb */ cnBitsPerWord - (nSlot * nBL),
                    psKeys[(nn < nPopCnt) ? nn : 0]);
        } else
#if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNATX(pwr, nPopCnt);
            SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                    piKeys[(nn < nPopCnt) ? nn : 0]);
        } else
#endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        {
            // I don't think we have to worry about adjusting ls_pwKeys
            // for PP_IN_LINK || POP_WORD_IN_LINK here since we will not
            // be at the top.
            assert(nBL != cnBitsPerWord);
// I'm thinking this should be ls_pwKeysNATX.
// Can't embed a full key.
            Word_t *pwKeys = ls_pwKeysX(pwr, nBL, nPopCnt);
            SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                    pwKeys[(nn < nPopCnt) ? nn : 0]);
        }
#ifdef B_JUDYL
        // Copy the value.
  #if defined(FILL_W_KEY)
        if (nn < nPopCnt)
  #endif // defined(FILL_W_KEY)
            *pwValueUp = pwr[~(nn
  #ifdef LIST_POP_IN_PREAMBLE
                               + 1
  #endif // LIST_POP_IN_PREAMBLE
                               )];
#endif // B_JUDYL
    }

    OldList(pwr, nPopCnt, nBL, wr_nType(*pwRoot));

    goto done;
done:;

    *pwRoot = wRoot; // install the new list

    DBGI(printf("DEL wRoot " OWx"\n", wRoot));

    assert(wRoot != WROOT_NULL);
    return wRoot;
}

#endif // defined(EMBED_KEYS)
#endif // (cwListPopCntMax != 0)

#ifdef BITMAP

// InsertAtBitmap is for a bitmap that is not at the bottom.
#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtBitmap(qp, Word_t wKey)
{
    qv;

    int nBLR = nBL;
#if defined(SKIP_TO_BITMAP)
    if (nType == T_SKIP_TO_BITMAP) {
        nBLR = gnBLR(qy);

        Word_t wPrefix;
#if defined(PP_IN_LINK)
        if (nBL == cnBitsPerWord) { wPrefix = 0; /* limitation */ } else
#endif // defined(PP_IN_LINK)
#if defined(SKIP_TO_BITMAP) && ! defined(PP_IN_LINK)
        if (nType == T_SKIP_TO_BITMAP) {
            wPrefix = gwBitmapPrefix(qy, nBLR);
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
  #ifdef B_JUDYL
            return InsertAtPrefixMismatch(qy, wKey, nBLR);
  #else // B_JUDYL
            InsertAtPrefixMismatch(qy, wKey, nBLR); return Success;
  #endif // B_JUDYL
        }
    }
#endif // defined(SKIP_TO_BITMAP)

    DBGI(printf("IAB SetBit(pwr " OWx" wKey " OWx") pwRoot %p\n",
                (Word_t)pwr, wKey & MSK(nBLR), (void *)pwRoot));

    // Mask to convert EXP(nBLR) back to 0 for newly created bitmap.
    // Is mask necessary if pop count has its own word?
 #if defined(SKIP_TO_BITMAP) && !defined(PREFIX_WORD_IN_BITMAP_LEAF)
    Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR) & MSK(nBLR);
 #else // defined(SKIP_TO_BITMAP) && !defined(PREFIX_WORD_IN_BITMAP_LEAF)
    Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR);
 #endif // #else defined(SKIP_TO_BITMAP) && !defined(PREFIX_WORD_IN_BITMAP_LEAF)
    assert(wPopCnt < EXP(nBLR));
    Word_t *pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
    assert(!BitIsSet(pwBitmap, wKey & MSK(nBLR)));
  #ifdef B_JUDYL
    Word_t *pwSrcValues = gpwBitmapValues(qy, nBLR);
    int nPos;
      #if (cnBitsPerWord > 32)
    if (wRoot & EXP(cnLsbBmUncompressed)) {
        nPos = wKey & MSK(nBLR);
        goto done;
    }
      #endif // (cnBitsPerWord > 32)
    nPos = ~BmIndex(qy, nBLR, wKey);
    Word_t wWords = BitmapWordCnt(nBLR, wPopCnt + 1); // new
    if (wWords != BitmapWordCnt(nBLR, wPopCnt)) {
        NewBitmap(qy, nBLR, wKey, wPopCnt + 1);
        // Prefix and popcnt are set; bits are not.
        // *pwRoot has been updated. qy is out of date.
        // Copy bits and update qy.
        wRoot = *pwRoot;
        assert(wr_nType(wRoot) == nType);
        Word_t *pwrOld = pwr;
        pwr = wr_pwr(wRoot);
        int nBmWords
            = (nBLR <= cnLogBitsPerWord) ? 1 : EXP(nBLR - cnLogBitsPerWord);
        Word_t *pwBitmapOld = pwBitmap;
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
        COPY(pwBitmap, pwBitmapOld, nBmWords);
        Word_t *pwTgtValues = gpwBitmapValues(qy, nBLR);
      #if (cnBitsPerWord > 32)
        if (wRoot & EXP(cnLsbBmUncompressed)) {
            for (int k = 0; k < (int)EXP(nBLR); ++k) {
                if (BitIsSet(pwBitmap, k)) {
                    pwTgtValues[k] = *pwSrcValues++;
                }
            }
            nPos = wKey & MSK(nBLR);
        } else
      #endif // (cnBitsPerWord > 32)
        {
            COPY(&pwTgtValues[nPos + 1], &pwSrcValues[nPos], wPopCnt - nPos);
            COPY(pwTgtValues, pwSrcValues, nPos);
        }
        OldBitmap(pwrOld, nBLR, wPopCnt);
        pwSrcValues = pwTgtValues;
    } else
  #endif // B_JUDYL
    {
#ifdef B_JUDYL
        MOVE(&pwSrcValues[nPos + 1], &pwSrcValues[nPos], wPopCnt - nPos);
        goto done;
done:
#endif // B_JUDYL
        swBitmapPopCnt(qy, nBLR, wPopCnt + 1);
    }
    SetBit(pwBitmap, wKey & MSK(nBLR));

#if defined(PP_IN_LINK)

    // Shouldn't we do this when we create the switch with the link
    // that points to this bitmap rather than on every insert into
    // the bitmap?

    // What about no_unnecessary_prefix?
    set_PWR_wPrefixBL(pwRoot, NULL, nBLR, wKey);

#endif // defined(PP_IN_LINK)


#ifdef B_JUDYL
    // Insert is responsible for zeroing the value.
    return &pwSrcValues[nPos];
#else // B_JUDYL
    return Success;
#endif // B_JUDYL
}

#endif // BITMAP

static Status_t RemoveAtBitmap(qp, Word_t wKey);

// RemoveCleanup needs work.
// All it does is look for switches (subtrees) with popcnt zero and free them.
// It should be converting sparse subtrees to lists, creating compressed
// switches from uncompressed switches, and removing links from compressed
// switches.
void
RemoveCleanup(Word_t wKey, int nBL, int nBLR, Word_t *pwRoot, Word_t wRoot)
{
    DBGR(printf("RC: pwRoot %p wRoot " OWx" nBL %d nBLR %d\n",
                (void *)pwRoot, wRoot, nBL, nBLR));
    (void)nBL; (void)nBLR;
    int nDL = nBL_to_nDL(nBL);
    int nDLR = nBL_to_nDL(nBLR);
    (void)wKey; (void)nDL; (void)nDLR; (void)pwRoot; (void)wRoot;

    int nType = wr_nType(wRoot); (void)nType;
    Word_t *pwr = wr_pwr(wRoot); (void)pwr;

  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
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

        int nBW;
      #if defined(CODE_XX_SW)
        if (tp_bIsXxSw(nType)) {
            nBW = pwr_nBW(pwRoot);
        } else
      #endif // defined(CODE_XX_SW)
        { nBW = nBL_to_nBW(nBL); }

        Word_t wIndex = (wKey >> (nBLR - nBW)) & MSK(nBW);
        for (Word_t ww = 0; ww < EXP(nBW); ww++)
        {
            Word_t *pwRootLn = &((
#if defined(CODE_BM_SW)
                tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
                                    pwr_pLinks((  Switch_t *)pwr))
                    [ww].ln_wRoot);

            int nBLX =
#if defined(SKIP_LINKS)
// Why are we testing bIsSwitch?
                wr_bIsSwitch(*pwRootLn) && tp_bIsSkip(wr_nType(*pwRootLn))
                    ? (int)wr_nBL(*pwRootLn) :
#endif // defined(SKIP_LINKS)
                    nDL_to_nBL(nDLR - 1);

            //--nDLX;
            if ((*pwRootLn != WROOT_NULL) // Can we avoid this test?
                 // Non-WROOT_NULL doesn't necessarily imply non-zero pop.
                 // We may have a switch with a zero pop.
                 && ((ww != wIndex)
                     || !tp_bIsSwitch(Get_nType(pwRootLn))
                     || PWR_wPopCntBL(pwRootLn,
                                      (Switch_t*)NULL, nBLX)
                         != 0))
            {
                DBGR(printf("wr_nBLX %d", nBLX));
                DBGR(printf(" PWR_wPopCntBL %" _fw"d " OWx"\n",
                            PWR_wPopCntBL(pwRootLn, NULL, nBLX),
                            PWR_wPopCntBL(pwRootLn, NULL, nBLX)));
#ifdef POP_WORD_IN_LINK
                DBGR(printf("PWR_wPopWord %" _fw"d " OWx"\n",
                            PWR_wPopWordBL(pwRootLn, NULL, nBLX),
                            PWR_wPopWordBL(pwRootLn, NULL, nBLX)));
#endif // POP_WORD_IN_LINK
                DBGR(printf("Not empty ww %zd wIndex %zd *pwRootLn " OWx"\n",
                     (size_t)ww, (size_t)wIndex, *pwRootLn));
                return; // may need cleanup lower; caller checks *pwRoot
            }
            // The only non-zero wRoot with zero pop count is a switch.
            // Lists and bitmaps are zeroed by RemoveGuts when their pop
            // goes to zero.
            // Why don't we also zero wRoot for a switch when its pop
            // goes to zero? Because it requires walking back up the tree.
            // Why don't we leave lists and bitmaps for RemoveCleanup to
            // zero? Because nPopCnt == 0 may be ambiguous for these?
            assert((*pwRootLn == 0) || tp_bIsSwitch(Get_nType(pwRootLn)));
        }
        // whole array pop is zero
        FreeArrayGuts(pwRoot, wKey, nBL, /* bDump */ 0
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                    , /*pwrUp*/ NULL, /*nBW*/ 0
  #ifdef CODE_BM_SW
                    , /* nTypeUp */ -1
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                      );
        // caller checks *pwRoot == NULL to see if cleanup is done
    } else
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    {
        assert(tp_bIsSwitch(nType));
        Word_t wPopCnt =
#if defined(CODE_BM_SW)
            tp_bIsBmSw(nType) ? PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDLR) :
#endif // defined(CODE_BM_SW)
                                PWR_wPopCnt(pwRoot, (  Switch_t *)pwr, nDLR);

        if (wPopCnt == 0) {
            FreeArrayGuts(pwRoot, wKey, nDL_to_nBL(nDL), /* bDump */ 0
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                        , /*pwrUp*/ NULL, /*nBW*/ 0
  #ifdef CODE_BM_SW
                        , /* nTypeUp */ -1
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                          );
        }
    }
}

Status_t
RemoveGuts(qp, Word_t wKey
#if defined(B_JUDYL) && defined(EMBED_KEYS)
         , Word_t *pwValueUp
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
           )
{
    qv;
    // nType is not valid for NO_TYPE_IN_XX_SW and nBL < nDL_to_nBL(2)
    // pwr is not valid for NO_TYPE_IN_XX_SW and nBL < nDL_to_nBL(2)
    int nDL = nBL_to_nDL(nBL); (void)nDL;

    DBGR(printf("RemoveGuts(pwRoot %p wKey " OWx" nBL %d wRoot " OWx")\n",
                (void *)pwRoot, wKey, nBL, wRoot));

#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        goto embeddedKeys;
    }
#endif // defined(NO_TYPE_IN_XX_SW)

#ifdef BITMAP
  // Could we be more specific in this ifdef, e.g. cnListPopCntMax16?
  #if (cwListPopCntMax != 0)
    if ((cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink))
      #if defined(SKIP_TO_BITMAP)
        || (nType == T_SKIP_TO_BITMAP)
      #endif // defined(SKIP_TO_BITMAP)
        || (nType == T_BITMAP))
  #else // (cwListPopCntMax != 0)
    assert((cbEmbeddedBitmap && (nBL <= cnLogBitsInLink))
      #if defined(SKIP_TO_BITMAP)
        || (nType == T_SKIP_TO_BITMAP)
      #endif // defined(SKIP_TO_BITMAP)
        || (nType == T_BITMAP));
  #endif // (cwListPopCntMax != 0)
    {
        return RemoveAtBitmap(qy, wKey);
    }
#endif // BITMAP

#if (cwListPopCntMax != 0)

    Word_t wPopCnt;

#if defined(EMBED_KEYS)
    if (((nType == T_EMBEDDED_KEYS))
// Why is nBL_to_nBitsPopCntSz irrelevant here?
        && (nBL <= cnBitsPerWord - cnBitsMallocMask))
    {
        goto embeddedKeys;
embeddedKeys:;
        wPopCnt = wr_nPopCnt(*pwRoot, nBL);
        wRoot = InflateEmbeddedList(pwRoot, wKey, nBL, wRoot
#ifdef B_JUDYL
                                  , pwValueUp
#endif // B_JUDYL
                                    );
      #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Remove would have decremented pop count in the link on the way in
        // if this had been a T_LIST at that time.
        set_PWR_wPopCntBL(pwRoot, NULL, nBL, wPopCnt - 1);
      #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Is there any reason to preserve *pwRoot?
        // Is it a problem to have an external list that could
        // be embedded?
// What about PP_IN_LINK and POP_WORD_IN_LINK?
// Shouldn't we update that word also?
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
    assert(nType == wr_nType(wRoot));

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    // this test only works if we disallow skip links from the top
    // for PP_IN_LINK.
    if (nBL < cnBitsPerWord) {
        wPopCnt = PWR_wPopCnt(pwRoot, (Switch_t *)NULL, nDL) + 1;
    } else
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    { wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBL); }

    // wPopCnt is the count before the remove.

    if (wPopCnt == 1) {
        assert( ! tp_bIsSwitch(nType) );
        OldList(pwr, wPopCnt, nBL, nType);
#if defined(NO_TYPE_IN_XX_SW)
        if (nBL < nDL_to_nBL(2)) { *pwRoot = ZERO_POP_MAGIC; } else
#endif // defined(NO_TYPE_IN_XX_SW)
        { *pwRoot = WROOT_NULL; }
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }

    Word_t *pwKeys = ls_pwKeysX(pwr, nBL, wPopCnt);
#if defined(COMPRESSED_LISTS)
  #if (cnBitsPerWord > 32)
    uint32_t *piKeys = ls_piKeysNATX(pwr, wPopCnt);
  #endif // (cnBitsPerWord > 32)
    uint16_t *psKeys = ls_psKeysNATX(pwr, wPopCnt);
    uint8_t *pcKeys = ls_pcKeysNATX(pwr, wPopCnt);
#endif // defined(COMPRESSED_LISTS)

    int nIndex;
    for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
        (nBL <=  8) ? (pcKeys[nIndex] != (uint8_t) wKey) :
        (nBL <= 16) ? (psKeys[nIndex] != (uint16_t)wKey) :
  #if (cnBitsPerWord > 32)
        (nBL <= 32) ? (piKeys[nIndex] != (uint32_t)wKey) :
  #endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        (pwKeys[nIndex] != wKey);
        nIndex++) ;

    // nIndex identifies the key being removed.

    Word_t *pwList;
    if (ListSlotCnt(wPopCnt - 1, nBL) < (int)wPopCnt) {
        DBGR(printf("ListWordCnt(wPopCnt %zd nBL %d) %d\n",
             wPopCnt-1, nBL, ListWordCnt(wPopCnt-1, nBL)));
        // Malloc a new, smaller list.
        assert(wPopCnt - 1 != 0);
        pwList = NewListTypeList(wPopCnt - 1, nBL);
        set_wr(wRoot, pwList, T_LIST);
    } else {
        pwList = pwr;
    }
    DBGR(printf("ListWordCnt(wPopCnt %zd nBL %d) %d\n",
                wPopCnt, nBL, ListWordCnt(wPopCnt, nBL)));

#if defined(UA_PARALLEL_128)
    if ((nBL == 16) && (wPopCnt - 1 <= 6)) {
        set_wr_nType(wRoot, T_LIST_UA);
    }
#endif // defined(UA_PARALLEL_128)
    // Init pop count in list before using ls_p[csiw]Keys below.
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    if (nBL >= cnBitsPerWord)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
    { Set_xListPopCnt(&wRoot, nBL, wPopCnt - 1); }

  #ifdef B_JUDYL
    Word_t *pwSrcValues = pwr;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcValues -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL

    *pwRoot = wRoot;
    Word_t *pwrOld = pwr;
    pwr = pwList;
    int nTypeOld = nType;
    nType = wr_nType(wRoot);
    DBGX(Log(qy, "RemoveGuts"));

  #ifdef B_JUDYL
    Word_t *pwTgtValues = gpwValues(qy);
  #endif // B_JUDYL

    if (pwr != pwrOld) {
        // Why are we copying the old list to the new one?
        // Because the beginning will be the same.
        // Except for the the pop count.
#ifdef COMPRESSED_LISTS
        switch (nBytesKeySz(nBL)) {
        case sizeof(Word_t):
#endif // COMPRESSED_LISTS
#ifdef B_JUDYL
             // copy values
             COPY(&pwTgtValues[-((int)wPopCnt - 1)],
                  &pwSrcValues[-((int)wPopCnt - 1)],
                  wPopCnt - 1);
// wPopCnt == 2
// Should copy keys [0, 0] to [0, 0]
// Should copy values [-1, -1] to [-1, -1]
// [ 0, wPopCnt-2]
// [~0, ~(wPopCnt-2)]
// [~(wPopCnt-2), ~0]
// [-(wPopCnt-1), ~0]
#endif // B_JUDYL
             // copy keys
             COPY(ls_pwKeysX(pwList, nBL, wPopCnt - 1), pwKeys, wPopCnt - 1);
#ifdef COMPRESSED_LISTS
             break;
#if (cnBitsPerWord > 32)
        case 4:
  #ifdef B_JUDYL
             // copy values
             COPY(&pwTgtValues[-((int)wPopCnt - 1)],
                  &pwSrcValues[-((int)wPopCnt - 1)],
                  wPopCnt - 1);
  #endif // B_JUDYL
             // copy keys
             COPY(ls_piKeysNATX(pwList, wPopCnt - 1), piKeys, wPopCnt - 1);
             break;
#endif // (cnBitsPerWord > 32)
        case 2:
  #ifdef B_JUDYL
             // copy values
             COPY(&pwTgtValues[-((int)wPopCnt - 1)],
                  &pwSrcValues[-((int)wPopCnt - 1)],
                  wPopCnt - 1);
  #endif // B_JUDYL
             // copy keys
             COPY(ls_psKeysNATX(pwList, wPopCnt - 1), psKeys, wPopCnt - 1);
             break;
        case 1:
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
             if (nBL != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
             {
                 // copy values
                 COPY(&pwTgtValues[-((int)wPopCnt - 1)],
                      &pwSrcValues[-((int)wPopCnt - 1)],
                      wPopCnt - 1);
             }
  #endif // B_JUDYL
             // copy keys
             COPY(ls_pcKeysNATX(pwList, wPopCnt - 1), pcKeys, wPopCnt - 1);
             break;
        }
#endif // COMPRESSED_LISTS
    }

#if defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
    unsigned nKeys = wPopCnt - 1; (void)nKeys;
#endif // defined(LIST_END_MARKERS) || defined(PSPLIT_PARALLEL)
#if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBL != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // move values
            MOVE(&pwTgtValues[-((int)wPopCnt - 1)],
                 &pwSrcValues[- (int)wPopCnt     ],
                 wPopCnt - nIndex - 1);
        }
  #endif // B_JUDYL
        // move keys
        MOVE(&ls_pcKeysNATX(pwList, wPopCnt - 1)[nIndex],
             &pcKeys[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n;
#if defined(PSPLIT_PARALLEL)
        // pad list to an integral number of parallel search buckets in length
// Our tests found nothing when this code was incorrectly padding the old
// list. How much, if any, of this is really necessary?
        for (; (n * 1) % sizeof(Bucket_t); ++n) {
            ls_pcKeysNATX(pwList, wPopCnt-1)[n]
                = ls_pcKeysNATX(pwList, wPopCnt-1)[n-1];
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_pcKeysNATX(pwList, wPopCnt - 1)[n] = -1;
#endif // defined(LIST_END_MARKERS)
    } else if (nBL <= 16) {
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtValues[-((int)wPopCnt - 1)],
             &pwSrcValues[- (int)wPopCnt     ],
             wPopCnt - nIndex - 1);
  #endif // B_JUDYL
        // move keys
        MOVE(&ls_psKeysNATX(pwList, wPopCnt - 1)[nIndex],
             &psKeys[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n; // first empty slot
#if defined(PSPLIT_PARALLEL)
  #if defined(UA_PARALLEL_128)
        if ((nType == T_LIST_UA) && (n <= 6)) {
            for (; (n * 2) % 12; ++n) {
                ls_psKeysNATX(pwList, wPopCnt-1)[n]
                    = ls_psKeysNATX(pwList, wPopCnt-1)[n-1];
            }
        } else
  #endif // defined(UA_PARALLEL_128)
        {
            for (; (n * 2) % sizeof(Bucket_t); ++n) {
                ls_psKeysNATX(pwList, wPopCnt-1)[n]
                    = ls_psKeysNATX(pwList, wPopCnt-1)[n-1];
            }
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_psKeysNATX(pwList, wPopCnt - 1)[n] = -1;
#endif // defined(LIST_END_MARKERS)
#if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtValues[-((int)wPopCnt - 1)],
             &pwSrcValues[- (int)wPopCnt     ],
             wPopCnt - nIndex - 1);
  #endif // B_JUDYL
        // move keys
        MOVE(&ls_piKeysNATX(pwList, wPopCnt - 1)[nIndex],
             &piKeys[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n;
#if defined(PSPLIT_PARALLEL)
        // pad list to an integral number of parallel search buckets in length
        for (; (n * 4) % sizeof(Bucket_t); ++n) {
            ls_piKeysNATX(pwList, wPopCnt-1)[n]
                = ls_piKeysNATX(pwList, wPopCnt-1)[n-1];
        }
#endif // defined(PSPLIT_PARALLEL)
#if defined(LIST_END_MARKERS)
        ls_piKeysNATX(pwList, wPopCnt - 1)[n] = -1;
#endif // defined(LIST_END_MARKERS)
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
#if defined(LIST_END_MARKERS)
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#if (cnDummiesInList == 0)
        assert(0);
#endif // (cnDummiesInList == 0)
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
#endif // defined(LIST_END_MARKERS)
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtValues[-((int)wPopCnt - 1)],
             &pwSrcValues[- (int)wPopCnt     ],
             wPopCnt - nIndex - 1);
// wPopCnt == 2, nIndex == 0
// Should move keys [1, 1] to [0, 0]
// Should move values [-2, -2] to [-1, -1]
// [nIndex+1, wPopCnt]
// [-nIndex-2, -wPopCnt-1]
// [-wPopCnt-1, -nIndex-2]
// [-(wPopCnt+1), -nIndex-2]
// [~wPopCnt, -nIndex-2]
  #endif // B_JUDYL
        // move keys
        MOVE(&ls_pwKeysX(pwList, nBL, wPopCnt - 1)[nIndex],
             &pwKeys[nIndex + 1], wPopCnt - nIndex - 1);
        int n = wPopCnt - 1; (void)n;
#if defined(PARALLEL_SEARCH_WORD)
        // pad list to an integral number of parallel search buckets in length
        for (; (n * sizeof(Word_t)) % sizeof(Bucket_t); ++n) {
            ls_pwKeysX(pwList, nBL, wPopCnt-1)[n]
                = ls_pwKeysX(pwList, nBL, wPopCnt-1)[n-1];
        }
#endif // defined(PARALLEL_SEARCH_WORD)
#if defined(LIST_END_MARKERS)
        ls_pwKeysX(pwList, nBL, wPopCnt - 1)[n] = -1;
#endif // defined(LIST_END_MARKERS)
    }

    if (pwr != pwrOld) {
        OldList(pwrOld, wPopCnt, nBL, nTypeOld);
    }

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
        DeflateExternalList(pwRoot, wPopCnt - 1, nBL, pwList
#ifdef B_JUDYL
                          , pwValueUp
#endif // B_JUDYL
                            );
    }
#endif // defined(EMBED_KEYS)

    return Success;

#endif // (cwListPopCntMax != 0)

    (void)pwRoot; (void)wKey; (void)nDL; (void)wRoot;
}

#ifdef BITMAP
// Clear the bit for wKey in the bitmap.
// And free the bitmap if it is empty and not embedded.
static Status_t
RemoveAtBitmap(qp, Word_t wKey)
{
    qv;

    DBGX(printf("RemoveAtBitmap\n"));
    // EXP(nBL) is risky because nBL could be cnBitsPerWord
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        ClrBit(pLn, wKey & MSK(nBL));
    } else {
        int nBLR = nBL;
  #if defined(SKIP_TO_BITMAP)
        if (nType == T_SKIP_TO_BITMAP) {
            nBLR = gnBLR(qy);
        }
  #endif // defined(SKIP_TO_BITMAP)

        Word_t wPopCnt = gwBitmapPopCnt(qy, nBLR) - 1;
        Word_t *pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
  #ifdef B_JUDYL
        Word_t wWords = BitmapWordCnt(nBLR, wPopCnt); // new
      #if (cnBitsPerWord > 32)
        Word_t bUncompressed = wRoot & EXP(cnLsbBmUncompressed);
        if (bUncompressed) {
            if (wWords == BitmapWordCnt(nBLR, EXP(nBLR))) {
                goto done;
            }
        }
      #endif // (cnBitsPerWord > 32)
        Word_t *pwSrcValues = gpwBitmapValues(qy, nBLR);
        int nPos = BmIndex(qy, nBLR, wKey);
        if (wWords != BitmapWordCnt(nBLR, wPopCnt + 1)) {
            NewBitmap(qy, nBLR, wKey, wPopCnt);
            // Prefix and popcnt are set; bits are not.
            // *pwRoot has been updated. qy is out of date.
            // Copy bits and update qy.
            wRoot = *pwRoot;
            assert(wr_nType(wRoot) == nType);
            Word_t *pwrOld = pwr;
            pwr = wr_pwr(wRoot);
            int nBmWords = (nBLR <= cnLogBitsPerWord)
                             ? 1 : EXP(nBLR - cnLogBitsPerWord);
            Word_t *pwBitmapOld = pwBitmap;
            pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
            COPY(pwBitmap, pwBitmapOld, nBmWords);
            Word_t *pwTgtValues = gpwBitmapValues(qy, nBLR);
      #if (cnBitsPerWord > 32)
            if (bUncompressed) {
                for (int k = 0; k < (int)EXP(nBLR); ++k) {
                    if ((k != (int)(wKey & MSK(nBLR)))
                        && BitIsSet(pwBitmap, k))
                    {
                        *pwTgtValues++ = pwSrcValues[k];
                    }
                }
            } else
      #endif // (cnBitsPerWord > 32)
            {
                COPY(&pwTgtValues[nPos], &pwSrcValues[nPos + 1],
                     wPopCnt - nPos);
                COPY(pwTgtValues, pwSrcValues, nPos);
            }
            OldBitmap(pwrOld, nBLR, wPopCnt + 1);
        } else
  #endif // B_JUDYL
        {
#ifdef B_JUDYL
            Word_t *pwSrcValues = gpwBitmapValues(qy, nBLR);
            MOVE(&pwSrcValues[nPos], &pwSrcValues[nPos + 1], wPopCnt - nPos);
            goto done;
done:
#endif // B_JUDYL
            swBitmapPopCnt(qy, nBLR, wPopCnt);
        }
        ClrBit(pwBitmap, wKey & MSK(nBLR));

#if defined(DEBUG_COUNT)
        Word_t wPopCntDbg = 0;
        int nWords
            = (nBLR <= cnLogBitsPerWord) ? 1 : EXP(nBLR - cnLogBitsPerWord);
        for (int i = 0; i < nWords; ++i) {
            wPopCntDbg += __builtin_popcountll(pwBitmap[i]);
        }
        assert(wPopCntDbg == wPopCnt);
#endif // defined(DEBUG_COUNT)

#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        assert(PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBLR) == wPopCnt);
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)

        // Free the bitmap if it is empty.
        if (wPopCnt == 0) {
            OldBitmap(pwr, nBLR, 0);
            *pwRoot = WROOT_NULL;
        }
    }

    return Success;
}
#endif // BITMAP

#endif // (cnDigitsPerWord != 1)

static void
Initialize(void)
{
    // Fine tune auListPopCntMax from cnListPopCntMaxDl*.
    // I wonder if we should apply cnListPopCntMaxDl* to more values of nBL.
    // For example, apply cnListPopCntMaxDl1 or cnListPopCntMaxDl2 for
    // (cnBitsInD1 < nBL <= cnListPopCntMaxDl2).
    // And/or, if COMPRESS_LISTS, apply cnListPopCntMaxDlx to all lists with
    // the same key size as that of nBL == cnBitsLeftAtDlx.
#if defined(cnListPopCntMaxDl1)
    auListPopCntMax[cnBitsInD1] = cnListPopCntMaxDl1;
#endif // defined(cnListPopCntMaxDl1)
#if defined(cnListPopCntMaxDl2)
    assert(cnBitsLeftAtDl2 > cnBitsInD1);
    auListPopCntMax[cnBitsLeftAtDl2] = cnListPopCntMaxDl2;
#endif // defined(cnListPopCntMaxDl2)
#if defined(cnListPopCntMaxDl3)
  #if !defined(cnListPopCntMaxDl2)
    #error No cnListPopCntMaxDl3 without cnListPopCntMaxDl2
  #endif // !defined(cnListPopCntMaxDl2)
    assert(cnBitsLeftAtDl3 > cnBitsLeftAtDl2);
    auListPopCntMax[cnBitsLeftAtDl3] = cnListPopCntMaxDl3;
#endif // defined(cnListPopCntMaxDl3)

#ifdef WROOT_NULL_IS_EK
    // We need a bit set in T_EMBEDDED_KEYS to maximize the size of key that
    // we can embed.
    assert(T_EMBEDDED_KEYS != 0);
#endif // WROOT_NULL_IS_EK
#ifdef SEPARATE_T_NULL
    assert(wr_nType(WROOT_NULL) == T_NULL);
#endif // SEPARATE_T_NULL

    // There may be an issue with dlmalloc and greater than 2MB (size of huge
    // page) requests. Dlmalloc may mmap something other than an integral
    // multiple of 2MB. Since our bitmaps contain an extra word at the end
    // we need to be careful about bitmaps that 2MB plus one word and bigger.
#ifdef BITMAP
    assert((cnBitsLeftAtDl2 < 24)
        || ((cn2dBmMaxWpkPercent == 0) && (cnBitsInD1 < 24)));
#endif // BITMAP
#if defined(UA_PARALLEL_128)
    assert(cnBitsMallocMask >= 4);
    for (int i = 1; i <= 6; i++) {
        if (ListWordCnt(i, 16) != 3) {
            printf("ListWordCnt(%d, 16) %d\n",
                   i, ListWordCnt(i, 16));
        }
        assert(ListWordCnt(i, 16) == 3);
    }
    assert(ListWordCnt(7, 16) > 3);
#endif // defined(UA_PARALLEL_128)
#if defined(NO_TYPE_IN_XX_SW)
  #if ! defined(REVERSE_SORT_EMBEDDED_KEYS)
    assert(T_EMBEDDED_KEYS != 0); // see b.h
  #endif // ! defined(REVERSE_SORT_EMBEDDED_KEYS)
#endif // defined(NO_TYPE_IN_XX_SW)
#ifndef BITMAP
    assert(cnListPopCntMaxDl1 >= EXP(cnBitsInD1));
    // What if cnBitsInD1 < sizeof(Link_t)*8 and -DALLOW_EMBEDDED_BITMAP?
#endif // BITMAP

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
    if (cnBitsLeftAtDl2 <= cnLogBitsPerLink) {
        printf("# Warning: (cnBitsLeftAtDl2 <= cnLogBitsPerLink)"
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
    else if (cnBitsInD1 <= cnLogBitsPerLink) {
        printf("# Warning: (cnBitsInD1 <= cnLogBitsPerLink)"
                 " makes no sense.\n");
        printf("# Mabye increase cnBitsInD1 or decrease sizeof(Link_t).\n");
    }

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
      #if ! defined(LVL_IN_PP)
    // We could be a lot more creative here w.r.t. mapping our scarce type
    // values to absolute depths.  But why?  We have to look at the prefix
    // in a different word anyway.  See comments at tp_to_nDL in b.h.
    if (nDL_to_tp(cnDigitsPerWord - 1) > cnMallocMask) {
        printf("\n");
        printf("# nDL_to_tp(cnDigitsPerWord   %2d) 0x%02x\n",
               cnDigitsPerWord, nDL_to_tp(cnDigitsPerWord));
        printf("# tp_to_nDL(cnMallocMask    0x%02x)   %2d\n",
               (int)cnMallocMask, (int)tp_to_nDL(cnMallocMask));
    }
      #endif // ! defined(LVL_IN_PP)
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
         nDL < sizeof(anDL_to_nBW) / sizeof(anDL_to_nBW[0]);
         nDL++)
    {
        anDL_to_nBW[nDL] = nBW_from_nDL(nDL);
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

#ifdef OLD_LIST_WORD_CNT
    printf("#    OLD_LIST_WORD_CNT\n");
#else // OLD_LIST_WORD_CNT
    printf("# No OLD_LIST_WORD_CNT\n");
#endif // #endif OLD_LIST_WORD_CNT

#if defined(FAST_LIST_WORDS)
    printf("#    FAST_LIST_WORDS\n");
#else // defined(FAST_LIST_WORDS)
    printf("# No FAST_LIST_WORDS\n");
#endif // defined(FAST_LIST_WORDS)

#if defined(FULL_ALLOC)
    printf("#    FULL_ALLOC\n");
#else // defined(FULL_ALLOC)
    printf("# No FULL_ALLOC\n");
#endif // defined(FULL_ALLOC)

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

// We don't have our story straight w.r.t. EXP(cnBitsInD1) <= sizeof(Link_t)*8.
// We disallow EXP(cnBitsInD1) <= sizeof(Link_t)*8 if !ALLOW_EMBEDDED_BITMAP.
// But what does it mean to have ALLOW_EMBEDED_BITMAP and
// EXP(cnBitsInD1) <= sizeof(Link_t)*8 or EXP(cnBitsInD1) > sizeof(Link_t)*8?
// What does the current definition do for us? Allows the code to assume
// EXP(cnBitsInD1) > sizeof(Link_t)*8 if !ALLOW_EMBEDDED_BITMAP. Can we not
// fashon a complile-time test for this and obviate the need for the current
// definition of ALLOW_EMBEDDED_BITMAP?
// Is (EXP(cnBitsInD1) > sizeof(Link_t)*8) a compile-time test?
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

#if defined(WORD_ALIGNED_VECTORS)
    printf("#    WORD_ALIGNED_VECTORS\n");
#else // defined(WORD_ALIGNED_VECTORS)
    printf("# No WORD_ALIGNED_VECTORS\n");
#endif // defined(WORD_ALIGNED_VECTORS)

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

#if defined(BITMAP)
    printf("#    BITMAP\n");
#else // defined(BITMAP)
    printf("# No BITMAP\n");
#endif // defined(BITMAP)

#if defined(SKIP_TO_BITMAP)
    printf("#    SKIP_TO_BITMAP\n");
#else // defined(SKIP_TO_BITMAP)
    printf("# No SKIP_TO_BITMAP\n");
#endif // defined(SKIP_TO_BITMAP)

#if defined(PACK_BM_VALUES)
    printf("#    PACK_BM_VALUES\n");
#else // defined(PACK_BM_VALUES)
    printf("# No PACK_BM_VALUES\n");
#endif // defined(PACK_BM_VALUES)

#if defined(PACK_L1_VALUES)
    printf("#    PACK_L1_VALUES\n");
#else // defined(PACK_L1_VALUES)
    printf("# No PACK_L1_VALUES\n");
#endif // defined(PACK_L1_VALUES)

#if defined(USE_LOCATE_FOR_NO_PACK)
    printf("#    USE_LOCATE_FOR_NO_PACK\n");
#else // defined(USE_LOCATE_FOR_NO_PACK)
    printf("# No USE_LOCATE_FOR_NO_PACK\n");
#endif // defined(USE_LOCATE_FOR_NO_PACK)

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

#ifdef           USE_XX_SW_AT_DLX
    printf("#    USE_XX_SW_AT_DLX\n");
#else //         USE_XX_SW_AT_DLX
    printf("# No USE_XX_SW_AT_DLX\n");
#endif //        USE_XX_SW_AT_DLX

#ifdef           USE_XX_SW_ONLY_AT_DL2
    printf("#    USE_XX_SW_ONLY_AT_DL2\n");
#else //         USE_XX_SW_ONLY_AT_DL2
    printf("# No USE_XX_SW_ONLY_AT_DL2\n");
#endif //        USE_XX_SW_ONLY_AT_DL2

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

#if defined(PARALLEL_SEARCH_WORD)
    printf("#    PARALLEL_SEARCH_WORD\n");
#else // defined(PARALLEL_SEARCH_WORD)
    printf("# No PARALLEL_SEARCH_WORD\n");
#endif // defined(PARALLEL_SEARCH_WORD)

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

#if defined(OLD_HK_64)
    printf("#    OLD_HK_64\n");
#else // defined(OLD_HK_64)
    printf("# No OLD_HK_64\n");
#endif // defined(OLD_HK_64)

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

#ifdef           GOTO_AT_FIRST_IN_LOOKUP
    printf("#    GOTO_AT_FIRST_IN_LOOKUP\n");
#else //         GOTO_AT_FIRST_IN_LOOKUP
    printf("# No GOTO_AT_FIRST_IN_LOOKUP\n");
#endif //        GOTO_AT_FIRST_IN_LOOKUP

#ifdef           ALL_SKIP_TO_SW_CASES
    printf("#    ALL_SKIP_TO_SW_CASES\n");
#else //         ALL_SKIP_TO_SW_CASES
    printf("# No ALL_SKIP_TO_SW_CASES\n");
#endif //        ALL_SKIP_TO_SW_CASES

#ifdef           DEFAULT_SKIP_TO_SW
    printf("#    DEFAULT_SKIP_TO_SW\n");
#else //         DEFAULT_SKIP_TO_SW
    printf("# No DEFAULT_SKIP_TO_SW\n");
#endif //        DEFAULT_SKIP_TO_SW

#ifdef           DEFAULT_SWITCH
    printf("#    DEFAULT_SWITCH\n");
#else //         DEFAULT_SWITCH
    printf("# No DEFAULT_SWITCH\n");
#endif //        DEFAULT_SWITCH

#ifdef           DEFAULT_LIST
    printf("#    DEFAULT_LIST\n");
#else //         DEFAULT_LIST
    printf("# No DEFAULT_LIST\n");
#endif //        DEFAULT_LIST

#ifdef           DEFAULT_BITMAP
    printf("#    DEFAULT_BITMAP\n");
#else //         DEFAULT_BITMAP
    printf("# No DEFAULT_BITMAP\n");
#endif //        DEFAULT_BITMAP

#ifdef           DEFAULT_AND_CASE
    printf("#    DEFAULT_AND_CASE\n");
#else //         DEFAULT_AND_CASE
    printf("# No DEFAULT_AND_CASE\n");
#endif //        DEFAULT_AND_CASE

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

#if defined(KISS)
    printf("#    KISS\n");
#else // defined(KISS)
    printf("# No KISS\n");
#endif // defined(KISS)

#if defined(KISS_BM)
    printf("#    KISS_BM\n");
#else // defined(KISS_BM)
    printf("# No KISS_BM\n");
#endif // defined(KISS_BM)

#if defined(PREFIX_WORD_IN_BITMAP_LEAF)
    printf("#    PREFIX_WORD_IN_BITMAP_LEAF\n");
#else // defined(PREFIX_WORD_IN_BITMAP_LEAF)
    printf("# No PREFIX_WORD_IN_BITMAP_LEAF\n");
#endif // defined(PREFIX_WORD_IN_BITMAP_LEAF)

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

#if defined(PLN_PARAM_FOR_LOOKUP)
    printf("#    PLN_PARAM_FOR_LOOKUP\n");
#else // defined(PLN_PARAM_FOR_LOOKUP)
    printf("# No PLN_PARAM_FOR_LOOKUP\n");
#endif // defined(PLN_PARAM_FOR_LOOKUP)

#if defined(LVL_IN_PP)
    printf("#    LVL_IN_PP\n");
#else // defined(LVL_IN_PP)
    printf("# No LVL_IN_PP\n");
#endif // defined(LVL_IN_PP)

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

#if defined(OFFSET_IN_SW_BM_WORD)
    printf("#    OFFSET_IN_SW_BM_WORD\n");
#else // defined(OFFSET_IN_SW_BM_WORD)
    printf("# No OFFSET_IN_SW_BM_WORD\n");
#endif // defined(OFFSET_IN_SW_BM_WORD)

#if defined(X_SW_BM_HALF_WORDS)
    printf("#    X_SW_BM_HALF_WORDS\n");
#else // defined(X_SW_BM_HALF_WORDS)
    printf("# No X_SW_BM_HALF_WORDS\n");
#endif // defined(X_SW_BM_HALF_WORDS)

#if defined(CACHE_ALIGN_BM_SW)
    printf("#    CACHE_ALIGN_BM_SW\n");
#else // defined(CACHE_ALIGN_BM_SW)
    printf("# No CACHE_ALIGN_BM_SW\n");
#endif // defined(CACHE_ALIGN_BM_SW)

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

#if defined(SEARCH_FROM_WRAPPER_I)
    printf("#    SEARCH_FROM_WRAPPER_I\n");
#else // defined(SEARCH_FROM_WRAPPER_I)
    printf("# No SEARCH_FROM_WRAPPER_I\n");
#endif // defined(SEARCH_FROM_WRAPPER_I)

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

#if defined(__clang__)
    printf("#    __clang__\n");
#else // defined(__clang__)
    printf("# No __clang__\n");
#endif // defined(__clang__)

#if defined(GCC_VECTORS)
    printf("#    GCC_VECTORS\n");
#else // defined(GCC_VECTORS)
    printf("# No GCC_VECTORS\n");
#endif // defined(GCC_VECTORS)

#if defined(NO_USE_XX_SW)
    printf("#    NO_USE_XX_SW\n");
#else // defined(NO_USE_XX_SW)
    printf("# No NO_USE_XX_SW\n");
#endif // defined(NO_USE_XX_SW)

#if defined(NO_PSPLIT_SEARCH_WORD)
    printf("#    NO_PSPLIT_SEARCH_WORD\n");
#else // defined(NO_PSPLIT_SEARCH_WORD)
    printf("# No NO_PSPLIT_SEARCH_WORD\n");
#endif // defined(NO_PSPLIT_SEARCH_WORD)

#if defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("#    NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#else // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)
    printf("# No NO_BL_SPECIFIC_PSPLIT_SEARCH\n");
#endif // defined(NO_BL_SPECIFIC_PSPLIT_SEARCH)

#if defined(NO_PARALLEL_SEARCH_WORD)
    printf("#    NO_PARALLEL_SEARCH_WORD\n");
#else // defined(NO_PARALLEL_SEARCH_WORD)
    printf("# No NO_PARALLEL_SEARCH_WORD\n");
#endif // defined(NO_PARALLEL_SEARCH_WORD)

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

#ifdef           NO_POP_WORD
    printf("#    NO_POP_WORD\n");
#else //         NO_POP_WORD
    printf("# No NO_POP_WORD\n");
#endif // #else  NO_POP_WORD

#if defined(NO_EMBED_KEYS)
    printf("#    NO_EMBED_KEYS\n");
#else // defined(NO_EMBED_KEYS)
    printf("# No NO_EMBED_KEYS\n");
#endif // defined(NO_EMBED_KEYS)

#if defined(NO_BITMAP)
    printf("#    NO_BITMAP\n");
#else // defined(NO_BITMAP)
    printf("# No NO_BITMAP\n");
#endif // defined(NO_BITMAP)

#if defined(NO_PACK_BM_VALUES)
    printf("#    NO_PACK_BM_VALUES\n");
#else // defined(NO_PACK_BM_VALUES)
    printf("# No NO_PACK_BM_VALUES\n");
#endif // defined(NO_PACK_BM_VALUES)

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
    printf("# With POP_WORD_IN_LINK unless NO_SKIP_AT_TOP?\n");
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
    printf("# cnListPopCntMax24 %d\n", cnListPopCntMax24);
    printf("# cnListPopCntMax32 %d\n", cnListPopCntMax32);
#if (cnBitsPerWord > 32)
    printf("# cnListPopCntMax40 %d\n", cnListPopCntMax40);
    printf("# cnListPopCntMax48 %d\n", cnListPopCntMax48);
    printf("# cnListPopCntMax56 %d\n", cnListPopCntMax56);
#if defined(cnListPopCntMax64)
    printf("# cnListPopCntMax64 %d\n", cnListPopCntMax64);
#endif // defined(cnListPopCntMax64)
#endif // (cnBitsPerWord > 32)
    printf("\n");
#if defined(cnListPopCntMaxDl1)
    printf("# cnListPopCntMaxDl1 %d\n", cnListPopCntMaxDl1);
#else // defined(cnListPopCntMaxDl1)
    printf("# cnListPopCntMaxDl1 n/a\n");
#endif // #else defined(cnListPopCntMaxDl1)
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
#ifdef BITMAP
    printf("# cn2dBmMaxWpkPercent %d\n", cn2dBmMaxWpkPercent);
#endif // BITMAP
#ifdef CODE_BM_SW
    printf("# cnBmSwConvert %d\n", cnBmSwConvert);
    printf("# cnBmSwRetain %d\n", cnBmSwRetain);
    printf("# N_WORDS_SWITCH_BM %d\n", N_WORDS_SWITCH_BM);
#endif // CODE_BM_SW

#ifndef OLD_LIST_WORD_CNT
    for (int nLogBytesPerKey = cnLogBytesPerWord;
             nLogBytesPerKey >= 0;
           --nLogBytesPerKey)
    {
        int nBL = EXP(3 + nLogBytesPerKey);
        int nMallocPrev = 0; (void)nMallocPrev;
        int nPopCntPrev = 0; (void)nPopCntPrev;
        printf("\n");
        for (int nPopCnt = 1; nPopCnt <= auListPopCntMax[nBL]; ++nPopCnt) {
            int nMalloc = ListWordCnt(nPopCnt, nBL);
            if (nMalloc > nMallocPrev) {
                if ((nMallocPrev != 0) && (nPopCntPrev != nPopCnt - 1)) {
                    printf("# BL %2d", nBL);
                    printf("   Pop %3d", nPopCnt - 1);
                    printf("   Min %3d", ListWordsMin(nPopCnt - 1, nBL));
                    printf("   Words %3d", nMallocPrev);
                    printf("   SlotCnt %3d", ListSlotCnt(nPopCnt - 1, nBL));
                    printf("\n");
                }
                printf("# BL %2d", nBL);
                printf("   Pop %3d", nPopCnt);
                printf("   Min %3d", ListWordsMin(nPopCnt, nBL));
                printf("   Words %3d", nMalloc);
                printf("   SlotCnt %3d", ListSlotCnt(nPopCnt, nBL));
                printf("\n");
                nPopCntPrev = nPopCnt;
            }
            nMallocPrev = nMalloc;
        }
    }
#endif // #ifndef OLD_LIST_WORD_CNT

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
        for (int nPopCnt = 1;
             nBoundaries <= 3 && nPopCnt <= auListPopCntMax[nBL];
             nPopCnt++)
        {
            if ((nWords = ListWordCnt(nPopCnt, nBL)) != nWordsPrev) {
                ++nBoundaries;
                if (nWordsPrev != 0) {
                    printf("# ListWordCnt(nBL %2d, nPopCnt %3d) %3d\n",
                           nBL, nPopCnt - 1,
                           ListWordCnt(nPopCnt - 1, nBL));
                }
                printf("# ListWordCnt(nBL %2d, nPopCnt %3d) %3d\n",
                       nBL, nPopCnt, nWords);
                nWordsPrev = nWords;
            }
        }
    }

    printf("\n");
    printf("# cnDummiesInList %d\n", cnDummiesInList);
    printf("# cnDummiesInSwitch %d\n", cnDummiesInSwitch);
    printf("# cnDummiesInBmSw %d\n", cnDummiesInBmSw);
    printf("# cnDummiesInLink %d\n", cnDummiesInLink);
#if defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords %d\n", cnMallocExtraWords);
#else // defined(cnMallocExtraWords)
    printf("# cnMallocExtraWords n/a\n");
#endif // defined(cnMallocExtraWords)

    printf("\n# cnBWAtTop %d\n", cnBWAtTop);
    for (int dd = 1; dd <= cnDigitsPerWord; dd++) {
        printf("# nDL_to_nBW(%d) %d\n", dd, nDL_to_nBW(dd));
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
#if defined(BITMAP)
    printf("# 0x%x %-20s\n", T_BITMAP, "T_BITMAP");
  #if defined(SKIP_TO_BITMAP)
    printf("# 0x%x %-20s\n", T_SKIP_TO_BITMAP, "T_SKIP_TO_BITMAP");
  #endif // defined(SKIP_TO_BITMAP)
#endif // defined(BITMAP)
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
  #ifdef SKIP_TO_BM_SW
    printf("# 0x%x %-20s\n", T_SKIP_TO_FULL_BM_SW, "T_SKIP_TO_FULL_BM_SW");
  #endif // SKIP_TO_BM_SW
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
#ifdef B_JUDYL
JudyLFreeArray(PPvoid_t PPArray, PJError_t PJError)
#else // B_JUDYL
Judy1FreeArray(PPvoid_t PPArray, PJError_t PJError)
#endif // B_JUDYL
{
    (void)PJError; // suppress "unused parameter" compiler warnings

    DBGR(printf("# JudyLFreeArray\n"));

    // A real user shouldn't pass NULL to Judy1FreeArray.
    // Judy1LHTime uses NULL to give us an opportunity to print
    // configuration info into the log file before we start testing.
    if (PPArray == NULL) { Initialize(); return 0; }

    if (*PPArray == 0) { return 0; }

#if (cnDigitsPerWord != 1)

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    Word_t *pwRoot = (Word_t*)PPArray;
    assert(pwRoot[-1] == 0);
    assert(pwRoot[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

  #if defined(DEBUG)
    Word_t wMallocsBefore = wMallocs; (void)wMallocsBefore;
    Word_t wWordsAllocatedBefore = wWordsAllocated;
    (void)wWordsAllocatedBefore;
      #if defined(RAMMETRICS)
    Word_t j__AllocWordsTOTBefore = j__AllocWordsTOT;
    (void)j__AllocWordsTOTBefore;
    Word_t j__RequestedWordsTOTBefore = j__RequestedWordsTOT;
    (void)j__RequestedWordsTOTBefore;
    Word_t j__MmapWordsTOTBefore = j__MmapWordsTOT;
    (void)j__MmapWordsTOTBefore;
      #endif // defined(RAMMETRICS)
  #endif // defined(DEBUG)

    Word_t wBytes = FreeArrayGuts((Word_t *)PPArray, /* wPrefix */ 0,
                                  cnBitsPerWord, /* bDump */ 0
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                                , /*pwrUp*/ NULL, /*nBW*/ 0
  #ifdef CODE_BM_SW
                                , /* nTypeUp */ -1
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                                  );

    DBGR(printf("# wPopCntTotal %" _fw"u 0x%" _fw"x\n",
               wPopCntTotal, wPopCntTotal));
#ifdef B_JUDYL
    DBGR(printf("# bPopCntTotalIsInvalid %d\n", bPopCntTotalIsInvalid));
#endif // B_JUDYL
    DBGR(printf("# Judy1FreeArray wBytes %" _fw"u words %" _fw"u\n",
               wBytes, wBytes/sizeof(Word_t)));
    DBGR(printf("# Judy1FreeArray wBytes 0x%" _fw"x words 0x%" _fw"x\n",
               wBytes, wBytes/sizeof(Word_t)));
    DBGR(printf("# wWordsAllocatedBefore %" _fw"u 0x%" _fw"x\n",
               wWordsAllocatedBefore, wWordsAllocatedBefore));
#if defined(RAMMETRICS)
    DBGR(printf("# j__AllocWordsTOTBefore %" _fw"u 0x%" _fw"x\n",
               j__AllocWordsTOTBefore, j__AllocWordsTOTBefore));
    DBGR(printf("# j__RequestedWordsTOTBefore %" _fw"u 0x%" _fw"x\n",
               j__RequestedWordsTOTBefore, j__RequestedWordsTOTBefore));
    DBGR(printf("# j__MmapWordsTOTBefore %" _fw"u\n", j__MmapWordsTOTBefore));
    DBGR(printf("# j__MmapWordsTOTBefore 0x%" _fw"x\n",
               j__MmapWordsTOTBefore));
    DBGR(printf("# Total MiB Before %" _fw"u rem %" _fw"u\n",
               j__MmapWordsTOTBefore * sizeof(Word_t) / (1024 * 1024),
               j__MmapWordsTOTBefore * sizeof(Word_t) % (1024 * 1024)));
    DBGR(printf("# Total MiB Before 0x%" _fw"x rem 0x%" _fw"x\n",
               j__MmapWordsTOTBefore * sizeof(Word_t) / (1024 * 1024),
               j__MmapWordsTOTBefore * sizeof(Word_t) % (1024 * 1024)));
#endif // defined(RAMMETRICS)
    DBGR(printf("# wMallocsBefore %" _fw"u 0x%" _fw"x\n",
               wMallocsBefore, wMallocsBefore));

    DBGR(printf("After Judy1FreeArray:\n"));
    DBGR(printf("# wWordsAllocated %" _fw"u\n", wWordsAllocated));
#if defined(RAMMETRICS)
    DBGR(printf("# j__AllocWordsTOT %" _fw"u\n", j__AllocWordsTOT));
    DBGR(printf("# j__RequestedWordsTOT %" _fw"u\n", j__RequestedWordsTOT));
    DBGR(printf("# j__MmapWordsTOT 0x%" _fw"x\n", j__MmapWordsTOT));
    DBGR(printf("# Total MiB 0x%" _fw"x rem 0x%" _fw"x\n",
               j__MmapWordsTOT * sizeof(Word_t) / (1024 * 1024),
               j__MmapWordsTOT * sizeof(Word_t) % (1024 * 1024)));
#endif // defined(RAMMETRICS)
    DBGR(printf("# wMallocs %" _fw"u\n", wMallocs));
    DBGR(printf("\n"));
    // Assertion below is not valid if MyMallocGuts is called with
    // nLogAlignment > cnBitsMallocMask.
    //assert((wWordsAllocatedBefore - wWordsAllocated)
    //    == (wBytes / sizeof(Word_t)));

    // Assuming wWordsAllocated is zero is presumptuous.
    // What if the application has more than one Judy1 array?
#if 0
#ifdef B_JUDYL
  if (!bPopCntTotalIsInvalid)
#endif // B_JUDYL
  {
    assert(wWordsAllocated == 0);
#if defined(RAMMETRICS)
    // Assuming j__AllocWordsTOT is zero is presumptuous.
    // What if the application has more than one Judy1 or JudyL array, e.g.
    // Judy1LHTime with -1L or Judy1LHCheck?
    //assert(j__AllocWordsTOT == 0);
    // Dlmalloc doesn't necessarily unmap everything even if we free it.
    //assert(j__MmapWordsTOT == 0);
#endif // defined(RAMMETRICS)
    // Assuming wMallocs is zero is presumptuous.
    // What if the application has more than one Judy1 array?
    assert(wMallocs == 0);
  }
#endif

    // Should have FreeArrayGuts adjust wPopCntTotal this as it goes.
#ifdef B_JUDYL
    assert(JudyLCount(*PPArray, 0, (Word_t)-1, NULL) == 0);
#else // B_JUDYL
    assert(Judy1Count(*PPArray, 0, (Word_t)-1, NULL) == 0);
#endif // B_JUDYL
    wPopCntTotal = 0; // What if there is more than one Judy1 array?

  // Judy1LHTime and Judy1LHCheck put a zero word before and after the root
  // word of the array. Let's make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(pwRoot[-1] == 0);
    assert(pwRoot[ 1] == 0);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

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
#ifdef B_JUDYL
JudyLCount(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, JError_t *pJError)
#else // B_JUDYL
Judy1Count(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, JError_t *pJError)
#endif // B_JUDYL
{
    DBGC(printf("Judy1Count(wKey0 " Owx" wKey1 " Owx")\n", wKey0, wKey1));
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
    Link_t *pLn = STRUCT_OF(&wRoot, Link_t, ln_wRoot);
    // Count returns the number of keys before the specified key.
    // It does not include the specified key.
    Word_t wCount0 = (wKey0 == 0) ? 0 : Count(cnBitsPerWord, pLn, wKey0);
    DBGC(printf("Count wKey0 " OWx" Count0 %" _fw"d\n", wKey0, wCount0));
    Word_t wCount1 = (wKey1 == 0) ? 0 : Count(cnBitsPerWord, pLn, wKey1);
    DBGC(printf("Count wKey1 " OWx" Count1 %" _fw"d\n", wKey1, wCount1));
    Word_t wCount = wCount1 - wCount0;
#ifdef B_JUDYL
    PPvoid_t ppvTest = JudyLGet(PArray, wKey1, NULL); (void)ppvTest;
    DBGC(printf("ppvTest %p\n", (void *)ppvTest));
    wCount += (ppvTest != NULL);
#else // B_JUDYL
    int bTest = Judy1Test(PArray, wKey1, NULL); (void)bTest;
    DBGC(printf("bTest %d\n", bTest));
    wCount += bTest;
#endif // B_JUDYL
    DBGC(printf("Judy1Count will return wCount %" _fw"d\n", wCount));

#if 0
    if ((wKey0 == 0) && (wKey1 == (Word_t)-1)) {
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
                // ls_wPopCnt is valid at top for PP_IN_LINK
                // and POP_WORD_IN_LINK.
                wPopCnt = PWR_xListPopCnt(&wRoot, pwr, cnBitsPerWord);
            }
        }
        else // ! tp_bIsSwitch(nType)
  #endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)
        { // tp_bIsSwitch(nType)
  #if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            wPopCnt = Sum(&wRoot, cnBitsPerWord);
  #else // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            wPopCnt = GetPopCnt(&wRoot, cnBitsPerWord);
  #endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        }

  #if defined(DEBUG)
        if ((wPopCnt != wPopCntTotal)
      #ifdef B_JUDYL
            && !bPopCntTotalIsInvalid
      #endif // B_JUDYL
            )
        {
            printf("\nAssertion error debug:\n");
            printf("\nwPopCnt %" _fw"u wPopCntTotal %" _fw"u\n",
                   wPopCnt, wPopCntTotal);
            if (wPopCntTotal < 0x1000) {
                Dump(pwRootLast, 0, cnBitsPerWord);
            }
        }
      #ifdef B_JUDYL
        assert(wPopCnt == wPopCntTotal || bPopCntTotalIsInvalid);
      #else // B_JUDYL
        assert(wPopCnt == wPopCntTotal);
      #endif // B_JUDYL

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
#endif // 0

    DBGC(printf("Judy1Count returning wCount %" _fw"d\n", wCount));
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
         Word_t *pwKey, Word_t wSkip,
         int bPrev, int bEmpty /* , Word_t **ppwVal */)
{
#define A(_zero) assert(_zero)
    (void)bEmpty;
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot;
    DBGN(printf("NextGuts(wRoot " OWx" nBL %d *pwKey " OWx
                    " wSkip %" _fw"d bPrev %d bEmpty %d)\n",
                wRoot, nBL, *pwKey, wSkip, bPrev, bEmpty));
    Word_t *pwr;
    Word_t *pwBitmap; (void)pwBitmap;
    int nBitNum; (void)nBitNum; // BITMAP
#ifdef ALLOW_EMBEDDED_BITMAP
    if (nBL <= cnLogBitsPerLink) {
        pwBitmap = (Word_t*)pLn;
        nBitNum = *pwKey & MSK(cnLogBitsPerWord) & MSK(nBL);
        goto embeddedBitmap;
    }
#endif // ALLOW_EMBEDDED_BITMAP
    pwr = wr_pwr(wRoot);
    int nType = wr_nType(wRoot);
    DBGN(printf("nBL %d pLn 0x%zx pwRoot 0x%zx nType %d pwr 0x%zx\n",
                nBL, (size_t)pLn, (size_t)pwRoot, nType, (size_t)pwr));
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
        int nPos = SearchList(qy, /* nBLR */ nBL, *pwKey);
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
            int nPopCnt =
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // Is this test sufficient even if we allow skip link
            // directly to T_LIST from the top?
                (nBL < cnBitsPerWord)
                    ? (int)PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL)
                    :
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                      PWR_xListPopCnt(&wRoot, pwr, nBL);
            *pwKey |= ls_pxKeyX(pwr, nBL, nPopCnt, nPos - wSkip);
#ifdef B_JUDYL
            //*ppwVal = &ls_pxKeyX(pwr, nBL, nPopCnt)[~(nPos - wSkip)];
#endif // B_JUDYL
        } else {
            //A(0);
            if (nPos < 0) { /*A(0);*/ nPos ^= -1; }
            //A(0);
            int nPopCnt =
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
            // Is this test sufficient even if we allow skip link
            // directly to T_LIST from the top?
                (nBL < cnBitsPerWord)
                    ? (int)PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL)
                    :
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                      PWR_xListPopCnt(&wRoot, pwr, nBL);
            if (nPos + wSkip >= (Word_t)nPopCnt) {
                //A(0);
                return nPos + wSkip - ((Word_t)nPopCnt - 1);
            }
            //A(0);
            *pwKey = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
            *pwKey |= ls_pxKeyX(pwr, nBL, nPopCnt, nPos + wSkip);
#ifdef B_JUDYL
            //*ppwVal = &ls_pxKeyX(pwr, nBL, nPopCnt)[~(nPos + wSkip)];
#endif // B_JUDYL
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
        assert(wRoot != WROOT_NULL); // Our tests aren't blowing this.
        if (wRoot == WROOT_NULL) {
            return wSkip + 1;
        }
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
            *pwKey &= ~MSK(nBL); // clear low bits
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
  #ifdef BITMAP
      #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: {
        DBGN(printf("T_SKIP_TO_BITMAP\n"));
        //A(0);
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL < cnBitsPerWord)
                ? PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR)
                :
      #endif // PP_IN_LINK
                  gwBitmapPrefix(qy, nBLR);
        if (wPrefix > (*pwKey & ~MSK(nBLR))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                // A(0); // -DUSE_XX_SW_ONLY_AT_DL2
                // A(0); // b -B64 -1v -S1 -s0x1000000 
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBLR))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBLR);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        }
        //A(0);
        nBL = nBLR;
    }
      #endif // defined(SKIP_TO_BITMAP)
    case T_BITMAP: {
        nBitNum = *pwKey & MSK(nBL) & MSK(cnLogBitsPerWord);
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
        goto embeddedBitmap;
embeddedBitmap:;
        DBGN(printf("T_BITMAP *pwKey " OWx" wSkip %" _fw"u\n", *pwKey, wSkip));
        assert(nBL != cnBitsPerWord);
        int nWordNum = (*pwKey & MSK(nBL)) >> cnLogBitsPerWord;
        if (bPrev) {
            //A(0);
            Word_t wBm = pwBitmap[nWordNum];
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
                wBm = pwBitmap[nWordNum];
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
            Word_t wBm = pwBitmap[nWordNum] & ~MSK(nBitNum);
            for (;;) {
                //A(0);
                int nPopCnt = __builtin_popcountll(wBm);
                if ((Word_t)nPopCnt > wSkip) { /*A(0);*/ break; }
                //A(0);
                wSkip -= nPopCnt;
                if (++nWordNum >= (int)(
#ifdef ALLOW_EMBEDDED_BITMAP
                    (nBL <= cnLogBitsPerWord) ? 0 :
#endif // ALLOW_EMBEDDED_BITMAP
                        EXP(nBL - cnLogBitsPerWord)))
                {
                    //A(0);
                    return wSkip + 1;
                }
                //A(0);
                wBm = pwBitmap[nWordNum];
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
  #endif // BITMAP
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
#if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
        DBG((nType != T_SKIP_TO_SWITCH)
            ? printf("NextGuts: Unhandled nType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
#endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
        DBGN(printf("SKIP_TO_SW\n"));
        //A(0);
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
#ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
#endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix > (*pwKey & ~MSK(nBLR))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                //A(0); b-32 -1ICvdgi --splay-key-bits=0xff00ffff
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBLR))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBLR);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        }
        //A(0);
        nBL = nBLR;
        goto t_switch; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_LINKS)
    case T_SWITCH: {
        goto t_switch;
t_switch:;
        //A(0);
        DBGN(printf("T_SW wSkip %" _fw"u\n", wSkip));
        int nBits = nBL_to_nBW(nBL); // bits decoded by switch
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
#ifdef B_JUDYL
// How is *pwKey getting set here?
// Prior to a recursive call?
// For which recursive calls do we need to set *ppwVal?
#endif // B_JUDYL
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
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
        if (wPrefix > (*pwKey & ~MSK(nBLR))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                // A(0); // -DUSE_XX_SW_ONLY_AT_DL2 -DcnListPopCntMax64=1
                // A(0); // b -W0 -Llv -n1e3 -S1 -s0x1000000
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBLR))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBLR);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        }
        //A(0);
        nBL = nBLR;
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(CODE_BM_SW)
    case T_BM_SW: {
        //A(0); // check -B17
        DBGN(printf("T_BM_SW wSkip %" _fw"u\n", wSkip));
        int nBits = nBL_to_nBW(nBL); // bits decoded by switch
        Word_t *pwBmWords = PWR_pwBm(pwRoot, pwr);
        int nLinks = 0;
        for (int nn = 0; nn < N_WORDS_SWITCH_BM; nn++) {
            nLinks += __builtin_popcountll(pwBmWords[nn]
  #if defined(OFFSET_IN_SW_BM_WORD)
                                   & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
  #endif // defined(OFFSET_IN_SW_BM_WORD)
                                           );
        }
        Link_t *pLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t wIndex = (*pwKey >> (nBL-nBits)) & MSK(nBits);
        int nBmWordNum = gnWordNumInSwBm(wIndex);
        int nBmBitNum = gnBitNumInSwBmWord(wIndex);
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
  #ifdef BM_SW_FOR_REAL
                assert(wPopCnt != 0);
  #endif // BM_SW_FOR_REAL
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
// remember to abstract for structured wBmWord
                    nBmBitNum = LOG(wBmWord);
                    DBGN(printf("T_BM_SW prev link nBmBitNum %d\n",
                                nBmBitNum));
                } else {
                    //A(0); // check -B17
                    nBmBitNum = 0;
                    while (--nBmWordNum >= 0) {
                        //A(0); // check -B17
                        wBmWord = pwBmWords[nBmWordNum];
  #if defined(OFFSET_IN_SW_BM_WORD)
                        // clear cumulative offset bits
                        wBmWord &= ((Word_t)1 << (cnBitsPerWord / 2)) - 1;
  #endif // defined(OFFSET_IN_SW_BM_WORD)
                        if (wBmWord != 0)
                        {
                            //A(0); // check -B17
                            assert(pLn >= pLinks);
// remember to abstract for structured wBmWord
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
                        wIndex
                            = (nBmWordNum << (nBits - LOG(N_WORDS_SWITCH_BM)))
                                + nBmBitNum;

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
                wIndex
                    = (nBmWordNum << (nBits - LOG(N_WORDS_SWITCH_BM)))
                        + nBmBitNum;
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
  #if defined(OFFSET_IN_SW_BM_WORD)
            // mask off cumulative offset bits
            wBmWord &= ((Word_t)1 << (cnBitsPerWord / 2)) - 1;
  #endif // defined(OFFSET_IN_SW_BM_WORD)
            //A(0); // check -B17
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
  #ifdef BM_SW_FOR_REAL
                assert(wPopCnt != 0);
  #endif // BM_SW_FOR_REAL
                if (wPopCnt != 0) {
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wIndex 0x%" _fw"x wBmSwIndex 0x%"
                                _fw"x pLn->ln_wRoot " OWx"\n",
                                wIndex, wBmSwIndex, pLn->ln_wRoot));
                    DBGN(printf("T_BM_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if ((wPopCnt > wSkip) /*|| (*pwKey & MSK(nBL - nBits))*/) {
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
// remember to abstract for structured wBmWord
                    nBmBitNum = __builtin_ctzll(wBmWord);
                    DBGN(printf("T_BM_SW next link"
                                " nBmWordNum %d nBmBitNum %d\n",
                                nBmWordNum, nBmBitNum));
                } else {
                    //A(0); // check -B17
                    nBmBitNum = 0;
                    while (++nBmWordNum < N_WORDS_SWITCH_BM) {
                        //A(0); // check -B17
                        wBmWord = pwBmWords[nBmWordNum];
  #if defined(OFFSET_IN_SW_BM_WORD)
                        // clear cumulative offset bits
                        wBmWord &= ((Word_t)1 << (cnBitsPerWord / 2)) - 1;
  #endif // defined(OFFSET_IN_SW_BM_WORD)
                        if (wBmWord != 0) {
                            //A(0); // check -B17
// remember to abstract for structured wBmWord
                            nBmBitNum = __builtin_ctzll(wBmWord);
                            DBGN(printf("T_BM_SW next link"
                                        " nBmWordNum %d nBmBitNum %d\n",
                                        nBmWordNum, nBmBitNum));
                            break;
                        }
                        //A(0); // check -B33 -S1
                    }
                    //A(0); // check -B17
                    DBGN(printf("bNext T_BM_SW no more links"
                                " nBmWordNum %d wIndex " OWx" *pwKey " OWx"\n",
                                nBmWordNum, wIndex, *pwKey));
                    if (nBmWordNum == N_WORDS_SWITCH_BM) {
                        if (*pwKey & ~MSK(nBL)) {
                            return wSkip + 1;
                        }
                    }
                }
                //A(0); // check -B17
                wIndex
                    = (nBmWordNum << (nBits - LOG(N_WORDS_SWITCH_BM)))
                        + nBmBitNum;
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
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix > (*pwKey & ~MSK(nBLR))) {
            //A(0); // check -B16 -S1
            if (bPrev) {
                // A(0); // -DUSE_XX_SW_ONLY_AT_DL2 -DcnBitsPerDigit=2
                // A(0); // -DcnBitsInD1=6 -DcnListPopCntMax64=1
                // A(0); // b -W0 -1lv -n1e3 -S1 -s0x1000000
                return wSkip + 1;
            } else {
                //A(0); -B16 -S1
                *pwKey = wPrefix;
            }
            //A(0); // check -B16 -S1
        } else if (wPrefix < (*pwKey & ~MSK(nBLR))) {
            //A(0);
            if (bPrev) {
                //A(0);
                *pwKey = wPrefix | MSK(nBLR);
            } else {
                //A(0); // check -B16
                return wSkip + 1;
            }
            //A(0);
        } else {
            //A(0);
            assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        }
        //A(0);
        nBL = nBLR;
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        //A(0);
        DBGN(printf("T_SW wSkip %" _fw"u\n", wSkip));
        int nBits = pwr_nBW(pwRoot); // bits decoded by switch
        Word_t wIndex = (*pwKey >> (nBL-nBits)) & MSK(nBits);
        if (bPrev) {
            //A(0);
            for (; wIndex != (Word_t)-1; wIndex--) {
                //A(0);
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                // Should pLn be NULL for PP_IN_LINK or POP_WORD_IN_LINK
                // and nBLUp == cnBitsPerWord?
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex " OWx" pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
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
                // Should pLn be NULL for PP_IN_LINK or POP_WORD_IN_LINK
                // and nBLUp == cnBitsPerWord?
                Word_t wPopCnt = GetPopCnt(&pLn->ln_wRoot, nBL - nBits);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x pLn->ln_wRoot "
                                OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
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
#ifdef B_JUDYL
PPvoid_t
JudyLByCount(Pcvoid_t PArray, Word_t wCount, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1ByCount(Pcvoid_t PArray, Word_t wCount, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    if (pwKey == NULL) {
#ifndef B_JUDYL
        int ret = -1;
#endif // B_JUDYL
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxBC: je_Errno %d\n", PJError->je_Errno));
        }
#ifdef B_JUDYL
        return NULL;
#else // B_JUDYL
        DBGN(printf("JxBC: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
#endif // B_JUDYL
    }
    *pwKey = 0;
    DBGN(printf("JxBC: wCount %" _fw"d *pwKey " OWx"\n", wCount, *pwKey));
    Word_t wKey = *pwKey;
    // The Judy1 man page specifies that wCount == 0 is reserved for
    // specifying the last key in a fully populated array.
    --wCount; // Judy API spec is off-by-one IMHO
    wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                      wCount, /* bPrev */ 0, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("JxBC: *pwKey " OWx"\n", *pwKey));
#ifdef B_JUDYL
        return JudyLGet(PArray, wKey, NULL);
#endif // B_JUDYL
    }
#ifdef B_JUDYL
    return NULL;
#else // B_JUDYL
    return wCount == 0;
#endif // B_JUDYL
}

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
#ifdef B_JUDYL
PPvoid_t
JudyLFirst(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1First(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    if (pwKey == NULL) {
#ifndef B_JUDYL
        int ret = -1;
#endif // B_JUDYL
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxF: je_Errno %d\n", PJError->je_Errno));
        }
#ifdef B_JUDYL
        return NULL;
#else // B_JUDYL
        DBGN(printf("JxF: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
#endif // B_JUDYL
    }
    DBGN(printf("\nJxF: *pwKey " OWx"\n", *pwKey));
    Word_t wKey = *pwKey;
    Word_t wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                             /* wCount */ 0, /* bPrev */ 0, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("JxF: *pwKey " OWx"\n", *pwKey));
#ifdef B_JUDYL
        return JudyLGet(PArray, wKey, NULL);
#endif // B_JUDYL
    }
#ifdef B_JUDYL
    return NULL;
#else // B_JUDYL
    DBGN(printf("JxF: returning %d\n", wCount == 0));
    return wCount == 0;
#endif // B_JUDYL
}

// Find the next bigger key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
#ifdef B_JUDYL
PPvoid_t
JudyLNext(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1Next(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    Word_t wKeyLocal, *pwKeyLocal;
#ifdef B_JUDYL
    PPvoid_t ppvVal;
#endif // B_JUDYL
    if (pwKey != NULL) {
        wKeyLocal = *pwKey + 1;
        if (wKeyLocal == 0) {
#ifdef B_JUDYL
            return NULL;
#else // B_JUDYL
            return 0; // What about PJError?
#endif // B_JUDYL
        }
        pwKeyLocal = &wKeyLocal;
    } else {
        pwKeyLocal = NULL;
    }
#ifdef B_JUDYL
    ppvVal = JudyLFirst(PArray, pwKeyLocal, PJError);
    if (ppvVal != NULL)
#else // B_JUDYL
    int ret = Judy1First(PArray, pwKeyLocal, PJError);
    if (ret == 1)
#endif // B_JUDYL
    {
        *pwKey = wKeyLocal;
    }
#ifdef B_JUDYL
    return ppvVal;
#else // B_JUDYL
    return ret;
#endif // B_JUDYL
}

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
#ifdef B_JUDYL
PPvoid_t
JudyLLast(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1Last(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    if (pwKey == NULL) {
#ifndef B_JUDYL
        int ret = -1;
#endif // B_JUDYL
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxL: je_Errno %d\n", PJError->je_Errno));
        }
#ifdef B_JUDYL
        return NULL;
#else // B_JUDYL
        DBGN(printf("JxL: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
#endif // B_JUDYL
    }
    DBGN(printf("\nJxL: *pwKey " OWx"\n", *pwKey));
    Word_t wKey = *pwKey;
    Word_t wCount = NextGuts((Word_t *)&PArray, cnBitsPerWord, &wKey,
                             /* wCount */ 0, /* bPrev */ 1, /* bEmpty */ 0);
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("JxL done: *pwKey " OWx"\n", *pwKey));
#ifdef B_JUDYL
        return JudyLGet(PArray, wKey, NULL);
#endif // B_JUDYL
    }
    DBGN(printf("JxL: returning %d\n", wCount == 0));
#ifdef B_JUDYL
    return NULL;
#else // B_JUDYL
    return wCount == 0;
#endif // B_JUDYL
}

// Find the next smaller key than *pwKey which is in the array.
// Put the resulting key in *pwKey on return.
// Return 1 if a key is found.
// Return 0 if no key is found.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey on error.
#ifdef B_JUDYL
PPvoid_t
JudyLPrev(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1Prev(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    Word_t wKeyLocal, *pwKeyLocal;
#ifdef B_JUDYL
    PPvoid_t ppvVal;
#endif // B_JUDYL
    if (pwKey != NULL) {
        if (*pwKey == 0) {
#ifdef B_JUDYL
            return NULL;
#else // B_JUDYL
            return 0; // What about PJError?
#endif // B_JUDYL
        }
        wKeyLocal = *pwKey - 1;
        pwKeyLocal = &wKeyLocal;
    } else {
        pwKeyLocal = NULL;
    }
#ifdef B_JUDYL
    ppvVal = JudyLLast(PArray, pwKeyLocal, PJError);
    if (ppvVal != NULL) {
        *pwKey = wKeyLocal;
    }
    return ppvVal;
#else // B_JUDYL
    int ret = Judy1Last(PArray, pwKeyLocal, PJError);
    if (ret == 1) {
        *pwKey = wKeyLocal;
    }
    return ret;
#endif // B_JUDYL
}

// If *pwKey is not in the array return Success and leave *pwKey unchanged.
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
    DBGN(printf("NextEmptyGuts(pwRoot %p *pwKey %p nBL %d bPrev %d)"
                    " wRoot %p\n",
                (void *)pwRoot, (void *)*pwKey, nBL, bPrev, (void *)wRoot));
    int nBLPrev = nBL; // test this in t_sw to determine if there was a skip
    Word_t *pwr;
    int nBitNum; (void)nBitNum; // BITMAP
    Word_t *pwBitmap; (void)pwBitmap;
#ifdef ALLOW_EMBEDDED_BITMAP
    if (nBL <= cnLogBitsPerLink) {
        pwBitmap = (Word_t*)pLn;
        nBitNum = *pwKey & MSK(cnLogBitsPerWord) & MSK(nBL);
        goto embeddedBitmap;
    }
#endif // ALLOW_EMBEDDED_BITMAP
    pwr = wr_pwr(wRoot);
    int nIncr;
    int nType = wr_nType(wRoot);
    //printf("\nnType 0x%x\n", nType);
    switch (nType) {
#if defined(UA_PARALLEL_128)
    case T_LIST_UA:
        goto t_list;
#endif // defined(UA_PARALLEL_128)
    case T_LIST:; {
        goto t_list;
t_list:;
        // skip over the list if it is full pop
        if (nBL < cnBitsPerWord) {
            if (GetPopCnt(pwRoot, nBL) == EXP(nBL)) {
                // printf("list is a full-pop expanse\n");
                return Failure;
            }
        }
        int nPos;
        if ((pwr == NULL)
                || ((nPos = SearchList(qy, /* nBLR */ nBL, *pwKey)) < 0)) {
            //printf("key is not in list\n");
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
        int nPopCnt =
#if defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
        // Is this test sufficient even if we allow skip link
        // directly to T_LIST from the top?
            (nBL < cnBitsPerWord)
                ? (int)PWR_wPopCntBL(pwRoot, (Switch_t *)NULL, nBL)
                :
#endif // defined(PP_IN_LINK) || defined(POP_WORD_IN_LINK)
                  PWR_xListPopCnt(&wRoot, pwr, nBL);
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
                    //printf("ran out of expanse in list\n");
                    //printf("*pwKey 0x%zx nBL %d\n", *pwKey, nBL);
                    return Failure;
                }
            }
            Word_t wKeyList = ls_pxKeyX(pwr, nBL, nPopCnt, nPos);
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
        if ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
            && (wRoot == WROOT_NULL))
        {
            return Success;
        }
        Word_t wKeyLast = *pwKey & ~MSK(nBL); // prefix
        if (bPrev) {
            nIncr = -1;
        } else {
            wKeyLast |= MSK(nBL);
            nIncr = 1;
        }
        while (SearchEmbeddedX(&wRoot, *pwKey, nBL) >= 0) {
            if (*pwKey == wKeyLast) {
                return Failure;
            }
            *pwKey += nIncr;
        }
        return Success;
    }
  #endif // defined(EMBED_KEYS)
  #ifdef BITMAP
      #if defined(SKIP_TO_BITMAP)
    case T_SKIP_TO_BITMAP: {
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL < cnBitsPerWord)
                ?  PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR)
                :
      #endif // PP_IN_LINK
                  gwBitmapPrefix(qy, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        nBLPrev = nBL;
        nBL = nBLR;
    }
      #endif // defined(SKIP_TO_BITMAP)
    case T_BITMAP:; {
        if ((wr_nType(WROOT_NULL) == T_BITMAP) && (wRoot == WROOT_NULL)) {
            return Success;
        }
        nBitNum = *pwKey & MSK(nBL) & MSK(cnLogBitsPerWord);
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
        goto embeddedBitmap;
embeddedBitmap:;
        // skip over the bitmap if it is full pop
        if (GetPopCnt(pwRoot, nBL) == EXP(nBL)) {
            assert(nBL == nBLPrev); // skip to bitmap
            return Failure;
        }
        int nWordNum = (*pwKey & MSK(nBL)) >> cnLogBitsPerWord;
// Word and bit will always be the last ones in the expanse for any
// recursive call.
        if (bPrev) {
            Word_t wBm = ~pwBitmap[nWordNum];
            if (nBitNum < cnBitsPerWord - 1) {
// Will always be true because nBitNum has already been masked with
// cnBitsPerWord - 1.
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
                    Word_t wPrefix
                        = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
                    if ((nBLPrev == cnBitsPerWord) && (wPrefix != 0)) {
                        *pwKey -= EXP(nBL);
                        *pwKey |= MSK(nBL);
                        return Success;
                    }
                    return Failure;
                }
                wBm = ~pwBitmap[nWordNum];
            }
        } else {
            // invert bits so empty looks full then keep high bits
            Word_t wBm = ~pwBitmap[nWordNum] & ~MSK(nBitNum);
            if (nBL < cnLogBitsPerWord) {
                wBm &= MSK(EXP(nBL));
            }
            for (;;) {
                if (wBm != 0) {
                    nBitNum = __builtin_ctzll(wBm);
                    *pwKey = (*pwKey & ~MSK(nBL))
                           | (nWordNum << cnLogBitsPerWord) | nBitNum;
                    return Success;
                }
                if (++nWordNum >= (int)(
#ifdef ALLOW_EMBEDDED_BITMAP
                    (nBL <= cnLogBitsPerWord) ? 0 :
#endif // ALLOW_EMBEDDED_BITMAP
                        EXP(nBL - cnLogBitsPerWord)))
                {
                    Word_t wPrefix
                        = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
                    if ((nBLPrev == cnBitsPerWord)
                        && (wPrefix != ((Word_t)-1 << nBL)))
                    {
                        *pwKey += EXP(nBL);
                        *pwKey &= ~MSK(nBL);
                        return Success;
                    }
                    return Failure;
                }
                wBm = ~pwBitmap[nWordNum];
            }
        }
    }
    assert(0);
  #endif // BITMAP
  #ifdef SEPARATE_T_NULL
    case T_NULL:
        return Success;
  #endif // SEPARATE_T_NULL
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
        int nType = wr_nType(wRoot); (void)nType;
        DBG((nType != T_SKIP_TO_SWITCH)
            ? printf("NextGuts: Unhandled nType: %d\n", nType) : 0);
        assert(nType == T_SKIP_TO_SWITCH);
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
#ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
#endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        nBLPrev = nBL;
        nBL = nBLR;
        goto t_switch; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_LINKS)
    case T_SWITCH: {
        goto t_switch;
t_switch:;
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            return Success;
        }
        // skip over the switch if it is full pop
        if (nBL < cnBitsPerWord) {
            if (GetPopCnt(pwRoot, nBL) == EXP(nBL)) {
                if (nBL != nBLPrev) {
                    //printf("full skip to switch is not at cnBitsPerWord\n");
                    // We skipped bits to get here.
                    Word_t wKey = *pwKey;
                    if (bPrev) wKey -= EXP(nBL); else wKey += EXP(nBL);
                    if (((wKey ^ *pwKey) & ~MSK(nBLPrev)) == 0) {
                        // There is at least one empty link in a
                        // skipped virtual switch.
                        *pwKey = wKey;
                        return Success;
                    }
                    //printf("nBLPrev %d *pwKey 0x%zx nBL %d wKey 0x%zx\n",
                    //        nBLPrev, *pwKey, nBL, wKey);
                }
                return Failure;
            }
            //printf("switch is not at cnBitsPerWord and it is not full\n");
        } else {
            //printf("switch is at cnBitsPerWord\n");
        }
        int nBits = nBL_to_nBW(nBL); // bits decoded by switch
        Word_t wPrefix = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
        Word_t wIndex = (*pwKey >> (nBL - nBits)) & MSK(nBits);
        for (;;) {
            Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            if (NextEmptyGuts(&pLn->ln_wRoot, pwKey, nBL - nBits, bPrev)
                    == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) {
                    // No links left in switch.
                    // If we skipped all the way from the top to get to this
                    // switch and wPrefix != 0, then we have a winner.
                    if ((nBLPrev == cnBitsPerWord) && (wPrefix != 0)) {
                        // A(0); // b -1v -S1 -s65536
                        *pwKey -= EXP(nBL);
                        *pwKey |= MSK(nBL);
                        return Success;
                    }
                    // A(0); // b -1v -S1
                    return Failure;
                }
                // A(0); // b -1v -S1
                // set all bits in suffix; index and prefix are done later
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                if (++wIndex >= EXP(nBits)) {
                    // No links left in switch.
                    // If we skipped all the way from the top to get to this
                    // switch and wPrefix != (-1 << nBL) then we have a winner.
                    // A(0); // b -1v -S1
                    if ((nBLPrev == cnBitsPerWord)
                        && (wPrefix != ((Word_t)-1 << nBL)))
                    {
                        // A(0); // b -1v -S-1 -s0xffffff
                        *pwKey += EXP(nBL);
                        *pwKey &= ~MSK(nBL);
                        return Success;
                    }
                    // A(0); // b -1v -S1
                    return Failure;
                }
                // A(0); // b -1v -S1
                // set all bits in suffix; index and prefix are done later
                *pwKey = 0; // suffix
            }
            *pwKey |= wPrefix + (wIndex << (nBL - nBits));
        }
    }
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: {
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        nBL = nBLR;
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(USE_BM_SW)
    case T_BM_SW: {
        //A(0); // check -B17
        //Word_t wKey = *pwKey;
        int nBits = nBL_to_nBW(nBL); // bits decoded by switch
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
                    if ((nBLPrev == cnBitsPerWord) && (wPrefix != 0)) {
                        *pwKey -= EXP(nBL);
                        *pwKey |= MSK(nBL);
                        return Success;
                    }
                    return Failure;
                }
                //A(0); // check -B17 -S1
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                //A(0); // check -B17 -S1
                if (++wIndex >= EXP(nBits)) {
                    //A(0); // check -B17 (with SKIP_TO_BM_SW)
                    if ((nBLPrev == cnBitsPerWord)
                        && (wPrefix != ((Word_t)-1 << nBL)))
                    {
                        *pwKey += EXP(nBL);
                        *pwKey &= ~MSK(nBL);
                        return Success;
                    }
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
        int nBLR = wr_nBL(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        nBL = nBLR;
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        int nBits = pwr_nBW(pwRoot); // bits decoded by switch
        Word_t wPrefix = (nBL == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBL);
        Word_t wIndex = (*pwKey >> (nBL - nBits)) & MSK(nBits);
        for (;;) {
            Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            if (NextEmptyGuts(&pLn->ln_wRoot, pwKey, nBL - nBits, bPrev)
                    == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) {
                    if ((nBLPrev == cnBitsPerWord) && (wPrefix != 0)) {
                        *pwKey -= EXP(nBL);
                        *pwKey |= MSK(nBL);
                        return Success;
                    }
                    return Failure;
                }
                *pwKey = MSK(nBL - nBits); // suffix
            } else {
                if (++wIndex >= EXP(nBits)) {
                    if ((nBLPrev == cnBitsPerWord)
                        && (wPrefix != ((Word_t)-1 << nBL)))
                    {
                        *pwKey += EXP(nBL);
                        *pwKey &= ~MSK(nBL);
                        return Success;
                    }
                    return Failure;
                }
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
#ifdef B_JUDYL
JudyLFirstEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1FirstEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("JxFE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxFE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("JxFE: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("\nJxFE: *pwKey " OWx"\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    Status_t status = NextEmptyGuts((Word_t *)&PArray,
                                    &wKeyLocal, cnBitsPerWord, /* bPrev */ 0);
    if (status == Success) {
        *pwKey = wKeyLocal;
        DBGN(printf("JxFE: *pwKey " OWx"\n", *pwKey));
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
#ifdef B_JUDYL
JudyLNextEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1NextEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("JxNE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
#ifdef B_JUDYL
        return JudyLFirstEmpty(PArray, pwKey, PJError);
#else // B_JUDYL
        return Judy1FirstEmpty(PArray, pwKey, PJError);
#endif // B_JUDYL
    }
    DBGN(printf("JxNE: *pwKey %zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    int ret = 0;
    if (++wKeyLocal != 0) {
#ifdef B_JUDYL
        if ((ret = JudyLFirstEmpty(PArray, &wKeyLocal, PJError)) == 1)
#else // B_JUDYL
        if ((ret = Judy1FirstEmpty(PArray, &wKeyLocal, PJError)) == 1)
#endif // B_JUDYL
        {
            *pwKey = wKeyLocal;
        }
    }
    DBGN(printf("JxNE: ret %d\n", ret));
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
#ifdef B_JUDYL
JudyLLastEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1LastEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("JxLE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxLE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("JxLE: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("\nJxLE: *pwKey " OWx"\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    Status_t status = NextEmptyGuts((Word_t *)&PArray,
                                     &wKeyLocal, cnBitsPerWord, /* bPrev */ 1);
    if (status == Success) {
        *pwKey = wKeyLocal;
        DBGN(printf("JxLE: *pwKey " OWx"\n", *pwKey));
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
#ifdef B_JUDYL
JudyLPrevEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1PrevEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("JxPE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
#ifdef B_JUDYL
        return JudyLLastEmpty(PArray, pwKey, PJError);
#else // B_JUDYL
        return Judy1LastEmpty(PArray, pwKey, PJError);
#endif // B_JUDYL
    }
    DBGN(printf("JxPE: *pwKey %zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    int ret = 0;
    if (wKeyLocal-- != 0) {
#ifdef B_JUDYL
        if ((ret = JudyLLastEmpty(PArray, &wKeyLocal, PJError)) == 1)
#else // B_JUDYL
        if ((ret = Judy1LastEmpty(PArray, &wKeyLocal, PJError)) == 1)
#endif // B_JUDYL
        {
            *pwKey = wKeyLocal;
        }
    }
    DBGN(printf("JxPE: ret %d\n", ret));
    return ret;
}

Word_t
#ifdef B_JUDYL
JudyLMemUsed(Pcvoid_t PArray)
#else // B_JUDYL
Judy1MemUsed(Pcvoid_t PArray)
#endif // B_JUDYL
{
    (void)PArray;
    return 0;
}

Word_t
#ifdef B_JUDYL
JudyLMemActive(Pcvoid_t PArray)
#else // B_JUDYL
Judy1MemActive(Pcvoid_t PArray)
#endif // B_JUDYL
{
    (void)PArray;
    return 0;
}

