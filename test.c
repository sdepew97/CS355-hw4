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


//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <string.h>
//#include <poll.h>
//#include "userthread.h"
//
//#define N 128
//
//void foo() {
//    poll(NULL, 0, 1);
//}
//
//int main(void) {
//    printf(" * Running 129 threads! \n");
//
//    if (thread_libinit(FIFO) == -1)
//        exit(EXIT_FAILURE);
//
//    int tids[N];
//    memset(tids, -1, sizeof(tids));
//
//    for (int i = 0; i < N; i++)  {
//        tids[i] = thread_create(foo, NULL, 1);
//    }
//
//    for (int i = 0; i < N; i++)  {
//        if (tids[i] == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    for (int i = 0; i < N; i++)  {
//        if (thread_join(tids[i]) == -1)
//            exit(EXIT_FAILURE);
//    }
//
////    if (thread_libterminate() == -1)
////        exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <poll.h>
//#include "userthread.h"
//
//int idx = 0;
//int created_tids[3] = { -1, -1, -1 };
//
//void foo() {}
//void foo_create() {
//    created_tids[idx] = thread_create(foo, NULL, 1);
//    thread_join(created_tids[idx++]);
//}
//
//int main(void) {
//    if (thread_libinit(FIFO) == -1)
//        exit(EXIT_FAILURE);
//
//    int tid1 = thread_create(foo_create, NULL, 1);
//    int tid2 = thread_create(foo_create, NULL, 1);
//    int tid3 = thread_create(foo_create, NULL, 1);
//
//    int n = 3;
//    int tids[] = { tid1, tid2, tid3 };
//
//    for (int i = 0; i < n; i++)  {
//        if (tids[i] == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    for (int i = 0; i < n; i++)  {
//        if (thread_join(tids[i]) == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    for (int i = 0; i < 3; i++)  {
//        if (created_tids[i] == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    printf(" * Testing thread_create within thread routines\n");
//    printf(" * Threads should end in this order ");
//    printf(" %d -> %d -> %d", created_tids[0], created_tids[1], created_tids[2]);
//    printf(" -> %d -> %d -> %d \n", tid1, tid2, tid3);

//    if (thread_libterminate() == -1) //TODO: check when implemented
//        exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}
//
//#include <stdio.h>
//#include <stdlib.h>
//#include "userthread.h"
//
//#define FAILURE -1
//
//void foo() {}
//
//void foo_yield() {
//    thread_yield();
//}
//
//void foo_join(void *tid) {
//    thread_join(*((int *)tid));
//}
//
//int main(void) {
//    if (thread_libinit(FIFO) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    int tid1 = thread_create(foo_yield, NULL, 0);
//    int tid2 = thread_create(foo_join, &tid1, 0);
//    int tid3 = thread_create(foo_join, &tid2, 0);
//    int tid4 = thread_create(foo_join, &tid3, 0);
//    int tid5 = thread_create(foo_join, &tid4, 0);
//    int tid6 = thread_create(foo_join, &tid5, 0);
//
//    int n  = 6;
//    int tids[] = { tid1, tid2, tid3, tid4, tid5, tid6 };
//
//    printf(" * Testing a chain of joins for FIFO\n");
//    printf(" * Threads should in this order: %d -> %d -> %d -> %d -> %d -> %d\n",
//           tid1, tid2 ,tid3, tid4, tid5, tid6);
//
//    for (int i = 0; i < n; i++)  {
//        if (tids[i] == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    if (thread_join(tid6) == -1)
//        exit(EXIT_FAILURE);
//
////    if (thread_libterminate() == FAILURE)
////        exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}

#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

void foo() {}
void bar() {}

int main(void) {
    printf(" * Testing a basic FIFO with some misuse of the userthread library\n");
    printf(" * Shouldn't cause any crash or memory leak! \n");
    // the following three should return -1
    if (thread_create(foo, NULL, 0) != -1)
        exit(EXIT_FAILURE);
    if (thread_create(bar, NULL, 0) != -1)
        exit(EXIT_FAILURE);
    if (thread_join(1) != -1)
        exit(EXIT_FAILURE);

    // calling |thread_libterminate| before calling |thread_libinit|
    // can either return 0 or -1... but shouldn't cause any thing weird.
//    thread_libterminate();
//    thread_libterminate();
    printf("thread_libinit\n");
    if (thread_libinit(FIFO) == -1)
        exit(EXIT_FAILURE);

    int tid1 = thread_create(foo, NULL, 0);
    int tid2 = thread_create(foo, NULL, 0);
    int tid3 = thread_create(foo, NULL, 0);
    int tid4 = thread_create(foo, NULL, 0);
    int tid5 = thread_create(foo, NULL, 0);
    int tid6 = thread_create(foo, NULL, 0);

    int n  = 6;
    int tids[] = { tid1, tid2, tid3, tid4, tid5, tid6 };

    for (int i = 0; i < n; i++)  {
        if (tids[i] == -1)
            exit(EXIT_FAILURE);
    }

    if (thread_join(tid6) == -1)
        exit(EXIT_FAILURE);

//    if (thread_libterminate() == -1)
//        exit(EXIT_FAILURE);

    printf(" * Threads should in this order: %d -> %d -> %d -> %d -> %d -> %d\n",
           tid1, tid2 ,tid3, tid4, tid5, tid6);

    // more misuses...
//    thread_libterminate();
    thread_join(123132);
    exit(EXIT_SUCCESS);

}

