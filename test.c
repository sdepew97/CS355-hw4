#include <stdio.h>
#include "userthread.h"
#include "logger.h"

void printHello () {
    printf("Hello world\n");
}

int main() {
    thread_libinit(FIFO);
    thread_create(printHello, NULL, -1);
    thread_create(printHello, NULL, -1);
    thread_join(2);
    printf("joining 1\n");
    thread_join(1);

    printf("Back to main\n");

    return 0;
}