#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "timer_utils.h"
#include "buffer_code.h"

#define M_PI 3.14159265358979323846 // Definição do valor de PI
#define PLANT_PERIOD 10 // Período da planta em milissegundos

// Declaração das variáveis globais
double delta;
double max = 100;
double level = 0.4;
double anguloIn = 50;
double anguloOut;
double dT = 10; // Intervalo de tempo em milissegundos
double fluxIn;
double fluxOut;
long simulationTime = 0;

clock_t startSimulationTime; // Armazena o tempo inicial da simulação

// Função para obter o ângulo de saída em função do tempo T (em milissegundos)
int get_angle_out(long T) {
    if (T <= 0) return 50;
    if (T <= 20000) return (50 + T / 400);
    if (T <= 30000) return 100;
    if (T <= 50000) return (100 - (T - 30000) / 250);
    if (T <= 70000) return (20 + (T - 50000) / 1000);
    if (T <= 100000) return (40 + 20 * cos((T - 70000) * 2 * M_PI / 10000));
    return 100; // Retorno padrão se T > 100000
}

// Função que simula a planta
void *simulate_plant() {

    // Inicia a simulação:
    simulationTime = 0; // Tempo de simulação em milissegundos
    startSimulationTime = clock(); // Armazena o clock do início da simulação
    buffer_put(&Start_scb, 0); // Envia o sinal de início para o buffer

    // Aguarda o comando "Start" para iniciar a simulação
    while (1) {
        MessageData DataReceived;

        DataReceived = buffer_get_MessageData(&messageData_scb);
        if (strlen(DataReceived.keyword) > 0) {  // Verifica se a palavra-chave não está vazia
            if (strcmp(DataReceived.keyword, "Start") == 0) {
                anguloIn = 50;
                level = 0.4;
                simulationTime = 0;
                buffer_put(&Start_scb, 1); // Envia sinal de início para o buffer
                break;
            }
        }
        buffer_put(&nivel_scb, 0.4); // Envia o nível inicial ao buffer
        buffer_put(&tempo_scb, 0); // Envia o tempo inicial ao buffer
        buffer_put(&angleIn_scb, 50); // Envia o ângulo de entrada inicial ao buffer
        buffer_put(&angleOut_scb, 50); // Envia o ângulo de saída inicial ao buffer
    }

    // Laço principal da simulação
    while (1) {

        MessageData DataReceived;

        DataReceived = buffer_get_MessageData(&messageData_scb);

        if (strlen(DataReceived.keyword) > 0) {  // Verifica se a palavra-chave não está vazia
            if (strcmp(DataReceived.keyword, "OpenValve") == 0) {
                delta += DataReceived.value; // Ajusta o valor de delta ao abrir a válvula
            } else if (strcmp(DataReceived.keyword, "CloseValve") == 0) {
                delta -= DataReceived.value; // Ajusta o valor de delta ao fechar a válvula
            } else if (strcmp(DataReceived.keyword, "SetMax") == 0) {
                max = DataReceived.value; // Define o valor máximo permitido
            } else if (strcmp(DataReceived.keyword, "GetLevel") == 0) {
                // Resposta foi dada em serverUDP (não há ação aqui)
            } else if (strcmp(DataReceived.keyword, "Start") == 0) {
                anguloIn = 50;
                level = 0.4;
                simulationTime = 0;
                buffer_put(&Start_scb, 1); // Reinicia a simulação
            }
        }

        // Ajuste do ângulo de entrada baseado no delta
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

        // Cálculo dos fluxos de entrada e saída
        fluxIn = 1 * sin(M_PI / 2 * anguloIn / 100);
        fluxOut = (max / 100) * (level / 1.25 + 0.2) * sin(M_PI / 2 * get_angle_out(simulationTime) / 100);

        // Atualização do nível do tanque
        level = level + (0.00002 * dT * (fluxIn - fluxOut));
        if (level >= 1) {
            level = 1;
        }

        // Envio dos valores atualizados para os buffers correspondentes
        buffer_put(&nivel_scb, level);
        buffer_put(&tempo_scb, simulationTime);
        buffer_put(&angleIn_scb, anguloIn);
        buffer_put(&angleOut_scb, get_angle_out(simulationTime));

        // Pausa de 10 ms para sincronizar a simulação
        sleepMs(10);

        // Atualiza o tempo de simulação
        simulationTime += PLANT_PERIOD;
    }
}