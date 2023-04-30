/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sensor.h"

// Global variables for messages sent and the pipe file descriptor
int msgs_sent = 0;
int sensor_fd;

// Function to handle the SIGINT signal
void handle_sigint(int sig) {
    printf("SIGINT CODE: %d\n", sig);
    printf("PROCESS TERMINATED. SENT MESSAGES: %d\n", msgs_sent);
    close(sensor_fd);
    exit(EXIT_SUCCESS);
}

// Function to handle the SIGTSTP signal
void handle_sigtstp(int sig) {
    printf("SIGTSTP CODE: %d\n", sig);
    printf("SENT MESSAGES: %d\n", msgs_sent);
}

// Function to validate the format sensor_id and key
int alnum_validation(const char *str, int underscore) {
    for (int i = 0; str[i] != '\0'; i++) {
        // Check if the character is an underscore
        if (underscore && str[i] == '_') {
            continue;
        }
        // Check if the character is alphanumeric
        if (!isalnum(str[i])) {
            return 0;
        }
    }
    return 1;
}

// Function to generate data and send it to the server
void sensor_run(SensorArgs args) {
    // Generate random seed of rand()
    srand(time(NULL));

    // Send the message
    while (1) {
        // Generate random value
        int value = (rand() % (args.max_value - args.min_value + 1)) + args.min_value;
        char msg[BUFFER_MESSAGE];
        sprintf(msg, "%s#%s#%d", args.sensor_id, args.key, value);
        printf("SENDING: %s\n", msg);
        // Send the message
        int write_code = write(sensor_fd, msg, strlen(msg));
        if (write_code < 0) {
            if (errno == EPIPE) {  // pipe closed by server 
                printf("PIPE CLOSED. EXITING.\n");
                break;
            } else {
                printf("ERROR SENDING DATA. EXITING\n");
                break;
            }
        }
        msgs_sent++;
        printf("args.interval_secs: %f\n", args.interval_secs);
        // Cannot sleep 0 seconds - pipe saturation
        usleep(args.interval_secs * 1000000);
    }
    exit(EXIT_FAILURE);
}

// Main function to initialize the sensor
SensorArgs main_initializer(char* argv[]) {
    // Get arguments
    SensorArgs args = {0};
    args.sensor_id = argv[1];
    args.interval_secs = atoi(argv[2]);
    args.key = argv[3];
    args.min_value = atoi(argv[4]);
    args.max_value = atoi(argv[5]);
    
    // Verify if all arguments are valid
    if (strlen(args.sensor_id) < MIN_LEN || strlen(args.sensor_id) > MAX_LEN) {
        printf("SENSOR_ID LENGTH SHOULD BE BETWEEN 3 AND 32\n");
        exit(EXIT_FAILURE);
    }
    if (args.interval_secs < 0) {
        printf("INTERVAL_SECS (>=0)\n");
        exit(EXIT_FAILURE);
    }
    if (args.interval_secs == 0) {
        args.interval_secs = 0.25;
    }
    if (strlen(args.key) < MIN_LEN || strlen(args.key) > MAX_LEN) {
        printf("ALPHANUMERIC KEY LENGTH SHOULD BE BETWEEN 3 AND 32\n");
        exit(EXIT_FAILURE);
    }
    if (args.min_value >= args.max_value) {
        printf("MIN_VALUE > MAX_VALUE\n");
        exit(EXIT_FAILURE);
    }

    // Verify if the sensor_id and key is valid
    if (!alnum_validation(args.sensor_id, 0)) {
        printf("SENSOR_ID SHOULD ONLY CONTAIN ALPHANUMERIC CHARACTERS\n");
        exit(EXIT_FAILURE);
    }
    if (!alnum_validation(args.key, 1)) {
        printf("KEY SHOULD ONLY CONTAIN ALPHANUMERIC CHARACTERS AND UNDERSCORES\n");
        exit(EXIT_FAILURE);
    }

    // Opens named pipe for writing only
    sensor_fd = open(SENSOR_PIPE, O_WRONLY);
    if (sensor_fd < 0) {
        printf("ERROR ACCESSING PIPE\n");
        exit(EXIT_FAILURE);
    }

    return args;
}

int main(int argc, char* argv[]) {
    // Verify if the number of arguments is correct
    if (argc != 6) {
        printf("SYNTAX: %s {SENSOR_ID} {INTERVAL_SECS} {KEY} {MIN_VALUE} {MAX_VALUE}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Configs signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGPIPE, SIG_IGN); // on write() to a closed pipe

    // Initializes and runs the sensor
    SensorArgs running_args = main_initializer(argv);
    sensor_run(running_args);

    return 0;
}