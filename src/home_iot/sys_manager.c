/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"
#include "sys_intqueue.h"
#include "sys_threads.h"
#include "sys_workers.h"
#include "sys_alerts.h"
#include "sys_shm.h"

// Global variables
pid_t parent;

// Loading
ConfigValues config_vals;

// Log writer
char log_buffer[BUFFER_MESSAGE];
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Threads
pthread_t console_reader;
pthread_t sensor_reader;
pthread_t dispatcher;

// Shared memory (internal and worker)
SharedMemory *shm = NULL;
WorkerSHM *worker_shm = NULL;

// Queue (internal and message)
Queue *intqueue = NULL;
int msgid = -1;

// Pipes (named and unnamed)
int **pipes_fd = NULL;
int sensor_fd, console_fd;

// Signals
volatile sig_atomic_t sigint = 0;

// Function to handle the SIGTSTP signal
void handle_sigtstp() {
    print_full_data(shm, worker_shm);
}

// Function to handle the SIGINT signal
void handle_signint(int sig) {
    // Signaling threads
    sigint = 1;

    // Releases dispatcher and readers from waiting (threads)
    if (intqueue != NULL) {
        pthread_cond_broadcast(&intqueue->cond_empty);
        pthread_cond_broadcast(&intqueue->cond_full);
    }

    // Only for childs to execute
    if (getpid() != parent) {
        // Waiting for workers to finish
        for (int i = 0; i < config_vals.nr_workers; i++) {
            wait(NULL);
        }

        // Wait for Watcher
        wait(NULL);

        exit(EXIT_SUCCESS);
    }

    // Log writer
    printf("\n");
    sprintf(log_buffer, "PREPARING TO SHUTDOWN...\n");
    log_writer(log_buffer);
    sprintf(log_buffer, "WORKERS, WATCHER AND READERS+DISPATCHER FINISHED\n");
    log_writer(log_buffer);

    // Closing and freeing unnamed pipes
    if (pipes_fd != NULL) {
        for (int i = 0; i < config_vals.nr_workers; i++) {
            close(pipes_fd[i][0]);
            close(pipes_fd[i][1]);
            free(pipes_fd[i]);
        }
        free(pipes_fd);
    }

    // Closing named pipes
    close(sensor_fd);
    close(console_fd);

    // Remove named pipes
    unlink("SENSOR_PIPE");
    unlink("CONSOLE_PIPE");

    // Log writer
    sprintf(log_buffer, "PIPES SUCCESSFULLY REMOVED\n");
    log_writer(log_buffer);

    // Store unhandled data to log
    // Grab data from internal queue
    if (intqueue != NULL) {
        QueueNode *node = intqueue->head;
        while (node != NULL) {
            sprintf(log_buffer, "INTERNAL QUEUE HOLD: %s\n", node->data);
            log_writer(log_buffer);
            QueueNode *temp = node;
            node = node->next;
            free(temp);
        }
        remove_queue(intqueue);
    }

    // Log writer
    sprintf(log_buffer, "INTERNAL QUEUE SUCCESSFULLY REMOVED\n");
    log_writer(log_buffer);
    
    // Freeing and detaching shm
    if (shm != NULL && shm != (void *) -1) remove_shm(shm);
    if (worker_shm != NULL && worker_shm != (void *) -1) remove_worker_queue(worker_shm);

    // Log writer
    sprintf(log_buffer, "SHARED MEMORY AND WORKER QUEUE SUCCESSFULLY REMOVED\n");
    log_writer(log_buffer);

    // Freeing queue
    if (msgid != -1) {
        if (msgctl(msgid, IPC_RMID, NULL) == -1) {
            sprintf(log_buffer, "COULD NOT REMOVE MESSAGE QUEUE\n");
            log_writer(log_buffer);
        } else {
            sprintf(log_buffer, "MESSAGE QUEUE SUCCESSFULLY REMOVED\n");
            log_writer(log_buffer);
        }
    }
    
    // Log writer closure
    sprintf(log_buffer, "HOME_IOT SHUTTING DOWN [SIG CODE %d]\n", sig);
    log_writer(log_buffer);

    // Destroying log mutex
    pthread_mutex_destroy(&log_mutex);
    exit(EXIT_SUCCESS);
}

