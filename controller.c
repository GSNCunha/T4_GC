#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define NIVEL_RP 80
#define KP 10
#define KI 1
#define KD 3
#define M_PI 3.14159265358979323846

void *start_controller()
{
    double Ang_saida_anterior = 100;
    double erro_anterior = 0;
    double erro_acumulado = 0;
    char buffer[100];

    buffer_put_string(&command_ccb, "SetMax#100!");
    //sleepMs(1000);
    buffer_put_string(&command_ccb, "Start!");
    //buffer_put(&Start_ccb, 1);
    //sleepMs(1000);
    buffer_put_string(&command_ccb, "OpenValve#001#50!");
    //sleepMs(1000);

    while(1)
    {
        double nivel = buffer_get(&nivel_ccb);
        buffer_put(&nivel_ccb_graph, nivel);
        
        if(nivel != 0)
        {
            double erro = (NIVEL_RP - nivel)/100;

            // Proportional term
            double P_term = erro * KP;

            // Derivative term
            double D_term = (erro - erro_anterior) * KD;

                    // Integral term
            double I_term;
            if(!(fabs(D_term + P_term + (erro_acumulado + erro)*KI) >1))//anti_windup
            {
                erro_acumulado += erro;
                I_term = erro_acumulado * KI;
            }else
            {
                I_term = erro_acumulado* KI;
            }

            // Control signal is the sum of P, I, and D terms
            double Sinal_controle = P_term + D_term+ I_term;

            // Clamp control signal to [-1, 1]
            if(Sinal_controle >= 1)
            {
                Sinal_controle = 1;
            }
            else if (Sinal_controle <= -1)
            {
                Sinal_controle = -1;
            }

            double Ang_saida = asin(Sinal_controle) * (200 / M_PI);
            Ang_saida = round(Ang_saida);
            /*if(fabs(sin(M_PI / 2 * Ang_saida  / 100) - sin(M_PI / 2 * Ang_saida_anterior / 100)) > 0.2)
            {
                Ang_saida = Ang_saida_anterior;
            }*/
            double delta;

                delta =(int) (Ang_saida - Ang_saida_anterior);
                

            char seq_str[4]; // To store the sequence as a string
            char buffer_delta_str[10] = {0}; // To store buffer_delta as a string
            int seq = rand() % 900 + 100; // Generate a random 3-digit number

            // Convert seq and buffer_delta to strings
            sprintf(seq_str, "%d", seq);
            memset(buffer_delta_str, 0, sizeof(buffer_delta_str)); // Reset buffer
            sprintf(buffer_delta_str, "%d", (int)fabs(delta));
           // printf("angulo de entrada: %.2f", Ang_saida);
           // printf("\n");

            if(delta != 0)
            {
                if (delta > 0) {
                    // Build the message for buffer_delta greater than zero
                    strcpy(buffer, "OpenValve#");
                } else if (delta < 0) {
                    // Build the message for buffer_delta less than zero
                    strcpy(buffer, "CloseValve#");
                }

                // Add the sequence and value to the message
                strcat(buffer, seq_str);
                strcat(buffer, "#");
                strcat(buffer, buffer_delta_str);
                strcat(buffer, "!");

                buffer_put_string(&command_ccb, buffer);
            }
            buffer_put(&angleIn_ccb, round(Ang_saida));
            Ang_saida_anterior = Ang_saida;
            erro_anterior = erro;
        }
            buffer_put_string(&command_ccb, "GetLevel!");
            sleepMs(1000);
    }
}