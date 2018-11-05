// =======================================================================
//                      -by-
//   Author Douglas L. Baskins, Aug 2003.
//   Permission to use this code is freely granted, provided that this
//   statement is retained.
//   email - dougbaskins .at. yahoo.com -or- dougbaskins .at. gmail.com
// =======================================================================

// MEB:
// TODO: Enhance -F to read the file in parts.

#include <unistd.h>                     // getopt()
#include <getopt.h>                     // getopt_long()
#include <stdlib.h>                     // exit(), malloc(), random()
#include <stdio.h>                      // printf(), setbuf()
#include <math.h>                       // pow()
#include <time.h>                       // clock_gettime()
#include <sys/utsname.h>                // uname()
#include <sys/mman.h>                   // mmap()
#include <errno.h>                      // errnoerrno
#include <string.h>                     // strtok_r()
#include <strings.h>                    // bzero()
#ifdef USE_PDEP_INTRINSIC
#include <immintrin.h>
#endif // USE_PDEP_INTRINSIC

// Judy1LHTime.c doesn't include bdefines.h so we want to be sure to
// define DEBUG if DEBUG_ALL is defined.
#ifdef DEBUG_ALL
#undef DEBUG
#define DEBUG
#endif // DEBUG_ALL

// Turn off assert(0) by default
#ifndef DEBUG
#define NDEBUG 1
#endif  // DEBUG

#include <assert.h>                     // assert()

#include <Judy.h>                       // for Judy macros J*()

#if defined(JUDY_V105) || defined(JUDY_METRICS_GLOBALS)
// Judy 1.0.5 doesn't have RAMMETRICS but this modern Time.c program
// assumes the globals exist. Use -DJUDY_V105 or -DJUDY_METRICS_GLOBALS
// to build this modern Time.c program in a Judy 1.0.5 working directory.
Word_t j__MalFreeCnt;
Word_t j__MFlag;
Word_t j__AllocWordsTOT;
Word_t j__AllocWordsJBB;
Word_t j__AllocWordsJBU;
Word_t j__AllocWordsJBL;
Word_t j__AllocWordsJLLW;
Word_t j__AllocWordsJLL7;
Word_t j__AllocWordsJLL6;
Word_t j__AllocWordsJLL5;
Word_t j__AllocWordsJLL4;
Word_t j__AllocWordsJLL3;
Word_t j__AllocWordsJLL2;
Word_t j__AllocWordsJLL1;
Word_t j__AllocWordsJLB1;
Word_t j__AllocWordsJV;
Word_t j__AllocWordsTOT;
Word_t j__TotalBytesAllocated;
#endif // defined(JUDY_V105) || defined(JUDY_METRICS_GLOBALS)

// The released Judy libraries do not, and some of Doug's work-in-progress
// libraries may not, have Judy1Dump and/or JudyLDump entry points.
// And Mike sometimes links Judy1LHTime with his own Judy1 library and the
// released or Doug's JudyL or with his own JudyL and the released or
// Doug's Judy1 libraries.
// We want to be able to use the same Time.c for all of these cases.
// The solution is to define JUDY1_V2 and/or JUDYL_V2 if/when we want Time.c
// to use Judy1Dump and/or JudyLDump for real.
// The solution is to define JUDY1_V2 and/or JUDY1_DUMP and/or JUDYL_V2
// and/or JUDYL_DUMP if/when we want Time.c to use Judy1Dump and/or
// JudyLDump for real.

#if !defined(JUDY1_V2) && !defined(JUDY1_DUMP)
#define Judy1Dump(wRoot, nBitsLeft, wKeyPrefix)
#endif // !defined(JUDY1_V2) && !defined(JUDY1DUMP)

#if !defined(JUDYL_V2) && !defined(JUDYL_DUMP)
#define JudyLDump(wRoot, nBitsLeft, wKeyPrefix)
#endif // !defined(JUDYL_V2) && !defined(JUDYLDUMP)

// Why did we create CALC_NEXT_KEY? Or, rather, why did we create the
// alternative, an array of keys, since CALC_NEXT_KEY used to be the only
// behavior? And why did we create LFSR_ONLY and -k/--lfsr-only.
//
// Because we discovered cases where the Tit flag was not effective at
// extracting GetNextKey overhead and normalizing the TestJudyGet times
// from different runs using different GetNextKey options.
// For example, we saw the time jump from 28 ns to 52 ns in the bitmap
// area by adding --splay-key-bits=0xffffffffffffffff to a 64-bit -B32
// Judy1 run (and --splay-key-bits=0xffffffffffffffff has absolutely no
// affect on the sequence of keys that are tested).
//
// This bump in get time was observed without a __sync_synchronize in
// the loop.  Adding a __sync_synchronize before the GetNextKey brings
// the times back within 2% of each other for most of the plot at the cost
// of bumping those 16-bit bitmap area times up to around 60 ns (the
// cost of a __sync_synchronize all by itself may be around eight ns).
// We found that adding a second __sync_synchronize after GetNextKey
// doesn't change our timings at all other than to increase the overhead
// number by the __sync_synchronize's own cost of around eight ns.
//
// So, even if we're willing to overlook the significant downside of being
// required to use the __sync_synchronize to get somewhat comparable
// results, there are other hints that this __sync_synchronize approach is
// not a perfect solution and comparing times from runs with different
// GetNextKey options and/or different code may be tricky. Be careful.
//
// The original goal of the alternative to CALC_NEXT_KEY was to have
// consistent overhead independent of GetNextKey options while not being
// required to do a __sync_synchronize. And wouldn't it be nice if the
// overhead were so low that we could do away with the Tit flag altogether.
// The latter is why we came up with LFSR_ONLY and -k/--lfsr-only.

#ifdef LFSR_ONLY
#define CALC_NEXT_KEY
#define NO_FVALUE
#define NO_TRIM_EXPANSE
#define NO_SPLAY_KEY_BITS
#define NO_DFLAG
#define NO_OFFSET
#define NO_SVALUE
#define NO_GAUSS
// KFLAG is in by default with LFSR_ONLY
#endif // LFSR_ONLY

// Judy 1.0.5 did not have RandomNumb.h. We must copy the RandomNumb.h that
// goes with this Time.c into judy/test along with this Time.c in order to
// build this modern time program in a Judy 1.0.5 working directory.
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
//   cc -m64 -O -g Judy1LHTime.c -lm -lrt -lJudy -o Judy1LHTime
//           -or-
//   cc -m64 -O2 -Wall -I../src Judy1LHTime.c -lm -lrt ../src/libJudy64.a -o Judy1LHTime64

// =======================================================================
//   A little help with printf("0x%016x...  vs printf(0x%08x...
// =======================================================================

// damn apple or posix
#ifndef MAP_ANONYMOUS
#define  MAP_ANONYMOUS MAP_ANON
#endif  // ! MAP_ANONYMOUS

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

extern Word_t    j__AllocWordsTOT;
extern Word_t    j__MFlag;                     // Print memory allocation on stderr
extern Word_t    j__TotalBytesAllocated;       //
extern Word_t    j__MalFreeCnt;                // JudyMalloc() + Judyfree() count

extern Word_t    j__AllocWordsJBB;
extern Word_t    j__AllocWordsJBU;
extern Word_t    j__AllocWordsJBL;
extern Word_t    j__AllocWordsJLB1;
extern Word_t    j__AllocWordsJLL1;
extern Word_t    j__AllocWordsJLL2;
extern Word_t    j__AllocWordsJLL3;
extern Word_t    j__AllocWordsJLL4;
extern Word_t    j__AllocWordsJLL5;
extern Word_t    j__AllocWordsJLL6;
extern Word_t    j__AllocWordsJLL7;
extern Word_t    j__AllocWordsJLLW;
extern Word_t    j__AllocWordsJV;
extern Word_t    j__NumbJV;

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



