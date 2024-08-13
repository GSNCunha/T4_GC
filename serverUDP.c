#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>

#define BUFFSIZE 255
#define KEYWORD_SIZE 50

typedef struct {
    char keyword[KEYWORD_SIZE];
    int seq;
    int value;
} MessageData;

typedef struct {
    char keyword[KEYWORD_SIZE];
    int seq;
    int value;
    bool has_seq;
    bool has_value;
} Message;


int parse_message(const char *message, MessageData *data) {
    // Initialize variables to hold the parts of the message
    char temp_message[100];
    strcpy(temp_message, message);

    char *token = strtok(temp_message, "#");
    if (token != NULL) {
        strncpy(data->keyword, token, KEYWORD_SIZE);
        data->keyword[KEYWORD_SIZE - 1] = '\0';  // Ensure null-termination
    } else {
        return -1;  // Error: Malformed message
    }

    token = strtok(NULL, "#");
    if (token != NULL) {
        data->seq = atoi(token);
    } else {
        return -1;  // Error: Malformed message
    }

    token = strtok(NULL, "!");
    if (token != NULL) {
        data->value = atoi(token);
    } else {
        return -1;  // Error: Malformed message
    }

    return 0;  // Success
}





void Die(char *mess) { perror(mess); exit(1); }

void *start_server() {
  int sock;
  struct sockaddr_in echoserver;
  struct sockaddr_in echoclient;
  char buffer[BUFFSIZE];
  unsigned int echolen, clientlen, serverlen;
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
    /* Receive a message from the client */
    clientlen = sizeof(echoclient);
    if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
			     (struct sockaddr *) &echoclient,
			     &clientlen)) < 0) {
      Die("Failed to receive message");
    }
  buffer[received] = '\0';

  MessageData data;
  if (parse_message(buffer, &data) == 0) {
      printf("Keyword: %s\n", data.keyword);
      printf("Seq: %d\n", data.seq);
      printf("Value: %d\n", data.value);
  } else {
    printf("Failed to parse message: %s\n", buffer);
  }

    fprintf(stderr,
	    "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
    /* Send the message back to client */
    if (sendto(sock, buffer, received, 0,
	       (struct sockaddr *) &echoclient,
	       sizeof(echoclient)) != received) {
      Die("Mismatch in number of echo'd bytes");
    }
  }
}
