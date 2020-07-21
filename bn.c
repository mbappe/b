
// This file is a wrapper for bli.c.
// It exists in part so we can generate bn.o, bn.s and bn.i files which
// include only Judy1Next/Next1.

#define strLookupOrInsertOrRemove  "Next"
#define IF_NEXT(_stmt)  _stmt
#define IF_NOT_NEXT(_stmt)
#define KeyFound  Success
#define DBGX  DBGN

#define NEXT
#include "b.h"
#include "bli.c"
#undef NEXT

#if !defined(NEW_NEXT) || defined(NEXT_QPA)
    #define _NEXT_QPA
#elif defined(NEXT_FROM_WRAPPER) || defined(NEXT_QP) // !NEW_NEXT || NEXT_QPA
    #define _NEXT_QP
#endif // !NEW_NEXT || NEXT_QPA elif NEXT_FROM_WRAPPER || NEXT_QP

// If *pwKey is in the array then return 1 and leave *pwKey unchanged.
// Otherwise find the next bigger key than *pwKey which is in the array.
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is bigger than the biggest key in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
#ifdef B_JUDYL
PPvoid_t
JudyLFirst(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1First(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
  #ifdef NEXT_CHECK_ARGS
    if (PArray == 0) {
        return 0; // NULL for JudyL
    }
    assert(PArray != WROOT_NULL); // WROOT_NULL is not used for an empty array.
    if (pwKey == NULL) {
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
        }
        return BJL(PPJERR) BJ1(JERR);
    }
  #else // NEXT_CHECK_ARGS
    (void)PJError;
  #endif // NEXT_CHECK_ARGS else
    DBGN(printf("\n# JxF: *pwKey " OWx"\n", *pwKey));
  #if defined(_NEXT_QPA) || defined(_NEXT_QP)
    int nBL = cnBitsPerWord;
      #ifdef QP_PLN
    Link_t* pLn = STRUCT_OF((Word_t*)&PArray, Link_t, ln_wRoot);
      #else // QP_PLN
    Word_t* pwRoot = (Word_t*)&PArray; (void)pwRoot;
      #endif // QP_PLN else
      #ifdef _NEXT_QPA
      #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL; (void)pwLnX;
      #endif // REMOTE_LNX
      #endif // _NEXT_QPA
  #endif // _NEXT_QPA || _NEXT_QP
  #ifdef NEXT_FROM_WRAPPER
    if (wr_nType((Word_t)PArray) == T_LIST) {
        int nPos = SearchList(qy, /*nBLR*/ nBL, *pwKey);
        if (nPos < 0) {
            int nPopCnt = gnListPopCnt(qy, /*nBLR*/ nBL);
            if ((nPos ^= -1) >= nPopCnt) {
                return BJL(NULL)BJ1(0);
            }
        }
        int nPopCnt = gnListPopCnt(qy, /*nBLR*/ nBL); (void)nPopCnt;
        *pwKey = ls_pwKeysX(wr_pwr((Word_t)PArray), nBL, nPopCnt)[nPos];
        return BJL((PPvoid_t)&gpwValues(qy)[~nPos])BJ1(1);
    }
  #endif // NEXT_FROM_WRAPPER
  #ifdef NEW_NEXT
      #ifdef NEW_NEXT_IS_EXCLUSIVE
    Word_t wKey = *pwKey;
    BJL(Word_t* pwVal);
    if (wKey == 0) {
        if (BJL((pwVal = (Word_t*)JudyLGet(PArray, 0, NULL)) != NULL)
            BJ1(Judy1Test(PArray, 0, NULL) == 1)
            )
        {
            return BJL((PPvoid_t)pwVal) BJ1(1);
        }
    } else {
        --wKey;
    }
          #ifdef NEXT_QPA
    BJL(pwVal = NextL(qya, &wKey));
    BJ1(Status_t status = Next1(qya, &wKey));
          #elif defined(NEXT_QP)
    BJL(pwVal = NextL(qy, &wKey));
    BJ1(Status_t status = Next1(qy, &wKey));
          #else // NEXT_QPA elif NEXT_QP
    BJL(pwVal = NextL((Word_t)PArray, &wKey));
    BJ1(Status_t status = Next1((Word_t)PArray, &wKey));
          #endif // NEXT_QPA elif NEXT_QP else
    if (BJL(pwVal != NULL) BJ1(status == Success)) {
        *pwKey = wKey;
    }
      #else // NEW_NEXT_IS_EXCLUSIVE
          #ifdef NEXT_QPA
    BJL(Word_t* pwVal = NextL(qya, pwKey));
    BJ1(Status_t status = Next1(qya, pwKey));
          #elif defined(NEXT_QP)
    BJL(Word_t* pwVal = NextL(qy, pwKey));
    BJ1(Status_t status = Next1(qy, pwKey));
          #else // NEXT_QPA elif NEXT_QP
    BJL(Word_t* pwVal = NextL((Word_t)PArray, pwKey));
    BJ1(Status_t status = Next1((Word_t)PArray, pwKey));
          #endif // NEXT_QPA elif NEXT_QP else
      #endif // NEW_NEXT_IS_EXCLUSIVE else
    BJL(DBGN(printf("# JxF: pwVal %p\n", pwVal)));
    BJ1(DBGN(printf("# JxF: status %d\n", status)));
    if (BJL(pwVal != NULL) BJ1(status == Success))
  #else // NEW_NEXT
    Word_t wKey = *pwKey;
    BJL(Word_t* pwVal = NULL);
    Status_t status = NextGuts(qya, &wKey, /*wCount*/0, /*bPrev*/0, /*bEmpty*/0
          #ifdef B_JUDYL
                             , &pwVal
          #endif // B_JUDYL
                               )
                        ? Failure : Success;
    if (status == Success) {
        *pwKey = wKey;
    }
    DBGN(printf("# JxF: status %d\n", status));
    if (status == Success)
  #endif // NEW_NEXT else
    {
  #ifndef NEW_NEXT
        *pwKey = wKey;
  #endif // !NEW_NEXT
        DBGN(printf("# JxF: *pwKey 0x%zx\n", *pwKey));
  #ifdef B_JUDYL
      #ifndef NEW_NEXT
        if (pwVal == NULL) {
            pwVal = (Word_t*)JudyLGet(PArray, *pwKey, NULL);
        }
        assert(pwVal != NULL);
      #endif // !NEW_NEXT
        DBGN(printf("# JxF: pwVal %p *pwVal 0x%02zx\n", pwVal, *pwVal));
  #endif // B_JUDYL
    }
    DBGN(printf("\n"));
    return BJL((PPvoid_t)pwVal) BJ1(status);
}

