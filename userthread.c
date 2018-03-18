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
#define LOGFILE	"log.txt\0"     // all Log(); messages will be appended to this file
#define QUANTA 100

//globals for logging
enum {CREATED, SCHEDULED, STOPPED, FINISHED};
char* states[] = {"CREATED\0", "SCHEDULED\0", "STOPPED\0", "FINISHED\0"};

//global variable to store the scheduling policy
static int POLICY; //policy for scheduling that the user passed
static int TID = 1; //start TID at 1 and get new TID's after that
static int startTime;
static int LogCreated = FALSE; //know if we append or not to the log.txt file
static int totalRuntime = 0;
static int totalRuns = 0;

//structs used in program
typedef struct TCB {
    int TID;
    ucontext_t *ucontext;
    unsigned int last;
    unsigned int secondToLast;
    unsigned int thirdToLast;
    unsigned int average; //sum of last, secondToLast, and thirdToLast over three
    unsigned int CPUusage;
    unsigned int priority;
    unsigned int state;
    struct TCB *joined;
} TCB;

enum {
    READY = 0, //ready when joined
    WAITING = 1, //waiting when has joined something that's now ready to run
    RUNNING = 2, //running when it has been joined
    BLOCKED = 3, //created and blocked in non-preemptive, since not joined in non-preemptive and when not joined in non-preemptive, then we don't run the thread!
    DONE = 4 //when thread is done running and it should be removed from the ready queue
};

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

//linkedList for non-preemptive
static linkedList *readyList = NULL;
static linkedList *lowList = NULL;
static linkedList *mediumList = NULL;
static linkedList *highList = NULL;

//the TCB for the main thread
static TCB *mainTCB = NULL;

//the current running thread's node
static node *running = NULL;

//the ucontext for the scheduler method that we switch to as needed
static ucontext_t *scheduler = NULL;

//extra local helper method declarations
static void stub(void (*func)(void *), void *arg);
static long getTicks();
static void Log (int ticks, int OPERATION, int TID, int PRIORITY);    // logs a message to LOGFILE
static void schedule();
static void printList(); //TODO: remove, since for debugging
static ucontext_t *newContext(ucontext_t *uc_link, void (*func)(void *), void* arg);
static TCB* newTCB(int TID, int CPUUsage, int priority, int state, TCB *joined);
static node* newNode(TCB *tcb, node* next, node* prev);
static int addNode(TCB *tcb, linkedList *list);
static int moveToEnd(node *nodeToMove);

