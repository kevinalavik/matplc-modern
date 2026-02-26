# include the system specific Makefile
include ../Makefile.$(shell uname)


default: all

all: libmatplc.la libmatplc_setup.la
all: libmatplc-cc.la
all: matplc.a matplc_setup.a
all: matplc-cc.a
all: ./util/plcshutdown


INSTALL_LIBS = libmatplc.la libmatplc_setup.la
install: $(INSTALL_LIBS)
	install -d $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)/ 
	$(LIBTOOL) --mode=install install libmatplc.la $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)/libmatplc.la
	$(LIBTOOL) --mode=install install libmatplc_setup.la $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)/libmatplc_setup.la
	$(LIBTOOL) -n --mode=finish $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)


uninstall:
	rm -f $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)/libmatplc.la
	rm -f $(INSTALL_PREDIR)/$(INSTALL_LIBDIR)/libmatplc_setup.la


clean:
	-rm -f *.o */*.o matplc.a matplc_setup.a
	-rm -f *.lo */*.lo libmatplc.la .libs/* .libs/*/*
	-rm -f ./util/plctest ./util/matplc ./util/plcshutdown
	-find . -name ".libs" | xargs rm -fr
	-find . -name Makefile.depend | xargs rm -f


#get warnings, debugging information and optimization
CFLAGS  = -Wall -Wpointer-arith -Wstrict-prototypes -Wwrite-strings
# CFLAGS += -Werror
# Note: if the optimizer crashes, we'll leave out the -O3 for those files
CFLAGS += -ggdb -O3 -funroll-loops -fPIC
CFLAGS += -I.
CFLAGS += $(PLCSYSTEM_CFLAGS)

CXXFLAGS = -I. -fPIC
CXXFLAGS += $(PLCSYSTEM_CFLAGS)

# list of object files ordinary modules should link against
PLC_LIB  = plc
PLC_LIB += gmm/gmm gmm/gmm_local gmm/gmm_isolate gmm/gmm_shared
PLC_LIB += gmm/protocol gmm/plcproxy
#PLC_LIB += misc/sin_util misc/shmem_util misc/string_util misc/sem_util
PLC_LIB += misc/sin_util misc/shmem_util misc/string_util 
PLC_LIB += misc/mutex_util misc/signal_util misc/timer_util misc/daemon_util
PLC_LIB += conffile/conffile
PLC_LIB += log/log
PLC_LIB += cmm/cmm
PLC_LIB += synch/synch synch/synch_sem
PLC_LIB += period/period
PLC_LIB += state/state
PLC_LIB += rt/rt
PLC_LIB += gnu/getdelim gnu/getline


#list of object files required to setup the plc
PLCSETUP_LIB  = plc_setup
PLCSETUP_LIB += log/log_setup conffile/conffile_setup
PLCSETUP_LIB += cmm/cmm_setup gmm/gmm_setup rt/rt_setup
PLCSETUP_LIB += synch/synch_setup synch/synch_sem
PLCSETUP_LIB += period/period_setup
PLCSETUP_LIB += state/state_setup


# list of object files required for the C++ interface to the MatLC
PLC-CC_LIB  = plc-cc



# turn them into a library for convenience
matplc.a: $(addsuffix .o, $(PLC_LIB))
	$(AR) rsuv $@ $?

matplc_setup.a: $(addsuffix .o, $(PLCSETUP_LIB))
	$(AR) rsuv $@ $?

matplc-cc.a: $(addsuffix .o, $(PLC-CC_LIB))
	$(AR) rsuv $@ $?



# shared libraries (using libtool)
libmatplc.la: $(addsuffix .lo, $(PLC_LIB)) plc.symbols
	$(LIBTOOL) --mode=link $(CC) -o $@ -rpath $(INSTALL_LIBDIR) $^

libmatplc-cc.la: $(addsuffix .lo, $(PLC-CC_LIB)) plc.symbols
	$(LIBTOOL) --mode=link $(CC) -o $@ -rpath $(INSTALL_LIBDIR) $^

libmatplc_setup.la: $(addsuffix .lo, $(PLCSETUP_LIB)) plc.symbols
	$(LIBTOOL) --mode=link $(CC) -o $@ -rpath $(INSTALL_LIBDIR) $^

#how to make things in subdirectories etc
../% /% conffile/% log/% misc/% cmm/% util/% gmm/% synch/% misc/%:
	$(MAKE) -C $(@D) $(@F)

# make a shareable object from C source
%.lo: %.c
	$(LIBTOOL) --mode=compile $(COMPILE.c) $(OUTPUT_OPTION) $<

# make a shareable object from C++ source
%.lo: %.cc
	$(LIBTOOL) --mode=compile $(COMPILE.cc) $(OUTPUT_OPTION) $<

Makefile.depend depend:
	$(CC) -MM -MG -I. $(PLCSYSTEM_CFLAGS) *.c \
	  | perl -pe 's/:/ Makefile.depend:/' > Makefile.depend

-include Makefile.depend
