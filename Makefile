all: logger test userthreads clean

test:
	gcc -o test test.c -L. -luserthread

userthreads: userthreads.o logger.o
	gcc -o libuserthread.so userthread.o logger.o -shared

userthreads.o: userthread.c userthread.h logger.h
	gcc -Wall -fpic -c userthread.c

logger.o: logger.c logger.h
	gcc -Wall -fpic -c logger.c

clean:
	rm -rf *.o *.gch *.dSYM