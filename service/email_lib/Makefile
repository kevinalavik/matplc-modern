MODNAME = email
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools/lib
NETWORK_IO = $(LPLC)/comm/network_io


all:	$(DEST)/$(MODNAME).so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER) -I$(NETWORK_IO)
LIBFLAG = -shared

$(NETWORK_IO)/network_io.o:
	cd $(NETWORK_IO); make

$(DEST)/$(MODNAME).so:	$(MODNAME).cpp $(NETWORK_IO)/network_io.o
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(DEST)/$(MODNAME).so $(NETWORK_IO)/network_io.o $(MODNAME).o

clean:
	rm -f *.o $(DEST)/$(MODNAME).so
