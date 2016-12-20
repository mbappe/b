
// This file is a wrapper for bli.c.
// It exists so we can generate bc.o, bc.s and bc.i files which
// include only Count.

#define COUNT
#include "b.h"
#include "bli.c"
#undef COUNT

