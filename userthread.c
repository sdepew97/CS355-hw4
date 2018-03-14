//
// Created by Sarah Depew on 3/13/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "userthread.h"
//#include "logger.h"

#define FAILURE -1
#define SUCCESS 0

//global variable to store the scheduling policy
static int POLICY; //policy for scheduling that the user passed
static int TID = 1;

//structs used in program
typedef struct TCB {
    int TID;
    ucontext_t ucontext;
    unsigned int CPUusage;
    unsigned int priority;
} TCB;

typedef struct node {
    void *TCB;
    struct node *next;
    struct node *prev;
} node;

typedef struct linkedList {
    struct node *head;
    struct node *tail;
    unsigned int size;
} linkedList;

static linkedList *FIFOList = NULL;
static linkedList *SJFList = NULL;
static linkedList *PRIORITYList = NULL;

int thread_libinit(int policy) {
    if(policy == FIFO) {
        //TODO: setup queues here
        POLICY = FIFO;

        //TODO: free memory malloced here!
        FIFOList = malloc(sizeof(linkedList));

        if(FIFOList == NULL) {
            return FAILURE;
        }

        FIFOList->head = NULL;
        FIFOList->tail = NULL;
        FIFOList->size = 0;

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

    if (POLICY == FIFO) {
        int currentTID = TID;
        //TODO: mask access to global variable!
        ucontext_t *newThread = malloc(sizeof(ucontext_t)); //TODO: error check malloc
        getcontext(thread);
        newThread->uc_link = NULL;
        newThread->ss_sp = malloc(STACKSIZE);
        newThread->ss_size = STACKSIZE;
        makecontext(newThread, func, 1, arg);
        //TODO: figure out what to do with masking here??

        TCB *newThreadTCB = malloc(sizeof(TCB));
        newThreadTCB->ucontext = newThread;
        newThreadTCB->CPUusage = 0;
        newThreadTCB->priority = -1; //we are not doing priority scheduling here
        newThreadTCB->TID = currentTID;
        TID++; //TODO: MASK!!

        node *newThreadNode = malloc(sizeof(node));
        newThreadNode->TCB = newThreadTCB;

        //TODO: mask this linked list interaction
        //first node ever
        if (FIFOList->head == NULL) {
            FIFOList->head = newThreadNode;
            FIFOList->tail = newThreadNode;
            newThreadNode->next = NULL;
            newThreadNode->prev = NULL;
        } else { //there are other nodes
            node *tailNode = FIFOList->tail;
            tailNode->next = newThreadNode;
            newThreadNode->prev = tailNode;
            FIFOList->tail = newThreadNode;
        }
        return currentTID;
    }

    return FAILURE;
}

int thread_yield(void);

int thread_join(int tid);
