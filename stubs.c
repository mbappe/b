
#include "Judy.h"

#if 0
int Judy1SetArray(PPvoid_t PPArray,
    Word_t Count, const Word_t * const PIndex, P_JE)
{
    // suppress "unused" compiler warnings
    (void)PPArray; (void) Count; (void)PIndex;(void)PJError;
    printf("Judy1SetArray n/a\n"); exit(1);
}
#endif

int Judy1ByCount(Pcvoid_t PArray, Word_t Count, Word_t * PIndex, P_JE)
{ printf("Judy1ByCount n/a\n"); exit(1); }

Word_t Judy1MemUsed(Pcvoid_t PArray) { printf("Judy1MemUsed n/a\n"); exit(5); }

Word_t Judy1MemActive(Pcvoid_t PArray) { printf("Judy1MemActive n/a\n"); exit(6); }

int Judy1First(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1First n/a\n"); exit(7); }

int Judy1Next(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1Next n/a\n"); exit(8); }

int Judy1Last(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1Last n/a\n"); exit(9); }

int Judy1Prev(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1Prev n/a\n"); exit(10); }

int Judy1FirstEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1FirstEmpty n/a\n"); exit(11); }

int Judy1NextEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1NextEmpty n/a\n"); exit(12); }

int Judy1LastEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1LastEmpty n/a\n"); exit(13); }

int Judy1PrevEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("Judy1PrevEmpty n/a\n"); exit(14); }

// ****************************************************************************
// JUDYL FUNCTIONS:

PPvoid_t JudyLGet(Pcvoid_t PArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLIns(PPvoid_t PPArray, Word_t Index, P_JE)
{ printf("JudyLIns n/a\n"); exit(1); }

int JudyLDel(PPvoid_t PPArray, Word_t Index, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyLCount(Pcvoid_t PArray, Word_t Index1, Word_t Index2, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLByCount(Pcvoid_t PArray, Word_t Count, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyLFreeArray(PPvoid_t PPArray, P_JE) { printf("n/a\n"); exit(1); }

Word_t JudyLMemUsed(Pcvoid_t PArray) { printf("n/a\n"); exit(1); }

Word_t JudyLMemActive(Pcvoid_t PArray) { printf("n/a\n"); exit(1); }

PPvoid_t JudyLFirst(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLNext(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLLast(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyLPrev(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLFirstEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLNextEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLLastEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyLPrevEmpty(Pcvoid_t PArray, Word_t * PIndex, P_JE)
{ printf("n/a\n"); exit(1); }

// ****************************************************************************
// JUDYHS FUNCTIONS:

PPvoid_t JudyHSGet(Pcvoid_t x, void *y, Word_t z)
{ printf("n/a\n"); exit(1); }

PPvoid_t JudyHSIns( PPvoid_t x, void *y, Word_t z, P_JE)
{ printf("n/a\n"); exit(1); }

int JudyHSDel(PPvoid_t x, void *y, Word_t z, P_JE)
{ printf("n/a\n"); exit(1); }

Word_t JudyHSFreeArray(PPvoid_t x, P_JE) { printf("n/a\n"); exit(1); }

const char *Judy1MallocSizes = "Judy1MallocSizes go here.";
const char *JudyLMallocSizes = "JudyLMallocSizes go here.";

