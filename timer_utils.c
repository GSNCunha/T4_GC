#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdbool.h>
#include <unistd.h>


// Global variable to store the start time
//clock_t startTime;
//bool startSimulation = false;

//void startTimer() {
//    startTime = clock();
//    startSimulation = true;
//}

double getTimePassed(clock_t startTime) {
    
    clock_t currentTime = clock();
    return ((double)(currentTime - startTime)) / CLOCKS_PER_SEC * 1000; // Returns time in milliseconds
}

void sleepMs(int milliseconds) {
    struct timespec req;
    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000L; // Convert milliseconds to nanoseconds
    nanosleep(&req, NULL);
}


long long get_elapsed_time_ms(struct timespec start_time) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);

    long long elapsed_sec = current_time.tv_sec - start_time.tv_sec;
    long long elapsed_ns = current_time.tv_nsec - start_time.tv_nsec;

    // Convert elapsed time to milliseconds
    return elapsed_sec * 1000 + elapsed_ns / 1e6;
}