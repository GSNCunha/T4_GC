// timer_utils.h

#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H
#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <time.h>

// Returns the time passed in milliseconds since the given start time.
double getTimePassed(clock_t startTime);

// Pauses the execution of the program for the specified number of milliseconds.
void sleepMs(int milliseconds);
long long get_elapsed_time_ms(struct timespec start_time);

#endif // TIMER_UTILS_H