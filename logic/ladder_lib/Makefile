MODNAME = ladder
LPLC = ../..
HANDLER = $(LPLC)/lib/lpc
DEST = $(LPLC)/tools/lib


all:	$(DEST)/$(MODNAME).so test_core

#CC = gcc
CFLAGS = -Wall -fPIC -I$(LPLC)/lib -I$(HANDLER)
LIBFLAG = -shared




#
# Makefile
#
# Last Modified: Feb 7, 2003
#

#all:	test_core ../lib/interp.so
tests:	test test_new


CC = g++
#CFLAGS = -Wall -g -rdynamic    # progamming and debugging flags
LIBS = -lm 
#LIBFLAG = -shared -g
#CLIENT = ../lib/process.o
#GLOBAL = ../include/global.h
PARSER = parser.o

#
# Global Routines
#

global.o:	global.cpp global.h
	$(CC) $(CFLAGS) -c global.cpp


#
# The Parser tool
#
PARSER.O = parser.o
parser.o:	parser.cpp parser.h $(GLOBAL)
	$(CC) $(CFLAGS) -c parser.cpp

#
# The function routines
#

FUNCTIONS.O = function.o
function.o:	function.cpp function.h $(GLOBAL)
		$(CC) $(CFLAGS) -c function.cpp


#
# The program step routines
#

PROGRAM_STEP.O = step.o
step.o:	step.cpp step.h $(GLOBAL)
		$(CC) $(CFLAGS) -c step.cpp


#
# The memory store routines
#

MEMORY_STORE.O = store.o
store.o:	store.cpp store.h $(GLOBAL)
		$(CC) $(CFLAGS) -c store.cpp


#
# The plc5 program
#

queue.o:		queue.cpp queue.h
	$(CC) $(CFLAGS) -c queue.cpp 

#
# The program core instruction and processor routines
#

CORE.O = core.o $(FUNCTIONS.O) $(PROGRAM_STEP.O) $(MEMORY_STORE.O) queue.o
core.o:	core.cpp core.h $(FUNCTIONS.O) $(PROGRAM_STEP.O) $(MEMORY_STORE.O) queue.o $(GLOBAL)
		$(CC) $(CFLAGS) -c core.cpp


#
# A Ladder logic front end module
#


LADDER.O = ladder.o $(CORE.O) $(PARSER) io_list.o
ladder.o:	ladder.cpp ladder.h $(CORE.O) $(PARSER)
		$(CC) $(CFLAGS) -c ladder.cpp


#
# IO Scanner
#


IO_LIST.O = io.o
io_list.o:		io_list.cpp io_list.h $(GLOBAL)
		$(CC) $(CFLAGS) -c io_list.cpp


#
# The test program for the new PLC
#
test_core:	test_core.cpp $(LADDER.O)
		$(CC) $(CFLAGS) test_core.cpp -o test_core $(LADDER.O) $(LIBS)



#
# Main Control Program (with IO for the LPC)
#

#../lib/interp.so:	interp.cpp $(LADDER.O) $(GLOBAL)
#		$(CC) $(CFLAGS) -c interp.cpp
#		$(CC) $(LIBFLAG) $(CLIENT) -o ../lib/interp.so interp.o $(LADDER.O)




$(DEST)/$(MODNAME).so:	interp.cpp  $(LADDER.O) $(GLOBAL)
		$(CC) $(CFLAGS) -c interp.cpp
		$(CC) $(LIBFLAG) -o $(DEST)/$(MODNAME).so interp.o $(LADDER.O)





#
# Housekeeping
#

#clean:
#	rm *.o test_core &



clean:
	rm -f *.o $(DEST)/$(MODNAME).so test_core *.log


