#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

// Constantes definidas no código
#define NIVEL_RP 80
#define KP 0.01
#define M_PI 3.14159265358979323846

// Declaração da função de controle
void *start_controller();

#endif /* CONTROLLER_H */
