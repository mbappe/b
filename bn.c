
// This file is a wrapper for bli.c.
// It exists so we can generate bn.o, bn.s and bn.i files which
// include only Next.

#define NEXT
#include "b.h"
#include "bli.c"
#undef NEXT

