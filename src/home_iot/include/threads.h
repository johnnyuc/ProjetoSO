#ifndef IOT_PROJECT_THREADS_H
#define IOT_PROJECT_THREADS_H

#include <pthread.h>

int create_threads();
void *console_reader(void *arg);
void *sensor_reader(void *arg);
void *dispatcher(void *arg);


#endif //IOT_PROJECT_THREADS_H