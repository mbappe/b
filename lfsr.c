#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <assert.h>

#if defined(__LP64__)
#define Owx "016lx"
#elif defined(_WIN64)
#define Owx "016llx"
#else // defined(__LP64__)
#define Owx "08x"
#endif // defined(__LP64__)

typedef uintptr_t Word_t;

#define LOG(_x) (63 - __builtin_clzll(_x))

static Word_t wMagicList[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       // 0..9
    0x27f,                              // 10 All from 10..36 are Good (Gd)
    0x27f,                              // 11 37,39,40,41,43 + fail as noted
    0x27f,                              // 12 so -- may be sensitive to 
    0x27f,                              // 13 starting Seed.
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
    0x13aab                             // 32 

#ifdef __LP64__
    ,
    0x14e73UL,                          // 33  
    0x145d7UL,                          // 34  
    0x145f9UL,                          // 35  
    0x151edUL,                          // 36 
    0x1F00000001,                       // 37 Good  137438953471  (0x1fffffffff) 
    0x151edUL,                          // 38 Good  274877906943  (0x3fffffffff)
    0x151edUL,                          // 39 Good  549755813887  (0x7fffffffff) 
    0x151edUL,                          // 40 Bad   157053914551  (0x2491248db7)
    0x146c3UL,                          // 41 Bad  1924145348601 (0x1bffffffff9)
    0x146c3UL,                          // 42 Good 4398046511103 (0x3ffffffffff)
    0x146c3UL,                          // 43 Bad  6597069766653 (0x5fffffffffd) 
    0x146c3UL,                          // 44 following tested to 35 billion
    0x146c3UL,                          // 45  
    0x146c3UL,                          // 46  
    0x146c3UL,                          // 47  
    0x146c3UL,                          // 48  
    0x146c3UL,                          // 49  
    0x146c3UL,                          // 50  
    0x146c3UL,                          // 51  
    0x146c3UL,                          // 52  
    0x146c3UL,                          // 53  
    0x146c3UL,                          // 54  
    0x146c3UL,                          // 55  
    0x146c3UL,                          // 56  
    0x146c3UL,                          // 57  
    0x146c3UL,                          // 58  
    0x146c3UL,                          // 59  
    0x146c3UL,                          // 60  
    0x146c3UL,                          // 61  
    0x146c3UL,                          // 62  
    0x146c3UL,                          // 63  
    0x146c3UL                           // 64  
#endif  // __LP64__

};

