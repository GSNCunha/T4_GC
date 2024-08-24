#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

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
#define RESPONSE_SIZE 10  // Assuming this size for the response string

// Function prototypes
void Die(char *mess);

int parse_message(const char *message, MessageData *data);

void construct_response(const MessageData *data, char *response);

void *start_server();

#endif // SERVER_UTILS_H
