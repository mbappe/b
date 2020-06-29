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
#if defined(DEBUG_ALL) || defined(DEBUG_LOOKUP)
#undef DEBUG
#define DEBUG
#endif // DEBUG_ALL || DEBUG_LOOKUP

#if defined(DEBUG_INSERT) || defined(DEBUG_REMOVE)
#undef DEBUG
#define DEBUG
#endif // DEBUG_INSERT || DEBUG_REMOVE

#if defined(DEBUG_COUNT) || defined(DEBUG_NEXT) || defined(DEBUG_MALLOC)
#undef DEBUG
#define DEBUG
#endif // DEBUG_COUNT || DEBUG_NEXT || DEBUG_MALLOC

// Turn off assert(0) by default
#ifndef DEBUG
#define NDEBUG 1
#endif  // DEBUG

// If TEST_NEXT_USING_JUDY_NEXT is not defined we use GetNextKey for
// successive iterations in TestJudyNext.
// If TEST_NEXT_USING_JUDY_NEXT is defined we use the result of Judy1Next
// as input for our next iteration. One could argue this more closely
// resembles the most likely use case. So we make this the default.
#ifndef   NO_TEST_NEXT_USING_JUDY_NEXT
  #undef     TEST_NEXT_USING_JUDY_NEXT
  #define    TEST_NEXT_USING_JUDY_NEXT
#endif // NO_TEST_NEXT_USING_JUDY_NEXT

#include <assert.h>                     // assert()

#include <Judy.h>                       // for Judy macros J*()

//=======================================================================
//             R A M   M E T R I C S
//=======================================================================
//  For figuring out how much different structures contribute.   Must be
//  turned on in source compile of Judy with -DRAMMETRICS

// This Judy1LHTime program uses RAMMETRICS globals which are, at present,
// defined conditionally in the Judy library in JudyMalloc.c.
// The Judy 1.0.5 library doesn't define the RAMMETRICS globals but we'd like
// to be able link this modern Judy1LHTime with the Judy 1.0.5 library.
// Use -URAMMETRICS to define the variables in the
// Judy1LHtime program, itself, in order to link this modern Judy1LHTime
// program with a Judy 1.0.5 library or build it in a Judy 1.0.5 working
// directory or build it with a modern Judy library with -URAMMETRICS.
#ifdef RAMMETRICS
#define RM_EXTERN extern
#else // RAMMETRICS
#define RM_EXTERN
#endif // RAMMETRICS
RM_EXTERN Word_t j__MmapWordsTOT; // allocated by mmap
RM_EXTERN Word_t j__MFlag; // print mmap and munmap calls on stderr
RM_EXTERN Word_t j__MalFreeCnt; // count of JudyMalloc + JudyFree calls
RM_EXTERN Word_t j__AllocWordsTOT;
RM_EXTERN Word_t j__AllocWordsJBB;
RM_EXTERN Word_t j__AllocWordsJBU;
RM_EXTERN Word_t j__AllocWordsJBL;
RM_EXTERN Word_t j__AllocWordsJLL[8];
//#define j__AllocWordsJLLW  j__AllocWordsJLL[0]
//#define j__AllocWordsJLL1  j__AllocWordsJLL[1]
//...
//#define j__AllocWordsJLL7  j__AllocWordsJLL[7]
RM_EXTERN Word_t j__AllocWordsJLB1;
RM_EXTERN Word_t j__AllocWordsJV;

// DSMETRICS_HITS ==> Derive DirectHits from GetCalls, j__NotDirectHits,
// j__GetCallsP and j__GetCallsM.
#ifdef DSMETRICS_HITS
  #ifdef DSMETRICS_NHITS
    #error DSMETRICS_HITS and DSMETRICS_NHITS are mutually exclusive.
  #endif // DSMETRICS_HITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_HITS

// DSMETRICS_NHITS ==> Derive NotDirectHits from GetCalls, j__DirectHits,
// j__GetCallsP and j__GetCallsM.
#ifdef DSMETRICS_NHITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_NHITS

// DSMETRICS_GETS ==> Derive GetCalls from Meas/Elements and j__GetCallsNot.
#ifdef DSMETRICS_GETS
  #undef  SEARCHMETRICS
  #define SEARCHMETRICS
#endif // DSMETRICS_GETS

#ifdef SEARCHMETRICS
#define SM_EXTERN extern
#else // SEARCHMETRICS
#define SM_EXTERN
#endif // SEARCHMETRICS
SM_EXTERN Word_t j__SearchPopulation; // Sum of pops searched of counted Gets.
// Num counted Gets that did not modify j__SearchPopulation.
SM_EXTERN Word_t j__GetCallsSansPop;
SM_EXTERN Word_t j__GetCallsNot; // Num Gets not counted or instrumented.
SM_EXTERN Word_t j__GetCalls; // Num instrumented Gets.
SM_EXTERN Word_t j__DirectHits; // Num direct hits - found key on first try.
// Num Gets with no direct hit not counted in j__GetCalls[PM].
SM_EXTERN Word_t j__NotDirectHits;
// Num Gets with no direct hit and resulting in forward search.
SM_EXTERN Word_t j__GetCallsP;
// Num Gets with no direct hit and resulting in backward search.
SM_EXTERN Word_t j__GetCallsM;
SM_EXTERN Word_t j__MisComparesP; // Sum of miscompares j__GetCallsP.
SM_EXTERN Word_t j__MisComparesM; // Sum of miscompares j__GetCallsM.

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

// The released Judy libraries do not, and some of Doug's work-in-progress
// libraries may not, have Judy[1L]Dump entry points.
// And Mike sometimes links Judy1LHTime with his own Judy1 library and the
// released or Doug's JudyL library, or links Judy1LHTime with his own JudyL
// library and the released or Doug's Judy1 library.
// We want to be able to use the same Judy1LHTime.c for all of these cases.
// The solution is to define JUDY1_V2 and/or JUDY1_DUMP when we want
// Judy1LHTime to use Judy1Dump for real. And to define JUDYL_V2
// and/or JUDYL_DUMP if/when we want Judy1LHTime to use JudyLDump for real.

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
//      T I M I N G   M A C R O S
//=======================================================================

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

// (D) is returned in nano-seconds from last STARTTm
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

int       TestJudyIns(void **J1, void **JL, PNewSeed_t PSeed,
                      Word_t Elems);

void      TestJudyLIns(void **JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudyDup(void **J1, void **JL, PNewSeed_t PSeed,
                      Word_t Elems);

int       TestJudyDel(void **J1, void **JL, PNewSeed_t PSeed,
                      Word_t Elems);

static inline int
TestJudyGet(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems,
            Word_t Tit, Word_t KFlag, Word_t hFlag, int bLfsrOnly);

void      TestJudyLGet(void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudy1Copy(void *J1, Word_t Elem);

int       TestJudyCount(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

Word_t    TestJudyNext(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

int       TestJudyPrev(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elems);

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

// PrintValFF - PrintValFreeForm
// Print dVal in nWidth provided with a format that fits with as
// much precision as possible.
static void
PrintValFF(double dVal, // raw value to be scaled, formatted and printed
           int nWidth, // field width for printed number
           int bUseSymbol // use engineering notation prefix symbol
           )
{
    char acFormat[24];
    double dExpWidth = pow(10, nWidth);
    if ((dVal > .05) && (dVal <= pow(10, nWidth - 2) - .5)) {
        // decimal point and no exponent
        sprintf(acFormat, "%%%d.%df", nWidth, nWidth - 2 - (int)log10(dVal));
         printf(acFormat, dVal);
    } else
    if ((dVal > pow(10, nWidth - 2) - .5) && (dVal <= pow(10, nWidth) + .5)) {
        // no decimal point or exponent
        sprintf(acFormat, "%%%dd", nWidth); // minimum field width
         printf(acFormat, (unsigned long)dVal);
    } else
    if (dVal < 5 / pow(10, nWidth)) {
        // Must use a variant of scientific notation with a negative exponent.
        if (dVal < 1e-9) {
            sprintf(acFormat, "%%%dd", nWidth); // minimum field width
             printf(acFormat, 0);
        } else {
            sprintf(acFormat, "%%%d.0e", nWidth); // minimum field width
             printf(acFormat, dVal);
        }
    } else /* if (dVal >= dExpWidth - .5) */ {
        // Must use a variant of scientific notation with a positive exponent.
        // Normalized scientific notation: 1 <= abs(significant) < 10.
        // Engineering notation: exponents are multiples of 3.
        // The '=' above is necessary to avoid assertion below.
        // How many digits of exponent are needed?
        //  999e9 is dMax for 5 characters with 1 digit  of exponent
        //        it represents up to  999.5e9
        // 9999e9 is dMax for 6 characters with 1 digit  of exponent
        //        it represents up to 9999.5e9
        //  99e99 is dMax for 5 characters with 2 digits of exponent
        //        it represents up to  99.5e99
        // 999e99 is dMax for 6 characters with 2 digits of exponent
        //        it represents up to 999.5e99
        // Does GNUplot understand SI prefixes, e.g. k, M, G, T? No.
        int nDigitsSig = nWidth - 2; // no decimal point in significand
        int nDigitsExp = 1; // arbitrary -- ok for now
        double dMaxE1 = (pow(10, nDigitsSig) - .5) * pow(10, pow(10, nDigitsExp) - 1);
        if (dVal >= dMaxE1) { // too big for one-digit exponent
            sprintf(acFormat, "%%%d.0e", nWidth);
             printf(acFormat, dVal);
        } else { // one-digit exponent
            int nExp = (int)log10(dVal) - nDigitsSig + 1;
            int nSig = dVal / pow(10, nExp) + .5;
            if (log10(nSig) >= nWidth - nDigitsExp - 1) {
                ++nExp; nSig /= 10;
            }
            int nEngMod = nExp % 3; (void)nEngMod;
            if ((dVal < dExpWidth - .5)
                || (!bUseSymbol && (nEngMod == 0))
                || (!bUseSymbol
                    && (((nWidth < 5) && (nEngMod == 1))
                        || (nWidth < 4))))
            {
                sprintf(acFormat, "%%%dde%%d", nWidth - nDigitsExp - 1);
                printf(acFormat, nSig, nExp);
            }
            else
            {
                int nExpAdj = (nEngMod == 0) ? 0 : 3 - nEngMod;
                nExp += nExpAdj;
                double dSig = (double)nSig / pow(10, nExpAdj);
                if (bUseSymbol) {
                    sprintf(acFormat, "%%%d.%df%s", nWidth - 1, nExpAdj,
                            nExp == 9 ? "G" : nExp == 6 ? "M" : "K");
                     printf(acFormat, dSig);
                } else {
                    // GNUplot can't handle 'k', 'M', 'G', ...
                    sprintf(acFormat, "%%%d.%dfe%%d", nWidth - nDigitsExp - 1, 3 - nEngMod - 1);
                     printf(acFormat, dSig, nExp);
                }
            }
        }
    }
}

// Print a space before the value.
#define PrintValFFX(a, b, c)  { putchar(' '); PrintValFF(a, b, c); }

// nWidth and nDigitsFraction specify the desired format
static void
PrintValX(double dVal, // raw value to be scaled, formatted and printed
          int nWidth, // field width for printed number
          int nDigitsFraction, // number of digits after the decimal point
          int bUseSymbol, // use engineering notation prefix symbol
          const char *strPrefix, // field separator printed before number
          double dScale // multiply dVal by dScale prior to format and print
          )
{
    fputs(strPrefix, stdout);
    dVal *= dScale;
    char acFormat[16];
    int nLogVal = log10(dVal);
    if ((nDigitsFraction > 0) && (nLogVal < nWidth - nDigitsFraction - 1)) {
        sprintf(acFormat, "%%%d.%df", nWidth, nDigitsFraction);
         printf(acFormat, dVal);
    } else if ((nDigitsFraction == 0) && (nLogVal < nWidth)) {
        sprintf(acFormat, "%%%dlu", nWidth);
         printf(acFormat, (unsigned long)dVal);
    } else {
        PrintValFF(dVal, nWidth, bUseSymbol);
    }
}

static void
PrintVal(double dVal, // raw value to be scaled, formatted and printed
         int nWidth, // field width for printed number
         int nDigitsFraction // number of digits after the decimal point
         )
{
    PrintValX(dVal, nWidth, nDigitsFraction, /* bUseSymbol */ 0,
              /* strPrefix */ " ", /* dScale */ 1);
}

static void
PrintValx100(double dVal, // raw value to be scaled, formatted and printed
             int nWidth, // field width for printed number
             int nDigitsFraction // number of digits after the decimal point
             )
{
    PrintValX(dVal, nWidth, nDigitsFraction, /* bUseSymbol */ 0,
              /* strPrefix */ " ", /* dScale */ 100);
}

#define PRINT5_1f(__X)  PrintValx100((__X), 5, 1)

static void
PRINT6_1f(double __X)
{
    if (__X >= .05)
        printf(" %6.1f", __X);
    else
        printf("      0");              // keep white space cleaner
}

#define DONTPRINTLESSTHANZERO(A,B)  PrintVal((A) - (B), 6, 1)

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

#define DEFAULT_TVALUES  1000000

// wDoTit0Max is max Delta or Meas for which we do Tit=0 phase.
// Use wDoTit0Max == 0 to skip overhead measurements altogether.
// Use wDoTit0Max == -1 to disable overhead smoothing between deltas.
// Use wDoTit0Max = DEFAULT_TVALUES / 2 to make sure we get to a place where we quit.
//Word_t wDoTit0Max = 0; // Should be fastest.
//Word_t wDoTit0Max = (Word_t)-1; // Should be slowest.
Word_t wDoTit0Max = DEFAULT_TVALUES / 2;

Word_t    Judy1Dups =  0;
Word_t    JudyLDups =  0;
Word_t    BitmapDups = 0;
Word_t    ByteDups   = 0;

Word_t    J1Flag = 0;                   // time Judy1
Word_t    JLFlag = 0;                   // time JudyL
Word_t    JRFlag = 0;                   // time JudyL with no cheat
Word_t    dFlag = 0;                    // time Judy1Unset JudyLDel
Word_t    vFlag = 0;                    // time Searching
Word_t    CFlag = 0;                    // time Counting
Word_t    cFlag = 0;                    // time Copy of Judy1 array
Word_t    IFlag = 0;                    // time duplicate inserts/sets
Word_t    bFlag = 0;                    // Time REAL bitmap of (2^-B #) in size
Word_t    Warmup = 1;                   // milliseconds to warm up CPU

PWord_t   B1 = NULL;                    // BitMap
#define cMaxColon ((int)sizeof(Word_t) * 2)  // Maximum -b suboption paramters
int       bParm[cMaxColon + 1] = { 0 }; // suboption parameters to -b#:#:# ...
Word_t    yFlag = 0;                    // Time REAL Bytemap of (2^-B #) in size
uint8_t  *By;                           // ByteMap

Word_t    mFlag = 0;                    // words/Key for all structures
Word_t    pFlag = 0;                    // Print number set
Word_t    lFlag = 0;                    // do not do multi-insert tests

Word_t    gFlag = 0;                    // do Get check(s) after Ins/Del
Word_t    GFlag = 0;                    // do infrequent narrow pointer checking
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
Word_t    wSplayBase = 0; // What to put in key at wSplayMask zero bits.
#if defined(__LP64__) || defined(_WIN64)
Word_t wSplayMask = 0x5555555555555555; // default splay mask
//Word_t wSplayMask = 0xeeee00804020aaff;
#else // defined(__LP64__) || defined(_WIN64)
Word_t wSplayMask = 0x55555555;         // default splay mask
//Word_t wSplayMask = 0xff00ffff;
#endif // defined(__LP64__) || defined(_WIN64)
Word_t    wCheckBits = 0;               // Bits for narrow pointer testing.

Word_t TValues = DEFAULT_TVALUES; // Maximum numb retrieve timing tests

// nElms is the total number of keys that are inserted into the test arrays.
// Default nElms is overridden by -n or -F.
// Looks like there may be no protection against -F followed by -n.
// It is then trimmed to MaxNumb.
// Should it be MaxNumb+1 in cases that allow 0, e.g. non-zero SValue?
// And trimmed if (GValue != 0) && (nElms > (MaxNumb >> 1)) to (MaxNumb >> 1).
// It is used for -p
// and to override TValues if TValues == 0 or nElms < TValues.
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
// DEFAULT_BVALUE of size of word makes it easier to design a regression test
// that doesn't know or care if it is running 64-bit or 32-bit.
// For example, we can use negative numbers as option arguments on the
// command line for -s to get numbers near the maximum of the expanse.
#define DEFAULT_BVALUE  (sizeof(Word_t) * 8)
Word_t    BValue = DEFAULT_BVALUE;
double    Bpercent = 100.0; // Default MaxNumb assumes 100.0.
// MaxNumb is initialized from DEFAULT_BVALUE and assumes Bpercent = 100.0.
// Then overridden by -N and/or by -B. The last one on the command line wins.
// Then trimmed if bSplayKeyBits
// and there aren't enough bits set in wSplayMask.
// MaxNumb is used to put an upper bound on RandomNumb values used in
// CalcNextKey, but -D, --splay-key-bits and Offset can all result in final
// key values getting bigger than MaxNumb.
// MaxNumb is also used to specifiy -b and -y array sizes.
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

// StartSequent is the starting seed value for the key generator.
// It is changed to SValue for non-zero SValue unless -s is used to set it
// explicitly. We set it to SValue for non-zero SValue by default so we can
// do random gets with non-zero SValue by default.
//
Word_t StartSequent = (Word_t)-1 / 3; // 0x5555555555555555 or 0x55555555

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

// CalcNextKeyX returns the next key.
// We use this same function for non-zero SValue inserts and for random gets.
// The globals, BValue and SValue, are used for inserts.
// The caller passes in nBValueArg == wLogPop1 and wSValueArg == 0 when it
// wants a random get and global SValue != 0.
//
static inline Word_t
CalcNextKeyX(PSeed_t PSeed, int nBValueArg, Word_t wSValueArg)
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
            Key = RandomNumb(PSeed, wSValueArg);
            if ((sizeof(Word_t) * 8) != nBValueArg)
            {
                 assert((Key < ((Word_t)1 << nBValueArg)) || wSValueArg);
            }
#ifndef NO_TRIM_EXPANSE
        } while (Key > MaxNumb);         // throw away of high keys
#endif // NO_TRIM_EXPANSE
    }

    if (wSValueArg /* local */ != /* global */ SValue) {
        Key *= SValue;
        //Key &= ((((Word_t)1 << (BValue - 1)) << 1) - 1);
        Key &= (Word_t)-1 >> ((sizeof(Word_t) * 8) - BValue);
    }

#ifdef DEBUG
    if (pFlag)
    {
        printf("Numb %016" PRIxPTR" ", Key);
    }
#endif // DEBUG

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
        Key >>= (sizeof(Word_t) * 8) - BValue; // global BValue
#ifdef DEBUG
        if (pFlag)
        {
            printf("Shift %016" PRIxPTR" ", Key);
        }
#endif // DEBUG

        // Key will be smaller than 1 << BValue, but it might be bigger
        // than MaxNumb.
    }
#endif // NO_DFLAG

#ifndef NO_SPLAY_KEY_BITS
    if (bSplayKeyBitsFlag) {
        // Splay the bits in the key.
        Key = MyPDEP(Key, wSplayMask);
#ifdef DEBUG
        if (pFlag)
        {
            printf("PDEP %016" PRIxPTR" ", Key);
        }
#endif // DEBUG
        Key |= wSplayBase & ~wSplayMask;
        // Key might be bigger than ((1 << BValue) - 1) -- by design.
    }
#endif // NO_SPLAY_KEY_BITS

#ifndef NO_OFFSET
    Key = Key + Offset;
    // Key might be bigger than ((1 << BValue) - 1) -- by design.
#endif // NO_OFFSET
#ifdef DEBUG
    if (pFlag) { printf("Key "); }
#endif // DEBUG
    return Key;
}

