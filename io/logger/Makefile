# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: logger logger_db

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install logger    $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/logger
	$(LIBTOOL) --mode=install install logger_db $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/logger_db


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/logger
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/logger_db


clean:
	-rm -f *.o logger logger_db Makefile.depend matplc.log


LDFLAGS = $(PLCSYSTEM_LDFLAGS)
LDFLAGS += -L/usr/local/lib/mysql -lmysqlclient

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
CFLAGS += -I$(MATPLC)/lib -I/usr/local/include/mysql


logger: $(MATPLC)/lib/logic/timer.o logger.o $(MATPLC)/lib/libmatplc.la

logger_db: $(MATPLC)/lib/logic/timer.o logger_db.o db_wrap.o $(MATPLC)/lib/libmatplc.la

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG $(CFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend
