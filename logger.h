//
// Created by Sarah Depew on 3/13/18.
// Used tutorial found on http://simplestcodings.blogspot.com/2010/10/simple-logger-in-c.html
// Also used http://www.falloutsoftware.com/tutorials/c/c1.htm
//

#ifndef HW4_LOGGER_H
#define HW4_LOGGER_H

#define TRUE 1
#define FALSE 0
#define LOGFILE	"scheduleLogger.txt\0"     // all Log(); messages will be appended to this file
extern int LogCreated;      // keeps track whether the log file is created or not

enum {CREATED, SCHEDULED, STOPPED, FINISHED};
char* states[] = {"CREATED\0", "SCHEDULED\0", "STOPPED\0", "FINISHED\0"};
void Log (int ticks, int OPERATION, int TID, int PRIORITY);    // logs a message to LOGFILE

#endif //HW4_LOGGER_H
