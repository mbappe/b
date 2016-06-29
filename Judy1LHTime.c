// @(#) $Revision: 1.5 $ $Source: /home/doug/judy-1.0.5_PSplit_goto_newLeaf3_U2_1K_1_L765_4th/test/RCS/Judy1LHTime.c,v $
// =======================================================================
//                      -by- 
//   Author Douglas L. Baskins, Aug 2003.
//   Permission to use this code is freely granted, provided that this
//   statement is retained.
//   email - dougbaskins .at. yahoo.com -or- dougbaskins .at. gmail.com
// =======================================================================



#include <unistd.h>                     // getopt()
#include <getopt.h>                     // getopt_long()
#include <stdlib.h>                     // exit(), malloc(), random()
#include <stdio.h>                      // printf(), setbuf()
#include <math.h>                       // pow()
#include <time.h>                       // clock_gettime()
#include <sys/utsname.h>                // uname()
#include <errno.h>                      // errnoerrno
#include <string.h>                     // strtok_r()
#include <strings.h>                    // bzero()

// Turn off assert(0) by default
#ifndef DEBUG
#define NDEBUG 1
#endif  // DEBUG

#include <assert.h>                     // assert()

#include <Judy.h>                       // for Judy macros J*()

#include "RandomNumb.h"                 // Random Number Generators

#define WARMUPCPU  10000000             // calls to random() to warmup CPU

#if defined(__LP64__)
#define        cLg2WS          3        // log2(sizeof(Word_t) - 64 bit
#else // defined(__LP64__)
#define        cLg2WS          2        // log2(sizeof(Word_t) - 32 bit
#endif // defined(__LP64__)

// =======================================================================
//   This program measures the performance of a Judy1, JudyL and 
//   limited to one size of string (sizeof Word_t) JudyHS Arrays.
//
// Compile examples: 
//
//   cc -O -g Judy1LHTime.c -lm -lrt -lJudy -o Judy1LHTime 
//           -or-
//   cc -m32 -O3 -Wall -I../src Judy1LHTime.c -lm -lrt ../src/libJudy32.a -o Judy1LHTime32

// =======================================================================
//   A little help with printf("0x%016x...  vs printf(0x%08x...
// =======================================================================

#if defined(_WIN64)

#define Owx   "%016llx"
#define OWx "0x%016llx"
#define wx "%llx"

#else // defined(_WIN64)

#if defined(__LP64__)

#define Owx   "%016lx"
#define OWx "0x%016lx"

#else // defined(__LP64__)

#define Owx   "%08lx"
#define OWx "0x%08lx"

#endif // defined(__LP64__)

#define wx "%lx"

#endif // defined(_WIN64)

//=======================================================================
//             R A M   M E T R I C S  
//=======================================================================
//  For figuring out how much different structures contribute.   Must be 
//  turned on in source compile of Judy with -DRAMMETRICS

Word_t    j__MissCompares;            // number times LGet/1Test called
Word_t    j__SearchPopulation;          // Population of Searched object
Word_t    j__DirectHits;                // Number of direct hits -- no search
Word_t    j__SearchGets;                // Number of object calls
//Word_t    j__TreeDepth;                 // number time Branch_U called

Word_t    j__AllocWordsTOT;
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


Word_t    j__AllocWordsJLLW;

Word_t    j__AllocWordsJV;

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
#define STARTTm                                                         \
{                                                                       \
    clock_gettime(CLOCK_MONOTONIC_RAW, &TVBeg__);                       \
/*  asm volatile("" ::: "memory");   */                                 \
}

#define ENDTm(D) 							\
{ 									\
/*    asm volatile("" ::: "memory");        */                          \
    clock_gettime(CLOCK_MONOTONIC_RAW, &TVEnd__);   	                \
                                                                        \
    (D) = (double)(TVEnd__.tv_sec - TVBeg__.tv_sec) * 1E9 +             \
         ((double)(TVEnd__.tv_nsec - TVBeg__.tv_nsec));                 \
}
#endif // POSIX Linux and Unix

Word_t    xFlag = 0;    // Turn ON 'waiting for Context Switch'

// Wait for an extraordinary long Delta time (context switch?)
static void
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

Word_t    j__MFlag;                    // Print memory allocation on stderr
Word_t    j__TotalBytesAllocated;

static Word_t MalFreeCnt = 0;

#ifdef SELFALIGNED
// Did not think very hard on this one.  I am sure their is a much
// faster way to do it. (dlb)

