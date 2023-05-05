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
    // Prevents flooding messages through message queue
    flood_prevent flood_buffer[FLOOD_LIMIT];
    int flood_index = 0;

    // Initialize flood buffer
    for (int i = 0; i < FLOOD_LIMIT; i++) {
        flood_buffer[i].id[0] = '\0';
        flood_buffer[i].timestamp = 0;
    }

    // Log writer
    char llog_buffer[BUFFER_MESSAGE];

    // Message queue
    msgqueue msg;
    sprintf(llog_buffer, "WATCHER %d ATTACHED TO SHARED MEMORY %d\n", getpid(), shm->shmid);
    log_writer(llog_buffer);

    // Wait for alert signal and process
    while (1) {
        // Lock mutex
        pthread_mutex_lock(&shm->mutex);
        // Wait for alert signal
        pthread_cond_wait(&shm->alert, &shm->mutex);

        // Process alert keys
        for (int i = 0; i < shm->maxAlertKeyInfo; i++) {

            // Check if alert key is in use
            if (shm->alertKeyInfoArray[i].key[0] != '\0') {

                // Check if sensor value is outside alert limits
                if (shm->sensorKeyInfoArray[i].averageValue < shm->alertKeyInfoArray[i].min ||
                    shm->sensorKeyInfoArray[i].averageValue > shm->alertKeyInfoArray[i].max) {
                    msg.msg_type = shm->alertKeyInfoArray[i].console_id;

                    // Check if alert key is already in flood buffer
                    int found = 0;
                    for (int j = 0; j < FLOOD_LIMIT; j++) {
                        if (strcmp(shm->alertKeyInfoArray[i].id, flood_buffer[j].id) == 0) {
                             printf("ID 1: %s ID 2: %s\n", shm->alertKeyInfoArray[i].id, flood_buffer[j].id);
                            // Check if alert key is in flood buffer for more than FLOOD_TIME seconds
                            if (time(NULL) - flood_buffer[j].timestamp >= FLOOD_TIME) {
                                // Update timestamp
                                flood_buffer[j].timestamp = time(NULL);

                                // Send alert message through message queue
                                sprintf(msg.msg_text, "ALERT ID [%s] WITH KEY [%s] GOT %.2f DEGREES [RANGE: %.2f - %.2f]", 
                                    shm->alertKeyInfoArray[i].id, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].averageValue,
                                    shm->alertKeyInfoArray[i].min, shm->alertKeyInfoArray[i].max);
                                msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                                sprintf(msg.msg_text, "END");
                                msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);

                                // Log alert message
                                sprintf(llog_buffer, "ALERT ID [%s] WITH KEY [%s] GOT %.2f DEGREES [RANGE: %.2f - %.2f]\n", 
                                    shm->alertKeyInfoArray[i].id, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].averageValue, 
                                    shm->alertKeyInfoArray[i].min, shm->alertKeyInfoArray[i].max);
                                log_writer(llog_buffer);

                                // Remove alert key
                                //remove_alert_key(shm, shm->alertKeyInfoArray[i].key);
                                found = 1;
                                break;
                            } else {
                                // FLOOD_TIME not reached yet
                                found = 1;
                                break;
                            }
                        }
                    }
                    if (!found) {
                        // Add alert key to flood buffer
                        strcpy(flood_buffer[flood_index].id, shm->alertKeyInfoArray[i].id);
                        flood_buffer[flood_index].timestamp = time(NULL);
                        flood_index = (flood_index + 1) % FLOOD_LIMIT;

                        // Send alert message through message queue
                        sprintf(msg.msg_text, "ALERT ID [%s] WITH KEY [%s] GOT %.2f DEGREES [RANGE: %.2f - %.2f]", 
                            shm->alertKeyInfoArray[i].id, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].averageValue,
                            shm->alertKeyInfoArray[i].min, shm->alertKeyInfoArray[i].max);
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        sprintf(msg.msg_text, "END");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);

                        // Log alert message
                        sprintf(llog_buffer, "ALERT ID [%s] WITH KEY [%s] GOT %.2f DEGREES [RANGE: %.2f - %.2f]\n", 
                            shm->alertKeyInfoArray[i].id, shm->alertKeyInfoArray[i].key, shm->sensorKeyInfoArray[i].averageValue, 
                            shm->alertKeyInfoArray[i].min, shm->alertKeyInfoArray[i].max);
                        log_writer(llog_buffer);

                        // Remove alert key
                        //remove_alert_key(shm, shm->alertKeyInfoArray[i].key);
                        break;
                    }
                    // Print whole flood buffer
                    //for (int j = 0; j < FLOOD_LIMIT; j++) {
                    //    printf("FLOOD BUFFER [%d] ID [%s] TIMESTAMP [%ld]\n", j, flood_buffer[j].id, flood_buffer[j].timestamp);
                    //}
                }
            }
        }
        // Unlock mutex
        pthread_mutex_unlock(&shm->mutex);
    }

    return 0;
}
