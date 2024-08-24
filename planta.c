#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "timer_utils.h"
#include "buffer_code.h"

#define M_PI 3.14159265358979323846
#define PLANT_PERIOD 10

double delta;
double max = 100;
double level = 0.4;
double anguloIn = 50;
double anguloOut;
double dT = 10; //em ms
double fluxIn;
double fluxOut;
long simulationTime = 0;


clock_t startSimulationTime;

int get_angle_out(long T) { // T em ms
    if(T <= 0) return 50;
    if(T <= 20000) return (50 + T / 400);
    if(T <= 30000) return 100;
    if(T <= 50000) return (100 - (T - 30000) / 250);
    if(T <= 70000) return (20 + (T - 50000) / 1000);
    if(T <= 100000) return (40 + 20 * cos((T - 70000) * 2 * M_PI / 10000));
    return 100; // Default return if T > 100000
}

void *simulate_plant() {

    //catches the start of the simulation:
    simulationTime = 0; // em ms
    startSimulationTime = clock(); //pega o clock do inicio
    buffer_put(&Start_scb, 0);
    
        while(1)
    {
        MessageData DataReceived;

        DataReceived = buffer_get_MessageData(&messageData_scb);
        if (strlen(DataReceived.keyword) > 0) {  // Ensure the keyword is not empty
            if (strcmp(DataReceived.keyword, "Start") == 0) {
                anguloIn = 50;
                level =0.4;
                simulationTime = 0;
                buffer_put(&Start_scb, 1);
                break;
            }
        }
    buffer_put(&nivel_scb, 0.4);
    buffer_put(&tempo_scb, 0);
    buffer_put(&angleIn_scb, 50);
    buffer_put(&angleOut_scb, 50);

    }

    while (1) {

    MessageData DataReceived;

    DataReceived = buffer_get_MessageData(&messageData_scb);


    if (strlen(DataReceived.keyword) > 0) {  // Ensure the keyword is not empty
        if (strcmp(DataReceived.keyword, "OpenValve") == 0) {
            delta += DataReceived.value;
        } else if (strcmp(DataReceived.keyword, "CloseValve") == 0) {
            delta -= DataReceived.value;
        } else if (strcmp(DataReceived.keyword, "SetMax") == 0) {
            max = DataReceived.value;
        } else if (strcmp(DataReceived.keyword, "GetLevel") == 0) {
            // resposta foi dada em serverUDP
        } else if (strcmp(DataReceived.keyword, "Start") == 0) {
            anguloIn = 50;
            level = 0.4;
            simulationTime = 0;
            buffer_put(&Start_scb, 1);
        }
    }

    if(delta >0)
    {   
        if (delta < 0.01 * dT) {
            anguloIn = anguloIn + delta;
            delta = 0;
        } else {
            anguloIn = anguloIn + 0.01 * dT;
            delta -= 0.01 * dT;
        }
    }else if(delta <0)
    {
        if (delta > -0.01 * dT) {
            anguloIn = anguloIn + delta;
            delta = 0;
        } else {
            anguloIn = anguloIn - 0.01 * dT;
            delta += 0.01 * dT;
        }
    }
    //printf("anguloIn: %.2f", anguloIn);
    fluxIn = 1 * sin(M_PI / 2 * anguloIn / 100);
    fluxOut = (max / 100) * (level / 1.25 + 0.2) * sin(M_PI / 2 *  get_angle_out(simulationTime) / 100);
    level = level + (0.00002 * dT * (fluxIn - fluxOut));
    if(level >=1)
        level = 1;
    buffer_put(&nivel_scb, level);
    //printf("nivel: %.2f", level);
    //printf("\n");
    buffer_put(&tempo_scb, simulationTime);
    buffer_put(&angleIn_scb, anguloIn);
    buffer_put(&angleOut_scb, get_angle_out(simulationTime));

    sleepMs(10);

    simulationTime += PLANT_PERIOD;
    }
}

