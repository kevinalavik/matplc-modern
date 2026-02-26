#! /usr/bin/env python

import sys, glob
sys.path.append('.')
sys.path[-1:-1] = glob.glob('build/lib.*')

# note: if you get:
#   ImportError: libmatplc.so.0: cannot open shared object file: No such
#   file or directory
# try setting the environment variable LD_LIBRARY_PATH to ../../lib/.libs/


import matplc

print "importing went OK"

matplc.init("test")
a = matplc.point("hello")
