# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: matplc mattest

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/ 
	$(LIBTOOL) --mode=install install matplc $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/matplc
	$(LIBTOOL) --mode=install install mattest $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/mattest


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/matplc
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/mattest


clean:
	-rm -f *.o *.lo Makefile.depend matplc.log matplc mattest
# note: this directory used to have plcshutdown, that's why it's here

LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
CFLAGS += $(PLCSYSTEM_CFLAGS)


MATLIB = ../../lib
CFLAGS += -I$(MATLIB)

# NOTE: Not all symbols are currently being exported in the libmatplc.so library,
#       including some functions required by the matplc_setup library.
#       So we link to the static library instead.
matplc: matplc.o $(MATLIB)/matplc.a $(MATLIB)/matplc_setup.a
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

mattest: mattest.o $(MATLIB)/libmatplc.la
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	$(CC) -MM -MG -I$(MATLIB) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend
