all: test logger userthreads clean

test:
	gcc -o test test.c -L. -luserthread

userthreads: userthreads.o
	gcc -o libuserthread.so userthread.o logger.o -shared

logger.o: logger.c logger.h
	gcc -Wall -fpic -c logger.c

userthreads.o: userthread.c userthread.h
	gcc -Wall -fpic -c userthread.c

clean:
	rm -rf *.o *.gch *.dSYM
