//
// Created by Sarah Depew on 3/13/18.
//

#include <stdio.h>
#include "userthread.h"

#define FAILURE -1
#define SUCCESS 0

//global variable to store the scheduling policy
static int POLICY; //policy for scheduling that the user passed

int thread_libinit(int policy) {
    if(policy == FIFO) {
        //TODO: setup queues here
        POLICY = FIFO;
        return SUCCESS;
    } else if(policy == SJF) {
        //TODO: setup queues here
        POLICY = SJF;
        return SUCCESS;
    } else if(policy == PRIORITY) {
        //TODO: setup queues here
        POLICY = PRIORITY;
        return SUCCESS;
    } else {
        return FAILURE;
    }

}

int thread_libterminate(void) {
    //free all queues

    //free all thread memory malloced

    //free all TCB's etc...
}

int thread_create(void (*func)(void *), void *arg, int priority) {
    //check type of scheduling

    //create a new context and TCB for the thread

    //get context and make context here to create the thread

    //assign a thread ID...use a global counter to keep track of TID's?

    //add to the ready queue for the job type

    //return the TID or the failure value
}

int thread_yield(void);

int thread_join(int tid);
