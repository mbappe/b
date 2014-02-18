// ********************************************************************
// RANDOM NUMBER GENERATOR Code
// ********************************************************************

// requires #include <Judy.h>

// =======================================================================
// These are LFSF feedback taps for bitwidths of 10..64 sized numbers.
// Tested with Seed=0xc1fc to 35 billion numbers
// =======================================================================
//
// Word_t    StartSeed = 0xc1fc; tested default beginning number
//
// List of "magic" feedback points for Random number generator
static Word_t MagicList[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       // 0..9
    0x27f,                              // 10 All from 10..36 are Good 
    0x27f,                              // 11   37,39,40,41,43 + fail as noted
    0x27f,                              // 12   so -- may be sensitive to 
    0x27f,                              // 13   starting Seed.
    0x27f,                              // 14
    0x27f,                              // 15
    0x1e71,                             // 16
    0xdc0b,                             // 17
    0xdc0b,                             // 18
    0xdc0b,                             // 19
    0xdc0b,                             // 20
    0xc4fb,                             // 21
    0xc4fb,                             // 22
    0xc4fb,                             // 23
    0x13aab,                            // 24 
    0x11ca3,                            // 25
    0x11ca3,                            // 26
    0x11ca3,                            // 27
    0x13aab,                            // 28
    0x11ca3,                            // 29
    0xc4fb,                             // 30
    0xc4fb,                             // 31
    0x13aab,                            // 32 
    0x14e73,                            // 33  
    0x145d7,                            // 34  
    0x145f9,                            // 35  
    0x151ed,                            // 36 
    0x151ed,                            // 37 Bad    45094060053   (0xa7fd0c015)
    0x151ed,                            // 38 Good  274877906943  (0x3fffffffff)
    0x151ed,                            // 39 Bad   549751488513  (0x7fffbe0001)
    0x151ed,                            // 40 Bad   157053914551  (0x2491248db7)
    0x146c3,                            // 41 Bad  1924145348601 (0x1bffffffff9)
    0x146c3,                            // 42 Good 4398046511103 (0x3ffffffffff)
    0x146c3,                            // 43 Bad  6597069766653 (0x5fffffffffd) 
    0x146c3,                            // 44 following tested to 35 billion
    0x146c3,                            // 45  
    0x146c3,                            // 46  
    0x146c3,                            // 47  
    0x146c3,                            // 48  
    0x146c3,                            // 49  
    0x146c3,                            // 50  
    0x146c3,                            // 51  
    0x146c3,                            // 52  
    0x146c3,                            // 53  
    0x146c3,                            // 54  
    0x146c3,                            // 55  
    0x146c3,                            // 56  
    0x146c3,                            // 57  
    0x146c3,                            // 58  
    0x146c3,                            // 59  
    0x146c3,                            // 60  
    0x146c3,                            // 61  
    0x146c3,                            // 62  
    0x146c3,                            // 63  
    0x146c3                             // 64  
};

// Allow up to 8 generators sums to run
typedef struct RANDOM_GENERATOR_SEEDS
{
    Word_t    BitPos;                   // Most Significent Bit
    Word_t    OutputMask;               // Max number output
    Word_t    FeedBTap;                 // Feedback taps (from MagicList)
    Word_t    Order;                    // Style of Curves
    Word_t    Seeds[8];                 // Seeds for each order
//    Word_t    Count[8];                 // temp!!!!!

} Seed_t , *PSeed_t;

static Seed_t InitialSeeds =            // used to Init CurrentSeeds
{
    0x80000000,                         // Most significant bit
    0xffffffff,                         // Output mask
    0x13aab,                            // White noise is default
    0,                                  // Type 0
    {
     0xc1fc,                            // Seed 0
     12345,                             // Seed 1
     54321,                             // Seed 2
     11111,                             // Seed 3
     22222,                             // Seed 4
     33333,                             // Seed 5
     44444,                             // Seed 6
     55555                              // Seed 7
    }
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
    Word_t    MaxBits;

    CurrentSeeds = InitialSeeds;
    CurrentSeeds.BitPos = 1UL << (Bits - 1);
    CurrentSeeds.OutputMask = (CurrentSeeds.BitPos * 2) - 1;
    CurrentSeeds.FeedBTap = MagicList[Bits];
    switch (Order)
    {
    case 0:
        CurrentSeeds.Order = 0;
        MaxBits = 64;
        break;
    case 1:
        CurrentSeeds.Order = 1;
        MaxBits = 63;
        break;
    case 2:
        CurrentSeeds.Order = 2;
        MaxBits = 62;
        break;
    case 3:
        CurrentSeeds.Order = 3;
        MaxBits = 62;
        break;
    case 4:
        CurrentSeeds.Order = 7;
        MaxBits = 61;
        break;
    default:
        return ((PSeed_t) NULL);
    }
    if (sizeof(Word_t) != 8)
        MaxBits = 32;

    if (Bits < 10 || Bits > MaxBits)    // cannot be larger that Word_t size
        return ((PSeed_t) NULL);

    return (&CurrentSeeds);
}


static inline Word_t                    // for performance
RandomNumb(PSeed_t PSeed, Word_t SValue)
{
    uint64_t  seedsum = 0;              // temp
    Word_t    order = 0;
    Word_t    seed;

    if (SValue)
    {
        PSeed->Seeds[0] += SValue;
        return(PSeed->Seeds[0]);
    }

    do
    {
        seed = PSeed->Seeds[order];
        if (seed & PSeed->BitPos)       // If carry, feedback
        {
            seed <<= 1;                 // double
            seed ^= PSeed->FeedBTap;
        }
        else
            seed <<= 1;

        seed &= PSeed->OutputMask;
        PSeed->Seeds[order] = seed;
//        if ((seed == InitialSeeds.Seeds[order]) & PSeed->Count[order])
//        {
//            printf("OOps duplicate number = 0x%lx, order = %d, Count = %lu\n", seed, (int)order, PSeed->Count[order]);
//            exit(1);
//        }
//        PSeed->Count[order]++;
        seedsum += seed;
    }
    while (order++ != CurrentSeeds.Order);

    switch (CurrentSeeds.Order)
    {
    case 0:
        return (seedsum);
    case 1:
        return (seedsum >> 1);
    case 2:
        return (seedsum / 3);
    case 3:
        return (seedsum >> 2);
    case 7:
        return (seedsum >> 3);
    }
    return (0);
}
