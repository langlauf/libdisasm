#!/bin/sh


gcc -fPIC -I../include -c i386.c -o i386.o

#make the shared library
gcc -shared -Wl,-soname,libi386.so -o libi386.so i386.o -lc
#make the static library
ar rcs libi386.a i386.o

#make the test file
gcc -I../include quikdis.c libi386.a -o quikdis
#with shared libs:
#gcc quikdis.c -li386 -L./ -o quikdis

#cleanup
rm i386.o 
