#ifndef SJUDY_H
#define SJUDY_H

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error This must be compiled under c99 mode.
#endif

// Platform assumptions:
// sizeof(uintptr_t) == sizeof(Word_t) as defined by Judy.
// The null pointer is represented as the all zero bit pattern.

// Naming scheme, all functions have the scheme
// j[t][k][v]Action
//
// where t is the type of array (L for map, 1 for bitset). if missing, the
// funcition is generic to both.
// k is the type of the index w for word, p for pointer.
// v is the type of the value w for word, p for pointer.
// when the argument types are omitted, 'w' is assumed.
//
// The action names match the names of the judy functions.
// 'word' is used as a synonym for a value of type uintptr_t.
// Although defined as macros, some exported names use the function
// naming convention when the forced use of a macro is independent of how it
// should be used. Such routines behave as functions would with respect to their
// arguments/return values.

#include <stdbool.h>
#include <stdint.h>

#include <Judy.h>

// psuedo-prototypes for macros
//
// JUDY_INIT - static initializer.
//
// uintptr_t jToWord(judyx_t a);
// bool jEmpty(judyx_t x);
// bool jWordIsArray(uintptr_t);
// judyl_t jlFromWord(uintptr_t w);
// judy1_t j1FromWord(uintptr_t w);

#define _JINLINE static inline

// JudyL
typedef struct { Pvoid_t v; } judyl_t;

// uintptr_t -> uintptr_t
_JINLINE uintptr_t *jlIns(judyl_t *j, uintptr_t i);
_JINLINE uintptr_t *jlGet(judyl_t j, uintptr_t i);
_JINLINE bool       jlDel(judyl_t *j, uintptr_t i);
_JINLINE size_t     jlCount(judyl_t j, uintptr_t i1, uintptr_t i2);
_JINLINE uintptr_t *jlByCount(judyl_t j, size_t count, uintptr_t *i);
_JINLINE size_t     jlFree(judyl_t *j);
_JINLINE size_t     jlMemUsed(judyl_t j);
_JINLINE uintptr_t *jlFirst(judyl_t a, uintptr_t *i);
_JINLINE uintptr_t *jlNext(judyl_t a, uintptr_t *i);
_JINLINE uintptr_t *jlLast(judyl_t a, uintptr_t *i);
_JINLINE uintptr_t *jlPrev(judyl_t a, uintptr_t *i);
_JINLINE bool       jlFirstEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlNextEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlLastEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlPrevEmpty(judyl_t a, uintptr_t *i);

// uintptr_t -> void *
_JINLINE void **    jlwpIns(judyl_t *j, uintptr_t i);
_JINLINE void **    jlwpGet(judyl_t j, uintptr_t i);
_JINLINE bool       jlwpDel(judyl_t *j, uintptr_t i);
_JINLINE size_t     jlwpCount(judyl_t j, uintptr_t i1, uintptr_t i2);
_JINLINE void **    jlwpByCount(judyl_t j, size_t count, uintptr_t *i);
_JINLINE void **    jlwpFirst(judyl_t a, uintptr_t *i);
_JINLINE void **    jlwpNext(judyl_t a, uintptr_t *i);
_JINLINE void **    jlwpLast(judyl_t a, uintptr_t *i);
_JINLINE void **    jlwpPrev(judyl_t a, uintptr_t *i);
_JINLINE bool       jlwpFirstEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlwpNextEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlwpLastEmpty(judyl_t a, uintptr_t *i);
_JINLINE bool       jlwpPrevEmpty(judyl_t a, uintptr_t *i);

