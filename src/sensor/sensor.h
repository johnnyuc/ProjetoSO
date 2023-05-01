#ifndef IOT_PROJECT_SENSOR_H
#define IOT_PROJECT_SENSOR_H

// Includes
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strlen, strcmp, strcpy, strtok etc
#include <ctype.h> // isalnum, isdigit, toupper
#include <signal.h> // signal
#include <unistd.h> // open, close, read, write
#include <fcntl.h> // O_RDONLY O_WRONLY
#include <errno.h> // errno
#include <time.h> // time

// Defines
#define SENSOR_PIPE "SENSOR_PIPE"
#define BUFFER_MESSAGE 256
#define MIN_LEN 3
#define MAX_LEN 32

// Structs
typedef struct {
    char* sensor_id;
    float interval_secs;
    char* key;
    int min_value;
    int max_value;
} SensorArgs;

#endif //IOT_PROJECT_SENSOR_H