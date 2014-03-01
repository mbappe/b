
CC = cc
#CC = icc
#CC = clang
#CC = gcc

# Judy1LHTime.c doesn't compile with -std=c99.  It complains about some
# time.h symbol.  time.h is not included?

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
B_DEFINES += -DRAM_METRICS -DSEARCH_METRICS -DDEBUG
B_DEFINES += -DSKIP_LINKS -DSKIP_PREFIX_CHECK -UNO_UNNECESSARY_PREFIX
B_DEFINES += -DSORT_LISTS -UMIN_MAX_LISTS -URECURSIVE_TWEAK
B_DEFINES += -ULOOKUP_NO_LIST_DEREF -ULOOKUP_NO_LIST_SEARCH
B_DEFINES += -ULOOKUP_NO_BITMAP_DEREF -ULOOKUP_NO_BITMAP_SEARCH
#B_DEBUG_DEFINES += -UDEBUG_INSERT -UDEBUG_LOOKUP -UDEBUG_MALLOC -UDEBUG_REMOVE
DEFINES += $(JUDY_DEFINES) $(B_DEFINES) $(B_DEBUG_DEFINES)

LIBS = -lm

FILES_FROM_ME = b.h b.c bli.c bl.c bi.c br.c stubs.c Makefile
FILES_FROM_DOUG_OR_DOUG = Judy.h RandomNumb.h Judy1LHTime.c dlmalloc.c
FILES = $(FILES_FROM_ME) $(FILES_FROM_DOUG_OR_DOUG)

# I can't figure out how to add dependencies for the .o files on b.h
# for the .c.o rules without ending up with b.h on the compile line
# coming from $^.  And b.h on the command line causes a problem for icc
# and possibly other compilers.

all:	clean b b.tar bl.i bl.s bl.o bi.i bi.s bi.o b.i b.s b.o Judy1LHTime.s

clean:
	rm -rf b *.tar *.o *.s *.i *.dSYM

b:	Judy1LHTime.o b.o bl.o bi.o br.o stubs.o dlmalloc.o
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

b.tar:	$(FILES)
	tar cf $@ $(FILES)

# Suppress warnings.
Judy1LHTime.o: Judy1LHTime.c
	$(CC) $(CFLAGS) ${DEFINES} -w -c $^

# Suppress warnings.
stubs.o: stubs.c
	$(CC) $(CFLAGS) ${DEFINES} -w -c $^

# Suppress warnings.
dlmalloc.o: dlmalloc.c
	$(CC) $(CFLAGS) ${DEFINES} -w -c $^

# The .c.i rule doesn't work for some reason.  Later.
bl.i: bl.c
	$(CC) $(CFLAGS) ${DEFINES} -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
bi.i: bi.c
	$(CC) $(CFLAGS) ${DEFINES} -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
b.i: b.c
	$(CC) $(CFLAGS) ${DEFINES} -E $^ | indent -i4 | expand > $@

# The .c.i rule doesn't work for some reason.  Later.
br.i: br.c
	$(CC) $(CFLAGS) ${DEFINES} -E $^ | indent -i4 | expand > $@

.c.o:
	$(CC) $(CFLAGS) ${DEFINES} -c $^

.c.s:
	$(CC) $(CFLAGS) ${DEFINES} -S $^

.c.i:
	$(CC) $(CFLAGS) ${DEFINES} -E $^ | indent -i4 | expand > $^

