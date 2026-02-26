# include the system specific Makefile
include ../../Makefile.$(shell uname)


default: run
run: ready
ready: all
all: hmi_gtk 
#	./demo


install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install hmi_gtk $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/hmi_gtk


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/hmi_gtk


clean:
	rm -f *.o core hmi_gtk Makefile.depend


LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
CFLAGS+=-g `gnome-config --cflags gnome gnomeui`
CFLAGS+=-g `gtk-config --cflags`
CFLAGS+=-g `libglade-config --cflags`
#CFLAGS+= -I/usr/include/gtkextra 
LDFLAGS+=`gnome-config --libs gnome gnomeui`
LDFLAGS+=`gtk-config --libs`
#LDFLAGS+= -lgtkextra
LDFLAGS+=`libglade-config --libs gnome`

MATPLC = ../..

CFLAGS += -I$(MATPLC)/lib



hmi_gtk: $(MATPLC)/lib/libmatplc.la
hmi_gtk: $(MATPLC)/lib/logic/timer.o 
hmi_gtk: hmi_gtk.o

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib $(CFLAGS) *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
