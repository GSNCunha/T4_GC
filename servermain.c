#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "buffer_code.h"
#include "graph.h"
#include "serverUDP.h"
#include "planta.h"


int main(){
    pthread_t graph_server, udp_server, plant_server;

    buffer_init(&nivel_cb);
    buffer_init(&tempo_cb);
    buffer_init(&angleIn_cb);
    buffer_init(&angleOut_cb);
    buffer_init_string(&command_cb);

    pthread_create(&plant_server, NULL, simulate_plant, NULL);
    pthread_create(&graph_server, NULL, plot_graph, NULL);
    pthread_create(&udp_server, NULL, start_server, NULL);

    pthread_join(plant_server, NULL);
    pthread_join(graph_server, NULL);
    pthread_join(udp_server, NULL);

    return 0;
}