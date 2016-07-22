
// Purpose of this program is to generate keys that will aggravate Judy using
// white-box information.
//
// 64-bit released Judy1 version 1.0.5:
// Leaf 8 up to 31 keys then splay to Branch 7 with Leaf 7s.
// Linear Branch at any level holds up to 7 JPs. We also uncompress when
// the population of the branch and below exceeds 1000.
// Bitmap Branch at any level holds up to 184 JPs. Or more because sometimes we
// delay uncompress to amortize cost of multiple uncompressions. We also
// uncompress when the population of the entire tree exceeds 750 and the
// poplulation of the branch and below exceeds 135.
// - Population of array must increase by at least 300 keys between Bitmap
// Branch uncompressions.
// Immediates hold up to 15 bytes worth of keys:
// - 7:2, 6:2, 5:3, 4:3, 3:5, 2:7, 1:15
// Leaf 7 is up to 256 bytes holding up to 36 keys.
// Leaf 6 is up to 256 bytes holding up to 42 keys.
// Leaf 5 is up to 256 bytes holding up to 51 keys.
// Leaf 4 is up to 256 bytes holding up to 64 keys.
// Leaf 3 is up to 256 bytes holding up to 85 keys.
// Leaf 2 is up to 256 bytes holding up to 128 keys.
// No Leaf1.
// Leaf and Bitmap Branch components allocation steps (in words):
// - 3, 5, 7, 11, 15, 23, 32, 47, 64
// Leaf holds keys and nothing else. Sorted smallest to largest.
// Structure of Bitmap Branch:
// - 4 bitmaps and 4 pointers; the pointers point to arrays of JPs
// Structure of a Linear Branch:
// - 7 JPs plus one word of seven one-byte indexes and one one-byte count of
// number of valid JPs.
// Structure of an Uncompressed Branch:
// - 256 JPs.
// Structure of a JP:
// - one word contains seven-byte DCD (which comprises prefix and pop) and
//   one-byte type field
//   - no DCD for immediate (immediate has 15-bytes of keys)
// - other word is a pointer to the object that is specified by the type field
// One-word root pointer and seven-byte prefix/pop are both reasons why we
// can't have a narrow pointer to the top branch.
// Pointers are portable by virtue of type field describing the level of the
// object pointed to. Think narrow pointers.
// Type field encodes the population of immediates.
// All list searches are binary except immeds.
// When Leaf 8 reaches pop of around 31 we insert a JPM between the root
// pointer and the top level Branch or Leaf 8.
// - JPM has: pop, JP to first Branch or Leaf, pop of last BB uncompression,
// errno, source line no, total words malloced
// - Judy Population and Memory.

// Key differences for JudyL:
// Value areas.
// Leaf 1 holds up to 40 keys.
// Immeds have only 7 bytes for keys. The other word is a pointer to the value
// area.
// Structure of Bitmap Leaf:
// - 4 bitmaps and 4 pointers to value areas (if only one bit set, then value
//   area pointer is used as value area)

// Key differences for enhanced, unreleased Judy:
// Leaf and Bitmap Branch components allocation steps (in words):
// Judy1 allocations: 3, 5, 7, 9, 11, 15, 23, 33, 47, 65, 95, 129
// Leaf2 max pop for JudyL is 52 keys.
// Leaf3 max pop for JudyL is 47 keys.
// Leaf4 max pop for JudyL is 43 keys.
// Leaf5 max pop for JudyL is 38 keys.
// Judy1 allocations: 1, 3, 5, 9, 15, 25, 41, 67, 109, 177
// Leaf2 max pop for Judy1 is 100 keys. 25 words.
// Leaf3 max pop for Judy1 is  66 keys. 25 words.
// Leaf4 max pop for Judy1 is  50 keys. 25 words.
// Leaf5 max pop for Judy1 is  40 keys. 40 words. 1 word per key.
// Reduced number of JPs required to 86 for uncompress of a Bitmap Branch.
// Reduced population delta to 150 required between uncompresses.
// Reduced maximum Leaf 1 pop for JudyL to 12 keys / 14 words; 1.5 words of
// keys.
// Leaf and Bitmap Branch components allocation steps (in words) is in flux:
// 1, 3, 5, 9, 15, 25, 41, 67, 109, 177
// Improved DCD check method: conditional goto in case before looping back to
// switch, byteswap instead of one word at a time.
// Non-power-of-two key-size lists are now swapped and compared instead of
// byte-by-byte.
// Psplit search for all lists including immediates. Exploit this?

