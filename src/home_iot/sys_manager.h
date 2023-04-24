// Author: Johnny Fernandes 2021190668
// LEI UC 2022-23 - Sistemas Operativos

#ifndef IOT_PROJECT_SYS_MANAGER_H
#define IOT_PROJECT_SYS_MANAGER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

// Defines
#define BUFFER_MESSAGE 256
#define BUFFER_TIME 20
#define LOG_PATH "logs/home_iot.log"

// TODO: Recheck usage of extern
extern char message[BUFFER_MESSAGE];

// Structs
typedef struct ConfigValues {
    int queue_size;
    int nr_workers;
    int max_shmkeys;
    int max_sensors;
    int max_alerts;
} ConfigValues;

// Functions
ConfigValues config_loader(char* filepath);
void log_writer(char* message);
void main_initializer();

#endif //IOT_PROJECT_SYS_MANAGER_H