
###########################
#
# Makefile for b. Code for learning about Judy and possible improvements.
#
###########################

# Run the following to build b:
#   make clean
#   [CC=<cc|clang[++]|gcc|icc|c++|g++>] [BPW=<32|64>] [DEFINES="..."] make b
# Run the following command to build everything:
#   make clean
#   [CC=<cc|clang[++]|gcc|c++|g++>] [BPW=<32|64>] [DEFINES="..."] make all
# examples:
#   make clean all
#   CC=clang CSTD=c11 DEFINES=-DDEBUG make clean default
#   CC=clang++ CSTD=c++14 make b
#   CXX=clang++ CXXSTD=c++11 make bcheck
CC ?= cc

# I recommend the make clean first because there are so many dependencies
# that are not discovered by make, e.g. changes in environment variables.
# Also, I didn't bother adding the header file dependencies.

# The default build is 64-bits.
# Use "BPW=32 make" to get a 32-bit build.
# I wonder if it works on a 32-bit system.
BPW ?= 64

# We can't use b.h to set MALLOC_ALIGNMENT because it is not included in
# JudyMalloc.c or dlmalloc.c. We have to set it here if we want something
# other than the dlmalloc.c default which is sizeof(void*) * 2.
# We want to bump MALLOC_ALIGNMENT to 16 for 32-bit for our own default.
ifeq "$(BPW)" "32"
ifeq "$(MALLOC_ALIGNMENT)" ""
        MALLOC_ALIGNMENT = 16
endif
endif
ifneq "$(MALLOC_ALIGNMENT)" ""
    # b.h will set cnBitsMallocMask appropriately based on MALLOC_ALIGNMENT
    DEFINES += -DMALLOC_ALIGNMENT=$(MALLOC_ALIGNMENT)
endif

# There are questions about how macOS handles libraries.
# It looks like macOS Sierra (10.12) may not honor the LIBRARY_PATH, et. al.,
# environment variables if exported by .profile but will honor them if set on
# the command line.
# It looks like macOS doesn't like having a 32-bit library called libJudy
# in one directory and a 64-bit called libJudy in another directory and
# both directories in the library search path because ld will abort if/when
# it encounters the library with the wrong architecture first.
# It looks like macOS uses its own 'libtool' to create a single dynamic
# library that holds both 32-bit and 64-bit objects.
# We are not that sophisticated yet.
ifeq "$(BPW)" "32"
  LDFLAGS = -L/usr/local/lib32
else
  LDFLAGS = -L/usr/local/lib
endif

# Potentially interesting gcc options:
# -ftree-vectorize (enabled by default with -O3)
# -ftree-vectorizer-verbose
# -fkeep-inline-functions
# -fverbose-asm
# -fpic vs. -fPIC
# -frecord-gcc-switches
# -grecord-gcc-switches
# -fpack-struct[=n]
# -fvisibility=[default|internal|hidden|protected]
# -fatal-errors
# -language-extension-token

##
# -std=gnu11 and -std=gnu99 give CLOCK_MONOTONIC which is not available with
# -std=c11 or -std=c99.
# We use CLOCK_MONOTONIC on GNU/Linux, but not on Mac.
# -std=c11 gives anonymous unions with no complaints.
# -std=gnu99 gives a warning for anonymous unions.
##
ifeq "$(CSTD)" ""
# CSTD = gnu11
# CSTD = c11
# CSTD = gnu99
# CSTD = c99
# CSTD = c90
# CSTD = c89
endif

ifneq "$(CSTD)" ""
  CSTDFLAG = -std=$(CSTD)
endif

# c++11 doesn't allow 0ULL.
ifeq "$(CXXSTD)" ""
# CXXSTD = -std=c++14
# CXXSTD = -std=c++11
endif

ifneq "$(CXXSTD)" ""
  CXXSTDFLAG = -std=$(CXXSTD)
endif

  MFLAGS += -m$(BPW)
# MFLAGS += -mmmx
# MFLAGS += -mno-mmx      # implies no -msse
# MFLAGS += -msse         # implies mmx
# MFLAGS += -mno-sse      # implies no -msse2
# MFLAGS += -msse2        # implies -msse
# MFLAGS += -no-msse2     # implies no -msse3
# MFLAGS += -msse3        # implies -msse2
# MFLAGS += -mno-sse3     # implies no -mssse3
# MFLAGS += -mssse3       # implies -msse3
# MFLAGS += -mno-ssse3    # implies no -msse4.1
# MFLAGS += -msse4.1      # implies -mssse3
# MFLAGS += -mno-sse4.1   # implies no -msse4.2
  MFLAGS += -msse4.2      # implies -msse4.1
