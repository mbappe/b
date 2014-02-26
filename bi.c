
// This file is a wrapper for bli.c.
// It exists so we can generate bi.o, bi.s and bi.i files which include
// only Insert.
// Unfortunately, the compiler complains that Insert isn't used and doesn't
// generate any code.  Why?  
// So we have __pInsert to work around it.

#include "b.h"

#if cnBitsPerDigit != 0

#define INSERT
#include "bli.c"
#undef INSERT

void *__pInsert = (void *)Insert;

#endif // cnBitsPerDigit != 0
