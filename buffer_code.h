#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 10
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
    int value;
    bool has_seq;
    bool has_value;
    char response[RESPONSE_SIZE]; // Added to handle "OK" responses
} MessageData;

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

// Global buffer instances
extern circular_buffer nivel_cb;
extern circular_buffer tempo_cb;
extern circular_buffer angleIn_cb;
extern circular_buffer angleOut_cb;
extern circular_buffer Start_cb;
extern circular_buffer_string command_cb;
extern circular_buffer_MessageData messageData_cb;

// Function declarations
void buffer_init_string(circular_buffer_string *cb);
void buffer_put_string(circular_buffer_string *cb, const char *item);
void buffer_get_string(circular_buffer_string *cb, char *item);

void buffer_init(circular_buffer *cb);
void buffer_put(circular_buffer *cb, double item);
double buffer_get(circular_buffer *cb);

void buffer_init_MessageData(circular_buffer_MessageData *cb);
void buffer_put_MessageData(circular_buffer_MessageData *cb, MessageData item);
MessageData buffer_get_MessageData(circular_buffer_MessageData *cb);

#endif // BUFFER_H
