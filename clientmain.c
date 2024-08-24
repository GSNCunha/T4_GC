#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "buffer_code.h"
#include "graph_client.h"
#include "clientUDP.h"
#include "clientUDP_receive.h"
#include "controller.h"

#define BUFFSIZE 255


int main(int argc, char *argv[]){
    pthread_t graph_client, udp_client, udp_client_receive, controller_client; 

    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    buffer_init(&nivel_ccb_graph);
    buffer_init(&nivel_ccb); //clientUDP - > graph_client
    buffer_init(&tempo_ccb); //clientUDP - > graph_client
    buffer_init(&angleIn_ccb); //controle - > graph_client
    buffer_init(&Start_ccb); //clientUDP - > graph_client
    buffer_init(&Start_ccb_graph); //clientUDP - > graph_client
    buffer_init(&delta_ccb); //clientUDP - > graph_client
    buffer_init_string(&command_ccb); //controle - > clientUDP
    buffer_init_MessageData_client_receive(&messageData_client_receive_ccb);
    //buffer_init_MessageData(&command_ccb);

    pthread_create(&graph_client, NULL, plot_graph, NULL);
    pthread_create(&controller_client, NULL, start_controller, NULL);
    pthread_create(&udp_client, NULL, start_udp_client, (void *)&argv[1]);
    pthread_create(&udp_client_receive, NULL, start_udp_client_receive, NULL);


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

    pthread_cancel(graph_client);
    pthread_cancel(udp_client);
    pthread_cancel(udp_client_receive);
    pthread_cancel(controller_client);

    pthread_join(graph_client, NULL);
    pthread_join(udp_client, NULL);
    pthread_join(udp_client_receive, NULL);
    pthread_join(controller_client, NULL);


    return 0;
}