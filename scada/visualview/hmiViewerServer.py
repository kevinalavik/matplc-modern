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
 
# A server to comunicate with the hmiViewer applet of the
# VISUAL project (visual.sourceforge.net). This server uses threads
#
# TODO:
#  - Use matplc.scan_beg, matplc.scan_end instead of time.sleep in mat_io.
#  - Get point names and type from MATplc.

import SocketServer 
import string 
import time 
import thread 
import socket 
import matplc

HOST = ''                 # Local host symbolic name
PORT = 50008              # Selected port number

MODULE_NAME = 'hmiViewerServer' # Choose a module name for MATplc

global points 
global lock
points = {}

debug = 0 

class point: 
    def set(self, val):
    	lock.acquire()
        self.value = val
	lock.release()
    def get(self): 
        if self.value == None:
            return 0.0
        return self.value
    def set_matpoint(self, matpoint):
        self.matpoint = matpoint
    def get_matpoint(self):
        try:
            ret = self.matpoint
        except AttributeError:
            ret = None
        return ret
 
def set_point(name, value): 
    try: 
        p = points[name] 
        p.set(value)
    except KeyError: 
        p = point() 
        p.set(value) 
        points[name] = p 
 
def get_point(name): 
    try: 
        p = points[name] 
        return p.get() 
    except KeyError: 
        p = point() 
        p.set(0.0) 
        points[name] = p 
        return 0.0

def send_point_list(point_list, conn): 
    if len(point_list) > 0: 
        for p_name in point_list: 
            val = get_point(p_name) 
            #print p_name
            #print val
            snd = "%s= %f\n" % (p_name, val) 
            #print snd 
            conn.sendall(snd) 
        conn.sendall("end of list\n") 
 
def parse(line, conn): 
    try: 
    	if string.find(line, "getlist")==0:
            c = 1
	    split1 = string.split(line, " ") 
            list = split1[1] 
            cut = len(list)-2 
            list_trim = list[0:cut] 
            point_list = string.split(list_trim, ",") 
            if debug:
	    	print "list" + list_trim 
            	print point_list 
            send_point_list(point_list, conn) 
        elif string.find(line, "set")==0: 
            c = 2 
            split1 = string.split(line, " ") 
            name_value = string.split(split1[1], "=") 
            set_point(name_value[0], float(name_value[1]))
            conn.sendall("\n")
        elif string.find(line, "savesetings")==0: 
            c = 3 
            conn.sendall("\n") 
        elif string.find(line, "sheet")==0: 
            c = 3 
            conn.sendall("OK\n") 
        else: 
            c = 4 
            conn.sendall("\n") 
        return 1
    except socket.error:
        conn.close()
        print "Socket Error"
        return 0

def handler(connection): 
    if debug:
        print "Handler start" 
    noerr = 1
    while(noerr):
        # Limit reception to 1024 bytes.
        line=connection.recv(1024) 
        if debug:
      	    print line 
        noerr = parse(line, connection) 

def write_points():
    for p in points.keys():
        mp = points[p].get_matpoint()
        if mp == None:
            mp = matplc.point_f(p)
            points[p].set_matpoint(mp)
        mp.set(points[p].get())

def read_points():
    for p in points.keys():
        mp = points[p].get_matpoint()
        if mp == None:
            mp = matplc.point_f(p)
            points[p].set_matpoint(mp)
        points[p].set(mp.get())
	if debug:
	    print p
	    print mp.get()

# For now I am asumming that all points are floats.
def mat_io():
    while(1):
        matplc.scan_beg()
        write_points()
        matplc.update()
        read_points()
        matplc.scan_end()

if __name__=='__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT)) 
    s.listen(1)

    # Initialize matplc interface
    if debug:
       print "%s module started." % MODULE_NAME
    matplc.init(MODULE_NAME)
    lock = thread.allocate_lock()
    thread.start_new_thread(mat_io, ())
    while 1:
    	try:
            connection, addr = s.accept()
            if debug:
	  	print 'Connected by', addr
            thread.start_new_thread(handler, (connection, ))
        except socket.error:
            dummy = 1
