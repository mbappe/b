
#include <stdint.h> // for gcc/linux
#include <assert.h>

#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t Word_t;
#else // defined(__LP64__) || defined(_WIN64)
typedef uint32_t Word_t;
#endif // defined(__LP64__) || defined(_WIN64)

#define BITSPW (sizeof(Word_t) * 8)

#define EXP(x)  ((Word_t)1 << (x))

#define LOG(x)  ((Word_t)64 - 1 - __builtin_clzll(x))

#define MASK(x)  ((x) - 1)

Word_t
Next(int nBitsPerDigit, int nDigitsAtBitmap, Word_t wKeysPerBitmap)
{
    static Word_t wPrefix = 1;
    static Word_t wKeyNext = 0;
    static int nExp = BITSPW;

    int nBitsAtBitmap = nDigitsAtBitmap * nBitsPerDigit;
    int wKeyNow = wKeyNext;

    Word_t wKeysPerNow
        = ((wKeyNext % EXP(nBitsAtBitmap + 1)) == EXP(nBitsAtBitmap))
            ? 1 : wKeysPerBitmap;

    if (((++wKeyNext & MASK(EXP(nBitsAtBitmap))) % wKeysPerNow) == 0)
    {
        wKeyNext &= ~MASK(EXP(nBitsAtBitmap));
        wKeyNext += EXP(nBitsAtBitmap);

        if ((wKeyNext % EXP(nBitsAtBitmap + 2)) == EXP(nBitsAtBitmap + 1))
        {
            if (nExp == nBitsAtBitmap + nBitsPerDigit)
            {
                wPrefix += 2;

                nExp = BITSPW - LOG(wPrefix) - 1;

                if (nExp < nBitsAtBitmap + nBitsPerDigit)
                {
                    //printf("wPrefix %x nExp %d\n", wPrefix, nExp);

                    assert(0);
                    // repeat
                    wKeyNext = 0;
                    wPrefix = 1;
                    nExp = BITSPW;
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
      "usage: a.out [[[[<BitsPerDigit>"
      " [[[<DigitsLeft> [[<KeysPerX> [<KeyStep>]]]]\n");

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
    long nBitsPerDigit = 4;
    long nDigitsAtBottom = 3;
    Word_t wKeysPerX = 200;

    switch (argc)
    {
    default: usage(); // usage exits
    case 4: wKeysPerX = oa2ul(argv[3], 0, 0);
    case 3: nDigitsAtBottom = oa2ul(argv[2], 0, 0);
    case 2: nBitsPerDigit = oa2ul(argv[1], 0, 0);
    case 1: ;
    }

    fprintf(stderr, "BitsPerDigit %ld\n", nBitsPerDigit);
    fprintf(stderr, "DigitsAtBottom %ld\n", nDigitsAtBottom);

#if defined(__LP64__) || defined(_WIN64)
    fprintf(stderr, "KeysPerX %lld\n", (long long)wKeysPerX);
#else // defined(__LP64__) || defined(_WIN64)
    fprintf(stderr, "KeysPerX %d\n", wKeysPerX);
#endif // defined(__LP64__) || defined(_WIN64)

    for (;;)
    {
#if defined(__LP64__) || defined(_WIN64)
        printf("0x%016llx\n",
            (long long)Next(nBitsPerDigit, nDigitsAtBottom, wKeysPerX));
#else // defined(__LP64__) || defined(_WIN64)
        printf("0x%08x\n", Next(nBitsPerDigit, nDigitsAtBottom, wKeysPerX));
#endif // defined(__LP64__) || defined(_WIN64)
    }

    return 0;
}

#endif

