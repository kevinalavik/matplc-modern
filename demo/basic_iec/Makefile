# include the system specific Makefile
include ../../Makefile.$(shell uname)

default: ready

clean:
	-rm -f chaser_st chaser_st.cc chaser_il chaser_il.cc kbd *.o Makefile.depend *.log

LDFLAGS = $(PLCSYSTEM_LDFLAGS)

#use libtool for linking
LINK.o = libtool $(CC) $(LDFLAGS) $(TARGET_ARCH)

#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
CFLAGS += -ggdb -O3 -funroll-loops
# Note: if the optimizer crashes, we'll leave out the -O3 for those files

#where to find the MatPLC directory
MATPLC = ../..

#get matplc includes (should point to main MatPLC directory)
CFLAGS += -I$(MATPLC)/lib

all: chaser_st
all: chaser_il

ready: all $(MATPLC)/tools/run/matplc $(MATPLC)/mmi/curses/vitrine
ready: $(MATPLC)/lib/util/plcshutdown
ready: ../basic/kbd
#ready: $(MATPLC)/logic/iec/iec2cc

# the next line is actually optional; it'll work without it (somewhat less
# nicely)
#ready: $(MATPLC)/mmi/kbd/linuxkbd

run: ready
	./demo


chaser_st: $(MATPLC)/lib/libmatplc.la chaser_st.o
	$(CXX) -o chaser_st chaser_st.o $(MATPLC)/lib/matplc.a -lrt

chaser_st.o: chaser_st.cc
	$(CXX) -c chaser_st.cc -I$(MATPLC)/logic/iec/stage4/generate_cc -I$(MATPLC)/lib

chaser_st.cc: chaser_st.st $(MATPLC)/logic/iec/iec2cc
	$(MATPLC)/logic/iec/iec2cc chaser_st.st -I ../../logic/iec/lib > chaser_st.cc



chaser_il: $(MATPLC)/lib/libmatplc.la chaser_il.o
	$(CXX) -o chaser_il chaser_il.o $(MATPLC)/lib/matplc.a -lrt


chaser_il.o: chaser_il.cc
	$(CXX) -c chaser_il.cc -I$(MATPLC)/logic/iec/stage4/generate_cc -I$(MATPLC)/lib

chaser_il.cc: chaser_il.il $(MATPLC)/logic/iec/iec2cc
	$(MATPLC)/logic/iec/iec2cc chaser_il.il -I ../../logic/iec/lib > chaser_il.cc




#how to make things from other directories if they are missing
../% /%:
	$(MAKE) -C $(@D) $(@F)

Makefile.depend depend: chaser_st.cc
	gcc -MM -MG -I$(MATPLC)/lib *.c \
		| perl -pe 's/:/ Makefile.depend:/' \
		> Makefile.depend

include Makefile.depend
