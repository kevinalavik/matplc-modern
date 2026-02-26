## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	@GTK_CFLAGS@

bin_PROGRAMS = config-edit

config_edit_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	conf_utils.c conf_utils.h

config_edit_LDADD = @GTK_LIBS@ $(INTLLIBS) \
        ../../../lib/conffile/conffile.o \
        ../../../lib/misc/string_util.o \
        ../../../lib/log/log.o