// void * -> void *
_JINLINE void **   jlppIns(judyl_t *j, void *i);
_JINLINE void **   jlppGet(judyl_t j, void *i);
_JINLINE bool      jlppDel(judyl_t *j, void *i);
_JINLINE size_t    jlppCount(judyl_t j, void *i1, void *i2);
_JINLINE void **   jlppByCount(judyl_t j, size_t count, void **i);
_JINLINE void **   jlppFirst(judyl_t a, void **i);
_JINLINE void **   jlppNext(judyl_t a, void **i);
_JINLINE void **   jlppLast(judyl_t a, void **i);
_JINLINE void **   jlppPrev(judyl_t a, void **i);
_JINLINE bool      jlppFirstEmpty(judyl_t a, void **i);
_JINLINE bool      jlppNextEmpty(judyl_t a, void **i);
_JINLINE bool      jlppLastEmpty(judyl_t a, void **i);
_JINLINE bool      jlppPrevEmpty(judyl_t a, void **i);

// void * -> uintptr_t
_JINLINE uintptr_t* jlpwIns(judyl_t *j, void *i);
_JINLINE uintptr_t* jlpwGet(judyl_t j, void *i);
_JINLINE bool       jlpwDel(judyl_t *j, void *i);
_JINLINE size_t     jlpwCount(judyl_t j, void *i1, void *i2);
_JINLINE uintptr_t* jlpwByCount(judyl_t j, size_t count, void **i);
_JINLINE uintptr_t* jlpwFirst(judyl_t a, void **i);
_JINLINE uintptr_t* jlpwNext(judyl_t a, void **i);
_JINLINE uintptr_t* jlpwLast(judyl_t a, void **i);
_JINLINE uintptr_t* jlpwPrev(judyl_t a, void **i);
_JINLINE bool       jlpwFirstEmpty(judyl_t a, void **i);
_JINLINE bool       jlpwNextEmpty(judyl_t a, void **i);
_JINLINE bool       jlpwLastEmpty(judyl_t a, void **i);
_JINLINE bool       jlpwPrevEmpty(judyl_t a, void **i);

// Judy1

typedef struct { Pvoid_t v; } judy1_t;

_JINLINE bool   j1Set(judy1_t *j, uintptr_t i);
_JINLINE bool   j1Unset(judy1_t *j, uintptr_t i);
_JINLINE bool   j1Test(judy1_t j, uintptr_t i);
_JINLINE bool   j1ByCount(judy1_t j, size_t count, uintptr_t *i);
_JINLINE size_t j1Count(judy1_t j, uintptr_t i1, uintptr_t i2);
_JINLINE size_t j1Free(judy1_t *j);
_JINLINE size_t j1MemUsed(judy1_t j);
_JINLINE bool   j1FirstEmpty(judy1_t a, uintptr_t *i);
_JINLINE bool   j1NextEmpty(judy1_t a, uintptr_t *i);
_JINLINE bool   j1LastEmpty(judy1_t a, uintptr_t *i);
_JINLINE bool   j1PrevEmpty(judy1_t a, uintptr_t *i);
_JINLINE bool   j1First(judy1_t a, uintptr_t *i);
_JINLINE bool   j1Next(judy1_t a, uintptr_t *i);
_JINLINE bool   j1Last(judy1_t a, uintptr_t *i);
_JINLINE bool   j1Prev(judy1_t a, uintptr_t *i);

/*
 defined as macros:
_JINLINE bool   j1pSet(judy1_t *arr, void *index);
_JINLINE bool   j1pUnset(judy1_t *arr, void *index);
_JINLINE bool   j1pTest(judy1_t arr, void *index);
_JINLINE bool   j1pByCount(judy1_t arr, size_t count, void **index);
_JINLINE size_t j1pCount(judy1_t arr, void *i1, void *i2);
_JINLINE size_t j1pFree(judy1_t *arr);
_JINLINE size_t j1pMemUsed(judy1_t arr);
_JINLINE bool   j1pFirstEmpty(judy1_t a, void **i);
_JINLINE bool   j1pNextEmpty(judy1_t a, void **i);
_JINLINE bool   j1pLastEmpty(judy1_t a, void **i);
_JINLINE bool   j1pPrevEmpty(judy1_t a, void **i);
_JINLINE bool   j1pFirst(judy1_t a, void **i);
_JINLINE bool   j1pNext(judy1_t a, void **i);
_JINLINE bool   j1pLast(judy1_t a, void **i);
_JINLINE bool   j1pPrev(judy1_t a, void **i);
*/

