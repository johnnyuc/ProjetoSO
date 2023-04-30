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

    // Waiting for threads to finish
    pthread_join(console_reader, NULL);
    pthread_join(sensor_reader, NULL);
    pthread_join(dispatcher, NULL);

    return 0;
}

// Main thread functions
void *console_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    // Console reader function

    // Open console pipe
    int console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
    if(console_fd == -1){
        sprintf(llog_buffer, "ERROR OPENING CONSOLE PIPE. EXITING\n");
        log_writer(llog_buffer);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // read from sensor pipe
        char buffer[BUFFER_MESSAGE];
        if (read(console_fd, buffer, BUFFER_MESSAGE) > 0) {
            // Write to queue
            enqueue(intqueue, buffer);
            printf("CONSOLE READER: %s\n", buffer);
        }
    }
    return NULL;
}

void *sensor_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    // Sensor reader function

    // Open sensor pipe
    int sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
    if (sensor_fd == -1) {
        sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
        log_writer(llog_buffer);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // read from sensor pipe
        char buffer[BUFFER_MESSAGE];
        if (read(sensor_fd, buffer, BUFFER_MESSAGE) > 0) {
            // Write to queue
            enqueue(intqueue, buffer);
            printf("SENSOR READER: %s\n", buffer);
        }
    }
    return NULL;
}

void *dispatcher_function() {
    //char llog_buffer[BUFFER_MESSAGE];
    // Dispatcher function

    // reads data from internal queue
    while (1) {
        // Read from queue
        char *buffer = dequeue(intqueue);
        printf("DISPATCHER: %s\n", buffer);
    }
    return NULL;
}
