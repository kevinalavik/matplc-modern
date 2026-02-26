SHELL		= /bin/sh

srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
libdir		= $(exec_prefix)/lib
includedir	= $(exec_prefix)/include


CC = gcc


CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
CFLAGS += -ggdb -O3 -funroll-loops

LPLC = ../../..
LLIB = $(LPLC)/lib

CFLAGS += -I$(LLIB)


all: mb_rtu.o mb_master.o

mb_rtu.o: mb_rtu.c
mb_master.o: mb_master.c



clean:
	-rm -rf *.o


../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend:
	gcc -MM -MG -I$(LLIB) *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend

