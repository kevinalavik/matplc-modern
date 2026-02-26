MODNAME = das08
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools/lib


all:	$(DEST)/$(MODNAME).so testdaq

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

das08_io.o:	das08_io.cpp das08_io.h
	$(CC) $(CFLAGS) -c das08_io.cpp

$(DEST)/$(MODNAME).so:	$(MODNAME).cpp das08_io.o
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(DEST)/$(MODNAME).so $(MODNAME).o das08_io.o

testdaq:	testdaq.cpp das08_io.o
		$(CC) $(CFLAGS) testdaq.cpp -o testdaq das08_io.o

clean:
	rm -f *.o $(DEST)/$(MODNAME).so testdaq
