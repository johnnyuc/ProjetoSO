// Author: Johnny Fernandes 2021190668
// LEI UC 2022-23 - Sistemas Operativos

#ifndef IOT_PROJECT_ALERTS_WATCHER_H
#define IOT_PROJECT_ALERTS_WATCHER_H


// Includes
#include <stdlib.h> // Used for NULL
#include <unistd.h> // Used for fork
#include <sys/wait.h> // Used for wait

// Functions
int create_watcher(int shmid);
int watcher_tasks();

#endif //IOT_PROJECT_ALERTS_WATCHER_H