int thread_libinit(int policy) {
    //this is when the program officially started
    startTime = (int) getTicks();

    //create context for scheduler
    scheduler = newContext(NULL, (void (*)(void *)) scheduler, NULL);
    if(scheduler == NULL) {
        return FAILURE;
    }

    makecontext(scheduler, (void (*)(void)) schedule, 0);

    //create main's TCB
    mainTCB = newTCB(-1, 0, 1, READY, NULL);
    if(mainTCB == NULL) {
        return FAILURE;
    }

    if(getcontext(mainTCB->ucontext) == FAILURE) {
        return FAILURE;
    }

    //set the global policy value
    POLICY = policy;

    //if we are scheduling in a non-preemptive fashion
    if (policy == FIFO || policy == SJF) {

        //TODO: free memory malloced here at the end!

        //create the ready list
        readyList = malloc(sizeof(linkedList));

        //if malloc failed, return FAILURE
        if (readyList == NULL) {
            return FAILURE;
        }

        //set ready list's value to running and update the size as necessary
        if(addNode(mainTCB, readyList) == FAILURE) {
            return FAILURE;
        }
        running = readyList->head; //have to set running to the proper node, main, here
        mainTCB->state = RUNNING;

        //LOG main's creation
        Log((int) getTicks() - startTime, CREATED, -1, -1);

        //everything went fine, so return success
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

    //This means that we have not called threadlib_init first, which is required
    if(running == NULL) {
        return FAILURE;
    }
    printList();

    //TODO: mask access to global variable!?
    int currentTID = TID;

    if (POLICY == FIFO || POLICY == SJF) {
        ucontext_t *newThread = newContext(NULL, func, arg);
        if(newThread == NULL) {
            return FAILURE;
        }

        makecontext(newThread, (void (*)(void)) stub, 2, func, arg);

//        TCB *newThreadTCB = newTCB(currentTID, 0, priority, BLOCKED, NULL);
        TCB *newThreadTCB = newTCB(currentTID, 0, priority, READY, NULL);
        newThreadTCB->ucontext = newThread;
        TID++; //TODO: MASK!!

        if(addNode(newThreadTCB, readyList) == FAILURE) {
            return FAILURE;
        }
        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        printList();
        return currentTID;
    } else { //we are priority scheduling
        ucontext_t *newThread = newContext(NULL, func, arg);
        if(newThread == NULL) {
            return FAILURE;
        }

        makecontext(newThread, (void (*)(void)) stub, 2, func, arg);

        TCB *newThreadTCB = newTCB(currentTID, 0, priority, READY, NULL); //For preemptive, don't require a join to run the thread, since the scheduler is called with SIGALARM
        newThreadTCB->ucontext = newThread;
        TID++; //TODO: MASK!!

        if (priority == -1) {
            if(addNode(newThreadTCB, lowList) == FAILURE) {
                return FAILURE;
            }
        } else if (priority == 0) {
            if(addNode(newThreadTCB, mediumList) == FAILURE) {
                return FAILURE;
            }
        } else if (priority == 1) {
            if(addNode(newThreadTCB, highList) == FAILURE) {
                return FAILURE;
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
    printf("yield hit\n");

    //This means that we have not called threadlib_init first, which is required
    if (running == NULL) {
        return FAILURE;
    }

    //put current running back onto the ready queue

    //change state of running thread to ready

    //ensure that running thread is back of the ready queue

    //call scheduler for the threads

    if (POLICY == FIFO || POLICY == SJF) {
        //running node is in the list, so have to 1) find it (have a pointer to it rn), 2) move it to the tail
        if (moveToEnd(running) == FAILURE) {
            return FAILURE;
        } else {
            running->tcb->state = READY;
            Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
            swapcontext(running->tcb->ucontext, scheduler);
            return SUCCESS;
        }
    } else {
        //TODO: fill in code here for priority
    }

    return FAILURE;
}

int thread_join(int tid) {
    printf("join called for %d\n", tid);

    //This means that we have not called threadlib_init first, which is required
    if(running == NULL) {
        return FAILURE;
    }
    printf("currently running %d\n", running->tcb->TID);
    printList();

    if (POLICY == FIFO || POLICY == SJF) {
        //move main to tail, since this must be the first join
        if(running->tcb->TID == -1 && readyList->head->tcb->TID == -1) { //TODO: ensure this works for SJF as well!
            printf("got into if in join\n");
            readyList->head = running->next;
            readyList->head->prev = NULL;
            running->next = NULL;
            running->prev = readyList->tail;
            readyList->tail->next = running;
            readyList->tail = running;
        }
        printList();

        printf("got into FIFO or SJF\n");
        node *currentNode = readyList->head;

        //find the node to join
        while(currentNode!=NULL && currentNode->tcb->TID != tid) {
            currentNode = currentNode->next;
        }

        if(currentNode != NULL && currentNode->tcb->state != DONE) {
            // case 2: TID does exist and running thread is waiting already, which would mean you'd get stuck forever, perhaps?
            if (running->tcb->joined != NULL && running->tcb->joined->state == WAITING) {
                if (running->tcb->joined->TID != currentNode->tcb->TID) {
                    running->tcb->state = WAITING;
                    Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
                    currentNode->tcb->joined = running->tcb;
                    swapcontext(running->tcb->ucontext, scheduler);
                } else {
                    //attempting a circular join, so a failure should occur
                    printf("failed on circular\n");
                    return FAILURE;
                }
            }
            else {
                //case 3: TID does exist and thread is ready to go! (set calling thread to waiting by this thread and set joined pointer)
                running->tcb->state = WAITING;
                currentNode->tcb->joined = running->tcb;
                Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
                printList();
                swapcontext(running->tcb->ucontext, scheduler);
            }

            return SUCCESS;
        }
        else {

            //case 1: TID doesn't exist/thread already finished
            if (currentNode == NULL) {
                printf("failed on null/node with %d doesn't exist\n", tid);
                return FAILURE;
            }
        }
    }
    printf("got to end here\n");
    return FAILURE;
}

void stub(void (*func)(void *), void *arg) {
    printf("entered stub\n");
    // thread starts here
    func(arg); // call root function
    //TODO: thread clean up mentioned in assignment guidelines on page 3
    printf("thread done\n");
    Log((int) getTicks() - startTime, FINISHED, ((TCB *) running->tcb)->TID, -1);
    running->tcb->state = DONE;
    printList();
    if (running->tcb->joined != NULL) {
        running->tcb->joined->state = READY;
    }
    //current thread is done, so we must get a new thread to run
    setcontext(scheduler);
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
        printf("%d, state %d, policy %d->", currentNode->tcb->TID, currentNode->tcb->state, POLICY);

        currentNode = currentNode->next;
    }
    printf("NULL, list size %d", readyList->size);
    printf("\n");
}

ucontext_t *newContext(ucontext_t *uc_link, void (*func)(void *), void* arg) {
    //TODO: mask access to global variable!
    ucontext_t *returnValue = malloc(sizeof(ucontext_t)); //TODO: error check malloc
    if(returnValue == NULL) {
        return NULL;
    }
    if(getcontext(returnValue) == FAILURE) {
        return NULL;
    }

    returnValue->uc_link = uc_link;
    returnValue->uc_stack.ss_sp = malloc(STACKSIZE);
    if(returnValue->uc_stack.ss_sp == NULL) {
        return NULL;
    }
    returnValue->uc_stack.ss_size = STACKSIZE;
    //TODO: figure out what to do with masking here??

    return returnValue;
}

//TODO: masking and error checking, here
TCB* newTCB(int TID, int CPUUsage, int priority, int state, TCB *joined) {
    TCB *returnValue = malloc(sizeof(TCB));
    if(returnValue == NULL) {
        return NULL;
    }
    returnValue->ucontext = malloc(sizeof(ucontext_t));
    if(returnValue->ucontext == NULL) {
        return NULL;
    }
    returnValue->TID = TID;
    returnValue->CPUusage = CPUUsage;
    returnValue->priority = priority;
    returnValue->joined = malloc(sizeof(TCB));
    if(returnValue->joined == NULL) {
        return  NULL;
    }
    returnValue->joined = joined;
    returnValue->state = state;

    return returnValue;
}

node* newNode(TCB *tcb, node* next, node* prev) {
    node *returnValue = malloc(sizeof(node));
    if(returnValue == NULL) {
        return NULL;
    }
    returnValue->tcb = tcb;
    returnValue->next = next;
    returnValue->prev = prev;

    return returnValue;
}

int addNode(TCB *tcb, linkedList *list) {
    //TODO: mask this linked list interaction

    //NOTE: this case should not occur, as long as libinit has been called
    if (readyList->size == 0) {
        node *newThreadNode = newNode(tcb, NULL, NULL);
        if(newThreadNode == NULL) {
            return FAILURE;
        }
        list->head = newThreadNode;
        list->tail = newThreadNode;
        list->size++;
    } else { //there are other nodes on the list, so this node should be added to the tail, since it arrived last (best for FIFO)
        node *tailNode = list->tail;
        node *newThreadNode = newNode(tcb, NULL, tailNode);
        if(newThreadNode == NULL) {
            return FAILURE;
        }
        tailNode->next = newThreadNode;
        tailNode->prev = tailNode;
        list->tail = newThreadNode;
        list->size++;
    }

    return SUCCESS;
}

int moveToEnd(node *nodeToMove) {
    node *prev = nodeToMove->prev;
    node *next = nodeToMove->next;
    node *currentTail = readyList->tail;

    //moving head to tail
    if (readyList->head->tcb->TID == nodeToMove->tcb->TID) {
        readyList->head = next;
        readyList->head->prev = NULL;
        //can keep the new head's next pointer

        nodeToMove->next = NULL;
        currentTail->next = nodeToMove;
        nodeToMove->prev = currentTail;
        readyList->tail = nodeToMove;

        return SUCCESS;
    } else if(readyList->tail->tcb->TID == nodeToMove->tcb->TID) {
        //do nothing, since node to move is already the tail

        return SUCCESS;
    } else {
        //node to move is a middle node, so have to reset pointers accordingly
        nodeToMove->next = NULL;
        nodeToMove->prev = currentTail;
        currentTail->next = nodeToMove;
        prev->next = next;
        next->prev = prev;
        readyList->tail = nodeToMove;
    }

    return FAILURE;
}