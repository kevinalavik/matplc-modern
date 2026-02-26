# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: ready

clean:
	-rm -f io io_socket *.o

LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

#where to find the MatPLC directory
MATPLC = ../..

#get matplc includes (should point to main MatPLC directory)
CFLAGS += -I$(MATPLC)/lib

########################################################################
#CFLAGS += -fno-common -pipe -mpreferred-stack-boundary=2 -march=i586
CFLAGS+=-g `gtk-config --cflags` -I/usr/include/gtkextra
CFLAGS+=-g `gnome-config --cflags gnome gnomeui libglade`
LDFLAGS+=`gnome-config --libs gnome gnomeui libglade`
LDFLAGS+=`gtk-config --libs` -lgtkextra
LARCH_PATH  = -I.
LARCH_PATH += -I../../lib
LARCH_PATH += -I/usr/include/gtkextra
LARCH_PATH += -I/usr/include/gnome-1.0
LARCH_PATH += -I/usr/include/gtk-1.2
LARCH_PATH += -I/usr/include/libglade-1.0
LARCH_PATH += -I/usr/lib/glib/include
LARCH_PATH += -I/usr/include/gnome-xml
LARCH_PATH += -I/usr/include/glib-1.2
LARCH_PATH += -I/usr/lib/gnome-libs/include
LARCH_PATH += -I/usr/lib/glib/include
LARCH_PATH += -I/usr/X11R6/include
########################################################################

all: io io_socket dsp  $(MATPLC)/mmi/hmi_gtk/hmi_gtk

ready: all $(MATPLC)/tools/run/matplc
ready: $(MATPLC)/lib/util/plcshutdown

run: ready
	../../tools/run/matplc -g

io: $(MATPLC)/lib/libmatplc.la
io: $(MATPLC)/lib/logic/timer.o io.o

io_socket: $(MATPLC)/lib/libmatplc.la
io_socket: $(MATPLC)/lib/logic/timer.o io_socket.o

dsp: $(MATPLC)/logic/dsp/dsp

#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

hmi_gtk/%:
	$(MAKE) -C $(@D) $(@F)

#Makefile.depend depend:
#	gcc -MM -MG -I$(MATPLC)/lib *.c \
#		| perl -pe 's/:/ Makefile.depend:/' \
#		> Makefile.depend
#
#include Makefile.depend

kill:
	pkill -9 lt-*
