// @(#) $Revision: 1.1 $ $Source: /home/doug/judy-1.0.5_dlmalloc/test/RCS/RandomNumb.h,v $
/* RANDOM NUMBER GENERATOR Code                                         */

#include <Judy.h>

// Notes on ifdefs:
// NO_SVALUE means do not support non-zero SValue.
// NO_LFSR means support ONLY non-zero SValue.
// IF_SVALUE means support non-zero SValue via conditional branch.
// GAUSS means support non-zero PSeed->order aka GValue from -G <GValue>.
// !defined(NO_SVALUE) && !defined(IF_SVALUE) means support non-zero SValue
// via no-branch, multiply by bSValue method.

// Default is IF_SVALUE ifndef NO_IF_SVALUE.
#ifndef NO_IF_SVALUE
#ifndef NO_SVALUE
#ifndef NO_LFSR
#define IF_SVALUE
#endif // NO_LFSR
#endif // NO_SVALUE
#endif // NO_IF_SVALUE

// Default is GAUSS ifndef NO_GAUSS.
#ifndef NO_GAUSS
#define GAUSS
#endif // NO_GAUSS

// =======================================================================
// These are LFSF feedback taps for bitwidths of 10..64 sized numbers.
// Tested with Seed=0xc1fc to 35 billion numbers
// =======================================================================
//
// Word_t    StartSeed = 0xc1fc; tested default beginning number
//
// List of "magic" feedback points for Random number generator
static Word_t MagicList[] = {
    0, 0,                       // 0..1
    0x3,                        // 2
    0x5,                        // 3
    0x9,                        // 4
    0x12,                       // 5
    0x21,                       // 6
    0x41,                       // 7
    0x8E,                       // 8
    0x108,                      // 9
    0x204,                      // 10
    0x402,                      // 11
    0x829,                      // 12
    0x100D,                     // 13
    0x2015,                     // 14
    0x4001,                     // 15
    0x8016,                     // 16
    0x10004,                    // 17
    0x20013,                    // 18
    0x40013,                    // 19
    0x80004,                    // 20
    0x100002,                   // 21
    0x200001,                   // 22
    0x400010,                   // 23
    0x80000D,                   // 24
    0x1000004,                  // 25
    0x2000023,                  // 26
    0x4000013,                  // 27
    0x8000004,                  // 28
    0x10000002,                 // 29
    0x20000029,                 // 30
    0x40000004,                 // 31
    0x80000057,                 // 32
#ifdef __LP64__
    0x100000029,                // 33
    0x200000073,                // 34
    0x400000002,                // 35
    0x80000003B,                // 36
    0x100000001F,               // 37
    0x2000000031,               // 38
    0x4000000008,               // 39
    0x800000001C,               // 40
    0x10000000004,              // 41
    0x2000000001F,              // 42
    0x4000000002C,              // 43
    0x80000000032,              // 44
    0x10000000000D,             // 45
    0x200000000097,             // 46
    0x400000000010,             // 47
    0x80000000005B,             // 48
    0x1000000000038,            // 49
    0x200000000000E,            // 50
    0x4000000000025,            // 51
    0x8000000000004,            // 52
    0x10000000000023,           // 53
    0x2000000000003E,           // 54
    0x40000000000023,           // 55
    0x8000000000004A,           // 56
    0x100000000000016,          // 57
    0x200000000000031,          // 58
    0x40000000000003D,          // 59
    0x800000000000001,          // 60
    0x1000000000000013,         // 61
    0x2000000000000034,         // 62
    0x4000000000000001,         // 63
    0x800000000000000D,         // 64
#endif  // __LP64__

};

// Allow up to 8 generators sums to run
// Caution: the order of the fields impacts timing
typedef struct RANDOM_GENERATOR_SEEDS
{
    Word_t    FeedBTap;                 // Feedback taps (from MagicList)
#ifdef GAUSS
    Word_t    Seeds[8];                 // Seeds for each order
    Word_t    Order;                    // Style of Curves
#else // GAUSS
    Word_t    Seeds[1];
#endif // GAUSS
#ifndef NO_SVALUE
  #ifndef IF_SVALUE
    int s_bSValue;
  #endif // IF_SVALUE
    Word_t    OutputMask;               // Max number output
#endif // NO_SVALUE
//    Word_t    Count[8];                 // temp!!!!!

} Seed_t , *PSeed_t;

static Seed_t InitialSeeds =            // used to Init CurrentSeeds
{
    0x13aab,                            // FeedBTap; White noise is default
    {
     0xc1fc,                            // Seed 0
#ifdef GAUSS
     12345,                             // Seed 1
     54321,                             // Seed 2
     11111,                             // Seed 3
     22222,                             // Seed 4
     33333,                             // Seed 5
     44444,                             // Seed 6
     55555                              // Seed 7
#endif // GAUSS
    },
#ifdef GAUSS
    0,                                  // Order
#endif // GAUSS
#ifndef NO_SVALUE
  #ifndef IF_SVALUE
    0,                                  // s_bSValue
  #endif // IF_SVALUE
    0xffffffff,                         // Output mask
#endif // NO_SVALUE
};

static Seed_t CurrentSeeds;

// Initalize Pseuto Random Number generator
// Caution: the Gaussian generator has numbers that repeat
// The Flat generator does not repeat until all numbers (except 0) are output
//    Bits == maximum random (number - 1) that can be returned
//    SpectrumKind == 0 > return a Gaussian spectrum
//    SpectrumKind == 1 > return a flat spectrum
//    Exit pointer to control buffer

