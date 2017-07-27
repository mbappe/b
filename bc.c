
// This file is a wrapper for bli.c.
// It exists in part so we can generate bc.o, bc.s and bc.i files which
// include only Count, et. al.

#define cbInsert  0
#define cbLookup  0
#define cbRemove  0
#define cbCount   1

#define COUNT
#include "b.h"
#include "bli.c"
#undef COUNT

