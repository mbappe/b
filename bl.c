
// This file is a wrapper for bli.c.
// It exists so we can generate bl.o, bl.s and bl.i files which include
// only Lookup.

#define LOOKUP
#include "b.h"
#include "bli.c"
#undef LOOKUP

