MODNAME = tcpip
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools/lib


all:	$(DEST)/$(MODNAME).so ../network_io/network_io.o

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

../network_io/network_io.o:
	cd ../network_io; make

$(DEST)/$(MODNAME).so:	$(MODNAME).cpp ../network_io/network_io.o
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(DEST)/$(MODNAME).so ../network_io/network_io.o $(MODNAME).o

clean:
	rm -f *.o $(DEST)/$(MODNAME).so
