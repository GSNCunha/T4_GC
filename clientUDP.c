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
    char buffer_send[BUFFSIZE];
    char buffer_receive[BUFFSIZE];
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
            int count = 0;
            while(1)
            {
                echolen = strlen(buffer_send);
                if (sendto(sock, buffer_send, echolen, 0, (struct sockaddr *)&echoserver, sizeof(echoserver)) != echolen) {
                    Die("Mismatch in number of sent bytes");
                }
                //printf("%s \n", buffer_send);
                // Receive the response from the server
                clientlen = sizeof(echoclient);
                received = recvfrom(sock, buffer_receive, BUFFSIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
                if (received < 0) {
                    perror("Failed to receive bytes from server");
                    continue; // Tente o mesmo comando novamente
                }
                buffer_receive[received] = '\0';  // Null-terminate the received data
                //printf("%s \n", buffer_receive);
                if (strncmp(buffer_send, "OpenValve#", 10) == 0) 
                {
                    if(strncmp(buffer_receive, "Open#", 5) == 0)
                    {
                        int num1 = atoi(buffer_send + 10);  // Posição do número na primeira string ("OpenValve#123#...")
                        int num2 = atoi(buffer_receive + 5);   // Posição do número na segunda string ("Open#123!...")

                        // Compara os números
                        if(num1 == num2)
                        {
                            break;
                        }
                    }
                    
                }else if(strncmp(buffer_send, "CloseValve#", 11) == 0) 
                {
                    if(strncmp(buffer_receive, "Close#", 6) == 0)
                    {
                        int num1 = atoi(buffer_send + 11);  // Posição do número na primeira string ("OpenValve#123#...")
                        int num2 = atoi(buffer_receive + 6);   // Posição do número na segunda string ("Open#123!...")

                        // Compara os números
                        if(num1 == num2)
                        {
                           break;
                        }
                    }

                }else if(strncmp(buffer_send, "GetLevel!", 9) == 0) 
                {

                    if (strncmp(buffer_receive, "Level#", 6) == 0) {
                        // Find the position of the '!' character
                        char *end = strchr(buffer_receive, '!');
                        if (end != NULL) {
                            *end = '\0'; // Null-terminate before '!'
                            char *value = buffer_receive + 6; // Skip "Level#"
                            
                            // Convert the extracted value to a double
                            double level_value = strtod(value, NULL);

                            buffer_put(&nivel_ccb, level_value/100);
                        }
                        break;
                    }

                }else if(strncmp(buffer_send, "CommTest!", 9) == 0) 
                {

                    if(strncmp(buffer_receive, "Comm#OK!", 8) == 0)
                    {
                        printf("Comm#OK!");
                        printf("\n");
                        break;
                    }
                
                }else if(strncmp(buffer_send, "SetMax#", 7) == 0) 
                {
                    if(strncmp(buffer_receive, "Max#", 4) == 0)
                    {
                        printf("%s", buffer_receive);
                        printf("\n");
                        break;
                    }

                }else if(strncmp(buffer_send, "Start!", 6) == 0) 
                {
                    if(strncmp(buffer_receive, "Start#OK!", 9) == 0)
                    {
                        printf("Start#OK!");
                        printf("\n");
                        buffer_put(&Start_ccb, 1);
                        buffer_put(&Start_ccb_graph, 1);
                        break;
                    }
                }
                printf("Error receiving buffer: %s\n", buffer_send);
                printf("\n");
                sleepMs(10);
                count ++;
                if (count >= 5)
                {
                    printf("Erro enviando mensagem");
                    printf("\n");
                    break;

                }
            }
        }

        sleepMs(10);//sleep 10ms
    }

    // Close the socket
    close(sock);
    return NULL;
}
