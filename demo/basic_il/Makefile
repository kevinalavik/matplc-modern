# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: ready

clean:
	-rm -f chaser_il chaser_il.c kbd *.o Makefile.depend *.log

LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

#where to find the MatPLC directory
MATPLC = ../..

#get matplc includes (should point to main MatPLC directory)
CFLAGS += -I$(MATPLC)/lib

all: chaser_il

ready: all $(MATPLC)/tools/run/matplc $(MATPLC)/mmi/curses/vitrine
ready: $(MATPLC)/lib/util/plcshutdown 
ready: ../basic/kbd 
#ready: $(MATPLC)/logic/il/il2c

# the next line is actually optional; it'll work without it (somewhat less
# nicely)
ready: $(MATPLC)/mmi/kbd/linuxkbd

run: ready
	./demo

chaser_il: $(MATPLC)/lib/libmatplc.la
#chaser_il: chaser_il.o $(MATPLC)/lib/libmatplc.la

%.c: %.il $(MATPLC)/logic/il/il2c
	$(MATPLC)/logic/il/il2c $*.il > $@

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend: chaser_il.c
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
