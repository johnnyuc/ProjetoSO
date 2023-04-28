/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_alerts.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];

// Workers
int create_watcher(int shmid) {
    pid_t pid = fork();
    if (pid == 0) {
        sprintf(log_buffer, "WATCHER PROCESS %d CREATED\n", getpid());
        log_writer(log_buffer);
        watcher_tasks(shmid);
        sprintf(log_buffer, "WATCHER PROCESS %d ENDED\n", getpid());
        log_writer(log_buffer);
        exit(EXIT_SUCCESS);
    }
    
    // Waiting for alert watcher to finish
    wait(NULL);
    return 0;
}

int watcher_tasks(int shmid) {
    char llog_buffer[BUFFER_MESSAGE];
    // Test attach
    SharedMemory *SHM = attach_shm(shmid);
    sprintf(llog_buffer, "WATCHER PROCESS %d ATTACHED TO %d SHM\n", getpid(), shmid);
    log_writer(llog_buffer);

    // Test detach
    detach_shm(SHM);
    sprintf(llog_buffer, "WATCHER PROCESS %d DETACHED FROM %d SHM\n", getpid(), shmid);
    log_writer(llog_buffer);
    return 0;
}