Word_t
BitReverse(Word_t word)
{
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

static inline Word_t
ParityLfsr(Word_t wSeed, Word_t wMagic, int nBitsM1, Word_t wMask, int bPrev)
{
   Word_t wNext = wSeed;
   if (bPrev) {
       wNext <<= 1;
       wNext &= wMask;
       wNext |= __builtin_parity(wNext & wMagic) ^ (wSeed >> nBitsM1);
   } else {
       Word_t wbParity = __builtin_parity(wSeed & wMagic);
       Word_t wMsb = wbParity << nBitsM1;
       wNext >>= 1; 
       wNext |= wMsb;
   }
   return wNext;
}

#define FAST_PREV(_wSeed, _wMagic, _nBitsM1, _wMask) \
    ( (((_wSeed) ^ ((_wMagic) & -((_wSeed) >> (_nBitsM1)))) << 1) \
        | ((_wSeed) >> (_nBitsM1)) /* & (_wMask) */ )

static inline Word_t
FastLfsr(Word_t wSeed, Word_t wMagic, int nBitsM1, Word_t wMask, int bPrev)
{
    Word_t wNext = wSeed;
    if (bPrev) {
#ifdef USE_MACROS
        wNext = FAST_PREV(wNext, wMagic, nBitsM1, wMask);
#else // USE_MACROS
        Word_t wbMsbSet = wNext >> nBitsM1;
        Word_t wXorOperand = wMagic & -wbMsbSet;
        wNext = wNext ^ wXorOperand;
        wNext = wNext << 1;
        wNext = wNext | wbMsbSet;
        // No need to mask; xor takes out high bit.
#endif // USE_MACROS
    } else {
        Word_t wbLsbSet = wSeed & 1;
        //Word_t wXorOperand = wMagic * wbLsbSet;
        Word_t wXorOperand = wMagic & -wbLsbSet;
        wNext >>= 1;
        wNext ^= wXorOperand;
    }
    return wNext;
}

static inline Word_t
OldLfsr(Word_t wSeed, Word_t wMagic, int nBitsM1, Word_t wMask, int bPrev)
{
    Word_t wNext = wSeed;
    if (bPrev) {
        Word_t wbLsbSet = wSeed & 1;
        Word_t wXorOperand = wMagic & -wbLsbSet;
        wNext ^= wXorOperand;
        wNext >>= 1;
        wNext |= wbLsbSet << nBitsM1;
    } else {
        Word_t wbMsbSet = wSeed >> nBitsM1;
        Word_t wXorOperand = wMagic & -wbMsbSet;
        wNext <<= 1;
        wNext ^= wXorOperand;
        wNext &= wMask;
    }
    return wNext;
}

static inline Word_t
lfsr(int nBits, Word_t wMagic, Word_t wSeed, char cAlg, int bPrev, int bPrint)
{
    int nBitsM1 = nBits - 1;
    Word_t wMask = (((Word_t)1 << nBitsM1) << 1) - 1;
    Word_t wNext = wSeed;
    Word_t wPrev;
    Word_t wPeriod = 0;
    do {
        if (bPrint) { printf("0x%" Owx"\n", wNext); }
        wPrev = wNext;
        wNext = (cAlg == 'p') ? ParityLfsr(wNext, wMagic,
                                           nBitsM1, wMask, bPrev)
              : (cAlg == 'o') ?    OldLfsr(wNext, wMagic,
                                           nBitsM1, wMask, bPrev)
              :                   FastLfsr(wNext, wMagic,
                                           nBitsM1, wMask, bPrev);
        assert((  (cAlg == 'p') ? ParityLfsr(wNext, wMagic,
                                             nBitsM1, wMask, !bPrev)
                : (cAlg == 'o') ?    OldLfsr(wNext, wMagic,
                                             nBitsM1, wMask, !bPrev)
                :                   FastLfsr(wNext, wMagic,
                                             nBitsM1, wMask, !bPrev) )
            == wPrev);
        ++wPeriod;
    } while (wNext != wSeed) ;
    return wPeriod;
}

//   fast: flfsrn, flfsrnp, flfsrp, flfsrpp
// parity: plfsrn, plfsrnp, plfsrp, plfsrpp
//    old: olfsrn, olfsrnp, olfsrp, olfsrpp

int
main(int argc, char *argv[])
{
    char *strName = basename(argv[0]);
    int len = strlen(strName);
    char cAlg = strName[0];
    int bPrev = (strName[strlen("flfsrnp") - 2] == 'p');
    int bPrint = (len == strlen("flfsrnp"));

    Word_t wBits = (argc > 1) ? strtoul(argv[1], 0, 0) : 12;
    int nBitsM1 = wBits - 1;
    Word_t wMask = (((Word_t)1 << nBitsM1) << 1) - 1;
    Word_t wSeed  = (argc > 2) ? strtoul(argv[2], 0, 0) : 1;
    Word_t wMagic = (argc > 3) ? strtoul(argv[3], 0, 0) : wMagicList[wBits];
    Word_t wReverseMagic = BitReverse(wMagic) >> ((sizeof(wMagic) * 8) - wBits);

    printf("cAlg %c\n", cAlg);
    printf("bPrev %d\n", bPrev);
    printf("bPrint %d\n", bPrint);
    printf("argc %d\n", argc);
    printf("wBits %zd nBitsM1 %d wMask 0x%zx\n", wBits, nBitsM1, wMask);
    printf("wSeed 0x%zx\n", wSeed);
    printf("wMagic 0x%zx\n", wMagic);
    printf("wReverseMagic 0x%zx\n", wReverseMagic);

    Word_t wPeriod;
    if (bPrint) {
        wPeriod = lfsr(wBits, cAlg == 'f' ? wReverseMagic : wMagic, wSeed,
                       cAlg, bPrev, /* bPrint */ 1);
    } else {
        switch (cAlg)
        {
        case 'p':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wMagic, wSeed,
                           'p', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wMagic, wSeed,
                           'p', /* bPrev */ 0, /* bPrint */ 0);
        case 'o':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wMagic, wSeed,
                           'o', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wMagic, wSeed,
                           'o', /* bPrev */ 0, /* bPrint */ 0);
        case 'f':
        default:
            wPeriod
                = bPrev
                    ? lfsr(wBits, wReverseMagic, wSeed,
                           'f', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wReverseMagic, wSeed,
                           'f', /* bPrev */ 0, /* bPrint */ 0);
        }
    }
    printf("wPeriod 0x%" Owx"\n", wPeriod);
}

