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

#ifdef RAMMETRICS
// Global in case anyone wants to know (kind of kludgy, but only for testing)
Word_t    j__AllocWordsTOT;     // words given by JudyMalloc including overhead
Word_t    j__MalFreeCnt;                // keep track of total malloc() + free()
Word_t    j__MFlag;                     // Print memory allocation on stderr
Word_t    j__MmapWordsTOT;       // mmapped by dlmalloc
Word_t    j__RequestedWordsTOT;       // words requested by Judy via JudyMalloc

// Globals used by JudyMallocIF.c.
// We define them here so they are not defined in both Judy1MallocIF.c and
// JudyLMallocIF.c.
Word_t    j__AllocWordsJBB;
Word_t    j__AllocWordsJBU;
Word_t    j__AllocWordsJBL;
Word_t    j__AllocWordsJLB1;
Word_t    j__AllocWordsJLL[8];
Word_t    j__AllocWordsJV; // j__AllocWordsJLB2 for JUDY1 for MIKEY_1
Word_t    j__NumbJV;
#endif // RAMMETRICS

#ifdef DSMETRICS_HITS
  #ifdef DSMETRICS_NHITS
    #error DSMETRICS_HITS and DSMETRICS_NHITS are mutually exclusive.
  #endif // DSMETRICS_NHITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_HITS

#ifdef DSMETRICS_NHITS
  #undef  DSMETRICS_GETS
  #define DSMETRICS_GETS
#endif // DSMETRICS_NHITS

#ifdef DSMETRICS_GETS
  #undef  SEARCHMETRICS
  #define SEARCHMETRICS
#endif // DSMETRICS_GETS

#ifdef SEARCHMETRICS
Word_t j__GetCalls;
Word_t j__GetCallsNot;
Word_t j__GetCallsSansPop;
Word_t j__SearchPopulation;
Word_t j__DirectHits;
Word_t j__NotDirectHits;
Word_t j__GetCallsP;
Word_t j__GetCallsM;
Word_t j__MisComparesP;
Word_t j__MisComparesM;
#endif // SEARCHMETRICS

// JUDY_MALLOC_ALIGNMENT is for internal use only.
#ifndef JUDY_MALLOC_ALIGNMENT
  #ifdef MALLOC_ALIGNMENT
    #define JUDY_MALLOC_ALIGNMENT  MALLOC_ALIGNMENT
  #else // MALLOC_ALIGNMENT
    #define JUDY_MALLOC_ALIGNMENT  (2 * sizeof(Word_t))
  #endif // MALLOC_ALIGNMENT
#endif // !JUDY_MALLOC_ALIGNMENT

// Use -DLIBCMALLOC if you want to use the libc malloc() instead of this
// internal memory allocator.  (This one is much faster on some OS).
#ifndef LIBCMALLOC

//===============
// dlmalloc stuff
//===============

#define USE_DL_PREFIX
#define HAVE_MORECORE 0 // do not use sbrk
#define HAVE_MMAP 1 // use mmap

#define USE_LOCKS 0 // no need for thread-safety

#ifdef DEBUG
  // Enable additional user error checking with FOOTERS=1.
  #define FOOTERS  1
  // Disable some user error checking with INSECURE=1.
#endif // DEBUG

// Default is DLMALLOC_2MiB_PAGES.
// Use -DNO_DLMALLOC_2MiB_PAGES if you don't want to use this
// dlmalloc configuration stuff for Haswell huge pages.
#ifndef   NO_DLMALLOC_2MiB_PAGES
  #undef     DLMALLOC_2MiB_PAGES
  #define    DLMALLOC_2MiB_PAGES
#endif // NO_DLMALLOC_2MiB_PAGES

#ifdef DLMALLOC_2MiB_PAGES
//=======================================================================
// J U D Y  /  D L M A L L O C interface for huge pages Ubuntu 3.13+ kernel
//=======================================================================
//

#include <sys/mman.h>

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

static void * pre_mmap(void *, Word_t, int, int, int, off_t);
static int    pre_munmap(void *, Word_t);

