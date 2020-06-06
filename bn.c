
// This file is a wrapper for bli.c.
// It exists in part so we can generate bn.o, bn.s and bn.i files which
// include only Judy1Next/Next1.

#define NEXT
#define strLookupOrInsertOrRemove  "Next"
#define IF_NEXT(_stmt)  _stmt
#define IF_NOT_NEXT(_stmt)
#define KeyFound  Success
#define DBGX  DBGN
#include "b.h"
#include "bli.c"
#undef NEXT

