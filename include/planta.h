#ifndef SIMULATE_PLANT_H
#define SIMULATE_PLANT_H

#include <time.h>

// Definição de constantes
#define M_PI 3.14159265358979323846
#define PLANT_PERIOD 10

// Declaração de variáveis globais
extern double delta;
extern double max;
extern double level;
extern double anguloIn;
extern double anguloOut;
extern double dT;
extern double fluxIn;
extern double fluxOut;

// Definição da estrutura de dados
struct DataPlant {
    int newMsg;
    char keyword[100]; // Comando como "OpenValve" etc.
    int seq;
    int value;
    double level;
};

// Declaração de funções
int get_angle_out(long T);
void *simulate_plant(struct DataPlant *DataReceived);

#endif // SIMULATE_PLANT_H
