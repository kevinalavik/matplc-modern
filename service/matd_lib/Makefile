MODNAME = matd
PROGRAM = admin
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools


all:	$(DEST)/run/$(PROGRAM) $(DEST)/lib/$(MODNAME).so 


CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared


user.o:		user.cpp user.h
		$(CC) $(CFLAGS) -c user.cpp


$(DEST)/run/$(PROGRAM): $(PROGRAM).cpp user.o
		$(CC) $(CFLAGS) $(PROGRAM).cpp user.o -o $(DEST)/run/$(PROGRAM) -lcrypt


$(DEST)/lib/$(MODNAME).so:	$(MODNAME).cpp user.o
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(DEST)/lib/$(MODNAME).so user.o $(MODNAME).o -lcrypt

clean:
	rm -f *.o $(DEST)/lib/$(MODNAME).so $(DEST)/run/$(PROGRAM)
