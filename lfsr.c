#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <assert.h>
#ifdef USE_PDEP_INTRINSIC
#include <immintrin.h>
#endif // USE_PDEP_INTRINSIC

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

static inline Word_t
PDEP(Word_t wSrc, Word_t wMask)
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

#if 0 // We're using SplayedFastLfsr now.

#define FAST_PREV(_wSeed, _wMagic, _nBitsM1) \
    ( (((_wSeed) ^ ((_wMagic) & -((_wSeed) >> (_nBitsM1)))) << 1) \
        | ((_wSeed) >> (_nBitsM1)) )

static inline Word_t
FastLfsr(Word_t wSeed, Word_t wMagic, int nBitsM1, int bPrev)
{
    Word_t wNext = wSeed;
    if (bPrev) {
#ifdef USE_MACROS
        wNext = FAST_PREV(wNext, wMagic, nBitsM1);
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

#endif // 0

#define SPLAYED_FAST_PREV(_wSeed, _wMagic, _nBitsM1, _nBitsShift) \
    ( (((_wSeed) ^ ((_wMagic) & -((_wSeed) >> (_nBitsM1)))) << (_nBitsShift)) \
        | ((_wSeed) >> (_nBitsM1)) )

static inline Word_t
SplayedFastLfsr(Word_t wSplayedSeed,
                Word_t wSplayedMagic, int nBitsM1, int nBitsShift, int bPrev)
{
    Word_t wNext = wSplayedSeed;
    if (bPrev) {
#ifdef USE_MACROS
        wNext = SPLAYED_FAST_PREV(wNext, wSplayedMagic, nBitsM1, nBitsShift);
#else // USE_MACROS
        Word_t wbMsbSet = wNext >> nBitsM1;
        Word_t wXorOperand = wSplayedMagic & -wbMsbSet;
        wNext = wNext ^ wXorOperand;
        wNext = wNext << nBitsShift;
        wNext = wNext | wbMsbSet;
        // No need to mask; xor takes out high bit.
#endif // USE_MACROS
    } else {
        Word_t wbLsbSet = wSplayedSeed & 1;
        //Word_t wXorOperand = wSplayedMagic * wbLsbSet;
        Word_t wXorOperand = wSplayedMagic & -wbLsbSet;
        wNext >>= nBitsShift;
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
#ifndef cnBitsShift
#define cnBitsShift  2 // pick a number for splay
#endif // cnBitsShift
    printf("cnBitsShift %d\n", cnBitsShift);
    Word_t wLsbs = (Word_t)-1 / ((1 << cnBitsShift) - 1);
    wLsbs >>= sizeof(Word_t) * 8 % cnBitsShift;
    printf("wLsbs 0x%" Owx"\n", wLsbs);
    Word_t wSplayedSeed = PDEP(wSeed, wLsbs);
    printf("wSplayedSeed 0x%" Owx"\n", wSplayedSeed);
    Word_t wSplayedMagic = PDEP(wMagic, wLsbs);
    printf("wSplayedMagic 0x%" Owx"\n", wSplayedMagic);
    if (cAlg == 'F') {
        wNext = wSplayedSeed;
        wMagic = wSplayedMagic;
        nBitsM1 *= cnBitsShift;
    }
#ifndef NDEBUG
    Word_t wBack;
#endif // NDEBUG
    do {
        if (bPrint) { printf("0x%" Owx"\n", wNext); }
        wPrev = wNext;
        switch (cAlg) {
        case 'F':     
            //wNext = FastLfsr(wNext, wMagic, nBitsM1, bPrev);
            wNext = SplayedFastLfsr(wNext, wMagic,
                                    nBitsM1, cnBitsShift, bPrev);
#ifndef NDEBUG
            //wBack = FastLfsr(wNext, wMagic, nBitsM1, !bPrev);
            wBack = SplayedFastLfsr(wNext, wMagic,
                                    nBitsM1, cnBitsShift, !bPrev);
#endif // NDEBUG
            break;
        case 'f':     
            //wNext = FastLfsr(wNext, wMagic, nBitsM1, bPrev);
            wNext = SplayedFastLfsr(wNext, wMagic, nBitsM1, 1, bPrev);
#ifndef NDEBUG
            //wBack = FastLfsr(wNext, wMagic, nBitsM1, !bPrev);
            wBack = SplayedFastLfsr(wNext, wMagic, nBitsM1, 1, !bPrev);
#endif // NDEBUG
            break;
        case 'o':     
            wNext = OldLfsr(wNext, wMagic, nBitsM1, wMask, bPrev);
#ifndef NDEBUG
            wBack = OldLfsr(wNext, wMagic, nBitsM1, wMask, !bPrev);
#endif // NDEBUG
            break;
        case 'p':     
            wNext = ParityLfsr(wNext, wMagic, nBitsM1, wMask, bPrev);
#ifndef NDEBUG
            wBack = ParityLfsr(wNext, wMagic, nBitsM1, wMask, !bPrev);
#endif // NDEBUG
            break;
        }
#ifndef NDEBUG
        if (wBack != wPrev) {
            printf("wNext 0x%" Owx" wPrev 0x%" Owx" wBack 0x%" Owx"\n",
                   wNext, wPrev, wBack);
        }
        assert(wBack == wPrev);
#endif // NDEBUG
        ++wPeriod;
    } while (wNext != wSeed);
    return wPeriod;
}

// Command name aka argv[0]
//   splayed-fast: Flfsrn, Flfsrnp, Flfsrp, Flfsrpp
//           fast: flfsrn, flfsrnp, flfsrp, flfsrpp
//         parity: plfsrn, plfsrnp, plfsrp, plfsrpp
//            old: olfsrn, olfsrnp, olfsrp, olfsrpp

// Usage: [Ffpo]lfsr[nr][p] [wBits [wSeed [wMagic]]]

int
main(int argc, char *argv[])
{
    if (argc > 4)
    {
        printf("Too many command-line arguments.\n");
        printf("Usage: [Ffpo]lfsr[nr][p] [wBits [wSeed [wMagic]]]\n");
        exit(1);
    }

    char *strName = basename(argv[0]);
    int len = strlen(strName);
    char cAlg = strName[0];

    int bPrev = 0;
    if (strlen(strName) >= strlen("flfsrn")) {
        bPrev = (strName[strlen("flfsrn") - 1] == 'p');
    }
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
        wPeriod = lfsr(wBits,
                       cAlg == 'f' || cAlg == 'F' ? wMagic : wReverseMagic,
                       wSeed, cAlg, bPrev, /* bPrint */ 1);
    } else {
        switch (cAlg)
        {
        case 'p':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wReverseMagic, wSeed,
                           'p', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wReverseMagic, wSeed,
                           'p', /* bPrev */ 0, /* bPrint */ 0);
            break;
        case 'o':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wReverseMagic, wSeed,
                           'o', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wReverseMagic, wSeed,
                           'o', /* bPrev */ 0, /* bPrint */ 0);
            break;
        case 'f':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wMagic, wSeed,
                           'f', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wMagic, wSeed,
                           'f', /* bPrev */ 0, /* bPrint */ 0);
            break;
        case 'F':
            wPeriod
                = bPrev
                    ? lfsr(wBits, wMagic, wSeed,
                           'F', /* bPrev */ 1, /* bPrint */ 0)
                    : lfsr(wBits, wMagic, wSeed,
                           'F', /* bPrev */ 0, /* bPrint */ 0);
            break;
        default:
            printf("Invalid cAlg '%c'\n", cAlg);
            printf("Usage: [Ffpo]lfsr[nr][p] [wBits [wSeed [wMagic]]]\n");
            exit(1);
        }
    }
    printf("wPeriod 0x%" Owx"\n", wPeriod);
    Word_t wTargetPeriod = ((Word_t)1 << nBitsM1) * 2 - 1;
    if (wPeriod != wTargetPeriod) {
        printf("wTargetPeriod 0x%" Owx"\n", wTargetPeriod);
        exit(1);
    }
    exit(0);
}

