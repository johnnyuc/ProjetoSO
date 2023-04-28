/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];

// Function to create and initialize shared memory
SharedMemory* create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo) {
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666 | IPC_CREAT);
    if (shmid == -1) {
        sprintf(log_buffer, "SHM: ERROR CREATING SHARED MEMORY\n");
        log_writer(log_buffer);
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    sprintf(log_buffer, "SHARED MEMORY %d CREATED\n", shmid);
    log_writer(log_buffer);

    // Object to be shared
    SharedMemory *sharedMemory = (SharedMemory *)shmat(shmid, NULL, 0);
    if (sharedMemory == (void *)-1) {
        sprintf(log_buffer, "SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory with calloc as the only possible way to get a dinamically sized array
    // Calloc also initializes the memory to 0, it's safer
    sharedMemory->sensorKeyInfoArray = (SensorKeyInfo *)calloc(maxSensorKeyInfo, sizeof(SensorKeyInfo));
    if (sharedMemory->sensorKeyInfoArray == NULL) {
        sprintf(log_buffer, "SHM: ERROR ALLOCATING SENSOR_KEY_INFO_ARRAY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
    sharedMemory->alertKeyInfoArray = (AlertKeyInfo *)calloc(maxAlertKeyInfo, sizeof(AlertKeyInfo));
    if (sharedMemory->alertKeyInfoArray == NULL) {
        sprintf(log_buffer, "SHM: ERROR ALLOCATING ALERT_KEY_INFO_ARRAY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    sharedMemory->maxSensorKeyInfo = maxSensorKeyInfo;
    sharedMemory->maxAlertKeyInfo = maxAlertKeyInfo;
    sharedMemory->shmid = shmid;

    sprintf(log_buffer, "SHARED MEMORY %d FULLY INITIALIZED\n", shmid);
    log_writer(log_buffer);

    return sharedMemory;
}

// Function to attach shared memory
SharedMemory *attach_shm(int shmid) {
    SharedMemory *sharedMemory = (SharedMemory *)shmat(shmid, NULL, 0);
    if (sharedMemory == (void *)-1) {
        sprintf(log_buffer, "SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
    return sharedMemory;
}

// Function to detach shared memory
void detach_shm(SharedMemory *sharedMemory) {
    if (shmdt(sharedMemory) == -1) {
        sprintf(log_buffer, "SHM: ERROR DETACHING SHARED MEMORY %d\n", sharedMemory->shmid);
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
}

// Function to free shared memory and remove it
void remove_shm(SharedMemory *sharedMemory) {
    int storedShmid = sharedMemory->shmid; // Otherwise unaccessible after detach

    // Free allocated memory
    free(sharedMemory->sensorKeyInfoArray);
    free(sharedMemory->alertKeyInfoArray);

    // Detach shared memory
    detach_shm(sharedMemory);

    // Remove shared memory
    if (shmctl(storedShmid, IPC_RMID, NULL) == -1) {
        sprintf(log_buffer, "SHM: ERROR REMOVING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
}

// Missing functions to read, write, remove, update and search for sensor and alert key info