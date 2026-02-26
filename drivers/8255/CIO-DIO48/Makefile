# Generated automatically from Makefile.in by configure.
#
#	
#	Makefile for building:
#
#		adc.o:    CIO-DAC A/D adapater Linux loadable module.
#		adcread:  Program to test adc.o.
#
#               Copyright (C) May 19, 1995
#               Written by:  Warren J. Jasper and Samual Moore
#                            North Carolina State Univerisity
#
#
# Set default major device and base address here if different from defaults
# (default MAJOR_DEV = 31, default BASE_ADDR = 0x300)
#
MAJOR_DEV=31
BASE_ADDR=0x300
ID=CIO-DIO48
CHANNELS=2
#
#
VERSION=1.0a
DIST_NAME=CIO-DIO48.$(VERSION).tar
SRCS	= ciodio48.c test/diotest.c
HEADERS = dio48.h
OBJS	= ciodio48.o
CFLAGS	= -DADAPTER_ID=\"$(ID)\" -D__KERNEL__ -DMODULE \
	  -DDEFAULT_MAJOR_DEV=$(MAJOR_DEV) -DBASE_REG=$(BASE_ADDR) \
	  -Wall -O6 -fomit-frame-pointer -m486 
LDFLAGS = -s -N
CC=gcc
BINDIR=/sbin

TARGETS=dio48.o test/diotest

# For the moment, do not compile this. It must be upgraded to support new kernel versions...
#all: $(TARGETS)
all:

dio48.o: $(OBJS) $(KERNEL_VERSION)
	$(LD) -r -o $@ $(OBJS) 

dio48.S: $(OBJS) $(KERNEL_VERSION)
	echo "Done"

test/diotest:	test/diotest.c
	$(CC) -Wall -g -o $@ $@.c

clean:
	rm -f *.o test/*.o \#* *~ $(TARGETS)

dist:
	tar cfv $(DIST_NAME) $(SRCS) $(HEADERS) Makefile README ModList
	/bin/gzip -f $(DIST_NAME)

install: 
	-/sbin/rmmod dio48
	-/sbin/insmod -f dio48.o
	-/bin/cp ./dio48.h /usr/local/include/dio48.h
	-/bin/chmod 644 /usr/local/include/dio48.h 

devices:
	-/bin/mkdir /dev/dio48
	-/bin/mknod /dev/dio48/dio0 c $(MAJOR_DEV) 0
	-/bin/mknod /dev/dio48/dio1 c $(MAJOR_DEV) 1
	-@if [ $(ID) = CIO-DIO96 -o $(ID) = CIO-DIO192 ]; then	\
		/bin/mknod /dev/dio48/dio2 c $(MAJOR_DEV) 2;	\
		/bin/mknod /dev/dio48/dio3 c $(MAJOR_DEV) 3;	\
	fi
	-@if [ $(ID) = CIO-DIO192 ]; then			\
		/bin/mknod /dev/dio48/dio4 c $(MAJOR_DEV) 4;	\
		/bin/mknod /dev/dio48/dio5 c $(MAJOR_DEV) 5;	\
	fi
	-/bin/chmod 666 /dev/dio48/dio*

