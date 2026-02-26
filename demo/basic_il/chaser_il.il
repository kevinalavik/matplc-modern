### chaser program ###
#
# author: Mario de Sousa
# file:   chaser_il.il
#
# This program serves as an example of what is possible to
# to with the _very_ simple instruction language (il)
# compiler.
#
# This program has the same functionality as the chaser.c
# program, but it is written in il, which will then get
# compiled into c code by the il2c perl script.
# The c code is then further compiled and linked to the
# PPLC libraries to generate a PPLC module.
#
# (Changed 7 Jun 2001 to use subroutines instead of jumps --Jiri)


# We use first_scan to verify if it is the first time
# the program is running, and therefore set the initial
# 'light'.
#
# Every plc point, including first_scan, will be set to 0
# on plc_setup, so if it is 0, we know it is the first
# scan. We set it to 1 forever after...
LDI first_scan
SET L1
SET first_scan


# We use a status flag to remember our current
#  scrolling direction.
# scroll_dir = 0 -> scroll right
# scroll_dir = 1 -> scroll left
LD left
SET scroll_dir

LD right
RST scroll_dir


# Now we do the scrolling
LD scroll_dir
JSR scroll_left
LDI scroll_dir
JSR scroll_right


# Here are the subroutines
SUB scroll_right
LD L4
OUT tmp_flag
LD L3
OUT L4
LD L2
OUT L3
LD L1
OUT L2
LD tmp_flag
OUT L1
END

SUB scroll_left
LD L1
OUT tmp_flag
LD L2
OUT L1
LD L3
OUT L2
LD L4
OUT L3
LD tmp_flag
OUT L4
END

###
