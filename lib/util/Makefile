# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: plcshutdown

install: all
	$(LIBTOOL) --mode=install cp plcshutdown $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/plcshutdown

clean:
	-rm -f *.o *.lo Makefile.depend matplc.log plcshutdown

LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

CFLAGS += -I..
CFLAGS += $(PLCSYSTEM_CFLAGS)

# note: uses the static library rather than the dynamic one, because the
# dynamic one doesn't export all the symbols it'd want to use
plcshutdown: ../matplc.a ../matplc_setup.a

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	$(CC) -MM -MG -I.. *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend
