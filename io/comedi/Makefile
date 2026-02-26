# include the system specific Makefile
include ../../Makefile.$(shell uname)


default: all

all: comedi


install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install comedi $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/comedi


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/comedi


clean:
	-rm -f comedi *.o Makefile.depend


LDFLAGS = $(PLCSYSTEM_LDFLAGS)
LDFLAGS += -lcomedi

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

comedi: comedi.o comedi_syntax.o
comedi: $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
