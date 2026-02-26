MODNAME = rs232c
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools/lib


all:	$(DEST)/$(MODNAME).so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

rs232c_io.o:	rs232c_io.cpp rs232c_io.h
		$(CC) $(CFLAGS) -c rs232c_io.cpp

$(DEST)/$(MODNAME).so:	$(MODNAME).cpp rs232c_io.o
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(DEST)/$(MODNAME).so rs232c_io.o $(MODNAME).o

clean:
	rm -f *.o $(DEST)/$(MODNAME).so
