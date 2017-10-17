
// This file is a wrapper for bli.c.
// It exists in part so we can generate br.o, br.s and br.i files which
// include only Judy1Unset/Remove, et. al.

#define cbInsert  0
#define cbLookup  0
#define cbRemove  1
#define cbCount   0

#define IF_INSERT(_expr)  0
#define IF_NOT_INSERT(_expr)  _expr

#define IF_LOOKUP(_expr)  0
#define IF_NOT_LOOKUP(_expr)  _expr

#define IF_COUNT(_expr)  0
#define IF_NOT_COUNT(_expr)  _expr

#define REMOVE
#include "b.h"
#include "bli.c"
#undef REMOVE

