# include the system specific Makefile
include ../../Makefile.$(shell uname)

all: vitrine

default: all

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install vitrine $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/vitrine


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/vitrine


clean:
	-rm -f vitrine *.o Makefile.depend

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

CFLAGS += $(PLCSYSTEM_CFLAGS)

vitrine: $(MATPLC)/lib/libmatplc.la -lncurses

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
