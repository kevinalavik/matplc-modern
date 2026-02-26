
MOD1 = echo_managed
MOD2 = echo_unmanaged
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
DEST = .

all:	$(MOD1).so $(MOD2).so $(EXECUTE)/run/handler $(LPLC)/tools/lib/tcpip.so

CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(LPLC)/tools/lib/tcpip.so:
		cd $(LPLC)/comm/tcpip_lib ; make

$(MOD1).so:	$(MOD1).cpp
		$(CC) $(CFLAGS) -c $(MOD1).cpp
		$(CC) $(LIBFLAG) -o $(MOD1).so $(MOD1).o

$(MOD2).so:	$(MOD2).cpp
		$(CC) $(CFLAGS) -c $(MOD2).cpp
		$(CC) $(LIBFLAG) -o $(MOD2).so $(MOD2).o

clean:
	rm -f *.o $(MOD1).so $(MOD2).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/comm/tcpip_lib; make clean
