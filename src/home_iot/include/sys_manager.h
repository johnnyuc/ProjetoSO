/**********************************************
* Author: António Silva 2020238160            *
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#ifndef IOT_PROJECT_SYSTEM_MANAGER_H
#define IOT_PROJECT_SYSTEM_MANAGER_H

// Includes
#include <stdlib.h>
#include <stdio.h> // File manipulation, printf
#include <string.h> // String manipulation, strtok
#include <time.h> // Timestamp

#include <pthread.h> // Threads, mutexes and condition variables
#include <sys/shm.h> // Shared memory
// #include <sys/stat.h> // File permissions
// #include <sys/types.h> // File permissions
// #include <sys/wait.h> // Waitpid
// #include <unistd.h> // Fork, pipe, close, dup, exec, sleep
// #include <fcntl.h> // File manipulation
// #include <errno.h> // Error handling
#include <sys/ipc.h> // Message queue

// Defines
#define BUFFER_MESSAGE 256
#define BUFFER_TIME 20
#define SENSOR_PIPE "SENSOR_PIPE"
#define CONSOLE_PIPE "CONSOLE_PIPE"
#define LOG_PATH "logs/log_sys_manager.txt"

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
void main_initializer();
void log_writer(char* message);

#endif //IOT_PROJECT_SYSTEM_MANAGER_H