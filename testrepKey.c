#include        <stdio.h>
#include        <stdint.h>
#include        <stdlib.h>
#include        <assert.h>

#define LEFT

typedef unsigned long Word_t;

// Macros for parallel searches
// cbPW = bits per Word
// MskK = Mask Key
// KEY  = Key
// REMb = Remainder Bits in Word_t, 
// cbPK = Bits Per Key

#define cbPW        (sizeof(Word_t) * 8)

// WTF, compilers should warn about this or fix it
// MEB: __builtin_clzl is undefined for 0;
// I think so it can be implemented with Intel's bsr.

#define JU_CLZ(BUCKET, _cbPW) \
    ((Word_t)(BUCKET) > 0 ? (int)__builtin_clzl((Word_t)BUCKET) : (int)(_cbPW))

#define JU_CTZ(BUCKET, _cbPW) \
    ((Word_t)(BUCKET) > 0 ? (int)__builtin_ctzll((Word_t)BUCKET) : (int)(_cbPW))

#if defined(LEFT)
#define JU_POP0(BUCKET, _cbPW, _nBL) \
    ((int)(((_cbPW) - 1) - JU_CTZ((BUCKET), (_cbPW))) / (_nBL))
#else // defined(LEFT)
#define JU_POP0(BUCKET, _cbPW, _nBL) \
    ((int)(((_cbPW) - 1) - JU_CLZ((BUCKET), (_cbPW))) / (_nBL))
#endif // defined(LEFT)

// Used to right justify the replicated Keys
#define REMb(cbPK)              (cbPW - ((cbPW / (cbPK)) * (cbPK)))

// Used to mask to single Key
#define MskK(cbPK, KEY)         ((Word_t)((KEY) & ((1 << (cbPK)) - 1)))
//define MskK(cbPK, KEY)        (((Word_t)(KEY)) % (1 << (cbPK))) longer

#if defined(LEFT)
#define REPKEY(cbPK, KEY)                                        \
    (((Word_t)(-1) / MskK(cbPK, -1)) * MskK(cbPK, KEY))
#else // defined(LEFT)
// Replicate a KEY as many times as possible in a Word_t
#define REPKEY(cbPK, KEY)                                        \
    ((((Word_t)(-1) / MskK(cbPK, -1)) * MskK(cbPK, KEY)) >> REMb(cbPK))
#endif // defined(LEFT)

// Check if any Key in the Word_t has a zero value
#define HAS0KEY(cbPK, BUCKET)                                      \
    (((BUCKET) - REPKEY(cbPK, 1)) & ~(BUCKET) & REPKEY(cbPK, -1))

// How many keys offset until the one found (in highest bit in zero)
#define KEYOFFSET(ZERO, CPBW)         (__builtin_ctzl(ZERO) / (CBPW))

// Mask to bits in Population0 
///#define Mskbs(cbPK, POP0)      ((1 << ((POP0) * (cbPK))))

// Mask to remove Keys that are not in the population
// broke?

//#define P_M(cbPK,POP0) (((Word_t)((1 << ((cbPK) - 1)) * 2) << ((POP0) * cbPK)) - 1)

#if 0
static void
printBits(Word_t num, int nbits)
{
    int bit;

//    printf(" [0x%lx ", num & MSK(nbits));

    for (bit = nbits - 1; bit >= 0; bit--)
    {
        printf("%i", (int)((num >> bit) & 0x1));
    }
//    printf("] ");
}
#endif

