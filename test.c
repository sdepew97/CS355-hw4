////
//// Created by Sarah Depew on 3/19/18.
////
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <poll.h>
//#include "userthread.h"
//
//#define SUCCESS 0
//#define FAILURE -1
//#define POLICY 2 //FIFO PRIORITY
//#define PRIORITY 0
//
//void printHello() {
//    printf("Hello World\n");
//    poll(NULL, 0, 200);
//}
//
///*
// * Simple FIFO test with main's functionality as a thread is tested.
// */
//int main(void) {
//    if (thread_libinit(POLICY) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("This is a Priority test where main is tested as a thread.\n");
//
//    int tid1 = thread_create(printHello, NULL, PRIORITY);
//
//    if (tid1 == FAILURE)
//        exit(EXIT_FAILURE);
//
//    if (thread_join(tid1) == FAILURE)
//        exit(EXIT_FAILURE);
//
//    printf("Back in Main\n");
//    printf("Should run main->%d and print Hello World and Back in Main if successful.\n", tid1);
//
//    if (thread_libterminate() == FAILURE)
//        exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}













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

//#include <stdio.h>
//#include <stdlib.h>
//#include "userthread.h"
//
//int tid_1;
//
//void hello(void *arg) {
//    thread_join(tid_1);
//    printf("%s\n", arg);
//}
//
//int main(void) {
//    if (thread_libinit(FIFO) == -1) exit(EXIT_FAILURE);
//
//    char *hello_str = "Hello, world!";
//    tid_1 = thread_create(hello, hello_str, 0);
//
//    printf("Test case for FIFO. Let one thread join itself.\n");
//    printf("The program gets stuck and prints nothing on success.\n");
//
//    if (thread_join(tid_1) < 0) exit(EXIT_FAILURE);
//
//    if (thread_libterminate() == -1) exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}

//#include <stdio.h>
//#include <stdlib.h>
//#include <poll.h>
//#include "userthread.h"
//
//void foo1() {
//    for (int i = 0; i < 4; i++) {
//        poll(NULL, 0, 1);
//        thread_yield();
//    }
//}
//
//void foo200() {
//    for (int i = 0; i < 4; i++) {
//        poll(NULL, 0, 200);
//        thread_yield();
//    }
//}
//
///**
// * A simple test for SJF
// */
//int main(void) {
//    if (thread_libinit(SJF) == -1)
//        exit(EXIT_FAILURE);
//
//    int tid1 = thread_create(foo1, NULL, 1);
//    int tid2 = thread_create(foo200, NULL, 1);
//    int tid3 = thread_create(foo1, NULL, 1);
//    int tid4 = thread_create(foo200, NULL, 1);
//    int tid5 = thread_create(foo1, NULL, 1);
//    int tid6 = thread_create(foo200, NULL, 1);
//
//    printf(" * A simple test for SJF scheduling\n");
//    printf(" Assumptions\n");
//    printf("(1) the default runtime value is less than 200 msec (real time rather than cputime).\n");
//    printf(" * The threads with tid %d, %d and %d should end earlier than the threads with tid %d, %d, %d\n",
//           tid1, tid3, tid5, tid2, tid4, tid6);
//
//    int n = 6;
//    int tids[] = {tid1, tid2, tid3, tid4, tid5, tid6};
//
//    for (int i = 0; i < n; i++) {
//        if (tids[i] == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    for (int i = 0; i < n; i++) {
//        if (thread_join(tids[i]) == -1)
//            exit(EXIT_FAILURE);
//    }
//
//    if (thread_libterminate() == -1)
//        exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}



//#include "userthread.h"
//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
//
//typedef struct {int num; char* arg;} PAIR;
//
//void printArg(void* argv){
//    PAIR* in = (PAIR* )argv;
//    char* word = in->arg;
//    int num = in->num;
//    //		poll(NULL, 0, 100);
//    printf("num: %d; string: %s\n", num, word);
//
//    //while(1);{printf("task: %d\n", num);}
//}
//
//
//int testLl() {
//    printf("\n\nTesting user functions with arguments\n");
//    printf("Should print the number and string passed in\n\n\n");
//
//
//    thread_libinit(SJF); // FIFO SJF PRIORITY
//
//    PAIR ins, ins2, ins3;
//    ins.num = 1;
//    ins.arg = "the message1";
//
//    ins2.num = 2;
//    ins2.arg = "the message2";
//
//    ins3.num = 3;
//    ins3.arg = "the message3";
//
//    //create
//    int tid1 = thread_create(printArg, &ins, 0);
//    int tid2 = thread_create(printArg, &ins2, -1);
//    int tid3 = thread_create(printArg, &ins3, 1);
//
//
//    // join
//    thread_join(tid1);
//    thread_join(tid2);
//    thread_join(tid3);
//
//
//    // term
//    thread_libterminate();
//
//    return 1;
//}
//
//
//int main() {
//    testLl();
//    exit(EXIT_SUCCESS);
//}

//#include <stdio.h>
//#include <stdlib.h>
//#include "userthread.h"
//
//int tid_1;
//
//void hello(void *arg) {
//    thread_join(tid_1);
//    printf("%s\n", arg);
//}
//
//int main(void) {
//    if (thread_libinit(FIFO) == -1) exit(EXIT_FAILURE);
//
//    char *hello_str = "Hello, world!";
//    tid_1 = thread_create(hello, hello_str, 0);
//
//    printf("Test case for FIFO. Let one thread join itself.\n");
//    printf("The program gets stuck and prints nothing on success.\n");
//
//    if (thread_join(tid_1) < 0) exit(EXIT_FAILURE);
//
//    if (thread_libterminate() == -1) exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}
//
//#include <stdio.h>
//#include <stdlib.h>
//#include "userthread.h"
//
//int tid_1, tid_2;
//
//void hello(void *arg) {
//    thread_join(tid_2);
//    printf("%s\n", arg);
//}
//
//void hello2(void *arg) {
//    thread_join(tid_1);
//    printf("%s\n", arg);
//}
//
//int main(void) {
//    if (thread_libinit(FIFO) == -1) exit(EXIT_FAILURE);
//
//    char *hello_str = "Hello, world!";
//    tid_1 = thread_create(hello, hello_str, 0);
//    tid_2 = thread_create(hello, hello_str, 0);
//
//    printf("Test case for FIFO. 2 threads join each other to create deadlock.\n");
//    printf("The program gets stuck and prints nothing on success.\n");
//
//    if (thread_join(tid_1) < 0) exit(EXIT_FAILURE);
//    if (thread_join(tid_2) < 0) exit(EXIT_FAILURE);
//
//    if (thread_libterminate() == -1) exit(EXIT_FAILURE);
//
//    exit(EXIT_SUCCESS);
//}

#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define FAILURE -1

void foo() {
    printf("Woo!\n");
}

int main(void) {
    printf("Testing some misuses of threads\n");
    printf("This assumes that the priority queue range is from -1 to 1\n");
    printf("because it tests adding a thread with priority 100\n");
    printf("On success, it prints 'Woo!', does not crash, and does not cause any memory leaks\n");

    int tidx = thread_create(foo, NULL, 0);
    if (tidx != -1)
        exit(EXIT_FAILURE);

//    thread_libterminate();

    if (thread_libinit(PRIORITY) == FAILURE)
        exit(EXIT_FAILURE);

    int tid1 = thread_create(foo, NULL, 100);
    if (tid1 == -1)
        exit(EXIT_FAILURE);

    if (thread_join(tid1) == -1)
        exit(EXIT_FAILURE);

//    if (thread_libterminate() == FAILURE)
//        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}