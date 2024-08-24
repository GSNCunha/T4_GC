#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 255
#define STRING_SIZE 50
#define KEYWORD_SIZE 50
#define RESPONSE_SIZE 10

// Circular buffer for doubles
typedef struct {
    double buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer;

// Circular buffer for strings
typedef struct {
    char buffer[BUFFER_SIZE][STRING_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_string;

// Struct to store message data
typedef struct {
    char keyword[KEYWORD_SIZE];
    int seq;
    double value;
    bool has_seq;
    bool has_value;
    char response[RESPONSE_SIZE]; // Added to handle "OK" responses
} MessageData;

typedef struct {
    char message[KEYWORD_SIZE];
    char keyword[KEYWORD_SIZE];
    char value[KEYWORD_SIZE];
    int num_conferencias;
} MessageData_client_receive;

// Circular buffer for MessageData
typedef struct {
    MessageData buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_MessageData;

// Circular buffer for MessageData_client_receive
typedef struct {
    MessageData_client_receive buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_MessageData_client_receive;

// Global buffer instances
extern circular_buffer nivel_scb;
extern circular_buffer tempo_scb;
extern circular_buffer angleIn_scb;
extern circular_buffer angleOut_scb;
extern circular_buffer Start_scb;
extern circular_buffer_string command_scb;
extern circular_buffer_MessageData messageData_scb;

extern circular_buffer nivel_ccb_graph;
extern circular_buffer nivel_ccb;
extern circular_buffer tempo_ccb;
extern circular_buffer angleIn_ccb;
extern circular_buffer angleOut_ccb;
extern circular_buffer Start_ccb;
extern circular_buffer Start_ccb_graph;
extern circular_buffer delta_ccb;
extern circular_buffer_string command_ccb;
extern circular_buffer_MessageData messageData_ccb;
extern circular_buffer_MessageData_client_receive messageData_client_receive_ccb;

// Function declarations
void buffer_init_string(circular_buffer_string *cb);
void buffer_put_string(circular_buffer_string *cb, const char *item);
int buffer_get_string(circular_buffer_string *cb, char *item);

void buffer_init(circular_buffer *cb);
void buffer_put(circular_buffer *cb, double item);
double buffer_get(circular_buffer *cb);
double buffer_get_last(circular_buffer *cb, double last);

void buffer_init_MessageData(circular_buffer_MessageData *cb);
void buffer_put_MessageData(circular_buffer_MessageData *cb, MessageData item);
MessageData buffer_get_MessageData(circular_buffer_MessageData *cb);

void buffer_init_MessageData_client_receive(circular_buffer_MessageData_client_receive *cb);
void buffer_put_MessageData_client_receive(circular_buffer_MessageData_client_receive *cb, MessageData_client_receive item);
MessageData_client_receive buffer_get_MessageData_client_receive(circular_buffer_MessageData_client_receive *cb);

#endif // BUFFER_H
