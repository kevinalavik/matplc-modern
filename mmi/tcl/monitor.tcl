#!/bin/sh
# The next line is executed by /bin/sh, not wish \
exec wish $0 ${1+"$@"}

# this script connects to the running MatPLC, and displays the names and
# current values of all its points in a pair of columns.  it will work with
# whatever MatPLC is running, as long as the matplc.conf is correct.  however,
# it has no special handling for a list of points too long to fit onscreen.

lappend auto_path /usr/local/lib
package require lplc
lplc init -module monitor.tcl -array plc 

label .l -text PLC
pack .l -side top -pady 5

# attach the value elements of the plc() array to display widgets (labels)

frame .f
pack .f -side top
for {set x 0} {$x < $plc(count)} {incr x} {
	label .f.l$x -text $plc($x,name)
	label .f.e$x -textvariable plc($x,value) -width 8 -relief sunken
	grid .f.l$x .f.e$x -sticky ew
}

frame .btns
pack .btns -side top
button .btns.exit -text Exit -command {
	global timer

	catch {after cancel $timer}
	lplc done
	exit
}
pack .btns.exit

# every 1/4 second or so, update the plc() array, so that current values
# are reflected onscreen.

proc do_update {} {
	global plc timer

	lplc update -array plc
	set timer [after 250 [list do_update]]
}

do_update
