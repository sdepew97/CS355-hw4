//
// Created by Sarah Depew on 3/21/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include "userthread.h"

#define N 128

void foo(void) {
    poll(NULL, 0, 200);
}

int main(void) {
    printf(" * Running 129 threads! \n");
    printf(" * We want to have this behave like a simple Round Robin scheduling algorithm with swapping out occurring. No memory leaks!\n");

    if (thread_libinit(PRIORITY) == -1)
        exit(EXIT_FAILURE);

    int tids[N];
    memset(tids, -1, sizeof(tids));

    for (int i = 0; i < N; i++)  {
        tids[i] = thread_create(foo, NULL, 1);
    }

    for (int i = 0; i < N; i++)  {
        if (tids[i] == -1)
            exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++)  {
        if (thread_join(tids[i]) == -1)
            exit(EXIT_FAILURE);
    }

    if (thread_libterminate() == -1)
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
