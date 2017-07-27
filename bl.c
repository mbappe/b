
// This file is a wrapper for bli.c.
// It exists in part so we can generate bl.o, bl.s and bl.i files which
// include only Judy1Test/Lookup, et. al.

#define cbInsert  0
#define cbLookup  1
#define cbRemove  0
#define cbCount   0

#define LOOKUP
#include "b.h"
#include "bli.c"
#undef LOOKUP

