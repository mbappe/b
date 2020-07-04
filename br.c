
// This file is a wrapper for bli.c.
// It exists in part so we can generate br.o, br.s and br.i files which
// include only Judy1Unset/Remove1.

#define REMOVE
#define strLookupOrInsertOrRemove  "Remove"
#define IF_REMOVE(_stmt)  _stmt
#define IF_NOT_REMOVE(_stmt)
#define IF_INS_OR_REM(_stmt)  _stmt
#define KeyFound  Success
#define DBGX  DBGR
#include "b.h"
#include "bli.c"
#undef REMOVE

