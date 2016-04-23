all: ramdisk postmark

ramdisk: 
	g++ -g -Wall ramdisk.cpp `pkg-config fuse --cflags --libs` -o filesystem.o

postmark:
	gcc postmark.c -o postmark.o

clean: 
	\rm *.o
