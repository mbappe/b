#ifndef SJUDYT_T
#define SJUDYT_T

#ifndef __GNUC__
#error strongly typed judy arrays require GCC extensions.
#endif

// Uses GCC extensions to create a strongly typed version of JudyL arrays.
//
// Declare one with the JUDYL_TYPED macro such as
//
// struct list_head {
//      struct list_entry *head;
// };
//
// JUDYL_TYPED(struct list_head, float) weight_map;
//
// which will declare a map from struct list_head's to a float. This avoids
// issues like silently casting the float to an integral type losing the
// mantissa or having to break abstraction to dig into the internal structure of
// the struct.
//
// The Following constraints on the types used must be followed
// sizeof(key type) == sizeof(uintptr_t)
// sizeof(value type) <= sizeof(uintptr_t)
// There are static assertions that will produce a compile time error if these
// rules are broken.
//
// The jltFoo routines as well as jFoo routines are then fully generic over
// such declared maps.
//
// Possible Platform Restriction: punning via 'union' behaves in the same way as
// accessing the value via an aliased pointer for the 'key' type.
// if this is not true for your chosen key, or your key type has a smaller size
// than a uintptr_t then use coercions. There is no such restriction on the value type.

#include "sjudy.h"

// Declare a typed version of a judyL array.

#define JUDYL_TYPED(kt,vt) \
union { judyl_t j;         \
        Pvoid_t v;         \
        kt kdummy[0];      \
        vt vdummy[0];      \
_Static_assert(sizeof(kt) == sizeof(uintptr_t), "key type size must be == sizeof(uintptr_t)"); \
_Static_assert(sizeof(vt) <= sizeof(uintptr_t), "value type size must be <= sizeof(uintptr_t)"); \
_Static_assert(__alignof__(kt) <= __alignof__(uintptr_t), "key type alignment must be <= alignof(uintptr_t)"); \
_Static_assert(__alignof__(vt) <= __alignof__(uintptr_t), "value type alignment must be <= alignof(uintptr_t)"); \
}

// Cast a value via punning.
#define PCAST(x,fty,tty) ((union { fty in; tty out;}){ .in = (x) }).out

#define _JLT_TYPE(v)  __typeof__ (&(v)->vdummy[0])
#define _JLT_KTYPE(v)  __typeof__ ((v)->kdummy[0])
#define _JLT_DTYPE(v)  __typeof__ (&(v).vdummy[0])
#define _JLT_DKTYPE(v)  __typeof__ ((v).kdummy[0])

//#define jltIns(a,i) (_JLT_TYPE(a))jlIns(&(a)->j,PCAST(i,_JLT_KTYPE(a),uintptr_t))
#define jltIns(a,i) (_JLT_TYPE(a))jlIns(&(a)->j,PCAST(i,_JLT_KTYPE(a),uintptr_t))
#define jltGet(a,i) (_JLT_TYPE(a))jlGet((a).j, PCAST(i,_JLT_KTYPE(a),uintptr_t))
#define jltDel(a,i) jlDel(&(a)->j, PCAST(i,_JLT_KTYPE(a),uintptr_t))
#define jltFirst(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); (_JLT_DTYPE(a))jlFirst((a).j, (uintptr_t *)(pi)); })
#define jltNext(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); (_JLT_DTYPE(a))jlNext((a).j, (uintptr_t *)(pi)); })
#define jltPrev(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); (_JLT_DTYPE(a))jlPrev((a).j, (uintptr_t *)(pi)); })
#define jltLast(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); (_JLT_DTYPE(a))jlLast((a).j, (uintptr_t *)(pi)); })

#define jltFirstEmpty(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); jlFirstEmpty((a).j, (uintptr_t *)(pi)); })
#define jltNextEmpty(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); jlNextEmpty((a).j, (uintptr_t *)(pi)); })
#define jltPrevEmpty(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); jlPrevEmpty((a).j, (uintptr_t *)(pi)); })
#define jltLastEmpty(a,pi) ({_Static_assert(__builtin_types_compatible_p(_JLT_DKTYPE(a) *, __typeof__ (pi)), "invalid key type"); jlLastEmpty((a).j, (uintptr_t *)(pi)); })

/*
Copyright (c) John Meacham, 2014
http://notanumber.net

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#endif
