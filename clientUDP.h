#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFSIZE 255

// Function to print an error message and exit the program
void Die(char *mess);

// Function that starts the UDP client in a separate thread
void *start_udp_client(void *args);

#endif // UDP_CLIENT_H
