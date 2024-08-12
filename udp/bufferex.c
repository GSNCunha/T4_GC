#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} circular_buffer;

circular_buffer cbuf;

void buffer_init(circular_buffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    pthread_cond_init(&cb->not_full, NULL);
}

void buffer_put(circular_buffer *cb, int item) {
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

int buffer_get(circular_buffer *cb) {
    pthread_mutex_lock(&cb->lock);
    while (cb->count == 0) {
        pthread_cond_wait(&cb->not_empty, &cb->lock);
    }
    int item = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    return item;
}

void* producer(void* arg) {
    int i;
    for (i = 0; i < 20; i++) {
        printf("Produzindo item %d\n", i);
        buffer_put(&cbuf, i);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    int i;
    for (i = 0; i < 20; i++) {
        int item = buffer_get(&cbuf);
        printf("Consumindo item %d\n", item);
        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    buffer_init(&cbuf);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}