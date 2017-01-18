
###########################
#
# Makefile for b. Code for learning about Judy and possible improvements.
#
###########################

# Run the following command to build b:
#   [CC=<cc|clang|gcc|icc>] [cnBitsPerWord=<32|64>] [DEFINES="..."] make
# Run the following command to build everything:
#   [CC=<cc|clang|gcc|icc>] [cnBitsPerWord=<32|64>] [DEFINES="..."] make all

# The default build is 64-bits.
# Use "cnBitsPerWord=32 make" to get a 32-bit build.
ifeq "$(cnBitsPerWord)" ""
cnBitsPerWord = 64
endif

# The default compiler is clang.
# Use "CC=gcc make" to use gcc.
# CC=cc means clang. CC=ccc means clang.
ifeq "$(CC)" "cc"
  CC = clang
endif
ifeq "$(CC)" "ccc"
  CC = clang
endif
# CC = cc
# CC = clang
# CC = ccc
# CC = gcc
# CC = icc

##
# -std=gnu11 and -std=gnu99 give CLOCK_MONOTONIC which is not available with
# -std=c11 or -std=c99.
# We use CLOCK_MONOTONIC on GNU/Linux, but not on Mac.
# -std=c11 gives anonymous unions with no complaints.
# -std=gnu99 gives a warning for anonymous unions.
##
# STDFLAG =
  STDFLAG = -std=gnu11
# STDFLAG = -std=c11
# STDFLAG = -std=gnu99
# STDFLAG = -std=c99
# STDFLAG = -std=c90
# STDFLAG = -std=c89

  MFLAGS += -m$(cnBitsPerWord)
# MFLAGS += -mmmx
# MFLAGS += -mno-mmx
# MFLAGS += -msse2
# MFLAGS += -no-msse2
# MFLAGS += -msse3
# MFLAGS += -mno-sse3
# MFLAGS += -mssse3
# MFLAGS += -mno-ssse3
# MFLAGS += -msse4.1
# MFLAGS += -mno-sse4.1
  MFLAGS += -msse4.2
# MFLAGS += -mno-sse4.2
# MFLAGS += -march=native
# MFLAGS += -mfpmath=sse

# Leave off -Wmissing-prototypes because I don't like providing prototypes
# that have no value at all, i.e. when the function definition appears
# before any use of the function. Nevermind. Looks like "static" addresses
# the missing prototype just as well as a prototype does. Yes!
  WFLAGS += -Wall -Werror
  WFLAGS += -pedantic -Wstrict-prototypes -W
  WFLAGS += -Wmissing-prototypes
  WFLAGS += -Wno-unused-function
# gcc gives false positives without -Wno-maybe-uninitialized.
# clang doesn't even try. So clang doesn't support the option.
ifeq "$(CC)" "gcc"
  WFLAGS += -Wno-maybe-uninitialized
endif

# -O0 no optimization
# -O1 between -O2 and -O0
# -O2 moderate optimization enabling most optimizations
# -Oz (reduce size further) ==> -Os (reduce size) ==> -O2
# -O3 ==> -O2 optimizations may increase code size
# -Ofast ==> -O3 (and may violate strict language standards)
# -O4 link-level optimizations by storing files in LLVM bitcode
# I have seen -O4 inline BitmapGet from bitmap.o into Judy1LHTime.o.
# I don't know if it is possible to get -O4 without -O3.  I'm assuming
# -O4 does not imply -Ofast.  I wonder if -Ofast and -O4 can go together.
# OFLAGS = -g -O0
# OFLAGS = -g -O1
  OFLAGS = -g -O2
# OFLAGS =    -O2
# OFLAGS = -g -O3
# OFLAGS = -g -O4
# OFLAGS = -g -Os
# OFLAGS = -g -Oz
# OFLAGS = -g -Ofast

CFLAGS = $(STDFLAG) $(MFLAGS) $(WFLAGS) $(OFLAGS) -I.
CFLAGS_NO_WFLAGS = $(STDFLAG) $(MFLAGS) -w $(OFLAGS) -I.

# Obsolete ifdefs used to figure out where overhead was coming from that
# was making Time -b get times faster than Time -1 get times for libb
# configured to allocate one big bitmap.
# Bitmap[Set|Get] don't have P_JE.  Judy1[Set|Test] do have P_JE.
# These ifdefs were used to level the field.
# TIME_DEFINES += -DBITMAP_P_JE -DNO_P_JE
# EXTERN_JUDY1 used to put Judy1[Test|Set] in judy1.c via judy1x.c.
# INTERN_JUDY1 used to put Judy1[Test|Set] in Judy1LHTime.c via judy1x.c.
# But Doug doesn't want judy1.c or judy1x.c.
# Neither puts Judy1[Test|Set] in bl.c via bli.c via judy1x.c.
# EXTERN_BITMAP used to put Bitmap[Set|Get] in bitmap.c via bitmapx.c.
# No EXTERN_BITMAP used to put Bitmap[Set|Get] in Judy1LHTime.c via bitmapx.c.
# But Doug doesn't want bitmap.c or bitmapx.c.
# TIME_DEFINES += -DEXTERN_BITMAP -DINTERN_JUDY1 -DEXTERN_JUDY1
# SWAP causes Time -1 to use Bitmap[S|G]et and Time -b to use Judy1[Set|Test].
# BITMAP_BY_BYTE enables byte address/mask in judy1x.c and bitmapx.c.
# TIME_DEFINES += -DSWAP -DBITMAP_BY_BYTE