// =======================================================================
// Common macro to handle a failure
// =======================================================================
#define FAILURE(STR, UL)                                                \
{                                                                       \
printf(        "\n--- Error: %s %" PRIuPTR", file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
fprintf(stderr,"\n--- Error: %s %" PRIuPTR", file='%s', 'function='%s', line %d\n", \
        STR, (Word_t)(UL), __FILE__, __FUNCTI0N__, __LINE__);           \
        exit(1);                                                        \
}

#define MINLOOPS 1
#define MAXLOOPS 1000

// Maximum of 10 loops with no improvement
#define ICNT 10

// Structure to keep track of times
typedef struct MEASUREMENTS_STRUCT
{
    Word_t    ms_delta;                 // leave room for more stuff
}
ms_t     , *Pms_t;

#ifdef CALC_NEXT_KEY
typedef Seed_t NewSeed_t;
#else // CALC_NEXT_KEY
typedef PWord_t NewSeed_t;
#endif // CALC_NEXT_KEY

typedef NewSeed_t *PNewSeed_t;

//=======================================================================
//             P R O T O T Y P E S
//=======================================================================


// Specify prototypes for each test routine
//int       NextSum(Word_t *PNumber, double *PDNumb, double DMult);

int       TestJudyIns(void **J1, void **JL, void **JH, PNewSeed_t PSeed,
                      Word_t Elems);

void      TestJudyLIns(void **JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudyDup(void **J1, void **JL, void **JH, PNewSeed_t PSeed,
                      Word_t Elems);

int       TestJudyDel(void **J1, void **JL, void **JH, PNewSeed_t PSeed,
                      Word_t Elems);

static inline int
TestJudyGet(void *J1, void *JL, void *JH, PNewSeed_t PSeed, Word_t Elems,
            Word_t Tit, Word_t KFlag, Word_t hFlag, int bLfsrOnly);

void      TestJudyLGet(void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudy1Copy(void *J1, Word_t Elem);

int       TestJudyCount(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

Word_t    TestJudyNext(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudyPrev(void *J1, void *JL, PNewSeed_t PSeed, Word_t HighKey, Word_t Elems);

int       TestJudyNextEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudyPrevEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestBitmapSet(PWord_t *pB1, PNewSeed_t PSeed, Word_t Meas);

int       TestBitmapTest(PWord_t B1, PNewSeed_t PSeed, Word_t Meas);

int       TestByteSet(PNewSeed_t PSeed, Word_t Meas);

int       TestByteTest(PNewSeed_t PSeed, Word_t Meas);

int       TimeNumberGen(void **TestRan, PNewSeed_t PSeed, Word_t Delta);


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
// Routine to reverse the bits in the input data word
static inline Word_t
Swizzle(Word_t word)
{
// BIT REVERSAL, Ron Gutman in Dr. Dobb's Journal, #316, Sept 2000, pp133-136
//

#ifdef __LP64__
    word = __builtin_bswap64(word);
    word = ((word & 0x0f0f0f0f0f0f0f0f) << 4) |
        ((word & 0xf0f0f0f0f0f0f0f0) >> 4);
    word = ((word & 0x3333333333333333) << 2) |
        ((word & 0xcccccccccccccccc) >> 2);
    word = ((word & 0x5555555555555555) << 1) |
        ((word & 0xaaaaaaaaaaaaaaaa) >> 1);
#else  // not __LP64__
    word = __builtin_bswap32(word);
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

#ifndef NO_KFLAG
#define KFLAG
#endif // NO_KFLAG

#ifdef KFLAG
#define SYNC_SYNC(_x)  if (KFlag) __sync_synchronize(); _x
#elif SYNC_SYNC
#undef SYNC_SYNC
#define SYNC_SYNC(_x)  __sync_synchronize(); _x
#else // KFLAG, SYNC_SYNC
#define SYNC_SYNC(_x)  _x
#endif // KFLAG, SYNC_SYNC


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
Word_t    Warmup = 2000;                // milliseconds to warm up CPU

PWord_t   B1 = NULL;                    // BitMap
#define cMaxColon ((int)sizeof(Word_t) * 2)  // Maximum -b suboption paramters
int       bParm[cMaxColon + 1] = { 0 }; // suboption parameters to -b#:#:# ...
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

// bLfsrOnly is a way to use a simple, fast lfsr with -UCALC_NEXT_KEY.
int       bLfsrOnly = 0;      // -k,--lfsr-only; use lfsr with no -B:DEFGNOoS
// -k with -DS1 requires -DLFSR_GET_FOR_DS1
int bLfsrForGetOnly = 0; // -k with -DS1 uses fast lfsr for gets only

Word_t wFeedBTap = (Word_t)-1; // for bLfsrOnly; -1 means uninitialized
static int krshift = 1; // for bLfsrOnly -- static makes it faster

Word_t    hFlag = 0;                    // add "holes" into the insert code
Word_t    PreStack = 0;                 // to test for TLB collisions with stack

Word_t    Offset = 0;                   // Added to Key
Word_t    bSplayKeyBitsFlag = 0;        // Splay key bits.
#if defined(__LP64__) || defined(_WIN64)
Word_t wSplayMask = 0x5555555555555555; // default splay mask
//Word_t wSplayMask = 0xeeee00804020aaff;
#else // defined(__LP64__) || defined(_WIN64)
Word_t wSplayMask = 0x55555555;         // default splay mask
//Word_t wSplayMask = 0xff00ffff;
#endif // defined(__LP64__) || defined(_WIN64)
Word_t    wCheckBit = 0;                // Bit for narrow ptr testing.

Word_t    TValues =  1000000;           // Maximum numb retrieve timing tests
// nElms is the total number of keys that are inserted into the test arrays.
// Default nElms is overridden by -n or -F.
// Looks like there may be no protection against -F followed by -n.
// It is then trimmed to MaxNumb.
// Should it be MaxNumb+1 in cases that allow 0, e.g. -S1?
// Then trimmed if ((GValue != 0) && (nElms > (MaxNumb >> 1))) to (MaxNumb >> 1).
// It is used for -p and to override TValues if TValues == 0 or nElms < TValues.
Word_t    nElms   = 10000000;           // Default population of arrays
Word_t    ErrorFlag = 0;

//  Measurement points per decade increase of population
//
//Word_t    PtsPdec = 10;               // 25.89% spacing
//Word_t    PtsPdec = 25;               // 9.65% spacing
//Word_t    PtsPdec = 40;               // 5.93% spacing
Word_t    PtsPdec = 50;                 // 4.71% spacing - default
//Word_t    PtsPdec = 231;              // 1% spacing

// For LFSR (Linear-Feedback-Shift-Register) pseudo random Number Generator
//
#define DEFAULT_BVALUE  32
Word_t    BValue = DEFAULT_BVALUE;
double    Bpercent = 100.0; // Default MaxNumb assumes 100.0.
// MaxNumb is initialized from DEFAULT_BVALUE and assumes Bpercent = 100.0.
// Then overridden by -N and/or by -B. The last one on the command line wins.
// Then trimmed if bSplayKeyBits and there aren't enough bits set in wSplayMask.
// MaxNumb is used to put an upper bound on RandomNumb values used in CalcNextKey.
// And to specifiy -b and -y array sizes.
// It's meaning is more confusing for GValue != 0.
Word_t MaxNumb = ((Word_t)1 << (DEFAULT_BVALUE-1)) * 2 - 1;
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

// Swizzle flag == 1 >> I.E. bit reverse (mirror) the data
//
Word_t    DFlag = 0;                    // bit reverse (mirror) the data stream

// Default starting seed value; -s
//
Word_t    StartSequent = 1;

// Global to store the current Value return from PSeed.
//
//Word_t    Key = 0xc1fc;

Word_t PartitionDeltaFlag = 1; // use -Z to disable small key array

#ifndef CALC_NEXT_KEY
Word_t TrimKeyArrayFlag = 1;
// The initial value of wPrevLogPop1 determines when we transition from
// sequential test keys to random test keys for -DS1.
// Use wPrevLogPop1 = 64 to use sequential test keys throughout.
int wPrevLogPop1 = 0; // wPrevLogPop1 is used only for -DS1.
#endif // CALC_NEXT_KEY

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

// returns next Key, depending on SValue, DFlag and GValue.
//
static inline Word_t
CalcNextKey(PSeed_t PSeed)
{
    Word_t Key;
#ifndef NO_FVALUE
    if (FValue)
    {
        Key = FileKeys[PSeed->Seeds[0]++];
    }
    else
#endif // NO_FVALUE
    {
#ifndef NO_TRIM_EXPANSE
        do
        {
#endif // NO_TRIM_EXPANSE
            Key = RandomNumb(PSeed, SValue);
            if ((sizeof(Word_t) * 8) != BValue)
            {
                 assert((Key < ((Word_t)1 << BValue)) || SValue);
            }
#ifndef NO_TRIM_EXPANSE
        } while (Key > MaxNumb);         // throw away of high keys
#endif // NO_TRIM_EXPANSE
    }

#ifdef DEBUG
    if (pFlag)
    {
        printf("Numb %016" PRIxPTR" ", Key);
    }
#endif // DEBUG

#ifndef NO_SPLAY_KEY_BITS
    if (bSplayKeyBitsFlag) {
        // Splay the bits in the key.
        // This is not subject to BValue.
        Key = MyPDEP(Key, wSplayMask);
#ifdef DEBUG
        if (pFlag)
        {
            printf("PDEP %016" PRIxPTR" ", Key);
        }
#endif // DEBUG
        // Key might be bigger than 1 << BValue -- by design.
    }
#endif // NO_SPLAY_KEY_BITS

#define LOG(_x) ((Word_t)63 - __builtin_clzll(_x))

#ifndef NO_DFLAG
    if (DFlag)
    {
        Key = Swizzle(Key); // Reverse order of bits.
#ifdef DEBUG
        if (pFlag)
        {
            printf("BitReverse %016" PRIxPTR" ", Key);
        }
#endif // DEBUG
//      move the mirror bits into the least bits determined -B# and -E
        if (bSplayKeyBitsFlag)
        {
            Key >>= (sizeof(Word_t) * 8) - 1
                - LOG(MyPDEP((((Word_t)1 << (BValue - 1)) * 2) - 1, wSplayMask));
        }
        else
        {
            Key >>= (sizeof(Word_t) * 8) - BValue;
        }
    }
#endif // NO_DFLAG

#ifndef NO_OFFSET
    Key = Key + Offset;
#endif // NO_OFFSET
#ifdef DEBUG
    if (pFlag) { printf("Key "); }
#endif // DEBUG
    return Key;
}

// GetNextKeyX has a bLfsrOnlyArg parameter so it can be called with a literal
// and the test will be compiled out of the test loop.
// It has a wFeedBTapArg parameter so the caller can use a local variable if
// that is faster.
#ifdef LFSR_GET_FOR_DS1
// We've hacked the code to overload the bLfsrOnlyArg parameter in case we
// want -k to cause us to use the fast lfsr in TestJudyGet for -DS1.
#endif // LFSR_GET_FOR_DS1
static inline Word_t
GetNextKeyX(PNewSeed_t PNewSeed, Word_t wFeedBTapArg, int bLfsrOnlyArg)
{
#ifdef CALC_NEXT_KEY
    (void)wFeedBTapArg; (void)bLfsrOnlyArg;
    // [P]NewSeed_t is the same as [P]Seed_t.
    return CalcNextKey(PNewSeed);
#else // CALC_NEXT_KEY
    if (bLfsrOnlyArg) {
        // PNewSeed is a pointer to a word with the next key value in it.
        Word_t wKey = (Word_t)*PNewSeed;
        *PNewSeed = (NewSeed_t)((wKey >> krshift) ^ (wFeedBTapArg & -(wKey & 1)));
#ifdef LFSR_GET_FOR_DS1
        wKey <<= bLfsrOnlyArg - 1; // for -DS1
#endif // LFSR_GET_FOR_DS1
        return wKey;
    } else {
        // PNewSeed is a pointer to a pointer into the key array.
        return *(*PNewSeed)++;
    }
#endif // CALC_NEXT_KEY
}

// GetNextKey exists for compatibility with old code we haven't updated yet.
// It uses the globals wFeedBTap and bLfsrOnly at runtime.
static inline Word_t
GetNextKey(PNewSeed_t PNewSeed)
{
    return GetNextKeyX(PNewSeed, wFeedBTap /*global*/, bLfsrOnly /*global*/);
}

static void
PrintHeader(const char *strFirstCol)
{
  //printf("# %s ............   ......  GetMeasmts", strFirstCol);
    printf("# %s . GetMeasmts", strFirstCol);

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

#if defined(MIKEY)
        printf(" REQ/K"); // words requested per key
#else // defined(MIKEY)
        printf("  L7/K");
#endif // defined(MIKEY)
        printf("  L6/K");
        printf("  L5/K");
        printf("  L4/K");
#if defined(MIKEY)
        printf("  B2/K"); // big bitmap leaf words per key
#else // defined(MIKEY)
        printf("  L3/K");
#endif // defined(MIKEY)
        printf("  L2/K");
        printf("  L1/K");
        printf("  B1/K");
        printf("  JV/K");

        printf(" MsCmp");
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

    printf("\n<<< Program to do performance measurements (and Diagnostics) on Judy Arrays >>>\n\n");
    printf("%s -n# -P# -S# [-B#[:#]|-N#] -T# -G# -1LH -bIcCvdtDlMK... -F <filename>\n\n", argv[0]);
    printf("   Where: # is a number, default is shown as [#]\n\n");
    printf("-X #  Scale numbers produced by '-m' flag (for plotting) [%d]\n", XScale);
    printf("-m #  Output measurements when libJudy is compiled with -DRAMMETRICS &| -DSEARCHMETRICS\n");
    printf("-n #  Number of Keys (Population) used in Measurement [10000000]\n");
    printf("-P #  Measurement points per decade (1..1000) [50] (231 = ~1%%, 46 = ~5%% delta Population)\n");
    printf("-S #  Key Generator skip amount, 0 = Random [0]\n");
    printf("-B #  Significant bits output (16..64) in Random Key Generator [32]\n");
    printf("-B #:#  Second # is percent expanse is limited [100]\n");
    printf("-N #  max key #; alternative to -B\n");
    printf("-E,--splay-key-bits [splay-mask]    Splay key bits\n");
    printf("-G #  Type (0..4) of random numbers 0 = flat spectrum, 1..4 = Gaussian [0]\n");
    printf("-l    Do not smooth data with iteration at low (<100) populations (Del/Unset not called)\n");
    printf("-F <filename>  Ascii file of Keys, zeros ignored -- must be last option!!!\n");
//    printf("-b #:#:# ... 1st number required [1] where each number is next level of tree\n");
    printf("-b    Time a BITMAP array  (-B # specifies size == 2^# bits) [-B32]\n");
    printf("-y    Time a BYTEMAP array (-B # specifies size == 2^# bytes) [-B32]\n");
    printf("-1    Time Judy1\n");
    printf("-L    Time JudyL\n");
    printf("-R    Time JudyL using *PValue as next TstKey\n");
    printf("-H    Time JudyHS\n");
    printf("-I    Time DUPLICATE (already in array) JudyIns/Set times\n");
    printf("-c    Time copying a Judy1 Array\n");
    printf("-C    Include timing of Judy[1L]Count tests\n");
    printf("-v    Include timing of Judy[1L]First/Last/Next/Prev tests\n");
    printf("-d    Include timing of JudyLDel/Judy1Unset\n");
    printf("-V    Turn OFF JudyLGet() verification tests (saving a cache-hit on 'Value')\n");
    printf("-x    Turn ON 'waiting for context switch' flag -- smoother plots??\n");
    printf("-t    Print measured 'overhead' (nS) that was subtracted out (Key Generator, etc)\n");
    printf("-D    'Mirror' Keys from Generator (as in binary viewed with a mirror)\n");
    printf("-T #  Number of Keys to measure JudyGet/Test times, 0 == MAX [1000000]\n");

    printf("\n   The following are primarly used for diagnosis and debugging\n");
    printf("-K    do a __sync_synchronize() in GetNextKey() (is mfence instruction in X86)\n");
    printf("-k,--lfsr-only    use fast lfsr key gen and ignore -DEFGNOoS and -B with colon\n");
    printf("-h    Skip 1/64th of the generated keys in Insert/Set, but not in Get/Test\n");
    printf("-s #  Starting number in Key Generator [0x%" PRIxPTR"]\n", StartSequent);
    printf("-o #, --Offset #     Key += #  additional add to Key\n");
    printf("-O #, --BigOffset #  Key += (# << (-B #))  additional add to Key above MSb (high) bits\n");
    printf("-p     Print number set used for testing  - takes presedence over other options\n");
    printf("-g     Do a JudyLGet/Judy1Test after every Ins/Set/Del/Unset (adds to times)\n");
    printf("-i     Do a JudyLIns/Judy1Set after every Ins/Set (adds to times)\n");
    printf("-M     Print on stderr Judy_mmap() and Judy_unmap() calls to kernel\n");
    printf("-h     Put 'Key holes' in the Insert path 1 and L options only\n");
    printf("-W #   Specify the 'CPU Warmup' time in milliseconds [%" PRIuPTR"]\n", Warmup);
    printf("-Z     Do not split insert delta into parts (yields large key array size)\n");

    printf("\n");

    exit(1);
}
#undef __FUNCTI0N__
#define __FUNCTI0N__ "main"

static int
BitmapGet(PWord_t B1, Word_t TstKey)
{
// Original -b did not have this check for NULL.
#if 0
    if (B1 == NULL)
    {
        return (0);
    }
#endif

#if defined(LOOKUP_NO_BITMAP_DEREF)

    (void)B1; (void)TstKey;

    return 1;

#else // defined(LOOKUP_NO_BITMAP_DEREF)

#if defined(BITMAP_BY_BYTE)
    unsigned char *p = (unsigned char *)B1;
#else // defined(BITMAP_BY_BYTE)
    Word_t        *p = B1;
#endif // defined(BITMAP_BY_BYTE)

    Word_t offset = TstKey / (sizeof(*p) * 8);
    unsigned bitnum = TstKey % (sizeof(*p) * 8);

    return (p[offset] & ((Word_t)1 << bitnum)) != 0;

#endif // defined(LOOKUP_NO_BITMAP_DEREF)

}

// Note that the prototype for BitmapSet is different from that of Judy1Set.
static int
BitmapSet(PWord_t B1, Word_t TstKey)
{
    Word_t    offset;
    Word_t    bitmap;
    int       bitnum;

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
    { "Offset", required_argument, NULL, 'o' },

    // Long option "--BigOffset=<#>" is equivalent to short option '-O<#>".
    { "BigOffset", required_argument, NULL, 'O' },

    // Long option '--splay-key-bits' is equivalent to short option '-E'.
    { "splay-key-bits", required_argument, NULL, 'E' },

    // Long option '--lfsr-only' is equivalent to short option '-k'.
    { "lfsr-only", no_argument, NULL, 'k' },

    // Last struct option in array must be filled with zeros.
    { NULL, 0, NULL, 0 }
};

#ifndef OLD_DS1_GROUPS

#define EXP(x) ((Word_t)1<<(x))

static Word_t Log16(Word_t n) { return LOG(n) / 4; }
static Word_t Pow16(Word_t n) { return EXP(4 * n); }

static Word_t
FinalGrpSz(Word_t nElms)
{
    // 17     groups of 16^0 up to 16^0 + 16^0.
    // 16^2-1 groups of 16^0 up to 16^2 + 16^1.
    // 16^2-1 groups of 16^1 up to 16^3 + 16^2.
    // 16^2-1 groups of 16^2 up to 16^4 + 16^3.
    // 16^2-1 groups of 16^3 up to 16^5 + 16^4.
    return Pow16(Log16(nElms - Pow16(Log16(nElms) - 1) - 1) - 1);
}

// Number of groups before the first group with the final group size.
static Word_t
GrpsBeforeFinalGrpSz(Word_t wFinalGrpSz)
{
    return 17 + 255 * Log16(wFinalGrpSz);
}

// Number of elements through last group before the final group size.
static Word_t
ElmsBeforeFinalGrpSz(Word_t wFinalGrpSz)
{
    return 272 * wFinalGrpSz / 16;
}

#define DIV_UP(dividend, divisor)  (((dividend) + (divisor) - 1) / (divisor))

// Number of groups.
static Word_t
//Groups(Word_t wGrpsBeforeFinalGrpSz, Word_t wElmsBeforeFinalGrpSz)
NumGrps(Word_t wElms)
{
    Word_t wFinalGrpSz = FinalGrpSz(wElms);
    return DIV_UP(wElms - ElmsBeforeFinalGrpSz(wFinalGrpSz), wFinalGrpSz)
        + GrpsBeforeFinalGrpSz(wFinalGrpSz);
}

#endif // OLD_DS1_GROUPS

int
main(int argc, char *argv[])
{
//  Names of Judy Arrays
#ifdef DEBUG
    // Make sure the word before and after J1's root word is zero. It's
    // pretty easy in some variants of Mikey's code to introduce a bug that
    // clobbers one or the other so his code depends on these words being
    // zero so it can verify that neither is getting clobbered.
    struct { void *pv0, *pv1, *pv2; } sj1 = { 0, 0, 0 };
#define J1 (sj1.pv1)
    struct { void *pv0, *pv1, *pv2; } sjL = { 0, 0, 0 };
#define JL (sjL.pv1)
#else // DEBUG
    void     *J1 = NULL;                // Judy1
    void     *JL = NULL;                // JudyL
#endif // DEBUG
    void     *JH = NULL;                // JudyHS

#ifdef DEADCODE                         // see TimeNumberGen()
    void     *TestRan = NULL;           // Test Random generator
#endif // DEADCODE

    Word_t    Count1 = 0;
    Word_t    CountL = 0;
    Word_t    Bytes;

    Pms_t     Pms;
    NewSeed_t    InsertSeed;               // for Judy testing (random)
    NewSeed_t    StartSeed;
    NewSeed_t    BitmapSeed;
    NewSeed_t    BeginSeed;
    NewSeed_t    DummySeed;
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

    int       Col;
    int       c;
    Word_t    ii;                       // temp iterator
    extern char *optarg;

#ifdef LATER
    double    Dmin = 9999999999.0;
    double    Dmax = 0.0;
    double    Davg = 0.0;
#endif // LATER

// MaxNumb is initialized from statically initialized BValue and Bpercent.
// Then overridden by -N and/or by -B. The last one on the command line wins.
// Then trimmed if bSplayKeyBits and there aren't enough bits set in wSplayMask.
    MaxNumb = pow(2.0, BValue) * Bpercent / 100;
    --MaxNumb;

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
    printf("Count of over 1uS = %" PRIuPTR"\n", CountL);

    exit(1);
#endif // LATER

// ============================================================
// PARSE INPUT PARAMETERS
// ============================================================

    errno = 0;
    while (1)
    {
        c = getopt_long(argc, argv,
                   "a:n:S:T:P:s:B:G:X:W:o:O:F:b"
#ifdef FANCY_b_flag
                                              ":"
#endif // FANCY_b_flag
                                              "N:dDcC1LHvIltmpxVfgiyRMKkhEZ",
                // Optstring sorted:
                // "1a:B:bCcDdEF:fG:gHhIiKklLMmN:n:O:o:P:pRS:s:T:tVvW:XXxy",
                // Used option characters sorted and with spaces for unused option characters:
                // " 1         a:B:bCcDdE F:fG:gHhIi  KkLlMmN:n:O:o:P:p  R S:s:T:t:  VvW: Xx yZ "
                // Unused option characters sorted and with spaces for used option characters:
                // "0 23456789A          e          Jj                 Qq r        Uu    w  Y  z"
                   longopts, NULL);
        if (c == -1)
            break;

        switch (c)
        {
        case 'Z':
            PartitionDeltaFlag = 0;
            break;
        case 'E':
            bSplayKeyBitsFlag = 1;
            if (optarg != NULL) {
                wSplayMask = oa2w(optarg, NULL, 0, c);
            }
#ifdef NO_SPLAY_KEY_BITS
            FAILURE("compile with -UNO_SPLAY_KEY_BITS to use -E", wSplayMask);
#endif // NO_SPLAY_KEY_BITS
            break;
        case 'a':                      // Max population of arrays
            PreStack = oa2w(optarg, NULL, 0, c);   // Size of PreStack
            break;

        case 'n':                      // Max population of arrays
            nElms = oa2w(optarg, NULL, 0, c);   // Size of Linear Array
            if (nElms == 0)
                FAILURE("Error --- No tests: -n", nElms);
            break;

        case 'N':       // max key number; alternative to -B
        {
            MaxNumb = oa2w(optarg, NULL, 0, c);
            if (MaxNumb == 0)
                FAILURE("Error --- No tests: -N", MaxNumb);

            BValue = (sizeof(Word_t) * 8) - __builtin_clzl(MaxNumb);
            Bpercent = (MaxNumb/2.0 + 0.5) / pow(2.0, BValue - 1) * 100;

            break;
        }
        case 'S':                      // Step Size, 0 == Random
        {
            SValue = oa2w(optarg, NULL, 0, c);
#ifdef NO_SVALUE
            if (SValue != 0)
            {
                FAILURE("compile with -UNO_SVALUE to use -S", SValue);
            }
#endif // NO_SVALUE
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
        {
            char *str, *tok;
            char *saveptr = NULL;

//          parse the sub parameters of the -b option
            str = optarg;
            tok = strtok_r(str, ":", &saveptr);
//            if (tok == NULL) cant happen, caught by getopt
            str = NULL; // for subsequent call to strtok_r

            BValue = oa2w(tok, NULL, 0, c);

            tok = strtok_r(str, ":", &saveptr);
            if (tok != NULL) {
                Bpercent = atof(tok); // default is Bpercent = 100
            }

            if (Bpercent == 50.0) {
                --BValue;
                Bpercent = 100.0;
            }

            if ((BValue > sizeof(Word_t) * 8) || (BValue < 10))
            {
                FAILURE("\n -B  is out of range, I.E. -B", BValue);
            }

            if (Bpercent < 50.0 || Bpercent > 100.0)
            {
                ErrorFlag++;
                printf("\nError --- Percent = %4.2f must be greater than 50 and less than 100 !!!\n", Bpercent);
            }

            MaxNumb = pow(2.0, BValue) * Bpercent / 100;
            MaxNumb--;

            break;
        }
        case 'G':                      // Gaussian Random numbers
            GValue = oa2w(optarg, NULL, 0, c);  // 0..4 -- check by RandomInit()
            if (GValue != 0)
            {
#ifndef GAUSS
                FAILURE("compile with -UNO_GAUSS to use -G", GValue);
#elif !defined(NEXT_FIRST)
                printf("\n# Warning -- '-G %zd' behaves like -DNEXT_FIRST\n", GValue);
#endif // GAUSS
            }
            break;

        case 'X':                      // Scale numbers under the '-m' flag
            XScale = strtol(optarg, NULL, 0);   // 1..1000
            if (XScale < 1)
            {
                ErrorFlag++;
                printf("\nError --- option -X%d must be greater than 0 !!!\n", XScale);
            }
            break;

        case 'W':                      // Warm up CPU number of random() calls
            Warmup = oa2w(optarg, NULL, 0, c);

            break;

        case 'o': // Add <#> to generated keys, aka --LittleOffset=<#>.
            LittleOffset = oa2w(optarg, NULL, 0, c);
            break;

        case 'O': // Add <#> << B# to generated keys, aka --BigOffset=<#>.
            BigOffset = oa2w(optarg, NULL, 0, c);
            break;

        case 'b':                      // Turn on REAL bitmap testing
        {
#ifdef FANCY_b_FLAG
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
                bParm[ii] = atoi(tok);
                sum += bParm[ii];

//              Limit maximum of decode in branches
                if (sum > cMaxSum)
                {
                    for (jj = 0; jj < ii; jj++)
                        printf("\nError: option -b%d:", bParm[jj]);

                    printf("\nError: Maximum sum of subparamters is %d\n", cMaxSum);
                    break;
                }
                if (bParm[ii] > 16)
                {
                    printf("\nError: Maximum Value of :%d is 16\n", bParm[jj]);
                    ErrorFlag++;
                    break;
                }
                ii++;
                if (ii == cMaxColon)
                {
                    for (jj = 0; jj < ii; jj++)
                        printf("\nError: option -b%d:", bParm[jj]);

                    printf(" - has a maximum of %d subprameters\n", cMaxColon);
                    ErrorFlag++;
                    break;
                }
            }
            if (sum == 0)
            {
                printf("Error: -b option needs at least one number -b 1..%d\n", (int)BValue - cLg2WS);
                ErrorFlag++;
            }

            }
#endif // FANCY_b_FLAG
            bFlag = 1;
            break;
        }
        case 'h':                      // Skip # Keys (holes) in the Insert Code
        {
            hFlag = 1;
            break;
        }
        case 'F':                      // Read Keys from file
        {
#ifdef NO_FVALUE
            FAILURE("compile with -UNO_FVALUE to use '-F'", optarg);
#endif // NO_FVALUE
            if (FValue) {
                FAILURE("only one '-F' allowed", -1);
            }
            FILE *Pfile;
            char Buffer[BUFSIZ];
            Word_t  KeyValue;
            Word_t  One100M;            // 100 million

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
            fprintf(stderr, "\n# Reading \"%s\" Key file ", keyfile);
            One100M = 0;
            while (fgets(Buffer, BUFSIZ, Pfile) != (char *)NULL)
            {
                if ((FValue % 1000000) == 0)
                {
                    if ((FValue % 100000000) == 0)
                    {
                        fprintf(stderr, "%" PRIuPTR"", One100M++);
                    }
                    else
                    {
                        fprintf(stderr, ".");
                    }
                }
                if (strtoul(Buffer, NULL, 0) != 0)
                    FValue++;
            }
            fprintf(stderr, "\n# Number of Keys = %" PRIuPTR"\n", FValue);
            if ((FileKeys = (PWord_t)malloc((FValue + 1) * sizeof(Word_t))) == 0)
            {
                FAILURE("malloc failure, Bytes =", (FValue + 1) * sizeof(Word_t));
            }
            fclose(Pfile);
            if ((Pfile = fopen(keyfile, "r")) == (FILE *) NULL)
            {
                printf("\nError: Illegal argument to \"-F %s\" -- errno = %d\n", keyfile, errno);
                printf("Error: Cannot open file \"%s\" to read it\n", keyfile);
                ErrorFlag++;
                exit(1);
            }
            fprintf(stderr, "# Re-Reading Key file ");
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
            FileKeys[FValue] = 0; // Set extra key for -R/TestJudyLIns to 0.
            fprintf(stderr, "\n");
            nElms = FValue;
            StartSequent = 0;
            PartitionDeltaFlag = 0;
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
#ifndef NO_TEST_NEXT // for turn-on testing
            vFlag = 1;                 // time Searching
#endif // NO_TEST_NEXT
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
#ifdef NO_DFLAG
            FAILURE("compile with -UNO_DFLAG to use -D", -1);
#endif // NO_DFLAG
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
#ifndef DO_TIT
            printf("\n# Warning -- compile with -DDO_TIT to use '-t'.\n");
#endif // DO_TIT
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
#ifndef KFLAG
            printf("\n# Warning -- '-K' ignored"
                   " because compiled with -DNO_KFLAG.\n");
#endif // KFLAG
            KFlag = 1;
            break;

        case 'k':                      // Use fast lfsr with no options.
            bLfsrOnly = 1;
            break;

        default:
            ErrorFlag++;
            break;
        }
    }

#ifdef NO_TRIM_EXPANSE
    Word_t MaxNumbP1 = MaxNumb + 1;
    if ((MaxNumbP1 & -MaxNumbP1) != MaxNumbP1)
    {
        FAILURE("compile with -UNO_TRIM_EXPANSE to use '-N'", MaxNumb);
    }
#endif // NO_TRIM_EXPANSE

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

    if (hFlag && vFlag)
    {
        printf("\nError --- '-v' and '-h' are incompatible\n");
        fprintf(stderr, "\nError --- '-v' and '-h' are incompatible\n");
        ErrorFlag++;
    }

    if (bSplayKeyBitsFlag)
    {
        if (wSplayMask == 0) {
            printf("\nError --- <splay-mask> cannot be zero.\n");
            ErrorFlag++;
        }
        int nBitsSet = __builtin_popcountll(wSplayMask);
        if (nBitsSet < (int)BValue)
        {
            BValue = nBitsSet;
            MaxNumb = ((Word_t)2 << (BValue - 1)) - 1;
            printf("\n# Warning -- trimming '-B' value to %d;"
                   " the number of bits set in <splay-mask> 0x%zx.\n",
                   (int)BValue, wSplayMask);
        }

        if (gFlag)
        {
#if defined(SPLAY_KEY_BITS) && defined(DFLAG)
            // Calculate wCheckBit for use with gFlag later.
            Word_t wEffMask = wSplayMask;
            if (DFlag) {
                wEffMask = Swizzle(wSplayMask);
                wEffMask >>= (sizeof(Word_t) * 8) - LOG(wSplayMask) - 1;
            }
#else // defined(SPLAY_KEY_BITS) && defined(DFLAG)
            Word_t wEffMask = -1;
#endif // defined(SPLAY_KEY_BITS) && defined(DFLAG)
            if (wEffMask != (Word_t)-1)
            {
                for (int ii = 2; ii < (int)sizeof(Word_t) - 1; ++ii)
                {
                    if (((uint8_t *)&wEffMask)[ii] == 0)
                    {
                        wCheckBit = (Word_t)1 << (ii * 8);
                        break;
                    }
                }
                if (wCheckBit == 0)
                {
                    if (LOG(wEffMask) != sizeof(Word_t) * 8 - 1) {
                        wCheckBit = (Word_t)2 << LOG(wEffMask);
                    } else {
                        wCheckBit = (Word_t)1 << LOG(~wEffMask);
                    }
                }
            }
            printf("# -g wCheckBit %p\n", (void *)wCheckBit);
        }
    }

//  BValue already check to be <= 64 or <=32 and >=16
    if (BigOffset)
    {
        Word_t bigoffset = BigOffset;

        if (BValue >= (sizeof(Word_t) * 8))
            BigOffset = 0;

        BigOffset <<= BValue;           // offset past BValue

        if (BigOffset == 0)
        {
            printf("\n# Warning -- '-O 0x%" PRIxPTR"' ignored, because '-B %" PRIuPTR"' option too big\n", bigoffset, BValue);
            fprintf(stderr, "\n# Warning -- '-O 0x%" PRIxPTR"' ignored, because '-B %" PRIuPTR"' option too big\n", bigoffset, BValue);
        }
    }
    Offset = BigOffset + LittleOffset;  // why not?
#ifdef NO_OFFSET
    if (Offset != 0)
    {
        FAILURE("compile with -UNO_OFFSET to use -O or -o", Offset);
    }
#endif // NO_OFFSET

    if (bLfsrOnly) {
        if (DFlag || FValue || GValue || Offset || SValue
            || (Bpercent != 100.0))
        {
            if (DFlag && (SValue == 1) && (StartSequent == 1)
                && !bSplayKeyBitsFlag && (Offset == 0) && (Bpercent == 100.0))
            {
#ifdef LFSR_GET_FOR_DS1
                bLfsrForGetOnly = 1;
                bLfsrOnly = 0;
#else // LFSR_GET_FOR_DS1
                FAILURE("Use -DLFSR_GET_FOR_DS1 to use -k with -DS1.", 0);
#endif // LFSR_GET_FOR_DS1
            }
            else
            {
                FAILURE("-k is not compatible with -B:DFGNOoS", 0);
            }
        }
        if (bSplayKeyBitsFlag) {
            if (wSplayMask !=
#if defined(__LP64__) || defined(_WIN64)
                0x5555555555555555
#else // defined(__LP64__) || defined(_WIN64)
                0x55555555
#endif // defined(__LP64__) || defined(_WIN64)
                )
            {
                FAILURE("-k --splay-key-bits requires mask 0x55...55 not ",
                        wSplayMask);
            }
        }
    }

//  Check if starting number is ok
    if (FValue == 0)
    {
        if (StartSequent > MaxNumb)
        {
            printf("\n# Trimming '-s 0x%zx' to 0x%zx.\n", StartSequent, MaxNumb);
            StartSequent = MaxNumb;
            //ErrorFlag++;
        }
        if (StartSequent == 0 && (SValue == 0))
        {
            printf("\nError --- '-s 0' option Illegal if Random\n");
            ErrorFlag++;
        }
    }

    if (ErrorFlag)
        Usage(argc, argv);

    {
        if (nElms > MaxNumb)
        {
            nElms = MaxNumb;

            printf("# Trim Max number of Elements -n%" PRIuPTR" due to max -B%" PRIuPTR" bit Keys",
                   MaxNumb, BValue);
            fprintf(stderr, "# Trim Max number of Elements -n%" PRIuPTR" due to max -B%" PRIuPTR" bit Keys",
                   MaxNumb, BValue);

            if (Offset)
            {
                printf(", add %" PRIdPTR" (0x%" PRIxPTR") to Key values", Offset, Offset);
                fprintf(stderr,", add %" PRIdPTR" (0x%" PRIxPTR") to Key values", Offset, Offset);
            }
            printf("\n");
            fprintf(stderr, "\n");

        }
        if (GValue != 0) // This needs work and review!!!!!
        {
// MEB: I have no idea what's going on here.
            if (nElms > (MaxNumb >> 1))
            {
                printf
                    ("# Trim Max number of Elements -n%" PRIuPTR" to -n%" PRIuPTR" due to -G%" PRIuPTR" spectrum of Keys\n",
                     MaxNumb, MaxNumb >> 1, GValue);
                nElms = MaxNumb >> 1;
            }
        }
    }

//  build the Random Number Generator starting seeds
    PSeed_t PInitSeed = RandomInit(BValue, GValue);

    if (PInitSeed == NULL)
    {
        printf("\nIllegal Number in -G%" PRIuPTR" !!!\n", GValue);
        ErrorFlag++;
    }

//  Set the starting number in number Generator
    PInitSeed->Seeds[0] = StartSequent;

    if (bLfsrOnly) {
        wFeedBTap = PInitSeed->FeedBTap;
        if (bSplayKeyBitsFlag) {
            krshift = 2;
            wFeedBTap = MyPDEP(wFeedBTap, wSplayMask);
            StartSequent = MyPDEP(StartSequent, wSplayMask);
        }
        //printf("# wFeedBTap 0x%zx\n", wFeedBTap);
    }

//  Print out the number set used for testing
    if (pFlag)
    {
        Word_t ii;
// 1      Word_t PrevPrintKey = 0;

//printf("PInitSeed->Seeds[0] 0x%zx\n", PInitSeed->Seeds[0]);
#if !defined(CALC_NEXT_KEY)
        StartSeed = (NewSeed_t)StartSequent;
#endif // !defined(CALC_NEXT_KEY)
        for (ii = 0; ii < nElms; ii++)
        {
            Word_t PrintKey;
// 1          Word_t LeftShift;
// 1          Word_t RightShift;

            PrintKey = CalcNextKey(PInitSeed);
#if !defined(CALC_NEXT_KEY)
            if (bLfsrOnly) {
                Word_t wKey = GetNextKey(&StartSeed); (void)wKey;
                assert(wKey == PrintKey);
            }
#endif // !defined(CALC_NEXT_KEY)

// 1          LeftShift  = __builtin_popcountll(PrintKey ^ (PrevPrintKey << 1));
// 1          RightShift = __builtin_popcountll(PrintKey ^ (PrevPrintKey >> 1));
// 1          PrevPrintKey = PrintKey;

//            printf("%" PRIxPTR"\n", PrintKey);

            printf("0x%016" PRIxPTR" log2+1 %2d %" PRIuPTR"\n",
                   PrintKey, (int)log2((double)(PrintKey)) + 1, PrintKey);

#ifdef __LP64__
//            printf("0x%016lx\n", PrintKey);
// 1          printf("0x%016lx %" PRIuPTR" %" PRIuPTR", %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#else   // ! __LP64__
 //           printf("0x%08lx\n", PrintKey);
// 1          printf("0x%08lx %" PRIuPTR" %" PRIuPTR", %4lu, %4lu\n", PrintKey, ii, BValue, LeftShift, RightShift);
#endif  // ! __LP64__

        }
        exit(0);
    }

//  print Title for plotting -- command + run arguments
//

    printf("# TITLE %s -W%" PRIuPTR"", argv[0], Warmup);

    printf(" -");

    if (bFlag)
        printf("b");
    if (yFlag)
        printf("y");
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
    if (PartitionDeltaFlag)
        printf("Z");

    if (Bpercent == 100.0) {
         printf(" -B%" PRIuPTR, BValue);
    } else {
         printf(" -N0x%" PRIxPTR, MaxNumb);
    }

    if (DFlag || SValue) {
        printf(" -");
        if (DFlag) printf("D");
        if (SValue)
            printf("S%" PRIuPTR" -s%" PRIuPTR"", SValue, StartSequent);
    }

    if (bSplayKeyBitsFlag) {
        printf(" --splay-key-bits=0x%" PRIxPTR, wSplayMask);
    }

//  print more options - default, adjusted or otherwise
    printf(" -n%" PRIuPTR" -T%" PRIuPTR" -P%" PRIuPTR" -X%d", nElms, TValues, PtsPdec, XScale);
    if (bFlag && (bParm[0] != 0))
    {
        int ii;
        printf(" -b");
        for (ii = 0; bParm[ii]; ii++)
        {
            if (ii) printf(":");
            printf("%d", bParm[ii]);
        }
    }

    if (FValue)
        printf(" -F %s", keyfile);

    if (Offset)
        printf(" -o 0x%" PRIxPTR"", Offset);

    printf(" -G%" PRIuPTR, GValue);

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
                (" Sorry, '-m' measurements compatable with exactly ONE of -1LRHby.\n");
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

    if (Bpercent != 100.0)
        printf("# MaxNumb of Random Number generator was trimed to 0x%" PRIxPTR" (%" PRIuPTR")\n", MaxNumb, MaxNumb);

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

//    Debug
    printf("# nElms (number of keys to be inserted) = %" PRIuPTR"[0x%" PRIxPTR"]\n", nElms, nElms);
    printf("# MaxNumb (maximum key in expanse) = %" PRIuPTR"[0x%" PRIxPTR"]\n", MaxNumb, MaxNumb);
    printf("# BValue = %" PRIuPTR"\n", BValue);
    printf("# Bpercent = %20.18f\n", Bpercent);


    printf("# XLABEL Array Population\n");
    printf("# YLABEL Nano-Seconds -or- Words per %d Key(s)\n", XScale);

    // Simplify subsequent handling of TValues by getting rid of special cases.
    if ((TValues == 0) || (TValues > nElms))
    {
        TValues = nElms;
    }

// ============================================================
// DETERMINE THE NUMBER OF MEASUREMENT GROUPS AND THEIR SIZES
// ============================================================

#ifndef CALC_NEXT_KEY
    Word_t wMaxEndDeltaKeys = 0; // key array size
#endif // CALC_NEXT_KEY

    // Use power of two group sizes for -DS1.
    // Does StartSequent matter?
    if (DFlag && (SValue == 1) && (Bpercent == 100.0))
    {
        // First splay is at insert of n[8]+1'th key,
        // where n[8] is max length of list 8.
        // There will be one splay.
        // Second splay is at insert of 256 * n[7]+1'th key,
        // where n[7] is max length of list 7.
        // It doesn't matter what n[8] is - there will be 256 splays.
        // Third wave of splays starts at insert of 64K * n[6]+1'th key,
        // where n[6] is max length of list 6.
        // It doesn't matter what n[8] or n[7] are - there will be 64K splays.
        // Fourth wave of splays starts at insert of 16M * n[5]+1'th key,
        // where n[5] is max length of list 5.
        // It doesn't matter what n[8] or n[7] or n[6] are - there will be
        // 16M splays.
        if (nElms <= 256) {
            Groups = nElms;
        } else {
#ifdef OLD_DS1_GROUPS
            // Old logGrpSz (switch to bigger groups after 16^n):
            Word_t logGrpSz = LOG(nElms-1)/4; // log base 16
            // Newer logGrpSz (switch to bigger groups sooner - after 16^n - 16^(n-1)):
            //Word_t logGrpSz = LOG(nElms)/4; // log base 16

            Word_t grpSz = (Word_t)1 << (logGrpSz-1) * 4; // final group size

            // Old Groups (switch to bigger groups after 16^n):
            Groups = 256 + (logGrpSz-2)*240 + (nElms - grpSz*15 - 1) / grpSz;
            // Newer Groups (switch to bigger groups sooner -- after 16^n - 16^(n-1)):
            //Groups = 255 + (logGrpSz-2)*240 + (nElms - grpSz*15) / grpSz;
#else // OLD_DS1_GROUPS
            // Newest Groups (switch to bigger groups latest - after 16^n + 16^(n-1)):
            Groups = NumGrps(nElms);
#endif // OLD_DS1_GROUPS
        }

        printf("#  Groups    0x%04zx == 0d%05zd\n", Groups, Groups);

        // Get memory for saving measurements
        Pms = (Pms_t) malloc(Groups * sizeof(ms_t));

        // Calculate number of Keys for each measurement point
#ifdef OLD_DS1_GROUPS
        // Old code (switch to bigger groups after 2^n):
        for (grp = 0; (grp < 256) && (grp < Groups); grp++)
        // New code (switch to bigger groups after 2^n-2^(n-4)):
        //for (grp = 0; (grp < 255) && (grp < Groups); grp++)
        {
            Pms[grp].ms_delta = 1;
        }
        Word_t wPrev;
        for (Word_t wNumb = grp; grp < Groups; ++grp) {
            wPrev = wNumb;
            // Old code (switch to bigger groups after 2^n):
            wNumb += Pms[grp].ms_delta = (Word_t)1 << (LOG(wNumb)/4 - 1) * 4;
            // New code (switch to bigger groups after 2^n -2^(n-4)):
            //wNumb += Pms[grp].ms_delta = (Word_t)1 << (LOG(wNumb+1)/4 - 1) * 4;
            if ((wNumb > nElms) || (wNumb < wPrev)) {
                wNumb = nElms;
                Pms[grp].ms_delta = wNumb - wPrev;
            }
            //printf("# wNumb 0x%04zx %zd\n", wNumb, wNumb);
        }
#else // OLD_DS1_GROUPS
        Word_t grp;
        for (grp = 0; (grp < 272) && (grp < Groups); grp++) {
            Pms[grp].ms_delta = 1;
        }
        Word_t wPrev;
        for (Word_t wNumb = grp; grp < Groups; ++grp) {
            wPrev = wNumb;
            Word_t wDelta = FinalGrpSz(wNumb + 1);
            //printf("# wDelta 0x%04zx %zd\n", wDelta, wDelta);
            wNumb += wDelta;
            if ((wNumb > nElms) || (wNumb < wPrev)) {
                wNumb = nElms;
                Pms[grp].ms_delta = nElms - wPrev;
                assert(grp == Groups - 1);
            } else {
                Pms[grp].ms_delta = wNumb - wPrev;
                assert(grp <= Groups - 1);
            }
            //printf("# wNumb 0x%04zx %zd\n", wNumb, wNumb);
        }
#endif // OLD_DS1_GROUPS
#ifndef CALC_NEXT_KEY
    #define MAX(_a, _b)  ((_a) > (_b) ? (_a) : (_b))
        if (nElms - Pms[grp-1].ms_delta > TValues) {
            wMaxEndDeltaKeys = TValues + Pms[grp-1].ms_delta;
            if (nElms - Pms[grp-1].ms_delta - Pms[grp-2].ms_delta > TValues) {
                if (TValues + Pms[grp-2].ms_delta > wMaxEndDeltaKeys) {
                    wMaxEndDeltaKeys = TValues + Pms[grp-2].ms_delta;
                }
            } else if (nElms - Pms[grp-1].ms_delta > wMaxEndDeltaKeys) {
                wMaxEndDeltaKeys = nElms - Pms[grp-1].ms_delta;
            }
        } else {
            wMaxEndDeltaKeys = nElms;
        }
        //printf("# nElms 0x%04zx %zd\n", nElms, nElms);
        //printf("# Pms[grp[-1].ms_delta 0x%04zx %zd\n", Pms[grp-1].ms_delta, Pms[grp-1].ms_delta);
        //printf("# Pms[grp[-2].ms_delta 0x%04zx %zd\n", Pms[grp-2].ms_delta, Pms[grp-2].ms_delta);
        //printf("# TValues 0x%04zx %zd\n", TValues, TValues);
        //printf("# wMaxEndDeltaKeys 0x%04zx %zd\n", wMaxEndDeltaKeys, wMaxEndDeltaKeys);
#endif // CALC_NEXT_KEY
    }
    else
    {
//  Calculate Multiplier for number of points per decade
//  Note: Fix, this algorithm chokes at about 1000 points/decade
        double DMult = pow(10.0, 1.0 / (double)PtsPdec);
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

        //printf("#  Groups    0x%04zx == 0d%05zd\n", Groups, Groups);

//      Get memory for saving measurements
        Pms = (Pms_t) malloc(Groups * sizeof(ms_t));
//        bzero((void *)Pms,  Groups * sizeof(ms_t));

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
            //printf("# ms_delta 0x%016zx\n", Pms[grp].ms_delta);

#ifndef CALC_NEXT_KEY
            #define MIN(_a, _b)  ((_a) < (_b) ? (_a) : (_b))
            Word_t wStartDeltaKeys = MIN(Isum, TValues);
            Word_t wEndDeltaKeys = wStartDeltaKeys + Pms[grp].ms_delta;
            if (wEndDeltaKeys > wMaxEndDeltaKeys)
            {
                wMaxEndDeltaKeys = wEndDeltaKeys;
            }
#endif // CALC_NEXT_KEY

            prevIsum = Isum;

            if (Pms[grp].ms_delta == 0)
                break;                  // for very high -P#
        }
    }
    // Groups = number of sizes

#ifndef CALC_NEXT_KEY
    // Trim size of key array based on partitioning of groups/deltas into
    // parts that are no bigger than TValues keys.
    // Might be able to trim this even more, but we've taken care of the
    // vast majority of waste with this.
    if (PartitionDeltaFlag && TrimKeyArrayFlag) {
        if (wMaxEndDeltaKeys > 2 * TValues) {
            wMaxEndDeltaKeys = 2 * TValues;
        }
    }
#endif // CALC_NEXT_KEY

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

#ifdef CALC_NEXT_KEY
    StartSeed = *PInitSeed;
#else // CALC_NEXT_KEY
    if (bLfsrOnly)
    {
        StartSeed = (NewSeed_t)StartSequent;
    }
    else
    {
        // Use FileKeys for Get measurements even without -F.
        if (FileKeys == NULL)
        {
            // Add one to wMaxEndDeltaKeys because TestJudyLInsert does one
            // extra GetNextKey.
            Word_t wKeysP1 = wMaxEndDeltaKeys + 1;
            // align wBytes and add an extra huge page so align after alloc is ok
            Word_t wBytes = ((wKeysP1 * sizeof(Word_t)) + 0x3fffff) & ~0x1fffff;
#ifdef USE_MALLOC
            FileKeys = (PWord_t)malloc(wBytes);
            if (FileKeys == NULL)
            {
                FAILURE("FileKeys malloc failure, Bytes =", wBytes);
            }
            printf("# malloc %zd bytes at %p for GetNextKey array.\n",
                   wBytes, (void *)FileKeys);
#else // USE_MALLOC
            FileKeys = (Word_t *)mmap(NULL, wBytes,
                                      (PROT_READ|PROT_WRITE),
                                      (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
            if (FileKeys == MAP_FAILED)
            {
                FAILURE("FileKeys mmap failure, Bytes =", wBytes);
            }
            printf("# mmap 0x%zx bytes at %p for GetNextKey array.\n",
                   wBytes, (void *)FileKeys);
#endif // USE_MALLOC
            // align the buffer
            FileKeys = (Word_t *)(((Word_t)FileKeys + 0x1fffff) & ~0x1fffff);
            printf("# FileKeys is %p after alignment.\n", (void *)FileKeys);

            Seed_t WorkingSeed = *PInitSeed;

            // These keys are used even for -DS1 until wLogPop is bigger than
            // wPrevLogPop and we want to reinitialize it using an LFSR.
            for (Word_t ww = 0; ww < TValues; ww++)
            {
                FileKeys[ww] = CalcNextKey(&WorkingSeed);
            }
        }
        // Could use StartSeed = &FileKeys[TValues] for -DS1
        // if PrevLogPop is initialized to 0.
        StartSeed = FileKeys;
    }
    Seed_t DeltaSeed = *PInitSeed; // for CalcNextKey
#endif // CALC_NEXT_KEY

// ============================================================
// PRINT HEADER TO PERFORMANCE TIMERS
// ============================================================

    Col = 1;
    printf("# COLHEAD %2d Population of the Array\n", Col++);
    printf("# COLHEAD %2d Place holder\n", Col++);
    printf("# COLHEAD %2d Number of Measurments done by JudyLGet/Judy1Test\n", Col++);

    if (tFlag)
        printf
            ("# COLHEAD %2d Ins  - Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
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
            ("# COLHEAD %2d Get  - Subtracted out (nS) measurement overhead (Number Generator etc..)\n",
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
#if defined(MIKEY)
        printf("# COLHEAD %2d REQ - Words requested/Key\n", Col++);
#else // defined(MIKEY)
        printf("# COLHEAD %2d L7  - Leaf 7 Byte Key/Key\n", Col++);
#endif // defined(MIKEY)
        printf("# COLHEAD %2d L6  - Leaf 6 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L5  - Leaf 5 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L4  - Leaf 4 Byte Key/Key\n", Col++);
#if defined(MIKEY)
        printf("# COLHEAD %2d B2  - Big bitmap leaf/Key\n", Col++);
#else // defined(MIKEY)
        printf("# COLHEAD %2d L3  - Leaf 3 Byte Key/Key\n", Col++);
#endif // defined(MIKEY)
        printf("# COLHEAD %2d L2  - Leaf 2 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d L1  - Leaf 1 Byte Key/Key\n", Col++);
        printf("# COLHEAD %2d B1  - Bitmap Leaf 1 Bit Key/Key\n", Col++);
        printf("# COLHEAD %2d VA  - Value area Words/Key\n", Col++);

        printf("# COLHEAD %2d MsCmp  - Average number missed Compares Per Leaf Search\n", Col++);
        printf("# COLHEAD %2d %%DiHt - %% of Direct Hits per Leaf Search\n", Col++);

//        printf("# COLHEAD %2d TrDep  - Tree depth with LGet/1Test searches\n", Col++);
        printf("# COLHEAD %2d AvPop  - Average Current Leaf Population\n", Col++);
        printf("# COLHEAD %2d %%MalEff - %% RAM JudyMalloc()ed vs mmap()ed from Kernel\n", Col++);

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
        Bytes = (MaxNumb + sizeof(Word_t)) / sizeof(Word_t);

        printf("# ========================================================\n");
        printf("#     WARNING '-b#' option with '-B%" PRIuPTR"...' option will malloc() a\n", BValue);
        printf("#     fixed sized bitmap of %" PRIuPTR" Bytes.\n", Bytes);
        printf("#  Measurements are WORTHLESS unless malloc() returns 2MiB aligned pointer\n");
        printf("# ========================================================\n");

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail

//      Allocate a Bitmap

#ifdef  USE_MALLOC
        B1 = (PWord_t)malloc(Bytes);
        if (B1 == (PWord_t)NULL)
        {
            FAILURE("malloc failure, Bytes =", Bytes);
        }
        printf("# B1 = 0x%" PRIxPTR" = malloc(%" PRIuPTR")\n", (Word_t)B1, Bytes);
#else   // ! USE_MALLOC

        JudyMalloc((Word_t)10); // get large page aligned
        B1 = (PWord_t)mmap(NULL, Bytes, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
        if (B1 == MAP_FAILED)
        {
            FAILURE("mmap failure, Bytes =", Bytes);
        }
        printf("# B1 = 0x%" PRIxPTR" = mmap(%" PRIuPTR",...)\n", (Word_t)B1, Bytes);
#endif // ! USE_MALLOC

//      clear 1/2 bitmap and bring into RAM
        STARTTm;
        bzero((void *)B1, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# bzero() (1st half) Bitmap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

//      copy 1st half to 2nd half
        STARTTm;
        memcpy((void *)((uint8_t *)B1 + (size_t)Bytes / 2), (void *)B1, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# memcpy() (to 2nd half) Bitmap at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

    if (yFlag)
    {
        Bytes = MaxNumb + 1;

        printf("# ========================================================\n");
        printf("#     WARNING '-y' option with '-B%" PRIuPTR"...' option will malloc() a\n", BValue);
        printf("#     fixed sized Bytemap of %" PRIuPTR" Bytes.\n", Bytes);
        printf("#  Measurements are WORTHLESS unless malloc() returns 2MiB aligned pointer\n");
        printf("# ========================================================\n");

//      Allocate a Bytemap
//      Make sure next mmap is 2Mib aligned (for Huge page TLBs)

#ifdef  USE_MALLOC
        By = (uint8_t *)malloc(Bytes);
        if (By == (uint8_t *)NULL)
        {
            FAILURE("malloc failure, Bytes =", Bytes);
        }
        printf("# By = 0x%" PRIxPTR" = malloc(%" PRIuPTR")\n", (Word_t)By, Bytes);

#else   // ! USE_MALLOC

        JudyMalloc((Word_t)1);
        By = (uint8_t *)mmap(NULL, Bytes, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
        if (By == MAP_FAILED)
        {
            FAILURE("mmap failure, Bytes =", Bytes);
        }
        printf("# By = 0x%" PRIxPTR" = mmap(%" PRIuPTR",...)\n", (Word_t)By, Bytes);
#endif // ! USE_MALLOC


//      clear 1/2 bitmap and bring into RAM
        STARTTm;
        bzero((void *)By, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# Zero Bytemap (1st half) at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

//      copy 1st half to 2nd half
        STARTTm;
        memcpy((void *)((uint8_t *)By + (size_t)Bytes / 2), (void *)By, (size_t)Bytes / 2);
        ENDTm(DeltanSecW);
        printf("# memcpy Bytemap (to 2nd half) at %5.2f Bytes per NanoSecond\n", (double)(Bytes / 2) / DeltanSecW);

        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
    }

// ============================================================
// Warm up the cpu
// ============================================================

//  Try to fool compiler and ACTUALLY execute random()
    Word_t WarmupVar = 0;
    STARTTm;
    do {
        for (ii = 0; ii < 1000000; ii++) {
            WarmupVar = random();
        }

        if (WarmupVar == 0) { // won't happen, but compiler doesn't know
            printf("!! Bug in random()\n");
        }

        ENDTm(DeltanSecW);      // get accumlated elapsed time
    } while (DeltanSecW < (Warmup * 1000000));

//  Now measure the execute time for 1M calls to random().
    STARTTm;
    for (ii = 0; ii < 1000000; ii++) {
        WarmupVar = random();
    }
    ENDTm(DeltanSecW);      // get elapsed time

    if (WarmupVar == 0) { // won't happen, but compiler doesn't know
        printf("\n");
    }

//  If this number is not consistant, then a longer warmup period is required
    printf("# random() = %4.2f nSec per/call\n", DeltanSecW/1000000);
    printf("#\n");

// ============================================================
// PRINT INITIALIZE COMMENTS IN YOUR VERSION OF JUDY FOR INFO
// ============================================================

    if (J1Flag)         // put initialize comments in
        Bytes = Judy1FreeArray(NULL, PJE0);
    if (JLFlag)
        Bytes = JudyLFreeArray(NULL, PJE0);
    if (JHFlag)
        Bytes = JudyHSFreeArray(NULL, PJE0);

// ============================================================
// PRINT COLUMNS HEADER TO PERFORMANCE TIMERS
// ============================================================

    PrintHeader("Population");

// ============================================================
// BEGIN TESTS AT EACH GROUP SIZE
// ============================================================

    InsertSeed = StartSeed;             // for JudyIns
    BitmapSeed = StartSeed;             // for bitmaps

#ifndef CALC_NEXT_KEY
    int wLogPop1 = wLogPop1; // wLogPop1 is used only for -DS1.
#endif // CALC_NEXT_KEY

    Word_t wFinalPop1 = 0;

    for (Pop1 = grp = 0; grp < Groups; grp++)
    {
        Word_t    Delta;
        double    DeltaGen1;
        double    DeltaGenL;
        double    DeltaGenHS;
        double DeltanSec1Sum = 0.0;
        double DeltanSecLSum = 0.0;
        double DeltanSecHSSum = 0.0;
        double DeltaMalFre1Sum = 0.0;
        double DeltaMalFreLSum = 0.0;
        double DeltaMalFreHSSum = 0.0;

        Delta = Pms[grp].ms_delta;

        if (Delta == 0)
            break;

        wFinalPop1 += Delta;
        int bFirstPart = 1;

        if (PartitionDeltaFlag) {
            if (Delta > TValues) {
                // Number of TValues size parts.
                // Plus 1 if there is any remainder.
                Word_t wParts = (Delta + TValues - 1) / TValues;
                // Equal size parts except the last part may be slightly
                // smaller.
                Delta = (Delta + wParts - 1) / wParts;
            }
nextPart:
            if (Pop1 + Delta > wFinalPop1) {
                Delta = wFinalPop1 - Pop1;
            }
        }

//      Accumulate the Total population of arrays
        Pop1 += Delta;

//      Only test for a maximum of TValues if not zero
        if (TValues && Pop1 > TValues)  // Trim measurments?
            Meas = TValues;
        else
            Meas = Pop1;

// MEB: Always testing the first keys inserted can give misleading performance
// numbers for Judy1Test in some cases, e.g. -S1 and -DS1.
// How can we test the -DS1 keys in a different order than the order in which
// they are inserted?
// For -DS1, we know what keys are in the array by virtue of knowing the first
// key inserted and the last key inserted.
#ifndef CALC_NEXT_KEY
// If we start at key=1, then we can use an lfsr with BValue == LOG(Pop1)
// and shift the result to generate at least half of the keys in
// a pseudo random order.
// So that is what we do.
// This doesn't work unless the -DS1 keys are not modified in any other way.
// MEB: We might be able to extend this approach to cover more of the -S cases
// than just -DS1.
        if (DFlag && (SValue == 1) && (StartSequent == 1)
            && !bSplayKeyBitsFlag && (Offset == 0) && (Bpercent == 100.0))
        {
            assert(!FValue);
            assert(!bLfsrOnly);
// If Pop1 is 2^n, then we will have inserted [1,2^n].
// LOG(Pop1) == n.
// LFSR(n) will generate values [1,2^n-1]. Ideal.
// If Pop1 is 2^n-1, then we will have inserted [1,2^n-1].
// LOG(Pop1) == n-1.
// LFSR(n) will generate values [1,2^(n-1)-1]. Not ideal.

// If Pop1 is 2^n-1, then we will have inserted [1,2^n-1].
// LOG(Pop1+1) == n.
// LFSR(n) will generate values in [1,2^n-1]. Ideal.
// If Pop1 is 2^n-2, then we will have inserted [1,2^n-2].
// LOG(Pop1+1) == n-1.
// LFSR(n) will generate values  [1,2^(n-1)-1]. Ideal.

// If Pop1 is 2^n-2, then we will have inserted [1,2^n-2].
// LOG(Pop1+2) == n.
// LFSR(n) will generate values in [1,2^n-1]. Bad.
            if ((wLogPop1 = LOG(Pop1+1)) > wPrevLogPop1) {
                wPrevLogPop1 = wLogPop1;
                // RandomInit always initializes the same Seed_t.  Luckily,
                // that one seed is not being used anymore at this point.
                // We use it here for the sole purpose of getting FeedBTap.
                PInitSeed = RandomInit(wLogPop1, 0);
#ifdef LFSR_GET_FOR_DS1
                wFeedBTap = PInitSeed->FeedBTap;
                BeginSeed = (NewSeed_t)StartSequent;
#else // LFSR_GET_FOR_DS1
                // RandomInit always initializes the same seed.
                // Copy it to RandomSeed.
                Seed_t RandomSeed = *PInitSeed;
                RandomSeed.Seeds[0] = StartSequent;
                // Reinitialize the TestJudyGet key array.
                // This method uses as few as half of the inserted keys
                // for testing. Now that we are using a key array we could
                // take a little more time if necessary and pick keys from
                // a larger and/or different subset.
#endif // LFSR_GET_FOR_DS1
                Meas = MIN(TValues, ((Word_t)1 << wLogPop1) - 1);
                for (Word_t ww = 0; ww < Meas; ++ww) {
                    // I wonder about using CalcNextKey here instead.
#ifdef LFSR_GET_FOR_DS1
                    // StartSeed[ww] = ...
                    FileKeys[ww] = GetNextKeyX(&BeginSeed,
                                               wFeedBTap,
                                               BValue - wLogPop1 + 1);
#else // LFSR_GET_FOR_DS1
                    // StartSeed[ww] = ...
                    Word_t wRand = RandomNumb(&RandomSeed, 0);
                    FileKeys[ww] = wRand << (BValue - wLogPop1);
#endif // LFSR_GET_FOR_DS1
                    if (ww == ((Word_t)1 << wLogPop1) - 1) {
                        break;
                    }
                }
            }
        }
#endif // CALC_NEXT_KEY

#ifdef DO_TIT
        int bDoTit = 1;
#else // DO_TIT
        int bDoTit = (Meas <= 256); // bDoTit threshold is 256
#endif // DO_TIT

        if (bFirstPart) {
            // first part of Delta
            // (Pop1 - Delta == wFinalPop1 - Pms[grp].ms_delta)
            if (grp && (grp % 16 == 0)) {
                PrintHeader("9876543210"); // print column headers periodically
            }
            printf("%12" PRIuPTR" .%11" PRIuPTR, wFinalPop1, Meas);
        }

#ifdef NEVER
        I dont think this code is ever executed (dlb)
        if (bFlag)
        {
//          Allocate a Bitmap, if not already done so
            if (B1 == NULL)
            {
                Word_t    ii;
                size_t    BMsize;

                // add one cache line for sister cache line read
                BMsize = ((Word_t)1 << (BValue - 3));
                if (posix_memalign((void **)&B1, 4096, BMsize) == 0)
                {
                    FAILURE("malloc failure, Bytes =", BMsize);
                }
//              clear the bitmap and bring into RAM
                for (ii = 0; ii < (BMsize / sizeof(Word_t)); ii++)
                    B1[ii] = 0;
            }
        }
#endif  // NEVER

#ifndef CALC_NEXT_KEY
        if (!bLfsrOnly && (FValue == 0))
        {
            // FileKeys[TValues] is where the delta keys begin.
            for (Word_t ww = 0; ww < Delta; ww++) {
                FileKeys[TValues + ww] = CalcNextKey(&DeltaSeed);
            }
            Seed_t TempSeedForJRFlag = DeltaSeed;
            FileKeys[TValues + Delta] = CalcNextKey(&TempSeedForJRFlag);
            InsertSeed = &FileKeys[TValues];
            BitmapSeed = InsertSeed;
        }
#endif // CALC_NEXT_KEY

        if (J1Flag || JLFlag || JHFlag)
        {
//          Test J1S, JLI, JLHS
//          Exit with InsertSeed/Key ready for next batch
//
            if (bFirstPart || (Pop1 == wFinalPop1)) {
                // first or last part of delta
                // last part might be a different size than the rest
                if (bDoTit) {
                    Tit = 0;                    // exclude Judy
                    DummySeed = InsertSeed;
                    WaitForContextSwitch(Delta);
                    TestJudyIns(&J1, &JL, &JH, &DummySeed, Delta);
                    DeltaGen1 = DeltanSec1 / Delta;     // save measurement overhead
                    DeltaGenL = DeltanSecL / Delta;
                    DeltaGenHS = DeltanSecHS / Delta;
                } else {
                    DeltaGen1 = DeltaGenL = DeltaGenHS = 0.1;
                } // end of bDoTit
            }

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyIns(&J1, &JL, &JH, &InsertSeed, Delta);
            DeltanSec1Sum += DeltanSec1;
            DeltanSecLSum += DeltanSecL;
            DeltanSecHSSum += DeltanSecHS;
            DeltaMalFre1Sum += DeltaMalFre1;
            DeltaMalFreLSum += DeltaMalFreL;
            DeltaMalFreHSSum += DeltaMalFreHS;

            if (Pop1 == wFinalPop1) {
                // last part of Delta
                if (J1Flag)
                {
                    if (tFlag)
                        PRINT6_1f(DeltaGen1);
                    DONTPRINTLESSTHANZERO(DeltanSec1Sum / Pms[grp].ms_delta, DeltaGen1);
                }
                if (JLFlag)
                {
                    if (tFlag)
                        PRINT6_1f(DeltaGenL);
                    DONTPRINTLESSTHANZERO(DeltanSecLSum / Pms[grp].ms_delta, DeltaGenL);
                }
                if (JHFlag)
                {
                    if (tFlag)
                        PRINT6_1f(DeltaGenHS);
                    DONTPRINTLESSTHANZERO(DeltanSecHSSum / Pms[grp].ms_delta, DeltaGenHS);
                }
                if (fFlag)
                    fflush(NULL);
            }

//      Note: the Get/Test code always tests from the "first" Key inserted.
//      The assumption is the "just inserted" Key would be unfair because
//      much of that would still be "in the cache".  Hopefully, the
//      "just inserted" would flush out the cache.  However, the Array
//      population has to grow beyond "TValues" before that will happen.
//      Currently, the default value of TValues is 1000000.  As caches
//      get bigger this value should be changed to a bigger value.

//          Test J1T, JLG, JHSG

            if (Pop1 == wFinalPop1) {
                if (bDoTit) {
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    if (bLfsrOnly) {
                        if (KFlag) {
                            if (hFlag) {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 1,
                                            /* bLfsrOnly */ 1);
                            } else {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 0,
                                            /* bLfsrOnly */ 1);
                            }
                        } else {
                            if (hFlag) {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 1,
                                            /* bLfsrOnly */ 1);
                            } else {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 0,
                                            /* bLfsrOnly */ 1);
                            }
                        }
                    } else {
                        if (KFlag) {
                            if (hFlag) {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 1,
                                            /* bLfsrOnly */ 0);
                            } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                                if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                    BeginSeed = (NewSeed_t)StartSequent;
                                    TestJudyGet(J1, JL, JH, &BeginSeed, Meas,
                                                /* Tit */ 0, /* KFlag */ 1,
                                                /* hFlag */ 0,
                                                /* bLfsrOnly */ BValue - wLogPop1 + 1);
                                } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 0,
                                            /* bLfsrOnly */ 0);
                            }
                        } else {
                            if (hFlag) {
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 1,
                                            /* bLfsrOnly */ 0);
                            } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                                if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                    BeginSeed = (NewSeed_t)StartSequent;
                                    TestJudyGet(J1, JL, JH, &BeginSeed, Meas,
                                                /* Tit */ 0, /* KFlag */ 0,
                                                /* hFlag */ 0,
                                                /* bLfsrOnly */ BValue - wLogPop1 + 1);
                                } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 0,
                                            /* bLfsrOnly */ 0);
                            }
                        }
                    }
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    DeltaGenHS = DeltanSecHS;
                } else {
                    DeltaGen1 = DeltaGenL = DeltaGenHS = 0.1;
                } // end of bDoTit

                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                if (bLfsrOnly) {
                    if (KFlag) {
                        if (hFlag) {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 1,
                                        /* bLfsrOnly */ 1);
                        } else {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 0,
                                        /* bLfsrOnly */ 1);
                        }
                    } else {
                        if (hFlag) {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 1,
                                        /* bLfsrOnly */ 1);
                        } else {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 0,
                                        /* bLfsrOnly */ 1);
                        }
                    }
                } else {
                    if (KFlag) {
                        if (hFlag) {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 1,
                                        /* bLfsrOnly */ 0);
                        } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                            if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                BeginSeed = (NewSeed_t)StartSequent;
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas,
                                            /* Tit */ 1, /* KFlag */ 1,
                                            /* hFlag */ 0,
                                            /* bLfsrOnly */ BValue - wLogPop1 + 1);
                            } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 0,
                                        /* bLfsrOnly */ 0);
                        }
                    } else {
                        if (hFlag) {
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 1,
                                        /* bLfsrOnly */ 0);
                        } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                            if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                BeginSeed = (NewSeed_t)StartSequent;
                                TestJudyGet(J1, JL, JH, &BeginSeed, Meas,
                                            /* Tit */ 1, /* KFlag */ 0,
                                            /* hFlag */ 0,
                                            /* bLfsrOnly */ BValue - wLogPop1 + 1);
                            } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                            TestJudyGet(J1, JL, JH, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 0,
                                        /* bLfsrOnly */ 0);
                        }
                    }
                }

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
        }

