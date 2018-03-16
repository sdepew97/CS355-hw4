//
// Created by Sarah Depew on 3/13/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <stdarg.h>
#include <signal.h>
#include "userthread.h"
#include "logger.h"

#define FAILURE -1
#define SUCCESS 0
#define TRUE 1
#define FALSE 0
#define LOGFILE	"scheduleLogger.txt\0"     // all Log(); messages will be appended to this file

//globals for logging
enum {CREATED, SCHEDULED, STOPPED, FINISHED};
char* states[] = {"CREATED\0", "SCHEDULED\0", "STOPPED\0", "FINISHED\0"};

//global variable to store the scheduling policy
static int POLICY; //policy for scheduling that the user passed //TODO: figure out why this is being deleted
static int TID = 1;
static int startTime;
static int LogCreated = FALSE;

//structs used in program
typedef struct TCB {
    int TID;
    ucontext_t *ucontext;
    unsigned int CPUusage;
    unsigned int priority;
    unsigned int state;
    struct TCB *joined;
} TCB;

typedef enum {
    READY = 0, //ready when joined
    WAITING = 1, //waiting when has joined something that's now ready to run
    RUNNING = 2, //running when it has been joined
    BLOCKED = 3, //created and blocked, since not joined
    DONE = 4 //when thread is done running
} status;

typedef struct node {
    struct TCB *tcb;
    struct node *next;
    struct node *prev;
} node;

typedef struct linkedList {
    struct node *head;
    struct node *tail;
    unsigned int size;
} linkedList;

static linkedList *readyList = NULL;
static linkedList *lowList = NULL;
static linkedList *mediumList = NULL;
static linkedList *highList = NULL;

TCB *mainTCB;
node *running;
ucontext_t *scheduler;

int stub(void (*func)(void *), void *arg);
long getTicks();
void Log (int ticks, int OPERATION, int TID, int PRIORITY);    // logs a message to LOGFILE
void schedule();
void printList();
void initMainTCB(int policy);
ucontext_t *newContext(ucontext_t *uc_link, void (*func)(void *), void* arg);
TCB* newTCB(int TID, int CPUUsage, int priority, int state, TCB *joined);
node* newNode(TCB *tcb, node* next, node* prev);

int thread_libinit(int policy) {
    //create context for scheduler
//    scheduler = newContext(NULL, )
    scheduler = malloc(sizeof(ucontext_t)); //TODO: error check malloc
    getcontext(scheduler);
    scheduler->uc_link = NULL;
    scheduler->uc_stack.ss_sp = malloc(STACKSIZE);
    scheduler->uc_stack.ss_size = STACKSIZE;
    makecontext(scheduler, (void (*)(void)) schedule, 0);

    //TODO: log here for main being created

    mainTCB = newTCB(-1, 0, 1, READY, NULL);
    getcontext(mainTCB->ucontext);

    running = newNode(mainTCB, NULL, NULL);
    mainTCB->state = RUNNING;

    startTime = (int) getTicks();

    POLICY = policy;

    if (policy == FIFO || policy == SJF) {
        //TODO: free memory malloced here!
        readyList = malloc(sizeof(linkedList));

        if (readyList == NULL) {
            return FAILURE;
        }

        readyList->head = running;
        readyList->tail = running;
        readyList->size++;

        return SUCCESS;
    } else if (policy == PRIORITY) {
        //TODO: setup queues here

        return SUCCESS;
    } else {
        return FAILURE;
    }

    return FAILURE;
}

