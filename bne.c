// This file is a wrapper for bli.c.
// It exists in part so we can generate bne.o, bne.s and bne.i files
// which include only Judy1NextEmpty and bneL.o bneL.s and bneL.i files
// which include only JudyLNextEmpty.


#define IF_NEXT_EMPTY(_stmt)  _stmt
#define IF_NEXT_OR_NEXT_EMPTY(_stmt)  _stmt
#define IF_NOT_NEXT_EMPTY(_stmt)
#define DBGX  DBGN

#define NEXT_EMPTY
#include "b.h"
#ifdef NEW_NEXT_EMPTY
  #define strLookupOrInsertOrRemove  "NextEmpty"
  #define KeyFound  Success
  #include "bli.c"
#endif // NEW_NEXT_EMPTY
#undef NEXT_EMPTY

// If *pwKey is not in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger number than *pwKey which is not in the array.
// Put the found number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys bigger than or equal to *pwKey are in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
int
#ifdef B_JUDYL
JudyLFirstEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1FirstEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("JxFE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxFE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("JxFE: returning -1; caller error\n\n"));
        return -1; // caller error
    }
    DBGN(printf("JxFE: *pwKey " OWx"\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
  #ifdef LOOKUP_BEFORE_NEXT_EMPTY
      #ifdef B_JUDYL
    if (JudyLGet(PArray, wKeyLocal, NULL) == NULL)
      #else // B_JUDYL
    if (Judy1Test(PArray, wKeyLocal, NULL) == 0)
      #endif // B_JUDYL else
    {
        DBGN(printf("JxFE: returning *pwKey " OWx"\n\n", *pwKey));
        return 1; // found next empty
    }
  #endif // LOOKUP_BEFORE_NEXT_EMPTY
  #ifdef NEW_NEXT_EMPTY
    Status_t status = NextEmpty((Word_t)PArray, &wKeyLocal);
  #else // NEW_NEXT_EMPTY
    int nBL = cnBitsPerWord;
    Word_t *pwRoot = (Word_t*)&PArray;
    Link_t* pLn = STRUCT_OF(pwRoot, Link_t, ln_wRoot); (void)pLn;
    Word_t* pwLnX = NULL; (void)pwLnX;
    Status_t status = NextEmptyGuts(qya, &wKeyLocal, /* bPrev */ 0);
  #endif // NEW_NEXT_EMPTY else
    if (status == Success) {
        *pwKey = wKeyLocal;
        DBGN(printf("JxFE: returning *pwKey " OWx"\n", *pwKey));
    } else {
        DBGN(printf("JxNE: returning 0; no first empty key\n\n"));
    }
    return status == Success;
}

// Find the next bigger number than *pwKey which is not in the array.
// Put the found number in *pwKey.
// Return 1 if a number is found.
// Return 0 if all keys bigger than *pwKey are in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
int
#ifdef B_JUDYL
JudyLNextEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
Judy1NextEmpty(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
    DBGN(printf("\n\nJxNE: pwKey %p\n", (void *)pwKey));
    if (pwKey == NULL) {
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
            DBGN(printf("JxNE: je_Errno %d\n", PJError->je_Errno));
        }
        DBGN(printf("JxNE: returning -1; caller error\n\n"));
        return -1; // caller error
    }
    DBGN(printf("JxNE: *pwKey 0x%zx\n", *pwKey));
    Word_t wKeyLocal = *pwKey;
    if (++wKeyLocal == 0) {
        DBGN(printf("JxNE: returning 0; no next empty key\n\n"));
        return 0; // no empty exists after *pwKey
    }
      #ifdef B_JUDYL
    if (JudyLFirstEmpty(PArray, &wKeyLocal, PJError) == 1)
      #else // B_JUDYL
    if (Judy1FirstEmpty(PArray, &wKeyLocal, PJError) == 1)
      #endif // B_JUDYL else
    {
        *pwKey = wKeyLocal;
        DBGN(printf("JxNE: returning *pwKey " OWx"\n\n", *pwKey));
        return 1; // found next empty
    }
    DBGN(printf("JxNE: returning 0\n\n"));
    return 0;
}

