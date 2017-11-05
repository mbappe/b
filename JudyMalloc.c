//
// This program is free software; you can redistribute it and/or modify it
// under the same terms as dlmalloc.c -- the
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.

// ********************************************************************** //
//                    JUDY - Memory Allocater                             //
//                              -by-					  //
//		            Doug Baskins				  //
//                     dougbaskins -at- yahoo.com                         //
//									  //
// ********************************************************************** //

// ****************************************************************************
// J U D Y   M A L L O C
//
// Higher-level "wrapper" for allocating objects that need not be in RAM,
// although at this time they are in fact only in RAM.  Later we hope that some
// entire subtrees (at a JPM or branch) can be "virtual", so their allocations
// and frees should go through this level.
// ****************************************************************************

// JUDY INCLUDE FILES
#include "Judy.h"

#ifdef  TRACEJM
#define RAMMETRICS 1
#endif  // TRACEJM

// Global in case anyone wants to know (kind of kludgy, but only for testing)
Word_t    j__AllocWordsTOT;     // words given by JudyMalloc including overhead
Word_t    j__MalFreeCnt;                // keep track of total malloc() + free()
Word_t    j__MFlag;                     // Print memory allocation on stderr
Word_t    j__TotalBytesAllocated;       // mmapped by dlmalloc
Word_t    j__RequestedWordsTOT;       // words requested by Judy via JudyMalloc

// Globals used by JudyMallocIF.c.
// We define them here so they are not defined in both Judy1MallocIF.c and
// JudyLMallocIF.c.
Word_t    j__AllocWordsJBB;
Word_t    j__AllocWordsJBU;
Word_t    j__AllocWordsJBL;
Word_t    j__AllocWordsJLB1;
// Word_t    j__AllocWordsJLB2;
Word_t    j__AllocWordsJLL1;
Word_t    j__AllocWordsJLL2;
Word_t    j__AllocWordsJLL3;
Word_t    j__AllocWordsJLL4;
Word_t    j__AllocWordsJLL5;
Word_t    j__AllocWordsJLL6;
Word_t    j__AllocWordsJLL7;
Word_t    j__AllocWordsJLLW;
Word_t    j__AllocWordsJV;
Word_t    j__NumbJV;

// Use -DLIBCMALLOC if you want to use the libc malloc() instead of this
// internal memory allocator.  (This one is much faster on some OS).
#ifdef LIBCMALLOC

  // JUDY_MALLOC_ALIGNMENT is for internal use only.
  #ifndef JUDY_MALLOC_ALIGNMENT
    #define JUDY_MALLOC_ALIGNMENT  (2 * sizeof(size_t))
  #endif // JUDY_MALLOC_ALIGNMENT

#else // LIBCMALLOC

  // JUDY_MALLOC_ALIGNMENT is for internal use only.
  // It is derived from MALLOC_ALIGNMENT.
  #ifndef JUDY_MALLOC_ALIGNMENT
    #ifdef MALLOC_ALIGNMENT
      #define JUDY_MALLOC_ALIGNMENT  MALLOC_ALIGNMENT
    #else // MALLOC_ALIGNMENT
      #define JUDY_MALLOC_ALIGNMENT  (2 * sizeof(size_t))
    #endif // MALLOC_ALIGNMENT
  #endif // JUDY_MALLOC_ALIGNMENT

// only use the libc malloc of defined
#include <sys/mman.h>

//=======================================================================
// J U D Y  /  D L M A L L O C interface for huge pages Ubuntu 3.13+ kernel
//=======================================================================
//
#define PRINTMMAP(BUF, LENGTH)                                          \
   fprintf(stderr,                                                      \
        "%p:buf = mmap(addr:0x0, length:%p, prot:0x%x, flags:0x%x, fd:%d) line %d\n", \
                (void *)(BUF), (void *)(LENGTH), prot, flags, fd, __LINE__)

#define PRINTMUNMAP(BUF, LENGTH)                                         \
   fprintf(stderr,                                                      \
        "%d = munmap(buf:%p, length:%p[%d]) line %d\n",                         \
                ret, (void *)(BUF), (void *)(LENGTH), (int)(LENGTH), __LINE__)

// Define the Huge TLB size (2MiB) for Intel Haswell+
#ifndef HUGETLBSZ
#define HUGETLBSZ       ((Word_t)0x200000)
#endif  // HUGETLBSZ

static void * pre_mmap(void *, size_t, int, int, int, off_t);
static int    pre_munmap(void *, size_t);

