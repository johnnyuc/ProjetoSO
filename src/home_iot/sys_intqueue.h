#ifndef IOT_PROJECT_INTQUEUE_H
#define IOT_PROJECT_INTQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structs
typedef struct QueueNode {
    char *data;
    struct QueueNode *next;
} QueueNode;

typedef struct Queue {
    int size;
    QueueNode *head;
    QueueNode *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} Queue;

// Functions
Queue *create_queue();
void enqueue(Queue *queue, char *data);
char *dequeue(Queue *queue);

#endif //IOT_PROJECT_INTQUEUE_H