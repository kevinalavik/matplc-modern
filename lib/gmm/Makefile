# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: gmm_setup.o gmm.o gmm_local.o gmm_isolate.o gmm_shared.o plcproxy.o protocol.o
all: gmm_setup.lo gmm.lo gmm_local.lo gmm_isolate.lo gmm_shared.lo plcproxy.lo protocol.lo

clean:
	-rm -f *.o *.lo Makefile.depend matplc.log

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
#CFLAGS += -ggdb -O3 -fPIC -funroll-loops
CFLAGS += -ggdb -O2 -fPIC -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
#
# Note: gcc version 2.96 does crash with -O3
#       We'll use -O2 while we don't start using autoconf and automake
#       to work around such problems elegantly.

CFLAGS += -I..
CFLAGS += $(PLCSYSTEM_CFLAGS)

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	$(CC) -MM -MG -I.. $(PLCSYSTEM_CFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend

# make a shareable object from C source
%.lo: %.c
	$(LIBTOOL) --mode=compile $(COMPILE.c) $(OUTPUT_OPTION) $<