# MFLAGS += -mno-sse4.2   # implies no -mavx
# MFLAGS += -msse4        # implies -msse4.2
# MFLAGS += -mno-sse4     # implies no -msse4.1
# MFLAGS += -mavx         # implies -msse4.2
# MFLAGS += -mno-avx      # implies no -mavx2
# MFLAGS += -mavx2        # implies -mavx
# MFLAGS += -mno-avx2
# MFLAGS += -mbmi         # for lzcnt, tzcnt
# MFLAGS += -mbmi2        # for pdep
# MFLAGS += -mavx512f
# MFLAGS += -march=native
# MFLAGS += -mfpmath=sse

# Leave off -Wmissing-prototypes because I don't like providing prototypes
# that have no value at all, i.e. when the function definition appears
# before any use of the function. Nevermind. Looks like "static" addresses
# the missing prototype just as well as a prototype does. Yes!
  WFLAGS += -Wall
  WFLAGS += -Wextra
# -Wpedantic doesn't like the gcc extension &&<label> even with -std=gnu11
# WFLAGS += -Wpedantic
  WFLAGS += -Wno-unused-function
  WFLAGS += -Wno-parentheses-equality
  WFLAGS += -Wno-deprecated
  WFLAGS += -Werror
  WFLAGS += -Wfatal-errors
  WFLAGS += -Wno-unused-value
# WFLAGS_C_ONLY += -Wstrict-prototypes
# WFLAGS_C_ONLY += -Wmissing-prototypes
  CWFLAGS += $(WFLAGS)
  CWFLAGS += $(WFLAGS_C_ONLY)
# gcc gives false positives without -Wno-maybe-uninitialized.
# clang doesn't even try. So clang doesn't support the option.
# macOS version of gcc doesn't support it either.
#ifeq "$(CC)" "gcc"
#  WFLAGS += -Wno-maybe-uninitialized
#endif
# gcc has -Wold-style-declaration clang doesn't
# clang and gcc both have -Wunknown-warning-option
# WFLAGS += -Wno-unknown-warning-option

ifeq "$(OFLAGS)" ""
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
  OFLAGS = -g -O2 -fno-stack-protector
# OFLAGS =    -O2
# OFLAGS = -g -O3
# OFLAGS = -g -O4
# OFLAGS = -g -Os
# OFLAGS = -g -Oz
# OFLAGS = -g -Ofast
endif # OFLAGS

CFLAGS =           $(CSTDFLAG)   $(MFLAGS) $(CWFLAGS)   $(OFLAGS) -I.
CFLAGS_NO_WFLAGS = $(CSTDFLAG)   $(MFLAGS) -w           $(OFLAGS) -I.
CXXFLAGS =         $(CXXSTDFLAG) $(MFLAGS) $(CXXWFLAGS) $(OFLAGS) -I.

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

# Default is -DRAMMETRICS.
# Use NO_RAMMETRICS=<anything> to get -URAMMETRICS.
ifeq "$(NO_RAMMETRICS)" ""
    JUDY_DEFINES += -DRAMMETRICS
endif

# Put cmdline DEFINES after default defines so defaults can be overridden.
MAKE_DEFINES = $(JUDY_DEFINES) $(TIME_DEFINES) $(B_DEFINES) $(B_DEBUG_DEFINES)
DEFINES := $(MAKE_DEFINES) $(DEFINES)

FILES_FROM_ME  = b.h b.c bli.c bl.c bi.c br.c bc.c t.c
FILES_FROM_ME += Makefile
FILES_FROM_ME += bb forx
FILES_FROM_ME += README.meb
# I periodically make changes to the files provided by Doug B.
FILES_FROM_DOUG_B_OR_DOUG_LEA  = Judy.h RandomNumb.h dlmalloc.c JudyMalloc.c
FILES_FROM_DOUG_B_OR_DOUG_LEA += Judy1LHCheck.c Judy1LHTime.c jbgraph
FILES = $(FILES_FROM_ME) $(FILES_FROM_DOUG_B_OR_DOUG_LEA)