// Stuff to modify dlmalloc to use 2MiB pages
#define DLMALLOC_EXPORT static
#define dlmalloc_usable_size static dlmalloc_usable_size
#define HAVE_MREMAP     0
#define DEFAULT_MMAP_THRESHOLD HUGETLBSZ
// normal default == 64 * 1024
#define DEFAULT_GRANULARITY HUGETLBSZ

#define mmap            pre_mmap        // re-define for dlmalloc
#define munmap          pre_munmap      // re-define for dlmalloc

#endif // DLMALLOC_2MiB_PAGES

#if JUDY_MALLOC_NUM_SPACES != 0
  // Include create/destroy_mspace and
  // mspace_malloc/memalign/free with MSPACES=1.
  #define MSPACES  1
  // Exclude regular malloc/free with ONLY_MSPACES=1.
#endif // JUDY_MALLOC_NUM_SPACES != 0

#include "dlmalloc.c"   // Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea

#ifdef DLMALLOC_2MiB_PAGES

#undef mmap // restore it for rest of routine
#undef munmap // restore it for rest of routine

// This code is not necessary except if j__MFlag is set
static int
pre_munmap(void *buf, Word_t length)
{
    int ret;

    ret =  munmap(buf, length);

#ifdef  RAMMETRICS
    j__MmapWordsTOT -= length / sizeof(Word_t);

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
pre_mmap(void *addr, Word_t length, int prot, int flags, int fd, off_t offset)
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
    j__MmapWordsTOT += length / sizeof(Word_t);
#endif  // RAMMETRICS

    return(buf);
}

#endif // DLMALLOC_2MiB_PAGES

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

#endif // !LIBCMALLOC

#ifdef RAMMETRICS
static Word_t
AllocWords(Word_t *pw, int nWords)
{
    (void)pw; (void)nWords;
  #ifdef EXCLUDE_MALLOC_OVERHEAD
    return nWords;
  #else // EXCLUDE_MALLOC_OVERHEAD
      #if !defined(LIBCMALLOC) || defined(__linux__)
    return (pw[-1] & 0xfffff8) / sizeof(Word_t); // dlmalloc head word
      #else // !defined(LIBCMALLOC) || defined(__linux__)
    return nWords;
      #endif //#else !defined(LIBCMALLOC) || defined(__linux__)
  #endif // EXCLUDE_MALLOC_OVERHEAD
}
#endif // RAMMETRICS

// ****************************************************************************
// J U D Y   M A L L O C
//
// Allocate RAM.  This is the single location in Judy code that calls
// malloc(3C).  Note:  JPM accounting occurs at a higher level.

