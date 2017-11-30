
// This file is a wrapper for bli.c.
// It exists in part so we can generate bc.o, bc.s and bc.i files which
// include only Count, et. al.

#define cbInsert  0
#define cbLookup  0
#define cbRemove  0
#define cbCount   1

#define IF_INSERT(_expr)  0
#define IF_NOT_INSERT(_expr)  _expr

#define IF_INS_OR_REM(_expr)  0

#define IF_LOOKUP(_expr)  0
#define IF_NOT_LOOKUP(_expr)  _expr

#define IF_COUNT(_expr)  _expr
#define IF_NOT_COUNT(_expr)

#define COUNT
#include "b.h"
#include "bli.c"
#undef COUNT

