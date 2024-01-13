#include "b.h"

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

void
Checkpoint(qpa, const char *str)
{
    qva;
    printf("# %20s: " qafmt "\n", str, qyap);
}

static void
Log(qp, const char *str)
{
    qv;
    printf("# %20s: " qfmt "\n", str, qyp);
}

// cnExtraUnitsMax is the maximum number of whole (1<<cnBitsMallocMask)-byte
// chunks of memory included by dlmalloc in any allocation beyond the minimum
// number required to satisfy the request after adding the one preamble
// overhead word and aligning the resulting size to an integral number of
// (1<<cnBitsMallocMask)-byte chunks.
// We don't have a good characterization for cnExtraUnitsMax.
// cnExtraUnitsMax values are based on our observations.
// Are our observations limited by our own malloc request behavior?
#if (cnBitsMallocMask == 3) && (cnBitsPerWord == 32)
    #define cnExtraUnitsMax 2 // 3
#elif (cnBitsMallocMask == 4) && (cnBitsPerWord == 64)
  #ifdef CACHE_ALIGN_L1
    #define cnExtraUnitsMax 3
  #else // CACHE_ALIGN_L1
    #define cnExtraUnitsMax 2
  #endif // #else CACHE_ALIGN_L1
#else
    #define cnExtraUnitsMax 1
#endif // cnBitsMallocMask && cnBitsPerWord

#define cnBitsUsed 2 // low bits used by dlmalloc for bookkeeping

#ifdef RAMMETRICS
// nWords is words requested from JudyMalloc
static inline Word_t
AllocWords(Word_t *pw, int nWords)
{
    (void)pw; (void)nWords;
  #ifdef EXCLUDE_MALLOC_OVERHEAD
    return nWords;
  #else // EXCLUDE_MALLOC_OVERHEAD
    // We are lucky that MacOS and Linux libc mallocs are similar to dlmalloc.
    // I wonder if we'll be so lucky with Windows.
      #if 1 // !defined(LIBCMALLOC) || defined(__linux__)
    ASSERT(cnBitsMallocMask >= cnLogBytesPerWord);
    assert((int)(pw[-1] >> cnLogBytesPerWord)
        <= nWords + cnGuardWords + 1
            + (cnExtraUnitsMax << (cnBitsMallocMask - cnLogBytesPerWord)));
    // All of our mallocs are way less than 2MB.
    assert(((pw[-1] & (~(Word_t)0x1fffff | cnMallocMask)) >> cnBitsUsed) == 0);
    return pw[-1] >> cnLogBytesPerWord;
      #else // !defined(LIBCMALLOC) || defined(__linux__)
    return nWords;
      #endif //#else !defined(LIBCMALLOC) || defined(__linux__)
  #endif // EXCLUDE_MALLOC_OVERHEAD
}
#endif // RAMMETRICS

  #ifdef FAST_MALLOC
static Word_t* apwFreeBufs[130];
  #endif // FAST_MALLOC

#define cnExtraUnitsBits 2 // number of bits used for saving alloc size

static int
CheckMalloc(Word_t* pw, Word_t wWords, int nLogAlignment)
{
    (void)pw; (void)wWords; (void)nLogAlignment;
  #ifdef FAST_MALLOC
    if ((pw[-1] & ~0x1fffff) != 0) { return -1; }
  #else // FAST_MALLOC
      #ifndef LIBCMALLOC
    if (!(pw[-1] & 2)) { return -1; }
      #if !defined(LIST_POP_IN_PREAMBLE) || defined(B_JUDYL)
          #ifdef MY_MALLOC_ALIGN
    if (nLogAlignment > cnBitsMallocMask) {
        Word_t wOff = pw[-1]; // number of bytes
        wWords += 1 << (nLogAlignment - cnLogBytesPerWord);
        pw = (Word_t*)((Word_t)pw - wOff);
    }
          #endif // MY_MALLOC_ALIGN
    size_t zUnitsRequired
        = ALIGN_UP(wWords + cnMallocExtraWords + cnGuardWords + 1,
                   EXP(cnBitsMallocMask - cnLogBytesPerWord))
            >> (cnBitsMallocMask - cnLogBytesPerWord);
    // Restore the value expected by dlmalloc.
    size_t zExtraUnits = (pw[-1] >> cnBitsUsed) & MSK(cnExtraUnitsBits);
    size_t zUnitsAllocated = zUnitsRequired + zExtraUnits;
    // Validate our assumption that twiddled bits haven't changed.
    // If the assertion blows then it's a bad assumption or we have
    // a corruption.
    if ((pw[-1] & ~MSK(cnExtraUnitsBits + cnBitsUsed))
        != ((zUnitsAllocated << cnBitsMallocMask)
            ^ ((Word_t)-1 << (cnExtraUnitsBits + cnBitsUsed))))
    {
        return -1;
    }
      #endif // !LIST_POP_IN_PREAMBLE || B_JUDYL
      #endif // !LIBCMALLOC
  #endif // FAST_MALLOC else
    return 0; // success
}

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
static inline Word_t
MyMallocGutsRM(Word_t wWords, int nLogAlignment, Word_t *pwAllocWords)
{
    (void)pwAllocWords; // RAMMETRICS
    Word_t ww, wOff;

  #ifdef FAST_MALLOC
    assert(nLogAlignment <= cnBitsMallocMask);
    int nUnits = (wWords + 2) >> 1;
    if (nUnits < 2) { nUnits = 2; }
    if (nUnits < (int)(sizeof(apwFreeBufs)/sizeof(apwFreeBufs[0]))) {
        if (apwFreeBufs[nUnits] != NULL) {
            ww = (Word_t)apwFreeBufs[nUnits];
            apwFreeBufs[nUnits] = (Word_t*)*(Word_t*)ww;
            //bzero((Word_t*)ww, (wWords | 1) * sizeof(Word_t));
            return ww;
        }
    }
    Word_t* pw = (Word_t*)JudyMalloc(wWords);
    assert((pw[-1] & ~0x1fffff) == 0);
    return (Word_t)pw;
  #endif // FAST_MALLOC

    if (nLogAlignment > cnBitsMallocMask) {
  #ifdef MY_MALLOC_ALIGN
        wWords += 1 << (nLogAlignment - cnLogBytesPerWord);
        ww = JudyMalloc(wWords + cnMallocExtraWords);
        wOff = ALIGN_UP(ww + 1, /* power of 2 */ 1 << nLogAlignment) - ww;
        ((Word_t*)(ww + wOff))[-1] = wOff;
  #else // MY_MALLOC_ALIGN
        ww = JudyMallocAlign(wWords + cnMallocExtraWords, nLogAlignment);
        wOff = 0;
  #endif // #else MY_MALLOC_ALIGN
    } else {
        // Check our own (or JudyMalloc's) arbitrary assumptions/constraints.
        assert(wWords + cnMallocExtraWords < (0x200000 >> cnLogBytesPerWord));
        ww = JudyMalloc(wWords + cnMallocExtraWords);
        // dlmalloc uses the low two bits of the preamble word for bookkeeping.
        // The other bits in ww[-1] & cnMallocMask should be zero.
        // And the bits representing sizes >= 2MB should be zero.
        assert(((((Word_t*)ww)[-1]
                       & (~(Word_t)0x1fffff | cnMallocMask)) >> cnBitsUsed)
                   == 0);
        wOff = 0;
    }
    DBGM(printf("\nM(%zd): %p *%p 0x%zx\n",
                wWords, (void *)ww, (void *)&((Word_t *)ww)[-1],
                ((Word_t *)ww)[-1]));

  #ifdef RAMMETRICS
    if (pwAllocWords != NULL) {
        *pwAllocWords += AllocWords((Word_t*)ww, wWords + cnMallocExtraWords);
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

    assert(cnExtraUnitsMax <= (int)MSK(cnExtraUnitsBits));
    // zExtraUnits is the number of extra EXP(cnBitsMallocMask)-byte units over
    // and above the minimum amount that could be allocated by malloc.
    size_t zExtraUnits = zUnitsAllocated - zUnitsRequired; (void)zExtraUnits;
  #ifdef DEBUG
    if (zExtraUnits > cnExtraUnitsMax) {
        printf("zExtraUnits %zd wWords %zd nLogAlignment %d ww[-1]>>3 %zx\n",
               zExtraUnits, wWords, nLogAlignment,
               ((Word_t*)ww)[-1] >> cnLogBytesPerWord);
    }
  #endif // DEBUG
    assert(zExtraUnits <= cnExtraUnitsMax);
  #ifdef LIBCMALLOC
  // We can't use the preamble word with LIBCMALLOC because it monitors the
  // preamble word for changes and kills the process if it detects a change.
  // I can't remember if this is true for Linux or MacOS.
  // LIST_POP_IN_PREAMBLE means something different for B_JUDYL so the
  // LIBCMALLOC limitation does not apply. It means use the word before pwr.
      #if defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
          #error LIST_POP_IN_PREAMBLE with LIBCMALLOC is not supported
      #endif // defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
  #else // LIBCMALLOC

    assert(!(((Word_t *)ww)[-1] & MSK(cnBitsMallocMask) & ~MSK(cnBitsUsed)));
    // Save the bits of ww[-1] that we need at free time and make sure
    // none of the bits we want to use are changed by malloc while we
    // own the buffer.
    ((Word_t *)ww)[-1] &= ~(MSK(cnExtraUnitsBits) << cnBitsUsed);
    ((Word_t *)ww)[-1] |= zExtraUnits << cnBitsUsed;
      #if defined(LIST_POP_IN_PREAMBLE) && !defined(B_JUDYL)
    // Zero the high bits for our use.
    ((Word_t *)ww)[-1] &= MSK(cnExtraUnitsBits + cnBitsUsed);
      #else // LIST_POP_IN_PREAMBLE && !B_JUDYL
    // Twiddle the bits to illustrate that we can use them.
    ((Word_t *)ww)[-1] ^= (Word_t)-1 << (cnExtraUnitsBits + cnBitsUsed);
      #endif // LIST_POP_IN_PREAMBLE && !B_JUDYL else
    DBGM(printf("ww[-1] 0x%zx\n", ((Word_t *)ww)[-1]));
  #endif // LIBCMALLOC else
    DBGM(printf("required %zd alloc %zd extra %zd\n",
                zUnitsRequired, zUnitsAllocated, zExtraUnits));
    assert(ww != 0);
    assert((ww & 0xffff000000000000UL) == 0);
    assert((ww & cnMallocMask) == 0);
    // wWordsAllocated excludes cnMallocExtraWords and malloc overhead
    ++wMallocs; wWordsAllocated += wWords;
    // ? should we keep track of sub-optimal-size requests ?
    ww += wOff; // number of bytes
    return ww;
}

static inline Word_t
MyMallocRM(Word_t wWords, Word_t* pwAllocWords)
{
    return MyMallocGutsRM(wWords, /*LogAlign*/ cnBitsMallocMask, pwAllocWords);
}

static inline void
MyFreeGutsRM(Word_t *pw, Word_t wWords, int nLogAlignment,
             Word_t* pwAllocWords)
{
    (void)pwAllocWords; // RAMMETRICS
    DBGM(printf("\nF(pw %p, wWords %zd): pw[-1] 0x%zx ...\n",
                (void *)pw, wWords, pw[-1]));

  #if 1 // example of how to use CheckMalloc
    if (CheckMalloc(pw, wWords, nLogAlignment) != 0) {
        DBGI(printf("\n# MyFreeGutsRM CheckMalloc failure.\n"));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        exit(1);
    }
    assert(CheckMalloc(pw, wWords, nLogAlignment) == 0);
  #endif
// I wonder about malloc overhead.
  #ifdef FAST_MALLOC
      #ifdef LIST_POP_IN_PREAMBLE
          #error LIST_POP_IN_PREAMBLE with FAST_MALLOC
      #endif // LIST_POP_IN_PREAMBLE
    assert((pw[-1] & ~0x1fffff) == 0);
    int nUnits = pw[-1] >> cnBitsMallocMask;
    if (nUnits < (int)(sizeof(apwFreeBufs)/sizeof(apwFreeBufs[0]))) {
      #ifdef FAST_MALLOC_1
        if ((apwFreeBufs[nUnits] == NULL)
          #ifdef FAST_MALLOC_2
            || (apwFreeBufs[nUnits][0] == 0)
          #endif // FAST_MALLOC_2
            )
      #endif // FAST_MALLOC_1
        {
            *pw = (Word_t)apwFreeBufs[nUnits];
            apwFreeBufs[nUnits] = pw;
            return;
        }
    }
    JudyFree((Word_t)pw, wWords);
    return;
  #endif // FAST_MALLOC
    (void)nLogAlignment;
  #ifdef MY_MALLOC_ALIGN
    if (nLogAlignment > cnBitsMallocMask) {
        Word_t wOff = pw[-1]; // number of bytes
        wWords += 1 << (nLogAlignment - cnLogBytesPerWord);
        pw = (Word_t*)((Word_t)pw - wOff);
    }
  #endif // MY_MALLOC_ALIGN
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
      #if !defined(LIST_POP_IN_PREAMBLE) || defined(B_JUDYL)
    // Validate our assumption that twiddled bits haven't changed.
    // If the assertion blows then it's a bad assumption or we have
    // a corruption.
    assert((pw[-1] & ~MSK(cnExtraUnitsBits + cnBitsUsed))
        == ((zUnitsAllocated << cnBitsMallocMask)
            ^ ((Word_t)-1 << (cnExtraUnitsBits + cnBitsUsed))));
      #endif // !LIST_POP_IN_PREAMBLE || B_JUDYL
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

    JudyFree((Word_t)pw, wWords + cnMallocExtraWords);
}

static inline void
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

// Max list length as a function of nBL.
// Array is indexed by nBL.
#ifndef POP_CNT_MAX_IS_KING
static
#endif // #ifndef POP_CNT_MAX_IS_KING
uListPopCntMax_t auListPopCntMax[] = {
    0,
    //  0 < nBL <=  4
    cnListPopCntMax4 , cnListPopCntMax4 , cnListPopCntMax4 , cnListPopCntMax4 ,
    //  4 < nBL <=  8
    cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 , cnListPopCntMax8 ,
    //  8 < nBL <= 12
    cnListPopCntMax12, cnListPopCntMax12, cnListPopCntMax12, cnListPopCntMax12,
    // 12 < nBL <= 16
    cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16, cnListPopCntMax16,
    // 16 < nBL <= 20
    cnListPopCntMax20, cnListPopCntMax20, cnListPopCntMax20, cnListPopCntMax20,
    // 20 < nBL <= 24
    cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24, cnListPopCntMax24,
    // 24 < nBL <= 28
    cnListPopCntMax28, cnListPopCntMax28, cnListPopCntMax28, cnListPopCntMax28,
    // 28 < nBL <= 32
    cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32, cnListPopCntMax32,
  #if (cnBitsPerWord >= 64)
    // 32 < nBL <= 36
    cnListPopCntMax36, cnListPopCntMax36, cnListPopCntMax36, cnListPopCntMax36,
    // 36 < nBL <= 40
    cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40, cnListPopCntMax40,
    // 40 < nBL <= 44
    cnListPopCntMax44, cnListPopCntMax44, cnListPopCntMax44, cnListPopCntMax44,
    // 44 < nBL <= 48
    cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48, cnListPopCntMax48,
    // 48 < nBL <= 52
    cnListPopCntMax52, cnListPopCntMax52, cnListPopCntMax52, cnListPopCntMax52,
    // 52 < nBL <= 56
    cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56, cnListPopCntMax56,
    // 56 < nBL <= 60
    cnListPopCntMax60, cnListPopCntMax60, cnListPopCntMax60, cnListPopCntMax60,
    // 60 < nBL <= 64
    cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64, cnListPopCntMax64,
  #endif // (cnBitsPerWord >= 64)
    };

static int16_t aauListWordCnt[cnBitsPerWord + 1][cwListPopCntMax + 1];
static uListPopCntMax_t aauListSlotCnt[cnBitsPerWord + 1][cwListPopCntMax + 1];

// OLD_LIST_WORD_CNT is independent of OLD_LISTS
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
    if (ALIGN_LIST_LEN(nBytesKeySz, wPopCntArg)) {
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
}

#else // OLD_LIST_WORD_CNT

// ListWordsMin returns the minimum number of words that will hold a list.
// It respects all alignment constraints.
// The return value could be passed to MyMalloc and we'd get a buffer
// big enough for the list.
// Our current approach for 64-bit JudyL is to put the value area before
// the key area with pwr pointing at the key area. One benefit of this
// approach is locating the value after finding the key is really easy.
// One disadvantage is we have to align the key area on a malloc boundary.
// What cases can we not handle yet? We assume pop is not in key area.
// We assume SKIP_TO_LIST prefix is not in key area.
#ifndef LIST_POP_IN_PREAMBLE
#ifndef POP_IN_WR_HB
  #error ListWordsMin requires LIST_POP_IN_PREAMBLE for 32-bit JudyL.
#endif // #ifndef POP_IN_WR_HB
#endif // #ifndef LIST_POP_IN_PREAMBLE
static int
ListWordsMin(int nPopCnt, int nBLR)
{
    int nBytesPerKey = ExtListBytesPerKey(nBLR);
  #ifdef B_JUDYL
  #ifndef PACK_L1_VALUES
    if ((cnBitsInD1 <= 8) && (nBLR == cnBitsInD1)) {
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
    if ((nPopCnt <= 6) && (nBLR == 16)) {
        return 3;
    }
  #endif // UA_PARALLEL_128
    int nBytesPerBucket // key allocation unit
        = ALIGN_LIST_LEN(nBytesPerKey, nPopCnt)
            ? sizeof(Bucket_t) : sizeof(Word_t);
    int nKeyBytes = ALIGN_UP(nPopCnt * nBytesPerKey, nBytesPerBucket);
    int nKeyWords = nKeyBytes / sizeof(Word_t); // accumulator
  #ifdef B_JUDYL
    int nValueWords = nPopCnt;
      #ifdef SKIP_TO_LIST
      #ifndef PP_IN_LINK
    // Make room for prefix.
    if (nBLR <= cnBitsPerDigit / 2) {
        ++nValueWords;
    }
      #endif // #ifndef PP_IN_LINK
      #endif // SKIP_TO_LIST
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
    assert((cnMallocAlignment >= cnBytesListKeysAlign)
               || !ALIGN_LIST(nBytesPerKey));
    nKeyWords += nValueWords;
#else // B_JUDYL
    // Prefix and/or pop have to be in the link or the preamble.
#endif // #else B_JUDYL
    return nKeyWords; // accumulated list words
}

  #ifdef SLOW_FIBONACCI
// Slow Fibonacci.
static int anListMallocSizes[] = {
     4,   6,   8,  10,  14,  18,  24,  32,  42,
    56,  74,  98, 130, 172, 228, 302, 400, 530  };
  #endif // SLOW_FIBONACCI

// CalcListWordCnt is for reducing the frequency of list mallocs and copies
// and the number of different sizes of malloc buffers that we would otherwise
// use if we had only ListWordsMin.
// We use sizes that are roughly the powers of the square root of two.
// CalcListWordCnt and CalcListSlotCnt have limitations.
// They may not have not been vetted with UA_PARALLEL_128.
// They have not been enhanced for header words
// LIST_END_MARKERS, !LIST_REQ_MIN_WORDS, !OLD_LISTS.
static int
CalcListWordCnt(int nPopCnt, int nBLR)
{
    int nListWordsMin = ListWordsMin(nPopCnt, nBLR);
    // Choose a malloc size for the list.
  #ifdef CACHE_ALIGN_L1
    // We need to address needing two words of keys for l1 and requiring
    // an even number of values to align pwr so we can fit two keys and
    // two values in a malloc of 3 words.
    // In the meantime nListWordsMin <= 4
    // allows us to request alignment of only 4 words and
    // know that no prefetch is necessary because we know all 4 words will
    // be in the same cache line and it doesn't result in much waste.
    // Isn't this left over from before EK_XV?
    // Requesting an alignment of 8 words results in a lot of waste.
    if ((nBLR == cnBitsInD1) && (nListWordsMin <= 4)) { return 4; }
    // Under normal ifdefs, nListWordsMin never equals 5 so why test for it.
    //assert((nBLR != cnBitsInD1) || (nListWordsMin > 5));
    //if ((nBLR == cnBitsInD1) && (nListWordsMin <= 5)) { return 5; }
    // Why are we special-casing nListWords <= 6?
    if ((nBLR == cnBitsInD1) && (nListWordsMin <= 6)) { return 6; }
  #endif // CACHE_ALIGN_L1
  #ifdef SLOW_FIBONACCI
    int n; for (n = 0; anListMallocSizes[n] <= nListWordsMin; ++n);
    int nWords = anListMallocSizes[n];
  #else // SLOW_FIBONACCI
    // We want to use powers of two (including the malloc overhead word).
    // What is the first power of two bigger than nListWordsMin?
    // It has to be bigger because we need a word for malloc overhead.
    int nWords = EXP(LOG(nListWordsMin) + 1); // first power of two bigger
      #ifndef POW_2_ALLOC
    // See if the power of two divided by the square root of two rounded
    // to an even number for malloc efficiency is big enough for our list
    // plus the malloc overhead word.
    int nWordsDivRoot2 = (nWords * 46340 / (1<<16) + 1) & ~1;
    if (nListWordsMin < nWordsDivRoot2) {
        nWords = nWordsDivRoot2;
    }
      #endif // !POW_2_ALLOC
  #endif // SLOW_FIBONACCI
    assert(!(nWords & 1));
    // Trim to avoid wasted space.
    // aulistPopCntMax[nBLR] incorporates EmbeddedListPopCntMax if
    // !POP_CNT_MAX_IS_KING.
    assert(nPopCnt <= auListPopCntMax[nBLR]);
    int nFullListWordsMin = ListWordsMin(auListPopCntMax[nBLR], nBLR);
    if (nFullListWordsMin < nWords) {
        nWords = nFullListWordsMin + 1;
    }
    // Returning less than 3 words has no memory saving benefit since malloc
    // never allocates less than 3 words.
    // Always returning at least 3 words benefits us by avoiding free/malloc on
    // Insert if CalcListWordCnt(nPopCnt) < CalcListWordCnt(nPopCnt + 1) <= 3.
    // But it can skew our requested words instrumentation to be higher than
    // necessary if ListWordsMin(nPopCnt) < 3 and ListWordsMin(nPopCnt+1) > 3.
  #ifdef LIST_REQ_MIN_WORDS // don't request words that have no benefit
    // Is it possible that nWords minus one malloc chunk holds
    // just as many keys so we're wasting a malloc chunk? Or wasting
    // more than one malloc chunk?
    // Requesting even a single word that we won't use, even if it doesn't take
    // memory from the heap, is inconsistent with LIST_REQ_MIN_WORDS.
  #endif // LIST_REQ_MIN_WORDS
    if (--nWords < 3) { // Subtract malloc overhead word to get request size.
        nWords = 3; // No sense requesting less than 3 words.
    }
    return nWords;
}

#endif // #else OLD_LIST_WORD_CNT

static int
ListWordCnt(int nPopCnt, int nBLR)
{
    assert(nBLR < (int)(sizeof(aauListWordCnt) / sizeof(aauListWordCnt[0])));
    // If the next one blows it might be because we do an imperfect job of
    // initializing _cnListPopCntMaxEK hence cwListPopCntMax.
    // aauListWordCnt has cwListPopCntMax + 1 entries.
    assert((Word_t)nPopCnt
               < sizeof(aauListWordCnt[0]) / sizeof(aauListWordCnt[0][0]));
    int nListWordCnt = aauListWordCnt[nBLR][nPopCnt];
    if (nListWordCnt == 0) {
        nListWordCnt = CalcListWordCnt(nPopCnt, nBLR);
        aauListWordCnt[nBLR][nPopCnt] = nListWordCnt;
    }
    return nListWordCnt;
}

// How many keys fit in a list buffer that must hold at least nPopCnt keys?
// We have to be able to figure out where to point pwr from nPopCnt and nBL.
// We can get list words from ListWordCnt.
// But we need to know the max capacity of that size.
static int
CalcListSlotCnt(int nPopCnt, int nBLR)
{
  #ifdef OLD_LIST_WORD_CNT
    // Looks like this variant of CalcListSlotCnt predated the implementation
    // of B_JUDYL.
    BJL(assert(0));
    int nBytesPerKey = ExtListBytesPerKey(nBL);
    int nSlots = ALIGN_UP(nPopCnt, sizeof(Bucket_t) / nBytesPerKey);
    return EXP(LOG(nSlots - 1) + 1);
  #else // OLD_LIST_WORD_CNT
      #ifdef B_JUDYL
      #ifndef PACK_L1_VALUES
    if ((cnBitsInD1 <= 8) && (nBLR == cnBitsInD1)) {
        return EXP(cnBitsInD1);
    }
      #endif // #ifndef PACK_L1_VALUES
      #endif // B_JUDYL
    int nListWords = ListWordCnt(nPopCnt, nBLR);
    int nBytesPerKey = ExtListBytesPerKey(nBLR);
    BJL(nBytesPerKey += sizeof(Word_t)); // key bytes plus value bytes
    int nListBytes = nListWords * sizeof(Word_t);
    int nListSlots = nListBytes / nBytesPerKey;
    if (nListSlots > auListPopCntMax[nBLR]) {
        nListSlots = auListPopCntMax[nBLR]; // respect ListWordCnt limitation
    }
    for (; ListWordCnt(nListSlots, nBLR) > nListWords; --nListSlots) ;
      #ifdef DEBUG
    int nListWordsX = CalcListWordCnt(nListSlots, nBLR);
    if (nListWordsX != nListWords) {
        printf("\n");
        printf("ListWordCnt(nPopCnt %d, nBLR %d) %d\n",
                nPopCnt, nBLR, nListWords);
        printf("ListSlotCnt(nPopCnt %d, nBLR %d) %d\n",
                nPopCnt, nBLR, nListSlots);
        printf("ListWordCnt(nPopCnt %d, nBLR %d) %d\n",
                nListSlots, nBLR, nListWordsX);
        printf("auListPopCntMax[nBLR %d] %d\n", nBLR, auListPopCntMax[nBLR]);
    }
    assert(nListWordsX == nListWords);
      #endif // DEBUG
    return nListSlots;
  #endif // OLD_LIST_WORD_CNT else
}

int
ListSlotCnt(int nPopCnt, int nBLR)
{
    assert(nBLR < (int)(sizeof(aauListSlotCnt) / sizeof(aauListSlotCnt[0])));
    assert(nPopCnt
           < (int)(sizeof(aauListSlotCnt[0]) / sizeof(aauListSlotCnt[0][0])));
    int nListSlotCnt = aauListSlotCnt[nBLR][nPopCnt];
    if (nListSlotCnt == 0) {
        nListSlotCnt = CalcListSlotCnt(nPopCnt, nBLR);
        assert(nListSlotCnt <= (int)MSK(sizeof(aauListSlotCnt[0][0]) * 8));
        aauListSlotCnt[nBLR][nPopCnt] = nListSlotCnt;
    }
    return nListSlotCnt;
}

#ifdef LIST_END_MARKERS
static void
NewListSetMarker(Word_t *pwList, Word_t wPopCnt, int nBL, int nWords)
{
    (void)pwList; (void)wPopCnt; (void)nBL; (void)nWords;
  #if defined(COMPRESSED_LISTS)
    if (nBL <= 8) {
        ls_pcKeysNAT(pwList)[-1] = 0;
    } else if (nBL <= 16) {
        ls_psKeysNAT(pwList)[-1] = 0;
      #if (cnBitsPerWord > 32)
    } else if (nBL <= 32) {
        ls_piKeysNAT(pwList)[-1] = 0;
      #endif // (cnBitsPerWord > 32)
    } else
  #endif // defined(COMPRESSED_LISTS)
    {
        ls_pwKeysNAT(pwList)[-1] = 0;
    }
}
#endif // LIST_END_MARKERS

// Allocate a new list leaf buffer.
// Return a pointer to the start of the key area.
// If we want to do a parallel search when doing PSPLIT_SEARCH then
// the key area must be bucket-aligned and have nothing but keys in
// the key area buckets. That is, no pop count or list end markers
// in the key area buckets.
Word_t *
NewList(Word_t wPopCnt, int nBL)
{
    assert(wPopCnt != 0);
    int nWords = ListWordCnt(wPopCnt, nBL);
    Word_t *pwList
        = (Word_t*)MyMallocGuts(nWords,
  #ifdef CACHE_ALIGN_L1
                                (nBL == cnBitsInD1) ? (nWords <= 4) ? 5 : 6 :
  #endif // CACHE_ALIGN_L1
                                0,
                                &j__AllocWordsJLL[(nBL + 7) / 8
                                                      & (sizeof(Word_t) - 1)]);
  #ifdef B_JUDYL
    //pwList = (Word_t*)ALIGN_UP((Word_t)&pwList[nKeySlots], sizeof(Bucket_t));
    int nKeySlots = ListSlotCnt(wPopCnt, nBL);
      #ifdef LIST_POP_IN_PREAMBLE
    ++nKeySlots; // make room for list pop count
      #endif // LIST_POP_IN_PREAMBLE
    pwList = (Word_t*)ALIGN_UP((Word_t)&pwList[nKeySlots], cnMallocAlignment);
  #endif // B_JUDYL
  #ifdef LIST_END_MARKERS
    NewListSetMarker(pwList, wPopCnt, nBL, nWords);
  #endif // LIST_END_MARKERS
    return pwList; // pwr
}

int
OldList(Word_t *pwList, int nPopCnt, int nBLR, int nType)
{
    (void)nType;
    int nWords = ListWordCnt(nPopCnt, nBLR);
  #ifdef B_JUDYL
    int nKeySlots = ListSlotCnt(nPopCnt, nBLR);
      #ifdef LIST_POP_IN_PREAMBLE
    ++nKeySlots; // make room for list pop count
      #endif // LIST_POP_IN_PREAMBLE
    pwList = (Word_t*)((Word_t)&pwList[-nKeySlots] & ~cnMallocMask);
  #endif // B_JUDYL
    MyFreeGuts(pwList, nWords,
  #ifdef CACHE_ALIGN_L1
               (nBLR == cnBitsInD1) ? (nWords <= 4) ? 5 : 6 :
  #endif // CACHE_ALIGN_L1
               0,
               &j__AllocWordsJLL[(nBLR + 7) / 8 & (sizeof(Word_t) - 1)]);
    return nWords * sizeof(Word_t);
}

#endif // (cwListPopCntMax != 0)

#ifdef BITMAP

#ifdef GUARDBAND
#define CheckBitmapGuardband(_pwr, _nBLR, _wPopCnt) \
    BJL(assert(pwr[BitmapWordCnt((_nBLR), (_wPopCnt))] == ~(Word_t)(_pwr)))
#else // GUARDBAND
#define CheckBitmapGuardband(_pwr, _nBLR, _wPopCnt)
#endif // else GUARDBAND

// We don't need NewBitmap for Judy1 unless (cnBitsInD1 > cnLogBitsPerLink).
static Word_t *
NewBitmap(qpa, int nBLR, Word_t wKey, Word_t wPopCnt)
{
    qva; (void)wKey;
    Word_t wWords = BitmapWordCnt(nBLR, wPopCnt);
  #ifndef SKIP_TO_BITMAP
    assert(nBL == nBLR);
  #endif // #ifndef SKIP_TO_BITMAP

    Word_t *pwBitmap
        = (Word_t *)MyMalloc(wWords,
  #if (cn2dBmMaxWpkPercent != 0)
                             (nBLR == nDL_to_nBL(2))
                                 ? &j__AllocWordsJLB2 :  // DL2 big bitmap leaf
  #endif // (cn2dBmMaxWpkPercent != 0)
                                   &j__AllocWordsJLB1    // bitmap leaf
                             );

    DBGM(printf("NewBitmap nBLR %d wPopCnt %zd nBits " OWx
      " nBytes in bitmap " OWx" wWords in leaf " OWx" pwBitmap " OWx"\n",
        nBLR, wPopCnt, EXP(nBLR), EXP(nBLR - cnLogBitsPerByte), wWords,
        (Word_t)pwBitmap));

    memset((void *)pwBitmap, 0, wWords * sizeof(Word_t));

    // *pwLnX may be used for different things based on ifdefs.
    // We want to zero it for some cases and not for others.
    #ifdef BMLFI_LNX
        #ifdef _LNX
    *pwLnX = 0;
        #endif // _LNX
    #else // BMLF_INTERLEAVE_LNX
        #ifdef _BMLF_BM_IN_LNX
    *pwLnX = 0;
        #endif // _BMLF_BM_IN_LNX
    #endif // BMLFI_LNX else

    // Init wRoot before calling set_wr because set_wr may try to preserve
    // the high bits, e.g. if LVL_IN_WR_HB, so we want them to
    // be initialized.
    wRoot = 0; set_wr(wRoot, pwBitmap, T_BITMAP);
  #ifdef B_JUDYL
    // We don't really need T_UNPACKED_BM for 1-digit bitmap if
    // BitmapWordCnt(cnBitsInD1, cnPopCntMaxDl1)
    //     == BitmapWordCnt(cnBitsInD1, EXP(cnBitsInD1))
    // Because we'll never have a packed bm even if PACK_BM_VALUES is defined.
    // But that is too hard to ifdef so we go ahead and use T_UNPACKED_BM even
    // though it adds no information assuming it doesn't cause any harm.
    // I suppose we could have separate UNPACK and PACK macros for 1-digit
    // and 2-digit bitmaps. We don't have 2-digit bitmaps for JudyL yet.
    // Later.
  #ifdef UNPACK_BM_VALUES
      #ifdef _TEST_BM_UNPACKED
    // Test to make T_UNPACKED_BM depends on ALLOC_WHOLE_PACKED_BMLF_EXP.
    // The rest of our code uses T_UNPACKED_BM.
          #ifdef ALLOC_WHOLE_PACKED_BMLF_EXP
    if (wPopCnt > EXP(nBLR) * (EXP(cnLogBmWordsX+1)+1) / EXP(cnLogBmWordsX+2))
          #else // ALLOC_WHOLE_PACKED_BMLF_EXP
    if (wWords == BitmapWordCnt(nBLR, EXP(nBLR)))
          #endif // ALLOC_WHOLE_PACKED_BMLF_EXP
      #endif // _TEST_BM_UNPACKED
    {
        set_wr_nType(wRoot, T_UNPACKED_BM);
    }
    DBGM(printf("NewBitmap wRoot 0x%zx\n", wRoot));
  #endif // UNPACKED_BM_VALUES
  #endif // B_JUDYL

    *pwRoot = wRoot;
#if defined(SKIP_TO_BITMAP)
    if (nBL != nBLR) {
      #ifdef BMLF_CNTS_IN_LNX
        assert(nBL != cnBitsPerWord);
      #endif // BMLF_CNTS_IN_LNX
        set_wr_nType(*pwRoot, T_SKIP_TO_BITMAP);
        snBLR(pwRoot, nBLR);
        wRoot = *pwRoot;
    }
#endif // defined(SKIP_TO_BITMAP)
    nType = wr_nType(wRoot);
    pwr = wr_pwr(wRoot);
#if defined(SKIP_TO_BITMAP)
    swBitmapPrefix(qy, nBLR, wKey);
#endif // defined(SKIP_TO_BITMAP)
    swBitmapPopCnt(qya, /* nBLR */ nBLR, wPopCnt);

    return pwBitmap;
}

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

#ifdef BM_SW_FOR_REAL

// Set the bit in the BmSw bitmap indicating that a link exists.
static void
SetBitInBmSwBm(qpa, int nDigit, int nBW)
{
    qva; (void)nBW;
    int nn = gnWordNumInSwBm(nDigit);
  #ifdef BM_SW_BM_IN_LNX // implies a bunch of things
    assert(nn == 0);
      #ifdef NO_BM_SW_AT_TOP
    assert(nBL < cnBitsPerWord);
      #else // NO_BM_SW_AT_TOP
    if (nBL < cnBitsPerWord)
      #endif // NO_BM_SW_AT_TOP else
    {
      #ifdef USE_BM_SW_BM_IN_LNX
        assert(*pwLnX == *PWR_pwBm(pwRoot, pwr, nBW));
      #endif // USE_BM_SW_BM_IN_LNX
        *pwLnX |= gwBitMaskInSwBmWord(nDigit);
    }
  #elif defined(BM_SW_BM_IN_WR_HB)
      #ifdef USE_BM_SW_BM_IN_WR_HB
    assert((wRoot >> cnBitsVirtAddr) == *PWR_pwBm(pwRoot, pwr, nBW));
      #endif // USE_BM_SW_BM_IN_WR_HB
    *pwRoot |= gwBitMaskInSwBmWord(nDigit) << cnBitsVirtAddr;
  #endif // BM_SW_BM_IN_LNX
    PWR_pwBm(pwRoot, pwr, nBW)[nn] |= gwBitMaskInSwBmWord(nDigit);
  #ifdef OFFSET_IN_SW_BM_WORD
    // UpdateOffsetsInSwBmWords
    while (++nn < N_WORDS_SW_BM(nBW)) {
        PWR_pwBm(pwRoot, pwr, nBW)[nn] += (Word_t)1 << (cnBitsPerWord / 2);
    }
  #endif // OFFSET_IN_SW_BM_WORD
}

// Set the bit in the BmSw bitmap indicating that a link exists.
static void
ClrBitInBmSwBm(qpa, int nDigit, int nBW)
{
    qva; (void)nBW;
    int nn = gnWordNumInSwBm(nDigit);
  #ifdef BM_SW_BM_IN_LNX
    assert(nn == 0);
      #ifdef NO_BM_SW_AT_TOP
    assert(nBL < cnBitsPerWord);
      #else // NO_BM_SW_AT_TOP
    if (nBL < cnBitsPerWord)
      #endif // NO_BM_SW_AT_TOP else
    {
      #ifdef USE_BM_SW_BM_IN_LNX
        assert(*pwLnX == *PWR_pwBm(pwRoot, pwr, nBW));
      #endif // USE_BM_SW_BM_IN_LNX
        *pwLnX &= ~gwBitMaskInSwBmWord(nDigit);
    }
  #elif defined(BM_SW_BM_IN_WR_HB)
      #ifdef USE_BM_SW_BM_IN_WR_HB
    assert((wRoot >> cnBitsVirtAddr) == *PWR_pwBm(pwRoot, pwr, nBW));
      #endif // USE_BM_SW_BM_IN_WR_HB
    *pwRoot &= ~gwBitMaskInSwBmWord(nDigit) << cnBitsVirtAddr;
  #endif // BM_SW_BM_IN_LNX
    PWR_pwBm(pwRoot, pwr, nBW)[nn] &= ~gwBitMaskInSwBmWord(nDigit);
  #ifdef OFFSET_IN_SW_BM_WORD
    // UpdateOffsetsInSwBmWords
    while (++nn < N_WORDS_SW_BM(nBW)) {
        PWR_pwBm(pwRoot, pwr, nBW)[nn] -= (Word_t)1 << (cnBitsPerWord / 2);
    }
  #endif // OFFSET_IN_SW_BM_WORD
}

#endif // BM_SW_FOR_REAL

// Allocate a new switch.
// Zero its links.
// Initialize its prefix if there is one.  Need to know pre-skip nBL for
// PP_IN_LINK to figure out if the prefix field exists.
// Initialize its bitmap if there is one.  Need to know pre-skip nBL for
// BM_IN_LINK to figure out if the bitmap field exists.
// Need to know pre-skip nBL to know if we need a skip link.
// Install wRoot at pwRoot.
// Account for the memory in Judy1LHTime.
// Need to know if we are at the bottom so we can count the memory as a
// bitmap leaf instead of a switch.
static Word_t *
NewSwitchX(qpa, Word_t wKey, int nBLR,
  #if defined(CODE_XX_SW)
           int nBWX,
  #endif // defined(CODE_XX_SW)
           int nTypeBase,
  #ifdef BM_SW_FOR_REAL
           int nLinkGrpCnt,
  #endif // BM_SW_FOR_REAL
           Word_t wPopCnt)
{
    qva;
    wRoot = *pwRoot = 0;
  #ifdef _LNX
    if (nBL < cnBitsPerWord) {
// This is new. For bm in lnx. I wonder if we have any old code which depends
// on NewSwitchX not mucking with the link extension.
// Like TransformList or Splay or DoubleIt or DoubleDown or DoubleUp or
// NewLink or InsertAtPrefixMismatch or converting a 2-digit bitmap to 1-digit
// or RemoveCleanup.
        *pwLnX = 0;
    }
  #endif // _LNX

    nType = nTypeBase; // Co-opt parameter. Used only if CODE_BM_SW?
    DBGI(printf("NewSwitch: pwRoot %p wKey " OWx" nBLR %d nType %d nBL %d"
                    " wPopCnt %" _fw"d.\n",
                (void *)pwRoot, wKey, nBLR, nType, nBL, wPopCnt));
  #ifdef CODE_BM_SW
      #if defined(BM_IN_LINK) || defined(NO_BM_SW_AT_TOP)
          #ifdef SMART_NEW_SWITCH
    if ((nType == T_BM_SW) && (nBL == cnBitsPerWord)) {
        nType = T_SWITCH;
    }
          #else // SMART_NEW_SWITCH
    assert((nType != T_BM_SW) || (nBL != cnBitsPerWord));
          #endif // SMART_NEW_SWITCH else
      #endif // BM_IN_LINK || NO_BM_SW_AT_TOP
  #endif // CODE_BM_SW
#if defined(CODE_XX_SW)
      #ifdef CODE_BM_SW
    if ((nType == T_BM_SW)
        && ((nDL_to_nBL(nBL_to_nDL(nBL)) != nBL)
            || (nDL_to_nBL(nBL_to_nDL(nBLR)) != nBLR)
            || (nBWX != nBLR_to_nBW(nBLR))))
    {
        nType = T_SWITCH;
    }
      #endif // CODE_BM_SW
    if ((nBWX != nBLR_to_nBW(nBLR)) /*&& (nBWX != cnBWMin)*/) {
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
    int nBW = nBLR_to_nBW(nBLR);
#endif // defined(CODE_XX_SW)
    Word_t wIndexCnt = EXP(nBW);

#ifndef USE_XX_SW_ONLY_AT_DL2
    assert(nBLR - nBW >= cnBitsInD1);
#endif // #ifndef USE_XX_SW_ONLY_AT_DL2

#if defined(CODE_BM_SW)
  #ifdef BM_SW_CNT_IN_WR
    // Make sure link count fits in the field.
    assert(nBW <= cnBitsCnt);
  #endif // BM_SW_CNT_IN_WR
    if (nType == T_BM_SW) {
        assert((int)(wIndexCnt >> cnLogBmSwLinksPerBit)
                   <= N_WORDS_SW_BM(nBW) * cnBitsPerWord
  #ifdef OFFSET_IN_SW_BM_WORD
                       / 2
  #endif // OFFSET_IN_SW_BM_WORD
               );
    }
#endif // defined(CODE_BM_SW)

    Word_t wLinks = wIndexCnt;

#if defined(BM_SW_FOR_REAL)
    if (nType == T_BM_SW) {
        wLinks = nLinkGrpCnt << cnLogBmSwLinksPerBit; // # of links
    }
#endif // defined(BM_SW_FOR_REAL)
    DBGI(printf("wLinks %zd\n", wLinks));

// cnMaskLsSwDL is for testing.
#if defined(USE_LIST_SW)
  #if !defined(cnMaskLsSwDL)
    #define cnMaskLsSwDL  0
  #endif // !defined(cnMaskLsSwDL)
    int bLsSw = ((1 << nBL_to_nDL(nBLR)) & cnMaskLsSwDL);
#endif // defined(USE_LIST_SW)

    Word_t wBytes =
#if defined(CODE_BM_SW)
        (nType == T_BM_SW)
            ? sizeof(BmSwitch_t)
  #ifndef BM_IN_LINK
                + ALIGN_UP(N_WORDS_SW_BM(nBW) * sizeof(Word_t),
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
  #if defined(REMOTE_LNX) || defined(DUMMY_REMOTE_LNX)
    wWords += wLinks;
  #endif // REMOTE_LNX || DUMMY_REMOTE_LNX

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
    pwr = (Word_t*)MyMalloc(wWords, pwAllocWords); // Co-opt parameter.
#endif // CACHE_ALIGN_BM_SW
    DBGI(printf("pwr %p wWords %zd\n", pwr, wWords));
#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
    if (nType == T_BM_SW) {
        pwr += ALIGN_UP(N_WORDS_SW_BM(nBW),
                        cnMallocAlignment >> cnLogBytesPerWord);
    }
#endif // defined(CODE_BM_SW) && !defined(BM_IN_LINK)
    DBGI(printf("pwr %p\n", pwr));

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
    if (cbEmbeddedBitmap && ((nBLR - nBW) <= cnLogBitsPerLink)) {
        // Zero the links, i.e. bitmaps. The links contain no type field.
        memset(pLinks, 0, sizeof(Link_t) * wLinks);
    } else {
        if (WROOT_NULL != 0) {
            for (int nn = 0; nn < (int)wLinks; ++nn) {
                pLinks[nn].ln_wRoot = WROOT_NULL;
            }
        } else {
            // This will initialize local link extensions unconditionally.
            bzero(pLinks, sizeof(Link_t) * wLinks);
        }
  // Is it really worth the complexity of avoiding the work of initializing
  // the remote link extensions when it is not necessary?
  #ifdef _BMLF_BM_IN_LNX
  #ifndef CHECK_TYPE_FOR_EBM
  #if defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
  #ifndef UNPACK_BM_VALUES
  #ifdef REMOTE_LNX
        bzero(&pLinks[wLinks], sizeof(Word_t) * wLinks);
  #else // REMOTE_LNX
        if (WROOT_NULL != 0) {
            for (int nn = 0; nn < (int)wLinks; ++nn) {
                if ((cnListPopCntMaxDl1 == 0) && (nBLR == cnBitsLeftAtDl2)) {
                    pLinks[nn].ln_wX = 0;
                }
            }
        }
  #endif // REMOTE_LNX else
  #endif // !UNPACK_BM_VALUES
  #endif // defined(POP_CNT_MAX_IS_KING) || !defined(EMBED_KEYS)
  #endif // !CHECK_TYPE_FOR_EBM
  #endif // _BMLF_BM_IN_LNX
    }
#if defined(CODE_BM_SW)
    DBGM(printf("NewSwitch(pwRoot %p wKey " OWx
                    " nBLR %d nType %d nBL %d wPopCnt %ld)"
                    " pwr %p\n",
                (void *)pwRoot, wKey,
                nBLR, nType, nBL, (long)wPopCnt, (void *)pwr));
#endif // defined(CODE_BM_SW)
    DBGI(printf("\nNewSwitch nBLR %d nDL %d nBL %d\n",
                nBLR, nBL_to_nDL(nBLR), nBL));
#if defined(CODE_BM_SW)
    if (nType == T_BM_SW) {
  #if defined(SKIP_TO_BM_SW)
        if (nBLR != nBL) {
            DBGI(printf("\nCreating T_SKIP_TO_BM_SW!\n"));
            set_wr(*pwRoot, pwr, T_SKIP_TO_BM_SW); // set type
            snBLRSw(pwRoot, nBLR); // set nBL
        } else
  #endif // defined(SKIP_TO_BM_SW)
        { set_wr(*pwRoot, pwr, T_BM_SW); }
  #if (cnBitsPerWord > 32)
  #ifdef BM_SW_CNT_IN_WR
        SetBits(pwRoot, cnBitsCnt, cnLsbCnt, wLinks - 1);
  #endif // BM_SW_CNT_IN_WR
  #endif // (cnBitsPerWord > 32)
    } else
#endif // defined(CODE_BM_SW)
    {
        set_wr_pwr(*pwRoot, pwr);
#if defined(NO_SKIP_AT_TOP)
        assert((nBL < cnBitsPerWord) || (nBLR == nBL));
#endif // defined(NO_SKIP_AT_TOP)
        assert(nBLR <= nBL);
#if defined(SKIP_LINKS)
        if (nBLR < nBL) {
            set_wr_nType(*pwRoot, T_SKIP_TO_SWITCH);
            snBLRSw(pwRoot, nBLR);
  #if defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
          #ifndef USE_XX_SW_ALWAYS
            if (nBW != nBLR_to_nBW(nBLR))
          #endif // #ifndef USE_XX_SW_ALWAYS
            {
                set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
            }
  #else // defined(USE_XX_SW) && defined(SKIP_TO_XX_SW)
            assert(nBW == nBLR_to_nBW(nBLR));
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
          #ifdef USE_XX_SW_ALWAYS
            if (1)
          #else // USE_XX_SW_ALWAYS
            if (nBW != nBLR_to_nBW(nBLR))
          #endif // #else USE_XX_SW_ALWAYS
            {
                set_wr_nType(*pwRoot, T_XX_SW);
            } else
  #else // defined(USE_XX_SW)
            assert(nBW == nBLR_to_nBW(nBLR));
  #endif // defined(USE_XX_SW)
            {
                set_wr_nType(*pwRoot,
  #if defined(USE_LIST_SW)
                             bLsSw ? T_LIST_SW :
  #endif // defined(USE_LIST_SW)
                             T_SWITCH);
            }
        }
#if defined(CODE_XX_SW)
  #if defined(SKIP_TO_XX_SW)
        snBLRSw(pwRoot, nBLR);
  #endif // defined(SKIP_TO_XX_SW)
        set_pwr_nBW(pwRoot, nBW);
#endif // defined(CODE_XX_SW)
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
        if (nBL < cnBitsPerWord)
#endif // defined(BM_IN_LINK)
        {
#if defined(BM_SW_FOR_REAL)
            memset(PWR_pwBm(pwRoot, pwr, nBW), 0,
                   N_WORDS_SW_BM(nBW) * sizeof(Word_t));
            if (nLinkGrpCnt == 1) {
                Word_t wIndex = (wKey >> (nBLR - nBW)) & (wIndexCnt - 1);
                // Set bit in bitmap indicating that the link exists.
                SetBitInBmSwBm(qya, wIndex, nBW);
            }
#else // defined(BM_SW_FOR_REAL)
            // Mind the high-order bits of the bitmap word if/when the bitmap
            // is smaller than a whole word.
            // Mind endianness.
            if ((nBW - cnLogBmSwLinksPerBit) < cnLogBitsPerWord) {
                *PWR_pwBm(pwRoot, pwr, nBW)
                    = NZ_MSK(wIndexCnt >> cnLogBmSwLinksPerBit);
            } else {
                memset(PWR_pwBm(pwRoot, pwr, nBW), -(Word_t)1,
                       N_WORDS_SW_BM(nBW) * sizeof(Word_t));
            }
#endif // defined(BM_SW_FOR_REAL)
        }
    }
#endif // defined(CODE_BM_SW)

#if defined(PP_IN_LINK)
    if (nBL < cnBitsPerWord)
#endif // defined(PP_IN_LINK)
    {
#if defined(SKIP_LINKS)
        assert(nBLR <= nBL);

        {
#if defined(CODE_BM_SW)
            if (nType == T_BM_SW) {
                set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR, wKey);
            } else
#endif // defined(CODE_BM_SW)
            {
                set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR, wKey);
            }
        }
#else // defined(SKIP_LINKS)

#if 0
        // Why do we bother with this?  Should we make it debug only?
#if defined(CODE_BM_SW)
        if (nType == T_BM_SW) {
            set_PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR, 0);
        } else
#endif // defined(CODE_BM_SW)
        {
            set_PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR, 0);
        }
#endif // defined(SKIP_LINKS)
#endif

#if defined(PP_IN_LINK)
        assert(nBL < cnBitsPerWord);
#endif // defined(PP_IN_LINK)
        swPopCnt(qya, nBLR, wPopCnt);
  #ifdef FULL_SW
        if ((nBLR < cnBitsPerWord) && (wPopCnt >= BPW_EXP(nBLR))) {
            int nType = wr_nType(*pwRoot);
      #ifdef _SKIP_TO_FULL_SW
            if (nType == T_SKIP_TO_SWITCH) {
                set_wr_nType(*pwRoot, T_SKIP_TO_FULL_SW);
            } else
      #endif // _SKIP_TO_FULL_SW
            if (nType == T_SWITCH) {
                set_wr_nType(*pwRoot, T_FULL_SW);
            }
        }
  #endif // FULL_SW
    }
  #if cnSwCnts != 0
    memset(((Switch_t*)pwr)->sw_awCnts, 0,
           sizeof(((Switch_t*)pwr)->sw_awCnts)); // caller must update
  #endif // cnSwCnts != 0

  #ifdef SKIP_LINKS
    DBGI(printf("NS: prefix " OWx"\n",
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR)));
  #endif // SKIP_LINKS
    //DBGI(printf("\n# Just before returning from NewSwitch "));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return pwr;

    (void)wKey; // fix "unused parameter" compiler warning
    (void)nBLR; // nBLR is not used for all ifdef combos
    (void)nBL; // nBL is not used for all ifdef combos
}

static Word_t *
NewSwitch(qpa, Word_t wKey, int nBLR,
  #if defined(CODE_XX_SW)
          int nBWX,
  #endif // defined(CODE_XX_SW)
          int nTypeBase, Word_t wPopCnt)
{
    qva;
    return NewSwitchX(qya, wKey, nBLR,
  #if defined(CODE_XX_SW)
                      nBWX,
  #endif // defined(CODE_XX_SW)
                      nTypeBase,
  #ifdef BM_SW_FOR_REAL
                      /* nLinkGrpCnt */ 1, // ignored if nType != T_BM_SW
  #endif // BM_SW_FOR_REAL
                      wPopCnt);
}

#if defined(CODE_BM_SW)
static Word_t
OldSwitch(Word_t *pwRoot, int nBL, int bBmSw, int nLinks);

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
InflateBmSw(qpa, Word_t wKey, int nBLR)
{
    qva;

    DBGI(printf("# InflateBmSw wKey " Owx" nBLR %d nBL %d\n",
                wKey, nBLR, nBL));

    int nBW = nBLR_to_nBW(nBLR);

    Word_t *pwrNew = NewSwitch(qya, wKey, nBLR,
  #if defined(CODE_XX_SW)
                               nBW,
  #endif // defined(CODE_XX_SW)
                               T_SWITCH,
                               PWR_wPopCntBL(pwRoot,
                                             (BmSwitch_t *)pwr, nBLR));
  #if cnSwCnts != 0
    memcpy(((BmSwitch_t*)pwrNew)->sw_awCnts, ((BmSwitch_t*)pwr)->sw_awCnts,
           sizeof(((BmSwitch_t*)pwr)->sw_awCnts));
  #endif // cnSwCnts != 0
    // NewSwitch installed the new wRoot at pwRoot.
    // What about PWR_pwBm(pwRoot, pwr) which we use below?
    // If BM_IN_LINK it will use pwRoot to find the bitmap.
    // How do we know NewSwitch hasn't overwritten the bitmap?
    // I guess we should make a copy of the link before calling NewSwitch.

    Word_t *pwBm = PWR_pwBm(pwRoot, pwr, nBW);
    Link_t *pSwLinks = pwr_pLinks((Switch_t*)pwrNew);
    Link_t *pBmSwLinks = pwr_pLinks((BmSwitch_t *)pwr);
    int nLinkCnt = 0; // link number in bm sw
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
        if (pwBm[gnWordNumInSwBm(nn)] & gwBitMaskInSwBmWord(nn)) {
            pSwLinks[nn] = pBmSwLinks[nLinkCnt];
            ++nLinkCnt;
        }
    }
  #ifdef REMOTE_LNX
    Word_t *pSwValues = (Word_t*)&pSwLinks[1<<nBW];
    Word_t *pBmSwValues = (Word_t*)&pBmSwLinks[nLinkCnt];
    nLinkCnt = 0;
    for (int nn = 0; nn < (int)EXP(nBW); nn++) {
        if (pwBm[gnWordNumInSwBm(nn)] & gwBitMaskInSwBmWord(nn)) {
            pSwValues[nn] = pBmSwValues[nLinkCnt];
            ++nLinkCnt;
        }
    }
  #endif // REMOTE_LNX

    OldSwitch(&wRoot, nBL, /* bBmSw */ 1, nLinkCnt);

  #ifdef B_JUDYL
  #ifdef EMBED_KEYS
    Word_t wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
    // How is it possible for pSwLinks[wDigit].ln_wRoot to be WROOT_NULL?
    // NO_BM_SW_FOR_REAL?
    //assert(pSwLinks[wDigit].ln_wRoot != WROOT_NULL);
    if ((pSwLinks[wDigit].ln_wRoot != WROOT_NULL)
        && (wr_nType(pSwLinks[wDigit].ln_wRoot) == T_EMBEDDED_KEYS))
    {
      #ifdef REMOTE_LNX
        Word_t* pwValue = &pSwValues[wDigit];
      #else // REMOTE_LNX
        Word_t* pwValue = &pSwLinks[wDigit].ln_wX;
      #endif // else REMOTE_LNX
        DBGX(printf("InflateBmSw returning pwValue %p\n", pwValue));
        return pwValue;
    }
    // NULL means pwValue didn't change, but not that nothing changed.
    return NULL;
  #endif // EMBED_KEYS
  #endif // B_JUDYL
}

// NewLink uses OldSwitch but not NewSwitch. Could we change it?
#if defined(BM_SW_FOR_REAL)
static void
NewLink(qpa, Word_t wKey, int nDLR, int nDLUp)
{
    qva;
    int nBLR = nDL_to_nBL(nDLR);
    int nBLUp = nDL_to_nBL(nDLUp);
    assert(nBLUp == nBL); // Can I get rid of nDLUp?

    DBGI(printf("NewLink(pLn %p wKey " OWx" nBLR %d)\n", pLn, wKey, nBLR));
    DBGI(printf("PWR_wPopCnt %" _fw"d\n",
         PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR)));
    DBGI(printf("gwPopCnt %zd\n", gwPopCnt(qya, nBL)));

#if defined(BM_IN_LINK)
    assert(nBLR != cnBitsPerWord);
#endif // defined(BM_IN_LINK)

    unsigned nBW = nBLR_to_nBW(nBLR);
    Word_t wDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
    DBGI(printf("wKey " OWx" nBLR %d nBW %d wDigit " OWx"\n",
                wKey, nBLR, nBW, wDigit));

    // How many links are there in the old switch?
    int nLinkCnt = BmSwLinkCnt(qya);

    unsigned nWordsOld
         = (sizeof(BmSwitch_t)
#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
                + ALIGN_UP(N_WORDS_SW_BM(nBW) * sizeof(Word_t),
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

    // Should we uncompress here? Or rely on InsertCleanup?
    // It's probably more efficient to do it here.
    if (InflateBmSwTest(qya)) {
        InflateBmSw(qya, wKey, nBLR);
    } else {
        Word_t wIndex;
        int bLinkPresent;
        BmSwIndex(qya, wDigit, &wIndex, &bLinkPresent);
        assert(!bLinkPresent);
        // Now we know where the new link goes.
        DBGI(printf("wIndex " OWx"\n", wIndex));

        // We replicate a bunch of newswitch here since
        // newswitch can create only empty bm sw.

        // Allocate memory for a new switch with one more link than the
        // old one.
        unsigned nWordsNew = nWordsOld
            + ((sizeof(Link_t) / sizeof(Word_t)) << cnLogBmSwLinksPerBit);
        int nLinkCntNew = nLinkCnt + NBPW_EXP(cnLogBmSwLinksPerBit);
        (void)nLinkCntNew;
  #if defined(REMOTE_LNX) || defined(DUMMY_REMOTE_LNX)
        nWordsNew += nLinkCntNew;
  #endif // REMOTE_LNX || DUMMY_REMOTE_LNX
        Word_t *pwBm = PWR_pwBm(pwRoot, pwr, nBW); (void)pwBm;
#if defined(CODE_BM_SW) && defined(CACHE_ALIGN_BM_SW)
        *pwRoot = MyMallocGuts(nWordsNew, /* logAlign */ 6, &j__AllocWordsJBB);
#else // CACHE_ALIGN_BM_SW
        *pwRoot = MyMalloc(nWordsNew, &j__AllocWordsJBB);
#endif // CACHE_ALIGN_BM_SW

#ifndef BM_IN_LINK
        *pwRoot += ALIGN_UP(N_WORDS_SW_BM(nBW) * sizeof(Word_t),
                            cnMallocAlignment);
#endif // BM_IN_LINK
        DBGI(printf("After malloc *pwRoot " OWx"\n", *pwRoot));

        // Copy the old switch to the new switch and insert the new link.
        // copy header and leading links from old switch to new switch
        int nLinksBefore = (wIndex & ~NBPW_MSK(cnLogBmSwLinksPerBit));
        int nLinksAfter = nLinkCnt - nLinksBefore;
        memcpy(wr_pwr(*pwRoot), pwr,
               sizeof(BmSwitch_t) + nLinksBefore * sizeof(Link_t));
#ifndef BM_IN_LINK
        memcpy(PWR_pwBm(pwRoot, wr_pwr(*pwRoot), nBW), pwBm,
               N_WORDS_SW_BM(nBW) * sizeof(Word_t));
#endif // BM_IN_LINK
        DBGI(printf("PWR_wPopCnt %" _fw"d\n",
             PWR_wPopCntBL(pwRoot, (BmSwitch_t *)*pwRoot, nBLR)));
        Link_t *pNewLinks = pwr_pLinks((BmSwitch_t*)*pwRoot);
        Link_t *pOldLinks = pwr_pLinks((BmSwitch_t*)pwr);
  #ifdef REMOTE_LNX
        Word_t *pNewValues = (Word_t*)&pNewLinks[nLinkCntNew];
        Word_t *pOldValues = (Word_t*)&pOldLinks[nLinkCnt];
        Word_t ww;
        for (ww = 0; (int)ww < nLinksBefore; ++ww) {
            pNewValues[ww] = pOldValues[ww];
        }
        // Zero *pwLnX for the links in the new group.
        bzero(&pNewValues[ww], sizeof(Word_t) << cnLogBmSwLinksPerBit);
        for (; ww < (Word_t)nLinkCnt; ++ww) {
            pNewValues[ww + NBPW_EXP(cnLogBmSwLinksPerBit)] = pOldValues[ww];
        }
  #endif // REMOTE_LNX

        // Initialize the links in the new group.
        if (cbEmbeddedBitmap && ((nBLR - nBW) <= cnLogBitsPerLink)) {
            memset(&pNewLinks[nLinksBefore], 0,
                   sizeof(Link_t) << cnLogBmSwLinksPerBit);
        } else {
            for (int ww = 0; ww < (int)NBPW_EXP(cnLogBmSwLinksPerBit); ++ww) {
                pNewLinks[nLinksBefore + ww].ln_wRoot = WROOT_NULL;
                // What about the rest of the link, if any?
                // What does NewSwitchX do with the rest of the link?
            }
        }
        // copy trailing links from old switch to new switch
        memcpy(&pNewLinks[nLinksBefore + NBPW_EXP(cnLogBmSwLinksPerBit)],
               &pOldLinks[nLinksBefore],
               nLinksAfter * sizeof(Link_t));
  #ifdef BM_SW_BM_IN_WR_HB
        // Copy the bitmap from the old *pwRoot to the new one.
        *pwRoot |= wRoot & ~MSK(cnBitsVirtAddr);
  #endif // BM_SW_BM_IN_WR_HB
        // Set the bit in the bitmap indicating that the new link exists.
        SetBitInBmSwBm(qya, wDigit, nBW);
        // Update the type field in *pwRoot if necessary.
#if defined(SKIP_LINKS) || (cwListPopCntMax != 0)
  #if defined(RETYPE_FULL_BM_SW)
        if (nLinkCntNew == (int)EXP(nBW)) {
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
            if (wr_nType(wRoot) == T_SKIP_TO_BM_SW) {
                snBLRSw(*pwRoot, nBLR);
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
            set_wr_nType(*pwRoot, nType);
            // depth is preserved if the beginning of the switch is copied
            // in some cases
  #if defined(LVL_IN_WR_HB)
      #if defined(SKIP_TO_BM_SW)
            if (nType == T_SKIP_TO_BM_SW) {
                snBLR(pwRoot, nBLR);
                assert(tp_bIsSkip(wr_nType(*pwRoot)));
                assert(wr_nBLR(*pwRoot) == wr_nBLR(wRoot));
            }
      #endif // defined(SKIP_TO_BM_SW)
  #endif // defined(LVL_IN_WR_HB)
        }
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

  #if (cnBitsPerWord > 32)
  #ifdef BM_SW_CNT_IN_WR
        SetBits(pwRoot, cnBitsCnt, cnLsbCnt, nLinkCntNew - 1);
  #endif // BM_SW_CNT_IN_WR
  #endif // (cnBitsPerWord > 32)

        // &wRoot won't cut it for BM_IN_LINK. Really?
        OldSwitch(&wRoot, nBLUp, /* bBmSw */ 1, nLinkCnt);
    }

    //DBGI(printf("After NewLink"));
    //DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
}
#endif // defined(BM_SW_FOR_REAL)
#endif // defined(CODE_BM_SW)

static Word_t
OldSwitch(Word_t *pwRoot, int nBL
#if defined(CODE_BM_SW)
        , int bBmSw, int nLinks // 0 means calculate
#endif // defined(CODE_BM_SW)
          )
{
    Link_t *pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wRoot = *pwRoot; (void)wRoot;
    int nBLR = gnBLR(qy); (void)nBLR;
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
    { nBW = nBLR_to_nBW(nBLR); }

    Word_t wLinks = EXP(nBW);

#if defined(CODE_BM_SW)
#if defined(BM_SW_FOR_REAL)
    if (bBmSw)
    {
        if (nLinks == 0) {
  #if defined(BM_IN_LINK)
            if (nBL != cnBitsPerWord)
  #endif // defined(BM_IN_LINK)
            {
                // How many links are there in the old switch?
                wLinks = 0;
                for (int nn = 0; nn < N_WORDS_SW_BM(nBW); nn++) {
                    wLinks
                        += __builtin_popcountll(
                            PWR_pwBm(pwRoot, pwr, nBW)[nn]
  #if defined(OFFSET_IN_SW_BM_WORD)
                                & (((Word_t)1 << (cnBitsPerWord / 2)) - 1)
  #endif // defined(OFFSET_IN_SW_BM_WORD)
                                                );
                }
                wLinks <<= cnLogBmSwLinksPerBit;
                assert(wLinks <= EXP(nBLR_to_nBW(nBLR)));
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
                  + ALIGN_UP(N_WORDS_SW_BM(nBW) * sizeof(Word_t),
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
  #if defined(REMOTE_LNX) || defined(DUMMY_REMOTE_LNX)
    wWords += wLinks;
  #endif // REMOTE_LNX || DUMMY_REMOTE_LNX

#ifdef RAMMETRICS
    Word_t *pwAllocWords =  // RAMMETRICS
#if defined(CODE_BM_SW)
        bBmSw ? &j__AllocWordsJBB :  // bitmap branch
#endif // defined(CODE_BM_SW)
                &j__AllocWordsJBU ;  // uncompressed branch
#endif // RAMMETRICS

    DBGR(printf("\nOldSwitch pwRoot %p nBL %d nBLR %d wWords %"
                    _fw"d " OWx"\n",
                pwRoot, nBL, nBLR, wWords, wWords));

#if defined(CODE_BM_SW) && !defined(BM_IN_LINK)
    if (bBmSw) {
        pwr -= ALIGN_UP(N_WORDS_SW_BM(nBW),
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
}

// wKey contains the prefix bits for qpa.
// It may also contain less significant bits.
static Word_t
FreeArrayGuts(qpa, Word_t wKey, int bDump
#if defined(B_JUDYL) && defined(EMBED_KEYS)
            , Word_t *pwrUp, int nBWUp
  #ifdef CODE_BM_SW
            , int nTypeUp
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
              )
{
    qva;
  #if defined(B_JUDYL) && defined(EMBED_KEYS)
    (void)pwrUp; (void)nBWUp;
      #ifdef CODE_BM_SW
    (void)nTypeUp;
      #endif // CODE_BM_SW
  #endif // defined(B_JUDYL) && defined(EMBED_KEYS)
    Word_t wKeyOrig = wKey; (void)wKeyOrig;
    Word_t *pwRootArg = pwRoot; (void)pwRootArg;
    int nBW; (void)nBW;
    Link_t *pLinks; (void)pLinks;
    Word_t wBytes = 0;
    int nBLR = nBL;

    assert((wRoot != 0) || (WROOT_NULL == 0));

#if ! defined(USE_XX_SW)
    assert(nBL >= cnBitsInD1);
#endif // ! defined(USE_XX_SW)

    if (!bDump && (nBL == cnBitsPerWord)) { DBGR(printf("FreeArrayGuts\n")); }

    Word_t wPrefix = wKey & ~NZ_MSK(nBL); (void)wPrefix;
    if (bDump) {
        // Check for embedded bitmap before assuming nType is valid.
        if (!cbEmbeddedBitmap || (nBL > cnLogBitsPerLink)) {
  #ifdef SKIP_LINKS
            if (tp_bIsSkip(nType)) {
                nBLR = GetBLR(pwRoot, nBL);
    #if defined(SKIP_TO_BITMAP) || defined(SKIP_TO_LIST)
                if (tp_bIsSwitch(nType))
    #endif // defined(SKIP_TO_BITMAP) || defined(SKIP_TO_LIST)
                {
                    wPrefix = PWR_wPrefixBL(pwRoot, (Switch_t*)pwr, nBLR);
                    //assert(gwPrefix(qy) == wPrefix); exposes bug in gwPrefix
                }
    #if defined(SKIP_TO_BITMAP) || defined(SKIP_TO_LIST)
                else
    #endif // defined(SKIP_TO_BITMAP) || defined(SKIP_TO_LIST)
    #ifdef SKIP_TO_BITMAP
            #ifdef SKIP_TO_LIST
                if (nType == T_SKIP_TO_BITMAP)
            #endif // SKIP_TO_LIST
                {
            #ifndef SKIP_TO_LIST
                        assert(nType == T_SKIP_TO_BITMAP);
            #endif // #ifndef SKIP_TO_LIST
                        wPrefix = gwBitmapPrefix(qy, nBLR);
                }
    #endif // SKIP_TO_BITMAP
    #ifdef SKIP_TO_LIST
                {
                        assert(nType == T_SKIP_TO_LIST);
                        wPrefix = gwListPrefix(qy, nBLR);
                }
    #endif // SKIP_TO_LIST
                // check for prefix mismatch and bail out
                if ((wKey & ~NZ_MSK(nBLR)) != wPrefix) {
    #ifndef FULL_DUMP
                        printf("Prefix mismatch\n");
                        return 0;
    #endif // #ifndef FULL_DUMP
                }
            }
  #endif // SKIP_LINKS
        }
        printf(" nBL %2d nBLR %2d", nBL, nBLR);
        printf(" wKey " OWx,
               (wPrefix == (wKey & ~NZ_MSK(nBLR))) ? wKey : wPrefix
               );
        printf(" pwRoot " OWx, (Word_t)pwRoot);
        printf(" wRoot " OWx, wRoot);
  #ifdef _LNX
      #ifdef REMOTE_LNX
        printf(" pwLnX %p", pwLnX);
      #endif // REMOTE_LNX
        if (pwLnX != NULL) {
            assert(nBL < cnBitsPerWord);
            printf(" wLnX 0x%02zx", *pwLnX);
        } else {
            assert(nBL >= cnBitsPerWord);
        }
  #endif // _LNX
        if (tp_bIsSwitch(nType)) {
            printf(" pLinks %p",
  #ifdef CODE_LIST_SW
                tp_bIsListSw(nType) ? pwr_pLinks((ListSw_t*)pwr) :
  #endif // CODE_LIST_SW
  #ifdef CODE_BM_SW
                tp_bIsBmSw(nType) ? pwr_pLinks((BmSwitch_t*)pwr) :
  #endif // CODE_BM_SW
                pwr_pLinks((Switch_t*)pwr));
        }
    }

    // Check for embedded bitmap before assuming nType is valid.
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        if (bDump) {
            int nWords
                = nBL > cnLogBitsPerWord ? EXP(nBL - cnLogBitsPerWord) : 1;
            if ((nWords > 1)
                || (pwRoot != (Word_t*)STRUCT_OF(pwRoot, Link_t, ln_wRoot)))
            {
                printf(" nWords %4d", nWords);
                for (int ww = 0; ww < nWords; ++ww) {
                    if ((ww % 8) == 0) { printf("\n"); }
                    printf(" " OWx,
                           ((Word_t*)STRUCT_OF(pwRoot, Link_t, ln_wRoot))[ww]);
                }
            } else {
                // We've already dumped wRoot.
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
        nBLR = GetBLR(pwRoot, nBL);
        Word_t wPopCnt = gwBitmapPopCnt(qya, nBLR);
        if (bDump) {
            printf(" nBLR %2d", nBLR);
            goto dumpBmTail;
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
    if (tp_bIsBitmap(nType)) {
        if (bDump) { printf(nType == T_BITMAP ? " BITMAP" : " UNPACKED_BM"); }

        // If the bitmap is not embedded, then we have more work to do.
        // The test can be done at compile time and will make one the
        // other clauses go away.
        if (!bDump) {
            assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
            Word_t wPopCnt = gwBitmapPopCnt(qya, nBL);
            wBytes = OldBitmap(pwr, nBL, wPopCnt);
            *pwRoot = WROOT_NULL;
            return wBytes;
        }
        assert(nBL != cnBitsPerWord); // wPopCntTotal, zeroLink
        goto dumpBmTail;
dumpBmTail:;
        int nWords
            = nBLR > cnLogBitsPerWord ? EXP(nBLR - cnLogBitsPerWord) : 1;
        printf(" nWords %4d", nWords);
        printf(" wPopCnt %5zd", gwBitmapPopCnt(qya, nBLR));
        BmLeaf_t* pBmLeaf = (BmLeaf_t*)pwr; (void)pBmLeaf;
      #ifdef BMLF_CNTS
          #ifdef BMLF_POP_COUNT_8
            HexDump("bmlf_au8Cnts", (Word_t*)pBmLeaf->bmlf_au8Cnts, 4);
          #elif defined(BMLF_POP_COUNT_1) // BMLF_POP_COUNT_8
            HexDump("bmlf_au8Cnts", (Word_t*)pBmLeaf->bmlf_au8Cnts, 64);
          #else // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1
            printf(" wCnts 0x%016zx",
              #if defined(BMLF_CNTS_IN_LNX)
                   *pwLnX
              #else // BMLF_CNTS_IN_LNX
                   *(Word_t*)(pBmLeaf->bmlf_au8Cnts)
              #endif // BMLF_CNTS_IN_LNX else
                   );
          #endif // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1 else
      #endif // BMLF_CNTS
      #if cn2dBmMaxWpkPercent != 0
      #if cnWordsBm2Cnts != 0
        if (nBLR == cnBitsLeftAtDl2) {
            printf("\n pxCnts %p\n", gpxBitmapCnts(qya, nBLR));
            for (int ww = 0; ww < cnWordsBm2Cnts; ++ww) {
                if ((ww != 0) && (ww % 4) == 0) {
                    printf("\n");
                }
                printf(" 0x%016zx", ((Word_t*)gpxBitmapCnts(qya, nBLR))[ww]);
            }
        }
      #endif // cnWordsBm2Cnts != 0
      #endif // cn2dBmMaxWpkPercent != 0
      #ifdef _BMLF_BM_IN_LNX
        printf(" " OWx, *pwLnX);
      #else // _BMLF_BM_IN_LNX
        Word_t *pwBitmap = pBmLeaf->bmlf_awBitmap;
        printf("\n pwBitmap %p", pwBitmap);
        Word_t wPopCntL = 0;
        for (Word_t ww = 0; (int)ww < nWords; ++ww) {
            if ((ww != 0) && (ww % 4) == 0) {
                printf(" %5zd", wPopCntL);
                wPopCntL = 0;
            }
            wPopCntL += __builtin_popcountll(pwBitmap[ww]);
            if ((ww % 8) == 0) {
                printf("\n");
            }
            printf(" " OWx, pwBitmap[ww]);
        }
      #endif // else _BMLF_BM_IN_LNX
      #ifdef B_JUDYL
        printf("\n Values %p\n", gpwBitmapValues(qy, nBLR));
        for (int ww = 0;
             ww < (int)(BM_UNPACKED(wRoot)
                            ? EXP(nBLR) : gwBitmapPopCnt(qya, nBLR));
             ++ww)
        {
            if ((ww != 0) && (ww % 4) == 0) {
                printf("\n");
            }
            printf(" 0x%016zx", gpwBitmapValues(qy, nBLR)[ww]);
        }
      #endif // B_JUDYL
        printf("\n");
        return 0;
    }
#endif // BITMAP

#if defined(SKIP_LINKS) // || (cwListPopCntMax != 0)
    assert( ! tp_bIsSkip(nType) || ((int)wr_nBLR(wRoot) < nBL) );
    assert( ! tp_bIsSkip(nType) || (wr_nBLR(wRoot) >= cnBitsInD1) );
#endif // defined(SKIP_LINKS) || (cwListPopCntMax != 0)

  #if (cwListPopCntMax != 0)

    if (!tp_bIsSwitch(nType))
    {
        Word_t wPopCnt;

      #ifdef EMBED_KEYS

        if (nType == T_EMBEDDED_KEYS) {
            goto embeddedKeys;
embeddedKeys:;
            if (!bDump) { return 0; }
            printf(" EMBEDDED_KEYS");
            wPopCnt = wr_nPopCnt(wRoot, nBL);
            assert(wPopCnt != 0);
            printf(" wr_nPopCnt %3d", (int)wPopCnt);
            int nPopCntMax = EmbeddedListPopCntMax(nBL);
            if (nPopCntMax != 0) {
                for (int nPos = 0; nPos < (int)wPopCnt; nPos++) {
                    printf(" 0x%02zx",
                           (wRoot >> (cnBitsPerWord - nBL *
          #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
                                    (nPopCntMax - nPos)
          #else // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
                                    (nPos + 1)
          #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS else
                                      ))
                               & MSK(nBL));
          #ifdef B_JUDYL
                    printf(",0x%02zx", *gpwEmbeddedValue(qya));
          #endif // B_JUDYL
                }
            } else { //  wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS
                printf(" " OWx, *pwr);
            }
            putchar('\n');
        }
          #ifdef EK_XV
        else if (nType == T_EK_XV) {
            wPopCnt = wr_nPopCnt(wRoot, nBL);
            if (!bDump) {
                int nWords = MAX(3, wPopCnt | 1);
                MyFree(pwr, nWords, &j__AllocWordsJV);
                return nWords * sizeof(Word_t);
            }
            printf(" EK_XV");
            printf(" wr_nPopCnt %3d", (int)wPopCnt);
            assert(wPopCnt > 1);
            if (wPopCnt > (Word_t)EmbeddedListPopCntMax(nBL)) {
                printf("nBL %d\n", nBL);
                printf("max %d\n", EmbeddedListPopCntMax(nBL));
            }
            assert(wPopCnt <= (Word_t)EmbeddedListPopCntMax(nBL));
            int nBLUp = nBL + nBWUp; (void)nBLUp;
            Link_t *pLnUp = STRUCT_OF(&pwrUp, Link_t, ln_wRoot); (void)pLnUp;
            printf(" keys 0x%zx", *pwLnX);
            printf(" values");
            for (Word_t ww = 0; ww < wPopCnt; ++ww) {
                printf(" 0x%zx", pwr[ww]);
            }
            printf("\n");
        }
          #endif // EK_XV
        else
      #endif // EMBED_KEYS
        {
            assert(tp_bIsList(nType));
            nBLR = gnListBLR(qy);

            assert((wRoot != 0) || (WROOT_NULL == 0));
            wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);
            assert(wPopCnt != 0);

            if (!bDump)
            {
      #ifdef XX_LISTS
                // Be careful not to free a shared list more than once.
                // Put WROOT_NULL in replicated links to inhibit subsequent
                // traversal.
                if (nType == T_XX_LIST) {
                    // Replicate the link with a new pop cnt and maybe pwr.
                    // nBLR determines size of the keys but nothing about
                    // the width of the switch.
                    int nBLRUp = gnListBLR(qy);
                    int nBWRUp = nBLRUp - nBL; // max nBWRUp
                    Word_t wKeyDigit = (wKey >> nBL) & MSK(nBWRUp);
                    // Beginning of virtual link array.
                    Link_t* pLinksUp = &pLn[-wKeyDigit];
                    // If actual nBLRUp < max nBLRUp we could walk past the
                    // end of the physical link array.
                    // Can this happen with USE_LOWER_XX_SW?
                    // I don't think we put an XX_LIST in a lower XX_SW
                    // since we use InsertAll or splay after creating a lower
                    // XX_SW and neither puts an XX_LIST in the lower XX_SW.
                    // I'm not sure we ever use a lower XX_SW now that we
                    // have InsertXxSw.
                    // We see lower XX_SW with:
                    // DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64
                    // -DNO_EMBED_KEYS -DNO_USE_BM_SW -DDEBUG"
                    // make clean default
                    // "b -1 -s-512 -S1 -B-21". Why?
                    // Will we access the memory following the last physical
                    // link and compare it to pLn->ln_wRoot with InsertXxSw?
                    // Nope.
                    // InsertXxSw uses nBLRUp = BL_to_DL(DL_to_BL(SigBitCnt)).
                    for (int nDigit = wKeyDigit + 1;
                             nDigit < (1 << nBWRUp); ++nDigit)
                    {
                        // I wonder if we could use SignificantBitCnt here.
                        if (pLinksUp[nDigit].ln_wRoot != pLn->ln_wRoot) {
                            break;
                        }
                        pLinksUp[nDigit].ln_wRoot = WROOT_NULL;
                    }
                }
      #endif // XX_LISTS

                wBytes = OldList(pwr, wPopCnt, nBLR, nType);
                assert(wr_pwr(*pwRootArg) == pwr);
                goto zeroLink;
            }

      #ifdef XX_LISTS
            if (nType == T_XX_LIST) {
                printf(" nBLR %d", nBLR);
            }
      #endif // XX_LISTS
            { printf(" ls_wPopCnt %3" _fw"u", wPopCnt); }

            printf(" ln_wKey ");
            { printf("       N/A"); }
            printf(" pKeys %p", ls_pwKeysX(pwr, nBLR, wPopCnt));

            for (int xx = 0; (xx < (int)wPopCnt); xx++) {
      #ifdef COMPRESSED_LISTS
                if (nBLR <= 8) {
                    uint8_t* pcKeys = ls_pcKeysNATX(pwr, wPopCnt);
                    printf(" %02x", pcKeys[xx]);
          #ifdef B_JUDYL
                    printf("," OWx,
              #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                        (nBLR == cnBitsInD1)
                            ? ((Word_t*)pcKeys)[~(pcKeys[xx] & MSK(nBLR))] :
              #endif // !PACK_L1_VALUES && cnBitsInD1 <= 8
                              ((Word_t*)pcKeys)[~xx]);
          #endif // B_JUDYL
                } else if (nBLR <= 16) {
                    printf(" %04x", ls_psKeysNATX(pwr, wPopCnt)[xx]);
          #ifdef B_JUDYL
                    printf("," OWx,
                           ((Word_t*)ls_psKeysNATX(pwr, wPopCnt))[~xx]);

          #endif // B_JUDYL
          #if (cnBitsPerWord > 32)
                } else if (nBLR <= 32) {
                    printf(" %08x", ls_piKeysNATX(pwr, wPopCnt)[xx]);
              #ifdef B_JUDYL
                    printf("," OWx,
                           ((Word_t*)ls_piKeysNATX(pwr, wPopCnt))[~xx]);
              #endif // B_JUDYL
          #endif // (cnBitsPerWord > 32)
                } else
      #endif // COMPRESSED_LISTS
                {
                    printf(" " OWx, ls_pwKeysX(pwr, nBLR, wPopCnt)[xx]);
      #ifdef B_JUDYL
                    printf("," OWx, gpwValues(qy)[~xx]);
      #endif // B_JUDYL
                }
            }
      #ifdef PSPLIT_PARALLEL
            if (ALIGN_LIST_LEN(ExtListBytesPerKey(nBLR), wPopCnt)) {
                printf(" pad");
                for (int nn = (int)wPopCnt;
                     nn * ExtListBytesPerKey(nBLR) % sizeof(Bucket_t);
                     ++nn)
                {
                    int xx = nn;
          #ifdef COMPRESSED_LISTS
                    if (nBLR <= 8) {
                        printf(" %02x", ls_pcKeysNATX(pwr, wPopCnt)[xx]);
              #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ((Word_t*)ls_pcKeysNATX(pwr,
                                                           wPopCnt))[~nn]);
                        }
              #endif // B_JUDYL
                    } else if (nBLR <= 16) {
              #ifdef UA_PARALLEL_128
                        if ((nBLR == 16) && (nn == 6)) {
                            assert(nType == T_LIST_UA);
                        }
              #endif // UA_PARALLEL_128
                        printf(" %04x", ls_psKeysNATX(pwr, wPopCnt)[xx]);
              #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ((Word_t*)ls_psKeysNATX(pwr,
                                                           wPopCnt))[~nn]);
                        }
              #endif // B_JUDYL
              #if (cnBitsPerWord > 32)
                    } else if (nBLR <= 32) {
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
                        printf(" " OWx, ls_pwKeysX(pwr, nBLR, wPopCnt)[xx]);
          #ifdef B_JUDYL
                        if (nn < (int)wPopCnt) {
                            printf("," OWx,
                                   ls_pwKeysX(pwr, nBLR, wPopCnt)[~nn]);
                        }
          #endif // B_JUDYL
                    }
                }
            }
      #endif // PSPLIT_PARALLEL
      #ifdef UA_PARALLEL_128
            if (nType == T_LIST_UA) {
                assert(nBLR == 16);
                assert(wPopCnt <= 6);
                assert(cnBitsPerWord == 32);
                assert(cnBitsMallocMask >= 4);
                assert(ListWordCnt(wPopCnt, nBLR) == 3);
                //printf("\nT_LIST_UA pwr %p\n", (void*)pwr);
                //HexDump(/* str */ "", /* pw */ &pwr[-1], /* nWords */ 5);
            }
      #endif // UA_PARALLEL_128
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
    assert(tp_bIsSwitch(nType));

    nBLR = nBL;

    { // make C++ happy

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
            nBLR = GetBLR(pwRoot, nBL);
        }
    }
#endif // defined(SKIP_LINKS)

#if defined(CODE_XX_SW)
    if (tp_bIsXxSw(nType)) {
        nBW = pwr_nBW(pwRoot);
    } else
#endif // defined(CODE_XX_SW)
    { nBW = nBLR_to_nBW(nBLR); }

    pLinks =
#if defined(CODE_BM_SW)
        bBmSw ? pwr_pLinks((BmSwitch_t *)pwr) :
#endif // defined(CODE_BM_SW)
#if defined(USE_LIST_SW)
        ((nType == T_LIST_SW) || (nType == T_SKIP_TO_LIST_SW))
            ? gpListSwLinks(qy) :
#endif // defined(USE_LIST_SW)
        pwr_pLinks((Switch_t *)pwr) ;

    if (bDump) {
        {
            Word_t wPopCnt =
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPopCntBL(pwRoot, (BmSwitch_t *)pwr, nBLR) :
#endif // defined(CODE_BM_SW)
                           PWR_wPopCntBL(pwRoot, (  Switch_t *)pwr, nBLR)  ;
            if (wPopCnt == 0) {
                // Full-pop or in-transition sub-tree.
                // wPopCnt = EXP(nBLR);
            }
            printf(" wr_wPopCnt %3" _fw"u", wPopCnt);
  #if cnSwCnts != 0
            printf(" awCnts");
            for (int i = 0; i < cnSwCnts; ++i) {
                printf(" 0x%016zx", ((Switch_t*)pwr)->sw_awCnts[i]);
            }
            if (nBLR <= 16) {
                int nSum = 0;
                for (int i = 0; i < cnSwCnts * (int)sizeof(Word_t) / 2; ++i) {
                    nSum += ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)[i];
                }
                printf(" awCnts nSum %d", nSum);
            }
  #endif // cnSwCnts != 0
            assert(nBLR == gnBLR(qy));
  #ifdef SKIP_LINKS
            printf(" wr_wKey " OWx,
#if defined(CODE_BM_SW)
                   bBmSw ? PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR) :
#endif // defined(CODE_BM_SW)
                           PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBLR) );
  #endif // SKIP_LINKS
        }

        printf(" wr_nBLR %2d", nBLR);
#if defined(CODE_XX_SW)
        if (tp_bIsXxSw(nType)) {
            printf(" wr_nBW %2d", nBW);
        }
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
        // should enhance this to check for zeros in suffix and to print
        // dots for suffix.
        //printf(" wKeyPopMask " OWx, wKeyPopMask(nDL));
        //printf(" pLinks " OWx, (Word_t)pLinks);
        if (bBmSw) {
#if defined(BM_IN_LINK)
            if (nBL != cnBitsPerWord)
#endif // defined(BM_IN_LINK)
            {
                printf(" pwBm " OWx" pLinks " OWx" Bm",
                       (Word_t)PWR_pwBm(pwRoot, pwr, nBW), (Word_t)pLinks);
                // Bitmaps are an integral number of words.
                for (int nn = 0; nn < N_WORDS_SW_BM(nBW); nn++) {
                    if ((nn % 8) == 0) {
                        printf("\n");
                    }
                    printf(" " OWx, PWR_pwBm(pwRoot, pwr, nBW)[nn]);
                }
            }
        }
#endif // defined(CODE_BM_SW)
        printf("\n");
    }

  #ifdef SKIP_LINKS
    // skip link has extra prefix bits
    if (nBL > nBLR) {
#ifdef PP_IN_LINK
        if (nBL < cnBitsPerWord)
#endif // PP_IN_LINK
        {
  #ifndef FULL_DUMP
  #ifdef DEBUG
            Word_t wKeyPrev = wKey;
  #endif // DEBUG
  #endif // #ifndef FULL_DUMP
            wKey =
#if defined(CODE_BM_SW)
                bBmSw ? PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR) :
#endif // defined(CODE_BM_SW)
#if defined(USE_LIST_SW)
                        PWR_wPrefixBL(pwRoot, (ListSw_t   *)pwr, nBLR) ;
#else // defined(USE_LIST_SW)
                        PWR_wPrefixBL(pwRoot, (  Switch_t *)pwr, nBLR) ;
#endif // defined(USE_LIST_SW)
  #ifndef FULL_DUMP
            if (bDump) {
  #ifdef DEBUG
                if (((wKey ^ wKeyPrev) & ~NZ_MSK(nBL)) != 0) {
                    printf("nBL %d nBLR %d wKeyPrev 0x%zx wKey 0x%zx\n",
                           nBL, nBLR, wKeyPrev, wKey);
                }
  #endif // DEBUG
                assert(((wKey ^ wKeyPrev) & ~NZ_MSK(nBL)) == 0);
            }
  #endif // #ifndef FULL_DUMP
        }
    }
  #else // SKIP_LINKS
    assert(nBL == nBLR);
  #endif // SKIP_LINKS

    int nBLLoop = nBLR - nBW;

  #ifdef CODE_BM_SW
      #if defined(BM_IN_LINK)
        assert(!bBmSw || (nBL != cnBitsPerWord));
      #endif // defined(BM_IN_LINK)
      #ifdef REMOTE_LNX
        int nLinks = 0;
        if (bBmSw) {
            nLinks = BmSwLinkCnt(qya);
        }
      #endif // REMOTE_LNX
  #endif // CODE_BM_SW

    for (Word_t ww = 0, nn = 0; nn < EXP(nBW); nn++) {
        // Here we are dumping (wPrefix | (nn << nBLLoop)).
        wKey &= ~NZ_MSK(nBLR); // prep for adding the digit
  #ifdef CODE_BM_SW
        if (!bBmSw
            || (PWR_pwBm(pwRoot, pwr, nBW)[gnWordNumInSwBm(nn)]
                & gwBitMaskInSwBmWord(nn)))
  #endif // CODE_BM_SW
        {
            if ((cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink))
                || (pLinks[ww].ln_wRoot != WROOT_NULL))
            {
  #ifdef XX_LISTS
                if (bDump
                    && (ww != 0)
                    && (pLinks[ww].ln_wRoot == pLinks[ww - 1].ln_wRoot)
                    && (wr_nType(pLinks[ww].ln_wRoot) == T_XX_LIST))
                {
                    printf(" Ditto ");
                    printf(" wPrefix " OWx,
                           /*wKeyLoop*/ wKey | (nn << nBLLoop));
                    printf(" pwRoot " OWx, (Word_t)&pLinks[ww].ln_wRoot);
                    printf(" Ditto\n");
                    ++ww;
                    continue;
                }
// See comment in T_XX_LIST handling about about running off the end of
// the physical link array.
// InsertXxSw uses nBLRUp = BL_to_DL(DL_to_BL(SigBitCnt)).
// We always InsertAll or Splay after creating lower XX_SW and neither
// creates an XX_LIST in the new lower XX_SW.
                assert((wr_nType(pLinks[ww].ln_wRoot) != T_XX_LIST)
                    || (nDL_to_nBL(nBL_to_nDL(nBLR)) == nBLR));
  #endif // XX_LISTS
  #ifndef FULL_DUMP
                if (!bDump || (((wKeyOrig >> nBLLoop) & MSK(nBW)) == nn))
  #endif // #ifndef FULL_DUMP
                {
                    if (((wKeyOrig >> nBLLoop) & MSK(nBW)) == nn) {
                        wKey = wKeyOrig;
                    }
                    Word_t* pwRootLoop = &pLinks[ww].ln_wRoot;
  #ifdef REMOTE_LNX
                    Word_t *pwLnXLoop = gpwLnX(qy,
      #ifdef CODE_BM_SW
                                               bBmSw ? nLinks :
      #endif // CODE_BM_SW
                                                   1<<nBW,
                                               ww);
  #endif // REMOTE_LNX
                    wBytes += FreeArrayGuts(qyax(Loop),
                                            wKey | (nn << nBLLoop), bDump
#if defined(B_JUDYL) && defined(EMBED_KEYS)
                                          , /*pwrUp*/ pwr, /*nBWUp*/ nBW
  #ifdef CODE_BM_SW
                                          , nType
  #endif // CODE_BM_SW
#endif // defined(B_JUDYL) && defined(EMBED_KEYS)
                                            );
                }
            }

            ww++;
        }
    }

    if (bDump) return 0;

    // Someone has to clear PP and BM if PP_IN_LINK and BM_IN_LINK.
    // OldSwitch looks at BM.

    wBytes += OldSwitch(pwRootArg, nBL
#if defined(CODE_BM_SW)
                      , bBmSw, /* nLinks */ 0
#endif // defined(CODE_BM_SW)
                        );
    }

#if cwListPopCntMax != 0
zeroLink:
#endif // cwListPopCntMax != 0
    *pwRootArg = (nBLR != cnBitsPerWord) ? WROOT_NULL : 0;
    return wBytes;
}

#if defined(DEBUG)

void
DumpX(qpa, Word_t wKey)
{
    qva;
    if (bHitDebugThreshold) {
  #ifdef FULL_DUMP
        printf("# Full Dump\n");
  #else // FULL_DUMP
        printf("# Partial Dump\n");
  #endif // #else FULL_DUMP
        FreeArrayGuts(qya, wKey, /* bDump */ 1
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

void
Dump(Word_t *pwRoot, Word_t wKey, int nBL)
{
  #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
    DumpX(qya, wKey);
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
// Which cases should InsertEmbedded handle?
// - NO_TYPE_IN_XX_SW -- not yet
// - (wRoot == WROOT_NULL) && wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS -- yes
// - nType == T_EMBEDDED_KEYS -- yes
// - nType == T_EK_XV -- not yet
#ifdef B_JUDYL
static Word_t* // pwValue
#else // B_JUDYL
static void
#endif // else B_JUDYL
InsertEmbedded(qpa, Word_t wKey)
{
    qva;
    // Does the compiler get rid of this 2nd call to EmbeddedListPopCntMax?
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
  #ifdef NO_TYPE_IN_XX_SW
    // We don't really expect this to be true, but want to be reminded
    // that the code needs to be enhanced if it is not true.
    assert(ZERO_POP_MAGIC == WROOT_NULL);
  #endif // NO_TYPE_IN_XX_SW
    // Does the compiler get rid of this 2nd call to wr_nPopCnt?
    int nPopCnt = (*pwRoot == WROOT_NULL) ? 0 : wr_nPopCnt(*pwRoot, nBL);
    DBGI(printf("\nInsertEmbedded: wRoot " OWx" nBL %d wKey " OWx
                    " nPopCnt %d Max %d\n",
                *pwRoot, nBL, wKey, nPopCnt, nPopCntMax));
    assert(nPopCnt < nPopCntMax);
    if (WROOT_NULL != T_EMBEDDED_KEYS) {
        if (nPopCnt == 0) {
            if (WROOT_NULL != 0) {
                *pwRoot = 0; // clear out WROOT_NULL
            }
            set_wr_nType(*pwRoot, T_EMBEDDED_KEYS);
        }
    }
  #ifdef B_JUDYL
      #ifdef EK_XV
    if (nPopCnt == 0)
      #else // EK_XV
    assert(nPopCnt == 0);
      #endif // EK_XV
    {
        for (int nPos = 0;
          #if defined(FILL_W_KEY) || defined(FILL_W_BIG_KEY)
             nPos < (cnBitsPerWord - 4) / nBL;
          #else // FILL_W_KEY || FILL_W_BIG_KEY
             nPos < 1;
          #endif // FILL_W_KEY || FILL_W_BIG_KEY else
            ++nPos)
        {
            SetBits(pwRoot, nBL,
                    cnBitsPerWord - nBL * (nPos + 1), wKey & MSK(nBL));
        }
        return gpwEmbeddedValue(qya); // Insert will zero *pwValue.
    }
      #ifdef EK_XV
    Word_t *pwValue; // for return
    assert(WROOT_NULL != T_EK_XV);
    int nLogKeyBytes = LOG(nBL - 1) - 2;
    if (nPopCnt == 1) {
        // Create the value area.
        Word_t *pwrNew = (Word_t*)MyMallocGuts(/* nWords */ 3,
                                               /* nLogAlign */
          #ifdef ALIGN_EK_XV
                                               5,
          #else // ALIGN_EK_XV
                                               cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                               &j__AllocWordsJV);
        Word_t wKey0 = GetBits(wRoot, nBL, cnBitsPerWord - nBL);
        wKey0 |= wKey & MSK(8 << nLogKeyBytes) & ~MSK(nBL);
        Word_t wKey1 = wKey & MSK(8 << nLogKeyBytes);
        if (wKey1 < wKey0) {
            wKey1 = wKey0;
            wKey0 = wKey & MSK(8 << nLogKeyBytes);
            pwrNew[1] = *gpwEmbeddedValue(qya);
            pwValue = &pwrNew[0];
        } else {
            pwrNew[0] = *gpwEmbeddedValue(qya);
            pwValue = &pwrNew[1];
        }
        switch (nLogKeyBytes) {
        case 0:
            ((uint8_t*)pwLnX)[0] = wKey0;
            ((uint8_t*)pwLnX)[1] = wKey1;
            PAD64((uint8_t*)pwLnX, 2);
            break;
        case 1:
            ((uint16_t*)pwLnX)[0] = wKey0;
            ((uint16_t*)pwLnX)[1] = wKey1;
            PAD64((uint16_t*)pwLnX, 2);
            break;
          #if (cnBitsPerWord > 32)
        case 2:
            ((uint32_t*)pwLnX)[0] = wKey0;
            ((uint32_t*)pwLnX)[1] = wKey1;
            break;
          #endif // (cnBitsPerWord > 32)
        }
        set_wr(*pwRoot, pwrNew, T_EK_XV);
        set_wr_nPopCnt(*pwRoot, nBL, /* nPopCnt */ 2);
        return pwValue;
    }
    wKey &= MSK(8 << nLogKeyBytes);
    int nSlot = 0;
    switch (nLogKeyBytes) {
    case 0: // nBLR <= 8
        for (; nSlot < nPopCnt; ++nSlot) {
            if (((uint8_t*)pwLnX)[nSlot] > wKey) {
                MOVE(&((uint8_t*)pwLnX)[nSlot + 1],
                     &((uint8_t*)pwLnX)[nSlot], nPopCnt - nSlot);
                break;
            }
        }
        ((uint8_t*)pwLnX)[nSlot] = wKey;
        PAD64((uint8_t*)pwLnX, nPopCnt + 1);
        break;
    case 1: // 8 < nBLR <= 16
        for (; nSlot < nPopCnt; ++nSlot) {
            if (((uint16_t*)pwLnX)[nSlot] > wKey) {
                MOVE(&((uint16_t*)pwLnX)[nSlot + 1],
                     &((uint16_t*)pwLnX)[nSlot], nPopCnt - nSlot);
                break;
            }
        }
        ((uint16_t*)pwLnX)[nSlot] = wKey;
        PAD64((uint16_t*)pwLnX, nPopCnt + 1);
          #if (cnBitsPerWord > 32)
        break;
    case 2: // 16 < nBLR <= 32
        for (; nSlot < nPopCnt; ++nSlot) {
            if (((uint32_t*)pwLnX)[nSlot] > wKey) {
                MOVE(&((uint32_t*)pwLnX)[nSlot + 1],
                     &((uint32_t*)pwLnX)[nSlot], nPopCnt - nSlot);
                break;
            }
        }
        ((uint32_t*)pwLnX)[nSlot] = wKey;
          #endif // (cnBitsPerWord > 32)
    }
    if (nPopCnt & 1) {
        Word_t* pwrNew = (Word_t*)MyMallocGuts(nPopCnt + 2,
          #ifdef ALIGN_EK_XV
                                               6
          #else // ALIGN_EK_XV
                                               cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                               &j__AllocWordsJV);
        COPY(pwrNew, pwr, nSlot);
        COPY(&pwrNew[nSlot + 1], &pwr[nSlot], nPopCnt - nSlot);
        MyFree(pwr, nPopCnt, &j__AllocWordsJV);
        pwr = pwrNew;
        set_wr_pwr(*pwRoot, pwr);
    } else {
        MOVE(&pwr[nSlot + 1], &pwr[nSlot], nPopCnt - nSlot);
    }
    set_wr_nPopCnt(*pwRoot, nBL, /* nPopCnt */ nPopCnt + 1);
    return &pwr[nSlot];
      #endif // EK_XV
  #else // B_JUDYL
    // find the slot
    wKey &= MSK(nBL);
    int nPos = 0;
    for (; nPos < nPopCnt; ++nPos) {
        if (GetBits(*pwRoot, nBL, cnBitsPerWord - nBL *
      #ifdef REVERSE_SORT_EMBEDDED_KEYS
                    (nPopCntMax - nPos)
      #else // REVERSE_SORT_EMBEDDED_KEYS
                    (nPos + 1)
      #endif // REVERSE_SORT_EMBEDDED_KEYS else
                    )
            > wKey)
        {
            break;
        }
    }
    DBGI(printf("InsertEmbedded: wKey " OWx" nPos %d", wKey, nPos));
    if (nPos < nPopCnt) {
      #ifdef REVERSE_SORT_EMBEDDED_KEYS
        int nLsb = cnBitsPerWord - (nPopCntMax - nPos) * nBL;
        Word_t wBigKeys = GetBits(*pwRoot, (nPopCnt - nPos) * nBL, nLsb);
        SetBits(pwRoot, (nPopCnt - nPos) * nBL, nLsb + nBL, wBigKeys);
      #else // REVERSE_SORT_EMBEDDED_KEYS
        int nLsb = cnBitsPerWord - nPopCnt * nBL;
        Word_t wBigKeys = GetBits(*pwRoot, (nPopCnt - nPos) * nBL, nLsb);
        SetBits(pwRoot, (nPopCnt - nPos) * nBL, nLsb - nBL, wBigKeys);
      #endif // REVERSE_SORT_EMBEDDED_KEYS else
    }
    SetBits(pwRoot, nBL, cnBitsPerWord - nBL *
      #ifdef REVERSE_SORT_EMBEDDED_KEYS
                             (nPopCntMax - nPos),
      #else // REVERSE_SORT_EMBEDDED_KEYS
                             (nPos + 1),
      #endif // REVERSE_SORT_EMBEDDED_KEYS else
            wKey);
      #ifdef FILL_W_BIG_KEY
    if (nPos == nPopCnt)
      #elif defined(FILL_W_KEY)
    if (nPos == 0)
      #else // FILL_W_BIG_KEY elif FILL_W_KEY
    if (0)
      #endif // FILL_W_BIG_KEY elif FILL_W_KEY else
    {
        for (nPos = nPopCnt + 1; nPos < nPopCntMax; ++nPos) {
            SetBits(pwRoot, nBL, cnBitsPerWord - nBL *
          #ifdef REVERSE_SORT_EMBEDDED_KEYS
                                     (nPopCntMax - nPos),
          #else // REVERSE_SORT_EMBEDDED_KEYS
                                     (nPos + 1),
          #endif // REVERSE_SORT_EMBEDDED_KEYS else
                                     wKey);
        }
    }
    set_wr_nPopCnt(*pwRoot, nBL, nPopCnt + 1);
    DBGI(printf(" wRoot " OWx" nPopCnt %d\n",
                *pwRoot, wr_nPopCnt(*pwRoot, nBL)));
  #endif // else B_JUDYL
}
#endif // defined(EMBED_KEYS)

#if (cwListPopCntMax != 0)

#define COPY_KEYS_WITH_INSERT(_pTgtKeys, _pSrcKeys, _nSrcCnt, _wKey, _nPos) \
{ \
    if ((void*)(_pTgtKeys) != (void*)(_pSrcKeys)) { \
        COPY((_pTgtKeys), (_pSrcKeys), (_nPos)); \
        COPY(&(_pTgtKeys)[(_nPos)+1], \
              &(_pSrcKeys)[_nPos], (_nSrcCnt) - (_nPos)); \
    } else { \
        MOVE(&(_pTgtKeys)[(_nPos)+1], \
             &(_pSrcKeys)[_nPos], (_nSrcCnt) - (_nPos)); \
    } \
    (_pTgtKeys)[_nPos] = (_wKey); \
    PAD((_pTgtKeys), (_nSrcCnt) + 1); \
}

#ifdef B_JUDYL

#define COPY_WITH_INSERT(_pTgtKeys, _pSrcKeys, _nSrcKeyCnt, _wKey, _nPos, \
                         _pwTgtVals, _pwSrcVals, _ppwRetVal) \
{ \
    COPY_KEYS_WITH_INSERT((_pTgtKeys), (_pSrcKeys), (_nSrcKeyCnt), \
                          (_wKey), (_nPos)); \
    if ((void*)(_pTgtKeys) != (void*)(_pSrcKeys)) { \
        COPY(&(_pwTgtVals)[~(_nSrcKeyCnt)], &(_pwSrcVals)[-(_nSrcKeyCnt)], \
             (_nSrcKeyCnt) - (_nPos)); \
        COPY(&(_pwTgtVals)[-(_nPos)], &(_pwSrcVals)[-(_nPos)], (_nPos)); \
    } else { \
        MOVE(&(_pwTgtVals)[~(_nSrcKeyCnt)], &(_pwSrcVals)[-(_nSrcKeyCnt)], \
             (_nSrcKeyCnt) - (_nPos)); \
    } \
    *(_ppwRetVal) = &(_pwTgtVals)[~(_nPos)]; \
}

#else // B_JUDYL

#define COPY_WITH_INSERT(_pTgtKeys, _pSrcKeys, _nSrcKeyCnt, _wKey, _nPos) \
{ \
    COPY_KEYS_WITH_INSERT((_pTgtKeys), (_pSrcKeys), (_nSrcKeyCnt), \
                          (_wKey), (_nPos)); \
}

#endif // #else B_JUDYL

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
CopyWithInsertWordX(qp, Word_t *pSrc,
  #ifdef B_JUDYL
                    Word_t *pwSrcVals,
  #endif // B_JUDYL
                    int nKeys, // number of keys excluding the new one
                    Word_t wKey, int nPos)
{
    DBGI(Log(qy, "CopyWithInsertWordX"));
    qv;
    DBGI(printf("\nCopyWithInsertWordX(pSrc %p nKeys %d wKey " OWx")\n",
                (void *)pSrc, nKeys, wKey));
    int nBLR = gnListBLR(qy); (void)nBLR;
//printf("nBLR %d\n", nBLR);
    Word_t *pTgt = ls_pwKeysX(pwr, nBLR, nKeys + 1);
//printf("pTgt %p\n", pTgt);
  #ifdef B_JUDYL
    Word_t *pwTgtVals = gpwValues(qy);
    Word_t* pwRetVal;
  #endif // B_JUDYL
    COPY_WITH_INSERT(pTgt, pSrc, nKeys, wKey, nPos
  #ifdef B_JUDYL
                   , pwTgtVals, pwSrcVals, &pwRetVal
  #endif // B_JUDYL
                     );
    DBGI(Log(qy, "CopyWithInsertWordX done "));
    //DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    BJL(return pwRetVal);
}

#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static void
#endif // B_JUDYL
CopyWithInsertWord(qp, Word_t *pSrc,
                   int nKeys, // number of keys excluding the new one
                   Word_t wKey, int nPos)
{
    assert(nPos >= 0);
  #ifdef B_JUDYL
    Word_t *pwSrcVals = pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    --pwSrcVals;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL
    BJL(return)
        CopyWithInsertWordX(qy, pSrc,
  #ifdef B_JUDYL
                            pwSrcVals,
  #endif // B_JUDYL
                            nKeys, wKey, nPos);
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
    int nBLR = gnListBLR(qy); (void)nBLR;
    uint32_t *pTgt = ls_piKeysX(pwr, nBLR, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtVals = gpwValues(qy);
    Word_t *pwSrcVals = (Word_t*)pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcVals -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL

    if (pTgt != pSrc) {
        // copy the values tail
        BJL(COPY(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos));
        // copy the values head
        BJL(COPY(&pwTgtVals[-nPos ], &pwSrcVals[-nPos ], nPos        ));
        COPY(pTgt, pSrc, nPos); // copy the head
        COPY(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // copy the tail
    } else {
        // move the values tail
        BJL(MOVE(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos));
        MOVE(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // move the tail
    }

    pTgt[nPos] = iKey; // insert the key
    PAD(pTgt, nKeys + 1);

    BJL(return &pwTgtVals[~nPos]);
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
    int nBLR = gnListBLR(qy); (void)nBLR;
    uint16_t *pTgt = ls_psKeysX(pwr, nBLR, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtVals = gpwValues(qy);
    Word_t *pwSrcVals = (Word_t*)pSrc;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcVals -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL

    if (pTgt != pSrc) {
        // copy the values tail
        BJL(COPY(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos));
        // copy the values head
        BJL(COPY(&pwTgtVals[-nPos ], &pwSrcVals[-nPos ], nPos        ));
        COPY(pTgt, pSrc, nPos); // copy the head
        COPY(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // copy the tail
    } else {
        // move the values tail
        BJL(MOVE(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos));
        MOVE(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // move the tail
    }

    pTgt[nPos] = sKey; // insert the key
    PAD(pTgt, nKeys + 1);

    BJL(return &pwTgtVals[~nPos]);
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
    int nBLR = gnListBLR(qy); (void)nBLR;
    uint8_t *pTgt = ls_pcKeysX(pwr, nBLR, nKeys + 1);
  #ifdef B_JUDYL
    Word_t *pwTgtVals = gpwValues(qy);
    Word_t *pwSrcVals = (Word_t*)pSrc; (void)pwSrcVals;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcVals -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL

    if (pTgt != pSrc) {
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBLR != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // copy the values tail
            COPY(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos);
            // copy the values head
            COPY(&pwTgtVals[-nPos    ], &pwSrcVals[-nPos    ], nPos        );
        }
  #endif // B_JUDYL
        COPY(pTgt, pSrc, nPos); // copy the head
        COPY(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // copy the tail
    }
    else
    {
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBLR != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // move the values tail
            MOVE(&pwTgtVals[~nKeys], &pwSrcVals[-nKeys], nKeys - nPos);
        }
  #endif // B_JUDYL
        MOVE(&pTgt[nPos+1], &pSrc[nPos], nKeys - nPos); // move the tail
    }

    pTgt[nPos] = cKey; // insert the key
    // Padding is redundant in some cases.
    // But avoiding it is probably more expensive than doing it.
    // This code assumes InflateEmbedded pads the list.
    //if ((pTgt != pSrc)
    //    || ((nKeys * sizeof(cKey) % sizeof(Bucket_t)) == 0)
    //    //|| 1
    //    || (nPos == nKeys))
    //{
    PAD(pTgt, nKeys + 1);
    //}

  #ifdef B_JUDYL
    return
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        (nBLR == cnBitsInD1) ?  &pwTgtVals[~(cKey & MSK(cnBitsInD1))] :
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
            &pwTgtVals[~nPos];
  #endif // B_JUDYL
}

#endif // defined(COMPRESSED_LISTS)

#endif // (cwListPopCntMax != 0)

// Figure out the length of the prefix that is common for all the keys in the
// list and wKey.
// Return the length of the remainder, i.e. the minimum nBL that will not
// result in a splay.
static int
SignificantBitCnt(qp, Word_t wKey, int nPopCnt)
{
    qv; (void)nPopCnt;
  #ifdef NO_SKIP_AT_TOP
    if (nBL >= cnBitsPerWord) {
        return nBL;
    }
  #endif // NO_SKIP_AT_TOP
    if (nPopCnt < 1) { // only if ListPopCntMax == 0 and no embedded keys
        return 1;
    }
    int nBLR = gnListBLR(qy); (void)nBLR;
    Word_t wMin, wMax, wSuffix;
  #if defined(COMPRESSED_LISTS)
    if (nBLR <= 8) {
        unsigned char *pcKeys = ls_pcKeysNATX(pwr, nPopCnt);
        wMin = pcKeys[0];
        wMax = pcKeys[nPopCnt - 1];
        wSuffix = wKey & MSK(8);
    } else if (nBLR <= 16) {
        unsigned short *psKeys = ls_psKeysNATX(pwr, nPopCnt);
        wMin = psKeys[0];
        wMax = psKeys[nPopCnt - 1];
        wSuffix = wKey & MSK(16);
      #if (cnBitsPerWord > 32)
    } else if (nBLR <= 32) {
        unsigned int *piKeys = ls_piKeysNATX(pwr, nPopCnt);
        wMin = piKeys[0];
        wMax = piKeys[nPopCnt - 1];
        wSuffix = wKey & MSK(32);
      #endif // (cnBitsPerWord > 32)
    } else
  #endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeys(pwr, nBL);
        wMin = pwKeys[0];
        wMax = pwKeys[nPopCnt - 1];
        wSuffix = wKey;
    }
    assert(((wSuffix ^ wMin) | (wSuffix ^ wMax)) != 0);
    return LOG((wSuffix ^ wMin) | (wSuffix ^ wMax)) + 1;
}

#ifdef B_JUDYL
Word_t *
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtBitmap(qpa, Word_t wKey);

#if (cwListPopCntMax != 0)

#if defined(EMBED_KEYS)

static Word_t
DeflateExternalList(qpa, int nPopCnt);

#endif // defined(EMBED_KEYS)

#endif // (cwListPopCntMax != 0)

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
InsertCleanup(qpa, Word_t wKey)
{
    qva; (void)wKey;
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
        if (InflateBmSwTest(qya)) {
  #if defined(B_JUDYL) && defined(EMBED_KEYS)
            // InflateBmSw may change pwValue of all embedded keys.
            Word_t *pwValueRet
                = InflateBmSw(qya, wKey, nBLR);
            if (pwValueRet != NULL) {
                pwValue = pwValueRet;
            }
  #else // defined(B_JUDYL) && defined(EMBED_KEYS)
            InflateBmSw(qya, wKey, nBLR);
  #endif // #else defined(B_JUDYL) && defined(EMBED_KEYS)
            DBGI(printf("\n## After InflateBmSw\n"));
            DBGI(Dump(pwRootLast, /*wPrefix*/ wKey, cnBitsPerWord));
        }
    }
#endif // defined(CODE_BM_SW)

  #ifdef BITMAP
      #if (cn2dBmMaxWpkPercent != 0) // conversion to big bitmap enabled
    int nDL = nBL_to_nDL(nBL);

    (void)nDL;
    Word_t wPopCnt;
    int nBLR = GetBLR(pwRoot, nBL);
    if ((nBLR == nDL_to_nBL(2))
        && tp_bIsSwitch(nType)
          #ifndef SKIP_TO_BITMAP
        && !tp_bIsSkip(nType)
          #endif // #ifndef SKIP_TO_BITMAP
          #if defined(CODE_BM_SW)
        && !tp_bIsBmSw(nType) // can't handle it yet
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
        // Things get funky with very small digit size:
        // - Full pop might be reached before embedded keys are exhausted.
        // - Bitmaps may not get created.
        // - Conversion may not occur before full pop.
        // Things get funky with POP_WORD:
        // - PWR_wPopCntBL may return full pop, i.e. 1 << nBLR.
        && ((wPopCnt = PWR_wPopCntBL(pwRoot, pwr, nBLR)),
            /*assert(wPopCnt < MSK(nBLR)),*/
            wPopCnt * cn2dBmMaxWpkPercent
                > (EXP(nBLR - cnLogBitsPerWord) * 100)))
    {
        DBGI(printf("Converting BM leaf.\n"));

        // Can't disable this one by ALLOW_EMBEDDED_BITMAP.
        // The code doesn't work yet.
        assert((cnBitsInD1 > cnLogBitsPerLink) || !cbEmbeddedBitmap);
        //assert((cnBitsInD2 > cnLogBitsPerLink) || !cbEmbeddedBitmap);

        //Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord);
        //printf("wRoot %p wPopCnt %ld\n", (void *)wRoot, wPopCnt);
        DBGI(printf("\n# IC: Creating a bitmap at nBL %d nBLR %d.\n",
                    nBL, nBLR));

        int nBW;
          #if defined(USE_XX_SW)
              #if defined(USE_XX_SW_ONLY_AT_DL2)
        assert(tp_bIsXxSw(nType));
              #endif // defined(USE_XX_SW_ONLY_AT_DL2)
        if (tp_bIsXxSw(nType)) { nBW = pwr_nBW(&wRoot); } else
          #endif // defined(USE_XX_SW)
        {
            assert(nBLR == nDL_to_nBL(nBL_to_nDL(nBLR)));
            nBW = nBLR_to_nBW(nBLR);
        }

        // Allocate a new bitmap.
        DBGI(printf("# IC: NewBitmap nBLR %d nBW %d wPopCnt %" _fw"d"
                        " wWordsAllocated %" _fw"d wPopCntTotal %" _fw"d.\n",
                    nBLR, nBW, wPopCnt, wWordsAllocated, wPopCntTotal));
        Word_t *pwrNew = NewBitmap(qya, /* nBLNew */ nBLR, wKey, wPopCnt);
// NewBitmap installs a new wRoot.
// nBL, pLn, pwRoot have not changed, but *pwRoot has.
        Word_t *pwBitmap = ((BmLeaf_t*)pwrNew)->bmlf_awBitmap;

        // Why are we not using InsertAll here to insert the keys?
        // It doesn't handle switches yet.

        int nBLLn = nBLR - nBW;
        assert(nBLLn >= cnLogBitsPerByte);

          #ifdef EMBED_KEYS
          #ifdef REVERSE_SORT_EMBEDDED_KEYS
        int nPopCntMaxLn = EmbeddedListPopCntMax(nBLLn);
          #endif // REVERSE_SORT_EMBEDDED_KEYS
          #endif // EMBED_KEYS
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
          #ifdef _LNX
            Word_t* pwLnXLn = gpwLnX(qyx(Ln), EXP(nBW), ww);
          #endif // _LNX
          #if defined(EMBED_KEYS)
            if (nTypeLn == T_EMBEDDED_KEYS) {
                goto embeddedKeys;
embeddedKeys:;
                int nPopCntLn = wr_nPopCnt(wRootLn, nBLLn);
                Word_t wBLM = MSK(nBLLn); // Bits left mask.
                for (int nPos = 0; nPos < nPopCntLn; nPos++) {
                    int nBitNum
                        = ((wRootLn
                            >> (cnBitsPerWord
                                - (
              #ifdef REVERSE_SORT_EMBEDDED_KEYS
                                    (nPopCntMaxLn - nPos)
              #else // REVERSE_SORT_EMBEDDED_KEYS
                                    (nPos + 1)
              #endif // REVERSE_SORT_EMBEDDED_KEYS else
                                        * nBLLn)))
                                 & wBLM);
              #if (cnBitsInD1 < cnLogBitsPerWord)
                    SetBitByByte(&((uint8_t*)pwBitmap)[
                                     ww * EXP(nBLLn - cnLogBitsPerByte)],
                                 nBitNum);
              #else // (cnBitsInD1 < cnLogBitsPerWord)
                    SetBit(&pwBitmap[ww * EXP(nBLLn - cnLogBitsPerWord)],
                           nBitNum);
              #endif // else (cnBitsInD1 < cnLogBitsPerWord)
              #if cnWordsBm2Cnts != 0
                    Word_t wKeySuffix = (ww << nBLLn) + nBitNum;
                    ++gpxBitmapCnts(qya, nBLR)[
                        wKeySuffix >> cnLogBmlfBitsPerCnt];
              #endif // cnWordsBm2Cnts != 0
                }
                continue;
            }
          #endif // defined(EMBED_KEYS)
            if (nTypeLn == T_BITMAP) {
                Word_t *pwBitmapLn = ((BmLeaf_t*)pwrLn)->bmlf_awBitmap;
                memcpy(&((uint8_t*)pwBitmap)[
                           ww * EXP(nBLLn - cnLogBitsPerByte)],
                       pwBitmapLn, EXP(nBLLn - cnLogBitsPerByte));
                for (int nW = 0;
                     nW < MAX((int)EXP(cnBitsInD1) / cnBitsPerWord, 1); ++nW)
                {
          #if cnWordsBm2Cnts != 0
                    int nPopCnt = __builtin_popcountll(pwBitmapLn[nW]);
                    gpxBitmapCnts(qya, nBLR)
                            [((ww << nBLLn)
                                + (nW << cnLogBitsPerWord))
                                    >> cnLogBmlfBitsPerCnt]
                        += nPopCnt;
          #endif // cnWordsBm2Cnts != 0
                }
                Word_t wPopCntLn = gwBitmapPopCnt(qyax(Ln), nBLLn);
                OldBitmap(pwrLn, nBLLn, wPopCntLn);
                continue;
            }
          #if (cwListPopCntMax != 0)
            {
                assert(tp_bIsList(nTypeLn));
                int nBLRLn = gnListBLR(qyx(Ln));
                assert((ww & NBPW_MSK(nBLRLn - nBLLn)) == 0);
                int nPopCntLn = PWR_xListPopCnt(pwRootLn, pwrLn, nBLRLn);
                Word_t wKeySuffix;
              #ifdef COMPRESSED_LISTS
                if (nBLRLn <= 8) {
                    uint8_t *pcKeysLn = ls_pcKeysNATX(pwrLn, nPopCntLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        wKeySuffix
                            = (ww << nBLLn) + (pcKeysLn[nn] & NZ_MSK(nBLRLn));
                        SetBit(pwBitmap, wKeySuffix);
                  #if cnWordsBm2Cnts != 0
                        ++gpxBitmapCnts(qya, nBLR)[
                            wKeySuffix >> cnLogBmlfBitsPerCnt];
                  #endif // cnWordsBm2Cnts != 0
                    }
                } else
                  #if (cnBitsPerWord == 64)
                if (nBLRLn <= 16)
                  #endif // (cnBitsPerWord == 64)
                {
                  #if (cnBitsPerWord == 32)
                    assert(nBLRLn <= 16);
                  #endif // (cnBitsPerWord == 32)
                    uint16_t *psKeysLn = ls_psKeysNATX(pwrLn, nPopCntLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        wKeySuffix
                            = (ww << nBLLn) + (psKeysLn[nn] & NZ_MSK(nBLRLn));
                        SetBit(pwBitmap, wKeySuffix);
                  #if cnWordsBm2Cnts != 0
                        ++gpxBitmapCnts(qya, nBLR)[
                            wKeySuffix >> cnLogBmlfBitsPerCnt];
                  #endif // cnWordsBm2Cnts != 0
                    }
                }
                  #if (cnBitsPerWord == 64)
                else {
                    assert(nBLRLn <= 32);
                    uint32_t *piKeysLn = ls_piKeysNATX(pwrLn, nPopCntLn);
                    for (int nn = 0; nn < nPopCntLn; nn++) {
                        wKeySuffix
                            = (ww << nBLLn) + (piKeysLn[nn] & NZ_MSK(nBLRLn));
                        SetBit(pwBitmap, wKeySuffix);
                      #if cnWordsBm2Cnts != 0
                        ++gpxBitmapCnts(qya, nBLR)[
                            wKeySuffix >> cnLogBmlfBitsPerCnt];
                      #endif // cnWordsBm2Cnts != 0
                    }
                }
                  #endif // (cnBitsPerWord == 64)
              #else // COMPRESSED_LISTS
                Word_t *pwKeysLn = ls_pwKeysNATX(pwrLn, nPopCntLn);
                for (int nn = 0; nn < nPopCntLn; nn++) {
                    wKeySuffix
                        = (ww << nBLLn) + (pwKeysLn[nn] & NZ_MSK(nBLRLn));
                    SetBit(pwBitmap, wKeySuffix);
                  #if cnWordsBm2Cnts != 0
                    ++gpxBitmapCnts(qya, nBLR)[
                        wKeySuffix >> cnLogBmlfBitsPerCnt];
                  #endif // cnWordsBm2Cnts != 0
                }
              #endif // COMPRESSED_LISTS
                assert(nPopCntLn != 0);
              #ifdef XX_LISTS
                if (nTypeLn == T_XX_LIST) {
                    for (Word_t wx = 1; wx < EXP(nBW) - ww; ++wx) {
                        if (pLnLn[wx].ln_wRoot != wRootLn) {
                            break;
                        }
                        pLnLn[wx].ln_wRoot = WROOT_NULL;
                    }
                }
              #endif // XX_LISTS
                OldList(pwrLn, nPopCntLn, nBLRLn, nTypeLn);
            }
          #endif // (cwListPopCntMax != 0)
        }

        OldSwitch(&wRoot, nBL
          #if defined(CODE_BM_SW)
                , /* bBmSw */ 0, /* nLinks */ 0
          #endif // defined(CODE_BM_SW)
                  );

          #if defined(DEBUG)
        int count = 0;
        for (int jj = 0; jj < (int)EXP(nBLR - cnLogBitsPerWord); jj++)
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
            Dump(pwRootLast, /* wPrefix */ wKey, cnBitsPerWord);
            exit(1);
        }
        assert(count == (int)wPopCnt);
          #endif // defined(DEBUG)
        DBGI(printf("\n## After Converting BM leaf\n"));
        DBGI(Dump(pwRootLast, /*wPrefix*/ wKey, cnBitsPerWord));
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

#ifdef BITMAP
// Insert all of the keys from a list into a bitmap that has already been
// created.
// For JudyL the bitmap should already be the right size.
// Make sure there is room for another key to be inserted because we're
// going to insert another key asap?
static void
InsertAllAtBitmap(qpa, qpx(Old), int nStart, int nPopCnt)
{
    qva; qvx(Old);
    DBGI(Log(qy,       "InsertAllAtBitmap qy      "));
    DBGI(Log(qyx(Old), "InsertAllAtBitmap qyx(Old)"));
    DBGI(printf("# nStart %d nPopCnt %d\n", nStart, nPopCnt));
    int nBLROld = gnListBLR(qyx(Old)); (void)nBLROld;
    DBGI(printf("# nBLROld %d\n", nBLROld));
//   assert(nBLOld == cnBitsInD1);
  #ifdef _BMLF_BM_IN_LNX
    Word_t* pwBitmap = pwLnX;
  #else // _BMLF_BM_IN_LNX
    Word_t* pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
  #endif // else _BMLF_BM_IN_LNX
    DBGI(printf("# pwBitmap %p\n", pwBitmap));
  #ifdef B_JUDYL
    Word_t *pwValues = gpwBitmapValues(qy, cnBitsInD1);
  #endif // B_JUDYL
    uint8_t *pcKeys = NULL; (void*)pcKeys;
    uint16_t *psKeys = NULL; (void*)psKeys;
  #if (cnBitsPerWord > 32)
    uint32_t *piKeys = NULL; (void*)piKeys;
  #endif // (cnBitsPerWord > 32)
    Word_t *pwKeys = NULL;
    //Link_t *pLnOld = STRUCT_OF(pwRoot, Link_t, ln_pwRoot);
    //Word_t *wRootOld = *pwRoot;
    //int nTypeOld = wr_nType(wRootOld);
    //Word_t *pwrOld = wr_pwr(wRootOld);
    BJL(Word_t *pwValuesOld = gpwValues(qyx(Old)));
  #ifdef COMPRESSED_LISTS
    if (nBLROld <= 8) {
        pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
    } else if (nBLROld <= 16) {
        psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
      #if (cnBitsPerWord > 32)
    } else if (nBLROld <= 32) {
        piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
      #endif // (cnBitsPerWord > 32)
    } else
  #endif // COMPRESSED_LISTS
    {
        pwKeys = ls_pwKeysNATX(pwrOld, nPopCnt);
    }
    for (int nn = nStart; nn < nStart + nPopCnt; ++nn) {
        Word_t wKeyLoop;
  #ifdef COMPRESSED_LISTS
        if (nBLROld <= 8) {
            wKeyLoop = pcKeys[nn];
        } else if (nBLROld <= 16) {
            wKeyLoop = psKeys[nn];
      #if (cnBitsPerWord > 32)
        } else if (nBLROld <= 32) {
            wKeyLoop = piKeys[nn];
      #endif // (cnBitsPerWord > 32)
        } else
  #endif // COMPRESSED_LISTS
        {
            wKeyLoop = pwKeys[nn];
        }
        wKeyLoop &= MSK(cnBitsInD1);
        //DBGI(printf("# SetBit(wKeyLoop 0x%zx)\n", wKeyLoop));
        // Streamlined version of InsertAtBitmap.
        // The bitmap has already been sized.
        SetBit(pwBitmap, wKeyLoop);
  #if cn2dBmMaxWpkPercent != 0
        int nBLR = gnBLR(qy);
        if (nBLR == cnBitsLeftAtDl2) {
      #if cnWordsBm2Cnts != 0
            ++gpxBitmapCnts(qya, nBLR)[
                (wKeyLoop & MSK(nBLR)) >> cnLogBmlfBitsPerCnt];
      #endif // cnWordsBm2Cnts != 0
        }
  #endif // cn2dBmMaxWpkPercent != 0
  #ifdef B_JUDYL
        if (BM_UNPACKED(wRoot)) {
  #ifdef BMLF_INTERLEAVE
            Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t);
            wWordsHdr += EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
            Word_t wDigit = wKeyLoop & MSK(cnBitsInD1);
            int nLogBmPartBmBits = cnBitsInD1 - cnLogBmlfParts;
            int nBmPartBmWords = (nLogBmPartBmBits < cnLogBitsPerWord)
                ? 1 : EXP(nLogBmPartBmBits - cnLogBitsPerWord);
            int nBmPartSz = nBmPartBmWords + EXP(cnBitsInD1 - cnLogBmlfParts);
            int nBmPartNum = wDigit >> (cnBitsInD1 - cnLogBmlfParts);
            Word_t wKeyLeft = wDigit & MSK(cnBitsInD1 - cnLogBmlfParts);
            Word_t* pwBmPart = &pwr[wWordsHdr] + nBmPartSz * nBmPartNum;
            int nBmBitPartNum = nBmPartNum;
      #ifdef BMLFI_SPLIT_BM
            nBmBitPartNum += !!(wDigit & EXP(cnBitsInD1 - cnLogBmlfParts - 1));
      #endif // BMLFI_SPLIT_BM
            Word_t* pwBmBitPart = &pwr[wWordsHdr] + nBmPartSz * nBmBitPartNum;
            SetBit(pwBmBitPart
      #ifdef BMLFI_BM_AT_END
                   + nBmPartSz - nBmPartBmWords
      #endif // BMLFI_BM_AT_END
                 , wKeyLeft);
            pwValues = pwBmPart;
      #ifndef BMLFI_BM_AT_END
            pwValues += nBmPartBmWords;
      #endif // !BMLFI_BM_AT_END
            pwValues[wKeyLeft] = pwValuesOld[~nn];
  #else // BMLF_INTERLEAVE
      #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
            if (nBLROld == cnBitsInD1) {
                pwValues[wKeyLoop]
                    = pwValuesOld[~(wKeyLoop & MSK(cnBitsInD1))];
            } else
      #endif // !PACK_L1_VALUES && cnBitsInD1 <= 8
            { pwValues[wKeyLoop] = pwValuesOld[~nn]; }
  #endif // BMLF_INTERLEAVE
        } else {
      #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
            if (nBLROld == cnBitsInD1) {
                *pwValues++ = pwValuesOld[~(wKeyLoop & MSK(cnBitsInD1))];
            } else
      #endif // !PACK_L1_VALUES && cnBitsInD1 <= 8
            *pwValues++ = pwValuesOld[~nn];
        }
  #endif // B_JUDYL
    }
  #ifdef BMLF_CNTS
    if (BM_UNPACKED(wRoot)) { } else
    {
      #ifdef BMLF_POP_COUNT_32
    uint32_t* pu32Bitmap = (uint32_t*)pwBitmap;
          #if defined(BMLF_CNTS_IN_LNX)
    Word_t* pwCnts = pwLnX;
          #else // BMLF_CNTS_IN_LNX
    Word_t* pwCnts = (Word_t*)((BmLeaf_t*)pwr)->bmlf_au8Cnts;
          #endif // BMLF_CNTS_IN_LNX else
    for (Word_t ww = 0; ww < EXP(cnBitsInD1 - 5); ++ww) {
        ((uint8_t*)pwCnts)[ww] = PopCount32(pu32Bitmap[ww]);
    }
          #ifdef BMLF_CNTS_CUM
    *pwCnts *= 0x0101010101010100;
          #endif // BMLF_CNTS_CUM
      #else // BMLF_POP_COUNT_32
          #ifdef BMLF_POP_COUNT_8
    int nSum = 0;
    for (int nn = 0; nn < (1 << (cnBitsInD1 - cnLogBitsPerByte)); ++nn) {
        ((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn] = nSum;
        nSum += PopCount8(((uint8_t*)pwBitmap)[nn]);
    }
          #elif defined(BMLF_POP_COUNT_1)
    int nSum = 0;
    for (int nn = 0; nn < (1 << cnBitsInD1); ++nn) {
        ((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn] = nSum;
        nSum += BitIsSet(pwBitmap, nn);
    }
          #else // BMLF_POP_COUNT_8
              #ifdef BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = (uint8_t*)pwLnX;
              #else // BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
              #endif // BMLF_CNTS_IN_LNX else
    for (int nn = 0; nn < (1 << (cnBitsInD1 - cnLogBitsPerWord)); ++nn) {
        pu8Cnts[nn] = PopCount64(pwBitmap[nn]);
    }
              #ifdef BMLF_CNTS_CUM
    *(Word_t*)pu8Cnts *= 0x01010100;
                  #ifdef BM_DSPLIT
    for (int nn = 0; nn < (1 << (cnBitsInD1 - cnLogBitsPerWord)); ++nn) {
        pu8Cnts[4+nn] = PopCount64(pwBitmap[nn]);
    }
                  #endif // BM_DSPLIT
              #endif // BMLF_CNTS_CUM
          #endif // #else BMLF_POP_COUNT_8
      #endif // #else BMLF_POP_COUNT_32
    }
  #endif // BMLF_CNTS
    CheckBitmapGuardband(pwr, nBL, nPopCnt);
}
#endif // BITMAP

#if (cwListPopCntMax != 0)

static int
SplayMaxPopCnt(Word_t *pwRootOld, int nBLOld, Word_t wKey, int nBLNew)
{
    Word_t wRootOld = *pwRootOld;
    assert(tp_bIsList(wr_nType(wRootOld)));
    if ((wr_nType(WROOT_NULL) == T_LIST) && (wRootOld == WROOT_NULL)) {
        assert(auListPopCntMax[nBLOld] == 0);
        return 0;
    }
    assert(wRootOld != WROOT_NULL);
    assert(auListPopCntMax[nBLOld] != 0);
    Word_t *pwrOld = wr_pwr(wRootOld);
    int nBLROld = gnListBLR(qyx(Old));
  #ifdef NO_TYPE_IN_XX_SW
    assert(nBLOld >= nDL_to_nBL(2));
  #endif // NO_TYPE_IN_XX_SW
    int nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLROld);
    assert(nPopCnt != 0); // should have been handled above
    Word_t wPrefixKey = wKey & ~MSK(nBLNew);
    int nPopCntMax = 0;
    int nnStart = 0; (void)nnStart;
#if defined(COMPRESSED_LISTS)
    if (nBLROld <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
        Word_t wPrefix = pcKeys[0] & ~MSK(nBLNew);
        wPrefixKey &= MSK(8);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefixNew = pcKeys[nn] & ~MSK(nBLNew);
            if (wPrefixNew != wPrefix) {
lastDigit8:;
                if (nn - nnStart + (wPrefixKey == wPrefix) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (wPrefixKey == wPrefix);
                }
                nnStart = nn;
                wPrefix = wPrefixNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                goto lastDigit8;
            }
        }
    } else if (nBLROld <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
        Word_t wPrefix = psKeys[0] & ~MSK(nBLNew);
        wPrefixKey &= MSK(16);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefixNew = psKeys[nn] & ~MSK(nBLNew);
            if (wPrefixNew != wPrefix) {
lastDigit16:;
                if (nn - nnStart + (wPrefixKey == wPrefix) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (wPrefixKey == wPrefix);
                }
                nnStart = nn;
                wPrefix = wPrefixNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                goto lastDigit16;
            }
        }
#if (cnBitsPerWord > 32)
    } else if (nBLROld <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
        Word_t wPrefix = piKeys[0] & ~MSK(nBLNew);
        wPrefixKey &= MSK(32);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefixNew = piKeys[nn] & ~MSK(nBLNew);
            if (wPrefixNew != wPrefix) {
lastDigit32:;
                if (nn - nnStart + (wPrefixKey == wPrefix) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (wPrefixKey == wPrefix);
                }
                nnStart = nn;
                wPrefix = wPrefixNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                goto lastDigit32;
            }
        }
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwrOld, nBLROld, nPopCnt);
        Word_t wPrefix = pwKeys[0] & ~MSK(nBLNew);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefixNew = pwKeys[nn] & ~MSK(nBLNew);
            if (wPrefixNew != wPrefix) {
lastDigit:;
                if (nn - nnStart + (wPrefixKey == wPrefix) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (wPrefixKey == wPrefix);
                }
                nnStart = nn;
                wPrefix = wPrefixNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                goto lastDigit;
            }
        }
    }
    return nPopCntMax;
}

static void
UpdateDist(qpa, int nPopCnt)
{
    qva;
    (void)nPopCnt;
  #ifdef DSPLIT_16
    uint16_t* psKeys = ls_psKeysNATX(pwr, nPopCnt);
      #ifdef DS_8_WAY
    int nn = 0;
    // for each subexpanse
    for (uint16_t xx = 0; xx < 8; ++xx) {
        // cnt keys in subexpanse
        int cnt = 0;
        while ((nn < nPopCnt) && ((psKeys[nn] >> 13) == xx)) {
            ++cnt;
            ++nn;
        }
        ((uint8_t*)pwLnX)[xx] = cnt;
    }
      #elif defined(DS_16_WAY) // DS_8_WAY
    *pwLnX = 0;
    int nn = 0;
    // for each subexpanse
    for (uint16_t xx = 0; xx < 16; ++xx) {
        // cnt keys in subexpanse
        int cnt = 0;
        while ((nn < nPopCnt) && ((psKeys[nn] >> 12) == xx)) {
            ++nn;
            if (++cnt == 15) {
                break;
            }
        }
        SetBits(pwLnX, /*nBits*/ 4, /*nLsb*/ xx * 4, cnt);
        assert(PopCount64(*pwLnX) <= nPopCnt);
    }
      #elif defined(DS_4_WAY_A)
    // pwLnx[n] == first virtual key in partition n+1
    *pwLnX = (Word_t)0xffff << 48;
    int nKeyNumX4 = nPopCnt;
    for (int xx = 1; xx < 4; ++xx) {
        int nKey0 = psKeys[ nKeyNumX4 / 4];
        int nKey1 = psKeys[(nKeyNumX4 + 3) / 4];
        ((uint16_t*)pwLnX)[xx - 1]
            = nKey0 + (nKey1 - nKey0) * (nKeyNumX4 % 4) / 4;
        nKeyNumX4 += nPopCnt;
    }
      #elif defined(DS_8_WAY_A)
    // pwLnx[nPart] == first key in partition nPart
    for (int nPart = 0; nPart < 8; ++nPart) {
        ((uint8_t*)pwLnX)[nPart] = psKeys[nPart * nPopCnt / 8] >> 8;
    }
// nPopCnt: 1; 0, 0, 0, 0, 0, 0, 0, 0
// nPopCnt: 2; 0, 0, 0, 0, 1, 1, 1, 1
// nPopCnt: 3; 0, 0, 0, 1, 1, 1, 2, 2
// nPopCnt: 4; 0, 0, 1, 1, 2, 2, 3, 3
      #elif defined(DS_8_WAY_B)
    // pwLnx[nPart] == last key in partition nPart
    for (int nPart = 0; nPart < 8; ++nPart) {
        ((uint8_t*)pwLnX)[nPart]
            = psKeys[((nPart + 1) * nPopCnt + 7) / 8 - 1] >> 8;
// nPopCnt: 1; 0, 0, 0, 0, 0, 0, 0, 0
// nPopCnt: 2; 0, 0, 0, 0, 1, 1, 1, 1
// nPopCnt: 3; 0, 0, 1, 1, 1, 2, 2, 2
// nPopCnt: 4; 0, 0, 1, 1, 2, 2, 3, 3
    }
      #elif defined(DS_16_WAY_A)
    // pwLnx[n] == first virtual key in partition n+1
    *pwLnX = (Word_t)0xf << 60;
    int nKeyNumX16 = nPopCnt;
    for (int xx = 1; xx < 16; ++xx) {
        int nKey0 = psKeys[ nKeyNumX16 / 16];
        int nKey1 = psKeys[(nKeyNumX16 + 15) / 16];
        SetBits(pwLnX, 4, (xx - 1) * 4,
            (nKey0 + (nKey1 - nKey0) * (nKeyNumX16 % 16) / 16) >> 12);
        nKeyNumX16 += nPopCnt;
    }
//printf("UpdateDist nPopCnt %d *pwLnX 0x%zx\n", nPopCnt, *pwLnX);
      #else // DS_8_WAY elif DS_16_WAY
          #ifdef DS_AVG
    // average subexpanse population
    // It is not possible for all subexp pops to be greater than avg.
    // It is not possible for all subexp pops to be less than avg.
    // It is possible that no subexp pop is less than avg.
    // It is possible that no subexp pop is greater than avg.
    // It is possible that all subexp pops are equal avg.
    // It is possible that all subexp pops are equal or less than avg.
    // It is possible that all subexp pops are equal or greater than avg.
    int nAvg = nPopCnt / 64;
    *pwLnX = -(Word_t)1;
    int nn = 0;
    // for each subexpanse
    for (uint16_t xx = 0; xx < 64; ++xx) {
        // cnt keys in subexpanse
        int cnt = 0;
        while ((nn < nPopCnt) && (psKeys[nn] >> 10) == xx) {
            ++cnt;
            ++nn;
        }
        // if subexp pop > average, set bit
        if (cnt <= nAvg) {
            *pwLnX &= ~((Word_t)1 << xx);
        }
    }
          #else // DS_AVG
    // pwLnX points at the 2nd word of the link (2nd word of the JP).
    // The 2nd word may be adjacent to the 1st word. Or it may be remote.
    // It depends on how the library was built.
    // Set each bit in *pwLnX if the corresponding Subxanse has any
    // keys in it.
    *pwLnX = 0; // 2nd word of link
    for (int xx = 0; xx < nPopCnt; ++xx) {
        *pwLnX |= (Word_t)1 << (psKeys[xx] >> 10);
    }
              #ifdef DS_NO_CHECK
    // Set the last bit instead of the bit that corresponds to the last
    // populated subexpanse so we don't have to check for out of bounds
    // to avoid a crash during lookup.
    *pwLnX &= ~((Word_t)1 << (psKeys[nPopCnt - 1] >> 10));
    *pwLnX |= (Word_t)1 << 63;
              #endif // DS_NO_CHECK
          #endif // DS_AVG else
          #ifdef DS_SAVE_DIV
    // Count the number of Subexpanses that have keys.
    int nSubxCnt = PopCount64(*pwLnX);
    // nMagic / 128 == nSubxCnt rounded up to a power of 2 / nSubxCnt
    // nMagic = 128 * nSubxCnt rounded up to a power of 2 / nSubxCnt
    // nMagic allows us to shift in Dsplit rather than dividing by nSubxCnt.
    int nMagic
        = 128 * (1 << (63 - __builtin_clzll((nSubxCnt << 1) - 1))) / nSubxCnt;
    // Save nMagic in the 1st word of the link (1st word of the JP).
    SetBits(pwRoot, cnBitsCnt1, cnLsbCnt1, nMagic);
          #endif // DS_SAVE_DIV
      #endif // DS_8_WAY elif DS_16_WAY else
  #endif // DSPLIT_16
}

// Insert each key from pwRootOld into qpa. Then free pwRootOld.
// pwRootOld is a link to an external list (possibly empty).
// qpa is a link to a switch.
// Is qpa always a newly-created, empty switch? No.
// Do we know that all the links in the switch into which *pwRootOld will be
// inserted are empty? Yes.
// wKey contains the prefix for pwRootOld and qpa.
static void
Splay(qpa, Word_t *pwRootOld, int nBLOld, Word_t wKey)
{
    qva;
  #ifdef DEBUG
    int nPopCntMax = 0;
  #endif // DEBUG
    Word_t wRootOld = *pwRootOld;
    DBGI(printf("\n# Splay nBLOld %d pwRootOld %p nBL %d pLn %p pwRoot %p",
                nBLOld, pwRootOld, nBL, pLn, pwRoot));
  #ifdef _LNX
    DBGI(printf(" pwLnX %p", pwLnX));
  #endif // _LNX
    DBGI(printf("\n"));
    // We don't have/know pwLnXOld for nBLOld < cnBitsPerWord if REMOTE_LNX.
  #ifndef REMOTE_LNX
    DBGI(DumpX(qyax(Old), wKey));
  #endif // !REMOTE_LNX
    DBGI(DumpX(qya, wKey));

    // Splay serves no purpose, and is intentionally a no-op, if wRootOld
    // is empty, i.e. if wRootOld == WROOT_NULL.
    // but we might have to check before calling it more than one place to
    // avoid handling it in just one place here.
    // If cnListPopCntMax<nBLOld> == 0 && EmbeddedListPopCntMax(nBLOld) == 0
    // or cnListPopCntMax<nBLOld> == 0 && NO_EMBED_KEYS || POP_CNT_MAX_IS_KING,
    // then the list is full with no keys in it, and TransformList and other
    // code might call Splay after creating a new switch.
    assert(tp_bIsList(wr_nType(wRootOld)));
    if ((wr_nType(WROOT_NULL) == T_LIST) && (wRootOld == WROOT_NULL)) {
        assert(auListPopCntMax[nBLOld] == 0);
        return;
    }
    assert(wRootOld != WROOT_NULL);
    assert(auListPopCntMax[nBLOld] != 0);

    int nBLROld = gnListBLR(qyx(Old));
    // Even the following commented-out assertion blows with DOUBLE_DOWN.
    // Too bad or we could simplify some code below.
    // assert(LOG(nBL - 1) <= LOG(nBLROld));

#ifdef KISS
    // Handle an empty link for KISS to possibly simplify callers.
    // NO_TYPE_IN_XX_SW really means "no type in XX_SW links with nBL < DL2".
  #ifdef NO_TYPE_IN_XX_SW
    if (nBLOld < nDL_to_nBL(2)) {
        if (wRootOld == ZERO_POP_MAGIC) { return; }
      #ifdef HANDLE_BLOWOUTS
        if ((wRootOld & BLOWOUT_MASK(nBLOld)) == ZERO_POP_MAGIC) { return; }
      #endif // HANDLE_BLOWOUTS
    }
  #endif // NO_TYPE_IN_XX_SW
#else // KISS
    // Assert that we have a non-empty link that may be a T_LIST[_UA] link
    // for non-KISS to streamline this function and make caller responsible
    // for not calling it with an empty link.
  #ifdef NO_TYPE_IN_XX_SW
    assert(nBLOld >= nDL_to_nBL(2));
  #endif // NO_TYPE_IN_XX_SW
#endif // KISS

    int nTypeOld = wr_nType(wRootOld);
    Word_t *pwrOld = wr_pwr(wRootOld);
    int nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLROld);
    assert(nPopCnt != 0); // empty list is handled above
    BJL(Word_t *pwValuesOld = gpwValues(qyx(Old)));

    // We can't assume this switch has nothing in it.
    // There may be non-empty links in the switch that don't overlap the
    // expanse of the list we are splaying.

    assert(tp_bIsList(nTypeOld));

    // We have an orphaned list. And a switch has been inserted that has
    // empty links just waiting for the list to be splayed into it.
    // This may not be the first list being inserted into the switch.
    // But it will be the first that will splay into the links that it
    // will splay into.

    int nBLR = GetBLR(pwRoot, nBL); // nBLR of switch
  #ifndef USE_XX_SW
    assert(nBLR <= nBLROld);
  #endif // #ifndef USE_XX_SW
    if (!tp_bIsSwitch(nType)
  #ifdef BITMAP
        && ((nBL != cnBitsInD1) || (nType != T_BITMAP))
  #endif // BITMAP
        )
    {
        printf("\nnBL %d nBLR %d nPopCnt %d nType %d\n",
               nBL, nBLR, nPopCnt, nType);
    }
    assert(tp_bIsSwitch(nType));

    int nBW = gnBW(qy, nBLR); // nBLR_to_nBW(nBLR);
  #ifdef CODE_BM_SW
    int bIsBmSw = tp_bIsBmSw(nType);
      #ifdef BM_SW_FOR_REAL
    int nLinkCntOrig; (void)nLinkCntOrig;
    // Save the old switch in case it ends up being suitable.
    Link_t linkOrig = *pLn;
      #endif // BM_SW_FOR_REAL
    if (bIsBmSw) {
        // We can't handle splay into a non-empty BmSw.
        // We pay no attention to the contents of the original switch.
        assert(GetPopCnt(qya) == 0);
        DBGI(printf("Splay bitmap switch"
                        " PopCnt %zd nBL %d nBLR %d nBW %d nBLROld %d\n",
                    GetPopCnt(qya), nBL, nBLR, nBW, nBLROld));
      #ifdef BM_SW_FOR_REAL
        nLinkCntOrig = BmSwLinkCnt(qya);
        // Create a new bitmap switch with EXP(nBW) links for staging.
        // No bits are set in the bitmap, but LinkCnt in WR is EXP(nBW).
        pwr = NewSwitchX(qya, wKey, nBLR,
          #if defined(CODE_XX_SW)
                         nBW,
          #endif // defined(CODE_XX_SW)
                         T_BM_SW,
                         /*nLinkGrpCnt*/ EXP(nBW - cnLogBmSwLinksPerBit), 0);
      #else // BM_SW_FOR_REAL
        pwr = wr_pwr(*pwRoot);
      #endif // #else BM_SW_FOR_REAL
        wRoot = *pwRoot;
        assert(wr_nType(wRoot) == nType);
        assert(wr_pwr(wRoot) == pwr);
        assert(GetBLR(pwRoot, nBL) == nBLR);
    }
  #endif // CODE_BM_SW

    Link_t *pLinks = pwr_pLinks((Switch_t *)pwr);
    int nBLLoop = nBLR - nBW; // nBL of the links in the switch
    assert(nBLLoop <= nBLROld);
#ifdef EMBED_KEYS
    int nEmbeddedListPopCntMax = EmbeddedListPopCntMax(nBLLoop);
#endif // EMBED_KEYS
    int nDigitKey = (wKey >> nBLLoop) & MSK(nBW); (void)nDigitKey;
    Word_t wDigitMask = ~MSK(nBLLoop) & NZ_MSK(nBLR);

    BJL(Word_t *pwValues = gpwValues(qyx(Old)));
    int nnStart = 0; (void)nnStart;
  #ifdef BM_SW_FOR_REAL
    int nLinkGrpCnt = 0;
  #endif // BM_SW_FOR_REAL
    int nIndex;
  #if cnSwCnts != 0
    Word_t* pwCnts = ((Switch_t*)pwr)->sw_awCnts;
  #endif // cnSwCnts != 0
#if defined(COMPRESSED_LISTS)
    if (nBLROld <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(8) & NZ_MSK(nBLR);
        int nDigit = ((pcKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((pcKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit8:;
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
#if 1
                int nShift = (nBW > nLogSwSubCnts(1))
                           ? (nBW - nLogSwSubCnts(1)) : 0;
                ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                        [nDigit >> (nBW - cnLogSwCnts - 2)]
                    += nPopCntLoop;
#endif
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, pcKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        assert(nBLLoop <= 8);
                        uint8_t* puc = (uint8_t*)&wLnXLoop;
                        COPY(puc, &pcKeys[nnStart], nPopCntLoop);
                        PAD64(puc, nPopCntLoop);
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), pcKeys[xx]);
                        }
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST; (void)nTypeLoop;
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    // copy the values
                    Word_t *pwValuesLoop = gpwValues(qyx(Loop));
                    COPY(&pwValuesLoop[-nPopCntLoop],
                         &pwValuesOld[-nPopCntLoop - nnStart], nPopCntLoop);
      #endif // B_JUDYL
                    // copy the keys
                    uint8_t *pcKeysLoop
                        = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                    COPY(pcKeysLoop, &pcKeys[nnStart], nPopCntLoop);
                    PAD(pcKeysLoop, nPopCntLoop);
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, pcKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                } else
  #endif // BITMAP
                {
                    // Don't know how to splay this sub list. Resort to Insert.
                    // Resorting to this is perilous.
                    // Insert could change *pLn/*pwRoot.
                    // Could it even change something higher up in the tree
                    // so pLn and pwRoot are not valid?
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
      #if cnSwCnts != 0
#if 1
                    int nShift = (nBW > nLogSwSubCnts(1))
                               ? (nBW - nLogSwSubCnts(1)) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                    ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                            [nDigit >> (nBW - cnLogSwCnts - 2)]
                        -= nPopCntLoop;
#endif
                    // Could this be problematic for XX_LISTS?
                    // The code for which carefully preps the subexpanse
                    // pop counts prior to calling Splay.
      #endif // cnSwCnts != 0
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, pcKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    swPopCnt(qya, nBLR, gwPopCnt(qya, nBLR) - nPopCntLoop);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit8;
            }
        }
    } else if (nBLROld <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(16) & NZ_MSK(nBLR);
        int nDigit = ((psKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((psKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit16:;
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
#if 1
                int nShift = (nBW > nLogSwSubCnts(1))
                           ? (nBW - nLogSwSubCnts(1)) : 0;
                ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                        [nDigit >> (nBW - cnLogSwCnts - 2)]
                    += nPopCntLoop;
#endif
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, psKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        assert(nBLLoop <= 16);
                        if (nBLLoop <= 8) {
                            uint8_t* puc = (uint8_t*)&wLnXLoop;
                            COPY(puc, &psKeys[nnStart], nPopCntLoop);
                            PAD64(puc, nPopCntLoop);
                        } else {
                            uint16_t* pus = (uint16_t*)&wLnXLoop;
                            COPY(pus, &psKeys[nnStart], nPopCntLoop);
                            PAD64(pus, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), psKeys[xx]);
                        }
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST;
  #ifdef UA_PARALLEL_128
                    if ((nBLLoop == 16) && (nPopCntLoop <= 6)) {
                        nTypeLoop = T_LIST_UA;
                    }
  #endif // UA_PARALLEL_128
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    // copy the values
                    Word_t *pwValuesLoop = gpwValues(qyx(Loop));
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    if (nBLLoop != cnBitsInD1)
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    {
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    // copy the keys
                    if (nBLLoop <= 8) {
                        uint8_t *pcKeysLoop
                            = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(pcKeysLoop, &psKeys[nnStart], nPopCntLoop);
                        PAD(pcKeysLoop, nPopCntLoop);
          #ifdef B_JUDYL
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                        if (nBLLoop == cnBitsInD1) {
                            for (int xx = 0; xx < nPopCntLoop; ++xx) {
                                pwValuesLoop[~(pcKeysLoop[xx] & MSK(nBLLoop))]
                                    = pwValuesOld[~(nnStart + xx)];
                            }
                        }
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
          #endif // B_JUDYL
                    } else {
                        uint16_t *psKeysLoop
                            = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(psKeysLoop, &psKeys[nnStart], nPopCntLoop);
                        PAD(psKeysLoop, nPopCntLoop);
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, psKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
      #if cnSwCnts != 0
#if 1
                    int nShift = (nBW > nLogSwSubCnts(1))
                               ? (nBW - nLogSwSubCnts(1)) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                    ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                            [nDigit >> (nBW - cnLogSwCnts - 2)]
                        -= nPopCntLoop;
#endif
      #endif // cnSwCnts != 0
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, psKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    swPopCnt(qya, nBLR, gwPopCnt(qya, nBLR) - nPopCntLoop);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit16;
            }
        }
#if (cnBitsPerWord > 32)
    } else if (nBLROld <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(32) & NZ_MSK(nBLR);
        int nDigit = ((piKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((piKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit32:;
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
                if (nBLR <= 16) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(1))
                               ? (nBW - nLogSwSubCnts(1)) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                        += nPopCntLoop;
#endif
                } else if (nBLR <= 32) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(2))
                               ? (nBW - nLogSwSubCnts(2)) : 0;
                    ((uint32_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                        += nPopCntLoop;
#endif
                } else {
      #if cnSwCnts == 1
                    if (!(nDigit >> (nBW - 1)))
      #endif // cnSwCnts == 1
#if 1
                    {
                        int nShift = (nBW > cnLogSwCnts)
                                   ? (nBW - cnLogSwCnts) : 0;
                        pwCnts[nDigit >> nShift] += nPopCntLoop;
                    }
#else
                    { pwCnts[nDigit >> (nBW - cnLogSwCnts)] += nPopCntLoop; }
#endif
                }
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, piKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        assert(nBLLoop <= 32);
                        if (nBLLoop <= 8) {
                            uint8_t* puc = (uint8_t*)&wLnXLoop;
                            COPY(puc, &piKeys[nnStart], nPopCntLoop);
                            PAD64(puc, nPopCntLoop);
                        } else
          #if (cnBitsPerWord > 32)
                        if (nBLLoop > 16) {
                            uint32_t* pui = (uint32_t*)&wLnXLoop;
                            COPY(pui, &piKeys[nnStart], nPopCntLoop);
                            assert(nPopCntLoop == 2);
                            // Work around bogus gcc array-bounds complaint.
                            PAD64(pui, /*nPopCntLoop*/ 2);
                        } else
          #endif // (cnBitsPerWord > 32)
                        {
                            uint16_t* pus = (uint16_t*)&wLnXLoop;
                            COPY(pus, &piKeys[nnStart], nPopCntLoop);
                            PAD64(pus, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
          #ifndef EK_XV
                        assert(nEmbeddedListPopCntMax == 1);
          #endif // #ifndef EK_XV
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), piKeys[xx]);
                        }
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST; (void)nTypeLoop;
      // BUG: What about UA_PARALLEL_128?
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    // copy the values
                    Word_t *pwValuesLoop = gpwValues(qyx(Loop));
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    if (nBLLoop != cnBitsInD1)
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    {
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    // copy the keys
                    if (nBLLoop <= 8) {
                        uint8_t *pcKeysLoop
                            = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(pcKeysLoop, &piKeys[nnStart], nPopCntLoop);
                        PAD(pcKeysLoop, nPopCntLoop);
          #ifdef B_JUDYL
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                        if (nBLLoop == cnBitsInD1) {
                            for (int xx = 0; xx < nPopCntLoop; ++xx) {
                                pwValuesLoop[~(pcKeysLoop[xx] & MSK(nBLLoop))]
                                    = pwValuesOld[~(nnStart + xx)];
                            }
                        }
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
          #endif // B_JUDYL
                    } else if (nBLLoop <= 16) {
                        uint16_t *psKeysLoop
                            = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(psKeysLoop, &piKeys[nnStart], nPopCntLoop);
                        PAD(psKeysLoop, nPopCntLoop);
      // BUG: What about UA_PARALLEL_128?
                        if (nBLLoop == 16) {
                            UpdateDist(qyax(Loop), nPopCntLoop);
                        }
                    } else {
                        uint32_t *piKeysLoop
                            = ls_piKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(piKeysLoop, &piKeys[nnStart], nPopCntLoop);
                        PAD(piKeysLoop, nPopCntLoop);
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, piKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    // Make this happen with:
                    // -DcnListPopCntMax16=8; btime -1 -E0x0f0f0fff
      #if cnSwCnts != 0
                if (nBLR <= 16) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(1))
                               ? (nBW - nLogSwSubCnts(1)) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                    ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                        -= nPopCntLoop;
#endif
                } else if (nBLR <= 32) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(2))
                               ? (nBW - nLogSwSubCnts(2)) : 0;
                    ((uint32_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                    ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                        -= nPopCntLoop;
#endif
                } else {
          #if cnSwCnts == 1
                    if (!(nDigit >> (nBW - 1)))
          #endif // cnSwCnts == 1
#if 1
                    {
                        int nShift = (nBW > cnLogSwCnts)
                                   ? (nBW - cnLogSwCnts) : 0;
                        pwCnts[nDigit >> nShift] -= nPopCntLoop;
                    }
#else
                    { pwCnts[nDigit >> (nBW - cnLogSwCnts)] -= nPopCntLoop; }
#endif
                }
      #endif // cnSwCnts != 0
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, piKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    swPopCnt(qya, nBLR, gwPopCnt(qya, nBLR) - nPopCntLoop);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit32;
            }
        }
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwrOld, nBLROld, nPopCnt);
        Word_t wBitsFromKey = 0;
        int nDigit = ((pwKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((pwKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit:;
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("pLnLoop %p nDigit 0x%02x nnStart %d nn %d\n",
                             pLnLoop, nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
                if (nBLR <= 16) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(1))
                               ? (nBW - nLogSwSubCnts(1)) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                        += nPopCntLoop;
#endif
                } else if (nBLR <= 32) {
#if 1
                    int nShift = (nBW > nLogSwSubCnts(2))
                               ? (nBW - nLogSwSubCnts(2)) : 0;
                    ((uint32_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                        += nPopCntLoop;
#endif
                } else {
      #if cnSwCnts == 1
                    if (!(nDigit >> (nBW - 1)))
      #endif // cnSwCnts == 1
#if 1
                    {
                        int nShift = (nBW > cnLogSwCnts)
                                   ? (nBW - cnLogSwCnts) : 0;
                        pwCnts[nDigit >> nShift] += nPopCntLoop;
                    }
#else
                    { pwCnts[nDigit >> (nBW - cnLogSwCnts)] += nPopCntLoop; }
#endif
                }
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, pwKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        if (nBLLoop <= 8) {
                            COPY((uint8_t*)pwLnXLoop, &pwKeys[nnStart],
                                 nPopCntLoop);
                            PAD64((uint8_t*)pwLnXLoop, nPopCntLoop);
                        } else
          #if (cnBitsPerWord > 32)
                        if (nBLLoop > 16) {
                            COPY((uint32_t*)pwLnXLoop, &pwKeys[nnStart],
                                 nPopCntLoop);
                            PAD64((uint32_t*)pwLnXLoop, nPopCntLoop);
                        } else
          #endif // (cnBitsPerWord > 32)
                        {
                            COPY((uint16_t*)pwLnXLoop, &pwKeys[nnStart],
                                 nPopCntLoop);
                            PAD64((uint16_t*)pwLnXLoop, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        DBGI(printf("Splay wRootLoop 0x%zx"
                                    " *(Word_t*)pwLnXLoop 0x%zx\n",
                                    wRootLoop, *(Word_t*)pwLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), pwKeys[xx]);
                        }
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST;
  #ifdef UA_PARALLEL_128
                    if ((nBLLoop == 16) && (nPopCntLoop <= 6)) {
                        nTypeLoop = T_LIST_UA;
                    }
  #endif // UA_PARALLEL_128
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    // copy the values
                    Word_t *pwValuesLoop = gpwValues(qyx(Loop));
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    if (nBLLoop != cnBitsInD1)
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                    {
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    // copy the keys
      #ifdef COMPRESSED_LISTS
                    if (nBLLoop <= 16) {
                        if (nBLLoop <= 8) {
                            uint8_t *pcKeysLoop
                                = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                            COPY(pcKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                            PAD(pcKeysLoop, nPopCntLoop);
          #ifdef B_JUDYL
          #if !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
                            if (nBLLoop == cnBitsInD1) {
                                for (int xx = 0; xx < nPopCntLoop; ++xx) {
                                    pwValuesLoop[
                                            ~(pcKeysLoop[xx] & MSK(nBLLoop))
                                                 ]
                                        = pwValuesOld[~(nnStart + xx)];
                                }
                            }
          #endif // !defined(PACK_L1_VALUES) && cnBitsInD1 <= 8
          #endif // B_JUDYL
                        } else {
                            uint16_t *psKeysLoop
                                = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                            COPY(psKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                            PAD(psKeysLoop, nPopCntLoop);
                            if (nBLLoop == 16) {
                                UpdateDist(qyax(Loop), nPopCntLoop);
                            }
                        }
          #if (cnBitsPerWord > 32)
                    } else if (nBLLoop <= 32) {
                        uint32_t *piKeysLoop
                            = ls_piKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(piKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                        PAD(piKeysLoop, nPopCntLoop);
          #endif // (cnBitsPerWord > 32)
                    } else
      #endif // COMPRESSED_LISTS
                    {
                        Word_t *pwKeysLoop
                            = ls_pwKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        COPY(pwKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                        PAD(pwKeysLoop, nPopCntLoop);
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, pwKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    // Make this happen with:
                    // -DcnListPopCntMax16=8; bcheck -B17
                    // How can we better handle this case?
                    // Avoid it with auListPopCntMax[m] <= auListPopCntMax[n]
                    // for m > n?
                    // I wonder if I could call ListIsFull here instead.
                    // I wonder if I could use InsertAll.
                    // I wonder if I could use InsertGuts in the loop.
                    // Can't use Insert(qya(Loop)) in this loop because Insert
                    // can't figure out pwLnXLoop.
      #if cnSwCnts != 0
                    if (nBLR <= 16) {
#if 1
                        int nShift = (nBW > nLogSwSubCnts(1))
                                   ? (nBW - nLogSwSubCnts(1)) : 0;
                        ((uint16_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                        ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                            -= nPopCntLoop;
#endif
                    } else if (nBLR <= 32) {
#if 1
                        int nShift = (nBW > nLogSwSubCnts(2))
                                   ? (nBW - nLogSwSubCnts(2)) : 0;
                        ((uint32_t*)pwCnts)[nDigit >> nShift] -= nPopCntLoop;
#else
                        ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                            -= nPopCntLoop;
#endif
                    } else {
          #if cnSwCnts == 1
                        if (!(nDigit >> (nBW - 1)))
          #endif // cnSwCnts == 1
                        {
#if 1
                            int nShift = (nBW > cnLogSwCnts)
                                       ? (nBW - cnLogSwCnts) : 0;
                            pwCnts[nDigit >> nShift] -= nPopCntLoop;
#else
                            pwCnts[nDigit >> (nBW - cnLogSwCnts)]
                                -= nPopCntLoop;
#endif
                        }
                    }
      #endif // cnSwCnts != 0
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, pwKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    swPopCnt(qya, nBLR, gwPopCnt(qya, nBLR) - nPopCntLoop);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit;
            }
        }
    }

  #ifdef DEBUG_INSERT
    int nSplayMaxPopCnt = SplayMaxPopCnt(pwRootOld, nBLOld, wKey, nBLLoop);
    if ((nPopCntMax != nSplayMaxPopCnt)
        && (nPopCntMax != nSplayMaxPopCnt + 1))
    {
        DBGI(printf("# nPopCntMax %d\n", nPopCntMax));
        DBGI(printf("# nSplayMaxPopCnt %d\n", nSplayMaxPopCnt));
        DBGI((printf("# nPopCntMax "), Dump(pwRootLast, 0, cnBitsPerWord)));
        DBGI((printf("# SplayMaxPopCnt "), Dump(pwRootOld, wKey, nBLOld)));
    }
    assert((nPopCntMax == nSplayMaxPopCnt)
        || (nPopCntMax == nSplayMaxPopCnt + 1));
  #endif // DEBUG_INSERT

    // Fix up pop count in the new switch.
    Word_t wPopCnt = 0;
    {
  #ifdef XX_LISTS
        if (wr_nType(*pwRootOld) != T_XX_LIST)
  #endif // XX_LISTS
        {
            // Is this in case the switch was not empty to begin with?
            wPopCnt = gwPopCnt(qya, nBLR) + nPopCnt;
            swPopCnt(qya, nBLR, wPopCnt);
        }
    }

    OldList(pwrOld, nPopCnt, nBLROld, nTypeOld);

  #ifdef BM_SW_FOR_REAL
    if (bIsBmSw) {
        //DBGI(printf("\n# Just before unstaging "));
        //DBGI(DumpX(qya, wKey));
        Link_t LnStaged = *pLn;
        Word_t* pwrStaged = pwr;
        Word_t* pwBmStaged = PWR_pwBm(&LnStaged.ln_wRoot, pwr, nBW);
      #ifdef REMOTE_LNX
        BJL(Word_t* pwLnXStaged = gpwLnX(qy, 1<<nBW, /*nDigit*/ 0));
      #endif // REMOTE_LNX
        if (nLinkGrpCnt == 1) {
            // Save the work of OldSwitch/NewSwitch if the switch that
            // came in is suitable.
            // How do we know the switch passed in has exactly one link?
            assert(nLinkCntOrig == (int)NBPW_EXP(cnLogBmSwLinksPerBit));
            // Restore original pwr and link count.
            // Do not overwrite *pwLnX in case SW_POP_IN_LNX && NO_REMOTE_LNX.
            pLn->ln_wRoot = linkOrig.ln_wRoot; // Both? Why?
      #ifdef BM_SW_BM_IN_WR_HB
            pLn->ln_wRoot &= MSK(cnBitsVirtAddr);
      #endif // BM_SW_BM_IN_WR_HB
            pwr = wr_pwr(linkOrig.ln_wRoot);
            // Pop count will be updated when we copy the switch.
        } else {
      #ifdef _LNX
            Word_t wLnX = (nBL < cnBitsPerWord) ? *pwLnX : 0;
      #endif // _LNX
            OldSwitch(&linkOrig.ln_wRoot, nBL, /*bBmSw*/ 1,
                      /*nLinks*/ 1 << cnLogBmSwLinksPerBit);
            pwr = NewSwitchX(qya, wKey, nBLR,
      #if defined(CODE_XX_SW)
                             nBW,
      #endif // defined(CODE_XX_SW)
                             T_BM_SW, nLinkGrpCnt, wPopCnt);
      #ifdef _LNX
            // NewSwitch zero'd *pwLnX.
            if (nBL < cnBitsPerWord) {
                *pwLnX = wLnX;
            }
      #endif // _LNX
        }
      #ifdef BM_SW_BM_IN_WR_HB
        *pwRoot |= LnStaged.ln_wRoot & ~MSK(cnBitsVirtAddr);
      #endif // BM_SW_BM_IN_WR_HB
        wRoot = *pwRoot; // Make everything right for qya.
        // copy bitmap
        memcpy(PWR_pwBm(pwRoot, pwr, nBW), pwBmStaged,
               N_WORDS_SW_BM(nBW) * sizeof(Word_t));
        // copy Switch_t
        *(Switch_t*)pwr = *(Switch_t*)pwrStaged;
        int nLinkCnt = nLinkGrpCnt << cnLogBmSwLinksPerBit;
        // copy links
        memcpy(pwr_pLinks((Switch_t*)pwr), pwr_pLinks((Switch_t*)pwrStaged),
               nLinkCnt * sizeof(Link_t));
      #ifdef REMOTE_LNX
        // copy remote link extensions
        BJL(memcpy(gpwLnX(qy, nLinkCnt, /*nDigit*/ 0), pwLnXStaged,
                   nLinkCnt * sizeof(Word_t)));
      #endif // REMOTE_LNX
        OldSwitch(&LnStaged.ln_wRoot, nBL, /* bBmSw */ 1, /* nLinks */ 1<<nBW);
    }
  #endif // BM_SW_FOR_REAL

    DBGI(printf("\n# Just after splay "));
    DBGI(Dump(pwRootLast, wKey, cnBitsPerWord));

    // Caller is going to insert wKey when we return.
}

#ifdef SPLAY_WITH_INSERT
#ifdef B_JUDYL
static Word_t*
#else // B_JUDYL
static Status_t
#endif // #else B_JUDYL
SplayWithInsert(qpa, Word_t *pwRootOld, int nBLOld, Word_t wKey, int nPos)
{
    qva;
    (void)wKey;
  #ifdef DEBUG
    int nPopCntMax = 0;
  #endif // DEBUG
    Word_t wRootOld = *pwRootOld;
    DBGI(printf("\n# SplayWithInsert nBLOld %d pwRootOld %p"
                    " nBL %d pLn %p pwRoot %p",
                nBLOld, pwRootOld, nBL, pLn, pwRoot));
  #ifdef _LNX
    DBGI(printf(" pwLnX %p", pwLnX));
  #endif // _LNX
    DBGI(printf("\n"));
    // We don't have/know pwLnXOld for nBLOld < cnBitsPerWord if REMOTE_LNX.
  #ifndef REMOTE_LNX
    DBGI(DumpX(qyax(Old), wKey));
  #endif // !REMOTE_LNX
    DBGI(DumpX(qya, wKey));
    int nBLROld = gnListBLR(qyx(Old));
    BJL(Word_t *pwValue = NULL);
    int bInsertNotDone = 0;

#ifdef KISS
    // Handle an empty link for KISS to possibly simplify callers.
    // NO_TYPE_IN_XX_SW really means "no type in XX_SW links with nBL < DL2".
  #ifdef NO_TYPE_IN_XX_SW
    if (nBLOld < nDL_to_nBL(2)) {
        if (wRootOld == ZERO_POP_MAGIC) { return; }
      #ifdef HANDLE_BLOWOUTS
        if ((wRootOld & BLOWOUT_MASK(nBLOld)) == ZERO_POP_MAGIC) { return; }
      #endif // HANDLE_BLOWOUTS
    }
  #endif // NO_TYPE_IN_XX_SW
    if (wRootOld == WROOT_NULL) { return; }
#else // KISS
    // Assert that we have a non-empty link that may be a T_LIST[_UA] link
    // for non-KISS to streamline this function and make caller responsible
    // for not calling it with an empty link.
  #ifdef NO_TYPE_IN_XX_SW
    assert(nBLOld >= nDL_to_nBL(2));
  #endif // NO_TYPE_IN_XX_SW
    assert(wRootOld != WROOT_NULL);
#endif // KISS

    int nTypeOld = wr_nType(wRootOld);
    Word_t *pwrOld = wr_pwr(wRootOld);
    int nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLROld);
    BJL(Word_t *pwValuesOld = gpwValues(qyx(Old)));

    // We can't assume this switch has nothing in it.
    // There may be non-empty links in the switch that don't overlap the
    // expanse of the list we are splaying.

    assert(tp_bIsList(nTypeOld));

    // We have an orphaned list. And a switch has been inserted that has
    // empty links just waiting for the list to be splayed into it.
    // This may not be the first list being inserted into the switch.
    // But it will be the first that will splay into the links that it
    // will splay into.

    int nBLR = GetBLR(pwRoot, nBL);
  #ifndef USE_XX_SW
    assert(nBLR <= nBLROld);
  #endif // #ifndef USE_XX_SW
    if (!tp_bIsSwitch(nType)
  #ifdef BITMAP
        && ((nBL != cnBitsInD1) || (nType != T_BITMAP))
  #endif // BITMAP
        )
    {
        printf("\nnBL %d nBLR %d nPopCnt %d nType %d\n",
               nBL, nBLR, nPopCnt, nType);
    }
    assert(tp_bIsSwitch(nType));

    int nBW = gnBW(qy, nBLR); // nBLR_to_nBW(nBLR);
  #ifdef CODE_BM_SW
    int bIsBmSw = tp_bIsBmSw(nType);
      #ifdef BM_SW_FOR_REAL
    // Save the old switch in case it ends up being suitable.
    Link_t linkOrig = *pLn;
      #endif // BM_SW_FOR_REAL
    if (bIsBmSw) {
        // We can't handle splay into a non-empty BmSw.
        // We pay no attention to the contents of the original switch.
        assert(GetPopCnt(qya) == 0);
        DBGI(printf("SplayWithInsert bitmap switch"
                        " PopCnt %zd nBL %d nBLR %d nBW %d nBLROld %d\n",
                    GetPopCnt(qya), nBL, nBLR, nBW, nBLROld));
      #ifdef BM_SW_FOR_REAL
        // Create a new switch for staging.
        pwr = NewSwitchX(qya, wKey, nBLR,
          #if defined(CODE_XX_SW)
                         nBW,
          #endif // defined(CODE_XX_SW)
                         T_BM_SW,
                         /*nLinkGrpCnt*/ EXP(nBW - cnLogBmSwLinksPerBit), 0);
      #else // BM_SW_FOR_REAL
        pwr = wr_pwr(*pwRoot);
      #endif // #else BM_SW_FOR_REAL
        wRoot = *pwRoot;
        assert(wr_nType(wRoot) == nType);
        assert(wr_pwr(wRoot) == pwr);
        assert(GetBLR(pwRoot, nBL) == nBLR);
    }
  #endif // CODE_BM_SW

    Link_t *pLinks = pwr_pLinks((Switch_t *)pwr);
    int nBLLoop = nBLR - nBW; // nBL of the links in the switch
    assert(nBLLoop <= nBLROld);
#ifdef EMBED_KEYS
    int nEmbeddedListPopCntMax = EmbeddedListPopCntMax(nBLLoop);
#endif // EMBED_KEYS
    int nDigitKey = (wKey >> nBLLoop) & MSK(nBW); (void)nDigitKey;
    Word_t wDigitMask = ~MSK(nBLLoop) & NZ_MSK(nBLR);

    BJL(Word_t *pwValues = gpwValues(qyx(Old)));
    int nnStart = 0; (void)nnStart;
  #ifdef BM_SW_FOR_REAL
    int nLinkGrpCnt = 0;
  #endif // BM_SW_FOR_REAL
    int nIndex;
  #if cnSwCnts != 0
    Word_t* pwCnts = ((Switch_t*)pwr)->sw_awCnts;
  #endif // cnSwCnts != 0
#if defined(COMPRESSED_LISTS)
    if (nBLROld <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(8) & NZ_MSK(nBLR);
        int nDigit = ((pcKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        int nDigit0 = nDigit;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((pcKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit8:;
                if (((nDigitKey < nDigitNew) && (nDigitKey > nDigit))
                    || ((nDigit == nDigit0) && (nDigitKey < nDigit)))
                {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
#if 1
                int nShift = (nBW > cnLogSwCnts + 2)
                               ? (nBW - cnLogSwCnts - 2) : 0;
                ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                        [nDigit >> (nBW - cnLogSwCnts - 2)]
                    += nPopCntLoop;
#endif
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, pcKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop + (nDigitKey == nDigit)
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        assert(nBLLoop <= 8);
                        uint8_t* puc = (uint8_t*)&wLnXLoop;
                        COPY(puc, &pcKeys[nnStart], nPopCntLoop);
                        PAD64(puc, nPopCntLoop);
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), pcKeys[xx]);
                        }
                    }
                    if (nDigit == nDigitKey) {
                        InsertEmbedded(qyax(Loop), wKey);
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST; (void)nTypeLoop;
                    if ((nDigit == nDigitKey)
                        && (nPopCntLoop < auListPopCntMax[nBLLoop]))
                    {
                        ++nPopCntLoop;
                    }
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    if (nPopCntLoop == (nn - nnStart)) {
                        // copy the values
                        Word_t *pwValuesLoop = gpwValues(qyx(Loop));
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    if (nDigit == nDigitKey) {
                        nPos -= nnStart;
                        bInsertNotDone = 1;
                    }
                    // copy the keys
                    uint8_t *pcKeysLoop
                        = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                    if ((nDigit == nDigitKey)
                        && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                    {
                        COPY_WITH_INSERT(pcKeysLoop, &pcKeys[nnStart],
                                         nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                       , gpwValues(qyx(Loop)),
                                         &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                         );
                        bInsertNotDone = 0;
                    } else {
                        COPY(pcKeysLoop, &pcKeys[nnStart], nPopCntLoop);
                        PAD(pcKeysLoop, nPopCntLoop);
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, pcKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) InsertAtBitmap(qyax(Loop), wKey);
                    }
                } else
  #endif // BITMAP
                {
                    // Resorting to this is perilous.
                    // Insert could change *pLn/*pwRoot.
                    // Could it even change something higher up in the tree
                    // so pLn and pwRoot are not valid?
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, pcKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) Insert(qya, wKey);
                        assert(!bInsertNotDone);
                    }
                    swPopCnt(qya, nBLR, 0);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                if (nDigitKey > nDigit) {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit8;
            }
        }
    } else if (nBLROld <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(16) & NZ_MSK(nBLR);
        int nDigit = ((psKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        int nDigit0 = nDigit;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((psKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit16:;
                if (((nDigitKey < nDigitNew) && (nDigitKey > nDigit))
                    || ((nDigit == nDigit0) && (nDigitKey < nDigit)))
                {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
#if 1
                int nShift = (nBW > cnLogSwCnts + 2)
                               ? (nBW - cnLogSwCnts - 2) : 0;
                ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                ((uint16_t*)((Switch_t*)pwr)->sw_awCnts)
                        [nDigit >> (nBW - cnLogSwCnts - 2)]
                    += nPopCntLoop;
#endif
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, psKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop + (nDigitKey == nDigit)
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        assert(nBLLoop <= 16);
                        if (nBLLoop <= 8) {
                            uint8_t* puc = (uint8_t*)&wLnXLoop;
                            COPY(puc, &psKeys[nnStart], nPopCntLoop);
                            PAD64(puc, nPopCntLoop);
                        } else {
                            uint16_t* pus = (uint16_t*)&wLnXLoop;
                            COPY(pus, &psKeys[nnStart], nPopCntLoop);
                            PAD64(pus, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), psKeys[xx]);
                        }
                    }
                    if (nDigit == nDigitKey) {
                        InsertEmbedded(qyax(Loop), wKey);
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST;
                    if ((nDigit == nDigitKey)
                        && (nPopCntLoop < auListPopCntMax[nBLLoop]))
                    {
                        ++nPopCntLoop;
                    }
  #ifdef UA_PARALLEL_128
                    if ((nBLLoop == 16) && (nPopCntLoop <= 6)) {
                        nTypeLoop = T_LIST_UA;
                    }
  #endif // UA_PARALLEL_128
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    if (nPopCntLoop == (nn - nnStart)) {
                        // copy the values
                        Word_t *pwValuesLoop = gpwValues(qyx(Loop));
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    if (nDigit == nDigitKey) {
                        nPos -= nnStart;
                        bInsertNotDone = 1;
                    }
                    // copy the keys
                    if (nBLLoop <= 8) {
                        uint8_t *pcKeysLoop
                            = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(pcKeysLoop, &psKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(pcKeysLoop, &psKeys[nnStart], nPopCntLoop);
                            PAD(pcKeysLoop, nPopCntLoop);
                        }
                    } else {
                        uint16_t *psKeysLoop
                            = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(psKeysLoop, &psKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(psKeysLoop, &psKeys[nnStart], nPopCntLoop);
                            PAD(psKeysLoop, nPopCntLoop);
                        }
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, psKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) InsertAtBitmap(qyax(Loop), wKey);
                    }
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, psKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) Insert(qya, wKey);
                        assert(!bInsertNotDone);
                    }
                    swPopCnt(qya, nBLR, 0);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                if (nDigitKey > nDigit) {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit16;
            }
        }
#if (cnBitsPerWord > 32)
    } else if (nBLROld <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
        Word_t wBitsFromKey = wKey & ~MSK(32) & NZ_MSK(nBLR);
        int nDigit = ((piKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        int nDigit0 = nDigit;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((piKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit32:;
                if (((nDigitKey < nDigitNew) && (nDigitKey > nDigit))
                    || ((nDigit == nDigit0) && (nDigitKey < nDigit)))
                {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("nDigit 0x%02x nnStart %d nn %d\n",
                             nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
                if (nBLR <= 16) {
#if 1
                    int nShift = (nBW > cnLogSwCnts + 2)
                                   ? (nBW - cnLogSwCnts - 2) : 0;
                    ((uint16_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                        += nPopCntLoop;
#endif
                } else if (nBLR <= 32) {
#if 1
                    int nShift = (nBW > cnLogSwCnts + 1)
                                   ? (nBW - cnLogSwCnts - 1) : 0;
                    ((uint32_t*)pwCnts)[nDigit >> nShift] += nPopCntLoop;
#else
                    ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                        += nPopCntLoop;
#endif
                } else {
      #if cnSwCnts == 1
                    if (!(nDigit >> (nBW - 1)))
      #endif // cnSwCnts == 1
#if 1
                    {
                        int nShift = (nBW > cnLogSwCnts)
                                       ? (nBW - cnLogSwCnts) : 0;
                        pwCnts[nDigit >> nShift] += nPopCntLoop;
                    }
#else
                    { pwCnts[nDigit >> (nBW - cnLogSwCnts)] += nPopCntLoop; }
#endif
                }
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, piKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop + (nDigitKey == nDigit)
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        if (nBLLoop <= 8) {
                            uint8_t* puc = (uint8_t*)&wLnXLoop;
                            COPY(puc, &piKeys[nnStart], nPopCntLoop);
                            PAD64(puc, nPopCntLoop);
                        } else
          #if (cnBitsPerWord > 32)
                        if (nBLLoop > 16) {
                            uint32_t* pui = (uint32_t*)&wLnXLoop;
                            COPY(pui, &piKeys[nnStart], nPopCntLoop);
                            PAD64(pui, nPopCntLoop);
                        } else
          #endif // (cnBitsPerWord > 32)
                        {
                            uint16_t* pus = (uint16_t*)&wLnXLoop;
                            COPY(pus, &piKeys[nnStart], nPopCntLoop);
                            PAD64(pus, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), piKeys[xx]);
                        }
                    }
                    if (nDigit == nDigitKey) {
                        InsertEmbedded(qyax(Loop), wKey);
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigitKey == nDigit) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST; (void)nTypeLoop;
                    if ((nDigit == nDigitKey)
                        && (nPopCntLoop < auListPopCntMax[nBLLoop]))
                    {
                        ++nPopCntLoop;
                    }
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    if (nPopCntLoop == (nn - nnStart)) {
                        // copy the values
                        Word_t *pwValuesLoop = gpwValues(qyx(Loop));
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    if (nDigit == nDigitKey) {
                        nPos -= nnStart;
                        bInsertNotDone = 1;
                    }
                    // copy the keys
                    if (nBLLoop <= 8) {
                        uint8_t *pcKeysLoop
                            = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(pcKeysLoop, &piKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(pcKeysLoop, &piKeys[nnStart], nPopCntLoop);
                            PAD(pcKeysLoop, nPopCntLoop);
                        }
                    } else if (nBLLoop <= 16) {
                        uint16_t *psKeysLoop
                            = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(psKeysLoop, &piKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(psKeysLoop, &piKeys[nnStart], nPopCntLoop);
                            PAD(psKeysLoop, nPopCntLoop);
                        }
                        if (nBLLoop == 16) {
                            UpdateDist(qyax(Loop), nPopCntLoop);
                        }
                    } else {
                        uint32_t *piKeysLoop
                            = ls_piKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(piKeysLoop, &piKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(piKeysLoop, &piKeys[nnStart], nPopCntLoop);
                            PAD(piKeysLoop, nPopCntLoop);
                        }
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, piKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) InsertAtBitmap(qyax(Loop), wKey);
                    }
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, piKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) Insert(qya, wKey);
                        assert(!bInsertNotDone);
                    }
                    swPopCnt(qya, nBLR, 0);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                if (nDigitKey > nDigit) {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
                ++nn;
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit32;
            }
        }
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwrOld, nBLROld, nPopCnt);
        Word_t wBitsFromKey = 0;
        int nDigit = ((pwKeys[0] & wDigitMask) | wBitsFromKey) >> nBLLoop;
        int nDigit0 = nDigit;
        for (int nn = 0; nn < nPopCnt; nn++) {
            int nDigitNew
                = ((pwKeys[nn] & wDigitMask) | wBitsFromKey) >> nBLLoop;
            if (nDigitNew != nDigit) {
lastDigit:;
                if (((nDigitKey < nDigitNew) && (nDigitKey > nDigit))
                    || ((nDigit == nDigit0) && (nDigitKey < nDigit)))
                {
                        nPos = 0;
                        bInsertNotDone = 1;
                }
  #ifdef DEBUG
                if (nn - nnStart + (nDigitKey == nDigit) > nPopCntMax) {
                    nPopCntMax = nn - nnStart + (nDigitKey == nDigit);
                }
  #endif // DEBUG
  #ifdef BM_SW_FOR_REAL
                if (bIsBmSw) {
                    nIndex = (nLinkGrpCnt << cnLogBmSwLinksPerBit)
                           + (nDigit & NBPW_MSK(cnLogBmSwLinksPerBit));
                    if ((nDigitNew ^ nDigit) >> cnLogBmSwLinksPerBit) {
                        SetBitInBmSwBm(qya, nDigit, nBW);
                        ++nLinkGrpCnt;
                    }
                } else
  #endif // BM_SW_FOR_REAL
                { nIndex = nDigit; }
                Link_t *pLnLoop = &pLinks[nIndex];
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
                Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nIndex);
                (void)pwLnXLoop;
  #endif // _LNX
                DBGI(printf("pLnLoop %p nDigit 0x%02x nnStart %d nn %d\n",
                             pLnLoop, nDigit, nnStart, nn));
                assert(pLnLoop->ln_wRoot == WROOT_NULL);
                int nPopCntLoop = nn - nnStart; (void)nPopCntLoop;
  #if cnSwCnts != 0
                if (nBLR <= 16) {
                    ((uint16_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 2)]
                        += nPopCntLoop;
                } else if (nBLR <= 32) {
                    ((uint32_t*)pwCnts)[nDigit >> (nBW - cnLogSwCnts - 1)]
                        += nPopCntLoop;
                } else {
      #if cnSwCnts == 1
                    if (!(nDigit >> (nBW - 1)))
      #endif // cnSwCnts == 1
                    { pwCnts[nDigit >> (nBW - cnLogSwCnts)] += nPopCntLoop; }
                }
  #endif // cnSwCnts != 0
  #ifdef BITMAP
      #ifndef B_JUDYL
                if (cbEmbeddedBitmap && (nBLLoop <= cnLogBitsPerLink)) {
                    // InsertAllAtEmbeddedBitmap
                    bzero(pLnLoop, sizeof(*pLnLoop));
                    for (int xx = nnStart; xx < nn; ++xx) {
                        SetBit(pLnLoop, pwKeys[xx] & MSK(nBLLoop));
                    }
                } else
      #endif // #ifndef B_JUDYL
  #endif // BITMAP
  #ifdef EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop + (nDigitKey == nDigit)
                        <= nEmbeddedListPopCntMax)
                {
      #ifdef EK_XV
                    if (nPopCntLoop > 1) {
                        Word_t wRootLoop = 0;
                        set_wr_nType(wRootLoop, T_EK_XV);
                        set_wr_nPopCnt(wRootLoop, nBLLoop, nPopCntLoop);
                        // Copy the keys.
                        Word_t wLnXLoop;
                        if (nBLLoop <= 8) {
                            uint8_t* puc = (uint8_t*)&wLnXLoop;
                            COPY(puc, &pwKeys[nnStart], nPopCntLoop);
                            PAD64(puc, nPopCntLoop);
                        } else
          #if (cnBitsPerWord > 32)
                        if (nBLLoop > 16) {
                            uint32_t* pui = (uint32_t*)&wLnXLoop;
                            COPY(pui, &pwKeys[nnStart], nPopCntLoop);
                            PAD64(pui, nPopCntLoop);
                        } else
          #endif // (cnBitsPerWord > 32)
                        {
                            uint16_t* pus = (uint16_t*)&wLnXLoop;
                            COPY(pus, &pwKeys[nnStart], nPopCntLoop);
                            PAD64(pus, nPopCntLoop);
                        }
                        // Create the value area and copy the values.
                        Word_t* pwrLoop
                            = (Word_t*)MyMallocGuts(nPopCntLoop | 1,
          #ifdef ALIGN_EK_XV
                                                    (nPopCntLoop <= 3) ? 5 : 6,
          #else // ALIGN_EK_XV
                                                    cnBitsMallocMask,
          #endif // #else ALIGN_EK_XV
                                                    &j__AllocWordsJV);
                        set_wr_pwr(wRootLoop, pwrLoop);
                        for (int nnLoop = 0; nnLoop < nPopCntLoop; nnLoop++) {
                            pwrLoop[nnLoop]
                                = pwValuesOld[~(nnStart + nnLoop)];
                        }
                        pLnLoop->ln_wRoot = wRootLoop; // install the new list
                        *gpwEmbeddedValue(qyax(Loop)) = wLnXLoop;
                        DBGI(printf("Splay wRootLoop 0x%zx wLnXLoop 0x%zx\n",
                                    wRootLoop, wLnXLoop));
                    } else
      #endif // EK_XV
                    {
      #ifdef B_JUDYL
                        assert(nPopCntLoop == 1);
                        *gpwEmbeddedValue(qyax(Loop)) = pwValuesOld[~nnStart];
      #endif // B_JUDYL
                        for (int xx = nnStart; xx < nn; ++xx) {
                            InsertEmbedded(qyax(Loop), pwKeys[xx]);
                        }
                    }
                    if (nDigit == nDigitKey) {
                        InsertEmbedded(qyax(Loop), wKey);
                    }
                } else
  #endif // EMBED_KEYS
                // Insert of wKey is soon to follow. Always?
                if (nPopCntLoop /* + (nDigit == nDigitKey) */
                        <= auListPopCntMax[nBLLoop])
                {
                    // We have a sub list.
                    // Let's make a new list and copy it over.
                    Word_t wRootLoop = 0;
                    int nTypeLoop = T_LIST;
                    if ((nDigit == nDigitKey)
                        && (nPopCntLoop < auListPopCntMax[nBLLoop]))
                    {
// We're bumping nPopCntLoop here.
                        ++nPopCntLoop;
                    }
// How do we know if we bumped it? nn - nnStart?
  #ifdef UA_PARALLEL_128
                    if ((nBLLoop == 16) && (nPopCntLoop <= 6)) {
                        nTypeLoop = T_LIST_UA;
                    }
  #endif // UA_PARALLEL_128
                    Word_t *pwrLoop = NewList(nPopCntLoop, nBLLoop);
                    set_wr(wRootLoop, pwrLoop, nTypeLoop);
                    pLnLoop->ln_wRoot = wRootLoop; // install
                    snListBLR(qyx(Loop), nBLLoop);
                    // Does this work for list pop cnt not in wRoot?
                    Set_xListPopCnt(&pLnLoop->ln_wRoot, nBLLoop, nPopCntLoop);
                    wRootLoop = pLnLoop->ln_wRoot;
      #ifdef B_JUDYL
                    if (nPopCntLoop == (nn - nnStart)) {
                        // copy the values
                        Word_t *pwValuesLoop = gpwValues(qyx(Loop));
                        COPY(&pwValuesLoop[-nPopCntLoop],
                             &pwValuesOld[-nPopCntLoop - nnStart],
                             nPopCntLoop);
                    }
      #endif // B_JUDYL
                    if (nDigit == nDigitKey) {
                        nPos -= nnStart;
                        bInsertNotDone = 1;
                    }
                    // copy the keys
      #ifdef COMPRESSED_LISTS
                    if (nBLLoop <= 16) {
                        if (nBLLoop <= 8) {
                            uint8_t *pcKeysLoop
                                = ls_pcKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                            if ((nDigit == nDigitKey)
                                && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                            {
                                COPY_WITH_INSERT(pcKeysLoop, &pwKeys[nnStart],
                                                 nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                               , gpwValues(qyx(Loop)),
                                                 &pwValuesOld[-nnStart],
                                                 &pwValue
       #endif // B_JUDYL
                                                 );
                                bInsertNotDone = 0;
                            } else {
                                COPY(pcKeysLoop, &pwKeys[nnStart],
                                     nPopCntLoop);
                                PAD(pcKeysLoop, nPopCntLoop);
                            }
                        } else {
                            uint16_t *psKeysLoop
                                = ls_psKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                            if ((nDigit == nDigitKey)
                                && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                            {
                                COPY_WITH_INSERT(psKeysLoop, &pwKeys[nnStart],
                                                 nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                               , gpwValues(qyx(Loop)),
                                                 &pwValuesOld[-nnStart],
                                                 &pwValue
       #endif // B_JUDYL
                                                 );
                                bInsertNotDone = 0;
                            } else {
                                COPY(psKeysLoop, &pwKeys[nnStart],
                                     nPopCntLoop);
                                PAD(psKeysLoop, nPopCntLoop);
                                if (nBLLoop == 16) {
                                    UpdateDist(qyax(Loop), nPopCntLoop);
                                }
                            }
                        }
          #if (cnBitsPerWord > 32)
                    } else if (nBLLoop <= 32) {
                        uint32_t *piKeysLoop
                            = ls_piKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(piKeysLoop, &pwKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(piKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                            PAD(piKeysLoop, nPopCntLoop);
                        }
          #endif // (cnBitsPerWord > 32)
                    } else
      #endif // COMPRESSED_LISTS
                    {
                        Word_t *pwKeysLoop
                            = ls_pwKeysX(pwrLoop, nBLLoop, nPopCntLoop);
                        if ((nDigit == nDigitKey)
                            && ((nn - nnStart) < auListPopCntMax[nBLLoop]))
                        {
                            COPY_WITH_INSERT(pwKeysLoop, &pwKeys[nnStart],
                                             nPopCntLoop - 1, wKey, nPos
       #ifdef B_JUDYL
                                           , gpwValues(qyx(Loop)),
                                             &pwValuesOld[-nnStart], &pwValue
       #endif // B_JUDYL
                                             );
                            bInsertNotDone = 0;
                        } else {
                            COPY(pwKeysLoop, &pwKeys[nnStart], nPopCntLoop);
                            PAD(pwKeysLoop, nPopCntLoop);
                        }
                    }
                } else
  #ifdef BITMAP
                if (nBLLoop == cnBitsInD1) {
                    NewBitmap(qyax(Loop), cnBitsInD1, pwKeys[nnStart],
                              nPopCntLoop);
                    InsertAllAtBitmap(qyax(Loop), qyx(Old),
                                      nnStart, nPopCntLoop);
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) InsertAtBitmap(qyax(Loop), wKey);
                    }
                } else
  #endif // BITMAP
                {
                    assert(nPopCntLoop > auListPopCntMax[nBLLoop]);
                    for (int xx = nnStart; xx < nn; ++xx) {
                        Insert(qya, pwKeys[xx])BJL([0] = pwValues[~xx]);
                    }
                    if (nDigit == nDigitKey) {
                        BJL(pwValue =) Insert(qya, wKey);
                        assert(!bInsertNotDone);
                    }
                    swPopCnt(qya, nBLR, 0);
                }
                nnStart = nn;
                nDigit = nDigitNew;
            }
            if (nn == nPopCnt - 1) {
                if (nDigitKey > nDigit) {
                    nPos = 0;
                    bInsertNotDone = 1;
                }
                ++nn;
                //printf("pLn %p\n", pLn);
                //printf("pLn->ln_wRoot 0x%zx\n", pLn->ln_wRoot);
                //printf("pLinks %p\n", pLinks);
                nDigitNew = nDigit + NBPW_EXP(cnLogBmSwLinksPerBit);
                goto lastDigit;
            }
        }
    }

  #ifdef DEBUG_INSERT
    int nSplayMaxPopCnt = SplayMaxPopCnt(pwRootOld, nBLOld, wKey, nBLLoop);
    if ((nPopCntMax != nSplayMaxPopCnt)
        && (nPopCntMax != nSplayMaxPopCnt + 1))
    {
        DBGI(printf("# nPopCntMax %d\n", nPopCntMax));
        DBGI(printf("# nSplayMaxPopCnt %d\n", nSplayMaxPopCnt));
        DBGI((printf("# nPopCntMax "), Dump(pwRootLast, 0, cnBitsPerWord)));
        DBGI((printf("# SplayMaxPopCnt "), Dump(pwRootOld, wKey, nBLOld)));
    }
    assert((nPopCntMax == nSplayMaxPopCnt)
        || (nPopCntMax == nSplayMaxPopCnt + 1));
  #endif // DEBUG_INSERT

    OldList(pwrOld, nPopCnt, nBLROld, nTypeOld);

    // Fix up pop count in the new switch.
    Word_t wPopCnt = 0;
    {
  #ifdef XX_LISTS
        if (wr_nType(*pwRootOld) != T_XX_LIST)
  #endif // XX_LISTS
        {
            wPopCnt = gwPopCnt(qya, nBLR) + nPopCnt /* + 1 */;
            swPopCnt(qya, nBLR, wPopCnt);
        }
    }

  #ifdef BM_SW_FOR_REAL
    if (bIsBmSw) {
        Link_t LnStaged = *pLn;
        Word_t* pwrStaged = pwr;
        Word_t* pwBmStaged = PWR_pwBm(&LnStaged.ln_wRoot, pwr, nBW);
      #ifdef REMOTE_LNX
        BJL(Word_t* pwLnXStaged = gpwLnX(qy, 1<<nBW, /*nDigit*/ 0));
      #endif // REMOTE_LNX
        if (nLinkGrpCnt == 1) {
            // Save the work of OldSwitch/NewSwitch if the switch that
            // came in is suitable.
            // Restore original pwr and link count.
            // Do not overwrite *pwLnX in case SW_POP_IN_LNX && NO_REMOTE_LNX.
            pLn->ln_wRoot = linkOrig.ln_wRoot; // Both? Why?
      #ifdef BM_SW_BM_IN_WR_HB
            pLn->ln_wRoot &= MSK(cnBitsVirtAddr);
      #endif // BM_SW_BM_IN_WR_HB
            pwr = wr_pwr(linkOrig.ln_wRoot);
            // Pop count will be updated when we copy the switch.
        } else {
      #ifdef _LNX
            Word_t wLnX = nBL < cnBitsPerWord ? *pwLnX : 0;
      #endif // _LNX
            OldSwitch(&linkOrig.ln_wRoot, nBL, /*bBmSw*/ 1,
                      /*nLinks*/ 1 << cnLogBmSwLinksPerBit);
            pwr = NewSwitchX(qya, wKey, nBLR,
      #if defined(CODE_XX_SW)
                             nBW,
      #endif // defined(CODE_XX_SW)
                             T_BM_SW, nLinkGrpCnt, wPopCnt);
      #ifdef _LNX
            // NewSwitch zero'd *pwLnX.
            if (nBL < cnBitsPerWord) {
                *pwLnX = wLnX;
            }
      #endif // _LNX
        }
      #ifdef BM_SW_BM_IN_WR_HB
        *pwRoot |= LnStaged.ln_wRoot & ~MSK(cnBitsVirtAddr);
      #endif // BM_SW_BM_IN_WR_HB
        wRoot = *pwRoot; // Make everything right for qya.
        // copy bitmap
        memcpy(PWR_pwBm(pwRoot, pwr, nBW), pwBmStaged,
               N_WORDS_SW_BM(nBW) * sizeof(Word_t));
        // copy Switch_t
        *(Switch_t*)pwr = *(Switch_t*)pwrStaged;
        int nLinkCnt = nLinkGrpCnt << cnLogBmSwLinksPerBit;
        // copy links
        memcpy(pwr_pLinks((Switch_t*)pwr), pwr_pLinks((Switch_t*)pwrStaged),
               nLinkCnt * sizeof(Link_t));
      #ifdef REMOTE_LNX
        // copy remote link extensions
        BJL(memcpy(gpwLnX(qy, nLinkCnt, /*nDigit*/ 0), pwLnXStaged,
                   nLinkCnt * sizeof(Word_t)));
      #endif // REMOTE_LNX
        OldSwitch(&LnStaged.ln_wRoot, nBL, /* bBmSw */ 1, /* nLinks */ 1<<nBW);
    }
  #endif // BM_SW_FOR_REAL

    BJ1(int status = Success);
    if (bInsertNotDone) {
        DBGI(printf("# About to do final insert.\n"));
        Link_t *pLnLoop = &pLinks[nDigitKey];
        Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
  #ifdef _LNX
        Word_t* pwLnXLoop = gpwLnX(qy, 1<<nBW, nDigitKey);
        (void)pwLnXLoop;
  #endif // _LNX
        DBGI(printf("\n# Just before InsertAtList in SplayWithInsert "));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        BJL(pwValue =) BJ1(status =)
            InsertAtList(qyax(Loop), wKey, nPos
  #if defined(CODE_XX_SW)
                       , /*pLnUp*/ tp_bIsXxSw(nType) ? pLn : NULL
                       , /*pBLUp*/ nBL
      #ifdef REMOTE_LNX
                       , pwLnX
      #endif // REMOTE_LNX
  #endif // defined(CODE_XX_SW)
                         );
    } else {
        DBGI(printf("# Final insert is not needed; it is already done.\n"));
    }

  #ifdef B_JUDYL
    DBGI(printf("\n# Just before returning %p from SplayWithInsert ",
                pwValue));
  #else // B_JUDYL
    DBGI(printf("\n# Just before returning %d from SplayWithInsert ",
                status));
  #endif // #else B_JUDYL
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    return BJL(pwValue) BJ1(status);
}
#endif // SPLAY_WITH_INSERT

// Insert each key from pwRootOld into pwRoot.  Then free pwRootOld.
// wKey contains the common prefix.
static void
InsertAll(qpa, Word_t *pwRootOld, int nBLOld, Word_t wKey)
{
    qva;
    Word_t wRootOld = *pwRootOld;
  #ifdef NO_TYPE_IN_XX_SW
    if (nBLOld < nDL_to_nBL(2)) {
        if (wRootOld == ZERO_POP_MAGIC) { return; }
      #if defined(HANDLE_BLOWOUTS)
        if ((wRootOld & BLOWOUT_MASK(nBLOld)) == ZERO_POP_MAGIC) { return; }
      #endif // defined(HANDLE_BLOWOUTS)
  #else // NO_TYPE_IN_XX_SW
    if (wRootOld == WROOT_NULL) { return; }
  #endif // #else NO_TYPE_IN_XX_SW
        DBGI(printf("InsertAll(nBLOld %d pwRootOld %p wKey " OWx
                    " nBL %d pLn %p pwRoot %p",
                    nBLOld, pwRootOld, wKey, nBL, pLn, pwRoot));
  #ifdef _LNX
        DBGI(printf(" pwLnX %p", pwLnX));
  #endif // _LNX
        DBGI(printf("\n"));
  #ifdef NO_TYPE_IN_XX_SW
        // Is NO_TYPE relevant only at dl2 and below?
        goto embeddedKeys; // How do we know we have XX_SW here?
    }
  #endif // NO_TYPE_IN_XX_SW

    int nTypeOld = wr_nType(wRootOld); (void)nTypeOld;
  #if defined(EMBED_KEYS)
      #if defined(CODE_XX_SW)
    if (tp_bIsEk(nTypeOld)) {
        goto embeddedKeys;
embeddedKeys:;
        // How inefficient can we be?
        DBGI(printf("IA: Calling IEL nBLOld %d wKey " OWx" nBL %d\n",
                    nBLOld, wKey, nBL));
        Link_t *pLnOld = STRUCT_OF(pwRootOld, Link_t, ln_wRoot); (void)pLnOld;
        // wRootOld here, but new from IEL's perspective
          #ifdef REMOTE_LNX
        // pwRootOld is a link in an XX_SW. We don't know nBWOld.
        // This is a lot of work to get something we should be passing in
        // as pwValueOld or nBWOld or (nBLUpOld, pwRootUpOld) or qyx(UpOld).
        // Or maybe we should be inflating the list in the caller before
        // we ever get here.
        // Let's assume nBLRUpOld == nBLR. It works for DoubleDown.
        int nBLR = GetBLR(pwRoot, nBL); // nBLRUpOld == nBLR
        int nBWUp = nBLR - nBLOld; // nBWUpOld
        // Using nBLR to figure nBWUp may be ok for doubling down, but I
        // don't think it is true for a more general InsertAll.
        int nDigitUp = ((wKey >> nBLOld) & MSK(nBWUp)); // nDigitUpOld
        Link_t *pLinksUp = pLnOld - nDigitUp; // pLinksUpOld
        // pwrUpOld
        Word_t *pwrUp = (Word_t*)STRUCT_OF(pLinksUp, Switch_t, sw_aLinks);
        // Fabricate a fake *pLnUpOld for use by gpwLnX.
        Link_t linkUp = {0};
        Link_t *pLnUp = &linkUp;
        Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
        set_wr_pwr(linkUp.ln_wRoot, pwrUp); // gpwLnX needs pwr
        int nBLUp = nBLR; // nBLUpOld -- assumes DoubleDown
        Word_t *pwLnXOld = gpwLnX(qyx(Up), EXP(nBWUp), nDigitUp);
        (void)pwLnXOld;
          #endif // REMOTE_LNX
        wRootOld = InflateEmbeddedList(qyax(Old), wKey);
        // If (nBLOld < nDL_to_nBL) Dump is going to think wRootOld is
        // embeddded keys.
        //DBGI(printf("After IEL\n"));
        //DBGI(Dump(&wRootOld, wKey & ~MSK(nBLOld), nBLOld));
        nTypeOld = wr_nType(wRootOld); // changed by IEL
    }
      #else // defined(CODE_XX_SW)
    assert(nTypeOld != T_EMBEDDED_KEYS); // Why?
    // Because, for !XX_SW, InsertAll is called only from TransformList from
    // for InsertAtList or ListIsFull which are both after InsertGuts has
    // inflated the embedded list.
      #endif // #else defined(CODE_XX_SW)
  #endif // defined(EMBED_KEYS)

    Word_t *pwrOld = wr_pwr(wRootOld); (void)pwrOld;

#if 0
#ifdef DEBUG
#ifdef CODE_XX_SW
    if (nType == T_XX_SW) {
        Word_t *pwRootLoop = pwRootOld;
        Link_t *pLnLoop = STRUCT_OF(pwRootLoop, Link_t, ln_wRoot);
        Word_t wRootLoop = *pwRoot;
        int nTypeLoop = wr_nType(wRootLoop);
        Word_t *pwrLoop = wr_pwr(wRootLoop);
        int nBLLoop = nBL;
        int nBW = gnBW(qyx(Loop), nBL);
        DBG(printf("IA: T_XX_SW nBL %d nBW %d\n", nBL, nBW));
    }
    assert(nType != T_XX_SW);
#endif // defined(CODE_XX_SW)
    if (!tp_bIsList(nTypeOld)) {
        printf("\n# InsertAll nBLOld %d *pwRootOld 0x%zx wKey 0x%zx\n",
               nBLOld, *pwRootOld, wKey);
        exit(1);
    }
#endif // DEBUG
#endif
    assert(tp_bIsList(nTypeOld));
    int nBLROld = gnListBLR(qyx(Old));
  #ifdef DEBUG_INSERT
    {
        Word_t wRoot = *pwRoot; (void)wRoot;
        printf("\n");
        printf("IA: Switch PopCnt %zd nBL %d nBLR %d nBW %d nBLOld %d",
               GetPopCnt(qya), nBL, GetBLR(pwRoot, nBL),
               gnBW(qy, GetBLR(pwRoot, nBL)), nBLOld);
        printf("\n");
    }
  #endif // DEBUG_INSERT
      #ifdef USE_XX_SW_ONLY_AT_DL2
    if (nBLOld >= nDL_to_nBL(2)) // Splay can't handle it.
      #endif // USE_XX_SW_ONLY_AT_DL2
    {
        int nType = wr_nType(*pwRoot);
        if (tp_bIsSwitch(nType)) {
            // Splay can't handle widened BM_SW yet.
            // So we don't create them.
            assert(!tp_bIsBmSw(nType));
            // Splay updates the pop count for (nBL, pwRoot) but not
            // for the switch that contains it.
            Splay(qya, pwRootOld, nBLOld, wKey);
            return;
        }
    }
    // wRootOld might be newer than *pwRootOld
    int nPopCnt = PWR_xListPopCnt(&wRootOld, pwrOld, nBLOld);
    Link_t *pLnOld = STRUCT_OF(pwRootOld, Link_t, ln_wRoot); (void)pLnOld;
    BJL(Word_t *pwValues = gpwValues(qyx(Old)));
#if defined(COMPRESSED_LISTS)
    if (nBLROld <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwrOld, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Insert(qya, pcKeys[nn] | (wKey & ~MSK(8)))
                BJL([0] = pwValues[~nn]);
        }
    } else if (nBLROld <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwrOld, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Insert(qya, psKeys[nn] | (wKey & ~MSK(16)))
                BJL([0] = pwValues[~nn]);
        }
#if (cnBitsPerWord > 32)
    } else if (nBLROld <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwrOld, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Insert(qya, piKeys[nn] | (wKey & ~MSK(32)))
                BJL([0] = pwValues[~nn]);
        }
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwrOld, nBL, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Insert(qya, pwKeys[nn])BJL([0] = pwValues[~nn]);
        }
    }
#if defined(NO_TYPE_IN_XX_SW)
    // OldList uses nType even if (nBL < nDL_to_nBL(2)) implies an
    // embedded list.
#endif // defined(NO_TYPE_IN_XX_SW)
    if (nPopCnt != 0) {
// Should we have OldList look for T_XX_LIST and get nBLR?
        OldList(pwrOld, nPopCnt, nBLROld, nTypeOld);
    }
}

#endif // (cwListPopCntMax != 0)

#ifdef USE_LOWER_XX_SW
  #ifndef SKIP_TO_XX_SW
    #error USE_LOWER_XX_SW requires SKIP_TO_XX_SW
  #endif // SKIP_TO_XX_SW
// Widen a switch.
// Replace *pLnUp, which is the link to the switch containing qp, with a link
// to a new, wider switch, and transfer the keys and values from the old
// switch to the new switch, and insert wKey.
  #ifdef B_JUDYL
static Word_t*
  #else // B_JUDYL
static Status_t
  #endif // B_JUDYL
DoubleUp(qpa, // (nBL, pLn) of skip link to original switch
         Word_t wKey, // key being inserted
         Word_t wPopCnt) // pop count of original sub tree at qp?
{
    qva; (void)wPopCnt;
//fprintf(stderr, "\n# DoubleUp\n");
    int nBLR = gnBLR(qy);
    int nDLR = nBL_to_nDL(nBLR);
    int nBW = gnBW(qy, nBLR);
    Word_t wPrefix =
  #ifdef PP_IN_LINK
         (nBL == cnBitsPerWord) ? 0 :
  #endif // PP_IN_LINK
             PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
    DBGI(printf("# DoubleUp from nBLR %d nBW %d.\n", nBLR, nBW));
    assert(tp_bIsSkip(nType));
    assert(tp_bIsXxSw(wr_nType(wRoot)));
    assert(nBLR > nDL_to_nBL(2)); // temp
  #if defined(NO_TYPE_IN_XX_SW)
    assert(0);
  #endif // defined(NO_TYPE_IN_XX_SW)
    // We're creating a new switch with a bigger nBW at a bigger nBLR.
    int nBLRNew = nDL_to_nBL(nDLR);
    assert(nBLRNew > nBLR);
    int nBWNew = nDLR_to_nBW(nDLR);
    assert(nBWNew > nBW);
    DBGI(printf("# To nBLRNew %d nBWNew %d.\n", nBLRNew, nBWNew));
    DBGI(printf("\n# DoubleUp just before NewSwitch "));
    DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
    // wRoot serves as our saved link
    assert(sizeof(Link_t) == sizeof(wRoot));
    NewSwitch(qya, wKey, nBLRNew, nBWNew, T_SWITCH, wPopCnt);

    DBGI(printf("\n# DoubleUp just after NewSwitch old tree "));
    DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));

    // Now we need to move the keys from the old subtree to the new
    // subtree.

    // NewSwitch changed *pwRoot.
    // But wPrefix, nBL, wRoot, nType, pwr, nBLR and nBW still all apply
    // to the tree whose keys must be reinserted.

    // nBWNew is for the new tree.
    DBGI(printf("# Copying 1<<nBW links from old switch to new switch.\n"));

    Word_t* pwrNew = wr_pwr(*pwRoot);
  #if cnSwCnts != 0
    memcpy(((Switch_t*)pwrNew)->sw_awCnts, ((Switch_t*)pwr)->sw_awCnts,
           sizeof(((Switch_t*)pwr)->sw_awCnts));
  #endif // cnSwCnts != 0
    //printf("wPrefix & MSK(nBLRNew) 0x%zx\n", wPrefix & MSK(nBLRNew));
    for (int nIndex = 0; nIndex < (int)EXP(nBW); nIndex++) {
        DBGI(printf("# New tree before nIndex 0x%02x ", nIndex));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        int nIndexNew = ((wPrefix & MSK(nBLRNew)) >> (nBLR - nBW)) | nIndex;
        //printf("nIndexNew 0x%02x\n", nIndexNew);
  #ifdef EMBED_KEYS
        assert(wr_nType(pwr_pLinks((Switch_t*)pwr)[nIndex].ln_wRoot)
               != T_EMBEDDED_KEYS);
  #endif // EMBED_KEYS
        pwr_pLinks((Switch_t*)pwrNew)[nIndexNew]
            = pwr_pLinks((Switch_t*)pwr)[nIndex];
    }

    OldSwitch(&wRoot, nBL
#if defined(CODE_BM_SW)
            , /* bBmSw */ 0, /* nLinks */ 0
#endif // defined(CODE_BM_SW)
              );

    DBGI(printf("# DoubleUp just before final Insert "));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return 0;
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
}
#endif // USE_LOWER_XX_SW

#ifdef SKIP_LINKS
// Handle a prefix mismatch by inserting a switch above and demoting
// the current *pwRoot.
#ifdef B_JUDYL
static Word_t *
#else // B_JUDYL
static Status_t
#endif // B_JUDYL
InsertAtPrefixMismatch(qpa, Word_t wKey, int nBLR)
{
    qva;
  #if defined(NO_SKIP_AT_TOP)
    // no skip link at top => no prefix mismatch at top
    assert(nBL < cnBitsPerWord);
  #endif // defined(NO_SKIP_AT_TOP)
    // Can't have a prefix mismatch if there is no skip.
    assert(nBLR < nBL);
  #ifndef USE_LOWER_XX_SW
    assert(nDL_to_nBL(nBL_to_nDL(nBLR)) == nBLR);
  #endif // #ifndef USE_LOWER_XX_SW

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
    int nBLNew = LOG(wPrefix ^ wKey) + 1; // sig bits
    // nBLNew is the highest order bit that is different changed into an nBL.
    int nDLNew = nBL_to_nDL(nBLNew);
    // nDLNew includes the highest order digit that is different.

// We may want to consider creating an upper or lower xx sw here.
    // Choose (nBLNew, nBWNew).
    nBLNew = nDL_to_nBL(nDLNew);
    int nBWNew = nDLR_to_nBW(nDLNew); (void)nBWNew;
    DBGX(printf("InsertAtPrefixMismatch choosing nBLNew %d nBwNew %d.\n",
                nBLNew, nBWNew));
    assert(nBLNew > nBLR);
// Make this happen with:
// -DNO_USE_BM_SW  -DcnBitsPerDigit=4 -DNO_ALLOW_EMBEDDED_BITMAP
    assert(nBLNew <= nBL);

    Word_t wPopCnt = GetPopCnt(qya);

  #ifdef USE_LOWER_XX_SW
    if (nBLNew - nBWNew < nBLR) {
        return DoubleUp(qya, wKey, wPopCnt);
    }
  #endif // USE_LOWER_XX_SW

    // todo nBW; wide switch
    int nIndex = (wPrefix >> nDL_to_nBL_NAT(nDLNew - 1))
               & (EXP(nDLR_to_nBW(nDLNew)) - 1);
    // nIndex is the logical index in new switch.
    // It may not be the same as the index in the old switch.

#if defined(CODE_BM_SW)

    // set bBmSwNew
  #if defined(USE_BM_SW)
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK) || defined(NO_BM_SW_AT_TOP)
              #ifdef SMART_NEW_SWITCH
    int bBmSwNew = 1;
              #else // SMART_NEW_SWITCH
    int bBmSwNew = (nBL != cnBitsPerWord);
              #endif // SMART_NEW_SWITCH else
          #else // BM_IN_LINK || NO_BM_SW_AT_TOP
    int bBmSwNew = 1;
          #endif // BM_IN_LINK || NO_BM_SW_AT_TOP else
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK) || defined(NO_BM_SW_AT_TOP)
              #ifdef SMART_NEW_SWITCH
    int bBmSwNew = 1;
              #else // SMART_NEW_SWITCH
    // If we need a skip and !SKIP_TO_BM_SW then we can't use BM_SW.
    int bBmSwNew = ((nBL != cnBitsPerWord) && (nBLNew == nBL));
              #endif // SMART_NEW_SWITCH else
          #else // BM_IN_LINK || NO_BM_SW_AT_TOP
    int bBmSwNew = (nBLNew == nBL);
          #endif // BM_IN_LINK || NO_BM_SW_AT_TOP else
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
    Word_t wIndexCnt = EXP(nDL_to_nBWNAT(nBL_to_nDL(nBLR)));
    if (bBmSwOld)
    {
        // Save the old bitmap before it is trashed by NewSwitch.
        // Is it possible that nDL != cnDigitsPerWord and
        // we are at the top?
        if (nBL != cnBitsPerWord)
        {
            memcpy(ln.ln_awBm, PWR_pwBm(pwRoot, NULL, nBW),
                   sizeof(ln.ln_awBm));
      #if ! defined(BM_SW_FOR_REAL)
            assert((wIndexCnt < cnBitsPerWord)
                || (ln.ln_awBm[0] == (Word_t)-1));
      #endif // ! defined(BM_SW_FOR_REAL)
        }
    }
  #endif // defined(CODE_BM_SW)
#endif // defined(BM_IN_LINK)
  #ifdef _LNX
    Word_t wLnX = 0;
    if (pwLnX != NULL) {
        wLnX = *pwLnX; // Save *pwLnX which is overwritten by NewSwitch.
    }
  #endif // _LNX

    Word_t *pwSw;
    // initialize prefix/pop for new switch
    // Make sure to pass the right key for BM_SW_FOR_REAL.
    DBGI(printf("IAPM: nBLNew %d nBL %d\n", nBLNew, nBL));
    assert(nBLNew <= nBL);
    // NewSwitch changes *pwRoot (and the link containing it).
    // It does not change our local wRoot and pwr (or pwRoot).
    pwSw = NewSwitch(qya, wPrefix, nBLNew,
#if defined(CODE_XX_SW)
                     nBWNew,
#endif // defined(CODE_XX_SW)
#if defined(CODE_BM_SW)
                     bBmSwNew ? T_BM_SW :
#endif // defined(CODE_BM_SW)
                     T_SWITCH,
                     wPopCnt);
  #if cnSwCnts != 0
    Word_t* pwCnts = ((Switch_t*)pwSw)->sw_awCnts;
    if (nBLNew <= 16) {
#if 1
        int nShift = (nBWNew > nLogSwSubCnts(1))
                   ? (nBWNew - nLogSwSubCnts(1)) : 0;
        ((uint16_t*)pwCnts)[nIndex >> nShift] = wPopCnt;
#else
        ((uint16_t*)pwCnts)[nIndex >> (nBWNew - cnLogSwCnts - 2)] = wPopCnt;
#endif
    } else if (nBLNew <= 32) {
#if 1
        int nShift = (nBWNew > nLogSwSubCnts(2))
                   ? (nBWNew - nLogSwSubCnts(2)) : 0;
        ((uint32_t*)pwCnts)[nIndex >> nShift] = wPopCnt;
#else
        ((uint32_t*)pwCnts)[nIndex >> (nBWNew - cnLogSwCnts - 1)] = wPopCnt;
#endif
    } else {
      #if cnSwCnts == 1
        if (!(nIndex >> (nBWNew - 1)))
      #endif // cnSwCnts == 1
#if 1
        {
            int nShift = (nBWNew > cnLogSwCnts) ? (nBWNew - cnLogSwCnts) : 0;
            pwCnts[nIndex >> nShift] = wPopCnt;
        }
#else
        { pwCnts[nIndex >> (nBWNew - cnLogSwCnts)] = wPopCnt; }
#endif
    }
  #endif // cnSwCnts != 0

    int nLinksNew = EXP(nBWNew); (void)nLinksNew;
  #if defined(CODE_BM_SW)
    // NewSwitchX may ignore our request for T_BM_SW.
      #ifdef SMART_NEW_SWITCH
    if (tp_bIsBmSw(wr_nType(*pwRoot)))
      #else // SMART_NEW_SWITCH
    if (bBmSwNew)
      #endif // SMART_NEW_SWITCH else
    {
      #if defined(BM_SW_FOR_REAL)
          #if defined(BM_IN_LINK)
        if (nBL != cnBitsPerWord)
          #endif // defined(BM_IN_LINK)
        {
            // Switch was created with only one link based on wKey
            // passed in.  Unless BM_IN_LINK && switch is at top.
            nIndex = (wPrefix >> nDL_to_nBL_NAT(nDLNew - 1))
                        & NBPW_MSK(cnLogBmSwLinksPerBit);
            nLinksNew = 1 << cnLogBmSwLinksPerBit;
        }
      #endif // defined(BM_SW_FOR_REAL)
    }
      #if defined(BM_IN_LINK)
    if (bBmSwOld) {
        if (nBL != cnBitsPerWord) {
            // Copy bitmap from old link to new link.
            memcpy(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm,
                   ln.ln_awBm,
                   DIV_UP(wIndexCnt >> cnLogBmSwLinksPerBit, cnBitsPerWord)
                       * sizeof(Word_t));
        } else {
            // Initialize bitmap in new link.
            // Mind the high-order bits of the bitmap word if/when the
            // bitmap is smaller than a whole word.
            // Mind endianness.
            if ((wIndexCnt >> cnLogBmSwLinksPerBit) < cnBitsPerWord) {
                *pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm
                    = EXP(wIndexCnt) - 1;
            } else {
                memset(pwr_pLinks((BmSwitch_t *)pwSw)[nIndex].ln_awBm, -1,
                       DIV_UP(wIndexCnt >> cnLogBmSwLinksPerBit,
                       cnBitsPerWord) * sizeof(Word_t));
            }
        }
    }
      #endif // defined(BM_IN_LINK)
  #endif // defined(CODE_BM_SW)

    // Initialize the link in the new switch that points to the
    // old *pwRoot.
    // Copy wRoot from old link (after being updated) to new link.
    Link_t *pLinks =
  #if defined(CODE_BM_SW)
        bBmSwNew ? pwr_pLinks((BmSwitch_t *)pwSw) :
  #endif // defined(CODE_BM_SW)
                   pwr_pLinks((  Switch_t *)pwSw) ;
    Link_t* pLnZ = &pLinks[nIndex];
    Word_t* pwRootZ = &pLnZ->ln_wRoot;
    *pwRootZ = wRoot;
    if (nBLNew - nBLR == nBWNew) {
        Clr_bIsSkip(pwRootZ); // Change to non-skip variant.
        int nBLZ = nBLR;
      #ifdef REMOTE_LNX
        Word_t* pwLnXZ = gpwLnX(qy, nLinksNew, nIndex);
      #endif // REMOTE_LNX
        if (wr_nType(*pwRootZ) == T_SWITCH) {
            swPopCnt(qyax(Z), nBLR, wPopCnt);
        }
      #ifdef SKIP_TO_BITMAP
      #ifdef BM_POP_IN_WR_HB
        // Move pop count from leaf into wr hb if possible.
        else if ((wr_nType(*pwRootZ) == T_BITMAP)
            && (nBLR > cnBitsCnt)
            && (nBLR <= cnBitsPerWord - cnBitsVirtAddr))
        {
            swBitmapPopCnt(qyax(Z), nBLR, wPopCnt);
        }
      #endif // BM_POP_IN_WR_HB
      #endif // SKIP_TO_BITMAP
    }
  #ifdef _LNX
    Word_t* pwLnXNew = gpwLnX(qy, nLinksNew, nIndex); // pwLnX in new switch
    if (nBL >= cnBitsPerWord) {
        //assert(*pwLnXNew == 0); // NewSwitch initializes it? Nope.
      #ifdef BM_SW_BM_IN_LNX
        if (!tp_bIsBmSw(nType))
      #endif // BM_SW_BM_IN_LNX
        {
      #ifdef SW_POP_IN_LNX
            *pwLnXNew = wPopCnt; // There is no old *pwLnX to copy.
      #endif // SW_POP_IN_LNX
        }
    } else {
        // Copy the link extension. When is it not correct?
        // Maybe when the link is changing from a skip to a non-skip?
        *pwLnXNew = wLnX;
    }
  #endif // _LNX

  #ifdef SKIP_TO_BITMAP
  #ifdef _TEST_BM_UNPACKED
    // What if SKIP_TO_BITMAP and UNPACK_BM_VALUES?
    // We may want to convert to T_UNPACKED_BM instead of T_BITMAP.
    // Will the code handle a packed bm with a population greater
    // than where the conversion to unpacked would normally happen?
    // Insert, Remove, Count, etc.?
    // BUG: We should be converting to unpacked bm in a lot more
    // situations than full pop.
    Word_t* pwRootNew = &pLinks[nIndex].ln_wRoot;
// Test for T_SKIP_TO_BITMAP and convert?
// Support T_SKIP_TO_UNPACKED_BM?
    if ((wr_nType(wRoot) == T_BITMAP)
        && (GetPopCnt(qyax(New)) == EXP(nBLR)))
    {
        set_wr_nType(pLinks[nIndex].ln_wRoot, T_UNPACKED_BM);
    }
  #endif // _TEST_BM_UNPACKED
  #endif // SKIP_TO_BITMAP

    DBGI(printf("Just before InsertAtPrefixMismatch calls Insert"
                    " for prefix mismatch.\n"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return 0; // call InsertGuts again
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
}
#endif // SKIP_LINKS

#ifdef XX_LISTS // deferred splay
#ifdef USE_XX_SW

static BJL(Word_t*)BJ1(Status_t)
DoubleIt(qpa, Word_t wKey, Word_t wPopCnt, int nBWNew);

// Widen a switch.
// Replace *pLn with a link
// to a new, wider switch, and transfer the keys and values from the old
// switch to the new switch, and insert wKey.
static BJL(Word_t*)BJ1(Status_t)
DoubleDown(qpa, // (nBL, pLn) of link to original switch
           Word_t wKey, // key being inserted
           Word_t wPopCnt, // pop count of original sub tree at qp?
           int nBWNew
           )
{
  #ifdef DOUBLE_IT
      #ifdef NO_DOUBLE_IT
    #error
      #endif // NO_DOUBLE_IT
    return DoubleIt(qya, wKey, wPopCnt, nBWNew);
  #else // DOUBLE_IT
    qva; (void)wPopCnt;
    int nBLR = gnBLR(qy);
    assert(nBLR == nDL_to_nBL(nBL_to_nDL(nBLR)));
    int nBW = gnBW(qy, nBLR);
    DBGI(printf("# DoubleDown nBLR %d nBW %d wPopCnt %zd nBWNew %d.\n",
                nBLR, nBW, wPopCnt, nBWNew));
    DBGI(Dump(pwRootLast, /*wPrefix*/ (Word_t)0, cnBitsPerWord));
  #ifndef NO_CHECK_EBM_IN_DOUBLE_DOWN
  #ifdef BITMAP
    if (nBLR - nBWNew <= cnLogBitsPerLink) {
        // Doubling here would use at least as much memory as a big bitmap.
        NewBitmap(qya, nBLR, wKey, /*wPopCnt*/ 0);
    // Now we need to move the keys from the old switch to the new switch or
    // new bitmap. NewSwitch or NewBitmap changed *pwRoot.
    // But wRoot, nType, pwr, nBL, nBLR and nBW still all apply
    // to the old switch whose keys must be reinserted.
    int nBLLoop = nBLR - nBW;
    Link_t* pLinks = pwr_pLinks((Switch_t*)pwr);
    for (int nIndex = 0; nIndex < (int)EXP(nBW); nIndex++) {
        // We're calling InsertAll to insert from one of the links of
        // the old switch into the new switch or new bitmap.
        Word_t* pwRootLoop = &pLinks[nIndex].ln_wRoot;
        InsertAll(qya, pwRootLoop, /*nBLOld*/ nBLLoop,
                  (wKey & ~NZ_MSK(nBLR)) | ((Word_t)nIndex << nBLLoop));
      #ifdef XX_LISTS
        // Skip over any replicated links.
        if (wr_nType(*pwRootLoop) == T_XX_LIST) {
            while ((nIndex + 1 < (int)EXP(nBW))
                   && (pLinks[nIndex + 1].ln_wRoot == *pwRootLoop))
            {
                ++nIndex;
            }
        }
      #endif // XX_LISTS
    }
    OldSwitch(&wRoot, /* nBL */ nBL
  #if defined(CODE_BM_SW)
            , /* bBmSw */ 0, /* nLinks */ 0
  #endif // defined(CODE_BM_SW)
              );
    } else
  #endif // BITMAP
  #endif // !NO_CHECK_EBM_IN_DOUBLE_DOWN
    {
    NewSwitch(qya, wKey, nBLR, nBWNew, T_SWITCH, /*wPopCnt*/ 0);
    DBGI(printf("\n# DoubleDown just after NewSwitch.\n"));
    DBGI(Dump(pwRootLast, /*wPrefix*/ (Word_t)0, cnBitsPerWord));

    // Now we need to copy the keys from the old switch to the new switch.
    // We must modify the type of the old links and replicate them into a
    // range of links in the new switch setting up a deferred splay.

    // NewSwitch changed *pwRoot.
    // But nBL, wRoot, nType, pwr, nBLR and nBW still all apply
    // to the tree whose keys must be reinserted.

    // nBWNew is for the new switch.
    DBGI(printf("# Copying 1<<nBW links from old switch to new switch.\n"));

    Word_t* pwrNew = wr_pwr(*pwRoot);
    int nBLLoop = nBLR - nBW;
    int nBLNew = nBLR - nBWNew; // need a better name than nBLNew
    for (int nIndex = 0; nIndex < (int)EXP(nBW); nIndex++) {
        DBGI(printf("# New tree before nIndex 0x%02x ", nIndex));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
        //printf("# nIndex 0x%02x\n", nIndex);
        Link_t *pLnLoop = &pwr_pLinks((Switch_t*)pwr)[nIndex];
      #ifdef REMOTE_LNX
        Word_t* pwLnXLoop = NULL; (void)pwLnXLoop;
      #endif // REMOTE_LNX
        Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
        Word_t wPrefix = (wKey & ~NZ_MSK(nBLR)) | ((Word_t)nIndex << nBLLoop);
        Link_t linkNewModel;
        Link_t *pLnNewModel = &linkNewModel;
        *pLnNewModel = *pLnLoop;
        if (pLnNewModel->ln_wRoot == WROOT_NULL) {
            // no need to update nType
        } else
        if (wr_nType(pLnNewModel->ln_wRoot) == T_LIST) {
            if (cbEmbeddedBitmap && (nBLNew <= cnLogBitsPerLink)) {
                // At present, it doesn't matter that wRoot, pwr and nType
                // have not been updated since NewSwitch because qya comprises
                // only nBL and pwRoot which are both still correct after
                // NewSwitch.
                // We have to adjust the pop count for the switch since we
                // are about to reinsert keys that were already inserted once.
                InsertAll(qya, pwRootLoop, /*nBLOld*/ nBLLoop, wPrefix);
// counted
                continue;
            }
            set_wr_nType(pLnNewModel->ln_wRoot, T_XX_LIST);
// not counted yet
            SwIncr(qya, nBLR,
                   /*nDigitLoop*/ nIndex << (nBWNew - nBW),
                   nBWNew,
                   /*nPopCntLoop*/ Get_xListPopCnt(pwRootLoop, nBLLoop));
// counted
        } else
        if (wr_nType(pLnNewModel->ln_wRoot) == T_XX_LIST) {
            if (cbEmbeddedBitmap && (nBLNew <= cnLogBitsPerLink)) {
                // At present, it doesn't matter that wRoot, pwr and nType
                // have not been updated since NewSwitch because qya comprises
                // only nBL and pwRoot which are both still correct after
                // NewSwitch.
                // We have to adjust the pop count for the switch since we
                // are about to reinsert keys that were already inserted once.
                InsertAll(qya, pwRootLoop, /*nBLOld*/ nBLLoop, wPrefix);
// counted
                continue;
            }
            // no need to update nType
// not counted yet
            // only count this T_XX_LIST once
            if ((nIndex & NZ_MSK(gnListBLR(qyx(Loop)) - nBLLoop)) == 0) {
//printf("counting nIndex %d nBLLoop %d gnListBLR %d\n", nIndex, nBLLoop, gnListBLR(qyx(Loop)));
            SwIncr(qya, nBLR,
                   /*nDigitLoop*/ nIndex << (nBWNew - nBW),
                   nBWNew,
                   /*nPopCntLoop*/ Get_xListPopCnt(pwRootLoop, nBLLoop));
            } else {
//printf("not counting nIndex %d nBLLoop %d gnListBLR %d\n", nIndex, nBLLoop, gnListBLR(qyx(Loop)));
            }
        } else
  #ifdef BITMAP
        if (wr_nType(pLnNewModel->ln_wRoot) == T_BITMAP) {
            assert(0);
        } else
  #endif // EMBED_KEYS
  #ifdef EMBED_KEYS
        if (tp_bIsEk(wr_nType(pLnNewModel->ln_wRoot))) {
            // We need to splay the embedded keys.
            // Or change them to an external T_XX_LIST.
            // Or add (T_XX_EK, nBLR).
            InsertAll(qya, pwRootLoop, nBLLoop, wPrefix);
// counted
            continue;
        } else
  #endif // EMBED_KEYS
  #ifdef UA_PARALLEL_128
        if (wr_nType(pLnNewModel->ln_wRoot) == T_LIST_UA) {
            assert(0);
        } else
  #endif // UA_PARALLEL_128
        if (tp_bIsSkip(wr_nType(pLnNewModel->ln_wRoot))) {
            assert(0);
        } else {
            DBGX(printf("pLnNew->ln_wRoot 0x%zx\n", pLnNewModel->ln_wRoot));
            assert(0);
        }
        // Replicate the new link.
        for (int nIndexNew = nIndex << (nBWNew - nBW);
             nIndexNew
                 < (nIndex << (nBWNew - nBW)) + (int)EXP(nBWNew - nBW);
             ++nIndexNew)
        {
// We could leave some links with WROOT_NULL if there are no keys in the list
// that belong in the expanse. But would that confuse other code that
// compares ln_wRoot to find the expanse of a shared list?
// And/or would it be expensive?
            Link_t *pLnNewLoop = &pwr_pLinks((Switch_t*)pwrNew)[nIndexNew];
            *pLnNewLoop = *pLnNewModel;
        }
#if 0
        if (wr_nType(pLnNewModel->ln_wRoot) == T_XX_LIST) {
            SwIncr(qya, nBLR,
                   /*nDigitLoop*/ nIndex << (nBWNew - nBW),
                   nBWNew,
                   /*nPopCntLoop*/ Get_xListPopCnt(pwRootLoop, nBLLoop));
        }
// counted
#endif
    }
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    swPopCnt(qya, nBLR, wPopCnt - 1);
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    if (gwPopCnt(qya, nBLR) != wPopCnt - 1) {
printf("# DoubleDown wPopCnt %zd gwPopCnt %zd\n", wPopCnt, gwPopCnt(qya, nBLR));
Dump(pwRootLast, 0, cnBitsPerWord);
exit(1);
    }
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
  #if cnSwCnts != 0
    // Update sw_awCnts.
    // Insert has already been counted.
    memcpy(((Switch_t*)pwrNew)->sw_awCnts, ((Switch_t*)pwr)->sw_awCnts,
           sizeof(((Switch_t*)pwr)->sw_awCnts));
  #endif // cnSwCnts != 0
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
    OldSwitch(&wRoot, nBL
  #if defined(CODE_BM_SW)
            , /* bBmSw */ 0, /* nLinks */ 0
  #endif // defined(CODE_BM_SW)
              );
    // We try to trigger the DoubleDown on an insert before a splay
    // will be needed by bumping ListPopCntMax by 1 for each half-digit.
    // Would be nice if the insert didn't require a new malloc for the list.
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
  #if cnSwCnts != 0
    Word_t* pwCnts = ((Switch_t*)pwrNew)->sw_awCnts;
    int nDigit = (wKey >> (nBLR - nBW)) & MSK(nBW);
    if (nBLR <= 16) {
        int nShift = (nBW > nLogSwSubCnts(1)) ? (nBW - nLogSwSubCnts(1)) : 0;
        --((uint16_t*)pwCnts)[nDigit >> nShift];
    } else if (nBLR <= 32) {
        int nShift = (nBW > nLogSwSubCnts(2)) ? (nBW - nLogSwSubCnts(2)) : 0;
        --((uint32_t*)pwCnts)[nDigit >> nShift];
    } else {
      #if cnSwCnts == 1
        if (!(nDigit >> (nBW - 1)))
      #endif // cnSwCnts == 1
        {
            int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
            --pwCnts[nDigit >> nShift];
        }
    }
  #endif // cnSwCnts != 0
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
    }
    DBGI(printf("# DoubleDown just before final Insert "));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return BJL((Word_t*)) -1; // call InsertGuts again
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
  #endif // DOUBLE_IT else
}

// Insert a narrow switch before the list.
// Splay the list if cnSwCnts != 0,
// otherwise convert the list to a shared list and fill the appropriate links
// in the new switch with links to the shared list.
// Then insert wKey.
  #ifdef B_JUDYL
static Word_t*
  #else // B_JUDYL
static Status_t
  #endif // B_JUDYL
InsertXxSw(qpa, // (nBL, pLn) of link to list
           Word_t wKey, // key being inserted
           int nPopCnt) // pop count of list before insert
{
    qva; (void)nPopCnt;
    assert(nType == T_LIST); // Temp? What about T_LIST_UA?
    int nDL = nBL_to_nDL(nBL); (void)nDL; // temp limit
    assert(nBL == nDL_to_nBL(nDL)); // temp limit
    int nBLR = gnListBLR(qy);
    assert(nBLR == nBL); // temp limit
    int nDLR = nBL_to_nDL(nBLR); (void)nDLR;
    assert(nDLR == nDL); // temp limit
    DBGI(printf("\n# InsertXxSw at nBL %d nBLR %d.\n", nBL, nBLR));
  #if defined(NO_TYPE_IN_XX_SW)
    assert(0);
  #endif // defined(NO_TYPE_IN_XX_SW)
    int nBitCnt = SignificantBitCnt(qy, wKey, nPopCnt);
    // All keys have the same prefix of 64 - nBitCnt bits.
    DBGX(printf("\n## nBitCnt %d\n", nBitCnt));
    // Max nBL after new switch decode that will divide the list is nBitCnt-1.
    int nBLNew = nBitCnt - 1; // two links will point to the list
    assert(nBLNew < nBL);
    int nDLRUpNew = nBL_to_nDL(nBLNew + 1); (void)nDLRUpNew;
    int nBLRUpNew = nDL_to_nBL(nDLRUpNew); // nBLR of new switch
  #if cnSwCnts == 0
    int nBWMin = cnBWMin;
  #else // cnSwCnts == 0
    // What is the smallest nBW we can use to make sure we never have a shared
    // list with keys in different subexpanses.
    // How many subexpanses do we have at nBL (nBLR if we ever support skip to
    // T_LIST)?
    // cnSwCnts * sizeof(Word_t) / (2 << LOG(MAX(nBL, 16) - 1))
    int nBWMin = MAX(cnBWMin,
                     LOG(cnSwCnts * sizeof(Word_t) * 8
                         / (2 << LOG(MAX(nBL, 16) - 1))));
  #endif // cnSwCnts == 0 else
    if (nBLRUpNew - nBLNew < nBWMin) {
        DBGI(printf("# IXS nBitCnt %d nBLNew %d\n", nBitCnt, nBLNew));
        nBLNew = nBLRUpNew - nBWMin;
        DBGI(printf("Satisfy nBWMin %d by adjusting nBLNew %d\n",
                    nBWMin, nBLNew));
        assert(nBL_to_nDL(nBLNew + 1) == nDLRUpNew);
    }
    assert(nBLNew >= cnBitsInD1);
    assert(nBLRUpNew >= cnBitsLeftAtDl2);
    int nBWUpNew = nBLRUpNew - nBLNew; // nBW of new switch
    DBGI(printf("# With nBLRUpNew %d nBWUpNew %d.\n", nBLRUpNew, nBWUpNew));
    DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
  #ifndef SKIP_TO_XX_SW
    // nBLRUpNew is nBitCnt rounded up to the nearest digit.
    // A full-width switch at nBLRUpNew will splay the list.
    if (nBLRUpNew != nBL) {
        // Why aren't we getting here with our testing?
        assert(0); // Do we have any tests that get us here?
        // Darn. We need to skip and we can't.
        // Maybe we should just forget about inserting a narrow switch here.
        // Just because we can't skip to a narrow switch doesn't mean we can't
        // skip to a bitmap or uncompressed switch.
        // What are the other options?
        assert(nBLRUpNew < nBL);
        nBLRUpNew = nBL;
        // nBLRUpNew is now nBL of list.
        // nBLNew is nBitCnt-1 reduced if necessary to satisfy nBWMin.
        assert(nBLNew < nBLRUpNew - nBLR_to_nBW(nBLRUpNew));
        nBWUpNew = 1; // too bad we can't use zero yet
    }
  #endif // #ifndef SKIP_TO_XX_SW

  #if !defined(_LNX) || defined(REMOTE_LNX)
    // wRoot serves as our saved link
    assert(sizeof(Link_t) == sizeof(wRoot));
  #endif // !_LNX || REMOTE_LNX
    NewSwitch(qya, wKey, nBLRUpNew, nBWUpNew, T_SWITCH, 0);

    DBGI(printf("\n# InsertXxSw just after NewSwitch new tree "));
    DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));

  #if cnSwCnts != 0
    // We have to splay the list.
    Splay(qya, /*pwRoot of list*/ &wRoot, /*nBL of list*/ nBL, wKey);
  #else // cnSwCnts != 0
    // We must modify the type of the old link and replicate it into all
    // of the links in the new switch setting up a deferred splay.

    // NewSwitch changed *pwRoot.
    // But wPrefix, nBL, wRoot, nType, pwr, and nBLR still all apply
    // to the tree whose keys must be reinserted.

    Link_t linkNewModel;
    linkNewModel.ln_wRoot = wRoot;
    set_wr_nType(linkNewModel.ln_wRoot, T_XX_LIST);
    pwr = wr_pwr(*pwRoot);
    int nDigit = (wKey >> nBLNew) & ~MSK(nBWUpNew);
    DBGI(printf("\n# IXS nBLNew %d nDigit 0x%02x.\n", nBLNew, nDigit));
    int nBLLoop = nBLNew;
    assert(nBLNew < nBitCnt); // just checking to see if this is true
    int nBLRNew = nBitCnt;
    // We have nBitCnt aka nBLRNew and nBLNew.
    int nFirstDigit = nDigit & ~NBPW_MSK(nBLRNew - nBLNew);
    int nLastDigit = nFirstDigit + NBPW_EXP(nBLRNew - nBLNew);
    for (int nIndex = nFirstDigit; nIndex < nLastDigit; nIndex++) {
        Link_t *pLnLoop = &pwr_pLinks((Switch_t*)pwr)[nIndex];
        *pLnLoop = linkNewModel;
        Word_t* pwRootLoop = &pLnLoop->ln_wRoot;
        snListBLR(qyx(Loop), nBLRNew);
    }
    swPopCnt(qya, nBLRUpNew, nPopCnt);
  #endif // cnSwCnts != 0 else

    DBGI(printf("# InsertXxSw just before final Insert "));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));

    // We try to trigger the InsertXxSw on an insert before the list is full.
    // Would be nice if the insert didn't require a new malloc for the list.
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return 0;
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
}

#endif // USE_XX_SW
#endif // XX_LISTS

#ifdef USE_XX_SW
// Widen the switch pointed to by qpa to nBWNew bits.
// Replace *pLn, which is the link to the switch, with a link
// to a new, wider switch, and transfer the keys and values from the old
// switch to the new switch.
// If a link in the new switch would fit an embedded bitmap then
// replace the switch with a single bitmap leaf instead.
// Then insert wKey.
static BJL(Word_t*)BJ1(Status_t)
DoubleIt(qpa, Word_t wKey, Word_t wPopCnt, int nBWNew)
{
  #ifdef NO_DOUBLE_IT
      #ifdef DOUBLE_IT
    #error
      #endif // DOUBLE_IT
    return DoubleDown(qya, wKey, wPopCnt, nBWNew);
  #else // NO_DOUBLE_IT
    qva; (void)wPopCnt;
    int nBLR = gnBLR(qy);
    assert(nBLR == nDL_to_nBL(nBL_to_nDL(nBLR)));
    assert(nBLR > cnLogBitsPerLink);
    int nBW = gnBW(qy, nBLR);
    DBGI(printf("# DoubleIt nBL %d nBLR %d nBW %d wPopCnt %zd nBWNew %d.\n",
                nBL, nBLR, nBW, wPopCnt, nBWNew));
    DBGI(Dump(pwRootLast, /*wPrefix*/ (Word_t)0, cnBitsPerWord));
  #ifndef NO_CHECK_EBM_IN_DOUBLE_IT
  #ifdef BITMAP
    if (nBLR - nBWNew <= cnLogBitsPerLink) {
        // Doubling here would use at least as much memory as a big bitmap.
        NewBitmap(qya, nBLR, wKey, /*wPopCnt*/ 0);
    } else
  #endif // BITMAP
  #endif // !NO_CHECK_EBM_IN_DOUBLE_IT
    {
        NewSwitch(qya, wKey, nBLR, nBWNew, T_SWITCH, /* wPopCnt */ 0);
        assert(gnBW(qy, nBLR) == nBWNew); // make sure its not overwritten
        assert(tp_bIsXxSw(wr_nType(*pwRoot)) == (nBWNew != nBLR_to_nBW(nBLR)));
    }
    // Now we need to move the keys from the old switch to the new switch or
    // new bitmap. NewSwitch or NewBitmap changed *pwRoot.
    // But wRoot, nType, pwr, nBL, nBLR and nBW still all apply
    // to the old switch whose keys must be reinserted.
    int nBLLoop = nBLR - nBW;
    Link_t* pLinks = pwr_pLinks((Switch_t*)pwr);
    for (int nIndex = 0; nIndex < (int)EXP(nBW); nIndex++) {
        // We're calling InsertAll to insert from one of the links of
        // the old switch into the new switch or new bitmap.
        Word_t* pwRootLoop = &pLinks[nIndex].ln_wRoot;
        InsertAll(qya, pwRootLoop, /*nBLOld*/ nBLLoop,
                  (wKey & ~NZ_MSK(nBLR)) | ((Word_t)nIndex << nBLLoop));
      #ifdef XX_LISTS
        // Skip over any replicated links.
        if (wr_nType(*pwRootLoop) == T_XX_LIST) {
            while ((nIndex + 1 < (int)EXP(nBW))
                   && (pLinks[nIndex + 1].ln_wRoot == *pwRootLoop))
            {
                ++nIndex;
            }
        }
      #endif // XX_LISTS
    }
    OldSwitch(&wRoot, /* nBL */ nBL
  #if defined(CODE_BM_SW)
            , /* bBmSw */ 0, /* nLinks */ 0
  #endif // defined(CODE_BM_SW)
              );
    DBGI(printf("Just before DoubleIt calls final Insert.\n"));
    DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return BJL((Word_t*)) -1;
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
  #endif // NO_DOUBLE_IT else
}
#endif // USE_XX_SW

#ifdef XX_LISTS
  #ifdef B_JUDYL
static Word_t*
  #else // B_JUDYL
static Status_t
  #endif // B_JUDYL
InsertAtFullXxList(qpa, Word_t wKey, int nPopCnt, int nPos,
                   int nBLUp, Link_t *pLnUp
          #ifdef REMOTE_LNX
                 , Word_t* pwLnXUp
          #endif // REMOTE_LNX
                   )

{
    qva; (void)nPopCnt; (void)nPos;
    (void)nBLUp;
    DBGI(printf("# IAXL pLn %p\n", pLn));
    DBGI(printf("# IAXL pwr %p\n", pwr));
    int nBLR = gnListBLR(qy); (void)nBLR;
    int nDLRUp = nBL_to_nDL(nBL + 1); (void)nDLRUp;
    int nBLRUp = nDL_to_nBL(nDLRUp);
    int nBWRUp = nBLRUp - nBL;
    //int nBLR = gnListBLR(qy);
    int nDigitUp = (wKey >> nBL) & MSK(nBWRUp);
    DBGI(printf("# IAXL nDigitUp 0x%02x\n", nDigitUp));
    Link_t* pLinks = &pLn[-nDigitUp];
    Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
    Word_t wRootUp = *pwRootUp; (void)wRootUp;
    // Set the links in the switch to WROOT_NULL.
// Ultimately, I would like to be able to limit the splay to 1-bit.
// The first step is a partial splay of one half of the list.
    for (int nDigit = 0; nDigit < (1 << nBWRUp); ++nDigit) {
        if (pLinks[nDigit].ln_wRoot == wRoot) {
            pLinks[nDigit].ln_wRoot = WROOT_NULL;
        }
    }
  #if cnSwCnts != 0
    // Adjust the pop counts in the list because Splay will adjust them back.
    DBGI(printf("\n# IAXL nBL %d nBWRUp %d nPopCnt %d\n",
                nBL, nBWRUp, nPopCnt));
    Word_t* pwCnts = ((Switch_t*)wr_pwr(wRootUp))->sw_awCnts;
    if (nBLR <= 16) {
#if 1
        int nShift = (nBWRUp > cnLogSwCnts + 2)
                       ? (nBWRUp - cnLogSwCnts - 2) : 0;
        ((uint16_t*)pwCnts)[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        --((uint16_t*)pwCnts)[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
#else
        ((uint16_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 2)] -= nPopCnt;
#endif
    } else if (nBLR <= 32) {
#if 1
        int nShift = (nBWRUp > cnLogSwCnts + 1)
                       ? (nBWRUp - cnLogSwCnts - 1) : 0;
        ((uint32_t*)pwCnts)[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        --((uint32_t*)pwCnts)[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
#else
        ((uint32_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 1)] -= nPopCnt;
#endif
    } else {
      #if cnSwCnts == 1
        if (!(nDigitUp >> (nBWRUp - 1)))
      #endif // cnSwCnts == 1
#if 1
        {
            int nShift = (nBWRUp > cnLogSwCnts) ? (nBWRUp - cnLogSwCnts) : 0;
            pwCnts[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
            --pwCnts[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
        }
#else
        { pwCnts[nDigitUp >> (nBWRUp - cnLogSwCnts)] -= nPopCnt; }
#endif
    }
  #endif // cnSwCnts != 0

    // We need to splay the XX list into the links in the full-width switch.
  #ifdef SPLAY_WITH_INSERT
    return SplayWithInsert(qyax(Up), /* pwRootOld */ &wRoot, nBL, wKey, nPos);
  #else // SPLAY_WITH_INSERT
    //printf("nBLR before splay %d\n", nBLR);
    Splay(qyax(Up), &wRoot, nBL, wKey);
      #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    swPopCnt(qyax(Up), nBLRUp, gwPopCnt(qyax(Up), nBLRUp) - 1);
    return BJL((Word_t*)) -1;
      #else // _RETURN_NULL_TO_INSERT_AGAIN
#if 1
    swPopCnt(qyax(Up), nBLRUp, gwPopCnt(qyax(Up), nBLRUp) - 1);
    return Insert(qyx(Up), wKey);
#else
// We can't use InsertGuts after Splay because Splay might create a switch and
// InsertGuts isn't coded to handle inserting into a switch that is not a
// prefix mismatch or missing link in BM SW yet.
// Should be easy to fix, but not done yet.
    // Update nPos for InsertGuts.
    if ((tp_bIsList(wr_nType(WROOT_NULL)) && (pLn->ln_wRoot != WROOT_NULL))
        && tp_bIsList(wr_nType(pLn->ln_wRoot)))
    {
        nBLR = gnListBLR(qy);
        nPos = ~SearchList(qya, nBLR, wKey);
    } else {
        nPos = -1;
    }
    return InsertGuts(qya, wKey, nPos
          #ifdef CODE_XX_SW
                    , pLnUp, nBLUp
              #ifdef REMOTE_LNX
                    , pwLnXUp
              #endif // REMOTE_LNX
          #endif // CODE_XX_SW
                      );
#endif
      #endif // _RETURN_NULL_TO_INSERT_AGAIN else
  #endif // SPLAY_WITH_INSERT else
}

  #ifdef B_JUDYL
static Word_t*
  #else // B_JUDYL
static Status_t
  #endif // B_JUDYL
InsertAtFullUnalignedXxList(qpa, Word_t wKey, int nPopCnt, int nPos,
                            int nBLUp, Link_t* pLnUp
  #ifdef REMOTE_LNX
                          , Word_t* pwLnXUp
  #endif // REMOTE_LNX
                            )
{
    qva; (void)nPopCnt; (void)nPos;
    Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
    Word_t wRootUp = *pwRootUp;
    DBGI(printf("# IAFUXL nBL %d gnListBLR %d\n", nBL, gnListBLR(qy)));
    DBGI(printf("# IAFUXL nBLUp %d gnBLR(Up) %d gwPopCnt(Up) %zd\n",
                nBLUp, gnBLR(qyx(Up)), gwPopCnt(qyax(Up), gnBLR(qyx(Up)))));
    int nBLRUp = gnBLR(qyx(Up));
    int nBWRUp = nBLRUp - nBL;
    assert(nBWRUp == gnBW(qyx(Up), nBLRUp));
    DBGI(printf("# IAFUXL nBWRUp %d\n", nBWRUp));
    Word_t *pwrUp = wr_pwr(wRootUp);
    Link_t* pLinks = ((Switch_t*)pwrUp)->sw_aLinks;
    DBGI(printf("# IAFUXL pwrUp %p\n", pwrUp));
    // Set the links in the switch to WROOT_NULL.
    for (int nDigit = 0; nDigit < (1 << nBWRUp); ++nDigit) {
        if (pLinks[nDigit].ln_wRoot == wRoot) {
            pLinks[nDigit].ln_wRoot = WROOT_NULL;
        }
    }
  #if cnSwCnts != 0
    // Adjust the pop counts in the list because Splay will adjust them back.
    // Is it possible that the subexpanse of the key isn't the same as the
    // first subexpanse of the shared list? Which implies that the shared list
    // spans a subexpanse boundary.
    DBGI(printf("\n# IAFUXL nBWRUp %d nPopCnt %d\n", nBWRUp, nPopCnt));
    int nDigitUp = (wKey >> nBL) & MSK(nBWRUp);
    int nBLR = gnListBLR(qy); (void)nBLR;
    DBGI(printf("\n# IAFUXL nBLR %d nBLRUp %d\n", nBLR, nBLRUp));
    Word_t* pwCnts = ((Switch_t*)wr_pwr(wRootUp))->sw_awCnts;
    if (nBLRUp <= 16) {
#if 1
        int nShift = (nBWRUp > cnLogSwCnts + 2)
                       ? (nBWRUp - cnLogSwCnts - 2) : 0;
        ((uint16_t*)pwCnts)[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        --((uint16_t*)pwCnts)[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
#else
        ((uint16_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 2)] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        // The final Insert below will increment the subexpanse pop count
        // And it was already done before IAFUXL was called.
        // Is there a reason we can't use InsertGuts below like we do
        // in IAXL? Or have we just not changed the code yet?
        --((uint16_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 2)];
      #endif // SPLAY_WITH_INSERT
#endif
    } else if (nBLRUp <= 32) {
        assert(nBLR <= nBLRUp - 1); // code doesn't handle this
#if 1
        int nShift = (nBWRUp > cnLogSwCnts + 1)
                       ? (nBWRUp - cnLogSwCnts - 1) : 0;
        ((uint32_t*)pwCnts)[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        --((uint32_t*)pwCnts)[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
#else
        ((uint32_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 1)] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
        --((uint32_t*)pwCnts)[nDigitUp >> (nBWRUp - cnLogSwCnts - 1)];
      #endif // SPLAY_WITH_INSERT
#endif
    } else {
        assert(nBLR < nBLRUp); // code doesn't handle this.
      #if cnSwCnts == 1
        if (!(nDigitUp >> (nBWRUp - 1)))
      #endif // cnSwCnts == 1
        {
#if 1
            int nShift = (nBWRUp > cnLogSwCnts) ? (nBWRUp - cnLogSwCnts) : 0;
            pwCnts[nDigitUp >> nShift] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
            --pwCnts[nDigitUp >> nShift];
      #endif // SPLAY_WITH_INSERT
#else
            pwCnts[nDigitUp >> (nBWRUp - cnLogSwCnts)] -= nPopCnt;
      #ifndef SPLAY_WITH_INSERT
            --pwCnts[nDigitUp >> (nBWRUp - cnLogSwCnts)];
      #endif // SPLAY_WITH_INSERT
#endif
        }
    }
  #endif // cnSwCnts != 0

  #ifdef SPLAY_WITH_INSERT
    // We need to splay the XX list into the links in the full-width switch.
    return SplayWithInsert(qyax(Up), /* pwRootOld */ &wRoot, nBL, wKey, nPos);
  #else // SPLAY_WITH_INSERT
    Splay(qyax(Up), /* pwRootOld */ &wRoot, nBL, wKey);
    swPopCnt(qyax(Up), nBLRUp, gwPopCnt(qyax(Up), nBLRUp) - 1);
      #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return BJL((Word_t*)) -1;
      #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qyx(Up), wKey);
      #endif // _RETURN_NULL_TO_INSERT_AGAIN
  #endif // #else SPLAY_WITH_INSERT
}
#endif // XX_LISTS

// Replace the link at qp that points to an external list with a link to a
// new switch or bitmap.
// Or replace the link to the switch that contains qp, i.e. (nBLUp, pLnUp),
// with a link to a new, widened switch using DoubleIt.
// Do we ever replace the link to the switch than contains qp with a link
// to a bitmap?
//
// Then transfer any keys and values from the array that was rooted at the
// replaced link to the tree that is rooted at the new link.
// And, finally, insert wKey.
// Why are we here? What is the caller hoping for?
// #1. Splay into a new switch because the list is too long.
//     - Used to be called with nBLNew <= nBL unless ONLY_AT_DL2?
// #2. Widen the parent switch at (nBLUp, pLnUp).
//     - Not enough space for another embedded key.
//     - Population warrants the widening.
//     - Used to be called with nBLNew == nBL.
// #3. Convert list to bitmap.
//     - Splay can't handle list to bitmap conversion.
//
// - Insert
//   - Insert with no allocation
//     - set bit in existing bitmap
//       - BJL: use free space in existing value area
//     - embedded keys
//       - fill an existing free slot in an embeded list
//         - BJL: use free space in switch for new value
//         - BJL: use free space in existing value area (n/a)
//     - external list
//       - fill an existing free key slot in an external list
//         - BJL: use free space in existing value area
//   - Insert with allocation but no type/structure changes
//     - replace an existing external list with a bigger one
//     - replace existing bitmap with one that has a larger value area
//     - grow the ek value area (n/a)
//     - add a missing link to a bitmap switch
//       - insert into embedded key slot (or create ext list if no ek)
//   - Add an external value area for ek (n/a)
//   - Inflate a bmsw
//   - Widen an existing switch
//   - Convert a list to a bitmap
//   - Splay embedded or external list to new switch in its place
//
// Grow:
// - add
// - convert list to bitmap
// - splay list into new switch in its place
// - widen existing switch
#ifdef B_JUDYL
static Word_t*
#else // B_JUDYL
static Status_t
#endif // B_JUDYL
TransformList(qpa,
              Word_t wKey,
  #ifdef CODE_XX_SW
              int nBLUp,
              Link_t *pLnUp,
      #ifdef REMOTE_LNX
              Word_t* pwLnXUp,
      #endif // REMOTE_LNX
  #endif // CODE_XX_SW
              Word_t wPopCnt)
{
    qva; (void)wPopCnt;
    int nDL = nBL_to_nDL(nBL); (void)nDL;
    // Narrow switches don't get here.
    assert(nDL_to_nBL(nDL) == nBL);
    // Are we going to insert a switch?
    // Or are we going to widen the switch at pLnUp?
    Link_t link; (void)link;
  #ifdef SKIP_LINKS
    int nBitCnt = SignificantBitCnt(qy, wKey, wPopCnt);
    // Maximum nBLNew that will splay.
    int nBLNew = nBitCnt; // nBLNew == nBLRUpNew - nBWUpNew
      #ifdef _SKIP_TO_LIST
    // Can we just move the list down?
    // Is nBLNew smaller than nBL?
    // Is the population small enough to fit in a list at nBLNew?
    // Think about USE_XX_SW[_ONLY_AT_DL2].
    if ((nBL == 64)
        && (nDL_to_nBL(nBL_to_nDL(cnBitsPerWord / 2)) == cnBitsPerWord / 2)
        && (nDL_to_nBL(nBL_to_nDL(nBLNew + 1)) <= cnBitsPerWord / 2)
        && ((wPopCnt + 1) < auListPopCntMax[cnBitsPerWord / 2]))
    {
        DBGI(printf("SKIP_TO_LIST\n"));
        int nBLR = cnBitsPerWord / 2;
        Word_t *pwKeysOld = ls_pwKeys(pwr, nBL);
      #ifdef B_JUDYL
        Word_t *pwValuesOld = gpwValues(qy);
      #endif // B_JUDYL
        //int nDLR = nBL_to_nDL(nBLR);
        int nTypeOld = nType;
        Word_t *pwrOld = pwr;
        Word_t *pwr = NewList(wPopCnt, nBLR);
        Word_t wRoot = 0;
        int nType = T_SKIP_TO_LIST;
        set_wr(wRoot, pwr, nType);
        //Word_t *pwRootOld = pwRoot;
        *pwRoot = wRoot; // install
        snListBLR(qy, nBLR);
        // Does this work for list pop cnt not in wRoot?
        Set_xListPopCnt(pwRoot, nBLR, wPopCnt);
        wRoot = *pwRoot;
      #ifdef B_JUDYL
        // copy the values
        Word_t *pwValues = gpwValues(qy);
        COPY(&pwValues[-wPopCnt], &pwValuesOld[-wPopCnt], wPopCnt);
      #endif // B_JUDYL
        // copy the keys
        uint32_t *piKeys = ls_piKeysNATX(pwr, wPopCnt);
        COPY(piKeys, pwKeysOld, wPopCnt);
        PAD(piKeys, wPopCnt);
        OldList(pwrOld, wPopCnt, nBL, nTypeOld);
        DBGI(printf("\n# Just before final Insert "));
        DBGI(Dump(pwRootLast, /* wPrefix */ (Word_t)0, cnBitsPerWord));
        assert(tp_bIsSkip(wr_nType(*pwRoot)));
        assert(wr_nBLR(*pwRoot) == cnBitsPerWord / 2);
        goto finalInsert;
    } else {
        //DBGI(printf("No SKIP_TO_LIST\n"));
    }
      #endif // _SKIP_TO_LIST
    // We were not able to simply move the list to a smaller nBL.
    // Round up maximum nBLNew that will splay to a digit boundary.
    // Should we call this nBLRUpNew?
    nBLNew = nDL_to_nBL(nBL_to_nDL(nBLNew));
  #else // SKIP_LINKS
    int nBLNew = nBL;
  #endif // SKIP_LINKS
    int nDLNew = nBL_to_nDL(nBLNew); (void)nDLNew;

    // How the heck is TransformList supposed to figure out what
    // the caller wants? Does it matter?
    // Does it matter if the list is full or not?
// To widen a switch when pop count justifies it for USE_XX_SW_ONLY_AT_DL2?
    // Can we limit TransformList to bitmap conversions and Splay?
    // nBLNew == nBL is quite ambiguous.
    // nBL < nBLNew means create a switch and Splay?

    DBGI(printf("TransformList 0 nDLNew %d nBLNew %d nDL %d nBL %d\n",
                nDLNew, nBLNew, nDL, nBL));
#if defined(SKIP_LINKS)

  #ifdef SKIP_TO_BITMAP
    // Assuming here that we cannot skip to a list so if we want to skip to
    // D1 we have to skip to a bitmap.
    // What if USE_XX_SW_ONLY_AT_DL2?
    if ((nDLNew == 1) && (nDL > 1) && !cbEmbeddedBitmap) {
      #ifdef BMLF_CNTS_IN_LNX
        if (nDL != cnDigitsPerWord)
      #endif // BMLF_CNTS_IN_LNX
            // Shouldn't we make sure the population justifies a bitmap?
            goto newSkipToBitmap;
    }
  #endif // SKIP_TO_BITMAP

    // Apply constraints that cause us to create the new switch
    // at a higher level than would be required if only the common
    // prefix, n[BD]LNew, were considered.

    // We don't create a switch below nDL == 2. Why? Policy?
    // Nor do we create a switch at or below nBL == cnLogBitsPerWord.
    // Why can't we?
    // The latter is enforced in one way by disallowing
    // cnBitsAtDl2 <= cnLogBitsPerWord no later than Initialize time.
  #ifndef SKIP_TO_XX_SW
  #ifdef USE_XX_SW_ONLY_AT_DL2
    // We don't skip to a switch below DL3.  Because we can't
    // skip to T_XX_SW and we want T_XX_SW at DL2 and below.
    if ((nBLNew < cnBitsLeftAtDl3) && (nBL >= cnBitsLeftAtDl3)) {
        nBLNew = cnBitsLeftAtDl3;
        nDLNew = 3;
    }
  #endif // USE_XX_SW_ONLY_AT_DL2
  #endif // #ifndef SKIP_TO_XX_SW
    if ((nBLNew < cnBitsLeftAtDl2) && (nBL >= cnBitsLeftAtDl2)) {
// Why don't we allow skip to bitmap here?
        nBLNew = cnBitsLeftAtDl2;
        nDLNew = 2;
    }
    DBGI(printf("TL 1 nDLNew %d nBLNew %d nDL %d nBL %d\n",
                nDLNew, nBLNew, nDL, nBL));
#ifdef BITMAP
    // How did we get here?
    // I don't think we should get here if (nBLNew <= cnLogBitsPerLink).
    assert((nBLNew > cnLogBitsPerLink) || !cbEmbeddedBitmap);
#endif // BITMAP

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
      //#ifdef USE_XX_SW_ONLY_AT_DL2
    // t_bitmap in Insert1 can't handle a bitmap at (nBL != cnBitsLeftAtD2)
    // for USE_XX_SW_ONLY_AT_DL2 because we don't want to have to test for it.
// I'm not seeing that limitation still exists when I look at the code.
    // We expect that once (nBL == cnBitsLeftAtDl2) we just keep doubling
    // until (nBL == cnLogBitsPerLink).
      //#else // USE_XX_SW_ONLY_AT_DL2
// Splay is complicated by this installation of a (close) bitmap.
// Why don't we allow skip to bitmap?
    if ((nDLNew == 1) && !cbEmbeddedBitmap) {
        if (nBLNew != nBL) {
            printf("\nnBLNew %d nBL %d\n", nBLNew, nBL);
        }
        assert(nBLNew == nBL);
        goto newSkipToBitmap;
newSkipToBitmap:;
        // NewBitmap changes *pwRoot and we may change the Link_t
        // containing it on return from NewBitmap.
        // We need to preserve the Link_t for subsequent InsertAll.
        link = *STRUCT_OF(pwRoot, Link_t, ln_wRoot);
        int nBLOld = nBL;
        Word_t* pwRootOld = &link.ln_wRoot;
        qvx(Old); // declare and init pwRootOld
// NewBitmap installs the new bitmap.
// How are we keeping track of the list being splayed?
// nBL, pLn, pwRoot, wRoot, nType, and pwr are all unchanged.
// *pLn and *pwRoot are changed.
      #ifdef REMOTE_LNX
        Word_t* pwLnXOld = NULL;
      #endif // REMOTE_LNX
        int nPopCntOld = GetPopCnt(qyax(Old));
        pwr = NewBitmap(qya, nBLNew, wKey, nPopCntOld);
        wRoot = *pwRoot;
        nType = wr_nType(wRoot);
          #ifdef SKIP_TO_BITMAP
        assert((nBLNew == nBL)
              #ifdef BMLF_CNTS_IN_LNX
               || (nBL == cnBitsPerWord)
              #endif // BMLF_CNTS_IN_LNX
               || ((wr_nType(*pwRoot) == T_SKIP_TO_BITMAP)
                   && (gnBLR(qy) == nBLNew)));
              #ifdef _BM_POP_IN_LINK_X
              #ifndef NO_SKIP_AT_TOP
        if (nBL < cnBitsPerWord) // no bits are set yet so cnt is wrong
              #endif // !NO_SKIP_AT_TOP
              #endif // _BM_POP_IN_LINK_X
          #endif // SKIP_TO_BITMAP
        {
            // I'm pretty sure this condition on the assertion is no longer
            // necessary. But I didn't want to remove it and wait for bi to
            // run again before committing.
            if (nPopCntOld != 0) {
                if ((int)gwBitmapPopCnt(qya, nBLNew) != nPopCntOld) {
                    printf("\n# nPopCntOld %d\n", nPopCntOld);
                }
                assert((int)gwBitmapPopCnt(qya, nBLNew) == nPopCntOld);
            }
        }
        // I wish I didn't have to call InsertAllAtBitmap if nPopCntOld==0.
        InsertAllAtBitmap(qya, qyx(Old), /*nnStart*/ 0, nPopCntOld);
  #if cwListPopCntMax != 0
        if (nPopCntOld != 0) {
            OldList(pwrOld, nPopCntOld, nBLOld, nTypeOld);
        }
  #endif // cwListPopCntMax != 0
        goto finalInsert;
    }
    else
      //#endif // #else USE_XX_SW_ONLY_AT_DL2
#endif // BITMAP
    {
        int nBW; (void)nBW;
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
            // cnBWMin may not be wide enough to splay the list.
            // What happens if it's not?
            // I suspect we would end up recursively widening the list
            // in a DoubleIt, InsertAll, Insert, InsertGuts, InsertAtList,
            // TransformList cycle. Yuck.
      #ifdef USE_XX_SW_ONLY_AT_DL2
            // cnBWMin may not be wide enough to splay the list into lists
            // that can all be embedded.
            if (nBLNew == nDL_to_nBL(2)) {
                nBW = cnBWMin; // nBWUp? nBWNew?
                int nSplayMaxPopCnt
                    = SplayMaxPopCnt(pwRoot, nBL, wKey, nBLNew - nBW);
                while (nSplayMaxPopCnt > EmbeddedListPopCntMax(nBLNew - nBW)) {
                    ++nBW;
                    nSplayMaxPopCnt
                        = SplayMaxPopCnt(pwRoot, nBL, wKey, nBLNew - nBW);
                    if (nBLNew - nBW == cnLogBitsPerWord) {
                        // Can we end up with skip to big bitmap?
                        // Or skip to a wide branch with embedded bitmaps?
                        // Fix in InsertCleanup?
                        break;
                    }
                }
          #if cnSwCnts != 0
                int nBWMin = LOG(cnSwCnts * sizeof(Word_t) * 8
                                     / (2 << LOG(MAX(nBLNew, 16) - 1)));
                if (nBW < nBWMin) {
                    nBW = nBWMin;
                }
          #endif // cnSwCnts != 0
            } else
      #endif // USE_XX_SW_ONLY_AT_DL2
            {
      #ifdef SKIP_LINKS
                assert(nBLNew <= nBL); // nBLNew applies to new switch
      #else // SKIP_LINKS
                assert(nBLNew == nBL);
      #endif // #else SKIP_LINKS
                if ((nBW = nBLR_to_nBW(nBLNew)) >= 2) {
                    int nBWTopPart = MAX(cnBWMin, 2);
      #ifdef USE_LOWER_XX_SW // implies SKIP_LINKS
                    if ((nBLNew > cnBitsLeftAtDl2) // temp
                        && (nBitCnt <= nBLNew - nBWTopPart))
                    {
//fprintf(stderr, "\n# Creating lower xx sw nBLUp %d.\n", nBLUp);
                    // This happens with:
                    // DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64
                    // -DNO_EMBED_KEYS -DNO_USE_BM_SW -DDEBUG"
                    // make clean default
                        nBLNew -= nBWTopPart;
                        nBW -= nBWTopPart;
                        DBGI(printf("# Creating bottom narrow switch nBL %d"
                                        " nBitCnt %d nBLNew %d nBW %d\n",
                                    nBL, nBitCnt, nBLNew, nBW));
                    } else
      #endif // USE_LOWER_XX_SW
                    if (SplayMaxPopCnt(pwRoot, nBL, wKey, nBLNew - nBWTopPart)
                            <= auListPopCntMax[nBLNew - nBWTopPart])
                    {
//fprintf(stderr, "\n# Creating upper xx sw nBLUp %d.\n", nBLUp);
                        nBW = nBWTopPart;
                    }
      #ifdef XX_LISTS
      #if cnSwCnts != 0
                    // Don't bother creating a narrow switch with lists whose
                    // expanses are too big to be converted to shared lists
                    // when we want to double the switch.
                    int nBWMin = LOG(cnSwCnts * sizeof(Word_t) * 8
                                         / (2 << LOG(MAX(nBLNew, 16) - 1)));
                    if (nBW < nBWMin) {
                        nBLNew = nDL_to_nBL(nDLNew);
          #ifdef RIGID_XX_SW
                        nBW += cnBWIncr;
          #else // RIGID_XX_SW
                        nBW = nBWMin;
          #endif // RIGID_XX_SW
                    }
      #endif // cnSwCnts != 0
      #endif // XX_LISTS
                }
            }
      #ifdef USE_XX_SW_ALWAYS
            nBW = nBLR_to_nBW(nBLNew);
        } else if (0) {
      #else // USE_XX_SW_ALWAYS
// I can't remember why we are testing pLnUp?
// Is it supposed to signal something for NO_TYPE_IN_XX_SW?
        } else if (/*(pLnUp != NULL) &&*/ (nBL <= nDL_to_nBL(2))) {
      #endif // USE_XX_SW_ALWAYS
// Shouldn't we think about some other option here?
// What about a small bitmap?
// Or another switch?
  #endif // defined(USE_XX_SW)
            DBGI(printf("TransformList: DoubleIt\n"));
            Word_t* pwRootUp = &pLnUp->ln_wRoot;
            int nBWUp = gnBW(qyx(Up), gnBLR(qyx(Up)));
            int nBWNew = nBWUp + cnBWIncr;
            Word_t wPopCntUp = GetPopCnt(qyax(Up));
            return DoubleIt(qyax(Up), wKey, wPopCntUp, nBWNew);
  #ifdef USE_XX_SW
        } else
  #endif // defined(USE_XX_SW)
        { nBW = nBLR_to_nBW(nBLNew); }
#elif defined(USE_BM_SW) // defined(CODE_XX_SW)
        nBW = nBLR_to_nBW(nBLNew);
#endif // #elif defined(USE_BM_SW)

#if defined(DEBUG)
        if (nBLNew > nBL) {
            printf("IG: pwRoot %p wKey " OWx" nBLNew %d wRoot " OWx"\n",
                   (void *)pwRoot, wKey, nBLNew, wRoot);
            printf("nBL %d\n", nBL);
        }
#endif // defined(DEBUG)
        assert(nBLNew <= nBL);

        NewSwitch(qya, wKey, nBLNew,
#if defined(CODE_XX_SW)
                  nBW,
#endif // defined(CODE_XX_SW)
  #if defined(USE_BM_SW)
      #if defined(USE_XX_SW)
                  (nBLNew <= nDL_to_nBL(2)) ? T_SWITCH :
      #endif // defined(USE_XX_SW)
      #if defined(DOUBLE_DOWN) || defined(PP_IN_LINK)
                  // Don't waste time with T_BM_SW if splaying a big list.
// Shouldn't we be using an equivalent to InflateBmSwTest(qy) here?
                  // Use T_BM_SW if splaying a small list or handling a prefix
                  // mismatch.
                  // This should be fine-tuned for Judy1 vs. JudyL with
                  // EMBED_KEYS vs. JudyL without EMBED_KEYS, etc.
                  ((GetPopCnt(qya) + 1) >= EXP(nBW))
                          && (nBLNew <= nBL)
                      ? T_SWITCH :
                  // Here only if small subtree or nBLNew > nBL.
                  // Use uncompressed switch in prefix mismatch case if pop
                  // handles it.
                  // This should be fine-tuned for Judy1 vs. JudyL with
                  // EMBED_KEYS vs. JudyL without EMBED_KEYS, as well as
                  // variable bits per digit, etc.
                  (nBLNew > nBL)
                          && (GetPopCnt(qya) >= nDLNew * EXP(cnBitsPerDigit))
                      ? T_SWITCH :
      #endif // defined(DOUBLE_DOWN) || defined(PP_IN_LINK)
      // Handle any no-skip-to-bm-sw limitation.
      // I wonder if we should handle the no-skip-to-bm-sw
      // limitation in NewSwitchX instead of here.
      #if defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK) || defined(NO_BM_SW_AT_TOP)
              #ifdef SMART_NEW_SWITCH
                  T_BM_SW,
              #else // SMART_NEW_SWITCH
                  nBL != cnBitsPerWord ? T_BM_SW : T_SWITCH,
              #endif // SMART_NEW_SWITCH
          #else // defined(BM_IN_LINK)
                  T_BM_SW,
          #endif // defined(BM_IN_LINK)
      #else // defined(SKIP_TO_BM_SW)
          #if defined(BM_IN_LINK) || defined(NO_BM_SW_AT_TOP)
              #ifdef SMART_NEW_SWITCH
                  T_BM_SW,
              #else // SMART_NEW_SWITCH
                  (nBL != cnBitsPerWord)
                      && (nBLNew == nBL) ? T_BM_SW : T_SWITCH,
              #endif // SMART_NEW_SWITCH
          #else // defined(BM_IN_LINK)
                  (nBLNew == nBL) ? T_BM_SW : T_SWITCH,
          #endif // defined(BM_IN_LINK)
      #endif // defined(SKIP_TO_BM_SW)
  #else // defined(USE_BM_SW)
                  T_SWITCH,
  #endif // defined(USE_BM_SW)
                  /* wPopCnt */ 0);

  #ifdef USE_XX_SW_ONLY_AT_DL2
        if (nBLNew <= nDL_to_nBL(2)) {
      #if defined(SKIP_TO_XX_SW)
            if (nBLNew != nBL) {
                assert(nBLNew == nDL_to_nBL(nBL_to_nDL(nBLNew)));
                assert(GetBLR(pwRoot, nBL) == nBLNew);
                //assert(wr_nType(*pwRoot) == T_SKIP_TO_XX_SW);
                //set_wr_nType(*pwRoot, T_SKIP_TO_XX_SW);
                //assert(tp_bIsXxSw(wr_nType(*pwRoot)));
                assert(GetBLR(pwRoot, nBL) == nBLNew);
            } else
      #endif // defined(SKIP_TO_XX_SW)
            {
                if (nBW >= 7) {
                    DBGI(printf("# Setting T_XX_SW nBW %d nBLNew %d.\n",
                                nBW, nBLNew));
                }
                //assert(wr_nType(*pwRoot) == T_XX_SW);
                //set_wr_nType(*pwRoot, T_XX_SW);
            }
        }
  #endif // USE_XX_SW_ONLY_AT_DL2

        if (nBLNew == nBL) {
            DBGI(printf("\n# TransformList After NewSwitch Dump\n"));
            DBGI(Dump(pwRootLast,
                      /* wPrefix */ (Word_t)0, cnBitsPerWord));
            DBGI(printf("\n"));
        }
    }

    // Now we need to move the keys from the old subtree to the new
    // subtree.

    // Shortcut to finalInsert for empty list.
    if (wPopCnt == 0) {
        assert(wRoot == WROOT_NULL);
#if cwListPopCntMax != 0
        assert(wr_nType(WROOT_NULL) == T_LIST);
        assert(auListPopCntMax[nBL] == 0);
#endif // cwListPopCntMax != 0
        goto finalInsert;
    }
#if cwListPopCntMax != 0
    assert(wRoot != WROOT_NULL);
    assert(auListPopCntMax[nBL] != 0);

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
        // Do we ever get here?
        assert(0);
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
            InsertAll(qya, &pwr_pLinks((Switch_t *)pwr)[nIndex].ln_wRoot,
                      nBLR,
                      (wKey & ~MSK(nBLNew)) | (nIndex << nBLR)
                      );
        }

#if ! defined(SKIP_TO_XX_SW)
        assert(nBLNew == nDL_to_nBL(2));
        assert(nBL == nBLNew);
#endif // ! defined(SKIP_TO_XX_SW)
        OldSwitch(&wRoot, nBL
#if defined(CODE_BM_SW)
                , /* bBmSw */ 0, /* nLinks */ 0
#endif // defined(CODE_BM_SW)
                  );

        //printf("# New tree after InsertAll done looping:\n");
        //DBG(Dump(pwRoot, wKey, nBL));

    } else
#endif // defined(USE_XX_SW)
    {
        {
            // *pwRoot now points to a switch
  #ifdef USE_XX_SW_ONLY_AT_DL2
            assert(nBL >= nDL_to_nBL(2));
  #endif // USE_XX_SW_ONLY_AT_DL2
            // Here we are calling Splay with an orphaned list and the link
            // where the list used to reside that now points to a newly
            // created switch. nBLOld == nBL.
            // Why are we ignoring SPLAY_WITH_INSERT here? Is it because we
            // don't know nPos?
            Splay(qya, /*old*/ &wRoot, /*old*/ nBL, wKey);
        }
    }
#endif // cwListPopCntMax != 0

    goto finalInsert;
finalInsert:;

    if (nBLNew == nBL) {
        DBGI(printf("# IG: just before final Insert "));
        DBGI(Dump(pwRootLast, 0, cnBitsPerWord));
    }

  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    return 0; // call InsertGuts again
  #else // _RETURN_NULL_TO_INSERT_AGAIN
    return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
}

#ifndef cnSplayPrepThreshold
  #define cnSplayPrepThreshold  2
#endif // #ifndef cnPrepForSplayThreshold

#ifndef cnBWIncrDD
  #define cnBWIncrDD  0
#endif // cnBWIncrDD

// List is full, i.e. code constraints and/or policy do not allow us to
// simply replace the current list with a longer one.
// What do we do now?
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

// InsertAtList is called to insert a key into an existing external list
// or into a WROOT_NULL link in an existing switch.
// InsertAtList is responsible for detecting policy and mechanism constraints
// on maximum list length and calling ListIsFull when inserting a key into
// the existing list would make the list too long.
// We should consider moving the code for inserting into a WROOT_NULL link
// out of InsertAtList and into a new function.
#ifdef B_JUDYL
Word_t*
#else // B_JUDYL
Status_t
#endif // B_JUDYL
InsertAtList(qpa,
             Word_t wKey,
             int nPos
#ifdef CODE_XX_SW
           , Link_t *pLnUp, int nBLUp
      #ifdef REMOTE_LNX
           , Word_t* pwLnXUp
      #endif // REMOTE_LNX
#endif // CODE_XX_SW
             )
{
    qva;
    (void)nPos;
  #ifdef CODE_XX_SW
  #ifdef REMOTE_LNX
    (void)pwLnXUp;
  #endif // REMOTE_LNX
  #endif // CODE_XX_SW
    int nDL = nBL_to_nDL(nBL); (void)nDL;
    int nBLR = gnListBLR(qy); (void)nBLR;
    Word_t wPopCnt = 0; (void)wPopCnt;
#ifdef B_JUDYL
    Word_t *pwValue = NULL;
#endif // B_JUDYL

    DBGI(Log(qy, "InsertAtList"));
    DBGI(printf("pwr %p\n", pwr));

    // Set prefix in link if PP_IN_LINK and the list is empty and
    // we're not at the top.
    assert((wRoot != 0) || (WROOT_NULL == 0));
    // We get here no matter which type of WROOT_NULL we have.
    // When we insert into an empty expanse we call InsertAtList.
    if (wRoot != WROOT_NULL) { // pwr is pointer to old List
#if cwListPopCntMax != 0
#if defined(EMBED_KEYS)
        assert(nType != T_EMBEDDED_KEYS);
#endif // defined(EMBED_KEYS)
        wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);
        // prefix is already set
#endif // cwListPopCntMax != 0
    } else {
#if defined(PP_IN_LINK)
        if (nBL != cnBitsPerWord) {
            // What about no_unnecessary_prefix?
            set_PWR_wPrefixBL(pwRoot, pwr, nBLR, wKey);
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

    int nBLOld = nBL; (void)nBLOld;

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
    // For NO_TYPE_IN_XX_SW we assume we always have embedded keys in all
    // links at or below DL2. Hence we have to double the switch when
    // inserting into an embedded list that is already full.
    // This will be true until we have code that can handle a blow-up with
    // NO_TYPE_IN_XX_SW.
    if ((nBL < nDL_to_nBL(2))
        && (wPopCnt >= (Word_t)nEmbeddedListPopCntMax))
    {
        assert((int)wPopCnt == nEmbeddedListPopCntMax);
      #ifdef XX_LISTS
        assert(nType != T_XX_LIST);
      #endif // XX_LISTS
        DBGI(printf("IAL: DoubleIt nBL %d cnt %d max %d.\n",
                    nBL, (int)wPopCnt, nEmbeddedListPopCntMax));
        Word_t* pwRootUp = &pLnUp->ln_wRoot;
        int nBWUp = gnBW(qyx(Up), gnBLR(qyx(Up)));
        int nBWNew = nBWUp + cnBWIncr;
        Word_t wPopCntUp = GetPopCnt(qyax(Up));
        return DoubleIt(qyax(Up), wKey, wPopCntUp, nBWNew);
    }
  #endif // defined(NO_TYPE_IN_XX_SW)

  #if (cwListPopCntMax != 0)
    if ((int)wPopCnt < auListPopCntMax[nBLR]) {
        // Here only if list has room for another key.
  #ifdef DOUBLE_DOWN
        if (wPopCnt != 0)
        if (((int)wPopCnt >= auListPopCntMax[nBLR] - cnSplayPrepThreshold)
      #ifdef USE_XX_SW_ONLY_AT_DL2
            && (nBL > nDL_to_nBL(2))
      #endif // USE_XX_SW_ONLY_AT_DL2
            )
        {
        // Let's DoubleDown before the list is full if appropriate.
// Does appropriate conversion length depend on SignificantBitCnt
// w.r.t. nBL?
// I can't remember why we are testing pLnUp?
// Is if supposed to signal something for NO_TYPE_IN_XX_SW?
        if (nType != T_XX_LIST) { // temp code limitation
        if (nBL < cnBitsPerWord /*pLnUp != NULL*/) {
// Someday we may be able to convert a shared list to a new nBLR but not yet.
            Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
          #if cnSwCnts != 0
            // Make sure shared list expanse is small enough.
            int nBLRUp = gnBLR(qyx(Up));
            int nBWUp = gnBW(qyx(Up), nBLRUp);
            int nBWMin = LOG(cnSwCnts * sizeof(Word_t) * 8
                                 / (2 << LOG(MAX(nBLRUp, 16) - 1)));
            // If nBWUp < nBWMin, then the resulting shared list expanse
            // would be bigger than a subexpanse count in the switch.
            if (nBWUp >= nBWMin)
          #endif // cnSwCnts != 0
            {
            int nBitCnt = SignificantBitCnt(qy, wKey, wPopCnt);
// Don't DoubleDown if we can't splay within the digit.
          if (nBitCnt > nDL_to_nBL(nDL - 1)) {
          if (nBL != nDL_to_nBL(nDL)) { // check for lower switch
            assert(nBL != nDL_to_nBL(nDL)); // temp code expectation
            if (ListSlotCnt(wPopCnt, nBLR) >= (int)wPopCnt + 1) {
                // This is where we DoubleDown an upper narrow switch
                // if !USE_XX_SW_ONLY_AT_DL2 || (nBL > nDL_to_nBL(2)).
                // We don't send a digit-unaligned list to TransformList.
                DBGI(printf("IAL: DoubleDown nBL %d nBLUp %d\n",
                            nBL, nBLUp));
                Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
                int nBWUpNew;
                if (cnBWIncrDD != 0) {
                    nBWUpNew = nDL_to_nBL(nDL) - nBL + cnBWIncrDD;
                    int nBWUpMax = nDL_to_nBL(nDL) - nDL_to_nBL(nDL - 1);
                    if (nBWUpNew > nBWUpMax) {
                        nBWUpNew = nBWUpMax;
                    }
                } else {
                    nBWUpNew = nDL_to_nBL(nDL) - nBitCnt + 1;
                }
                return DoubleDown(qyax(Up), wKey,
                                  GetPopCnt(qyax(Up)),
                                  nBWUpNew
                                  );
            }
            assert((ListSlotCnt(wPopCnt + 1, nBLR) >= (int)wPopCnt + 2)
               || ((int)wPopCnt == auListPopCntMax[nBLR] - 1));
          }
          }
            }
        } else {
          if (nBLR > cnBitsInD1)
          {
            int nBitCnt = SignificantBitCnt(qy, wKey, wPopCnt);
            if (nBitCnt > cnBitsInD1)
            if (nBitCnt > nDL_to_nBL(nDL - 1))
            if (ListSlotCnt(wPopCnt, nBLR) >= (int)wPopCnt + 1) {
                return InsertXxSw(qya, wKey, wPopCnt);
            }
          }
        }
        }
        }
  #endif // DOUBLE_DOWN

        // Inserting into an XX_LIST is a little more involved than inserting
        // into a regular, unshared, external list.
        // If we're not going to splay the list we have to update the pop
        // count for all of the links that share the list.
        // If ListWordCnt increases we have to update pwr in all of the links
        // that share the list.
        // Neither is a problem because the links are clones of each other.
        // Or we can splay the list before inserting.
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
      #ifdef USE_XX_SW_ONLY_AT_DL2
        if (nBL == nDL_to_nBL(2)) {
          #ifdef XX_LISTS
            assert(nType != T_XX_LIST);
          #endif // XX_LISTS
            if (((wr_nType(WROOT_NULL) == T_LIST) && (wRoot != WROOT_NULL))
          #if defined(EMBED_KEYS)
                && ((int)wPopCnt >= nEmbeddedListPopCntMax)
          #endif // defined(EMBED_KEYS)
                )
            {
          #ifdef PERF_TODO
          #ifndef POP_CNT_MAX_IS_KING
                printf("\n");
                printf("wPopCnt %zd nEmbeddedListPopCntMax %d\n",
                        wPopCnt, nEmbeddedListPopCntMax);
                printf("auListPopCntMax[nBL %d] %d\n",
                       nBL, auListPopCntMax[nBL]);
                assert(0);
          #endif // #ifndef POP_CNT_MAX_IS_KING
          #endif // PERF_TODO
                if ((wWordsAllocated * 100 / wPopCntTotal)
                        < cnXxSwWpkPercent)
                {
                    // Create a switch and splay into it before the list is
                    // full.
                    return TransformList(qya, wKey,
          #ifdef CODE_XX_SW
                                         nBLUp, pLnUp,
              #ifdef REMOTE_LNX
                                         pwLnXUp,
              #endif // REMOTE_LNX
          #endif // CODE_XX_SW
                                         wPopCnt);
                }
            }
        } else
      #endif // USE_XX_SW_ONLY_AT_DL2
        // We get here only if list is not full.
        // And here we check if embedded list is full?
        // The earlier requirement that the list is full inhibits us
        // from taking this opportunity to double instead of splay?
        // Do we need auListPopCntMax to be bigger for this case?
        // Or ifdef USE_XX_SW_ONLY_AT_DL2 in the above list is full test?
        // I'm not a fan of putting the list is full and needs to double
        // case in with the code for handling the list is not full case.
      #ifdef USE_XX_SW_ONLY_AT_DL2
        if (nBL < nDL_to_nBL(2)) {
          #ifdef XX_LISTS
            if (nType != T_XX_LIST)
          #endif // XX_LISTS
            {
          #if defined(EMBED_KEYS)
                DBGI(printf("\n"));
                DBGI(printf("wPopCnt %zd nEmbeddedListPopCntMax %d\n",
                            wPopCnt, nEmbeddedListPopCntMax));
                if ((int)wPopCnt >= nEmbeddedListPopCntMax)
          #endif // defined(EMBED_KEYS)
                {
                    if ((wWordsAllocated * 100 / wPopCntTotal)
                            < cnXxSwWpkPercent)
                    {
                        DBGI(printf("IAL: DoubleIt\n"));
                        Word_t* pwRootUp = &pLnUp->ln_wRoot;
                        int nBWUp = gnBW(qyx(Up), gnBLR(qyx(Up)));
                        int nBWNew = nBWUp + cnBWIncr;
                        Word_t wPopCntUp = GetPopCnt(qyax(Up));
                        return DoubleIt(qyax(Up), wKey, wPopCntUp, nBWNew);
                    }
                }
            }
        } else
      #endif // USE_XX_SW_ONLY_AT_DL2
        { }
    #endif // defined(CODE_XX_SW)

        Word_t *pwList;

        // Allocate memory for a new list if necessary.
        // Init or update pop count if necessary.
        if ((wPopCnt == 0)
            || (ListSlotCnt(wPopCnt, nBLR) < (int)(wPopCnt + 1)))
        {
            DBGI(printf("\n"));
            DBGI(printf("nBL %d", nBL));
            DBGI(printf(" nType %d", nType));
            DBGI(printf(" pwr %p", pwr));
            DBGI(printf(" nBLR %d", nBLR));
            DBGI(printf(" wPopCnt %d", (int)wPopCnt));
            if (wPopCnt != 0) {
                DBGI(printf(" ListSlotCnt(wPopCnt %d) %d",
                            (int)wPopCnt, ListSlotCnt(wPopCnt, nBLR)));
            }
            DBGI(printf("\n"));
            // Allocate a new list and init pop count if pop count is
            // in the list.  Also init the beginning of the list marker
            // if LIST_END_MARKERS.
            assert(wPopCnt + 1 != 0);
            pwList = NewList(wPopCnt + 1, nBLR);
  #if defined(UA_PARALLEL_128)
            if ((nBL == 16) && (wPopCnt == 0)) {
      #ifdef XX_LISTS
                assert(nType != T_XX_LIST);
      #endif // XX_LISTS
                set_wr(wRoot, pwList, T_LIST_UA);
            } else if ((nBL == 16) && (wPopCnt == 6)) {
      #ifdef XX_LISTS
                assert(nType != T_XX_LIST);
      #endif // XX_LISTS
                set_wr(wRoot, pwList, T_LIST);
            } else
  #endif // defined(UA_PARALLEL_128)
            { set_wr(wRoot, pwList, nType); }

            // qy is no longer correct.
            // wRoot has been updated.
            // pLn->ln_wRoot has not been updated.
            // pwr, nType have not been updated.
            // Can we delay updating wRoot?
            // Can we update the others now?
        } else {
            pwList = pwr;
        }

        // It's a bit ugly that we are installing this T_LIST for
        // NO_TYPE_IN_XX_SW.  But we know that DEL is going to fix it.
        // We could give DeflateExternalList &wRoot, then install the new
        // wRoot ourselves (if DEL returned it).

        pLn->ln_wRoot = wRoot; // install new
        snListBLR(qy, nBLR);
        Set_xListPopCnt(pwRoot, nBLR, wPopCnt + 1);
        wRoot = pLn->ln_wRoot;
        int nTypeOld = nType;
        nType = wr_nType(wRoot);
        Word_t *pwrOld = pwr;
        pwr = pwList;
        DBGX(Log(qy, "After NewList"));

        if (wPopCnt != 0) {
#if defined(COMPRESSED_LISTS)
            if (nBLR <= 8) {
                goto copyWithInsert8;
copyWithInsert8:
                BJL(pwValue =)
                    CopyWithInsert8(qy, ls_pcKeysNATX(pwrOld, wPopCnt),
                                    wPopCnt, (unsigned char)wKey, nPos);
            } else if (nBLR <= 16) {
                goto copyWithInsert16;
copyWithInsert16:
                BJL(pwValue =)
                    CopyWithInsert16(qy, ls_psKeysNATX(pwrOld, wPopCnt),
                                     wPopCnt, (uint16_t)wKey, nPos);
                if (nBLR == 16) {
                    // BUG: there is probably a faster way to do this.
                    UpdateDist(qya, wPopCnt + 1);
                }
#if (cnBitsPerWord > 32)
            } else if (nBLR <= 32) {
                goto copyWithInsert32;
copyWithInsert32:
                BJL(pwValue =)
                    CopyWithInsert32(qy, ls_piKeysNATX(pwrOld, wPopCnt),
                                     wPopCnt, (unsigned int)wKey, nPos);
#endif // (cnBitsPerWord > 32)
            } else
#endif // defined(COMPRESSED_LISTS)
            {
                goto copyWithInsertWord;
copyWithInsertWord:
                BJL(pwValue =)
                    CopyWithInsertWord(qy, ls_pwKeys(pwrOld, nBLR),
                                       wPopCnt, wKey, nPos);
            }
        } else {
#if defined(COMPRESSED_LISTS)
            if (nBLR <= 8) {
  #if !defined(EMBED_KEYS) \
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
                if (nBLR == cnBitsInD1) {
                    pwValue = &gpwValues(qy)[~(wKey & MSK(cnBitsInD1))];
                } else
          #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
                { pwValue = &gpwValues(qy)[~wPopCnt]; }
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && ... && defined(PSPLIT_PARALLEL)
            } else if (nBLR <= 16) {
  #ifdef EMBED_KEYS
                ls_psKeysNATX(pwList, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
                pwValue = &gpwValues(qy)[~wPopCnt];
      #endif // B_JUDYL
  #else // EMBED_KEYS
                // PSPLIT_PARALLEL && PSPLIT_SEARCH_16 needs to pad the
                // bucket, or DSPLIT_16 needs to call UpdateDist.
                nPos = 0;
                //printf("goto copyWithInsert16\n");
                goto copyWithInsert16;
  #endif // EMBED_KEYS
#if (cnBitsPerWord > 32)
            } else if (nBLR <= 32) {
  #if !defined(EMBED_KEYS) \
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
  #if defined(PARALLEL_SEARCH_WORD)
                //printf("goto copyWithInsertWord\n");
                goto copyWithInsertWord;
  #else // defined(PARALLEL_SEARCH_WORD)
                ls_pwKeysX(pwList, nBLR, wPopCnt + 1)[wPopCnt] = wKey;
      #ifdef B_JUDYL
                pwValue = &gpwValues(qy)[~wPopCnt];
      #endif // B_JUDYL
  #endif // !defined(EMBED_KEYS) && defined(PARALLEL_SEARCH_WORD)
            }
            // Shouldn't we be padding the extra key slots
            // for parallel search?
            // Is the uncompressed lists so dead that we
            // should abandon it?
        }

  #ifdef XX_LISTS
        if (nType == T_XX_LIST) {
            // Replicate the link with a new pop cnt and maybe pwr.
            int nBWRUp;
            Link_t *pLinksUp;
            assert(pLnUp != NULL);
            if (pLnUp == NULL) {
                int nDLRUp = nDL + 1; (void)nDLRUp;
                int nBLRUp = nDL_to_nBL(nDLRUp); (void)nBLRUp;
                nBWRUp = nDLR_to_nBW(nDLRUp);
                // replicate the link with a new pop cnt and maybe pwr
                int nDigitUp = (wKey >> nBL) & MSK(nBWRUp);
                pLinksUp = &pLn[-nDigitUp];
                DBGI(printf("pLinksUp %p\n", pLinksUp));
            } else {
                Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
                Word_t wRootUp = *pwRootUp;
                Word_t* pwrUp = wr_pwr(wRootUp);
                nBWRUp = gnBW(qyx(Up), gnBLR(qyx(Up)));
                pLinksUp = ((Switch_t*)pwrUp)->sw_aLinks;
            }
            for (int nDigit = 0; nDigit < (1 << nBWRUp); ++nDigit) {
                if (wr_pwr(pLinksUp[nDigit].ln_wRoot) == pwrOld) {
                    pLinksUp[nDigit] = *pLn;
                }
            }
        }
  #endif // XX_LISTS

        if ((wPopCnt != 0) && (pwr != pwrOld)) {
            OldList(pwrOld, wPopCnt, nBLR, nTypeOld);
        }

        assert(tp_bIsList(nType));
#if defined(EMBED_KEYS)
        if ((int)wPopCnt < EmbeddedListPopCntMax(nBL)
  #ifdef XX_LISTS
            && (nType != T_XX_LIST)
  #endif // XX_LISTS
            )
        {
            DeflateExternalList(qya, wPopCnt + 1);
            assert(*pwRoot != WROOT_NULL);
#ifdef B_JUDYL
      #ifdef EK_XV
            if (wPopCnt != 0) {
                pwValue = &wr_pwr(*pwRoot)[nPos];
            } else
      #endif // EK_XV
            // Update pwValue for return.
            pwValue = pwLnX;
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
            if (!tp_bIsEk(wr_nType(*pwRoot))) {
                printf("\nnType 0x%x wPopCnt " OWx"\n",
                       wr_nType(*pwRoot), wPopCnt);
            }
  #endif // defined(DEBUG)
            assert(tp_bIsEk(wr_nType(*pwRoot)));
#endif // defined(NO_TYPE_IN_XX_SW)
            DBGR(printf("IG: after DEL *pwRoot " OWx"\n", *pwRoot));
        }
#endif // defined(EMBED_KEYS)
    }
    else
  #endif // (cwListPopCntMax != 0)
    {
        DBGI(printf("List is full nBLR %d wPopCnt %zd.\n", nBLR, wPopCnt));
      #ifdef XX_LISTS
        if (nType == T_XX_LIST) {
            DBGI(printf("# nDL %d.\n", nDL));
            if (nDL_to_nBL(nDL) == nBL) {
                return InsertAtFullXxList(qya, wKey, wPopCnt, nPos,
                                          nBLUp, pLnUp
          #ifdef REMOTE_LNX
                                        , pwLnXUp
          #endif // REMOTE_LNX
                                          );
            } else {
                return InsertAtFullUnalignedXxList(qya, wKey, wPopCnt,
                                                   nPos,
                                                   nBLUp, pLnUp
          #ifdef REMOTE_LNX
                                                 , pwLnXUp
          #endif // REMOTE_LNX
                                                   );
            }
        }
      #endif // XX_LISTS
  #ifdef CODE_XX_SW
        // This is where we DoubleDown an upper narrow switch
        // if !USE_XX_SW_ONLY_AT_DL2 || (nBLR > cnBitsLeftAtDl2).
        // We don't send it to TransformList with a digit-unaligned list.
        if (nBL != nDL_to_nBL(nBL_to_nDL(nBL))) {
            Word_t* pwRootUp = &pLnUp->ln_wRoot; (void)pwRootUp;
            int nBLRUp = gnBLR(qyx(Up));
            int nBWUp = gnBW(qyx(Up), nBLRUp);
      #ifdef DOUBLE_DOWN
          #if cnSwCnts != 0
            // Make sure shared list expanse is small enough.
            int nBWMin = LOG(cnSwCnts * sizeof(Word_t) * 8
                                 / (2 << LOG(MAX(nBLRUp, 16) - 1)));
            // If nBWUp < nBWMin, then the resulting shared list expanse
            // would be bigger than a subexpanse count in the switch.
            if (nBWUp >= nBWMin)
          #endif // cnSwCnts != 0
            {
                int nBitCnt = SignificantBitCnt(qy, wKey, wPopCnt);
// Don't DoubleDown if we can't splay within the digit?
            DBGI(printf("IAL: ListIsFull DD nBL %d nBLUp %d nBitCnt %d\n",
                        nBL, nBLUp, nBitCnt));
                int nBWUpNew;
                if (cnBWIncrDD != 0) {
                    nBWUpNew = nDL_to_nBL(nDL) - nBL + cnBWIncrDD;
                    int nBWUpMax = nDL_to_nBL(nDL) - nDL_to_nBL(nDL - 1);
                    if (nBWUpNew > nBWUpMax) {
                        nBWUpNew = nBWUpMax;
                    }
                } else {
                    nBWUpNew = nBLRUp - nBitCnt + 1;
          #ifndef USE_XX_SW_ONLY_AT_DL2
                    assert(nBL_to_nDL(nBLRUp) == nDL);
                    int nBWUpMax = nBLRUp - nDL_to_nBL(nDL - 1);
                    if (nBWUpNew > nBWUpMax) {
// We need to enhance TransformList so we don't have to do a useless
// DoubleDown here.
                        nBWUpNew = nBWUpMax;
                    }
          #endif // !USE_XX_SW_ONLY_AT_DL2
                }
                return DoubleDown(qyax(Up), wKey,
                                  GetPopCnt(qyax(Up)),
                                  nBWUpNew
                                  );
            }
      #endif // DOUBLE_DOWN
            DBGI(printf("IAL: ListIsFull DoubleIt nBL %d nBLUp %d\n",
                        nBL, nBLUp));
            int nBWNew = nBWUp + cnBWIncr;
            Word_t wPopCntUp = GetPopCnt(qyax(Up));
            return DoubleIt(qyax(Up), wKey, wPopCntUp, nBWNew);
        }
  #endif // CODE_XX_SW
        {
            return TransformList(qya, wKey,
      #ifdef CODE_XX_SW
                                 nBLUp, pLnUp,
          #ifdef REMOTE_LNX
                                 pwLnXUp,
          #endif // REMOTE_LNX
      #endif // CODE_XX_SW
                                 wPopCnt);
        }
    }
  #ifdef B_JUDYL
    DBGI(printf("InsertAtList returning %p\n", (void*)pwValue));
    return pwValue;
  #else // B_JUDYL
    return Success;
  #endif // B_JUDYL else
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
InsertGuts(qpa, Word_t wKey, int nPos
  #if defined(CODE_XX_SW)
         , Link_t *pLnUp, int nBLUp
      #ifdef REMOTE_LNX
         , Word_t* pwLnXUp
      #endif // REMOTE_LNX
  #endif // defined(CODE_XX_SW)
           )
{
    qva;
#if defined(CODE_XX_SW)
    (void)nBLUp;
      #ifdef REMOTE_LNX
    (void)pwLnXUp;
      #endif // REMOTE_LNX
    int nBW; (void)nBW;
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    //assert((nBL >= cnBitsPerWord) == (pLnUp == NULL));
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
#endif // defined(CODE_XX_SW)
  #ifdef _LNX
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
    assert((pwLnX != NULL) || (nBL == cnBitsPerWord));
  #endif // _RETURN_NULL_TO_INSERT_AGAIN
  #endif // _LNX
    DBGI(printf("InsertGuts nBL %d pwRoot %p wKey " OWx" wRoot " OWx"\n",
                nBL, pwRoot, wKey, wRoot));
    DBGI(printf("IG nBLR %d\n",
                tp_bIsList(nType) ? gnListBLR(qy) : gnBLR(qy)));
    DBGI(printf("IG: nPos %d\n", nPos));
  #ifdef _LNX
    DBGI(printf("IG: pwLnX %p\n", pwLnX));
  #endif // _LNX
  #ifdef CODE_XX_SW
    DBGI(printf("IG: nBLUp %d pLnUp %p\n", nBLUp, pLnUp));
  #ifdef REMOTE_LNX
    DBGI(printf("IG: pwLnXUp %p\n", pwLnXUp));
  #endif // REMOTE_LNX
  #endif // CODE_XX_SW

  // One of the key aspects of USE_XX_SW_ONLY_AT_DL2 is that we go ahead and
  // widen a DL2 switch right on past DL1 and all the way to an embedded
  // bitmap so the DL2 switch with embedded bitmaps is equivalent to a DL2
  // bitmap.
  // Another key aspect is that we try to avoid external leaves, i.e. we
  // widen the switch when we have to insert a key and it can't be embedded
  // in the link. The name, USE_XX_SW_ONLY_AT_DL2, leaves a lot to be
  // desired.
  // USE_XX_SW_ONLY_AT_DL2 significantly impacts our behavior at DL2 and
  // below.
  #ifndef USE_XX_SW_ONLY_AT_DL2
    assert(nBL >= cnBitsInD1);
  #endif // #ifndef USE_XX_SW_ONLY_AT_DL2

    // Handle cases where wRoot has no type field before looking at
    // the type field in wRoot.

  #ifdef NO_TYPE_IN_XX_SW
    // This code makes it look like NO_TYPE_IN_XX_SW applies to all T_XX_SW
    // nodes, but I wonder if it was written when we only used T_XX_SW at
    // DL2 and below.
    // We may someday handle external leaves even with NO_TYPE_IN_XX_SW
    // using some magic, but its not coded yet.
    // So, for now, if NO_TYPE_IN_XX_SW applies to this link, then it
    // contains embedded keys.
// Shouldn't we check for embedded bitmap before this?
// We use pLnUp != NULL to indicate XX_SW?
// I can't remember why we are testing pLnUp here.
// Is it supposed to signal something for NO_TYPE_IN_XX_SW?
    if (/*(pLnUp != NULL) &&*/ (nBL <= nDL_to_nBL(2))) {
        DBGR(printf("IG: goto embeddedKeys.\n"));
        goto embeddedKeys; // no type field is handled by embeddedKeys
    }
  #endif // NO_TYPE_IN_XX_SW

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

    // Call InsertAtBitmap to do the insert if we have an external bitmap
    // leaf. We don't want to call InsertAtBitmap for an empty link.
    // Inserting into an empty link is handled elsewhere.
    // The 2nd half of this test is intended to make the whole test
    // go away at compile time if appropriate.
    if ((wRoot != WROOT_NULL) || (wr_nType(WROOT_NULL) != T_BITMAP)) {
        if (tp_bIsBitmap(nType)) {
            return InsertAtBitmap(qya, wKey);
        }
    }
  #endif // BITMAP

    // Widen an XX_SW if the population warrants it.
    // I think the idea was to do some work now in order to avoid some
    // later in an effort to control the worst-case insert time.
    // I fear the idea was ill-conceived.
    // I think we need a better way of deciding when to widen.
  #ifdef USE_XX_SW
  #ifndef DOUBLE_DOWN
    if ((nBL != nDL_to_nBL(nBL_to_nDL(nBL))) && tp_bIsList(nType)) {
        // What about when nBL <= nDL_to_nBL(2)?
        // And USE_XX_SW_ONLY_AT_DL2?
        Word_t *pwRootUp = &pLnUp->ln_wRoot;
        // How are we supposed to determine nBW for
        // pLnUp without having nBLUp?
        Word_t wPopCntUp = GetPopCnt(qyax(Up));
        //int nBLRUp = GetBLR(pwRootUp, nBLUp);
        int nBWUp = Get_nBW(pwRootUp);
        if (wPopCntUp >= EXP(nBWUp + cnBWIncr) * 2) {
// Can DoubleIt handle a deflated list?
            Word_t* pwRootUp = &pLnUp->ln_wRoot;
            int nBWUp = gnBW(qyx(Up), gnBLR(qyx(Up)));
            int nBWNew = nBWUp + cnBWIncr;
            return DoubleIt(qyax(Up), wKey, wPopCntUp, nBWNew);
        }
    }
  #endif // !DOUBLE_DOWN
  #endif // USE_XX_SW

    // Should the following be moved into the if ! switch block?
  #if (cwListPopCntMax != 0)
  #if defined(EMBED_KEYS)
    int nPopCnt;
    // Revisit:
    // Not sure we want to enhance wr_nPopCnt to be able to handle
    // WROOT_NULL in all cases. It might be too costly for lookup.
    if (wRoot == WROOT_NULL) {
        nPopCnt = 0;
        goto wRootNull; // handle with embedded keys
    }
    if (tp_bIsEk(nType)) {
        goto embeddedKeys;
embeddedKeys:;
        // If the key will fit in an embedded list then use InsertEmbedded
        // to do the insert and avoid the overhead of inlating the list
        // and going through the heavier weight insert process.
        assert(wRoot != WROOT_NULL);
        nPopCnt = wr_nPopCnt(wRoot, nBL);
        goto wRootNull;
wRootNull:;
        // This is a performance shortcut that is not necessary.
        if (nPopCnt < EmbeddedListPopCntMax(nBL)) {
            BJL(return) InsertEmbedded(qya, wKey);
            BJ1(return Success);
        }
        // Change an embedded list into an external list to make things
        // easier for Insert.  We'll change it back later if it makes sense.
        if (nPopCnt != 0) {
            wRoot = InflateEmbeddedList(qya, wKey);
            nPos = ~SearchList(qya, /*nBLR*/ nBL, wKey);
        } else {
            nPos = 0;
        }

        // InflateEmbeddedList installs wRoot. It also initializes the
        // other words in the link if there are any.

        nType = wr_nType(wRoot);
        assert(tp_bIsList(nType));
        DBGI(printf("IG: wRoot " OWx" nType %d PWR_xListPopCnt %d\n",
                    wRoot, nType,
                    wRoot == WROOT_NULL ? 0
                        : (int)PWR_xListPopCnt(pwRoot, wr_pwr(*pwRoot), nBL)));
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
    // Why are we calling InsertAtList for WROOT_NULL if cwListPopCntMax == 0?
    // Because InsertAtList is where we handle inserting into an empty link
    // even if cwListPopCntMax == 0?
    if (wRoot == WROOT_NULL)
      #endif // (cwListPopCntMax == 0)
  #endif // defined(BM_SW_FOR_REAL)
#endif // defined(SKIP_LINKS)
    {
        return InsertAtList(qya, wKey, nPos
#ifdef CODE_XX_SW
                          , pLnUp, nBLUp
      #ifdef REMOTE_LNX
                          , pwLnXUp
      #endif // REMOTE_LNX
#endif // CODE_XX_SW
                            );
    }
#if defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)
    else
    {
        assert(*pwRoot != WROOT_NULL);
  #if defined(EMBED_KEYS)
        assert(wr_nType(*pwRoot) != T_EMBEDDED_KEYS);
  #endif // defined(EMBED_KEYS)
        int nBLR = gnBLR(qy);
  #if defined(BM_SW_FOR_REAL)
      #if defined(SKIP_LINKS)
        Word_t wPrefix;
        // Test to see if this is a missing link case.
        // If not, then it is a prefix mismatch case.
        // If nBLR == nBL there is no skip hence no prefix mismatch hence it
        // must be a missing link case.
        // Unfortunately, nBLR != nBL does not imply a prefix mismatch.
        // It's a bit of a bummer that we are doing the prefix check again.
        // But at least we only do it if there is a skip.
        if ((nBLR == nBL)
                || ((wPrefix =
          #ifdef PP_IN_LINK
                         (nBL == cnBitsPerWord) ? 0 :
          #endif // PP_IN_LINK
                             PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR))
                    == w_wPrefixNotAtTopBL(wKey, nBLR))
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
            DBGI(printf("wPrefix " OWx" w_wPrefix " OWx" nBLR %d\n",
                        PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR),
                        w_wPrefixBL(wKey, nBLR), nBLR));
#endif // defined(SKIP_LINKS)
            assert(nDL_to_nBL(nBL_to_nDL(nBLR)) == nBLR);
            NewLink(qya, wKey, nBL_to_nDL(nBLR), /* nDLUp */ nBL_to_nDL(nBL));
  #ifdef _RETURN_NULL_TO_INSERT_AGAIN
            return 0; // call InsertGuts again
  #else // _RETURN_NULL_TO_INSERT_AGAIN
            return Insert(qya, wKey);
  #endif // _RETURN_NULL_TO_INSERT_AGAIN else
        }
#endif // defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
        else
#endif // defined(SKIP_LINKS) && defined(BM_SW_FOR_REAL)
#if defined(SKIP_LINKS)
        {
            return InsertAtPrefixMismatch(qya, wKey, nBLR);
        }
#endif // defined(SKIP_LINKS)
    }
#endif // defined(SKIP_LINKS) || defined(BM_SW_FOR_REAL)
}

#if (cwListPopCntMax != 0)
#if defined(EMBED_KEYS)

#ifdef EK_XV // implies COMPRESSED_LISTS
static Word_t
InflateList(qpa, Word_t wKey, int nPopCnt)
{
    qva;
    Word_t wRootNew = 0;
    Word_t *pwList = NewList(nPopCnt, nBL);
  #ifdef UA_PARALLEL_128
    if ((nBL == 16) && (nPopCnt <= 6)) {
        set_wr(wRootNew, pwList, T_LIST_UA);
    } else
  #endif // UA_PARALLEL_128
    set_wr(wRootNew, pwList, T_LIST);

    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
    Word_t wBLM = MSK(nBL); // Bits left mask.

    for (int nn = 0; nn < nPopCnt; nn++) {
        if (nBL <= 8) {
            uint8_t* pcKeys = ls_pcKeysNATX(pwList, nPopCnt);
            pcKeys[nn] = (uint8_t)(wKey & ~wBLM) | ((uint8_t*)pwLnX)[nn];
        } else
        if (nBL <= 16) {
            uint16_t* psKeys = ls_psKeysNATX(pwList, nPopCnt);
            psKeys[nn] = (uint16_t)(wKey & ~wBLM) | ((uint16_t*)pwLnX)[nn];
        } else
#if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            uint32_t* piKeys = ls_piKeysNATX(pwList, nPopCnt);
            piKeys[nn] = (uint32_t)(wKey & ~wBLM) | ((uint32_t*)pwLnX)[nn];
        } else
#endif // (cnBitsPerWord > 32)
        {
            assert(nPopCnt == 1);
            Word_t* pwKeys = ls_pwKeysNATX(pwList, nPopCnt);
            pwKeys[nn] = (wKey & ~wBLM) | pwLnX[nn];
        }
        // Copy the values.
        // BUG: Fix this to use gpwValues
        // gpwValues(qy)[~nn] = pwLnX[nn];
        pwList[~(nn
  #ifdef LIST_POP_IN_PREAMBLE
                 + 1
  #endif // LIST_POP_IN_PREAMBLE
                 )] = wr_pwr(pLn->ln_wRoot)[nn];
    }

    MyFree(wr_pwr(pLn->ln_wRoot), MAX(3, nPopCnt | 1), &j__AllocWordsJV);

    // Pad the bucket. Discovered this bug when working with LocateLtKey128.
  #ifdef COMPRESSED_LISTS
    if (nBL <= 8) {
        PAD(ls_pcKeys(pwList, nPopCnt), nPopCnt);
    } else
    if (nBL <= 16) {
        PAD(ls_psKeys(pwList, nPopCnt), nPopCnt);
    } else
  #if (cnBitsPerWord > 32)
    if (nBL <= 32) {
        PAD(ls_piKeys(pwList, nPopCnt), nPopCnt);
    } else
  #endif // (cnBitsPerWord > 32)
  #endif // COMPRESSED_LISTS
    {
        PAD(ls_pwKeysNATX(pwList, nPopCnt), nPopCnt);
    }

    pLn->ln_wRoot = wRootNew;
    snListBLR(qy, nBL);
    Set_xListPopCnt(&pLn->ln_wRoot, nBL, nPopCnt);
    if (nBL == 16) {
        // BUG: there is probably a faster way to do this.
        UpdateDist(qya, nPopCnt);
    }

    return wRootNew; // wRootNew is installed
}
#endif // EK_XV

// Replace a wRoot that has embedded keys with an external T_LIST leaf.
// It assumes the input is an embedded list.
Word_t
InflateEmbeddedList(qpa, Word_t wKey)
{
    qva;
    DBGI(printf(
         "InflateEmbeddedList pwRoot %p wKey " OWx" nBL %d wRoot " OWx"\n",
         (void*)pwRoot, wKey, nBL, wRoot));
#if defined(NO_TYPE_IN_XX_SW)
    if (nBL < nDL_to_nBL(2)) {
        if (wRoot == ZERO_POP_MAGIC) { return 0; }
  #if defined(HANDLE_BLOWOUTS)
        if ((wRoot & BLOWOUT_MASK(nBL)) == ZERO_POP_MAGIC) { return 0; }
  #endif // defined(HANDLE_BLOWOUTS)
    }
#else // defined(NO_TYPE_IN_XX_SW)
    assert(tp_bIsEk(wr_nType(wRoot)));
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
  #ifdef EK_XV
    if (nPopCnt > 1) {
        return InflateList(qya, wKey, nPopCnt);
    }
  #endif // EK_XV

    // Why not allocate a big enough list to hold the new key that we're about
    // to insert? Maybe we already do for most cases but why not make sure by
    // adding one to the nPopCnt argument here? Would it make some things more
    // complicated later?
    Word_t *pwList = NewList(nPopCnt, nBL);
    Word_t wRootNew = 0;
  #ifdef UA_PARALLEL_128
    if ((nBL == 16) && (nPopCnt <= 6)) {
        set_wr(wRootNew, pwList, T_LIST_UA);
    } else
  #endif // UA_PARALLEL_128
    { set_wr(wRootNew, pwList, T_LIST); }

    Word_t wBLM = MSK(nBL); // Bits left mask.

    for (int nPos = 0; nPos < nPopCnt; nPos++) {
  #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
        int nSlot = (nPopCntMax - nPos);
  #else // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
        int nSlot = (nPos + 1);
  #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS else
  #ifdef COMPRESSED_LISTS
        if (nBL <= 8) {
            pcKeys = ls_pcKeysNATX(pwList, nPopCnt);
            pcKeys[nPos] = (uint8_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
            DBGR(printf("nPos %d nSlot %d pcKeys[?] 0x%x\n",
                        nPos, nSlot, pcKeys[nPos]));
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNATX(pwList, nPopCnt);
            psKeys[nPos] = (uint16_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
        } else
      #if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNATX(pwList, nPopCnt);
            piKeys[nPos] = (uint32_t)((wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM));
            if (nBL == 24) {
                /*printf("pwList %p piKeys %p piKeys[%d] %x\n",
                       (void *)pwList, (void *)piKeys, nPos, piKeys[nPos]);*/
            }
        } else
      #endif // (cnBitsPerWord > 32)
  #endif // COMPRESSED_LISTS
        {
#if defined(COMPRESSED_LISTS)
            assert(nPopCnt == 1);
#endif // defined(COMPRESSED_LISTS)
            pwKeys = ls_pwKeysNATX(pwList, nPopCnt);
            pwKeys[nPos] = (wKey & ~wBLM)
                       | ((wRoot >> (cnBitsPerWord - (nSlot * nBL))) & wBLM);
        }
#ifdef B_JUDYL
        // Copy the value.
        // BUG: Fix this to use gpwValues
        pwList[~(nPos
  #ifdef LIST_POP_IN_PREAMBLE
                 + 1
  #endif // LIST_POP_IN_PREAMBLE
                 )] = *pwLnX; // gpwValues(qy)[~nPos] = *pwLnX;
#endif // B_JUDYL
    }

    // Pad the bucket. Discovered this bug when working with LocateLtKey128.
  #ifdef COMPRESSED_LISTS
    if (nBL <= 8) {
        PAD(ls_pcKeys(pwList, nPopCnt), nPopCnt);
    } else
    if (nBL <= 16) {
        PAD(ls_psKeys(pwList, nPopCnt), nPopCnt);
    } else
  #if (cnBitsPerWord > 32)
    if (nBL <= 32) {
        PAD(ls_piKeys(pwList, nPopCnt), nPopCnt);
    } else
  #endif // (cnBitsPerWord > 32)
  #endif // COMPRESSED_LISTS
    {
        PAD(ls_pwKeysNATX(pwList, nPopCnt), nPopCnt);
    }

    *pwRoot = wRoot = wRootNew;
    snListBLR(qy, nBL);
    Set_xListPopCnt(pwRoot, nBL, nPopCnt);

    return *pwRoot; // wRootNew is installed
}

#ifdef EK_XV // implies COMPRESSED_LISTS
// Replace list leaf with embedded keys and a value area leaf.
// The function assumes it is possible.
static Word_t
DeflateList(qpa, int nPopCnt)
{
    qva;
  #ifdef DEBUG_INSERT
    DBGI(printf("# DeflateList nBL %d pwRoot %p wRoot 0x%zx nPopCnt %d: ",
                nBL, pwRoot, wRoot, nPopCnt));
    DumpX(qya, /*wKey*/ 0);
  #endif // DEBUG_INSERT
    assert(tp_bIsList(nType));
    assert(nPopCnt > 1);
    assert(nPopCnt <= EmbeddedListPopCntMax(nBL));

    Word_t wRootNew = 0;
    set_wr_nType(wRootNew, T_EK_XV); // embedded keys, external values
    set_wr_nPopCnt(wRootNew, nBL, nPopCnt);
    assert(wRootNew != WROOT_NULL);

    // Copy the keys.
    Word_t wLnXNew;
    if (nBL <= 8) {
        memcpy(&wLnXNew, ls_pcKeysNATX(pwr, nPopCnt), sizeof(Word_t));
        // What about PAD64? Do we know that any list being deflated has
        // already been padded?
  #ifdef PSPLIT_PARALLEL
        assert(ls_pcKeysNATX(pwr, nPopCnt)[7]
            == ls_pcKeysNATX(pwr, nPopCnt)[nPopCnt-1]);
  #endif // PSPLIT_PARALLEL
    } else
  #if (cnBitsPerWord > 32)
    if (nBL > 16) {
        assert(nBL <= 32);
        memcpy(&wLnXNew, ls_piKeysNATX(pwr, nPopCnt), sizeof(Word_t));
        assert(ls_piKeysNATX(pwr, nPopCnt)[1]
            == ls_piKeysNATX(pwr, nPopCnt)[nPopCnt-1]);
    } else
  #endif // (cnBitsPerWord > 32)
    {
        assert(nBL <= 16);
        memcpy(&wLnXNew, ls_psKeysNATX(pwr, nPopCnt), sizeof(Word_t));
  #ifdef PSPLIT_PARALLEL
        assert(ls_psKeysNATX(pwr, nPopCnt)[3]
            == ls_psKeysNATX(pwr, nPopCnt)[nPopCnt-1]);
  #endif // PSPLIT_PARALLEL
    }

    // Create the value area and copy the values.
    Word_t *pwrNew = (Word_t*)MyMallocGuts(nPopCnt | 1,
  #ifdef ALIGN_EK_XV
                                           (nPopCnt <= 3) ? 5 : 6,
  #else // ALIGN_EK_XV
                                           cnBitsMallocMask,
  #endif // #else ALIGN_EK_XV
                                           &j__AllocWordsJV);
    for (int nPos = 0; nPos < nPopCnt; nPos++) {
        pwrNew[nPos] = gpwValues(qy)[~nPos];
    }
    set_wr_pwr(wRootNew, pwrNew);

    OldList(pwr, nPopCnt, nBL, wr_nType(*pwRoot));

    *pwRoot = wRootNew; // install the new list
  #ifdef _LNX
    *pwLnX = wLnXNew;
  #endif // _LNX

    DBGI(printf("DeflateList wRootNew 0x%zx wLnXNew 0x%zx\n",
                wRootNew, wLnXNew));

    return wRootNew;
}
#endif // EK_XV

// Replace an external T_LIST leaf with a wRoot with embedded keys.
// The function assumes it is possible.
static Word_t
DeflateExternalList(qpa, int nPopCnt)
{
    qva;
    DBGI(printf("DeflateExternalList pwRoot %p nPopCnt %d nBL %d pwr %p\n",
               (void *)pwRoot, nPopCnt, nBL, (void *)pwr));
  #ifdef EK_XV
    if (nPopCnt > 1) {
        return DeflateList(qya, nPopCnt);
    }
  #endif // EK_XV
  #if defined(REVERSE_SORT_EMBEDDED_KEYS) && defined(EK_CALC_POP)
    assert(0); // not yet
  #endif // REVERSE_SORT_EMBEDDED_KEYS && EK_CALC_POP
    int nPopCntMax = EmbeddedListPopCntMax(nBL); (void)nPopCntMax;
    assert(nPopCnt <= nPopCntMax);

    assert( (wr_nType(*pwRoot) == T_LIST)
#if defined(UA_PARALLEL_128)
           || (wr_nType(*pwRoot) == T_LIST_UA)
#endif // defined(UA_PARALLEL_128)
           );

#if defined(DEBUG_INSERT)
    //HexDump("External List", pwr, nPopCnt + 1);
    DumpX(qya, /*wKey*/ 0);
#endif // defined(DEBUG_INSERT)

  #ifdef FILL_WITH_ONES
    wRoot = -(Word_t)-1;
  #else // FILL_WITH_ONES
    wRoot = 0; // Fill with zeros if nothing else is specified?
  #endif // FILL_WITH_ONES

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

    for (int nPos = 0;
  #ifdef FILL_W_KEY
      #ifdef B_JUDYL
             // EmbeddedListPopCntMax is for EK_XV?
             nPos < (cnBitsPerWord - 4) / nBL;
      #else // B_JUDYL
             nPos < nPopCntMax;
      #endif // B_JUDYL else
  #else // FILL_W_KEY
             nPos < nPopCnt;
  #endif // FILL_W_KEY else
             nPos++)
    {
        // Slots are numbered from msb to lsb starting at 1.
        // Ugly but helps avoid a lot of ifdefs?
  #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
        int nSlot = (nPopCntMax - nPos);
  #else // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
        int nSlot = (nPos + 1);
  #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS else
  #if defined(COMPRESSED_LISTS)
        if (nBL <= 8) {
            pcKeys = ls_pcKeysNATX(pwr, nPopCnt);
            // Uh oh.  Are we really padding with the smallest key?
            // Isn't this contrary to some assumptions in the code?
            // Hmm.  I think we changed it to the smallest key so
            // we could calculate offset of found key using magic.
            SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                    pcKeys[(nPos < nPopCnt) ? nPos :
      #ifdef FILL_W_BIG_KEY
                               nPopCnt - 1
      #else // FILL_W_BIG_KEY
                               0
      #endif // FILL_W_BIG_KEY else
                           ]);
            DBGR(printf("nPos %d nSlot %d pcKeys[?] 0x%x\n", nPos, nSlot,
                    pcKeys[(nPos < nPopCnt) ? nPos : 0]));
        } else
        if (nBL <= 16) {
            psKeys = ls_psKeysNATX(pwr, nPopCnt);
            DBGI(printf("nPos %d nSlot %d psKeys[?] 0x%x\n", nPos, nSlot,
                    psKeys[(nPos < nPopCnt) ? nPos : 0]));
            SetBits(&wRoot, nBL, /* lsb */ cnBitsPerWord - (nSlot * nBL),
                    psKeys[(nPos < nPopCnt) ? nPos :
      #ifdef FILL_W_BIG_KEY
                               nPopCnt - 1
      #else // FILL_W_BIG_KEY
                               0
      #endif // FILL_W_BIG_KEY else
                           ]);
        } else
      #if (cnBitsPerWord > 32)
        if (nBL <= 32) {
            piKeys = ls_piKeysNATX(pwr, nPopCnt);
            SetBits(&wRoot, nBL, cnBitsPerWord - (nSlot * nBL),
                    piKeys[(nPos < nPopCnt) ? nPos :
          #ifdef FILL_W_BIG_KEY
                               nPopCnt - 1
          #else // FILL_W_BIG_KEY
                               0
          #endif // FILL_W_BIG_KEY else
                           ]);
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
                    pwKeys[(nPos < nPopCnt) ? nPos :
  #ifdef FILL_W_BIG_KEY
                               nPopCnt - 1
  #else // FILL_W_BIG_KEY
                               0
  #endif // FILL_W_BIG_KEY else
                           ]);
        }
  #ifdef B_JUDYL
        // Copy the value.
        DBGI(printf("DEL: pwLnX %p\n", pwLnX));
      #ifdef FILL_W_KEY
        if (nPos < nPopCnt)
      #endif // FILL_W_KEY
            *pwLnX = pwr[~(nPos
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
InsertAtBitmap(qpa, Word_t wKey)
{
    qva;
    assert(tp_bIsBitmap(nType));
    int nBLR = nBL;
  #ifdef SKIP_TO_BITMAP
    if (nType == T_SKIP_TO_BITMAP) {
        nBLR = gnBLR(qy);

        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL == cnBitsPerWord)
                ? 0 : PWR_wPrefixBL(pwRoot, (Switch_t*)pwr, nBLR);
      #else // PP_IN_LINK
            gwBitmapPrefix(qy, nBLR);
      #endif // else PP_IN_LINK

        int bPrefixMismatch =
      #ifdef PP_IN_LINK
            (nBL == cnBitsPerWord) ? (wKey >= EXP(nBLR)) :
      #endif // PP_IN_LINK
                ((int)LOG(1 | (wPrefix ^ wKey)) >= nBLR);

        if (bPrefixMismatch) {
            return InsertAtPrefixMismatch(qya, wKey, nBLR);
        }
    }
  #endif // SKIP_TO_BITMAP

    DBGI(printf("IAB SetBit(pwr " OWx" wKey " OWx") pwRoot %p\n",
                (Word_t)pwr, wKey & MSK(nBLR), (void *)pwRoot));

    // Mask to convert EXP(nBLR) back to 0 for newly created bitmap.
    // Is mask necessary if pop count has its own word?
  #if defined(SKIP_TO_BITMAP) && !defined(PREFIX_WORD_IN_BITMAP_LEAF)
    Word_t wPopCnt = gwBitmapPopCnt(qya, nBLR) & MSK(nBLR);
  #else // defined(SKIP_TO_BITMAP) && !defined(PREFIX_WORD_IN_BITMAP_LEAF)
    Word_t wPopCnt = gwBitmapPopCnt(qya, nBLR);
      #if defined(PP_IN_LINK)
    wPopCnt &= MSK(nBLR);
      #endif // defined(PP_IN_LINK)
  #endif // #else defined(SKIP_TO_BITMAP) && !def(PREFIX_WORD_IN_BITMAP_LEAF)
    assert(wPopCnt < EXP(nBLR));
  #ifdef _BMLF_BM_IN_LNX
    Word_t* pwBitmap = pwLnX;
  #else // _BMLF_BM_IN_LNX
    Word_t* pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
  #endif // else _BMLF_BM_IN_LNX
    assert(!BitIsSet(pwBitmap, wKey & MSK(nBLR)));
  #ifdef B_JUDYL
    Word_t *pwSrcVals = gpwBitmapValues(qy, nBLR);
    int nPos;
    if (BM_UNPACKED(wRoot)) {
      #ifdef BMLF_INTERLEAVE
        (void)pwSrcVals;
        (void)nPos;
          #ifdef PACK_BM_VALUES
        nPos = 0; // make compiler happy
          #endif // PACK_BM_VALUES
      #else // BMLF_INTERLEAVE
        nPos = wKey & MSK(nBLR);
      #endif // BMLF_INTERLEAVE else
        goto done;
    }
      #ifdef PACK_BM_VALUES
    nPos = BmIndex(qya, nBLR, wKey);
    Word_t wWords = BitmapWordCnt(nBLR, wPopCnt + 1); // new
    if (wWords != BitmapWordCnt(nBLR, wPopCnt)) {
        // NewBitmap will zero the bits.
      #ifdef _BMLF_BM_IN_LNX
        Word_t wBitmapOld = *pwBitmap;
      #endif // _BMLF_BM_IN_LNX
        NewBitmap(qya, nBLR, wKey, wPopCnt + 1);
        // Prefix and popcnt are set; bits are not.
        // *pwRoot has been updated. qy is out of date.
        // Copy bits and update qy.
        wRoot = *pwRoot;
        Word_t *pwrOld = pwr;
        pwr = wr_pwr(wRoot);
        int nBmWords
            = (nBLR <= cnLogBitsPerWord) ? 1 : EXP(nBLR - cnLogBitsPerWord);
      #ifdef _BMLF_BM_IN_LNX
        Word_t* pwBitmapOld = &wBitmapOld;
        pwBitmap = pwLnX;
      #else // _BMLF_BM_IN_LNX
        Word_t* pwBitmapOld = pwBitmap;
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
      #endif // else _BMLF_BM_IN_LNX
        COPY(pwBitmap, pwBitmapOld, nBmWords);
      #ifdef BMLF_CNTS
        if (BM_UNPACKED(wRoot)) { } else
        {
          #if defined(BMLF_POP_COUNT_8) || defined(BMLF_POP_COUNT_1)
        COPY(((BmLeaf_t*)pwr)->bmlf_au8Cnts,
             ((BmLeaf_t*)pwrOld)->bmlf_au8Cnts,
             sizeof((BmLeaf_t*)pwr)->bmlf_au8Cnts);
          #else // BMLF_POP_COUNT_8 || BMLF_POP_COUNT_1
              #ifndef BMLF_CNTS_IN_LNX
        *(Word_t*)(((BmLeaf_t*)pwr)->bmlf_au8Cnts)
            = *(Word_t*)((BmLeaf_t*)pwrOld)->bmlf_au8Cnts;
              #endif // !BMLF_CNTS_IN_LNX
          #endif // BMLF_POP_COUNT_8 || BMLF_POP_COUNT_1 else
        }
      #endif // BMLF_CNTS
        Word_t *pwTgtVals = gpwBitmapValues(qy, nBLR);
        if (BM_UNPACKED(wRoot)) {
            for (int k = 0; k < (int)EXP(nBLR); ++k) {
                if (BitIsSet(pwBitmap, k)) {
                    pwTgtVals[k] = *pwSrcVals++;
                }
            }
            nPos = wKey & MSK(nBLR);
        } else {
            COPY(&pwTgtVals[nPos + 1], &pwSrcVals[nPos], wPopCnt - nPos);
            COPY(pwTgtVals, pwSrcVals, nPos);
        }
        OldBitmap(pwrOld, nBLR, wPopCnt);
        pwSrcVals = pwTgtVals;
    } else
      #endif // PACK_BM_VALUES
  #endif // B_JUDYL
    {
#ifdef B_JUDYL
      #ifdef PACK_BM_VALUES
        MOVE(&pwSrcVals[nPos + 1], &pwSrcVals[nPos], wPopCnt - nPos);
      #endif // PACK_BM_VALUES
        goto done;
done:
#endif // B_JUDYL
        swBitmapPopCnt(qya, nBLR, wPopCnt + 1);
    }
  #ifdef BMLF_CNTS
    if (BM_UNPACKED(wRoot)) { }
      #ifdef PACK_BM_VALUES
    else
    {
      #ifdef BMLF_POP_COUNT_32
    int nBm = (wKey >> 5) & MSK(cnBitsInD1 - 5);
          #ifdef BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = (uint8_t*)pwLnX;
          #else // BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
          #endif // BMLF_CNTS_IN_LNX else
          #ifdef BMLF_CNTS_CUM
    *(Word_t*)pu8Cnts += 0x0101010101010100 & ~NZ_MSK((nBm + 1) * 8);
          #else // BMLF_CNTS_CUM
    ++pu8Cnts[nBm];
          #endif // BMLF_CNTS_CUM else
      #else // BMLF_POP_COUNT_32
          #ifdef BMLF_POP_COUNT_8
    int nBmByte
        = (wKey >> cnLogBitsPerByte) & MSK(cnBitsInD1 - cnLogBitsPerByte);
    for (int nn = nBmByte + 1; nn < (1 << (cnBitsInD1-cnLogBitsPerByte));
         ++nn)
    {
        ++((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn];
    }
          #elif defined(BMLF_POP_COUNT_1) // BMLF_POP_COUNT_8
    int nBmByte = wKey & MSK(cnBitsInD1);
    for (int nn = nBmByte + 1; nn < (1 << cnBitsInD1); ++nn) {
        ++((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn];
    }
          #else // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1
    int nBmWord
        = (wKey >> cnLogBitsPerWord) & MSK(cnBitsInD1 - cnLogBitsPerWord);
              #ifdef BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = (uint8_t*)pwLnX;
              #else // BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
              #endif // BMLF_CNTS_IN_LNX else
              #ifdef BMLF_CNTS_CUM
    for (int nn = nBmWord + 1; nn < (1 << (cnBitsInD1 - cnLogBitsPerWord));
         ++nn)
    {
        ++pu8Cnts[nn];
    }
                  #ifdef BM_DSPLIT
    ++pu8Cnts[4+nBmWord];
                  #endif // BM_DSPLIT
              #else // BMLF_CNTS_CUM
    ++pu8Cnts[nBmWord];
              #endif // BMLF_CNTS_CUM else
          #endif // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1 else
      #endif // BMLF_POP_COUNT_32 else
    }
      #endif // PACK_BM_VALUES
  #endif // BMLF_CNTS
    SetBit(pwBitmap, wKey & MSK(nBLR));
  #if cn2dBmMaxWpkPercent != 0
    if (nBLR == cnBitsLeftAtDl2) {
      #if cnWordsBm2Cnts != 0
        ++gpxBitmapCnts(qya, nBLR)[(wKey & MSK(nBLR)) >> cnLogBmlfBitsPerCnt];
      #endif // cnWordsBm2Cnts != 0
    }
  #endif // cn2dBmMaxWpkPercent != 0
  #ifdef BMLF_INTERLEAVE
    int nBmPartBmWords;
    Word_t wKeyLeft;
    Word_t* pwBmPart;
      #ifdef PACK_BM_VALUES
    if (!BM_UNPACKED(wRoot)) {
        nBmPartBmWords = 0; // make compiler happy
        wKeyLeft = 0; // make compiler happy
        pwBmPart = NULL; // make compiler happy
    } else
      #endif // PACK_BM_VALUES
    {
        Word_t wWordsHdr = sizeof(BmLeaf_t) / sizeof(Word_t);
        wWordsHdr += EXP(MAX(1, cnBitsInD1 - cnLogBitsPerWord));
        Word_t wDigit = wKey & MSK(cnBitsInD1);
        int nLogBmPartBmBits = cnBitsInD1 - cnLogBmlfParts;
        nBmPartBmWords = (nLogBmPartBmBits < cnLogBitsPerWord)
            ? 1 : EXP(nLogBmPartBmBits - cnLogBitsPerWord);
        int nBmPartSz = nBmPartBmWords + EXP(cnBitsInD1 - cnLogBmlfParts);
        int nBmPartNum = wDigit >> (cnBitsInD1 - cnLogBmlfParts);
        wKeyLeft = wDigit & MSK(cnBitsInD1 - cnLogBmlfParts);
        pwBmPart = &pwr[wWordsHdr] + nBmPartSz * nBmPartNum;
        int nBmBitPartNum = nBmPartNum;
      #ifdef BMLFI_SPLIT_BM
        nBmBitPartNum += !!(wDigit & EXP(cnBitsInD1 - cnLogBmlfParts - 1));
      #endif // BMLFI_SPLIT_BM
        Word_t* pwBmBitPart = &pwr[wWordsHdr] + nBmPartSz * nBmBitPartNum;
        SetBit(pwBmBitPart
      #ifdef BMLFI_BM_AT_END
               + nBmPartSz - nBmPartBmWords
      #endif // BMLFI_BM_AT_END
             , wKeyLeft);

    }
  #endif // BMLF_INTERLEAVE

#if defined(PP_IN_LINK)

    // Shouldn't we do this when we create the switch with the link
    // that points to this bitmap rather than on every insert into
    // the bitmap?

    // What about no_unnecessary_prefix?
    set_PWR_wPrefixBL(pwRoot, NULL, nBLR, wKey);

#endif // defined(PP_IN_LINK)

    CheckBitmapGuardband(pwr, nBLR, wPopCnt + 1);
    // Insert is responsible for zeroing the value.
      #ifdef BMLF_INTERLEAVE
    if (BM_UNPACKED(wRoot)) {
        return &pwBmPart[wKeyLeft
          #ifndef BMLFI_BM_AT_END
                         + nBmPartBmWords
          #endif // !BMLFI_BM_AT_END
                         ];
    }
      #endif // BMLF_INTERLEAVE
#ifdef B_JUDYL
      #if !defined(BMLF_INTERLEAVE) || defined(PACK_BM_VALUES)
    return &pwSrcVals[nPos];
      #endif // !defined(BMLF_INTERLEAVE) || defined(PACK_BM_VALUES)
#else // B_JUDYL
    return Success;
#endif // B_JUDYL
}

#endif // BITMAP

static Status_t
RemoveAtBitmap(qpa, Word_t wKey);

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

    {
        assert(tp_bIsSwitch(nType));
        Word_t wPopCnt =
#if defined(CODE_BM_SW)
            tp_bIsBmSw(nType) ? PWR_wPopCnt(pwRoot, (BmSwitch_t *)pwr, nDLR) :
#endif // defined(CODE_BM_SW)
                                PWR_wPopCnt(pwRoot, (  Switch_t *)pwr, nDLR);

        if (wPopCnt == 0) {
            FreeArrayGuts(qy,
  #ifdef REMOTE_LNX
                          /* pwLnX */ NULL,
  #endif // REMOTE_LNX
                          wKey,
                          /* bDump */ 0
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
RemoveGuts(qpa, Word_t wKey)
{
    qva;
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
        || tp_bIsBitmap(nType))
  #else // (cwListPopCntMax != 0)
    assert((cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink))
        || tp_bIsBitmap(nType));
  #endif // (cwListPopCntMax != 0)
    {
        return RemoveAtBitmap(qya, wKey);
    }
#endif // BITMAP

#if (cwListPopCntMax != 0)

    Word_t wPopCnt;

#if defined(EMBED_KEYS)
    if (tp_bIsEk(nType))
    {
// Why is nBL_to_nBitsPopCntSz irrelevant here?
        assert(nBL <= cnBitsPerWord - cnBitsMallocMask);
        goto embeddedKeys;
embeddedKeys:;
        wPopCnt = wr_nPopCnt(*pwRoot, nBL);
        wRoot = InflateEmbeddedList(qya, wKey);
        // Is there any reason to preserve *pwRoot?
        // Is it a problem to have an external list that could
        // be embedded?
        nType = wr_nType(wRoot);
        pwr = wr_pwr(wRoot);
    }
#endif // defined(EMBED_KEYS)

    assert(tp_bIsList(nType));

    int nBLR = gnListBLR(qy);
//printf("\n# nBLR %d nType %d\n", nBLR, nType);
    wPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);

    // wPopCnt is the count before the remove.

//printf("nBL %d wPopCnt %zd\n", nBL, wPopCnt);
    if (wPopCnt == 1) {
        assert( ! tp_bIsSwitch(nType) );
        OldList(pwr, wPopCnt, nBLR, nType);
#if defined(NO_TYPE_IN_XX_SW)
        if (nBL < nDL_to_nBL(2)) {
            assert(nBLR == nBL);
            *pwRoot = ZERO_POP_MAGIC;
        } else
#endif // defined(NO_TYPE_IN_XX_SW)
        {
  #ifdef XX_LISTS
            if (nType == T_XX_LIST) {
                int nBLRUp = gnListBLR(qy);
                int nBWRUp = nBLRUp - nBL;
                int nDigit = (wKey >> nBL) & MSK(nBWRUp);
                Link_t *pLinks = &pLn[-nDigit];
//printf("wRoot 0x%zx pLn->ln_wRoot 0x%zx\n", wRoot, pLn->ln_wRoot);
                for (int nDigit = 0; nDigit < (1 << nBWRUp); ++nDigit) {
//printf("pLinks[%d].ln_wRoot 0x%zx\n", nDigit, pLinks[nDigit].ln_wRoot);
// I wonder if we could use SignificantBitCnt here.
                    if (pLinks[nDigit].ln_wRoot == wRoot) {
                        pLinks[nDigit].ln_wRoot = WROOT_NULL;
                    }
                }
            } else
  #endif // XX_LISTS
            { *pwRoot = WROOT_NULL; }
        }
//printf("returning\n");
        // Do we need to clear the rest of the link also?
        // See bCleanup in Lookup/Remove for the rest.
        return Success;
    }

    Word_t *pwKeys = ls_pwKeysX(pwr, nBLR, wPopCnt);
#if defined(COMPRESSED_LISTS)
  #if (cnBitsPerWord > 32)
    uint32_t *piKeys = ls_piKeysNATX(pwr, wPopCnt);
  #endif // (cnBitsPerWord > 32)
    uint16_t *psKeys = ls_psKeysNATX(pwr, wPopCnt);
    uint8_t *pcKeys = ls_pcKeysNATX(pwr, wPopCnt);
#endif // defined(COMPRESSED_LISTS)
  #if 0
    if (CheckMalloc(pwr, ListWordCnt(wPopCnt, nBLR), cnBitsMallocMask) != 0) {
        printf("RG CheckMalloc failure\n");
        exit(1);
    }
  #endif
    int nIndex;
    for (nIndex = 0;
#if defined(COMPRESSED_LISTS)
        (nBLR <=  8) ? (pcKeys[nIndex] != (uint8_t) wKey) :
        (nBLR <= 16) ? (psKeys[nIndex] != (uint16_t)wKey) :
  #if (cnBitsPerWord > 32)
        (nBLR <= 32) ? (piKeys[nIndex] != (uint32_t)wKey) :
  #endif // (cnBitsPerWord > 32)
#endif // defined(COMPRESSED_LISTS)
        (pwKeys[nIndex] != wKey);
        nIndex++) ;

    // nIndex identifies the key being removed.

    Word_t *pwList;
    if (ListSlotCnt(wPopCnt - 1, nBLR) < (int)wPopCnt) {
        DBGR(printf("RG ListWordCnt(wPopCnt %zd nBLR %d) %d\n",
             wPopCnt-1, nBLR, ListWordCnt(wPopCnt-1, nBLR)));
        // Malloc a new, smaller list.
        assert(wPopCnt - 1 != 0);
        pwList = NewList(wPopCnt - 1, nBLR);
        set_wr(wRoot, pwList, nType);
    } else {
        pwList = pwr;
    }
    DBGR(printf("RG: ListWordCnt(wPopCnt %zd nBLR %d) %d\n",
                wPopCnt, nBLR, ListWordCnt(wPopCnt, nBLR)));

#if defined(UA_PARALLEL_128)
    if ((nBL == 16) && (wPopCnt - 1 <= 6)) {
  #ifdef XX_LISTS
        assert(nType != T_XX_LIST);
  #endif // XX_LISTS
        assert(nBLR == nBL);
        set_wr_nType(wRoot, T_LIST_UA);
    }
#endif // defined(UA_PARALLEL_128)

  #ifdef B_JUDYL
    Word_t *pwSrcVals = pwr;
      #ifdef LIST_POP_IN_PREAMBLE
    pwSrcVals -= 1;
      #endif // LIST_POP_IN_PREAMBLE
  #endif // B_JUDYL

    *pwRoot = wRoot;
    snListBLR(qy, nBLR);
    // Init pop count in list before using ls_p[csiw]Keys below.
    Set_xListPopCnt(pwRoot, nBLR, wPopCnt - 1);
    wRoot = pLn->ln_wRoot;
    Word_t *pwrOld = pwr;
    pwr = pwList;
    int nTypeOld = nType;
    nType = wr_nType(wRoot);
    DBGX(Log(qy, "RemoveGuts"));

  #ifdef B_JUDYL
    Word_t *pwTgtVals = gpwValues(qy);
  #endif // B_JUDYL

    if (pwr != pwrOld) {
        // Why are we copying the old list to the new one?
        // Because the beginning will be the same.
        // Except for the the pop count.
#ifdef COMPRESSED_LISTS
        switch (nBytesKeySz(nBLR)) {
        case sizeof(Word_t):
#endif // COMPRESSED_LISTS
#ifdef B_JUDYL
             // copy values
             COPY(&pwTgtVals[-((int)wPopCnt - 1)],
                  &pwSrcVals[-((int)wPopCnt - 1)],
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
             COPY(ls_pwKeysX(pwList, nBLR, wPopCnt - 1), pwKeys, wPopCnt - 1);
#ifdef COMPRESSED_LISTS
             break;
#if (cnBitsPerWord > 32)
        case 4:
  #ifdef B_JUDYL
             // copy values
             COPY(&pwTgtVals[-((int)wPopCnt - 1)],
                  &pwSrcVals[-((int)wPopCnt - 1)],
                  wPopCnt - 1);
  #endif // B_JUDYL
             // copy keys
             COPY(ls_piKeysNATX(pwList, wPopCnt - 1), piKeys, wPopCnt - 1);
             break;
#endif // (cnBitsPerWord > 32)
        case 2:
  #ifdef B_JUDYL
             // copy values
             COPY(&pwTgtVals[-((int)wPopCnt - 1)],
                  &pwSrcVals[-((int)wPopCnt - 1)],
                  wPopCnt - 1);
  #endif // B_JUDYL
             // copy keys
             COPY(ls_psKeysNATX(pwList, wPopCnt - 1), psKeys, wPopCnt - 1);
             break;
        case 1:
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
             if (nBLR != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
             {
                 // copy values
                 COPY(&pwTgtVals[-((int)wPopCnt - 1)],
                      &pwSrcVals[-((int)wPopCnt - 1)],
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
    if (nBLR <= 8) {
  #ifdef B_JUDYL
      #if !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        if (nBLR != cnBitsInD1)
      #endif // !defined(PACK_L1_VALUES) && (cnBitsInD1 <= 8)
        {
            // move values
            MOVE(&pwTgtVals[-((int)wPopCnt - 1)],
                 &pwSrcVals[- (int)wPopCnt     ],
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
    } else if (nBLR <= 16) {
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtVals[-((int)wPopCnt - 1)],
             &pwSrcVals[- (int)wPopCnt     ],
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
    } else if (nBLR <= 32) {
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtVals[-((int)wPopCnt - 1)],
             &pwSrcVals[- (int)wPopCnt     ],
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
  #ifdef B_JUDYL
        // move values
        MOVE(&pwTgtVals[-((int)wPopCnt - 1)],
             &pwSrcVals[- (int)wPopCnt     ],
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
        MOVE(&ls_pwKeysX(pwList, nBLR, wPopCnt - 1)[nIndex],
             &pwKeys[nIndex + 1], wPopCnt - nIndex - 1);
#if defined(PARALLEL_SEARCH_WORD)
        // Pad list to an integral number of parallel search buckets in
        // length, if necessary.
        PAD(ls_pwKeysX(pwList, nBLR, wPopCnt - 1), wPopCnt - 1);
#endif // defined(PARALLEL_SEARCH_WORD)
#if defined(LIST_END_MARKERS)
        ls_pwKeysX(pwList, nBLR, wPopCnt - 1)[
                ALIGN_LIST_LEN(ExtListBytesPerKey(nBL), wPopCnt - 1)
                    ? ALIGN_UP(wPopCnt - 1, sizeof(Bucket_t)) : wPopCnt - 1]
            = -1;
#endif // defined(LIST_END_MARKERS)
    }

  #ifdef XX_LISTS
    if (nType == T_XX_LIST) {
        // Replicate the link with a new pop cnt and maybe pwr.
        int nBLRUp = gnListBLR(qy);
        int nBWRUp = nBLRUp - nBL;
        int nDigit = (wKey >> nBL) & MSK(nBWRUp);
        Link_t *pLinks = &pLn[-nDigit];
        wRoot = pLn->ln_wRoot;
        for (int nDigit = 0; nDigit < (1 << nBWRUp); ++nDigit) {
// I wonder if we could use SignificantBitCnt here.
            if (wr_pwr(pLinks[nDigit].ln_wRoot) == pwrOld) {
                pLinks[nDigit].ln_wRoot = wRoot;
            }
        }
    }
  #endif // XX_LISTS

    if (pwr != pwrOld) {
        OldList(pwrOld, wPopCnt, nBLR, nTypeOld);
    }

    assert(tp_bIsList(wr_nType(wRoot)));
#if defined(EMBED_KEYS)
    // Embed the list if it fits.
    if (1
  #ifdef XX_LISTS
        && (wr_nType(wRoot) != T_XX_LIST)
  #endif // XX_LISTS
        && ((int)wPopCnt <= EmbeddedListPopCntMax(nBL) + 1))
    {
        assert(nBLR == nBL);
        DeflateExternalList(qya, wPopCnt - 1);
    } else
#endif // defined(EMBED_KEYS)
    {
        if (nBL == 16) {
            UpdateDist(qya, wPopCnt - 1);
        }
    }

    return Success;

#endif // (cwListPopCntMax != 0)

    (void)pwRoot; (void)wKey; (void)nDL; (void)wRoot;
}

#ifdef BITMAP
// Clear the bit for wKey in the bitmap.
// And free the bitmap if it is empty and not embedded.
static Status_t
RemoveAtBitmap(qpa, Word_t wKey)
{
    qva;

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

        Word_t wPopCnt = gwBitmapPopCnt(qya, nBLR) - 1;
  #ifdef _BMLF_BM_IN_LNX
        Word_t *pwBitmap = pwLnX;
  #else // _BMLF_BM_IN_LNX
        Word_t *pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
  #endif // else _BMLF_BM_IN_LNX
  #ifdef B_JUDYL
      #ifdef PACK_BM_VALUES
        Word_t wWords = BitmapWordCnt(nBLR, wPopCnt); // new
      #endif // PACK_BM_VALUES
        Word_t bUnpacked = BM_UNPACKED(wRoot);
        if (bUnpacked) {
      #ifdef PACK_BM_VALUES
            if (wWords == BitmapWordCnt(nBLR, EXP(nBLR)))
      #endif // PACK_BM_VALUES
            {
                goto done;
            }
        } else {
        }
      #ifdef PACK_BM_VALUES
        Word_t *pwSrcVals = gpwBitmapValues(qy, nBLR);
        int nPos = BmIndex(qya, nBLR, wKey);
        if (wWords != BitmapWordCnt(nBLR, wPopCnt + 1)) {
        // NewBitmap will zero the bits.
      #ifdef _BMLF_BM_IN_LNX
            Word_t wBitmapOld = *pwBitmap;
      #endif // _BMLF_BM_IN_LNX
            NewBitmap(qya, nBLR, wKey, wPopCnt);
            // Prefix and popcnt are set; bits are not.
            // *pwRoot has been updated. qy is out of date.
            // Copy bits and update qy.
            wRoot = *pwRoot;
            Word_t *pwrOld = pwr;
            pwr = wr_pwr(wRoot);
            int nBmWords = (nBLR <= cnLogBitsPerWord)
                             ? 1 : EXP(nBLR - cnLogBitsPerWord);
      #ifdef _BMLF_BM_IN_LNX
            Word_t* pwBitmapOld = &wBitmapOld;
            pwBitmap = pwLnX;
      #else // _BMLF_BM_IN_LNX
            Word_t* pwBitmapOld = pwBitmap;
            pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
      #endif // else _BMLF_BM_IN_LNX
            COPY(pwBitmap, pwBitmapOld, nBmWords);
      #ifdef BMLF_CNTS
            if (bUnpacked) {
      #ifdef BMLF_POP_COUNT_32
    uint32_t* pu32Bitmap = (uint32_t*)pwBitmap;
          #if defined(BMLF_CNTS_IN_LNX)
    Word_t* pwCnts = pwLnX;
          #else // BMLF_CNTS_IN_LNX
    Word_t* pwCnts = (Word_t*)((BmLeaf_t*)pwr)->bmlf_au8Cnts;
          #endif // BMLF_CNTS_IN_LNX else
    for (Word_t ww = 0; ww < EXP(cnBitsInD1 - 5); ++ww) {
        ((uint8_t*)pwCnts)[ww] = PopCount32(pu32Bitmap[ww]);
    }
          #ifdef BMLF_CNTS_CUM
    *pwCnts *= 0x0101010101010100;
          #endif // BMLF_CNTS_CUM
      #else // BMLF_POP_COUNT_32
          #ifdef BMLF_POP_COUNT_8
    int nSum = 0;
    for (int nn = 0; nn < (1 << (cnBitsInD1 - cnLogBitsPerByte)); ++nn) {
        ((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn] = nSum;
        nSum += PopCount8(((uint8_t*)pwBitmap)[nn]);
    }
          #elif defined(BMLF_POP_COUNT_1)
    int nSum = 0;
    for (int nn = 0; nn < (1 << cnBitsInD1); ++nn) {
        ((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn] = nSum;
        nSum += BitIsSet(pwBitmap, nn);
    }
          #else // BMLF_POP_COUNT_8
              #ifdef BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = (uint8_t*)pwLnX;
              #else // BMLF_CNTS_IN_LNX
    uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
              #endif // BMLF_CNTS_IN_LNX else
    for (int nn = 0;
         nn < (1 << (cnBitsInD1 - cnLogBitsPerWord)); ++nn)
    {
        pu8Cnts[nn] = PopCount64(pwBitmap[nn]);
    }
              #ifdef BMLF_CNTS_CUM
                  #ifdef BM_DSPLIT
    Word_t wCnts = *(Word_t*)pu8Cnts;
                  #endif // BM_DSPLIT
    *(Word_t*)pu8Cnts *= 0x01010100;
                  #ifdef BM_DSPLIT
    *(Word_t*)pu8Cnts &= (Word_t)0xffffffffUL;
    *(Word_t*)pu8Cnts |= wCnts << 32;
                  #endif // BM_DSPLIT
              #endif // BMLF_CNTS_CUM
          #endif // #else BMLF_POP_COUNT_8
      #endif // #else BMLF_POP_COUNT_32
            } else
            {
          #if defined(BMLF_POP_COUNT_8) || defined(BMLF_POP_COUNT_1)
            COPY(((BmLeaf_t*)pwr)->bmlf_au8Cnts,
                 ((BmLeaf_t*)pwrOld)->bmlf_au8Cnts,
                 sizeof((BmLeaf_t*)pwr)->bmlf_au8Cnts);
          #else // BMLF_POP_COUNT_8 || BMLF_POP_COUNT_1
              #ifndef BMLF_CNTS_IN_LNX
            *(Word_t*)(((BmLeaf_t*)pwr)->bmlf_au8Cnts)
                = *(Word_t*)((BmLeaf_t*)pwrOld)->bmlf_au8Cnts;
              #endif // !BMLF_CNTS_IN_LNX
          #endif // BMLF_POP_COUNT_8 || BMLF_POP_COUNT_1 else
            }
      #endif // BMLF_CNTS
            Word_t *pwTgtVals = gpwBitmapValues(qy, nBLR);
            if (bUnpacked) {
                for (int k = 0; k < (int)EXP(nBLR); ++k) {
                    if ((k != (int)(wKey & MSK(nBLR)))
                        && BitIsSet(pwBitmap, k))
                    {
                        *pwTgtVals++ = pwSrcVals[k];
                    }
                }
            } else {
                COPY(&pwTgtVals[nPos], &pwSrcVals[nPos + 1],
                     wPopCnt - nPos);
                COPY(pwTgtVals, pwSrcVals, nPos);
            }
            OldBitmap(pwrOld, nBLR, wPopCnt + 1);
        } else
      #endif // PACK_BM_VALUES
  #endif // B_JUDYL
        {
#ifdef B_JUDYL
      #ifdef PACK_BM_VALUES
            Word_t *pwSrcVals = gpwBitmapValues(qy, nBLR);
            MOVE(&pwSrcVals[nPos], &pwSrcVals[nPos + 1], wPopCnt - nPos);
      #endif // PACK_BM_VALUES
            goto done;
done:
#endif // B_JUDYL
            swBitmapPopCnt(qya, nBLR, wPopCnt);
        }
        ClrBit(pwBitmap, wKey & MSK(nBLR));
  #if cn2dBmMaxWpkPercent != 0
    if (nBLR == cnBitsLeftAtDl2) {
      #if cnWordsBm2Cnts != 0
        --gpxBitmapCnts(qya, nBLR)[(wKey & MSK(nBLR)) >> cnLogBmlfBitsPerCnt];
      #endif // cnWordsBm2Cnts != 0
    }
  #endif // cn2dBmMaxWpkPercent != 0
  #ifdef BMLF_CNTS
        if (BM_UNPACKED(wRoot)) { }
      #ifdef PACK_BM_VALUES
        else
        {
      #ifdef BMLF_POP_COUNT_32
        int nBm = (wKey >> 5) & MSK(cnBitsInD1 - 5);
          #ifdef BMLF_CNTS_IN_LNX
        uint8_t* pu8Cnts = (uint8_t*)pwLnX;
          #else // BMLF_CNTS_IN_LNX
        uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
          #endif // BMLF_CNTS_IN_LNX else
          #ifdef BMLF_CNTS_CUM
        *(Word_t*)pu8Cnts -= 0x0101010101010100 & ~NZ_MSK((nBm + 1) * 8);
          #else // BMLF_CNTS_CUM
        --pu8Cnts[nBm];
          #endif // BMLF_CNTS_CUM else
      #else // BMLF_POP_COUNT_32
          #ifdef BMLF_POP_COUNT_8
        int nBmByte
            = (wKey >> cnLogBitsPerByte) & MSK(cnBitsInD1 - cnLogBitsPerByte);
        for (int nn = nBmByte + 1;
             nn < (1 << (cnBitsInD1 - cnLogBitsPerByte));
             ++nn)
        {
            --((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn];
        }
          #elif defined(BMLF_POP_COUNT_1) // BMLF_POP_COUNT_8
        int nBmByte = wKey & MSK(cnBitsInD1);
        for (int nn = nBmByte + 1; nn < (1 << cnBitsInD1); ++nn) {
            --((BmLeaf_t*)pwr)->bmlf_au8Cnts[nn];
        }
          #else // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1
        int nBmWord
            = (wKey >> cnLogBitsPerWord) & MSK(cnBitsInD1 - cnLogBitsPerWord);
              #ifdef BMLF_CNTS_IN_LNX
        uint8_t* pu8Cnts = (uint8_t*)pwLnX;
              #else // BMLF_CNTS_IN_LNX
        uint8_t* pu8Cnts = ((BmLeaf_t*)pwr)->bmlf_au8Cnts;
              #endif // BMLF_CNTS_IN_LNX else
              #ifdef BMLF_CNTS_CUM
        for (int nn = nBmWord + 1; nn < (1 << (cnBitsInD1 - cnLogBitsPerWord));
             ++nn)
        {
            --pu8Cnts[nn];
        }
                  #ifdef BM_DSPLIT
        --pu8Cnts[4+nBmWord];
                  #endif // BM_DSPLIT
              #else // BMLF_CNTS_CUM
        --pu8Cnts[nBmWord];
              #endif // BMLF_CNTS_CUM else
          #endif // BMLF_POP_COUNT_8 elif BMLF_POP_COUNT_1 else
      #endif // BMLF_POP_COUNT_32 else
        }
      #endif // PACK_BM_VALUES
  #endif // BMLF_CNTS

#if defined(DEBUG_COUNT)
        Word_t wPopCntDbg = 0;
        int nWords
            = (nBLR <= cnLogBitsPerWord) ? 1 : EXP(nBLR - cnLogBitsPerWord);
        for (int i = 0; i < nWords; ++i) {
            wPopCntDbg += __builtin_popcountll(pwBitmap[i]);
        }
        assert(wPopCntDbg == wPopCnt);
#endif // defined(DEBUG_COUNT)

        // Free the bitmap if it is empty.
        if (wPopCnt == 0) {
            OldBitmap(pwr, nBLR, 0);
            *pwRoot = WROOT_NULL;
        } else {
            assert(gwBitmapPopCnt(qya, nBLR) == wPopCnt);
        }
    }

    return Success;
}
#endif // BITMAP

#endif // (cnDigitsPerWord != 1)

static void
Initialize(void)
{
#if cwListPopCntMax != 0
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
#endif // cwListPopCntMax != 0
    // If EmbeddedListPopCntMax > auListPopCntMax adjust auListPopCntMax
    // to simplify code elsewhere.
    for (int nBL = cnBitsPerWord;
         nBL >=
      #ifdef USE_XX_SW_ONLY_AT_DL2
             (assert(cbEmbeddedBitmap), cnLogBitsPerLink + 1)
      #else // USE_XX_SW_ONLY_AT_DL2
             cnBitsInD1
      #endif // USE_XX_SW_ONLY_AT_DL2
             ;
         --nBL)
    {
  #ifdef EMBED_KEYS
  #ifndef POP_CNT_MAX_IS_KING
        if (EmbeddedListPopCntMax(nBL) > auListPopCntMax[nBL]) {
            fprintf(stderr, "# auListPopCntMax[nBL %d] %d changed to %d\n",
                    nBL, auListPopCntMax[nBL], EmbeddedListPopCntMax(nBL));
            auListPopCntMax[nBL] = EmbeddedListPopCntMax(nBL);
        }
  #endif // #ifndef POP_CNT_MAX_IS_KING
  #endif // EMBED_KEYS
  #if cwListPopCntMax != 0
        // Splay gets pretty costly and ugly if the new lists don't fit.
        // The simplest way to avoid it is ensure that
        // auListPopCntMax[nBL] >= auListPopCntMax[nBL+1] for all nBL.
        // And, for XX_LISTS, if each half-digit is at least one key
        // bigger, then we can trigger the switch widening on a different
        // insert than the first splay and further reduce our worst-case
        // insert times.
        // And, icing on the cake would be to trigger switch widening on
        // an insert that doesn't require a new malloc of the list.
        // Crap, what was I thinking. nBLR of the list does not change
        // when doubling down. So if it is full before the insert that
        // triggers DoubleDown, then it will have to be splayed during
        // the same insert as the DoubleDown.
        printf("# auListPopCntMax[%d] %d\n", nBL, auListPopCntMax[nBL]);
        if ((nBL != cnBitsPerWord) // there is no nBL+1
            && (nBL > cnBitsInD1) // can resort to bitmap
            && (auListPopCntMax[nBL] < auListPopCntMax[nBL+1]))
        {
            fprintf(stderr, "# Warning"
                " auListPopCntMax[%d] %d < auListPopCntMax[%d] %d\n",
                nBL, auListPopCntMax[nBL], nBL+1, auListPopCntMax[nBL+1]);
        }
  #if 0
        if (ListSlotCnt(auListPopCntMax[nBL] - 1, nBL) < auListPopCntMax[nBL])
        {
            fprintf(stderr, "# Bummer"
                    " ListSlotCnt(auListPopCntMax[nBL %d] - 1, nBL) %d"
                    " < auListPopCntMax[nBL] %d\n",
                nBL, ListSlotCnt(auListPopCntMax[nBL] - 1, nBL),
               auListPopCntMax[nBL]);
        }
  #endif
  #endif // cwListPopCntMax != 0
    }

#ifdef WROOT_NULL_IS_EK
    // We need a bit set in T_EMBEDDED_KEYS to maximize the size of key that
    // we can embed.
    assert(T_EMBEDDED_KEYS != 0);
#endif // WROOT_NULL_IS_EK
#ifdef SEPARATE_T_NULL
    assert(wr_nType(WROOT_NULL) == T_NULL);
#endif // SEPARATE_T_NULL

  #ifdef DEBUG
    // There may be an issue with dlmalloc and greater than 2MB (size of huge
    // page) requests. Dlmalloc may mmap something other than an integral
    // multiple of 2MB.
  #ifdef BITMAP
    if (!cbEmbeddedBitmap
            && (BitmapWordCnt(cnBitsInD1, EXP(cnBitsInD1)) * sizeof(Word_t)
                > 0x200000 - sizeof(Word_t)))
    {
        fprintf(stderr,
                "# Warning. Full-pop bitmap at nBL %d is close to 2MB.\n",
                cnBitsInD1);
    }
    if (1
      #ifndef USE_XX_SW_ONLY_AT_DL2
        && (cn2dBmMaxWpkPercent != 0)
      #endif // #ifndef USE_XX_SW_ONLY_AT_DL2
        && (BitmapWordCnt(cnBitsLeftAtDl2, EXP(cnBitsLeftAtDl2))
                    * sizeof(Word_t)
                > 0x200000 - sizeof(Word_t)))
    {
        fprintf(stderr,
                "# Warning. Full-pop bitmap at nBL %d is close to 2MB.\n",
                cnBitsLeftAtDl2);
    }
  #endif // BITMAP
    // What other big objects should we check? Switches? Merged switches?
    // We check external lists below.
    // JudyMalloc complains and exits if it sees a malloc size that is
    // problematic at run time.
  #endif // DEBUG
  #ifdef UA_PARALLEL_128
    assert(cnBitsMallocMask >= 4);
    for (int i = 1; i <= 6; i++) {
        if (ListWordCnt(i, 16) != 3) {
            printf("ListWordCnt(%d, 16) %d\n",
                   i, ListWordCnt(i, 16));
        }
        assert(ListWordCnt(i, 16) == 3);
    }
    assert(ListWordCnt(7, 16) > 3);
  #endif // UA_PARALLEL_128
  #ifdef NO_TYPE_IN_XX_SW
  #ifndef REVERSE_SORT_EMBEDDED_KEYS
    assert(T_EMBEDDED_KEYS != 0); // see b.h
  #endif // !REVERSE_SORT_EMBEDDED_KEYS
  #endif // NO_TYPE_IN_XX_SW

  #ifdef USE_XX_SW_ONLY_AT_DL2
  #ifdef SKIP_TO_XX_SW
  #ifndef SKIP_TO_BITMAP
    #error USE_XX_SW_ONLY_AT_DL2 and SKIP_TO_XX_SW without SKIP_TO_BITMAP
  #endif // #ifndef SKIP_TO_BITMAP
  #endif // SKIP_TO_XX_SW
  #endif // USE_XX_SW_ONLY_AT_DL2

  #ifndef B_JUDYL
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
  #endif // ifndef B_JUDYL

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
        printf("# tp_to_nDLR(cnMallocMask    0x%02x)   %2d\n",
               (int)cnMallocMask, (int)tp_to_nDLR(cnMallocMask));
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


  #ifdef NO_TYPE_IN_XX_SW
  #ifndef REVERSE_SORT_EMBEDDED_KEYS
    // Not sure if/why this matters.
    assert (wr_nType(ZERO_POP_MAGIC) == T_EMBEDDED_KEYS);
  #endif // !REVERSE_SORT_EMBEDDED_KEYS
  #endif // NO_TYPE_IN_XX_SW

  #if defined(CODE_XX_SW)
    // Make sure nBW field is big enough.
    assert((cnBitsLeftAtDl2 - cnBWMin - (cnLogBitsPerWord + 1))
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

    // Log ifdefs.

#ifdef           _USE_LOCATE_KEY
    printf("#    _USE_LOCATE_KEY\n");
#else //         _USE_LOCATE_KEY
    printf("# No _USE_LOCATE_KEY\n");
#endif //        _USE_LOCATE_KEY else

                 // Prefetch bitmap leaf value using subexpanse psplit.
#ifdef           BM_DSPLIT
    printf("#    BM_DSPLIT\n");
#else //         BM_DSPLIT
    printf("# No BM_DSPLIT\n");
#endif //        BM_DSPLIT else

#ifdef           HYPERTUNE_PF_BM
    printf("#    HYPERTUNE_PF_BM\n");
#else //         HYPERTUNE_PF_BM
    printf("# No HYPERTUNE_PF_BM\n");
#endif //        HYPERTUNE_PF_BM else

#ifdef           AUG_TYPE_64_LOOKUP
    printf("#    AUG_TYPE_64_LOOKUP\n");
#else //         AUG_TYPE_64_LOOKUP
    printf("# No AUG_TYPE_64_LOOKUP\n");
#endif //        AUG_TYPE_64_LOOKUP else

#ifdef           AUG_TYPE_32_LOOKUP
    printf("#    AUG_TYPE_32_LOOKUP\n");
#else //         AUG_TYPE_32_LOOKUP
    printf("# No AUG_TYPE_32_LOOKUP\n");
#endif //        AUG_TYPE_32_LOOKUP else

#ifdef           AUG_TYPE_16_LOOKUP
    printf("#    AUG_TYPE_16_LOOKUP\n");
#else //         AUG_TYPE_16_LOOKUP
    printf("# No AUG_TYPE_16_LOOKUP\n");
#endif //        AUG_TYPE_16_LOOKUP else

#ifdef           AUG_TYPE_8_LOOKUP
    printf("#    AUG_TYPE_8_LOOKUP\n");
#else //         AUG_TYPE_8_LOOKUP
    printf("# No AUG_TYPE_8_LOOKUP\n");
#endif //        AUG_TYPE_8_LOOKUP else

#ifdef           AUGMENT_TYPE
    printf("#    AUGMENT_TYPE\n");
#else //         AUGMENT_TYPE
    printf("# No AUGMENT_TYPE\n");
#endif //        AUGMENT_TYPE else

#ifdef           AUGMENT_TYPE_8
    printf("#    AUGMENT_TYPE_8\n");
  #if       cnBitsPerDigit != 8
    #error  cnBitsPerDigit != 8
  #endif // cnBitsPerDigit != 8
  #ifndef AUGMENT_TYPE_8_PLUS_4
    #if       cnBitsInD1 != 8 || cnBitsInD2 != 8 || cnBitsInD3 != 8
      #error  cnBitsInD1 != 8 || cnBitsInD2 != 8 || cnBitsInD3 != 8
    #endif // cnBitsInD1 != 8 || cnBitsInD2 != 8 || cnBitsInD3 != 8
  #endif // AUGMENT_TYPE_8_PLUS_4
#else //         AUGMENT_TYPE_8
    printf("# No AUGMENT_TYPE_8\n");
#endif //        AUGMENT_TYPE_8 else

#ifdef           AUGMENT_TYPE_8_PLUS_4
    printf("#    AUGMENT_TYPE_8_PLUS_4\n");
    #if       cnBitsLeftAtDl1 <  4 || 12 <= cnBitsLeftAtDl1
      #error  cnBitsLeftAtDl1 <  4 || 12 <= cnBitsLeftAtDl1
    #endif // cnBitsLeftAtDl1 <  4 || 12 <= cnBitsLeftAtDl1
    #if       cnBitsLeftAtDl2 < 12 || 20 <= cnBitsLeftAtDl2
      #error  cnBitsLeftAtDl2 < 12 || 20 <= cnBitsLeftAtDl2
    #endif // cnBitsLeftAtDl2 < 12 || 20 <= cnBitsLeftAtDl2
    #if       cnBitsLeftAtDl3 < 20 || 28 <= cnBitsLeftAtDl3
      #error  cnBitsLeftAtDl3 < 20 || 28 <= cnBitsLeftAtDl3
    #endif // cnBitsLeftAtDl3 < 20 || 28 <= cnBitsLeftAtDl3
#else //         AUGMENT_TYPE_8_PLUS_4
    printf("# No AUGMENT_TYPE_8_PLUS_4\n");
#endif // else   AUGMENT_TYPE_8_PLUS_4

#ifdef           AUG_TYPE_8_SW_NEXT
    printf("#    AUG_TYPE_8_SW_NEXT\n");
#else //         AUG_TYPE_8_SW_NEXT
    printf("# No AUG_TYPE_8_SW_NEXT\n");
#endif //        AUG_TYPE_8_SW_NEXT else

#ifdef           AUG_TYPE_8_NEXT_EK_XV
    printf("#    AUG_TYPE_8_NEXT_EK_XV\n");
#else //         AUG_TYPE_8_NEXT_EK_XV
    printf("# No AUG_TYPE_8_NEXT_EK_XV\n");
#endif //        AUG_TYPE_8_NEXT_EK_XV else

#ifdef           AUG_TYPE_8_NEXT_EMPTY
    printf("#    AUG_TYPE_8_NEXT_EMPTY\n");
#else //         AUG_TYPE_8_NEXT_EMPTY
    printf("# No AUG_TYPE_8_NEXT_EMPTY\n");
#endif //        AUG_TYPE_8_NEXT_EMPTY else

#ifdef           BL_SPECIFIC_LIST
    printf("#    BL_SPECIFIC_LIST\n");
#else //         BL_SPECIFIC_LIST
    printf("# No BL_SPECIFIC_LIST\n");
#endif //        BL_SPECIFIC_LIST else

#ifdef           BL_SPECIFIC_SKIP
    printf("#    BL_SPECIFIC_SKIP\n");
#else //         BL_SPECIFIC_SKIP
    printf("# No BL_SPECIFIC_SKIP\n");
#endif //        BL_SPECIFIC_SKIP else

#ifdef           BL_SPECIFIC_SKIP_JT
    printf("#    BL_SPECIFIC_SKIP_JT\n");
#else //         BL_SPECIFIC_SKIP_JT
    printf("# No BL_SPECIFIC_SKIP_JT\n");
#endif //        BL_SPECIFIC_SKIP_JT else

#ifdef           MASK_NBLR
    printf("#    MASK_NBLR\n");
#else //         MASK_NBLR
    printf("# No MASK_NBLR\n");
#endif //        MASK_NBLR else

#ifdef           DSPLIT_16
    printf("#    DSPLIT_16\n");
#else //         DSPLIT_16
    printf("# No DSPLIT_16\n");
#endif // #else  DSPLIT_16

#ifdef           DS_4_WAY
    printf("#    DS_4_WAY\n");
#else //         DS_4_WAY
    printf("# No DS_4_WAY\n");
#endif // #else  DS_4_WAY

#ifdef           DS_4_WAY_A
    printf("#    DS_4_WAY_A\n");
#else //         DS_4_WAY_A
    printf("# No DS_4_WAY_A\n");
#endif // #else  DS_4_WAY_A

#ifdef           DS_8_WAY
    printf("#    DS_8_WAY\n");
#else //         DS_8_WAY
    printf("# No DS_8_WAY\n");
#endif // #else  DS_8_WAY

#ifdef           DS_8_WAY_A
    printf("#    DS_8_WAY_A\n");
#else //         DS_8_WAY_A
    printf("# No DS_8_WAY_A\n");
#endif // #else  DS_8_WAY_A

#ifdef           DS_16_WAY
    printf("#    DS_16_WAY\n");
#else //         DS_16_WAY
    printf("# No DS_16_WAY\n");
#endif // #else  DS_16_WAY

#ifdef           DS_16_WAY_A
    printf("#    DS_16_WAY_A\n");
#else //         DS_16_WAY_A
    printf("# No DS_16_WAY_A\n");
#endif // #else  DS_16_WAY_A

#ifdef           DS_AVG
    printf("#    DS_AVG\n");
#else //         DS_AVG
    printf("# No DS_AVG\n");
#endif // #else  DS_AVG

#ifdef           DS_ONE_DIV
    printf("#    DS_ONE_DIV\n");
#else //         DS_ONE_DIV
    printf("# No DS_ONE_DIV\n");
#endif // #else  DS_ONE_DIV

#ifdef           DS_SAVE_DIV
    printf("#    DS_SAVE_DIV\n");
#else //         DS_SAVE_DIV
    printf("# No DS_SAVE_DIV\n");
#endif // #else  DS_SAVE_DIV

#ifdef           DS_NO_CHECK
    printf("#    DS_NO_CHECK\n");
#else //         DS_NO_CHECK
    printf("# No DS_NO_CHECK\n");
#endif // #else  DS_NO_CHECK

#ifdef           DS_EARLY_OUT_CHECK
    printf("#    DS_EARLY_OUT_CHECK\n");
#else //         DS_EARLY_OUT_CHECK
    printf("# No DS_EARLY_OUT_CHECK\n");
#endif // #else  DS_EARLY_OUT_CHECK

#ifdef           CACHE_ALIGN_L1
    printf("#    CACHE_ALIGN_L1\n");
#else //         CACHE_ALIGN_L1
    printf("# No CACHE_ALIGN_L1\n");
#endif // #else  CACHE_ALIGN_L1

#ifdef           USE_POPCOUNT_IN_LK8
    printf("#    USE_POPCOUNT_IN_LK8\n");
#else //         USE_POPCOUNT_IN_LK8
    printf("# No USE_POPCOUNT_IN_LK8\n");
#endif // #else  USE_POPCOUNT_IN_LK8

#ifdef           USE_FFS_IN_LK8
    printf("#    USE_FFS_IN_LK8\n");
#else //         USE_FFS_IN_LK8
    printf("# No USE_FFS_IN_LK8\n");
#endif // #else  USE_FFS_IN_LK8

#ifdef           SLOW_FIBONACCI
    printf("#    SLOW_FIBONACCI\n");
#else //         SLOW_FIBONACCI
    printf("# No SLOW_FIBONACCI\n");
#endif // #else  SLOW_FIBONACCI

#ifdef           POW_2_ALLOC
    printf("#    POW_2_ALLOC\n");
#else //         POW_2_ALLOC
    printf("# No POW_2_ALLOC\n");
#endif // #else  POW_2_ALLOC

#ifdef           POP_COUNT_64
    printf("#    POP_COUNT_64\n");
#else //         POP_COUNT_64
    printf("# No POP_COUNT_64\n");
#endif // #else  POP_COUNT_64

#ifdef           BEST_POP_COUNT_64
    printf("#    BEST_POP_COUNT_64\n");
#else //         BEST_POP_COUNT_64
    printf("# No BEST_POP_COUNT_64\n");
#endif // #else  BEST_POP_COUNT_64

#ifdef           MOD_POP_COUNT_32
    printf("#    MOD_POP_COUNT_32\n");
#else //         MOD_POP_COUNT_32
    printf("# No MOD_POP_COUNT_32\n");
#endif // #else  MOD_POP_COUNT_32

#ifdef           BEST_POP_COUNT_32_
    printf("#    BEST_POP_COUNT_32\n");
#else //         BEST_POP_COUNT_32
    printf("# No BEST_POP_COUNT_32\n");
#endif // #else  BEST_POP_COUNT_32

#ifdef           BM_SW_CNT_IN_WR
    printf("#    BM_SW_CNT_IN_WR\n");
#else //         BM_SW_CNT_IN_WR
    printf("# No BM_SW_CNT_IN_WR\n");
#endif // #else  BM_SW_CNT_IN_WR

#ifdef           PF_BM_SW_WORDS
    printf("#    PF_BM_SW_WORDS\n");
#else //         PF_BM_SW_WORDS
    printf("# No PF_BM_SW_WORDS\n");
#endif // #else  PF_BM_SW_WORDS

#ifdef           PF_BM_SW_LN
    printf("#    PF_BM_SW_LN\n");
#else //         PF_BM_SW_LN
    printf("# No PF_BM_SW_LN\n");
#endif // #else  PF_BM_SW_LN

#ifdef           PF_BM_SW_LNX
    printf("#    PF_BM_SW_LNX\n");
#else //         PF_BM_SW_LNX
    printf("# No PF_BM_SW_LNX\n");
#endif // #else  PF_BM_SW_LNX

#ifdef           BMLF_INTERLEAVE
    printf("#    BMLF_INTERLEAVE\n");
    printf("#    cnLogBmlfParts %d\n", cnLogBmlfParts);
#else //         BMLF_INTERLEAVE
    printf("# No BMLF_INTERLEAVE\n");
#endif // #else  BMLF_INTERLEAVE

#ifdef           BMLFI_VARIABLE_SZ
    printf("#    BMLFI_VARIABLE_SZ\n");
#else //         BMLFI_VARIABLE_SZ
    printf("# No BMLFI_VARIABLE_SZ\n");
#endif // #else  BMLFI_VARIABLE_SZ

#ifdef           BMLFI_LNX
    printf("#    BMLFI_LNX\n");
#else //         BMLFI_LNX
    printf("# No BMLFI_LNX\n");
#endif // #else  BMLFI_LNX

#ifdef           BMLFI_BM_HB
    printf("#    BMLFI_BM_HB\n");
#else //         BMLFI_BM_HB
    printf("# No BMLFI_BM_HB\n");
#endif // #else  BMLFI_BM_HB

#ifdef           BMLF_POP_COUNT_32
    printf("#    BMLF_POP_COUNT_32\n");
#else //         BMLF_POP_COUNT_32
    printf("# No BMLF_POP_COUNT_32\n");
#endif // #else  BMLF_POP_COUNT_32

#ifdef           BMLF_POP_COUNT_8_
    printf("#    BMLF_POP_COUNT_8\n");
#else //         BMLF_POP_COUNT_8
    printf("# No BMLF_POP_COUNT_8\n");
#endif // #else  BMLF_POP_COUNT_8

#ifdef           MOD_POP_COUNT_8
    printf("#    MOD_POP_COUNT_8\n");
#else //         MOD_POP_COUNT_8
    printf("# No MOD_POP_COUNT_8\n");
#endif // #else  MOD_POP_COUNT_8

#ifdef           BMLF_POP_COUNT_1
    printf("#    BMLF_POP_COUNT_1\n");
#else //         BMLF_POP_COUNT_1
    printf("# No BMLF_POP_COUNT_1\n");
#endif // #else  BMLF_POP_COUNT_1

#ifdef           BMLF_POP_COUNT_1_NO_TEST
    printf("#    BMLF_POP_COUNT_1_NO_TEST\n");
#else //         BMLF_POP_COUNT_1_NO_TEST
    printf("# No BMLF_POP_COUNT_1_NO_TEST\n");
#endif // #else  BMLF_POP_COUNT_1_NO_TEST

#ifdef           BMLF_CNTS
    printf("#    BMLF_CNTS\n");
#else //         BMLF_CNTS
    printf("# No BMLF_CNTS\n");
#endif // #else  BMLF_CNTS

#ifdef           BMLF_CNTS_CUM
    printf("#    BMLF_CNTS_CUM\n");
#else //         BMLF_CNTS_CUM
    printf("# No BMLF_CNTS_CUM\n");
#endif // #else  BMLF_CNTS_CUM

#ifdef           BMLF_CNTS_IN_LNX
    printf("#    BMLF_CNTS_IN_LNX\n");
#else //         BMLF_CNTS_IN_LNX
    printf("# No BMLF_CNTS_IN_LNX\n");
#endif // #else  BMLF_CNTS_IN_LNX

#ifdef          _BMLF_BM_IN_LNX
    printf("#    BMLF_BM_IN_LNX\n");
#else //        _BMLF_BM_IN_LNX
    printf("# No BMLF_BM_IN_LNX\n");
#endif //       _BMLF_BM_IN_LNX else

#ifdef           XX_LISTS
    printf("#    XX_LISTS\n");
#else //         XX_LISTS
    printf("# No XX_LISTS\n");
#endif // #else  XX_LISTS

#ifdef           DOUBLE_DOWN
    printf("#    DOUBLE_DOWN\n");
#else //         DOUBLE_DOWN
    printf("# No DOUBLE_DOWN\n");
#endif // #else  DOUBLE_DOWN

#ifdef           SPLAY_WITH_INSERT
    printf("#    SPLAY_WITH_INSERT\n");
#else //         SPLAY_WITH_INSERT
    printf("# No SPLAY_WITH_INSERT\n");
#endif // #else  SPLAY_WITH_INSERT

#ifdef OLD_LIST_WORD_CNT
    printf("#    OLD_LIST_WORD_CNT\n");
#else // OLD_LIST_WORD_CNT
    printf("# No OLD_LIST_WORD_CNT\n");
#endif // #else  OLD_LIST_WORD_CNT

#if defined(POP_IN_WR_HB)
    printf("#    POP_IN_WR_HB\n");
#else // defined(POP_IN_WR_HB)
    printf("# No POP_IN_WR_HB\n");
#endif // defined(POP_IN_WR_HB)

#ifdef           BM_POP_IN_WR_HB
    printf("#    BM_POP_IN_WR_HB\n");
#else //         BM_POP_IN_WR_HB
    printf("# No BM_POP_IN_WR_HB\n");
#endif // #else  BM_POP_IN_WR_HB

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
    printf("# cbEmbeddedBitmap %d\n", cbEmbeddedBitmap);

    // EBM is JudyL embedded bitmap.
    // CHECK_TYPE_FOR_EBM means test type before goto t_bitmap even when
    // ifdefs make it unnecesary. Primarily for measurement only.
  #ifdef         CHECK_TYPE_FOR_EBM
    printf("#    CHECK_TYPE_FOR_EBM\n");
  #else //       CHECK_TYPE_FOR_EBM
    printf("# No CHECK_TYPE_FOR_EBM\n");
  #endif // else CHECK_TYPE_FOR_EBM

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

#ifdef NO_SKIP_LINKS
    printf("#    NO_SKIP_LINKS\n");
#else // NO_SKIP_LINKS
    printf("# No NO_SKIP_LINKS\n");
#endif // NO_SKIP_LINKS else

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

#ifdef           PACK_BM_VALUES
    printf("#    PACK_BM_VALUES\n");
#else //         PACK_BM_VALUES
    printf("# No PACK_BM_VALUES\n");
#endif // #else  PACK_BM_VALUES

#ifdef           UNPACK_BM_VALUES
    printf("#    UNPACK_BM_VALUES\n");
#else //         UNPACK_BM_VALUES
    printf("# No UNPACK_BM_VALUES\n");
#endif // #else  UNPACK_BM_VALUES

  #ifdef         ALLOC_WHOLE_PACKED_BMLF_EXP
    printf("#    ALLOC_WHOLE_PACKED_BMLF_EXP\n");
  #else //       ALLOC_WHOLE_PACKED_BMLF_EXP
    printf("# No ALLOC_WHOLE_PACKED_BMLF_EXP\n");
  #endif // else ALLOC_WHOLE_PACKED_BMLF_EXP

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

#ifdef           NO_BM_SW_CNT_IN_WR
    printf("#    NO_BM_SW_CNT_IN_WR\n");
#else //         NO_BM_SW_CNT_IN_WR
    printf("# No NO_BM_SW_CNT_IN_WR\n");
#endif //        NO_BM_SW_CNT_IN_WR

#ifdef           NO_OFFSET_IN_SW_BM_WORD
    printf("#    NO_OFFSET_IN_SW_BM_WORD\n");
#else //         NO_OFFSET_IN_SW_BM_WORD
    printf("# No NO_OFFSET_IN_SW_BM_WORD\n");
#endif //        NO_OFFSET_IN_SW_BM_WORD

#ifdef           NO_PF_BM_SW_LN
    printf("#    NO_PF_BM_SW_LN\n");
#else //         NO_PF_BM_SW_LN
    printf("# No NO_PF_BM_SW_LN\n");
#endif // #else  NO_PF_BM_SW_LN

#ifdef           NO_PF_LK
    printf("#    NO_PF_LK\n");
#else //         NO_PF_LK
    printf("# No NO_PF_LK\n");
#endif // #else  NO_PF_LK

#ifdef           PF_LK_PSPLIT_VAL
    printf("#    PF_LK_PSPLIT_VAL\n");
#else //         PF_LK_PSPLIT_VAL
    printf("# No PF_LK_PSPLIT_VAL\n");
#endif // #else  PF_LK_PSPLIT_VAL

#ifdef           PF_LK_NEXT_VAL
    printf("#    PF_LK_NEXT_VAL\n");
#else //         PF_LK_NEXT_VAL
    printf("# No PF_LK_NEXT_VAL\n");
#endif // #else  PF_LK_NEXT_VAL

#ifdef           PF_LK_PREV_VAL
    printf("#    PF_LK_PREV_VAL\n");
#else //         PF_LK_PREV_VAL
    printf("# No PF_LK_PREV_VAL\n");
#endif // #else  PF_LK_PREV_VAL

#ifdef           PF_LK_8_BEG_VAL
    printf("#    PF_LK_8_BEG_VAL\n");
#else //         PF_LK_8_BEG_VAL
    printf("# No PF_LK_8_BEG_VAL\n");
#endif // #else  PF_LK_8_BEG_VAL

#ifdef           PF_LK_8_END_VAL
    printf("#    PF_LK_8_END_VAL\n");
#else //         PF_LK_8_END_VAL
    printf("# No PF_LK_8_END_VAL\n");
#endif // #else  PF_LK_8_END_VAL

#ifdef           LKIL8_ONE_BUCKET
    printf("#    LKIL8_ONE_BUCKET\n");
#else //         LKIL8_ONE_BUCKET
    printf("# No LKIL8_ONE_BUCKET\n");
#endif // #else  LKIL8_ONE_BUCKET

#ifdef           TEST_END_BEFORE_BUCKET
    printf("#    TEST_END_BEFORE_BUCKET\n");
#else //         TEST_END_BEFORE_BUCKET
    printf("# No TEST_END_BEFORE_BUCKET\n");
#endif // #else  TEST_END_BEFORE_BUCKET

#ifdef           TEST_NPOS_BEFORE_END
    printf("#    TEST_NPOS_BEFORE_END\n");
#else //         TEST_NPOS_BEFORE_END
    printf("# No TEST_NPOS_BEFORE_END\n");
#endif // #else  TEST_NPOS_BEFORE_END

#ifdef           PF_EK_VAL
    printf("#    PF_EK_VAL\n");
#else //         PF_EK_VAL
    printf("# No PF_EK_VAL\n");
#endif // #else  PF_EK_VAL

#ifdef           PF_UNPACKED_BM_VAL
    printf("#    PF_UNPACKED_BM_VAL\n");
#else //         PF_UNPACKED_BM_VAL
    printf("# No PF_UNPACKED_BM_VAL\n");
#endif // #else  PF_UNPACKED_BM_VAL

#ifdef           PF_BM_PSPLIT_VAL
    printf("#    PF_BM_PSPLIT_VAL\n");
#else //         PF_BM_PSPLIT_VAL
    printf("# No PF_BM_PSPLIT_VAL\n");
#endif // #else  PF_BM_PSPLIT_VAL

#ifdef           PF_BM_NEXT_VAL
    printf("#    PF_BM_NEXT_VAL\n");
#else //         PF_BM_NEXT_VAL
    printf("# No PF_BM_NEXT_VAL\n");
#endif // #else  PF_BM_NEXT_VAL

#ifdef           PF_BM_PREV_VAL
    printf("#    PF_BM_PREV_VAL\n");
#else //         PF_BM_PREV_VAL
    printf("# No PF_BM_PREV_VAL\n");
#endif // #else  PF_BM_PREV_VAL

#ifdef           PF_BM_PREV_HALF_VAL
    printf("#    PF_BM_PREV_HALF_VAL\n");
#else //         PF_BM_PREV_HALF_VAL
    printf("# No PF_BM_PREV_HALF_VAL\n");
#endif // #else  PF_BM_PREV_HALF_VAL

#ifdef           NO_PF_BM_PREV_HALF_VAL
    printf("#    NO_PF_BM_PREV_HALF_VAL\n");
#else //         NO_PF_BM_PREV_HALF_VAL
    printf("# No NO_PF_BM_PREV_HALF_VAL\n");
#endif // #else  NO_PF_BM_PREV_HALF_VAL

#ifdef           PF_BM_NEXT_HALF_VAL
    printf("#    PF_BM_NEXT_HALF_VAL\n");
#else //         PF_BM_NEXT_HALF_VAL
    printf("# No PF_BM_NEXT_HALF_VAL\n");
#endif // #else  PF_BM_NEXT_HALF_VAL

#ifdef           NO_PF_BM_NEXT_HALF_VAL
    printf("#    NO_PF_BM_NEXT_HALF_VAL\n");
#else //         NO_PF_BM_NEXT_HALF_VAL
    printf("# No NO_PF_BM_NEXT_HALF_VAL\n");
#endif // #else  NO_PF_BM_NEXT_HALF_VAL

#ifdef           PREREAD_BM_PSPLIT_VAL
    printf("#    PREREAD_BM_PSPLIT_VAL\n");
#else //         PREREAD_BM_PSPLIT_VAL
    printf("# No PREREAD_BM_PSPLIT_VAL\n");
#endif // #else  PREREAD_BM_PSPLIT_VAL

#ifdef           PF_PWR
    printf("#    PF_PWR\n");
#else //         PF_PWR
    printf("# No PF_PWR\n");
#endif // #else  PF_PWR

#if defined(BUILTIN_PREFETCH_0)
    printf("# BUILTIN_PREFETCH_0\n");
#elif defined(BUILTIN_PREFETCH_1)
    printf("# BUILTIN_PREFETCH_1\n");
#elif defined(BUILTIN_PREFETCH_2)
    printf("# BUILTIN_PREFETCH_2\n");
#elif defined(BUILTIN_PREFETCH_3)
    printf("# BUILTIN_PREFETCH_3\n");
#elif defined(INTEL_PREFETCH_NTA)
    printf("# INTEL_PREFETCH_NTA\n");
#elif defined(INTEL_PREFETCH_T0)
    printf("# INTEL_PREFETCH_T0\n");
#elif defined(INTEL_PREFETCH_T1)
    printf("# INTEL_PREFETCH_T1\n");
#elif defined(INTEL_PREFETCH_T2)
    printf("# INTEL_PREFETCH_T1\n");
#endif

#if defined(USE_XX_SW)
    printf("#    USE_XX_SW\n");
#else // defined(USE_XX_SW)
    printf("# No USE_XX_SW\n");
#endif // defined(USE_XX_SW)

#ifdef           RIGID_XX_SW
    printf("#    RIGID_XX_SW\n");
#else //         RIGID_XX_SW
    printf("# No RIGID_XX_SW\n");
#endif //        RIGID_XX_SW

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

#ifdef           USE_LOWER_XX_SW
    printf("#    USE_LOWER_XX_SW\n");
#else //         USE_LOWER_XX_SW
    printf("# No USE_LOWER_XX_SW\n");
#endif //        USE_LOWER_XX_SW

#ifdef           USE_XX_SW_ONLY_AT_DL2
    printf("#    USE_XX_SW_ONLY_AT_DL2\n");
#else //         USE_XX_SW_ONLY_AT_DL2
    printf("# No USE_XX_SW_ONLY_AT_DL2\n");
#endif //        USE_XX_SW_ONLY_AT_DL2

#ifdef           USE_XX_SW_ALWAYS
    printf("#    USE_XX_SW_ALWAYS\n");
#else //         USE_XX_SW_ALWAYS
    printf("# No USE_XX_SW_ALWAYS\n");
#endif //        USE_XX_SW_ALWAYS

#if defined(EMBED_KEYS)
    printf("#    EMBED_KEYS\n");
#else // defined(EMBED_KEYS)
    printf("# No EMBED_KEYS\n");
#endif // defined(EMBED_KEYS)

#ifdef           EK_XV
    printf("#    EK_XV\n");
#else //         EK_XV
    printf("# No EK_XV\n");
#endif //        EK_XV

#ifdef           ALIGN_EK_XV
    printf("#    ALIGN_EK_XV\n");
#else //         ALIGN_EK_XV
    printf("# No ALIGN_EK_XV\n");
#endif //        ALIGN_EK_XV

#ifdef           PF_EK_XV
    printf("#    PF_EK_XV\n");
#else //         PF_EK_XV
    printf("# No PF_EK_XV\n");
#endif //        PF_EK_XV

#ifdef           PF_EK_XV_2
    printf("#    PF_EK_XV_2\n");
#else //         PF_EK_XV_2
    printf("# No PF_EK_XV_2\n");
#endif //        PF_EK_XV_2

#if defined(EK_CALC_POP)
    printf("#    EK_CALC_POP\n");
#else // defined(EK_CALC_POP)
    printf("# No EK_CALC_POP\n");
#endif // defined(EK_CALC_POP)

#ifdef           NO_EK_XV_AT_EMBEDDED_BM
    printf("#    NO_EK_XV_AT_EMBEDDED_BM\n");
#else //         NO_EK_XV_AT_EMBEDDED_BM
    printf("# No NO_EK_XV_AT_EMBEDDED_BM\n");
#endif //        NO_EK_XV_AT_EMBEDDED_BM else

#ifdef           _LNX
    printf("#    _LNX\n");
#else //         _LNX
    printf("# No _LNX\n");
#endif //        _LNX

#ifdef           REMOTE_LNX
    printf("#    REMOTE_LNX\n");
#else //         REMOTE_LNX
    printf("# No REMOTE_LNX\n");
#endif //        REMOTE_LNX

#ifdef           NO_REMOTE_LNX
    printf("#    NO_REMOTE_LNX\n");
#else //         NO_REMOTE_LNX
    printf("# No NO_REMOTE_LNX\n");
#endif //        NO_REMOTE_LNX

#ifdef           DUMMY_REMOTE_LNX
    printf("#    DUMMY_REMOTE_LNX\n");
#else //         DUMMY_REMOTE_LNX
    printf("# No DUMMY_REMOTE_LNX\n");
#endif //        DUMMY_REMOTE_LNX

#ifdef          _RETURN_NULL_TO_INSERT_AGAIN
    printf("#    RETURN_NULL_TO_INSERT_AGAIN\n");
#else //        _RETURN_NULL_TO_INSERT_AGAIN
    printf("# No RETURN_NULL_TO_INSERT_AGAIN\n");
#endif //       _RETURN_NULL_TO_INSERT_AGAIN

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

#ifdef           PARALLEL_EK_FOR_NEXT
    printf("#    PARALLEL_EK_FOR_NEXT\n");
#else //         PARALLEL_EK_FOR_NEXT
    printf("# No PARALLEL_EK_FOR_NEXT\n");
#endif // #else  PARALLEL_EK_FOR_NEXT

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

#ifdef           SEARCH_USING_LOCATE_GE
    printf("#    SEARCH_USING_LOCATE_GE\n");
#else //         SEARCH_USING_LOCATE_GE
    printf("# No SEARCH_USING_LOCATE_GE\n");
#endif // #else  SEARCH_USING_LOCATE_GE

#if defined(PSPLIT_PARALLEL)
    printf("#    PSPLIT_PARALLEL\n");
#else // defined(PSPLIT_PARALLEL)
    printf("# No PSPLIT_PARALLEL\n");
#endif // defined(PSPLIT_PARALLEL)

#ifdef           PARALLEL_LOCATEKEY_8
    printf("#    PARALLEL_LOCATEKEY_8\n");
#else //         PARALLEL_LOCATEKEY_8
    printf("# No PARALLEL_LOCATEKEY_8\n");
#endif // #else  PARALLEL_LOCATEKEY_8

#ifdef           LOCATE_GE_USING_EQ_M1
    printf("#    LOCATE_GE_USING_EQ_M1\n");
#else //         LOCATE_GE_USING_EQ_M1
    printf("# No LOCATE_GE_USING_EQ_M1\n");
#endif // #else  LOCATE_GE_USING_EQ_M1

#ifdef           NO_LOCATE_GE_USING_EQ_M1
    printf("#    NO_LOCATE_GE_USING_EQ_M1\n");
#else //         NO_LOCATE_GE_USING_EQ_M1
    printf("# No NO_LOCATE_GE_USING_EQ_M1\n");
#endif // #else  NO_LOCATE_GE_USING_EQ_M1

#ifdef           DEBUG_LOCATE_GE
    printf("#    DEBUG_LOCATE_GE\n");
#else //         DEBUG_LOCATE_GE
    printf("# No DEBUG_LOCATE_GE\n");
#endif //        DEBUG_LOCATE_GE else

#ifdef           LOCATE_GE_AFTER_LOCATE_EQ
    printf("#    LOCATE_GE_AFTER_LOCATE_EQ\n");
#else //         LOCATE_GE_AFTER_LOCATE_EQ
    printf("# No LOCATE_GE_AFTER_LOCATE_EQ\n");
#endif // #else  LOCATE_GE_AFTER_LOCATE_EQ

#ifdef           NO_LOCATE_GE_AFTER_LOCATE_EQ
    printf("#    NO_LOCATE_GE_AFTER_LOCATE_EQ\n");
#else //         NO_LOCATE_GE_AFTER_LOCATE_EQ
    printf("# No NO_LOCATE_GE_AFTER_LOCATE_EQ\n");
#endif // #else  NO_LOCATE_GE_AFTER_LOCATE_EQ

#ifdef           NO_LOCATE_GE_KEY_X
    printf("#    NO_LOCATE_GE_KEY_X\n");
#else //         NO_LOCATE_GE_KEY_X
    printf("# No NO_LOCATE_GE_KEY_X\n");
#endif // #else  NO_LOCATE_GE_KEY_X

#ifdef           LOCATE_GE_KEY_8
    printf("#    LOCATE_GE_KEY_8\n");
#else //         LOCATE_GE_KEY_8
    printf("# No LOCATE_GE_KEY_8\n");
#endif // #else  LOCATE_GE_KEY_8

#ifdef           LOCATE_GE_KEY_16
    printf("#    LOCATE_GE_KEY_16\n");
#else //         LOCATE_GE_KEY_16
    printf("# No LOCATE_GE_KEY_16\n");
#endif // #else  LOCATE_GE_KEY_16

#ifdef           LOCATE_GE_KEY_24
    printf("#    LOCATE_GE_KEY_24\n");
#else //         LOCATE_GE_KEY_24
    printf("# No LOCATE_GE_KEY_24\n");
#endif // #else  LOCATE_GE_KEY_24

#ifdef           LOCATE_GE_KEY_32
    printf("#    LOCATE_GE_KEY_32\n");
#else //         LOCATE_GE_KEY_32
    printf("# No LOCATE_GE_KEY_32\n");
#endif // #else  LOCATE_GE_KEY_32

#ifdef           PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK
    printf("#    PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK\n");
#else //         PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK
    printf("# No PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK\n");
#endif // #else  PARALLEL_LOCATE_GE_KEY_8_USING_UNPACK

#ifdef           PARALLEL_LOCATE_GE_KEY_16_IN_EK
    printf("#    PARALLEL_LOCATE_GE_KEY_16_IN_EK\n");
#else //         PARALLEL_LOCATE_GE_KEY_16_IN_EK
    printf("# No PARALLEL_LOCATE_GE_KEY_16_IN_EK\n");
#endif // #else  PARALLEL_LOCATE_GE_KEY_16_IN_EK

#ifdef           PARALLEL_LOCATE_GE_KEY_16_USING_UNPACK
    printf("#    PARALLEL_LOCATE_GE_KEY_16_USING_UNPACK\n");
#else //         PARALLEL_LOCATE_GE_KEY_16_USING_UNPACK
    printf("# No PARALLEL_LOCATE_GE_KEY_16_USING_UNPACK\n");
#endif // #else  PARALLEL_LOCATE_GE_KEY_16_USING_UNPACK

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

#ifdef           PARALLEL_256
    printf("#    PARALLEL_256\n");
#else //         PARALLEL_256
    printf("# No PARALLEL_256\n");
#endif //        PARALLEL_256 else

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

#if defined(NO_PARALLEL_HK_128_64)
    printf("#    NO_PARALLEL_HK_128_64\n");
#else // defined(NO_PARALLEL_HK_128_64)
    printf("# No NO_PARALLEL_HK_128_64\n");
#endif // defined(NO_PARALLEL_HK_128_64)

#ifdef           NEW_HK_64
    printf("#    NEW_HK_64\n");
#else //         NEW_HK_64
    printf("# No NEW_HK_64\n");
#endif //        NEW_HK_64 else

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

#ifdef           NO_LKIL8_ONE_BUCKET
    printf("#    NO_LKIL8_ONE_BUCKET\n");
#else //         NO_LKIL8_ONE_BUCKET
    printf("# No NO_LKIL8_ONE_BUCKET\n");
#endif // #else  NO_LKIL8_ONE_BUCKET

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

#ifdef           NO_REVERSE_SORT_EMBEDDED_KEYS
    printf("#    NO_REVERSE_SORT_EMBEDDED_KEYS\n");
#else //         NO_REVERSE_SORT_EMBEDDED_KEYS
    printf("# No NO_REVERSE_SORT_EMBEDDED_KEYS\n");
#endif //        NO_REVERSE_SORT_EMBEDDED_KEYS

#ifdef              REVERSE_SORT_EMBEDDED_KEYS
    printf("#       REVERSE_SORT_EMBEDDED_KEYS\n");
#else //            REVERSE_SORT_EMBEDDED_KEYS
    printf("# No    REVERSE_SORT_EMBEDDED_KEYS\n");
#endif //           REVERSE_SORT_EMBEDDED_KEYS

    printf("# No EXTRA_TYPES\n");

#ifdef           SW_POP_IN_LNX
    printf("#    SW_POP_IN_LNX\n");
#else //         SW_POP_IN_LNX
    printf("# No SW_POP_IN_LNX\n");
#endif //        SW_POP_IN_LNX else

#ifdef           USE_BM_SW_BM_IN_WR_OR_LNX
    printf("#    USE_BM_SW_BM_IN_WR_OR_LNX\n");
#else //         USE_BM_SW_BM_IN_WR_OR_LNX
    printf("# No USE_BM_SW_BM_IN_WR_OR_LNX\n");
#endif //        USE_BM_SW_BM_IN_WR_OR_LNX else

#ifdef           BM_SW_BM_IN_LNX
    printf("#    BM_SW_BM_IN_LNX\n");
#else //         BM_SW_BM_IN_LNX
    printf("# No BM_SW_BM_IN_LNX\n");
#endif //        BM_SW_BM_IN_LNX else

#ifdef           USE_BM_SW_BM_IN_LNX
    printf("#    USE_BM_SW_BM_IN_LNX\n");
#else //         USE_BM_SW_BM_IN_LNX
    printf("# No USE_BM_SW_BM_IN_LNX\n");
#endif //        USE_BM_SW_BM_IN_LNX else

#ifdef           BM_SW_BM_IN_WR_HB
    printf("#    BM_SW_BM_IN_WR_HB\n");
#else //         BM_SW_BM_IN_WR_HB
    printf("# No BM_SW_BM_IN_WR_HB\n");
#endif //        BM_SW_BM_IN_WR_HB else

#ifdef           USE_BM_SW_BM_IN_WR_HB
    printf("#    USE_BM_SW_BM_IN_WR_HB\n");
#else //         USE_BM_SW_BM_IN_WR_HB
    printf("# No USE_BM_SW_BM_IN_WR_HB\n");
#endif //        USE_BM_SW_BM_IN_WR_HB else

  #ifdef CODE_BM_SW
    printf("# cnLogBmSwLinksPerBit %d\n", cnLogBmSwLinksPerBit);
  #endif // CODE_BM_SW

#ifdef           NO_BM_SW_AT_TOP
    printf("#    NO_BM_SW_AT_TOP\n");
#else //         NO_BM_SW_AT_TOP
    printf("# No NO_BM_SW_AT_TOP\n");
#endif //        NO_BM_SW_AT_TOP else

#ifdef           SEARCHMETRICS
    printf("#    SEARCHMETRICS\n");
#else //         SEARCHMETRICS
    printf("# No SEARCHMETRICS\n");
#endif //        SEARCHMETRICS else

#ifdef           DSMETRICS_GETS
    printf("#    DSMETRICS_GETS\n");
#else //         DSMETRICS_GETS
    printf("# No DSMETRICS_GETS\n");
#endif //        DSMETRICS_GETS else

#ifdef           SMETRICS_HITS
    printf("#    SMETRICS_HITS\n");
#else //         SMETRICS_HITS
    printf("# No SMETRICS_HITS\n");
#endif //        SMETRICS_HITS else

#ifdef           NO_SMETRICS_HITS
    printf("#    NO_SMETRICS_HITS\n");
#else //         NO_SMETRICS_HITS
    printf("# No NO_SMETRICS_HITS\n");
#endif //        NO_SMETRICS_HITS

#ifdef           DSMETRICS_HITS
    printf("#    DSMETRICS_HITS\n");
#else //         DSMETRICS_HITS
    printf("# No DSMETRICS_HITS\n");
#endif //        DSMETRICS_HITS

#ifdef           DSMETRICS_NHITS
    printf("#    DSMETRICS_NHITS\n");
#else //         DSMETRICS_NHITS
    printf("# No DSMETRICS_NHITS\n");
#endif //        DSMETRICS_NHITS

#ifdef           SMETRICS_SEARCH_POP
    printf("#    SMETRICS_SEARCH_POP\n");
#else //         SMETRICS_SEARCH_POP
    printf("# No SMETRICS_SEARCH_POP\n");
#endif //        SMETRICS_SEARCH_POP else

#ifdef           SMETRICS_MISCOMPARES
    printf("#    SMETRICS_MISCOMPARES\n");
#else //         SMETRICS_MISCOMPARES
    printf("# No SMETRICS_MISCOMPARES\n");
#endif //        SMETRICS_MISCOMPARES else

#ifdef           SMETRICS_EK
    printf("#    SMETRICS_EK\n");
#else //         SMETRICS_EK
    printf("# No SMETRICS_EK\n");
#endif //        SMETRICS_EK else

#ifdef           SMETRICS_EK_XV
    printf("#    SMETRICS_EK_XV\n");
#else //         SMETRICS_EK_XV
    printf("# No SMETRICS_EK_XV\n");
#endif //        SMETRICS_EK_XV else

#ifdef           SMETRICS_UNPACKED_BM
    printf("#    SMETRICS_UNPACKED_BM\n");
#else //         SMETRICS_UNPACKED_BM
    printf("# No SMETRICS_UNPACKED_BM\n");
#endif //        SMETRICS_UNPACKED_BM else

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

#ifdef           SKIP_TO_SW_SHORTCUT
    printf("#    SKIP_TO_SW_SHORTCUT\n");
#else //         SKIP_TO_SW_SHORTCUT
    printf("# No SKIP_TO_SW_SHORTCUT\n");
#endif //        SKIP_TO_SW_SHORTCUT

#ifdef           SKIP_TO_LIST_SW
    printf("#    SKIP_TO_LIST_SW\n");
#else //         SKIP_TO_LIST_SW
    printf("# No SKIP_TO_LIST_SW\n");
#endif //        SKIP_TO_LIST_SW

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

#ifdef           NEW_NEXT
    printf("#    NEW_NEXT\n");
#else //         NEW_NEXT
    printf("# No NEW_NEXT\n");
#endif //        NEW_NEXT else

#ifdef           NEW_NEXT_EMPTY
    printf("#    NEW_NEXT_EMPTY\n");
#else //         NEW_NEXT_EMPTY
    printf("# No NEW_NEXT_EMPTY\n");
#endif //        NEW_NEXT_EMPTY else

#ifdef           LOOKUP_BEFORE_NEXT_EMPTY
    printf("#    LOOKUP_BEFORE_NEXT_EMPTY\n");
#else //         LOOKUP_BEFORE_NEXT_EMPTY
    printf("# No LOOKUP_BEFORE_NEXT_EMPTY\n");
#endif //        LOOKUP_BEFORE_NEXT_EMPTY else

#ifdef           NO_LOOKUP_BEFORE_PREV_EMPTY
    printf("#    NO_LOOKUP_BEFORE_PREV_EMPTY\n");
#else //         NO_LOOKUP_BEFORE_PREV_EMPTY
    printf("# No NO_LOOKUP_BEFORE_PREV_EMPTY\n");
#endif //        NO_LOOKUP_BEFORE_PREV_EMPTY else

#ifdef           TINY_SUBS_FOR_NEXT_EMPTY
    printf("#    TINY_SUBS_FOR_NEXT_EMPTY\n");
#else //         TINY_SUBS_FOR_NEXT_EMPTY
    printf("# No TINY_SUBS_FOR_NEXT_EMPTY\n");
#endif // #else  TINY_SUBS_FOR_NEXT_EMPTY else

#ifdef           _NO_NEW_NEXT_EMPTY_FOR_JUDY1
    printf("#    _NO_NEW_NEXT_EMPTY_FOR_JUDY1\n");
#else //         _NO_NEW_NEXT_EMPTY_FOR_JUDY1
    printf("# No _NO_NEW_NEXT_EMPTY_FOR_JUDY1\n");
#endif //        _NO_NEW_NEXT_EMPTY_FOR_JUDY1 else

#ifdef           FULL_SW
    printf("#    FULL_SW\n");
#else //         FULL_SW
    printf("# No FULL_SW\n");
#endif //        FULL_SW else

#ifdef           _SKIP_TO_FULL_SW
    printf("#    _SKIP_TO_FULL_SW\n");
#else //         _SKIP_TO_FULL_SW
    printf("# No _SKIP_TO_FULL_SW\n");
#endif //        _SKIP_TO_FULL_SW else

#ifdef           NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
    printf("#    NEXT_EMPTY_TEST_WHOLE_BEFORE_PART\n");
#else //         NEXT_EMPTY_TEST_WHOLE_BEFORE_PART
    printf("# No NEXT_EMPTY_TEST_WHOLE_BEFORE_PART\n");
#endif //        NEXT_EMPTY_TEST_WHOLE_BEFORE_PART else

#ifdef           NEW_NEXT_IS_EXCLUSIVE
    printf("#    NEW_NEXT_IS_EXCLUSIVE\n");
#else //         NEW_NEXT_IS_EXCLUSIVE
    printf("# No NEW_NEXT_IS_EXCLUSIVE\n");
#endif //        NEW_NEXT_IS_EXCLUSIVE else

#ifdef           NEXT_QPA
    printf("#    NEXT_QPA\n");
#else //         NEXT_QPA
    printf("# No NEXT_QPA\n");
#endif // #else  NEXT_QPA

#ifdef           NEXT_QP
    printf("#    NEXT_QP\n");
#else //         NEXT_QP
    printf("# No NEXT_QP\n");
#endif // #else  NEXT_QP

#ifdef           NO_NEW_NEXT
    printf("#    NO_NEW_NEXT\n");
#else //         NO_NEW_NEXT
    printf("# No NO_NEW_NEXT\n");
#endif // #else  NO_NEW_NEXT

#ifdef           NO_NEW_NEXT_EMPTY
    printf("#    NO_NEW_NEXT_EMPTY\n");
#else //         NO_NEW_NEXT_EMPTY
    printf("# No NO_NEW_NEXT_EMPTY\n");
#endif // #else  NO_NEW_NEXT_EMPTY

#if defined(NO_SKIP_AT_TOP)
    printf("#    NO_SKIP_AT_TOP\n");
#else // defined(NO_SKIP_AT_TOP)
    printf("# No NO_SKIP_AT_TOP\n");
#endif // defined(NO_SKIP_AT_TOP)

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

#ifdef           SEARCH_FROM_WRAPPER
    printf("#    SEARCH_FROM_WRAPPER\n");
#else //         SEARCH_FROM_WRAPPER
    printf("# No SEARCH_FROM_WRAPPER\n");
#endif //        SEARCH_FROM_WRAPPER else

#ifdef           NEXT_FROM_WRAPPER
    printf("#    NEXT_FROM_WRAPPER\n");
#else //         NEXT_FROM_WRAPPER
    printf("# No NEXT_FROM_WRAPPER\n");
#endif //        NEXT_FROM_WRAPPER else

#ifdef           NO_NEXT_FROM_WRAPPER
    printf("#    NO_NEXT_FROM_WRAPPER\n");
#else //         NO_NEXT_FROM_WRAPPER
    printf("# No NO_NEXT_FROM_WRAPPER\n");
#endif //        NO_NEXT_FROM_WRAPPER else

#ifdef           NEXT_CHECK_ARGS
    printf("#    NEXT_CHECK_ARGS\n");
#else //         NEXT_CHECK_ARGS
    printf("# No NEXT_CHECK_ARGS\n");
#endif //        NEXT_CHECK_ARGS else

#ifdef           RESTART_UP_FOR_NEXT
    printf("#    RESTART_UP_FOR_NEXT\n");
#else //         RESTART_UP_FOR_NEXT
    printf("# No RESTART_UP_FOR_NEXT\n");
#endif //        RESTART_UP_FOR_NEXT else

#ifdef           RESTART_UP_FOR_NEXT_EMPTY
    printf("#    RESTART_UP_FOR_NEXT_EMPTY\n");
#else //         RESTART_UP_FOR_NEXT_EMPTY
    printf("# No RESTART_UP_FOR_NEXT_EMPTY\n");
#endif //        RESTART_UP_FOR_NEXT_EMPTY else

#ifdef           USE_BM2CNTS_FOR_NEXT_EMPTY
    printf("#    USE_BM2CNTS_FOR_NEXT_EMPTY\n");
#else //         USE_BM2CNTS_FOR_NEXT_EMPTY
    printf("# No USE_BM2CNTS_FOR_NEXT_EMPTY\n");
#endif //        USE_BM2CNTS_FOR_NEXT_EMPTY else

#ifdef           NEXT_SHORTCUT
    printf("#    NEXT_SHORTCUT\n");
#else //         NEXT_SHORTCUT
    printf("# No NEXT_SHORTCUT\n");
#endif //        NEXT_SHORTCUT else

#ifdef           NEXT_SHORTCUT_NULL
    printf("#    NEXT_SHORTCUT_NULL\n");
#else //         NEXT_SHORTCUT_NULL
    printf("# No NEXT_SHORTCUT_NULL\n");
#endif //        NEXT_SHORTCUT_NULL else

#ifdef           NEXT_SHORTCUT_SWITCH
    printf("#    NEXT_SHORTCUT_SWITCH\n");
#else //         NEXT_SHORTCUT_SWITCH
    printf("# No NEXT_SHORTCUT_SWITCH\n");
#endif //        NEXT_SHORTCUT_SWITCH else

#if defined(SEARCH_FROM_WRAPPER_I)
    printf("#    SEARCH_FROM_WRAPPER_I\n");
#else // defined(SEARCH_FROM_WRAPPER_I)
    printf("# No SEARCH_FROM_WRAPPER_I\n");
#endif // defined(SEARCH_FROM_WRAPPER_I)

#ifdef           SWFI_INSERT_AT_LIST
    printf("#    SWFI_INSERT_AT_LIST\n");
#else //         SWFI_INSERT_AT_LIST
    printf("# No SWFI_INSERT_AT_LIST\n");
#endif // #else  SWFI_INSERT_AT_LIST

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

#ifdef           FAST_MALLOC
    printf("#    FAST_MALLOC\n");
#else //         FAST_MALLOC
    printf("# No FAST_MALLOC\n");
#endif //        FAST_MALLOC else

#if defined(MY_MALLOC_ALIGN)
    printf("#    MY_MALLOC_ALIGN\n");
#else // defined(MY_MALLOC_ALIGN)
    printf("# No MY_MALLOC_ALIGN\n");
#endif // defined(MY_MALLOC_ALIGN)

  #ifdef MALLOC_ALIGNMENT
    printf("# MALLOC_ALIGNMENT %d\n", MALLOC_ALIGNMENT);
  #endif // MALLOC_ALIGNMENT

  #ifdef JUDY_MALLOC_ALIGNMENT
    printf("# JUDY_MALLOC_ALIGNMENT %d\n", JUDY_MALLOC_ALIGNMENT);
  #endif // JUDY_MALLOC_ALIGNMENT

  #ifdef JUDY_MALLOC_NUM_SPACES
    printf("# JUDY_MALLOC_NUM_SPACES %d\n", JUDY_MALLOC_NUM_SPACES);
  #endif // JUDY_MALLOC_NUM_SPACES

  #ifdef         EXCLUDE_MALLOC_OVERHEAD
    printf("#    EXCLUDE_MALLOC_OVERHEAD\n");
  #else //       EXCLUDE_MALLOC_OVERHEAD
    printf("# No EXCLUDE_MALLOC_OVERHEAD\n");
  #endif // else EXCLUDE_MALLOC_OVERHEAD

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

// COUNT_2 directs the use of Count with two key parameters.
#ifdef           COUNT_2
    printf("#    COUNT_2\n");
#else //         COUNT_2
    printf("# No COUNT_2\n");
#endif // #else  COUNT_2

#ifdef           COUNT_2_PREFIX
    printf("#    COUNT_2_PREFIX\n");
#else //         COUNT_2_PREFIX
    printf("# No COUNT_2_PREFIX\n");
#endif // #else  COUNT_2_PREFIX

  #ifdef         SW_POP_IN_WR_HB
    printf("#    SW_POP_IN_WR_HB\n");
  #else //       SW_POP_IN_WR_HB
    printf("# No SW_POP_IN_WR_HB\n");
  #endif //      SW_POP_IN_WR_HB else

  #ifdef         GPC_ALL_SKIP_TO_SW_CASES
    printf("#    GPC_ALL_SKIP_TO_SW_CASES\n");
  #else //       GPC_ALL_SKIP_TO_SW_CASES
    printf("# No GPC_ALL_SKIP_TO_SW_CASES\n");
  #endif //      GPC_ALL_SKIP_TO_SW_CASES else

  #ifdef         LIBCMALLOC
    printf("#    LIBCMALLOC\n");
  #else //       LIBCMALLOC
    printf("# No LIBCMALLOC\n");
  #endif //      LIBCMALLOC else

  #ifdef         USE_DLMALLOC_DEFAULT_SIZES
    printf("#    USE_DLMALLOC_DEFAULT_SIZES\n");
  #else //       USE_DLMALLOC_DEFAULT_SIZES
    printf("# No USE_DLMALLOC_DEFAULT_SIZES\n");
  #endif //      USE_DLMALLOC_DEFAULT_SIZES else

#ifdef           NO_BM_DSPLIT
    printf("#    NO_BM_DSPLIT\n");
#else //         NO_BM_DSPLIT
    printf("# No NO_BM_DSPLIT\n");
#endif //        NO_BM_DSPLIT else

#ifdef           NO_PF_BM
    printf("#    NO_PF_BM\n");
#else //         NO_PF_BM
    printf("# No NO_PF_BM\n");
#endif //        NO_PF_BM else

#ifdef           NO_PF_LK_PSPLIT_VAL
    printf("#    NO_PF_LK_PSPLIT_VAL\n");
#else //         NO_PF_LK_PSPLIT_VAL
    printf("# No NO_PF_LK_PSPLIT_VAL\n");
#endif // #else  NO_PF_LK_PSPLIT_VAL

#ifdef           NO_PF_LK_NEXT_VAL
    printf("#    NO_PF_LK_NEXT_VAL\n");
#else //         NO_PF_LK_NEXT_VAL
    printf("# No NO_PF_LK_NEXT_VAL\n");
#endif // #else  NO_PF_LK_NEXT_VAL

#ifdef           NO_PF_LK_PREV_VAL
    printf("#    NO_PF_LK_PREV_VAL\n");
#else //         NO_PF_LK_PREV_VAL
    printf("# No NO_PF_LK_PREV_VAL\n");
#endif // #else  NO_PF_LK_PREV_VAL

#ifdef           NO_PF_LK_8_BEG_VAL
    printf("#    NO_PF_LK_8_BEG_VAL\n");
#else //         NO_PF_LK_8_BEG_VAL
    printf("# No NO_PF_LK_8_BEG_VAL\n");
#endif // #else  NO_PF_LK_8_BEG_VAL

#ifdef           NO_PF_LK_8_END_VAL
    printf("#    NO_PF_LK_8_END_VAL\n");
#else //         NO_PF_LK_8_END_VAL
    printf("# No NO_PF_LK_8_END_VAL\n");
#endif // #else  NO_PF_LK_8_END_VAL

#ifdef           NO_PF_EK_VAL
    printf("#    NO_PF_EK_VAL\n");
#else //         NO_PF_EK_VAL
    printf("# No NO_PF_EK_VAL\n");
#endif // #else  NO_PF_EK_VAL

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

#if defined(NO_PARALLEL_256)
    printf("#    NO_PARALLEL_256\n");
#else // defined(NO_PARALLEL_256)
    printf("# No NO_PARALLEL_256\n");
#endif // defined(NO_PARALLEL_256)

#if defined(NO_PSPLIT_PARALLEL)
    printf("#    NO_PSPLIT_PARALLEL\n");
#else // defined(NO_PSPLIT_PARALLEL)
    printf("# No NO_PSPLIT_PARALLEL\n");
#endif // defined(NO_PSPLIT_PARALLEL)

#ifdef           NO_PARALLEL_LOCATEKEY_8
    printf("#    NO_PARALLEL_LOCATEKEY_8\n");
#else //         NO_PARALLEL_LOCATEKEY_8
    printf("# No NO_PARALLEL_LOCATEKEY_8\n");
#endif // #else  NO_PARALLEL_LOCATEKEY_8

#if defined(NO_LVL_IN_WR_HB)
    printf("#    NO_LVL_IN_WR_HB\n");
#else // defined(NO_LVL_IN_WR_HB)
    printf("# No NO_LVL_IN_WR_HB\n");
#endif // defined(NO_LVL_IN_WR_HB)

#ifdef           _LVL_IN_TYPE
    printf("#    _LVL_IN_TYPE\n");
#else //         _LVL_IN_TYPE
    printf("# No _LVL_IN_TYPE\n");
#endif //        _LVL_IN_TYPE else

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

#ifdef           NO_SW_POP_IN_LNX
    printf("#    NO_SW_POP_IN_LNX\n");
#else //         NO_SW_POP_IN_LNX
    printf("# No NO_SW_POP_IN_LNX\n");
#endif // #else  NO_SW_POP_IN_LNX

#ifdef           NO_BMLF_CNTS
    printf("#    NO_BMLF_CNTS\n");
#else //         NO_BMLF_CNTS
    printf("# No NO_BMLF_CNTS\n");
#endif // #else  NO_BMLF_CNTS

#ifdef           NO_BMLF_CNTS_CUM
    printf("#    NO_BMLF_CNTS_CUM\n");
#else //         NO_BMLF_CNTS_CUM
    printf("# No NO_BMLF_CNTS_CUM\n");
#endif // #else  NO_BMLF_CNTS_CUM

#ifdef           NO_BMLF_CNTS_IN_LNX
    printf("#    NO_BMLF_CNTS_IN_LNX\n");
#else //         NO_BMLF_CNTS_IN_LNX
    printf("# No NO_BMLF_CNTS_IN_LNX\n");
#endif // #else  NO_BMLF_CNTS_IN_LNX

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

#ifdef           NO_PACK_BM_VALUES
    printf("#    NO_PACK_BM_VALUES\n");
#else //         NO_PACK_BM_VALUES
    printf("# No NO_PACK_BM_VALUES\n");
#endif // #else  NO_PACK_BM_VALUES

#ifdef           NO_UNPACK_BM_VALUES
    printf("#    NO_UNPACK_BM_VALUES\n");
#else //         NO_UNPACK_BM_VALUES
    printf("# No NO_UNPACK_BM_VALUES\n");
#endif // #else  NO_UNPACK_BM_VALUES

#ifdef           BMLF_COUNT_CNTS_BACKWARD
    printf("#    BMLF_COUNT_CNTS_BACKWARD\n");
#else //         BMLF_COUNT_CNTS_BACKWARD
    printf("# No BMLF_COUNT_CNTS_BACKWARD\n");
#endif // #else  BMLF_COUNT_CNTS_BACKWARD

#ifdef           BMLF_COUNT_BITS_BACKWARD
    printf("#    BMLF_COUNT_BITS_BACKWARD\n");
#else //         BMLF_COUNT_BITS_BACKWARD
    printf("# No BMLF_COUNT_BITS_BACKWARD\n");
#endif // #else  BMLF_COUNT_BITS_BACKWARD

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

#ifdef           NO_SEARCH_FROM_WRAPPER
    printf("#    NO_SEARCH_FROM_WRAPPER\n");
#else //         NO_SEARCH_FROM_WRAPPER
    printf("# No NO_SEARCH_FROM_WRAPPER\n");
#endif //        NO_SEARCH_FROM_WRAPPER

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
#endif // CODE_BM_SW
    printf("# cnGuardWords %d\n", cnGuardWords);

  #ifdef         cnSwCnts
    printf("#    cnSwCnts %d\n", cnSwCnts);
  #else //       cnSwCnts
    printf("# No cnSwCnts\n");
  #endif //      cnSwCnts else

  #ifdef         cnLogBmlfCnts
    printf("#    cnLogBmlfCnts %d\n", cnLogBmlfCnts);
  #else //       cnLogBmlfCnts
    printf("# No cnLogBmlfCnts\n");
  #endif //      cnLogBmlfCnts else

#if cwListPopCntMax != 0
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
            if (nMalloc > 0x200000 / (int)sizeof(Word_t) - 1) {
                fprintf(stderr,
                        "# Warning. ListWordCnt(nPopCnt %d nBL %d) %d"
                            " is close to 2MB.\n",
                        nPopCnt, nBL, nMalloc);
            }
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
#endif // cwListPopCntMax != 0

#if defined(CODE_XX_SW)
    printf("\n");
    printf("# cnBWMin %d\n", cnBWMin);
    printf("# cnBWIncr %d\n", cnBWIncr);
#endif // defined(CODE_XX_SW)
  #ifdef DOUBLE_DOWN
    printf("# cnBWIncrDD %d\n", cnBWIncrDD);
  #endif // DOUBLE_DOWN

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

#if cwListPopCntMax != 0
    // How big are T_LIST leaves.
    for (int nBL = cnBitsPerWord; nBL >= 8; nBL >>= 1) {
        printf("\n");
        int nWordsPrev = 0, nBoundaries = 0, nWords;
        for (int nPopCnt = 1;
             nBoundaries <= 6 && nPopCnt <= auListPopCntMax[nBL];
             nPopCnt++)
        {
            if (nBL == 9) { continue; } // ? work around monster ?
            if ((nWords = ListWordCnt(nPopCnt, nBL)) != nWordsPrev) {
  #ifdef DEBUG
                if (nWords > 0x200000 / (int)sizeof(Word_t) - 1) {
                    fprintf(stderr,
                            "# Warning. ListWordCnt(nPopCnt %d nBL %d) %d"
                                " is close to 2MB.\n",
                            nPopCnt, nBL, nWords);
                }
  #endif // DEBUG
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
#endif // cwListPopCntMax != 0

  #ifdef BITMAP
    printf("\n");
    int nBLR = cbEmbeddedBitmap ? cnBitsLeftAtDl2 : cnBitsInD1;
    for (int ii = 4; (ii <= 17) && (ii <= (1 << nBLR)); ++ii) {
        printf("# BitmapWordCnt(nBL %d nPopCnt %d) %zd\n",
               nBLR, ii, BitmapWordCnt(nBLR, ii));
    }
  #endif // BITMAP

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
        printf("# nDLR_to_nBW(%d) %d\n", dd, nDLR_to_nBW(dd));
    }

    printf("\n# cnBitsTypeMask %d\n", cnBitsTypeMask);
    printf("\n# cnTypeMask 0x%02x\n", cnTypeMask);
  #ifdef SKIP_LINKS
    assert(T_SKIP_TO_SWITCH <= cnTypeMask);
  #elif defined(CODE_XX_SW) // SKIP_LINKS
    assert(T_XX_SW <= cnTypeMask);
  #else // SKIP_LINKS elif CODE_XX_SW
    assert(T_SWITCH <= cnTypeMask);
      #ifdef FULL_SW
    assert(T_FULL_SW <= cnTypeMask);
          #ifdef _SKIP_TO_FULL_SW
    assert(T_SKIP_TO_FULL_SW <= cnTypeMask);
          #endif // _SKIP_TO_FULL_SW
      #endif // FULL_SW
  #endif // SKIP_LINKS elif CODE_XX_SW else

    printf("\n");

    // Print the type values.

    printf("# Link types:\n\n");
#if defined(SEPARATE_T_NULL)
    printf("# 0x%x %s\n", T_NULL, "T_NULL");
#endif // defined(SEPARATE_T_NULL)
#if (cwListPopCntMax != 0)
    printf("# 0x%x %s\n", T_LIST, "T_LIST");
  #ifdef XX_LISTS
    printf("# 0x%x %s\n", T_XX_LIST, "T_XX_LIST");
  #endif // XX_LISTS
#if defined(SKIP_TO_LIST)
    printf("# 0x%x %s\n", T_SKIP_TO_LIST, "T_SKIP_TO_LIST");
#endif // defined(SKIP_TO_LIST)
#if defined(UA_PARALLEL_128)
    printf("# 0x%x %s\n", T_LIST_UA, "T_LIST_UA");
#endif // defined(UA_PARALLEL_128)
#endif // (cwListPopCntMax != 0)
#if defined(BITMAP)
    printf("# 0x%x %s\n", T_BITMAP, "T_BITMAP");
  #if defined(SKIP_TO_BITMAP)
    printf("# 0x%x %s\n", T_SKIP_TO_BITMAP, "T_SKIP_TO_BITMAP");
  #endif // defined(SKIP_TO_BITMAP)
  #if defined(UNPACK_BM_VALUES)
    printf("# 0x%x %s\n", T_UNPACKED_BM, "T_UNPACKED_BM");
  #endif // defined(UNPACK_BM_VALUES)
#endif // defined(BITMAP)
#if defined(CODE_LIST_SW)
    printf("# 0x%x %s\n", T_LIST_SW, "T_LIST_SW");
#endif // defined(CODE_LIST_SW)
#if defined(SKIP_TO_LIST_SW)
    printf("# 0x%x %s\n", T_SKIP_TO_LIST_SW, "T_SKIP_TO_LIST_SW");
#endif // defined(SKIP_TO_LIST_SW)
#if defined(CODE_BM_SW)
    printf("# 0x%x %s\n", T_BM_SW, "T_BM_SW");
#endif // defined(CODE_BM_SW)
#if defined(SKIP_TO_BM_SW)
    printf("# 0x%x %s\n", T_SKIP_TO_BM_SW, "T_SKIP_TO_BM_SW");
#endif // defined(SKIP_TO_BM_SW)
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
    printf("# 0x%x %s\n", T_FULL_BM_SW, "T_FULL_BM_SW");
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
#if defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
  #ifdef SKIP_TO_BM_SW
    printf("# 0x%x %s\n", T_SKIP_TO_FULL_BM_SW, "T_SKIP_TO_FULL_BM_SW");
  #endif // SKIP_TO_BM_SW
#endif // defined(RETYPE_FULL_BM_SW) && ! defined(USE_BM_IN_NON_BM_SW)
#if defined(EMBED_KEYS)
    printf("# 0x%x %s\n", T_EMBEDDED_KEYS, "T_EMBEDDED_KEYS");
      #ifdef EK_XV
    printf("# 0x%x %s\n", T_EK_XV, "T_EK_XV");
      #endif // EK_XV
#endif // defined(EMBED_KEYS)
    printf("# 0x%x %s\n", T_SWITCH, "T_SWITCH");
    assert(T_SWITCH < EXP(cnBitsMallocMask));
  #ifdef FULL_SW
    printf("# 0x%x %s\n", T_FULL_SW, "T_FULL_SW");
    assert(T_FULL_SW < EXP(cnBitsMallocMask));
      #ifdef _SKIP_TO_FULL_SW
    printf("# 0x%x %s\n", T_SKIP_TO_FULL_SW, "T_SKIP_TO_FULL_SW");
    assert(T_SKIP_TO_FULL_SW < EXP(cnBitsMallocMask));
      #endif // _SKIP_TO_FULL_SW
  #endif // FULL_SW
#if defined(CODE_XX_SW)
    printf("# 0x%x %s\n", T_XX_SW, "T_XX_SW");
    assert(T_XX_SW < EXP(cnBitsMallocMask));
#endif // defined(CODE_XX_SW)
#if defined(SKIP_TO_XX_SW) // doesn't work yet
    printf("# 0x%x %s\n", T_SKIP_TO_XX_SW, "T_SKIP_TO_XX_SW");
    assert(T_SKIP_TO_XX_SW < EXP(cnBitsMallocMask));
#endif // defined(SKIP_TO_XX_SW) // doesn't work yet
#if defined(SKIP_LINKS)
    printf("# 0x%x %s\n", T_SKIP_TO_SWITCH, "T_SKIP_TO_SWITCH");
    assert(T_SKIP_TO_SWITCH < EXP(cnBitsMallocMask));
#endif // defined(SKIP_LINKS)
  #ifdef ALL_SKIP_TO_SW_CASES
  #ifdef AUGMENT_TYPE
    printf("\n");
    printf("# NUM_TYPES %d\n", NUM_TYPES);
  #endif // AUGMENT_TYPE
  #endif // ALL_SKIP_TO_SW_CASES
    printf("\n");
    printf("# WROOT_NULL 0x%zx\n", (Word_t)WROOT_NULL);
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

    DBGR(printf("# JudyXFreeArray\n"));

    // A real user shouldn't pass NULL to Judy1FreeArray.
    // Judy1LHTime uses NULL to give us an opportunity to print
    // configuration info into the log file before we start testing.
    if (PPArray == NULL) { Initialize(); return 0; }

    if (*PPArray == 0) { return 0; }

#if (cnDigitsPerWord != 1)

  // Judy1LHTime and Judy1LHCheck put a -1 word before and after the root
  // word of the array solely so we can make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    Word_t *pwRoot = (Word_t*)PPArray;
    assert(pwRoot[-1] == (Word_t)-1);
    assert(pwRoot[ 1] == (Word_t)-1);
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

    Word_t wBytes = FreeArrayGuts(/* nBL */ cnBitsPerWord,
                                  (Word_t*)PPArray,
  #ifdef REMOTE_LNX
                                  /* pwLnX */ NULL,
  #endif // REMOTE_LNX
                                  /* wKey/Prefix */ 0, /* bDump */ 0
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

  // Judy1LHTime and Judy1LHCheck put a -1 word before and after the root
  // word of the array solely so we can make sure we don't corrupt it.
  #if defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)
    assert(pwRoot[-1] == (Word_t)-1);
    assert(pwRoot[ 1] == (Word_t)-1);
  #endif // defined(DEBUG) && !defined(NO_ROOT_WORD_CHECK)

    return wBytes;

#else // (cnDigitsPerWord != 1)
    JudyFree((Word_t)*PPArray, EXP(cnBitsPerWord - cnLogBitsPerWord));
    return EXP(cnBitsPerWord - cnLogBitsPerByte);
#endif // (cnDigitsPerWord != 1)
}

static Word_t
GetPopCntX(qpa, Word_t wPrefix)
{
    qva; (void)wPrefix;
    DBGC(Log(qy, "GetPopCntX"));
    DBGC(printf("# wPrefix 0x%zx\n", wPrefix));
  #ifdef XX_LISTS
    if (wr_nType(wRoot) == T_XX_LIST) {
        return SubexpansePopCnt(qy, wPrefix);
    }
  #endif // XX_LISTS
    return GetPopCnt(qya);
}

// NextGuts(wRoot, nBL, pwKey, wSkip, bPrev, bEmpty
//   #ifdef B_JUDYL
//        , ppwVal
//   #endif // B_JUDYL
//          )
//
// For bPrev == 0:
//
// Find the (wSkip+1)'th present key greater than or equal to *pwKey.
// Return 0 if such a key exists.
// Otherwise return the amount of the population deficiency, i.e. if
// able to find the n'th present key but not able to find the (n+1)'th
// present key, then return n - wSkip.
//
// If wSkip=0, then find the 1st present key equal to or bigger than *pwKey.
// If wSkip=1, then find the 2nd present key equal to or bigger than *pwKey.
// If wSkip=2, then find the 3rd present key equal to or bigger then *pwKey.
//
// For bPrev == 1 change "bigger" to "smaller" in the above.
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
Word_t
NextGuts(qpa, Word_t *pwKey, Word_t wSkip, int bPrev, int bEmpty
  #ifdef B_JUDYL
       , Word_t **ppwVal
  #endif // B_JUDYL
         )
{
    qva;
    int nBLR = nBL;
#define A(_zero) assert(_zero)
    (void)bEmpty;
    DBGN(printf("NextGuts(pLn %p pwRoot %p)\n", pLn, pwRoot));
    DBGN(printf("NextGuts(wRoot " OWx" nBL %d *pwKey " OWx
                    " wSkip %" _fw"d bPrev %d bEmpty %d)\n",
                wRoot, nBL, *pwKey, wSkip, bPrev, bEmpty));
    Word_t *pwBitmap; (void)pwBitmap;
    int nBitNum; (void)nBitNum; // BITMAP
  #ifdef BITMAP
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        pwBitmap = (Word_t*)pLn;
        goto t_bitmapA;
    }
  #endif // BITMAP
    DBGN(printf("nBL %d pLn 0x%zx pwRoot 0x%zx nType %d pwr 0x%zx\n",
                nBL, (size_t)pLn, (size_t)pwRoot, nType, (size_t)pwr));
    switch (nType) {
  #ifdef UA_PARALLEL_128
    case T_LIST_UA:
  #endif // UA_PARALLEL_128
  #ifdef XX_LISTS
    case T_XX_LIST:
  #endif // XX_LISTS
  #if cwListPopCntMax != 0
    case T_LIST: {
        DBGN(printf("T_LIST nType %d\n", nType));
        //A(0);
        if (pwr == NULL) {
            assert(wRoot == WROOT_NULL);
            //A(0); // startup
            return wSkip + 1;
        }
        assert(wRoot != WROOT_NULL);
        //A(0);
        nBLR = gnListBLR(qy);
        int nPos = SearchList(qya, nBLR, *pwKey);
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
            *pwKey &= ~NZ_MSK(nBLR);
            int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);
            *pwKey |= ls_pxKeyX(pwr, nBLR, nPopCnt, nPos - wSkip);
#ifdef B_JUDYL
            //*ppwVal = &ls_pxKeyX(pwr, nBLR, nPopCnt)[~(nPos - wSkip)];
#endif // B_JUDYL
        } else {
            //A(0);
            if (nPos < 0) { /*A(0);*/ nPos ^= -1; }
            //A(0);
            int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);
            if (nPos + wSkip >= (Word_t)nPopCnt) {
                //A(0);
                return nPos + wSkip - ((Word_t)nPopCnt - 1);
            }
            //A(0);
            *pwKey &= ~NZ_MSK(nBLR);
            *pwKey |= ls_pxKeyX(pwr, nBLR, nPopCnt, nPos + wSkip);
#ifdef B_JUDYL
            //*ppwVal = &ls_pxKeyX(pwr, nBLR, nPopCnt)[~(nPos + wSkip)];
#endif // B_JUDYL
        }
        //A(0);
        return 0;
    }
  #endif // cwListPopCntMax != 0
  #ifdef EK_XV
    case T_EK_XV:
        DBGN(printf("T_EK_XV: *pwKey " OWx" wSkip %" _fw"u\n",
                    *pwKey, wSkip));
        goto t_embedded_keys;
  #endif // EK_XV
  #if defined(EMBED_KEYS)
    case T_EMBEDDED_KEYS: {
        DBGN(printf("T_EMBEDDED_KEYS: *pwKey " OWx" wSkip %" _fw"u\n",
                    *pwKey, wSkip));
        goto t_embedded_keys;
t_embedded_keys:;
        //A(0); // check -B10 -DS1
        assert(nBL != cnBitsPerWord);
        assert(wRoot != WROOT_NULL); // Our tests aren't blowing this.
        if (wRoot == WROOT_NULL) {
            return wSkip + 1;
        }
        int nPos;
      #ifdef B_JUDYL
        if (nType == T_EMBEDDED_KEYS) {
            Word_t wSuffix = wRoot >> (cnBitsPerWord - nBL);
            if ((*pwKey & NBPW_MSK(nBL)) == wSuffix) {
                nPos = 0;
            } else if ((*pwKey & NBPW_MSK(nBL)) < wSuffix) {
                nPos = ~0;
            } else {
                nPos = ~1;
            }
        } else
      #endif // B_JUDYL
        {
            nPos = SearchEmbeddedX(qya, *pwKey);
        }
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
      #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
            int nPopCntMax = EmbeddedListPopCntMax(nBL);
      #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
            *pwKey &= ~MSK(nBL); // clear low bits
            *pwKey |=
      #ifdef EK_XV
                (nType == T_EK_XV)
                    ? GetBits(*pwLnX, nBL,
                              (nPos - wSkip)
          #if (cnBitsInD1 < cnLogBitsPerByte)
                                  * MAX(8, (1 << (LOG(nBL - 1) + 1)))
          #else // (cnBitsInD1 < cnLogBitsPerByte)
                                  * (1 << (LOG(nBL - 1) + 1))
          #endif // (cnBitsInD1 < cnLogBitsPerByte)
                              )
                    :
      #endif // #else EK_XV
                      GetBits(wRoot, nBL,
                              cnBitsPerWord - nBL *
      #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
                                  (nPopCntMax + wSkip - nPos)
      #else // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
                                  (nPos - wSkip + 1)
      #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS else
                              );
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
      #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
            int nPopCntMax = EmbeddedListPopCntMax(nBL);
      #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
            *pwKey &= ~MSK(nBL);
            *pwKey |=
      #ifdef EK_XV
                (nType == T_EK_XV)
                    ? GetBits(*pwLnX, nBL,
                              (nPos + wSkip)
          #if (cnBitsInD1 < cnLogBitsPerByte)
                                  * MAX(8, (1 << (LOG(nBL - 1) + 1)))
          #else // (cnBitsInD1 < cnLogBitsPerByte)
                                  * (1 << (LOG(nBL - 1) + 1))
          #endif // (cnBitsInD1 < cnLogBitsPerByte)
                              )
                    :
      #endif // #else EK_XV
                      GetBits(wRoot, nBL,
                              cnBitsPerWord - nBL *
      #if !defined(B_JUDYL) && defined(REVERSE_SORT_EMBEDDED_KEYS)
                                  (nPopCntMax - wSkip - nPos)
      #else // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS
                                  (nPos + wSkip + 1)
      #endif // !B_JUDYL && REVERSE_SORT_EMBEDDED_KEYS else
                              );
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
        nBLR = wr_nBLR(wRoot);
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
        goto t_bitmap; // address gcc implicit fall-through warning
    }
      #endif // defined(SKIP_TO_BITMAP)
  #ifdef UNPACK_BM_VALUES
    case T_UNPACKED_BM:
  #endif // UNPACK_BM_VALUES
    case T_BITMAP: {
        goto t_bitmap;
t_bitmap:;
      #ifdef _BMLF_BM_IN_LNX
        pwBitmap = pwLnX;
      #else // _BMLF_BM_IN_LNX
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
      #endif // else _BMLF_BM_IN_LNX
        goto t_bitmapA;
t_bitmapA:;
        nBitNum = *pwKey & MSK(nBLR) & MSK(cnLogBitsPerWord);
        DBGN(printf("T_BITMAP *pwKey " OWx" wSkip %" _fw"u\n", *pwKey, wSkip));
        assert(nBLR != cnBitsPerWord);
        int nWordNum = (*pwKey & MSK(nBLR)) >> cnLogBitsPerWord;
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
                if (++nWordNum
                    >= (int)((cbEmbeddedBitmap && (nBLR <= cnLogBitsPerWord))
                        ? 0 : EXP(nBLR - cnLogBitsPerWord)))
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
        *pwKey &= ~MSK(nBLR);
        *pwKey |= (nWordNum << cnLogBitsPerWord) + nBitNum;
        return 0;
    }
  #endif // BITMAP
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
        DBG((nType != T_SKIP_TO_SWITCH)
          #ifdef _SKIP_TO_FULL_SW
                && (nType != T_SKIP_TO_FULL_SW)
          #endif // _SKIP_TO_FULL_SW
            ? printf("NextGuts: Unhandled nType: %d\n", nType) : 0);
        assert((nType == T_SKIP_TO_SWITCH)
          #ifdef _SKIP_TO_FULL_SW
                || (nType == T_SKIP_TO_FULL_SW)
          #endif // _SKIP_TO_FULL_SW
               );
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
        DBGN(printf("SKIP_TO_SW\n"));
        //A(0);
        nBLR = wr_nBLR(wRoot);
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
        goto t_switch; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_LINKS)
  #ifdef FULL_SW
    case T_FULL_SW:
  #endif // FULL_SW
    case T_SWITCH: {
        goto t_switch;
t_switch:;
        //A(0);
        DBGN(printf("T_SW wSkip %" _fw"u\n", wSkip));
        int nBW = nBLR_to_nBW(nBLR); // bits decoded by switch
        int nBLLoop = nBLR - nBW;
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW);
  #ifdef XX_LISTS
        //Word_t wRootPrev = (Word_t)-1;
  #endif // XX_LISTS
        if (bPrev) {
            //A(0);
            for (; wIndex != (Word_t)-1; wIndex--) {
                //A(0);
  #ifdef REMOTE_LNX
                Word_t* pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t* pwRootLoop = &pLn->ln_wRoot;
                Word_t wPopCnt = GetPopCntX(qyax(Loop), *pwKey);
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
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                        assert(*pwKey & MSK(nBLLoop));
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey |= MSK(nBLLoop); // set low bits below index
                *pwKey -= EXP(nBLLoop); // subtract one from index
            }
            //A(0);
            *pwKey += BPW_EXP(nBLR); // add one to prefix; why ?
            DBGN(printf("T_SW: Failure\n"));
            return wSkip + 1;
        } else {
            //A(0);
    DBGN(printf("NextGuts T_SWITCH !bPrev (pLn %p pwRoot %p)\n", pLn, pwRoot));
            for (; wIndex < EXP(nBW); wIndex++) {
                //A(0);
  #ifdef REMOTE_LNX
                Word_t* pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t* pwRootLoop = &pLn->ln_wRoot;
                Word_t wPopCnt = GetPopCntX(qyax(Loop), *pwKey);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x"
                                    " pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if ((wPopCnt > wSkip) /* || *pwKey & MSK(nBLLoop) */) {
                        //A(0);
                        // next might be in here
                        Word_t wCount;
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                        assert((*pwKey & MSK(nBLLoop)) == 0);
                        wSkip -= wPopCnt;
                    }
                    //A(0);
                    DBGN(printf("T_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0);
                *pwKey &= ~MSK(nBLLoop); // clear low bits below index
                *pwKey += EXP(nBLLoop); // add one to index
            }
            //A(0);
            *pwKey -= BPW_EXP(nBLR); // subtract one from prefix; why ?
                                    // are we just undoing the add from above
                                    // and low bits are already zero?
                                    // restoring prefix to what it was
                                    // originally?
            DBGN(printf("T_SW: Failure *pwKey 0x%016" _fw"x\n", *pwKey));
            return wSkip + 1;
        }
        assert(0); // not expected to get here
        break;
    }
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: {
        DBGN(printf("T_SKIP_TO_BM_SW\n"));
        //A(0);
        nBLR = wr_nBLR(wRoot);
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
        goto t_bm_sw; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(CODE_BM_SW)
    case T_BM_SW: {
        goto t_bm_sw;
t_bm_sw:;
        //A(0); // check -B17
        DBGN(printf("T_BM_SW wSkip %" _fw"u\n", wSkip));
        int nBW = nBLR_to_nBW(nBLR); // bits decoded by switch
        int nBLLoop = nBLR - nBW;
        Word_t *pwBmWords = PWR_pwBm(pwRoot, pwr, nBW);
        int nLinks = BmSwLinkCnt(qya);
        Link_t *pLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW); // digit
        int nBmWordNum = gnWordNumInSwBm(wIndex);
        int nBmBitNum = gnBitNumInSwBmWord(wIndex);
        Word_t wBmWord = pwBmWords[nBmWordNum];
        // find starting link
        Word_t wBmSwIndex = 0; // avoid bogus gcc may be uninitialized warning
        int wBmSwBit;
        BmSwIndex(qya, wIndex, &wBmSwIndex, &wBmSwBit);
        DBGN(printf("T_BM_SW wIndex 0x%zx wBmSwIndex %" _fw"u bLnPresent %d\n",
                    wIndex, wBmSwIndex, wBmSwBit));

        if (bPrev) {
            //A(0); // check -B17
            wBmWord &= NZ_MSK(nBmBitNum + 1); // mask off high bits
            DBGN(printf("T_BM_SW masked wBmWord 0x%016" _fw"x\n", wBmWord));

            Link_t *pLnLoop = &pLinks[wBmSwIndex];
            DBGN(printf("T_BM_SW pLnLoop %p\n", (void *)pLnLoop));
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                // go to beginning of link group
                wIndex &= ~NBPW_MSK(cnLogBmSwLinksPerBit);
                goto BmSwGetPrevIndex;
            }
            Word_t wPopCnt; // declare outside while for goto above
            //A(0); // check -B17
            while (pLnLoop >= pLinks) {
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
                //A(0); // check -B17
  #ifdef REMOTE_LNX
                Word_t* pwLnXLoop = gpwLnX(qy, nLinks, pLnLoop - pLinks);
  #endif // REMOTE_LNX
                wPopCnt = GetPopCnt(qyax(Loop));
  #ifdef BM_SW_FOR_REAL
                assert(wPopCnt != 0 || (cnLogBmSwLinksPerBit != 0));
  #endif // BM_SW_FOR_REAL
                if (wPopCnt != 0) {
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wIndex 0x%" _fw"x pLnLoop->ln_wRoot "
                                OWx"\n", wIndex, pLnLoop->ln_wRoot));
                    DBGN(printf("T_BM_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        // prev might be in here
                        //A(0); // check -B17
                        Word_t wCount;
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                        assert(*pwKey & MSK(nBLLoop));
                        wSkip -= wPopCnt;
                    }
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0); // check -B17
BmSwGetPrevIndex:
                if ((wIndex & NBPW_MSK(cnLogBmSwLinksPerBit)) == 0) {
                    wBmWord &= ~EXP(nBmBitNum); // clear current wIndex bit
                }
                --pLnLoop;
                DBGN(printf("T_BM_SW pLnLoop %p\n", (void *)pLnLoop));
                DBGN(printf("T_BM_SW pLinks %p\n", (void *)pLinks));
                // Find prev set bit in bitmap.
                DBGN(printf("T_BM_SW wBmWord 0x%016" _fw"x\n", wBmWord));
                if (wBmWord != 0) {
                    assert(pLnLoop >= pLinks);
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
                            assert(pLnLoop >= pLinks);
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
                        if (*pwKey & ~NZ_MSK(nBLR))
                        //if (wIndex == 0)
                        {
                            //*pwKey += EXP(nBLR); // ? necessary ? not sure
                            DBGN(printf("T_BM_SW: Failure *pwKey 0x%016" _fw"x"
                                            " ret wSkip+1 %zd\n",
                                        *pwKey, wSkip+1));
                            return wSkip + 1;
                        }
                        wIndex
                            = (nBmWordNum << (nBW - LOG(N_WORDS_SW_BM(nBW))))
                                + nBmBitNum;

                        //wIndex &= MSK(nBLR);
                        DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                        *pwKey &= ~NZ_MSK(nBLR);
                        *pwKey |= MSK(nBLLoop);
                        *pwKey |= wIndex << nBLLoop;
                        DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
                        continue;
                    }
                }
                //A(0); // check -B17
                wIndex
                    = ((nBmWordNum * (cnBitsPerWord >> _SW_BM_HALF_WORDS)
                                + nBmBitNum)
                            << cnLogBmSwLinksPerBit)
                        + ((wIndex - 1) & NBPW_MSK(cnLogBmSwLinksPerBit));
                DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                if (nBLR == cnBitsPerWord) {
                    *pwKey = 0;
                } else {
                    *pwKey &= ~MSK(nBLR);
                }
                *pwKey |= MSK(nBLLoop);
                *pwKey |= wIndex << nBLLoop;
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
            }
            //A(0); // check -B17
            *pwKey += BPW_EXP(nBLR); // ? does this matter ? guess it does
            DBGN(printf("T_BM_SW: Failure *pwKey 0x%016" _fw"x\n", *pwKey));
            return wSkip + 1;
        } else {
    DBGN(printf("NextGuts T_BM_SW !bPrev (pLn %p pwRoot %p)\n", pLn, pwRoot));
  #if defined(OFFSET_IN_SW_BM_WORD)
            // mask off cumulative offset bits
            wBmWord &= ((Word_t)1 << (cnBitsPerWord / 2)) - 1;
  #endif // defined(OFFSET_IN_SW_BM_WORD)
            //A(0); // check -B17
            wBmWord &= ~MSK(nBmBitNum); // mask off low bits
            DBGN(printf("T_BM_SW masked wBmWord 0x%016" _fw"x\n", wBmWord));
            Link_t *pLnLoop = &pLinks[wBmSwIndex];
            DBGN(printf("T_BM_SW pLnLoop %p\n", (void *)pLnLoop));
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                wIndex |= NBPW_MSK(cnLogBmSwLinksPerBit);
                goto BmSwGetNextIndex;
            }
            Word_t wPopCnt; // declare outside while for goto above
            //A(0); // check -B17
            while (pLnLoop < &pLinks[nLinks]) {
                Word_t* pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
                //A(0); // check -B17
  #ifdef REMOTE_LNX
                Word_t* pwLnXLoop = gpwLnX(qy, nLinks, pLnLoop - pLinks);
  #endif // REMOTE_LNX
                wPopCnt = GetPopCnt(qyax(Loop));
  #ifdef BM_SW_FOR_REAL
      #if cnLogBmSwLinksPerBit == 0
                assert(wPopCnt != 0);
      #endif // cnLogBmSwLinksPerBit == 0
  #endif // BM_SW_FOR_REAL
                if (wPopCnt != 0) {
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wIndex 0x%" _fw"x wBmSwIndex 0x%"
                                _fw"x pLnLoop->ln_wRoot " OWx"\n",
                                wIndex, wBmSwIndex, pLnLoop->ln_wRoot));
                    DBGN(printf("T_BM_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if ((wPopCnt > wSkip) /*|| (*pwKey & MSK(nBLLoop))*/) {
                        //A(0); // check -B17
                        DBGN(printf("T_BM_SW: wSkip %" _fw"d *pwKey " OWx"\n",
                                    wSkip, *pwKey));
                        // next might be in here
                        Word_t wCount;
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                        //A(0); // bcheck
                        //assert(*pwKey & MSK(nBLLoop));
                        wSkip -= wPopCnt;
                    }
                    //A(0); // check -B17
                    DBGN(printf("T_BM_SW: wSkip %" _fw"d\n", wSkip));
                }
                //A(0); // check -B17
                ++pLnLoop;
BmSwGetNextIndex:
                if ((wIndex & NBPW_MSK(cnLogBmSwLinksPerBit))
                    == NBPW_MSK(cnLogBmSwLinksPerBit))
                {
                    wBmWord &= ~EXP(nBmBitNum); // clear current link group bit
                }
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
                    while (++nBmWordNum < N_WORDS_SW_BM(nBW)) {
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
                    DBGN(printf("bLoop T_BM_SW no more links"
                                " nBmWordNum %d wIndex " OWx" *pwKey " OWx"\n",
                                nBmWordNum, wIndex, *pwKey));
                    if (nBmWordNum == N_WORDS_SW_BM(nBW)) {
                        return wSkip + 1;
                    }
                }
                //A(0); // check -B17
                wIndex
                    = ((nBmWordNum * (cnBitsPerWord >> _SW_BM_HALF_WORDS)
                                + nBmBitNum)
                            << cnLogBmSwLinksPerBit)
                        + ((wIndex + 1) & NBPW_MSK(cnLogBmSwLinksPerBit));
                DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
                if (nBLR == cnBitsPerWord) {
                    *pwKey = 0;
                } else {
                    *pwKey &= ~MSK(nBLR); // clear low bits
                }
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
                *pwKey |= wIndex << nBLLoop; // set index bits
                DBGN(printf("T_BM_SW *pwKey 0x%016" _fw"x\n", *pwKey));
            }
            //A(0);
            // What is wKey supposed to contain on return if/when count > 0?
            // Undefined? The last key found? Unchanged from entry?
            // Look at the recursive call.
            *pwKey -= BPW_EXP(nBLR);
            DBGN(printf("T_BM_SW: Failure\n"));
            return wSkip + 1;
        }
        assert(0); // not expected to get here
        break; // make compiler happy
    }
  #endif // defined(CODE_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: {
        DBGN(printf("T_SKIP_TO_XX_SW\n"));
        //A(0);
        nBLR = wr_nBLR(wRoot);
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
        goto t_xx_sw;
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        goto t_xx_sw;
t_xx_sw:;
        //A(0);
        int nBW = pwr_nBW(pwRoot); // bits decoded by switch
        DBGN(printf("T_XX_SW nBLR %d nBW %d wSkip %" _fw"u\n",
                    nBLR, nBW, wSkip));
        if (cbEmbeddedBitmap && ((nBLR - nBW) <= cnLogBitsPerLink)) {
            assert((nBLR - nBW) == cnLogBitsPerLink);
            pwBitmap = (Word_t*)((Switch_t*)pwr)->sw_aLinks;
            goto t_bitmapA;
        }
        int nBLLoop = nBLR - nBW;
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW);
  #ifdef XX_LISTS
        //Word_t wRootPrev = (Word_t)-1;
  #endif // XX_LISTS
        if (bPrev) {
            //A(0);
            for (; wIndex != (Word_t)-1; wIndex--) {
                //A(0);
  #ifdef REMOTE_LNX
                Word_t *pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t* pwRootLoop = &pLn->ln_wRoot;
                Word_t wPopCnt = GetPopCntX(qyax(Loop), *pwKey);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex " OWx" pLn->ln_wRoot " OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        //A(0);
                        Word_t wCount;
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                *pwKey |= MSK(nBLLoop);
                *pwKey -= EXP(nBLLoop);
            }
            //A(0);
            *pwKey += BPW_EXP(nBLR);
            DBGN(printf("T_SW: Failure\n"));
            return wSkip + 1;
        } else {
            //A(0);
            for (; wIndex < EXP(nBW); wIndex++) {
                //A(0);
  #ifdef REMOTE_LNX
                Word_t *pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX
                Link_t *pLn = &((Switch_t *)pwr)->sw_aLinks[wIndex];
                Word_t* pwRootLoop = &pLn->ln_wRoot;
                Word_t wPopCnt = GetPopCntX(qyax(Loop), *pwKey);
                if (wPopCnt != 0) {
                    //A(0);
                    DBGN(printf("T_SW: wIndex 0x%" _fw"x pLn->ln_wRoot "
                                OWx"\n",
                                wIndex, pLn->ln_wRoot));
                    DBGN(printf("T_SW: wPopCnt %" _fw"d\n", wPopCnt));
                    if (wPopCnt > wSkip) {
                        //A(0);
                        Word_t wCount;
                        if ((wCount = NextGuts(qyax(Loop),
                                               pwKey, wSkip, bPrev, bEmpty
  #ifdef B_JUDYL
                                             , ppwVal
  #endif // B_JUDYL
                                               ))
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
                *pwKey &= ~MSK(nBLLoop);
                *pwKey += EXP(nBLLoop);
            }
            //A(0);
            *pwKey -= BPW_EXP(nBLR);
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
    BJL(Word_t* pwVal = NULL);
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
    int nBL = cnBitsPerWord;
    Word_t* pwRoot = (Word_t*)&PArray;
  #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
    Link_t* pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    wCount = NextGuts(qya, &wKey, wCount, /* bPrev */ 0, /* bEmpty */ 0
  #ifdef B_JUDYL
                                             , &pwVal
  #endif // B_JUDYL
                                               );
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
// Otherwise find the next smaller key than *pwKey which is in the array.
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is smaller than the smallest key in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
#ifdef B_JUDYL
PPvoid_t
JudyLLast(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1Last(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    BJL(Word_t* pwVal = NULL);
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
    int nBL = cnBitsPerWord;
    Word_t* pwRoot = (Word_t*)&PArray;
  #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
    Link_t* pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t wCount = NextGuts(qya, &wKey,
                             /* wCount */ 0, /* bPrev */ 1, /* bEmpty */ 0
  #ifdef B_JUDYL
                                             , &pwVal
  #endif // B_JUDYL
                                               );
    if (wCount == 0) {
        *pwKey = wKey;
        DBGN(printf("JxL done: *pwKey " OWx"\n", *pwKey));
#ifdef B_JUDYL
        if (pwVal != NULL) { return (PPvoid_t)pwVal; }
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
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is smaller than or equal to the smallest key
// in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
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
// Otherwise find the next bigger (or smaller if bPrev) number than *pwKey
// which is not in the array.
// Put the found number in *pwKey.
// Return Success if a number is found.
// Return Failure if all keys bigger (or smaller if bPrev) than or equal
// to *pwKey are in the array.
// *pwKey is undefined if Failure is returned.
Status_t
NextEmptyGuts(qpa, Word_t *pwKey, int bPrev)
{
    qva;
    DBGN(printf("NextEmptyGuts(pwRoot %p *pwKey 0x%zx nBL %d bPrev %d)"
                    " wRoot 0x%zx\n",
                (void *)pwRoot, *pwKey, nBL, bPrev, wRoot));
    int nBLR = nBL;
    int nBitNum; (void)nBitNum; // BITMAP
    Word_t *pwBitmap; (void)pwBitmap;
  #ifdef BITMAP
    if (cbEmbeddedBitmap && (nBL <= cnLogBitsPerLink)) {
        pwBitmap = (Word_t*)pLn;
//printf("pLn %p pwBitmap %p *pwBitmap 0x%zx\n", pLn, pwBitmap, *pwBitmap);
        nBitNum = *pwKey & MSK(cnLogBitsPerWord) & MSK(nBL);
        goto embeddedBitmap;
    }
  #endif // BITMAP
  #if cwListPopCntMax != 0 || defined(EMBED_KEYS)
    int nIncr;
  #endif // cwListPopCntMax != 0 || EMBED_KEYS
    DBGN(printf("NEG: nType 0x%x\n", nType));
    switch (nType) {
  #ifdef UA_PARALLEL_128
    case T_LIST_UA:
  #endif // UA_PARALLEL_128
  #ifdef XX_LISTS
    case T_XX_LIST:
  #endif // XX_LISTS
  #if cwListPopCntMax != 0
    case T_LIST:; {
        nBLR = gnListBLR(qy);
  #ifndef BITMAP
        // Checking for a full-pop list is a shortcut that is not necessary
        // for correct behavior. Is it worth it? Under what circumstances?
        if (((wr_nType(WROOT_NULL) != T_LIST)
                || (wRoot != WROOT_NULL))
            && (cnListPopCntMax64 != 0)
            && (nBLR != cnBitsPerWord)
            && (auListPopCntMax[nBLR] >= EXP(nBLR))
            && (GetPopCntX(qya, *pwKey) == EXP(nBLR)))
        {
            DBGI(printf("full-pop list nBL %d nBLR %d GetPopCnt %zd\n",
                   nBL, nBLR, GetPopCntX(qya, *pwKey)));
            return Failure;
        }
  #endif // #ifndef BITMAP
        int nPos;
// Using LocateKeyInList here instead of SearchList breaks
// -DDEBUG -DUSE_XX_SW -DUSE_XX_SW_ONLY_AT_DL2. I don't know why yet.
        if ((pwr == NULL) || ((nPos = SearchList(qya, nBLR, *pwKey)) < 0))
        {
            //printf("key is not in list\n");
            return Success;
        }
        Word_t wKeyLoop = *pwKey;
        wKeyLoop &= NZ_MSK(nBLR);
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
        *pwKey &= ~NZ_MSK(nBLR);
        int nPopCnt = PWR_xListPopCnt(pwRoot, pwr, nBLR);
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
                    if (wKeyLoop != ((Word_t)1 << (nBLR - 1)) * 2) { break; }
                    //printf("ran out of expanse in list\n");
                    //printf("*pwKey 0x%zx nBLR %d\n", *pwKey, nBLR);
                    return Failure;
                }
            }
            Word_t wKeyList = ls_pxKeyX(pwr, nBLR, nPopCnt, nPos);
            wKeyList &= NZ_MSK(nBLR);
            if (wKeyList == wKeyLoop) { continue; }
            *pwKey |= wKeyLoop;
            return Success;
        }
        *pwKey |= wKeyLoop;
        return Success;
    }
  #endif // cwListPopCntMax != 0
  #ifdef EK_XV
    case T_EK_XV:
        goto t_embedded_keys;
  #endif // EK_XV
  #if defined(EMBED_KEYS)
    case T_EMBEDDED_KEYS:; {
        if ((wr_nType(WROOT_NULL) == T_EMBEDDED_KEYS)
            && (wRoot == WROOT_NULL))
        {
            return Success;
        }
        goto t_embedded_keys;
t_embedded_keys:;
        Word_t wKeyLast = *pwKey & ~MSK(nBL); // prefix
        if (bPrev) {
            nIncr = -1;
        } else {
            wKeyLast |= MSK(nBL);
            nIncr = 1;
        }
      #ifdef B_JUDYL
        if (nType == T_EMBEDDED_KEYS) {
            Word_t wSuffix = wRoot >> (cnBitsPerWord - nBL);
            if ((*pwKey & NBPW_MSK(nBL)) == wSuffix) {
                if (*pwKey == wKeyLast) {
                    return Failure;
                }
                *pwKey += nIncr;
            }
        } else
      #endif // B_JUDYL
        while (SearchEmbeddedX(qya, *pwKey) >= 0)
        {
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
        nBLR = wr_nBLR(wRoot);
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
        goto t_bitmap; // address gcc implicit fall-through warning
    }
      #endif // defined(SKIP_TO_BITMAP)
#ifdef UNPACK_BM_VALUES
    case T_UNPACKED_BM:
#endif // UNPACK_BM_VALUES
    case T_BITMAP:; {
        goto t_bitmap;
t_bitmap:;
        if ((wr_nType(WROOT_NULL) == T_BITMAP) && (wRoot == WROOT_NULL)) {
            return Success;
        }
        nBitNum = *pwKey & MSK(nBLR) & MSK(cnLogBitsPerWord);
      #ifdef _BMLF_BM_IN_LNX
        pwBitmap = pwLnX;
      #else // _BMLF_BM_IN_LNX
        pwBitmap = ((BmLeaf_t*)pwr)->bmlf_awBitmap;
      #endif // else _BMLF_BM_IN_LNX
        goto embeddedBitmap;
embeddedBitmap:;
        // skip over the bitmap if it is full pop
        if (nBLR < cnBitsPerWord) {
            assert(nBLR <= nBL);
            if (GetPopCnt(qya) == EXP(nBLR)) {
                if (nBLR != nBL) {
                    //printf("full skip to bitmap not at cnBitsPerWord\n");
                    // We skipped bits to get here.
                    Word_t wKey = *pwKey;
                    if (bPrev) wKey -= EXP(nBLR); else wKey += EXP(nBLR);
                    if (((wKey ^ *pwKey) & ~NZ_MSK(nBL)) == 0) {
                        // There is at least one empty link in a
                        // skipped virtual switch.
                        *pwKey = wKey;
                        return Success;
                    }
                    //printf("nBL %d *pwKey 0x%zx nBLR %d wKey 0x%zx\n",
                    //        nBL, *pwKey, nBLR, wKey);
                }
                return Failure;
            }
        } else {
            // What about array full pop?
        }
        int nWordNum = (*pwKey & MSK(nBLR)) >> cnLogBitsPerWord;
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
                    *pwKey = (*pwKey & ~MSK(nBLR))
                           | (nWordNum << cnLogBitsPerWord) | nBitNum;
                    return Success;
                }
                if (nWordNum-- <= 0) {
                    Word_t wPrefix
                        = (nBLR == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBLR);
                    if ((nBL == cnBitsPerWord) && (wPrefix != 0)) {
                        *pwKey -= EXP(nBLR);
                        *pwKey |= MSK(nBLR);
                        return Success;
                    }
                    return Failure;
                }
                wBm = ~pwBitmap[nWordNum];
            }
        } else {
            // invert bits so empty looks full then keep high bits
            Word_t wBm = ~pwBitmap[nWordNum] & ~MSK(nBitNum);
            if (nBLR < cnLogBitsPerWord) {
                wBm &= MSK(EXP(nBLR));
            }
            for (;;) {
                if (wBm != 0) {
                    nBitNum = __builtin_ctzll(wBm);
                    *pwKey = (*pwKey & ~MSK(nBLR))
                           | (nWordNum << cnLogBitsPerWord) | nBitNum;
                    return Success;
                }
                if (++nWordNum
                    >= (int)((cbEmbeddedBitmap && (nBLR <= cnLogBitsPerWord))
                       ? 0 : EXP(nBLR - cnLogBitsPerWord)))
                {
                    Word_t wPrefix
                        = (nBLR == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBLR);
                    if ((nBL == cnBitsPerWord)
                        && (wPrefix != ((Word_t)-1 << nBLR)))
                    {
                        *pwKey += EXP(nBLR);
                        *pwKey &= ~MSK(nBLR);
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
  #if defined(SEPARATE_T_NULL) || (cwListPopCntMax == 0)
    case T_NULL:
        return Success;
  #endif // SEPARATE_T_NULL || (cwListPopCntMax == 0)
  #if defined(SKIP_LINKS)
    default: {
    /* case T_SKIP_TO_SWITCH */
      #if defined(LVL_IN_WR_HB) || defined(LVL_IN_PP)
        int nType = wr_nType(wRoot); (void)nType;
        DBG((nType != T_SKIP_TO_SWITCH)
          #ifdef _SKIP_TO_FULL_SW
                && (nType != T_SKIP_TO_FULL_SW)
          #endif // _SKIP_TO_FULL_SW
            ? printf("NextEmptyGuts: Unhandled nType: %d\n", nType) : 0);
        assert((nType == T_SKIP_TO_SWITCH)
          #ifdef _SKIP_TO_FULL_SW
                || (nType == T_SKIP_TO_FULL_SW)
          #endif // _SKIP_TO_FULL_SW
               );
      #endif // ! defined(LVL_IN_WR_HB) && ! defined(LVL_IN_PP)
        nBLR = wr_nBLR(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        goto t_switch; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_LINKS)
  #ifdef FULL_SW
    case T_FULL_SW:
  #endif // FULL_SW
    case T_SWITCH: {
        goto t_switch;
t_switch:;
        if ((wr_nType(WROOT_NULL) == T_SWITCH) && (wRoot == WROOT_NULL)) {
            return Success;
        }
        // skip over the switch if it is full pop
        if (nBLR < cnBitsPerWord) {
            if (GetPopCnt(qya) == EXP(nBLR)) {
                if (nBLR != nBL) {
                    //printf("full skip to switch is not at cnBitsPerWord\n");
                    // We skipped bits to get here.
                    Word_t wKey = *pwKey;
                    if (bPrev) wKey -= EXP(nBLR); else wKey += EXP(nBLR);
                    if (((wKey ^ *pwKey) & ~NZ_MSK(nBL)) == 0) {
                        // There is at least one empty link in a
                        // skipped virtual switch.
                        *pwKey = wKey;
                        return Success;
                    }
                    //printf("nBL %d *pwKey 0x%zx nBLR %d wKey 0x%zx\n",
                    //        nBL, *pwKey, nBLR, wKey);
                }
                return Failure;
            }
            //printf("switch is not at cnBitsPerWord and it is not full\n");
        } else {
            //printf("switch is at cnBitsPerWord\n");
        }
        int nBW = nBLR_to_nBW(nBLR); // bits decoded by switch
        int nBLLoop = nBLR - nBW;
        Word_t wPrefix = (nBLR == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBLR);
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW);
        for (;;) {
// I wonder if it would be reasonable and faster to update these
// loop variables by incrementing rather than refiguring them with a
// new index.
            Link_t *pLnLoop = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            Word_t* pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
  #ifdef REMOTE_LNX
            Word_t* pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX


#if 0
    goto t_full_sw;
t_full_sw:; // check for full sub-expanse if cnSwCnts else for full sw
    {
      #if cnSwCnts != 0
        Word_t* pwCnts = ((Switch_t*)pwr)->sw_awCnts;
        if (nBLR <= 16) {
            int nShift
                = (nBW > nLogSwSubCnts(1)) ? (nBW - nLogSwSubCnts(1)) : 0;
            int nCntNum = wIndex >> nShift;
            while (((uint16_t*)pwCnts)[nCntNum--] == EXP(nBLR - nBW + nShift)) {
                if (nCntNum < 0) {
                    *pwKey = (*pwKey & ~MSK(nBLR)) - EXP(nBLR);
                    if (NextEmptyGuts(qyax(Loop), pwKey, bPrev) == Success) {
                        return Success;
                    }
                }
            }
            if (nCntNum + 1 < (int)(wIndex >> nShift)) {
                assert(((nCntNum + 1) << nShift) < (int)wIndex);
                wIndex = (nCntNum + 1) << nShift;
                pLnLoop = &pwr_pLinks((Switch_t *)pwr)[wIndex];
          #ifdef _LNX
                pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
          #endif // _LNX
                *pwKey = (*pwKey & ~MSK(nBLR)) - (wIndex << (nBLR - nBW));
            }
          #if cnBitsPerWord > 32
        } else if (nBLR <= 32) {
            int nShift
                = (nBW > nLogSwSubCnts(2)) ? (nBW - nLogSwSubCnts(2)) : 0;
            int nCntNum = wIndex >> nShift;
            while (((uint32_t*)pwCnts)[nCntNum--] == EXP(nBLR - nBW + nShift)) {
                if (nCntNum < 0) {
                    *pwKey = (*pwKey & ~MSK(nBLR)) - EXP(nBLR);
                    goto break_from_main_switch;
                }
            }
            if (nCntNum + 1 < (int)(wIndex >> nShift)) {
                assert(((nCntNum + 1) << nShift) < (int)wIndex);
                wIndex = (nCntNum + 1) << nShift;
                pLnLoop = &pwr_pLinks((Switch_t *)pwr)[wIndex];
              #ifdef _LNX
                pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
              #endif // _LNX
                *pwKey = (*pwKey & ~MSK(nBLR)) - (wIndex << (nBLR - nBW));
            }
          #endif // cnBitsPerWord > 32
        } else {
          #if cnSwCnts > 1
            int nShift = (nBW > cnLogSwCnts) ? (nBW - cnLogSwCnts) : 0;
            int nCntNum = wIndex >> nShift;
            while (pwCnts[nCntNum--] == EXP(nBLR - nBW + nShift)) {
                if (nCntNum < 0) {
                    if (nBLR == cnBitsPerWord) {
                        return Failure;
                    }
                    *pwKey = (*pwKey & ~NZ_MSK(nBLR)) - EXP(nBLR);
                    goto break_from_main_switch;
                }
            }
            if (nCntNum + 1 < (int)(wIndex >> nShift)) {
                assert(((nCntNum + 1) << nShift) < (int)wIndex);
                wIndex = (nCntNum + 1) << nShift;
                pLnLoop = &pwr_pLinks((Switch_t *)pwr)[wIndex];
                  #ifdef _LNX
                pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
                  #endif // _LNX
                *pwKey = (*pwKey & ~NZ_MSK(nBLR)) - (wIndex << (nBLR - nBW));
            }
          #endif // cnSwCnts > 1
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
        *pwKey = (*pwKey & ~NZ_MSK(nBLR)) + BPW_EXP(nBLR);
        goto break_from_main_switch;
      #endif // cnSwCnts != 0 else
    }
#endif



            if (NextEmptyGuts(qyax(Loop), pwKey, bPrev) == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) {
                    // No links left in switch.
                    // If we skipped to get to this switch and
                    // skipped bits != 0, then we have a winner.
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL)) != 0))
                    {
                        // A(0); // b -1v -S1 -s65536
                        *pwKey -= EXP(nBLR);
                        *pwKey |= MSK(nBLR);
                        return Success;
                    }
                    // A(0); // b -1v -S1
                    return Failure;
                }
                // A(0); // b -1v -S1
                // set all bits in suffix; index and prefix are done later
                *pwKey = MSK(nBLLoop); // suffix
            } else {
                if (++wIndex >= EXP(nBW)) {
                    // No links left in switch.
                    // If we skipped to get to this switch and
                    // skipped bits != ~0 then we have a winner.
                    // A(0); // b -1v -S1
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL))
                            != (NZ_MSK(nBL) - NZ_MSK(nBLR))))
                    {
                        // A(0); // b -1v -S-1 -s0xffffff
                        *pwKey += EXP(nBLR);
                        *pwKey &= ~MSK(nBLR);
                        return Success;
                    }
                    // A(0); // b -1v -S1
                    return Failure;
                }
                // A(0); // b -1v -S1
                // set all bits in suffix; index and prefix are done later
                *pwKey = 0; // suffix
            }
            *pwKey |= wPrefix + (wIndex << nBLLoop);
        }
    }
  #if defined(SKIP_TO_BM_SW)
    case T_SKIP_TO_BM_SW: {
        nBLR = wr_nBLR(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (BmSwitch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        goto t_bm_sw; // address gcc implicit fall-through warning
    }
  #endif // defined(SKIP_TO_BM_SW)
  #if defined(USE_BM_SW)
    case T_BM_SW: {
        goto t_bm_sw;
t_bm_sw:;
        //A(0); // check -B17
        //Word_t wKey = *pwKey;
        int nBW = nBLR_to_nBW(nBLR); // bits decoded by switch
        int nBLLoop = nBLR - nBW;
        Link_t *pLinks = pwr_pLinks((BmSwitch_t *)pwr);
        Word_t wPrefix = (nBLR == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBLR);
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW);
        DBGN(printf("T_BM_SW wIndex 0x%" _fw"x\n", wIndex));
        for (;;) {
            //A(0); // check -B17
            Word_t wBmSwIndex;
            int wBmSwBit;
            BmSwIndex(qya, wIndex, &wBmSwIndex, &wBmSwBit);
            Link_t *pLnLoop = &pLinks[wBmSwIndex];
            Word_t* pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
  #ifdef REMOTE_LNX
            Word_t *pwLnXLoop = gpwLnX(qy, BmSwLinkCnt(qya), wBmSwIndex);
  #endif // REMOTE_LNX
            if ( ! wBmSwBit ) {
                //A(0); // check -B17
                return Success;
            }
            //A(0); // check -B17
            // link is present
            if (NextEmptyGuts(qyax(Loop), pwKey, bPrev) == Success) {
                //A(0); // check -B17
                return Success;
            }
            //A(0); // check -B25
            if (bPrev) {
                //A(0); // check -B17 -S1
                if (wIndex-- <= 0) {
                    //A(0); // check -B17 -S1
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL)) != 0))
                    {
                        *pwKey -= EXP(nBLR);
                        *pwKey |= MSK(nBLR);
                        return Success;
                    }
                    return Failure;
                }
                //A(0); // check -B17 -S1
                *pwKey = MSK(nBLLoop); // suffix
            } else {
                //A(0); // check -B17 -S1
                if (++wIndex >= EXP(nBW)) {
                    //A(0); // check -B17 (with SKIP_TO_BM_SW)
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL))
                            != (NZ_MSK(nBL) - NZ_MSK(nBLR))))
                    {
                        *pwKey += EXP(nBLR);
                        *pwKey &= ~MSK(nBLR);
                        return Success;
                    }
                    return Failure;
                }
                //A(0); // check -B17 -S1
                *pwKey = 0; // suffix
            }
            //A(0); // check -B17 -S1
            *pwKey |= wPrefix + (wIndex << nBLLoop);
        }
        A(0);
        break; // make compiler happy
    }
  #endif // defined(USE_BM_SW)
  #if defined(SKIP_TO_XX_SW)
    case T_SKIP_TO_XX_SW: {
        nBLR = wr_nBLR(wRoot);
        Word_t wPrefix =
      #ifdef PP_IN_LINK
            (nBL >= cnBitsPerWord) ? 0 :
      #endif // PP_IN_LINK
                PWR_wPrefixBL(pwRoot, (Switch_t *)pwr, nBLR);
        if (wPrefix != (*pwKey & ~MSK(nBLR))) {
            return Success;
        }
        assert(*pwKey == (wPrefix | (*pwKey & MSK(nBLR))));
        goto t_xx_sw;
    }
  #endif // defined(SKIP_TO_XX_SW)
  #if defined(USE_XX_SW)
    case T_XX_SW: {
        goto t_xx_sw;
t_xx_sw:;
        int nBW = pwr_nBW(pwRoot); // bits decoded by switch
        int nBLLoop = nBLR - nBW;
        Word_t wPrefix = (nBLR == cnBitsPerWord) ? 0 : *pwKey & ~MSK(nBLR);
        Word_t wIndex = (*pwKey >> nBLLoop) & MSK(nBW);
        for (;;) {
            Link_t *pLnLoop = &((Switch_t *)pwr)->sw_aLinks[wIndex];
            Word_t *pwRootLoop = &pLnLoop->ln_wRoot; (void)pwRootLoop;
  #ifdef REMOTE_LNX
            Word_t* pwLnXLoop = gpwLnX(qy, EXP(nBW), wIndex);
  #endif // REMOTE_LNX
            if (NextEmptyGuts(qyax(Loop), pwKey, bPrev) == Success) {
                return Success;
            }
            if (bPrev) {
                if (wIndex-- <= 0) {
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL)) != 0))
                    {
                        *pwKey -= EXP(nBLR);
                        *pwKey |= MSK(nBLR);
                        return Success;
                    }
                    return Failure;
                }
                *pwKey = MSK(nBLLoop); // suffix
            } else {
                if (++wIndex >= EXP(nBW)) {
                    if ((nBLR != nBL)
                        && ((wPrefix & NZ_MSK(nBL))
                            != (NZ_MSK(nBL) - NZ_MSK(nBLR))))
                    {
                        *pwKey += EXP(nBLR);
                        *pwKey &= ~MSK(nBLR);
                        return Success;
                    }
                    return Failure;
                }
                *pwKey = 0; // suffix
            }
            *pwKey |= wPrefix + (wIndex << nBLLoop);
        }
    }
  #endif // defined(USE_XX_SW)
    }
    assert(0); // not handled yet
    exit(1);
}

// If *pwKey is not in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller number than *pwKey which is not
// in the array.
// Put the found number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys smaller than or equal to *pwKey are in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
int
#ifdef B_JUDYL
JudyLLastEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1LastEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    //DBGN(printf("JxLE: pwKey %p\n", pwKey));
    if (pwKey == NULL) {
        int ret = -1;
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxLE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("JxLE: ret %d\n", ret));
        return ret; // JERRI (for Judy1) or PPJERR (for JudyL)
    }
    DBGN(printf("JxLE: *pwKey 0x%02zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    int nBL = cnBitsPerWord;
    Word_t *pwRoot = (Word_t*)&PArray;
    Link_t* pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t* pwLnX = NULL; (void)pwLnX;
  #ifndef NO_LOOKUP_BEFORE_PREV_EMPTY
      #ifdef B_JUDYL
    if (JudyLGet(PArray, wKeyLocal, NULL) == NULL)
      #else // B_JUDYL
    if (Judy1Test(PArray, wKeyLocal, NULL) == 0)
      #endif // B_JUDYL else
    {
        DBGN(printf("JxLE: returning *pwKey " OWx"\n\n", *pwKey));
        return 1; // found last empty
    }
  #endif // !NO_LOOKUP_BEFORE_PREV_EMPTY
    Status_t status = NextEmptyGuts(qya, &wKeyLocal, /* bPrev */ 1);
    if (status == Success) {
        *pwKey = wKeyLocal;
    }
    int ret = (status == Success);
    DBGN(printf("JxLE: ret %d *pwKey 0x%zx\n", ret, *pwKey));
    return ret;
}

// Find the next smaller number than *pwKey which is not in the array.
// Put the found number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys smaller than *pwKey are in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
int
#ifdef B_JUDYL
JudyLPrevEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1PrevEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("\n\n"));
    //DBGN(printf("JxPE: pwKey %p\n", pwKey));
    if (pwKey == NULL) {
#ifdef B_JUDYL
        return JudyLLastEmpty(PArray, pwKey, PJError);
#else // B_JUDYL
        return Judy1LastEmpty(PArray, pwKey, PJError);
#endif // B_JUDYL
    }
    DBGN(printf("JxPE: *pwKey 0x%02zx\n", *pwKey));
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
    DBGN(printf("JxPE: ret %d *pwKey 0x%zx\n\n", ret, *pwKey));
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

#ifdef XX_LISTS
// Cannot assume wKey exists or not.
// Should we count:
// (a) The keys before the subexpanse containing wKey?
// (b) The keys before wKey in the list?
//     (a) + (d)
// (c) The keys in the list?
//     list pop count
// (d) The keys before wKey in the same subexpanse as wKey?
// (e) The keys in the same subexpanse as wKey?
// (f) The keys including wKey and after in the same subexpanse as wKey?
//     (e) - (d)
// (g) The keys including wKey and after in the list?
//     (c) - (b)
// (h) The keys after wKey in the same subexpanse as wKey?
//     (e) - (d) - bKeyInList
// (i) The keys after wKey in the list?
//     (h) + (k)
// (k) The keys in the subexpanses after the subexpanse containing wKey?
//     (c) - (a) - (e)
// (a), (d), (e) and bKeyInList are certainly sufficient.
// Can we get away with fewer?
// Neither NextGuts nor NextEmptyGuts are using any of it.
// Search gives us (b) and bKeyInList. Is it sufficient for our needs?
// Augmenting Search with any two of (a), (e) and (k) would be thorough.
// Let's figure out if we can get by with Search.
// GetPopCnt uses (e).
int
SubexpansePopCnt(qp, Word_t wKey)
{
    DBGC(Log(qy, "SubexpansePopCnt"));
    DBGC(printf("# wKey 0x%zx\n", wKey));
    qv;
    assert(wRoot != WROOT_NULL);
    assert(nType == T_XX_LIST);
    int nBLR = gnListBLR(qy);
    DBGC(printf("# nBLR %d\n", nBLR));

    // Assert that we have a non-empty link that may be a T_LIST[_UA] link
    // for non-KISS to streamline this function and make caller responsible
    // for not calling it with an empty link.
  #ifdef NO_TYPE_IN_XX_SW
    assert(nBL >= nDL_to_nBL(2));
  #endif // NO_TYPE_IN_XX_SW

    int nPopCnt = PWR_xListPopCnt(&wRoot, pwr, nBLR);
    DBGC(printf("# nPopCnt %d\n", nPopCnt));
    int nPopCntKey = 0;

    // Subexpanse of interest.
    // Can we count on wKey having the correct bits > nBLR? No.
    Word_t wPrefixKey = wKey & ~MSK(nBL);
    wPrefixKey &= NZ_MSK(nBLR);
    DBGC(printf("# wPrefixKey 0x%zx\n", wPrefixKey));

    int nnStart = 0; (void)nnStart;
#if defined(COMPRESSED_LISTS)
    if (nBLR <= (int)sizeof(uint8_t) * 8) {
        uint8_t *pcKeys = ls_pcKeysNATX(pwr, nPopCnt);
// Maybe use psplit search to find the starting point?
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefix = pcKeys[nn] & ~MSK(nBL) & MSK(nBLR);
            if (wPrefix > wPrefixKey) {
                break;
            }
            if (wPrefix == wPrefixKey) {
                ++nPopCntKey;
            }
        }
    } else if (nBLR <= (int)sizeof(uint16_t) * 8) {
        uint16_t *psKeys = ls_psKeysNATX(pwr, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            DBGC(printf("# psKeys[nn] 0x%x\n", psKeys[nn]));
            Word_t wPrefix = psKeys[nn] & ~MSK(nBL) & MSK(nBLR);
            DBGC(printf("# wPrefix 0x%zx\n", wPrefix));
            if (wPrefix > wPrefixKey) {
                break;
            }
            if (wPrefix == wPrefixKey) {
                ++nPopCntKey;
            }
        }
#if (cnBitsPerWord > 32)
    } else if (nBLR <= (int)sizeof(uint32_t) * 8) {
        uint32_t *piKeys = ls_piKeysNATX(pwr, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            Word_t wPrefix = piKeys[nn] & ~MSK(nBL) & MSK(nBLR);
            if (wPrefix > wPrefixKey) {
                break;
            }
            if (wPrefix == wPrefixKey) {
                ++nPopCntKey;
            }
        }
#endif // (cnBitsPerWord > 32)
    } else
#endif // defined(COMPRESSED_LISTS)
    {
        Word_t *pwKeys = ls_pwKeysX(pwr, nBLR, nPopCnt);
        for (int nn = 0; nn < nPopCnt; nn++) {
            DBGC(printf("# pwKeys[nn] 0x%zx\n", pwKeys[nn]));
            Word_t wPrefix = pwKeys[nn] & ~MSK(nBL) & NZ_MSK(nBLR);
            DBGC(printf("# wPrefix 0x%zx\n", wPrefix));
            if (wPrefix > wPrefixKey) {
                break;
            }
            if (wPrefix == wPrefixKey) {
                ++nPopCntKey;
            }
        }
    }
    DBGC(printf("# nPopCntKey %d\n", nPopCntKey));
    return nPopCntKey;
}
#endif // XX_LISTS