// Find the next bigger key than *pwKey which is in the array.
// Put the found key in *pwKey.
// Return 1 if a key is found.
// Return 0 if *pwKey is bigger than or equal to the biggest key in the array.
// Return -1 if pwKey is NULL.
// *pwKey is undefined if anything other than 1 is returned.
// But we go to the trouble of preserving *pwKey if anything other than 1
// is returned to compare with JudyA.
// pwKey must be a valid, non-NULL pointer.
// *pwKey must be less than -(Word_t)1.
// PArray must be a non-empty array.
#ifdef B_JUDYL
PPvoid_t
JudyLNext(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#else // B_JUDYL
int
Judy1Next(Pcvoid_t PArray, Word_t *pwKey, PJError_t PJError)
#endif // B_JUDYL
{
  #ifdef NEXT_CHECK_ARGS
    if (PArray == 0) {
        return 0; // NULL for JudyL
    }
    assert(PArray != WROOT_NULL); // WROOT_NULL is not used for an empty array.
    if (pwKey == NULL) {
        if (PJError != NULL) {
            PJError->je_Errno = JU_ERRNO_NULLPINDEX;
        }
        return BJL(PPJERR) BJ1(JERR);
    }
  #else // NEXT_CHECK_ARGS
    (void)PJError;
  #endif // NEXT_CHECK_ARGS else
    DBGN(printf("\n# JxN: *pwKey 0x%zx\n", *pwKey));
  #ifndef NEW_NEXT_IS_EXCLUSIVE
    Word_t wKey = *pwKey + 1;
      #if defined(DEBUG) || defined(NEXT_CHECK_ARGS)
    if (wKey == 0) {
        return 0; // NULL for JudyL
    }
      #endif // DEBUG || NEXT_CHECK_ARGS
  #endif // !NEW_NEXT_IS_EXCLUSIVE
  #if defined(_NEXT_QPA) || defined(_NEXT_QP)
    int nBL = cnBitsPerWord;
      #ifdef QP_PLN
    Link_t* pLn = STRUCT_OF((Word_t*)&PArray, Link_t, ln_wRoot);
      #else // QP_PLN
    Word_t* pwRoot = (Word_t*)&PArray; (void)pwRoot;
      #endif // QP_PLN else
      #ifdef _NEXT_QPA
      #ifdef REMOTE_LNX
    Word_t* pwLnX = NULL; (void)pwLnX;
      #endif // REMOTE_LNX
      #endif // _NEXT_QPA
  #endif // _NEXT_QPA || _NEXT_QP
  #ifdef NEXT_FROM_WRAPPER
    if (wr_nType((Word_t)PArray) == T_LIST) {
        int nPos = SearchList(qy, /*nBLR*/ nBL, wKey);
        if (nPos < 0) {
            int nPopCnt = gnListPopCnt(qy, /*nBLR*/ nBL);
            if ((nPos ^= -1) >= nPopCnt) {
                return BJL(NULL)BJ1(0);
            }
        }
        int nPopCnt = gnListPopCnt(qy, /*nBLR*/ nBL); (void)nPopCnt;
        *pwKey = ls_pwKeysX(wr_pwr((Word_t)PArray), nBL, nPopCnt)[nPos];
        return BJL((PPvoid_t)&gpwValues(qy)[~nPos])BJ1(1);
    }
  #endif // NEXT_FROM_WRAPPER
  #ifdef NEW_NEXT
      #ifdef NEW_NEXT_IS_EXCLUSIVE
          #ifdef NEXT_QPA
    BJL(Word_t* pwVal = NextL(qya, pwKey));
    BJ1(Status_t status = Next1(qya, pwKey));
          #elif defined(NEXT_QP)
    BJL(Word_t* pwVal = NextL(qy, pwKey));
    BJ1(Status_t status = Next1(qy, pwKey));
          #else // NEXT_QPA elif NEXT_QP
    BJL(Word_t* pwVal = NextL((Word_t)PArray, pwKey));
    BJ1(Status_t status = Next1((Word_t)PArray, pwKey));
          #endif // NEXT_QPA elif NEXT_QP else
      #else // NEW_NEXT_IS_EXCLUSIVE
          #ifdef NEXT_QPA
    BJL(Word_t* pwVal = NextL(qya, &wKey));
    BJ1(Status_t status = Next1(qya, &wKey));
          #elif defined(NEXT_QP)
    BJL(Word_t* pwVal = NextL(qy, &wKey));
    BJ1(Status_t status = Next1(qy, &wKey));
          #else // NEXT_QPA elif NEXT_QP
    BJL(Word_t* pwVal = NextL((Word_t)PArray, &wKey));
    BJ1(Status_t status = Next1((Word_t)PArray, &wKey));
          #endif // NEXT_QPA elif NEXT_QP else
      #endif // NEW_NEXT_IS_EXCLUSIVE else
    BJL(DBGN(printf("# JxN: pwVal %p\n", pwVal)));
    BJ1(DBGN(printf("# JxN: status %d\n", status)));
    if (BJL(pwVal != NULL) BJ1(status == Success))
  #else // NEW_NEXT
    BJL(Word_t* pwVal = NULL);
    Status_t status = NextGuts(qya, &wKey, /*wCount*/0, /*bPrev*/0, /*bEmpty*/0
          #ifdef B_JUDYL
                             , &pwVal
          #endif // B_JUDYL
                               )
                        ? Failure : Success;
    DBGN(printf("# JxN: status %d\n", status));
    if (status == Success)
  #endif // NEW_NEXT else
    {
  #ifndef NEW_NEXT_IS_EXCLUSIVE
        *pwKey = wKey;
  #endif // !NEW_NEXT_IS_EXCLUSIVE
        DBGN(printf("# JxN B: *pwKey 0x%zx\n", *pwKey));
  #ifdef B_JUDYL
      #ifndef NEW_NEXT
        if (pwVal == NULL) {
            pwVal = (Word_t*)JudyLGet(PArray, *pwKey, NULL);
        }
        assert(pwVal != NULL);
      #endif // !NEW_NEXT
        DBGN(printf("# JxN: pwVal %p *pwVal 0x%02zx\n", pwVal, *pwVal));
  #endif // B_JUDYL
    }
    DBGN(printf("\n"));
    return BJL((PPvoid_t)pwVal) BJ1(status);
}
