/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_workers.h"
#include "sys_shm.h"

// Workers
int create_workers(int nr_workers, int shmid) {
    pid_t pid;
    char message[BUFFER_MESSAGE];
    for (int i = 0; i < nr_workers; i++) {
        pid = fork();
        if (pid == 0) {
            sprintf(message, "WORKER PROCESS %d CREATED\n", getpid());
            log_writer(message);
            worker_tasks(shmid); // Main worker function
            sprintf(message, "WORKER PROCESS %d ENDED\n", getpid());
            log_writer(message);
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
int worker_tasks(int shmid) {
    // Test attach
    SharedMemory *SHM = attach_shm(shmid);
    sprintf(message, "WORKER PROCESS %d ATTACHED TO %d SHM\n", getpid(), shmid);
    log_writer(message);

    // Test detach
    detach_shm(SHM);
    sprintf(message, "WORKER PROCESS %d DETACHED FROM %d SHM\n", getpid(), shmid);
    log_writer(message);
    return 0;
}