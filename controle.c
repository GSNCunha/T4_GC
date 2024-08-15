/*
o client tem que ter acesso ao nivel por IP e mandar o controle por ip
o client tem que ter acesso ao nivel ae a abertur atual da valvula para plotagem, a cada 50ms

o server tem que simular a planta a cada 10ms
o server precisa plotar o nivel, var de entrada e de saídaa cada 50ms
 */

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define M_PI 3.14159265358979323846
#define PLANT_PERIOD 10

double delta;
double max;
double level;
double anguloIn;
double anguloOut;
double dT = 10; //em ms
double fluxIn;
double fluxOut;

struct DataPlant {
    int newMsg;
    char keyword[100]; // open valve etc.
    int seq;
    int value;
    double level;
};

void *simulate_plant(struct DataPlant *DataReceived) {
    long simulationTime = 0; // em ms
    struct timespec start, end, sleepTime, elapsed;

    while (1) {
        // Get the start time
        clock_gettime(CLOCK_MONOTONIC, &start);

        if (DataReceived != NULL) {
            if (strcmp(DataReceived->keyword, "OpenValve") == 0) {
                delta += DataReceived->value;
            } else if (strcmp(DataReceived->keyword, "CloseValve") == 0) {
                delta -= DataReceived->value;
            } else if (strcmp(DataReceived->keyword, "SetMax") == 0) {
                max = DataReceived->value;
            }

            if (delta > 0) {
                if (delta < 0.01 * dT) {
                    anguloIn = anguloIn + delta;
                    delta = 0;
                } else {
                    anguloIn = anguloIn + 0.01 * dT;
                    delta -= 0.01 * dT;
                }
            } else if (delta < 0) {
                if (delta > -0.01 * dT) {
                    anguloIn = anguloIn + delta;
                    delta = 0;
                } else {
                    anguloIn = anguloIn - 0.01 * dT;
                    delta += 0.01 * dT;
                }
            }

            if (strcmp(DataReceived->keyword, "Start") == 0) {
                simulationTime = 0;
                anguloIn = 50;
                level = 0.4;
            }
        }

        if (simulationTime == 0) {
            anguloIn = 50;
            level = 0.4;
        }

        fluxIn = 1 * sin(M_PI / 2 * anguloIn / 100);
        fluxOut = (max / 100) * (level / 1.25 + 0.2) * sin(M_PI / 2 * anguloOut / 100);
        level = level + 0.00002 * dT * (fluxIn - fluxOut);

        // Get the end time
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate the elapsed time
        elapsed.tv_sec = end.tv_sec - start.tv_sec;
        elapsed.tv_nsec = end.tv_nsec - start.tv_nsec;
        if (elapsed.tv_nsec < 0) {
            elapsed.tv_sec -= 1;
            elapsed.tv_nsec += 1000000000L;
        }

        // Calculate the remaining time to sleep
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = PLANT_PERIOD * 1000000L - elapsed.tv_nsec;

        // Sleep for the remaining time if any
        if (sleepTime.tv_nsec > 0) {
            nanosleep(&sleepTime, NULL);
        }

        simulationTime += PLANT_PERIOD;
    }
}