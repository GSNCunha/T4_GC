#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>  // Inclua esta linha para usar errno, EAGAIN e EWOULDBLOCK

#include "timer_utils.h"
#include "buffer_code.h"

#define BUFFSIZE 255
#define MAX_MESSAGES 250
#define KEYWORD_SIZE 50
#define CLIENT_UDP_PERIOD 10
#define RETRY_PERIOD 50 //ms

void Die(char *mess) { 
    perror(mess); 
    exit(1); 
}

void reset_message(MessageData_client_receive *msg) {
    memset(msg, 0, sizeof(MessageData_client_receive));
}

MessageData_client_receive mensagens[MAX_MESSAGES];

void loop_de_conferencia(char *keyword, char *value) {
    int x;
    for (x = 0; x < MAX_MESSAGES; x++) {
        if (strncmp(mensagens[x].keyword, keyword, strlen(keyword)) == 0) {
            if (value != NULL) {
                if (atoi(mensagens[x].value) == atoi(value)) {
                    memset(&mensagens[x], '\0', sizeof(MessageData_client_receive));
                    //printf("mensagem conferida %s \n", keyword);
                    break;
                }
            } else {
                memset(&mensagens[x], '\0', sizeof(MessageData_client_receive));
                //printf("mensagem conferida %s \n", keyword);
                break;
            }
        }
        mensagens[x].num_conferencias += 1;
    }
}

void add_message_to_array(MessageData_client_receive *mensagem, MessageData_client_receive array[MAX_MESSAGES]) {
    int i;
    for (i = 0; i < MAX_MESSAGES; i++) {
        if (array[i].keyword[0] == '\0') {  // Verifica se a posição está vazia
            array[i] = *mensagem;
            break;
        }
    }
}

void *start_udp_client(void *args) {

    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer_send[BUFFSIZE];
    char buffer_receive[BUFFSIZE];
    int received = 0;
    unsigned int echolen, clientlen;
    struct timespec t_spec;
    struct timespec t_spec_retry;


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
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec_retry);
    while (1) {
        while ((get_elapsed_time_ms(t_spec)) < CLIENT_UDP_PERIOD); //verifica se ja se passou o periodo
        clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec); //reseta o tempo inicial


        if (buffer_get_string(&command_ccb, buffer_send)) {
            echolen = strlen(buffer_send);
            if (sendto(sock, buffer_send, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                Die("Mismatch in number of sent bytes");
            }

            if (strncmp(buffer_send, "OpenValve#", 9) == 0) {
                printf("%s \n", buffer_send);
                char seq_str[10];
                sscanf(buffer_send, "OpenValve#%[^#]", seq_str);
                MessageData_client_receive message;
                strcpy(message.keyword, "OpenValve#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, seq_str);
                message.num_conferencias = 0;

                add_message_to_array(&message, mensagens);

            } else if (strncmp(buffer_send, "CloseValve#", 10) == 0) {
                printf("%s \n", buffer_send);
                char seq_str[10];
                sscanf(buffer_send, "CloseValve#%[^#]", seq_str);

                MessageData_client_receive message;
                strcpy(message.keyword, "CloseValve#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, seq_str);
                message.num_conferencias = 0;

                add_message_to_array(&message, mensagens);

            } else if (strncmp(buffer_send, "GetLevel!", 9) == 0) {

            } else if (strncmp(buffer_send, "CommTest!", 9) == 0) {
                MessageData_client_receive message;
                strcpy(message.keyword, "CommTest!");
                strcpy(message.message, buffer_send);
                message.num_conferencias = 0;

                add_message_to_array(&message, mensagens);

            } else if (strncmp(buffer_send, "SetMax#", 7) == 0) {
                char value_str[10];
                sscanf(buffer_send, "SetMax#%[^!]", value_str);
                MessageData_client_receive message;
                strcpy(message.keyword, "SetMax#");
                strcpy(message.message, buffer_send);
                strcpy(message.value, value_str);
                message.num_conferencias = 0;

                add_message_to_array(&message, mensagens);

            } else if (strncmp(buffer_send, "Start!", 6) == 0) {
                MessageData_client_receive message;
                strcpy(message.keyword, "Start!");
                strcpy(message.message, buffer_send);
                message.num_conferencias = 0;

                add_message_to_array(&message, mensagens);
            }
        }

        // Receber a resposta do servidor
        memset(buffer_receive, 0, sizeof(buffer_receive));
        clientlen = sizeof(echoclient);
        received = recvfrom(sock, buffer_receive, BUFFSIZE, MSG_DONTWAIT, (struct sockaddr *)&echoclient, &clientlen);
        
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Sem dados disponíveis, continuar imediatamente
                continue;
            } else {
                perror("Failed to receive bytes from server");
                continue; // Tente o mesmo comando novamente
            }
        }
        if (received > 0) {
            //printf("%s \n", buffer_receive);

            // Processar a mensagem recebida
            if (strncmp(buffer_receive, "Open#", 5) == 0) {
                char numero_extraido[10];
                sscanf(buffer_receive, "Open#%[^!]!", numero_extraido);
                loop_de_conferencia("OpenValve#", numero_extraido);

            } else if (strncmp(buffer_receive, "Close#", 6) == 0) {
                char numero_extraido[10];
                sscanf(buffer_receive, "Close#%[^!]!", numero_extraido);
                loop_de_conferencia("CloseValve#", numero_extraido);

            } else if (strncmp(buffer_receive, "Level#", 6) == 0) {
                //loop_de_conferencia("GetLevel!", NULL);
                char numero_extraido[10];
                sscanf(buffer_receive, "Level#%[^!]!", numero_extraido);
                int nivel = strtod(numero_extraido, NULL);
                buffer_put(&nivel_ccb, nivel);
                //printf("%s \n", buffer_receive);
                //fflush(stdout);

            } else if (strncmp(buffer_receive, "Comm#OK!", 8) == 0) {
                loop_de_conferencia("CommTest!", NULL);

            } else if (strncmp(buffer_receive, "Max#", 4) == 0) {
                char numero_extraido[10];
                sscanf(buffer_receive, "Max#%[^!]!", numero_extraido);
                loop_de_conferencia("SetMax#", numero_extraido);

            } else if (strncmp(buffer_receive, "Start#OK!", 9) == 0) {
                loop_de_conferencia("Start!", NULL);
            }

            // Remover mensagens que falharam em 20 conferências

            if((get_elapsed_time_ms(t_spec_retry)) >= RETRY_PERIOD){ //verifica se ja se passou o periodo
            clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec_retry); //reseta o tempo inicial

            int x;
            for (x = 0; x < MAX_MESSAGES; x++) {
                    if (mensagens[x].num_conferencias >= 0) {

                        echolen = strlen(mensagens[x].message);

                        // Verifica se a mensagem não está vazia antes de tentar enviar
                        if (echolen == 0) {
                           // printf("Mensagem vazia, não será enviada.\n");
                            reset_message(&mensagens[x]);
                            continue;
                        }

                        // Tenta enviar a mensagem
                        if (sendto(sock, mensagens[x].message, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                            perror("Falha ao enviar mensagem");
                           
                            continue; // Continua para o próximo loop para evitar ficar preso nesse erro
                        }
                        //fflush(stdout);
                        printf("tentar mandar msg novamente %s %s \n", mensagens[x].keyword, mensagens[x].value);

                        // Reseta a estrutura de mensagem para evitar repetir o envio
                        //reset_message(&mensagens[x]);
                    }
                    
                }
            }



        }

        sleepMs(10); // sleep 10ms
    }

    // Close the socket
    close(sock);
    return NULL;
}
