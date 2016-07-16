
// This file is a wrapper for bli.c.
// It exists so we can generate bl.o, bl.s and bl.i files which include
// only Lookup.
// Unfortunately, the compiler complains that Lookup isn't used and doesn't
// generate any code.  Why?  
// So we have __pLookup to work around it.

#define LOOKUP
#include "b.h"
#include "bli.c"
#undef LOOKUP

