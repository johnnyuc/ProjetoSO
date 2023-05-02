#include "sys_manager.h"
#include "sys_workers.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];
extern int **pipes_fd;

// Create unnamed pipes
void create_unnamed_pipes(int **pipes_fd, int nr_workers) {
    for (int i = 0; i < nr_workers; i++) {
        pipes_fd[i] = malloc(2 * sizeof(int));
        pipe(pipes_fd[i]);
    }
}

// Workers
int create_workers(int nr_workers, int shmid, int worker_shmid, int msgid) {
    pid_t pid;
    for (int i = 0; i < nr_workers; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process
            sprintf(log_buffer, "WORKER PROCESS %d CREATED\n", getpid());
            log_writer(log_buffer);

            // Starting worker process
            close(pipes_fd[i][1]); // Close write end of pipe
            SharedMemory *shm = attach_shm(shmid); // Attach shared memory
            WorkerSHM *worker_shm = attach_worker_queue(worker_shmid);  // Attach worker queue

            // Worker process
            worker_tasks(i, worker_shm, shm, pipes_fd[i], msgid); // Main worker function

            // Closing worker process
            detach_shm(shm); // Detach shared memory
            detach_worker_queue(worker_shm); // Detach worker queue

            // Log writer
            sprintf(log_buffer, "WORKER PROCESS %d ENDED\n", getpid());
            log_writer(log_buffer);
            exit(EXIT_SUCCESS);
        } 
    }
    return 0;
}

// Message splitter
int split_message(char *message, char **tokens) {
    int num_tokens = 0;
    char *token = strtok(message, "#");
    while (token != NULL && num_tokens < MAX_TOKENS) {
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, "#");
    }
    return num_tokens;
}

// Main worker function
int worker_tasks(int selfid, WorkerSHM *worker_shm, SharedMemory *shm, int *pipe_fd, int msgid) {
    char llog_buffer[BUFFER_MESSAGE];
    char *tokens[MAX_TOKENS];
    int num_tokens;

    printf("%d", msgid);

    while (1) {
        read(pipe_fd[0], llog_buffer, BUFFER_MESSAGE);
        num_tokens = split_message(llog_buffer, tokens);

        if (num_tokens > 0) {
            if (strcmp(tokens[0], "SENSOR") == 0) {
                int result = insert_sensor_key(shm, tokens[2], atoi(tokens[3]));
                if (result == 2) {
                    sprintf(llog_buffer, "SENSOR %s NOT ADDED. FULL\n", tokens[1]);
                    log_writer(llog_buffer);
                }
                //print_shared_memory(shm);
            } else if (strcmp(tokens[1], "STATS") == 0) {
                print_shared_memory(shm);
            } else if (strcmp(tokens[1], "RESET") == 0) {
                reset_sensor_data(shm);
                print_shared_memory(shm);
            } else if (strcmp(tokens[1], "SENSORS") == 0) {
                
            } else if (strcmp(tokens[1], "ADD_ALERT") == 0) {
                int result = insert_alert_key(shm, tokens[3], atoi(tokens[0]), atof(tokens[4]), atof(tokens[5]));
                if (result == 0) {
                    sprintf(llog_buffer, "ALERT %s ADDED\n", tokens[2]);
                    log_writer(llog_buffer);
                    // Send to message queue from here
                } else if (result == 1) {
                    sprintf(llog_buffer, "ALERT %s ALREADY EXISTS\n", tokens[2]);
                    log_writer(llog_buffer);
                    // Send to message queue from here
                } else if (result == 2) {
                    sprintf(llog_buffer, "ALERT %s NOT ADDED. FULL\n", tokens[2]);
                    log_writer(llog_buffer);
                    // Send to message queue from here
                }
            } else if (strcmp(tokens[1], "REMOVE_ALERT") == 0 ) {
                int result = remove_alert_key(shm, tokens[2]);
                if (result == 1) {
                    sprintf(llog_buffer, "ALERT %s NOT FOUND\n", tokens[2]);
                    log_writer(llog_buffer);
                }
                // Send to message queue from here
            } else if (strcmp(tokens[1], "LIST_ALERTS") == 0) {
                
            }

            // Debug info
            //print_shared_memory(shm);
            //print_worker_queue(worker_shm);
        }
        //print_worker_queue(worker_shm);
        enqueue_worker(worker_shm, selfid);
    }
    return 0;
}