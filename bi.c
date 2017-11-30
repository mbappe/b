
// This file is a wrapper for bli.c.
// It exists in part so we can generate bi.o, bi.s and bi.i files which
// include only Judy1Set/Insert, et. al.

#define cbInsert  1
#define cbLookup  0
#define cbRemove  0
#define cbCount   0

#define IF_INSERT(_expr)  _expr
#define IF_NOT_INSERT(_expr)

#define IF_INS_OR_REM(_expr)  _expr

#define IF_LOOKUP(_expr)  0
#define IF_NOT_LOOKUP(_expr)  _expr

#define IF_COUNT(_expr)  0
#define IF_NOT_COUNT(_expr)  _expr

#define INSERT
#include "b.h"
#include "bli.c"
#undef INSERT

