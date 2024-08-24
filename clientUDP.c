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
#include "clientUDP_receive.h"

#define BUFFSIZE 255

void Die(char *mess) { 
    perror(mess); 
    exit(1); 
}

void *start_udp_client(void *args) {

    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer_send[BUFFSIZE];
    int message_ver = 0;
    unsigned int echolen, clientlen;
    int received = 0;
    MessageData Ver_mensagem;

    // Extract IP and port from arguments
    char **argv = (char **)args;
    
    // Create the UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Failed to create socket");
    }
    
    // Construct the server sockaddr_in structure
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = inet_addr(argv[0]);
    echoserver.sin_port = htons(atoi(argv[1]));
    char command[50];
    while (1) {

        if (buffer_get_string(&command_ccb, buffer_send))
        {
            echolen = strlen(buffer_send);
            if (sendto(sock, buffer_send, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                Die("Mismatch in number of sent bytes");
            }
            //printf("%s \n", buffer_send);

            if (strncmp(buffer_send, "OpenValve#", 9) == 0) {

                char seq_str[10]; // Defina o tamanho de acordo com a expectativa do número
                sscanf(buffer_send, "OpenValve#%[^#]", seq_str);

                MessageData_client_receive message;
                strcpy(message.keyword, "OpenValve#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, seq_str);
                message.num_conferencias = 0;

                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);

            } else if (strncmp(buffer_send, "CloseValve#", 10) == 0) {

                char seq_str[10]; // Defina o tamanho de acordo com a expectativa do número
                sscanf(buffer_send, "CloseValve#%[^#]", seq_str);

                MessageData_client_receive message;
                strcpy(message.keyword, "CloseValve#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, seq_str);
                message.num_conferencias = 0;

                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);

            } else if (strncmp(buffer_send, "GetLevel!", 9) == 0) {

                MessageData_client_receive message;
                strcpy(message.keyword, "GetLevel!");
                strcpy(message.message, buffer_send);
                message.num_conferencias = 0;
                fflush(stdout);
                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);

            } else if (strncmp(buffer_send, "CommTest!", 9) == 0) {

                MessageData_client_receive message;
                strcpy(message.keyword, "CommTest!");
                strcpy(message.message, buffer_send);
                message.num_conferencias = 0;

                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);

            } else if (strncmp(buffer_send, "SetMax#", 7) == 0) {

                char value_str[10]; // Defina o tamanho de acordo com a expectativa do número
                sscanf(buffer_send, "SetMax#%[^!]", value_str);

                MessageData_client_receive message;
                strcpy(message.keyword, "SetMax#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, value_str);
                message.num_conferencias = 0;

                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);

            } else if (strncmp(buffer_send, "Start!", 6) == 0) {

                MessageData_client_receive message;
                strcpy(message.keyword, "Start!");
                strcpy(message.message, buffer_send);
                message.num_conferencias = 0;

                buffer_put_MessageData_client_receive(&messageData_client_receive_ccb, message);
            }

        }

        sleepMs(10);//sleep 10ms
    }

    // Close the socket
    close(sock);
    return NULL;
}
