#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "buffer_code.h"
#include "graph_server.h"
#include "planta.h"


int main(){
    pthread_t graph_server_client, udp_server_client, controller_server_client;

    buffer_init(&nivel_scb);
    buffer_init(&tempo_scb);
    buffer_init(&angleIn_scb);
    buffer_init(&angleOut_scb);
    buffer_init(&Start_scb);
    buffer_init_string(&command_scb);
    buffer_init_MessageData(&messageData_scb);

    pthread_create(&graph_server_client, NULL, simulate_plant, NULL);
    pthread_create(&udp_server_client, NULL, plot_graph, NULL);
    pthread_create(&controller_server_client, NULL, start_server, NULL);

    pthread_join(graph_server_client NULL);
    pthread_join(udp_server_client, NULL);
    pthread_join(controller_server_client, NULL);

    return 0;
}