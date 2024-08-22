#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "buffer_code.h"

#define BUFFSIZE 255
#define KEYWORD_SIZE 50
#define RESPONSE_SIZE 10  // Tamanho assumido para a string de resposta

// Variáveis globais para armazenar o último estado
int last_seq = 0;
double last_value = 0;
char last_keyword[11] = {0};

// Função para imprimir erro e encerrar o programa
void Die(char *mess) { 
    perror(mess); 
    exit(1); 
}

// Função para analisar a mensagem recebida
int parse_message(const char *message, MessageData *data) {
    char temp_message[BUFFSIZE];
    strcpy(temp_message, message);

    data->seq = -1;
    data->value = -1;
    data->has_seq = false;
    data->has_value = false;
    memset(data->response, 0, RESPONSE_SIZE);

    char *token = strtok(temp_message, "#!");

    // Verifica o keyword
    if (token != NULL) {
        strncpy(data->keyword, token, KEYWORD_SIZE);
        data->keyword[KEYWORD_SIZE - 1] = '\0';
    } else {
        return -1;  // Erro: Mensagem malformada
    }

    char next_char = message[strlen(token)];
    
    // Verifica o keyword e processa conforme o tipo de comando
    if (strcmp(data->keyword, "OpenValve") == 0 || strcmp(data->keyword, "CloseValve") == 0) {
        if (next_char == '#') {
            // Lê o seq
            token = strtok(NULL, "#");
            if (token != NULL) {
                data->seq = atoi(token);
                data->has_seq = true;
            } else {
                return -1;  // Erro: Esperado número de sequência
            }

            // Lê o valor
            token = strtok(NULL, "!");
            if (token != NULL) {
                data->value = atoi(token);
                data->has_value = true;
            } else {
                return -1;  // Erro: Esperado valor
            }
        } else {
            return -1;  // Erro: Mensagem malformada
        }

        // Verifica se a mensagem é repetida
        if (data->seq == last_seq && last_seq != 0 && data->value == last_value && !strcmp(last_keyword, data->keyword)) {
            return 2;
        }

        // Atualiza o último estado
        strcpy(last_keyword, data->keyword);
        last_seq = data->seq;
        last_value = data->value; 

    } else if (strcmp(data->keyword, "GetLevel") == 0 || strcmp(data->keyword, "CommTest") == 0 || strcmp(data->keyword, "Start") == 0) {
        if (next_char == '!') {
            if (strtok(NULL, "#!") != NULL) {
                return -1;  // Erro: Conteúdo adicional inesperado após o keyword
            }
        } else {
            return -1;  // Erro: Mensagem malformada
        }

    } else if (strcmp(data->keyword, "SetMax") == 0) {
        if (next_char == '#') {
            // Lê o valor
            token = strtok(NULL, "!");
            if (token != NULL) {
                data->value = atoi(token);
                data->has_value = true;
            } else {
                return -1;  // Erro: Esperado valor
            }
        } else {
            return -1;  // Erro: Mensagem malformada
        }

    } else {
        return -1;  // Erro: Keyword não reconhecido
    }

    // Verifica se a mensagem termina com '!'
    if (message[strlen(message) - 1] != '!') {
        return -1;  // Erro: Mensagem malformada
    }

    return 0;  // Sucesso
}

// Função para construir a resposta com base na mensagem recebida
void construct_response(const MessageData *data, char *response) {
    if (strcmp(data->keyword, "OpenValve") == 0) {
        if (data->has_seq) {
            sprintf(response, "Open#%d!", data->seq);
        } else {
            sprintf(response, "Err!");
        }
    } else if (strcmp(data->keyword, "CloseValve") == 0) {
        if (data->has_seq) {
            sprintf(response, "Close#%d!", data->seq);
        } else {
            sprintf(response, "Err!");
        }
    } else if (strcmp(data->keyword, "GetLevel") == 0) {
        double nivel_atual = buffer_get(&nivel_scb);
        sprintf(response, "Level#%.2f!", nivel_atual);
    } else if (strcmp(data->keyword, "CommTest") == 0) {
        sprintf(response, "Comm#OK!");
    } else if (strcmp(data->keyword, "SetMax") == 0) {
        if (data->has_value) {
            int max = (int)data->value;
            sprintf(response, "Max#%d!", max);
        } else {
            sprintf(response, "Err!");
        }
    } else if (strcmp(data->keyword, "Start") == 0) {
        sprintf(response, "Start#OK!");
    } else {
        sprintf(response, "Err!");
    }
}

// Função principal para iniciar o servidor
void *start_server() {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer[BUFFSIZE];
    char response[BUFFSIZE] = {0};
    unsigned int clientlen, serverlen;
    int received = 0;

    // Cria o socket UDP
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Falha ao criar socket");
    }
    
    // Configura a estrutura sockaddr_in do servidor
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);
    echoserver.sin_port = htons(8080);
    
    // Faz o bind do socket
    serverlen = sizeof(echoserver);
    if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
        Die("Falha ao fazer bind no socket do servidor");
    }
    
    // Executa até ser cancelado
    while (1) {
        // Recebe uma mensagem do cliente
        clientlen = sizeof(echoclient);
        if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
                                 (struct sockaddr *) &echoclient,
                                 &clientlen)) < 0) {
            Die("Falha ao receber mensagem");
        }
        buffer[received] = '\0';

        MessageData data;
        int var_aux = parse_message(buffer, &data);
        if (var_aux == 0) {
            double angulo;
            while (angleIn_scb.count > 0) {
                angulo = buffer_get(&angleIn_scb);
            }

            buffer_put_MessageData(&messageData_scb, data);

            // Constrói a resposta com base no comando
            construct_response(&data, response);
        } else if (var_aux == 2) {
            printf("Mensagem repetida");
            construct_response(&data, response);
        } else {
            printf("Falha ao analisar mensagem: %s\n", buffer);
            strcpy(response, "Err!");
        }
        
        // Envia a resposta de volta ao cliente
        if (sendto(sock, response, strlen(response), 0,
                   (struct sockaddr *) &echoclient,
                   sizeof(echoclient)) != strlen(response)) {
            Die("Número de bytes enviados não corresponde");
        }
    }
}