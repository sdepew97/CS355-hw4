all: userthreads test clean

test:
	gcc -g -ggdb -o test testsRightNow.c -L. -luserthread

userthreads: userthreads.o
	gcc -g -ggdb -o libuserthread.so userthread.o -shared

userthreads.o: userthread.c userthread.h
	gcc -g -ggdb -Wall -fpic -c userthread.c

clean:
	rm -rf *.o *.gch *.dSYM