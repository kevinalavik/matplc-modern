# include the system specific Makefile
include ../../Makefile.$(shell uname)



# remove the all for now, as this code is not yet complete...
#all:	plc5
all:
	echo Skipping PLC5. Code not yet finished...

#tests:	test_new test_io test testdaq embedded shared

#where to find the MatPLC directory
MATPLC = ../..

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
CFLAGS += -I$(MATPLC)/lib

CXXFLAGS = $(CFLAGS)


LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = $(LIBTOOL) --mode=link $(CC) -rpath $(INSTALL_LIBDIR) $(LDFLAGS) $(TARGET_ARCH)


LIBS = -lcrypt
LIBS += $(LDFLAGS)

#
# Global Routines
#

global.o:	global.cpp global.h


#
# The Parser tool
#
PARSER.O = parser.o
parser.o:	parser.cpp parser.h global.h


#
# The Comments and symbols tool
#
COMMENTS.O = comments.o
comments.o:	comments.cpp comments.h global.h
	$(CC) $(CFLAGS) -c comments.cpp


#
# The memory store routines
#

MEMORY_STORE.O = store.o
store.o:	store.cpp store.h global.h
		$(CC) $(CFLAGS) -c store.cpp


#
# Memory Addresses and Program Memory
#

#MEMORY_ADDRESS.O = memory_address.o $(COMMENTS.O)
#memory_address.o:	memory_address.cpp memory_address.h global.h
#	$(CC) $(CFLAGS) -c memory_address.cpp

PROGRAM_IL.O = program_il.o $(MEMORY_STORE.O) $(COMMENTS.O)
program_il.o:	program_il.cpp program_il.h $(MEMORY_STORE.O) $(COMMENTS.O)
	$(CC) $(CFLAGS) -c program_il.cpp

PROGRAM.O = program.o $(PROGRAM_IL.O)
program.o:	program.cpp program.h $(PROGRAM_IL.O)
	$(CC) $(CFLAGS) -c program.cpp


#
# The plc5 program
#

queue.o:		queue.cpp queue.h
	$(CC) $(CFLAGS) -c queue.cpp 

instructions.o: instructions.cpp instructions.h program_il.h
	$(CC) $(CFLAGS) -c instructions.cpp 

PLC5.O = $(PROGRAM.O) $(PARSER.O) plc5.o instructions.o queue.o
plc5.o: plc5.cpp plc5.h $(MEMORY.O) $(PROGRAM.O) $(PARSER.O) instructions.o queue.o
	$(CC) $(CFLAGS) -c plc5.cpp


#
# A shared memory manager module
#

#shared:	shared.cpp shared.h global.h
#	$(CC) $(CFLAGS) shared.cpp -o shared

#
# IO Scanner
#

#network.o:	network.cpp network.h global.h
#		$(CC) $(CFLAGS) -c network.cpp

#serial.o:	serial.cpp serial.h global.h
#		$(CC) $(CFLAGS) -c serial.cpp

#das08.o:	das08.cpp das08.h
#		$(CC) $(CFLAGS) -c das08.cpp

#testdaq:	testdaq.cpp das08.o
#		$(CC) $(CFLAGS) testdaq.cpp -o testdaq das08.o global.o

#IO.O = io.o serial.o network.o das08.o
IO.O = io.o
#io.o:		io.cpp io.h global.h serial.h network.h
io.o:		io.cpp io.h global.h
		$(CC) $(CFLAGS) -c io.cpp

#test_io:test_io.cpp $(IO.O)
#		$(CC) $(CFLAGS) test_io.cpp $(IO.O) -o test_io $(LIBS) global.o


#
# Main Control Program
#
CONTROLLER.O = controller.o $(IO.O) $(PLC5.O) global.o
controller.o:	controller.cpp controller.h $(IO.O) $(PLC5.O)
		$(CC) $(CFLAGS) -c controller.cpp


#
# GUI Support programs
#
#x_gui_lowlevel.o:	x_gui_lowlevel.cpp x_gui_lowlevel.h
#		$(CC) $(XFLAGS) -c x_gui_lowlevel.cpp

#gui_memory.o:	gui_memory.cpp gui_memory.h x_gui_lowlevel.o $(CONTROLLER.O)
#		$(CC) $(XFLAGS) -c gui_memory.cpp

#gui_ladder.o:	gui_ladder.cpp gui_ladder.h x_gui_lowlevel.o $(CONTROLLER.O)
#		$(CC) $(XFLAGS) -c gui_ladder.cpp

#gui_api.o:	gui_api.cpp gui_api.h gui_ladder.o gui_memory.o
#		$(CC) $(XFLAGS) -c gui_api.cpp

#x_gui.o:	x_gui.cpp x_gui.h
#		$(CC) $(XFLAGS) -c x_gui.cpp

#GUI = x_gui_lowlevel.o gui_api.o gui_ladder.o gui_memory.o x_gui.o $(CONTROLLER.O)



#
# Main Test Program
#
#test:		test.cpp $(CONTROLLER.O)
#		$(CC) $(CFLAGS) test.cpp -o test $(CONTROLLER.O) $(LIBS)

#
# Embedded Controller Program
#
#embedded:	embedded.cpp $(CONTROLLER.O)
#		$(CC) $(CFLAGS) embedded.cpp -o embedded $(CONTROLLER.O) $(LIBS)

#
# Main GUI interface
#

#LPCedit:	LPCedit.cpp LPCedit.h $(GUI)
#		$(CC) $(XFLAGS) LPCedit.cpp -o LPCedit $(GUI) $(XLIBS)

#
# All those funny java programs
#


#Io.class:	Io.java
#	javac $(JFLAGS) -deprecation Io.java

#Xml.class:	Xml.java
#	javac $(JFLAGS) -classpath jaxp.jar:parser.jar -deprecation Xml.java

#Message.class:	Message.java
#	javac $(JFLAGS) -deprecation Message.java

#LPCmessage.class:	LPCmessage.java Message.class Xml.class Io.class
#	javac $(JFLAGS) -deprecation LPCmessage.java

#LPCedit.class:	LPCedit.java
#	javac $(JFLAGS) -deprecation LPCedit.java

#LPCconnect.class:	LPCconnect.java Message.class
#	javac $(JFLAGS) -deprecation LPCconnect.java

#
# The test program for the new PLC
#

#test_new:	test_new.cpp $(CONTROLLER.O)
#		$(CC) $(CFLAGS) test_new.cpp -o test_new $(CONTROLLER.O) $(LIBS)


#
# Main Daemon for the LPC
#

#lpcd: 	lpcd.cpp $(CONTROLLER.O)
#		$(CC) $(CFLAGS) lpcd.cpp -o lpcd $(CONTROLLER.O) $(LIBS)

#
# The main plc5 interpreter program
#
PLC_LIBS  = ../../lib/libmatplc.la

plc5: plc5_interpreter.o $(CONTROLLER.O) $(PLC_LIBS) $(CONTROLLER.O) $(LIBS)

#
# Housekeeping
#

clean:
	rm *.o plc.log

install: all
	install -d $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/ 
	$(LIBTOOL) --mode=install install plc5 $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/plc5


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_BINDIR)/plc5