// slow way to replicate a Key in a Word_t
static Word_t
REPKEY1(int bitsPKey, Word_t Key)
{
    int ii;
    Word_t repkeys = 0;

//  mask Key to bitsPKey bits
    Key = MskK(bitsPKey, Key); // Key & ((1 << bitsPKey) - 1)

    for (ii = 0; ii < (int)(cbPW / bitsPKey); ii++) {
#if defined(LEFT)
        // Keys are packed at the most significant end of the word and the
        // word is padded with zeros at the least significant end if there
        // are any bits left over.
        repkeys |= Key << (cbPW - (ii + 1) * bitsPKey);
#else // defined(LEFT)
        // Keys are packed at the least significant end of the word and the
        // word is padded with zeros at the most significant end if there
        // are any bits left over.
        repkeys |= Key << (bitsPKey * ii);
#endif // if defined(LEFT)
    }

    return(repkeys);
}

// Build and return random valid Bucket and SearchKey, given bits-Per-Key
// Warning: This Does not generate a NULL ( == -1) Key
// MEB: Huh?
static int 
GetValidBucket(Word_t *Bucket, Word_t *SearchKey, int bitsPerKey)
{
    int    KeyMask;                     // Key mask or max Key
    int    KeysPerBucket;               // Max Keys per Bucket
    int    nKeys;                       // number of Key
    int    PreviousKey;
    int    Key;                         // 16 bits max
    int    offset;                      // offset of Key in Bucket
    int    retoffset;                   // 

    KeyMask = (Word_t)((1 << bitsPerKey) - 1);  // max Key value or Mask
    KeysPerBucket = cbPW / bitsPerKey;          // max Keys per Bucket

//  Get a valid Key to search - anything 0..KeyMask
    *SearchKey = (Word_t)(random() & KeyMask);

//  get a random population  keys 1..maxKey 
    nKeys = (random() % KeysPerBucket) + 1;
    retoffset = -1000; 

    int pop1 = 0;

//  fill out Bucket with assending Keys values
//  Set previous key to impossible low
    PreviousKey = -1;
    for (*Bucket = offset = 0; offset < nKeys; offset++)
    {
//      Get a Key larger than previous Key  (0..KeyMask)
        while ((Key = (int)(random() & KeyMask)) <= PreviousKey);
        PreviousKey = Key;

//      Put in the bucket
#if defined(LEFT)
        *Bucket |= (Word_t)Key << (cbPW - (bitsPerKey * (offset + 1)));
#else // defined(LEFT)
        *Bucket |= (Word_t)Key << (bitsPerKey * offset);
#endif // defined(LEFT)
        pop1++;

        if (Key == (int)*SearchKey)
            retoffset = offset;

        if ((retoffset == -1000) && (Key > (int)*SearchKey))
            retoffset = ~offset;

        if (Key == KeyMask)
            break;
    }

    int         clzpop0;
    int         loop = 0;

    Word_t buck = *Bucket;

    do 
    {
#if defined(LEFT)
        buck <<= bitsPerKey;
#else // defined(LEFT)
        buck >>= bitsPerKey;
#endif // defined(LEFT)
        loop++;
    } while (buck);

    clzpop0 = JU_POP0(*Bucket, cbPW, bitsPerKey);

    if ((loop != (clzpop0 + 1)) || (loop != pop1))
        printf(" Oops!! Bucket = 0x%lx, Key = 0x%x,Pop1=%d, loop=%d, clz=%d\n", 
                *Bucket,
                Key,
                pop1,
                loop,
                clzpop0 + 1);


    if (retoffset == -1000) 
        retoffset = ~(offset);

    return (retoffset);                    // ~offset

}

// If matching Key in Bucket, then offset into Bucket is returned
// else -1 if no Key was found

#if defined(LEFT)
#define EXP(_x)  ((Word_t)1 << (_x))
#define MSK(_x)  (EXP(_x) - 1)

static int
BucketHasKey(Word_t ww, Word_t wKey, int nBL)
{
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    wKey &= wMask;
    if (ww == 0) { return -(wKey != 0); }
    int nPopCnt = JU_POP0(ww, cbPW, nBL) + 1;
    int nBitsOfKeys = nPopCnt * nBL;
    // lsb in each slot that has a key
    Word_t wLsbs = ((Word_t)-1 / wMask)
                    & ((Word_t)-1 << (cbPW - nBitsOfKeys));
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    wXor |= MSK(cbPW - nBitsOfKeys);
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs;
    int bXorHasZero = (wMagic != 0);
    // where is the match?
    int offset = JU_POP0(wMagic, cbPW, nBL);
    if ( ! bXorHasZero ) { return -1; }
    return offset;
}

