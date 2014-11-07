#include        <stdio.h>
#include        <stdint.h>
#include        <stdlib.h>
#include        <assert.h>

#define LEFT
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

#if 0
// Do a parallel search of a list embedded in a word given the key size,
// and assuming the list has at least one key.
// EmbeddedListHasKey expects the keys to be packed towards the most
// significant bits.
// The cnBitsMallocMask least-significant bits of the word are used for a
// type field and the next least-significant nBL_to_nBitsPopCntSz(nBL) bits
// of the word are used for a population count.
// It helps Lookup performance to eliminate the need to know nPopCnt.
// So, if PAD_T_ONE, we replicate the last key in the list into the unused
// slots at insert time to make sure the unused slots don't cause a false
// bXorHasZero.
// But how do we make sure the type and pop count bits don't
// cause a false bXorHasZero due to a slot that can't really be used?
// Or'ing MSK(nBL_to_nBitsPopCntSz(nBL) + cnBitsMallocMask)
// would be sufficient, but it may be expensive.
// Can we do something simpler/faster?  Something at insert time?
// Unfortunately, it doesn't matter how the pop and type bits are set
// in the word since we are xoring them with the key we're looking for
// before calculating bXorHasZero.  And whatever they are set to will
// match the key/keys that is/are the same.
// I wonder if the next best thing is to have a constant that we can
// or into wXor before calculating bXorHasZero.
// Does cnMallocMask work?  It will cover any key slot that extends
// into cnMallocMask.  But what about a slot that extends into the pop
// count field and not into cnMallocMask?
// Sure would be nice if we had a constant width pop field.  What would
// be the cost?  3-bits of pop for 64-bit costs one 29-bit key slot.
// 2-bits of pop for 32-bit costs one 14-bit key slot.
// If we're not using those key sizes, then there is no cost.
int EmbeddedListHasKey(Word_t wRoot, Word_t wKey, unsigned nBL);
int
EmbeddedListHasKey(Word_t wRoot, Word_t wKey, unsigned nBL)
{
#if (cbPW == 64)
#define cnLogBytesPerWord  3
#else // (cbPW == 64)
#define cnLogBytesPerWord  2
#endif // (cbPW == 64)

#define cnBitsMallocMask  (cnLogBytesPerWord + 1)
#define cnMallocMask  MSK(cnBitsMallocMask)

// Fixed-size pop count field to make code simpler.
// We only give up one 29-bit slot in 64-bit and one 14-bit slot in 32-bit.
#if ! defined(NO_EMBEDDED_LIST_FIXED_POP)
#undef  EMBEDDED_LIST_FIXED_POP
#define EMBEDDED_LIST_FIXED_POP
#endif // ! defined(NO_EMBEDDED_LIST_FIXED_POP)
#if (cbPW == 64)
#if defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  3
#else // defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(88 / (_nBL))
#endif // defined(EMBEDDED_LIST_FIXED_POP)
#elif (cbPW == 32)
#if defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  2
#else // defined(EMBEDDED_LIST_FIXED_POP)
#define nBL_to_nBitsPopCntSz(_nBL)  LOG(44 / (_nBL))
#endif // defined(EMBEDDED_LIST_FIXED_POP)
#else
#error "Unsupported cbPW."
#endif

#define     wr_nPopCnt(_wr, _nBL) \
    ((((_wr) >> cnBitsMallocMask) & MSK(nBL_to_nBitsPopCntSz(_nBL))) + 1)

#if ! defined(PAD_WITH_KEY)
    unsigned nPopCnt = wr_nPopCnt(wRoot, nBL); // number of keys present
    unsigned nBitsOfKeys = nPopCnt * nBL;
#endif // ! defined(PAD_WITH_KEY)
    Word_t wMask = MSK(nBL); // (1 << nBL) - 1
    Word_t wLsbs = (Word_t)-1 / wMask;
#if ! defined(PAD_WITH_KEY) && ! defined(ONE_WAY)
    wLsbs &= (Word_t)-1 << (cbPW - nBitsOfKeys); // type and empties
#endif // ! defined(PAD_WITH_KEY) && ! defined(ONE_WAY)
    Word_t wKeys = (wKey & wMask) * wLsbs; // replicate key; put in every slot
    Word_t wXor = wKeys ^ wRoot; // get zero in slot with matching key
#if ! defined(PAD_WITH_KEY) && defined(ONE_WAY)
    wXor |= MSK(cnBitsPerWord - nBitsOfKeys); // type and empty slots
#endif // ! defined(PAD_WITH_KEY) && defined(ONE_WAY)
#if defined(PAD_WITH_KEY)
    wXor |= MSK(cnBitsMallocMask + nBL_to_nBitsPopCntSz(nBL)); // pop and type
#endif // defined(PAD_WITH_KEY)
    Word_t wMsbs = wLsbs << (nBL - 1); // msb in each key slot
    int bXorHasZero = (((wXor - wLsbs) & ~wXor & wMsbs) != 0); // magic
    return bXorHasZero;
}
#endif

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
// don't sort
//
// empty slots are on most-significant end
// empty slots are on least-significant end
// empty slots are anywhere

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
    int offset = JU_POP0(wMagic, cbPW, nBL);
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
static int
BucketHasKey(Word_t ww, Word_t wKey, int nBL)
{
    Word_t wMask = MSK(nBL);
    wKey &= wMask;
    if (ww == 0) { return -(wKey != 0); }
    int nPopCnt = JU_POP0(ww, cbPW, nBL) + 1;
    int nBitsOfKeys = nPopCnt * nBL;
    // lsb in each slot that has a key
    Word_t wLsbs = (Word_t)-1 / wMask;
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

#endif // defined(USE_WORD_HAS_KEY)

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