#define JUDY_INIT { .v = 0 }

// Convert a judy array to a word sized integer, use the appropriate FromWord
// routine to convert back.
#define jToWord(j)  ((uintptr_t)((j).v))
#define jEmpty(j) (jToWord(j) == 0)
#define jWordIsArray(w) (!((w) & JLAP_INVALID))

// takes a word obtained from jToWord and convert it back to a judyl_t
#define jlFromWord(w)  (judyl_t){ .v = (Pvoid_t)w; }

uintptr_t *
jlIns(judyl_t *arr, uintptr_t index)
{
        return (uintptr_t *)JudyLIns(&arr->v, index, PJE0);
}

uintptr_t *
jlGet(judyl_t arr, uintptr_t index)
{
        return (uintptr_t *)JudyLGet(arr.v, index, PJE0);
}

bool
jlDel(judyl_t * arr, uintptr_t index)
{
        return (bool)JudyLDel(&arr->v, index, PJE0);
}

size_t
jlCount(judyl_t arr, uintptr_t i1, uintptr_t i2)
{
        return JudyLCount(arr.v, i1, i2, PJE0);
}

uintptr_t *
jlByCount(judyl_t arr, size_t count, uintptr_t *index)
{
        return (uintptr_t *)JudyLByCount(arr.v, count, index, PJE0);
}

size_t
jlFree(judyl_t * arr)
{
        return (size_t)JudyLFreeArray(&arr->v, PJE0);
}

size_t
jlMemUsed(judyl_t arr)
{
        return (size_t)JudyLMemUsed(arr.v);
}

#define SJUDY_ITER(x)   \
        uintptr_t *jl##x(judyl_t arr, uintptr_t * index) \
        { return (uintptr_t *)JudyL##x(arr.v, index, PJE0); }\
        bool jl##x##Empty(judyl_t arr, uintptr_t * index) \
        { return (bool)JudyL##x##Empty(arr.v, index, PJE0); }

SJUDY_ITER(First)
SJUDY_ITER(Next)
SJUDY_ITER(Last)
SJUDY_ITER(Prev)

#undef SJUDY_ITER

#define JLPP(func,p,r) \
            r jlpp##func(judyl_t p a, void *i) { \
                    return (r)jl##func(a, (uintptr_t)i); }     \
            r jlwp##func(judyl_t p a, uintptr_t i) { \
                    return (r)jl##func(a, i); }   \
            uintptr_t *jlpw##func(judyl_t p a, void *i) { \
                    return jl##func(a, (uintptr_t)i); }
#define JLPP2(func,p,r) \
            r jlpp##func(judyl_t p a, void *i1, void *i2) { \
                    return (r)jl##func(a, (uintptr_t)i1, (uintptr_t)i2); }        \
            r jlwp##func(judyl_t p a, uintptr_t i1, uintptr_t i2) { \
                    return (r)jl##func(a, i1, i2); }                              \
            uintptr_t jlpw##func(judyl_t p a, void *i1, void *i2) { \
                    return jl##func(a, (uintptr_t)i1, (uintptr_t)i2); }
#define JLPPI(func) \
            void **jlpp##func(judyl_t a, void **i) { \
                    return (void **)jl##func(a, (uintptr_t *)i); }  \
            bool jlpp##func##Empty(judyl_t a, void **i) { \
                    return jl##func##Empty(a, (uintptr_t *)i); }       \
            void **jlwp##func(judyl_t a, uintptr_t *i) { \
                    return (void **)jl##func(a, i); }  \
            bool jlwp##func##Empty(judyl_t a, uintptr_t *i) { \
                    return jl##func##Empty(a, i); } \
            uintptr_t *jlpw##func(judyl_t a, void **i) { \
                    return jl##func(a, (uintptr_t *)i); }  \
            bool jlpw##func##Empty(judyl_t a, void **i) { \
                    return jl##func##Empty(a, (uintptr_t *)i); }
#define JLPD(pw, ai) \
             bool jl##pw##Del(judyl_t *a, ai i) { \
                    return jlDel(a, (uintptr_t)i); }
#define _JLBC(pw, r, ai) \
             r *jl##pw##ByCount(judyl_t a, size_t count, ai *i2) { \
                    return (r *)jlByCount(a, count, (uintptr_t *)i2); }

