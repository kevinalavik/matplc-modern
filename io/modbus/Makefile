# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: all

all: modbus_m_rtu modbus_m_asc modbus_m_tcp modbus_s_tcp


install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install modebus_m_rtu $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modebus_m_rtu
	$(LIBTOOL) --mode=install install modebus_m_asc $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modebus_m_asc
	$(LIBTOOL) --mode=install install modebus_m_tcp $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modebus_m_tcp
	$(LIBTOOL) --mode=install install modebus_s_tcp $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modebus_s_tcp


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modbus_m_rtu
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modbus_m_asc
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modbus_m_tcp
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/modbus_s_tcp


clean:
	-rm -f modbus_?_??? *.o Makefile.depend matplc.log

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
CFLAGS += -Iprotocol/

#NOTE: get the modbus_m.c file to not produce an error when being parsed for 
#dependencies. Since this file needs to be compiled with one, and only one
#of the following defines: MODBUS_M_RTU MODBUS_M_ASCII MODBUS_M_TCP
DFLAGS = -DMODBUS_M_TCP -DMODBUS_S_TCP

#Just some utilities I (Mario) was using to test the module.
#May be safely deleted...
test_m: test_m.o protocol/mb_tcp.o protocol/mb_master.o $(MATPLC)/lib/libmatplc.la
test_s: test_s.o protocol/mb_tcp.o protocol/mb_master.o $(MATPLC)/lib/libmatplc.la

#How to compile and link each version of the modbus_m module
modbus_m_rtu.o: modbus_m.c modbus_m.h
	$(CC) $(CFLAGS) -c modbus_m.c -o $@ -DMODBUS_M_RTU

modbus_m_asc.o: modbus_m.c modbus_m.h
	$(CC) $(CFLAGS) -c modbus_m.c -o $@ -DMODBUS_M_ASCII

modbus_m_tcp.o: modbus_m.c modbus_m.h
	$(CC) $(CFLAGS) -c modbus_m.c -o $@ -DMODBUS_M_TCP

modbus_s_tcp.o: modbus_s.c modbus_s.h
	$(CC) $(CFLAGS) -c modbus_s.c -o $@ -DMODBUS_S_TCP

modbus_m_rtu: modbus_m_rtu.o protocol/mb_rtu.o protocol/mb_master.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

modbus_m_asc: modbus_m_asc.o protocol/mb_ascii.o protocol/mb_master.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

modbus_m_tcp: modbus_m_tcp.o protocol/mb_tcp.o protocol/mb_master.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

modbus_s_tcp: modbus_s_tcp.o protocol/mb_tcp.o protocol/mb_slave.o $(MATPLC)/lib/io/io.o $(MATPLC)/lib/libmatplc.la
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@


#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG $(CFLAGS) $(DFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

include Makefile.depend
