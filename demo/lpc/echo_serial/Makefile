
MOD1 = echo_managed
MOD2 = echo_unmanaged
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
DEST = .

all:	$(MOD1).so $(MOD2).so $(EXECUTE)/lib/rs232c.so 

CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/rs232c.so:
		cd $(LPLC)/comm/rs232c_lib ; make

$(MOD1).so:	$(MOD1).cpp
		$(CC) $(CFLAGS) -c $(MOD1).cpp
		$(CC) $(LIBFLAG) -o $(MOD1).so $(MOD1).o

$(MOD2).so:	$(MOD2).cpp
		$(CC) $(CFLAGS) -c $(MOD2).cpp
		$(CC) $(LIBFLAG) -o $(MOD2).so $(MOD2).o

clean:
	rm -f *.o $(MOD1).so $(MOD2).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/comm/rs232c_lib; make clean
