// @(#) $Revision: 1.4 $ $Source: /home/doug/judy-1.0.5_no1_L1D_mod/test/RCS/Judy1LHTime.c,v $
// =======================================================================
//                      -by- 
//   Author Douglas L. Baskins, Aug 2003.
//   Permission to use this code is freely granted, provided that this
//   statement is retained.
//   email - dougbaskins .at. yahoo.com -or- dougbaskins .at. gmail.com
// =======================================================================

#if (! (defined(JUDYA) || defined(JUDYB)))
#error "-DJUDYA or -DJUDYB MUST be specified."
#endif

#if (defined(JUDYA) && defined(JUDYB))
#error  "Only one of -DJUDYA or -DJUDYB may be specified."
#endif

#include <unistd.h>                     // sbrk()
#include <stdlib.h>                     // exit(), malloc()
#include <stdio.h>                      // printf(), setbuf()
#include <math.h>                       // pow()
#include <time.h>                       // clock_gettime()
#include <sys/utsname.h>                // uname()
#include <errno.h>                      // errnoerrno

// Turn off assert(0) by default
#ifndef DEBUG
#define NDEBUG 1
#endif  // DEBUG

#include <assert.h>                     // assert()

#include <Judy.h>                       // for Judy macros J*()

#include "RandomNumb.h"                 // Random Number Generators

#define WARMUPCPU  10000000             // calls to random() to warmup CPU

// =======================================================================
//   This program measures the performance of a Judy1, JudyL and 
//   limited to one size of string (sizeof Word_t) JudyHS Arrays.
//
// Compile examples: 
//
//   cc -O -g Judy1LHTime.c -lm -lrt -lJudy -o Judy1LHTime 
//           -or-
//   cc -m32 -O3 -Wall -I../src Judy1LHTime.c -lm -lrt ../src/libJudy32.a -o Judy1LHTime32

//=======================================================================
//             R A M   M E T R I C S  
//=======================================================================
//  For figuring out how much different structures contribute.   Must be 
//  turned on in source compile of Judy with -DRAMMETRICS

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

// This 64 Bit define may NOT work on all compilers


//=======================================================================
//      T I M I N G   M A C R O S
//=======================================================================

// (D) is returned in nano-seconds from last STARTTm

#include <time.h>

#if defined __APPLE__ && defined __MACH__

#include <mach/mach_time.h>

uint64_t  start__;

#define STARTTm  (start__ = mach_absolute_time())
#define ENDTm(D) ((D) = (double)(mach_absolute_time() - start__))

#else  // POSIX Linux and Unix

struct timespec TVBeg__, TVEnd__;

// CLOCK_PROCESS_CPUTIME_ID
#define STARTTm         (clock_gettime(CLOCK_MONOTONIC, &TVBeg__))

#define ENDTm(D) 							\
{ 									\
    clock_gettime(CLOCK_MONOTONIC, &TVEnd__);   	                \
                                                                        \
    (D) = (double)(TVEnd__.tv_sec - TVBeg__.tv_sec) * 1E9 +             \
         ((double)(TVEnd__.tv_nsec - TVBeg__.tv_nsec));                 \
}
#endif // POSIX Linux and Unix

Word_t    xFlag = 0;    // Turn ON 'waiting for Context Switch'

// Wait for an extraordinary long Delta time (context switch?)
void
WaitForContextSwitch(Word_t Loops)
{
    double    DeltanSecw;
    int       kk;

    if (xFlag == 0)
        return;

    if (Loops > 200)
        return;

//  But, dont wait too long
    for (kk = 0; kk < 1000000; kk++)
    {
        STARTTm;
        Loops = (Word_t)random();
        ENDTm(DeltanSecw);

//      5 microseconds should to it
        if (DeltanSecw > 5000)
            break;
    }
}

// =======================================================================
// J U D Y   M A L L O C
// =======================================================================
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JRS accounting occurs at a higher level.

enum
{
    JudyMal1,
    JudyMalL,
    JudyMalHS
} MalFlag;

static Word_t MalFreeCnt = 0;

Word_t    TotMalOvdWords = 0;
Word_t    TotJudy1MemUsed = 0;
Word_t    TotJudyLMemUsed = 0;
Word_t    TotJudyHSMemUsed = 0;

#ifdef SELFALIGNED
// Did not think very hard on this one.  I am sure their is a much
// faster way to do it. (dlb)

static inline int
j__log2(uint32_t num)
{
    int       __lb = 0;

    if (num >= (1 << 16))
    {
        num >>= 16;
        __lb += 16;
    }
    if (num >= (1 << 8))
    {
        num >>= 8;
        __lb += 8;
    }
    if (num >= (1 << 4))
    {
        num >>= 4;
        __lb += 4;
    }
    if (num >= (1 << 2))
    {
        num >>= 2;
        __lb += 2;
    }
    if (num >= (1 << 1))
    {
        __lb += 1;
    }
    return (__lb);
}

Word_t
j__roundup2(Word_t Size)
{
    Word_t ret;

    ret = j__log2(Size);
    if ((Size & -Size) != Size) ret++;

    return(1 << ret);
}

#endif // SELFALIGNED

// This routine replaces the one in libJudy.a so we can get a
// measure of the memory used and counts of malloc().
// It assumes that a dlmalloc() style of memory allocation is used.

Word_t
JudyMalloc(Word_t Words)
{
    size_t    Bytes;
    Word_t    Addr;
    Word_t    MalOvd;
    Bytes = Words * sizeof(Word_t);

//printf("Malloc(%lu)\n", Words);

#ifdef SELFALIGNED
    {
        int ret;

        ret = posix_memalign((void **)(&Addr), j__roundup2(Bytes), Bytes);
        if (ret)
        {
            printf("posix_memalign() failure code = %d\n", ret);
            exit(1);
        }
    }
#else // ! SELFALIGNED

#ifdef  GUARDBAND
    Bytes += sizeof(Word_t);    // one word
#endif  // GUARDBAND

    Addr = (Word_t)malloc(Bytes);

#ifdef  GUARDBAND
    *((Word_t *)Addr + ((Bytes/sizeof(Word_t)) - 1)) = ~Addr;
#endif  // GUARDBAND

#endif // ! SELFALIGNED

//  Verify that all mallocs are 2 word aligned
    if (Addr & ((sizeof(Word_t) * 2) - 1))
    {
        printf("\nmalloc() Addr not 2 word aligned = 0x%lx\n", Addr);
        exit(1);
    }
    if (Addr)
    {
        MalFreeCnt++;                   // keep track of total malloc() + free()

//      Calculate RAM overhead of (dl)malloc() (used in Linux)
//      Odd sized (Word_t) mallocs have 1 word overhead while
//      Even sized mallocs have a 2 word overhead.
//
        if (Words & 1)
            MalOvd = 1;
        else
            MalOvd = 2;

// printf("# JudyMalloc %d(%d) words\n", (int)Words, (int)MalOvd);

        TotMalOvdWords += MalOvd;

        switch (MalFlag)
        {
        case JudyMal1:
            TotJudy1MemUsed += Words;
            break;
        case JudyMalL:
            TotJudyLMemUsed += Words;
            break;
        case JudyMalHS:
            TotJudyHSMemUsed += Words;
            break;
        }
    }
    return (Addr);
}                                       // JudyMalloc()

// ****************************************************************************
// J U D Y   F R E E
//
// Note: Judy knows amount of memory being freed, though not needed by free()

void
JudyFree(void *PWord, Word_t Words)
{
    Word_t    MalOvd;

    if (Words == 0)
    {
        fprintf(stderr, "--- OOps JudyFree called with 0 words\n");
        printf("--- OOps JudyFree called with 0 words\n");
        exit(1);
    }

#ifdef  GUARDBAND
    {
        Word_t GuardWord;

        GuardWord = *((((Word_t *)PWord) + Words));

        if (~GuardWord != (Word_t)PWord)
        {
            printf("Oops GuardWord = 0x%lx != PWord = 0x%lx\n", GuardWord, (Word_t)PWord);
            assert(0);
        }
    }
#endif  // GUARDBAND

    free(PWord);

    MalFreeCnt++;                       // keep track of total malloc() + free()

//  Calculate RAM overhead of (dl)malloc
    if (Words & 1)
        MalOvd = 1;
    else
        MalOvd = 2;

// printf("# JudyFree %d(%d) words\n", (int)Words, (int)MalOvd);

    TotMalOvdWords -= MalOvd;

    switch (MalFlag)
    {
    case JudyMal1:
        TotJudy1MemUsed -= Words;
        break;
    case JudyMalL:
        TotJudyLMemUsed -= Words;
        break;
    case JudyMalHS:
        TotJudyHSMemUsed -= Words;
        break;
    }
}                                       // JudyFree()

// ****************************************************************************
// J U D Y   M A L L O C
//
// Higher-level "wrapper" for allocating objects that need not be in RAM,
// although at this time they are in fact only in RAM.  Later we hope that some
// entire subtrees (at a JRS or branch) can be "virtual", so their allocations
// and frees should go through this level.

Word_t
JudyMallocVirtual(Word_t Words)
{
    return (JudyMalloc(Words));

}                                       // JudyMallocVirtual()

// ****************************************************************************
// J U D Y   F R E E

void
JudyFreeVirtual(void *PWord, Word_t Words)
{
    JudyFree(PWord, Words);
}                                       // JudyFreeVirtual()


