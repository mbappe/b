//      This program tests the accuracy of a Judy1 with a JudyL Array.
//                      -by-
//      Douglas L. Baskins (8/2001)  doug@sourcejudy.com

#include <stdlib.h>             // calloc()
#include <unistd.h>             // getopt()
#include <math.h>               // pow()
#include <stdio.h>              // printf()
#include <assert.h>

#include <Judy.h>

// Judy1LHCheck.c doesn't include bdefines.h so we want to be sure to
// define DEBUG if DEBUG_ALL is defined.
#ifdef DEBUG_ALL
#undef DEBUG
#define DEBUG
#endif // DEBUG_ALL

// The released Judy libraries do not, and some of Doug's work-in-progress
// libraries may not, have Judy1Dump and/or JudyLDump entry points.
// And Mike sometimes links Judy1LHCheck with his own Judy1 library and the
// released or Doug's JudyL or with his own JudyL and the released or
// Doug's Judy1 libraries.
// We want to be able to use the same Check.c for all of these cases.
// The solution is to define JUDY1_V2 and/or JUDY1_DUMP and/or JUDYL_V2
// and/or JUDYL_DUMP if/when we want Check.c to use Judy1Dump and/or
// JudyLDump for real.

#if !defined(JUDY1_V2) && !defined(JUDY1_DUMP)
#define Judy1Dump(wRoot, nBitsLeft, wKeyPrefix)
#endif // !defined(JUDY1_V2) && !defined(JUDY1_DUMP)

#if !defined(JUDYL_V2) && !defined(JUDYL_DUMP)
#define JudyLDump(wRoot, nBitsLeft, wKeyPrefix)
#endif // !defined(JUDYL_V2) && !defined(JUDYL_DUMP)

// Compile:
// # cc -O Judy1LHCheck.c -lm -lJudy -o Judy1LHCheck