# Performance:
# DEBUG adds some internal sanity checking in my code not covered by
# assertions only.
# It does not log anything unless something wrong is detected.
# DEBUG is used by Judy1LHTime.c to turn off NDEBUG.
# No NDEBUG is used by assert.h to enable assertions.
# TIME_DEFINES += -UDEBUG -DNDEBUG

# Debug/Check/Instrument:
#
# GUARDBAND checks for corruption of word following malloc buffer on free
# TIME_DEFINES += -DGUARDBAND
# SISTER_READ is for measuring the speed of dereferencing subsequent
# cache lines using "Judy1LHTime -b".
# TIME_DEFINES += -DSISTER_READ

# Always:
#
# _POSIX_C_SOURCE=199309L gives CLOCK_MONOTONIC on GNU/Linux even though
# -std=c11 and -std=c99 don't.
# TIME_DEFINES += -D_POSIX_C_SOURCE=199309L
# JUDYA specifies JUDYA-style instrumentation variables.
# TIME_DEFINES += -DJUDYA

DEFINES += $(JUDY_DEFINES) $(TIME_DEFINES) $(B_DEFINES) $(B_DEBUG_DEFINES)

FILES_FROM_ME  = b.h b.c bli.c bl.c bi.c br.c bc.c bn.c t.c
FILES_FROM_ME += stubsL.c stubsHS.c Makefile
FILES_FROM_ME += bb forx
FILES_FROM_ME += README.meb
# I periodically make changes to the files provided by Doug B.
FILES_FROM_DOUG_B_OR_DOUG_LEA  = Judy.h RandomNumb.h dlmalloc.c JudyMalloc.c
FILES_FROM_DOUG_B_OR_DOUG_LEA += Judy1LHCheck.c Judy1LHTime.c jbgraph
FILES = $(FILES_FROM_ME) $(FILES_FROM_DOUG_B_OR_DOUG_LEA)

EXES = b check # t
LIBS = libb1.a libb1.so libb.a libb.so
LIB1_OBJS = b.o bl.o bi.o br.o bc.o bn.o JudyMalloc.o
LIB1_SRCS = b.c bl.c bi.c br.c bc.c bn.c JudyMalloc.c
LIB_OBJS = $(LIB1_OBJS) stubsL.o stubsHS.o
LIB_SRCS = $(LIB1_SRCS) stubsL.c stubsHS.c
ASMS  = b.s bl.s bi.s br.s bc.s bn.s
ASMS += stubsL.s stubsHS.s JudyMalloc.s # t.s
CPPS  = b.i bl.i bi.i br.i bc.i bn.i
CPPS += stubsL.i stubsHS.i JudyMalloc.i # t.i

T_OBJS = stubsL.o stubsHS.o JudyMalloc.o

##################################
#
# Start "default" and "all" with "clean" for these reasons:
#
# 1. This Makefile does not distinguish 64-bit from 32-bit objects.  When
#    switching from one to the other it is necessary to rebuild everything.
# 2. I couldn't figure out how to add the .h file dependencies without
#    breaking the .c.o rule -- I tried creating a separate rule but that
#    didn't work.
# 3. To ensure that all objects are built with the same/compatible
#    "DEFINES=..." on the command line.
#
##################################

default: clean b check

all: clean $(EXES) $(LIBS) $(ASMS) $(CPPS) b.tjz

clean:
	rm -f $(EXES) $(LIBS) $(LIB_OBJS) $(ASMS) $(CPPS) b.tjz

t:	t.c $(T_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ -lm

b:	Judy1LHTime.c libb.a
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -o $@ $^ -lm

# Set LIBRARY_PATH environment variable to find libJudy.a.
# Need -lm on Ubuntu. Appears to be unnecessary on macOS.
check:	Judy1LHCheck.c libb1.a
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -o $@ $^ -lJudy -lm
#check:	Judy1LHCheck.c
#	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -o $@ \
#		$^ $(LIB1_OBJS) /usr/local/lib/libJudy.a -lm

b.tjz:	$(FILES)
	tar cjf $@ $^

libb.a: $(LIB_OBJS)
	ar -r $@ $(LIB_OBJS)

libb1.a: $(LIB1_OBJS)
	ar -r $@ $(LIB1_OBJS)

# Build libb.so directly from sources rather than from
# objects so this Makefile doesn't have to deal with the complexity
# of -fPIC objects and non -fPIC objecs with the same names.
libb.so:
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -shared -o $@ $(LIB_SRCS)

libb1.so:
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -shared -o $@ $(LIB1_SRCS)

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
.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $^

stubsL.o: stubsL.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -c $^

stubsHS.o: stubsHS.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -c $^

JudyMalloc.o: JudyMalloc.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -DRAMMETRICS -c $^

############################
#
# Rules for building .s files.
#
############################

.c.s:
	$(CC) $(CFLAGS) $(DEFINES) -S $^

# Suppress warnings. Transitive warnings. t.c just includes other files.
t.s: t.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -S $^

# Suppress warnings.
Judy1LHTime.s: Judy1LHTime.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -S $^

stubsL.s: stubsL.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -S $^

stubsHS.s: stubsHS.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -S $^

# Suppress warnings.  sbrk is deprecated.
JudyMalloc.s: JudyMalloc.c
	$(CC) $(CFLAGS_NO_WFLAGS) $(DEFINES) -S $^

############################
#
# Rules for building .i files.
#
############################

.c.i:
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $^

# The .c.i rule doesn't work for some reason.  Later.
b.i: b.c
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
bc.i: bc.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
bn.i: bn.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
stubsL.i: stubsL.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
stubsHS.i: stubsHS.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
Judy1LHTime.i: Judy1LHTime.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
t.i: t.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
JudyMalloc.i: JudyMalloc.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