//      Insert/Get JudyL using Value area as next Key
        if (JRFlag)
        {
//          Test JLI
//          Exit with InsertSeed/Key ready for next batch

            if (bFirstPart || (Pop1 == wFinalPop1)) {
                // first or last part of delta
                // last part might be a different size than the rest
                if (bDoTit) {
                    Tit = 0;                    // exclude Judy
                    DummySeed = InsertSeed;
                    WaitForContextSwitch(Delta);
                    TestJudyLIns(&JL, &DummySeed, Delta);
                    DeltaGenL = DeltanSecL;
                } else {
                    DeltaGenL = 0.1;
                } // end of bDoTit
            }

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyLIns(&JL, &InsertSeed, Delta);
            DeltanSecLSum += DeltanSecL;
            DeltaMalFreLSum += DeltaMalFreL;
            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltaGenL);
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (fFlag)
                    fflush(NULL);
            }

          if (bDoTit) {
            Tit = 0;                    // exclude Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyLGet(JL, &BeginSeed, Meas);
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;                    // include Judy
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyLGet(JL, &BeginSeed, Meas);

//            TreeDepth        = j__TreeDepth;

//            SearchPopulation = j__SearchPopulation;
            DirectHits       = j__DirectHits;           // Number of direct hits
            SearchGets       = j__SearchGets;           // Number of object calls

            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltaGenL);
                DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test a REAL bitmap
        if (bFlag)
        {
            double    DeltanBit;

            //DummySeed = BitmapSeed;
            //GetNextKey(&DummySeed);   // warm up cache

          if (bDoTit) {
            Tit = 0;
            DummySeed = BitmapSeed;
            WaitForContextSwitch(Delta);
            TestBitmapSet(&B1, &DummySeed, Delta);
            DeltanBit = DeltanSecBt;
          } else {
            DeltanBit = 0.1;
          } // end of bDoTit

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestBitmapSet(&B1, &BitmapSeed, Delta);

            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltanBit);
                DONTPRINTLESSTHANZERO(DeltanSecBt, DeltanBit);
            }

          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestBitmapTest(B1, &BeginSeed, Meas);
            DeltanBit = DeltanSecBt;
          } else {
            DeltanBit = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestBitmapTest(B1, &BeginSeed, Meas);

            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltanBit);
                DONTPRINTLESSTHANZERO(DeltanSecBt, DeltanBit);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test a REAL ByteMap
        if (yFlag)
        {
            double    DeltanByte;

            //DummySeed = BitmapSeed;
            //GetNextKey(&DummySeed);   // warm up cache

          if (bDoTit) {
            Tit = 0;
            DummySeed = BitmapSeed;
            WaitForContextSwitch(Delta);
            TestByteSet(&DummySeed, Delta);
            DeltanByte = DeltanSecBy;
          } else {
            DeltanByte = 0.1;
          } // end of bDoTit

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestByteSet(&BitmapSeed, Delta);

            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltanByte);
                DONTPRINTLESSTHANZERO(DeltanSecBy, DeltanByte);
            }

          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestByteTest(&BeginSeed, Meas);
            DeltanByte = DeltanSecBy;
          } else {
            DeltanByte = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestByteTest(&BeginSeed, Meas);

            if (Pop1 == wFinalPop1) {
                if (tFlag)
                    PRINT6_1f(DeltanByte);
                DONTPRINTLESSTHANZERO(DeltanSecBy, DeltanByte);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test J1T, JLI, JHSI - duplicates

        if (IFlag)
        {
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDup(&J1, &JL, &JH, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;
          } else {
            DeltaGen1 = DeltaGenL = DeltaGenHS = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDup(&J1, &JL, &JH, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
                if (JLFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (JHFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
                if (fFlag)
                    fflush(NULL);
            }
        }
        if (cFlag && J1Flag)
        {
            WaitForContextSwitch(Meas);
            TestJudy1Copy(J1, Meas);
            if (Pop1 == wFinalPop1) {
                PRINT6_1f(DeltanSec1);
                if (fFlag)
                    fflush(NULL);
            }
        }
        if (CFlag)
        {
          if (bDoTit) {
            Tit = 0;
            WaitForContextSwitch(Meas);
            TestJudyCount(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGen1 = DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;
            WaitForContextSwitch(Meas);
            TestJudyCount(J1, JL, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
                if (JLFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (fFlag)
                    fflush(NULL);
            }
        }
        if (vFlag)
        {
//          Test J1N, JLN
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNext(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGen1 = DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNext(J1, JL, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    PRINT6_1f(DeltanSec1);
                if (JLFlag)
                    PRINT6_1f(DeltanSecL);
                if (fFlag)
                    fflush(NULL);
            }

//          Test J1P, JLP
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrev(J1, JL, &BeginSeed, ~(Word_t)0, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGen1 = DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrev(J1, JL, &BeginSeed, ~(Word_t)0, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    PRINT6_1f(DeltanSec1);
                if (JLFlag)
                    PRINT6_1f(DeltanSecL);
                if (fFlag)
                    fflush(NULL);
            }

//          Test J1NE, JLNE
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGen1 = DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
                if (JLFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (fFlag)
                    fflush(NULL);
            }

//          Test J1PE, JLPE
//
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
          } else {
            DeltaGen1 = DeltaGenL = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
                if (JLFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test J1U, JLD, JHSD
        if (dFlag)
        {
          if (bDoTit) {
            Tit = 0;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDel(&J1, &JL, &JH, &BeginSeed, Meas);
            DeltaGen1 = DeltanSec1;     // save measurement overhead
            DeltaGenL = DeltanSecL;
            DeltaGenHS = DeltanSecHS;
          } else {
            DeltaGen1 = DeltaGenL = DeltaGenHS = 0.1;
          } // end of bDoTit

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDel(&J1, &JL, &JH, &BeginSeed, Meas);
            if (Pop1 == wFinalPop1) {
                if (J1Flag)
                    DONTPRINTLESSTHANZERO(DeltanSec1, DeltaGen1);
                if (JLFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecL, DeltaGenL);
                if (JHFlag)
                    DONTPRINTLESSTHANZERO(DeltanSecHS, DeltaGenHS);
                if (fFlag)
                    fflush(NULL);
            }

//          Now put back the Just deleted Keys
            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyIns(&J1, &JL, &JH, &BeginSeed, Meas);
        }

        if (Pop1 == wFinalPop1) {

            if ((J1Flag + JLFlag + JHFlag) == 1)            // only 1 Heap
                PRINT7_3f((double)j__AllocWordsTOT / (double)Pop1);

            if (mFlag && (bFlag == 0) && (yFlag == 0))
            {
//                double AveSrcCmp, PercentLeafSearched;
                double PercentLeafWithDirectHits;

//              Calc average compares done in Leaf for this measurement interval
//                AveSrcCmp = SearchCompares / (double)Meas;
//                AveSrcCmp = DirectHits / SearchGets;

//              Calc average percent of Leaf searched
//                if (SearchPopulation == 0)
//                    PercentLeafWithDirectHits = 0.0;
//                else
//                    PercentLeafWithDirectHits = SearchCompares / SearchPopulation * 100.0;
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


//              print average number of failed compares done in leaf search
//                printf(" %6.1f", AveSrcCmp);
//                PRINT5_2f(j__MissCompares / (double)Meas);
                printf(" %5.1f", (double)j__MissCompares / (double)Meas);

//printf("\nj__MissCompares = %" PRIuPTR", Meas = %" PRIuPTR"\n", j__MissCompares, Meas);

//              print average percent of Leaf searched (with compares)
                printf(" %5.1f", PercentLeafWithDirectHits);

//              print average number of Branches traversed per lookup
//                printf(" %5.1f", TreeDepth / (double)Meas);
//
                if (j__SearchGets == 0)
                    printf(" %5.1f", 0.0);
                else
                    printf(" %5.1f", (double)j__SearchPopulation / (double)j__SearchGets);

//              reset for next measurement
//                j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
                j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;


// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSss


//              Print the percent efficiency of dlmalloc
                PRINT7_3f(j__AllocWordsTOT / (double)(j__TotalBytesAllocated / sizeof(Word_t)));
                if (J1Flag)
                    PRINT5_2f(DeltaMalFre1Sum / Pms[grp].ms_delta);
                if (JLFlag || JRFlag)
                    PRINT5_2f(DeltaMalFreLSum / Pms[grp].ms_delta);
                if (JHFlag)
                    PRINT5_2f(DeltaMalFreHSSum / Pms[grp].ms_delta);
            }
            if (yFlag || bFlag)
            {
                printf(" %14.2f", ((double)j__TotalBytesAllocated / sizeof(Word_t)) / (double)Pop1);
            }
            printf("\n");
            if (fFlag)
                fflush(NULL);                   // assure data gets to file in case malloc fail
        }
        if (Pop1 != wFinalPop1) {
            bFirstPart = 0;
            goto nextPart;
        }
    }

    if (J1Flag)
        Count1 = Judy1Count(J1, 0, -1, PJE0);
    if (JLFlag || JRFlag)
        CountL = JudyLCount(JL, 0, -1, PJE0);

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
            ("# Judy1FreeArray:  %" PRIuPTR", %0.3f Words/Key, %" PRIuPTR" Bytes released, %0.3f nSec/Key\n",
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
            ("# JudyLFreeArray:  %" PRIuPTR", %0.3f Words/Key, %" PRIuPTR" Bytes released, %0.3f nSec/Key\n",
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
            ("# JudyHSFreeArray: %" PRIuPTR", %0.3f Words/Key, %0.3f nSec/Key\n",
             nElms, (double)(Bytes / sizeof(Word_t)) / (double)nElms,
             DeltanSecHS);
    }

//    if (bFlag && GValue)
//         printf("\n# %" PRIuPTR" Duplicate Keys were found with -G%" PRIuPTR"\n", BitmapDups, GValue);

    exit(0);
#undef JL
#undef J1
}

#ifdef DEADCODE
// This code is for testing the basic timing loop used everywhere else
// Enable it for whatever you want
#undef __FUNCTI0N__
#define __FUNCTI0N__ "TimeNumberGen"

static int
TimeNumberGen(void **TestRan, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    DummyAccum = 0;

    NewSeed_t WorkingSeed;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));
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
TestJudyIns(void **J1, void **JL, void **JH, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    int       Rc = 0;

    NewSeed_t WorkingSeed = *PSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    StartMallocs;

    DeltanSec1 = 0.0;
    DeltanSecL = 0.0;
    DeltanSecHS = 0.0;

    Word_t Loops = 1; // loops don't work as expected for array modifying ops

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
                    Rc = Judy1Unset(J1, TstKey, PJE0);
                }
            }

            StartMallocs = j__MalFreeCnt;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

//              Skip Judy1Set
                if (hFlag && ((TstKey & 0x3F) == 13))
                {
                    if (Tit)
                    {
//         printf("\nSkip Set at Key = 0x%" PRIxPTR", elm %" PRIuPTR"\n", TstKey, elm);

//                      Nothing to do, skip the Insert
                    }
                }
                else
                {
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
                                printf("\nTstKey = 0x%" PRIxPTR"\n", TstKey);
                                Judy1Dump((Word_t)*J1, sizeof(Word_t) * 8, 0);
                                FAILURE("Judy1Set failed - DUP Key at elm", elm);
                            }
                        }
                        if (iFlag)
                        {
                            J1S(Rc, *J1, TstKey);
                            if (Rc != 0)
                            {
                                printf("\n--- Judy1Set Rc = %d after Judy1Set, Key = 0x%" PRIxPTR", elm = %" PRIuPTR"",
                                     Rc, TstKey, elm);
                                FAILURE("Judy1Test failed at", elm);
                            }
                        }
                        if (gFlag)
                        {
                            Rc = Judy1Test(*J1, TstKey, PJE0);
                            if (Rc != 1)
                            {
                                printf("\n--- Judy1Test Rc = %d after Judy1Set, Key = 0x%" PRIxPTR", elm = %" PRIuPTR"",
                                     Rc, TstKey, elm);
                                FAILURE("Judy1Test failed at", elm);
                            }

                            // Test for a key that has not been inserted.
                            // Pick one that differs from a key that has
                            // been inserted by only a single digit in an
                            // attempt to test narrow pointers.
                            if (bSplayKeyBitsFlag)
                            {
                                if (wCheckBit != 0)
                                {
                                    Word_t TstKeyNot = TstKey ^ wCheckBit;
                                    Rc = Judy1Test(*J1, TstKeyNot, PJE0);
                                    if (Rc != 0)
                                    {
                                        printf("\n--- Judy1Test(0x%zx) Rc = %d after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                               TstKeyNot, Rc, TstKey, elm);
                                        FAILURE("Judy1Test failed at", elm);
                                    }
                                }
                            }
                            else if (BValue < sizeof(Word_t) * 8)
                            {
                                if (BValue < sizeof(Word_t) * 8 - 8)
                                {
                                    // try changing a bit in the next 8-bit digit up from the BValue
                                    Word_t TstKeyNot = TstKey ^ ((Word_t)1 << ((BValue + 7) & ~(Word_t)7));
                                    Rc = Judy1Test(*J1, TstKeyNot, PJE0);
                                    if (Rc != 0)
                                    {
                                        printf("\n--- Judy1Test(0x%zx) Rc = %d after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                               TstKeyNot, Rc, TstKey, elm);
                                        FAILURE("Judy1Test failed at", elm);
                                    }
                                }
                                else
                                {
                                    Word_t TstKeyNot = TstKey ^ ((Word_t)1 << (sizeof(Word_t) * 8 - 1));
                                    Rc = Judy1Test(*J1, TstKeyNot, PJE0);
                                    if (Rc != 0)
                                    {
                                        printf("\n--- Judy1Test(0x%zx) Rc = %d after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                               TstKeyNot, Rc, TstKey, elm);
                                        FAILURE("Judy1Test failed at", elm);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ENDTm(DeltanSec1);
            DeltaMalFre1 = j__MalFreeCnt - StartMallocs;

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
        DeltanSec1 = DminTime;
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
                    Rc = JudyLDel(JL, TstKey, PJE0);
                }
            }

            StartMallocs = j__MalFreeCnt;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

//              Skip JudyLIns
                if (hFlag && ((TstKey & 0x3F) == 13))
                {
                    if (Tit)
                    {
//         printf("\nSkip Set at Key = 0x%" PRIxPTR", elm %" PRIuPTR"\n", TstKey, elm);
//                      Nothing to do, skip the Insert
                    }
                }
                else
                {
                    if (Tit)
                    {
                        if (TstKey == (Word_t)0)
                        {
                            PValue = (PWord_t)JudyLGet(*JL, (Word_t)0, PJE0);
                            if (PValue != NULL)
                                FAILURE("JudyLIns failed - Duplicate *PValue =", *PValue);
                        }
                        PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
                        if (PValue == (PWord_t)NULL)
                        {
                                FAILURE("JudyLIns failed - NULL PValue", TstKey);
                        }
                        if (*PValue != 0)               // Oops, Insert made error
                        {
                            if (GValue && (*PValue == TstKey))
                            {
                                JudyLDups++;
                            }
                            else
                            {
                                printf("\nTstKey = 0x%" PRIxPTR", *PValue = 0x%" PRIxPTR"\n", TstKey, *PValue);
                                FAILURE("JudyLIns returned wrong *PValue after Insert", TstKey);
                            }
                        }
                        if (VFlag)
                        {
                            *PValue = TstKey;     // save Key in Value

                            if (iFlag)  // mainly for debug
                            {
                                PWord_t   PValueNew;

                                PValueNew = (PWord_t)JudyLIns(JL, TstKey, PJE0);
                                if (PValueNew == NULL)
                                {
                                    printf("\nTstKey = 0x%" PRIxPTR"\n", TstKey);
                                    FAILURE("JudyLIns failed with NULL after Insert", TstKey);
                                }
                                if (PValueNew != PValue)
                                {
//                                    printf("\n#Line = %d, Caution: PValueNew = 0x%" PRIxPTR", PValueold = 0x%" PRIxPTR" changed\n", __LINE__, (Word_t)PValueNew, (Word_t)PValue);
//                                    printf("- ValueNew = 0x%" PRIxPTR", Valueold = 0x%" PRIxPTR"\n", *PValueNew, *PValue);
//                                    FAILURE("Second JudyLIns failed with wrong PValue after Insert", TstKey);
                                }
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n*PValueNew = 0x%" PRIxPTR"\n", *PValueNew);
                                    printf("TstKey = 0x%" PRIxPTR" = %" PRIdPTR"\n", TstKey, TstKey);
                                    FAILURE("Second JudyLIns failed with wrong *PValue after Insert", TstKey);
                                }
                            }
                            if (gFlag)  // mainly for debug
                            {
                                PWord_t   PValueNew;

                                PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                                if (PValueNew == NULL)
                                {
                                    printf("\n--- TstKey = 0x%" PRIxPTR"", TstKey);
                                    FAILURE("JudyLGet failed after Insert", TstKey);
                                }
                                else
                                {
                                    if (*PValueNew != TstKey)
                                    {
                                        printf("\n--- *PValueNew = 0x%" PRIxPTR"\n", *PValueNew);
                                        printf("--- TstKey = 0x%" PRIxPTR" = %" PRIdPTR"", TstKey, TstKey);
                                        FAILURE("JudyLGet failed after Insert", TstKey);
                                    }
                                }

                                // Test for a key that has not been inserted.
                                // Pick one that differs from a key that has
                                // been inserted by only a single digit in an
                                // attempt to test narrow pointers.
                                if (bSplayKeyBitsFlag)
                                {
                                    if (wCheckBit != 0)
                                    {
                                        Word_t TstKeyNot = TstKey ^ wCheckBit;
                                        PValueNew = (PWord_t)JudyLGet(*JL, TstKeyNot, PJE0);
                                        if (PValueNew != NULL)
                                        {
                                            printf("\n--- JudyLGet(0x%zx) *PValue = 0x%zx after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                                   TstKeyNot, *PValueNew, TstKey, elm);
                                            FAILURE("JudyLGet failed at", elm);
                                        }
                                    }
                                }
                                else if (BValue < sizeof(Word_t) * 8)
                                {
                                    if (BValue < sizeof(Word_t) * 8 - 8)
                                    {
                                        // try changing a bit in the next 8-bit digit up from the BValue
                                        Word_t TstKeyNot = TstKey ^ ((Word_t)1 << ((BValue + 7) & ~(Word_t)7));
                                        PValueNew = (PWord_t)JudyLGet(*JL, TstKeyNot, PJE0);
                                        if (PValueNew != NULL)
                                        {
                                            printf("\n--- JudyLGet(0x%zx) *PValue = 0x%zx after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                                   TstKeyNot, *PValueNew, TstKey, elm);
                                            FAILURE("JudyLGet failed at", elm);
                                        }
                                    }
                                    else
                                    {
                                        Word_t TstKeyNot = TstKey ^ ((Word_t)1 << (sizeof(Word_t) * 8 - 1));
                                        PValueNew = (PWord_t)JudyLGet(*JL, TstKeyNot, PJE0);
                                        if (PValueNew != NULL)
                                        {
                                            printf("\n--- JudyLGet(0x%zx) *PValue = 0x%zx after Judy1Set, Key = 0x%zx, elm = %zu\n",
                                                   TstKeyNot, *PValueNew, TstKey, elm);
                                            FAILURE("JudyLGet failed at", elm);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ENDTm(DeltanSecL);
            DeltaMalFreL = j__MalFreeCnt - StartMallocs;

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

            StartMallocs = j__MalFreeCnt;
            WorkingSeed = *PSeed;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

                if (Tit)
                {
                    PValue = (PWord_t)JudyHSIns(JH, &TstKey, sizeof(Word_t), PJE0);
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
            DeltaMalFreHS = j__MalFreeCnt - StartMallocs;

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
TestJudyLIns(void **JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    PWord_t   PValue;
    Word_t DummyValue;

    NewSeed_t WorkingSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    StartMallocs;

    DeltanSecL = 0.0;
    PValue = (PWord_t)NULL;

    Word_t Loops = 1;

//  JudyLIns timings
    DminTime = 1e40; icnt = ICNT; lp = 0;
    do
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

        StartMallocs = j__MalFreeCnt;
        WorkingSeed = *PSeed; // restore after Del

        PValue = &DummyValue;

        STARTTm;                        // start timer
        for (elm = 0; elm < Elements; elm++)
        {
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

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
        ENDTm(DeltanSecL);

        // Remember to set the value for the last insert.
        // But don't mess up WorkingSeed before copying it back to the caller.
        // And don't go past the end of the key array.
        // How do I avoid going past the end of the array?
        // By making the array one bigger than it would otherwise need to be.
        NewSeed_t TempSeed = WorkingSeed;
        // This GetNextKey is the reason the key array has to be one bigger
        // than the maximum delta size.
        TstKey = GetNextKey(&TempSeed);
        *PValue = TstKey;

        DeltaMalFreL = j__MalFreeCnt - StartMallocs;

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
    while (++lp < Loops);

    *PSeed = WorkingSeed;               // advance
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDup"

int
TestJudyDup(void **J1, void **JL, void **JH, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    NewSeed_t WorkingSeed;
    int       Rc;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

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
                    PValue = (PWord_t)JudyLIns(JL, TstKey, PJE0);
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
                    PValue =
                        (PWord_t)JudyHSIns(JH, &TstKey, sizeof(Word_t), PJE0);
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

static inline int
TestJudyGet(void *J1, void *JL, void *JH, PNewSeed_t PSeed, Word_t Elements,
            Word_t Tit, Word_t KFlag, Word_t hFlag, int bLfsrOnly)
{
    Word_t    TstKey;
    Word_t    elm;
    Word_t   *PValue;
    int       Rc;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    PWord_t   DmyStackMem = NULL;

    NewSeed_t WorkingSeed;

    if (PreStack)
            DmyStackMem = (Word_t *)alloca(PreStack);  // move stack a bit

    if (DmyStackMem == (PWord_t)1)                      // shut up compiler
    {
        printf("BUG -- fixme\n");
        exit(1);
    }

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            WorkingSeed = *PSeed;
            Word_t wFeedBTapLocal = wFeedBTap; // don't know why this is faster

//          reset for next measurement
//            j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                SYNC_SYNC(TstKey = GetNextKeyX(&WorkingSeed, wFeedBTapLocal, bLfsrOnly));
                //SYNC_SYNC(TstKey = GetNextKeyX(&WorkingSeed, wFeedBTap, bLfsrOnly));

//              Holes in the Set Code ?
                if (hFlag && ((TstKey & 0x3F) == 13))
                {
                    if (Tit)
                    {
                        Rc = Judy1Test(J1, TstKey, PJE0);
                        if (Rc != 0)
                        {
                            printf("\n--- Judy1Test wrong Rc = %d, Key = 0x%" PRIxPTR", elm = %" PRIuPTR"",
                                 Rc, TstKey, elm);
                            FAILURE("Judy1Test Rc != 0", Rc);
                        }
                    }
                }
                else
                {
                    if (Tit)
                    {
                        Rc = Judy1Test(J1, TstKey, PJE0);
                        if (Rc != 1)
                        {
                            printf("\n--- Judy1Test wrong Rc = %d, Key = 0x%" PRIxPTR", elm = %" PRIuPTR"",
                                 Rc, TstKey, elm);
                            FAILURE("Judy1Test Rc != 1", Rc);
                        }
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
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

//              Holes in the Insert Code ?
                if (hFlag && ((TstKey & 0x3F) == 13))
                {
                    if (Tit)
                    {
                        PValue = (PWord_t)JudyLGet(JL, TstKey, PJE0);
                        if (PValue != (Word_t *)NULL)
                        {
                            printf("\n--- JudyLGet Key = 0x%" PRIxPTR"", TstKey);
                            FAILURE("JudyLGet ret PValue != NULL -- Key inserted???", 0L);
                        }
                    }
                }
                else
                {
                    if (Tit)
                    {
                        PValue = (PWord_t)JudyLGet(JL, TstKey, PJE0);
                        if (PValue == (Word_t *)NULL)
                        {
                            printf("\n--- JudyLGet Key = 0x%" PRIxPTR"", TstKey);
                            FAILURE("JudyLGet ret PValue = NULL", 0L);
                        }
                        else if (VFlag && (*PValue != TstKey))
                        {
                            printf
                                ("--- JudyLGet returned Value=0x%" PRIxPTR", should be=0x%" PRIxPTR"",
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
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

                if (Tit)
                {
                    PValue = (PWord_t)JudyHSGet(JH, &TstKey, sizeof(Word_t));
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
TestJudyLGet(void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
//    Word_t   *PValue;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;

    NewSeed_t WorkingSeed;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    icnt = ICNT;

    for (DminTime = 1e40, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

//      reset for next measurement
//        j__SearchPopulation = j__TreeDepth = j__MissCompares = j__DirectHits = j__SearchGets = 0;
            j__SearchPopulation = j__MissCompares = j__DirectHits = j__SearchGets = 0;

        TstKey = GetNextKey(&WorkingSeed);    // Get 1st Key

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

    J1a = NULL;                         // Initialize To array

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        Word_t    NextKey;
        int       Rc;

        NextKey = 0;                  // Start at the beginning
        elm = 0;
        STARTTm;

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
TestJudyCount(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;
    Word_t    Count1, CountL;
    Word_t    TstKey;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
#ifdef TEST_COUNT_USING_JUDY_NEXT
            (void)PSeed;
            TstKey = 0;
            int Rc = Judy1First(J1, &TstKey, PJE0);
#else // TEST_COUNT_USING_JUDY_NEXT
            NewSeed_t WorkingSeed = *PSeed;
#endif // TEST_COUNT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_COUNT_USING_JUDY_NEXT
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));
#endif // TEST_COUNT_USING_JUDY_NEXT
                if (Tit)
                {
                    Count1 = Judy1Count(J1, 0, TstKey, PJE0);
#ifdef TEST_COUNT_USING_JUDY_NEXT
                    if (Count1 != (elm + 1))
#else // TEST_COUNT_USING_JUDY_NEXT
                    if (Count1 > TstKey + 1)
#endif // TEST_COUNT_USING_JUDY_NEXT
                    {
                        printf("Count1 = %" PRIuPTR", TstKey = %" PRIuPTR"\n", Count1, TstKey);
                        FAILURE("J1C at", elm);
                    }
                }
#ifdef TEST_COUNT_USING_JUDY_NEXT
                Rc = Judy1Next(J1, &TstKey, PJE0);
#endif // TEST_COUNT_USING_JUDY_NEXT
            }
            ENDTm(DeltanSec1);

#ifdef TEST_COUNT_USING_JUDY_NEXT
            if (Rc == 1234)             // impossible value
                exit(-1);               // shut up compiler only
#endif // TEST_COUNT_USING_JUDY_NEXT

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

            PValue = (PWord_t)JudyLFirst(JL, &TstKey, PJE0);

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                if (Tit)
                {

                    CountL = JudyLCount(JL, 0, TstKey, PJE0);

                    if (CountL != (elm + 1))
                    {
                        printf("CountL = %" PRIuPTR", elm +1 = %" PRIuPTR"\n", CountL, elm + 1);
                        FAILURE("JLC at", elm);
                    }
                }

                PValue = (PWord_t)JudyLNext(JL, &TstKey, PJE0);
                if (VFlag && PValue && (*PValue != TstKey))
                {
                    printf("\nPValue=0x%" PRIxPTR", *PValue=0x%" PRIxPTR", TstKey=0x%" PRIxPTR"\n",
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
TestJudyNext(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    JLKey;
    Word_t    J1Key;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            int Rc;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            (void)PSeed;
            J1Key = 0;
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t J1LastKey = -1;
            Judy1Last(J1, &J1LastKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(J1Key = GetNextKey(&WorkingSeed));
#endif // TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    Word_t J1KeyBefore = J1Key;
#endif // TEST_NEXT_USING_JUDY_NEXT
                    Rc = Judy1Next(J1, &J1Key, PJE0);
                    if ((Rc != 1)
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        && (J1KeyBefore != J1LastKey)
#endif // TEST_NEXT_USING_JUDY_NEXT
                        )
                    {
                        printf("\nElements = %" PRIuPTR", elm = %" PRIuPTR"\n", Elements, elm);
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1LastKey %" PRIuPTR"\n", J1LastKey);
                        printf("J1KeyBefore %" PRIuPTR"\n", J1KeyBefore);
#endif // TEST_NEXT_USING_JUDY_NEXT
                        FAILURE("Judy1Next Rc != 1 J1Key", J1Key);
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

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t   *PValue;

//          Get an Key low enough for Elements
            JLKey = 0;

            STARTTm;

            PValue = (PWord_t)JudyLFirst(JL, &JLKey, PJE0);

            for (elm = 0; elm < Elements; elm++)
            {
                Word_t    Prev;
                if (PValue == NULL)
                {
                    printf("\nElements = %" PRIuPTR", elm = %" PRIuPTR"\n", Elements, elm);
                    FAILURE("JudyLNext ret NULL PValue at", elm);
                }
                if (VFlag && (*PValue != JLKey))
                {
                    printf("\n*PValue=0x%" PRIxPTR", JLKey=0x%" PRIxPTR"\n", *PValue,
                           JLKey);
                    FAILURE("JudyLNext ret bad *PValue at", elm);
                }
                Prev = JLKey;
                PValue = (PWord_t)JudyLNext(JL, &JLKey, PJE0);
                if ((PValue != NULL) && (JLKey == Prev))
                {
                    printf("OOPs, JLN did not advance 0x%" PRIxPTR"\n", Prev);
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
TestJudyPrev(void *J1, void *JL, PNewSeed_t PSeed, Word_t HighKey, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    Word_t    J1Key;
    Word_t    JLKey;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    if (J1Flag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            int Rc;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            (void)PSeed;
            J1Key = HighKey;
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t J1FirstKey = 0;
            Judy1First(J1, &J1FirstKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(J1Key = GetNextKey(&WorkingSeed));
#endif // TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    Word_t J1KeyBefore = J1Key;
#endif // TEST_NEXT_USING_JUDY_NEXT
                    Rc = Judy1Prev(J1, &J1Key, PJE0);
                    if ((Rc != 1)
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        && (J1KeyBefore != J1FirstKey)
#endif // TEST_NEXT_USING_JUDY_NEXT
                        )
                    {
                        printf("\nElements = %" PRIuPTR", elm = %" PRIuPTR"\n", Elements, elm);
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1FirstKey %" PRIuPTR"\n", J1FirstKey);
                        printf("J1KeyBefore %" PRIuPTR"\n", J1KeyBefore);
#endif // TEST_NEXT_USING_JUDY_NEXT
                        FAILURE("Judy1Prev Rc != 1 J1Key", J1Key);
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

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t   *PValue;
            JLKey = HighKey;

            STARTTm;

            PValue = (PWord_t)JudyLLast(JL, &JLKey, PJE0);

            for (elm = 0; elm < Elements; elm++)
            {
                if (PValue == NULL)
                {
                    printf("\nElements = %" PRIuPTR", elm = %" PRIuPTR"\n", Elements, elm);
                    FAILURE("JudyLPrev ret NULL PValue at", elm);
                }
                if (VFlag && (*PValue != JLKey))
                    FAILURE("JudyLPrev ret bad *PValue at", elm);

                PValue = (PWord_t)JudyLPrev(JL, &JLKey, PJE0);
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
TestJudyNextEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    NewSeed_t WorkingSeed;
    int       Rc;                       // Return code

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

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
                    Word_t J1KeyBefore = J1Key;
                    Rc = Judy1NextEmpty(J1, &J1Key, PJE0);
                    if (Rc != 1) {
                        Judy1Dump((Word_t)J1, sizeof(Word_t) * 8, 0);
                        FAILURE("Judy1NextEmpty Rcode != 1 Key", J1KeyBefore);
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
                    Word_t JLKeyBefore = JLKey;
                    Rc = JudyLNextEmpty(JL, &JLKey, PJE0);
                    if (Rc != 1)
                        FAILURE("JudyLNextEmpty Rcode != 1 Key", JLKeyBefore);
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
TestJudyPrevEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    elm;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;
    NewSeed_t WorkingSeed;
    int       Rc;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

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
                    Word_t J1KeyBefore = J1Key;
                    Rc = Judy1PrevEmpty(J1, &J1Key, PJE0);
                    if (Rc != 1) {
                        Word_t wCount = Judy1Count(J1, 0, J1KeyBefore, PJE0);
                        if (wCount < J1KeyBefore + 1) {
                            FAILURE("Judy1PrevEmpty Rc != 1 Key", J1KeyBefore);
                        }
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
                    Word_t JLKeyBefore = JLKey;
                    Rc = JudyLPrevEmpty(JL, &JLKey, PJE0);
                    if (Rc != 1) {
                        Word_t wCount = JudyLCount(JL, 0, JLKeyBefore, PJE0);
                        if (wCount < JLKeyBefore + 1) {
                            FAILURE("JudyLPrevEmpty Rc != 1 Key", JLKeyBefore);
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
TestJudyDel(void **J1, void **JL, void **JH, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    int       Rc;

    NewSeed_t WorkingSeed;

// set PSeed to beginning of Inserts
//
    if (J1Flag)
    {
        WorkingSeed = *PSeed;

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

//          Holes in the tree ?
            if (hFlag && ((TstKey & 0x3F) == 13))
            {
                if (Tit)
                {
                    Rc = Judy1Unset(J1, TstKey, PJE0);

                    if (Rc != 0)
                    {
                        printf("--- Key = 0x%" PRIxPTR"", TstKey);
                        FAILURE("Judy1Unset ret Rcode != 0", Rc);
                    }
                }
            }
            else
            {
                if (Tit)
                {
                    Rc = Judy1Unset(J1, TstKey, PJE0);

                    if (Rc != 1)
                    {
                        printf("--- Key = 0x%" PRIxPTR"", TstKey);
                        FAILURE("Judy1Unset ret Rcode != 1", Rc);
                    }

                    if (gFlag)
                    {
                        Rc = Judy1Test(*J1, TstKey, PJE0);

                        if (Rc)
                        {
                            printf("\n--- Judy1Test success after Judy1Unset, Key = 0x%" PRIxPTR"", TstKey);
                            //Judy1Dump((Word_t)*J1, sizeof(Word_t) * 8, 0);
                            FAILURE("Judy1Test success after Judy1Unset", TstKey);
                        }
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
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

//          Holes in the tree ?
            if (hFlag && ((TstKey & 0x3F) == 13))
            {
                if (Tit)
                {
                    Rc = JudyLDel(JL, TstKey, PJE0);

                    if (Rc != 0)
                    {
                        printf("\n--- Key = 0x%" PRIxPTR"", TstKey);
                        FAILURE("JudyLDel ret Rcode != 0", Rc);
                    }
                }
            }
            else
            {
                if (Tit)
                {

                    Rc = JudyLDel(JL, TstKey, PJE0);

                    if (Rc != 1)
                    {
                        printf("\n--- Key = 0x%" PRIxPTR"", TstKey);
                        FAILURE("JudyLDel ret Rcode != 1", Rc);
                    }

                    if (gFlag)
                    {
                        PWord_t   PValueNew;

                        PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                        if (PValueNew != NULL)
                        {
                            printf("\n--- JudyLGet success after JudyLDel, Key = 0x%" PRIxPTR", Value = 0x%" PRIxPTR"", TstKey, *PValueNew);
                            FAILURE("JudyLGet success after JudyLDel", TstKey);
                        }
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
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

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
TestByteTest(PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    NewSeed_t WorkingSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

        STARTTm;

        for (elm = 0; elm < Elements; elm++)
        {
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

            if (Tit)
            {
                if (By[TstKey] == 0)
                {
                    printf("\nByteGet -- missing bit, Key = 0x%" PRIxPTR"",
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
TestByteSet(PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;

    STARTTm;
    for (elm = 0; elm < Elements; elm++)
    {
        SYNC_SYNC(TstKey = GetNextKey(PSeed));

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
                    printf("\nByteSet -- Set bit, Key = 0x%" PRIxPTR"", TstKey);
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
TestBitmapSet(PWord_t *pB1, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;

    STARTTm;
    for (elm = 0; elm < Elements; elm++)
    {
        SYNC_SYNC(TstKey = GetNextKey(PSeed));

        if (Tit)
        {
            if (BitmapSet(*pB1, TstKey) == 0)
            {
                if (GValue)
                {
                    BitmapDups++;
                }
                else
                {
                    printf("\nBitMapSet -- Set bit, Key = 0x%" PRIxPTR"", TstKey);
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
TestBitmapTest(PWord_t B1, PNewSeed_t PSeed, Word_t Elements)
{
    Word_t    TstKey;
    Word_t    elm;
    NewSeed_t WorkingSeed;

    double    DminTime;
    Word_t    icnt;
    Word_t    lp;

    Word_t Loops = lFlag ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

    for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
    {
        WorkingSeed = *PSeed;

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));

            if (Tit)
            {
                if (BitmapGet(B1, TstKey) == 0)
                {
                    printf("\nBitMapGet -- missing bit, Key = 0x%" PRIxPTR"",
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