// Key differences for 32-bit? Worry about it later.
// Leaf1 exists for Judy1. 41 keys for released. Shrinks to 20 keys for
// unreleased version.

// Is it possible to visit more than one Bitmap Branch on one key lookup?
// Splay on 32nd key into 7 JPs with one immediate plus one JP with a 25-key
// Leaf 7.
// Grow Leaf 7 to 37 keys and splay into 7 JPs with one immediate plus one
// JP with 30 keys.
// Grow Leaf 6 to 43 keys and splay into 7 JPs with one immediate plus one
// JP with 36 keys.
// Grow Leaf 5 to 52 keys and splay into 7 JPs with one immediate plus one
// JP with 45 keys.
// Grow Leaf 4 to 64 keys and splay into 7 JPs with one immediate plus one
// JP with 57 keys.
// Grow Leaf 3 to 85 keys and splay into 7 JPs with one immediate plus one
// JP with a Leaf 2 with 78 keys.
// How about Bitmap Branches and 3 and 4 replicated 4G times for a total
// pop of 4G * 92 = 400G?
// Or Bitmap Branches and 3, 4 and 5 replicated 24M times for a total
// pop of 4G * 92 = 400G?

// 106-0, [(1<< 8)+14, 1<< 8], [(2<< 8)+14, 2<< 8], [(3<< 8)+14, 3<< 8],
//       [(4<< 8)+14, 4<< 8], [(5<< 8)+14, 5<< 8], [(6<< 8)+14, 6<< 8],
//       [(7<< 8)+14, 7<< 8],
// 2^48 branches like the one above.
//       [(1<<16)+ 6, 1<<16], [(2<<16)+ 6, 2<<16], [(3<<16)+ 6, 3<<16],
//       [(4<<16)+ 6, 4<<16], [(5<<16)+ 6, 5<<16], [(6<<16)+ 6, 6<<16],
//       [(7<<16)+ 6, 7<<16],
// 2^40 branches like the one above.
//       [(1<<24)+ 4, 1<<24], [(2<<24)+ 4, 2<<24], [(3<<24)+ 4, 3<<24],
//       [(4<<24)+ 4, 4<<24], [(5<<24)+ 4, 5<<24], [(6<<24)+ 4, 6<<24],
//       [(7<<24)+ 4, 7<<24],
// 2^32 branches like the one above.
//       [(1<<32)+ 2, 1<<32], [(2<<32)+ 2, 2<<32], [(3<<32)+ 2, 3<<32],
//       [(4<<32)+ 2, 4<<32], [(5<<32)+ 2, 5<<32], [(6<<32)+ 2, 6<<32],
//       [(7<<32)+ 2, 7<<32],
// 2^24 branches like the one above. Add 1*2^40, 2*2^40, ..., (2^24-1)*2^40
// to each of the numbers above.
// Sequence of insert:
// 106 + 0*2^40,
// 106 + 1*2^40,
// 106 + 2*2^40,
// ...
// 106 + (2^24-1)*2^40
// 105 + 0*2^40,
// 105 + 1*2^40,
// 105 + 2*2^40,
// ...
// 105 + (2^24-1)*2^40
// ...
//   0 + 0*2^40,
//   0 + 1*2^40,
//   0 + 2*2^40,
// ...
//   0 + (2^24-1)*2^40
// Leaf2s: 2^24
// for (j = 106; j >= 0; --j) {
//     for (i = 0; i < 2**24*2**40; i += 2**40) {
//         print("0x%16x\n", i + j);
//     }
// }
// 1*2^8+14 + 0*2^40,
// 1*2^8+14 + 1*2^40,
// 1*2^8+14 + 2*2^40,
// ...
// 1*2^8+14 + (2^24-1)*2^40
// 1*2^8+13 + 0*2^40,
// 1*2^8+13 + 1*2^40,
// 1*2^8+13 + 2*2^40,
// ...
// 1*2^8+13 + (2^24-1)*2^40
// ...
// 1*2^8+ 0 + 0*2^40,
// 1*2^8+ 0 + 1*2^40,
// 1*2^8+ 0 + 2*2^40,
// ...
// 1*2^8+ 0 + (2^24-1)*2^40
// 2*2^8+14 + 0*2^40,
// 2*2^8+14 + 1*2^40,
// 2*2^8+14 + 2*2^40,
// ...
// 2*2^8+14 + (2^24-1)*2^40
// 2*2^8+13 + 0*2^40,
// 2*2^8+13 + 1*2^40,
// 2*2^8+13 + 2*2^40,
// ...
// 2*2^8+13 + (2^24-1)*2^40
// ...
// 2*2^8+ 0 + 0*2^40,
// 2*2^8+ 0 + 1*2^40,
// 2*2^8+ 0 + 2*2^40,
// ...
// 2*2^8+ 0 + (2^24-1)*2^40
// ...
// 7*2^8+ 0 + 0*2^40,
// 7*2^8+ 0 + 1*2^40,
// 7*2^8+ 0 + 2*2^40,
// ...
// 7*2^8+ 0 + (2^24-1)*2^40
// Immeds in BB3s: 7 * 2^24
// for (k = 2**8; k <= 7*2**8; k += 2**8) {
//     for (j = 14; j >= 0; --j) {
//         for (i = 0; i < 2**24; i += 2**40) {
//             print("0x%16x\n", i + j + k);
//         }
//     }
// }
// Immeds in BB4s: 7 * 2^16
// Immeds in BB5s: 7 * 2^8
// Immeds in BB6s: 7 * 2^24
// Immeds in BB7s: 7 * 2^24
// Immeds in BB8: 7 * 2^24

