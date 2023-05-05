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
    //char llog_buffer_extra[BUFFER_MESSAGE+32];
    char *tokens[MAX_TOKENS];
    int num_tokens;
    msgqueue msg;

    while (1) {
        // Read message from pipe
        read(pipe_fd[0], llog_buffer, BUFFER_MESSAGE);

        // Register worker activity
        //sprintf(llog_buffer_extra, "DISPATCHER: %s\n", llog_buffer);
        //log_writer(llog_buffer_extra);

        // Split message
        char *tokenizer = strdup(llog_buffer);
        num_tokens = split_message(tokenizer, tokens);
        msg.msg_type = strtol(tokens[1], NULL, 10);

        // Print all tokens
        //for (int i = 0; i < num_tokens; i++) printf("Token [%d]: %s\n", i, tokens[i]);

        if (num_tokens > 0) {
            if (strcmp(tokens[0], "SENSOR") == 0) {
                int result = insert_sensor_key(shm, tokens[1], tokens[2], atoi(tokens[3]));
                if (result == 1) {
                    sprintf(llog_buffer, "WORKER %d: SENSOR [%s] HAS A DUPLICATED ID. DISCARDING\n", selfid, tokens[1]);
                    log_writer(llog_buffer);
                } else if (result == 2) {
                    sprintf(llog_buffer, "WORKER %d: SENSOR [%s] COULD NOT BE ADDED. FULL LIST\n", selfid, tokens[1]);
                    log_writer(llog_buffer);
                } else if (result == 3) {
                    sprintf(llog_buffer, "WORKER %d: SENSOR KEY LIST FULL. DISCARDING DATA\n", selfid);
                    log_writer(llog_buffer);
                }
                //print_shared_memory(shm);
            } else if (strcmp(tokens[0], "CONSOLE") == 0) {
                if (strcmp(tokens[2], "STATS") == 0) {
                    // Message queue
                    sprintf(msg.msg_text, "KEY\t\tLAST\tMIN\tMAX\tAVG\tCOUNT\n");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    // Looking for values to send
                    for (int i = 0; i < shm->maxSensorKeyInfo; i++) {
                        if (strcmp(shm->sensorKeyInfoArray[i].key, "") != 0) {
                            // Message queue
                            sprintf(msg.msg_text, "%s\t\t%d\t%d\t%d\t%.1f\t%d", 
                            shm->sensorKeyInfoArray[i].key, 
                            shm->sensorKeyInfoArray[i].lastValue, 
                            shm->sensorKeyInfoArray[i].minValue, 
                            shm->sensorKeyInfoArray[i].maxValue, 
                            shm->sensorKeyInfoArray[i].averageValue, 
                            shm->sensorKeyInfoArray[i].updateCount);
                            msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        }
                    }
                    sprintf(msg.msg_text, "END");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    //print_shared_memory(shm);
                    // Log writer
                    sprintf(llog_buffer, "WORKER %d: STATISTICAL DATA REQUESTED [CONSOLE %d]\n", selfid, atoi(tokens[1]));
                    log_writer(llog_buffer);
                } else if (strcmp(tokens[2], "RESET") == 0) {
                    int result = reset_sensor_data(shm);
                    if (result == 0) {
                        // Message queue
                        sprintf(msg.msg_text, "OK");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: STATISTICAL DATA RESET REQUESTED [CONSOLE %d]\n", selfid, atoi(tokens[1]));
                        log_writer(llog_buffer);
                    }
                } else if (strcmp(tokens[2], "SENSORS") == 0) {
                    // Message queue
                    sprintf(msg.msg_text, "ID\t\tKEY\n");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    // Looking for sensors to send
                    for (int i = 0; i < shm->maxSensors; i++) {
                        if (strcmp(shm->sensorArray[i].id, "") != 0) {
                            // Message queue
                            sprintf(msg.msg_text, "%s\t\t%s", 
                            shm->sensorArray[i].id, 
                            shm->sensorArray[i].key);
                            msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        }
                    }
                    sprintf(msg.msg_text, "END");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    // Log writer
                    sprintf(llog_buffer, "WORKER %d: SENSOR LIST REQUESTED [CONSOLE %d]\n", selfid, atoi(tokens[1]));
                    log_writer(llog_buffer);
                } else if (strcmp(tokens[2], "ADD_ALERT") == 0) {
                    int result = insert_alert_key(shm, atoi(tokens[1]), tokens[3], tokens[4], atof(tokens[5]), atof(tokens[6]));
                    if (result == 0) { // GOOD
                        // Message queue
                        sprintf(msg.msg_text, "OK");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        printf("%s\n", tokens[3]);
                        sprintf(llog_buffer, "WORKER %d: NEW ALERT [%s] ADDED TO LIST [CONSOLE %d\n]", selfid, tokens[3], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    } else if (result == 1) { // NOT GOOD
                        // Message queue
                        sprintf(msg.msg_text, "ERROR");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: ALERT [%s] EXISTS [CONSOLE %d]\n", selfid, tokens[3], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    } else if (result == 2) { // NOT GOOD
                        // Message queue
                        sprintf(msg.msg_text, "ERROR");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: KEY [%s] NOT FOUND, ALERT NOT ADDED [CONSOLE %d]\n", selfid, tokens[4], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    } else if (result == 3) { // NOT GOOD
                        // Message queue
                        sprintf(msg.msg_text, "ERROR");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: ALERT [%s] NOT ADDED, FULL LIST [CONSOLE %d]\n", selfid, tokens[3], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    }
                } else if (strcmp(tokens[2], "REMOVE_ALERT") == 0 ) {
                    int result = remove_alert_key(shm, tokens[3]);
                    if (result == 0) { // GOOD
                        // Message queue
                        sprintf(msg.msg_text, "OK");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: ALERT [%s] REMOVED FROM LIST BY [CONSOLE %d]\n", selfid, tokens[3], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    } else if (result == 1) { // NOT GOOD
                        // Message queue
                        sprintf(msg.msg_text, "ERROR");
                        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        // Log writer
                        sprintf(llog_buffer, "WORKER %d: ALERT [%s] NOT FOUND INSIDE LIST [CONSOLE %d]\n", selfid, tokens[3], atoi(tokens[1]));
                        log_writer(llog_buffer);
                    }
                } else if (strcmp(tokens[2], "LIST_ALERTS") == 0) {
                    // Message queue
                    sprintf(msg.msg_text, "ID\t\tKEY\t\tMIN\tMAX\n");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    // Looking for alerts to send
                    for (int i = 0; i < shm->maxAlertKeyInfo; i++) {
                        if (strcmp(shm->alertKeyInfoArray[i].id, "") != 0) {
                            sprintf(msg.msg_text, "%s\t\t%s\t\t%0.1f\t%0.1f", 
                            shm->alertKeyInfoArray[i].id,
                            shm->alertKeyInfoArray[i].key, 
                            shm->alertKeyInfoArray[i].min, 
                            shm->alertKeyInfoArray[i].max);
                            msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                        }
                    }
                    sprintf(msg.msg_text, "END");
                    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
                    // Log writer
                    sprintf(llog_buffer, "WORKER %d: ALERT LIST REQUESTED [CONSOLE %d]\n", selfid, atoi(tokens[1]));
                    log_writer(llog_buffer);
                }
            }
        }
        // Debug info
        //print_shared_memory(shm);
        //print_worker_queue(worker_shm);

        // Enables it to receive new tasks
        free(tokenizer); // Freeing strdup memory
        enqueue_worker(worker_shm, selfid);
    }

    return 0;
}