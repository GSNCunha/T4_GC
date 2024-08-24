#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#include "timer_utils.h"
#include "buffer_code.h"
#include "clientUDP.h"

#define BUFFSIZE 255
#define MAX_MESSAGES 50
#define KEYWORD_SIZE 50

MessageData_client_receive mensagens[MAX_MESSAGES];

void loop_de_conferencia(char *keyword, char *value) {
    for(int x = 0; x < MAX_MESSAGES; x++) {
        if(strncmp(mensagens[x].keyword, keyword, strlen(keyword)) == 0) {
            if(value != NULL) {
                if(strncmp(mensagens[x].value, value, strlen(value)) == 0) {
                    // Remover a mensagem do array (limpar o slot)
                    memset(&mensagens[x], 0, sizeof(MessageData_client_receive));
                    printf("mensagem conferida \n");
                    break;
                }
            } else {
                // Caso em que value é NULL, apenas remove a mensagem com o keyword correspondente
                memset(&mensagens[x], 0, sizeof(MessageData_client_receive));
                printf("mensagem conferida \n");
                break;
            }
        }
        mensagens[x].num_conferencias += 1;
    }
}

void add_message_to_array(MessageData_client_receive *mensagem, MessageData_client_receive array[MAX_MESSAGES]) {
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (array[i].keyword[0] == '\0') {  // Verifica se a posição está vazia
            array[i] = *mensagem;
            break;
        }
    }
}

void *start_udp_client_receive() {

    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer_receive[BUFFSIZE];
    char command[50];
    unsigned int clientlen;
    int received = 0;
    
    // Create the UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Failed to create socket");
    }
    
    // Construct the server sockaddr_in structure
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = inet_addr("127.0.0.1");  // O endereço IP deve ser passado como uma string
    echoserver.sin_port = htons(8080);

    // Bind the socket to the server address
    if (bind(sock, (struct sockaddr *)&echoserver, sizeof(echoserver)) < 0) {
        perror("Failed to bind socket");
        exit(1);
    }

    while (1) {

        MessageData_client_receive message = buffer_get_MessageData_client_receive(&messageData_client_receive_ccb);

        if(message.keyword[0] != '\0') {
            add_message_to_array(&message, mensagens);
        }

        clientlen = sizeof(echoclient);
        received = recvfrom(sock, buffer_receive, BUFFSIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
        if (received < 0) {
            perror("Failed to receive bytes from server");
            continue; // Tente o mesmo comando novamente
        }
        
        printf("%s", buffer_receive);
        printf("aqui");
        fflush(stdout);
        
        if (strncmp(buffer_receive, "Open#", 5) == 0) {
            char numero_extraido[10];
            sscanf(buffer_receive, "Open#%[^!]!", numero_extraido);
            loop_de_conferencia("OpenValve#", numero_extraido);

        } else if (strncmp(buffer_receive, "Close#", 6) == 0) {
            char numero_extraido[10];
            sscanf(buffer_receive, "Close#%[^!]!", numero_extraido);
            loop_de_conferencia("CloseValve#", numero_extraido);

        } else if (strncmp(buffer_receive, "Level#", 6) == 0) {
            loop_de_conferencia("GetLevel!", NULL);

        } else if (strncmp(buffer_receive, "Comm#OK!", 8) == 0) {
            loop_de_conferencia("CommTest!", NULL);

        } else if (strncmp(buffer_receive, "SetMax#", 7) == 0) {
            char numero_extraido[10];
            sscanf(buffer_receive, "SetMax#%[^!]!", numero_extraido);
            loop_de_conferencia("Max#", numero_extraido);

        } else if (strncmp(buffer_receive, "Start#OK!", 9) == 0) {
            loop_de_conferencia("Start!", NULL);
        }

        // Remover mensagens que falharam em 5 conferências
        for(int x = 0; x < MAX_MESSAGES; x++) {
            if(mensagens[x].num_conferencias >= 8) {
                buffer_put_string(&command_ccb, mensagens[x].message);
                memset(&mensagens[x], 0, sizeof(MessageData_client_receive));  // Limpa o slot
                printf("tentar mandar msg novamente \n");
            }
        }
    }

    // Close the socket
    close(sock);
    return NULL;
}