int thread_libterminate(void) {
    //free all queues

    //free all thread memory malloced

    //free all TCB's etc...

    //mark main as finished and free
    return FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority) {
    printf("creating new thread %d\n", TID);
    printList();
    //check type of scheduling

    //create a new context and TCB for the thread

    //get context and make context here to create the thread

    //assign a thread ID...use a global counter to keep track of TID's?

    //add to the ready queue for the job type

    //return the TID or the failure value
    int currentTID = TID;

    //TODO: mask access to global variable!
    ucontext_t *newThread = newContext(NULL, func, arg);

    TCB *newThreadTCB = newTCB(currentTID, 0, priority, BLOCKED, NULL);
    newThreadTCB->ucontext = newThread;
    TID++; //TODO: MASK!!

    if (POLICY == FIFO || POLICY == SJF) {
        //TODO: mask this linked list interaction
        //first node ever on the list
        if (readyList->size == 0) {
            node *newThreadNode = newNode(newThreadTCB, NULL, NULL);
            readyList->head = newThreadNode;
            readyList->tail = newThreadNode;
            readyList->size++;
        } else { //there are other nodes on the list
            node *tailNode = readyList->tail;
            node *newThreadNode = newNode(newThreadTCB, NULL, tailNode);
            tailNode->next = newThreadNode;
            tailNode->prev = tailNode;
            readyList->tail = newThreadNode;
            readyList->size++;
        }
        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        printList();
        return currentTID;
    }
    /*
     * else { //we are priority scheduling
        if (priority == -1) {
            //first node ever on the list
            if (lowList->size == 0) {
                node *newThreadNode = newNode(newThreadTCB, NULL, NULL);
                lowList->head = newThreadNode;
                lowList->tail = newThreadNode;
                lowList->size++;
                newThreadNode->next = NULL;
                newThreadNode->prev = NULL;
            } else { //there are other nodes on the list
                node *tailNode = lowList->tail;
                tailNode->next = newThreadNode;
                tailNode->prev = tailNode;
                lowList->tail = newThreadNode;
                lowList->size++;
            }
        } else if (priority == 0) {
            //first node ever on the list
            if (mediumList->size == 0) {
                node *newThreadNode = newNode(newThreadTCB, NULL, NULL);
                mediumList->head = newThreadNode;
                mediumList->tail = newThreadNode;
                mediumList->size++;
                newThreadNode->next = NULL;
                newThreadNode->prev = NULL;
            } else { //there are other nodes on the list
                node *tailNode = mediumList->tail;
                tailNode->next = newThreadNode;
                tailNode->prev = tailNode;
                mediumList->tail = newThreadNode;
            }
        } else if (priority == 1) {
            //first node ever on the list
            if (highList->size == 0) {
                node *newThreadNode = newNode(newThreadTCB, NULL, NULL);
                highList->head = newThreadNode;
                highList->tail = newThreadNode;
                highList->size++;
                newThreadNode->next = NULL;
                newThreadNode->prev = NULL;
            } else { //there are other nodes on the list
                node *tailNode = highList->tail;
                tailNode->next = newThreadNode;
                tailNode->prev = tailNode;
                highList->tail = newThreadNode;
                highList->size++;
            }
        } else {
            //priority is invalid
            return FAILURE;
        }
        */

//        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
//        return currentTID;
//    }

    return FAILURE;
}

int thread_yield(void) {
    //put current running back onto the ready queue

    //change state of running thread to ready

    //ensure that running thread is back of the ready queue

    //call scheduler for the threads

    if (POLICY == FIFO || POLICY == SJF) {
        //running node is in the list, so have to 1) find it (have a pointer to it rn), 2) move it to the tail
        node *currentRunning = running;
        node *currentRunningPrev = running->prev;
        node *currentRunningNext = running->next;
        node *currentTail = readyList->tail;

        //running is head
        if (currentRunningPrev == NULL) {
            readyList->head = currentRunningNext;
            readyList->head->prev = NULL;
            currentRunning->next = NULL;
            currentTail->next = currentRunning;
            currentRunning->prev = currentTail;
            readyList->tail = currentRunning;
            ((TCB *) currentRunning->tcb)->state = READY;
            Log((int) getTicks() - startTime, STOPPED, ((TCB *) currentRunning->tcb)->TID, -1);
//            schedule();
            swapcontext(running->tcb->ucontext, scheduler);
        }

            //running is tail (do nothing)
        else if (currentRunningNext == NULL) {
            //node is already at the tail, so mark as ready and then call scheduler
            ((TCB *) currentRunning->tcb)->state = READY;
            Log((int) getTicks() - startTime, STOPPED, ((TCB *) currentRunning->tcb)->TID, -1);
//            schedule();
            swapcontext(running->tcb->ucontext, scheduler);
        }

            //running is middle node
        else {
            currentRunning->next = NULL;
            currentRunning->prev = currentTail;
            currentRunningPrev->next = currentRunningNext;
            currentRunningNext->prev = currentRunningPrev;
            readyList->tail = currentRunning;
            ((TCB *) currentRunning->tcb)->state = READY;
            Log((int) getTicks() - startTime, STOPPED, ((TCB *) currentRunning->tcb)->TID, -1);
//            schedule();
            swapcontext(running->tcb->ucontext, scheduler);
        }

        schedule();
    } else {
        //TODO: fill in code here for priority
    }

    return FAILURE;
}

int thread_join(int tid) {
    printf("join called for %d\n", tid);
    printList();

    if (POLICY == FIFO || POLICY == SJF) {
        printf("got into FIFO or SJF\n");
        node *currentNode = readyList->head;

        //find the node to join
        while(currentNode!=NULL && ((TCB *) currentNode->tcb)->TID !=tid) {
            currentNode = currentNode->next;
        }

        if(currentNode != NULL) {
            ((TCB *) currentNode->tcb)->state = READY; //change to ready, since it's been joined and can run as a result
//            getcontext(
//                    ((TCB *) running)->ucontext); //as soon as calls thread join, get context, since this is where we want to return


            //case 2: TID does exist and found thread is waiting already, which would mean you'd get stuck forever, perhaps?
            if (((TCB *) currentNode->tcb)->state == WAITING) {
                if (currentNode->tcb->joined->TID != running->tcb->TID) {
                    running->tcb->state = WAITING;
                    Log((int) getTicks() - startTime, STOPPED, ((TCB *) running->tcb)->TID, -1);
                    ((TCB *) currentNode->tcb)->joined = running->tcb;
                    schedule();
                } else {
                    //attempting a circular join
                    printf("failed on circular\n");
                    return FAILURE;
                }
            }
            //case 3: TID does exist and thread is ready to go! (set calling thread to waiting by this thread and set joined pointer)
            printf("third case\n");
            ((TCB *) running->tcb)->state = WAITING;
            ((TCB *) currentNode->tcb)->joined = running->tcb;
            //schedule();
            swapcontext(running->tcb->ucontext, scheduler);

            return SUCCESS;
        }
        else {

            //case 1: TID doesn't exist/thread already finished
            if (currentNode == NULL) {
                printf("failed on null\n");
                return FAILURE;
            }
            //not found
//            return FAILURE;
        }
    }
    printf("got to end here\n");
    return FAILURE;
}

