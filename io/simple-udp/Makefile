# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: send recv

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install send $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/send
	$(LIBTOOL) --mode=install install recv $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/recv


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/send
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/recv


clean:
	-rm -f send recv *.o


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

send recv: $(MATPLC)/lib/libmatplc.la

send recv: common.o

send: send.o
recv: recv.o

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
