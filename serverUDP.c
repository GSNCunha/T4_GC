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

int last_seq = 0;
double  last_value = 0;
char last_keyword[11] = {0};

void Die(char *mess) { perror(mess); exit(1); }

int parse_message(const char *message, MessageData *data) {
    // Initialize variables to hold the parts of the message
    char temp_message[BUFFSIZE];
    strcpy(temp_message, message);

    // Initialize fields to indicate whether we have seq and value parts
    data->seq = -1;
    data->value = -1;
    data->has_seq = false;
    data->has_value = false;
    memset(data->response, 0, RESPONSE_SIZE); // Clear the response field

    char *token = strtok(temp_message, "#!");

    // Check the keyword
    if (token != NULL) {
        strncpy(data->keyword, token, KEYWORD_SIZE);
        data->keyword[KEYWORD_SIZE - 1] = '\0';  // Ensure null-termination
    } else {
        return -1;  // Error: Malformed message
    }

    // Determine what follows the keyword
    char next_char = message[strlen(token)];
    
    // Switch-case like structure based on the keyword
    if (strcmp(data->keyword, "OpenValve") == 0 || strcmp(data->keyword, "CloseValve") == 0) {
        if (next_char == '#') {
            // Read sequence
            token = strtok(NULL, "#");
            if (token != NULL) {
                data->seq = atoi(token);
                data->has_seq = true;

            } else {
                return -1;  // Error: Expected sequence number
            }

            // Read value
            token = strtok(NULL, "!");
            if (token != NULL) {
                data->value = atoi(token);
                data->has_value = true;
            } else {
                return -1;  // Error: Expected value
            }
        } else {
            return -1;  // Error: Malformed message
        }
        if(data->seq == last_seq && last_seq !=0 && data->value == last_value && !strcmp(last_keyword, data->keyword))
            return 2;

        strcpy(last_keyword, data->keyword);
        last_seq = data->seq;
        last_value = data->value; 

    } else if (strcmp(data->keyword, "GetLevel") == 0 || strcmp(data->keyword, "CommTest") == 0 || strcmp(data->keyword, "Start") == 0) {
        if (next_char == '!') {
            // These commands should only have the keyword and a '!'
            if (strtok(NULL, "#!") != NULL) {
                return -1;  // Error: Unexpected additional content after keyword
            }
        } else {
            return -1;  // Error: Malformed message
        }

    } else if (strcmp(data->keyword, "SetMax") == 0) {
        if (next_char == '#') {
            // Read value
            token = strtok(NULL, "!");
            if (token != NULL) {
                data->value = atoi(token);
                data->has_value = true;
            } else {
                return -1;  // Error: Expected value
            }
        } else {
            return -1;  // Error: Malformed message
        }

    } else {
        return -1;  // Error: Unrecognized keyword
    }

    // Ensure the final token ends with '!'
    if (message[strlen(message) - 1] != '!') {
        return -1;  // Error: Malformed message
    }

    return 0;  // Success
}


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

void *start_server() {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer[BUFFSIZE];
    char response[BUFFSIZE] = {0};
    unsigned int clientlen, serverlen;
    int received = 0;

    /* Create the UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Failed to create socket");
    }
    
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
    echoserver.sin_port = htons(8080);                /* Hardcoded server port */
    
    /* Bind the socket */
    serverlen = sizeof(echoserver);
    if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
        Die("Failed to bind server socket");
    }
    
    /* Run until cancelled */

    while (1) {

        while (nivel_scb.count > 0 || tempo_scb.count > 0 || angleIn_scb.count > 0 || angleOut_scb.count > 0) {
            double t = buffer_get(&tempo_scb) / 1000;
            double lvl = 100*buffer_get(&nivel_scb);
            double var_aux = buffer_get(&angleIn_scb);
            double angleOut = buffer_get(&angleOut_scb);
        }
        
        /* Receive a message from the client */
        clientlen = sizeof(echoclient);
        if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
                                 (struct sockaddr *) &echoclient,
                                 &clientlen)) < 0) {
            Die("Failed to receive message");
        }
        buffer[received] = '\0';

        //printf("%s \n", buffer);

        MessageData data;
        int var_aux = parse_message(buffer, &data);
        if (var_aux == 0) {
            double angulo;
            while (angleIn_scb.count > 0)
            {
                angulo = buffer_get(&angleIn_scb);
            }
            
            //printf("Keyword: %s\n", data.keyword);
            //printf("Seq: %d\n", data.seq);
            //printf("Value: %d\n", data.value);

            buffer_put_MessageData(&messageData_scb, data);

            // Construct the response based on the command
            construct_response(&data, response);
        } else if(var_aux == 2){
           printf("Repeated message");
           construct_response(&data, response);
        }else
        {
            printf("Failed to parse message: %s\n", buffer);
            strcpy(response, "Err!");
        }

      //  fprintf(stderr, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
        //printf("%s \n", response);
        
        /* Send the response back to the client */
        if (sendto(sock, response, strlen(response), 0,
                   (struct sockaddr *) &echoclient,
                   sizeof(echoclient)) != strlen(response)) {
            Die("Mismatch in number of sent bytes");
        }
    }
}
