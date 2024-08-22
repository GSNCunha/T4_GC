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

// Estrutura de buffer circular para valores double
typedef struct {
    double buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer;

// Estrutura de buffer circular para strings
typedef struct {
    char buffer[BUFFER_SIZE][STRING_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_string;

// Estrutura de dados para armazenar mensagens com palavra-chave, sequência e valor
typedef struct {
    char keyword[KEYWORD_SIZE];
    int seq;
    double value;
    bool has_seq;
    bool has_value;
    char response[RESPONSE_SIZE]; // Adicionado para lidar com respostas "OK"
} MessageData;

// Estrutura de buffer circular para MessageData
typedef struct {
    MessageData buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer_MessageData;

// Declaração dos buffers circulares para diferentes tipos de dados
circular_buffer nivel_scb;
circular_buffer tempo_scb;
circular_buffer angleIn_scb;
circular_buffer angleOut_scb;
circular_buffer Start_scb;
circular_buffer_string command_scb;
circular_buffer_string message_scb;
circular_buffer_MessageData messageData_scb; // Buffer para MessageData

circular_buffer nivel_ccb;
circular_buffer nivel_ccb_graph;
circular_buffer tempo_ccb;
circular_buffer angleIn_ccb;
circular_buffer angleOut_ccb;
circular_buffer Start_ccb;
circular_buffer Start_ccb_graph;
circular_buffer_string command_ccb;
circular_buffer_string message_ccb;
circular_buffer delta_ccb;
circular_buffer_MessageData messageData_ccb; // Buffer para MessageData

// Função para inicializar um buffer circular de strings
void buffer_init_string(circular_buffer_string *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

// Função para inserir uma string no buffer circular
void buffer_put_string(circular_buffer_string *cb, const char *item) {
    pthread_mutex_lock(&cb->lock);
    while (cb->count == BUFFER_SIZE) {
        pthread_cond_wait(&cb->not_full, &cb->lock);
    }
    strncpy(cb->buffer[cb->tail], item, STRING_SIZE - 1);
    cb->buffer[cb->tail][STRING_SIZE - 1] = '\0';  // Garantir terminação nula
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->lock);
}

// Função para recuperar uma string do buffer circular
int buffer_get_string(circular_buffer_string *cb, char *item) {
    pthread_mutex_lock(&cb->lock);
    if (cb->count == 0) {
        // Buffer está vazio, retornar um valor especial
        pthread_mutex_unlock(&cb->lock);
        return 0;  // Ou outro valor especial que indique "nada para ler"
    }
    strncpy(item, cb->buffer[cb->head], STRING_SIZE);
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    return 1;
}

// Função para inicializar um buffer circular de double
void buffer_init(circular_buffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

// Função para inserir um valor double no buffer circular
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

// Função para recuperar um valor double do buffer circular
double buffer_get(circular_buffer *cb) {
    pthread_mutex_lock(&cb->lock);
    if (cb->count == 0) {
        // Buffer está vazio, retornar um valor especial
        pthread_mutex_unlock(&cb->lock);
        return 0;  // Ou outro valor especial que indique "nada para ler"
    }
    
    double item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    
    return item;
}

// Função para recuperar o último valor válido do buffer circular
double buffer_get_last(circular_buffer *cb, double last) {
    pthread_mutex_lock(&cb->lock);
    if (cb->count == 0) {
        // Buffer está vazio, retornar o valor passado como parâmetro
        pthread_mutex_unlock(&cb->lock);
        return last;
    }
    
    // Recupera o último item inserido
    int last_index = (cb->tail - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    double item = cb->buffer[last_index];
    
    pthread_mutex_unlock(&cb->lock);
    
    return item;
}

// Função para inicializar um buffer circular de MessageData
void buffer_init_MessageData(circular_buffer_MessageData *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

// Função para inserir uma MessageData no buffer circular
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

// Função para recuperar uma MessageData do buffer circular
MessageData buffer_get_MessageData(circular_buffer_MessageData *cb) {
    pthread_mutex_lock(&cb->lock);

    // Verifica se o buffer está vazio
    if (cb->count == 0) {
        pthread_mutex_unlock(&cb->lock);
        // Retornar uma MessageData "vazia" como valor especial
        MessageData empty_message = {0};  // Ou outra forma de criar uma mensagem "nula"
        return empty_message;
    }

    // Recupera a MessageData
    MessageData item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);

    return item;
}
