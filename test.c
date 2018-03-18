#include <stdio.h>
#include "userthread.h"
#include "logger.h"

//void printHello () {
//    printf("Hello world\n");
//}
//
//void tryYield() {
//    printf("start yield\n");
//    thread_yield();
//    printf("end yield\n");
//    printHello();
//}
//
//int main() {
//    thread_libinit(FIFO);
//    thread_create(printHello, NULL, -1);
//    thread_create(tryYield, NULL, -1);
////    thread_create(printHello, NULL, -1);
//
//    //expected is that 2 runs last...
//
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
//    printf("joining 2\n");
//    thread_join(2);
//    printf("joining 1\n");
//    thread_join(1);
//
////    thread_join(3);
////    printf("joining 2\n");
////    printf("%d\n", thread_join(2));
//
//
//    printf("Back to main\n");
//
//    return 0;
//}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include "userthread.h"

#define N 128

void foo() {
    poll(NULL, 0, 1);
}

int main(void) {
    printf(" * Running 129 threads! \n");

    if (thread_libinit(FIFO) == -1)
        exit(EXIT_FAILURE);

    int tids[N];
    memset(tids, -1, sizeof(tids));

    for (int i = 0; i < N; i++)  {
        tids[i] = thread_create(foo, NULL, 1);
    }

    for (int i = 0; i < N; i++)  {
        if (tids[i] == -1)
            exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++)  {
        if (thread_join(tids[i]) == -1)
            exit(EXIT_FAILURE);
    }

//    if (thread_libterminate() == -1)
//        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}