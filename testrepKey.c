#include        <stdio.h>
#include        <stdint.h>
#include        <stdlib.h>
#include        <assert.h>

#define EXP(_x)  ((Word_t)1 << (_x))
#define MSK(_x)  (EXP(_x) - 1)

//#define LEFT
//#define USE_WORD_HAS_KEY

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

// Count leading zeros.
// __builtin_clz is undefined for zero.
#define JU_CLZ(BUCKET) \
    ((Word_t)(BUCKET) > 0 ? (int)__builtin_clzl((Word_t)BUCKET) : (int)cbPW)

// Count trailing zeros.
// __builtin_ctz is undefined for zero.
#define JU_CTZ(BUCKET) \
    ((Word_t)(BUCKET) > 0 ? (int)__builtin_ctzll((Word_t)BUCKET) : (int)cbPW)

#if defined(LEFT)
#if 0
// This POP0 func will return max pop for a bucket of all zeros.
#define JU_POP0(_wBucket, _nBL) \
    ((int)(cbPW - __builtin_ffsll(_wBucket)) / (_nBL))
#endif
// This POP0 func will return 0 for a bucket of all zeros.
#define JU_POP0(_wBucket, _nBL) \
    ((int)((cbPW - __builtin_ffsll(_wBucket | ((Word_t)1 << (cbPW - 1)))) \
        / (_nBL)))

#else // defined(LEFT)
#define JU_POP0(BUCKET, _nBL) \
    ((int)((cbPW - 1) - JU_CLZ(BUCKET)) / (_nBL))
#endif // defined(LEFT)


// Used to right justify the replicated Keys
#define REMb(cbPK)              (cbPW - ((cbPW / (cbPK)) * (cbPK)))
//#define REMb(cbPK)              (cbPW % (cbPK))

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

#define EMPTY_BUCKET  ((Word_t)-1)

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
    *SearchKey = (Word_t)random() & KeyMask;

//  get a random population from 0 through number of slots
    nKeys = random() % (KeysPerBucket + 1);

    if (nKeys == 0) { *Bucket = EMPTY_BUCKET; return ~0; }

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

        if (Key == KeyMask) { break; }
    }

#if 0
    for (int fill = pop1; fill < KeysPerBucket; fill++) {
#if defined(LEFT)
        *Bucket |= (Word_t)Key << (cbPW - (bitsPerKey * (fill + 1)));
#else // defined(LEFT)
        *Bucket |= (Word_t)Key << (bitsPerKey * fill);
#endif // defined(LEFT)
    }
if ((bitsPerKey % 4) == 0) {
    printf("nBL %d pop1 %d Key %x *Bucket 0x%016lx\n", bitsPerKey, pop1, Key, *Bucket);
}
#endif

    if (*SearchKey > (Word_t)Key) { retoffset = ~offset; }

    assert(JU_POP0(*Bucket, bitsPerKey) == pop1 - 1);

    return retoffset;
}

int BucketHasKey(Word_t wBucket, Word_t wKey, int nBL);

// If matching Key in Bucket, then offset into Bucket is returned
// else -1 if no Key was found

#if defined(LEFT)

// There are a lot of ways we can represent a bucket.
// Which way will be fastest?
// Should we require that the keys in the list be sorted?  Why?
// Is has-key faster if we fill empty slots with a present key?
// What about offset?  Assumes key is present.
// What about has-key-and-offset?  No offset returned if key is not present.
// What about offset-of-missing-key?  Assumes key is not present.
// What about has-key-or-offset-of-missing-key?  No off ret if key is present.
// What about has-key-and-offset-or-offset-of-missing-key?
//
// If keys are sorted and key[n+1] < key[n] && k[n+1] != 0, then bucket is
// illegal and we could use that to mean the bucket is empty.  It only
// requires that buckets can always hold more than one key,
// i.e. key_size <= word_size / 2.
// The second half of the test isn't necessary if we fill empty slots
// with the biggest present key.
//
// fill empty slots with 0
// fill empty slots with smallest key
// fill empty slots with largest key
// fill empty slots with -1
//
// sort with most-significant non-empty slot having the smallest key
// sort with least-significant slot having the smallest key
// don't sort; couldn't possibly help search
//
// empty slots are on most-significant end
// empty slots are on least-significant end
// empty slots are anywhere; can't imagine this helping search
//
// remainder bits are at the most-significant end
// remainder bits are at the least-significant end
//
// The value(s) we can use to represent an empty bucket depend
// on the choices made for the others.
//
// Key observations about HasKey:
// HasKey creates a magic number with the high bit set in the key slots
// that match the target key.  It also sets the high bit in the key slot
// to the left of any other slot with its high bit set if the key in that
// slot is one less than the target key.

// (EMPTY_SLOT == 0) && (EMPTY_BUCKET == (Word_t)-1)
// (EMPTY_SLOT == -1 & MSK(nBL)) && (EMPTY_BUCKET == 0)
#if ! defined(EMPTY_SLOT)
#define EMPTY_SLOT  (0)
#define EMPTY_BUCKET  ((Word_t)-1)
#else // ! defined(EMPTY_SLOT)
#define EMPTY_BUCKET  ((EMPTY_SLOT) - (Word_t)1)
#endif // ! defined(EMPTY_SLOT)