// Stuff to modify dlmalloc to use 2MiB pages
#define DLMALLOC_EXPORT static
#define dlmalloc_usable_size static dlmalloc_usable_size
#define USE_DL_PREFIX
#define HAVE_MREMAP     0
#define DEFAULT_MMAP_THRESHOLD HUGETLBSZ
// normal default == 64 * 1024
#define DEFAULT_GRANULARITY HUGETLBSZ

// #define DARWIN  1
#define HAVE_MORECORE 0
#define HAVE_MMAP 1

#define mmap            pre_mmap        // re-define for dlmalloc
#define munmap          pre_munmap      // re-define for dlmalloc

#if JUDY_MALLOC_NUM_SPACES != 0
  // Include create/destroy_mspace and
  // mspace_malloc/memalign/free with MSPACES=1.
  #define MSPACES  1
  // Exclude regular malloc/free with ONLY_MSPACES=1.
#endif // JUDY_MALLOC_NUM_SPACES != 0

#ifdef DEBUG
  // Enable additional user error checking with FOOTERS=1.
  #define FOOTERS  1
  // Disable some user error checking with INSECURE=1.
#endif // DEBUG

#include "dlmalloc.c"   // Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea

#undef mmap // restore it for rest of routine
#undef munmap // restore it for rest of routine

#if JUDY_MALLOC_NUM_SPACES != 0

static mspace JudyMallocSpaces[JUDY_MALLOC_NUM_SPACES];

static inline void
JudyMallocPrepSpace(int nSpace)
{
    if (JudyMallocSpaces[nSpace] == 0) {
        JudyMallocSpaces[nSpace] = create_mspace(
            /* initial capacity */ 0, /* locked */ 0);
        assert(JudyMallocSpaces[nSpace] != NULL);
    }
}

#endif // JUDY_MALLOC_NUM_SPACES != 0

// This code is not necessary except if j__MFlag is set
static int
pre_munmap(void *buf, size_t length)
{
    int ret;

    ret =  munmap(buf, length);

#ifdef  RAMMETRICS
    j__TotalBytesAllocated -= length;

    if (j__MFlag)
        PRINTMUNMAP(buf, length);
#endif  // RAMMETRICS

    return(ret);
}

// ********************************************************************
// All this nonsence is because of a flaw in the Linux/Ubuntu Kernel.
// Any mmap equal or larger than 2MiB should be "HUGE TLB aligned" (dlb)
// ********************************************************************

static void *
pre_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    char *buf;
    int   ret;

    (void) addr;        // in case DEBUG is not defined
    (void) ret;         // in case RAMMETRICS is not defined

    assert(addr == NULL);

//  early out if buffer not 2MB
    if (length != HUGETLBSZ)
    {
////        return(buf);
        fprintf(stderr, "\nSorry, JudyMalloc() is not ready for %zd allocations\n", length);
        exit(-1);
    }

//  Map a normal self-aligned 2Mib buffer for dlmalloc pagesize
    buf = (char *)mmap(NULL, length, prot, flags, fd, offset);

#ifdef  RAMMETRICS
    if (j__MFlag)
    {
        fprintf(stderr, "\n");
        PRINTMMAP(buf, length);
    }
#endif  // RAMMETRICS

    if (buf == MAP_FAILED)           // out of memory(RAM)
        return(buf);

//  if we get a mis-aligned buffer, change it to aligned
    if (((Word_t)buf % HUGETLBSZ) != 0) // if not aligned to 2MB
    {
        Word_t remain;

//      free the mis-aligned buffer
        ret = munmap(buf, length);

#ifdef  RAMMETRICS
        if (j__MFlag)
            PRINTMUNMAP(buf, length);
#endif  // RAMMETRICS

//      Allocate again big enough (4Mib) to insure getting a buffer big enough that
//      can be trimmed to  2MB alignment.
        buf = (char *)mmap(NULL, length + HUGETLBSZ, prot, flags, fd, offset);

#ifdef  RAMMETRICS
        if (j__MFlag)
            PRINTMMAP(buf, length + HUGETLBSZ);
#endif  // RAMMETRICS

        if (buf == MAP_FAILED)               // sorry out of RAM
            return(buf);

        remain = (Word_t)buf % HUGETLBSZ;

        ret = munmap(buf, HUGETLBSZ - remain);     // free front to alignment

#ifdef  RAMMETRICS
        if (j__MFlag)
            PRINTMUNMAP(buf, HUGETLBSZ - remain);
#endif  // RAMMETRICS

//      calc where memory is at end of buffer to free
        buf += HUGETLBSZ - remain;

//      free it too -- only if 4MiB was unaligned
        if (remain) {
            ret = munmap(buf + length, remain);
        }

#ifdef  RAMMETRICS
        if (j__MFlag)
        {
            PRINTMUNMAP(buf + length, remain);
            fprintf(stderr, "%p == buf\n", (void *)buf);
        }
#endif  // RAMMETRICS

    }