#else // defined(LEFT)

static int
BucketHasKey(Word_t Bucket, Word_t Key, int bPK)
{
    Word_t      repLsbKey;
    Word_t      repMsbKey;
    Word_t      newBucket;
    Word_t      KeyMask;
    int         offset;

    KeyMask = (Word_t)((1 << bPK) - 1);  // max Key value or Mask

    Key &= KeyMask;                     // get rid of high bits
//  Check special case of Key == 0
    if (Key == 0)
    {
        if (Bucket & KeyMask)
            return (-1);        // no match and offset = -1
        else
            return (0);         // match and offset = 0
    }
//  replicate the Lsb in every Key position
    repLsbKey = REPKEY(bPK, 1);

//  replicate the Msb in every Key position
    repMsbKey = repLsbKey << (bPK - 1);

//  make zero the searched for Key in new Bucket
    newBucket = Bucket ^ REPKEY(bPK, Key);

//  Magic, the Lsb is located in the matching Key position
    newBucket = (newBucket - repLsbKey) & (~newBucket) & repMsbKey;

    if (newBucket == 0)                    // Key not found in Bucket
        return(-1);

//  calc offset for Pjv_t -- this should be done outside of this subroutine because it
//  is not needed the Judy1Test code
    offset = __builtin_ctzl(newBucket) / bPK;

//    printf(" bPK = %d, Key =0x%lx, Bucket = 0x%lx, newBucket = 0x%lx, offset = %d\n",
//        bPK,
//        Key & (Word_t)((1 << bPK) - 1),
//        Bucket,
//        newBucket,
//        offset);

//  return the offset into Bucket that Key was found
    return (offset);
}
#endif // defined(LEFT)

// Given Bucket, Key and bitsPerKey, return offset 0..KPB-1 of found Key
// if not found, return ~offset of where to insert new Key
static int
BucketHasKeyIns(Word_t Bucket, Word_t Key, int bitsPerKey)
{
    Word_t KeyMask;                     // Key mask or max Key
    int    offset;                      // offset of Key in Bucket
    int    KeysPerBucket;               // Max Keys per Bucket

//  Since a 0 Bucket is legal, use an impossible Bucket as NULL (no Keys).
    if (Bucket == (Word_t)-1)           // if NULL, return bogas offset
    {
        printf("\n Oops!!, no NULLs allowed yet!!!\n");
        exit(-1);
    }
    KeyMask = (Word_t)((1 << bitsPerKey) - 1);  // max Key value and Mask
    KeysPerBucket = cbPW / bitsPerKey;          // max Keys per Bucket

//  Note: Keys must be sorted and no synonym Keys in Bucket
    Key &= KeyMask;

#if 0 // ! defined(LEFT)
//  Check special case of Key & Bucket[0] & offset == 0
//  This is because a zero Key terminates the list
    if (((Key | Bucket) & KeyMask) == 0) 
        return(0);                      // match and offset = 0
#endif // ! defined(LEFT)

//  first Key in Bucket or Key not equal 0
    for (offset = 0; offset < KeysPerBucket; offset++)
    {
        Word_t NextKey;

//      Check next Key in Bucket
#if defined(LEFT)
        NextKey = (Bucket >> ((cbPW - ((offset + 1) * bitsPerKey))) & KeyMask);
#else // defined(LEFT)
        NextKey = (Bucket >> (offset * bitsPerKey)) & KeyMask;
#endif // defined(LEFT)

        if (NextKey == Key)             // Found
            return(offset);

//      Terminating Key value in Bucket, except 1st Key
        if ((NextKey == 0) && offset)   
            return(~offset);

        if (NextKey > Key)              // offset to put Key
            return(~offset);
    }
    return(~offset);       // ~offset or ~where to insert new Key
}