#ifdef LIBCMALLOC
static
#endif // LIBCMALLOC
Word_t
JudyMallocX(int Words, int nSpace, int nLogAlign)
{
        (void)nSpace;
        Word_t Addr;
        Word_t Bytes;

        Bytes = Words * sizeof(Word_t);

//  Note: This define is only for DEBUGGING
#ifdef  GUARDBAND
        if (Bytes == 0)
        {
            fprintf(stderr, "\nOops -- JudyMalloc(0) -- not legal for Judy");
            printf("\nOops -- JudyMalloc(0) -- not legal for Judy");
            exit(-1);
        }
//      Add one word to the size of malloc
  #if cnGuardWords > 1
        int BytesBefore = Bytes;
        Bytes += cnGuardWords * sizeof(Word_t);    // one word
  #else // cnGuardWords > 1
        Bytes += sizeof(Word_t);    // one word
  #endif // #else cnGuardWords > 1
#endif  // GUARDBAND

        Word_t zAlign = (Word_t)1 << nLogAlign; (void)zAlign;
#ifdef  LIBCMALLOC
        if (zAlign > JUDY_MALLOC_ALIGNMENT) {
            if (posix_memalign((void*)&Addr, zAlign, Bytes) != 0) {
                Addr = (Word_t)NULL;
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
            j__AllocWordsTOT += AllocWords((void*)Addr, Words);
        }
#endif  // RAMMETRICS

#ifdef  TRACEJM
        printf("%p = JudyMalloc(%zu)\n", (void *)Addr, Bytes / sizeof(Word_t));
#endif  // TRACEJM

#ifdef  GUARDBAND
//      Put the ~Addr in that extra word
        *((Word_t *)Addr + ((Bytes/sizeof(Word_t)) - 1)) = ~Addr;
  #if cnGuardWords > 1
        *((Word_t *)Addr + ((BytesBefore/sizeof(Word_t)))) = ~Addr;
  #endif // cnGuardWords > 1

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

Word_t
JudyMallocAlign(int Words, int nLogAlign)
{
    return JudyMallocX(Words, /* nSpace */ -1, nLogAlign);
}

// Use -DJUDY_105 to build this JudyMalloc.c in a Judy 1.0.5 build area.
#ifdef JUDY_105
Word_t JudyMalloc(Word_t Words)
#else // JUDY_105
Word_t JudyMalloc(int Words)
#endif // #else JUDY_105
{
    return JudyMallocX(Words, /* nSpace */ -1, /* nLogAlign */ 0);
}

// ****************************************************************************
// J U D Y   F R E E

#ifdef LIBCMALLOC
static
#endif // LIBCMALLOC
void
JudyFreeX(Word_t PWord, int Words, int nSpace)
{
    (void) Words;
    (void)nSpace;

#ifdef  RAMMETRICS
    j__MalFreeCnt++;        // keep track of total malloc() + free()
    j__AllocWordsTOT -= AllocWords((void*)PWord, Words);
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
                   " GuardWord aka PWord[Words] 0x%zx != ~PWord 0x%zx"
                   " &PWord[Words] %p\n",
                   (void *)PWord, Words, GuardWord, ~PWord,
                   (((Word_t*)PWord) + Words));
            exit(-1);
        }

  #if cnGuardWords > 1
        GuardWord = *((((Word_t *)PWord) + Words + cnGuardWords - 1));
        if (GuardWord != ~(Word_t)PWord)
        {
            printf("\n\nOops JF(PWord %p Words 0x%x)"
                   " GuardWord aka PWord[Words + cnGuardWords - 1] 0x%zx"
                   " != ~PWord 0x%zx\n",
                   (void *)PWord, Words, GuardWord, ~(Word_t)PWord);
            exit(-1);
        }
  #endif // cnGuardWords > 1
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

#ifdef JUDY_105
void JudyFree(Pvoid_t PWord, Word_t Words)
#else // JUDY_105
void JudyFree(Word_t PWord, int Words)
#endif // #else JUDY_105
{
    (void) Words;
    JudyFreeX(PWord, Words, /* nSpace */ -1);
}


#ifdef JUDY_105
Word_t JudyMallocVirtual(Word_t Words)
#else // JUDY_105
Word_t JudyMallocVirtual(int Words)
#endif // #else JUDY_105
{
	return(JudyMalloc(Words));

} // JudyMallocVirtual()


// ****************************************************************************
// J U D Y   F R E E

#ifdef JUDY_105
void JudyFreeVirtual(Pvoid_t PWord, Word_t Words)
#else // JUDY_105
void JudyFreeVirtual(Word_t PWord, int Words)
#endif // #else JUDY_105
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
Word_t
JudyMallocInfoNonMmapped(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, arena);
}

// mmapped space allocated from system
Word_t
JudyMallocInfoMmapped(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, hblkhd);
}

// releasable space (via JudyMallocTrim)
Word_t
JudyMallocInfoReleasable(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, keepcost);
}

// total allocated space
Word_t
JudyMallocInfoAllocated(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, uordblks);
}

// total free space
Word_t
JudyMallocInfoFree(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, fordblks);
}

// number of free chunks
Word_t
JudyMallocInfoFreeChunks(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, ordblks);
}

// maximum total allocated space
Word_t
JudyMallocInfoMaxAllocated(int nSpace)
{
    (void)nSpace;
    return JUDY_MALLOC_INFO(nSpace, usmblks);
}

#endif // LIBCMALLOC
