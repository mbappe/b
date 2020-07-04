
// This file is a wrapper for bli.c.
// It exists in part so we can generate bi.o, bi.s and bi.i files which
// include only Judy1Set/Insert1.

#define INSERT
#define strLookupOrInsertOrRemove  "Insert"
#define IF_INSERT(_stmt)  _stmt
#define IF_NOT_INSERT(_stmt)
#define IF_INS_OR_REM(_stmt)  _stmt
#define KeyFound  Failure
#define DBGX  DBGI
#include "b.h"
#include "bli.c"
#undef INSERT

