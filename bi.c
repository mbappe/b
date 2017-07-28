
// This file is a wrapper for bli.c.
// It exists in part so we can generate bi.o, bi.s and bi.i files which
// include only Judy1Set/Insert, et. al.

#define cbInsert  1
#define cbLookup  0
#define cbRemove  0
#define cbCount   0

#define IF_LOOKUP(_stmt)
#define IF_NOT_LOOKUP(_stmt)  _stmt

#define IF_COUNT(_stmt)
#define IF_NOT_COUNT(_stmt)  _stmt

#define INSERT
#include "b.h"
#include "bli.c"
#undef INSERT

