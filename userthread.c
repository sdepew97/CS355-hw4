//
// Created by Sarah Depew on 3/13/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <stdarg.h>
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
static int POLICY; //policy for scheduling that the user passed
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
    READY = 0,
    WAITING = 1,
    RUNNING = 2,
    BLOCKED = 3,
    DONE = 4
} status;

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

static linkedList *readyList = NULL;
static linkedList *lowList = NULL;
static linkedList *mediumList = NULL;
static linkedList *highList = NULL;

TCB *mainTCB;
node *running;

int stub(void (*func)(void *), void *arg);
long getTicks();
void Log (int ticks, int OPERATION, int TID, int PRIORITY);    // logs a message to LOGFILE
int schedule();
void printList();

int thread_libinit(int policy) {
    mainTCB = malloc(sizeof(TCB));
    mainTCB->ucontext = malloc(sizeof(ucontext_t));
    mainTCB->state = READY;
    mainTCB->TID = -1; //set a unique TID for the main context, so we know when it's doing the switching
    //TODO: mark main here/get context as needed

    running = malloc(sizeof(node));
    running->TCB = malloc(sizeof(TCB));
    ((TCB *) running->TCB)->ucontext = malloc(sizeof(ucontext_t));
    mainTCB->state = RUNNING;
    running->TCB = mainTCB;
    running->next = NULL;
    running->prev = NULL;

//    readyList = malloc(sizeof(linkedList));
//    //add main to the ready queue
//    if (readyList->size == 0) {
//        readyList->head = running;
//        readyList->tail = running;
//        readyList->size++;
//        running->next = NULL;
//        running->prev = NULL;
//    }

    startTime = (int) getTicks();

    POLICY = policy;

    if (policy == FIFO || policy == SJF) {
        //TODO: free memory malloced here!
        readyList = malloc(sizeof(linkedList));

        if (readyList == NULL) {
            return FAILURE;
        }

//        readyList->head = NULL;
//        readyList->tail = NULL;
//        readyList->size = 0;

        readyList->head = running;
        readyList->tail = running;
        readyList->size++;

        return SUCCESS;
    } else if (policy == PRIORITY) {
        //TODO: setup queues here
        POLICY = PRIORITY;
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

    return FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority) {
    //check type of scheduling

    //create a new context and TCB for the thread

    //get context and make context here to create the thread

    //assign a thread ID...use a global counter to keep track of TID's?

    //add to the ready queue for the job type

    //return the TID or the failure value
    int currentTID = TID;

    //TODO: mask access to global variable!
    ucontext_t *newThread = malloc(sizeof(ucontext_t)); //TODO: error check malloc
    getcontext(newThread);
    newThread->uc_link = NULL;
    newThread->uc_stack.ss_sp = malloc(STACKSIZE);
    newThread->uc_stack.ss_size = STACKSIZE;
    makecontext(newThread, (void (*)(void)) stub, 2, func, arg);
    //TODO: figure out what to do with masking here??

    TCB *newThreadTCB = malloc(sizeof(TCB));
    newThreadTCB->ucontext = newThread;
    newThreadTCB->CPUusage = 0;//TODO: update for the Priority scheduling needed
    newThreadTCB->priority = priority;
    newThreadTCB->TID = currentTID;
    newThreadTCB->state = READY;
    TID++; //TODO: MASK!!

    node *newThreadNode = malloc(sizeof(node));
    newThreadNode->TCB = newThreadTCB;

    if (POLICY == FIFO || POLICY == SJF) {
        //TODO: mask this linked list interaction
        //first node ever on the list
        if (readyList->size == 0) {
            readyList->head = newThreadNode;
            readyList->tail = newThreadNode;
            readyList->size++;
            newThreadNode->next = NULL;
            newThreadNode->prev = NULL;
        } else { //there are other nodes on the list
            node *tailNode = readyList->tail;
            tailNode->next = newThreadNode;
            tailNode->prev = tailNode;
            readyList->tail = newThreadNode;
            readyList->size++;
        }
        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        printList();
        return currentTID;
    } else { //we are priority scheduling
        if (priority == -1) {
            //first node ever on the list
            if (lowList->size == 0) {
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
        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        return currentTID;
    }

    return FAILURE;
}

int thread_yield(void) {
    //put current running back onto the ready queue

    //change state of running thread to ready

    //ensure that running thread is back of the ready queue

    //call scheduler for the threads

    if(POLICY == FIFO || POLICY == SJF) {
        //running node is in the list, so have to 1) find it (have a pointer to it rn), 2) move it to the tail
        node *currentRunning = running;
        node *currentRunningPrev = running->prev;
        node *currentRunningNext = running->next;
        node *currentTail = readyList->tail;

        //running is head
        if(currentRunningPrev == NULL) {
            readyList->head = currentRunningNext;
            readyList->head->prev = NULL;
            currentRunning->next = NULL;
            currentTail->next = currentRunning;
            currentRunning->prev = currentTail;
            readyList->tail = currentRunning;
            ((TCB *) currentRunning->TCB)->state = READY;
            Log((int) getTicks()-startTime, STOPPED, ((TCB *) currentRunning->TCB)->TID, -1);
            schedule();
        }

        //running is tail (do nothing)
        else if(currentRunningNext == NULL) {
            //node is already at the tail, so mark as ready and then call scheduler
            ((TCB *) currentRunning->TCB)->state = READY;
            Log((int) getTicks()-startTime, STOPPED, ((TCB *) currentRunning->TCB)->TID, -1);
            schedule();
        }

        //running is middle node
        else {
            currentRunning->next = NULL;
            currentRunning->prev = currentTail;
            currentRunningPrev->next = currentRunningNext;
            currentRunningNext->prev = currentRunningPrev;
            readyList->tail = currentRunning;
            ((TCB *) currentRunning->TCB)->state = READY;
            Log((int) getTicks()-startTime, STOPPED, ((TCB *) currentRunning->TCB)->TID, -1);
            schedule();
        }

//        node* currentTail = readyList->tail;
//        currentTail->next = running;
//        running->prev = currentTail;
//        readyList->tail = running;



        schedule();
    } else {
        //TODO: fill in code here for priority
    }

    return FAILURE;
}

int thread_join(int tid) {
    //TODO call scheduler here!
    if (POLICY == FIFO || POLICY == SJF) {
        node *currentNode = readyList->head;
        //getcontext(((TCB *) running)->ucontext);

        while (currentNode != NULL && ((TCB *) currentNode->TCB)->TID != tid) {
            currentNode = currentNode->next;
        }

        //case 1: TID doesn't exist/thread already finished
        if (currentNode == NULL) {
            return FAILURE;
        }

        //case 2: TID does exist and found thread is waiting already, which would mean you'd get stuck forever, perhaps?
        if (((TCB *) currentNode->TCB)->state == WAITING) {
            if (((TCB *) currentNode->TCB)->joined->TID != ((TCB *) running->TCB)->TID) {
                ((TCB *) running->TCB)->state = WAITING;
                Log((int) getTicks()-startTime, STOPPED, ((TCB *) running->TCB)->TID, -1);
                ((TCB *) currentNode->TCB)->joined = running->TCB;
                schedule();
            } else {
                //attempting a circular join
                return FAILURE;
            }
        }
        //case 3: TID does exist and thread is ready to go! (set calling thread to waiting by this thread and set joined pointer)
        ((TCB *) running->TCB)->state = WAITING;
        ((TCB *) currentNode->TCB)->joined = running->TCB;
        schedule();

//        // set state of running node to waiting
//        ((TCB *) running->TCB)->state = WAITING;

        //set tid sought node as running is waiting on it


//        //make sure main thread waits
//        if(((TCB*) running->TCB)->TID == -1) {
//            printf("hello\n");
//            getcontext(mainTCB->ucontext); //TODO: determine why I need to save main here?!?
//        }
////        getcontext(mainTCB->ucontext);
//        printf("running %d\n", ((TCB*) running->TCB)->TID);
//        printf("main %d\n", mainTCB->TID);
//        schedule();

        return SUCCESS;
    }
    return FAILURE;
}

int stub(void (*func)(void *), void *arg) {
    printf("entered stub\n");
    // thread starts here
    func(arg); // call root function
    //TODO: thread clean up mentioned in assignment guidelines on page 3
    printf("thread done\n");
    //Log((int) getTicks()-startTime, FINISHED, 1, -1); //TODO: fix logging here
    Log((int) getTicks()-startTime, FINISHED, ((TCB *) running->TCB)->TID, -1);
    ((TCB*) running->TCB)->state = DONE;
    ((TCB*) running->TCB)->joined->state = READY;
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
    }
    else {
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
int schedule() {

    if (POLICY == FIFO) {
        //TODO: ensure this interaction is masked
        //In FIFO run the head of the ready queue and make the global running tcb correct
        //change state

//        if(((TCB*) running->TCB)->TID == -1) {
//            //we know that we have the main context trying to make a thread join and that's the thread to suspend
//            mainTCB->state = WAITING;
//            node *toRun = readyList->head;
//            readyList->head = toRun->next;
//            readyList->head = NULL;
//            toRun->next = NULL;
//            ((TCB *) toRun->TCB)->state = RUNNING;
//            running = toRun;
//            setcontext(((TCB*) running->TCB)->ucontext);
//        } else { //the thread running isn't main, so we don't have to worry about updating the main's context
//            ((TCB*) readyList->head->TCB)->state = RUNNING;
//            TCB *lastRunning = running->TCB;
//            running->TCB = ((TCB*) readyList->head->TCB);
//            lastRunning->state = WAITING;
//            swapcontext(lastRunning->ucontext, ((TCB*) running->TCB)->ucontext);
//        }
        printf("entered scheduler\n");
//        if (readyList->head != NULL) {
//            printf("running %d\n", ((TCB*) running->TCB)->TID);
//            //take node to run out of queue
//            node *toRun = readyList->head;
//            readyList->head = toRun->next;
//            readyList->head = NULL;
//            toRun->next = NULL;
//            ((TCB *) toRun->TCB)->state = RUNNING;
//
//            printf("running two %d\n", ((TCB*) running->TCB)->TID);
//
//            //put currently running into queue with a state of waiting at the tail
//            if(((TCB*) running->TCB)->state==READY) {
//                node *currentTail = readyList->tail;
//                currentTail->next = running;
//                running->prev = currentTail;
//                running->next = NULL;
//                readyList->tail = running;
//
//                if (readyList->head == NULL) {
//                    readyList->head = running;
//                }
//            }
//
//            running = toRun;

        node *currentNode = readyList->head;
        while(currentNode!=NULL && ((TCB *) currentNode->TCB)->state!=READY) {
            currentNode = currentNode->next;
        }

        //now current node is ready to run, so have to run it here
        Log((int) getTicks()-startTime, SCHEDULED, ((TCB *) currentNode->TCB)->TID, -1);
        if(((TCB*) running->TCB)->TID == -1) { //update main
            printf("hello\n");
            running = currentNode;
//            setcontext(((TCB *) currentNode->TCB)->ucontext);
            swapcontext(mainTCB->ucontext, ((TCB *) currentNode->TCB)->ucontext); //TODO: determine why I need to save main here?!?
        } else {
            printf("hello from not main\n");
            getcontext(((TCB *) running->TCB)->ucontext);
            running = currentNode;
            setcontext(((TCB *) currentNode->TCB)->ucontext);
        }

//            printf("running TID %d\n", ((TCB *) running->TCB)->TID);
//            if(((TCB *) running->TCB)->ucontext != NULL) {
//                setcontext(((TCB *) running->TCB)->ucontext);
//            }
        //}
    } else if (POLICY == SJF) {

    } else if (POLICY == PRIORITY) {

    }
}

void printList() {
    node *currentNode = readyList->head;
    while(currentNode!=NULL) {
        printf("%d->", ((TCB *) currentNode->TCB)->TID);
        currentNode = currentNode->next;
    }
    printf("\n");
}