all: logger test userthreads clean

test:
	gcc -o test test.c -L. -luserthread

logger: logger.o
	gcc -o logger logger.o

logger.o: logger.c logger.h
	gcc -c -Wall logger.c

userthreads: userthreads.o
	gcc -o libuserthread.so userthread.o -shared

userthreads.o: userthread.c userthread.h
	gcc -Wall -fpic -c userthread.c

clean:
	rm -rf *.o *.gch *.dSYM