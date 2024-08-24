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
    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}
