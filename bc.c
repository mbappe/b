
// This file is a wrapper for bli.c.
// It exists in part so we can generate bc.o, bc.s and bc.i files which
// include only Judy1Count/Count1.

#define strLookupOrInsertOrRemove  "Count"
#define IF_COUNT(_stmt)  _stmt
#define IF_NOT_COUNT(_stmt)
#define KeyFound  Failure
#define DBGX  DBGC

#define COUNT
#include "b.h"
#include "bli.c"
#undef COUNT

// Return the number of keys that are present from wKey0 through wKey1.
// Include wKey0 and wKey1 in the count if they are present.
// Return zero if (wKey0 == 0) and (wKey1 == (Word_t)-1) and all keys are
// present.
// Use (*pwRoot == 0) to disambiguate no keys present from all keys present.
//
// 'typedef const void * Pcvoid_t' aka 'typedef void * const Pcvoid_t'
// Pcvoid_t is a pointer to a constant.
// The value of *PArray cannot be changed.
Word_t
#ifdef B_JUDYL
JudyLCount(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, JError_t *pJError)
#else // B_JUDYL
Judy1Count(Pcvoid_t PArray, Word_t wKey0, Word_t wKey1, JError_t *pJError)
#endif // B_JUDYL
{
    DBGC(printf("\n\nJudy1Count(wKey0 0x%02zx wKey1 0x%02zx)\n",
                wKey0, wKey1));

    // There is really no need for us to disambiguate since the caller
    // can do it just fine without our help.
    // The interesting case is when we return zero for full pop.
    // But the caller can identify this case by:
    // (wKey0 == 0) && (wKey1 == -1) && (PArray != NULL).
    // This is no more onerous than the Judy way of checking
    // pJError->je_Errno.
    // If we supported exceptions there might be an argument for us
    // making the distinction.

    // Return 0 if the array is empty or wKey0 > wKey1.
    // JudyCommon/JudyCount.c defines C_JERR for this case.
    // The 'C_' is an abbreviation for count.
    if ((PArray == (Pvoid_t)NULL) || (wKey0 > wKey1))
    {
        if (pJError != NULL)
        {
            pJError->je_Errno = JU_ERRNO_NONE; // zero pop
            pJError->je_ErrID = __LINE__;
        }

        return 0; // C_JERR
    }

    // Set je_Errno just in case the array is full and we return 0?
    // So we always set je_Errno to something? Unlike Unix which
    // sets errno only in the case of an error?
    if (pJError != NULL)
    {
        pJError->je_Errno = JU_ERRNO_FULL; // full pop
        pJError->je_ErrID = __LINE__;
    }

    int nBL = cnBitsPerWord;
  #ifdef QP_PLN
    Link_t *pLn = STRUCT_OF((Word_t*)&PArray, Link_t, ln_wRoot);
  #else // QP_PLN
    Word_t* pwRoot = (Word_t*)&PArray;
  #endif // QP_PLN else
  #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL;
  #endif // REMOTE_LNX
    qva;
  #ifdef COUNT_2
    // Count keys [wKey0, wKey1).
    Word_t wCount = Count(qya, wKey0, wKey1);
  #else // COUNT_2
    // Count returns the number of keys before the specified key.
    // It does not include the specified key.
    Word_t wCount0 = (wKey0 == 0) ? 0 : Count(qya, wKey0 - 1);
    DBGC(printf("Count wKey0 0x%02zx Count0 %zd\n", wKey0, wCount0));
    Word_t wCount1 = Count(qya, wKey1);
    DBGC(printf("Count wKey1 0x%02zx Count1 %zd\n", wKey1, wCount1));
    Word_t wCount = wCount1 - wCount0;
  #endif // COUNT_2 else

    DBGC(printf("Judy1Count returning wCount %" _fw"d\n\n", wCount));

    return wCount;
}

