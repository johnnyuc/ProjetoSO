/**********************************************
* Author: António Silva e Johnny Fernandes    *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"

// Global variables
pthread_mutex_t log_writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

char message[BUFFER_MESSAGE];
ConfigValues config_vals;


// Main function
int main(int argc, char *argv[]) {
    // Check if the correct number of arguments was passed
    if (argc != 2) {
        char message[BUFFER_MESSAGE];
        sprintf(message, "Invalid set of parameters. System manager now exiting...\n");
        log_writer(message);
        exit(1);
    }

    config_vals = config_loader(argv[1]);

    // DEBUGGING SHIT AND WHATNOT --------------------------------------
    sprintf(message, "Internal queue size: %d\n", config_vals.queue_size);
    log_writer(message);
    sprintf(message, "Number of worker processes: %d\n", config_vals.nr_workers);
    log_writer(message);
    sprintf(message, "Max keys in SHM: %d\n", config_vals.max_shmkeys);
    log_writer(message);
    sprintf(message, "Max sensors: %d\n", config_vals.max_sensors);
    log_writer(message);
    sprintf(message, "Max alerts: %d\n", config_vals.max_alerts);
    log_writer(message);
    // -----------------------------------------------------------------

    main_initializer();
    return 0;
}

// Config loader function
ConfigValues config_loader(char* filepath) {
    ConfigValues values = {0};
    FILE* config_file = fopen(filepath, "r");
    if (config_file == NULL) {
        printf("Failed to open config file. System manager now exiting...\n");
        exit(1);
    }
    char line[BUFFER_MESSAGE];
    int filler = 0;
    while (fgets(line, sizeof(line), config_file)) {
        char* token = strtok(line, " \t\r\n");  // for any split token, use strtok
        if (token == NULL) {
            continue;  // skip empty lines, comments, etc.
        }
        int value = atoi(token);

        // Incrementally fill the struct
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
        filler++;
    }

    // Check if all values were filled
    if (filler != 5) {
        printf("Invalid config file. System manager now exiting...\n");
        exit(1);
    }

    fclose(config_file);
    return values;
}

void main_initializer() {

    // Create shared memory
    

    // Create threads
    pthread_t console_reader_thread, sensor_reader_thread, dispatcher_thread;
    int rc;

    // create console reader thread
    rc = pthread_create(&console_reader_thread, NULL, console_reader, NULL);
    if (rc != 0) {
        fprintf(stderr, "Error creating console reader thread: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    // create sensor reader thread
    rc = pthread_create(&sensor_reader_thread, NULL, sensor_reader, NULL);
    if (rc != 0) {
        fprintf(stderr, "Error creating sensor reader thread: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    // create dispatcher thread
    rc = pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);
    if (rc != 0) {
        fprintf(stderr, "Error creating dispatcher thread: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    // wait for threads to finish
    pthread_join(console_reader_thread, NULL);
    pthread_join(sensor_reader_thread, NULL);
    pthread_join(dispatcher_thread, NULL);

    // Create worker processes
    for (int i = 0; i < config_vals.nr_workers; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            worker();
            exit(0);
        } else if (pid < 0) {
            // Error
            sprintf(message, "Failed to create worker process. System manager now exiting...\n");
            log_writer(message);
            exit(1);
        }
    }

    // // Create SENSOR_PIPE, permissions set at 666, write on sensor
    // if ((mkfifo(SENSOR_PIPE, O_CREAT|O_EXCL|0666)<0) && (errno!= EEXIST)) {
    //     perror("Cannot create pipe: ");
    //     exit(0);
    // }
    
    // // Creating CONSOLE_PIPE, permissions set at 666, write on user_console
    // if ((mkfifo(CONSOLE_PIPE, O_CREAT|O_EXCL|0666)<0) && (errno!= EEXIST)) {
    //     perror("Cannot create pipe: ");
    //     exit(0);
    // }

    

}

// Logger function
void log_writer(char* message) {
    // If the log file is being written to, wait until it's done
    // Maintains order of log messages, both printing to screen and writing to file
    pthread_mutex_lock(&log_writer_mutex);

    // Get the current time
    // https://en.cppreference.com/w/c/chrono/strftime
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    // Format the time as a string
    char timestamp[BUFFER_TIME];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", tm_info);

    // Write the log message to file
    FILE* log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        printf("LOG ERROR: Could not write to file!\n");
        return;
    }
    fprintf(log_file, "[%s] %s", timestamp, message);
    fclose(log_file);

    // Write the log message to screen
    printf("[%s] %s", timestamp, message);

    // Unlock the mutex
    pthread_mutex_unlock(&log_writer_mutex);

    memset(message, 0, BUFFER_MESSAGE); // clears the message buffer
}


// CENAS

void *sensor_reader(void *arg) {
    int fd;
    char buffer[BUFFER_MESSAGE];
    int bytes_read;

    fd = open(SENSOR_PIPE, O_RDONLY);

    while(1) {
        bytes_read = read(fd, buffer, BUFFER_MESSAGE);

        if (bytes_read > 0) {
            // Lock the queue_mutex to access the shared queue
            pthread_mutex_lock(&queue_mutex);

            // Add the message to the shared queue
            enqueue(&queue, buffer);

            // Unlock the queue_mutex to allow other threads to access the queue
            pthread_mutex_unlock(&queue_mutex);
        }
    }

    close(fd);
    pthread_exit(NULL);
}

void *console_reader(void *arg) {
    int fd;
    char buffer[BUFFER_MESSAGE];
    int bytes_read;

    fd = open(CONSOLE_PIPE, O_RDONLY);

    while(1) {
        bytes_read = read(fd, buffer, BUFFER_MESSAGE);

        if (bytes_read > 0) {
            // Lock the queue_mutex to access the shared queue
            pthread_mutex_lock(&queue_mutex);

            // Add the message to the shared queue
            enqueue(&queue, buffer);

            // Unlock the queue_mutex to allow other threads to access the queue
            pthread_mutex_unlock(&queue_mutex);
        }
    }

    close(fd);
    pthread_exit(NULL);
}

void* dispatcher(void* arg) {
    int message;

    while (1) {
        // wait for queue to not be empty
        pthread_mutex_lock(&queue_mutex);
        while (config_vals.queue_size == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_mutex);
        }

        // remove message from queue
        message = internal_queue[config_vals.queue_size-1];
        config_vals.queue_size--;

        pthread_mutex_unlock(&queue_mutex);

        // dispatch message to worker processes
        // ...
    }
}