// Test some algorithms program
int
main()
{
    int         bitsPKey;            // bits per key
    int ii;

    printf("\n\nBits per Bucket(Word_t) = %d\n", (int)cbPW);

    // verify that REPKEY == REPKEY1 for all keys at all key sizes
    for (bitsPKey = 2; bitsPKey <= 16; bitsPKey++)
    {
        Word_t Key;

        printf("BitsPerKey = %2d, KeyMask = 0x%04lx, KeysPerBucket = %2d, unused Bits = %2d, -", 
                bitsPKey, 
                MskK(bitsPKey, -1),
                (int)(cbPW/bitsPKey), 
                (int)REMb(bitsPKey)); 

//      for all Key values
        for (Key = 0; Key < ((Word_t)1 << bitsPKey); Key++)
        {
            Word_t      repkeys;
            Word_t      repkeys1;
            repkeys1 = REPKEY1(bitsPKey, Key);
            repkeys  = REPKEY(bitsPKey, Key);
            if (repkeys1 != repkeys)
            {
                printf("\n Oops!! - 0x%lx 0x%lx != 0x%lx\n", Key, repkeys1, repkeys);
                exit(-1);
            }
        }
        printf(" REPKEY() == REPKEY1() for all Key values\n");
    }
    printf(" Passed the REPKEY() macro == REPKEY1() subroutine\n\n");

    printf(" Begin testing Parallel Search of random valid Buckets (loops = 1000000000 * 15 (#bitsPerKey))\n");

    for (ii = 0; ii < 1000000000; ii++)
    {
        if ((ii % 1000000) == 0) printf("%d\n", ii);

        for (int nBitsPKey = 4; nBitsPKey <= 16; nBitsPKey++)
        {
            Word_t      Bucket;
            Word_t      Key;
            int         offset;
            int         offset1;
            int         offset2;

            offset =  GetValidBucket(&Bucket, &Key, nBitsPKey);

            offset1 = BucketHasKeyIns(Bucket, Key, nBitsPKey);
            offset2 = BucketHasKey(Bucket, Key, nBitsPKey);

            if (offset != offset1)
            {
                printf(" Oops!! -  offset = %d, offset1 = %d, Bucket = 0x%016lx Key = 0x%lx, nBits = %d, ii = %d\n", 
                        offset,
                        offset1, 
                        Bucket,
                        Key,
                        nBitsPKey,
                        ii);
                    //exit(-1);
                    continue;
            }
//          BucketHasKey() only returns -1 for No match
            if (offset < 0)
            {
                if (offset2 != -1)
                {
                    printf(" Oops!! -  offset = %d, offset2 = %d, Bucket = 0x%016lx Key = 0x%lx, nBits = %d, ii = %d\n", 
                        offset,
                        offset2, 
                        Bucket,
                        Key,
                        nBitsPKey,
                        ii);
                    //exit(-1);
                    continue;
                }
            }
            else        // if (offset >= 0) 
            {
                if (offset != offset2)
                {
                    printf(" Oops!! -  offset = %d, offset2 = %d, Bucket = 0x%016lx Key = 0x%lx, nBits = %d, ii = %d\n", 
                        offset,
                        offset2, 
                        Bucket,
                        Key,
                        nBitsPKey,
                        ii);
                    //exit(-1);
                    continue;
                }
            }
#if 0
            if (offset2 != -1) {
                printf(" offset = %d, offset2 = %d, Bucket = 0x%016lx Key = 0x%lx, nBits = %d, ii = %d\n", 
                        offset, offset2, Bucket, Key, nBitsPKey, ii);
            }
#endif
        }
    }
    printf(" Good finish if after 32 * 15 billion tests\n");
    return(0);
}
