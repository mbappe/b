
CC = cc

#M = -m64
M = -m32

CFLAGS = $(M) -g -O2 -Wall -Werror
#CFLAGS = $(M) -O2 -Wall -Werror

#DEFINES += -DJUDYB -DNDEBUG -DSORT_LISTS
DEFINES += -DJUDYB -DGUARDBAND -DSORT_LISTS

DLMALLOC = dlmalloc.o

SRCS = b.c Judy1LHTime.c

LIBS = -lm

TAR_FILES = $(SRCS) bli.c b.c b.h Judy.h RandomNumb.h Makefile

TGTS = b b.o b.s b.i bl.o bl.s bl.i b.tar

all:	clean $(TGTS)

b:
	$(CC) $(CFLAGS) $(DEFINES) -I. -o b $(SRCS) $(DLMALLOC) $(LIBS)

dlmalloc.o: dlmalloc.c
	$(CC) $(M) -O2 -c dlmalloc.c

b.o:
	$(CC) $(CFLAGS) ${DEFINES} -c b.c

b.s:
	$(CC) $(CFLAGS) ${DEFINES} -S b.c

b.i:
	$(CC) $(CFLAGS) ${DEFINES} -E b.c | indent -i4 | expand > b.i

bl.o:
	$(CC) $(CFLAGS) ${DEFINES} -c bl.c

bl.s:
	$(CC) $(CFLAGS) ${DEFINES} -S bl.c

bl.i:
	$(CC) $(CFLAGS) ${DEFINES} -E bl.c | indent -i4 | expand > bl.i

b.tar:
	tar cf b.tar $(TAR_FILES)

clean:
	rm -rf $(TGTS) b.dSYM
