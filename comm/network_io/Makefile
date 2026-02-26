
all: network_io.o

LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
GLOBAL = $(HANDLER)/global.h
LIBFLAG = -shared
DEPS = network_io.o
CC = g++
CFLAGS = -Wall -fPIC -I$(HANDLER)

clean:
	rm *.o

#
#

network_io.o: network_io.cpp network_io.h $(GLOBAL)
		$(CC) $(CFLAGS) -c network_io.cpp -o network_io.o
