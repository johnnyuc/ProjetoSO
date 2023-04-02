/**********************************************
* Author: António Silva e Johnny Fernandes    *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"

// Global variables
ConfigValues config_vals;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Main function
int main() {
    config_vals = config_loader(CONFIG_PATH);

    // DEBUGGING SHIT AND WHATNOT --------------------------------------
    printf("Internal queue size: %d\n", config_vals.queue_size);
    printf("Number of worker processes: %d\n", config_vals.nr_workers);
    printf("Max keys in SHM: %d\n", config_vals.max_shmkeys);
    printf("Max sensors: %d\n", config_vals.max_sensors);
    printf("Max alerts: %d\n", config_vals.max_alerts);
    return 0;
    // -----------------------------------------------------------------
    
}

// Config loader function
ConfigValues config_loader(const char* filepath) {
    ConfigValues values = {0};
    FILE* config_file = fopen(filepath, "r");
    if (config_file == NULL) {
        printf("Failed to open config file. System manager now exiting...\n");
        exit(1);
    }
    char line[BUFFER256];
    int filler = 0;
    while (fgets(line, sizeof(line), config_file)) {
        char* token = strtok(line, " \t\r\n");
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


// Logger function
void logger(const char* message) {
    // If the log file is being written to, wait until it's done
    // Mantains order of log messages, both printing to screen and writing to file
    pthread_mutex_lock(&log_mutex);
    
    // Log file writing
    FILE* log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        printf("Failed to open log file. System manager now exiting...\n");
        exit(1);
    }
    fprintf(log_file, "%s", message);
    fclose(log_file);

    // Also prints to screen
    printf("%s", message);

    // Unlock mutex
    pthread_mutex_unlock(&log_mutex);
}