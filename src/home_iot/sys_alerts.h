#ifndef IOT_PROJECT_ALERTS_WATCHER_H
#define IOT_PROJECT_ALERTS_WATCHER_H

// Includes
#include <stdlib.h> // Used for NULL
#include <unistd.h> // Used for fork
#include <sys/wait.h> // Used for wait
#include "sys_shm.h" // Used for SharedMemory

// Defines
#define FLOOD_LIMIT 32
#define FLOOD_TIME 30

// Structs
typedef struct flood_prevent {
    time_t timestamp;
    char id[BUFFER_MESSAGE];
} flood_prevent;

// Functions
int create_watcher(int shmid, int msgid);
int watcher_tasks(SharedMemory *shm, int msgid);

#endif //IOT_PROJECT_ALERTS_WATCHER_H