/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_threads.h"
#include "sys_intqueue.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];
extern pthread_t console_reader;
extern pthread_t sensor_reader;
extern pthread_t dispatcher;
extern Queue *intqueue;

// Create named pipes
void create_named_pipes() {
    char *sensor_pipe = "SENSOR_PIPE";
    char *console_pipe = "CONSOLE_PIPE";

    // Creating named pipes for sensor and console
    if (mkfifo(sensor_pipe, 0666) == -1) {
        perror("ERROR CREATING SENSOR PIPE. EXITING");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(console_pipe, 0666) == -1) {
        perror("ERROR CREATING CONSOLE PIPE. EXITING");
        exit(EXIT_FAILURE);
    }
}

// Threads
int create_threads() {
    // Creates console_reader thread
    if (pthread_create(&console_reader, NULL, console_reader_function, NULL) != 0) {
        sprintf(log_buffer, "PTHREAD_CREATE_ERROR IN CONSOLE_READER\n");
        log_writer(log_buffer);
    } else {
        sprintf(log_buffer, "THREAD CONSOLE_READER CREATED\n");
        log_writer(log_buffer);
    }

    // Creates sensor_reader thread
    if (pthread_create(&sensor_reader, NULL, sensor_reader_function, NULL) != 0) {
        sprintf(log_buffer, "PTHREAD_CREATE_ERROR IN SENSOR_READER\n");
        log_writer(log_buffer);
    } else {
        sprintf(log_buffer, "THREAD SENSOR_READER CREATED\n");
        log_writer(log_buffer);
    }

    // Creates dispatcher thread
    if (pthread_create(&dispatcher, NULL, dispatcher_function, NULL) != 0) {
        sprintf(log_buffer, "PTHREAD_CREATE_ERROR IN DISPATCHER\n");
        log_writer(log_buffer);
    } else {
        sprintf(log_buffer, "THREAD DISPATCHER CREATED\n");
        log_writer(log_buffer);
    }

    // Waits for threads to finish
    pthread_join(console_reader, NULL);
    pthread_join(sensor_reader, NULL);
    pthread_join(dispatcher, NULL);
    return 0;
}

// Main thread functions
void *console_reader_function() {
    // char llog_buffer[BUFFER_MESSAGE];
    // Console reader function
    return NULL;
}

void *sensor_reader_function() {
    // char llog_buffer[BUFFER_MESSAGE];
    // Sensor reader function
    return NULL;
}


void *dispatcher_function() {
    // char llog_buffer[BUFFER_MESSAGE];
    // Dispatcher function
    return NULL;
}
