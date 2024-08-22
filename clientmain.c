#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "buffer_code.h"
#include "graph_client.h"
#include "clientUDP.h"
#include "controller.h"

#define BUFFSIZE 255

int main(int argc, char *argv[]) {
    pthread_t graph_client, udp_client, controller_client; 

    // Verifica se os argumentos foram passados corretamente
    if (argc != 3) {
        fprintf(stderr, "USO: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    // Inicializa os buffers usados na comunicação entre os componentes
    buffer_init(&nivel_ccb_graph);
    buffer_init(&nivel_ccb);          // Buffer para comunicação entre clientUDP e graph_client
    buffer_init(&tempo_ccb);          // Buffer para comunicação entre clientUDP e graph_client
    buffer_init(&angleIn_ccb);        // Buffer para comunicação entre controle e graph_client
    buffer_init(&Start_ccb);          // Buffer para comunicação entre clientUDP e graph_client
    buffer_init(&Start_ccb_graph);    // Buffer para comunicação entre clientUDP e graph_client
    buffer_init(&delta_ccb);          // Buffer para comunicação entre clientUDP e graph_client
    buffer_init_string(&command_ccb); // Buffer para enviar comandos do controle para o clientUDP
    //buffer_init_MessageData(&command_ccb); // Linha comentada, não necessária

    // Criação das threads para o cliente gráfico, controle e cliente UDP
    pthread_create(&graph_client, NULL, plot_graph, NULL);
    pthread_create(&controller_client, NULL, start_controller, NULL);
    pthread_create(&udp_client, NULL, start_udp_client, (void *)&argv[1]);

    while(1) {
        // Solicita ao usuário que digite uma mensagem
        char buffer[BUFFSIZE];
        fgets(buffer, BUFFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove o caractere de nova linha

        // Sai do loop se o usuário digitar 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Coloca a mensagem digitada no buffer de comandos
        buffer_put_string(&command_ccb, buffer);
    }

    // Espera as threads finalizarem
    pthread_join(graph_client, NULL);
    pthread_join(udp_client, NULL);
    pthread_join(controller_client, NULL);

    return 0;
}