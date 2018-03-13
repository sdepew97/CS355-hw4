//
// Created by Sarah Depew on 3/13/18.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

int LogCreated = FALSE;

int main() {
    Log(100, SCHEDULED, 1, -1);
    Log(200, STOPPED, 1, -1);
    return 0;
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

        fprintf(file, "[%d]\t\t%s\t\t\t%d\t\t\t%d\n", ticks, states[OPERATION], TID, PRIORITY);
    }

    if (file) {
        fclose(file);
    }
}