# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: il2c test

install: il2c
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/ 
	$(LIBTOOL) --mode=install install il2c $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/il2c


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/il2c


clean:
	-rm -f il2c test test.c *.o


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

il2c: il2c.pl
	cp $< $@

%.c: %.il il2c
	./il2c $*.il > $@

test: $(MATPLC)/lib/libmatplc.la

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

# comment out Makefile.depend for now - we don't need it right now, and it
# can be a pain when messing about with generated .c files...

#Makefile.depend depend:
#	gcc -MM -MG -I$(MATPLC)/lib *.c \
#		| perl -pe 's/:/ Makefile.depend:/' \
#		> Makefile.depend
#
#include Makefile.depend
