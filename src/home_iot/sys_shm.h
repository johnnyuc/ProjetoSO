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
#include <unistd.h>

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

typedef struct WorkerAvailability {
    int available;
} WorkerAvailability;

// Shared memory worker main struct
typedef struct WorkerSHM {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    // Array of worker ids
    WorkerAvailability *workerAvailability;
    // Max values to iterate
    int nr_workers;
    // Shared memory id
    int shmid;
    // Queue
    int front;
    int rear;
    int size;
} WorkerSHM;

// Connects
SharedMemory *create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo);
SharedMemory *attach_shm(int shmid);
WorkerSHM *create_worker_queue(int nr_workers);
WorkerSHM *attach_worker_queue(int shmid);

// Disconnects
void detach_shm(SharedMemory *sharedMemory);
void remove_shm(SharedMemory *sharedMemory);
void detach_worker_queue(WorkerSHM *worker_shm);
void remove_worker_queue(WorkerSHM *worker_shm);

// Writes, reads, removes
void print_shared_memory(SharedMemory *sharedMemory);
void print_worker_queue(WorkerSHM *worker_shm);
void enqueue_worker(WorkerSHM *worker_shm, int worker_id);
int dequeue_worker(WorkerSHM *worker_shm);

void insert_sensor_key(SharedMemory* sharedMemory, char* key, int lastValue, int minValue, int maxValue, double averageValue, int updateCount);
void insert_alert_key(SharedMemory* sharedMemory, char* key, float min, float max);
void remove_sensor_key(SharedMemory *sharedMemory, char *key);
void remove_alert_key(SharedMemory *sharedMemory, char *key);
void update_sensor_key(SharedMemory* sharedMemory, char* key, int new_value);
void update_alert_key(SharedMemory* sharedMemory, char* key, float new_min, float new_max);

#endif //IOT_PROJECT_SHM_H