
// This file is a wrapper for bli.c.
// It exists so we can generate br.o, br.s and br.i files which include
// only Remove.
// Unfortunately, the compiler complains that Remove isn't used and doesn't
// generate any code.  Why?  
// So we have __pRemove to work around it.

#include "b.h"

#define REMOVE
#include "bli.c"
#undef REMOVE

