/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#ifndef IOT_PROJECT_ALERTS_WATCHER_H
#define IOT_PROJECT_ALERTS_WATCHER_H

// Includes
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "sys_shm.h" // Needed for SharedMemory struct

// Defines
#define FLOOD_LIMIT 8
#define FLOOD_TIME 30

// Structs
typedef struct flood_prevent {
    time_t timestamp;
    char id[BUFFER_MESSAGE];
} flood_prevent;

// Functions
void print_buffer();
void create_watcher(int shmid, int msgid);
void watcher_tasks(SharedMemory *shm, int msgid);

#endif //IOT_PROJECT_ALERTS_WATCHER_H