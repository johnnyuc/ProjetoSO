/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#ifndef IOT_PROJECT_SENSOR_H
#define IOT_PROJECT_SENSOR_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

// Defines
#define SENSOR_PIPE "SENSOR_PIPE"
#define BUFFER_MESSAGE 256
#define MAX_LEN 32
#define MIN_LEN 3

// Structs
typedef struct {
    char* sensor_id;
    float interval_secs;
    char* key;
    int min_value;
    int max_value;
} SensorArgs;

#endif //IOT_PROJECT_SENSOR_H