static int
j__log2(Word_t num)
{
    int       __lb = 0;

#ifdef __LP64__
    if (num >= ((Word_t)1 << 32))
    {
        num >>= 32;
        __lb += 32;
    }
#endif  // __LP64__

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

static Word_t
j__roundup2(Word_t Size)
{
    Word_t ret;

    ret = j__log2(Size);
    if ((Size & -Size) != Size) ret++;

    return((Word_t)1 << ret);
}

#endif // SELFALIGNED

#ifdef internal_JudyMalloc
// This routine replaces the one in libJudy.a so we can get a
// measure of the memory used and counts of malloc().
// It assumes that a dlmalloc() style of memory allocation is used.

Word_t
JudyMalloc(Word_t Words)
{
    size_t    Bytes;
    Word_t    Addr;
    Bytes = Words * sizeof(Word_t);


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
        MalFreeCnt++;                   // keep track of total malloc() + free()
    return (Addr);
}                                       // JudyMalloc()

// ****************************************************************************
// J U D Y   F R E E
//
// Note: Judy knows amount of memory being released, though not needed by free()

void
JudyFree(void *PWord, Word_t Words)
{

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
#endif  // internal JudyMalloc


// =======================================================================
// Common macro to handle a failure
// =======================================================================
#define FAILURE(STR, UL)                                                \
{                                                                       \
printf(        "\n--- Error: %s %lu, file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
fprintf(stderr,"\n--- Error: %s %lu, file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
        exit(1);                                                        \
}

// Interations without improvement
//
// Minimum of 2 loops, maximum of 1000000
#define MINLOOPS 2
#define MAXLOOPS 1000

// Maximum of 10 loops with no improvement
#define ICNT 10

// Structure to keep track of times
typedef struct MEASUREMENTS_STRUCT
{
    Word_t    ms_delta;                 // leave room for more stuff
}
ms_t     , *Pms_t;

//=======================================================================
//             P R O T O T Y P E S
//=======================================================================


// Specify prototypes for each test routine
//int       NextSum(Word_t *PNumber, double *PDNumb, double DMult);

int       TestJudyIns(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

void      TestJudyLIns(void **JL, PSeed_t PSeed, Word_t Elems);

int       TestJudyDup(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudyDel(void **J1, void **JL, void **JH, PSeed_t PSeed,
                      Word_t Elems);

int       TestJudyGet(void *J1, void *JL, void *JH, PSeed_t PSeed,
                      Word_t Elems);

void      TestJudyLGet(void *JL, PSeed_t PSeed, Word_t Elems);

int       TestJudy1Copy(void *J1, Word_t Elem);

int       TestJudyCount(void *J1, void *JL, Word_t Elems);

Word_t    TestJudyNext(void *J1, void *JL, Word_t Elems);

int       TestJudyPrev(void *J1, void *JL, Word_t HighKey, Word_t Elems);

int       TestJudyNextEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elems);

int       TestJudyPrevEmpty(void *J1, void *JL, PSeed_t PSeed, Word_t Elems);

int       TestBitmapSet(PWord_t *pB1, PSeed_t PSeed, Word_t Meas);

int       TestBitmapTest(PWord_t B1, PSeed_t PSeed, Word_t Meas);

int       TestByteSet(PSeed_t PSeed, Word_t Meas);

int       TestByteTest(PSeed_t PSeed, Word_t Meas);

int       TimeNumberGen(void **TestRan, PSeed_t PSeed, Word_t Delta);


// Routine to get next size of Keys
static int                              // return 1 if last number
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

static void
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

static void
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

static void
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
double    DeltanSecBy = 0.0;            // Global for measuring delta times
double    DeltaMalFre1 = 0.0;           // Delta mallocs/frees per inserted Key
double    DeltaMalFreL = 0.0;           // Delta mallocs/frees per inserted Key
double    DeltaMalFreHS = 0.0;          // Delta mallocs/frees per inserted Key

Word_t    Judy1Dups =  0;
Word_t    JudyLDups =  0;
Word_t    JudyHSDups = 0;
Word_t    BitmapDups = 0;
Word_t    ByteDups   = 0;

Word_t    J1Flag = 0;                   // time Judy1
Word_t    JLFlag = 0;                   // time JudyL
Word_t    JRFlag = 0;                   // time JudyL with no cheat
Word_t    JHFlag = 0;                   // time JudyHS
Word_t    dFlag = 0;                    // time Judy1Unset JudyLDel
Word_t    vFlag = 0;                    // time Searching 
Word_t    CFlag = 0;                    // time Counting
Word_t    cFlag = 0;                    // time Copy of Judy1 array
Word_t    IFlag = 0;                    // time duplicate inserts/sets
Word_t    bFlag = 0;                    // Time REAL bitmap of (2^-B #) in size
PWord_t   B1 = NULL;                    // BitMap
#define cMaxColon ((int)sizeof(Word_t) * 2)  // Maximum -b suboption paramters
int       Parm[cMaxColon + 1] = { 0 };  // suboption parameters to -b#:#:# ...
Word_t    yFlag = 0;                    // Time REAL Bytemap of (2^-B #) in size
uint8_t  *By;                           // ByteMap

Word_t    mFlag = 0;                    // words/Key for all structures
Word_t    pFlag = 0;                    // Print number set
Word_t    lFlag = 0;                    // do not do multi-insert tests

Word_t    gFlag = 0;                    // do Get after Ins (that succeds)
Word_t    iFlag = 0;                    // do another Ins (that fails) after Ins
Word_t    tFlag = 0;                    // for general new testing
Word_t    Tit = 1;                      // to measure with calling Judy
Word_t    VFlag = 1;                    // To verify Value Area contains good Data
Word_t    fFlag = 0;
Word_t    KFlag = 0;                    // do a __sync_synchronize() in GetNextKey()
Word_t    Warmup = 2000000000;          // nSec to warm up CPU
Word_t    PreStack = 0;                 // to test for TLB collisions with stack

Word_t    Offset = 0;                   // Added to Key

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
//Word_t    Key = 0xc1fc;

// returns next Key, depending on SValue, DFlag and GValue.
//
static inline Word_t
GetNextKey(PSeed_t PSeed)
{
    Word_t    Key = 0xc1fc;

    if (KFlag)
            __sync_synchronize();  /* really slow */ 

    if (FValue)
        Key = FileKeys[PSeed->Order++];
    else
        Key = RandomNumb(PSeed, SValue);
   
    if (DFlag)
    {
        Word_t SwizzledKey;

//      move the mirror bits into the least bits determined -B#
        SwizzledKey = Swizzle(Key) >> ((sizeof(Word_t) * 8) - BValue);

        return (SwizzledKey + Offset);
    }
    else
    {
        if ((sizeof(Word_t) * 8) != BValue)
            Key %= (Word_t)1 << BValue;
        
        return (Key + Offset);         // add in Offset;
    }
}

static void
PrintHeader(void)
{
    printf("# Population  DeltaIns GetMeasmts");

    if (tFlag)
        printf(" MeasOv");
    if (J1Flag)
        printf("    J1S");
    if (JLFlag)
        printf("    JLI");
    if (JRFlag)
        printf("  JLI-R");
    if (JHFlag)
        printf("   JHSI");
    if (bFlag)
        printf("  BMSet");
    if (yFlag)
        printf("  ByMSet");
    if (tFlag)
        printf(" MeasOv");
    if (J1Flag)
        printf("    J1T");
    if (JLFlag)
        printf("    JLG");
    if (JRFlag)
        printf("  JLG-R");
    if (JHFlag)
        printf("   JHSG");
    if (yFlag)
        printf(" ByTest");
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

    if (bFlag)
        printf("  Heap: Words/Key");
    if (yFlag)
        printf("  Heap: Words/Key");

    if ((J1Flag + JLFlag + JHFlag) == 1)    // only if 1 Heap
    {
        if (J1Flag)
            printf(" 1heap/K");
        if (JLFlag || JRFlag)
            printf(" Lheap/K");
        if (JHFlag)
            printf(" HSheap/K");
    }

    if (mFlag && (bFlag == 0) && (yFlag == 0))
    {

        printf(" JBB/K");
        printf(" JBU/K");
        printf(" JBL/K");


        printf(" LWd/K");

        printf("  L7/K");
        printf("  L6/K");
        printf("  L5/K");
        printf("  L4/K");
        printf("  L3/K");
        printf("  L2/K");
        printf("  L1/K");
        printf("  B1/K");
        printf("  JV/K");

        printf(" MisCmp");
        printf(" %%DiHt");

//        printf(" TrDep");
        printf(" AvPop");
        printf(" %%MalEff");

        if (J1Flag)
            printf(" MF1/K");
        if (JLFlag || JRFlag)
            printf(" MFL/K");
        if (JHFlag)
            printf(" MFHS/K");
    }
    printf("\n");
}

static void 
Usage(int argc, char **argv)
{
    (void) argc;

    printf
        ("\n<<< Program to do performance measurements on Judy Arrays >>>\n\n");
    printf("%s -n# -P# -S# -B# -T# -G# -s# -X# -1LHbIcCvdtDlMK\n\n", argv[0]);
    printf("Where: (default value is shown as [#])\n");
    printf("-n <#>  Max number of Keys used in JudyLGet/1Test [10000000]\n");
    printf("-P <#>  Measurement points per decade (1..1000) [50]\n");
    printf("-S <#>  Number Generator skip amount, 0 == Random [0]\n");
    printf
        ("-B <#>  Significant bits output (15..%d) in random Number Generator [32]\n",
         (int)sizeof(Word_t) * 8);
    printf ("-G <#>  Type (0..4) of random numbers 0==flat spectrum, 1..4==Gaussian [0]\n");
    printf ("-X <#>  Scale the numbers produced by '-m' flag (for plotting) [%d]\n", XScale);

    printf ("-o <#>   Key += (#) for Diag only\n");
    printf ("--Offset=<#>   Key += (#) for Diag only\n");

    printf ("-O <#>   Key += (# << (-B #)) for Diag only\n");
    printf ("--BigOffset=<#>   Key += (# << (-B #)) for Diag only\n");

    printf
        ("-F <filename>  Ascii file of Keys, zeros ignored -- must be last option!!!\n");
    printf
        ("-b <#> :#:# ... 1st number required [1] where each number is the next level of tree\n");
    printf("-1      Time Judy1\n");
    printf("-L      Time JudyL\n");
    printf("-R      Time JudyL using *PValue as next TstKey\n");
    printf("-H      Time JudyHS\n");
    printf("-y      Time a REAL BYTEMAP (-B # sets size == 2^<#>) [-B32]\n");
    printf("-I      Time DUPLICATE (already in array) JudyIns/Set times\n");
    printf("-c      Time copying a Judy1 Array\n");
    printf("-C      Include timing of JudyCount tests\n");
    printf("-v      Include timing of Judy First/Last/Next/Prev tests\n");
    printf("-d      Include timing of Del/Unset\n");
    printf("-p      Print number set used for testing (Diag only) - takes presedence\n");
    printf
        ("-V      Turn OFF JudyLGet() verification tests (saving a cache-hit on 'Value')\n");
    printf
        ("-x      Turn ON 'waiting for context switch' flag -- smoother plots??\n");
    printf
        ("-t      Print measured 'overhead' (nS) that was subtracted out (Number Generator, etc)\n");
    printf
        ("-D      'Mirror' Numbers from Generator (as in binary viewed with a mirror)\n");
    printf
        ("-l      Do not smooth data with iteration at low (<100) populations (I.E do not JudyLDel/Judy1Unset)\n");
    printf
        ("-T <#>  Number of Keys to average JudyGet/Test times, 0 == MAX [1000000]\n");
    printf("-s <#>  Starting number in Number Generator [0x%lx]\n", StartSequent);
    printf("-g      Do a Get/Test right after every Ins/Set/Del/Unset (Diag only - adds to times)\n");
    printf("-i      Do a Ins/Set right after every Ins/Set (Diag only - adds to times)\n");
    printf("-M      Print on stderr Judy_mmap() and Judy_unmap() calls (Diag only)\n");
    printf("-K      do a __sync_synchronize() in GetNextKey() (is mfence instruction in X86)\n"); 

    printf("\n");

    exit(1);
}
#undef __FUNCTI0N__
#define __FUNCTI0N__ "main"

// Defining SWAP causes the program to use J1S/J1T with the -b option
// instead of BitmapSet/BitmapGet.
// It also causes the program to use BitmapSet/BitmapGet with the -1 option
// instead of using J1S/J1T.
// Hence it is called SWAP.  We are swapping the places where J1S/JIT
// and BitmapSet/BitmapGet calls are made.
// The purpose was to be able to measure the differences induced by the
// infrastructure around the calls.

static int
BitmapGet(PWord_t B1, Word_t TstKey)
{
    Word_t        offset;
#if defined(SISTER_READ)
    unsigned      n;
    unsigned      nSisters = 0;
#endif // defined(SISTER_READ)
#if defined(BITMAP_BY_BYTE)
    unsigned char *p = (unsigned char *)B1;
#else // defined(BITMAP_BY_BYTE)
    Word_t        *p = B1;
#endif // defined(BITMAP_BY_BYTE)
    unsigned      bitnum;
    int      bSet;

// Original -b did not have this check for NULL.
#if 0
    if (B1 == NULL)
    {
        return (0);
    }
#endif

#if defined(LOOKUP_NO_BITMAP_DEREF)

    return 1;

#else // defined(LOOKUP_NO_BITMAP_DEREF)

    offset = TstKey / (sizeof(*p) * 8);
    bitnum = TstKey % (sizeof(*p) * 8);

    bSet = ((p[offset] & ((Word_t)1 << bitnum)) != 0);
#if defined(SISTER_READ)
    // What about even sister and odd sister and same page sister?
    for (unsigned n = 1; n <= nSisters; n++)
    {
        offset += 64 / sizeof(*p);
        bSet += p[offset];
    }
#endif // defined(SISTER_READ)

    return bSet;

#endif // defined(LOOKUP_NO_BITMAP_DEREF)

}

// Note that the prototype for BitmapSet is different from that of Judy1Set.
static int
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

//
// oa2w is an abbreviation for OptArgToWord.
// It is a wrapper for strtoul for use when converting an optarg to an
// unsigned long to help save the user from a painful mistake.
// The mistake is the user typing "-S -y" on the command line where
// -S requires an argument and getopt uses -y as that argument and strtoul
// silently treats the -y as a 0 and the -y is never processed as an option
// letter by getopt.  Make sense?
//
static Word_t
oa2w(char *str, char **endptr, int base, int ch)
{
    char *lendptr;
    Word_t ul;

    if ((str == NULL) || *str == '\0') {
        printf("\nError --- Illegal optarg, \"\", for option \"-%c\".\n", ch);
        exit(1);
    }

    errno = 0;

    ul = strtoul(str, &lendptr, base);

    if (errno != 0) {
        printf("\nError --- Illegal optarg, \"%s\", for option \"-%c\": %s.\n",
               str, ch, strerror(errno));
        exit(1);
    }

    if (*lendptr != '\0') {
        printf(
          "\nError --- Illegal optarg, \"%s\", for option \"-%c\" is not a number.\n",
            str, ch);
        exit(1);
    }

    if (endptr != NULL) {
        *endptr = lendptr;
    }

    return (ul);
}

static struct option longopts[] = {

 // { name,               has_arg,          flag,      val },

    // Long option "--LittleOffset=<#>" is equivalent to short option "-o<#>".
    { "Offset",    required_argument, NULL,      'o' },

    // Long option "--BigOffset=<#>" is equivalent to short option '-o<#>".
    { "BigOffset",       required_argument, NULL,      'O' },

    // Last struct option in array must be filled with zeros.
    { NULL,              0,                 NULL,      0 }
};

int
main(int argc, char *argv[])
{
//  Names of Judy Arrays
    void     *J1 = NULL;                // Judy1
    void     *JL = NULL;                // JudyL
    void     *JH = NULL;                // JudyHS


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
//    double    TreeDepth = 0;
    Word_t    LittleOffset = 0;
    Word_t    BigOffset = 0;

//    double    SearchPopulation = 0;     // Population of Searched object
    double    DirectHits = 0;           // Number of direct hits
    double    SearchGets = 0;           // Number of object calls

//    long      MaxNumb;
    Word_t    MaxNumb;
    int       Col;
    int       c;
    Word_t    ii;                       // temp iterator
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
    while (1)
    {
        c = getopt_long(argc, argv,
                   "a:n:S:T:P:s:B:G:X:W:o:O:F:b:dDcC1LHvIltmpxVfgiyRMK", longopts, NULL);
        if (c == -1)
            break;

        switch (c)
        {
        case 'a':                      // Max population of arrays
            PreStack = oa2w(optarg, NULL, 0, c);   // Size of PreStack
            break;
        case 'n':                      // Max population of arrays
            nElms = oa2w(optarg, NULL, 0, c);   // Size of Linear Array
            if (nElms == 0)
                FAILURE("Error --- No tests: -n", nElms);
            break;

        case 'S':                      // Step Size, 0 == Random
        {
            SValue = oa2w(optarg, NULL, 0, c);
            break;
        }
        case 'T':                      // Maximum retrieve tests for timing 
            TValues = oa2w(optarg, NULL, 0, c);
            break;

        case 'P':                      // measurement points per decade
            PtsPdec = oa2w(optarg, NULL, 0, c);
            break;

        case 's':
            StartSequent = oa2w(optarg, NULL, 0, c);
            break;

        case 'B':                      // expanse of data points (random only)
            BValue = oa2w(optarg, NULL, 0, c);
            if ((BValue > sizeof(Word_t) * 8) || (BValue < 15))
            {
                ErrorFlag++;
                printf("\nError --- Illegal number of random bits of %lu !!!\n", BValue);
            }
            break;

        case 'G':                      // Gaussian Random numbers
            GValue = oa2w(optarg, NULL, 0, c);  // 0..4 -- check by RandomInit()
            break;

        case 'X':                      // Scale numbers under the '-m' flag
            XScale = strtol(optarg, NULL, 0);   // 1..1000
            if (XScale < 1)
            {
                ErrorFlag++;
                printf("\nError --- option -X%d must be greater than 0 !!!\n", XScale);
            }

        case 'W':                      // Warm up CPU number of random() calls
            Warmup = oa2w(optarg, NULL, 0, c);
            printf("\n\n-------------Warmup %lu\n", Warmup);
            break;

        case 'o': // Add <#> to generated keys, aka --LittleOffset=<#>.
            LittleOffset = oa2w(optarg, NULL, 0, c);
            break;

        case 'O': // Add <#> << B# to generated keys, aka --BigOffset=<#>.
            BigOffset = oa2w(optarg, NULL, 0, c);
            break;

        case 'b':                      // Turn on REAL bitmap testing
        {
            char *str, *tok;
            char *saveptr;
            int ii      = 0;
            int jj      = 0;
            int sum     = 0;

// Leave the minimum bitmap size of 256 Keys (2 ^ 8)
#define cMaxSum ((int)(sizeof(Word_t) * 8) - 8)      // maximum bits decoded in Branches

//            printf("\n\n\n--------------## .-b. '%s' -----\n", optarg);
            if (optarg[0] == '-')
            {
                optind--;
            }
            else
            {

//            printf("optarg[0] = %c\n", optarg[0]);

//          parse the sub parameters of the -b option
            for (str = optarg; (tok = strtok_r(str, ":", &saveptr)); str = NULL)
            {
                Parm[ii] = atoi(tok);
                sum += Parm[ii];

//              Limit maximum of decode in branches
                if (sum > cMaxSum)
                {
                    for (jj = 0; jj < ii; jj++)
                        printf("\nError: option -b%d:", Parm[jj]);

                    printf("\nError: Maximum sum of subparamters is %d\n", cMaxSum);
                    break;
                }
                if (Parm[ii] > 16)
                {
                    printf("\nError: Maximum Value of :%d is 16\n", Parm[jj]);
                    ErrorFlag++;
                    break;
                }
                ii++;
                if (ii == cMaxColon)
                {
                    for (jj = 0; jj < ii; jj++)
                        printf("\nError: option -b%d:", Parm[jj]);

                    printf(" - has a maximum of %d subprameters\n", cMaxColon);
                    ErrorFlag++;
                    break;
                }
            }
            if (sum == 0)
            {
                printf("Error: -b option needs at least one number -b 0..%d\n", (int)BValue - cLg2WS);
                ErrorFlag++;
            }

            }
            bFlag = 1;
            break;
        }
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
                printf("\nError --- Illegal argument to \"-F %s\" -- errno = %d\n", optarg, errno);
                printf("Error --- Cannot open file \"%s\" to read it\n", optarg);
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
                printf("\nError: Illegal argument to \"-F %s\" -- errno = %d\n", keyfile, errno);
                printf("Error: Cannot open file \"%s\" to read it\n", keyfile);
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

        case 'y':                      // Turn on REAL Bytemap testing
            yFlag = 1;
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

        case 'g':                      // do a Get after an Ins/Del
            gFlag = 1;
            break;

        case 'i':                      // do a Ins after an Ins
            iFlag = 1;
            break;

        case 'R':                      // Use *PValue as next Key
            JRFlag = 1;
            break;

        case 'M':                      // print Judy_mmap, Judy_unmap etc.
            j__MFlag = 1;
            break;

        case 'K':                      // do a __sync_synchronize() in GetNextKey()
            KFlag = 1;
            break;

        default:
            ErrorFlag++;
            break;
        }
        if (FValue)
            break;
    }
    if (JLFlag && JRFlag)
    {
        printf (" ========================================================\n");
        printf(" Sorry '-L' and '-R' options are mutually exclusive\n");
        printf (" ========================================================\n");
        exit(1);
    }
    if (JRFlag && !VFlag)
    {
        printf("\n# Warning -- '-V' ignored, because '-R' is set\n");
        fprintf(stderr, "\n# Warning -- '-V' ignored, because '-R' is set\n");
    }
//  build the Random Number Generator starting seeds
    PStartSeed = RandomInit(BValue, GValue);

//  BValue already check to be <= 64 or <=32 and >=15
    if (BigOffset)
    {
        Word_t bigoffset = BigOffset;

        if (BValue >= (sizeof(Word_t) * 8))
            BigOffset = 0;

        BigOffset <<= BValue;           // offset past BValue 

        if (BigOffset == 0)
        {
            printf("\n# Warning -- '-O 0x%lx' ignored, because '-B %lu' option too big\n", bigoffset, BValue);
            fprintf(stderr, "\n# Warning -- '-O 0x%lx' ignored, because '-B %lu' option too big\n", bigoffset, BValue);
        }
    }
    Offset = BigOffset + LittleOffset;  // why not?

    if (PStartSeed == (PSeed_t) NULL)
    {
        printf("\nIllegal Number in -G%lu !!!\n", GValue);
        ErrorFlag++;
    }

//  Set MSB number of Random bits in LFSR
    RandomBit = (Word_t)1 << (BValue - 1);
    MaxNumb = (RandomBit * 2) - 1;

//  Check if starting number is too big
    if (StartSequent > MaxNumb)
    {
        printf("\nArgument in '-s %lu' option is greater than %lu\n", StartSequent, MaxNumb);
        ErrorFlag++;
    }
    if (StartSequent == 0 && (SValue == 0))
    {
        printf("\nError --- '-s 0' option Illegal if Random\n");
        ErrorFlag++;
    }

//  Set the starting number in number Generator
    PStartSeed->Seeds[0] = StartSequent;

    if (ErrorFlag)
        Usage(argc, argv);

    PStartSeed->Seeds[0] = StartSequent;
    {
        MaxNumb = (RandomBit * 2) - 1;
        if (nElms > MaxNumb)
        {
            printf("# Trim Max number of Elements -n%lu due to max -B%lu bit Keys",
                   MaxNumb, BValue);
            fprintf(stderr, "# Trim Max number of Elements -n%lu due to max -B%lu bit Keys",
                   MaxNumb, BValue);

            if (Offset)
            {
                printf(", add %ld (0x%lx) to Key values", Offset, Offset);
                fprintf(stderr,", add %ld (0x%lx) to Key values", Offset, Offset);
            }
            printf("\n");
            fprintf(stderr, "\n");

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

//            printf(""OWx"\n", PrintKey);

            printf("0x"Owx", %2d\n", PrintKey, (int)log2((double)(PrintKey)) + 1);

#ifdef __LP64__
//            printf("0x%016lx\n", PrintKey);
// 1          printf("0x%016lx %lu %lu, %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#else   // ! __LP64__
 //           printf("0x%08lx\n", PrintKey);
// 1          printf("0x%08lx %lu %lu, %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#endif  // ! __LP64__

        }
        exit(0);
    }


//  print Title for plotting -- command + run arguements
    printf("# TITLE %s -", argv[0]);
    if (bFlag && (Parm[0] == 0))
        printf("b");
    if (J1Flag)
        printf("1");
    if (JLFlag)
        printf("L");
    if (JRFlag)
        printf("R");
    if (JHFlag)
        printf("H");
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
    if (j__MFlag)
        printf("M");
    if (KFlag)
        printf("K");

//  print more options - default, adjusted or otherwise
    printf(" -n%lu -T%lu -P%lu -X%d", nElms, TValues, PtsPdec, XScale);
    if (bFlag && (Parm[0] != 0))
    {
        int ii;
        printf(" -b");
        for (ii = 0; Parm[ii]; ii++)
        {
            if (ii) printf(":");
            printf("%d", Parm[ii]);
        }
    }
    if (SValue)
        printf(" -S%lu -s%lu", SValue, StartSequent);
    else                                // some flavor of random Keys
        printf(" -B%lu -G%lu", BValue, GValue);

    if (FValue)
        printf(" -F %s", keyfile);

    if (Offset) 
        printf(" -o 0x%lx", Offset);

    printf("\n");

    if (mFlag)
    {
        int       count = 0;
        if (JLFlag)
            count++;
        if (JRFlag)
            count++;
        if (J1Flag)
            count++;
        if (JHFlag)
            count++;
        if (bFlag)
            count++;
        if (yFlag)
            count++;

        if (count != 1)
        {
            printf
                (" ========================================================\n");
            printf
                (" Sorry, '-m' measurements compatable with only ONE of -1LRHb.\n");
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
    printf("# COLHEAD %2d Number of Measurments done by JudyLGet/Judy1Test\n", Col++);

    if (tFlag)
        printf
            ("# COLHEAD %2d 'Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
             Col++);
    if (J1Flag)
        printf("# COLHEAD %2d J1S  - Judy1Set\n", Col++);
    if (JLFlag)
        printf("# COLHEAD %2d JLI  - JudyLIns\n", Col++);
    if (JRFlag)
        printf("# COLHEAD %2d JLI-R  - JudyLIns\n", Col++);
    if (JHFlag)
        printf("# COLHEAD %2d JHSI - JudyHSIns\n", Col++);
    if (bFlag)
        printf("# COLHEAD %2d BMSet  - Bitmap Set\n", Col++);
    if (yFlag)
        printf("# COLHEAD %2d BySet  - ByteMap Set\n", Col++);

    if (tFlag)
        printf
            ("# COLHEAD %2d 'Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
             Col++);
    if (J1Flag)
        printf("# COLHEAD %2d J1T  - Judy1Test\n", Col++);
    if (JLFlag)
        printf("# COLHEAD %2d JLG  - JudyLGet\n", Col++);
    if (JRFlag)
        printf("# COLHEAD %2d JLG-R  - JudyLGet\n", Col++);
    if (JHFlag)
        printf("# COLHEAD %2d JHSG - JudyHSGet\n", Col++);
    if (bFlag)
        printf("# COLHEAD %2d BMTest  - Bitmap Test\n", Col++);
    if (yFlag)
        printf("# COLHEAD %2d ByTest  - ByteMap Test\n", Col++);

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
            ("# COLHEAD %2d 1heap/K  - Judy1 malloc'ed words per Key\n",
             Col++);
    }
    if (JLFlag)
    {
        printf
            ("# COLHEAD %2d Lheap/K  - JudyL malloc'ed words per Key\n",
             Col++);
    }
    if (JHFlag)
    {
        printf
            ("# COLHEAD %2d HSheap/K - JudyHS malloc'ed words per Key\n",
             Col++);
    }
    if (bFlag)
    {
        printf
            ("# COLHEAD %2d Btheap/K - Bitmap malloc'ed words per Key\n",
             Col++);
    }
    if (yFlag)
    {
        printf
            ("# COLHEAD %2d Byheap/K - Bytemap malloc'ed words per Key\n",
             Col++);
    }
    if (mFlag)
    {

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


        printf("# COLHEAD %2d MisCmp - Average number missed Compares Per Leaf Search\n", Col++);
        printf("# COLHEAD %2d %%DiHt - %% of Direct Hits per Leaf Search\n", Col++);

//        printf("# COLHEAD %2d TrDep  - Tree depth with LGet/1Test searches\n", Col++);
        printf("# COLHEAD %2d AvPop  - Average Current Leaf Population\n", Col++);
        printf("# COLHEAD %2d %%MalEff - %% RAM malloc()ed vs mmap()ed from Kernel\n", Col++);

        if (J1Flag)
            printf
                ("# COLHEAD %2d MF1/K    - Judy1 average malloc+free's per Key\n",
                 Col++);
        if (JLFlag)
            printf
                ("# COLHEAD %2d MFL/K    - JudyL average malloc+free's per Key\n",
                 Col++);
        if (JHFlag)
            printf
                ("# COLHEAD %2d MFHS/K   - JudyHS average malloc+free's per Key\n",
                 Col++);
    }
    if (J1Flag)
        printf("# %s - Leaf sizes in Words\n", Judy1MallocSizes);

    if (JLFlag)
        printf("# %s - Leaf sizes in Words\n#\n", JudyLMallocSizes);


    if (bFlag)
    {
        Word_t Words;

        printf("# ========================================================\n");
        printf
            ("#     WARNING '-b' option with '-B%lu' option will malloc() a\n",
             BValue);
        printf("#     fixed sized Bitmap of %lu bytes.\n", (Word_t)1 << (BValue - 3));
        printf("# ========================================================\n");

//      Allocate a Bitmap
        Words = (Word_t)1 << (BValue - (3 + cLg2WS));
        Bytes = Words * sizeof(Word_t);

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail

        B1 = (PWord_t)JudyMalloc(Bytes / sizeof(Word_t));
        if (B1 == (PWord_t)NULL)
        {
            FAILURE("malloc failure, Bytes =", Bytes);
        }
//      clear 1/2 bitmap and bring into RAM
        STARTTm;
//        for (ii = 0; ii < Words; ii++)
//            ((Word_t *)B1)[ii] = 0;
        bzero((void *)B1, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# bzero() Bitmap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

//      copy 1st half to 2nd half
        STARTTm;
        memcpy((void *)((uint8_t *)B1 + (size_t)Bytes / 2), (void *)B1, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# memcpy() Bitmap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

    if (yFlag)
    {
        Word_t    Words;

        if ((sizeof(Word_t) == 4) && (BValue == 32))
        {
            FAILURE("Must be Less than -B32 on 32 Bit machines =", BValue);
        }

        printf("# ========================================================\n");
        printf
            ("#     WARNING '-y' option with '-B%lu' option will malloc() a\n",
             BValue);
        printf("#     fixed sized Bytemap of %lu bytes.\n", (Word_t)1 << BValue);
        printf("# ========================================================\n");

//      Allocate a Bytemap

        Words = (Word_t)1 << (BValue - cLg2WS);
        Bytes = Words * sizeof(Word_t);

        printf("# Attempting to malloc(%lu) Words (%lu Bytes)\n", Words, Bytes);

        By = (uint8_t *)JudyMalloc(Words);
        if (By == (uint8_t *)NULL)
        {
            FAILURE("malloc failure, Bytes =", Bytes);
        }
//      clear 1/2 bitmap and bring into RAM
        STARTTm;
//        for (ii = 0; ii < Words; ii++)
//            ((Word_t *)By)[ii] = 0;
        bzero((void *)By, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# Zero Bytemap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

//      copy 1st half to 2nd half
        STARTTm;
//        for (ii = 0; ii < Words; ii++)
//            ((Word_t *)By)[ii] = 0;
        memcpy((void *)((uint8_t *)By + (size_t)Bytes / 2), (void *)By, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# memcpy Bytemap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

// ============================================================
// Warm up the cpu
// ============================================================

//  Try to fool compiler and really execute random()
    STARTTm;
    do {
        for (MaxNumb = ii = 0; ii < 1000000; ii++)
            MaxNumb = random();

        if (MaxNumb == 0)       // will never happen, but compiler does not know
            printf("\n");

        ENDTm(DeltanSecW);      // get accumlated elapsed time
    } while (DeltanSecW < Warmup);       // until 2 seconds elapsed

//  Now measure the execute time for 1M calls to random().
    STARTTm;
    for (MaxNumb = ii = 0; ii < 1000000; ii++)
        MaxNumb = random();
    ENDTm(DeltanSecW);      // get elapsed time

    if (MaxNumb == 0)       // will never happen, but compiler does not know
            printf("\n");

//  If this number is not consistant, then a longer warmup period is required
    printf("# random() = %4.2f nSec per/call\n", DeltanSecW/1000000);
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

    // Let Mike's code print out some initialization junk.
    Judy1FreeArray(&J1, PJE0);

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

#if defined(SWAP)
        if (J1Flag)
#else // defined(SWAP)
        if (bFlag)
#endif // defined(SWAP)
        {
//          Allocate a Bitmap, if not already done so
            if (B1 == NULL)
            {
                Word_t    ii;
                size_t    BMsize;

                // add one cache line for sister cache line read
                BMsize = (1UL << (BValue - 3));
#if defined(SISTER_READ)
                BMsize += 0x1000000 + 0x1000000;
#endif // defined(SISTER_READ)
                if (posix_memalign((void **)&B1, 4096, BMsize) == 0)
                {
                    FAILURE("malloc failure, Bytes =", BMsize);
                }
//              clear the bitmap and bring into RAM
                for (ii = 0; ii < (BMsize / sizeof(Word_t)); ii++)
                    B1[ii] = 0;
#if defined(SISTER_READ)
                B1 += 0x1000000 / sizeof(Word_t);
#endif // defined(SISTER_READ)
#if defined(SWAP)
                J1 = B1;
#endif // defined(SWAP)
            }
        }

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

//            TreeDepth        = j__TreeDepth;
//            SearchPopulation = j__SearchPopulation;
            DirectHits      = j__DirectHits;           // Number of direct hits
            SearchGets       = j__SearchGets;           // Number of object calls

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

//      Insert/Get JudyL using Value area as next Key
        if (JRFlag)
        {
//          Test JLI
//          Exit with InsertSeed/Key ready for next batch

            Tit = 0;                    // exclude Judy
            DummySeed = InsertSeed;
            WaitForContextSwitch(Delta);
            TestJudyLIns(&JL, &DummySeed, Delta);
            DeltaGenL = DeltanSecL;

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyLIns(&JL, &InsertSeed, Delta);
            if (tFlag)
                PRINT6_1f(DeltaGenL);
            DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (fFlag)
                fflush(NULL);

            Tit = 0;                    // exclude Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyLGet(JL, &BeginSeed, Meas);
            DeltaGenL = DeltanSecL;

            Tit = 1;                    // include Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyLGet(JL, &BeginSeed, Meas);

//            TreeDepth        = j__TreeDepth;

//            SearchPopulation = j__SearchPopulation;
            DirectHits       = j__DirectHits;           // Number of direct hits
            SearchGets       = j__SearchGets;           // Number of object calls

            if (tFlag)
                PRINT6_1f(DeltaGenL);
            DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
            if (fFlag)
                fflush(NULL);
        }

//      Test a REAL bitmap
        if (bFlag)
        {
            double    DeltanBit;

            DummySeed = BitmapSeed;
            GetNextKey(&DummySeed);   // warm up cache

            Tit = 0;
            DummySeed = BitmapSeed;
            WaitForContextSwitch(Delta);
            TestBitmapSet(&B1, &DummySeed, Delta);
            DeltanBit = DeltanSecBt;

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestBitmapSet(&B1, &BitmapSeed, Delta);

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

//      Test a REAL ByteMap
        if (yFlag)
        {
            double    DeltanByte;

            DummySeed = BitmapSeed;
            GetNextKey(&DummySeed);   // warm up cache

            Tit = 0;
            DummySeed = BitmapSeed;
            WaitForContextSwitch(Delta);
            TestByteSet(&DummySeed, Delta);
            DeltanByte = DeltanSecBy;

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestByteSet(&BitmapSeed, Delta);

            if (tFlag)
                PRINT6_1f(DeltanByte);
            DONTPRINTLESSTHANZERO(DeltanSecBy, DeltanByte);

            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestByteTest(&BeginSeed, Meas);
            DeltanByte = DeltanSecBy;

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestByteTest(&BeginSeed, Meas);

            if (tFlag)
                PRINT6_1f(DeltanByte);
            DONTPRINTLESSTHANZERO(DeltanSecBy, DeltanByte);
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

        if ((J1Flag + JLFlag + JHFlag) == 1)            // only 1 Heap
            PRINT7_3f((double)j__AllocWordsTOT / (double)Pop1);

        if (mFlag && (bFlag == 0) && (yFlag == 0))
        {
//            double AveSrcCmp, PercentLeafSearched;
            double PercentLeafWithDirectHits;
            
//          Calc average compares done in Leaf for this measurement interval
//            AveSrcCmp = SearchCompares / (double)Meas;
//            AveSrcCmp = DirectHits / SearchGets;

//          Calc average percent of Leaf searched
//            if (SearchPopulation == 0)
//                PercentLeafWithDirectHits = 0.0;
//            else
//                PercentLeafWithDirectHits = SearchCompares / SearchPopulation * 100.0;
//
            if (SearchGets == 0)
                PercentLeafWithDirectHits = 0.0;
            else
                PercentLeafWithDirectHits = DirectHits / SearchGets * 100.0;

            PRINT5_2f((double)j__AllocWordsJBB   / (double)Pop1);       // 256 node branch
            PRINT5_2f((double)j__AllocWordsJBU   / (double)Pop1);       // 256 node branch
            PRINT5_2f((double)j__AllocWordsJBL   / (double)Pop1);       // xx node branch


            PRINT5_2f((double)j__AllocWordsJLLW  / (double)Pop1);       // 32[64] Key

            PRINT5_2f((double)j__AllocWordsJLL7  / (double)Pop1);       // 32 bit Key
            PRINT5_2f((double)j__AllocWordsJLL6  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL5  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL4  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL3  / (double)Pop1);       // 16 bit Key
            PRINT5_2f((double)j__AllocWordsJLL2  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJLL1  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJLB1  / (double)Pop1);       // 12 bit Key
            PRINT5_2f((double)j__AllocWordsJV    / (double)Pop1);       // Values for 12 bit


// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSss


//          print average number of failed compares done in leaf search
//            printf(" %6.1f", AveSrcCmp);
            printf(" %6.1f", j__MissCompares / (double)Meas);

//          print average percent of Leaf searched (with compares)
            printf(" %5.1f", PercentLeafWithDirectHits);

//          print average number of Branches traversed per lookup
//            printf(" %5.1f", TreeDepth / (double)Meas);
//
            if (j__SearchGets == 0)
                printf(" %5.1f", 0.0);
            else
                printf(" %5.1f", (double)j__SearchPopulation / (double)j__SearchGets);

//          reset for next measurement
//            j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;


// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSss


//          Print the percent efficiency of dlmalloc
            PRINT7_3f(j__AllocWordsTOT / (double)(j__TotalBytesAllocated / sizeof(Word_t)));
            if (J1Flag)
                PRINT5_2f((double)DeltaMalFre1);
            if (JLFlag || JRFlag)
                PRINT5_2f((double)DeltaMalFreL);
            if (JHFlag)
                PRINT5_2f((double)DeltaMalFreHS);
        }
        if (yFlag || bFlag)
        {
            printf(" %14.2f", ((double)j__TotalBytesAllocated / sizeof(Word_t)) / (double)Pop1);
        }
        printf("\n");
        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

#ifdef SKIPMACRO
    if (J1Flag)
        Count1 = Judy1Count(J1, 0, -1, PJE0);
    if (JLFlag || JRFlag)
        CountL = JudyLCount(JL, 0, -1, PJE0);
#else
    if (J1Flag)
        J1C(Count1, J1, 0, -1);
    if (JLFlag || JRFlag)
        JLC(CountL, JL, 0, -1);         // get the counts
#endif // SKIPMACRO

    if ((JLFlag | JRFlag) && J1Flag)
    {
        if (CountL != Count1)
            FAILURE("Judy1/LCount not equal", Count1);
    }

    if (J1Flag)
    {
        STARTTm;

//#ifdef SKIPMACRO
        Bytes = Judy1FreeArray(&J1, PJE0);
//#else
//        J1FA(Bytes, J1);                // Free the Judy1 Array
//#endif // SKIPMACRO

        ENDTm(DeltanSec1);

        DeltanSec1 /= (double)Count1;

        printf
            ("# Judy1FreeArray:  %lu, %0.3f Words/Key, %lu Bytes released, %0.3f nSec/Key\n",
             Count1, (double)(Bytes / sizeof(Word_t)) / (double)Count1, Bytes,
             DeltanSec1);
    }

    if (JLFlag || JRFlag)
    {
        STARTTm;
        Bytes = JudyLFreeArray(&JL, PJE0);
        ENDTm(DeltanSecL);

        DeltanSecL /= (double)CountL;
        printf
            ("# JudyLFreeArray:  %lu, %0.3f Words/Key, %lu Bytes released, %0.3f nSec/Key\n",
             CountL, (double)(Bytes / sizeof(Word_t)) / (double)CountL, Bytes,
             DeltanSecL);
    }

    if (JHFlag)
    {

        STARTTm;

//#ifdef SKIPMACRO
        Bytes = JudyHSFreeArray(&JH, PJE0);     // Free the JudyHS Array
//#else
//        JHSFA(Bytes, JH);               // Free the JudyHS Array
//#endif // SKIPMACRO

        ENDTm(DeltanSecHS);

        DeltanSecHS /= (double)nElms;   // no Counts yet
        printf
            ("# JudyHSFreeArray: %lu, %0.3f Words/Key, %0.3f nSec/Key\n",
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

static int
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
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0 && Tit)                // Remove previously inserted
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
#if defined(SWAP)
                    Rc = BitmapSet(*J1, TstKey);
#else // defined(SWAP)
                    J1S(Rc, *J1, TstKey);
#endif // defined(SWAP)
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
                            ("\n--- Judy1Set Rc = %d after Judy1Set, Key = 0x%lx, elm = %lu",
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
                            ("\n--- Judy1Test Rc = %d after Judy1Set, Key = 0x%lx, elm = %lu",
                             Rc, TstKey, elm);
                            FAILURE("Judy1Test failed at", elm);
                        }
                    }
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
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0 && Tit)                // Remove previously inserted
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
                            if (TstKey)
                            {
                                printf("TstKey = 0x%lx", TstKey);
                                FAILURE("JudyLIns failed - DUP Key =", TstKey);
                            }
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
                            if (PValueNew != PValue)
                            {
                                printf("\n#Line = %d, Caution: PValueNew = 0x%lx, PValueold = 0x%lx changed\n", __LINE__, (Word_t)PValueNew, (Word_t)PValue);
//                                printf("- ValueNew = 0x%lx, Valueold = 0x%lx\n", *PValueNew, *PValue);
//                                FAILURE("Second JudyLIns failed with wrong PValue after Insert", TstKey);
                            }
                            if (*PValueNew != TstKey)
                            {
                                printf("\n*PValueNew = 0x%lx\n", *PValueNew);
                                printf("TstKey = 0x%lx = %ld\n", TstKey, TstKey);
                                FAILURE("Second JudyLIns failed with wrong *PValue after Insert", TstKey);
                            }
                        }
                        if (gFlag)
                        {
                            PWord_t   PValueNew;

                            PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                            if (PValueNew == NULL)
                            {
                                printf("\n--- TstKey = 0x%lx", TstKey);
                                FAILURE("JudyLGet failed after Insert", TstKey);
                            }
                            else
                            {
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n--- *PValueNew = 0x%lx\n", *PValueNew);
                                    printf("--- TstKey = 0x%lx = %ld", TstKey, TstKey);
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
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            if (lp != 0 && Tit)                // Remove previously inserted
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


//  JudyLIns timings
void
TestJudyLIns(void **JL, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    Seed_t    WorkingSeed;
    Seed_t    SaveSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Word_t    StartMallocs;

    DeltanSecL = 0.0;
    PValue = (PWord_t)NULL;

    WorkingSeed = *PSeed;

    Loops = 1;
    if (!lFlag)
    {
        if (Elements < 100)
            Loops = (MAXLOOPS / Elements) + MINLOOPS;
    }

//  JudyLIns timings
    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;
        if (lp != 0 && Tit)                // Remove previously inserted
        {
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);
                JudyLDel(JL, TstKey, PJE0);
            }
        }

        StartMallocs = MalFreeCnt;
        WorkingSeed = *PSeed;           // restore after Del

        STARTTm;                        // start timer
        TstKey = GetNextKey(&WorkingSeed);
        if (Tit)
        {
            PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
            if (PValue == (PWord_t)NULL)
                FAILURE("JudyLIns failed - NULL PValue", TstKey);
            if ((*PValue != 0) && (TstKey != 0))
                FAILURE("JudyLIns failed - *PValue not = 0", TstKey);
        }

        for (elm = 0; elm < (Elements - 1); elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);
            if (Tit)
            {
                *PValue = TstKey;                   // save in previous
                PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
                if (PValue == (PWord_t)NULL)
                    FAILURE("JudyLIns failed - NULL PValue", TstKey);
                if ((*PValue != 0) && (TstKey != 0))
                    FAILURE("JudyLIns failed - *PValue not = 0", TstKey);
            }
        }

        SaveSeed = WorkingSeed;           // save for next time
        TstKey = GetNextKey(&WorkingSeed);
        if (Tit)
            *PValue = TstKey;
        WorkingSeed = SaveSeed;

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
    *PSeed = WorkingSeed;               // advance
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
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

//          reset for next measurement
//            j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                TstKey = GetNextKey(&WorkingSeed);

                if (Tit)
                {
#if defined(SWAP)
                    Rc = BitmapGet(J1, TstKey);
#else // defined(SWAP)
#ifdef SKIPMACRO
                    Rc = Judy1Test(J1, TstKey, PJE0);
#else
                    J1T(Rc, J1, TstKey);
#endif // SKIPMACRO
#endif // defined(SWAP)

                    if (Rc != 1)
                    {
                        printf
                            ("\n--- Judy1Test wrong Rc = %d, Key = 0x%lx, elm = %lu",
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
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

//          reset for next measurement
//            j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

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
                        printf("\n--- JudyGet Key = 0x%lx", TstKey);
                        FAILURE("JudyLGet ret PValue = NULL", 0L);
                    }
                    else if (VFlag && (*PValue != TstKey))
                    {
                        printf
                            ("--- JudyLGet returned Value=0x%lx, should be=0x%lx",
                             *PValue, TstKey);
                        FAILURE("JudyLGet ret wrong Value at", elm);
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
        for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;

//            j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

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

void
TestJudyLGet(void *JL, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
//    Word_t   *PValue;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;
    Seed_t    WorkingSeed;

    Loops = (MAXLOOPS / Elements) + MINLOOPS;

    if (lFlag)
        Loops = 1;

    icnt = ICNT;

    for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

//      reset for next measurement
//        j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

        TstKey = GetNextKey(&WorkingSeed);      // Get 1st Key
        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            if (Tit)
                TstKey = *(PWord_t)JudyLGet(JL, TstKey, PJE0);
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

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudy1Copy"

int
TestJudy1Copy(void *J1, Word_t Elements)
{
    Pvoid_t   J1a;                      // Judy1 new array
    Word_t    elm = 0;
//    Word_t    Bytes;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    Loops;

    J1a = NULL;                         // Initialize To array

    Loops = (MAXLOOPS / Elements) + MINLOOPS;
    if (lFlag)
        Loops = 1;

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

//#ifdef SKIPMACRO
        Judy1FreeArray(&J1a, PJE0);
//#else
//        J1FA(Bytes, J1a);               // no need to keep it around
//#endif // SKIPMACRO

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

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            TstKey = GetNextKey(&WorkingSeed);

            if (Tit)
            {

#ifdef SKIPMACRO
                Rc = Judy1Unset(J1, TstKey, PJE0);
#else
                J1U(Rc, *J1, TstKey);
#endif // SKIPMACRO

                if (Rc != 1)
                {
                    printf("--- Key = 0x%lx", TstKey);
                    FAILURE("Judy1Unset ret Rcode != 1", Rc);
                }

                if (gFlag)
                {
                    Rc = Judy1Test(*J1, TstKey, PJE0);

                    if (Rc)
                    {
                        printf("\n--- Judy1Test success after Judy1Unset, Key = 0x%lx", TstKey);
                        FAILURE("Judy1Test success after Judy1Unset", TstKey);
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
                {
                    printf("\n--- Key = 0x%lx", TstKey);
                    FAILURE("JudyLDel ret Rcode != 1", Rc);
                }

                if (gFlag)
                {
                    PWord_t   PValueNew;

                    PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                    if (PValueNew != NULL)
                    {
                        printf("\n--- JudyLGet success after JudyLDel, Key = 0x%lx, Value = 0x%lx", TstKey, *PValueNew);
                        FAILURE("JudyLGet success after JudyLDel", TstKey);
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

// ********************************************************************

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestByteTest"

int
TestByteTest(PSeed_t PSeed, Word_t Elements)
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
                if (By[TstKey] == 0)
                {
                    printf("\nByteGet -- missing bit, Key = 0x%lx",
                           TstKey);
                    FAILURE("ByteGet Word = ", elm);
                }
            }
        }
        ENDTm(DeltanSecBy);

        if (DminTime > DeltanSecBy)
        {
            icnt = ICNT;
            if (DeltanSecBy > 0.0)        // Ignore 0
                DminTime = DeltanSecBy;
        }
        else
        {
            if (--icnt == 0)
                break;
        }
    }
    DeltanSecBy = DminTime / (double)Elements;

    return 0;
}       // TestByteTest()

int
TestByteSet(PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;

    STARTTm;
    for (elm = 0; elm < Elements; elm++)
    {
        TstKey = GetNextKey(PSeed);

        if (Tit)
        {
            if (By[TstKey])
            {
                if (GValue)
                {
                    ByteDups++;
                }
                else
                {
                    printf("\nByteSet -- Set bit, Key = 0x%lx", TstKey);
                    FAILURE("ByteSet Word = ", elm);
                }
            }
            else
            {
                By[TstKey] = 1;
            }
        }
    }
    ENDTm(DeltanSecBy);

    DeltanSecBy /= (double)Elements;

    return (0);
}       // TestByteSet()


// ********************************************************************

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestBitmapSet"

int
TestBitmapSet(PWord_t *pB1, PSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;

    STARTTm;
    for (elm = 0; elm < Elements; elm++)
    {
        TstKey = GetNextKey(PSeed);

        if (Tit)
        {
#if defined(SWAP)
            if (Judy1Set((PPvoid_t)pB1, TstKey, PJE0) == 0)
#else // defined(SWAP)
            if (BitmapSet(*pB1, TstKey) == 0)
#endif // defined(SWAP)
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
#if defined(SWAP)
                if (Judy1Test(B1, TstKey, PJE0) == 0)
#else // defined(SWAP)
                if (BitmapGet(B1, TstKey) == 0)
#endif // defined(SWAP)
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

