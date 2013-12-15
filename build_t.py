#!/usr/bin/python

import os

#del /q Out/liblsdc.a

if not os.path.exists( "Out" ): os.makedirs( "Out" )
if os.path.exists( "Out/liblsdc.a" ): os.remove( "Out/liblsdc.a" )

os.system("make -j4 lsdclib -f Makefile.lib")
os.system("make -j4 lsdctest -f Makefile.test")
os.system("make -j4 lsdc -f Makefile.lsdc")
