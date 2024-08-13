#ifndef SERVERUDP_H
#define SERVERUDP_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255

void Die(char *mess);
void *start_server();

#endif // UDP_SERVER_H
