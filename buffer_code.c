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

typedef struct {
    double buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer;

typedef struct {
    char buffer[BUFFER_SIZE][STRING_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_string;

typedef struct {
    char keyword[KEYWORD_SIZE];
    int seq;
    int value;
    bool has_seq;
    bool has_value;
    char response[RESPONSE_SIZE]; // Added to handle "OK" responses
} MessageData;

typedef struct {
    MessageData buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_MessageData;

circular_buffer nivel_scb;
circular_buffer tempo_scb;
circular_buffer angleIn_scb;
circular_buffer angleOut_scb;
circular_buffer Start_scb;
circular_buffer_string command_scb;
circular_buffer_string message_scb;
circular_buffer_MessageData messageData_scb; // Buffer for MessageData

circular_buffer nivel_ccb;
circular_buffer tempo_ccb;
circular_buffer angleIn_ccb;
circular_buffer angleOut_ccb;
circular_buffer Start_ccb;
circular_buffer_string command_ccb;
circular_buffer_string message_ccb;
circular_buffer delta_ccb;
circular_buffer_MessageData messageData_ccb; // Buffer for MessageData
circular_buffer nivel_controller;

void buffer_init_string(circular_buffer_string *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void buffer_put_string(circular_buffer_string *cb, const char *item) {
    pthread_mutex_lock(&cb->lock);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->lock);
    }
    strncpy(cb->buffer[cb->tail], item, STRING_SIZE - 1);
    cb->buffer[cb->tail][STRING_SIZE - 1] = '\0';  // Ensure null-termination
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->lock);
}

int buffer_get_string(circular_buffer_string *cb, char *item) {
    pthread_mutex_lock(&cb->lock);
    if (cb->count == 0) {
        // Buffer is empty, return a special value
        pthread_mutex_unlock(&cb->lock);
        return 0;  // Or any other special value that indicates "nothing to read"
    }
    strncpy(item, cb->buffer[cb->head], STRING_SIZE);
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    return 1;
}

void buffer_init(circular_buffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void buffer_put(circular_buffer *cb, double item) {
    pthread_mutex_lock(&cb->lock);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->lock);
    }
    cb->buffer[cb->tail] = item;
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->lock);
}

double buffer_get(circular_buffer *cb) {
    pthread_mutex_lock(&cb->lock);
    if (cb->count == 0) {
        // Buffer is empty, return a special value
        pthread_mutex_unlock(&cb->lock);
        return 0;  // Or any other special value that indicates "nothing to read"
    }
    
    double item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    
    return item;
}


void buffer_init_MessageData(circular_buffer_MessageData *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void buffer_put_MessageData(circular_buffer_MessageData *cb, MessageData item) {
    pthread_mutex_lock(&cb->lock);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->lock);
    }
    cb->buffer[cb->tail] = item;
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->lock);
}

MessageData buffer_get_MessageData(circular_buffer_MessageData *cb) {
    pthread_mutex_lock(&cb->lock);

    // Check if the buffer is empty
    if (cb->count == 0) {
        pthread_mutex_unlock(&cb->lock);
        // Handle the case when the buffer is empty
        // This could be returning a special value, logging an error, etc.
        // Here, we'll assume you return a default-constructed MessageData
        MessageData empty_message = {0};  // or another way to create a "null" message
        return empty_message;
    }

    // Proceed with reading the message
    MessageData item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);

    return item;
}
