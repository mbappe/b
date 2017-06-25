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

#ifdef  RAMMETRICS
Word_t    j__AllocWordsTOT;             // words in buffers given by malloc
Word_t    j__ExtraWordsTOT;             // extra words above previous estimate
                                        // of one or two or three
Word_t    j__ExtraWordsCnt;             // how many buffers have extra words
Word_t    j__MalFreeCnt;                // keep track of total malloc() + free()
Word_t    j__MFlag;                     // Print memory allocation on stderr
Word_t    j__TotalBytesAllocated;       // from kernel from dlmalloc
#endif  // RAMMETRICS

// Use -DLIBCMALLOC if you want to use the libc malloc() instead of this
// internal memory allocator.  (This one is much faster on some OS).

#ifndef  LIBCMALLOC

// only use the libc malloc of defined
#include <sys/mman.h>

//=======================================================================
// J U D Y  /  D L M A L L O C interface for huge pages Ubuntu 3.13+ kernel
//=======================================================================
//
#define PRINTMMAP(BUF, LENGTH)                                          \
   fprintf(stderr,                                                      \
        "%p:buf = mmap(addr:0x0, length:%p, prot:0x%x, flags:0x%x, fd:%d)\n", \
                (void *)(BUF), (void *)(LENGTH), prot, flags, fd)
   
#define PRINTMUMAP(BUF, LENGTH)                                         \
   fprintf(stderr,                                                      \
        "%d = munmap(buf:%p, length:%p[%d])\n",                         \
                ret, (void *)(BUF), (void *)(LENGTH), (int)(LENGTH));

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

#include "dlmalloc.c"   // Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea

#undef mmap             
#define mmap            mmap    // restore it for rest of routine
#undef munmap           
#define munmap          munmap  // restore it for rest of routine

// This code is not necessary except if j__MFlag is set
static int 
pre_munmap(void *buf, size_t length)
{
    int ret;

    ret =  munmap(buf, length);

#ifdef  RAMMETRICS
    j__TotalBytesAllocated -= length;

    if (j__MFlag)
        PRINTMUMAP(buf, length);
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
        fprintf(stderr, "\nSorry, JudyMalloc() is not ready for %d allocations\n", (int)length);
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

    if (buf == MFAIL)           // out of memory(RAM)
        return(buf);

//  if we get a mis-aligned buffer, change it to aligned
    if (((Word_t)buf % HUGETLBSZ) != 0) // if not aligned to 2MB
    {
        Word_t remain;

//      free the mis-aligned buffer
        ret = munmap(buf, length);          

#ifdef  RAMMETRICS
        if (j__MFlag)
            PRINTMUMAP(buf, length);
#endif  // RAMMETRICS

//      Allocate again big enough (4Mib) to insure getting a buffer big enough that
//      can be trimmed to  2MB alignment.
        buf = (char *)mmap(NULL, length + HUGETLBSZ, prot, flags, fd, offset);

#ifdef  RAMMETRICS
        if (j__MFlag)
            PRINTMMAP(buf, length + HUGETLBSZ);
#endif  // RAMMETRICS

        if (buf == MFAIL)               // sorry out of RAM
            return(buf);

        remain = (Word_t)buf % HUGETLBSZ;
        if (remain)
        {
            ret = munmap(buf, HUGETLBSZ - remain);          

#ifdef  RAMMETRICS
            if (j__MFlag)
                PRINTMUMAP(buf, HUGETLBSZ - remain);
#endif  // RAMMETRICS

        }
//      calc where memory is at end of buffer
        buf += HUGETLBSZ - remain;

//      and free it too.
        ret = munmap(buf + length, remain);

#ifdef  RAMMETRICS
        if (j__MFlag)
        {
            PRINTMUMAP(buf + length, remain);
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

#endif	// ! LIBCMALLOC


// ****************************************************************************
// J U D Y   M A L L O C
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JPM accounting occurs at a higher level.

RawP_t JudyMalloc(
	int Words)
{
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

#ifdef  LIBCMALLOC
	Addr = (Word_t) malloc(Bytes);
#else	// ! system libc
	Addr = (Word_t) dlmalloc(Bytes);
#endif	// ! LIBCMALLOC
#ifdef  RAMMETRICS
        if (Addr)
        {
            // get # bytes in malloc buffer from preamble
            size_t zAllocWords = (((Word_t *)Addr)[-1] & ~3) / sizeof(Word_t);
            size_t zMinWords = (Words + 2) & ~1;
            if (zMinWords < 4) { zMinWords = 4; }
            j__AllocWordsTOT += zAllocWords;
            j__ExtraWordsTOT += zAllocWords - zMinWords;
            if (zAllocWords != zMinWords)
                j__ExtraWordsCnt++;
        }
#endif  // RAMMETRICS

#ifdef  TRACEJM
        printf("%p = JudyMalloc(%u)\n", (void *)Addr, (int)Bytes / sizeof(Word_t));
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


// ****************************************************************************
// J U D Y   F R E E

void JudyFree(
	RawP_t PWord,
	int    Words)
{
	(void) Words;

#ifdef  RAMMETRICS
        // get # bytes in malloc buffer from preamble
        size_t zAllocWords = (((Word_t *)PWord)[-1] & ~3) / sizeof(Word_t);
        size_t zMinWords = (Words + 2) & ~1;
        if (zMinWords < 4) { zMinWords = 4; }
        j__AllocWordsTOT -= zAllocWords;
        j__ExtraWordsTOT -= zAllocWords - zMinWords;
        if (zAllocWords != zMinWords)
            j__ExtraWordsCnt--;

        j__MalFreeCnt++;        // keep track of total malloc() + free()
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

        if (~GuardWord != (Word_t)PWord)
        {
            printf("\n\nOops GuardWord = %p != PWord = %p\n", 
                    (void *)GuardWord, (void *)PWord);
            exit(-1);
        }
    }
#endif  // GUARDBAND

#ifdef  TRACEJM
        printf("%p   JudyFree(%u)\n", (void *)PWord, (int)Words);
#endif  // TRACEJM

#ifdef  LIBCMALLOC
	free((void *) PWord);
#else	// ! system lib
	dlfree((void *) PWord);
#endif	// Judy malloc


} // JudyFree()



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
