
// This file is a wrapper for bli.c.
// It exists so we can generate bl.o, bl.s and bl.i files which include
// only Lookup.
// Unfortunately, the compiler complains that Lookup isn't used and doesn't
// generate any code.  Why?  
// So we have __p to work around it.

#include "b.h"

#define LOOKUP
#include "bli.c"

void *__p = Lookup;