// --------------------------------
//       [(1<<40)+ 2, 1<<40], [(2<<40)+ 2, 2<<40], [(3<<40)+ 2, 3<<40],
//       [(4<<40)+ 2, 4<<40], [(5<<40)+ 2, 5<<40], [(6<<40)+ 2, 6<<40],
//       [(7<<40)+ 2, 7<<40],
// 2^16 branches like the one above.
//       [(1<<40)+ 2, 1<<40], [(2<<40)+ 2, 2<<40], [(3<<40)+ 2, 3<<40],
//       [(4<<48)+ 1, 1<<48], [(5<<48)+ 1, 1<<48], [(6<<48)+ 1, 1<<48],
//       [(7<<48)+ 1, 1<<48],
// 2^8 branches like the one above.
//       [(1<<56)+ 1, 1<<56], [(2<<56)+ 1, 2<<56], [(3<<56)+ 1, 3<<56],
//       [(4<<56)+ 1, 4<<56], [(5<<56)+ 1, 5<<56], [(6<<56)+ 1, 6<<56],
//       [(7<<56)+ 1, 7<<56],
// 2^0 branches like the one above.


#if defined(_WIN64)
typedef unsigned long long Word_t;
#define W  "ll"
#else // defined(_WIN64)
typedef unsigned long Word_t;
#define W  "l"
#endif // defined(_WIN64)

#if defined(__LP64__) || defined(_WIN64)
#define ZWX  "016"
#else // defined(__LP64__) || defined(_WIN64)
#define ZWX  "08"
#endif // defined(__LP64__) || defined(_WIN64)

#define BITSPW (sizeof(Word_t) * 8)

#define EXP(x)  ((Word_t)1 << (x))

#define LOG(x)  ((Word_t)64 - 1 - __builtin_clzll(x))

#define MASK(x)  ((x) - 1)

#define KEYS_PER_OTHER_INDEX  1

