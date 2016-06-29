//
// This program is free software; you can redistribute it and/or modify it
// under the same terms as dlmalloc.c -- the 
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.

// @(#) $Revision: 1.6 $ $Source: /home/doug/judy-1.0.5_PSplit_goto_newLeaf3/src/JudyCommon/RCS/JudyMalloc.c,v $
//
// ********************************************************************** //
//                    JUDY - Memory Allocater                             //
//                              -by-					  //
//		         Douglas L. Baskins				  //
//                     dougbaskins -at- yahoo.com                         //
//									  //
// ********************************************************************** //

//=======================================================================
// J U D Y  /  D L M A L L O C interface for huge pages Ubuntu 3.13+ kernel
//=======================================================================

// -DLIBCMALLOC if you want to use libc malloc()
#ifdef  LIBCMALLOC

// JUDY INCLUDE FILES
#include "Judy.h"

// ****************************************************************************
// J U D Y   M A L L O C
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JPM accounting occurs at a higher level.

Word_t JudyMalloc(
	Word_t Words)
{
	Word_t Addr;

	Addr = (Word_t) malloc(Words * sizeof(Word_t));
	return(Addr);

} // JudyMalloc()


// ****************************************************************************
// J U D Y   F R E E

void JudyFree(
	void * PWord,
	Word_t Words)
{
	(void) Words;
	free(PWord);

} // JudyFree()


// ****************************************************************************
// J U D Y   M A L L O C
//
// Higher-level "wrapper" for allocating objects that need not be in RAM,
// although at this time they are in fact only in RAM.  Later we hope that some
// entire subtrees (at a JPM or branch) can be "virtual", so their allocations
// and frees should go through this level.

Word_t JudyMallocVirtual(
	Word_t Words)
{
	return(JudyMalloc(Words));

} // JudyMallocVirtual()


// ****************************************************************************
// J U D Y   F R E E

void JudyFreeVirtual(
	void * PWord,
	Word_t Words)
{
        JudyFree(PWord, Words);

} // JudyFreeVirtual()

#else   // LIBCMALLOC

#include <sys/mman.h>
#include "Judy.h"

// Define the Huge TLB size (2MB) for Intel Haswell
#ifndef HUGETLBSZ       
#define HUGETLBSZ       ((Word_t)0x200000)
#endif  // HUGETLBSZ

static void * pre_mmap(void *, size_t, int, int, int, off_t);
static int pre_munmap(void *, size_t);

// Stuff to modify dlmalloc to use 2MB pages
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

#define mmap            pre_mmap 
#define munmap          pre_munmap

#include "dlmalloc.c"   // Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea

#undef mmap             // undo it
#define mmap            mmap 
#undef munmap           // undo it
#define munmap          munmap

#ifdef  RAMMETRICS
Word_t    j__MFlag = 0;                    // Print memory allocation on stderr
Word_t    j__TotalBytesAllocated = 0;
#endif  // RAMMETRICS

// This code is not necessary except if MFlag is set
static int 
pre_munmap(void *addr, size_t length)
{
    int ret;

    ret =  munmap(addr, length);

#ifdef  RAMMETRICS
    j__TotalBytesAllocated -= length;
    if (j__MFlag)
        fprintf(stderr, "%d = munmap(0x%lx, 0x%lx(%ld)\n", ret, (Word_t)addr, (Word_t)length, (Word_t)length);
#endif  // RAMMETRICS

    return(ret);
}

// ********************************************************************
// All this nonsence is because of a flaw in the Linux/Ubuntu Kernel.
// Any mmap equal or larger than 2MB should be "HUGE TLB aligned" (dlb)
// ********************************************************************

static void *
pre_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    char *buf;
    Word_t remain, fronttrim;
    (void)addr;

    assert(addr == 0);

    buf = mmap(NULL, length, prot, flags, fd, offset);

#ifdef  RAMMETRICS
    j__TotalBytesAllocated += length;
    if (j__MFlag)
        fprintf(stderr, "0x%lx = mmap(0x%lx)[%ld]\n", (Word_t)buf, \
                (Word_t)length, (Word_t)length);
#endif  // RAMMETRICS

    if (buf == MFAIL)
        return(buf);

//  early out if buffer not 2MB
    if (length != HUGETLBSZ)
        return(buf);

    remain = (Word_t)buf % HUGETLBSZ;

    if (remain)            // if not aligned to 2MB
    {
//      free the mis-aligned buffer
        pre_munmap(buf, length);          

//      Allocate again big enough to insure getting a buffer big enough that
//      can be trimmed to  2MB alignment.
        buf = mmap(NULL, length + HUGETLBSZ, prot, flags, fd, offset);

#ifdef  RAMMETRICS
        j__TotalBytesAllocated += (long)length + HUGETLBSZ;
        if (j__MFlag)
            fprintf(stderr, "0x%lx = mmap(0x%lx)[%ld]\n", (long)buf, \
                    (long)length + HUGETLBSZ, (long)length + HUGETLBSZ);
#endif  // RAMMETRICS

        if (buf == MFAIL)
            return(buf);

//      Now trim off mis-alignment bytes in beginning of buf
        fronttrim = (long)HUGETLBSZ - remain;

// fprintf(stderr, "fronttrim = 0x%lx, new buf = 0x%lx\n", fronttrim, (Word_t)buf + fronttrim);

        pre_munmap(buf, fronttrim);          
        buf += fronttrim;                       // produce aligned buffer

        pre_munmap(buf + (Word_t)length, remain);          
    }
    return(buf);
}

// JUDY INCLUDE FILES
//#include "Judy.h"

Word_t j__AllocWordsTOT;

// ****************************************************************************
// J U D Y   M A L L O C
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JPM accounting occurs at a higher level.

Word_t JudyMalloc(
	Word_t Words)
{
	Word_t Addr;

        if (Words < 4) {
            j__AllocWordsTOT += 4;
        } else {
            j__AllocWordsTOT += Words + 1;
            if ( (Words & 1) == 0 ) {
                j__AllocWordsTOT += 1;
            }
        }
	Addr = (Word_t) dlmalloc(Words * sizeof(Word_t));
	return(Addr);

} // JudyMalloc()


// ****************************************************************************
// J U D Y   F R E E

void JudyFree(
	void * PWord,
	Word_t Words)
{
	(void) Words;
        if (Words < 4) {
            j__AllocWordsTOT -= 4;
        } else {
            j__AllocWordsTOT -= Words + 1;
            if ( (Words & 1) == 0 ) {
                j__AllocWordsTOT -= 1;
            }
        }
	dlfree(PWord);

} // JudyFree()


// ****************************************************************************
// J U D Y   M A L L O C
//
// Higher-level "wrapper" for allocating objects that need not be in RAM,
// although at this time they are in fact only in RAM.  Later we hope that some
// entire subtrees (at a JPM or branch) can be "virtual", so their allocations
// and frees should go through this level.

Word_t JudyMallocVirtual(
	Word_t Words)
{
	return(JudyMalloc(Words));

} // JudyMallocVirtual()


// ****************************************************************************
// J U D Y   F R E E

void JudyFreeVirtual(
	void * PWord,
	Word_t Words)
{
        JudyFree(PWord, Words);

} // JudyFreeVirtual()

#endif  // LIBCMALLOC
