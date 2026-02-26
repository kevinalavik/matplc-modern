LPLC = ../..
LIBS = $(LPLC)/lib
EXECUTE = $(LPLC)/tools/run
HANDLER = ../..
TOOLS = $(LPLC)/tools


default: ready
all: $(EXECUTE)/handler
ready: all



CC = g++
CFLAGS  = -Wall -rdynamic -Wpointer-arith -Wwrite-strings -I$(LIBS) 
##-I$(HANDLER)/src/include
#CFLAGS  += -lobjc
LDFLAGS = -lrt -ldl -lm

client_queue_t.o: client_queue_t.cpp client_queue_t.h
	$(CC) $(CFLAGS) -c client_queue_t.cpp

server_queue_t.o: server_queue_t.cpp server_queue_t.h
	$(CC) $(CFLAGS) -c server_queue_t.cpp

queue_t.o: queue_t.cpp queue_t.h
	$(CC) $(CFLAGS) -c queue_t.cpp

handler_t.o: handler_t.cpp handler_t.h
	$(CC) $(CFLAGS) -c handler_t.cpp

PIECES = handler_t.o server_queue_t.o client_queue_t.o queue_t.o

MATPLCLIB = $(LPLC)/lib/matplc.a
$(MATPLCLIB):
	cd $(LIBS); make

$(EXECUTE)/handler: handler.cpp $(PIECES) $(MATPLCLIB)
	$(CC) $(CFLAGS) handler.cpp $(PIECES) -o handler $(MATPLCLIB) $(LDFLAGS)
	strip handler
	mv handler $(EXECUTE)/handler
	mkdir -p $(TOOLS)/lib

clean:
	rm -f *.o $(EXECUTE)/handler
#	-rm -f $(HANDLER)/lib/handler handler *.o