// Loading config file values
ConfigValues config_loader(char* filepath) {
    char config[BUFFER_MESSAGE];
    ConfigValues values = {0};
    FILE* config_file = fopen(filepath, "r");

    if (config_file == NULL) {
        sprintf(config, "FAILED TO OPEN CONFIG FILE. EXITING\n");
        log_writer(config);
        exit(EXIT_FAILURE); // No need for resource cleanup
    }
    
    int param_count = 0;
    while (fgets(config, sizeof(config), config_file)) {
        char* token = strtok(config, " \t\r\n"); // For each line, get the first token
        if (token == NULL ||  token[0] == '#') {
            continue;  // Skip empty lines and comments
        }

        int value = atoi(token);
        if (value == 0) continue;

        // Fill the values struct with the values from the config file
        if (values.queue_size == 0) {
            values.queue_size = value;
        } else if (values.nr_workers == 0) {
            values.nr_workers = value;
        } else if (values.max_shmkeys == 0) {
            values.max_shmkeys = value;
        } else if (values.max_sensors == 0) {
            values.max_sensors = value;
        } else if (values.max_alerts == 0) {
            values.max_alerts = value;
        }
        param_count++;
    }

    // Checks if the config file has the correct number of values
    if (param_count != 5) {
        sprintf(config, "INCORRECT CONFIG FILE PARAMS. EXITING\n");
        log_writer(config);
        exit(EXIT_FAILURE); // No need for resource cleanup
    }
    fclose(config_file);
    return values;
}

// Function to write to log file and to the screen
void log_writer(char* log_buffer) {
    pthread_mutex_lock(&log_mutex);

    // Timestamp format: dd/mm/yy hh:mm:ss
    // https://en.cppreference.com/w/c/chrono/strftime
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    // Formats the timestamp
    char timestamp[BUFFER_TIME];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", tm_info);

    // Writes the message to the screen
    printf("[%s] %s", timestamp, log_buffer);

    // Writes the message to the log file
    FILE* log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        printf("LOG ERROR: CANNOT WRITE TO LOG\n");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "[%s] %s", timestamp, log_buffer);
    fclose(log_file);

    // Unlocks the mutex
    pthread_mutex_unlock(&log_mutex);
    memset(log_buffer, 0, BUFFER_MESSAGE); // Clears the message buffer
}

// Main function to initialize the system manager
void main_initializer() { // Crash safe
    // Creating shared memory
    shm = create_shm(config_vals.max_shmkeys, config_vals.max_alerts, config_vals.max_sensors);
    sprintf(log_buffer, "SHARED MEMORY %d SUCCESSFULLY CREATED\n", shm->shmid);
    log_writer(log_buffer);

    // Creating workers and it's own shared memory
    worker_shm = create_worker_queue(config_vals.nr_workers);
    sprintf(log_buffer, "WORKER SHARED MEMORY %d SUCCESSFULLY CREATED\n", worker_shm->shmid);
    log_writer(log_buffer);

    // Creating internal queue
    intqueue = create_queue(config_vals.queue_size);
    sprintf(log_buffer, "INTERNAL QUEUE SUCCESSFULLY CREATED\n");
    log_writer(log_buffer);

    // Creating message queue
    key_t key = ftok(".", 'a');
    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        sprintf(log_buffer, "COULD NOT CREATE MESSAGE QUEUE\n");
        log_writer(log_buffer);
        raise(SIGINT);
    } else {
        sprintf(log_buffer, "MESSAGE QUEUE %d SUCCESSFULLY CREATED\n", msgid);
        log_writer(log_buffer);
    }

    // Creating unnamed pipes for workers
    pipes_fd = malloc(config_vals.nr_workers * sizeof(int *));
    create_unnamed_pipes(pipes_fd, config_vals.nr_workers);
    sprintf(log_buffer, "UNNAMED PIPES SUCCESSFULLY CREATED\n");
    log_writer(log_buffer);

    // Creating named pipes for sensor and console
    create_named_pipes();
    sprintf(log_buffer, "NAMED PIPES SUCCESSFULLY CREATED\n");
    log_writer(log_buffer);

    // Creating processes and threads
    create_workers(config_vals.nr_workers, shm->shmid, worker_shm->shmid, msgid);
    create_watcher(shm->shmid, msgid);
    create_threads();
}

// Main function
int main(int argc, char *argv[]) {
    parent = getpid();

    // Signal handlers
    signal(SIGINT, handle_signint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGQUIT, SIG_IGN);

    // Verifies if the config file path was passed as a parameter
    if (argc != 2) {
        sprintf(log_buffer, "INVALID CONFIG ARGUMENT ON START. EXITING\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    // Inits log writer
    FILE *fp = fopen(LOG_PATH, "w");
    if (fp == NULL) {
        printf("ERROR OPENING LOG WRITER\n");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    // Log writer
    sprintf(log_buffer, "HOME_IOT SIMULATOR STARTING\n");
    log_writer(log_buffer);
    
    // Loading config file values
    config_vals = config_loader(argv[1]);
    // Boots up
    main_initializer();

    return 0;
}