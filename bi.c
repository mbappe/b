
// This file is a wrapper for bli.c.
// It exists so we can generate bi.o, bi.s and bi.i files which include
// only Insert.

#define INSERT
#include "b.h"
#include "bli.c"
#undef INSERT

