# include the system specific Makefile
include ../../Makefile.$(shell uname)


default: run
run: ready
ready: all
all: hmi_gtk2 
#	./demo


install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/
	$(LIBTOOL) --mode=install install hmi_gtk2 $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/hmi_gtk2


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/hmi_gtk2


clean:
	rm -f *.o core hmi_gtk2 Makefile.depend



LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)



#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
##CFLAGS+=-g `gnome-config --cflags gnome gnomeui`
#CFLAGS+=-g `pkg-config --cflags libgnome-2.0 libgnomeui-2.0`
##CFLAGS+=-g `gtk-config --cflags`
CFLAGS+=-g `pkg-config --cflags gtk+-2.0`
##CFLAGS+=-g `libglade-config --cflags` -I/usr/include/gtkextra 
CFLAGS+=-g `pkg-config --cflags libglade-2.0` 


##LDFLAGS+=`gnome-config --libs gnome gnomeui`
#LDFLAGS+=`pkg-config --libs libgnome-2.0 libgnomeui-2.0`
##LDFLAGS+=`gtk-config --libs` -lgtkextra
LDFLAGS+=`pkg-config --libs gtk+-2.0`
##LDFLAGS+=`libglade-config --libs gnome`
LDFLAGS+=`pkg-config --libs libglade-2.0`

MATPLC = ../..

CFLAGS += -I$(MATPLC)/lib



hmi_gtk2: $(MATPLC)/lib/libmatplc.la
hmi_gtk2: $(MATPLC)/lib/logic/timer.o 
hmi_gtk2: hmi_gtk2.o

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(MATPLC)/lib $(CFLAGS) *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
