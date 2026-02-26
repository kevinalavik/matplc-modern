# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: timer.o timer_ext.o
all: timer.lo timer_ext.lo

clean:
	-rm -f *.o *.lo Makefile.depend matplc.log

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

CFLAGS += -I..
CFLAGS += $(PLCSYSTEM_CFLAGS)

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	$(CC) -MM -MG -I.. *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend

# make a shareable object from C source
%.lo: %.c
	$(LIBTOOL) --mode=compile $(COMPILE.c) $(OUTPUT_OPTION) $<
