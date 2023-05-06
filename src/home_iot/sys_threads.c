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

#include <errno.h>

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
    char enqueuing[BUFFER_MESSAGE];
    char buffer[READ_PIPE];
    
    // Open console pipe
    int console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
    if (console_fd == -1) {
        sprintf(llog_buffer, "ERROR OPENING CONSOLE PIPE. EXITING\n");
        log_writer(llog_buffer);
        exit(EXIT_FAILURE);
    }

    // Polling
    struct pollfd pfd;
    pfd.fd = console_fd;
    pfd.events = POLLIN;
    
    // Console reader function
    while (1) {
        int poll_result = poll(&pfd, 1, -1); // Blocking poll
        if (poll_result > 0 && (pfd.revents & POLLIN)) {
            ssize_t bytes_read = read(console_fd, buffer, READ_PIPE);
            if (bytes_read > 0) {
                // Write to queue
                strcpy(enqueuing, "CONSOLE#"); // Add console prefix
                strcat(enqueuing, buffer);
                enqueue(intqueue, enqueuing);
                //printf("CONSOLE READER: %s\n", enqueuing);
            }
            memset(buffer, 0, READ_PIPE);
        } else if (poll_result > 0 && (pfd.revents & POLLHUP)) {
            // Reopen the pipe if it was closed
            close(console_fd);
            console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);
            if (console_fd == -1) {
                sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
                log_writer(llog_buffer);
                exit(EXIT_FAILURE);
            }
            pfd.fd = console_fd;
        }
    }

    return NULL;
}


void *sensor_reader_function() {
    char llog_buffer[BUFFER_MESSAGE];
    char enqueuing[BUFFER_MESSAGE];
    char buffer[READ_PIPE];

    // Open sensor pipe
    int sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
        if (sensor_fd == -1) {
            sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
            log_writer(llog_buffer);
            exit(EXIT_FAILURE);
        }

    // Polling
    struct pollfd pfd;
    pfd.fd = sensor_fd;
    pfd.events = POLLIN;
    
    // Sensor reader function
    while (1) {
        int poll_result = poll(&pfd, 1, -1); // Blocking poll
        if (poll_result > 0 && (pfd.revents & POLLIN)) {
            ssize_t bytes_read = read(sensor_fd, buffer, READ_PIPE);
            if (bytes_read > 0) {
                // Write to queue
                strcpy(enqueuing, "SENSOR#"); // Add sensor prefix
                strcat(enqueuing, buffer);
                enqueue(intqueue, enqueuing);
                //printf("CONSOLE READER: %s\n", enqueuing);
            }
            memset(buffer, 0, READ_PIPE);
        } else if (poll_result > 0 && (pfd.revents & POLLHUP)) {
            // Reopen the pipe if it was closed
            close(sensor_fd);
            sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
            if (sensor_fd == -1) {
                sprintf(llog_buffer, "ERROR OPENING SENSOR PIPE. EXITING\n");
                log_writer(llog_buffer);
                exit(EXIT_FAILURE);
            }
            pfd.fd = sensor_fd;
        }
    }

    return NULL;
}


void *dispatcher_function() {
    
    // Dispatcher function
    while (1) {
        // Takes one message from queue
        char *buffer = dequeue(intqueue);
        char *buffer_copy = strdup(buffer);

        // Places it into the first avaialble worker pipe
        int worker_task = dequeue_worker(worker_shm);
        
        // Write to log
        //sprintf(llog_buffer, "DISPATCHER SENT %s TO WORKER %d\n", buffer, worker_task);
        //log_writer(llog_buffer);
        
        // Write to worker pipe
        write(pipes_fd[worker_task][1], buffer_copy, BUFFER_MESSAGE);
        
        free(buffer_copy);
    }

    return NULL;
}
