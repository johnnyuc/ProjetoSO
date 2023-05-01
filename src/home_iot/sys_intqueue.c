#include "sys_manager.h"
#include "sys_intqueue.h"

// System global variables [created in sys_manager.c]
extern ConfigValues config_vals;

Queue *create_queue() {
    Queue *queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond_full, NULL);
    pthread_cond_init(&queue->cond_empty, NULL);
    return queue;
}

void enqueue(Queue *queue, char *data) {
    QueueNode *node = malloc(sizeof(QueueNode));
    node->data = strdup(data);
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    while (queue->size >= config_vals.queue_size) {
        pthread_cond_wait(&queue->cond_full, &queue->mutex);
    }
    if (queue->size == 0) {
        queue->head = node;
        queue->tail = node;
        pthread_cond_broadcast(&queue->cond_empty);
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
    queue->size++;
    pthread_mutex_unlock(&queue->mutex);
}


char *dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond_empty, &queue->mutex);
    }

    char *data = queue->head->data;
    QueueNode *node = queue->head;
    queue->head = queue->head->next;
    queue->size--;

    if (queue->size == 0) {
        queue->tail = NULL;
    }
    
    free(node);
    pthread_cond_signal(&queue->cond_full);
    pthread_mutex_unlock(&queue->mutex);
    return data;
}