#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// Definições de tamanho
#define BUFFER_SIZE 10
#define STRING_SIZE 50

// Estrutura para buffer circular de double
typedef struct {
    double buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer;

// Estrutura para buffer circular de string
typedef struct {
    char buffer[BUFFER_SIZE][STRING_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_string;

// Estrutura para armazenar mensagens
typedef struct {
    char keyword[50];
    int seq;
    int value;
    bool has_seq;
    bool has_value;
} Message;


extern circular_buffer nivel_cb;
extern circular_buffer tempo_cb;
extern circular_buffer angleIn_cb;
extern circular_buffer angleOut_cb;
extern circular_buffer_string command_cb;


// Declarações das funções
void buffer_init_string(circular_buffer_string *cb);
void buffer_put_string(circular_buffer_string *cb, const char *item);
void buffer_get_string(circular_buffer_string *cb, Message *item);

void buffer_init(circular_buffer *cb);
void buffer_put(circular_buffer *cb, double item);
double buffer_get(circular_buffer *cb);

#endif // CIRCULAR_BUFFER_H
