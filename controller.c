#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define NIVEL_RP 80
#define KP 0.01
#define M_PI 3.14159265358979323846

void *start_controller()
{
    double angulo = 50; 
    double Ang_saida_anterior;
    while(1)
    {

    double Sinal_controle = (NIVEL_RP - buffer_get(&nivel_controller))*KP;
    if(Sinal_controle >=1)
        Sinal_controle = 1;
    
    double Ang_saida = asin(Sinal_controle)* (180.0 / M_PI);
    double delta = Ang_saida - Ang_saida_anterior;

    buffer_put(&delta_ccb, delta);

    Ang_saida_anterior = Ang_saida;


    

    sleepMs(1000);

    }
}