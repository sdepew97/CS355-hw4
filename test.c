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
//#include "userthread.h"
//#include "logger.h"
//
//#define SUCCESS 0
//#define FAILURE -1
//#define PRIORITY 1
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
//    if (thread_libinit(SJF) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    int tid1 = thread_create(printHello, NULL, -1);
//
//    if (tid1 == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("joining 1\n");
//    if (thread_join(tid1) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    int tid2 = thread_create(tryYield, NULL, -1);
//
//    if (tid2 == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("joining 2\n");
//    if (thread_join(tid2) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("Back to main\n");
//
//    if (thread_libterminate() == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("Congratulations, your test was successful!\n");
//    exit(EXIT_SUCCESS);
//}

/*
Test for user thread library
Uses FIFO scheduling policy
Makes 1st thread wait for 2nd thread to finish
Expected output:
from f2
from f1
terminated
*/

#include <ucontext.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "userthread.h"

void f1() {
    printf("\nfrom f1\n\n");
}
void f2() {
    printf("\nfrom f2\n\n");
}

int main() {
    int tid, tid2;

    thread_libinit(FIFO);

    // create threads with functions f1, f2
    tid = thread_create(f1, NULL, 0);
    tid2 = thread_create(f2, NULL, 0);

    // join f2 thread first so it finishes before f1
    thread_join(tid2);
    thread_join(tid);

    thread_libterminate();
    printf("terminated\n");
    return 0;
}