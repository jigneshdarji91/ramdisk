all: ramdisk

ramdisk: 
	g++ -Wall ramdisk.cpp `pkg-config fuse --cflags --libs` -o filesystem.o

clean: 
	\rm filesystem.o
