/*
basicamente essa planta pega s valores de uma struct construida no server.c que se referem ao profess

 */


#include <stdio.h>
#include <math.h>

#include <simulation.h>
#include <udp_common.h>
#include <shared.h>
#include <timer.h>

#define M_PI 3.14159265359

#define PERIOD_MS 10

void processAngleIn(long dt, Angle* delta, Angle* angleIn) {
    Angle maxDelta = 0.01*dt; 
    if (*delta > 0) {
        if (*delta < maxDelta) {
            *angleIn += *delta;
            *delta = 0;
        }
        else {
            *angleIn += maxDelta;
            *delta -= maxDelta; 
        }
    }
    else if (*delta < 0) {
        if (*delta > -maxDelta) {
            *angleIn += *delta;
            *delta = 0;
        }
        else {
            *angleIn -= maxDelta;
            *delta += maxDelta;
        }
    }
}

Angle getAngleOut(long t) {
    if (t <= 0) return 50;
    if (t < 20000) return (50+t/400);
    if (t < 30000) return 100;
    if (t < 50000) return (100-(t-30000)/250);
    if (t < 70000) return (20+(t-50000)/1000);
    if (t < 100000) return (40+20*cos((t-70000)*2*M_PI/10000));
    return 100;
}
// essa função é criada para se comportar como uma thread, pois uma thread precisa receber ponteiros
void *simulation(void *sdptr) {
     //tank level, angle of the input valve, basicamente pega dados de uma estrutura sdptr que foi feita para ser acessada por varias parte do codigo
     //SharedData* serverData se refere a estrutura da struct
    SharedData* serverData = (SharedData*)sdptr;

    //pega os dados de serverData do tipo incomingQueue
    SCMQ* incomingQueue = serverData->incomingQueue;

    //pega os dados de serverData do tipo started
    int* started = &(serverData->started);

    //variavel mutex criada para poder ser usada para travar o acesso das variável level para poderem ser usadas 
    pthread_mutex_t* levelLock = &(serverData->levelLock);

     //pega os dados de serverData do tipo level
    Level* level = &(serverData->level);
    //variavel mutex criada para poder ser usada para travar o acesso das variável Angle para poderem ser usadas 
    pthread_mutex_t* angleLock = &(serverData->angleLock);
    Angle* angleIn = &(serverData->angleIn);
    Angle* angleOut = &(serverData->angleOut);

    //Other simulation data.
    Message* mes;
    struct timespec t_start, t;
    long deltaMs;
    Level nextLevel;
    Angle nextAngleIn, nextAngleOut;
    Angle deltaAngle;
    double max;
    double nextInflux, nextOutflux;

    // Runs simulation
    puts("[SIMULATION] Waiting start");
    getCurrentTime(&t);
    while (1) {
        // trava por delta segundos para executar
        while (( deltaMs= getPassedTimeMs(&t)) < PERIOD_MS);
        getCurrentTime(&t);

        
        while ((mes = SCMQdequeue(incomingQueue)) != NULL) { //enquanto há mensagens para serem lidas e executadas

            if (mes->messageType == START) {//se a mensagem for de começo de simulação pegamos as condições iniciais
                getCurrentTime(&t_start);
                t = t_start;

                nextAngleIn = INITIAL_ANGLE_IN;
                nextLevel = INITIAL_LEVEL;
                deltaAngle = 0;
                max = INITIAL_MAX;

                *started = 1;
                puts("[SIMULATION] Started");
                continue;
            }
            //as mudanças de angulo de entrada ja estão inclusas na estrutura da mensagem, nos campos deltaAngle
            else if (*started) {
                switch (mes->messageType) {
                    case OPEN_VALVE:
                        deltaAngle += mes->value;
                        break;
                    case CLOSE_VALVE:
                        deltaAngle -= mes->value;
                        break;
                    case SET_MAX:
                        max = mes->value;
                        break;
                    default:
                        break;
                }
            }
        }

        //Se não existem mensagens para serem lidas, vamos vicar atualizando os valores de angulos e nivel.
        if (!(*started)) continue;

        //Calculates next values.
        processAngleIn(deltaMs, &deltaAngle, &nextAngleIn);
        nextAngleOut = getAngleOut(getDeltaMs(&t_start, &t));
        nextInflux = sin(M_PI/2*nextAngleIn/100);
        nextOutflux = (max/100)*(nextLevel/1.25+0.2)*sin(M_PI/2*nextAngleOut/100);
        nextLevel += 0.00002*deltaMs*(nextInflux-nextOutflux);

        //trava as variáveis de nivel e as altera a partir da simulação da planta
        pthread_mutex_lock(levelLock);
        *level = nextLevel;
        pthread_mutex_unlock(levelLock);

        pthread_mutex_lock(angleLock);
        *angleIn = nextAngleIn;
        *angleOut = nextAngleOut;
        pthread_mutex_unlock(angleLock);
    }
}
