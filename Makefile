# include the system specific Makefile
include Makefile.$(shell uname)

default: all
	@echo
	@echo 'For a demo of the MatPLC, change to demo/oven_gtk'
	@echo '                             and run the "demo" script.'

# make something everywhere (ie, in all Makefiles that have that target)
clean depend: 
	find . -depth -mindepth 2 -name Makefile -printf %h\\n | xargs -i make -C{} $@

# make all (well, most, anyway)
# first, the library
all: lib/libmatplc.la
# the main executables
all: tools/run/all
# the lpc executables
all: lib/lpc/all
# logic modules
all: logic/dsp/all logic/iec/all logic/il/il2c logic/ladder_lib/all
# I/O modules
#all: io/comedi/all io/modbus/all io/dio48/all io/parport/all io/simple-udp/all
all: io/modbus/all io/dio48/all io/parport/all io/simple-udp/all io/das08_lib/all
# mmi modules
#all: mmi/curses/all mmi/hmi_gtk/all
all: mmi/curses/all mmi/curses_lib/all mmi/kbd_lib/all
# misc module
all: lib/util/plcshutdown
# communication modules
all: comm/network_io/all comm/tcpip_lib/all comm/rs232c_lib/all
# service modules
all: service/email_lib/all service/matd_lib/all
# some of the demos
#all: demo/basic/ready demo/oven_gtk/ready
all: demo/basic/ready demo/lpc/all

all:
	@echo
	@echo "Note: some of the I/O modules (eg abel) are not included in 'make all'"
	@echo "      since they depend on libraries you may or may not have."
	@echo "      Some other minor modules are also excluded."
	@echo "      Please make them individually if you need them."
	@echo
	@echo "      Support for the python and tcl languages has also not"
	@echo "      been installed. To do this, please follow the"
	@echo "      instructions in the manual."
	@echo
	@echo "      If you wish to run a demo, change to one of the demo"
	@echo "      directories (eg. demo/oven_gtk) and run the script"
	@echo "      './demo'"


#clean:
#	rm -f config.status config.cache config.log

#how to make things in subdirectories etc
lib/% tools/% logic/% io/% mmi/% comm/% service/% demo/%:
	$(MAKE) -C $(@D) $(@F)
