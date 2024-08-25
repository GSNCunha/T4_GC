#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "buffer_code.h"
#include "graph_server.h"
#include "serverUDP.h"
#include "planta.h"


int main(int argc, char *argv[]){

     if (argc != 2) {
        fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
        exit(1);
    }
    
    pthread_t graph_server, udp_server, plant_server;

    buffer_init(&nivel_scb);
    buffer_init(&nivel_scb_graph);
    buffer_init(&tempo_scb);
    buffer_init(&angleIn_scb);
    buffer_init(&angleOut_scb);
    buffer_init(&Start_scb);
    buffer_init_string(&command_scb);
    buffer_init_MessageData(&messageData_scb);

    pthread_create(&plant_server, NULL, simulate_plant, NULL);
    pthread_create(&graph_server, NULL, plot_graph, NULL);
    pthread_create(&udp_server, NULL, start_server, (void *)&argv[1]);

    while(1){
        // Prompt the user to enter a message
        //printf("Enter message (or type 'exit' to quit): ");
        char buffer[BUFFSIZE];
        fgets(buffer, BUFFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove the newline character

        // Exit if the user types 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
            //fechar o bgl todo
        }
    }
    pthread_cancel(plant_server);
    pthread_cancel(graph_server);
    pthread_cancel(udp_server);

    pthread_join(plant_server, NULL);
    pthread_join(graph_server, NULL);
    pthread_join(udp_server, NULL);
    system("clear");
    printf("Server closed!\n");

    return 0;
}