EXES = Judy1LHTime Judy1LHCheck c++time c++check # t
LINKS = b btime bcheck
LIBS = libb1.a libb1.so libbL.a libbL.so libb.a libb.so
LIBB1_OBJS = b.o bl.o bi.o br.o bc.o JudyMalloc.o
LIBB1_SRCS = b.c bl.c bi.c br.c bc.c
LIBBL_OBJS = b-L.o blL.o biL.o brL.o bcL.o JudyMalloc.o
LIBBL_SRCS = b-L.c blL.c biL.c brL.c bcL.c
LIBB_OBJS = $(LIBB1_OBJS) $(LIBBL_OBJS)
LIBB_SRCS = $(LIBB1_SRCS) $(LIBBL_SRCS)
ASMS  = b.s bl.s bi.s br.s bc.s
ASMS += JudyMalloc.s # t.s
ASMS += Judy1LHTime.s Judy1LHCheck.s
CPPS  = b.i bl.i bi.i br.i bc.i
CPPS += JudyMalloc.i # t.i

T_OBJS = JudyMalloc.o

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

default: $(EXES) $(LINKS)

all: $(EXES) $(LINKS) $(LIBS) $(ASMS) $(CPPS)

clean:
	rm -f $(EXES) $(LINKS) $(LIBS) $(LIBB_OBJS) $(ASMS) $(CPPS)

t: t.c $(T_OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ -lm

# -DMIKEY tells Judy1LHTime to use different RAMMETRICS column headings.
# -lJudy is for JudyHS.
Judy1LHTime: Judy1LHTime.c libb.a
#Judy1LHTime: Judy1LHTime.c libb1.a
	$(CC) $(CFLAGS) -DMIKEY $(DEFINES) -o $@ $^ -lm $(LDFLAGS) -lJudy
# ../judy/src/obj/.libs/libJudy.a

c++time: Judy1LHTime.c libb.a
	$(CXX) $(CXXFLAGS) -DMIKEY $(DEFINES) \
 -x c++ Judy1LHTime.c -x none libb.a -o $@ -lm -lJudy

b: Judy1LHTime
	ln -sf Judy1LHTime b

btime: Judy1LHTime
	ln -sf Judy1LHTime btime

# Set LIBRARY_PATH environment variable to find libJudy.a.
# Need -lm on Ubuntu. Appears to be unnecessary on macOS.
Judy1LHCheck: Judy1LHCheck.c libb.a
#Judy1LHCheck: Judy1LHCheck.c libb1.a
	$(CC) $(CFLAGS) -Wno-sign-compare $(DEFINES) \
 -o $@ $^ -lm $(LDFLAGS) -lJudy

c++check: Judy1LHCheck.c libb.a
	$(CXX) $(CXXFLAGS) $(DEFINES) \
 -x c++ Judy1LHCheck.c -x none libb.a -o $@ -lm $(LDFLAGS) -lJudy

bcheck: Judy1LHCheck
	ln -sf Judy1LHCheck bcheck

libb.a: $(LIBB_OBJS)
	ar -r $@ $^

libb1.a: $(LIBB1_OBJS)
	ar -r $@ $^

libbL.a: $(LIBBL_OBJS)
	ar -r $@ $^

# Build libb1.so and libbL.so directly from sources rather than from
# objects so this Makefile doesn't have to deal with the complexity
# of -fPIC objects and non -fPIC objecs with the same names.

libb1.so: $(LIBB1_SRCS) JudyMalloc.so
	$(CC) $(CFLAGS) -fPIC $(DEFINES) -shared -o $@ $^

libbL.so: $(LIBBL_SRCS) JudyMalloc.so
	$(CC) $(CFLAGS) -fPIC $(DEFINES) -DB_JUDYL -shared -o $@ $^

libb.so: libb1.so libbL.so
	$(CC) -shared -o $@ $^

ifneq "$(CC)" "c++"
ifneq "$(CC)" "g++"
ifneq "$(CC)" "clang++"
  MALLOC_FLAGS = -Wno-old-style-declaration -Wno-unknown-warning-option \
    -Wno-expansion-to-defined -Wno-null-pointer-arithmetic
endif
endif
endif

JudyMalloc.so: JudyMalloc.c
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) -fPIC $(DEFINES) -shared -o $@ $^

b-L.o: b.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^
b1L.o: b1.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^
biL.o: bi.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^
blL.o: bl.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^
brL.o: br.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^
bcL.o: bc.c
	$(CC) $(CFLAGS) $(DEFINES) -DB_JUDYL -o $@ -c $^

b-L.c: b.c
	ln -s $< $@
b1L.c: b1.c
	ln -s $< $@
biL.c: bi.c
	ln -s $< $@
blL.c: bl.c
	ln -s $< $@