// =======================================================================
// Common macro to handle a failure
// =======================================================================
#define FAILURE(STR, UL)                                                \
{                                                                       \
printf(        "\nError: %s %lu, file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
fprintf(stderr,"\nError: %s %lu, file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
        exit(1);                                                        \
}

// Interations without improvement
//
// Minimum of 2 loops, maximum of 1000000
#define MINLOOPS 2
#define MAXLOOPS 1000

// Maximum or 10 loops with no improvement
#define ICNT 10

// Structure to keep track of times
typedef struct MEASUREMENTS_STRUCT
{
    Word_t    ms_delta;                 // leave room for more stuff
}
ms_t     , *Pms_t;

// Specify prototypes for each test routine
int       NextSum(Word_t *PNumber, double *PDNumb, double DMult);

int       TestJudyIns(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudyDup(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudyDel(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudyGet(void *J1, void *JL, void *JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudy1Copy(void *J1, Word_t Elem);

int       TestJudyCount(void *J1, void *JL, Word_t Elems);

Word_t    TestJudyNext(void *J1, void *JL, Word_t Elems);

int       TestJudyPrev(void *J1, void *JL, Word_t HighKey, Word_t Elems);

int       TestJudyNextEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elems);

int       TestJudyPrevEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elems);

int       TestBitmapSet(PWord_t B1, PSeed_t PSeed, Word_t Meas);

int       TestBitmapTest(PWord_t B1, PSeed_t PSeed, Word_t Meas);

int       TimeNumberGen(void **TestRan, PSeed_t PSeed, Word_t Delta);

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
#else  // not __LP64__
    word = ((word & 0x0000ffff) << 16) | ((word & 0xffff0000) >> 16);
    word = ((word & 0x00ff00ff) << 8) | ((word & 0xff00ff00) >> 8);
    word = ((word & 0x0f0f0f0f) << 4) | ((word & 0xf0f0f0f0) >> 4);
    word = ((word & 0x33333333) << 2) | ((word & 0xcccccccc) >> 2);
    word = ((word & 0x55555555) << 1) | ((word & 0xaaaaaaaa) >> 1);
#endif // not __LP64__

    return (word);
}

int       XScale = 100;                   // for scaling number output under the -m flag

void
PRINT5_2f(double __X)
{
    if (XScale == 100)
    {
        __X *= XScale;
        if (__X == 0.0)
        {
            printf("     0");               // keep white space cleaner
        }
        else if (__X < .0005)
        {
            printf(" %5.0f", __X);
        }
        else if (__X < .005)
        {
            printf(" %5.3f", __X);
        }
        else if (__X < .05)
        {
            printf(" %5.2f", __X);
        }
        else if (__X < .5)
        {
            printf(" %5.1f", __X);
        }
        else
        {
            printf(" %5.0f", __X);
        }
    } 
    else
    {
        if (__X > .005)
        {
            if (XScale <= 2)
            {
                printf(" %5.2f", __X * XScale);
            }
            else if (XScale <= 20)
            {
                printf(" %5.1f", __X * XScale);
            }
            else
            {
                printf(" %5.0f", __X * XScale);
            }
        }
        else
        {
            printf("     0");               // keep white space cleaner
        }
    }
}

void
PRINT7_3f(double __X)
{
    if (__X > .0005)
    {
        if (XScale <= 2)
        {
            printf(" %7.3f", __X * XScale);
        }
        else if (XScale <= 20)
        {
            printf(" %7.2f", __X * XScale);
        }
        else
        {
            printf(" %7.1f", __X * XScale);
        }
    }
    else
    {
        printf("     0");               // keep white space cleaner
    }
}

void
PRINT6_1f(double __X)
{
    if (__X >= .05)
        printf(" %6.1f", __X);
    else
        printf("      0");              // keep white space cleaner
}

#define DONTPRINTLESSTHANZERO(A,B)                              \
{                                                               \
    if (((A) - (B)) >= 0.05)                                     \
        printf(" %6.1f", ((A) - (B)));                          \
    else if (((A) - (B)) < 0.0)                                 \
        printf("   -0.0");      /* minus time makes no sense */ \
    else                                                        \
        printf("      0");      /* make 0 less noisy         */ \
}

double    DeltanSecW = 0.0;             // Global for measuring delta times
double    DeltanSec1 = 0.0;             // Global for measuring delta times
double    DeltanSecL = 0.0;             // Global for measuring delta times
double    DeltanSecHS = 0.0;            // Global for measuring delta times
double    DeltanSecBt = 0.0;            // Global for measuring delta times
double    DeltaMalFre1 = 0.0;           // Delta mallocs/frees per inserted Key
double    DeltaMalFreL = 0.0;           // Delta mallocs/frees per inserted Key
double    DeltaMalFreHS = 0.0;          // Delta mallocs/frees per inserted Key

Word_t    Judy1Dups = 0;
Word_t    JudyLDups = 0;
Word_t    JudyHSDups = 0;
Word_t    BitmapDups = 0;

Word_t    J1Flag = 0;                   // time Judy1
Word_t    JLFlag = 0;                   // time JudyL
Word_t    JHFlag = 0;                   // time JudyHS
Word_t    dFlag = 0;                    // time Judy1Unset JudyLDel
Word_t    vFlag = 0;                    // time Searching 
Word_t    CFlag = 0;                    // time Counting
Word_t    cFlag = 0;                    // time Copy of Judy1 array
Word_t    IFlag = 0;                    // time duplicate inserts/sets
Word_t    bFlag = 0;                    // Time REAL bitmap of (2^-B #) in size
Word_t    mFlag = 0;                    // words/Key for all structures
Word_t    pFlag = 0;                    // Print number set
Word_t    lFlag = 0;                    // do not do multi-insert tests

Word_t    gFlag = 0;                    // do Get after Ins (that succeds)
Word_t    iFlag = 0;                    // do another Ins (that fails) after Ins
Word_t    tFlag = 0;                    // for general new testing
Word_t    Tit = 1;                      // to measure with calling Judy
Word_t    VFlag = 1;                    // To verify Value Area contains good Data
Word_t    fFlag = 0;
Word_t    Warmup = WARMUPCPU;           // default calls to random() to warm up CPU
Word_t    PreStack = 0;                 // to test for TLB collisions with stack

Word_t    TValues = 1000000;            // Maximum numb retrieve timing tests
Word_t    nElms = 10000000;             // Default population of arrays
Word_t    ErrorFlag = 0;

//  Measurement points per decade increase of population
//
//Word_t    PtsPdec = 10;               // 25.89% spacing
//Word_t    PtsPdec = 25;               // 9.65% spacing
//Word_t    PtsPdec = 40;               // 5.93% spacing
Word_t    PtsPdec = 50;                 // 4.71% spacing - default

// For LFSR (Linear-Feedback-Shift-Register) pseudo random Number Generator
//
Word_t    RandomBit;                    // MSB (Most-Significant-Bit) in data
Word_t    BValue = 32;                  // bigger generates unrealistic data
Word_t    GValue = 0;                   // 0 = flat spectrum random numbers
//Word_t    GValue = 1;                 // 1 = pyramid spectrum random numbers
//Word_t    GValue = 2;                 // 2 = Gaussian random numbers
//Word_t    GValue = 3;                 // 3 = higher density Gaussian random numbers
//Word_t    GValue = 4;                 // 4 = even higher density Gaussian numbers

// Sequential change of Key by SValue
// Zero uses an pseuto random method
//
Word_t    SValue = 0;                   // default == Random skip
Word_t    FValue = 0;                   // Keys, read from file
char     *keyfile;                      // -F filename ^ to string
PWord_t   FileKeys = NULL;              // array of FValue keys

// Sizzle flag == 1 >> I.E. bit reverse (mirror) the data
//
Word_t    DFlag = 0;                    // bit reverse (mirror) the data stream

// Stuff for Sequential number generation
//
Word_t    StartSequent = 0xc1fc;        // default beginning sequential numbers

// Global to store the current Value return from PSeed.
//
Word_t    Key = 0xc1fc;

// returns next Key, depending on SValue, DFlag and GValue.
//
static inline Word_t
GetNextKey(PSeed_t PSeed)
{
    if (FValue)
        Key = FileKeys[PSeed->Order++];
    else
        Key = RandomNumb(PSeed, SValue);

    if (DFlag)
        return (Swizzle(Key));
    else
        return (Key);
}

void
PrintHeader()
{
    printf("# Population  DeltaIns GetMeasmts");

    if (tFlag)
        printf(" MeasOv");
    if (J1Flag)
        printf("    J1S");
    if (JLFlag)
        printf("    JLI");
    if (JHFlag)
        printf("   JHSI");
    if (bFlag)
        printf("  BMSet");
    if (tFlag)
        printf(" MeasOv");
    if (J1Flag)
        printf("    J1T");
    if (JLFlag)
        printf("    JLG");
    if (JHFlag)
        printf("   JHSG");
    if (bFlag)
        printf(" BMTest");

    if (IFlag)
    {
        if (J1Flag)
            printf(" dupJ1S");
        if (JLFlag)
            printf(" dupJLI");
        if (JHFlag)
            printf(" dupJHI");
    }

    if (cFlag)
    {
        printf(" CopyJ1");
    }
    if (CFlag)
    {
        if (J1Flag)
            printf("    J1C");
        if (JLFlag)
            printf("    JLC");
    }
    if (vFlag)
    {
        if (J1Flag)
            printf("    J1N");
        if (JLFlag)
            printf("    JLN");
        if (J1Flag)
            printf("    J1P");
        if (JLFlag)
            printf("    JLP");
        if (J1Flag)
            printf("   J1NE");
        if (JLFlag)
            printf("   JLNE");
        if (J1Flag)
            printf("   J1PE");
        if (JLFlag)
            printf("   JLPE");
    }

    if (dFlag)
    {
        if (J1Flag)
            printf("    J1U");
        if (JLFlag)
            printf("    JLD");
        if (JHFlag)
            printf("   JHSD");
    }

    if (J1Flag)
        printf(" 1heap/I");
    if (JLFlag)
        printf(" Lheap/I");
    if (JHFlag)
        printf(" HSheap/I");

    if (mFlag && (bFlag == 0))
    {

#ifdef  JUDYA
        printf(" JBB/I");
        printf(" JBU/I");
        printf(" JBL/I");
#endif  // JUDYA  

#ifdef  JUDYB
        printf("  B4/I");
        printf("  B8/I");
        printf(" B16/I");
#endif  // JUDYB   

        printf(" LWd/I");

#ifdef  JUDYA
        printf("  L7/I");
        printf("  L6/I");
        printf("  L5/I");
        printf("  L4/I");
        printf("  L3/I");
        printf("  L2/I");
        printf("  L1/I");
        printf("  B1/I");
        printf("  JV/I");
#endif  // JUDYA  

#ifdef  JUDYB
        printf(" L32/I");
        printf(" L16/I");
        printf(" L12/I");
        printf(" V12/I");
        printf(" #Cmprs");
        printf(" %%Cmps");
#endif  // JUDYB   

        printf(" TrDep");
        printf(" MalOv/I");

        if (J1Flag)
            printf(" MF1/I");
        if (JLFlag)
            printf(" MFL/I");
        if (JHFlag)
            printf(" MFHS/I");
    }
    printf("\n");
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "main"

int
main(int argc, char *argv[])
{
//  Names of Judy Arrays
    void     *J1 = NULL;                // Judy1
    void     *JL = NULL;                // JudyL
    void     *JH = NULL;                // JudyHS
    PWord_t   B1 = NULL;                // BitMap

#ifdef DEADCODE                         // see TimeNumberGen()
    void     *TestRan = NULL;           // Test Random generator
#endif // DEADCODE

    Word_t    Count1 = 0;
    Word_t    CountL = 0;
    Word_t    Bytes;

    double    DMult;
    Pms_t     Pms;
    Seed_t    InsertSeed;               // for Judy testing (random)
    PSeed_t   PStartSeed = NULL;
    Seed_t    StartSeed;
    Seed_t    BitmapSeed;
    Seed_t    BeginSeed;
    Seed_t    DummySeed;
    Word_t    Groups;                   // Number of measurement groups
    Word_t    grp;
    Word_t    Pop1;
    Word_t    Meas;
    double    TreeDepth = 0;
    double    SearchCompares = 0;
    double    SearchPopulation = 0;          // Population of Searched object
//    long      MaxNumb;
    Word_t    MaxNumb;
    int       Col;
    int       c;
    int       ii;                       // temp iterator
    double       LastPPop;
    extern char *optarg;

#ifdef LATER
    double    Dmin = 9999999999.0;
    double    Dmax = 0.0;
    double    Davg = 0.0;
#endif // LATER

    setbuf(stdout, NULL);               // unbuffer output

#ifdef LATER
    MaxNumb = 0;
    for (ii = 0; ii < 1000; ii++)
    {
        int       kk;

        for (kk = 0; kk < 1000000; kk++)
        {
            STARTTm;
            ENDTm(DeltanSec1);
            if (DeltanSec1 > 10000)
                break;
        }
        printf("kk = %d\n", kk);

        STARTTm;
//        MaxNumb += random();
        ENDTm(DeltanSec1);

        if (DeltanSec1 == 0.0)
            continue;

        if (DeltanSec1 < Dmin)
            Dmin = DeltanSec1;
        if (DeltanSec1 > Dmax)
            Dmax = DeltanSec1;

        if (DeltanSec1 > 1000.0)
        {
//            printf("over 1uS = %10.0f, %d\n", DeltanSec1, ii);
            CountL++;
        }

    }
    printf("Dmin = %10.0f, Dmax = %10.0f, Davg = %10.0f\n", Dmin, Dmax,
           Davg / 10000000.0);
    printf("Count of over 1uS = %lu\n", CountL);

    exit(1);
#endif // LATER

// ============================================================
// PARSE INPUT PARAMETERS
// ============================================================

    errno = 0;
    while ((c = getopt(argc, argv, "a:n:S:T:P:s:B:G:X:W:F:bdDcC1LHvIltmpxVfgi")) != -1)
    {
        switch (c)
        {
        case 'a':                      // Max population of arrays
            PreStack = strtoul(optarg, NULL, 0);   // Size of PreStack
            if (PreStack == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-a %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;
        case 'n':                      // Max population of arrays
            nElms = strtoul(optarg, NULL, 0);   // Size of Linear Array
            if (nElms == 0)
                FAILURE("No tests: -n", nElms);
            break;

        case 'S':                      // Step Size, 0 == Random
        {
            SValue = strtoul(optarg, NULL, 0);
            if (SValue == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-S %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;
        }
        case 'T':                      // Maximum retrieve tests for timing 
            TValues = strtoul(optarg, NULL, 0);
            if (TValues == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-T %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;

        case 'P':                      // measurement points per decade
            PtsPdec = strtoul(optarg, NULL, 0);
            if (PtsPdec == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-P %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;

        case 's':                      // May not work past 35 bits if changed
            StartSequent = strtoul(optarg, NULL, 0);
            if (StartSequent == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-s %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;

        case 'B':                      // expanse of data points (random only)
            BValue = strtoul(optarg, NULL, 0);
            if ((BValue > sizeof(Word_t) * 8) || (BValue < 15))
            {
                ErrorFlag++;
                printf("\nIllegal number of random bits of %lu !!!\n", BValue);
            }
            break;

        case 'G':                      // Gaussian Random numbers
            GValue = strtoul(optarg, NULL, 0);  // 0..4 -- check by RandomInit()
            if (GValue == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-G %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;

        case 'X':                      // Scale numbers under the '-m' flag
            XScale = strtol(optarg, NULL, 0);   // 1..1000
            if (XScale < 1)
            {
                ErrorFlag++;
                printf("\noption -X%d must be greater than 0 !!!\n", XScale);
            }

        case 'W':                      // Warm up CPU number of random() calls
            Warmup = strtoul(optarg, NULL, 0);
            if (Warmup == 0 && errno != 0)
            {
                printf("\nIllegal argument to \"-W %s\" -- errno = %d\n", optarg, errno);
                ErrorFlag++;
            }
            break;

        case 'F':                      // Read Keys from file
            {
                FILE *Pfile;
                char Buffer[BUFSIZ];
                Word_t  KeyValue;

                keyfile = optarg;
                errno  = 0;
                FValue = 0;             // numb non-zero Keys in file
                if ((Pfile = fopen(keyfile, "r")) == (FILE *) NULL)
                {
                    printf("\nIllegal argument to \"-F %s\" -- errno = %d\n", optarg, errno);
                    printf("Cannot open file \"%s\" to read it\n", optarg);
                    ErrorFlag++;
                    exit(1);
                }
                fprintf(stderr, "\n# Reading \"%s\" Key file.", keyfile);
                while (fgets(Buffer, BUFSIZ, Pfile) != (char *)NULL)
                {
                    if ((FValue % 1000000) == 0) fprintf(stderr, ".");
                    {
                        if (strtoul(Buffer, NULL, 0) != 0) 
                            FValue++;
                    }
                }
                fprintf(stderr, "\n# Number of Keys = %lu\n", FValue);
                if ((FileKeys = (PWord_t)malloc(FValue * sizeof(Word_t))) == 0)
                {
                    FAILURE("malloc failure, Bytes =", FValue * sizeof(Word_t));
                }
                fclose(Pfile);
                if ((Pfile = fopen(keyfile, "r")) == (FILE *) NULL)
                {
                    printf("\nIllegal argument to \"-F %s\" -- errno = %d\n", keyfile, errno);
                    printf("Cannot open file \"%s\" to read it\n", keyfile);
                    ErrorFlag++;
                    exit(1);
                }
                fprintf(stderr, "# Re-Reading Key file.");
                FValue = 0;
                while (fgets(Buffer, BUFSIZ, Pfile) != (char *)NULL)
                {
                    if ((FValue % 1000000) == 0) 
                        fprintf(stderr, ".");

                    if ((KeyValue = strtoul(Buffer, NULL, 0)) != 0) 
                    {
                        FileKeys[FValue++] = KeyValue;
                    }
                }
                fprintf(stderr, "\n");
                nElms = FValue;
                GValue = 0;
                break;
            }

        case 'm':                      // Turn on RAMMETRICS
            mFlag = 1;
            break;

        case 'p':                      // Turn on Printing of number set
            pFlag = 1;
            break;

        case 'b':                      // Turn on REAL bitmap testing
            bFlag = 1;
            break;

        case 'v':
            vFlag = 1;                 // time Searching
            break;

        case '1':                      // time Judy1
            J1Flag = 1;
            break;

        case 'L':                      // time JudyL
            JLFlag = 1;
            break;

        case 'H':                      // time JudyHS
            JHFlag = 1;
            break;

        case 'd':                      // time Judy1Unset JudyLDel
            dFlag = 1;
            break;

        case 'D':                      // bit reverse the data stream
            DFlag = 1;
            break;

        case 'c':                      // time Copy Judy1 array
            cFlag = 1;
            break;

        case 'C':                      // time Counting
            CFlag = 1;
            break;

        case 'I':                      // time duplicate insert/set
            IFlag = 1;
            break;

        case 'l':                      // do not loop in tests
            lFlag = 1;
            break;

        case 't':                      // print Number Generator cost
            tFlag = 1;
            break;

        case 'x':                      // Turn ON 'waiting for Context Switch'
            xFlag = 1;
            break;

        case 'V':                      // Turn OFF verify it flag in JudyLGet
            VFlag = 0;
            break;

        case 'f':                      // Turn on the flush flag after printf cycle
            fFlag = 0;
            break;

        case 'g':                      // do a Get after an Ins
            gFlag = 1;
            break;

        case 'i':                      // do a Ins after an Ins
            iFlag = 1;
            break;

        default:
            ErrorFlag++;
            break;
        }
        if (FValue)
            break;
    }

//  build the Random Number Generator starting seeds
    PStartSeed = RandomInit(BValue, GValue);
    if (PStartSeed == (PSeed_t) NULL)
    {
        printf("\nIllegal Number in -G%lu !!!\n", GValue);
        ErrorFlag++;
    }

    if (ErrorFlag)
    {
        printf
            ("\n<<< Program to do performance measurements on Judy Arrays >>>\n\n");
        printf("%s -n# -P# -S# -B# -T# -G# -s# -X# -1LHbIcCvdtDl\n\n", argv[0]);
        printf("Where: (default value is shown as [#])\n");
        printf("-n <#>  Max number of Keys used in JudyLGet/1Test [10000000]\n");
        printf("-P <#>  Measurement points per decade (1..1000) [50]\n");
        printf("-S <#>  Number Generator skip amount, 0 == Random [0]\n");
        printf
            ("-B <#>  Significant bits output (15..%d) in random Number Generator [32]\n",
             (int)sizeof(Word_t) * 8);
        printf
            ("-G <#>  Type (0..4) of random numbers 0==flat spectrum, 1..4==Gaussian [0]\n");
        printf
            ("-X <#>  Scale the numbers produced by '-m' flag (for plotting) [%d]\n", XScale);
        printf
            ("-F <filename>  Ascii file of Keys, zeros ignored -- must be last option!!!\n");
        printf("-1      Time Judy1\n");
        printf("-L      Time JudyL\n");
        printf("-H      Time JudyHS\n");
        printf("-b      Time a REAL BITMAP (-B # sets size == 2^<#>) [-B32]\n");
        printf("-I      Time DUPLICATE (already in array) JudyIns/Set times\n");
        printf("-c      Time copying a Judy1 Array\n");
        printf("-C      Include timing of JudyCount tests\n");
        printf("-v      Include timing of Judy First/Last/Next/Prev tests\n");
        printf("-d      Include timing of Del/Unset\n");
        printf("-p      Print number set used for testing (Diag)\n");
        printf
            ("-V      Turn OFF JudyLGet() verification tests (saving a cache-hit on 'Value')\n");
        printf
            ("-x      Turn ON 'waiting for context switch' flag -- smoother plots??\n");
        printf
            ("-t      Print measured 'overhead' (nS) subtracted out (Number Generator, etc)\n");
        printf
            ("-D      'Mirror' Numbers from Generator (as in binary viewed with a mirror)\n");
        printf
            ("-l      Do not smooth data with loops on same Keys (JudyGet/Test)\n");
        printf
            ("-T <#>  Number of Keys to average JudyGet/Test times, 0 == MAX [1000000]\n");
        printf("-s <#>  Starting number in Number Generator [0x%lx]\n",
               StartSequent);
        printf("-g      Do a Get/Test right after every Ins/Set (Diag only)\n");
        printf("-i      Do a Ins/Set right after every Ins/Set (Diag only)\n");
        printf("\n");

        exit(1);
    }

//  Set MSB number of Random bits in LFSR
    RandomBit = 1UL << (BValue - 1);

    if (SValue)                         // if sequential numbers
    {
        PStartSeed->Seeds[0] = StartSequent;
    }
    else
    {
        MaxNumb = (RandomBit * 2) - 1;
        if (nElms > MaxNumb)
        {
            printf("# Trim Max number of Elements -n%lu due to max -B%lu bit Keys\n",
                   MaxNumb, BValue);
            nElms = MaxNumb;
        }
        if (GValue != 0)
        {
            if (nElms > (MaxNumb >> 1))
            {
                printf
                    ("# Trim Max number of Elements -n%lu to -n%lu due to -G%lu spectrum of Keys\n",
                     MaxNumb, MaxNumb >> 1, GValue);
                nElms = MaxNumb >> 1;
            }
        }
    }

    StartSeed = *PStartSeed;            // save a copy

//  Print out the number set used for testing
    if (pFlag)
    {
        Word_t ii;
// 1      Word_t PrevPrintKey = 0;

        for (ii = 0; ii < nElms; ii++)
        {
            Word_t PrintKey;
// 1          Word_t LeftShift;
// 1          Word_t RightShift;

            PrintKey = GetNextKey(&StartSeed);

// 1          LeftShift  = __builtin_popcountll(PrintKey ^ (PrevPrintKey << 1));
// 1          RightShift = __builtin_popcountll(PrintKey ^ (PrevPrintKey >> 1));
// 1          PrevPrintKey = PrintKey;

#ifdef __LP64__
            printf("0x%016lx\n", PrintKey);
// 1          printf("0x%016lx %lu %lu, %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#else   // ! __LP64__
            printf("0x%08lx\n", PrintKey);
// 1          printf("0x%08lx %lu %lu, %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#endif  // ! __LP64__

        }
        exit(0);
    }


//  print Title for plotting -- command + run arguements
    printf("# TITLE %s -", argv[0]);
    if (J1Flag)
        printf("1");
    if (JLFlag)
        printf("L");
    if (JHFlag)
        printf("H");
    if (bFlag)
        printf("b");
    if (tFlag)
        printf("t");
    if (DFlag)
        printf("D");
    if (dFlag)
        printf("d");
    if (cFlag)
        printf("c");
    if (CFlag)
        printf("C");
    if (IFlag)
        printf("I");
    if (lFlag)
        printf("l");
    if (vFlag)
        printf("v");
    if (mFlag)
        printf("m");
    if (pFlag)
        printf("p");
    if (gFlag)
        printf("g");
    if (iFlag)
        printf("i");
    if (xFlag)
        printf("x");
    if (!VFlag)
        printf("V");

//  print more options - default, adjusted or otherwise
    printf(" -n%lu -T%lu -P%lu -X%d", nElms, TValues, PtsPdec, XScale);

    if (SValue)
        printf(" -S%lu -s%lu", SValue, StartSequent);
    else                                // some flavor of random Keys
        printf(" -B%lu -G%lu", BValue, GValue);

    if (FValue)
        printf(" -F %s", keyfile);
    printf("\n");

    if (mFlag)
    {
        int       count = 0;
        if (JLFlag)
            count++;
        if (J1Flag)
            count++;
        if (JHFlag)
            count++;
        if (bFlag)
            count++;

        if (count != 1)
        {
            printf
                (" ========================================================\n");
            printf
                (" Sorry, '-m' measurements compatable with only ONE of -1LHb.\n");
            printf
                (" This is because Judy object measurements include RAM sum of all.\n");
            printf
                (" ========================================================\n");
            exit(1);
        }
    }
    if (tFlag)
    {
        if (mFlag == 0)
        {
            printf
                (" ========================================================\n");
            printf(" Sorry, '-t' measurements must include '-m' set\n");
            printf
                (" ========================================================\n");
            exit(1);
        }
    }

//  uname(2) strings describing the machine
    {
        struct utsname ubuf;            // for system name

        if (uname(&ubuf) == -1)
            printf("# Uname(2) failed\n");
        else
            printf("# %s %s %s %s %s\n", ubuf.sysname, ubuf.nodename,
                   ubuf.release, ubuf.version, ubuf.machine);
    }
    if (sizeof(Word_t) == 8)
        printf("# %s 64 Bit version\n", argv[0]);
    else if (sizeof(Word_t) == 4)
        printf("# %s 32 Bit version\n", argv[0]);

    printf("# XLABEL Array Population\n");
    printf("# YLABEL Nano-Seconds -or- Words per %d Key(s)\n", XScale);

// ============================================================
// CALCULATE NUMBER OF MEASUREMENT GROUPS based on measurement points/decade
// ============================================================

//  Calculate Multiplier for number of points per decade
//  Note: Fix, this algorithm chokes at about 1000 points/decade
    DMult = pow(10.0, 1.0 / (double)PtsPdec);
    {
        double    Dsum;
        Word_t    Isum, prevIsum;

        prevIsum = Isum = 1;
        Dsum = (double)Isum;

//      Count number of measurements needed (10K max)
        for (Groups = 2; Groups < 10000; Groups++)
        {
            NextSum(&Isum, &Dsum, DMult);

            if (Dsum > nElms)
                break;

            prevIsum = Isum;
        }

//      Get memory for measurements saveing measurements
        Pms = (Pms_t) calloc(Groups, sizeof(ms_t));

//      Calculate number of Keys for each measurement point
        prevIsum = 0;
        Isum = 0;
        Dsum = 1.0;
        for (grp = 0; grp < Groups; grp++)
        {
            NextSum(&Isum, &Dsum, DMult);

            if (Dsum > (double)nElms || Isum > nElms)
            {
                Isum = nElms;
            }
            Pms[grp].ms_delta = Isum - prevIsum;

            prevIsum = Isum;

            if (Pms[grp].ms_delta == 0)
                break;                  // for very high -P#
        }
    }                                   // Groups = number of sizes
    if (GValue)
    {
        if (CFlag || vFlag || dFlag)
        {
            printf
                ("# ========================================================\n");
            printf
                ("#     Sorry, '-C' '-v' '-d' test(s) not compatable with -G[1..4].\n");
            printf
                ("#     This is because -G[1..4] generates duplicate numbers.\n");
            printf
                ("# ========================================================\n");
            dFlag = vFlag = CFlag = 0;
        }
    }

// ============================================================
// PRINT HEADER TO PERFORMANCE TIMERS
// ============================================================

    Col = 1;
    printf("# COLHEAD %2d Population of the Array\n", Col++);
    printf("# COLHEAD %2d Delta Inserts to the Array\n", Col++);
    printf("# COLHEAD %2d Number of Measurments done by Get/Test\n", Col++);

    if (tFlag)
        printf
            ("# COLHEAD %2d 'Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
             Col++);
    if (J1Flag)
        printf("# COLHEAD %2d J1S  - Judy1Set\n", Col++);
    if (JLFlag)
        printf("# COLHEAD %2d JLI  - JudyLIns\n", Col++);
    if (JHFlag)
        printf("# COLHEAD %2d JHSI - JudyHSIns\n", Col++);
    if (bFlag)
        printf("# COLHEAD %2d BMSet  - Bitmap Set\n", Col++);

    if (tFlag)
        printf
            ("# COLHEAD %2d 'Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
             Col++);
    if (J1Flag)
        printf("# COLHEAD %2d J1T  - Judy1Test\n", Col++);
    if (JLFlag)
        printf("# COLHEAD %2d JLG  - JudyLGet\n", Col++);
    if (JHFlag)
        printf("# COLHEAD %2d JHSG - JudyHSGet\n", Col++);
    if (bFlag)
        printf("# COLHEAD %2d BMTest  - Bitmap Test\n", Col++);

    if (IFlag)
    {
        if (J1Flag)
            printf("# COLHEAD %2d J1S-duplicate entry\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLI-duplicate entry\n", Col++);
        if (JHFlag)
            printf("# COLHEAD %2d JHSI-duplicate entry\n", Col++);
    }
    if (cFlag && J1Flag)
    {
        printf("# COLHEAD %2d Copy J1N->J1S loop\n", Col++);
    }
    if (CFlag)
    {
        if (J1Flag)
            printf("# COLHEAD %2d J1C\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLC\n", Col++);
    }
    if (vFlag)
    {
        if (J1Flag)
            printf("# COLHEAD %2d J1N\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLN\n", Col++);
        if (J1Flag)
            printf("# COLHEAD %2d J1P\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLP\n", Col++);
        if (J1Flag)
            printf("# COLHEAD %2d J1NE\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLNE\n", Col++);
        if (J1Flag)
            printf("# COLHEAD %2d J1PE\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLPE\n", Col++);
    }
    if (dFlag)
    {
        if (J1Flag)
            printf("# COLHEAD %2d J1U\n", Col++);
        if (JLFlag)
            printf("# COLHEAD %2d JLD\n", Col++);
        if (JHFlag)
            printf("# COLHEAD %2d JHSD\n", Col++);
    }
    if (J1Flag)
    {
        printf
            ("# COLHEAD %2d 1heap/I  - Judy1 malloc'ed words per Key\n",
             Col++);
    }
    if (JLFlag)
    {
        printf
            ("# COLHEAD %2d Lheap/I  - JudyL malloc'ed words per Key\n",
             Col++);
    }
    if (JHFlag)
    {
        printf
            ("# COLHEAD %2d HSheap/I - JudyHS malloc'ed words per Key\n",
             Col++);
    }
    if (mFlag)
    {

#ifdef  JUDYA
        printf("# COLHEAD %2d JBB - Bitmap node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d JBU - 256 node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d JBL - Linear node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d LW  - Leaf Word_t Key/Key\n", Col++);
        printf("# COLHEAD %2d L7  - Leaf 7 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L6  - Leaf 6 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L5  - Leaf 5 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L4  - Leaf 4 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L3  - Leaf 3 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L2  - Leaf 2 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L1  - Leaf 1 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d B1  - Bitmap Leaf 1 Bit Key/Key\n", Col++);
        printf("# COLHEAD %2d VA  - Value area Words/Key\n", Col++);
#endif  // JUDYA  

#ifdef  JUDYB
        printf("# COLHEAD %2d B4     - 16 node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d B8     - 256 node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d B16    - 65536 node Branch Words/Key\n", Col++);
        printf("# COLHEAD %2d LW     - Leaf Word_t Key/Key\n", Col++);
        printf("# COLHEAD %2d L32    - Leaf 32 bit Key/Key\n", Col++);
        printf("# COLHEAD %2d L16    - Leaf 16 bit Key/Key\n", Col++);
        printf("# COLHEAD %2d L12    - Leaf 4096 bit Key/Key\n", Col++);
        printf("# COLHEAD %2d V12    - Value area Words/Key\n", Col++);
        printf("# COLHEAD %2d #Cmprs - Average number missed Compares Per Leaf Search\n", Col++);
        printf("# COLHEAD %2d %%Cmps - Average %% of maximum Leaf Search\n", Col++);
#endif  // JUDYB   

        printf("# COLHEAD %2d TrDep  - Tree depth with LGet/1Test searches\n", Col++);
        printf("# COLHEAD %2d MalOvd/I - Judy malloc'ed words per Key\n",
               Col++);

        if (J1Flag)
            printf
                ("# COLHEAD %2d MF1/I    - Judy1 average malloc+free's per Key\n",
                 Col++);
        if (JLFlag)
            printf
                ("# COLHEAD %2d MFL/I    - JudyL average malloc+free's per Key\n",
                 Col++);
        if (JHFlag)
            printf
                ("# COLHEAD %2d MFHS/I   - JudyHS average malloc+free's per Key\n",
                 Col++);
    }
    if (J1Flag)
        printf("# %s - Leaf sizes in Words\n", Judy1MallocSizes);

    if (JLFlag)
        printf("# %s - Leaf sizes in Words\n#\n", JudyLMallocSizes);


    if (bFlag && (BValue > 31))
    {
        printf("# ========================================================\n");
        printf
            ("#     WARNING '-b' option with '-B%lu' option will malloc() a\n",
             BValue);
        printf("#     fixed sized Bitmap of %lu bytes.\n", 1UL << (BValue - 3));
        printf("# ========================================================\n");
        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

// ============================================================
// Warm up the cpu
// ============================================================

//  Try to fool compiler and really execute random() WARMUPCPU times
    STARTTm;
    for (MaxNumb = ii = 0; ii < Warmup; ii++)
        MaxNumb = random();

    ENDTm(DeltanSecW);

    if (MaxNumb == 0)           // never happpen?
        printf("\n");

    printf("# Warm up CPU (-W %lu) =  %3.1f mSec, random() = %4.2f nSec per/call\n", 
            Warmup, DeltanSecW/1E6, DeltanSecW/Warmup);
    printf("#\n");

// ============================================================
// PRINT COLUMNS HEADER TO PERFORMANCE TIMERS
// ============================================================

    PrintHeader();

// ============================================================
// BEGIN TESTS AT EACH GROUP SIZE
// ============================================================

    InsertSeed = StartSeed;             // for JudyIns
    BitmapSeed = StartSeed;             // for bitmaps
    LastPPop = 100.0;

    for (Pop1 = grp = 0; grp < Groups; grp++)
    {
        Word_t    Delta;
        double    DeltaGen1;
        double    DeltaGenL;
        double    DeltaGenHS;

        Delta = Pms[grp].ms_delta;

        if (Delta == 0)
            break;

//      Accumulate the Total population of arrays
        Pop1 += Delta;

//      Only test for a maximum of TValues if not zero
        if (TValues && Pop1 > TValues)  // Trim measurments?
            Meas = TValues;
        else
            Meas = Pop1;

        if ((double)Pop1 >= LastPPop)
        {
            LastPPop *= 10.0;
            PrintHeader();
        }

        printf("%11lu %10lu %10lu", Pop1, Delta, Meas);

        if (J1Flag || JLFlag || JHFlag)
        {
//          Test J1S, JLI, JLHS
//          Exit with InsertSeed/Key ready for next batch
//
            Tit = 0;                    // exclude Judy
            DummySeed = InsertSeed;
            WaitForContextSwitch(Delta);
            TestJudyIns(&J1, &JL, &JH, &DummySeed, Delta);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyIns(&J1, &JL, &JH, &InsertSeed, Delta);
            if (J1Flag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGen1);
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            }
            if (JLFlag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGenL);
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            }
            if (JHFlag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGenHS);
                DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
            }
            if (fFlag)
                fflush(NULL);

//      Note: the Get/Test code always tests from the "first" Key inserted.
//      The assumption is the "just inserted" Key would be unfair because
//      much of that would still be "in the cache".  Hopefully, the 
//      "just inserted" would flush out the cache.  However, the Array 
//      population has to grow beyond "TValues" before that will happen.
//      Currently, the default value of TValues is 1000000.  As caches
//      get bigger this value should be changed to a bigger value.

//          Test J1T, JLG, JHSG

            Tit = 0;                    // exclude Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyGet(J1, JL, JH, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;

            Tit = 1;                    // include Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyGet(J1, JL, JH, &BeginSeed, Meas);

            TreeDepth        = j__TreeDepth;
            SearchCompares   = j__SearchCompares;
            SearchPopulation = j__SearchPopulation;

            if (J1Flag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGen1);
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            }
            if (JLFlag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGenL);
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            }
            if (JHFlag)
            {
                if (tFlag)
                    PRINT6_1f(DeltaGenHS);
                DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
            }
            if (fFlag)
                fflush(NULL);
        }

//      Test a REAL bitmap
        if (bFlag)
        {
            double    DeltanBit;

//          Allocate a Bitmap, if not already done so
            if (B1 == NULL)
            {
                Word_t    ii;
                size_t    BMsize;

                BMsize = 1UL << (BValue - 3);
                B1 = (PWord_t)malloc(BMsize);
                if (B1 == (PWord_t)NULL)
                {
                    FAILURE("malloc failure, Bytes =", BMsize);
                }
//              clear the bitmap and bring into RAM
                for (ii = 0; ii < (BMsize / sizeof(Word_t)); ii++)
                    B1[ii] = 0;
            }
            DummySeed = BitmapSeed;
            GetNextKey(&DummySeed);   // warm up cache

            Tit = 0;
            DummySeed = BitmapSeed;
            WaitForContextSwitch(Delta);
            TestBitmapSet(B1, &DummySeed, Delta);
            DeltanBit = DeltanSecBt;

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestBitmapSet(B1, &BitmapSeed, Delta);

            if (tFlag)
                PRINT6_1f(DeltanBit);
            DONTPRINTLESSTHANZERO(DeltanSecBt, DeltanBit);

            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestBitmapTest(B1, &BeginSeed, Meas);
            DeltanBit = DeltanSecBt;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestBitmapTest(B1, &BeginSeed, Meas);

            if (tFlag)
                PRINT6_1f(DeltanBit);
            DONTPRINTLESSTHANZERO(DeltanSecBt, DeltanBit);
            if (fFlag)
                fflush(NULL);
        }

//      Test J1T, JLI, JHSI - duplicates

        if (IFlag)
        {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDup(&J1, &JL, &JH, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDup(&J1, &JL, &JH, &BeginSeed, Meas);
            if (J1Flag)
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            if (JLFlag)
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (JHFlag)
                DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
            if (fFlag)
                fflush(NULL);
        }
        if (cFlag && J1Flag)
        {
            WaitForContextSwitch(Meas);
            TestJudy1Copy(J1, Meas);
            PRINT6_1f(DeltanSec1);
            if (fFlag)
                fflush(NULL);
        }
        if (CFlag)
        {
            Tit = 0;
            WaitForContextSwitch(Meas);
            TestJudyCount(J1, JL, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;

            Tit = 1;
            WaitForContextSwitch(Meas);
            TestJudyCount(J1, JL, Meas);
            if (J1Flag)
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            if (JLFlag)
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (fFlag)
                fflush(NULL);
        }
        if (vFlag)
        {
//          Test J1N, JLN
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNext(J1, JL, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNext(J1, JL, Meas);
            if (J1Flag)
                PRINT6_1f(DeltanSec1);
            if (JLFlag)
                PRINT6_1f(DeltanSecL);
            if (fFlag)
                fflush(NULL);

//          Test J1P, JLP
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrev(J1, JL, ~0UL, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrev(J1, JL, ~0UL, Meas);
            if (J1Flag)
                PRINT6_1f(DeltanSec1);
            if (JLFlag)
                PRINT6_1f(DeltanSecL);
            if (fFlag)
                fflush(NULL);

//          Test J1NE, JLNE
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
            if (J1Flag)
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            if (JLFlag)
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (fFlag)
                fflush(NULL);

//          Test J1PE, JLPE
//
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
            if (J1Flag)
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            if (JLFlag)
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (fFlag)
                fflush(NULL);
        }

//      Test J1U, JLD, JHSD
        if (dFlag)
        {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDel(&J1, &JL, &JH, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDel(&J1, &JL, &JH, &BeginSeed, Meas);
            if (J1Flag)
                DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
            if (JLFlag)
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (JHFlag)
                DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
            if (fFlag)
                fflush(NULL);

//          Now put back the Just deleted Keys
            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyIns(&J1, &JL, &JH, &BeginSeed, Meas);
        }

        if (J1Flag)
            PRINT7_3f((double)TotJudy1MemUsed / (double)Pop1);

        if (JLFlag)
            PRINT7_3f((double)TotJudyLMemUsed / (double)Pop1);

        if (JHFlag)
            PRINT7_3f((double)TotJudyHSMemUsed / (double)Pop1);

        if (mFlag && (bFlag == 0))
        {
            double AveSrcCmp, PercentLeafSearched;
            
//          Calc average compares done in Leaf for this measurement interval
            AveSrcCmp = SearchCompares / (double)Meas;

//          Calc average percent of Leaf searched
            if (SearchPopulation == 0)
                PercentLeafSearched = 0.0;
            else
                PercentLeafSearched = SearchCompares / SearchPopulation * 100.0;

#ifdef  JUDYA
            PRINT5_2f((double)j__AllocWordsJBB   / (double)Pop1);       // 16 node branch
            PRINT5_2f((double)j__AllocWordsJBU   / (double)Pop1);       // 256 node branch
            PRINT5_2f((double)j__AllocWordsJBL   / (double)Pop1);       // 16384 node branch
#endif  // JUDYA  

#ifdef  JUDYB
            PRINT5_2f((double)j__AllocWordsJBU4  / (double)Pop1);       // 16 node branch
            PRINT5_2f((double)j__AllocWordsJBU8  / (double)Pop1);       // 256 node branch
            PRINT5_2f((double)j__AllocWordsJBU16 / (double)Pop1);       // 16384 node branch
#endif  // JUDYB   

            PRINT5_2f((double)j__AllocWordsJLLW  / (double)Pop1);       // 32[64] Key

#ifdef  JUDYA
            PRINT5_2f((double)j__AllocWordsJLL7  / (double)Pop1);       // 32 bit Key
            PRINT5_2f((double)j__AllocWordsJLL6  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL5  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL4  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL3  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL2  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJLL1  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJLB1  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJV    / (double)Pop1);       // Values for 12 bit
#endif  // JUDYA  

#ifdef  JUDYB
            PRINT5_2f((double)j__AllocWordsJL32  / (double)Pop1);       // 32 bit Key
            PRINT5_2f((double)j__AllocWordsJL16  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJL12  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJV12  / (double)Pop1);       // Values for 12 bit

//          print average number of failed compares done in leaf search
            printf(" %6.1f", AveSrcCmp);

//          print average percent of Leaf searched (with compares)
            printf(" %5.1f", PercentLeafSearched);
#endif  // JUDYB   

//          print average number of Branches traversed per lookup
            printf(" %5.1f", TreeDepth / (double)Meas);

//          reset for next measurement
            j__SearchPopulation = j__TreeDepth = j__SearchCompares = 0;

//          print the number of words used in malloc overhead per Key
            PRINT5_2f((double)TotMalOvdWords / (double)Pop1);

            if (J1Flag)
                PRINT5_2f((double)DeltaMalFre1);
            if (JLFlag)
                PRINT5_2f((double)DeltaMalFreL);
            if (JHFlag)
                PRINT5_2f((double)DeltaMalFreHS);
        }

        printf("\n");
        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

#ifdef SKIPMACRO
    if (J1Flag)
        Count1 = Judy1Count(J1, 0, -1, PJE0);
    if (JLFlag)
        CountL = JudyLCount(JL, 0, -1, PJE0);
#else
    if (J1Flag)
        J1C(Count1, J1, 0, -1);
    if (JLFlag)
        JLC(CountL, JL, 0, -1);         // get the counts
#endif // SKIPMACRO

    if (JLFlag && J1Flag)
    {
        if (CountL != Count1)
            FAILURE("Judy1/LCount not equal", Count1);
    }

    if (J1Flag)
    {
        STARTTm;

#ifdef SKIPMACRO
        Bytes = Judy1FreeArray(&J1, PJE0);
#else
        J1FA(Bytes, J1);                // Free the Judy1 Array
#endif // SKIPMACRO

        ENDTm(DeltanSec1);

        DeltanSec1 /= (double)Count1;

        printf
            ("# Judy1FreeArray:  %lu, %0.3f Words/Key, %lu Bytes, %0.2f NSec/Key\n",
             Count1, (double)(Bytes / sizeof(Word_t)) / (double)Count1, Bytes,
             DeltanSec1);

    }

    if (JLFlag)
    {
        STARTTm;

#ifdef SKIPMACRO
        Bytes = JudyLFreeArray(&JL, PJE0);
#else
        JLFA(Bytes, JL);                // Free the JudyL Array
#endif // SKIPMACRO

        ENDTm(DeltanSecL);

        DeltanSecL /= (double)CountL;
        printf
            ("# JudyLFreeArray:  %lu, %0.3f Words/Key, %lu Bytes, %0.2f NSec/Key\n",
             CountL, (double)(Bytes / sizeof(Word_t)) / (double)CountL, Bytes,
             DeltanSecL);
    }

    if (JHFlag)
    {

        STARTTm;

#ifdef SKIPMACRO
        Bytes = JudyHSFreeArray(&JH, PJE0);     // Free the JudyHS Array
#else
        JHSFA(Bytes, JH);               // Free the JudyHS Array
#endif // SKIPMACRO

        ENDTm(DeltanSecHS);

        DeltanSecHS /= (double)nElms;   // no Counts yet
        printf
            ("# JudyHSFreeArray: %lu, %0.3f Words/Key, %0.2f NSec/Key\n",
             nElms, (double)(Bytes / sizeof(Word_t)) / (double)nElms,
             DeltanSecHS);
    }

//    if (bFlag && GValue)
//         printf("\n# %lu Duplicate Keys were found with -G%lu\n", BitmapDups, GValue);

    exit(0);
}

#ifdef DEADCODE
// This code is for testing the basic timing loop used everywhere else
// Enable it for whatever you want
#undef __FUNCTI0N__
#define __FUNCTI0N__ "TimeNumberGen"

int
TimeNumberGen(void **TestRan, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Word_t    DummyAccum = 0;
    Seed_t    WorkingSeed;

    if (Elements < 100)
        Loops = (MAXLOOPS / Elements) + MINLOOPS;
    else
        Loops = 1;

    if (lFlag)
        Loops = 1;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;
        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);
            DummyAccum += TstKey;     // prevent cc from optimizing out
        }
        ENDTm(DeltanSec);

        if (DminTime > DeltanSec)
        {
            icnt = ICNT;
            if (DeltanSec > 0.0)        // Ignore 0
                DminTime = DeltanSec;
        }
        else
        {
            if (--icnt == 0)
                break;
        }
    }
    DeltanSec = DminTime / (double)Elements;

//  A little dummy code to not allow compiler to optimize out
    if (DummyAccum == 1234)
        printf("Do not let 'cc' optimize out\n");

    return 0;
}
#endif // DEADCODE

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyIns"

// static int Flag = 0;

int
TestJudyIns(void **J1, void **JL, void **JH, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    Seed_t    WorkingSeed;
    int       Rc = 0;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Word_t    StartMallocs;

    DeltanSec1 = 0.0;
    DeltanSecL = 0.0;
    DeltanSecHS = 0.0;

    WorkingSeed = *PSeed;

    if (Elements < 100)
        Loops = (MAXLOOPS / Elements) + MINLOOPS;
    else
        Loops = 1;

    if (lFlag)
        Loops = 1;

//  Judy1Set timings

    if (J1Flag)
    {
        MalFlag = JudyMal1;
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0)                // Remove previously inserted
            {
                for (elm = 0; elm < Elements; elm++)
                {
                    TstKey = GetNextKey(&WorkingSeed);

#ifdef SKIPMACRO
                    Rc = Judy1Unset(J1, TstKey, PJE0);
#else
                    J1U(Rc, *J1, TstKey);
#endif // SKIPMACRO

                }
            }

            StartMallocs = MalFreeCnt;
            WorkingSeed = *PSeed;
            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {
                    J1S(Rc, *J1, TstKey);
                    if (Rc == 0)
                    {
                        if (GValue)
                        {
                            Judy1Dups++;
                        }
                        else
                        {
                            printf("\nTstKey = 0x%lx\n", TstKey);
                            FAILURE("Judy1Set failed - DUP Key at elm", elm);
                        }
                    }
                    if (iFlag)
                    {
                        J1S(Rc, *J1, TstKey);
                        if (Rc != 0)
                        {
                            printf
                            ("\nJudy1Set Rc = %d after Judy1Set, Key = 0x%lx, elm = %lu",
                             Rc, TstKey, elm);
                            FAILURE("Judy1Test failed at", elm);
                        }
                    }
                    if (gFlag)
                    {
                        Rc = Judy1Test(*J1, TstKey, PJE0);
                        if (Rc != 1)
                        {
                            printf
                            ("\nJudy1Test Rc = %d after Judy1Set, Key = 0x%lx, elm = %lu",
                             Rc, TstKey, elm);
                            FAILURE("Judy1Test failed at", elm);
                        }
                    }

#ifdef DIAG
//printf("Try reinsert dup Key = 0x%lx\n", TstKey);
                    J1S(Rc, *J1, TstKey);
                    if (Rc != 0)
                        FAILURE("Judy1Set failed at", elm);

//                    if (TstKey == 0x521c2) Flag++;
//
//                    if (Flag)
//                    {
//                    J1S(Rc, *J1, 0x521c2);
//                    if (Rc != 0)
//                        printf("0x521c2 successful -- wrong\n");
//                    }
#endif // DIAG

                }
            }
            ENDTm(DeltanSec1);
            DeltaMalFre1 = (double)(MalFreeCnt - StartMallocs) / Elements;

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

//  JudyLIns timings

    if (JLFlag)
    {
        MalFlag = JudyMalL;
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0)                // Remove previously inserted
            {
                for (elm = 0; elm < Elements; elm++)
                {
                    TstKey = GetNextKey(&WorkingSeed);

#ifdef SKIPMACRO
                    Rc = JudyLDel(JL, TstKey, PJE0);
#else
                    JLD(Rc, *JL, TstKey);
#endif // SKIPMACRO

                }
            }

            StartMallocs = MalFreeCnt;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
#else
                    JLI(PValue, *JL, TstKey);
#endif // SKIPMACRO
                    if (PValue == (PWord_t)NULL)
                    {
                        FAILURE("JudyLIns failed - NULL PValue", TstKey);
                    }
                    if (*PValue == TstKey)
                    {
                        if (GValue)
                        {
                            JudyLDups++;
                        }
                        else
                        {
                            printf("TstKey = 0x%lx", TstKey);
                            FAILURE("JudyLIns failed - DUP Key =", TstKey);
                        }
                    }
                    if (VFlag)
                    {
                        *PValue = TstKey;     // save Key in Value

                        if (iFlag)
                        {
                            PWord_t   PValueNew;

                            PValueNew = (PWord_t)JudyLIns(JL, TstKey, PJE0);
                            if (PValueNew == NULL)
                            {
                                printf("\nTstKey = 0x%lx\n", TstKey);
                                FAILURE("JudyLIns failed with NULL after Insert", TstKey);
                            }
                            else if (PValueNew != PValue)
                            {
                                printf("\n- PValueNew = 0x%lx, PValueold = 0x%lx\n", (Word_t)PValueNew, (Word_t)PValue);
                                printf("- ValueNew = 0x%lx, Valueold = 0x%lx\n", *PValueNew, *PValue);
                                FAILURE("Second JudyLIns failed with wrong PValue after Insert", TstKey);
                            }
                            else
                            {
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n*PValueNew = 0x%lx\n", *PValueNew);
                                    printf("TstKey = 0x%lx = %ld\n", TstKey, TstKey);
                                    FAILURE("Second JudyLIns failed with wrong *PValue after Insert", TstKey);
                                }
                            }
                        }
                        if (gFlag)
                        {
                            PWord_t   PValueNew;

                            PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                            if (PValueNew == NULL)
                            {
                                printf("\nTstKey = 0x%lx\n", TstKey);
                                FAILURE("JudyLGet failed after Insert", TstKey);
                            }
                            else
                            {
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n*PValueNew = 0x%lx\n", *PValueNew);
                                    printf("TstKey = 0x%lx = %ld\n", TstKey, TstKey);
                                    FAILURE("JudyLGet failed after Insert", TstKey);
                                }
                            }
                        }
                    }
                }
            }
            ENDTm(DeltanSecL);
            DeltanSecL /= Elements;
            DeltaMalFreL = (double)(MalFreeCnt - StartMallocs) / Elements;

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
    }

//  JudyHSIns timings

    if (JHFlag)
    {
        MalFlag = JudyMalHS;
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0)                // Remove previously inserted
            {
                for (elm = 0; elm < Elements; elm++)
                {
                    TstKey = GetNextKey(&WorkingSeed);

                    JHSD(Rc, *JH, &TstKey, sizeof(Word_t));
                }
            }

            StartMallocs = MalFreeCnt;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue =
                        (PWord_t)JudyHSIns(JH, &TstKey, sizeof(Word_t), PJE0);
#else
                    JHSI(PValue, *JH, &TstKey, sizeof(Word_t));
#endif // SKIPMACRO

                    if (*PValue == TstKey)
                    {
                        if (GValue)
                            JudyHSDups++;
                        else
                            FAILURE("JudyHSIns failed - DUP Key =", TstKey);
                    }
                    *PValue = TstKey; // save Key in Value
                }
            }
            ENDTm(DeltanSecHS);
            DeltanSecHS /= Elements;
            DeltaMalFreHS = (double)(MalFreeCnt - StartMallocs) / Elements;

            if (DminTime > DeltanSecHS)
            {
                icnt = ICNT;
                if (DeltanSecHS > 0.0)  // Ignore 0
                    DminTime = DeltanSecHS;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
    }

    *PSeed = WorkingSeed;
    return 0;
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDup"

int
TestJudyDup(void **J1, void **JL, void **JH, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    Seed_t    WorkingSeed;
    int       Rc;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        MalFlag = JudyMal1;
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {
                    J1S(Rc, *J1, TstKey);
                    if (Rc != 0)
                        FAILURE("Judy1Test Rc != 0", Rc);
                }
            }
            ENDTm(DeltanSec1);

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    icnt = ICNT;

    if (JLFlag)
    {
        MalFlag = JudyMalL;
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
#else
                    JLI(PValue, *JL, TstKey);
#endif // SKIPMACRO

                    if (PValue == (Word_t *)NULL)
                        FAILURE("JudyLIns ret PValue = NULL", 0L);
                    if (VFlag && (*PValue != TstKey))
                        FAILURE("JudyLIns ret wrong Value at", elm);
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

    icnt = ICNT;

    if (JHFlag)
    {
        MalFlag = JudyMalHS;
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue =
                        (PWord_t)JudyHSIns(JH, &TstKey, sizeof(Word_t), PJE0);
#else
                    JHSI(PValue, *JH, &TstKey, sizeof(Word_t));
#endif // SKIPMACRO

                    if (PValue == (Word_t *)NULL)
                        FAILURE("JudyHSGet ret PValue = NULL", 0L);
                    if (*PValue != TstKey)
                        FAILURE("JudyHSGet ret wrong Value at", elm);
                }
            }
            ENDTm(DeltanSecHS);

            if (DminTime > DeltanSecHS)
            {
                icnt = ICNT;
                if (DeltanSecHS > 0.0)  // Ignore 0
                    DminTime = DeltanSecHS;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecHS = DminTime / (double)Elements;
    }
    return 0;
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyGet"

int
TestJudyGet(void *J1, void *JL, void *JH, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    int       Rc;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Seed_t    WorkingSeed;
    PWord_t   DmyStackMem = NULL;

    if (PreStack)
            DmyStackMem = (Word_t *)alloca(PreStack);  // move stack a bit

    if (DmyStackMem == (PWord_t)1)                      // shut up compiler
    {
        printf("BUG -- fixme\n");
        exit(1);
    }

    Loops = (MAXLOOPS / Elements) + MINLOOPS;

    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        MalFlag = JudyMal1;
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

//          reset for next measurement
            j__SearchPopulation = j__TreeDepth = j__SearchCompares = 0;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {

#ifdef SKIPMACRO
                    Rc = Judy1Test(J1, TstKey, PJE0);
#else
                    J1T(Rc, J1, TstKey);
#endif // SKIPMACRO

                    if (Rc != 1)
                    {
                        printf
                            ("\nJudy1Test wrong Rc = %d, Key = 0x%lx, elm = %lu",
                             Rc, TstKey, elm);
                        FAILURE("Judy1Test Rc != 1", Rc);
                    }
                }
            }
            ENDTm(DeltanSec1);

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    icnt = ICNT;

    if (JLFlag)
    {
        MalFlag = JudyMalL;
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

//          reset for next measurement
            j__SearchPopulation = j__TreeDepth = j__SearchCompares = 0;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue = (PWord_t)JudyLGet(JL, TstKey, PJE0);
#else
                    JLG(PValue, JL, TstKey);
#endif // SKIPMACRO

                    if (PValue == (Word_t *)NULL)
                    {
//                        if (TstKey == 0)
                        {
                            printf("\nJudyGet Key = 0x%lx", TstKey);
                            FAILURE("JudyLGet ret PValue = NULL", 0L);
                        }
                    }
                    else if (VFlag && (*PValue != TstKey))
                    {
//                        if (TstKey == 0)
                        {
                            printf
                                ("JudyLGet returned Value=0x%lx, should be=0x%lx\n",
                                 *PValue, TstKey);
                            FAILURE("JudyLGet ret wrong Value at", elm);
                        }
                    }
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }

            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

    icnt = ICNT;

    if (JHFlag)
    {
        MalFlag = JudyMalHS;
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

            j__SearchPopulation = j__TreeDepth = j__SearchCompares = 0;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {

#ifdef SKIPMACRO
                    PValue = (PWord_t)JudyHSGet(JH, &TstKey, sizeof(Word_t));
#else
                    JHSG(PValue, JH, &TstKey, sizeof(Word_t));
#endif // SKIPMACRO

                    if (PValue == (Word_t *)NULL)
                        FAILURE("JudyHSGet ret PValue = NULL", 0L);
                    if (VFlag && (*PValue != TstKey))
                        FAILURE("JudyHSGet ret wrong Value at", elm);
                }
            }
            ENDTm(DeltanSecHS);

            if (DminTime > DeltanSecHS)
            {
                icnt = ICNT;
                if (DeltanSecHS > 0.0)  // Ignore 0
                    DminTime = DeltanSecHS;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecHS = DminTime / (double)Elements;
    }
    return 0;
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudy1Copy"

int
TestJudy1Copy(void *J1, Word_t Elements)
{
    Pvoid_t   J1a;                      // Judy1 new array
    Word_t    elm = 0;
    Word_t    Bytes;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;

    J1a = NULL;                         // Initialize To array

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    MalFlag = JudyMal1;
    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        Word_t    NextKey;
        int       Rc;

        NextKey = 0;                  // Start at the beginning
        elm = 0;
        STARTTm;

#ifdef SKIPMACRO
        Rc = Judy1First(J1, &NextKey, PJE0);
        while (Rc == 1)
        {
            if (elm++ == Elements)      // get out before whole array
                break;

            Rc = Judy1Set(&J1a, NextKey, PJE0);
            if (Rc != 1)
                FAILURE("Judy1Set at", elm);
            Rc = Judy1Next(J1, &NextKey, PJE0);
        }
#else  // not SKIPMACRO
        J1F(Rc, J1, NextKey);         // return first Key
        while (Rc == 1)
        {
            if (elm++ == Elements)      // get out before whole array
                break;

            J1S(Rc, J1a, NextKey);
            if (Rc != 1)
                FAILURE("J1S at", elm);
            J1N(Rc, J1, NextKey);
        }
#endif // SKIPMACRO

        ENDTm(DeltanSec1);

#ifdef SKIPMACRO
        Bytes = Judy1FreeArray(&J1a, PJE0);
#else
        J1FA(Bytes, J1a);               // no need to keep it around
#endif // SKIPMACRO

        if (DminTime > DeltanSec1)
        {
            icnt = ICNT;
            if (DeltanSec1 > 0.0)       // Ignore 0
                DminTime = DeltanSec1;
        }
        else
        {
            if (--icnt == 0)
                break;
        }
    }
    DeltanSec1 = DminTime / (double)elm;

    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyCount"

int
TestJudyCount(void *J1, void *JL, Word_t Elements)
{
    Word_t    elm;
    Word_t    Count1, CountL;
    Word_t    TstKey;
    int       Rc;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            TstKey = 0;

#ifdef SKIPMACRO
            Rc = Judy1First(J1, &TstKey, PJE0);
#else
            J1F(Rc, J1, TstKey);      // return first Key
#endif // SKIPMACRO

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                if (Tit)
                {

#ifdef SKIPMACRO
                    Count1 = Judy1Count(J1, 0, TstKey, PJE0);
#else
                    J1C(Count1, J1, 0, TstKey);
#endif // SKIPMACRO

                    if (Count1 != (elm + 1))
                    {
                        printf("Count1 = %lu, elm +1 = %lu\n", Count1, elm + 1);
                        FAILURE("J1C at", elm);
                    }
                }

#ifdef SKIPMACRO
                Rc = Judy1Next(J1, &TstKey, PJE0);
#else
                J1N(Rc, J1, TstKey);
#endif // SKIPMACRO

            }
            ENDTm(DeltanSec1);

            if (Rc == 1234)             // impossible value
                exit(-1);               // shut up compiler only

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    if (JLFlag)
    {
        Word_t   *PValue;

        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            TstKey = 0;

#ifdef SKIPMACRO
            PValue = (PWord_t)JudyLFirst(JL, &TstKey, PJE0);
#else
            JLF(PValue, JL, TstKey);  // return first Key
#endif // SKIPMACRO

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                if (Tit)
                {

#ifdef SKIPMACRO
                    CountL = JudyLCount(JL, 0, TstKey, PJE0);
#else
                    JLC(CountL, JL, 0, TstKey);
#endif // SKIPMACRO

                    if (CountL != (elm + 1))
                    {
                        printf("CountL = %lu, elm +1 = %lu\n", CountL, elm + 1);
                        FAILURE("JLC at", elm);
                    }
                }

#ifdef SKIPMACRO
                PValue = (PWord_t)JudyLNext(JL, &TstKey, PJE0);
#else
                JLN(PValue, JL, TstKey);
#endif // SKIPMACRO

                if (VFlag && PValue && (*PValue != TstKey))
                {
                    printf("\nPValue=0x%lx, *PValue=0x%lx, TstKey=0x%lx\n",
                           (Word_t)PValue, *PValue, TstKey);
                    FAILURE("JudyLNext ret bad *PValue at", elm);
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNext"

Word_t
TestJudyNext(void *J1, void *JL, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Word_t    JLKey;
    Word_t    J1Key;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            int       Rc;

            J1Key = 0;
            STARTTm;

#ifdef SKIPMACRO
            Rc = Judy1First(J1, &J1Key, PJE0);
#else
            J1F(Rc, J1, J1Key);
#endif // SKIPMACRO

            for (elm = 0; elm < Elements; elm++)
            {
                if (Rc != 1)
                {
                    printf("\nElements = %lu, elm = %lu\n", Elements, elm);
                    FAILURE("Judy1Next Rc != 1 =", Rc);
                }

#ifdef SKIPMACRO
                Rc = Judy1Next(J1, &J1Key, PJE0);
#else
                J1N(Rc, J1, J1Key);   // Get next one
#endif // SKIPMACRO

            }
            ENDTm(DeltanSec1);

            if ((TValues == 0) && (Rc != 0))
            {
                FAILURE("Judy1Next Rc != 0", Rc);
            }

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t   *PValue;

//          Get an Key low enough for Elements
            JLKey = 0;

            STARTTm;

#ifdef SKIPMACRO
            PValue = (PWord_t)JudyLFirst(JL, &JLKey, PJE0);
#else
            JLF(PValue, JL, JLKey);
#endif // SKIPMACRO

            for (elm = 0; elm < Elements; elm++)
            {
                Word_t    Prev;
                if (PValue == NULL)
                {
                    printf("\nElements = %lu, elm = %lu\n", Elements, elm);
                    FAILURE("JudyLNext ret NULL PValue at", elm);
                }
                if (VFlag && (*PValue != JLKey))
                {
                    printf("\n*PValue=0x%lx, JLKey=0x%lx\n", *PValue,
                           JLKey);
                    FAILURE("JudyLNext ret bad *PValue at", elm);
                }
                Prev = JLKey;

#ifdef SKIPMACRO
                PValue = (PWord_t)JudyLNext(JL, &JLKey, PJE0);
#else
                JLN(PValue, JL, JLKey);       // Get next one
#endif // SKIPMACRO

                if (JLKey == Prev)
                {
                    printf("OOPs, JLN did not advance 0x%lx\n", Prev);
                    FAILURE("JudyLNext ret did not advance", Prev);
                }
            }
            ENDTm(DeltanSecL);

            if ((TValues == 0) && (PValue != NULL))
            {
                FAILURE("JudyLNext PValue != NULL", PValue);
            }

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

//  perhaps a check should be done here -- if I knew what to expect.
    if (JLFlag)
        return (JLKey);
    if (J1Flag)
        return (J1Key);
    return (-1);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyPrev"

int
TestJudyPrev(void *J1, void *JL, Word_t HighKey, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Word_t    J1Key;
    Word_t    JLKey;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            int       Rc;
            J1Key = HighKey;

            STARTTm;

#ifdef SKIPMACRO
            Rc = Judy1Last(J1, &J1Key, PJE0);
#else
            J1L(Rc, J1, J1Key);
#endif // SKIPMACRO

            for (elm = 0; elm < Elements; elm++)
            {
                if (Rc != 1)
                    FAILURE("Judy1Prev Rc != 1, is: ", Rc);

#ifdef SKIPMACRO
                Rc = Judy1Prev(J1, &J1Key, PJE0);
#else
                J1P(Rc, J1, J1Key);   // Get previous one
#endif // SKIPMACRO

            }
            ENDTm(DeltanSec1);

            if ((TValues == 0) && (Rc != 0))
            {
                FAILURE("Judy1Prev Rc != 0", Rc);
            }

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t   *PValue;
            JLKey = HighKey;

            STARTTm;

#ifdef SKIPMACRO
            PValue = (PWord_t)JudyLLast(JL, &JLKey, PJE0);
#else
            JLL(PValue, JL, JLKey);
#endif // SKIPMACRO

            for (elm = 0; elm < Elements; elm++)
            {
                if (PValue == NULL)
                {
                    printf("\nElements = %lu, elm = %lu\n", Elements, elm);
                    FAILURE("JudyLPrev ret NULL PValue at", elm);
                }
                if (VFlag && (*PValue != JLKey))
                    FAILURE("JudyLPrev ret bad *PValue at", elm);

#ifdef SKIPMACRO
                PValue = (PWord_t)JudyLPrev(JL, &JLKey, PJE0);
#else
                JLP(PValue, JL, JLKey);       // Get previous one
#endif // SKIPMACRO

            }
            ENDTm(DeltanSecL);

            if ((TValues == 0) && (PValue != NULL))
                FAILURE("JudyLPrev PValue != NULL", PValue);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

//  perhaps a check should be done here -- if I knew what to expect.
    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNextEmpty"

// Returns number of consecutive Keys
int
TestJudyNextEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Seed_t    WorkingSeed;
    int       Rc;                       // Return code

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t    J1Key;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                J1Key = GetNextKey(&WorkingSeed);
                if (Tit)
                {
#ifdef SKIPMACRO
                    Rc = Judy1NextEmpty(J1, &J1Key, PJE0);
#else
                    J1NE(Rc, J1, J1Key);
#endif // SKIPMACRO

                    if (Rc != 1)
                        FAILURE("Judy1NextEmpty Rcode != 1 =", Rc);
                }
            }
            ENDTm(DeltanSec1);

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t    JLKey;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                JLKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {
#ifdef SKIPMACRO
                    Rc = JudyLNextEmpty(JL, &JLKey, PJE0);
#else
                    JLNE(Rc, JL, JLKey);
#endif // SKIPMACRO

                    if (Rc != 1)
                        FAILURE("JudyLNextEmpty Rcode != 1 =", Rc);
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }
    return (0);
}

// Routine to time and test JudyPrevEmpty routines

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyPrevEmpty"

int
TestJudyPrevEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Seed_t    WorkingSeed;
    int       Rc;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t    J1Key;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                J1Key = GetNextKey(&WorkingSeed);
                if (Tit)
                {
#ifdef SKIPMACRO
                    Rc = Judy1PrevEmpty(J1, &J1Key, PJE0);
#else
                    J1PE(Rc, J1, J1Key);
#endif // SKIPMACRO

                    if (Rc != 1)
                        FAILURE("Judy1PrevEmpty Rc != 1 =", Rc);
                }

            }
            ENDTm(DeltanSec1);

            if (DminTime > DeltanSec1)
            {
                icnt = ICNT;
                if (DeltanSec1 > 0.0)   // Ignore 0
                    DminTime = DeltanSec1;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSec1 = DminTime / (double)Elements;
    }

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t    JLKey;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                JLKey = GetNextKey(&WorkingSeed);
                if (Tit)
                {
#ifdef SKIPMACRO
                    Rc = JudyLPrevEmpty(JL, &JLKey, PJE0);
#else
                    JLPE(Rc, JL, JLKey);
#endif // SKIPMACRO

                    if (Rc != 1)
                        FAILURE("JudyLPrevEmpty Rcode != 1 =", Rc);
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                    DminTime = DeltanSecL;
            }
            else
            {
                if (--icnt == 0)
                    break;
            }
        }
        DeltanSecL = DminTime / (double)Elements;
    }

    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDel"

int
TestJudyDel(void **J1, void **JL, void **JH, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Seed_t    WorkingSeed;
    int       Rc;

// set PSeed to beginning of Inserts
//
    if (J1Flag)
    {
        WorkingSeed = *PSeed;

        MalFlag = JudyMal1;
        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);

            if (Tit)
            {

#ifdef DIAG
   Rc = Judy1Test(*J1, TstKey, PJE0);

   if (Rc == 0) printf("nothingto delete 0x%lx\n", TstKey);
#endif  // DIAG

#ifdef SKIPMACRO
                Rc = Judy1Unset(J1, TstKey, PJE0);
#else
                J1U(Rc, *J1, TstKey);
#endif // SKIPMACRO

                if (Rc != 1)
                    FAILURE("Judy1Unset ret Rcode != 1", Rc);

                if (gFlag)
                {
                    Rc = Judy1Test(*J1, TstKey, PJE0);

                    if (Rc)
                    {
                        FAILURE("Judy1Unset failed", TstKey);
                    }
                }
            }
        }
        ENDTm(DeltanSec1);
        DeltanSec1 /= Elements;
    }

    if (JLFlag)
    {
        WorkingSeed = *PSeed;

// reset PSeed to beginning of Inserts
//
        MalFlag = JudyMalL;
        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);

            if (Tit)
            {

#ifdef SKIPMACRO
                Rc = JudyLDel(JL, TstKey, PJE0);
#else
                JLD(Rc, *JL, TstKey);
#endif // SKIPMACRO

                if (Rc != 1)
                    FAILURE("JudyLDel ret Rcode != 1", Rc);

                if (gFlag)
                {
                    PWord_t   PValueNew;

                    PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                    if (PValueNew != NULL)
                    {
                        FAILURE("JudyLDel failed Delete", TstKey);
                    }
                }
            }
        }
        ENDTm(DeltanSecL);
        DeltanSecL /= Elements;
    }

    if (JHFlag)
    {
        WorkingSeed = *PSeed;

// reset PSeed to beginning of Inserts
//
        MalFlag = JudyMalHS;
        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);

            if (Tit)
            {
                JHSD(Rc, *JH, &TstKey, sizeof(Word_t));
                if (Rc != 1)
                    FAILURE("JudyHSDel ret Rcode != 1", Rc);
            }
        }
        ENDTm(DeltanSecHS);
        DeltanSecHS /= Elements;
    }
    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestBitmapSet"

