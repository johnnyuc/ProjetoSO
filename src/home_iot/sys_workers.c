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
int create_workers(int nr_workers, int shmid, int worker_shmid) {
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
            worker_tasks(i, worker_shm, shm, pipes_fd[i]); // Main worker function

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
int worker_tasks(int selfid, WorkerSHM *worker_shm, SharedMemory *shm, int *pipe_fd) {
    char llog_buffer[BUFFER_MESSAGE];
    char *tokens[MAX_TOKENS];
    int num_tokens;

    while (1) {
        read(pipe_fd[0], llog_buffer, BUFFER_MESSAGE);
        num_tokens = split_message(llog_buffer, tokens);

        if (num_tokens > 0) {
            if (strcmp(tokens[1], "STATS") == 0) {
                printf("Worker %d received message: %s\n", selfid, tokens[1]);
                print_shared_memory(shm);
            } else if (strcmp(tokens[1], "RESET") == 0) {
                
            } else if (strcmp(tokens[1], "SENSORS") == 0) {
                
            } else if (strcmp(tokens[1], "ADD_ALERT") == 0) {
                insert_alert_key(shm, tokens[2], atof(tokens[4]), atof(tokens[5]));
                print_shared_memory(shm);
            } else if (strcmp(tokens[1], "REMOVE_ALERT") == 0 ) {
                remove_alert_key(shm, tokens[2]);
                print_shared_memory(shm);
            } else if (strcmp(tokens[1], "LIST_ALERTS") == 0) {
                
            }
        }
        //print_worker_queue(worker_shm);
        printf("Trying to enqueue worker %d\n", selfid);
        enqueue_worker(worker_shm, selfid);
        printf("Enqueued worker %d\n", selfid);
    }
    return 0;
}