#!/usr/bin/python

# (c) 2002 Juan Carlos Orozco
#
# Offered to the public under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# This code is made available on the understanding that it will not be
# used in safety-critical situations without a full and competent review.
#


import matplc

MODULE_NAME = "sim_tank"

debug = 0

class tank:
    def __init__(self, capacity): # Lts
        self.capacity = capacity 
        self.contains = 0.0 # Lts 
        self.percent = 0.0 # % 
 
    def flow(self, lts): 
        self.contains += lts 
        if self.contains > self.capacity: 
            self.contains = self.capacity 
        if self.contains < 0.0: 
            self.contains = 0.0 
        self.percent = self.contains/self.capacity 
        #print "Flow " + str(lts) + " Contains " + str(self.contains) + " Cap " + str(self.capacity) 
 
    def get_percent(self): 
        return self.percent * 100.0 
 
class valve: 
    def __init__(self, max_flow): # lts/sec 
        self.max_flow = max_flow 
        self.pos = 0.0 # % 
        self.flow = 0.0 # lts/sec 
         
    def set_pos(self, pos): 
        self.pos = pos 
        if self.pos > 99.0: 
            self.pos = 99.0 
        if self.pos < 0.0: 
            self.pos = 0.0 
        self.flow = (self.pos/100.0)*self.max_flow 

    def get_flow(self, T): 
        return self.flow * T 
 
def control(out, real, des, T): 
    P = 5 * T 
    I = 0.1 * T 
    maxInt = 40.0 

    global integ 
     
    err = des - real 
    if debug: 
        print "Out: " + str(out) + " err: " + str(err) 
    integ += I*err 
    if integ > maxInt: 
        integ = maxInt 
    if integ < -maxInt: 
        integ = -maxInt 
    out = 50.0 + err * P + integ 
    if out > 99.0: 
        out = 99.0 
    if out < 0.0: 
        out = 0.0 
    if debug:
    	print "control out %f, err %f, integ %f" % (out, err, integ) 
    return out 
 
def process(): 
    global tank1 
    tank1 = tank(2000.0) 
    global in_valve 
    in_valve = valve(100.0) 
    global out_valve 
    out_valve = valve(100.0) 
    global integ 
    
    integ = 0.0 
    T = 0.5 

    matplc.init(MODULE_NAME)
    p_level = matplc.point_f("level")
    p_level_sp = matplc.point_f("level_sp")
    p_in_valve = matplc.point_f("in_valve")
    p_out_valve = matplc.point_f("out_valve")
    
    print "start process!" 
    while 1: 
        matplc.scan_beg()
        matplc.update()
        pos = p_in_valve.get()
        in_valve.set_pos(float(pos)) 
        if debug: 
            print "in_valve " + str(pos) 
        tank1.flow(in_valve.get_flow(T)) 
        out_valve.set_pos(p_out_valve.get()) 
        if debug: 
            print "in_valve flow " + str(in_valve.get_flow(T)) 
        tank1.flow(-out_valve.get_flow(T)) 
        p_in_valve.set(control(p_in_valve.get(), tank1.get_percent(), p_level_sp.get(), T)) 
        p_level.set(tank1.get_percent())
        matplc.update()
        matplc.scan_end()
 
if __name__=='__main__': 
    process() 
 
     
