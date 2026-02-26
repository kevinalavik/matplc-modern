
MODNAME = test
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
DEST = .
EXECUTE = $(LPLC)/tools

all:	$(MODNAME).so $(EXECUTE)/run/handler

CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(MODNAME).so:	$(MODNAME).cpp
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

clean:
	rm -f *.o $(MODNAME).so *.log
	cd $(HANDLER) ; make clean
