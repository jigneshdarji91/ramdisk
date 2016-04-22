all: hello

hello: 
	gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello

clean: 
	\rm hello 
