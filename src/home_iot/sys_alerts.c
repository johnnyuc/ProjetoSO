/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_alerts.h"
#include "sys_shm.h"

char message[BUFFER_MESSAGE];

// Workers
int create_watcher(int shmid) {
    pid_t pid = fork();
    if (pid == 0) {
        sprintf(message, "WATCHER PROCESS %d CREATED\n", getpid());
        log_writer(message);
        watcher_tasks(shmid);
        sprintf(message, "WATCHER PROCESS %d ENDED\n", getpid());
        log_writer(message);
        exit(EXIT_SUCCESS);
    }
    
    // Waiting for alert watcher to finish
    wait(NULL);
    return 0;
}

int watcher_tasks(int shmid) {
    // Test attach
    SharedMemory *SHM = attach_shm(shmid);
    sprintf(message, "WATCHER PROCESS %d ATTACHED TO %d SHM\n", getpid(), shmid);
    log_writer(message);

    // Test detach
    detach_shm(SHM);
    sprintf(message, "WATCHER PROCESS %d DETACHED FROM %d SHM\n", getpid(), shmid);
    log_writer(message);
    return 0;
}