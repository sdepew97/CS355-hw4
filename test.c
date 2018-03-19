//#include <stdio.h>
//#include "userthread.h"
//#include "logger.h"
//
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
////    thread_libinit(FIFO);
//    thread_libinit(SJF);
//    thread_create(printHello, NULL, -1);
////    thread_create(tryYield, NULL, -1);
////    thread_create(printHello, NULL, -1);
//
//    //expected is that 2 runs last...
//
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
////    thread_join(thread_create(printHello, NULL, -1));
////    printf("joining 2\n");
////    thread_join(2);
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
#include <poll.h>
#include "userthread.h"

int idx = 0;
int created_tids[3] = { -1, -1, -1 };

void foo() {}
void foo_create() {
    created_tids[idx] = thread_create(foo, NULL, 1);
    thread_join(created_tids[idx++]);
}

int main(void) {
    if (thread_libinit(FIFO) == -1)
        exit(EXIT_FAILURE);

    int tid1 = thread_create(foo_create, NULL, 1);
    int tid2 = thread_create(foo_create, NULL, 1);
    int tid3 = thread_create(foo_create, NULL, 1);

    int n = 3;
    int tids[] = { tid1, tid2, tid3 };

    for (int i = 0; i < n; i++)  {
        if (tids[i] == -1)
            exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)  {
        if (thread_join(tids[i]) == -1)
            exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 3; i++)  {
        if (created_tids[i] == -1)
            exit(EXIT_FAILURE);
    }

    printf(" * Testing thread_create within thread routines\n");
    printf(" * Threads should end in this order ");
    printf(" %d -> %d -> %d", created_tids[0], created_tids[1], created_tids[2]);
    printf(" -> %d -> %d -> %d \n", tid1, tid2, tid3);

    if (thread_libterminate() == -1)
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}