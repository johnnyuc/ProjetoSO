// Author: Johnny Fernandes 2021190668
// LEI UC 2022-23 - Sistemas Operativos

#ifndef IOT_PROJECT_THREADS_H
#define IOT_PROJECT_THREADS_H

// Includes
#include <stdio.h>
#include <pthread.h>

// Functions
int create_threads();
void *console_reader_function();
void *sensor_reader_function();
void *dispatcher_function();

#endif //IOT_PROJECT_THREADS_H