
// This file is a wrapper for bli.c.
// It exists so we can generate .o, .s and .i files which include only Lookup.

#include "b.h"

#define LOOKUP
#include "bli.c"

