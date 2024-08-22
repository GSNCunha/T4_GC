#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h> // Biblioteca para manipulação de timeval

#include "timer_utils.h"
#include "buffer_code.h"

#define BUFFSIZE 255

// Função para imprimir uma mensagem de erro e sair do programa
void Die(char *mess) { 
    perror(mess); 
    exit(1); 
}

// Função que inicia um cliente UDP em uma thread
void *start_udp_client(void *args) {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer_send[BUFFSIZE];
    char buffer_receive[BUFFSIZE];
    int message_ver = 0;
    unsigned int echolen, clientlen;
    int received = 0;
    MessageData Ver_mensagem;

    // Configuração do timeout
    struct timeval tv;
    tv.tv_sec = 1.5;  // 1,5 segundos de timeout
    tv.tv_usec = 0;

    // Extrai IP e porta dos argumentos
    char **argv = (char **)args;
    
    // Criação do socket UDP
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Falha ao criar socket");
    }

    // Configura o timeout do socket
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        Die("Falha ao definir timeout do socket");
    }
    
    // Configura a estrutura sockaddr_in do servidor
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = inet_addr(argv[0]);
    echoserver.sin_port = htons(atoi(argv[1]));
    char command[50];

    while (1) {

        if (buffer_get_string(&command_ccb, buffer_send)) {
            int count = 0;
            while(1) {
                echolen = strlen(buffer_send);
                
                // Envia a mensagem para o servidor
                if (sendto(sock, buffer_send, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                    Die("Número de bytes enviados não corresponde");
                }

                // Recebe a resposta do servidor
                clientlen = sizeof(echoclient);
                received = recvfrom(sock, buffer_receive, BUFFSIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
                if (received < 0) {
                    perror("Falha ao receber bytes do servidor ou ocorreu timeout");
                    continue; // Tenta o mesmo comando novamente
                }
                buffer_receive[received] = '\0';  // Termina a string recebida

                // Verifica e compara as mensagens enviadas e recebidas
                if (strncmp(buffer_send, "OpenValve#", 10) == 0) {
                    if(strncmp(buffer_receive, "Open#", 5) == 0) {
                        int num1 = atoi(buffer_send + 10);  // Posição do número na primeira string ("OpenValve#123#...")
                        int num2 = atoi(buffer_receive + 5); // Posição do número na segunda string ("Open#123!...")

                        if(num1 == num2) {
                            break;
                        }
                    }
                    
                } else if(strncmp(buffer_send, "CloseValve#", 11) == 0) {
                    if(strncmp(buffer_receive, "Close#", 6) == 0) {
                        int num1 = atoi(buffer_send + 11);  // Posição do número na primeira string ("CloseValve#123#...")
                        int num2 = atoi(buffer_receive + 6); // Posição do número na segunda string ("Close#123!...")

                        if(num1 == num2) {
                           break;
                        }
                    }

                } else if(strncmp(buffer_send, "GetLevel!", 9) == 0) {

                    if (strncmp(buffer_receive, "Level#", 6) == 0) {
                        // Encontra a posição do caractere '!'
                        char *end = strchr(buffer_receive, '!');
                        if (end != NULL) {
                            *end = '\0'; // Termina antes do '!'
                            char *value = buffer_receive + 6; // Pula "Level#"
                            
                            // Converte o valor extraído para double
                            double level_value = strtod(value, NULL);

                            buffer_put(&nivel_ccb, level_value);
                        }
                        break;
                    }

                } else if(strncmp(buffer_send, "CommTest!", 9) == 0) {

                    if(strncmp(buffer_receive, "Comm#OK!", 8) == 0) {
                        printf("Comm#OK!");
                        printf("\n");
                        break;
                    }
                
                } else if(strncmp(buffer_send, "SetMax#", 7) == 0) {
                    if(strncmp(buffer_receive, "Max#", 4) == 0) {
                        printf("%s", buffer_receive);
                        printf("\n");
                        break;
                    }

                } else if(strncmp(buffer_send, "Start!", 6) == 0) {
                    if(strncmp(buffer_receive, "Start#OK!", 9) == 0) {
                        printf("Start#OK!");
                        printf("\n");
                        buffer_put(&Start_ccb, 1);
                        buffer_put(&Start_ccb_graph, 1);
                        break;
                    }
                }

                printf("Erro ao receber buffer: %s\n", buffer_send);
                printf("\n");
                sleepMs(10);
                count++;

                if (count >= 5) {
                    printf("Erro enviando mensagem");
                    printf("\n");
                    break;
                }
            }
        }

        sleepMs(10); // Dorme por 10 ms
    }

    // Fecha o socket
    close(sock);
    return NULL;
}