
// This file is a wrapper for bli.c.
// It exists so we can generate bl.o, bl.s and bl.i files which include
// only Lookup.
// Unfortunately, the compiler complains that Lookup isn't used and doesn't
// generate any code.  Why?  
// So we have __pLookup to work around it.

#include "b.h"

#if cnBitsPerDigit != 0

#define LOOKUP
#include "bli.c"
#undef LOOKUP

void *__pLookup = (void *)Lookup;

#endif // cnBitsPerDigit != 0