JLPP(Ins,*,void **);
JLPP(Get,,void **);
JLPD(pp, void *);
JLPD(wp, uintptr_t);
JLPD(pw, void *);

_JLBC(pp, void *, void *);
_JLBC(wp, void *,uintptr_t);
_JLBC(pw, uintptr_t, void *);

JLPP2(Count,,size_t);
JLPPI(First);
JLPPI(Prev);
JLPPI(Last);
JLPPI(Next);

#undef _JLBC
#undef JLPP2
#undef JLPPI

// Judy1 Bitsets

static inline bool
j1Set(judy1_t * arr, uintptr_t index)
{
        return (bool)Judy1Set(&arr->v, index, PJE0);
}
static inline bool
j1Unset(judy1_t * arr, uintptr_t index)
{
        return (bool)Judy1Unset(&arr->v, index, PJE0);
}

static inline bool
j1Test(judy1_t arr, uintptr_t index)
{
        return (bool)Judy1Test(arr.v, index, PJE0);
}

static inline bool
j1ByCount(judy1_t arr, size_t count, uintptr_t *index)
{
        return (bool)Judy1ByCount(arr.v, count, index, PJE0);
}

static inline size_t
j1Count(judy1_t arr, uintptr_t i1, uintptr_t i2)
{
        return Judy1Count(arr.v, i1, i2, PJE0);
}

static inline size_t
j1Free(judy1_t * arr)
{
        return (size_t)Judy1FreeArray(&arr->v, PJE0);
}

static inline size_t
j1MemUsed(judy1_t arr)
{
        return (size_t)Judy1MemUsed(arr.v);
}

#define SJUDY_ITER(x)   \
        static inline bool \
        j1##x(judy1_t arr, uintptr_t * index) \
        { return (bool)Judy1##x(arr.v, index, PJE0); }\
        static inline bool \
        j1##x##Empty(judy1_t arr, uintptr_t * index) \
        { return (bool)Judy1##x##Empty(arr.v, index, PJE0); }

SJUDY_ITER(First)
SJUDY_ITER(Next)
SJUDY_ITER(Last)
SJUDY_ITER(Prev)

#undef SJUDY_ITER

#define j1pSet(a,i) j1Set(a, (uintptr_t)(i))
#define j1pUnset(a,i) j1Unset(a, (uintptr_t)(i))
#define j1pTest(a,i) j1Test(a, (uintptr_t)(i))
#define j1pCount(a,i1,i2) j1Count(a, (uintptr_t)(i1), (uintptr_t)(i2))
#define j1pFirst(a,pi) j1First(a, (uintptr_t)(pi))
#define j1pNext(a,pi) j1Next(a, (uintptr_t)(pi))
#define j1pPrev(a,pi) j1Prev(a, (uintptr_t)(pi))
#define j1pLast(a,pi) j1Last(a, (uintptr_t)(pi))
#define j1pFirstEmpty(a,pi) j1FirstEmpty(a, (uintptr_t)(pi))
#define j1pNextEmpty(a,pi) j1NextEmpty(a, (uintptr_t)(pi))
#define j1pPrevEmpty(a,pi) j1PrevEmpty(a, (uintptr_t)(pi))
#define j1pLastEmpty(a,pi) j1LastEmpty(a, (uintptr_t)(pi))

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
