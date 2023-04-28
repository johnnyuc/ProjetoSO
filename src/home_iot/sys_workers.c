/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

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
int create_workers(int nr_workers, int shmid) {
    pid_t pid;
    for (int i = 0; i < nr_workers; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process
            sprintf(log_buffer, "WORKER PROCESS %d CREATED\n", getpid());
            log_writer(log_buffer);
            close(pipes_fd[i][1]); // Close write end of pipe
            worker_tasks(shmid, pipes_fd[i]); // Main worker function
            sprintf(log_buffer, "WORKER PROCESS %d ENDED\n", getpid());
            log_writer(log_buffer);
            exit(EXIT_SUCCESS);
        } 
    }
 
    // Waiting for all child processes to finish
    for (int i = 0; i < nr_workers; i++) {
        wait(NULL);
    }
    return 0;
}

// Main worker function
int worker_tasks(int shmid, int *pipe_fd) {
    char llog_buffer[BUFFER_MESSAGE];
    char *command;
    SharedMemory *shm = attach_shm(shmid);

    while (1) {
        read(pipe_fd[0], llog_buffer, BUFFER_MESSAGE);
        command = strtok(llog_buffer, "#");
        pthread_mutex_lock(&shm->mutex);
        if (strcmp(command, "stats") == 0) {
            printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        } else if (strcmp(command, "reset") == 0) {
            printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        } else if (strcmp(command, "sensors") == 0) {
            printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        } else if (strcmp(command, "add_alert") == 0) {
            printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        } else if (strcmp(command, "remove_alert") == 0 ) {
           printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        } else if (strcmp(command, "list_alerts") == 0) {
            printf("WORKER %d RECEIVED: %s\n", getpid(), llog_buffer);
        }
        pthread_mutex_unlock(&shm->mutex);
    }

    detach_shm(shm);
    return 0;
}