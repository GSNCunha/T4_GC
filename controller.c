#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define NIVEL_RP 0.8
#define KP 10
#define KI 1
#define KD 3
#define M_PI 3.14159265358979323846

void *start_controller() {
    double Ang_saida_anterior = 100;
    double erro_anterior = 0;
    double erro_acumulado = 0;
    char buffer[100];

    // Inicializa o sistema de controle
    buffer_put_string(&command_ccb, "SetMax#100!");
    sleepMs(1000);
    buffer_put_string(&command_ccb, "Start!");
    sleepMs(1000);
    buffer_put_string(&command_ccb, "OpenValve#000#50!");
    sleepMs(1000);
    
    while(1) {
        double nivel = buffer_get(&nivel_ccb); // Obtém o nível atual do buffer
        buffer_put(&nivel_ccb_graph, nivel);  // Envia o nível para o gráfico

        if(nivel != 0) {
            double erro = NIVEL_RP - nivel;

            // Termo Proporcional
            double P_term = erro * KP;

            // Termo Derivativo
            double D_term = (erro - erro_anterior) * KD;

            // Termo Integral com anti-windup
            double I_term;
            if(!(fabs(D_term + P_term + (erro_acumulado + erro) * KI) > 1)) {
                erro_acumulado += erro;
                I_term = erro_acumulado * KI;
            } else {
                I_term = erro_acumulado * KI;
            }

            // Sinal de controle é a soma dos termos P, I e D
            double Sinal_controle = P_term + D_term + I_term;

            // Limita o sinal de controle para [-1, 1]
            if(Sinal_controle >= 1) {
                Sinal_controle = 1;
            } else if (Sinal_controle <= -1) {
                Sinal_controle = -1;
            }

            // Calcula o ângulo de saída com base no sinal de controle
            double Ang_saida = asin(Sinal_controle) * (200 / M_PI);
            Ang_saida = round(Ang_saida * 100.0) / 100.0;

            // Calcula a variação do ângulo de saída
            double delta = Ang_saida - Ang_saida_anterior;

            char seq_str[4]; // Para armazenar a sequência como string
            char buffer_delta_str[10] = {0}; // Para armazenar delta como string
            int seq = rand() % 900 + 100; // Gera um número aleatório de 3 dígitos

            // Converte seq e delta para strings
            sprintf(seq_str, "%d", seq);
            memset(buffer_delta_str, 0, sizeof(buffer_delta_str)); // Reseta o buffer
            sprintf(buffer_delta_str, "%.2f", fabs(delta));

            // Monta a mensagem de comando com base na variação do ângulo
            if(delta != 0) {
                if (delta > 0) {
                    strcpy(buffer, "OpenValve#");
                } else if (delta < 0) {
                    strcpy(buffer, "CloseValve#");
                }

                strcat(buffer, seq_str);
                strcat(buffer, "#");
                strcat(buffer, buffer_delta_str);
                strcat(buffer, "!");

                buffer_put_string(&command_ccb, buffer);
            }

            buffer_put(&angleIn_ccb, round(Ang_saida)); // Armazena o ângulo de entrada
            Ang_saida_anterior = Ang_saida; // Atualiza o ângulo anterior
            erro_anterior = erro; // Atualiza o erro anterior
        }

        sleepMs(100);
        buffer_put_string(&command_ccb, "GetLevel!"); // Solicita o nível do sistema
        sleepMs(800);
    }
}