int WordHasKey(Word_t ww, Word_t wKey, int nBL);
int
WordHasKey(Word_t ww, Word_t wKey, int nBL)
{
#if defined(OPTION_A)
    if (ww == EMPTY_BUCKET) { return 0; }
#endif // defined(OPTION_A)
    Word_t wMask = MSK(nBL);
    wKey &= wMask; // get rid of already-decoded bits
#if defined(OPTION_B)
    if ((wKey == EMPTY_SLOT) && (ww == EMPTY_BUCKET)) { return 0; }
#endif // defined(OPTION_B)
    Word_t wLsbs = (Word_t)-1 / wMask; // lsb in each key slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wKeys = wKey * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    //wXor |= MSK(cbPW - nBitsOfKeys); // type and empty slots
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs; 
    int bXorHasZero = (wMagic != 0);
#if defined(OPTION_A) || defined(OPTION_B)
    int bHasKey = bXorHasZero;
#else // defined(OPTION_A) || defined(OPTION_B)
    int bHasKey = bXorHasZero && (ww != EMPTY_BUCKET);
#endif // defined(OPTION_A) || defined(OPTION_B)
#if defined(HAS_KEY_ONLY)
    // any sort; empty slots anywhere
    return bHasKey;
#else // defined(HAS_KEY_ONLY)
    if ( ! bHasKey ) { return -1; }
    // sort with most-significant non-empty slot having the smallest key
    // empty slots are on most-significant end
    if (wKey == 0) { return 0; }
    //int offset = (cbPW - 1 - __builtin_ctzll(wMagic)) / nBL;
    int offset = JU_POP0(wMagic, nBL);
    return offset;
#endif // defined(HAS_KEY_ONLY)
}

#if defined(USE_WORD_HAS_KEY)

#define BucketHasKey  WordHasKey

#else // defined(USE_WORD_HAS_KEY)

// Judy1Test only needs to know if the key is present or not.
// Judy1Set needs to know where the key belongs if it is not present.
// JudyLGet needs to know the offset of the key if the key is present.
// JudyLInsert needs to know the offset of the key if the key is present.
// 
// It needs to determine the offset of a found key for JudyL.
// only for Lookup.
int
BucketHasKey(Word_t ww, Word_t wKey, int nBL)
{
//nBL=7;
    if (ww == EMPTY_BUCKET) { return -1; } // could be common
    Word_t wMask = MSK(nBL);
    wKey &= wMask;
    Word_t wLsbs = (Word_t)-1 / wMask;
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key

#define TEST_ZERO
#if ! defined(TEST_ZERO)
    int nPopCnt = JU_POP0(ww, nBL) + 1;
    int nBitsOfKeys = nPopCnt * nBL;
    // lsb in each slot that has a key
    wXor |= MSK(cbPW - nBitsOfKeys);
#endif // ! defined(TEST_ZERO)

    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs; // xor-has-zero
    if (wMagic == 0) { return -1; }
#if defined(TEST_ZERO)
    if (wKey == 0) { return (ww >> (cbPW - nBL)) ? -1 : 0; }
#endif // defined(TEST_ZERO)
    // where is the match?
    int offset = (int)(cbPW - __builtin_ffsll(wMagic)) / nBL; // POP0
    int offset2 = __builtin_clzll(wMagic) / nBL;
if (offset2 != offset) {
    assert(offset2 == offset - 1);
    Word_t wPrev = (ww >> (cbPW - (offset2 + 1) * nBL)) & wMask;
    assert(wPrev == wKey - 1);
    //printf("wKey %lx wPrev %lx\n", wKey, wPrev);
    //printf("offset %d offset2 %d wMagic %016lx nBL %d ww %016lx wKey %lx\n", offset, offset2, wMagic, nBL, ww, wKey);
}
    return offset;
}

#if 0
// fill with some key
int
BucketHasKey(Word_t ww, Word_t wKey, int nBL)
{
//nBL=7;
    if (ww == EMPTY_BUCKET) { return -1; } // could be common
    Word_t wMask = MSK(nBL);
    wKey &= wMask;
    Word_t wLsbs = (Word_t)-1 / wMask;
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    Word_t wXor = wKeys ^ ww; // get zero in slot with matching key
    Word_t wMagic = (wXor - wLsbs) & ~wXor & wMsbs; // xor-has-zero
    if (wMagic == 0) { return -1; }
    // The key is present.  Where is it?
    // where is the match?
    // Does wMagic always have the high bit set in every matching slot?
    // And no other bits set?  If so can we just find the most or least
    // significant match?
    int offset = (int)(cbPW - __builtin_ffsll(wMagic)) / nBL; // POP0
    return offset;

abccc

}
#endif

#endif // defined(USE_WORD_HAS_KEY)

#else // defined(LEFT)

int
BucketHasKey(Word_t Bucket, Word_t Key, int bPK)
{
//bPK=7;
    Word_t      repLsbKey;
    Word_t      repMsbKey;
    Word_t      newBucket;
    int         offset;

    if (Bucket == EMPTY_BUCKET) { return ~0; }
    Word_t KeyMask = (Word_t)((1 << bPK) - 1);  // max Key value or Mask
    Key &= KeyMask;                     // get rid of high bits
//  Check special case of Key == 0
    if (Key == 0) { return (Bucket & KeyMask) ? -1 : 0; }
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
    int offset2 = (cbPW - 1 - __builtin_clzl(newBucket)) / bPK;
if (offset2 != offset) {
    //printf("offset %d offset2 %d\n", offset, offset2);
    if (offset2 != offset + 1) {
assert(Key == 1);
// could it be that the problematic case is when the slot to the left of the
// match is one less than the key and each of the slots left of that are the same?
    //printf("Key %lx wPrev %lx\n", Key, wPrev);
    //printf("offset %d offset2 %d wMagic %016lx nBL %d ww %016lx wKey %lx\n", offset, offset2, newBucket, bPK, Bucket, Key);
    }
}

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

    if (Bucket == EMPTY_BUCKET) { return -1; }
 
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
