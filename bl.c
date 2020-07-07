
// This file is a wrapper for bli.c.
// It exists in part so we can generate bl.o, bl.s and bl.i files which
// include only Judy1Test/Lookup1.

#define LOOKUP
#define strLookupOrInsertOrRemove  "Lookup"
#define IF_LOOKUP(_stmt)  _stmt
#define IF_NOT_LOOKUP(_stmt)
#define KeyFound  Success
#define DBGX  DBGL
#include "b.h"
#include "bli.c"
#undef LOOKUP

