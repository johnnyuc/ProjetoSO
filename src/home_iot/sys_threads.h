#ifndef IOT_PROJECT_THREADS_H
#define IOT_PROJECT_THREADS_H

// Includes
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

// Define
#define READ_PIPE 128

// Functions
void create_named_pipes();
int create_threads();
void *console_reader_function();
void *sensor_reader_function();
void *dispatcher_function();

#endif //IOT_PROJECT_THREADS_H