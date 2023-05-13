/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
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
extern volatile sig_atomic_t sigint;

// Function to create named pipes
void create_named_pipes() {
    char *sensor_pipe = "SENSOR_PIPE";
    char *console_pipe = "CONSOLE_PIPE";

    // Creating named pipes for sensor and console
    if (mkfifo(sensor_pipe, 0666) == -1) {
        sprintf(log_buffer, "ERROR CREATING SENSOR PIPE. EXITING\n");
        log_writer(log_buffer);
        raise(SIGINT);
    }

    if (mkfifo(console_pipe, 0666) == -1) {
        sprintf(log_buffer, "ERROR CREATING CONSOLE PIPE. EXITING\n");
        log_writer(log_buffer);
        raise(SIGINT);
    }
}

// Function to create threads
void create_threads() {
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
}

// Main console reader function
void *console_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    char enqueuing[BUFFER_MESSAGE];
    char buffer[READ_PIPE];
    
    // Open console pipe
    int console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
    if (console_fd == -1) {
        sprintf(llog_buffer, "ERROR OPENING CONSOLE PIPE. EXITING\n");
        log_writer(llog_buffer);
        raise(SIGINT);
    }

    // Polling
    struct pollfd pfd;
    pfd.fd = console_fd;
    pfd.events = POLLIN;
    
    // Console reader function
    while (!sigint) {
        int poll_result = poll(&pfd, 1, 1000); // Blocking poll
        if (poll_result > 0 && (pfd.revents & POLLIN)) {
            ssize_t bytes_read = read(console_fd, buffer, READ_PIPE);
            if (bytes_read > 0) {
                // Write to queue
                strcpy(enqueuing, "CONSOLE#"); // Add console prefix
                strcat(enqueuing, buffer);
                enqueue(intqueue, enqueuing);
            }
            memset(buffer, 0, READ_PIPE);
        } else if (poll_result > 0 && (pfd.revents & POLLHUP)) {
            // Reopen the pipe if it was closed
            close(console_fd);
            console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
            if (console_fd == -1) {
                sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
                log_writer(llog_buffer);
                raise(SIGINT);
            }
            pfd.fd = console_fd;
        }
    }
    return NULL;
}

// Main sensor reader function
void *sensor_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    char enqueuing[BUFFER_MESSAGE];
    char buffer[READ_PIPE];

    // Open sensor pipe
    int sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
        if (sensor_fd == -1) {
            sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
            log_writer(llog_buffer);
            raise(SIGINT);
        }

    // Polling
    struct pollfd pfd;
    pfd.fd = sensor_fd;
    pfd.events = POLLIN;
    
    // Sensor reader function
    while (!sigint) {
        int poll_result = poll(&pfd, 1, 1000); // Blocking poll
        if (poll_result > 0 && (pfd.revents & POLLIN)) {
            ssize_t bytes_read = read(sensor_fd, buffer, READ_PIPE);
            if (bytes_read > 0) {
                char *token = strtok(buffer, "\n"); // tokenize buffer by newline
                while (token != NULL) {
                    // Write to queue
                    sprintf(enqueuing, "SENSOR#%s", token); // Add sensor prefix
                    enqueue(intqueue, enqueuing);
                    token = strtok(NULL, "\n"); // get next token
                }
            }
            memset(buffer, 0, READ_PIPE);
        } else if (poll_result > 0 && (pfd.revents & POLLHUP)) {
            // Reopen the pipe if it was closed
            close(sensor_fd);
            sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
            if (sensor_fd == -1) {
                sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
                log_writer(llog_buffer);
                raise(SIGINT);
            }
            pfd.fd = sensor_fd;
        }
    }

    return NULL;
}

// Main dispatcher function
void *dispatcher_function() {
    
    // Dispatcher function
    while (!sigint) {
        // Takes one message from queue
        char *buffer = dequeue(intqueue);
        if (sigint) break;

        char *buffer_copy = strdup(buffer);

        // Places it into the first avaialble worker pipe
        int worker_task = dequeue_worker(worker_shm);
        if (sigint) break;

        // Write to worker pipe
        int result = write(pipes_fd[worker_task][1], buffer_copy, BUFFER_MESSAGE);
        if (result == -1) {
            sprintf(log_buffer, "ERROR WRITING TO WORKER PIPE. EXITING\n");
            log_writer(log_buffer);
            handle_signint(-1);
        }
        free(buffer_copy);
    }
    return NULL;
}
