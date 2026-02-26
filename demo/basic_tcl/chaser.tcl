#!/bin/sh
# The next line is executed by /bin/sh, not wish \
exec wish $0 ${1+"$@"}

	# on my system, the Tcl/Tk interpreters do not include /usr/local/lib
	# (where the extension is installed) in the auto_path
	# (search path for loadable modules)

lappend auto_path /usr/local/lib

	# we need the MatPLC extension to Tcl

package require lplc

	# connect to the MatPLC, and get the point config into the "plc" array
	# note: we specify module "Kbd" so we get write permission on the 
	# "left" and "right" points.

#lplc init -module Kbd -array plc --PLCdebug=0
lplc init -array plc --PLCmodule=Kbd

	# the connection gives us names, indexed by position.
	# i like to code using names, so i need to be able to look up the position 
	# (see the labelling of the "lights" below)

for {set x 0} {$x < $plc(count)} {incr x} {
	set lookup($plc($x,name)) $x
}

	# this GUI is hard-coded with the names it expects in the config file.
	# if you want to change the config file and the chaser.c program to
	# use a different number of lights, change this list to match the config.

	# when the lplc extension provides access to the complete loaded
	# matplc.conf, we can use it to configure the GUI 

#set lights [list L1 L2 L3 L4 L5 L6]
set lights [list L1 L2 L3 L4]

	# the GUI is built from basic Tk widgets (frames, labels, and buttons)

label .l -text Chaser
pack .l -side top -pady 5

frame .f
pack .f -side top
foreach p $lights {
	set widgets($p) [label .f.l$p -text $p]
	pack .f.l$p -side left -padx 5
}

frame .btns
pack .btns -side top -pady 5
button .btns.left -text Left -command {
	lplc setpt -name left -value 1
	lplc setpt -name right -value 0
	.btns.left configure -relief sunken
	.btns.right configure -relief raised
}
button .btns.right -text Right -command {
	lplc setpt -name left -value 0
	lplc setpt -name right -value 1
	.btns.left configure -relief raised
	.btns.right configure -relief sunken
}
button .btns.exit -text Exit -command {
	global timer

	catch {after cancel $timer}

	lplc setpt -name quit -value 1

# for reasons i have yet to determine, this causes wish to segfault
#	lplc done

	exit
}
pack .btns.left .btns.right -side left -padx 3
pack .btns.exit -side left -padx 10

	# every so often (1/10 second) we update our copy of PLC memory,
	# and reflect the current value of the interesting points onscreen

	# we update so frequently because we know the config file tells the
	# the chaser program to update values every 1/5 second.

proc do_update {} {
	global plc lookup lights widgets timer

	lplc update -array plc
	foreach p $lights {
		if $plc($lookup($p),value) {
			$widgets($p) configure -bg green
		} else {
			$widgets($p) configure -bg gray
		}
	}
	set timer [after 100 [list do_update]]
}

	# start the periodic update cycle

do_update

	# and make the buttons reflect the current direction

if $plc($lookup(right),value) {
	.btns.right invoke
} else {
	.btns.left invoke
}
