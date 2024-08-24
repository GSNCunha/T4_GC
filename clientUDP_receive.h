// udp_client_receive.h

#ifndef UDP_CLIENT_RECEIVE_H
#define UDP_CLIENT_RECEIVE_H

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

#define BUFFSIZE 255
#define MAX_MESSAGES 10
#define KEYWORD_SIZE 50

// Variáveis globais
extern MessageData_client_receive mensagens[MAX_MESSAGES];

// Funções
void loop_de_conferencia(char *keyword, char *value);
void add_message_to_array(MessageData_client_receive *mensagem, MessageData_client_receive array[MAX_MESSAGES]);
void *start_udp_client_receive(void *args);

#endif // UDP_CLIENT_RECEIVE_H
