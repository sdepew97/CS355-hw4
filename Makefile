all: logger test userthreads clean

test: logger.h
	gcc -o test test.c -L. -luserthread

logger:
	gcc -o logger logger.c -L. -luserthread

userthreads: userthreads.o
	gcc -o libuserthread.so userthread.o -shared

userthreads.o: userthread.c userthread.h
	gcc -Wall -fpic -c userthread.c

clean:
	rm -rf *.o *.gch *.dSYM
