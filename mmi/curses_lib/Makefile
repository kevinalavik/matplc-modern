MODNAME = vitrine
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
EXECUTE = $(LPLC)/tools/lib


all:	$(EXECUTE)/$(MODNAME).so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared -lncurses

$(EXECUTE)/$(MODNAME).so:	$(MODNAME).c
		$(CC) $(CFLAGS) -c $(MODNAME).c
		$(CC) $(LIBFLAG) -o $(EXECUTE)/$(MODNAME).so $(MODNAME).o

clean:
	rm -f *.o $(EXECUTE)/$(MODNAME).so
