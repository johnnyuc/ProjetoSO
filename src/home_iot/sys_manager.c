/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"
#include "sys_threads.h"
#include "sys_workers.h"
#include "sys_alerts.h"
#include "sys_shm.h"

// Global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
ConfigValues config_vals;
SharedMemory *SHM;

// Function to handle the SIGINT signal
void handle_signint(int sig) {
    // Tricky part:
    // Careful about cleaning up resources like waiting for all threads to finish
    // processes to finish, detaching access to shm, free calloc and then exit and write to log
    remove_shm(SHM);
    sprintf(message, "EXITING, CODE %d\n", sig);
    log_writer(message);
    pthread_mutex_destroy(&mutex);
    exit(EXIT_SUCCESS);
}

// Loading config file values
ConfigValues config_loader(char* filepath) {
    ConfigValues values = {0};
    FILE* config_file = fopen(filepath, "r");

    if (config_file == NULL) {
        sprintf(message, "FAILED TO OPEN CONFIG FILE. EXITING\n");
        log_writer(message);
        exit(EXIT_FAILURE);
    }
    
    int param_count = 0;
    while (fgets(message, sizeof(message), config_file)) {
        char* token = strtok(message, " \t\r\n");  // for each line, get the first token
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
        sprintf(message, "INCORRECT CONFIG FILE PARAMS. EXITING\n");
        log_writer(message);
        exit(EXIT_FAILURE);
    }

    fclose(config_file);
    return values;
}

// Function to write to log file and to the screen
void log_writer(char* message) {
    // If the mutex is locked, the thread will wait until it is unlocked
    // Makes sure that only one thread can write to the log file at a time
    // Also guarantees that the log file will be written to in the correct order
    pthread_mutex_lock(&mutex);

    // Timestamp format: dd/mm/yy hh:mm:ss
    // https://en.cppreference.com/w/c/chrono/strftime
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    // Formats the timestamp
    char timestamp[BUFFER_TIME];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", tm_info);

    // Writes the message to the screen
    printf("[%s] %s", timestamp, message);

    // Writes the message to the log file
    FILE* log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        printf("LOG ERROR: CANNOT WRITE TO LOG\n");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "[%s] %s", timestamp, message);
    fclose(log_file);

    // Unlocks the mutex
    pthread_mutex_unlock(&mutex);
    memset(message, 0, BUFFER_MESSAGE); // clears the message buffer
}

// Main function to initialize the system manager
void main_initializer() {
    create_threads();
    SHM = create_shm(config_vals.max_shmkeys, config_vals.max_alerts);
    create_workers(config_vals.nr_workers, SHM->shmid);
    create_watcher(SHM->shmid);
}

// Main function
int main(int argc, char *argv[]) {
    char message[BUFFER_MESSAGE];
    // Verifies if the config file path was passed as a parameter
    if (argc != 2) {
        sprintf(message, "INVALID CONFIG ARGUMENT ON START. EXITING\n");
        log_writer(message);
        exit(EXIT_FAILURE);
    }

    sprintf(message, "HOME_IOT SIMULATOR STARTING\n");
    log_writer(message);
    
    config_vals = config_loader(argv[1]);
    main_initializer();

    handle_signint(0);
    return EXIT_SUCCESS;
}