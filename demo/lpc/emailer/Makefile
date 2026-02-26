
MODNAME = emailer
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
DEST = .

all:	$(MODNAME).so $(EXECUTE)/run/handler $(EXECUTE)/lib/email.so 

CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/email.so:
		cd $(LPLC)/service/email_lib ; make

$(MODNAME).so:	$(MODNAME).cpp
		$(CC) $(CFLAGS) -c $(MODNAME).cpp
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

clean:
	rm -f *.o $(MODNAME).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/service/email_lib; make clean
