# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: ready

clean:
	-rm -f chaser kbd *.o

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

all: chaser kbd

ready: all $(MATPLC)/tools/run/matplc $(MATPLC)/mmi/curses/vitrine
ready: $(MATPLC)/lib/util/plcshutdown

# the linuxkbd optional; try to compile it, give warning if we can't
ready:
	make -C $(MATPLC)/mmi/kbd linuxkbd || make linuxkbd-warning

linuxkbd-warning:
	@echo ""
	@echo "+-------------------------------------------------------+"
	@echo "| Error compiling linuxkbd, but don't worry!            |"
	@echo "| The demo will work without it, somewhat less nicely   |"
	@echo "| (you will have toggle buttons instead of pushbuttons) |"
	@echo "| Press Enter to continue...                            |"
	@echo "+-------------------------------------------------------+"
	@head -1

run: ready
	@echo Use
	@echo "  ./demo"
	@echo to start the demo.
#	../../tools/run/matplc -g

chaser kbd: $(MATPLC)/lib/libmatplc.la

chaser: $(MATPLC)/lib/logic/timer.o chaser.o

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
