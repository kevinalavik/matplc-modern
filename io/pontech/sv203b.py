/*
 * (c) 2003 Juan Carlos Orozco
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

import serial
import time

class sv203b:
    "Servomotor driver using board sv203b from pontech" 
    def open(self, port, speed, rtscts, timeout):
        self.ser = serial.Serial(port, speed, rtscts=rtscts, timeout=timeout)
	    
    def close(self):
        self.ser.close()

    def move(self, motor, pos):
        #command = "BD1 SV%d M%d\r" % (motor, pos)
        command = "SV%d M%d\r" % (motor, pos)
        #print command
        self.ser.write(command)

    def incr(self, motor, pos):
        #command = "BD1 SV%d I%d\r" % (motor, pos)
        command = "SV%d I%d\r" % (motor, pos)
        #print command
        self.ser.write(command)

    def ret(self):
        comm = "\r\n"
        self.ser.write(comm)
        x = self.ser.read(3)

    def pos(self, servo):
        mem = 50+servo
        self.ret()
        comm = "RR%d\r\n" % mem
        self.ser.write(comm)
        time.sleep(0.1)
        x = self.ser.inWaiting()
	ret = self.ser.read(x)
	if(not ret):
	    return 0
	else:
	    return int(ret)
        
