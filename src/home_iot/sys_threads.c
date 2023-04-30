/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_threads.h"
#include "sys_intqueue.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];
extern pthread_t console_reader;
extern pthread_t sensor_reader;
extern pthread_t dispatcher;
extern Queue *intqueue;
extern WorkerSHM *worker_shm;
extern int **pipes_fd;

#include <sys/select.h>

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
    
    // Open console pipe
    int console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
    if (console_fd == -1) {
        sprintf(llog_buffer, "ERROR OPENING CONSOLE PIPE. EXITING\n");
        log_writer(llog_buffer);
        exit(EXIT_FAILURE);
    }
    
    // Console reader function
    while (1) {
        // Use select() to wait for data to become available on the console pipe
        // Safer because it won't nor block nor accept pipe overloading even though client is limited to 0.25s
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(console_fd, &rfds);
        int result = select(console_fd + 1, &rfds, NULL, NULL, NULL);
        if (result == -1) {
            sprintf(llog_buffer, "ERROR READING FROM CONSOLE PIPE. EXITING\n");
            log_writer(llog_buffer);
            exit(EXIT_FAILURE);
        } else if (result > 0 && FD_ISSET(console_fd, &rfds)) {
            // Data is available on the console pipe, read it
            char buffer[BUFFER_MESSAGE];
            if (read(console_fd, buffer, BUFFER_MESSAGE) > 0) {
                // Write to queue
                enqueue(intqueue, buffer);
                printf("CONSOLE READER: %s\n", buffer);
            }
        }
    }

    return NULL;
}


void *sensor_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    
    // Open sensor pipe
    int sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
    if (sensor_fd == -1) {
        sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
        log_writer(llog_buffer);
        exit(EXIT_FAILURE);
    }
    
    // Sensor reader function
    while (1) {
        // Use select() to wait for data to become available on the sensor pipe
        // Safer because it won't nor block nor accept pipe overloading even though sensor is limited to 0.25s
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sensor_fd, &rfds);
        int result = select(sensor_fd + 1, &rfds, NULL, NULL, NULL);
        if (result == -1) {
            sprintf(llog_buffer, "ERROR READING FROM SENSOR PIPE. EXITING\n");
            log_writer(llog_buffer);
            exit(EXIT_FAILURE);
        } else if (result > 0 && FD_ISSET(sensor_fd, &rfds)) {
            // Data is available on the sensor pipe, read it
            char buffer[BUFFER_MESSAGE];
            if (read(sensor_fd, buffer, BUFFER_MESSAGE) > 0) {
                // Write to queue
                enqueue(intqueue, buffer);
                printf("SENSOR READER: %s\n", buffer);
            }
        }
    }

    return NULL;
}


void *dispatcher_function() {
    char llog_buffer[BUFFER_MESSAGE];

    // Dispatcher function
    while (1) {
        // Takes one message from queue
        char *buffer = dequeue(intqueue);
        // Places it into the first avaialble worker pipe
        int worker_task = dequeue_worker(worker_shm);

        // Write to log
        sprintf(llog_buffer, "DISPATCHER SENT %s TO WORKER %d\n", buffer, worker_task);
        log_writer(llog_buffer);
        
        // Write to worker pipe
        write(pipes_fd[worker_task][1], buffer, BUFFER_MESSAGE);

        //print_worker_queue(worker_shm);
    }

    return NULL;
}
