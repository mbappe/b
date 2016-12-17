
// This file is a wrapper for bli.c.
// It exists so we can generate br.o, br.s and br.i files which include
// only Remove.

#define REMOVE
#include "b.h"
#include "bli.c"
#undef REMOVE

