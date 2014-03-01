
###########################
#
# Makefile for b.  Code for learning about Judy and possible improvements.
#
###########################

###########################
#
# How do I do things in here such that the Makefile provides a default
# that I can override on the command line?
#
###########################

CC = cc
#CC = icc
#CC = clang
#CC = gcc

##################################
#
# Judy1LHTime.c doesn't compile with -std=c99.  It complains about some
# time.h symbol.  time.h is not included with -std=c99?
#
##################################

#STDFLAG = -std=c99
#STDFLAG = -std=c90
#STDFLAG = -std=c89
#STDFLAG =

#MFLAG = -m64
MFLAG = -m32

#WFLAGS = -Wall -pedantic -Wstrict-prototypes -Wmissing-prototypes -W -Werror
WFLAGS = -Wall -Werror

OFLAGS = -g -O2

CFLAGS = $(STDFLAG) $(MFLAG) $(WFLAGS) $(OFLAGS) -I.

JUDY_DEFINES += -DJUDYA -DGUARDBAND -UNDEBUG
B_DEFINES += -DRAM_METRICS -DSEARCH_METRICS
B_DEFINES += -DSKIP_LINKS -DSKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX
B_DEFINES += -DSORT_LISTS -UMIN_MAX_LISTS
B_DEFINES += -URECURSIVE_INSERT -URECURSIVE_REMOVE
B_DEFINES += -ULOOKUP_NO_LIST_DEREF -ULOOKUP_NO_LIST_SEARCH
B_DEFINES += -ULOOKUP_NO_BITMAP_DEREF -ULOOKUP_NO_BITMAP_SEARCH
# -DDEBUG adds some internal sanity checking not covered by assertions only.
# It does not log anything unless something wrong is detected.
B_DEFINES += -DDEBUG
#B_DEBUG_DEFINES += -UDEBUG_INSERT -UDEBUG_LOOKUP -UDEBUG_MALLOC -UDEBUG_REMOVE
DEFINES += $(JUDY_DEFINES) $(B_DEFINES) $(B_DEBUG_DEFINES)

LIBS = -lm

FILES_FROM_ME = b.h t.c b.c bli.c bl.c bi.c br.c stubs.c Makefile
FILES_FROM_DOUG_OR_DOUG = Judy.h RandomNumb.h Judy1LHTime.c dlmalloc.c
FILES = $(FILES_FROM_ME) $(FILES_FROM_DOUG_OR_DOUG)

EXES = b bx t
OBJS = b.o bl.o bi.o br.o stubs.o Judy1LHTime.o dlmalloc.o
ASMS = b.s bl.s bi.s br.s stubs.s Judy1LHTime.s dlmalloc.s t.s
CPPS = b.i bl.i bi.i br.i stubs.i Judy1LHTime.i dlmalloc.i

##
# Bx is an attempt to see if skipping intermediate .o file creation
# can result in more inlining or otherwise better code.
# Can we make Judy1LHTime -1 run as fast as Judy1LHTime -b?
##
BX_SRCS = Judy1LHTime.c b.c bl.c bi.c br.c
BX_OBJS = stubs.o dlmalloc.o

BY_SRCS = Judy1LHTime.c b.c bl.c bi.c br.c
BX_OBJS = stubs.o dlmalloc.o

T_SRCS = t.c
T_OBJS = stubs.o dlmalloc.o

##################################
#
# Start "all" with "clean" for two reasons:
#
# 1. This Makefile does not distinguish 64-bit from 32-bit objects.  When
#    switching from one to the other it is necessary to rebuild everything.
# 2. I couldn't figure out how to add the .h file dependencies without
#    breaking the .c.o rule -- I tried creating a separate rule but that
#    didn't work.
#
##################################

all:	clean $(EXES) $(ASMS) $(CPPS) b.tar

clean:
	rm -f $(EXES) *.tar *.o *.s *.i
	rm -rf *.dSYM

b:	$(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

bx:	$(BX_SRCS) $(BX_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

t:	$(T_SRCS) $(T_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

b.tar:	$(FILES)
	tar cf $@ $(FILES)

############################
#
# Rules for building .o files.
#
# I can't figure out how to add dependencies for the .o files on b.h
# for the .c.o rule without ending up with b.h on the compile line
# coming from $^.  And b.h on the command line causes a problem for icc
# and possibly other compilers.
# So we are going without the .h file dependencies for now which is
# part of the reason the "all" target starts with "clean".
#
############################

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $^

# Suppress warnings.
Judy1LHTime.o: Judy1LHTime.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.
stubs.o: stubs.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.
dlmalloc.o: dlmalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

############################
#
# Rules for building .s files.
#
############################

.c.s:
	$(CC) $(CFLAGS) $(DEFINES) -S $^

# Suppress warnings.
dlmalloc.s: dlmalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

############################
#
# Rules for building .i files.
#
############################

.c.i:
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $^

# The .c.i rule doesn't work for some reason.  Later.
bl.i: bl.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
bi.i: bi.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
br.i: br.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
b.i: b.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
stubs.i: stubs.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
Judy1LHTime.i: Judy1LHTime.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
dlmalloc.i: dlmalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

