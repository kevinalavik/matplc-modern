
MODNAME = arguments
LPLC = ../../../
HANDLER = $(LPLC)/lib/lpc
DEST = .

all:	$(MODNAME).so $(EXECUTE)/run/handler

CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(MODNAME).so:	$(MODNAME).cpp
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

clean:
	cd $(HANDLER) ; make clean
	rm -f *.o $(MODNAME).so *.log
