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

void Die(char *mess) { 
    perror(mess); 
    exit(1); 
}

void *start_udp_client(void *args) {
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer[BUFFSIZE];
    unsigned int echolen, clientlen;
    int received = 0;
    
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
        // Prompt the user to enter a message
        /*printf("Enter message (or type 'exit' to quit): ");
        fgets(buffer, BUFFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove the newline character

        // Exit if the user types 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
        }*/
        //trocar oq tem acima p ficar escutando o controlador
        if(buffer_get(&delta_ccb) || buffer_get_string(&command_ccb,command)){

            echolen = strlen(buffer);
            if (sendto(sock, buffer, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                Die("Mismatch in number of sent bytes");
            }
            
            // Receive the response from the server
            clientlen = sizeof(echoclient);
            if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, (struct sockaddr *)&echoclient, &clientlen)) < 0) {
                Die("Failed to receive bytes from server");
            }
            
            buffer[received] = '\0';  // Null-terminate the received data
            printf("Received: %s\n", buffer);

            // Check if the message is in the form "Level#<value>!"
            if (strncmp(buffer, "Level#", 6) == 0) {
                // Find the position of the '!' character
                char *end = strchr(buffer, '!');
                if (end != NULL) {
                    *end = '\0'; // Null-terminate before '!'
                    char *value = buffer + 6; // Skip "Level#"
                    
                    // Convert the extracted value to a double
                    double level_value = strtod(value, NULL);

                    buffer_put(&nivel_ccb, level_value);
                    printf("Extracted value as string: %s\n", value);
                    printf("Extracted value as double: %f\n", level_value);
                }
            }else if (strcmp(buffer, "Start!") == 0) {
                buffer_put(&Start_ccb, 1);

            }
            buffer_put(&nivel_ccb, buffer_get(&nivel_ccb));

            /*double lvl = 0;
            double t = 0;
            double start_flag = 0;
            buffer_put(&nivel_ccb,lvl);
            buffer_put(&tempo_ccb, t);
            buffer_put(&Start_ccb, start_flag);*/
        }

        sleepMs(10); //sleep 10ms
    }

    // Close the socket
    close(sock);
    return NULL;
}