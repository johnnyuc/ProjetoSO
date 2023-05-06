// Includes
#include "sys_manager.h"
#include "sys_intqueue.h"
#include "sys_threads.h"
#include "sys_workers.h"
#include "sys_alerts.h"
#include "sys_shm.h"

// Global variables
// Loading
ConfigValues config_vals;

// Log writer
char log_buffer[BUFFER_MESSAGE];
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Shared memory
SharedMemory *shm;
WorkerSHM *worker_shm;

// Threads
pthread_t console_reader;
pthread_t sensor_reader;
pthread_t dispatcher;

// Queue
Queue *intqueue;
int msgid; // Message queue id

// Pipes
int **pipes_fd;
int sensor_fd, console_fd;

void handle_sigtstp() {
    print_full_data(shm, worker_shm);
}

// Function to handle the SIGINT signal
void handle_signint(int sig) {
    // Tricky part:
    // Careful about cleaning up resources like waiting for all threads to finish
    // processes to finish, detaching access to shm, free calloc and then exit and write to log
    
    // Waiting for workers to finish
    for (int i = 0; i < config_vals.nr_workers; i++) {
        wait(NULL);
    }

    // Wait for Watcher
    wait(NULL);

    // Gracefully exiting threads
    pthread_cancel(console_reader);
    pthread_cancel(sensor_reader);
    pthread_cancel(dispatcher);

    sprintf(log_buffer, "PREPARING TO SHUTDOWN...\n");
    log_writer(log_buffer);

    // Closing and freeing unnamed pipes
    for (int i = 0; i < config_vals.nr_workers; i++) {
        close(pipes_fd[i][0]);
        close(pipes_fd[i][1]);
        free(pipes_fd[i]);
    }
    free(pipes_fd);

    // Store unhandled data to log
    pthread_mutex_destroy(&intqueue->mutex); // Destroying mutex

    // Grab data from internal queue
    QueueNode *node = intqueue->head;
    while (node != NULL) {
        sprintf(log_buffer, "INTERNAL QUEUE HOLD: %s\n", node->data);
        log_writer(log_buffer);
        node = node->next;
    }

    // Closing named pipes
    close(sensor_fd);
    close(console_fd);
    
    // To remove the named pipe from filesystem
    unlink("SENSOR_PIPE");
    unlink("CONSOLE_PIPE");

    // Freeing and detaching shm
    remove_shm(shm);
    remove_worker_queue(worker_shm);

    // Freeing queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    
    sprintf(log_buffer, "HOME_IOT SHUTTING DOWN [SIG CODE %d]\n", sig);
    log_writer(log_buffer);

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
        exit(EXIT_FAILURE);
    }
    
    int param_count = 0;
    while (fgets(config, sizeof(config), config_file)) {
        char* token = strtok(config, " \t\r\n");  // for each line, get the first token
        if (token == NULL ||  token[0] == '#') {
            continue;  // skip empty lines and comments
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
        exit(EXIT_FAILURE);
    }

    fclose(config_file);
    return values;
}

// Function to write to log file and to the screen
void log_writer(char* log_buffer) {
    // If the mutex is locked, the thread will wait until it is unlocked
    // Makes sure that only one thread can write to the log file at a time
    // Also guarantees that the log file will be written to in the correct order
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
    memset(log_buffer, 0, BUFFER_MESSAGE); // clears the message buffer
}

// Main function to initialize the system manager
void main_initializer() {
    // Creating shared memory
    shm = create_shm(config_vals.max_shmkeys, config_vals.max_alerts, config_vals.max_sensors);

    // Creating internal queue
    intqueue = create_queue(config_vals.queue_size);

    // Creating message queue
    key_t key = ftok(".", 'a');
    msgid = msgget(key, IPC_CREAT | 0666);

    // Creating unnamed pipes for workers
    pipes_fd = malloc(config_vals.nr_workers * sizeof(int *));
    create_unnamed_pipes(pipes_fd, config_vals.nr_workers);

    // Creating named pipes for sensor and console
    create_named_pipes();

    // Opening named pipes
    sensor_fd = open("SENSOR_PIPE", O_RDONLY | O_NONBLOCK);
    console_fd = open("CONSOLE_PIPE", O_RDONLY | O_NONBLOCK);

    // Creating workers and it's own shared memory
    worker_shm = create_worker_queue(config_vals.nr_workers);
    worker_shm = attach_worker_queue(worker_shm->shmid);
    create_workers(config_vals.nr_workers, shm->shmid, worker_shm->shmid, msgid);
    
    // Creating watcher
    create_watcher(shm->shmid, msgid);

    // Creating threads
    create_threads();
}

// Main function
int main(int argc, char *argv[]) {
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

    sprintf(log_buffer, "HOME_IOT SIMULATOR STARTING\n");
    log_writer(log_buffer);
    
    config_vals = config_loader(argv[1]);
    main_initializer();

    handle_signint(0);  // Exits
    return EXIT_SUCCESS;
}