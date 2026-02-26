
MODNAME = ladder_test
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc

all:	$(MODNAME).so $(EXECUTE)/run/handler $(EXECUTE)/lib/ladder.so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/ladder.so:
		cd $(LPLC)/logic/ladder_lib ; make

$(MODNAME).so:	$(MODNAME).c
		$(CC) $(CFLAGS) -c $(MODNAME).c
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

clean:
	rm -f *.o $(MODNAME).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/logic/ladder_lib; make clean
