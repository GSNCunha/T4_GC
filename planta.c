#include <math.h>
#include <stdio.h>
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


// nivel,
struct DataPlant
{
    int newMsg;
    char keyword[100]; //open valve etc etc
    int seq;
    int value;
    double level;

};


int get_angle_out(long T){ //T ta em ms 
    if(T<=0){
        return 50;
    }
    if(T<=20000){
        return (50+T/400);
    }
    if(T<=30000){
        return 100;
    }
    if(T<=50000){
        return (100-(T-30000)/250);
    }
    if(T<=70000){
        return (20+(T-50000)/1000);
    }
    if(T<=100000){
        return (40+20*cos((T-70000)*2*M_PI/10000));
    }
}

void *simulate_plant(DataPlant *DataReceived){
    long simulationTime = 0; //em ms
    while(1){
        
    if (DataReceived != NULL){
        if (DataReceived->msg == "OpenValve"){
            delta += DataReceived->value;
        }
        if (DataReceived->msg == "CloseValve"){
            delta -= DataReceived->value;
        }
        if (DataReceived->msg == "SetMax"){
            max = DataReceived->value;
        }

        if (delta > 0){
            if (delta < 0.01*dT){
                anguloIn = anguloIn + delta;
                delta = 0;
            }else{
                anguloIn = anguloIn + 0.01*dT;
                delta -= 0.01*dT;
            }

        }else{
            if (delta < 0){
                if (delta > -0.01*dT){
                    anguloIn = anguloIn + delta;
                    delta = 0;
                }else{
                    anguloIn = anguloIn - 0.01*dT;
                    delta += 0.01*dT;
                }
            }
        }
        
        if (DataReceived->msg == "Start"){
            simulationTime = 0;
            anguloIn = 50;
            level = 0.4;
        }
    }

    if (simulationTime == 0){
        anguloIn = 50;
        level = 0.4;
    }

        fluxIn = 1*sin(M_PI/2*anguloIn/100);
        fluxOut = (max/100)*(level/1.25+0.2)*sin(M_PI/2*anguloOut/100);
        level = level + 0.00002*dT*(fluxIn-fluxOut);
        //-----
        usleep(PLANT_PERIOD*1000); //periodo de 10ms para  cada ciclo
        simulationTime += 10;
    }

}