/**********************************************
* Author: António Silva 2020238160            *
* Author: Johnny Fernandes 2021190668         *
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

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// TESTES PARA CARALHO

    // METE AS MERDAS EM TODO ESTE BLOCO

// Struct that represents the internal_queue
typedef struct {
        int read_pos, write_pos;
        int count;

        int internal_buffer[];
} InternalQueue;

// Function that initializes internal_queue
InternalQueue* queue_init() {
    InternalQueue* q = malloc(sizeof(InternalQueue)+config_vals.queue_size*sizeof(int));
    q->read_pos = 0;
    q->write_pos = 0;
    q->count = 0;
    return q;
}

// Function that removes internal_queue
void remove_queue(InternalQueue* q) {
    free(q);
}


void queue_push(InternalQueue* q, int value) {
    pthread_mutex_lock(&queue_mutex);
    while (q->count == config_vals.queue_size) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    q->internal_buffer[q->write_pos] = value;
    q->write_pos = (q->write_pos + 1) % config_vals.queue_size;
    q->count++;
    pthread_mutex_unlock(&queue_mutex);
}

void queue_pop(InternalQueue* q) {
    pthread_mutex_lock(&queue_mutex);
    while (q->count == 0) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    q->read_pos = (q->read_pos + 1) % config_vals.queue_size;
    q->count--;
    pthread_mutex_unlock(&queue_mutex);
}

void queue_print(InternalQueue* q) {
    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < q->count; i++) {
        printf("%d ", q->internal_buffer[(q->read_pos + i) % config_vals.queue_size]);
    }
    printf("\n");
    pthread_mutex_unlock(&queue_mutex);
}

void main_initializer() {
    // Initialize the internal_queue
    InternalQueue* internal_queue = queue_init();
    // Cenas
    // Agora vamos testar aqui se a queue funciona ou o caralho

    // Push 10 values to the queue
    for (int i = 0; i < 10; i++) {
        queue_push(internal_queue, i);
    }

    // Print the queue
    queue_print(internal_queue);

    // Pop 5 values from the queue
    for (int i = 0; i < 5; i++) {
        queue_pop(internal_queue);
    }

    // Print the queue
    queue_print(internal_queue);

    // Remove the internal_queue, should be called somewhere else
    free(internal_queue);
    
}
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------


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