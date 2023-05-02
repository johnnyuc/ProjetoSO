#include "sys_manager.h"
#include "sys_alerts.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];

// Workers
int create_watcher(int shmid, int msgid) {
    pid_t pid = fork();
    if (pid == 0) {
        // Watcher process
        sprintf(log_buffer, "WATCHER PROCESS %d CREATED\n", getpid());
        log_writer(log_buffer);

        // Starting watcher process
        SharedMemory *shm = attach_shm(shmid);

        // Watcher process
        watcher_tasks(shm, msgid);

        // Closing worker process
        detach_shm(shm); // Detach shared memory

        // Log writer
        sprintf(log_buffer, "WATCHER PROCESS %d ENDED\n", getpid());
        log_writer(log_buffer);
        exit(EXIT_SUCCESS);
    }
    
    // Waiting for alert watcher to finish
    return 0;
}

int watcher_tasks(SharedMemory *shm, int msgid) {
    char llog_buffer[BUFFER_MESSAGE];
    msgqueue msg;
    sprintf(llog_buffer, "WATCHER PROCESS %d ATTACHED TO %d SHM\n", getpid(), shm->shmid);
    log_writer(llog_buffer);

    // Wait for alert signal and process
    while (1) {
        pthread_mutex_lock(&shm->mutex);
        // Wait for alert signal
        pthread_cond_wait(&shm->alert, &shm->mutex);
        // Process alert keys
        for (int i = 0; i < shm->maxAlertKeyInfo; i++) {
            // Check if alert key is in use
            if (shm->alertKeyInfoArray[i].key[0] != '\0') {
                // Check if sensor value is outside alert limits
                if (shm->sensorKeyInfoArray[i].lastValue < shm->alertKeyInfoArray[i].min ||
                    shm->sensorKeyInfoArray[i].lastValue > shm->alertKeyInfoArray[i].max) {
                    // Send alert message through message queue
                    msg.msg_type = shm->alertKeyInfoArray[i].id;
                    sprintf(msg.msg_text, "ALERT ID %s WITH KEY: %s GOT VALUE: %d (RANGE)", 
                        shm->alertKeyInfoArray[i].key, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].lastValue);
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);

                    // Log alert message
                    sprintf(llog_buffer, "ALERT TRIGGERED. ALERT ID %s WITH KEY: %s GOT VALUE: %d (RANGE)\n", 
                        shm->alertKeyInfoArray[i].key, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].lastValue);
                    log_writer(llog_buffer);

                    // Remove alert key
                    //remove_alert_key(shm, shm->alertKeyInfoArray[i].key);
                }
            }
        }
        pthread_mutex_unlock(&shm->mutex);
    }
    return 0;
}
