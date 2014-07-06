#!/usr/bin/python

import os

if not os.path.exists( "out" ): os.makedirs( "out" )

os.system("make -j4 -B lsdclib -f Makefile.lib")
os.system("make -j4 -B lsdctest -f Makefile.test")
os.system("make -j4 -B lsdc -f Makefile.lsdc")
