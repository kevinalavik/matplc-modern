
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
EXECUTE = $(LPLC)/tools

all:	$(EXECUTE)/run/handler $(EXECUTE)/lib/ladder.so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/logic.so:
		cd $(LPLC)/logic/ladder_lib ; make

clean:
	rm -f *.o $(MODNAME).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/logic/ladder_lib; make clean