#define FLAT 0
#define PYRAMID 1
#define GAUSSIAN1 2
#define GAUSSIAN2 3
#define GAUSSIAN3 4
#define GAUSSIAN4 5
#define GAUSSIAN5 6
#define GAUSSIAN6 7

static    PSeed_t
RandomInit(Word_t Bits, Word_t Order)
{
    (void)Order;
    CurrentSeeds = InitialSeeds;
#ifndef NO_SVALUE
  #ifndef IF_SVALUE
    extern Word_t SValue;
    CurrentSeeds.s_bSValue = (SValue != 0);
  #endif // IF_SVALUE
    CurrentSeeds.OutputMask = (((Word_t)1 << (Bits - 1)) * 2) - 1;
#endif // NO_SVALUE
    CurrentSeeds.FeedBTap = MagicList[Bits];
    Word_t MaxBits = sizeof(Word_t) * 8;
#ifdef GAUSS
    Word_t aOrders[] = { 0, 1, 2, 3, 7 };
    if (Order > sizeof(aOrders) / sizeof(aOrders[0])) {
        return NULL;
    }
    CurrentSeeds.Order = aOrders[Order];
    MaxBits -= CurrentSeeds.Order;
#endif // GAUSS
    if (Bits < 10 || Bits > MaxBits)    // cannot be larger than Word_t size
        return ((PSeed_t) NULL);
    return (&CurrentSeeds);
}

static inline Word_t                    // for performance
RandomNumb(PSeed_t PSeed, Word_t SValue)
{
#ifdef NO_SVALUE
  #ifdef IF_SVALUE
    #error (NO_SVALUE && IF_SVALUE) makes no sense
  #endif // IF_SVALUE
  #ifdef NO_LFSR
    #error (NO_SVALUE && NO_LFSR) makes no sense
  #endif // NO_LFSR
    (void)SValue;
#endif // NO_SVALUE
    Word_t order = 0;

    // -S<x> allows the user to specify the first number generated by
    // RandomNumb. But it doesn't work with non-zero -G. We haven't figured
    // out how to do it right yet.
    Word_t wNumb = PSeed->Seeds[order];
// With IF_SVALUE defined RandomNumb avoids the lfsr work when SValue != 0.
// The cost is different overhead behavior, including the very predictable
// conditional branch, between SValue == 0 and SValue != 0.
// Still, IF_SVALUE might be valuable by speeding things up when doing a lot
// of work with -DS1.
#ifdef IF_SVALUE // implies !defined(NO_SVALUE)
  #ifdef NO_LFSR
    #error (NO_LFSR && IF_SVALUE) makes no sense
  #endif // NO_LFSR
    if (SValue != 0)
#endif // IF_SVALUE
#if defined(NO_LFSR) || defined(IF_SVALUE)
    {
        PSeed->Seeds[order] = (wNumb + SValue) & PSeed->OutputMask;
  #if defined(NEXT_FIRST)
        return PSeed->Seeds[order];
  #else // NEXT_FIRST
        return wNumb;
  #endif // NEXT_FIRST
    }
#endif // defined(NO_LFSR) || defined(IF_SVALUE)
#ifndef NO_LFSR
    Word_t seed = wNumb;

    // Is it faster to save and read bSValue to/from *PSeed or
    // to use popcount to calculate it on every call?
    // Preliminary answer is read from memory is faster
    // for no -K and possibly a tiny bit faster  with -K.
    //
    // int bSValue = __builtin_popcount(SValue & -SValue);
  #ifdef GAUSS
    uint64_t  seedsum = 0;              // temp
    goto first;
    do
    {
        seed = PSeed->Seeds[order];
first:
  #endif // GAUSS

        seed = (seed >> 1) ^ (PSeed->FeedBTap & -(seed & 1));

  #if !defined(NO_SVALUE) && !defined(IF_SVALUE)
        //seed *= (SValue == 0);
        //seed *= !bSValue;
        seed *= !PSeed->s_bSValue;

        //seed += (wNumb + SValue) * (SValue != 0);
        //seed += (wNumb + SValue) * bSValue;
        seed += (wNumb + SValue) * PSeed->s_bSValue;
  #endif // !defined(NO_SVALUE) && !defined(IF_SVALUE)
        //seed &= (((Word_t)1 << PSeed->s_nBitsM1) << 1) - 1;
  #if !defined(IF_SVALUE) && !defined(NO_SVALUE)
        seed &= PSeed->OutputMask;
  #endif // !defined(IF_SVALUE) && !defined(NO_SVALUE)
        PSeed->Seeds[order] = seed;
//        if ((seed == InitialSeeds.Seeds[order]) & PSeed->Count[order])
//        {
//            printf("OOps duplicate number = 0x%lx, order = %d, Count = %lu\n", seed, (int)order, PSeed->Count[order]);
//            exit(1);
//        }
//        PSeed->Count[order]++;
  #ifdef GAUSS
        seedsum += seed;
    }
    while (order++ != CurrentSeeds.Order);

    // Be careful. GValue != 0 always acts like -DNEXT_FIRST.
    switch (CurrentSeeds.Order)
    {
    case 1:
        return (seedsum >> 1);
    case 2:
        return (seedsum / 3);
    case 3:
        return (seedsum >> 2);
    case 7:
        return (seedsum >> 3);
    }
  #endif // GAUSS
  #ifdef NEXT_FIRST
    return seed;
  #else // NEXT_FIRST
    return wNumb;
  #endif // NEXT_FIRST
#endif // NO_LFSR
}