// Common macro to handle a failure
#define FAILURE(STR, UL)                                                \
{                                                                       \
printf(         "Error: %s %" PRIuPTR", file='%s', 'function='%s', line %d\n",  \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
fprintf(stderr, "Error: %s %" PRIuPTR", file='%s', 'function='%s', line %d\n",  \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
        exit(1);                                                        \
}

// Structure to keep track of times
typedef struct MEASUREMENTS_STRUCT
{
    Word_t ms_delta;
}
ms_t, *Pms_t;

// Specify prototypes for each test routine
int
NextNumb(Word_t * PNumber, double *PDNumb, double DMult, Word_t MaxNumb);

Word_t TestJudyIns(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements);

Word_t TestJudyDup(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements);

int TestJudyDel(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements);

Word_t TestJudyGet(void *J1, void *JL, void *JH, Word_t Seed, Word_t Elements);

int TestJudyCount(void *J1, void *JL, Word_t LowIndex, Word_t Elements);

#ifdef NO_TEST_NEXT // for turn-on testing
  #define NO_TEST_NEXT_EMPTY // NO_TEST_NEXT ==> NO_TEST_NEXT_EMPTY
#else // NO_TEST_NEXT

Word_t TestJudyNext(void *J1, void *JL, Word_t LowIndex, Word_t Elements);

int TestJudyPrev(void *J1, void *JL, Word_t HighIndex, Word_t Elements);

#endif // #else NO_TEST_NEXT

#ifndef NO_TEST_NEXT_EMPTY // for turn-on testing

int TestJudyNextEmpty(void *J1, void *JL, Word_t LowIndex, Word_t Elements);

int TestJudyPrevEmpty(void *J1, void *JL, Word_t HighIndex, Word_t Elements);

#endif // NO_TEST_NEXT_EMPTY

Word_t MagicList[] =
{
    0,0,0,0,0,0,0,0,0,0, // 0..9
    0x27f,      // 10
    0x27f,      // 11
    0x27f,      // 12
    0x27f,      // 13
    0x27f,      // 14
    0x27f,      // 15
    0x1e71,     // 16
    0xdc0b,     // 17
    0xdc0b,     // 18
    0xdc0b,     // 19
    0xdc0b,     // 20
    0xc4fb,     // 21
    0xc4fb,     // 22
    0xc4fb,     // 23
    0x13aab,    // 24
    0x11ca3,    // 25
    0x11ca3,    // 26
    0x11ca3,    // 27
    0x13aab,    // 28
    0x11ca3,    // 29
    0xc4fb,     // 30
    0xc4fb,     // 31
    0x13aab,    // 32
    0x14e73,    // 33
    0x145d7,    // 34
    0x145f9,    // 35  following tested with Seed=0xc1fc to 35Gig numbers
    0x151ed,    // 36 .. 41
    0x151ed,    // 37
    0x151ed,    // 38
    0x151ed,    // 39  fails at 549751488512 (549Gig)
    0x151ed,    // 40
    0x146c3,    // 41 .. 64
    0x146c3,    // 42
    0x146c3,    // 43
    0x146c3,    // 44
    0x146c3,    // 45
    0x146c3,    // 46
    0x146c3,    // 47
    0x146c3,    // 48
    0x146c3,    // 49
    0x146c3,    // 50
    0x146c3,    // 51
    0x146c3,    // 52
    0x146c3,    // 53
    0x146c3,    // 54
    0x146c3,    // 55
    0x146c3,    // 56
    0x146c3,    // 57
    0x146c3,    // 58
    0x146c3,    // 59
    0x146c3,    // 60
    0x146c3,    // 61
    0x146c3,    // 62
    0x146c3,    // 63
    0x146c3     // 64
};

Word_t BValue    = sizeof(Word_t) * 8;

// Routine to "mirror" the input data word
static Word_t
Swizzle(Word_t word)
{
// BIT REVERSAL, Ron Gutman in Dr. Dobb's Journal, #316, Sept 2000, pp133-136
//

#ifdef __LP64__
    word = ((word & 0x00000000ffffffff) << 32) |
        ((word & 0xffffffff00000000) >> 32);
    word = ((word & 0x0000ffff0000ffff) << 16) |
        ((word & 0xffff0000ffff0000) >> 16);
    word = ((word & 0x00ff00ff00ff00ff) << 8) |
        ((word & 0xff00ff00ff00ff00) >> 8);
    word = ((word & 0x0f0f0f0f0f0f0f0f) << 4) |
        ((word & 0xf0f0f0f0f0f0f0f0) >> 4);
    word = ((word & 0x3333333333333333) << 2) |
        ((word & 0xcccccccccccccccc) >> 2);
    word = ((word & 0x5555555555555555) << 1) |
        ((word & 0xaaaaaaaaaaaaaaaa) >> 1);
#else // __LP64__
    word = ((word & 0x0000ffff) << 16) | ((word & 0xffff0000) >> 16);
    word = ((word & 0x00ff00ff) << 8) | ((word & 0xff00ff00) >> 8);
    word = ((word & 0x0f0f0f0f) << 4) | ((word & 0xf0f0f0f0) >> 4);
    word = ((word & 0x33333333) << 2) | ((word & 0xcccccccc) >> 2);
    word = ((word & 0x55555555) << 1) | ((word & 0xaaaaaaaa) >> 1);
#endif // __LP64__

    word = word >> ((sizeof(Word_t) * 8) - BValue);

    return(word);
}

Word_t dFlag = 1;
Word_t pFlag = 0;
#ifdef NO_TEST_COUNT // for turn-on testing
Word_t CFlag = 1;
#else // NO_TEST_COUNT
Word_t CFlag = 0;
#endif // NO_TEST_COUNT
Word_t DFlag = 0;
Word_t SkipN = 0;               // default == Random skip
Word_t wSplayMask = -(Word_t)1;
Word_t wSplayBase = 0;
Word_t wOffset = 0;
Word_t nElms = 1000000; // Default = 1M
Word_t ErrorFlag = 0;
Word_t TotalIns = 0;
Word_t TotalPop = 0;
Word_t TotalDel = 0;

// Stuff for LFSR (pseudo random number generator)
Word_t RandomBit = (Word_t)~0 / 2 + 1;
Word_t Magic;
Word_t StartSeed = 0xc1fc;      // default beginning number
Word_t FirstSeed;

#undef __FUNCTI0N__
#define __FUNCTI0N__ "Random"

static Word_t                   // Placed here so INLINING compilers get to look at it.
Random(Word_t newseed)
{
    if (newseed & RandomBit)
    {
        newseed += newseed;
        newseed ^= Magic;
    }
    else
    {
        newseed += newseed;
    }
    newseed &= RandomBit * 2 - 1;
    if (newseed == FirstSeed)
    {
        printf("Passed (End of LFSR) Judy1, JudyL, JudyHS tests for %" PRIuPTR" numbers with <= %" PRIdPTR" bits\n", TotalPop, BValue);
        exit(0);
    }
    return(newseed);
}

static inline Word_t
MyPDEP(Word_t wSrc, Word_t wMask)
{
  #ifdef USE_PDEP_INTRINSIC
    // requires gcc -mbmi2
      #if defined(__LP64__) || defined(_WIN64)
    return _pdep_u64(wSrc, wMask);
      #else // defined(__LP64__) || defined(_WIN64)
    return _pdep_u32(wSrc, wMask);
      #endif // defined(__LP64__) || defined(_WIN64)
  #else // USE_PDEP_INTRINSIC
    Word_t wTgt = 0;
      #if defined(SLOW_PDEP) || defined(EXTRA_SLOW_PDEP)
    // This loop assumes popcount(wMask) >= popcount(wSrc).
    for (int nMaskBitNum = 0;; ++nMaskBitNum)
    {
          #ifdef EXTRA_SLOW_PDEP
        int nLsb = __builtin_ctzll(wMask);
        nMaskBitNum += nLsb;
        wTgt |= (wSrc & 1) << nMaskBitNum;
        if ((wSrc >>= 1) == 0) { break; }
        wMask >>= nLsb + 1;
          #else // EXTRA_SLOW_PDEP
        if (wMask & 1)
        {
            wTgt |= (wSrc & 1) << nMaskBitNum;
            if ((wSrc >>= 1) == 0) { break; }
        }
        wMask >>= 1;
          #endif // EXTRA_SLOW_PDEP
    }
      #else // defined(SLOW_PDEP) || defined(EXTRA_SLOW_PDEP)
    do
    {
        Word_t wLsbMask = (-wMask & wMask);
        wTgt |= wLsbMask * (wSrc & 1);
        wMask &= ~wLsbMask;
    } while ((wSrc >>= 1) != 0);
      #endif // defined(SLOW_PDEP) || defined(EXTRA_SLOW_PDEP)
    return wTgt;
  #endif // USE_PDEP_INTRINSIC
}

static Word_t                   // Placed here so INLINING compilers get to look at it.
GetNextIndex(Word_t Index)
{
    if (SkipN)
        Index += SkipN;
    else
        Index = Random(Index);

    return(Index);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "main"

int
main(int argc, char *argv[])
{
//  Names of Judy Arrays
#ifdef DEBUG
    // Make sure the word before and after J1's root word is zero. It's
    // pretty easy in some variants of Mikey's code to introduce a bug that
    // clobbers one or the other so his debug code depends on these words
    // being zero so it can verify that neither is getting clobbered.
    struct { void *pv0, *pv1, *pv2; } sj1 = { (void*)-1, NULL, (void*)-1 };
#define J1 (sj1.pv1)
    struct { void *pv0, *pv1, *pv2; } sjL = { (void*)-1, NULL, (void*)-1 };
#define JL (sjL.pv1)
#else // DEBUG
    void *J1 = NULL;            // Judy1
    void *JL = NULL;            // JudyL
#endif // DEBUG
    void *JH = NULL;            // JudyHS

    double Mult;
    Pms_t Pms;
    Word_t Seed;
    Word_t PtsPdec = 10;        // points per decade
    Word_t Groups;              // Number of measurement groups
    Word_t grp;

    int    c;
    extern char *optarg;

//////////////////////////////////////////////////////////////
// PARSE INPUT PARAMETERS
//////////////////////////////////////////////////////////////

    while ((c = getopt(argc, argv, "n:S:E:e:o:P:b:L:B:pdDC")) != -1)
    {
        switch (c)
        {
        case 'n':               // Number of elements
            nElms = strtoul(optarg, NULL, 0);   // Size of Linear Array
            if (nElms == 0)
                FAILURE("No tests: -n", nElms);

//          Check if more than a trillion (64 bit only)
            if ((double)nElms > 1e12)
                FAILURE("Too many Indexes=", nElms);
            break;

        case 'S':               // Step Size, 0 == Random
            SkipN = strtoul(optarg, NULL, 0);
            break;

        case 'E':
            // wSplayMask must have at least BValue one bits.
            wSplayMask = strtoul(optarg, NULL, 0);
            break;

        case 'e':
            // wSplayBase is used if we want the zero bits in
            // wSplayMask to be something other than zero in the
            // indexes used for testing.
            wSplayBase = strtoul(optarg, NULL, 0);
            break;

        case 'o':
            wOffset = strtoul(optarg, NULL, 0);
            break;

        case 'P':               //
            PtsPdec = strtoul(optarg, NULL, 0);
            break;

        case 'b':               // May not work past 35 bits if changed
            StartSeed = strtoul(optarg, NULL, 0);
            break;

        case 'B':
            BValue = strtoul(optarg, NULL, 0);

            // Allow -B0 to mean -B64 on 64-bit and -B32 on 32-bit.
            // Allow -B-1 to mean -B63 on 64-bit and -B31 on 32-bit.
            // To simplify writing shell scripts for testing that
            // are compatible with 32-bit and 64-bit.
            BValue = (BValue - 1) % (sizeof(Word_t) * 8) + 1;

            if  (
                    (BValue > (sizeof(Word_t) * 8))
                           ||
                    (MagicList[BValue] == 0)
                )
            {
                ErrorFlag++;
                printf("\nIllegal number of random bits of %" PRIuPTR" !!!\n", BValue);
            }
            break;

        case 'p':               // Print test indexes
            pFlag = 1;
            break;

        case 'd':               // Delete indexes
            dFlag = 0;
            break;

        case 'D':               // Swizzle indexes
            DFlag = 1;
            break;

        case 'C':               // Skip counting test.
            CFlag = 1;
            break;

        default:
            ErrorFlag++;
            break;
        }
    }

    if (ErrorFlag)
    {
        printf("\n%s -n# -S# -B# -P# -b # -DRCpd\n\n", argv[0]);
        printf("Where:\n");
        printf("-n <#>  number of indexes used in tests\n");
        printf("-C      skip JudyCount tests\n");
        printf("-p      print index set - for debug\n");
        printf("-d      do not call JudyLDel/Judy1Unset\n");
        printf("-D      Swizzle data (mirror)\n");
        printf("-E <#>  index = PDEP(index, <wSplayMask>)\n");
        printf("-e <#>  index |= (<wSplayBase> & ~wSplayMask)\n");
        printf("-o <#>  index += <wOffset>\n");
        printf("-S <#>  index skip amount, 0 = random\n");
        printf("-B <#>  # bits-1 in random number generator\n");
        printf("-P <#>  number measurement points per decade\n");
        printf("\n");
        printf("For good coverage run:\n");
        printf("# for i in `seq 10 64`; do Judy1LHCheck -B$i      ; done  | grep Pass\n");
        printf("# for i in `seq 10 64`; do Judy1LHCheck -B$i -S1  ; done  | grep Pass\n");
        printf("# for i in `seq 10 64`; do Judy1LHCheck -B$i -DS1 ; done  | grep Pass\n");
        printf("\n");

        exit(1);
    }
//  Set number of Random bits in LFSR
    RandomBit = (Word_t)1 << (BValue - 1);
    Magic     = MagicList[BValue];

    if (nElms > (RandomBit * 2) - 1)
    {
        printf("# Number = -n%" PRIuPTR" of Indexes reduced to max expanse of Random numbers\n", nElms);
        nElms =  (RandomBit * 2) - 1;
    }

    printf("\n%s -n%" PRIuPTR" -S%" PRIuPTR" -B%" PRIuPTR"", argv[0], nElms, SkipN, BValue);

    if (DFlag)
        printf(" -D");
    if (!dFlag)
        printf(" -d");
    if (pFlag)
        printf(" -p");
    if (CFlag)
        printf(" -C");
    printf("\n\n");

    if (sizeof(Word_t) == 8)
        printf("%s 64 Bit version\n", argv[0]);
    else if (sizeof(Word_t) == 4)
        printf("%s 32 Bit version\n", argv[0]);

    // Put run date (not build date) in output.
    printf("# Run date "); fflush(stdout);
    int sysret = system("date"); (void)sysret;

//////////////////////////////////////////////////////////////
// CALCULATE NUMBER OF MEASUREMENT GROUPS
//////////////////////////////////////////////////////////////

//  Calculate Multiplier for number of points per decade
    Mult = pow(10.0, 1.0 / (double)PtsPdec);
    {
        double sum;
        Word_t numb, prevnumb;

//      Count number of measurements needed (10K max)
        sum = numb = 1;
        for (Groups = 2; Groups < 10000; Groups++)
            if (NextNumb(&numb, &sum, Mult, nElms))
                break;

//      Get memory for measurements
        Pms = (Pms_t) calloc(Groups, sizeof(ms_t));

//      Now calculate number of Indexes for each measurement point
        numb = sum = 1;
        prevnumb = 0;
        for (grp = 0; grp < Groups; grp++)
        {
            Pms[grp].ms_delta = numb - prevnumb;
            prevnumb = numb;

            NextNumb(&numb, &sum, Mult, nElms);
        }
    }                           // Groups = number of sizes

    Judy1FreeArray(NULL, NULL);
    JudyLFreeArray(NULL, NULL);

//////////////////////////////////////////////////////////////
// BEGIN TESTS AT EACH GROUP SIZE
//////////////////////////////////////////////////////////////

//  Get the kicker to test the LFSR
    FirstSeed = Seed = StartSeed & (RandomBit * 2 - 1);

    printf("Total Pop Total Ins New Ins Total Del");
    printf(" J1MU/I JLMU/I\n");

#ifdef testLFSR
    {
        Word_t Seed1  = Seed;

        printf("Begin test of LSFR, BValue = %" PRIuPTR"\n", BValue);
        while(1)
        {
            Seed1 = GetNextIndex(Seed1);
            TotalPop++;
            if (TotalPop == 0) printf("BUG!!!\n");
        }
        exit(0);
    }
#endif // testLFSR

    for (grp = 0; grp < Groups; grp++)
    {
        Word_t LowIndex;
        Word_t Delta;
        Word_t NewSeed;

        Delta = Pms[grp].ms_delta;

//      Test JLI, J1S
        NewSeed = TestJudyIns(&J1, &JL, &JH, Seed, Delta);

//      Test JLG, J1T
        // Test/Get the indexes that were just inserted in the same order they
        // were inserted. Return the lowest index tested.
        LowIndex = TestJudyGet(J1, JL, JH, Seed, Delta);

//      Test JLI, J1S -dup
        // Make sure Set fails and Ins returns a pointer to the correct value.
        LowIndex = TestJudyDup(&J1, &JL, &JH, Seed, Delta);

//      Test Judy1Count, JudyLCount
        if (!CFlag)
        {
            TestJudyCount(J1, JL, LowIndex, Delta);
        }
#ifndef NO_TEST_NEXT // for turn-on testing
        Word_t HighIndex;
//      Test JLN, J1N
        HighIndex = TestJudyNext(J1, JL, (Word_t)0, TotalPop);

//      Test JLP, J1P
        TestJudyPrev(J1, JL, (Word_t)~0, TotalPop);
#endif // #ifndef NO_TEST_NEXT
#ifndef NO_TEST_NEXT_EMPTY // for turn-on testing
//      Test JLNE, J1NE
        TestJudyNextEmpty(J1, JL, LowIndex, Delta);

//      Test JLPE, J1PE
        TestJudyPrevEmpty(J1, JL, HighIndex, Delta);
#endif // #ifndef NO_TEST_NEXT_EMPTY

//      Test JLD, J1U
        if (dFlag)
        {
            TestJudyDel(&J1, &JL, &JH, Seed, Delta);
        }

        printf("%9" PRIuPTR" %9" PRIuPTR" %7" PRIuPTR" %9" PRIuPTR"", TotalPop, TotalIns, Delta, TotalDel);
        {
            Word_t Count1, CountL;

//          Print the number of bytes used per Index
//            J1C(Count1, J1, 0, ~0);
            Count1 = Judy1Count(J1, 0, ~0, NULL);
            printf(" %6.3f", (double)Judy1MemUsed(J1) / (double)Count1);
//            JLC(CountL, JL, 0, ~0);
            CountL = JudyLCount(JL, 0, ~0, NULL);
            printf(" %6.3f", (double)JudyLMemUsed(JL) / (double)CountL);
        }
        printf("\n");

//      Advance Index number set
        Seed = NewSeed;
        Word_t Count1, CountL;
        Word_t Bytes;

//        JLC(CountL, JL, 0, ~0);
        CountL = JudyLCount(JL, 0, ~0, NULL);
//        J1C(Count1, J1, 0, ~0);
        Count1 = Judy1Count(J1, 0, ~0, NULL);

        if (CountL != TotalPop)
            FAILURE("JudyLCount wrong", CountL);

        if (Count1 != TotalPop) {
            printf("Count1 %zd TotalPop %zd\n", Count1, TotalPop);
            FAILURE("Judy1Count wrong", Count1);
        }

        if (TotalPop)
        {
            // Mikey's FreeArray code assumes and asserts that there is no
            // unfreed memory when it is done.
            // So we have to free whichever array is being handled by Mikey's
            // Judy library last.

#ifdef TEST_HS
            // JudyHS assumes JudyL does not use the JLAP_INVALID bit (see
            // Judy.h) of an array's root word. JudyHS uses the bit for its
            // own purpose. Since Mikey's JudyL uses (or reserves the right
            // to use) ALL of the bits of the root word it is incompatible
            // with the current implementation of JudyHS.
//            JHSFA(Bytes, JH);   // Free the JudyL Array
            Bytes = JudyHSFreeArray(&JH, NULL);
            if (Bytes == (Word_t)JERR)
                FAILURE("JudyHSFreeArray ret Bytes", Bytes);

            if (pFlag) printf("JudyHSFreeArray = %6.3f Bytes/Index\n",
                   (double)Bytes / (double)TotalPop); // Count not available yet
#endif // TEST_HS

//            J1FA(Bytes, J1);    // Free the Judy1 Array
            Bytes = Judy1FreeArray(&J1, NULL);
            if (Bytes == (Word_t)JERR)
                FAILURE("Judy1FreeArray ret Bytes", Bytes);

            if (pFlag) printf("Judy1FreeArray  = %6.3f Bytes/Index\n",
                   (double)Bytes / (double)Count1);

//            JLFA(Bytes, JL);    // Free the JudyL Array
            Bytes = JudyLFreeArray(&JL, NULL);
            if (Bytes == (Word_t)JERR)
                FAILURE("JudyLFreeArray ret Bytes", Bytes);

            if (pFlag) printf("JudyLFreeArray  = %6.3f Bytes/Index\n",
                   (double)Bytes / (double)CountL);

            TotalPop = 0;
        }
    }
    printf("Passed Judy1, JudyL, JudyHS tests for %" PRIuPTR" numbers with <= %" PRIdPTR" bits\n", nElms, BValue);
    exit(0);
#undef JL
#undef J1
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyIns"

// Insert Elements indexes in each array. The same indexes for each array.
// Generate the indexes by successive calls to GetNextIndex(Seed) followed
// by Swizzle if Dflag followed by Splay, aka MyPDEP, if wSplayMask != -1
// followed by adding wOffset.
// If SkipN == 0, then GetNextIndex uses an LFSR to generate the next index.
// Otherwise GetNextIndex simply adds SkipN to previous index.
// Set the *PValue = TstIndex for JudyL.
// Follow each Insert with a Get and another Insert of the same index
// and verify the results.
// Verify that Count(0, ~0) == TotalPop after each Insert.
// Return the resulting Seed.
Word_t
TestJudyIns(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements)
{
    Word_t TstIndex;
    Word_t elm;
    Word_t *PValue, *PValue1;
    Word_t Seed1;
    int Rcode;
    (void)JH;

    for (Seed1 = Seed, elm = 0; elm < Elements; elm++)
    {
        Seed1 = GetNextIndex(Seed1);
        if (Seed1 == 0)
            FAILURE("This command not robust if Index == 0", elm);

        if (DFlag)
            TstIndex = Swizzle(Seed1);
        else
            TstIndex = Seed1;

        if (wSplayMask != -(Word_t)1) { TstIndex = MyPDEP(TstIndex, wSplayMask) | (wSplayBase & ~wSplayMask); }
        TstIndex += wOffset;

        if (pFlag) { printf("JudyLIns: %8" PRIuPTR"\t%p\n", elm, (void *)TstIndex); }

//      Judy1
        Word_t wCount = Judy1Count(*J1, 0, ~0, NULL);
        if (wCount != TotalPop) {
            Judy1Dump((Word_t)*J1, sizeof(Word_t) * 8, 0);
            printf("wCount %zd TotalPop %zd\n", wCount, TotalPop);
            FAILURE("Count TotalIns", TotalIns);
        }

        J1S(Rcode, *J1, TstIndex);
        if (Rcode == JERR)
            FAILURE("Judy1Set failed at", elm);
        if (Rcode == 0)
            FAILURE("Judy1Set failed - DUP Index, population =", TotalPop);

        Rcode = Judy1Test(*J1, TstIndex, NULL);
        if (Rcode != 1) {
            Judy1Dump((Word_t)*J1, sizeof(Word_t) * 8, 0);
            printf("TstIndex %zu 0x%zx\n", TstIndex, TstIndex);
            FAILURE("Judy1Test failed - Index missing, population =", TotalPop);
        }

        J1S(Rcode, *J1, TstIndex);
        if (Rcode != 0)
            FAILURE("Judy1Set failed - Index missing, population =", TotalPop);

//      JudyL
        PValue = (PWord_t)JudyLIns(JL, TstIndex, NULL);
        if (PValue == PJERR)
            FAILURE("JudyLIns failed at", elm);
        if (*PValue == TstIndex)
        {
            if (TstIndex != 0)
            {
                printf("TstIndex %zu 0x%zx\n", TstIndex, TstIndex);
                FAILURE("JudyLIns failed - DUP Index, population =", TotalPop);
            }
        }

//      Save Index in Value
        *PValue = TstIndex;

        PValue1 = (PWord_t)JudyLGet(*JL, TstIndex, NULL);
        if (PValue != PValue1) {
            printf("TstIndex %zu 0x%zx PValue1 %p\n", TstIndex, TstIndex, PValue1);
            FAILURE("JudyLGet failed - Incorrect PValue, population =", TotalPop);
        }

        PValue1 = (PWord_t)JudyLIns(JL, TstIndex, NULL);
        if (PValue != PValue1)
        {
            if (*PValue1 != TstIndex)
            {
               FAILURE("JudyLIns failed - Incorrect value, population =", TotalPop);
            }
            else
            {
// not ready for this yet! printf("Index moved -- TotalPop = %" PRIuPTR"\n", TotalPop);
            }
        }
#if defined(TEST_HS)
//      JudyHS
        PValue = (PWord_t)JudyHSIns(JH, (void *)(&TstIndex), sizeof(Word_t), NULL);
        if (PValue == PJERR)
            FAILURE("JudyHSIns failed at", elm);
        if (*PValue == TstIndex)
        {
            if (TstIndex != 0)
            {
                FAILURE("JudyHSIns failed - DUP Index, population =", TotalPop);
            }
        }

//      Save Index in Value
        *PValue = TstIndex;

        PValue1 = (PWord_t)JudyHSGet(*JH, (void *)(&TstIndex), sizeof(Word_t));
        if (PValue != PValue1)
            FAILURE("JudyHSGet failed - Index missing, population =", TotalPop);

        PValue1 = (PWord_t)JudyHSIns(JH, (void *)(&TstIndex), sizeof(Word_t), NULL);
        if (PValue != PValue1)
        {
            if (*PValue1 != TstIndex)
            {
               FAILURE("JudyHSIns failed - Index missing, population =", TotalPop);
            }
            else
            {
// not ready for this yet! printf("Index moved -- TotalPop = %" PRIuPTR"\n", TotalPop);
            }
        }
#endif // defined(TEST_HS)
        TotalPop++;
        TotalIns++;
    }
    return (Seed1);             // New seed
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyGet"

// Do Elements Tests/Gets on each array.
// Generate the indexes using GetNextIndex and post-processing in the same way
// it is done for TestJudyIns.
// Check that PValue is not NULL and *PValue is equal to the index for JudyL.
// Check that Test returns 1 for Judy1.
// Return the lowest index tested.
Word_t
TestJudyGet(void *J1, void *JL, void *JH, Word_t Seed, Word_t Elements)
{
    Word_t LowIndex = ~(Word_t)0;
    Word_t TstIndex;
    Word_t elm;
    Word_t *PValue;
    Word_t Seed1;
    int Rcode;
    (void)JH;

    for (Seed1 = Seed, elm = 0; elm < Elements; elm++)
    {
        Seed1 = GetNextIndex(Seed1);

        if (DFlag)
            TstIndex = Swizzle(Seed1);
        else
            TstIndex = Seed1;

        if (wSplayMask != -(Word_t)1) { TstIndex = MyPDEP(TstIndex, wSplayMask) | (wSplayBase & ~wSplayMask); }
        TstIndex += wOffset;

        if (TstIndex < LowIndex)
            LowIndex = TstIndex;

        Rcode = Judy1Test(J1, TstIndex, NULL);

        if (pFlag) { printf("Judy1Test: elm=%8" PRIuPTR"\t%p\n", elm, (void *)TstIndex); }

        if (Rcode != 1)
            FAILURE("Judy1Test Rcode != 1 =", Rcode);

        PValue = (PWord_t)JudyLGet(JL, TstIndex, NULL);
        if (PValue == (Word_t *) NULL)
            FAILURE("JudyLGet ret PValue = NULL", 0L);
        if (*PValue != TstIndex)
        {
            printf("TstIndex 0x%zx PValue %p *PValue 0x%zx\n",
                   TstIndex, PValue, *PValue);
            JudyLDump((Word_t)JL, sizeof(Word_t) * 8, TstIndex);
            FAILURE("JudyLGet ret wrong Value at", elm);
        }

#ifdef TEST_HS
        PValue = (PWord_t)JudyHSGet(JH, (void *)(&TstIndex), sizeof(Word_t));
        if (PValue == (Word_t *) NULL)
            FAILURE("JudyHSGet ret PValue = NULL", 0L);
        if (*PValue != TstIndex)
            FAILURE("JudyHSGet ret wrong Value at", elm);
#endif // TEST_HS
    }

    return(LowIndex);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDup"

Word_t
TestJudyDup(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements)
{
    Word_t LowIndex = (Word_t)~0;
    Word_t TstIndex;
    Word_t elm;
    Word_t *PValue;
    Word_t Seed1;
    int Rcode;
    (void)JH;

    for (Seed1 = Seed, elm = 0; elm < Elements; elm++)
    {
        Seed1 = GetNextIndex(Seed1);

        if (DFlag)
            TstIndex = Swizzle(Seed1);
        else
            TstIndex = Seed1;

        if (wSplayMask != -(Word_t)1) { TstIndex = MyPDEP(TstIndex, wSplayMask) | (wSplayBase & ~wSplayMask); }
        TstIndex += wOffset;

        if (TstIndex < LowIndex)
            LowIndex = TstIndex;

        J1S(Rcode, *J1, TstIndex);
        if (Rcode != 0)
            FAILURE("Judy1Set Rcode != 0", Rcode);

        PValue = (PWord_t)JudyLIns(JL, TstIndex, NULL);
        if (PValue == (Word_t *) NULL)
            FAILURE("JudyLIns ret PValue = NULL", 0L);
        if (*PValue != TstIndex)
            FAILURE("JudyLIns ret wrong Value at", elm);

#ifdef TEST_HS
        PValue = (PWord_t)JudyHSIns(JH, &TstIndex, sizeof(Word_t), NULL);
        if (PValue == (Word_t *) NULL)
            FAILURE("JudyHSIns ret PValue = NULL", 0L);
        if (*PValue != TstIndex)
            FAILURE("JudyHSIns ret wrong Value at", elm);
#endif // TEST_HS
    }

    return(LowIndex);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyCount"

// Count(LowIndex, TstIndex) on each array then advance TstIndex using Next
// and repeat.
// Do it Elements times on each array.
// LowIndex is the lowest index inserted in the most recent delta.
// Start with TstIndex = LowIndex.
// Verify the result of each Count.
// Make sure Next returns the same thing for both arrays unless NO_TEST_NEXT.
// Both indexes passed to JudyCount always exist.
// We should enhance the test to specify indexes that are not present and
// to do important testing around narrow pointers and prefix mismatches.
int
TestJudyCount(void *J1, void *JL, Word_t LowIndex, Word_t Elements)
{
    Word_t elm;
    Word_t Count1, CountL;
    Word_t TstIndex = LowIndex;
    PWord_t PValue; (void)PValue;
    int Rc; (void)Rc;
    Word_t TstIndexL = TstIndex; (void)TstIndexL;
    Word_t TstIndex1 = TstIndex; (void)TstIndex1;

    Count1 = Judy1Count(J1, 0, -(Word_t)1, NULL);
    if (Count1 != TotalPop) {
        printf("TotalPop %zd\n", TotalPop);
        FAILURE("Judy1Count(0, -1)", Count1);
    }
    CountL = JudyLCount(JL, 0, -(Word_t)1, NULL);
    if (CountL != TotalPop) {
        printf("TotalPop %zd\n", TotalPop);
        FAILURE("JudyLCount(0, -1)", CountL);
    }
    // We never Insert index == 0.
    // Unless user makes a mistake with -S.
    Count1 = Judy1Count(J1, 1, -(Word_t)1, NULL);
    if (Count1 != TotalPop) {
        printf("TotalPop %zd\n", TotalPop);
        Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
        FAILURE("Judy1Count(1, -1)", Count1);
    }
    CountL = JudyLCount(JL, 1, -(Word_t)1, NULL);
    if (CountL != TotalPop) {
        printf("TotalPop %zd\n", TotalPop);
        FAILURE("JudyLCount(1, -1)", CountL);
    }

    // Tests designed specifically for missing keys around a narrow pointer.
    // Need to make it more flexible.
    // For now it requires wOffset have a high byte greater than zero and less than 0xff
    // and the rest of the bits must be zero.
    // E.g. Check -B32 -o0x5500000000000000
    if ((wSplayMask == -(Word_t)1) // keep it simple for now
        && (BValue < sizeof(Word_t) * 8 - 8) // we have a narrow pointer at the top
        && (wOffset >= ((Word_t)1 << (sizeof(Word_t) * 8 - 8))) // high byte is not zero
        && (wOffset < ((Word_t)0xff << (sizeof(Word_t) * 8 - 8))) // high byte is not 0xff
        && ((wOffset & (((Word_t)1 << (sizeof(Word_t) * 8 - 8)) - 1)) == 0)
        )
    {
        // Test index == 1 and index == -2 so special treatment of 0 and -1
        // can't hide a bug as easily.
        Count1 = Judy1Count(J1, 1, -(Word_t)2, NULL);
        if (Count1 != TotalPop) {
            printf("TotalPop %zd\n", TotalPop);
            FAILURE("Judy1Count(1, -2)", Count1);
        }
        CountL = JudyLCount(JL, 1, -(Word_t)2, NULL);
        if (CountL != TotalPop) {
            printf("TotalPop %zd\n", TotalPop);
            FAILURE("JudyLCount(1, -2)", CountL);
        }

        // wIndexA is the index just before the narrow pointer expanse.
        Word_t wIndexA = wOffset - 1;
        //printf("wIndexA 0x%016zx\n", wIndexA);
        // wIndexB is the first index after the narrow pointer expanse with a different
        // high byte.
        Word_t wIndexB = wOffset + ((Word_t)2 << (sizeof(Word_t) * 8 - 8));
        //printf("wIndexB 0x%016zx\n", wIndexB);

        Count1 = Judy1Count(J1, 0, wIndexA, NULL);
        if (Count1 != 0) {
            printf("wIndexA 0x%016zx TotalPop %zd\n", wIndexA, TotalPop);
            FAILURE("Judy1Count(0, wIndexA)", Count1);
        }
        CountL = JudyLCount(JL, 0, wIndexA, NULL);
        if (CountL != 0) {
            printf("wIndexA 0x%016zx TotalPop %zd\n", wIndexA, TotalPop);
            FAILURE("JudyLCount(0, wIndexA)", CountL);
        }

        Count1 = Judy1Count(J1, 1, wIndexA, NULL);
        if (Count1 != 0) {
            printf("wIndexA 0x%016zx TotalPop %zd\n", wIndexA, TotalPop);
            FAILURE("Judy1Count(1, wIndexA)", Count1);
        }
        CountL = JudyLCount(JL, 1, wIndexA, NULL);
        if (CountL != 0) {
            printf("wIndexA 0x%016zx TotalPop %zd\n", wIndexA, TotalPop);
            FAILURE("JudyLCount(1, wIndexA)", CountL);
        }

        Count1 = Judy1Count(J1, wIndexB, -(Word_t)1, NULL);
        if (Count1 != 0) {
            printf("wIndexB 0x%016zx TotalPop %zd\n", wIndexB, TotalPop);
            FAILURE("Judy1Count(wIndexB, -1)", Count1);
        }
        CountL = JudyLCount(JL, wIndexB, -(Word_t)1, NULL);
        if (CountL != 0) {
            printf("wIndexB 0x%016zx TotalPop %zd\n", wIndexB, TotalPop);
            FAILURE("JudyLCount(wIndexB, -1)", CountL);
        }

        // Test index == -2 so special treatment of -1 can't hide a bug as easily.
        Count1 = Judy1Count(J1, wIndexB, -(Word_t)2, NULL);
        if (Count1 != 0) {
            printf("wIndexB 0x%016zx TotalPop %zd\n", wIndexB, TotalPop);
            FAILURE("Judy1Count(wIndexB, -2)", Count1);
        }
        CountL = JudyLCount(JL, wIndexB, -(Word_t)2, NULL);
        if (CountL != 0) {
            printf("wIndexB 0x%016zx TotalPop %zd\n", wIndexB, TotalPop);
            FAILURE("JudyLCount(wIndexB, -2)", CountL);
        }

        Count1 = Judy1Count(J1, wIndexA, wIndexB, NULL);
        if (Count1 != TotalPop) {
            printf("wIndexA 0x%016zx wIndexB 0x%016zx TotalPop %zd\n", wIndexA, wIndexB, TotalPop);
            FAILURE("Judy1Count(wIndexA, wIndexB)", Count1);
        }
        CountL = JudyLCount(JL, wIndexA, wIndexB, NULL);
        if (CountL != TotalPop) {
            printf("wIndexA 0x%016zx wIndexB 0x%016zx TotalPop %zd\n", wIndexA, wIndexB, TotalPop);
            FAILURE("JudyLCount(wIndexA, wIndexB)", CountL);
        }

        Count1 = Judy1Count(J1, wIndexA + 1, wIndexB - 1, NULL);
        if (Count1 != TotalPop) {
            printf("wIndexA 0x%016zx wIndexB 0x%016zx TotalPop %zd\n", wIndexA, wIndexB, TotalPop);
            FAILURE("Judy1Count(wIndexA + 1, wIndexB - 1)", Count1);
        }
        CountL = JudyLCount(JL, wIndexA + 1, wIndexB - 1, NULL);
        if (CountL != TotalPop) {
            printf("wIndexA 0x%016zx wIndexB 0x%016zx TotalPop %zd\n", wIndexA, wIndexB, TotalPop);
            FAILURE("JudyLCount(wIndexA + 1, wIndexB - 1)", CountL);
        }
    }

  #ifndef NO_TEST_BY_COUNT
    // Number of keys preceding LowIndex.
    // LowIndex is not counted in wLowCount.
    Word_t wLowCount
        = (LowIndex == 0) ? 0 : Judy1Count(J1, 0, LowIndex - 1, NULL);
    // It is a problem that there is no validation that wLowCount is correct.
  #endif // !NO_TEST_BY_COUNT

    for (elm = 0; elm < Elements; elm++)
    {
        Count1 = Judy1Count(J1, LowIndex, TstIndex, NULL);
        if (Count1 == (Word_t)JERR)
            FAILURE("Judy1Count ret JERR", Count1);

        if (pFlag) {
            printf("Judy1Count: Count=%" PRIuPTR" Low=%p High=%p\n",
                   Count1, (void *)LowIndex, (void *)TstIndex);
        }

        CountL = JudyLCount(JL, LowIndex, TstIndex, NULL);
        if (CountL == (Word_t)JERR)
            FAILURE("JudyLCount ret JERR", CountL);

        if (pFlag) {
            printf("JudyLCount: Count=%" PRIuPTR" Low=%p High=%p\n",
                   CountL, (void *)LowIndex, (void *)TstIndex);
        }

        if ((Count1 != (elm + 1)) || (CountL != (elm + 1)))
        {
            printf("Count1 = %" PRIuPTR", CountL = %" PRIuPTR
                   ", should be: elm + 1 = %" PRIuPTR"\n",
                   Count1, CountL, elm + 1);
            printf("LowIndex 0x%zx TstIndex 0x%zx\n", LowIndex, TstIndex);
            if (Count1 != elm + 1) {
                Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, TstIndex);
            }
            if (CountL != elm + 1) {
                JudyLDump((Word_t)JL, sizeof(Word_t) * 8, 0);
            }
            FAILURE("Count at", elm);
        }

  #ifndef NO_TEST_BY_COUNT
        // LowIndex is counted in Count1 if it is present.
        Word_t wByKey = LowIndex;
        Word_t wByCnt = Count1;
        int retBy = Judy1ByCount(J1, wByCnt, &wByKey, NULL);
        if (retBy != 1) {
            printf("LowIndex 0x%zx TstIndex 0x%zx Count1 %zd\n",
                   LowIndex, TstIndex, Count1);
            printf("wByCnt %zd wByKey 0x%zx\n", wByCnt, wByKey);
            FAILURE("ByCount != 1; elm", elm);
        }
        if (wByKey != TstIndex) {
            printf("LowIndex 0x%zx TstIndex 0x%zx Count1 %zd\n",
                   LowIndex, TstIndex, Count1);
            printf("wByCnt %zd wByKey 0x%zx\n", wByCnt, wByKey);
            FAILURE("ByCount from LowIndex at elm", elm);
        }
        wByKey = 0;
        wByCnt += wLowCount;
        retBy = Judy1ByCount(J1, wByCnt, &wByKey, NULL);
        if (retBy != 1) {
            printf("wLowCount %zd\n", wLowCount);
            printf("LowIndex 0x%zx TstIndex 0x%zx Count1 %zd\n",
                   LowIndex, TstIndex, Count1);
            printf("wByCnt %zd wByKey 0x%zx\n", wByCnt, wByKey);
            FAILURE("ByCount = -1; elm", elm);
        }
        if (wByKey != TstIndex) {
            printf("wLowCount %zd\n", wLowCount);
            printf("LowIndex 0x%zx TstIndex 0x%zx Count1 %zd\n",
                   LowIndex, TstIndex, Count1);
            printf("wByCnt %zd wByKey 0x%zx\n", wByCnt, wByKey);
            FAILURE("ByCount from 0 at elm", elm);
        }
  #endif // !NO_TEST_BY_COUNT

  #ifndef NO_TEST_NEXT
        assert(TstIndexL == TstIndex);
        assert(TstIndex1 == TstIndex);
        Word_t PrevIndex = TstIndex;
  #endif // !NO_TEST_NEXT
#if defined(USE_JUDY1_NEXT_IN_COUNT)
        Rc = Judy1Next(J1, &TstIndex1, NULL);
        TstIndex = TstIndex1;
#else // defined(USE_JUDY1_NEXT_IN_COUNT)
        PValue = (PWord_t)JudyLNext(JL, &TstIndexL, NULL);
        TstIndex = TstIndexL;
#endif // defined(USE_JUDY1_NEXT_IN_COUNT)
        // Count test depends on Next.
        // But it doesn't require both Judy1Next and JudyLNext.
#ifndef NO_TEST_NEXT
  #if defined(USE_JUDY1_NEXT_IN_COUNT)
        PValue = (PWord_t)JudyLNext(JL, &TstIndexL, NULL);
  #else // defined(USE_JUDY1_NEXT_IN_COUNT)
        Rc = Judy1Next(J1, &TstIndex1, NULL);
  #endif // defined(USE_JUDY1_NEXT_IN_COUNT)
        if (TstIndexL != TstIndex1) {
            if ((PValue != NULL) || (Rc == 1)) {
                Word_t LastIndex1 = -1;
                Judy1Last(J1, &LastIndex1, NULL);
                Word_t LastIndexL = -1;
                JudyLLast(JL, &LastIndexL, NULL);
                printf("PValue = %p Rc = %d\n", (void*)PValue, Rc);
                printf("Elements = %zd\n", Elements);
                printf("LastIndexL 0x%zx LastIndex1 0x%zx\n",
                       LastIndexL, LastIndex1);
                printf("PrevIndex 0x%zx\n", PrevIndex);
                printf("Next TstIndexL 0x%zx != TstIndex1 0x%zx\n",
                       TstIndexL, TstIndex1);
                FAILURE("Count at", elm);
            }
        }
#endif // #ifndef NO_TEST_NEXT
    }
    return(0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNext"

// Do First on LowIndex followed by Elements Nexts.
// Vefify that we find the same indexes in both arrays.
// Verify that the last Next finds no index.
// Maybe we should verify the value for JudyL.
Word_t TestJudyNext(void *J1, void *JL, Word_t LowIndex, Word_t Elements)
{
    Word_t JLindex, J1index, JPindex = 0;
    Word_t *PValue;
    Word_t elm;
    int Rcode;

//  Get an Index low enough for Elements
    J1index = JLindex = LowIndex;

    PValue = (PWord_t)JudyLFirst(JL, &JLindex, NULL);
//    J1F(Rcode, J1, J1index);
    Rcode = Judy1First(J1, &J1index, NULL); // Get next one

    for (elm = 0; elm < Elements; elm++)
    {
        if (PValue == NULL)
            FAILURE("JudyLNext ret NULL PValue at", elm);
        if (Rcode != 1)
            FAILURE("Judy1Next Rcode != 1 =", Rcode);
        if (JLindex != J1index)
        {
            printf("JudyLNext = %zx Judy1Next = %zx\n", JLindex, J1index);
            Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
            FAILURE("JudyLNext & Judy1Next ret different *PIndex at", elm);
        }

        JPindex = J1index;              // save the last found index

        PValue = (PWord_t)JudyLNext(JL, &JLindex, NULL); // Get next one
//        J1N(Rcode, J1, J1index);        // Get next one
        Rcode = Judy1Next(J1, &J1index, NULL); // Get next one
    }

    if (PValue != NULL)
        FAILURE("JudyLNext PValue != NULL", PValue);
    if (Rcode != 0)
        FAILURE("Judy1Next Rcode != 0 =", Rcode);

//  perhaps a check should be done here -- if I knew what to expect.
    return(JPindex);            // return last one
}


#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyPrev"

int
TestJudyPrev(void *J1, void *JL, Word_t HighIndex, Word_t Elements)
{
    Word_t JLindex, J1index;
    Word_t *PValue;
    Word_t elm;
    int Rcode;

//  Get an Index high enough for Elements
    J1index = JLindex = HighIndex;

    PValue = (PWord_t)JudyLLast(JL, &JLindex, NULL);
    J1L(Rcode, J1, J1index);

    for (elm = 0; elm < Elements; elm++)
    {
        if (PValue == NULL)
            FAILURE("JudyLPrev ret NULL PValue at", elm);
        if (Rcode != 1)
            FAILURE("Judy1Prev Rcode != 1 at elm", elm);
        if (JLindex != J1index)
        {
            printf("JLindex %zx J1index %zx\n", JLindex, J1index);
            FAILURE("JudyLPrev & Judy1Prev ret different PIndex at", elm);
        }

        PValue = (PWord_t)JudyLPrev(JL, &JLindex, NULL); // Get previous one
        J1P(Rcode, J1, J1index);        // Get previous one
    }
    if (PValue != NULL)
        FAILURE("JudyLPrev PValue != NULL", PValue);
    if (Rcode != 0)
        FAILURE("Judy1Prev Rcode != 1 =", Rcode);
//  perhaps a check should be done here -- if I knew what to expect.
    return(0);
}


#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNextEmpty"

// Start with LowIndex and do NextEmpty.
// Verify that Test/Get don't find the resulting index.
// Do Next to find the next index.
// Repeat.
int
TestJudyNextEmpty(void *J1, void *JL, Word_t LowIndex, Word_t Elements)
{
    Word_t PrevKey;
    Word_t elm;
    Word_t JLindex, J1index;
    int Rcode1;         // Return code
    int RcodeL;         // Return code

//  Set 1st search to  ..
    J1index = JLindex = LowIndex;

    for (elm = 0; elm < Elements; elm++)
    {
        Word_t *PValue;

        if (pFlag) { printf("JudyLNextEmpty: elm=%8" PRIuPTR"\t%p\n", elm, (void *)JLindex); }
        PrevKey = JLindex;
        Word_t PrevKey1 = J1index;

//      Find next Empty Index, JLindex is modified by JLNE
        JLNE(RcodeL, JL, JLindex);      // Rcode = JudyLNextEmpty(JL, &JLindex, PJE0)

//      Find next Empty Index, J1index is modified by J1NE
        J1NE(Rcode1, J1, J1index);      // Rcode = Judy1NextEmpty(J1, &J1index, PJE0)

        if ((Rcode1 != 1) || (RcodeL != 1))
        {
            printf("RcodeL = %d, Rcode1 = %d, Index1 = 0x%" PRIxPTR", IndexL = 0x%" PRIxPTR"\n",
                    RcodeL, Rcode1, J1index, JLindex);
            FAILURE("Judy1NextEmpty Rcode != 1 =", Rcode1);
        }

        if (J1index != JLindex) {
            printf("RcodeL = %d, Rcode1 = %d, Index1 = 0x%" PRIxPTR", IndexL = 0x%" PRIxPTR"\n",
                    RcodeL, Rcode1, J1index, JLindex);
            printf("PrevKey 0x%zx\n", PrevKey);
            printf("PrevKey1 0x%zx\n", PrevKey1);
            Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, PrevKey);
  #ifndef FULL_DUMP
            Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, J1index);
  #endif // !FULL_DUMP
            JudyLDump((Word_t)JL, sizeof(Word_t) * 8, PrevKey);
            FAILURE("JLNE != J1NE returned index at", elm);
        }

        if (pFlag)
        {
            Word_t SeqKeys = JLindex - PrevKey;

            if (SeqKeys > 1)
                printf("JudyLNextEmpty:skipped %" PRIuPTR" sequential Keys\n", SeqKeys);
        }

        Rcode1 = Judy1Test(J1, J1index, NULL);

        if (Rcode1 != 0) {
            printf("PrevKey 0x%zx J1index 0x%zx\n", PrevKey, J1index);
            FAILURE("J1NE returned non-empty Index =", J1index);
        }

        PValue = (PWord_t)JudyLGet(JL, JLindex, NULL);
        if (PValue != (Word_t *) NULL)
            FAILURE("JLNE returned non-empty Index =", JLindex);

//      find next Index (Key) in array
        Rcode1 = Judy1Next(J1, &J1index, PJE0);
        PValue = (Word_t *)JudyLNext(JL, &JLindex, PJE0);

        if ((Rcode1 != 1) && (PValue == NULL))
            break;

        if ((Rcode1 != 1) || (PValue == NULL)) {
            printf("Rcode1 = %d, PValue = %p\n", Rcode1, (void *)PValue);
            FAILURE("Judy1Next != JudyLNext return code at", elm);
        }

        if (J1index != JLindex) {
            printf("Index1 = 0x%" PRIxPTR", IndexL = 0x%" PRIxPTR"\n", J1index, JLindex);
            FAILURE("Judy1Next != JudyLNext returned index at", elm);
        }
    }
    return(0);
}


// Routine to JudyPrevEmpty routines

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyPrevEmpty"

int
TestJudyPrevEmpty(void *J1, void *JL, Word_t HighIndex, Word_t Elements)
{
    Word_t elm;
    Word_t JLindex, J1index;
    int Rcode1;
    int RcodeL;

//  Set 1st search to  ..
    J1index = JLindex = HighIndex;

    for (elm = 0; elm < Elements; elm++)
    {
        Word_t *PValue;
        Word_t  PrevKey = J1index;

        if (pFlag) { printf("JudyPrevEmpty: %8" PRIuPTR"\t%p\n", elm, (void *)J1index); }

//      Find Previous Empty Index, JLindex/J1index is modified by J[1L]PE
        assert(JLindex == J1index);
        J1PE(Rcode1, J1, J1index);      // Rcode = Judy1PrevEmpty(J1, &J1index, PJE0)
        JLPE(RcodeL, JL, JLindex);      // RcodeL = JudyLPrevEmpty(JL, &JLindex, PJE0)
        if ((RcodeL != 1) || (Rcode1 != 1))
        {
            if (PrevKey != 0)
            {
                printf("RcodeL %d Rcode1 %d Index1 0x%zx IndexL 0x%zx J*PE(0x%zx)\n",
                        RcodeL, Rcode1, J1index, JLindex, PrevKey);
                Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
                FAILURE("Judy[1L]PrevEmpty Rcode* != 1 =", RcodeL);
            }
            if ((RcodeL != 0) || (Rcode1 != 0))
            {
                printf("RcodeL %d Rcode1 %d Index1 0x%zx IndexL 0x%zx J*PE(0x%zx)\n",
                        RcodeL, Rcode1, J1index, JLindex, PrevKey);
                Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
                FAILURE("Judy[1L]PrevEmpty Rcode* != 0 =", RcodeL);
            }
            break;
        }
        if (J1index != JLindex) {
            printf("JLPE != J1PE Key 0x%zx J1PE 0x%zx JLPE 0x%zx\n",
                    PrevKey, J1index, JLindex);
            Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
            JudyLDump((Word_t)JL, sizeof(Word_t) * 8, 0);
            FAILURE("JLPE != J1PE returned index at", elm);
        }

        if (pFlag)
        {
            Word_t SeqKeys = PrevKey - JLindex;

            if (SeqKeys > 1)
                printf("J[1L]PE:skipped %" PRIuPTR" sequential Keys\n", SeqKeys);
        }

        Rcode1 = Judy1Test(J1, J1index, NULL);

        if (Rcode1 != 0)
            FAILURE("J1PE returned non-empty Index =", J1index);

        PValue = (PWord_t)JudyLGet(JL, JLindex, NULL);

        if (PValue != (Word_t *) NULL)
            FAILURE("JLPE returned non-empty Index =", JLindex);

        Word_t J1indexBefore = J1index;
        Word_t JLindexBefore = JLindex;
//      find next Index (Key) in array
        Rcode1 = Judy1Prev(J1, &J1index, PJE0);
        PValue = (Word_t *)JudyLPrev(JL, &JLindex, PJE0);

        if ((Rcode1 != 1) && (PValue == NULL))
            break;

        if ((Rcode1 != 1) || (PValue == NULL)) {
            printf("J1indexBefore 0x%zx JLindexBefore 0x%zx\n", J1indexBefore, JLindexBefore);
            printf("J1index 0x%zx JLindex 0x%zx\n", J1index, JLindex);
            printf("Rcode1 = %d, PValue = %p\n", Rcode1, (void *)PValue);
            Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
            printf("HighIndex 0x%zx Elements %zd\n", HighIndex, Elements);
            FAILURE("Judy1Prev != JudyLPrev return code at elm", elm);
        }

        if (J1index != JLindex) {
            printf("Index1 = 0x%" PRIxPTR", IndexL = 0x%" PRIxPTR"\n", J1index, JLindex);
            FAILURE("Judy1Prev != JudyLPrev returned index at", elm);
        }
    }
    return(0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDel"

int
TestJudyDel(void **J1, void **JL, void **JH, Word_t Seed, Word_t Elements)
{
    Word_t TstIndex;
    Word_t elm;
    Word_t Seed1;
    int Rcode;
    (void)JH;

//  Only delete half of those inserted
    for (Seed1 = Seed, elm = 0; elm < (Elements / 2); elm++)
    {
        Seed1 = GetNextIndex(Seed1);

        if (DFlag)
            TstIndex = Swizzle(Seed1);
        else
            TstIndex = Seed1;

        if (wSplayMask != -(Word_t)1) { TstIndex = MyPDEP(TstIndex, wSplayMask) | (wSplayBase & ~wSplayMask); }
        TstIndex += wOffset;

        if (pFlag) { printf("JudyLDel: %8" PRIuPTR"\t0x%p\n", elm, (void *)TstIndex); }

        TotalDel++;

        J1U(Rcode, *J1, TstIndex);
        if (Rcode != 1)
            FAILURE("Judy1Unset ret Rcode != 1", Rcode);

        JLD(Rcode, *JL, TstIndex);
        if (Rcode != 1)
            FAILURE("JudyLDel ret Rcode != 1", Rcode);

#ifdef TEST_HS
        JHSD(Rcode, *JH, (void *)(&TstIndex), sizeof(Word_t));
        if (Rcode != 1)
            FAILURE("JudyHSDel ret Rcode != 1", Rcode);
#endif // TEST_HS

        TotalPop--;
    }
    return(0);
}

// Routine to get next size of Indexes
int                             // return 1 if last number
NextNumb(Word_t * PNumber,      // pointer to returned next number
         double *PDNumb,        // Temp double of above
         double DMult,          // Multiplier
         Word_t MaxNumb)        // Max number to return
{
    Word_t num;

//  Save prev number
    Word_t PrevNumb = *PNumber;

//  Verify integer number increased
    for (num = 0; num < 1000; num++)
    {
//      Calc next number
        *PDNumb *= DMult;

//      Return it in integer format
        *PNumber = (Word_t) (*PDNumb + 0.5);

        if (*PNumber != PrevNumb)
            break;
    }

//  Verify it did exceed max ulong
    if ((*PDNumb + 0.5) > (double)((Word_t)-1))
    {
//      It did, so return max number
        *PNumber = (Word_t)-1;
        return (1);             // flag it
    }

//  Verify it did not exceed max number
    if ((*PDNumb + 0.5) > (double)MaxNumb)
    {
//      it did, so return max
        *PNumber = MaxNumb;
        return(1);              // flag it
    }
    return(0);                  // more available
}
