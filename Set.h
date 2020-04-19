// API for set of Word_t-sized unsigned integer keys.

typedef size_t Word_t; // same size as uintptr_t.

// Insert:
// Insert wKey into the set represented by *pwRoot.
// *pwRoot must be initialized to zero before the first Insert.
// *pwRoot will be maintained by the package until Free is called.
// Return 1 if the key was not already in the set.
// Return 0 if the key was already in the set.
extern int Insert(Word_t* pwRoot, Word_t wKey);

// Lookup:
// Return 1 if wKey is in the set.
// Return 0 if wKey is not in the set.
extern int Lookup(Word_t wRoot, Word_t wKey);

// Remove:
// Remove *pwKey from the set represented by *pwRoot.
// Return 1 if the key was already in the set.
// Return 0 if the key was not in the set.
extern int Remove(Word_t* pwRoot, Word_t wKey);

// Return the number of keys that are in the set from wKeyA through wKeyB.
// Include wKeyA and wKeyB in the count if they are present.
// Return zero if (wKeyA == 0) and (wKeyB == (Word_t)-1) and all keys are
// present.
// Use (*pwRoot == 0) to disambiguate no keys preset from all keys present.
extern Word_t Count(Word_t wRoot, Word_t wKeyA, Word_t wKeyB);

// ByCount:
// Return the wCount'th key in the set. wCount == 0 means return the 1st key.
// Return 1 if a key is found.
// Return 0 if wCount is bigger than the number of keys in the set.
// Return -1 if pwKey is NULL.
extern int ByCount(Word_t wRoot, Word_t wCount, Word_t* pwRoot);

// Next:
// If *pwKey is in the set return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger key than *pwKey which is in the set.
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is bigger than the biggest key in the set.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
extern int Next(Word_t wRoot, Word_t* pwKey);

// Prev:
// If *pwKey is in the set then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller key than *pwKey which is in the set.
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is smaller the smallest key in the set.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
extern int Prev(Word_t wRoot, Word_t* pwKey);

// NextHole:
// If *pwKey is not in the set then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger number than *pwKey which is not in the set.
// Put the number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys bigger or equal to *pwKey are in the set.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
extern int NextHole(Word_t wRoot, Word_t* pwKey);

// PrevHole:
// If *pwKey is not in the set then return 1 and leave *pwKey unchanged.
// Otherwise find the next smaller number than *pwKey which is not in the set.
// Put the number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys smaller or equal to *pwKey are in the set.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
extern int PrevHole(Word_t wRoot, Word_t* pwKey);

// Unset all keys, free the memory used by the set, and set *pwRoot = 0.
Word_t Free(Word_t* pwRoot);