brL.c: br.c
	ln -s $< $@
bcL.c: bc.c
	ln -s $< $@

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

# Default MALLOC_ALIGNMENT is 2 * sizeof(void *), except possibly on OSX.
JudyMalloc.o: JudyMalloc.c
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) $(DEFINES) -c $^

############################
#
# Rules for building .s files.
#
############################

.c.s:
	$(CC) $(CFLAGS) $(DEFINES) -S $^

# Suppress warnings. Transitive warnings. t.c just includes other files.
t.s: t.c
	$(CC) $(CFLAGS) $(DEFINES) -S $^

# Suppress warnings.
Judy1LHTime.s: Judy1LHTime.c
	$(CC) $(CFLAGS) -DMIKEY $(DEFINES) -S $^

# Suppress warnings.  sbrk is deprecated.
JudyMalloc.s: JudyMalloc.c
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) $(DEFINES) -S $^

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
Judy1LHTime.i: Judy1LHTime.c
	$(CC) $(CFLAGS) -DMIKEY $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
t.i: t.c
	$(CC) $(CFLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
JudyMalloc.i: JudyMalloc.c
	$(CC) $(CFLAGS) $(MALLOC_FLAGS) $(DEFINES) -E $^ | indent -i4 | expand > $@

#
# -mmmx -msse -msse2 -mno-sse4 is the default for -m64 as of this writing
# -mmmx -mno-sse4 is the default for -m32 as of this writing
# -malign-stringops is default
# -mavx256-split-unaligned-load -mavx256-split-unaligned-store is default
#
# -mavx512f => -mavx2 => -mavx => -msse4 <=> -msse4.2 => -msse4.1
# => -mssse3 => -msse3 => -msse2 => msse => -mmmx
#
# -mno-mmx does not set -mno-sse
#
# -mno-sse does set -mno-sse2
#
# Use cc -Q -v <blah>.c to see what gets set.
#
psearch-all:
	X=-mavx512f Y= Z=64 make psearch
	X=-mavx512f Y= Z=32 make psearch
	X=-mavx2 Y= Z=64 make psearch
	X=-mavx2 Y= Z=32 make psearch
	X=-mavx Y= Z=64 make psearch
	X=-mavx Y= Z=32 make psearch
	X=-msse4 Y= Z=64 make psearch
	X=-msse4 Y= Z=32 make psearch
	X=-mssse3 Y= Z=64 make psearch
	X=-mssse3 Y= Z=32 make psearch
	X=-msse3 Y= Z=64 make psearch
	X=-msse3 Y= Z=32 make psearch
	X=-msse2 Y= Z=64 make psearch
	X=-msse2 Y= Z=32 make psearch
	X=-mno-sse2 Y= Z=64 make psearch
	X=-mno-sse2 Y= Z=32 make psearch
	# -mno-sse is invalid for -m64
	#X=-mno-sse Y= Z=64 make psearch
	X=-mno-sse Y= Z=32 make psearch
	# -mno-sse is invalid for -m64
	#X=-mno-mmx Y='-mno-sse' Z=64 make psearch
	X=-mno-mmx Y='-mno-sse' Z=32 make psearch

psearch: psearch$(X)-$(Z) ;

psearch$(X)-$(Z): psearch.cpp
	$(CXX) -std=gnu++11 -fno-lax-vector-conversions \
 -Wno-psabi -Wno-unknown-warning-option \
 $(X) $(Y) -m$(Z) $(DEFINES) -I. psearch.cpp -o $@

# PSC_FLAGS = -mbmi
# -O3 is supposed to imply -ftree-vectorize
psc: psc.c
	gcc -O3 $(PSC_FLAGS) -g -o psc-gcc $<
	gcc -O3 $(PSC_FLAGS) -S -fkeep-inline-functions -o psc-gcc.s $<
	g++ -O3 $(PSC_FLAGS) -g -o psc-g++ $<
	g++ -O3 $(PSC_FLAGS) -S -fkeep-inline-functions -o psc-g++.s $<
	clang -O3 $(PSC_FLAGS) -g -o psc-clang $<
	clang -O3 $(PSC_FLAGS) -S -o psc-clang.s $<
	rm -f psc.cpp
	ln -s psc.c psc.cpp
	clang++ -O3 $(PSC_FLAGS) -g -o psc-clang++ psc.cpp
	clang++ -O3 $(PSC_FLAGS) -S -o psc-clang++.s psc.cpp
	rm -f psc
	ln -s psc-gcc psc
