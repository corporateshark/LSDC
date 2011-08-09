#! /usr/bin/python

import os

#del /q Out/liblsdc.a

if not os.path.exists( "Out" ): os.makedirs( "Out" )
if os.path.exists( "Out/liblsdc.a" ): os.remove( "Out/liblsdc.a" )

os.system("make lsdclib -f Makefile.lib")
os.system("make lsdctest -f Makefile.test")
os.system("make lsdc -f Makefile.lsdc")
