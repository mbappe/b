
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // for gcc/linux
#include <string.h>
#include <errno.h>

#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t Word_t;
#else // defined(__LP64__) || defined(_WIN64)
typedef uint32_t Word_t;
#endif // defined(__LP64__) || defined(_WIN64)

#define BITSPW (sizeof(Word_t) * 8)

#define EXP(x)  ((Word_t)1 << (x))

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

#if 0

void
Outliers(int nBitsPerDigit, int nDigitsAtBitmap,
    Word_t wKeysPerX, Word_t wKeyStep, Word_t wKey, int nDigitsLeft)
{
    Word_t w;

#if 0
    printf("Outliers: KeysPerX %d KeyStep %x Key %llx nDigitsLeft %d\n",
        (int)wKeysPerX, (int)wKeyStep, (long long)wKey, nDigitsLeft);
#endif

    wKey <<= nBitsPerDigit;

    if (nDigitsLeft-- <= nDigitsAtBitmap)
    {
        wKey <<= (nDigitsAtBitmap - 1) * nBitsPerDigit;

        if (wKey % EXP((nDigitsAtBitmap + 1) * nBitsPerDigit))
        {
            wKeysPerX = 1;
        }

        for (w = 0; w < wKeysPerX * wKeyStep; w += wKeyStep)
        {
#if defined(__LP64__) || defined(_WIN64)
            printf("0x%016llx\n", (long long)wKey + w);
#else // defined(__LP64__) || defined(_WIN64)
            printf("0x%08x\n", wKey + w);
#endif // defined(__LP64__) || defined(_WIN64)
        }

        return;
    }

    for (w = 0;
         w < ((nDigitsLeft > nDigitsAtBitmap + 1) ? EXP(nBitsPerDigit) : 2);
         w++)
    {
        Outliers(nBitsPerDigit, nDigitsAtBitmap, wKeysPerX, wKeyStep,
            wKey + w, nDigitsLeft);
    }
}

#else 

void
Outliers(int nBitsPerDigit, int nDigitsAtBitmap,
    Word_t wKeysPerX, Word_t wKeyStep, Word_t wKey, int nDigitsLeft)
{
    static Word_t awDigits[BITSPW];
    int i;

again:

    for (i = 0; i < (BITSPW + nBitsPerDigit - 1) / nBitsPerDigit; i++)
    {
        printf("%x", awDigits[i]);
    }
    printf("\n");

    for (nDigitsLeft
             = (BITSPW + nBitsPerDigit - 1) / nBitsPerDigit
                 - nDigitsAtBitmap - 1;
         nDigitsLeft >= 0;
         nDigitsLeft--)
    {
        if (++awDigits[nDigitsLeft] < EXP(nBitsPerDigit))
        {
            goto again;
        }

        awDigits[nDigitsLeft] = 0;
    }
}

#endif

//
// nBitsPerDigit is minimum bits decoded by each branch/node.
// nDigitsLeft is where smallest bitmap resides.
// wKeysPerX is number of keys to insert at each key base.
// wKeyStep = 0 means divide nDigitsLeft evenly.
//
int
main(int argc, char *argv[])
{
    long nBitsPerDigit = BITSPW / 2;
    long nDigitsAtBottom = 1;
    Word_t wKeysPerX = 1;
    Word_t wKeyStep = 0;

    switch (argc)
    {
    default: usage(); // usage exits
    case 5: wKeyStep = oa2ul(argv[4], 0, 0);
    case 4: wKeysPerX = oa2ul(argv[3], 0, 0);
    case 3: nDigitsAtBottom = oa2ul(argv[2], 0, 0);
    case 2: nBitsPerDigit = oa2ul(argv[1], 0, 0);
    case 1: ;
    }

    if (wKeyStep == 0)
    {
        wKeyStep = EXP(nBitsPerDigit * nDigitsAtBottom) / wKeysPerX;
    }

    fprintf(stderr, "BitsPerDigit %ld\n", nBitsPerDigit);
    fprintf(stderr, "DigitsAtBottom %ld\n", nDigitsAtBottom);

#if defined(__LP64__) || defined(_WIN64)
    fprintf(stderr, "KeysPerX %lld\n", (long long)wKeysPerX);
    fprintf(stderr, "wKeyStep 0x%llx\n", (long long)wKeyStep);
#else // defined(__LP64__) || defined(_WIN64)
    fprintf(stderr, "KeysPerX %d\n", wKeysPerX);
    fprintf(stderr, "KeyStep 0x%x\n", wKeyStep);
#endif // defined(__LP64__) || defined(_WIN64)

    Outliers(nBitsPerDigit, nDigitsAtBottom, wKeysPerX, wKeyStep, 0,
        /* DIGITSPW */ (BITSPW + nBitsPerDigit - 1) / nBitsPerDigit);

    return 0;
}

