
// This file is a wrapper for bli.c.
// It exists in part so we can generate bc.o, bc.s and bc.i files which
// include only Judy1Count/Count1.

#define COUNT
#define strLookupOrInsertOrRemove  "Count"
#define IF_COUNT(_stmt)  _stmt
#define IF_NOT_COUNT(_stmt)
#define KeyFound  Failure
#define DBGX  DBGC
#include "b.h"
#include "bli.c"
#undef COUNT

