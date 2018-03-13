//
// Created by Sarah Depew on 3/13/18.
// Used tutorial found on http://simplestcodings.blogspot.com/2010/10/simple-logger-in-c.html
// Also used http://www.falloutsoftware.com/tutorials/c/c1.htm
//

#ifndef HW4_LOGGER_H
#define HW4_LOGGER_H
#define LOGFILE	"scheduleLogger.txt"     // all Log(); messages will be appended to this file

enum {CREATED, SCHEDULED, STOPPED, FINISHED};
char states[] = {"CREATED", "SCHEDULED", "STOPPED", "FINISHED"};
void Log (int ticks, int OPERATION, int TID, int PRIORITY);    // logs a message to LOGFILE

#endif //HW4_LOGGER_H