//  DONE, have self-aligned buffer to return to dlmalloc

#ifdef  RAMMETRICS
    j__TotalBytesAllocated += length;
#endif  // RAMMETRICS

    return(buf);
}

#endif // LIBCMALLOC

// ****************************************************************************
// J U D Y   M A L L O C
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JPM accounting occurs at a higher level.

#ifdef LIBCMALLOC
static
#endif // LIBCMALLOC
RawP_t
JudyMallocX(int Words, int nSpace, int nLogAlign)
{
        (void)nSpace;
        size_t Addr;
        size_t Bytes;

        Bytes = Words * sizeof(size_t);

//  Note: This define is only for DEBUGGING
#ifdef  GUARDBAND
        if (Bytes == 0)
        {
            fprintf(stderr, "\nOops -- JudyMalloc(0) -- not legal for Judy");
            printf("\nOops -- JudyMalloc(0) -- not legal for Judy");
            exit(-1);
        }
//      Add one word to the size of malloc
        Bytes += sizeof(Word_t);    // one word
#endif  // GUARDBAND

        size_t zAlign = (size_t)1 << nLogAlign; (void)zAlign;
#ifdef  LIBCMALLOC
        if (zAlign > JUDY_MALLOC_ALIGNMENT) {
            if (posix_memalign((void*)&Addr, zAlign, Bytes) != 0) {
                Addr = (size_t)NULL;
            }
        } else {
            Addr = (Word_t) malloc(Bytes);
        }
#else   // ! system libc
  #if JUDY_MALLOC_NUM_SPACES != 0
        if ((nSpace >= 0) && (nSpace < JUDY_MALLOC_NUM_SPACES)) {
            JudyMallocPrepSpace(nSpace);
            if (zAlign > JUDY_MALLOC_ALIGNMENT) {
                Addr = (Word_t)mspace_memalign(JudyMallocSpaces[nSpace], zAlign, Bytes);
            } else {
                Addr = (Word_t)mspace_malloc(JudyMallocSpaces[nSpace], Bytes);
            }
        } else
  #endif // JUDY_MALLOC_NUM_SPACES != 0
        {
            if (zAlign > JUDY_MALLOC_ALIGNMENT) {
                Addr = (Word_t)dlmemalign(zAlign, Bytes);
            } else {
                Addr = (Word_t) dlmalloc(Bytes);
            }
        }
#endif  // ! LIBCMALLOC
#ifdef  RAMMETRICS
        if (Addr)
        {
            j__RequestedWordsTOT += Words;
            // get # bytes in malloc buffer from preamble
            size_t zAllocWords = (((Word_t *)Addr)[-1] & ~3) / sizeof(Word_t);
            j__AllocWordsTOT += zAllocWords;
        }
#endif  // RAMMETRICS

#ifdef  TRACEJM
        printf("%p = JudyMalloc(%zu)\n", (void *)Addr, Bytes / sizeof(Word_t));
#endif  // TRACEJM

#ifdef  GUARDBAND
//      Put the ~Addr in that extra word
        *((Word_t *)Addr + ((Bytes/sizeof(Word_t)) - 1)) = ~Addr;

//      Verify that all mallocs are 2 word aligned
        if (Addr & ((sizeof(Word_t) * 2) - 1))
        {
            fprintf(stderr, "\nmalloc() Addr not 2 word aligned = %p\n", (void *)Addr);
            printf("\nmalloc() Addr not 2 word aligned = %p\n", (void *)Addr);
            exit(-1);
        }
#endif  // GUARDBAND

#ifdef  RAMMETRICS
        if (Addr)
            j__MalFreeCnt++;            // keep track of total malloc() + free()
#endif  // RAMMETRICS

        return(Addr);

} // JudyMalloc()

RawP_t
JudyMallocAlign(int Words, int nLogAlign)
{
    return JudyMallocX(Words, /* nSpace */ -1, nLogAlign);
}

RawP_t
JudyMalloc(int Words)
{
    return JudyMallocX(Words, /* nSpace */ -1, /* nLogAlign */ 0);
}

// ****************************************************************************
// J U D Y   F R E E