int stub(void (*func)(void *), void *arg) {
    printf("entered stub\n");
    // thread starts here
    func(arg); // call root function
    //TODO: thread clean up mentioned in assignment guidelines on page 3
    printf("thread done\n");
//    printList();
    Log((int) getTicks()-startTime, FINISHED, ((TCB *) running->tcb)->TID, -1);
    running->tcb->state = DONE;
    running->tcb->joined->state = READY;
    printList();
    schedule();
    exit(0); // all threads are done, so process should exit
}

long getTicks() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_usec;
}

void Log (int ticks, int OPERATION, int TID, int PRIORITY) {
    FILE *file;

    if (!LogCreated) {
        file = fopen(LOGFILE, "w");
        LogCreated = TRUE;
    } else {
        file = fopen(LOGFILE, "a");
    }

    if (file == NULL) {
        //there was an error here...
        return;
    } else {

        fprintf(file, "[%d]\t%s\t%d\t%d\n", ticks, states[OPERATION], TID, PRIORITY);
    }

    if (file) {
        fclose(file);
    }
}

/* Method with the scheduling algorithms */
void schedule() {
    getcontext(scheduler);
    printf("schedule called\n");
    printf("POLICY in schedule: %d\n", POLICY);
    printList();

    //TODO: ensure this interaction is masked
    node *currentNode = readyList->head;
    while (currentNode != NULL && ((TCB *) currentNode->tcb)->state != READY) {
        currentNode = currentNode->next;
    }

    if (POLICY == FIFO) {
        //now current node is ready to run, so have to run it here
        running = currentNode;
        Log((int) getTicks() - startTime, SCHEDULED, ((TCB *) currentNode->tcb)->TID, -1);
        running->tcb->state = RUNNING;
        printf("running TID %d\n", ((TCB *) running->tcb)->TID);
        printf("POLICY in schedule two: %d\n", POLICY);
        printList();
//        if (((TCB *) running->tcb)->ucontext != NULL) {
        setcontext(((TCB *) running->tcb)->ucontext);
//        }
    } else if (POLICY == SJF) {

    } else if (POLICY == PRIORITY) {

    }
}

void printList() {
    node *currentNode = readyList->head;
    while (currentNode != NULL) {
        printf("%d, state %d, policy %d->", ((TCB *) currentNode->tcb)->TID, ((TCB *) currentNode->tcb)->state, POLICY);

        currentNode = currentNode->next;
    }
    printf("NULL, list size %d", readyList->size);
    printf("\n");
}

ucontext_t *newContext(ucontext_t *uc_link, void (*func)(void *), void* arg) {
    //TODO: mask access to global variable!
    ucontext_t *returnValue = malloc(sizeof(ucontext_t)); //TODO: error check malloc
    getcontext(returnValue);
    returnValue->uc_link = uc_link;
//    returnValue->uc_sigmask = uc_sigmask;
    returnValue->uc_stack.ss_sp = malloc(STACKSIZE);
    returnValue->uc_stack.ss_size = STACKSIZE;
    //count list values //TODO: change 2 to argc (which is length of arg)
//    int argc;
    makecontext(returnValue, (void (*)(void)) stub, 2, func, arg);
//    makecontext(returnValue, (void (*)(void)) func, 1, arg);
    //TODO: figure out what to do with masking here??
    return returnValue;
}


//TODO: masking
TCB* newTCB(int TID, int CPUUsage, int priority, int state, TCB *joined) {
    TCB *returnValue = malloc(sizeof(TCB));
    returnValue->ucontext = malloc(sizeof(ucontext_t));
    returnValue->TID = TID;
    returnValue->CPUusage = CPUUsage;
    returnValue->priority = priority;
    returnValue->joined = malloc(sizeof(TCB));
    returnValue->joined = joined;
    returnValue->state = state;
    return returnValue;
}

node* newNode(TCB *tcb, node* next, node* prev) {
    node *returnValue = malloc(sizeof(node));
    returnValue->tcb = tcb;
    returnValue->next = next;
    returnValue->prev = prev;
    return returnValue;
}