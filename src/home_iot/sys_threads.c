/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_threads.h"

// Threads
int create_threads() {
    char message[BUFFER_MESSAGE];
    // Creates console_reader thread
    pthread_t console_reader;
    if (pthread_create(&console_reader, NULL, console_reader_function, NULL) != 0) {
        sprintf(message, "PTHREAD_CREATE_ERROR IN CONSOLE_READER\n");
        log_writer(message);
    } else {
        sprintf(message, "THREAD CONSOLE_READER CREATED\n");
        log_writer(message);
        console_reader_function();
    }

    // Creates sensor_reader thread
    pthread_t sensor_reader;
    if (pthread_create(&sensor_reader, NULL, sensor_reader_function, NULL) != 0) {
        sprintf(message, "PTHREAD_CREATE_ERROR IN SENSOR_READER\n");
        log_writer(message);
    } else {
        sprintf(message, "THREAD SENSOR_READER CREATED\n");
        log_writer(message);
        sensor_reader_function();
    }

    // Creates dispatcher thread
    pthread_t dispatcher;
    if (pthread_create(&dispatcher, NULL, dispatcher_function, NULL) != 0) {
        sprintf(message, "PTHREAD_CREATE_ERROR IN DISPATCHER\n");
        log_writer(message);
    } else {
        sprintf(message, "THREAD DISPATCHER CREATED\n");
        log_writer(message);
        dispatcher_function();
    }

    // Waits for threads to finish
    pthread_join(console_reader, NULL);
    pthread_join(sensor_reader, NULL);
    pthread_join(dispatcher, NULL);
    return 0;
}

// Main thread functions
void *console_reader_function() {
    // Console reader function
    return NULL;
}

void *sensor_reader_function() {
    // Sensor reader function
    return NULL;
}

void *dispatcher_function() {
    // Dispatcher function
    return NULL;
}
