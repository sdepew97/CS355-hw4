#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include "userthread.h"
#include "logger.h"

#define MAINPRIORITY 1
#define MAINTID -1
#define FAILURE -1
#define SUCCESS 0
#define TRUE 1
#define FALSE 0
#define LOGFILE	"log.txt\0"     // all Log(); messages will be appended to this file
#define QUANTA 100
#define INTERVAL_SECS 		0
#define INTERVAL_MICROSECS 	100000
#define VALUE_SECS 		0
#define VALUE_MICROSECS 100000
#define LOW -1
#define MEDIUM 0
#define HIGH 1

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
static sigset_t set;
static struct itimerval realt;

//structs used in program
typedef struct TCB {
    int TID;
    ucontext_t *ucontext;
    unsigned int usage1;
    unsigned int usage2;
    unsigned int usage3;
    unsigned int averageOfUsages; //sum of last, secondToLast, and thirdToLast over three
    unsigned int CPUusage; //TODO: remove this
    unsigned int start;
    unsigned int stop;
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
static TCB* newTCB(int TID, int usage1, int usage2, int usage3, int averageOfUsages, int CPUUsage, int start, int stop, int priority, int state, TCB *joined);
static node* newNode(TCB *tcb, node* next, node* prev);
static int addNode(TCB *tcb, linkedList *list);
static int moveToEnd(node *nodeToMove, linkedList *list);
static void shiftUsages(int newUsageValue, TCB *tcb);
static int computeAverage(TCB *tcb);
static void freeNode(node *nodeToFree);
static int removeNode(node *nodeToRemove);
static void setAverage(TCB *tcb);
void setrtimer(struct itimerval *ivPtr);
static int setupSignals(void);
static void sigHandler(int j, siginfo_t *si, void *old_context);

//TODO: Ask rachel about FIFO scheduling (DONE), ask her about masking for the methods (DONE), ask her about SJF and ask her about testing? (DONE) and all comments in body

//TODO: masking, then done!
int thread_libinit(int policy) {
    //this is when the program officially started
    startTime = (int) getTicks(); //TODO: move this for correct timing

    //create context for scheduler
    scheduler = newContext(NULL, (void (*)(void *)) scheduler, NULL);
    if (scheduler == NULL) {
        return FAILURE;
    }

    makecontext(scheduler, (void (*)(void)) schedule, 0);

    //create main's TCB
    mainTCB = newTCB(MAINTID, 0, 0, 0, QUANTA / 2, 0, (int) getTicks(), 0, MAINPRIORITY, READY, NULL);
    totalRuntime += QUANTA / 2; //TODO: ask about this here (DONE)
    totalRuns++;

    if (mainTCB == NULL) {
        return FAILURE;
    }

    if (getcontext(mainTCB->ucontext) == FAILURE) {
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
        if (addNode(mainTCB, readyList) == FAILURE) {
            return FAILURE;
        }
        running = readyList->head; //have to set running to the proper node, main, here
        mainTCB->state = RUNNING;

        //LOG main's creation
        Log((int) getTicks() - startTime, CREATED, MAINTID, MAINPRIORITY);

        //everything went fine, so return success
        return SUCCESS;
    } else if (policy == PRIORITY) {
        //TODO: setup queues here and the signal handler (DONE)
        if (setupSignals() == FAILURE) {
            return FAILURE;
        }

        setrtimer(&realt);
        if (setitimer(ITIMER_REAL, &realt, NULL) == FAILURE) {
            return FAILURE;
        }

        //create the lists
        lowList = malloc(sizeof(linkedList));
        if (lowList == NULL) {
            return FAILURE;
        }

        mediumList = malloc(sizeof(linkedList));
        if (mediumList == NULL) {
            return FAILURE;
        }

        highList = malloc(sizeof(linkedList));
        if (highList == NULL) {
            return FAILURE;
        }

        if (MAINPRIORITY == HIGH) {
            if (addNode(mainTCB, highList) == FAILURE) {
                return FAILURE;
            }
            else {
                running = highList->head; //have to set running to the proper node, main, here
            }
        } else if (MAINPRIORITY == MEDIUM) {
            if (addNode(mainTCB, mediumList) == FAILURE) {
                return FAILURE;
            }
            else {
                running = mediumList->head; //have to set running to the proper node, main, here
            }
        } else if (MAINPRIORITY == LOW) {
            if (addNode(mainTCB, lowList) == FAILURE) {
                return FAILURE;
            }
            else {
                running = lowList->head; //have to set running to the proper node, main, here
            }
        }

        mainTCB->state = RUNNING;

        //LOG main's creation
        Log((int) getTicks() - startTime, CREATED, MAINTID, MAINPRIORITY);

        //everything went fine, so return success
        return SUCCESS;
    } else {
        //passed in an invalid scheduling policy, which was stated to not occur, but could occur
        return FAILURE;
    }

    return FAILURE;
}

//TODO: free all memory here
int thread_libterminate(void) {
    //free all queues

    //free all thread memory malloced

    //free all TCB's etc...

    //mark main as finished and free
    return FAILURE;
}

//TODO: masking, then done!
int thread_create(void (*func)(void *), void *arg, int priority) {
    printf("creating new thread %d\n", TID);

    //This means that we have not called threadlib_init first, which is required
    if(running == NULL || func == NULL) {
        printf("Failure\n");
        return FAILURE;
    }
    printList();

    //TODO: mask access to global variable!? (YES)
    int currentTID = TID;

    if (POLICY == FIFO || POLICY == SJF) {
        ucontext_t *newThread = newContext(NULL, func, arg);
        if(newThread == NULL) {
            return FAILURE;
        }

        makecontext(newThread, (void (*)(void)) stub, 2, func, arg);
        TCB *newThreadTCB = newTCB(currentTID, 0, 0, 0, (totalRuntime/totalRuns), 0, 0, 0, priority, READY, NULL); //TODO: ask Rachel about this tonight (DONE)
        newThreadTCB->ucontext = newThread;
        TID++; //TODO: MASK!!

        if(addNode(newThreadTCB, readyList) == FAILURE) {
            return FAILURE;
        }
        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        printList();
        return currentTID;
    } else { //we are priority scheduling
        printf("we are creating in priority\n");
        ucontext_t *newThread = newContext(NULL, func, arg);
        if(newThread == NULL) {
            return FAILURE;
        }

        makecontext(newThread, (void (*)(void)) stub, 2, func, arg);

        TCB *newThreadTCB = newTCB(currentTID, 0, 0, 0, (totalRuntime/totalRuns), 0, 0, 0, priority, READY, NULL); //For preemptive, don't require a join to run the thread, since the scheduler is called with SIGALARM
        newThreadTCB->ucontext = newThread;
        TID++; //TODO: MASK!!

        if (priority == LOW) {
            if(addNode(newThreadTCB, lowList) == FAILURE) {
                return FAILURE;
            }
        } else if (priority == MEDIUM) {
            if(addNode(newThreadTCB, mediumList) == FAILURE) {
                return FAILURE;
            }
        } else if (priority == HIGH) {
            if(addNode(newThreadTCB, highList) == FAILURE) {
                return FAILURE;
            }
        } else {
            //priority is invalid
            return FAILURE;
        }

        Log((int) getTicks() - startTime, CREATED, currentTID, -1);
        printList();
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
        if (moveToEnd(running, readyList) == FAILURE) {
            return FAILURE;
        } else {
            running->tcb->state = READY;
            Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
            running->tcb->stop = (int) getTicks();
            totalRuntime += running->tcb->stop - running->tcb->start;
            totalRuns++;
            //TODO: (Yes, do this) ask Rachel here about shifting and averaging and the whole runtime thing...since this changes the runtime with the zero's going into computing the average (Mark's idea is to use latest if 1 or 2, but then average if three or more)
            shiftUsages(running->tcb->stop - running->tcb->start, running->tcb);
            setAverage(running->tcb);
            swapcontext(running->tcb->ucontext, scheduler);
            return SUCCESS;
        }
    } else {
        //TODO: fill in code here for priority
        if (running->tcb->priority == HIGH) {
            if (moveToEnd(running, highList) == FAILURE) {
                return FAILURE;
            }
        } else if (running->tcb->priority == MEDIUM) {
            if (moveToEnd(running, mediumList) == FAILURE) {
                return FAILURE;
            }
        } else if (running->tcb->priority == LOW) {
            if (moveToEnd(running, lowList) == FAILURE) {
                return FAILURE;
            }
        } else {
            return FAILURE;
        }

        running->tcb->state = READY;
        Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
        running->tcb->stop = (int) getTicks();
        totalRuntime += running->tcb->stop - running->tcb->start;
        totalRuns++;
        //TODO: (Yes, do this) ask Rachel here about shifting and averaging and the whole runtime thing...since this changes the runtime with the zero's going into computing the average (Mark's idea is to use latest if 1 or 2, but then average if three or more)
        shiftUsages(running->tcb->stop - running->tcb->start, running->tcb);
        setAverage(running->tcb);
        swapcontext(running->tcb->ucontext, scheduler);
        return SUCCESS;
    }

    return FAILURE;
}

int thread_join(int tid) {
    printf("join called for %d\n", tid);
    //TODO: ignore if joined something that is done/had been scheduled
    node *currentNode = NULL;

    //This means that we have not called threadlib_init first, which is required or thread is trying to join itself
    if (running == NULL || running->tcb->TID == tid) {
        return FAILURE;
    }
    printf("currently running %d\n", running->tcb->TID);
    printList();

    if (POLICY == FIFO || POLICY == SJF) {
        if(moveToEnd(running, readyList) == FAILURE) {
            return FAILURE;
        }
        printList();

        printf("got into FIFO or SJF\n");
        currentNode = readyList->head;

        //find the node to join
        while (currentNode != NULL && currentNode->tcb->TID != tid) {
            currentNode = currentNode->next;
        }
    } else if (POLICY == PRIORITY) {
        if (running->tcb->priority == HIGH) {
            if (moveToEnd(running, highList) == FAILURE) {
                return FAILURE;
            }
        } else if (running->tcb->priority == MEDIUM) {
            if (moveToEnd(running, mediumList) == FAILURE) {
                return FAILURE;
            }
        } else if (running->tcb->priority == LOW) {
            if (moveToEnd(running, lowList) == FAILURE) {
                return FAILURE;
            }
        } else {
            return FAILURE;
        }

        currentNode = highList->head;
        while (currentNode != NULL && currentNode->tcb->TID != tid) {
            currentNode = currentNode->next;
        }
        if(currentNode==NULL) {
            currentNode = mediumList->head;
            while (currentNode != NULL && currentNode->tcb->TID != tid) {
                currentNode = currentNode->next;
            }
            if (currentNode == NULL) {
                currentNode = lowList->head;
                while (currentNode != NULL && currentNode->tcb->TID != tid) {
                    currentNode = currentNode->next;
                }
            }
        }
    } else {
        return FAILURE;
    }

    if (currentNode != NULL && currentNode->tcb->state != DONE) {
        // case where TID does exist and running thread is waiting already, which would mean you'd get stuck forever, perhaps?
        if (running->tcb->joined != NULL && running->tcb->joined->state == WAITING) {
            if (running->tcb->joined->TID != currentNode->tcb->TID) {
                running->tcb->state = WAITING;
                Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
                running->tcb->stop = (int) getTicks();
                totalRuntime += running->tcb->stop - running->tcb->start;
                totalRuns++;
                shiftUsages(running->tcb->stop - running->tcb->start, running->tcb);
                setAverage(running->tcb);
                currentNode->tcb->joined = running->tcb;
                swapcontext(running->tcb->ucontext, scheduler);
            } else {
                //attempting a circular join, so a failure should occur
                printf("failed on circular\n");
                return FAILURE;
            }
        } else {
            //case where TID does exist and thread is ready to go! (set calling thread to waiting by this thread and set joined pointer)
            running->tcb->state = WAITING;
            currentNode->tcb->joined = running->tcb;
            Log((int) getTicks() - startTime, STOPPED, running->tcb->TID, -1);
            running->tcb->stop = (int) getTicks();
            totalRuntime += running->tcb->stop - running->tcb->start;
            totalRuns++;
            shiftUsages(running->tcb->stop - running->tcb->start, running->tcb);
            setAverage(running->tcb);
            printList();
            swapcontext(running->tcb->ucontext, scheduler);
        }
        return SUCCESS;
    } else {
        printf("in else\n");
        //case where TID doesn't exist/thread with that TID wasn't created
        if (currentNode == NULL && tid <= TID) { //TODO: bring back after doing swap function finish
//            if(currentNode != NULL && currentNode->tcb->state == DONE) {
            //shouldn't raise an error if trying to join a prior created thread that's already finished
            return SUCCESS;
        } else if (currentNode == NULL) {
            printf("failed on null with node %d\n", tid);
            return FAILURE;
        }
    }

    printf("got to end here\n");
    return FAILURE;
}

//TODO: clean up here
void stub(void (*func)(void *), void *arg) {
    printf("entered stub\n");
    // thread starts here
    func(arg); // call root function
    //TODO: thread clean up mentioned in assignment guidelines on page 3
    printf("thread done\n");
    Log((int) getTicks() - startTime, FINISHED, ((TCB *) running->tcb)->TID, -1);
    running->tcb->state = DONE;

    if (running->tcb->joined != NULL) {
        running->tcb->joined->state = READY;

        node *currentNode = readyList->head;
        while(currentNode!=NULL && currentNode->tcb->TID != running->tcb->joined->TID) {
            currentNode = currentNode->next;
        }

        printf("Current node TID %d, running node joined TID %d\n", currentNode->tcb->TID, running->tcb->joined->TID);

        //current node is now the one we're looking for
        if(POLICY == FIFO || POLICY == SJF) {
            moveToEnd(currentNode, readyList);
        } else {
            if(currentNode->tcb->priority == HIGH) {
                moveToEnd(currentNode, highList);
            } else if(currentNode->tcb->priority == MEDIUM) {
                moveToEnd(currentNode, mediumList);
            } else if(currentNode->tcb->priority == LOW) {
                moveToEnd(currentNode, lowList);
            }
        }
    }

    printf("Free node result %d\n", removeNode(running));
    printList();

    running = NULL;

    //current thread is done, so we must get a new thread to run
    setcontext(scheduler);
}

 long getTicks() {
    struct timeval time;
    gettimeofday(&time, NULL);
    //TODO: fix timing here
    long time_in_mill = (time.tv_sec) * 1000 + (time.tv_usec) / 1000 ;
    return time_in_mill;
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
    node *currentNode;

    if (POLICY == FIFO) {
        //TODO: ensure this interaction is masked
        currentNode = readyList->head;
        while (currentNode != NULL && ((TCB *) currentNode->tcb)->state != READY) {
            currentNode = currentNode->next;
        }

        //now current node is ready to run, so have to run it here
        running = currentNode;
        Log((int) getTicks() - startTime, SCHEDULED, currentNode->tcb->TID, -1);
        //start timing here
        running->tcb->start = (int) getTicks(); //TODO: Ask Rachel: milliseconds of same time?? account for seconds?
        running->tcb->state = RUNNING;
        printf("running TID %d\n", running->tcb->TID);
        printf("POLICY in schedule two: %d\n", POLICY);
        printList();
        setcontext(((TCB *) running->tcb)->ucontext);
    } else if (POLICY == SJF) {
        node *currentNode = readyList->head;
        int minRuntime = 0 ;
        node *minRuntimeNode = NULL; //TODO: should be set to main here and gonna get segfault if not!!

        //use a loop to find a ready candidate here in the list of nodes to use as a initial value
        while(currentNode != NULL && currentNode->tcb->state != READY) { //TODO: ask rachel if thread here can be scheduled as running in SJF?
            printf("Current node %d and state %d\n", currentNode->tcb->TID, currentNode->tcb->state);
            currentNode = currentNode->next;
        }
        //current node is now in a ready state, since at least one node must be ready to run in the list
        minRuntime = currentNode->tcb->averageOfUsages;
        minRuntimeNode = currentNode;

        currentNode = readyList->head;
        while (currentNode != NULL){
            printf("Current node tid %d\n", currentNode->tcb->TID);
            if(currentNode->tcb->state != READY) {
                currentNode = currentNode->next;
            }
            else {
                if(currentNode->tcb->averageOfUsages < minRuntime) {
                    minRuntime = currentNode->tcb->averageOfUsages;
                    minRuntimeNode = currentNode;
                    currentNode = currentNode->next;
                } else {
                    currentNode = currentNode->next;
                }
            }
        }

        //now currentNode is the node with min runtime, so run this node
        currentNode = minRuntimeNode;
        running = currentNode;
        Log((int) getTicks() - startTime, SCHEDULED, currentNode->tcb->TID, -1);
        //start timing here
        running->tcb->start = (int) getTicks(); //TODO: Ask Rachel: milliseconds of same time?? account for seconds?
        running->tcb->state = RUNNING;
        printf("running TID %d\n", ((TCB *) running->tcb)->TID);
        printf("POLICY in schedule two: %d\n", POLICY);
        printList();
        setcontext(running->tcb->ucontext);
    } else if (POLICY == PRIORITY) {
        //TODO: ensure thread runs for 100 miliseconds, so reset timer here each time I schedule a thread??
        printf("schedule\n");
        setcontext(mediumList->head->tcb->ucontext);
    }
}

void printList() {
    node *currentNode = NULL;

    if(PRIORITY == FIFO || PRIORITY == SJF) {
        currentNode = readyList->head;

        while (currentNode != NULL) {
            printf("%d, state %d, policy %d, average %d, u1 %d, u2 %d, u3 %d->", currentNode->tcb->TID, currentNode->tcb->state, POLICY, currentNode->tcb->averageOfUsages, currentNode->tcb->usage1, currentNode->tcb->usage2, currentNode->tcb->usage3);

            currentNode = currentNode->next;
        }

        printf("NULL, list size %d", readyList->size);

        printf("\n");
    } else {
        currentNode = highList->head;

        while (currentNode != NULL) {
            printf("%d, state %d, policy %d, average %d, u1 %d, u2 %d, u3 %d->", currentNode->tcb->TID, currentNode->tcb->state, POLICY, currentNode->tcb->averageOfUsages, currentNode->tcb->usage1, currentNode->tcb->usage2, currentNode->tcb->usage3);

            currentNode = currentNode->next;
        }

        printf("NULL, high list size %d", highList->size);
        printf("\n");

        currentNode = mediumList->head;

        while (currentNode != NULL) {
            printf("%d, state %d, policy %d, average %d, u1 %d, u2 %d, u3 %d->", currentNode->tcb->TID, currentNode->tcb->state, POLICY, currentNode->tcb->averageOfUsages, currentNode->tcb->usage1, currentNode->tcb->usage2, currentNode->tcb->usage3);

            currentNode = currentNode->next;
        }

        printf("NULL, medium list size %d", mediumList->size);
        printf("\n");

        currentNode = lowList->head;

        while (currentNode != NULL) {
            printf("%d, state %d, policy %d, average %d, u1 %d, u2 %d, u3 %d->", currentNode->tcb->TID, currentNode->tcb->state, POLICY, currentNode->tcb->averageOfUsages, currentNode->tcb->usage1, currentNode->tcb->usage2, currentNode->tcb->usage3);

            currentNode = currentNode->next;
        }

        printf("NULL, low list size %d", lowList->size);
        printf("\n");
    }
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
TCB* newTCB(int TID, int usage1, int usage2, int usage3, int averageOfUsages, int CPUUsage, int start, int stop, int priority, int state, TCB *joined) {
    TCB *returnValue = malloc(sizeof(TCB));
    if(returnValue == NULL) {
        return NULL;
    }
    returnValue->ucontext = malloc(sizeof(ucontext_t));
    if(returnValue->ucontext == NULL) {
        return NULL;
    }
    returnValue->TID = TID;
    returnValue->usage1 = usage1;
    returnValue->usage2 = usage2;
    returnValue->usage3 = usage3;
    returnValue->averageOfUsages = averageOfUsages;
    returnValue->CPUusage = CPUUsage;
    returnValue->start = start;
    returnValue->stop = stop;
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
    if (list->size == 0) {
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
        newThreadNode->prev = tailNode;
        list->tail = newThreadNode;
        list->size++;
    }

    return SUCCESS;
}

int moveToEnd(node *nodeToMove, linkedList *list) {
    printList();
    node *prev = nodeToMove->prev;
    node *next = nodeToMove->next;
    node *currentTail = list->tail;

    if (list->head->tcb->TID == list->tail->tcb->TID && list->head->tcb->TID == nodeToMove->tcb->TID) {
        //node is the only one in the list, so do nothing
        return SUCCESS;
    } else if (list->tail->tcb->TID == nodeToMove->tcb->TID) {
        //do nothing, since node to move is already the tail
        return SUCCESS;
    }
        //moving head to tail
    else if (list->head->tcb->TID == nodeToMove->tcb->TID) {
        list->head = next;
        list->head->prev = NULL;
        //can keep the new head's next pointer

        nodeToMove->next = NULL;
        currentTail->next = nodeToMove;
        nodeToMove->prev = currentTail;
        list->tail = nodeToMove;

        return SUCCESS;
    } else {
        //node to move is a middle node, so have to reset pointers accordingly
        nodeToMove->next = NULL;
        nodeToMove->prev = currentTail;
        currentTail->next = nodeToMove;
        prev->next = next;
        next->prev = prev;
        list->tail = nodeToMove;
        return SUCCESS;
    }

    return FAILURE;
}

void freeNode(node *nodeToFree) {
    //TODO: fill in body here
}

int removeNode(node *nodeToRemove) {
    node *prev = nodeToRemove->prev;
    node *next = nodeToRemove->next;

    if(readyList->head->tcb->TID == readyList->tail->tcb->TID && readyList->head->tcb->TID == nodeToRemove->tcb->TID) {
        //the node is the only one in the list, so we can simply remove it here
        readyList->head = NULL;
        readyList->tail = NULL;
        freeNode(nodeToRemove);
        readyList->size--;
    }
    //removing head with more than one node in the list (don't have to reset the tail)
    else if (readyList->head->tcb->TID == nodeToRemove->tcb->TID) {
        readyList->head = next;
        readyList->head->prev = NULL;
        //can keep the new head's next pointer

        nodeToRemove->next = NULL;
        nodeToRemove->prev = NULL;
        freeNode(nodeToRemove);
        readyList->size--;

        return SUCCESS;
    } else if(readyList->tail->tcb->TID == nodeToRemove->tcb->TID) { //remove node from tail
        readyList->tail = prev;
        readyList->tail->next = NULL;
        //can keep the new tail's prev pointer

        nodeToRemove->next = NULL;
        nodeToRemove->prev = NULL;
        freeNode(nodeToRemove);
        readyList->size--;
        return SUCCESS;
    } else {
        //node to move is a middle node, so have to reset pointers accordingly
        prev->next = next;
        next->prev = prev;

        nodeToRemove->next = NULL;
        nodeToRemove->prev = NULL;
        freeNode(nodeToRemove);
        readyList->size--;
        return SUCCESS;
    }

    return FAILURE;
}

void shiftUsages(int newUsageValue, TCB *tcb) {
    tcb->usage3 = tcb->usage2;
    tcb->usage2 = tcb->usage1;
    tcb->usage1 = newUsageValue;
}

int computeAverage(TCB *tcb) {
    return ((tcb->usage1+tcb->usage2+tcb->usage3)/3);
}

void setAverage(TCB *tcb) {
    tcb->averageOfUsages = computeAverage(tcb);
}

/*
  Initialize the ITIMER_REAL interval timer.
  Its interval is 100 milliseconds.  Its initial value is 100 milliseconds.
*/
void setrtimer(struct itimerval *ivPtr) {
    ivPtr->it_interval.tv_sec = INTERVAL_SECS;
    ivPtr->it_interval.tv_usec = INTERVAL_MICROSECS;
    ivPtr->it_value.tv_sec = VALUE_SECS;
    ivPtr->it_interval.tv_usec = VALUE_MICROSECS;
}

/* Set up SIGALRM signal handler */
//referenced https://gist.github.com/DanGe42/7148946
//referenced https://gist.github.com/aspyct/3462238
int setupSignals(void) {
    struct sigaction act;

    act.sa_sigaction = sigHandler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
//    act.sa_flags = SA_SIGINFO; //TODO: see if I also want SIGRESTART or any other flags here (texted Rachel...she said yes...)
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    if (sigaction(SIGALRM, &act, NULL) != 0) {
        return FAILURE;
    }

    return SUCCESS;
}

void sigHandler(int j, siginfo_t *si, void *old_context) {
    printf("hello world\n");
    Log(getTicks(), FINISHED, mainTCB->TID, 1);
    //save thread and go to scheduler
//    swapcontext(running->tcb->ucontext, scheduler); //TODO: bring this back...
}

//TODO: add masking here! :)
/*
 * sigset_t mask;

            if (sigemptyset(&mask) == ERROR) {
                perror("I am sorry, but sigemptyset failed.\n");
                exit(EXIT_FAILURE);
            }

            if (sigaddset(&mask, SIGCHLD) == ERROR) {
                perror("I am sorry, but sigaddset failed.\n");
                exit(EXIT_FAILURE);
            }
            if (sigprocmask(SIG_BLOCK, &mask, NULL) == ERROR) {
                perror("I am sorry, but sigprocmask failed.\n");
                exit(EXIT_FAILURE);
            }

            put_job_in_background(j, ZERO, RUNNING);

            if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == ERROR) {
                perror("I am sorry, but sigprocmask failed.\n");
                exit(EXIT_FAILURE);
            }
 */