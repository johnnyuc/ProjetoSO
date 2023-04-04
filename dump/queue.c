#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define QUEUE_SIZE 10

typedef struct queue {
    int buffer[QUEUE_SIZE];
    pthread_mutex_t lock;
    int readpos, writepos;
    int count;
} queue;

void queue_init(queue *q) {
    pthread_mutex_init(&q->lock, NULL);
    q->readpos = 0;
    q->writepos = 0;
    q->count = 0;
}

void queue_push(queue *q, int data) {
    pthread_mutex_lock(&q->lock);
    while (q->count == QUEUE_SIZE) {
        pthread_mutex_unlock(&q->lock);
        pthread_self();
        pthread_mutex_lock(&q->lock);
    }
    q->buffer[q->writepos] = data;
    q->writepos++;
    if (q->writepos == QUEUE_SIZE)
        q->writepos = 0;
    q->count++;
    pthread_mutex_unlock(&q->lock);
}

int queue_pop(queue *q) {
    int data;
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        pthread_mutex_unlock(&q->lock);
        pthread_self();
        pthread_mutex_lock(&q->lock);
    }
    data = q->buffer[q->readpos];
    q->readpos++;
    if (q->readpos == QUEUE_SIZE)
        q->readpos = 0;
    q->count--;
    pthread_mutex_unlock(&q->lock);
    return data;
}

void* producer_thread(void* arg) {
    queue *q = (queue*) arg;
    int i;
    for (i = 0; i < 100; i++) {
        printf("Producer thread pushing data: %d\n", i);
        queue_push(q, i);
    }
    return NULL;
}

void* consumer_thread(void* arg) {
    queue *q = (queue*) arg;
    int i, data;
    for (i = 0; i < 100; i++) {
        data = queue_pop(q);
        printf("Consumer thread popped data: %d\n", data);
    }
    return NULL;
}

int main() {
    queue q;
    pthread_t producer, consumer1, consumer2;
    queue_init(&q);
    pthread_create(&producer, NULL, producer_thread, &q);
    pthread_create(&consumer1, NULL, consumer_thread, &q);
    pthread_create(&consumer2, NULL, consumer_thread, &q);
    pthread_join(producer, NULL);
    pthread_join(consumer1, NULL);
    pthread_join(consumer2, NULL);
    pthread_mutex_destroy(&q.lock);
    return 0;
}
