#
# Makefile for the parallel port DCF77 radio clock driver for Linux
#

NAME := plc_parport

PREFIX := /usr
MODULES := $(NAME).o
MAN_PAGE := $(NAME).4
MAN_ROOT = $(shell test -d $(PREFIX)/share/man && echo $(PREFIX)/share/man || echo $(PREFIX)/man )
MAN_DIR = $(MAN_ROOT)/man4
PLC_PARPORT_MAJOR := 240

KSRC ?= /usr/src/linux

LINUX_HEADERS := $(shell test -d $(KSRC)/include && echo $(KSRC)/include || echo /usr/include)

KVERS ?= $(shell sed -n 's/.*REL.*"\(.*\)"/\1/p' $(LINUX_HEADERS)/linux/version.h)

MOD_DIR ?= $(DESTDIR)/lib/modules/$(KVERS)/misc

# Uncomment the third line on SMP systems if the magic invocation fails.
SMP := $(shell if grep -q '^[[:space:]]*\#define[[:space:]]*CONFIG_SMP[[:space:]]*1' $(LINUX_HEADERS)/linux/autoconf.h ; \
               then echo 1; else echo 0; fi)
#SMP := 0
#SMP := 1

# We need to know whether CONFIG_MODVERSIONS is defined.
# Uncomment the second or third line if the magic invocation fails.
MODVER := $(shell if cat $(LINUX_HEADERS)/linux/autoconf.h 2>/dev/null | grep -q '^[[:space:]]*\#define[[:space:]]*CONFIG_MODVERSIONS[[:space:]]*1'; then echo 1; else echo 0; fi)
#MODVER := 0
#MODVER := 1

CFLAGS := -O2 -Wall -fomit-frame-pointer -I$(LINUX_HEADERS) -D__KERNEL__ -DMODULE -DPLC_PARPORT_MAJOR=$(PLC_PARPORT_MAJOR)

ifeq ($(SMP),1)
CFLAGS += -D__SMP__
endif

ifeq ($(MODVER),1)
#CFLAGS += -DMODVERSIONS -include $(LINUX_HEADERS)/linux/modversions.h
CFLAGS += -DMODVERSIONS
endif

default: print_help
all: $(MODULES)

install-all: install install-man install-devs
clean-all: clean unload-modules

modules: $(MODULES)
	@echo Compiling for Linux kernel Ver $(KVERS)

print_help:
	@echo
	@echo "make [install-all | modules | install | install-man | install-devs | load-modules | clean | unload-modules]"
	@echo "install-all    : modules, install, install-man, install-devs, load-modules"
	@echo "clean-all      : clean, unload-modules"
	@echo
	@echo "clean          : remove intermediate files"
	@echo "modules        : compile the modules"
	@echo "install        : install the modules"
	@echo "install-man    : install the man pages (if available ;-] )"
	@echo "install-devs   : make the device files under /dev"
	@echo "load-modules   : load (insert) the modules into the kernel"
	@echo "unload-modules : unload (remove) the modules from the kernel"
	@echo
	@echo "NOTE: installing and loading has to be run as root."
	@echo
	
clean:
	rm -f $(MODULES) $(MAN_PAGE).gz *~

	
install: $(MODULES)
	install -d -o root -g root -m 755 $(MOD_DIR)
	install -o root -g root -m 644 $(MODULES) $(MOD_DIR)

install-man:
	install -d -o root -g root -m 755 $(MAN_DIR)
	gzip -c $(MAN_PAGE) >$(MAN_PAGE).gz
	install -o root -g root -m 644 $(MAN_PAGE).gz $(MAN_DIR)

install-devs:
	mknod -m 666 /dev/plc_parport0 c $(PLC_PARPORT_MAJOR) 0
	mknod -m 666 /dev/plc_parport1 c $(PLC_PARPORT_MAJOR) 1
	mknod -m 666 /dev/plc_parport2 c $(PLC_PARPORT_MAJOR) 2

load-modules:
	insmod plc_parport

unload-modules:
	rmmod plc_parport

.PHONY: all install-all clean install install-man install-devs
