#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>


#define BUFFER_SIZE 10
#define STRING_SIZE 50

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
    char keyword[50];
    int seq;
    int value;
    bool has_seq;
    bool has_value;
} Message;


circular_buffer nivel_cb;
circular_buffer tempo_cb;
circular_buffer angleIn_cb;
circular_buffer angleOut_cb;
circular_buffer_string command_cb;


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

void buffer_get_string(circular_buffer_string *cb, Message *item) {
    char temp[STRING_SIZE];
    pthread_mutex_lock(&cb->lock);
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->lock);
    }
    strncpy(temp, cb->buffer[cb->head], STRING_SIZE);
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);

    // Parsing the string
    char *token;
    token = strtok(temp, "#!");
    if (token != NULL) {
        strncpy(item->keyword, token, sizeof(item->keyword) - 1);
        item->keyword[sizeof(item->keyword) - 1] = '\0';  // Ensure null-termination
    }

    token = strtok(NULL, "#!");
    if (token != NULL) {
        item->seq = atoi(token);
        item->has_seq = true;
    } else {
        item->has_seq = false;
    }

    token = strtok(NULL, "#!");
    if (token != NULL) {
        item->value = atoi(token);
        item->has_value = true;
    } else {
        item->has_value = false;
    }
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
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->lock);
        return -1;
    }
    double item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    return item;
}