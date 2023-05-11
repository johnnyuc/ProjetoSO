/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"
#include "sys_intqueue.h"

// System global variables [created in sys_manager.c]
extern ConfigValues config_vals;

// Function to create internal queue
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

void remove_queue(Queue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond_full);
    pthread_cond_destroy(&queue->cond_empty);
    free(queue);
}

// Function to enqueue data
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

// Function to dequeue data
char *dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);

    // If queue is empty, wait for it to be filled
    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond_empty, &queue->mutex);
    }

    // Searcher
    char *data = NULL;
    QueueNode *node = queue->head;

    // Prioritization of "CONSOLE" over "SENSOR"
    // Find the first node that does not start with "SENSOR"
    while (node != NULL) {
        if (strncmp(node->data, "SENSOR", strlen("SENSOR")) != 0) {
            data = node->data;
            break;
        }
        node = node->next;
    }

    // If no node was found, dequeue the first node that starts with "SENSOR"
    if (data == NULL) {
        node = queue->head;
        while (node != NULL) {
            if (strncmp(node->data, "SENSOR", strlen("SENSOR")) == 0) {
                data = node->data;
                break;
            }
            node = node->next;
        }
    }

    // If a node was found, dequeue it
    if (data != NULL) {
        if (queue->head == node) {
            queue->head = node->next;
            if (queue->head == NULL) {
                queue->tail = NULL;
            }
        } else {
            QueueNode *prev = queue->head;
            while (prev->next != node) {
                prev = prev->next;
            }
            prev->next = node->next;
            if (prev->next == NULL) {
                queue->tail = prev;
            }
        }
        queue->size--;
        free(node);
        pthread_cond_signal(&queue->cond_full);
    }
    pthread_mutex_unlock(&queue->mutex);
    return data;
}