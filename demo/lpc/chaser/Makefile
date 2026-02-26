
MODNAME = chaser
LPLC = ../../..
HANDLER = $(LPLC)/lib/lpc
EXECUTE = $(LPLC)/tools

all:	$(MODNAME).so libraries $(EXECUTE)/run/handler $(EXECUTE)/lib/kbd.so $(EXECUTE)/lib/vitrine.so

CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared

$(EXECUTE)/run/handler:
		cd $(HANDLER) ; make

$(EXECUTE)/lib/kbd.so:
		cd $(LPLC)/mmi/kbd_lib ; make

$(EXECUTE)/lib/vitrine.so:
		cd $(LPLC)/mmi/curses_lib ; make

$(MODNAME).so:	$(MODNAME).c
		$(CC) $(CFLAGS) -c $(MODNAME).c
		$(CC) $(LIBFLAG) -o $(MODNAME).so $(MODNAME).o

libraries:
	cd $(LPLC)/mmi/curses_lib ; make
	cd $(LPLC)/mmi/kbd_lib ; make

clean:
	rm -f *.o $(MODNAME).so *.log
	cd $(HANDLER) ; make clean
	cd $(LPLC)/mmi/curses_lib; make clean
	cd $(LPLC)/mmi/kbd_lib; make clean


