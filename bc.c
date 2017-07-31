
// This file is a wrapper for bli.c.
// It exists in part so we can generate bc.o, bc.s and bc.i files which
// include only Count, et. al.

#define cbInsert  0
#define cbLookup  0
#define cbRemove  0
#define cbCount   1

#define IF_INSERT(_stmt)  0
#define IF_NOT_INSERT(_stmt)  _stmt

#define IF_LOOKUP(_stmt)
#define IF_NOT_LOOKUP(_stmt)  _stmt

#define IF_COUNT(_stmt)  _stmt
#define IF_NOT_COUNT(_stmt)

#define COUNT
#include "b.h"
#include "bli.c"
#undef COUNT

