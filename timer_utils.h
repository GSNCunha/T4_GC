// timer_utils.h

#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H

#include <time.h>

// Returns the time passed in milliseconds since the given start time.
double getTimePassed(clock_t startTime);

// Pauses the execution of the program for the specified number of milliseconds.
void sleepMs(int milliseconds);

#endif // TIMER_UTILS_H