#ifdef LIBCMALLOC
static
#endif // LIBCMALLOC
void
JudyFreeX(RawP_t PWord, int Words, int nSpace)
{
    (void) Words;
    (void)nSpace;

#ifdef  RAMMETRICS
    // get # bytes in malloc buffer from preamble
    size_t zAllocWords = (((Word_t *)PWord)[-1] & ~3) / sizeof(Word_t);
    j__AllocWordsTOT -= zAllocWords;

    j__MalFreeCnt++;        // keep track of total malloc() + free()
    j__RequestedWordsTOT -= Words;
#endif  // RAMMETRICS

#ifdef  GUARDBAND
    if (Words == 0)
    {
        fprintf(stderr, "--- OOps JudyFree called with 0 words\n");
        printf("--- OOps JudyFree called with 0 words\n");
        exit(-1);
    }
    {
        Word_t GuardWord;

//      Verify that the Word_t past the end is same as ~PWord freed
        GuardWord = *((((Word_t *)PWord) + Words));

        if (GuardWord != ~(Word_t)PWord)
        {
            printf("\n\nOops JF(PWord %p Words 0x%x)"
                   " GuardWord aka PWord[Words] 0x%zx != ~PWord 0x%zx\n",
                   (void *)PWord, Words, GuardWord, ~(Word_t)PWord);
            exit(-1);
        }
    }
#endif  // GUARDBAND

#ifdef  TRACEJM
        printf("%p   JudyFree(%u)\n", (void *)PWord, (int)Words);
#endif  // TRACEJM

#ifdef  LIBCMALLOC
        free((void *) PWord);
#else   // ! system lib
  #if JUDY_MALLOC_NUM_SPACES != 0
        if ((nSpace >= 0) && (nSpace < JUDY_MALLOC_NUM_SPACES)) {
            // JudyMallocPrepSpace(nSpace);
            mspace_free(JudyMallocSpaces[nSpace], (void*)PWord);
        } else
  #endif // JUDY_MALLOC_NUM_SPACES != 0
            dlfree((void *) PWord);
#endif  // Judy malloc

} // JudyFree()

void
JudyFree(RawP_t PWord, int Words)
{
    (void) Words;
    JudyFreeX(PWord, Words, /* nSpace */ -1);
}


RawP_t JudyMallocVirtual(
	int Words)
{
	return(JudyMalloc(Words));

} // JudyMallocVirtual()


// ****************************************************************************
// J U D Y   F R E E

void JudyFreeVirtual(
	RawP_t PWord,
	int    Words)
{
        JudyFree(PWord, Words);

} // JudyFreeVirtual()

#ifndef LIBCMALLOC

// Returns 1 if it actually returned any memory; 0 otherwise.
int
JudyMallocTrim(int nSpace)
{
    (void)nSpace;
  #if JUDY_MALLOC_NUM_SPACES != 0
    if ((nSpace >= 0) && (nSpace < JUDY_MALLOC_NUM_SPACES)) {
        return mspace_trim(JudyMallocSpaces[nSpace], /* pad */ 0);
    }
  #endif // JUDY_MALLOC_NUM_SPACES != 0
    return dlmalloc_trim(/* pad */ 0);
}

#if JUDY_MALLOC_NUM_SPACES != 0
  #define JUDY_MALLOC_INFO(nSpace, field) \
    ((nSpace >= 0) && (nSpace < JUDY_MALLOC_NUM_SPACES)) \
        ? JudyMallocPrepSpace(nSpace), \
            mspace_mallinfo(JudyMallocSpaces[nSpace]).field \
        : dlmallinfo().field;
#else // JUDY_MALLOC_NUM_SPACES != 0
  #define JUDY_MALLOC_INFO(nSpace, field)  dlmallinfo().field
#endif // JUDY_MALLOC_NUM_SPACES != 0

// non-mmapped space allocated from system
size_t
JudyMallocInfoNonMmapped(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, arena);
}

// mmapped space allocated from system
size_t
JudyMallocInfoMmapped(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, hblkhd);
}

// releasable space (via JudyMallocTrim)
size_t
JudyMallocInfoReleasable(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, keepcost);
}

// total allocated space
size_t
JudyMallocInfoAllocated(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, uordblks);
}

// total free space
size_t
JudyMallocInfoFree(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, fordblks);
}

// number of free chunks
size_t
JudyMallocInfoFreeChunks(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, ordblks);
}

// maximum total allocated space
size_t
JudyMallocInfoMaxAllocated(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, usmblks);
}

#endif // LIBCMALLOC
