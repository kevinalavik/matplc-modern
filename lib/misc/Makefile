# include the system specific Makefile
include ../../Makefile.$(shell uname)

# Generated automatically from Makefile.in by configure.
default: all

all: shmem_util.o sin_util.o string_util.o signal_util.o
all: timer_util.o daemon_util.o mutex_util.o sem_util.o
all: shmem_util.lo sin_util.lo string_util.lo signal_util.lo
all: timer_util.lo daemon_util.o mutex_util.o sem_util.lo 

clean:
	-rm -f *.o *.lo Makefile.depend

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -fPIC -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

CFLAGS += -I..
CFLAGS += $(PLCSYSTEM_CFLAGS)

Makefile.depend depend:
	$(CC) -MM -MG -I.. $(PLCSYSTEM_CFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend

# make a shareable object from C source
%.lo: %.c
	$(LIBTOOL) --mode=compile $(COMPILE.c) $(OUTPUT_OPTION) $<