Word_t
Next(int nBitsPerDigit, int nDigitsAtBottom, Word_t wKeysAtBottom)
{
    static Word_t wPrefix = 1;
    static Word_t wKeyNext = 0;
    static int nExp = BITSPW;

    int nBitsAtBitmap = nDigitsAtBitmap * nBitsPerDigit;
    Word_t wKeyNow = wKeyNext;
    Word_t wBottomDigitsMax = MASK(EXP(nBitsPerDigit));

    //
    // Use wKeysPerBitmap for the digit 0 in the node/branch above the bitmap
    // leaves.
    // Use 1 for the digit 1 in the node/branch.
    //
    Word_t wKeysPerNow
#if defined(PACK_KEYS_IN_FIRST_BITMAP)
        = (((wKeyNext >> nBitsAtBitmap) & wDigitMax) == 0)
            ? wKeysPerBitmap : KEYS_PER_OTHER_INDEX;
#else
        = (wKeysAtBottom >> nBitsPerDigit >> nDigitsAtBottom) + 1;
#endif

    if (((++wKeyNext & wDigitMax) % wKeysPerNow) == 0)
    {
        // We've filled the link/jp.  Next link/jp.

        wKeyNext &= ~MASK(EXP(nBitsAtBitmap));
        wKeyNext += EXP(nBitsAtBitmap);

        if ((wKeyNext % EXP(nBitsAtBitmap + 2)) == EXP(nBitsAtBitmap + 1))
        {
            // We've filled two of the links/jps.

            if (nExp == nBitsAtBitmap + nBitsPerDigit)
            {
                wPrefix += 2;

                nExp = BITSPW - LOG(wPrefix) - 1;

                if (nExp < nBitsAtBitmap + nBitsPerDigit)
                {
                    // Out of ideas -- repeat.

                    wKeyNext = 0;
                    wPrefix = 1;
                    nExp = BITSPW;

                    return wKeyNow;
                }

                wKeyNext = wPrefix << nExp;
            }
            else
            {
                wKeyNext = wPrefix << --nExp;
            }
        }
    }

    return wKeyNow;
}

#if 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

void
usage(void)
{
    fprintf(stderr,
      "usage: a.out [ <BitsPerDigit> [ <DigitsLeft> [ <KeysPerX>]]]\n");

    exit(1);
}

//
// oa2ul is an abbreviation for OptArgToUnsignedLong.
// It is a wrapper for strtoul for use when converting an optarg to an
// unsigned long to help save the user from a painful mistake.
// The mistake is the user typing "-S -y" on the command line where
// -S requires an argument and getopt uses -y as that argument and strtoul
// silently treats the -y as a 0 and the -y is never processed as an option
// letter by getopt.  Make sense?
//
unsigned long
oa2ul(char *str, char **endptr, int base)
{
    char *lendptr;
    unsigned long ul;

    if ((str == NULL) || *str == '\0')
    {
        printf("oa2ul: str is NULL or \"\".\n");

        usage();
    }

    errno = 0;

    ul = strtoul(str, &lendptr, base);

    if (errno != 0)
    {
        printf("oa2ul: %s.\n", strerror(errno));

        usage();
    }

    if (*lendptr != '\0')
    {
        printf("oa2ul: str is not a number.\n");

        usage();
    }

    if (endptr != NULL)
    {
        *endptr = lendptr;
    }

    return ul;
}

//
// nBitsPerDigit is minimum bits decoded by each branch/node.
// nDigitsLeft is where smallest bitmap resides.
// wKeysPerX is number of keys to insert at each key base.
//
int
main(int argc, char *argv[])
{
    int nBitsPerDigit = 4;
    int nDigitsAtBottom = 3;
    Word_t wKeysPerX = 200;
    Word_t wKey;

    switch (argc)
    {
    default: usage(); // usage exits
    case 4: wKeysPerX = oa2ul(argv[3], 0, 0);
    case 3: nDigitsAtBottom = oa2ul(argv[2], 0, 0);
    case 2: nBitsPerDigit = oa2ul(argv[1], 0, 0);
    case 1: ;
    }

    fprintf(stderr, "BitsPerDigit %d\n", nBitsPerDigit);
    fprintf(stderr, "DigitsAtBottom %d\n", nDigitsAtBottom);

    fprintf(stderr, "KeysPerX %"W"d\n", wKeysPerX);

    wKey = Next(nBitsPerDigit, nDigitsAtBottom, wKeysPerX);

    for (;;)
    {
#define PRINT
#if defined(PRINT)
        printf("0x%"ZWX W"x\n", wKey);
#endif // defined(PRINT)

        wKey = Next(nBitsPerDigit, nDigitsAtBottom, wKeysPerX);

        if (wKey == 0)
        {
            exit(1);
        }
    }

    return 0;
}

#endif

