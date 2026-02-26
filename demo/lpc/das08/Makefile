
MODNAME = logic
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc

all:	$(MODNAME).so $(EXECUTE)/run/handler $(EXECUTE)/lib/das08.so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/das08.so:
		cd $(LPLC)/io/das08_lib ; make

$(MODNAME).so:	$(MODNAME).c
		$(CC) $(CFLAGS) -c $(MODNAME).c
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

clean:
	cd $(HANDLER) ; make clean
	cd $(LPLC)/io/das08_lib; make clean
	rm -f *.o $(MODNAME).so *.log

