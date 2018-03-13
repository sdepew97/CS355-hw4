all: test logger userthreads clean

test:
	gcc -o test test.c -L. -luserthread

logger: logger.o
	gcc -o libuserthread.so logger.o -shared

userthreads: userthreads.o
	gcc -o libuserthread.so userthread.o -shared

userthreads.o: userthread.c userthread.h
	gcc -Wall -fpic -c userthread.c

logger.o: logger.c logger.h
	gcc -Wall -fpic -c logger.c

clean:
	rm -rf *.o *.gch *.dSYM