int
BitmapGet(PWord_t B1, Word_t TstKey)
{
    int       offset;
    int       bitnum;

    offset = TstKey / (sizeof(Word_t) * 8);
    bitnum = TstKey % (sizeof(Word_t) * 8);

    if ((B1[offset] & (((Word_t)1) << bitnum)) == 0)
        return (0);
    else
        return (1);
}

int
BitmapSet(PWord_t B1, Word_t TstKey)
{
    int       offset;
    int       bitnum;
    Word_t    bitmap;

    offset = TstKey / (sizeof(Word_t) * 8);
    bitnum = TstKey % (sizeof(Word_t) * 8);

    bitmap = ((Word_t)1) << bitnum;
    if (B1[offset] & bitmap)
        return (0);             // Duplicate

    B1[offset] |= bitmap;       // set the bit
    return (1);
}

int
TestBitmapSet(PWord_t B1, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;

    STARTTm;
    for (elm = 0; elm < Elements; elm++)
    {
        TstKey = GetNextKey(PSeed);

        if (Tit)
        {
            if (BitmapSet(B1, TstKey) == 0)
            {
                if (GValue)
                {
                    BitmapDups++;
                }
                else
                {
                    printf("\nBitMapSet -- Set bit, Key = 0x%lx", TstKey);
                    FAILURE("BitMapSet Word = ", elm);
                }
            }
        }
    }
    ENDTm(DeltanSecBt);

    DeltanSecBt /= (double)Elements;

    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestBitmapTest"

int
TestBitmapTest(PWord_t B1, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Seed_t    WorkingSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;

    if (lFlag)
        Loops = 1;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);

            if (Tit)
            {
                if (BitmapGet(B1, TstKey) == 0)
                {
                    printf("\nBitMapGet -- missing bit, Key = 0x%lx",
                           TstKey);
                    FAILURE("BitMapGet Word = ", elm);
                }
            }
        }
        ENDTm(DeltanSecBt);

        if (DminTime > DeltanSecBt)
        {
            icnt = ICNT;
            if (DeltanSecBt > 0.0)        // Ignore 0
                DminTime = DeltanSecBt;
        }
        else
        {
            if (--icnt == 0)
                break;
        }
    }
    DeltanSecBt = DminTime / (double)Elements;

    return 0;
}

//
// Routine to get next size of Keys
int                                     // return 1 if last number
NextSum(Word_t *PIsum,                  // pointer to returned next number
        double *PDsum,                  // Temp double of above
        double DMult)                   // Multiplier
{
//  Save prev number
    double    PrevPDsum = *PDsum;
    double    DDiff;

//  Calc next number >= 1.0 beyond previous
    do
    {
        *PDsum *= DMult;                // advance floating sum
        DDiff = *PDsum - PrevPDsum;     // Calc diff next - prev
    } while (DDiff < 0.5);              // and make sure at least + 1

//  Return it in integer format
    *PIsum += (Word_t)(DDiff + 0.5);
    return (0);                         // more available
}
