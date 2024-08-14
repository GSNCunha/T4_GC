#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define NIVEL_RP 0.8
#define KP 0.001
#define M_PI 3.14159265358979323846

void *start_controller()
{
    double angulo = 50; 
    double Ang_saida_anterior;
    char buffer[100];
    while(1)
    {

    double Sinal_controle = (NIVEL_RP - buffer_get(&nivel_ccb))*KP;
    if(Sinal_controle >=1)
        Sinal_controle = 1;
    else if  (Sinal_controle >=-1)
        Sinal_controle = -1;

    double Ang_saida = asin(Sinal_controle)* (180.0 / M_PI);
    double delta = Ang_saida - Ang_saida_anterior;


            char seq_str[4]; // Para armazenar a sequência como string
            char buffer_delta_str[10] = {0}; // Para armazenar buffer_delta como string
            int seq = rand() % 900 + 100; // Gera um número aleatório de 3 dígitos

            // Converte o seq e buffer_delta para strings
            sprintf(seq_str, "%d", seq);
            sprintf(buffer_delta_str, "%d", abs(delta));

            if (Sinal_controle> 0) {
                // Constrói a mensagem para buffer_delta maior que zero
                strcpy(buffer, "OpenValve#222#1");
            } else if (Sinal_controle < 0) {
                // Constrói a mensagem para buffer_delta menor que zero
                strcpy(buffer, "CloseValve#222#1");
            }

            // Adiciona a sequência e o valor à mensagem
            //strcat(buffer, seq_str);
            //strcat(buffer, "#");
            //strcat(buffer, buffer_delta_str);
            strcat(buffer, "!");
    

    buffer_put_string(&command_ccb, buffer);

    Ang_saida_anterior = Ang_saida;

    sleepMs(950);
        buffer_put_string(&command_ccb,"GetLevel!");
    sleepMs(50);

    }
}