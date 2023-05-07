/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

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
#define FLOOD_TIME 30

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
    int console_id;
    char id[MAX_LEN];
    char key[MAX_LEN];
    double min;
    double max;
} AlertKeyInfo;

typedef struct Sensor {
    char id[MAX_LEN];
    char key[MAX_LEN];
} Sensor;

// Shared memory main struct
typedef struct SharedMemory {
    pthread_mutex_t mutex;
    pthread_cond_t alert;
    // Array of sensor keys and alert keys
    SensorKeyInfo* sensorKeyInfoArray;
    AlertKeyInfo* alertKeyInfoArray;
    // Array of sensor
    Sensor* sensorArray;
    int sensorCount;
    // Max values to iterate
    int maxSensorKeyInfo;
    int maxAlertKeyInfo;
    int maxSensors;
    // Shared memory id
    int shmid;
    // Anti flood
    time_t flood_buffer[3];
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
SharedMemory* create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo, int maxSensors);
SharedMemory* attach_shm(int shmid);
WorkerSHM* create_worker_queue(int nr_workers);
WorkerSHM* attach_worker_queue(int shmid);

// Disconnects
void detach_shm(SharedMemory* sharedMemory);
void remove_shm(SharedMemory* sharedMemory);
void detach_worker_queue(WorkerSHM* worker_shm);
void remove_worker_queue(WorkerSHM* worker_shm);

// Writes, reads, removes
// Worker queue
void enqueue_worker(WorkerSHM* worker_shm, int worker_id);
int dequeue_worker(WorkerSHM* worker_shm);

// Sensor keys
void insert_sensor_key(SharedMemory* sharedMemory, char* id, char* key, int lastValue);
void reset_sensor_data(SharedMemory* sharedMemory);

// Alert keys
int insert_alert_key(SharedMemory* sharedMemory, int console_id, char* id, char* key, float min, float max);
int remove_alert_key(SharedMemory* sharedMemory, char* key);

// Prints
void print_full_data(SharedMemory *sharedMemory, WorkerSHM *workerSHM);
#endif //IOT_PROJECT_SHM_H