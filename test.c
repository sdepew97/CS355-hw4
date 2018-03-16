#include <stdio.h>
#include "userthread.h"
#include "logger.h"

void printHello () {
    printf("Hello world\n");
}

void tryYield() {
    printf("start yield\n");
    thread_yield();
    printf("end yield\n");
}

int main() {
    thread_libinit(FIFO);
//    thread_create(printHello, NULL, -1);
    thread_create(tryYield, NULL, -1);
//    thread_create(printHello, NULL, -1);

    //expected is that 2 runs last...

//    thread_join(thread_create(printHello, NULL, -1));
//    thread_join(thread_create(printHello, NULL, -1));
//    thread_join(thread_create(printHello, NULL, -1));
//    thread_join(thread_create(printHello, NULL, -1));
    thread_join(1);
//    thread_join(2);
    printf("joining 1\n");
//    thread_join(3);
//    printf("joining 2\n");
//    printf("%d\n", thread_join(2));


    printf("Back to main\n");

    return 0;
}
