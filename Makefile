all: userthreads test clean

test:
	gcc -g -o test test.c -L. -luserthread

userthreads: userthreads.o
	gcc -g -o libuserthread.so userthread.o -shared

userthreads.o: userthread.c userthread.h
	gcc -g -Wall -fpic -c userthread.c

clean:
	rm -rf *.o *.gch *.dSYM