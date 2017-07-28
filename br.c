
// This file is a wrapper for bli.c.
// It exists in part so we can generate br.o, br.s and br.i files which
// include only Judy1Unset/Remove, et. al.

#define cbInsert  0
#define cbLookup  0
#define cbRemove  1
#define cbCount   0

#define IF_LOOKUP(_stmt)
#define IF_NOT_LOOKUP(_stmt)  _stmt

#define IF_COUNT(_stmt)
#define IF_NOT_COUNT(_stmt)  _stmt

#define REMOVE
#include "b.h"
#include "bli.c"
#undef REMOVE