static inline Word_t
CalcNextKey(PSeed_t PSeed)
{
    return CalcNextKeyX(PSeed, BValue, SValue);
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
PrintHeaderX(const char *strFirstCol, int nRow)
{
    printf("# %4s", strFirstCol);

    printf(nRow ? "   Ins" : " Delta");
    printf(nRow ? "  Meas" : "   Get");

    if (J1Flag)
        printf(nRow ? "      " : "   J1S");
    if (JLFlag)
        printf(nRow ? "      " : "   JLI");
    if (JRFlag)
        printf(nRow ? "      " : " JLI-R");
    if (bFlag)
        printf(nRow ? "      " : " BMSet");
    if (yFlag)
        printf(nRow ? "      " : " BySet");
    if (tFlag)
        printf(nRow ? "      " : (J1Flag|bFlag|yFlag) ? " SetOv" : " InsOv");
    if (J1Flag)
        printf(nRow ? "      " : "   J1T");
    if (JLFlag)
        printf(nRow ? "      " : "   JLG");
    if (JRFlag)
        printf(nRow ? "      " : " JLG-R");
    if (bFlag)
        printf(nRow ? "      " : " BMTst");
    if (yFlag)
        printf(nRow ? "      " : " ByTst");
    if (tFlag)
        printf(nRow ? "      " : (J1Flag|bFlag|yFlag) ? " TstOv" : " GetOv");

    if (IFlag)
    {
        if (J1Flag)
            printf(nRow ? "   J1S" : "   DUP");
        if (JLFlag)
            printf(nRow ? "   JLI" : "   DUP");
    }

    if (cFlag)
    {
        printf(nRow ? "       " : " CopyJ1");
    }

    if (CFlag)
    {
        if (J1Flag)
            printf(nRow ? "      " : "   J1C");
        if (JLFlag)
            printf(nRow ? "      " : "   JLC");
    }
    if (vFlag)
    {
        if (J1Flag)
            printf(nRow ? "      " : "   J1N");
        if (JLFlag)
            printf(nRow ? "      " : "   JLN");
        if (J1Flag)
            printf(nRow ? "      " : "   J1P");
        if (JLFlag)
            printf(nRow ? "      " : "   JLP");
        if (J1Flag)
            printf(nRow ? "      " : "  J1NE");
        if (JLFlag)
            printf(nRow ? "      " : "  JLNE");
        if (J1Flag)
            printf(nRow ? "      " : "  J1PE");
        if (JLFlag)
            printf(nRow ? "      " : "  JLPE");
    }

    if (dFlag)
    {
        if (J1Flag)
            printf(nRow ? "      " : "   J1U");
        if (JLFlag)
            printf(nRow ? "      " : "   JLD");
    }

#ifdef RAMMETRICS
    if (J1Flag | JLFlag | JRFlag) {
        printf(nRow ? "  %%W/K" : "  Heap");
    }
#endif // RAMMETRICS

    if (mFlag && (bFlag == 0) && (yFlag == 0))
    {
        printf(nRow ? "      " : " JBB/K");
        printf(nRow ? "      " : " JBU/K");
        printf(nRow ? "      " : " JBL/K");

        printf(nRow ? "      " : " LWd/K");

        printf(nRow ? "      " : "  L7/K");
        printf(nRow ? "      " : "  L6/K");
        printf(nRow ? "      " : "  L5/K");
        printf(nRow ? "      " : "  L4/K");
        printf(nRow ? "      " : "  L3/K");
        printf(nRow ? "      " : "  L2/K");
        printf(nRow ? "      " : "  L1/K");
        printf(nRow ? "      " : "  B1/K");
#if defined(MIKEY_1)
        // Doug's code doesn't have a B2 yet.
        if (J1Flag) {
            printf(nRow ? "      " : "  B2/K"); // big bitmap leaf words per key
        } else
#endif // defined(MIKEY_1)
        printf(nRow ? "      " : "  JV/K");

        printf(nRow ? "      " : " %%MEff");

        if (J1Flag)
            printf(nRow ? "      " : " MF1/K");
        if (JLFlag || JRFlag)
            printf(nRow ? "      " : " MFL/K");
#ifdef SEARCHMETRICS
        printf(nRow ? "      " : " +MsCm");
        printf(nRow ? "      " : " -MsCm");
        printf(nRow ? " Lenth" : " Srch ");
        printf(nRow ? "      " : " %%DiHt");
        printf(nRow ? "      " : " %%GetP");
        printf(nRow ? "      " : " %%GetM");
        printf(nRow ? "   Cnt" : "  Gets");
#endif // SEARCHMETRICS
    }

    printf("\n");
}

static void
PrintHeader(const char *strFirstCol)
{
    PrintHeaderX(strFirstCol, /* nRow */ 0);
    PrintHeaderX("",          /* nRow */ 1);
}

static void
Usage(int argc, char **argv)
{
    (void) argc;

    printf("\n<<< Program to do performance measurements (and Diagnostics) on Judy Arrays >>>\n\n");
    printf("%s -n# -P# -S# [-B#[:#]|-N#] -T# -1LRbY -gGIcCvdtDlMK... -F <filename>\n\n", argv[0]);
    printf("   Where: # is a number, default is shown as [#]\n\n");
    printf("-m #  Output measurements when libJudy is compiled with -DRAMMETRICS &| -DSEARCHMETRICS\n");
    printf("-n #  Number of Keys (Population) used in Measurement [10000000]\n");
    printf("-P #  Measurement points per decade (1..1000) [50] (231 = ~1%%, 46 = ~5%% delta Population)\n");
    printf("-S #  Key Generator skip amount, 0 = Random [0]\n");
    printf("-B #  Significant bits output (16..64) in Random Key Generator [32]\n");
    printf("-B #:#  Second # is percent expanse is limited [100]\n");
    printf("-N #  max key #; alternative to -B\n");
    printf("-E,--splay-key-bits [splay-mask|:]"
           "  Splay key bits with default splay-mask 0x55..55\n");
    printf("  ':' means fill the holes with ones instead of zeros\n");
    printf("-e [splay-mask][:]"
             "  Splay key bits with default splay-mask 0xaa..aa\n");
    printf("  ':' means fill the holes with ones instead of zeros\n");
    printf("-l    Do not smooth data with iteration at low (<100) populations (Del/Unset not called)\n");
    printf("-F <filename>  Ascii file of Keys, zeros ignored -- must be last option!!!\n");
//    printf("-b #:#:# ... 1st number required [1] where each number is next level of tree\n");
    printf("-b    Time a BITMAP array  (-B # specifies size == 2^# bits) [-B32]\n");
    printf("-y    Time a BYTEMAP array (-B # specifies size == 2^# bytes) [-B32]\n");
    printf("-1    Time Judy1\n");
    printf("-L    Time JudyL\n");
    printf("-R    Time JudyL using *PValue as next TstKey\n");
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
    printf("-g     Do Get/Test sanity check(s) after every Ins/Set/Del/Unset (adds to times)\n");
    printf("-G     Do infrequent narrow pointer checking\n");
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
    (void)base;
    char *lendptr;
    Word_t ul;

    if ((str == NULL) || *str == '\0') {
        printf("\nError --- Illegal optarg, \"\", for option \"-%c\".\n", ch);
        exit(1);
    }

    errno = 0;

    long double ld = strtold(str, &lendptr);
    ul = (Word_t)ld;

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
    { "splay-key-bits", optional_argument, NULL, 'E' },

    // Long option '--lfsr-only' is equivalent to short option '-k'.
    { "lfsr-only", no_argument, NULL, 'k' },

    // Last struct option in array must be filled with zeros.
    { NULL, 0, NULL, 0 }
};

Word_t    MisComparesP;            // number times LGet/1Test called
Word_t    MisComparesM;            // number times LGet/1Test called
Word_t    DirectHits;               // Number of direct hits -- no search
Word_t    NotDirectHits; // not counted in GetCallsP or GetCallsM
Word_t    SearchPopulation;         // Population of Searched object
Word_t    GetCallsSansPop;
Word_t    GetCallsP;                 // number of search calls
Word_t    GetCallsM;                 // number of search calls
Word_t    GetCalls;                  // number of search calls

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

static int bPureDS1 = 0; // Shorthand to trigger pure -DS1 special behaviors.
// Get random keys for TestJudyGet (w/o random inserts).
static int bRandomGets = 0;

#define MIN(_a, _b)  ((_a) < (_b) ? (_a) : (_b))
#define MAX(_a, _b)  ((_a) > (_b) ? (_a) : (_b))

// Log ifdefs to make plot files more self-describing.
// Sort lexicographically ignoring leading underscores.
void
LogIfdefs(void)
{
    printf("# === Time program ifdefs ===\n");

    // BITMAP_BY_BYTE

  #ifdef         CALC_NEXT_KEY
    printf("#    CALC_NEXT_KEY\n");
  #else //       CALC_NEXT_KEY
    printf("# No CALC_NEXT_KEY\n");
  #endif //      CALC_NEXT_KEY else

    // CLEVER_RANDOM_KEYS

  #ifdef         DEBUG
    printf("#    DEBUG\n");
  #else //       DEBUG
    printf("# No DEBUG\n");
  #endif //      DEBUG else

  #ifdef         DEBUG_ALL
    printf("#    DEBUG_ALL\n");
  #else //       DEBUG_ALL
    printf("# No DEBUG_ALL\n");
  #endif //      DEBUG_ALL else

    // __APPLE__
    // __MACH__

  #ifdef         DSMETRICS_GETS
    printf("#    DSMETRICS_GETS\n");
  #else //       DSMETRICS_GETS
    printf("# No DSMETRICS_GETS\n");
  #endif //      DSMETRICS_GETS else

  #ifdef         DSMETRICS_HITS
    printf("#    DSMETRICS_HITS\n");
  #else //       DSMETRICS_HITS
    printf("# No DSMETRICS_HITS\n");
  #endif //      DSMETRICS_HITS else

  #ifdef         DSMETRICS_NHITS
    printf("#    DSMETRICS_NHITS\n");
  #else //       DSMETRICS_NHITS
    printf("# No DSMETRICS_NHITS\n");
  #endif //      DSMETRICS_NHITS else

    // EXTRA_SLOW_PDEP
    // FANCY_b_flag
    // FANCY_b_FLAG

  #ifdef         JUDY1_DUMP
    printf("#    JUDY1_DUMP\n");
  #else //       JUDY1_DUMP
    printf("# No JUDY1_DUMP\n");
  #endif //      JUDY1_DUMP else

  #ifdef         JUDY1_V2
    printf("#    JUDY1_V2\n");
  #else //       JUDY1_V2
    printf("# No JUDY1_V2\n");
  #endif //      JUDY1_V2 else

  #ifdef         JUDYL_DUMP
    printf("#    JUDYL_DUMP\n");
  #else //       JUDYL_DUMP
    printf("# No JUDYL_DUMP\n");
  #endif //      JUDYL_DUMP else

  #ifdef         JUDYL_V2
    printf("#    JUDYL_V2\n");
  #else //       JUDYL_V2
    printf("# No JUDYL_V2\n");
  #endif //      JUDYL_V2 else

  #ifdef         KFLAG
    printf("#    KFLAG\n");
  #else //       KFLAG
    printf("# No KFLAG\n");
  #endif //      KFLAG else

  #ifdef         LFSR_GET_FOR_DS1
    printf("#    LFSR_GET_FOR_DS1\n");
  #else //       LFSR_GET_FOR_DS1
    printf("# No LFSR_GET_FOR_DS1\n");
  #endif //      LFSR_GET_FOR_DS1 else

  #ifdef         LFSR_ONLY
    printf("#    LFSR_ONLY\n");
  #else //       LFSR_ONLY
    printf("# No LFSR_ONLY\n");
  #endif //      LFSR_ONLY else

    // LOOKUP_NO_BITMAP_DEREF
    // __LP64__
    // _WIN64
    // MAP_ANONYMOUS

  #ifdef         MIKEY_1
    printf("#    MIKEY_1\n");
  #else //       MIKEY_1
    printf("# No MIKEY_1\n");
  #endif //      MIKEY_1 else

  #ifdef         MIKEY_L
    printf("#    MIKEY_L\n");
  #else //       MIKEY_L
    printf("# No MIKEY_L\n");
  #endif //      MIKEY_L else

  #ifdef         NDEBUG
    printf("#    NDEBUG\n");
  #else //       NDEBUG
    printf("# No NDEBUG\n");
  #endif //      NDEBUG else

    // NO_DFLAG
    // NO_FVALUE
    // NO_KFLAG
    // NO_OFFSET
    // NO_PRESERVED_KEY
    // NO_SPLAY_KEY_BITS
    // NO_SVALUE

  #ifdef         NO_TEST_NEXT_EMPTY // for turn-on
    printf("#    NO_TEST_NEXT_EMPTY\n");
  #else //       NO_TEST_NEXT_EMPTY
    printf("# No NO_TEST_NEXT_EMPTY\n");
  #endif //      NO_TEST_NEXT_EMPTY else

  #ifdef         NO_TEST_NEXT // for turn-on
    printf("#    NO_TEST_NEXT\n");
  #else //       NO_TEST_NEXT
    printf("# No NO_TEST_NEXT\n");
  #endif //      NO_TEST_NEXT else

  #ifdef         NO_TEST_NEXT_USING_JUDY_NEXT
    printf("#    NO_TEST_NEXT_USING_JUDY_NEXT\n");
  #else //       NO_TEST_NEXT_USING_JUDY_NEXT
    printf("# No NO_TEST_NEXT_USING_JUDY_NEXT\n");
  #endif //      NO_TEST_NEXT_USING_JUDY_NEXT else

    // NO_TRIM_EXPANSE

  #ifdef         OLD_DS1_GROUPS
    printf("#    OLD_DS1_GROUPS\n");
  #else //       OLD_DS1_GROUPS
    printf("# No OLD_DS1_GROUPS\n");
  #endif //      OLD_DS1_GROUPS else

  #ifdef         RAMMETRICS
    printf("#    RAMMETRICS\n");
  #else //       RAMMETRICS
    printf("# No RAMMETRICS\n");
  #endif //      RAMMETRICS else

  #ifdef         SEARCHMETRICS
    printf("#    SEARCHMETRICS\n");
  #else //       SEARCHMETRICS
    printf("# No SEARCHMETRICS\n");
  #endif //      SEARCHMETRICS else

    // SKIPMACRO
    // SLOW_PDEP
    // SYNC_SYNC
    // EXTRA_SLOW_PDEP

  #ifdef         TEST_COUNT_USING_JUDY_NEXT
    printf("#    TEST_COUNT_USING_JUDY_NEXT\n");
  #else //       TEST_COUNT_USING_JUDY_NEXT
    printf("# No TEST_COUNT_USING_JUDY_NEXT\n");
  #endif //      TEST_COUNT_USING_JUDY_NEXT else

  #ifdef         TEST_NEXT_USING_JUDY_NEXT
    printf("#    TEST_NEXT_USING_JUDY_NEXT\n");
  #else //       TEST_NEXT_USING_JUDY_NEXT
    printf("# No TEST_NEXT_USING_JUDY_NEXT\n");
  #endif //      TEST_NEXT_USING_JUDY_NEXT else

  #ifdef         USE_MALLOC
    printf("#    USE_MALLOC\n");
  #else //       USE_MALLOC
    printf("# No USE_MALLOC\n");
  #endif //      USE_MALLOC else

  #ifdef         USE_PDEP_INTRINSIC
    printf("#    USE_PDEP_INTRINSIC\n");
  #else //       USE_PDEP_INTRINSIC
    printf("# No USE_PDEP_INTRINSIC\n");
  #endif //      USE_PDEP_INTRINSIC else

    printf("# === End of Time program ifdefs ===\n");
}

void
PrintRowHeader(Word_t wPop, Word_t wDelta, Word_t wMeas)
{
#if 0
    if (bPureDS1) {
        printf(" 0x%-10" PRIxPTR" 0x%-8" PRIxPTR" %10" PRIuPTR,
        //printf("%6" PRIxPTR" %5" PRIxPTR" %5" PRIuPTR,
               wFinalPop1, Pms[grp].ms_delta, Meas);
    } else
#endif
    {
        PrintValX(wPop,   6, 0, /*bUseSymbol*/ 0, /*strPrefix*/ "" , /*dScale*/ 1);
        PrintValX(wDelta, 5, 0, /*bUseSymbol*/ 1, /*strPrefix*/ " ", /*dScale*/ 1);
        PrintValX(wMeas,  5, 0, /*bUseSymbol*/ 1, /*strPrefix*/ " ", /*dScale*/ 1);
    }
}

int
main(int argc, char *argv[])
{
//  Names of Judy Arrays
#ifdef DEBUG
    // Make sure the word before and after J1's root word is zero. It's
    // pretty easy in some variants of Mikey's code to introduce a bug that
    // clobbers one or the other so his code depends on these words being
    // zero so it can verify that neither is getting clobbered.
    struct { void *pv0, *pv1, *pv2; } sj1 = { (void*)-1, NULL, (void*)-1 };
#define J1 (sj1.pv1)
    struct { void *pv0, *pv1, *pv2; } sjL = { (void*)-1, NULL, (void*)-1 };
#define JL (sjL.pv1)
#else // DEBUG
    void     *J1 = NULL;                // Judy1
    void     *JL = NULL;                // JudyL
#endif // DEBUG

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
    Word_t    LittleOffset = 0;
    Word_t    BigOffset = 0;

    int       Col;
    int       c;
    extern char *optarg;

#ifdef LATER
    double    Dmin = 9999999999.0;
    double    Dmax = 0.0;
    double    Davg = 0.0;
#endif // LATER

    // Validate static initialization of MaxNumb, BValue and Bpercent.
    assert(MaxNumb == (Word_t)(pow(2.0, BValue) * Bpercent / 100) - 1);

    setbuf(stdout, NULL);               // unbuffer output

#if 0
    // Different get time functions on Linux.
    {
        struct timespec tv;

        if (clock_getres(CLOCK_REALTIME, &tv) == 0) {
            printf("getres(CLOCK_REALTIME)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_getres(CLOCK_MONOTONIC, &tv) == 0) {
            printf("getres(CLOCK_MONTONIC)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_getres(CLOCK_MONOTONIC_RAW, &tv) == 0) {
            printf("getres(CLOCK_MONTONIC_RAW)       %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_getres(CLOCK_BOOTTIME, &tv) == 0) {
            printf("getres(CLOCK_BOOTTIME)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tv) == 0) {
            printf("getres(CLOCK_PROCESS_CPUTIME_ID) %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_getres(CLOCK_THREAD_CPUTIME_ID, &tv) == 0) {
            printf("getres(CLOCK_THREAD_CPUTIME_ID)  %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        //if (clock_getres(CLOCK_REALTIME_COURSE, &tv) == 0) { }
        //if (clock_getres(CLOCK_MONOTONIC_COURSE, &tv) == 0) { }
        if (clock_gettime(CLOCK_REALTIME, &tv) == 0) {
            printf("clock_gettime(CLOCK_REALTIME)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_gettime(CLOCK_MONOTONIC, &tv) == 0) {
            printf("clock_gettime(CLOCK_MONTONIC)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == 0) {
            printf("clock_gettime(CLOCK_MONTONIC_RAW)       %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_gettime(CLOCK_BOOTTIME, &tv) == 0) {
            printf("clock_gettime(CLOCK_BOOTTIME)           %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv) == 0) {
            printf("clock_gettime(CLOCK_PROCESS_CPUTIME_ID) %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tv) == 0) {
            printf("clock_gettime(CLOCK_THREAD_CPUTIME_ID)  %6ld.%09ld\n", tv.tv_sec, tv.tv_nsec);
        }
        //if (clock_gettime(CLOCK_REALTIME_COURSE, &tv) == 0) { }
        //if (clock_gettime(CLOCK_MONOTONIC_COURSE, &tv) == 0) { }
    }
#endif // 0

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

    int sFlag = 0; // boolean; has -s been seen
    errno = 0;
    while (1)
    {
        c = getopt_long(argc, argv,
                        "1a:B:b"
  #ifdef FANCY_b_flag
                            ":"
  #endif // FANCY_b_flag
                            "CcDdE::e::F:fGghIiKklLMm"
                            "N:n:O:o:P:pRS:s:T:tVvW:xyZ",
                        longopts, NULL);

                // Used upper-case option letters sorted and with spaces for
                // unused letters. Unused letters are on the following line.
                //   " B:CDE::F:G I KLMN:O:P: RS:T: VW:  Z"
                //   "A          H J         Q     U   XY "
                //
                // Used lower-case option letters sorted and with spaces for
                // unused letters. Unused letters are on the following line.
                //
                //   "a:bcde::fghi klmn:o:p  s:t: v xy "
                //   "            j        qr    u w  z"
                //
                // Used option numbers sorted and with spaces for unused
                // numbers. Unused numbers are on the following line.
                //
                //   "0 23456789"
                //   " 1        "
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
Eopt:
                if ((optarg[0] >= '0') && (optarg[0] <= '9')) {
                    wSplayMask = oa2w(optarg, NULL, 0, c);
                } else if (optarg[0] == ':') {
                    wSplayBase = ~(Word_t)0;
                    if (*++optarg != '\0') {
                        goto Eopt;
                    }
                } else {
                    --optind; // rewind
                    // skip over -E and put a '-' in place
                    if (optarg != argv[optind]) {
                        argv[optind] = optarg - 1;
                        argv[optind][0] = '-';
                    }
                }
            }
#ifdef NO_SPLAY_KEY_BITS
            FAILURE("compile with -UNO_SPLAY_KEY_BITS to use -E", wSplayMask);
#endif // NO_SPLAY_KEY_BITS
            break;
        case 'e':
            bSplayKeyBitsFlag = 1;
eopt:
            if (optarg != NULL) {
                if ((optarg[0] >= '0') && (optarg[0] <= '9')) {
                    wSplayMask = oa2w(optarg, NULL, 0, c);
                } else if (optarg[0] == ':') {
                    wSplayMask <<= 1;
                    wSplayBase = ~(Word_t)0;
                    if (*++optarg != '\0') {
                        goto eopt;
                    }
                } else {
                    --optind; // rewind
                    // skip over -e and put a '-' in place
                    if (optarg != argv[optind]) {
                        argv[optind] = optarg - 1;
                        argv[optind][0] = '-';
                    }
                }
            } else {
                wSplayMask <<= 1;
            }
#ifdef NO_SPLAY_KEY_BITS
            FAILURE("compile with -UNO_SPLAY_KEY_BITS to use -e", wSplayMask);
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
            // Change default StartSequent to one for non-zero SValue.
            // This makes it possible to to random gets for -DS1.
            if ((SValue != 0) && !sFlag) {
                StartSequent = SValue;
            }
            break;
        }
        case 'T':                      // Maximum retrieve tests for timing
            TValues = oa2w(optarg, NULL, 0, c);
            break;

        case 'P':                      // measurement points per decade
            PtsPdec = oa2w(optarg, NULL, 0, c);
            break;

        case 's':
            sFlag = 1;
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

            // Allow -B0 to mean -B64 on 64-bit and -B32 on 32-bit.
            // Allow -B-1 to mean -B63 on 64-bit and -B31 on 32-bit.
            // To simplify writing shell scripts for testing that
            // are compatible with 32-bit and 64-bit.
            BValue = (BValue - 1) % (sizeof(Word_t) * 8) + 1;

            if (BValue < 10) {
                FAILURE("-B value must be at least 10; BValue", BValue);
            }

            if (Bpercent < 50.0 || Bpercent > 100.0)
            {
                ErrorFlag++;
                printf("\nError --- Bpercent = %.2f must be at least 50"
                           " and no more than 100 !!!\n",
                       Bpercent);
                break;
            }

            MaxNumb = pow(2.0, BValue) * Bpercent / 100;
            --MaxNumb; // Do this after converting back to Word_t.

            break;
        }

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

        case 'g':                      // do Get check(s) after each Ins/Del
            gFlag = 1;
            break;

        case 'G':                      // do infrequent narrow pointer checking
            GFlag = 1;
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

    // Make sure there are no rounding errors in MaxNumb calculation for
    // Bpercent values that are integral multiples of reciprocals of powers
    // of two up to (1<<10) == 1024.
    // Choose 1024 because bigger would be problematic with -B10.
    assert((MaxNumb + 1
            == ((Word_t)1 << (BValue - 10)) * (int)(Bpercent * 1024 / 100))
        || ((int)(Bpercent * 1024 / 100) == Bpercent * 1024 / 100));

    if ((   bFlag && (J1Flag|JLFlag|JRFlag|yFlag))
        || (yFlag && (J1Flag|JLFlag|JRFlag|bFlag)))
    {
        FAILURE("-b and -y don't get along with each other"
                    " nor with any of -1LR",
                0);
    }

#ifdef NO_TRIM_EXPANSE
    if (((MaxNumb + 1) & MaxNumb) != 0) {
        FAILURE("compile with -ULFSR_ONLY -UNO_TRIM_EXPANSE to use -N",
                MaxNumb);
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
            if (nBitsSet < 10) {
                FAILURE("Splay mask must have at least 10 bits set; mask",
                        wSplayMask);
            }
            BValue = nBitsSet;
            MaxNumb = ((Word_t)2 << (BValue - 1)) - 1;
            printf("\n# Trimming '-B' value to %d;"
                   " the number of bits set in <splay-mask> 0x%zx.\n",
                   (int)BValue, wSplayMask);

        }
    }

    if (GFlag)
    {
        // Calculate wCheckBits for use with GFlag later.
        wCheckBits = (((Word_t)1 << (BValue - 1)) << 1) - 1;
        if (bSplayKeyBitsFlag) {
            wCheckBits = wSplayMask;
            if (DFlag) {
                wCheckBits = Swizzle(wSplayMask);
                wCheckBits >>= (sizeof(Word_t) * 8) - LOG(wSplayMask) - 1;
            }
        }
        if (wCheckBits == (Word_t)-1) {
            FAILURE("-G requires -B value less than bits per word"
                    " or splay mask not equal to -1; bits per word",
                    sizeof(Word_t) * 8);
        }
        wCheckBits ^= (Word_t)-1;
        printf("# -G wCheckBits 0x%zx\n", wCheckBits);
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

    if (DFlag && (SValue == 1) && (StartSequent == 1) && !bSplayKeyBitsFlag
        && (Offset == 0) && (((MaxNumb + 1) & MaxNumb) == 0))
    {
        bPureDS1 = 1;
    }

    // Currently using StartSequent == 1 to trigger bRandomGets.
    // Would be better to use an explicit option so we can use StartSequent
    // for !bRandomGets also.
    if ((SValue != 0) && (StartSequent == SValue)) {
        bRandomGets = 1;
        StartSequent &= (Word_t)-1 >> (sizeof(Word_t) * 8 - BValue);
    }

    if (bLfsrOnly) {
        if (DFlag || FValue || GValue || Offset || SValue
            || (((MaxNumb + 1) & MaxNumb) != 0))
        {
            if (bPureDS1) {
#ifdef LFSR_GET_FOR_DS1
                bLfsrForGetOnly = 1;
                bLfsrOnly = 0;
#else // LFSR_GET_FOR_DS1
                FAILURE("Use -DLFSR_GET_FOR_DS1 to use -k with -DS1.", 0);
#endif // LFSR_GET_FOR_DS1
            } else {
                FAILURE("-k is not compatible with -B:DFGNOoS", 0);
            }
        }

        // We allow --splay-key-bits=0x5555555555555555 with -k/--lfsr-only
        // because we were able to tweak the lfsr code to act like -E without
        // doing a PDEP and with zero additional cost over a regular lfsr.
        // We do this by splaying an lfsr magic number seed at the outset and
        // shifting by two instead of one within the lfsr itself.
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
            printf("\n# Trimming '-s 0x%zx'", StartSequent);
            StartSequent %= MaxNumb + 1;
            printf(" to 0x%zx.\n", StartSequent);
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

  #if defined(__LP64__) || defined(_WIN64)
    // bPureDS1 group calc code can't handle nElms > (0x11 << 56). Why?
    if (bPureDS1 && (nElms > ((Word_t)0x11 << 56))) {
        nElms = (Word_t)0x11 << 56;
        printf("# Trim Max number of Elements -n%" PRIuPTR
                   " due to bRandomGets groups limitation.\n",
               nElms);
        fprintf(stderr,
                "# Trim Max number of Elements -n%" PRIuPTR
                    " due to bRandomGets groups limitation.\n",
                nElms);
    }
  #endif // defined(__LP64__) || defined(_WIN64)

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

    printf("# StartSequent 0x%zx\n", StartSequent);

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
            printf("%10zd ", ii);
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
    if (GFlag)
        printf("G");
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
    if (!PartitionDeltaFlag)
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
    printf(" -n%" PRIuPTR" -T%" PRIuPTR" -P%" PRIuPTR, nElms, TValues, PtsPdec);
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

    printf("\n");

    {
        int       count = 0;
        if (JLFlag)
            count++;
        if (JRFlag)
            count++;
        if (J1Flag)
            count++;
        if (bFlag)
            count++;
        if (yFlag)
            count++;

        if (mFlag && (count != 1))
        {
            printf
                (" ========================================================\n");
            printf
                (" Sorry, '-m' measurements compatible with exactly ONE of -1LRHby.\n");
            printf
                (" This is because Judy object measurements include RAM sum of all.\n");
            printf
                (" ========================================================\n");
            exit(1);
        }

        if (tFlag && (count != 1))
        {
            printf
                (" ========================================================\n");
            printf(" Sorry, '-t' measurements compatible with exactly ONE of -1LRHby.\n");
            printf
                (" This is because we haven't coded otherwise yet.\n");
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
    printf("# YLABEL Nano-Seconds -or- ???\n");

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

    // Use multiples of power of two group sizes for pure -DS1.
    if (bPureDS1)
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
                // MEB: not sure why but assert blows for nElms > (0x11 << 56).
                assert(grp == Groups - 1);
            } else {
                Pms[grp].ms_delta = wNumb - wPrev;
                assert(grp <= Groups - 1);
            }
            //printf("# wNumb 0x%04zx %zd\n", wNumb, wNumb);
        }
#endif // OLD_DS1_GROUPS
#ifndef CALC_NEXT_KEY
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
            for (Word_t ww = 0; ww < TValues; ww++) {
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
    printf("# COLHEAD %2d Delta Inserts to the Array\n", Col++);
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
    }
#ifdef RAMMETRICS
    if (J1Flag | JLFlag | JRFlag)
    {
        printf
            ("# COLHEAD %2d Heap/K  - Judy1 malloc'ed %% words per Key\n",
             Col++);
    }
#endif // RAMMETRICS
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
#if defined(MIKEY_1)
        if (J1Flag) {
            printf("# COLHEAD %2d B2  - Big bitmap leaf/Key\n", Col++);
        } else
#endif // defined(MIKEY_1)
        printf("# COLHEAD %2d VA  - Value area Words/Key\n", Col++);

        printf("# COLHEAD %2d %%MEff - %% RAM JudyMalloc()ed vs mmap()ed from Kernel\n", Col++);

        if (J1Flag)
            printf
                ("# COLHEAD %2d MF1/K    - Judy1 average malloc+free's per Key\n",
                 Col++);
        if (JLFlag)
            printf
                ("# COLHEAD %2d MFL/K    - JudyL average malloc+free's per Key\n",
                 Col++);

#ifdef SEARCHMETRICS
        printf("# COLHEAD %2d +MsCm - Average number forward Compares failed Per Leaf Search\n", Col++);
        printf("# COLHEAD %2d -MsCm - Average number reverse Compares failed Per Leaf Search\n", Col++);
        printf("# COLHEAD %2d Srch Lenth - Average Search Length (number of keys in list)\n", Col++);
        printf("# COLHEAD %2d %%DiHt - %% get calls the result in a direct hit\n", Col++);
        printf("# COLHEAD %2d %%GetP - %% get calls that miss and search forward\n", Col++);
        printf("# COLHEAD %2d %%GetM - %% get calls that miss and search backward\n", Col++);
        printf("# COLHEAD %2d Gets  - Num get calls\n", Col++);
#endif // SEARCHMETRICS
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

    LogIfdefs();

// ============================================================
// Warm up the cpu
// ============================================================

//  Try to fool compiler and ACTUALLY execute random()
    Word_t WarmupVar = 0;
    STARTTm;
    do {
        for (int ii = 0; ii < 1000000; ii++) {
            WarmupVar = random();
        }

        if (WarmupVar == 0) { // won't happen, but compiler doesn't know
            printf("!! Bug in random()\n");
        }

        ENDTm(DeltanSecW);      // get accumlated elapsed time
    } while (DeltanSecW < (Warmup * 1000000));

//  Now measure the execute time for 1M calls to random().
    STARTTm;
    for (int ii = 0; ii < 1000000; ii++) {
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

    // Calling JudyXFreeArray with a NULL pointer gives Judy an
    // opportunity to put configuration information, e.g. ifdefs,
    // into the output for later reference.
    // Maybe we should have used JudyX[Set|Ins](NULL) for global
    // initialization and JudyXFreeArray for global cleanup.
    if (J1Flag) { Judy1FreeArray(NULL, PJE0); }
    if (JLFlag || JRFlag) { JudyLFreeArray(NULL, PJE0); }

    // Warm up, e.g. JudyMalloc and caches.
    if (Warmup) {
#define N_WARMUP_KEYS  1000
        InsertSeed = StartSeed; // Test values
#ifndef CALC_NEXT_KEY
        if (!bLfsrOnly && (FValue == 0)
            && (TValues < N_WARMUP_KEYS) && (nElms > TValues))
        {
            // Initialize delta key array.
            // FileKeys[TValues] is where the delta keys begin.
            for (Word_t ww = 0; ww < MIN(N_WARMUP_KEYS, nElms); ww++) {
                FileKeys[TValues + ww] = CalcNextKey(&DeltaSeed);
            }
            DeltaSeed = *PInitSeed;
            InsertSeed = &FileKeys[TValues];
        }
#endif // CALC_NEXT_KEY
        Tit = 1;
        TestJudyIns(&J1, &JL, &InsertSeed,
                    /* nElms */ MIN(N_WARMUP_KEYS, nElms));
        DummySeed = StartSeed;
        TestJudyGet(J1, JL, &DummySeed,
                    /* nElms */ MIN(N_WARMUP_KEYS, TValues),
                    Tit, KFlag, hFlag, bLfsrOnly);
    }

    if (J1Flag) { Judy1FreeArray(&J1, NULL); }
    if (JLFlag) { JudyLFreeArray(&JL, NULL); }

// ============================================================
// PRINT COLUMNS HEADER TO PERFORMANCE TIMERS
// ============================================================

    PrintHeader("Pop");

// ============================================================
// BEGIN TESTS AT EACH GROUP SIZE
// ============================================================

    InsertSeed = StartSeed;             // for JudyIns
    BitmapSeed = StartSeed;             // for bitmaps

#ifndef CALC_NEXT_KEY
    int wLogPop1 = 0;
      #ifdef CLEVER_RANDOM_KEYS
    Word_t wSValueMagnitude = SValue;
    if ((intptr_t)SValue < 0) {
        wSValueMagnitude *= -1;
    }
    int nRandomGetsShift = 0; (void)nRandomGetsShift;
      #endif // CLEVER_RANDOM_KEYS
#endif // CALC_NEXT_KEY

    Word_t wFinalPop1 = 0;

    double DeltaGenInitMin = (wDoTit0Max == 0) ? .005 : 9e9;

    double DeltaGenIns1Min = DeltaGenInitMin;
    double DeltaGenInsLMin = DeltaGenInitMin;
    double DeltaGenInsHSMin = DeltaGenInitMin;
    double DeltaGenInsBtMin = DeltaGenInitMin;
    double DeltaGenInsByMin = DeltaGenInitMin;

    double DeltaGenGet1Min = DeltaGenInitMin;
    double DeltaGenGetLMin = DeltaGenInitMin;
    double DeltaGenGetHSMin = DeltaGenInitMin;
    double DeltaGenGetBtMin = DeltaGenInitMin;
    double DeltaGenGetByMin = DeltaGenInitMin;

    for (Pop1 = grp = 0; grp < Groups; grp++)
    {
        Word_t Delta;
        double DeltaGen1 = 0.0;
        double DeltaGenL = 0.0;
        double DeltaGenHS = 0.0;
        double DeltaGenBt = 0.0;
        double DeltaGenBy = 0.0;
        double DeltanSec1Sum = 0.0;
        double DeltanSecLSum = 0.0;
        double DeltanSecHSSum = 0.0;
        double DeltanSecBtSum = 0.0;
        double DeltanSecBySum = 0.0;
        double DeltaMalFre1Sum = 0.0;
        double DeltaMalFreLSum = 0.0;
        double DeltaMalFreHSSum = 0.0;

        if (mFlag) {
            // reset SEARCHMETRICS for this delta
            SearchPopulation = GetCallsSansPop = GetCalls = 0;
            MisComparesP = MisComparesM = 0;
            DirectHits = NotDirectHits = GetCallsP = GetCallsM = 0;
        }

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
                // Some code below mildly assumes that every delta is at
                // least as big as the previous delta. I'm thinking of
                // the measurement overhead smoothing that assumes the
                // overhead from a bigger delta can't really be bigger
                // than the overhead from a smaller delta and implements
                // a monotonically decreasing overhead value called "Min".
                // Unfortunatly, the parts on one delta aren't always
                // bigger than the parts of the previous delta.
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
        if (bRandomGets) {
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
            wLogPop1 = LOG(Pop1 + 1);
            Meas = MIN(TValues, EXP(wLogPop1) - 1);
            if (wLogPop1 > wPrevLogPop1) {
                wPrevLogPop1 = wLogPop1;
      #ifdef CLEVER_RANDOM_KEYS
                nRandomGetsShift = BValue - wLogPop1 - LOG(wSValueMagnitude);
      #endif // CLEVER_RANDOM_KEYS
                // RandomInit always initializes the same Seed_t.  Luckily,
                // that one seed is not being used anymore at this point.
                // We use it here for bPureDS1 and LFSR_GET_FOR_DS1 the sole
                // purpose of getting FeedBTap.
                PInitSeed = RandomInit(wLogPop1, 0);
                Seed_t RandomSeed = *PInitSeed;
#ifdef LFSR_GET_FOR_DS1
                if (bPureDS1) {
                    wFeedBTap = PInitSeed->FeedBTap;
                    BeginSeed = (NewSeed_t)StartSequent;
                } else
#endif // LFSR_GET_FOR_DS1
                {
                    RandomSeed.Seeds[0] = 1;
                }
                // Reinitialize the TestJudyGet key array.
                // This method uses as few as half of the inserted keys
                // for testing. Since we are using a key array it would be ok
                // to take a little more time if necessary and pick keys from
                // a larger and/or different subset.
                for (Word_t ww = 0; ww < Meas; ++ww) {
      #ifdef CLEVER_RANDOM_KEYS
          #ifdef LFSR_GET_FOR_DS1
                    // If LFSR_GET_FOR_DS1, then GetNextKeyX is capable of
                    // of generating random keys for pure -DS1.
                    // Is using it here just stupidly clever?
                    if (bPureDS1) {
                        FileKeys[ww] = GetNextKeyX(&BeginSeed,
                                                   wFeedBTap,
                                                   BValue - wLogPop1 + 1);
                    } else
          #endif // LFSR_GET_FOR_DS1
                    // For DFlag if abs(SValue) is a power of two we can
                    // avoid Swizzle.
                    // Is this method just stupidly clever with no
                    // redeeming qualities?
                    if (((wSValueMagnitude & (wSValueMagnitude - 1)) == 0)
                        && DFlag && !bSplayKeyBitsFlag && (Offset == 0)
                        && (((MaxNumb + 1) & MaxNumb) == 0))
                    {
                        Word_t wRand = RandomNumb(&RandomSeed, 0);
                        FileKeys[ww] = wRand << nRandomGetsShift;
                        if ((intptr_t)SValue < 0) {
                            FileKeys[ww]
                                |= ((Word_t)-1
                                    >> (sizeof(Word_t) * 8
                                        - nRandomGetsShift));
                        }
                    } else
      #endif // CLEVER_RANDOM_KEYS
                    {
                        FileKeys[ww]
                            = CalcNextKeyX(&RandomSeed, wLogPop1, 0);
                    }
                    assert(ww < EXP(wLogPop1) - 1);
                }
            }
        }
#endif // CALC_NEXT_KEY

        if (bFirstPart) {
            // first part of Delta
            // (Pop1 - Delta == wFinalPop1 - Pms[grp].ms_delta)
            if (grp && (grp % 32 == 0)) {
                PrintHeader("Pop"); // print column headers periodically
            }
        }

#ifndef CALC_NEXT_KEY
        if (!bLfsrOnly && (FValue == 0))
        {
            // Initialize delta key array.
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

        if (J1Flag || JLFlag)
        {
//          Test J1S, JLI, JLHS
//          Exit with InsertSeed/Key ready for next batch
//
            if (bFirstPart || (Pop1 == wFinalPop1)) {
                // first or last part of delta
                // last part might be a different size than the rest
                if (Delta <= wDoTit0Max) {
                    Tit = 0;                    // exclude Judy
                    DummySeed = InsertSeed;
                    WaitForContextSwitch(Delta);
                    TestJudyIns(&J1, &JL, &DummySeed, Delta);
                    DeltaGen1 = DeltanSec1 / Delta;     // save measurement overhead
                    DeltaGenL = DeltanSecL / Delta;
                    DeltaGenHS = DeltanSecHS / Delta;
                    if (DeltaGen1 < DeltaGenIns1Min) { DeltaGenIns1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenInsLMin) { DeltaGenInsLMin = DeltaGenL; }
                    if (DeltaGenHS < DeltaGenInsHSMin) { DeltaGenInsHSMin = DeltaGenHS; }
                    if (wDoTit0Max != (Word_t)-1) {
                        // use the previous miniumum to avoid erratic overhead numbers
                        if (DeltaGen1 > DeltaGenIns1Min) { DeltaGen1 = DeltaGenIns1Min; }
                        if (DeltaGenL > DeltaGenInsLMin) { DeltaGenL = DeltaGenInsLMin; }
                        if (DeltaGenHS > DeltaGenInsHSMin) { DeltaGenHS = DeltaGenInsHSMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGenIns1Min;
                    DeltaGenL = DeltaGenInsLMin;
                    DeltaGenHS = DeltaGenInsHSMin;
                } // end of Delta <= wDoTit0Max
            }

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyIns(&J1, &JL, &InsertSeed, Delta);
            DeltanSec1Sum += DeltanSec1;
            DeltanSecLSum += DeltanSecL;
            DeltanSecHSSum += DeltanSecHS;
            DeltaMalFre1Sum += DeltaMalFre1;
            DeltaMalFreLSum += DeltaMalFreL;
            DeltaMalFreHSSum += DeltaMalFreHS;

            if (Pop1 == wFinalPop1) {
                // last part of Delta
                // Deferring the printing of the row header until here allows
                // the library to print lines during Insert that do not muck up
                // the Time program output lines.
                PrintRowHeader(Pop1, Pms[grp].ms_delta, Meas);
                if (J1Flag)
                {
                    PrintValFFX(DeltanSec1Sum / Pms[grp].ms_delta - DeltaGen1, 5, 0);
                    if (tFlag)
                        PrintValFFX(DeltaGen1, 5, 0);
                }
                if (JLFlag)
                {
                    PrintValFFX(DeltanSecLSum / Pms[grp].ms_delta - DeltaGenL, 5, 0);
                    if (tFlag)
                        PrintValFFX(DeltaGenL, 5, 0);
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

                if (Meas <= wDoTit0Max) {
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    if (bLfsrOnly) {
                        if (KFlag) {
                            if (hFlag) {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 1,
                                            /* bLfsrOnly */ 1);
                            } else {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 0,
                                            /* bLfsrOnly */ 1);
                            }
                        } else {
                            if (hFlag) {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 1,
                                            /* bLfsrOnly */ 1);
                            } else {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 0,
                                            /* bLfsrOnly */ 1);
                            }
                        }
                    } else {
                        if (KFlag) {
                            if (hFlag) {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 1,
                                            /* bLfsrOnly */ 0);
                            } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                                if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                    BeginSeed = (NewSeed_t)StartSequent;
                                    TestJudyGet(J1, JL, &BeginSeed, Meas,
                                                /* Tit */ 0, /* KFlag */ 1,
                                                /* hFlag */ 0,
                                                /* bLfsrOnly */ BValue - wLogPop1 + 1);
                                } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 1, /* hFlag */ 0,
                                            /* bLfsrOnly */ 0);
                            }
                        } else {
                            if (hFlag) {
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 1,
                                            /* bLfsrOnly */ 0);
                            } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                                if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                    BeginSeed = (NewSeed_t)StartSequent;
                                    TestJudyGet(J1, JL, &BeginSeed, Meas,
                                                /* Tit */ 0, /* KFlag */ 0,
                                                /* hFlag */ 0,
                                                /* bLfsrOnly */ BValue - wLogPop1 + 1);
                                } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                                TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 0,
                                            /* KFlag */ 0, /* hFlag */ 0,
                                            /* bLfsrOnly */ 0);
                            }
                        }
                    }
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    DeltaGenHS = DeltanSecHS;
                    if (DeltaGen1 < DeltaGenGet1Min) { DeltaGenGet1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenGetLMin) { DeltaGenGetLMin = DeltaGenL; }
                    if (DeltaGenHS < DeltaGenGetHSMin) { DeltaGenGetHSMin = DeltaGenHS; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGen1 > DeltaGenGet1Min) { DeltaGen1 = DeltaGenGet1Min; }
                        if (DeltaGenL > DeltaGenGetLMin) { DeltaGenL = DeltaGenGetLMin; }
                        if (DeltaGenHS > DeltaGenGetHSMin) { DeltaGenHS = DeltaGenGetHSMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGenGet1Min;
                    DeltaGenL = DeltaGenGetLMin;
                    DeltaGenHS = DeltaGenGetHSMin;
                } // end of Meas <= wDoTit0Max

                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                if (bLfsrOnly) {
                    if (KFlag) {
                        if (hFlag) {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 1,
                                        /* bLfsrOnly */ 1);
                        } else {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 0,
                                        /* bLfsrOnly */ 1);
                        }
                    } else {
                        if (hFlag) {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 1,
                                        /* bLfsrOnly */ 1);
                        } else {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 0,
                                        /* bLfsrOnly */ 1);
                        }
                    }
                } else {
                    if (KFlag) {
                        if (hFlag) {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 1,
                                        /* bLfsrOnly */ 0);
                        } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                            if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                BeginSeed = (NewSeed_t)StartSequent;
                                TestJudyGet(J1, JL, &BeginSeed, Meas,
                                            /* Tit */ 1, /* KFlag */ 1,
                                            /* hFlag */ 0,
                                            /* bLfsrOnly */ BValue - wLogPop1 + 1);
                            } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 1, /* hFlag */ 0,
                                        /* bLfsrOnly */ 0);
                        }
                    } else {
                        if (hFlag) {
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 1,
                                        /* bLfsrOnly */ 0);
                        } else {
#ifdef LFSR_GET_FOR_DS1
#ifndef CALC_NEXT_KEY
                            if (bLfsrForGetOnly && (wFeedBTap != (Word_t)-1)) {
                                BeginSeed = (NewSeed_t)StartSequent;
                                TestJudyGet(J1, JL, &BeginSeed, Meas,
                                            /* Tit */ 1, /* KFlag */ 0,
                                            /* hFlag */ 0,
                                            /* bLfsrOnly */ BValue - wLogPop1 + 1);
                            } else
#endif // CALC_NEXT_KEY
#endif // LFSR_GET_FOR_DS1
                            TestJudyGet(J1, JL, &BeginSeed, Meas, /* Tit */ 1,
                                        /* KFlag */ 0, /* hFlag */ 0,
                                        /* bLfsrOnly */ 0);
                        }
                    }
                }

                if (J1Flag)
                {
                    PrintValFFX(DeltanSec1 - DeltaGen1, 5, /* bUseSymbol */ 0);
                    if (tFlag)
                        PrintValFFX(DeltaGen1, 5, /* bUseSymbol */ 0);
                }
                if (JLFlag)
                {
                    PrintValFFX(DeltanSecL - DeltaGenL, 5, /* bUseSymbol */ 0);
                    if (tFlag)
                        PrintValFFX(DeltaGenL, 5, /* bUseSymbol */ 0);
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
                if (Delta <= wDoTit0Max) { // Delta < wDoTit0Max
                    Tit = 0;                    // exclude Judy
                    DummySeed = BitmapSeed;
                    WaitForContextSwitch(Delta);
                    TestJudyLIns(&JL, &DummySeed, Delta);
                    DeltaGenL = DeltanSecL / Delta;
                    if (DeltaGenL < DeltaGenInsLMin) { DeltaGenInsLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGenL > DeltaGenInsLMin) { DeltaGenL = DeltaGenInsLMin; }
                    }
                } else {
                    DeltaGenL = DeltaGenInsLMin;
                } // end of Delta <= wDoTit0Max
            }

            Tit = 1;                    // include Judy
            WaitForContextSwitch(Delta);
            TestJudyLIns(&JL, &BitmapSeed, Delta);
            DeltanSecLSum += DeltanSecL;
            DeltaMalFreLSum += DeltaMalFreL;

            if (Pop1 == wFinalPop1) {
                PrintRowHeader(Pop1, Pms[grp].ms_delta, Meas);
                PrintValFFX(DeltanSecLSum / Pms[grp].ms_delta - DeltaGenL, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);

                if (Meas <= wDoTit0Max) {
                    Tit = 0;                    // exclude Judy
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestJudyLGet(JL, &BeginSeed, Meas);
                    DeltaGenL = DeltanSecL;
                    if (DeltaGenL < DeltaGenGetLMin) { DeltaGenGetLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGenL > DeltaGenGetLMin) { DeltaGenL = DeltaGenGetLMin; }
                    }
                } else {
                    DeltaGenL = DeltaGenGetLMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;                    // include Judy
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyLGet(JL, &BeginSeed, Meas);

                PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test a REAL bitmap
        if (bFlag)
        {
            //DummySeed = BitmapSeed;
            //GetNextKey(&DummySeed);   // warm up cache

            if (Delta <= wDoTit0Max) {
                Tit = 0;
                DummySeed = BitmapSeed;
                WaitForContextSwitch(Delta);
                TestBitmapSet(&B1, &DummySeed, Delta);
                DeltaGenBt = DeltanSecBt / Delta;
                if (DeltaGenBt < DeltaGenInsBtMin) { DeltaGenInsBtMin = DeltaGenBt; }
                if (wDoTit0Max != (Word_t)-1) {
                    if (DeltaGenBt > DeltaGenInsBtMin) { DeltaGenBt = DeltaGenInsBtMin; }
                }
            } else {
                DeltaGenBt = DeltaGenInsBtMin;
            } // end of Delta <= wDoTit0Max

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestBitmapSet(&B1, &BitmapSeed, Delta);
            DeltanSecBtSum += DeltanSecBt;

            if (Pop1 == wFinalPop1) {
                PrintRowHeader(Pop1, Pms[grp].ms_delta, Meas);
                PrintValFFX(DeltanSecBtSum / Pms[grp].ms_delta - DeltaGenBt, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenBt, 5, 0);

                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestBitmapTest(B1, &BeginSeed, Meas);
                    DeltaGenBt = DeltanSecBt;
                    if (DeltaGenBt < DeltaGenGetBtMin) { DeltaGenGetBtMin = DeltaGenBt; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGenBt > DeltaGenGetBtMin) { DeltaGenBt = DeltaGenGetBtMin; }
                    }
                } else {
                    DeltaGenBt = DeltaGenGetBtMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestBitmapTest(B1, &BeginSeed, Meas);

                PrintValFFX(DeltanSecBt - DeltaGenBt, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenBt, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test a REAL ByteMap
        if (yFlag)
        {
            //DummySeed = BitmapSeed;
            //GetNextKey(&DummySeed);   // warm up cache

            if (Delta <= wDoTit0Max) {
                Tit = 0;
                DummySeed = BitmapSeed;
                WaitForContextSwitch(Delta);
                TestByteSet(&DummySeed, Delta);
                DeltaGenBy = DeltanSecBy / Delta;
                if (DeltaGenBy < DeltaGenInsByMin) { DeltaGenInsByMin = DeltaGenBy; }
                if (wDoTit0Max != (Word_t)-1) {
                    if (DeltaGenBy > DeltaGenInsByMin) { DeltaGenBy = DeltaGenInsByMin; }
                }
            } else {
                DeltaGenBy = DeltaGenInsByMin;
            } // end of Delta <= wDoTit0Max

            Tit = 1;
            WaitForContextSwitch(Delta);
            TestByteSet(&BitmapSeed, Delta);
            DeltanSecBySum += DeltanSecBy;

            if (Pop1 == wFinalPop1) {
                PrintRowHeader(Pop1, Pms[grp].ms_delta, Meas);
                PrintValFFX(DeltanSecBySum / Pms[grp].ms_delta - DeltaGenBy, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenBy, 5, 0);

                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestByteTest(&BeginSeed, Meas);
                    DeltaGenBy = DeltanSecBy;
                    if (DeltaGenBy < DeltaGenGetByMin) { DeltaGenGetByMin = DeltaGenBy; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGenBy > DeltaGenGetByMin) { DeltaGenBy = DeltaGenGetByMin; }
                    }
                } else {
                    DeltaGenBy = DeltaGenGetByMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestByteTest(&BeginSeed, Meas);

                PrintValFFX(DeltanSecBy - DeltaGenBy, 5, 0);
                if (tFlag)
                    PrintValFFX(DeltaGenBy, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
        }

  // SEARCHMETRICS are just for Get/Test.
  #ifdef SEARCHMETRICS
      // We added DSMETRICS_GETS in an effort to
      // reduce the overhead of SEARCHMETRICS in the library.
      // The idea is that the Time program already knows how many GetCalls it's
      // done so we don't need the library to keep track.
      // One disadvantage of DSMETRICS_GETS is that the library has
      // to instrument every get call because the Time program will be assuming
      // all of them are instrumented, e.g. unpacked bitmaps and immediates and
      // gets of keys that do not exist, but we might prefer to instrument only
      // a subset in the library, and without DSMETRICS_GETS we can use
      // j__GetCalls to indicate how many calls were actually instrumented.
      // Should we add j__GetCallsNot for DSMETRICS_GETS? Done.
      #ifdef DSMETRICS_GETS
        Word_t NewGetCalls = Meas - j__GetCallsNot;
      #else // DSMETRICS_GETS
        Word_t NewGetCalls = j__GetCalls; // count of gets with instrumentation
      #endif // DSMETRICS_GETS else
        GetCalls += NewGetCalls;
      // We added DSMETRICS_[N]HITS in the same vein, i.e. to
      // reduce the overhead of SEARCHMETRICS in the library.
      // Since GetCalls = DirectHits + GetCallsP + GetCallsM
      // we can derive one of DirectHits, GetCallsP, and GetCallsM
      // if we know two of them so the library only has to keep track of two.
      // One bummer is that there is no simple j__NotDirectHits counter, i.e.
      // the library has to decide between j__GetCallsP and j__GetCallsM if not
      // maintaining j__DirectHits and this may involve an additional test in
      // the library. One option is for the library to simply record all misses
      // as j__GetCallsP or j__GetCallsM to avoid the overhead of figuring out
      // which one. Another is to add j__NotDirectHits for cases when we don't
      // know the direction? Done.
      // Now GetCalls = DirectHits + NotDirectHits + GetCallsP + GetCallsM.
      // But we only support derivation of DirectHits or NotDirectHits using
      // DSMETRICS_[N]HITS.
      #ifdef DSMETRICS_HITS
      #ifdef DSMETRICS_NHITS
        #error Cannot have both DSMETRICS_HITS and DSMETRICS_NHITS
      #endif // DSMETRICS_HITS
      #endif // DSMETRICS_NHITS
      #if defined(DSMETRICS_HITS)
        assert(!mFlag || (j__DirectHits == 0));
        DirectHits
            += NewGetCalls - j__NotDirectHits - j__GetCallsP - j__GetCallsM;
      #else // DSMETRICS_HITS
        DirectHits += j__DirectHits; // direct hits
      #endif // DSMETRICS_HITS else
      #ifdef DSMETRICS_NHITS
        assert(!mFlag || (j__NotDirectHits == 0));
        NotDirectHits
            += NewGetCalls - j__DirectHits - j__GetCallsP - j__GetCallsM;
      #else // DSMETRICS_NHITS
        NotDirectHits += j__NotDirectHits; // unknown direction of miss
      #endif // DSMETRICS_NHITS else
      #ifndef DSMETRICS_HITS
      #ifndef DSMETRICS_NHITS
        if (mFlag) {
            if (j__DirectHits + j__NotDirectHits + j__GetCallsP + j__GetCallsM
                   != NewGetCalls)
            {
                printf("\n# Meas %zd j__GetCalls %zd j__GetCallsNot %zd"
                           " NewGetCalls %zd\n",
                       Meas, j__GetCalls, j__GetCallsNot, NewGetCalls);
                printf("# j__DirectHits %zd j__NotDirectHits %zd j__GetCallsP %zd"
                           " j__GetCallsM %zd\n",
                       j__DirectHits, j__NotDirectHits, j__GetCallsP,
                       j__GetCallsM);
            }
            assert(j__DirectHits + j__NotDirectHits + j__GetCallsP + j__GetCallsM
                   == NewGetCalls);
        }
      #endif // !DSMETRICS_NHITS
      #endif // !DSMETRICS_HITS
        GetCallsP += j__GetCallsP;
        GetCallsM += j__GetCallsM;
        // Some leaves don't need to know population to do a Get.
        // Hence adding it to j__SearchPopulation means figuring out the
        // otherwise unneeded population and this may be expensive.
        SearchPopulation += j__SearchPopulation;
      // Another bummer about DSMETRICS_GETS is that it doesn't help with
      // j__SearchPopulation which Get/Test don't always need to know except
      // for SEARCHMETRICS.
      // Could the library simply bump a count of GetCalls without a valid
      // search population, e.g. GetCallsSansPop, and Time exclude them from
      // the average search pop calculation? Done.
        GetCallsSansPop += j__GetCallsSansPop;
        MisComparesP += j__MisComparesP;
        MisComparesM += j__MisComparesM;
  #endif // SEARCHMETRICS

        if (Pop1 != wFinalPop1)
        {
            bFirstPart = 0;
            goto nextPart;
        }

        // (Pop1 == wFinalPop1)

//      Test J1T, JLI, JHSI - duplicates
        if (IFlag)
        {
            static double DeltaGen1Min, DeltaGenLMin, DeltaGenHSMin;
            if (Pop1 == Delta) {
                DeltaGen1Min = DeltaGenLMin = DeltaGenHSMin = DeltaGenInitMin;
            }
            if (Meas <= wDoTit0Max) {
                Tit = 0;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyDup(&J1, &JL, &BeginSeed, Meas);
                DeltaGen1 = DeltanSec1;     // save measurement overhead
                DeltaGenL = DeltanSecL;
                DeltaGenHS = DeltanSecHS;
                if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                if (DeltaGenHS < DeltaGenHSMin) { DeltaGenHSMin = DeltaGenHS; }
                if (wDoTit0Max != (Word_t)-1) {
                    if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                    if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    if (DeltaGenHS > DeltaGenHSMin) { DeltaGenHS = DeltaGenHSMin; }
                }
            } else {
                DeltaGen1 = DeltaGen1Min;
                DeltaGenL = DeltaGenLMin;
                DeltaGenHS = DeltaGenHSMin;
            } // end of Meas <= wDoTit0Max

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDup(&J1, &JL, &BeginSeed, Meas);
            if (J1Flag)
                PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
            if (JLFlag)
                PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
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
            static double DeltaGen1Min, DeltaGenLMin;
            if (Pop1 == Delta) {
                DeltaGen1Min = DeltaGenLMin = DeltaGenInitMin;
            }
            if (Meas <= wDoTit0Max) {
                Tit = 0;
                WaitForContextSwitch(Meas);
                TestJudyCount(J1, JL, &BeginSeed, Meas);
                DeltaGen1 = DeltanSec1;     // save measurement overhead
                DeltaGenL = DeltanSecL;
                if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                if (wDoTit0Max != (Word_t)-1) {
                    if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                    if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                }
            } else {
                DeltaGen1 = DeltaGen1Min;
                DeltaGenL = DeltaGenLMin;
            } // end of Meas <= wDoTit0Max

            Tit = 1;
            WaitForContextSwitch(Meas);
            TestJudyCount(J1, JL, &BeginSeed, Meas);
            if (J1Flag)
                PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
            if (JLFlag)
                PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
            if (fFlag)
                fflush(NULL);
        }

        if (vFlag)
        {
            { // open a new scope because we want to reuse variable names
                static double DeltaGen1Min, DeltaGenLMin;
                if (Pop1 == Delta) {
                    DeltaGen1Min = DeltaGenLMin = DeltaGenInitMin;
                }
//              Test J1N, JLN
                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestJudyNext(J1, JL, &BeginSeed, Meas);
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                        if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGen1Min;
                    DeltaGenL = DeltaGenLMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyNext(J1, JL, &BeginSeed, Meas);
                if (J1Flag)
                    PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
                if (JLFlag)
                    PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
            { // open a new scope because we want to reuse variable names
                static double DeltaGen1Min, DeltaGenLMin;
                if (Pop1 == Delta) {
                    DeltaGen1Min = DeltaGenLMin = DeltaGenInitMin;
                }
//              Test J1P, JLP
                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestJudyPrev(J1, JL, &BeginSeed, Meas);
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                        if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGen1Min;
                    DeltaGenL = DeltaGenLMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyPrev(J1, JL, &BeginSeed, Meas);
                if (J1Flag)
                    PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
                if (JLFlag)
                    PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
            { // open a new scope because we want to reuse variable names
                static double DeltaGen1Min, DeltaGenLMin;
                if (Pop1 == Delta) {
                    DeltaGen1Min = DeltaGenLMin = DeltaGenInitMin;
                }
//              Test J1NE, JLNE
                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                        if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGen1Min;
                    DeltaGenL = DeltaGenLMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyNextEmpty(J1, JL, &BeginSeed, Meas);
                if (J1Flag)
                    PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
                if (JLFlag)
                    PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
            { // open a new scope because we want to reuse variable names
                static double DeltaGen1Min, DeltaGenLMin;
                if (Pop1 == Delta) {
                    DeltaGen1Min = DeltaGenLMin = DeltaGenInitMin;
                }
//              Test J1PE, JLPE
                if (Meas <= wDoTit0Max) {
                    Tit = 0;
                    BeginSeed = StartSeed;      // reset at beginning
                    WaitForContextSwitch(Meas);
                    TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
                    DeltaGen1 = DeltanSec1;     // save measurement overhead
                    DeltaGenL = DeltanSecL;
                    if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                    if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                    if (wDoTit0Max != (Word_t)-1) {
                        if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                        if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    }
                } else {
                    DeltaGen1 = DeltaGen1Min;
                    DeltaGenL = DeltaGenLMin;
                } // end of Meas <= wDoTit0Max

                Tit = 1;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyPrevEmpty(J1, JL, &BeginSeed, Meas);
                if (J1Flag)
                    PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
                if (JLFlag)
                    PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
                if (fFlag)
                    fflush(NULL);
            }
        }

//      Test J1U, JLD, JHSD
        if (dFlag)
        {
            static double DeltaGen1Min, DeltaGenLMin, DeltaGenHSMin;
            if (Pop1 == Delta) {
                DeltaGen1Min = DeltaGenLMin = DeltaGenHSMin = DeltaGenInitMin;
            }
            if (Meas <= wDoTit0Max) {
                Tit = 0;
                BeginSeed = StartSeed;      // reset at beginning
                WaitForContextSwitch(Meas);
                TestJudyDel(&J1, &JL, &BeginSeed, Meas);
                DeltaGen1 = DeltanSec1;     // save measurement overhead
                DeltaGenL = DeltanSecL;
                DeltaGenHS = DeltanSecHS;
                if (DeltaGen1 < DeltaGen1Min) { DeltaGen1Min = DeltaGen1; }
                if (DeltaGenL < DeltaGenLMin) { DeltaGenLMin = DeltaGenL; }
                if (DeltaGenHS < DeltaGenHSMin) { DeltaGenHSMin = DeltaGenHS; }
                if (wDoTit0Max != (Word_t)-1) {
                    if (DeltaGen1 > DeltaGen1Min) { DeltaGen1 = DeltaGen1Min; }
                    if (DeltaGenL > DeltaGenLMin) { DeltaGenL = DeltaGenLMin; }
                    if (DeltaGenHS > DeltaGenHSMin) { DeltaGenHS = DeltaGenHSMin; }
                }
            } else {
                DeltaGen1 = DeltaGen1Min;
                DeltaGenL = DeltaGenLMin;
                DeltaGenHS = DeltaGenHSMin;
            } // end of Meas <= wDoTit0Max

            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyDel(&J1, &JL, &BeginSeed, Meas);
            if (J1Flag)
                PrintValFFX(DeltanSec1 - DeltaGen1, 5, 0);
            if (JLFlag)
                PrintValFFX(DeltanSecL - DeltaGenL, 5, 0);
            if (fFlag)
                fflush(NULL);

//          Now put back the Just deleted Keys
            Tit = 1;
            BeginSeed = StartSeed;      // reset at beginning
            WaitForContextSwitch(Meas);
            TestJudyIns(&J1, &JL, &BeginSeed, Meas);
        }

#ifdef RAMMETRICS
        if (J1Flag | JLFlag | JRFlag) {
            PRINT5_1f((double)j__AllocWordsTOT / Pop1);
        }
#endif // RAMMETRICS

        if (mFlag && (bFlag == 0) && (yFlag == 0))
        {
            PRINT5_1f((double)j__AllocWordsJBB   / (double)Pop1);       // 256 node branch
            PRINT5_1f((double)j__AllocWordsJBU   / (double)Pop1);       // 256 node branch
            PRINT5_1f((double)j__AllocWordsJBL   / (double)Pop1);       // xx node branch
            PRINT5_1f((double)j__AllocWordsJLL[0] / (double)Pop1);      // > 56-bit key
            for (int ll = 7; ll > 0; --ll) {
                PRINT5_1f((double)j__AllocWordsJLL[ll] / (double)Pop1); // ll * 8 bit Key
            }
            PRINT5_1f((double)j__AllocWordsJLB1  / (double)Pop1);       // 1 digit bimap
            PRINT5_1f((double)j__AllocWordsJV    / (double)Pop1);       // Values

// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSss

//          Print the percent efficiency of dlmalloc
            PRINT5_1f((double)j__AllocWordsTOT / j__MmapWordsTOT);
            if (J1Flag)
                PRINT5_1f(DeltaMalFre1Sum / Pms[grp].ms_delta);
            if (JLFlag || JRFlag)
                PRINT5_1f(DeltaMalFreLSum / Pms[grp].ms_delta);
#ifdef SEARCHMETRICS
//          print average number of failed compares done in leaf search
            PrintValx100((double)MisComparesP / MAX(GetCallsP + DirectHits, 1), 5, 1);
            PrintValx100((double)MisComparesM / MAX(GetCallsM + DirectHits, 1), 5, 1);
            PrintVal((double)SearchPopulation / MAX(GetCalls - GetCallsSansPop, 1), 5, 1);
            PrintValx100((double)DirectHits / GetCalls, 5, 1);
            PrintValx100((double)GetCallsP / GetCalls, 5, 1);
            PrintValx100((double)GetCallsM / GetCalls, 5, 1);
            PrintVal(GetCalls, 5, 0);
#endif // SEARCHMETRICS
        }

        printf("\n");
        if (fFlag)
            fflush(NULL);                   // assure data gets to file in case malloc fail
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
TestJudyIns(void **J1, void **JL, PNewSeed_t PSeed, Word_t Elements)
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

    // Loops don't work as expected for array modifying ops.
    Word_t Loops = Tit ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

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
                        }
                        if (GFlag
                            && (elm < sizeof(Word_t) * 8)
                            && (wCheckBits & ((Word_t)1 << elm)))
                        {
                            // Test for a key that has not been inserted.
                            // Pick one that differs from a key that has
                            // been inserted by only a single digit in an
                            // attempt to test narrow pointers.
                            Word_t TstKeyNot = TstKey ^ ((Word_t)1 << elm);
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
                        if ((*PValue != 0) && (TstKey != 0)) // Oops, Insert made error
                        {
                            if (GValue && (*PValue == TstKey))
                            {
                                JudyLDups++;
                            }
                            else
                            {
                                printf("\n*PValue != 0 after Insert PValue %p\n", PValue);
                                printf("TstKey = 0x%" PRIxPTR", *PValue = 0x%" PRIxPTR"\n", TstKey, *PValue);
                                FAILURE("JudyLIns returned wrong *PValue after Insert", TstKey);
                            }
                        }
                        if (VFlag)
                        {
                            *PValue = TstKey;     // save Key in Value
                        }
                        if (iFlag)  // mainly for debug
                        {
                            PWord_t   PValueNew;

                            PValueNew = (PWord_t)JudyLIns(JL, TstKey, PJE0);
                            if (PValueNew == NULL)
                            {
                                printf("\nTstKey = 0x%" PRIxPTR"\n", TstKey);
                                FAILURE("JudyLIns failed with NULL after Insert", TstKey);
                            }
  #ifdef MIKEY_L
                            if (PValueNew != PValue)
                            {
                                FAILURE("Second JudyLIns failed with"
                                        " wrong PValue after Insert",
                                        TstKey);
      #if 0
                                // Doug's code isn't strict about this yet
                                // because it modifies the array on the way
                                // down.
                                printf("\n#Line = %d,"
                                       " Caution: PValueNew = 0x%" PRIxPTR
                                       ", PValueold = 0x%" PRIxPTR
                                       " changed\n", __LINE__,
                                       (Word_t)PValueNew, (Word_t)PValue);
                                printf("- ValueNew = 0x%" PRIxPTR
                                       ", Valueold = 0x%" PRIxPTR"\n",
                                       *PValueNew, *PValue);
      #endif // 0
                            }
  #endif // MIKEY_L
                            if (VFlag)
                            {
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n*PValueNew = 0x%" PRIxPTR"\n", *PValueNew);
                                    printf("TstKey = 0x%" PRIxPTR" = %" PRIdPTR"\n", TstKey, TstKey);
                                    FAILURE("Second JudyLIns failed with wrong *PValue after Insert", TstKey);
                                }
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
                            else if (VFlag)
                            {
                                if (*PValueNew != TstKey)
                                {
                                    printf("\n--- *PValueNew = 0x%" PRIxPTR"\n", *PValueNew);
                                    printf("--- TstKey = 0x%" PRIxPTR" = %" PRIdPTR"", TstKey, TstKey);
                                    FAILURE("JudyLGet failed after Insert", TstKey);
                                }
                            }
                        }
                        if (GFlag
                            && (elm < sizeof(Word_t) * 8)
                            && (wCheckBits & ((Word_t)1 << elm)))
                        {
                            // Test for a key that has not been inserted.
                            // Pick one that differs from a key that has
                            // been inserted by only a single digit in an
                            // attempt to test narrow pointers.
                            Word_t TstKeyNot = TstKey ^ ((Word_t)1 << elm);
                            PWord_t PValueNew = (PWord_t)JudyLGet(*JL, TstKeyNot, PJE0);
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
        DeltanSecL = DminTime;
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

    // Loops don't work as expected for array modifying ops.
    Word_t Loops = Tit ? 1 : (MAXLOOPS / Elements) + MINLOOPS;

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

    DeltanSecL = DminTime;

    *PSeed = WorkingSeed;               // advance
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDup"

int
TestJudyDup(void **J1, void **JL, PNewSeed_t PSeed, Word_t Elements)
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

    return 0;
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyGet"

static inline int
TestJudyGet(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements,
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

            if (mFlag) {
                // reset j__* for this lp loop
                // caller will examine j__* for final lp loop when we return
  #ifdef DSMETRICS_GETS
                j__GetCallsNot = 0;
  #else // DSMETRICS_GETS
                j__GetCalls = 0;
  #endif // DSMETRICS_GETS else
                j__GetCallsSansPop = 0;
                j__SearchPopulation = 0;
                j__MisComparesP = j__MisComparesM = 0;
                j__DirectHits = j__NotDirectHits = j__GetCallsP = j__GetCallsM = 0;
            }

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
            Word_t wFeedBTapLocal = wFeedBTap; // don't know why this is faster

            if (mFlag) {
                // reset j__* for this lp loop
                // caller will examine j__* for final lp loop when we return
  #ifdef DSMETRICS_GETS
                j__GetCallsNot = 0;
  #else // DSMETRICS_GETS
                j__GetCalls = 0;
  #endif // DSMETRICS_GETS else
                j__GetCallsSansPop = 0;
                j__SearchPopulation = 0;
                j__MisComparesP = j__MisComparesM = 0;
                j__DirectHits = j__NotDirectHits = j__GetCallsP = j__GetCallsM = 0;
            }

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
                SYNC_SYNC(TstKey = GetNextKeyX(&WorkingSeed, wFeedBTapLocal, bLfsrOnly));
                //SYNC_SYNC(TstKey = GetNextKeyX(&WorkingSeed, wFeedBTap, bLfsrOnly));

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
                                ("--- JudyLGet Key 0x%" PRIxPTR" returned PValue %p Value=0x%" PRIxPTR", should be=0x%" PRIxPTR"",
                                 TstKey, PValue, *PValue, TstKey);
                            JudyLDump((Word_t)JL, sizeof(Word_t) * 8, TstKey);
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

        if (mFlag) {
            // reset j__* for this lp loop
            // caller will examine j__* for final lp loop when we return
  #ifdef DSMETRICS_GETS
            j__GetCallsNot = 0;
  #else // DSMETRICS_GETS
            j__GetCalls = 0;
  #endif // DSMETRICS_GETS else
            j__GetCallsSansPop = 0;
            j__SearchPopulation = 0;
            j__MisComparesP = j__MisComparesM = 0;
            j__DirectHits = j__NotDirectHits = j__GetCallsP = j__GetCallsM = 0;
        }

        TstKey = GetNextKey(&WorkingSeed);    // Get 1st Key

        STARTTm;
        for (elm = 0; elm < Elements; elm++)
        {
            if (Tit) {
                Word_t* pwValue = (Word_t*)JudyLGet(JL, TstKey, PJE0);
                assert(pwValue != NULL);
                TstKey = *pwValue;
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
    (void)PSeed; // for TEST_COUNT_USING_JUDY_NEXT

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
// TEST_COUNT_USING_JUDY_NEXT is incompatible with hFlag.
#ifdef TEST_COUNT_USING_JUDY_NEXT // the old way
            TstKey = 0;
            int Rc = Judy1First(J1, &TstKey, PJE0);
            elm = 0;
#else // TEST_COUNT_USING_JUDY_NEXT
            Word_t TstKey2;
            NewSeed_t WorkingSeed = *PSeed;
            elm = 1;
#endif // TEST_COUNT_USING_JUDY_NEXT

            STARTTm;
            while (elm < Elements)
            {
#ifndef TEST_COUNT_USING_JUDY_NEXT // the new way
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));
                TstKey2 = GetNextKey(&WorkingSeed);
#endif // TEST_COUNT_USING_JUDY_NEXT
                if (Tit)
                {
#ifdef TEST_COUNT_USING_JUDY_NEXT
                    Count1 = Judy1Count(J1, 0, TstKey, PJE0);
                    if (Count1 != (elm + 1))
                    {
                        printf("\n");
                        printf("Count1 = %" PRIuPTR", TstKey = %" PRIuPTR"\n",
                               Count1, TstKey);
                        printf("Elements %zd\n", Elements);
                        FAILURE("J1C at", elm);
                    }
#else // TEST_COUNT_USING_JUDY_NEXT
                    if (TstKey > TstKey2) {
                        // swap keys
                        Word_t w = TstKey; TstKey = TstKey2; TstKey2 = w;
                    }
                    --TstKey2;
                    Count1 = Judy1Count(J1, TstKey, TstKey2, PJE0);
                    if ((Count1 < 1) || (Count1 > TstKey2 - TstKey + 1))
                    {
                        printf("\n");
                        printf("Count1 %zd TstKey 0x%zx TstKey2 0x%zx\n",
                               Count1, TstKey, TstKey2);
                        printf("Elements %zd\n", Elements);
                        FAILURE("J1C at", elm);
                    }
#endif // TEST_COUNT_USING_JUDY_NEXT
                }
#ifdef TEST_COUNT_USING_JUDY_NEXT
                Rc = Judy1Next(J1, &TstKey, PJE0);
#else // TEST_COUNT_USING_JUDY_NEXT
                ++elm;
#endif // TEST_COUNT_USING_JUDY_NEXT
                ++elm;
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
#ifndef TEST_COUNT_USING_JUDY_NEXT
        DeltanSec1 *= 2;
#endif // TEST_COUNT_USING_JUDY_NEXT
    }

    if (JLFlag)
    {
        for (DminTime = 1e40, icnt = ICNT, lp = 0; lp < Loops; lp++)
        {
            Word_t *PValue; (void)PValue; // void for !TEST_COUNT_USING_JUDY_NEXT

#ifdef TEST_COUNT_USING_JUDY_NEXT // the old way
            TstKey = 0;
            PValue = (PWord_t)JudyLFirst(JL, &TstKey, PJE0);
            elm = 0;
#else // TEST_COUNT_USING_JUDY_NEXT
            Word_t TstKey2;
            NewSeed_t WorkingSeed = *PSeed;
            elm = 1;
#endif // TEST_COUNT_USING_JUDY_NEXT

            STARTTm;
            while (elm < Elements)
            {
#ifndef TEST_COUNT_USING_JUDY_NEXT // the new way
                SYNC_SYNC(TstKey = GetNextKey(&WorkingSeed));
                TstKey2 = GetNextKey(&WorkingSeed);
#endif // TEST_COUNT_USING_JUDY_NEXT
                if (Tit)
                {
#ifdef TEST_COUNT_USING_JUDY_NEXT
                    CountL = JudyLCount(JL, 0, TstKey, PJE0);
                    if (CountL != (elm + 1))
                    {
                        printf("CountL = %" PRIuPTR", elm +1 = %" PRIuPTR"\n", CountL, elm + 1);
                        printf("Elements %zd\n", Elements);
                        FAILURE("JLC at", elm);
                    }
#else // TEST_COUNT_USING_JUDY_NEXT
                    if (TstKey > TstKey2) {
                        // swap keys
                        Word_t w = TstKey; TstKey = TstKey2; TstKey2 = w;
                    }
                    --TstKey2;
                    CountL = JudyLCount(JL, TstKey, TstKey2, PJE0);
                    if ((CountL < 1) || (CountL > TstKey2 - TstKey + 1))
                    {
                        printf("\n");
                        printf("CountL %zd TstKey 0x%zx TstKey2 0x%zx\n",
                               CountL, TstKey, TstKey2);
                        printf("Elements %zd\n", Elements);
                        FAILURE("JLC at", elm);
                    }
#endif // TEST_COUNT_USING_JUDY_NEXT
                }
#ifdef TEST_COUNT_USING_JUDY_NEXT
                PValue = (PWord_t)JudyLNext(JL, &TstKey, PJE0);
                if (VFlag && PValue && (*PValue != TstKey))
                {
                    printf("\nPValue=0x%" PRIxPTR", *PValue=0x%" PRIxPTR", TstKey=0x%" PRIxPTR"\n",
                           (Word_t)PValue, *PValue, TstKey);
                    FAILURE("JudyLNext ret bad *PValue at", elm);
                }
#else // TEST_COUNT_USING_JUDY_NEXT
                ++elm;
#endif // TEST_COUNT_USING_JUDY_NEXT
                ++elm;
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
#ifndef TEST_COUNT_USING_JUDY_NEXT
        DeltanSecL *= 2;
#endif // TEST_COUNT_USING_JUDY_NEXT
    }

    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNext"

Word_t
TestJudyNext(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    (void)PSeed; // for TEST_NEXT_USING_JUDY_NEXT

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
            Word_t J1KeyBefore;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            J1KeyBefore = -1;
            J1Key = 0;
            Rc = Judy1First(J1, &J1Key, PJE0);
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t J1LastKey = -1;
            Judy1Last(J1, &J1LastKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // #else TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(J1Key = GetNextKey(&WorkingSeed));
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    J1KeyBefore = J1Key;
                    Rc = Judy1Next(J1, &J1Key, PJE0);
                    if (J1KeyBefore == J1LastKey)
                    {
                        if ((Rc == 1)
  // Judy 1.0.5 did not preserve the key value on next/prev failure.
  #ifndef NO_PRESERVED_KEY
                            || (J1Key != J1KeyBefore)
  #endif // #ifndef NO_PRESERVED_KEY
                            )
                        {
                            printf("\n");
                            printf("J1LastKey %" PRIuPTR"\n", J1LastKey);
                            printf("J1KeyBefore %" PRIuPTR"\n", J1KeyBefore);
                            printf("\nElements = %" PRIuPTR
                                   ", elm = %" PRIuPTR"\n",
                                   Elements, elm);
  #ifndef NO_PRESERVED_KEY
                            if (Rc != 1) {
                                printf("Build with -DNO_PRESERVED_KEY");
                                printf(" to disable this test.");
                            }
  #endif // #ifndef NO_PRESERVED_KEY
                            FAILURE("J1N succeeded on last key J1Key", J1Key);
                        }
                        if (Elements == 1) {
                            break;
                        }
                        J1Key = 0;
                        Rc = Judy1First(J1, &J1Key, PJE0);
                    }
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                    if ((Rc != 1) || (J1Key == J1KeyBefore))
                    {
                        printf("\n");
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1LastKey 0x%zx\n", J1LastKey);
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1KeyBefore 0x%zx\n", J1KeyBefore);
                        printf("Rc %d\n", Rc);
                        printf("J1Key 0x%zx\n", J1Key);
                        printf("Elements %zu elm %zu\n", Elements, elm);
                        FAILURE("J1N failed J1Key", J1Key);
                    }
#ifdef TEST_NEXT_USING_JUDY_NEXT
                    J1KeyBefore = J1Key;
                    Rc = Judy1Next(J1, &J1Key, PJE0);
#endif // TEST_NEXT_USING_JUDY_NEXT
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
            Word_t JLKeyBefore;
            Word_t *PValue = NULL;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            JLKeyBefore = -1;
            JLKey = 0;
            PValue = (Word_t*)JudyLFirst(JL, &JLKey, PJE0);
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t JLLastKey = -1;
            JudyLLast(JL, &JLLastKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // #else TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(JLKey = GetNextKey(&WorkingSeed));
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    JLKeyBefore = JLKey;
                    PValue = (Word_t*)JudyLNext(JL, &JLKey, PJE0);
                    if (JLKeyBefore == JLLastKey)
                    {
                        if ((PValue != NULL)
  #ifndef NO_PRESERVED_KEY
                            || (JLKey != JLKeyBefore)
  #endif // #ifndef NO_PRESERVED_KEY
                            )
                        {
                            printf("\n");
                            printf("JLLastKey %" PRIuPTR"\n", JLLastKey);
                            printf("JLKeyBefore %" PRIuPTR"\n", JLKeyBefore);
                            printf("\nElements = %" PRIuPTR
                                   ", elm = %" PRIuPTR"\n",
                                   Elements, elm);
  #ifndef NO_PRESERVED_KEY
                            if (PValue == NULL) {
                                printf("Build with -DNO_PRESERVED_KEY");
                                printf(" to disable this test.");
                            }
  #endif // #ifndef NO_PRESERVED_KEY
                            FAILURE("JLN succeeded on last key JLKey", JLKey);
                        }
                        if (Elements == 1) {
                            break;
                        }
                        JLKey = 0;
                        PValue = (Word_t*)JudyLFirst(JL, &JLKey, PJE0);
                    }
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                    if ((PValue == NULL)
                        || (JLKey == JLKeyBefore)
                        // Could verify that PValue != PValueBefore.
                        || (VFlag && (*PValue != JLKey)))
                    {
                        printf("\n");
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("JLLastKey %" PRIuPTR"\n", JLLastKey);
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("JLKeyBefore %" PRIuPTR"\n", JLKeyBefore);
                        printf("JLKeyBefore 0x%zx PValue %p\n", JLKeyBefore, PValue);
                        if (PValue != NULL)
                        {
                            printf("*PValue=0x%" PRIxPTR"\n", *PValue);
                        }
                        printf("\nElements = %" PRIuPTR
                               ", elm = %" PRIuPTR"\n",
                               Elements, elm);
                        printf("JLN failed JLKey 0x%zx PValue %p\n", JLKey, PValue);
                        if (PValue != NULL) {
                            printf("JLN failed Value = 0x%zx\n", *PValue);
                        }
                        FAILURE("JLN failed JLKey", JLKey);
                    }
#ifdef TEST_NEXT_USING_JUDY_NEXT
                    JLKeyBefore = JLKey;
                    PValue = (Word_t*)JudyLNext(JL, &JLKey, PJE0);
#endif // TEST_NEXT_USING_JUDY_NEXT
                }
            }
            ENDTm(DeltanSecL);

            if (DminTime > DeltanSecL)
            {
                icnt = ICNT;
                if (DeltanSecL > 0.0)   // Ignore 0
                {
                    DminTime = DeltanSecL;
                }
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
TestJudyPrev(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
    (void)PSeed; // for TEST_NEXT_USING_JUDY_NEXT

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
            Word_t J1KeyBefore;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            J1KeyBefore = 0;
            J1Key = -1;
            Rc = Judy1Last(J1, &J1Key, PJE0);
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t J1FirstKey = 0;
            Judy1First(J1, &J1FirstKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // #else TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(J1Key = GetNextKey(&WorkingSeed));
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    J1KeyBefore = J1Key;
                    Rc = Judy1Prev(J1, &J1Key, PJE0);
                    if (J1KeyBefore == J1FirstKey)
                    {
                        if ((Rc == 1)
  #ifndef NO_PRESERVED_KEY
                            || (J1Key != J1KeyBefore)
  #endif // #ifndef NO_PRESERVED_KEY
                            )
                        {
                            printf("\n");
                            printf("J1FirstKey %" PRIuPTR"\n", J1FirstKey);
                            printf("J1KeyBefore %" PRIuPTR"\n", J1KeyBefore);
                            printf("\nElements = %" PRIuPTR
                                   ", elm = %" PRIuPTR"\n",
                                   Elements, elm);
  #ifndef NO_PRESERVED_KEY
                            if (Rc != 1) {
                                printf("Build with -DNO_PRESERVED_KEY");
                                printf(" to disable this test.");
                            }
  #endif // #ifndef NO_PRESERVED_KEY
                            FAILURE("J1P succeeded on first key J1Key", J1Key);
                        }
                        if (Elements == 1) {
                            break;
                        }
                        J1Key = -1;
                        Rc = Judy1Last(J1, &J1Key, PJE0);
                    }
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                    if ((Rc != 1) || (J1Key == J1KeyBefore))
                    {
                        printf("\n");
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1FirstKey %" PRIuPTR"\n", J1FirstKey);
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("J1KeyBefore %" PRIuPTR"\n", J1KeyBefore);
                        printf("\nElements = %" PRIuPTR
                               ", elm = %" PRIuPTR"\n",
                               Elements, elm);
                        FAILURE("J1P failed J1Key", J1Key);
                    }
#ifdef TEST_NEXT_USING_JUDY_NEXT
                    J1KeyBefore = J1Key;
                    Rc = Judy1Prev(J1, &J1Key, PJE0);
#endif // TEST_NEXT_USING_JUDY_NEXT
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
            Word_t JLKeyBefore;
            Word_t *PValue = NULL;
#ifdef TEST_NEXT_USING_JUDY_NEXT
            JLKeyBefore = 0;
            JLKey = -1;
            PValue = (Word_t*)JudyLLast(JL, &JLKey, PJE0);
#else // TEST_NEXT_USING_JUDY_NEXT
            Word_t JLFirstKey = 0;
            JudyLFirst(JL, &JLFirstKey, PJE0);
            NewSeed_t WorkingSeed = *PSeed;
#endif // #else TEST_NEXT_USING_JUDY_NEXT

            STARTTm;
            for (elm = 0; elm < Elements; elm++)
            {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                SYNC_SYNC(JLKey = GetNextKey(&WorkingSeed));
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                if (Tit)
                {
#ifndef TEST_NEXT_USING_JUDY_NEXT
                    JLKeyBefore = JLKey;
                    PValue = (Word_t*)JudyLPrev(JL, &JLKey, PJE0);
                    if (JLKeyBefore == JLFirstKey)
                    {
                        if ((PValue != NULL)
  #ifndef NO_PRESERVED_KEY
                            || (JLKey != JLKeyBefore)
  #endif // #ifndef NO_PRESERVED_KEY
                            )
                        {
                            printf("\n");
                            printf("JLFirstKey %" PRIuPTR"\n", JLFirstKey);
                            printf("JLKeyBefore %" PRIuPTR"\n", JLKeyBefore);
                            printf("\nElements = %" PRIuPTR
                                   ", elm = %" PRIuPTR"\n",
                                   Elements, elm);
  #ifndef NO_PRESERVED_KEY
                            if (PValue == NULL) {
                                printf("Build with -DNO_PRESERVED_KEY");
                                printf(" to disable this test.");
                            }
  #endif // #ifndef NO_PRESERVED_KEY
                            FAILURE("JLP succeeded on first key JLKey", JLKey);
                        }
                        if (Elements == 1) {
                            break;
                        }
                        JLKey = -1;
                        PValue = (Word_t*)JudyLLast(JL, &JLKey, PJE0);
                    }
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                    if ((PValue == NULL)
                        || (JLKey == JLKeyBefore)
                        || (VFlag && (*PValue != JLKey)))
                    {
                        printf("\n");
#ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("JLFirstKey %" PRIuPTR"\n", JLFirstKey);
#endif // #ifndef TEST_NEXT_USING_JUDY_NEXT
                        printf("JLKeyBefore %" PRIuPTR"\n", JLKeyBefore);
                        if (PValue != NULL)
                        {
                            printf("*PValue=0x%" PRIxPTR"\n", *PValue);
                        }
                        printf("\nElements = %" PRIuPTR
                               ", elm = %" PRIuPTR"\n",
                               Elements, elm);
                        FAILURE("JLP failed JLKey", JLKey);
                    }
#ifdef TEST_NEXT_USING_JUDY_NEXT
                    JLKeyBefore = JLKey;
                    PValue = (Word_t*)JudyLPrev(JL, &JLKey, PJE0);
#endif // TEST_NEXT_USING_JUDY_NEXT
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

//  perhaps a check should be done here -- if I knew what to expect.
    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyNextEmpty"

// Returns number of consecutive Keys
int
TestJudyNextEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
  #ifdef NO_TEST_NEXT_EMPTY
    (void)J1; (void)JL; (void)PSeed; (void)Elements;
    DeltanSecL = 0;
  #else // NO_TEST_NEXT_EMPTY
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
                    if ((Rc != 1)
                        && ((Judy1Count(J1, J1KeyBefore, -1, PJE0)
                                != -J1KeyBefore)
                            || (J1Key != J1KeyBefore)))
                    {
                        printf("\n");
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
                    if ((Rc != 1)
                        && ((JudyLCount(JL, JLKeyBefore, -1, PJE0)
                                != -JLKeyBefore)
                            || (JLKey != JLKeyBefore)))
                    {
                        FAILURE("JudyLNextEmpty Rcode != 1 Key", JLKeyBefore);
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
  #endif // !NO_TEST_NEXT_EMPTY
    return (0);
}

// Routine to time and test JudyPrevEmpty routines

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyPrevEmpty"

int
TestJudyPrevEmpty(void *J1, void *JL, PNewSeed_t PSeed, Word_t Elements)
{
  #ifdef NO_TEST_NEXT_EMPTY
    (void)J1; (void)JL; (void)PSeed; (void)Elements;
    DeltanSecL = 0;
  #else // NO_TEST_NEXT_EMPTY
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
                    if ((Rc != 1)
                        && (Judy1Count(J1, 0, J1KeyBefore, PJE0)
                            != (J1KeyBefore + 1)))
                    {
                        FAILURE("Judy1PrevEmpty Rc != 1 Key", J1KeyBefore);
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
                    if ((Rc != 1)
                        && (JudyLCount(JL, 0, JLKeyBefore, PJE0)
                            != (JLKeyBefore + 1)))
                    {
                        FAILURE("JudyLPrevEmpty Rc != 1 Key", JLKeyBefore);
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
  #endif // !NO_TEST_NEXT_EMPTY
    return (0);
}

#undef __FUNCTI0N__
#define __FUNCTI0N__ "TestJudyDel"

int
TestJudyDel(void **J1, void **JL, PNewSeed_t PSeed, Word_t Elements)
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

                        if (Rc != 0)
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
                    if (gFlag)
                    {
                        PWord_t   PValueNew;

                        PValueNew = (PWord_t)JudyLGet(*JL, TstKey, PJE0);
                        if (PValueNew == NULL)
                        {
                            printf("\n--- JudyLGet failure before JudyLDel"
                                   ", Key = 0x%" PRIxPTR,
                                   TstKey);
                            FAILURE("JudyLGet failure before JudyLDel",
                                    TstKey);
                        }
                        else if (*PValueNew != TstKey)
                        {
                            printf("\n--- JudyLGet wrong value before JudyLDel"
                                   ", Key = 0x%" PRIxPTR
                                   ", Value = 0x%" PRIxPTR"",
                                   TstKey, *PValueNew);
                            FAILURE("JudyLGet wrong value before JudyLDel",
                                    TstKey);
                        }
                    }

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

