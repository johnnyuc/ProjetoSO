// Author: Johnny Fernandes 2021190668
// LEI UC 2022-23 - Sistemas Operativos

#ifndef IOT_PROJECT_SHM_H
#define IOT_PROJECT_SHM_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

// Defines
#define MAX_LEN 32

// Structs
typedef struct SensorKeyInfo {
    char key[MAX_LEN];
    int lastValue;
    int minValue;
    int maxValue;
    double averageValue;
    int updateCount;
} SensorKeyInfo;

typedef struct AlertKeyInfo {
    char key[MAX_LEN];
    float min;
    float max;
} AlertKeyInfo;

// Shared memory main struct
typedef struct SharedMemory {
    pthread_mutex_t mutex;
    // Array of sensor keys and alert keys
    SensorKeyInfo *sensorKeyInfoArray;
    AlertKeyInfo *alertKeyInfoArray;
    // Max values to iterate
    int maxSensorKeyInfo;
    int maxAlertKeyInfo;
    // Shared memory id
    int shmid;
} SharedMemory;

// Connects
SharedMemory *create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo);
SharedMemory *attach_shm(int shmid);

// Disconnects
void detach_shm(SharedMemory *sharedMemory);
void remove_shm(SharedMemory *sharedMemory);

// Writes, reads, removes

#endif //IOT_PROJECT_SHM_H