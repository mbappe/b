
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

CC = clang
#CC = cc
#CC = icc
#CC = gcc

##################################
#
# Judy1LHTime.c doesn't compile with -std=c99.  It complains about some
# time.h symbol.  time.h is not included with -std=c99?
#
##################################

# Use -std=gnu99 for CLOCK_MONOTONIC which is not available with -std=c99. 
# We use CLOCK_MONOTONIC on GNU/Linux, but not on Mac.  So we can use
# -std=c99 on Mac, but not on GNU/Linux.
STDFLAG = -std=gnu99
#STDFLAG =
#STDFLAG = -std=c99
#STDFLAG = -std=c90
#STDFLAG = -std=c89

#MFLAG = -m64
MFLAG = -m32

# Leave off -Wmissing-prototypes because I don't like providing prototypes
# that have no value at all, i.e. when the function definition appears
# before any use of the function.
WFLAGS = -Wall -pedantic -Wstrict-prototypes -W -Werror
#WFLAGS = -Wall -pedantic -Wstrict-prototypes -Wmissing-prototypes -W -Werror
#WFLAGS = -Wall -Werror

# I have seen -O4 inline BitmapGet from bitmap.o into Judy1LHTime.o.
# I don't know if it is possible to get -O4 without -O3.  I'm hoping
# -O4 does not imply -Ofast which admits to violating the standard.
# OFLAGS = -g -O4
OFLAGS = -g -O2

CFLAGS = $(STDFLAG) $(MFLAG) $(WFLAGS) $(OFLAGS) -I.

TIME_DEFINES += -UBITMAP_P_JE -UEXTERN_BITMAP -UINTERN_JUDY1 -UEXTERN_JUDY1
# -DBITMAP_BY_BYTE enables byte address/mask in judy1x.c and bitmapx.c.
TIME_DEFINES += -USWAP -UBITMAP_BY_BYTE
TIME_DEFINES += -DJUDYB -DGUARDBAND -UNDEBUG
B_DEFINES += -DRAM_METRICS -DSEARCH_METRICS
B_DEFINES += -DSKIP_LINKS -DSKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX
B_DEFINES += -DSORT_LISTS -UMIN_MAX_LISTS
B_DEFINES += -URECURSIVE_INSERT -URECURSIVE_REMOVE
B_DEFINES += -ULOOKUP_NO_LIST_DEREF -ULOOKUP_NO_LIST_SEARCH
B_DEFINES += -ULOOKUP_NO_BITMAP_DEREF -ULOOKUP_NO_BITMAP_SEARCH
# -DDEBUG adds some internal sanity checking not covered by assertions only.
# It does not log anything unless something wrong is detected.
B_DEFINES += -DDEBUG
##
# Here are other defines that can be specified on the command line with
# "DEFINES = ... make"
#
# -UDEBUG_INSERT -UDEBUG_LOOKUP -UDEBUG_MALLOC -UDEBUG_REMOVE
##
DEFINES += $(JUDY_DEFINES) $(TIME_DEFINES) $(B_DEFINES) $(B_DEBUG_DEFINES)

LIBS = -lm

FILES_FROM_ME = b.h b.c bli.c bl.c bi.c br.c t.c stubs.c Makefile
FILES_FROM_ME += bitmap.c bitmapx.c judy1.c judy1x.c
# I periodically make changes to the files provided by Doug.
FILES_FROM_DOUG_OR_DOUG = Judy.h RandomNumb.h Judy1LHTime.c dlmalloc.c
FILES = $(FILES_FROM_ME) $(FILES_FROM_DOUG_OR_DOUG)

EXES = t b bxc
OBJS = Judy1LHTime.o judy1.o bitmap.o bl.o bi.o br.o b.o stubs.o dlmalloc.o
ASMS = Judy1LHTime.s judy1.s bitmap.s bl.s bi.s br.s b.s stubs.s dlmalloc.s t.s
CPPS = Judy1LHTime.i judy1.i bitmap.i bl.i bi.i br.i b.i stubs.i dlmalloc.i t.i
SYMS = t.dSYM bxc.dSYM

##
# Bx is an attempt to see if skipping intermediate .o file creation
# can result in more inlining or otherwise better code.
# Can we make Judy1LHTime -1 run as fast as Judy1LHTime -b?
##
BXC_SRCS = Judy1LHTime.c judy1.c bitmap.c bl.c bi.c br.c b.c
BXC_OBJS = stubs.o dlmalloc.o

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
	rm -f $(EXES) *.tar $(OBJS) $(ASMS) $(CPPS)
	rm -rf $(SYMS)

t:	$(T_SRCS) $(T_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -w -o $@ $^ $(LIBS)

b:	$(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

bxc:	$(BXC_SRCS) $(BXC_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -w -o $@ $^ $(LIBS)

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

# Suppress warnings.  Empty translation unit for some ifdef combinations.
judy1.o: judy1.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.  Empty translation unit for some ifdef combinations.
bitmap.o: bitmap.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.
Judy1LHTime.o: Judy1LHTime.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.  Unused parameters.
stubs.o: stubs.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

# Suppress warnings.  sbrk is deprecated.
dlmalloc.o: dlmalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -w -c $^

############################
#
# Rules for building .s files.
#
############################

.c.s:
	$(CC) $(CFLAGS) $(DEFINES) -S $^

# Suppress warnings.  Transitive warnings.  t.c just includes other files.
t.s: t.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

# Suppress warnings.  Empty translation unit for some ifdef combinations.
judy1.s: judy1.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

# Suppress warnings.  Empty translation unit for some ifdef combinations.
bitmap.s: bitmap.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

# Suppress warnings.
Judy1LHTime.s: Judy1LHTime.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

# Suppress warnings.  Unused parameters.
stubs.s: stubs.c
	$(CC) $(CFLAGS) $(DEFINES) -w -S $^

# Suppress warnings.  sbrk is deprecated.
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
judy1.i: bitmap.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
bitmap.i: bitmap.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

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
t.i: t.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
dlmalloc.i: dlmalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

