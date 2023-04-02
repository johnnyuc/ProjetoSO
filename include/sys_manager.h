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
#include <pthread.h>

// Defines
#define BUFFER256 256
#define CONFIG_PATH "config/config.txt"

// Structs
typedef struct {
    int queue_size;
    int nr_workers;
    int max_shmkeys;
    int max_sensors;
    int max_alerts;
} ConfigValues;

// Functions 
ConfigValues config_loader(const char* filepath);
void log_writer(const char* message)

#endif //IOT_PROJECT_SYSTEM_MANAGER_H