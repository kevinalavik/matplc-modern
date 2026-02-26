# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: cif

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install cif $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/cif


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/cif


clean:
	-rm -f *.o Makefile.depend matplc.log cif

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

#get hilscher's cif device driver includes
#CFLAGS += -I$(MATPLC)/drivers/cif/cifv1003/inc
#CFLAGS += -I$(MATPLC)/drivers/cif/cif-V2.000/usr-inc
CFLAGS += -I$(MATPLC)/drivers/cif/include

#get matplc includes (should point to main MatPLC directory)
CFLAGS += -I$(MATPLC)/lib


#cif: cif.o cif_util.o $(MATPLC)/lib/libmatplc.la $(MATPLC)/drivers/cif/cifv1003/test/cifAPI.o
#cif: cif.o cif_util.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la $(MATPLC)/drivers/cif/cif-V2.000/api/cif_api.o
cif: cif.o cif_util.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la $(MATPLC)/drivers/cif/lib/cif_api.o

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG $(CFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend
