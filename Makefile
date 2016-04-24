all: ramdisk 

ramdisk: 
	g++ -g -Wall ramdisk.cpp `pkg-config fuse --cflags --libs` -o ramdisk

postmark:
	gcc postmark.c -o postmark.o

tar: 
	tar czvf jndarji_ramdisk.tar.gz ramdisk.h ramdisk.cpp ramnode.h ramnode.cpp debug.h constants.h Makefile README REFERENCES postmark.c 

clean: 
	\rm *.o
