//
// Created by Sarah Depew on 3/13/18.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

int main() {
    return 0;
}

void Log (int ticks, int OPERATION, int TID, int PRIORITY) {
    FILE *file;

    file = fopen(LOGFILE, "w");

    if (file == NULL) {
        //there was an error here...
        return;
    } else {

        fprintf(file, "[%d]\t%s\t%d\t\t%d", ticks, states[OPERATION], TID, PRIORITY);
        fclose(file);
    }

    if (file) {
        fclose(file);
    }
}