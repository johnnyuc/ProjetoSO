/**********************************************
* Author: António Silva e Johnny Fernandes    *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#ifndef IOT_PROJECT_SYSTEM_MANAGER_H
#define IOT_PROJECT_SYSTEM_MANAGER_H

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <sys/stat.h> // mkfifo()
#include <fcntl.h> // O_CREAT, O_EXCL
#include <errno.h>

// Defines
#define BUFFER_MESSAGE 256
#define BUFFER_TIME 20
#define SENSOR_PIPE "SENSOR_PIPE"
#define CONSOLE_PIPE "CONSOLE_PIPE"
#define LOG_PATH "logs/log_sys_manager.txt"

// Structs
typedef struct {
    int queue_size;
    int nr_workers;
    int max_shmkeys;
    int max_sensors;
    int max_alerts;
} ConfigValues;

// Functions 
ConfigValues config_loader(char* filepath);
void log_writer(char* message);

void *sensor_reader(void *arg);
void *console_reader(void *arg);

#endif //IOT_PROJECT_SYSTEM_MANAGER_H