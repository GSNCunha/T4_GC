#ifndef PLANT_SIMULATION_H
#define PLANT_SIMULATION_H
#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Constants
#define M_PI 3.14159265358979323846
#define PLANT_PERIOD 10

// Global Variables
extern double delta;
extern double max;
extern double level;
extern double anguloIn;
extern double anguloOut;
extern double dT; // em ms
extern double fluxIn;
extern double fluxOut;
extern long simulationTime;

extern clock_t startSimulationTime;

// Function Prototypes
int get_angle_out(long T);
void *simulate_plant();

#endif // PLANT_SIMULATION_H
