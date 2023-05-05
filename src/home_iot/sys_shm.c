#include "sys_manager.h"
#include "sys_shm.h"

#include <errno.h>

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];

// Function to create shared memory
SharedMemory* create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo, int maxSensors) {
    //size_t shmsize = sizeof(SharedMemory) + (sizeof(SensorKeyInfo) * maxSensorKeyInfo) + (sizeof(AlertKeyInfo) * maxAlertKeyInfo);
    size_t shmsize = sizeof(SharedMemory) + (sizeof(SensorKeyInfo) * maxSensorKeyInfo) + (sizeof(AlertKeyInfo) * maxAlertKeyInfo) + (sizeof(Sensor) * maxSensors);
    int shmid = shmget(IPC_PRIVATE, shmsize, 0666 | IPC_CREAT);
    
    sprintf(log_buffer, "SHARED MEMORY %d CREATED\n", shmid);
    log_writer(log_buffer);

    // Object to be shared
    SharedMemory *sharedMemory = (SharedMemory *) shmat(shmid, NULL, 0);

    // Initialize shared memory
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_setpshared(&attrs, PTHREAD_PROCESS_SHARED);

    pthread_condattr_t alert_attrs;
    pthread_condattr_init(&alert_attrs);
    pthread_condattr_setpshared(&alert_attrs, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&sharedMemory->mutex, &attrs);
    pthread_cond_init(&sharedMemory->alert, &alert_attrs);
    sharedMemory->maxSensors = maxSensors;
    sharedMemory->sensorCount = 0;
    sharedMemory->maxSensorKeyInfo = maxSensorKeyInfo;
    sharedMemory->maxAlertKeyInfo = maxAlertKeyInfo;
    sharedMemory->shmid = shmid;
    
    // Allocate memory for sensor key info array directly in shared memory
    SensorKeyInfo* sensorKeyInfoArray = (SensorKeyInfo*)((char*)sharedMemory + sizeof(SharedMemory));
        for (int i = 0; i < maxSensorKeyInfo; i++) {
        // Initialize nested char array in struct
        for (int j = 0; j < MAX_LEN; j++) {
            sensorKeyInfoArray[i].key[j] = '\0';
        }
    }
    sharedMemory->sensorKeyInfoArray = sensorKeyInfoArray;
    
    // Allocate memory for alert key info array directly in shared memory
    AlertKeyInfo* alertKeyInfoArray = (AlertKeyInfo*)((char*)sharedMemory + sizeof(SharedMemory) + (sizeof(SensorKeyInfo) * maxSensorKeyInfo));
    for (int i = 0; i < maxAlertKeyInfo; i++) {
        // Initialize nested char array in struct
        for (int j = 0; j < MAX_LEN; j++) {
            alertKeyInfoArray[i].key[j] = '\0';
        }
    }
    sharedMemory->alertKeyInfoArray = alertKeyInfoArray;

    // Allocate memory for strings array directly in shared memory
    Sensor *sensorArray = (Sensor*)((char*)sharedMemory + sizeof(SharedMemory) + (sizeof(SensorKeyInfo) * maxSensorKeyInfo) + (sizeof(AlertKeyInfo) * maxAlertKeyInfo));
    for (int i = 0; i < maxSensors; i++) {
        // Initialize nested char array in struct
        for (int j = 0; j < MAX_LEN; j++) {
            sensorArray[i].id[j] = '\0';
            sensorArray[i].key[j] = '\0';
        }
    }
    sharedMemory->sensorArray = sensorArray;

    sprintf(log_buffer, "SHARED MEMORY %d FULLY INITIALIZED\n", shmid);
    log_writer(log_buffer);

    return sharedMemory;
}

// Function to attach shared memory
SharedMemory *attach_shm(int shmid) {
    SharedMemory *sharedMemory = (SharedMemory *)shmat(shmid, NULL, 0);
    if (sharedMemory == (void *)-1) {
        sprintf(log_buffer, "WORKER SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    } else {
        sprintf(log_buffer, "WORKER %d ATTACHED TO SHARED MEMORY %d\n", getpid(), shmid);
        log_writer(log_buffer);
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

    // Detach shared memory
    detach_shm(sharedMemory);

    // Remove shared memory{
    if (shmctl(storedShmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        sprintf(log_buffer, "SHM: ERROR REMOVING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    } else {
        sprintf(log_buffer, "SHARED MEMORY %d REMOVED\n", storedShmid);
        log_writer(log_buffer);
    }
}

void print_shared_memory(SharedMemory *sharedMemory) {
    pthread_mutex_lock(&sharedMemory->mutex);
    
    printf("SENSOR KEY INFO:\n");
    for (int i = 0; i < sharedMemory->maxSensorKeyInfo; i++) {
        //if (strcmp(sharedMemory->sensorKeyInfoArray[i].key, "") == 0) continue;
        printf("  Key: %s, Last Value: %d, Min: %d, Max: %d, Av.: %.2f, Count: %d\n",
            sharedMemory->sensorKeyInfoArray[i].key,
            sharedMemory->sensorKeyInfoArray[i].lastValue,
            sharedMemory->sensorKeyInfoArray[i].minValue,
            sharedMemory->sensorKeyInfoArray[i].maxValue,
            sharedMemory->sensorKeyInfoArray[i].averageValue,
            sharedMemory->sensorKeyInfoArray[i].updateCount);
    }
    
    printf("ALERT KEY INFO:\n");
    for (int i = 0; i < sharedMemory->maxAlertKeyInfo; i++) {
        //if (strcmp(sharedMemory->alertKeyInfoArray[i].key, "") == 0) continue;
        printf("  Key: %s, Min: %.2f, Max: %.2f\n",
            sharedMemory->alertKeyInfoArray[i].key,
            sharedMemory->alertKeyInfoArray[i].min,
            sharedMemory->alertKeyInfoArray[i].max);
    }

    printf("SENSORS:\n");
    for (int i = 0; i < sharedMemory->maxSensors; i++) {
        //if (strcmp(sharedMemory->sensors[i], "") == 0) continue;
        printf("  ID: %s, Key: %s\n",
            sharedMemory->sensorArray[i].id,
            sharedMemory->sensorArray[i].key);
    }
    
    pthread_mutex_unlock(&sharedMemory->mutex);
}

// Missing functions to read, write, remove, update and search for sensor and alert key info
int insert_sensor_key(SharedMemory* sharedMemory, char* id, char* key, int lastValue) {
    // Locks mutex
    pthread_mutex_lock(&sharedMemory->mutex);
    //printf("INSERTING DATA: %s, %s, %d\n", id, key, lastValue);

    // Verify if sensor exists in sensorArray
    int sensor = 0;
    int found = 0;
    while (sensor < sharedMemory->sensorCount) {
        if (strcmp(sharedMemory->sensorArray[sensor].id, id) == 0) {
            found = 1;
            if (strcmp(sharedMemory->sensorArray[sensor].key, key) != 0) {
                // Sensor already exists but key is different, reject
                pthread_mutex_unlock(&sharedMemory->mutex);
                return 1;
            }
            break;
        }
        sensor++;
    }

    if (found == 0 && sensor == sharedMemory->maxSensors) {
        // Sensor array is full, reject
        pthread_mutex_unlock(&sharedMemory->mutex);
        return 2;
    }

    if (found == 0) {
        // Adding sensor name to sensors array
        strcpy(sharedMemory->sensorArray[sharedMemory->sensorCount].key, key);
        strcpy(sharedMemory->sensorArray[sharedMemory->sensorCount].id, id);
        sharedMemory->sensorCount++;
    }

    // Check if sensor key already exists
    int insert = 0;
    while (insert < sharedMemory->maxSensorKeyInfo && sharedMemory->sensorKeyInfoArray[insert].key[0] != '\0') {
        if (strcmp(sharedMemory->sensorKeyInfoArray[insert].key, key) == 0) {
            // A sensor key with the same name already exists, so proceed to update it
            sharedMemory->sensorKeyInfoArray[insert].lastValue = lastValue;
            sharedMemory->sensorKeyInfoArray[insert].averageValue = (sharedMemory->sensorKeyInfoArray[insert].averageValue*
            sharedMemory->sensorKeyInfoArray[insert].updateCount + lastValue) / (sharedMemory->sensorKeyInfoArray[insert].updateCount+1);
            sharedMemory->sensorKeyInfoArray[insert].updateCount++;
            if (lastValue < sharedMemory->sensorKeyInfoArray[insert].minValue) sharedMemory->sensorKeyInfoArray[insert].minValue = lastValue;
            if (lastValue > sharedMemory->sensorKeyInfoArray[insert].maxValue) sharedMemory->sensorKeyInfoArray[insert].maxValue = lastValue;
            
            // Search all alert keys to see if this sensor key is in any of them
            for (int alert = 0; alert < sharedMemory->maxAlertKeyInfo; alert++) {
                if (strcmp(sharedMemory->alertKeyInfoArray[alert].key, key) == 0) {
                    // Found match, send signal to watcher process
                    pthread_cond_broadcast(&sharedMemory->alert);
                }
            }
            pthread_mutex_unlock(&sharedMemory->mutex);
            return 0;
        }
        insert++;
    }

    if (insert == sharedMemory->maxSensorKeyInfo) {
        // Array is full
        pthread_mutex_unlock(&sharedMemory->mutex);
        return 3;
    }

    // Insert new sensor key
    strcpy(sharedMemory->sensorKeyInfoArray[insert].key, key);
    sharedMemory->sensorKeyInfoArray[insert].lastValue = lastValue;
    sharedMemory->sensorKeyInfoArray[insert].minValue = lastValue;
    sharedMemory->sensorKeyInfoArray[insert].maxValue = lastValue;
    sharedMemory->sensorKeyInfoArray[insert].averageValue = lastValue;
    sharedMemory->sensorKeyInfoArray[insert].updateCount++;

    pthread_mutex_unlock(&sharedMemory->mutex);
    return 0;
}

// Remove a SensorKeyInfo com a chave especificada da estrutura SharedMemory
int reset_sensor_data(SharedMemory *sharedMemory) {
    // Locks mutex
    pthread_mutex_lock(&sharedMemory->mutex);

    // Reset sensor key info
    for (int i = 0; i < sharedMemory->maxSensorKeyInfo; i++) {
        sharedMemory->sensorKeyInfoArray[i].key[0] = '\0';
        sharedMemory->sensorKeyInfoArray[i].lastValue = 0;
        sharedMemory->sensorKeyInfoArray[i].minValue = 0;
        sharedMemory->sensorKeyInfoArray[i].maxValue = 0;
        sharedMemory->sensorKeyInfoArray[i].averageValue = 0;
        sharedMemory->sensorKeyInfoArray[i].updateCount = 0;
    }

    // Reset sensors array
    for (int i = 0; i < sharedMemory->sensorCount; i++) {
        sharedMemory->sensorArray[i].key[0] = '\0';
        sharedMemory->sensorArray[i].id[0] = '\0';
    }
    sharedMemory->sensorCount = 0;

    pthread_mutex_unlock(&sharedMemory->mutex);
    return 0;
}

// Function to insert a new alert key - it doesn't update
int insert_alert_key(SharedMemory *sharedMemory, int console_id, char *id, char *key, float min, float max) {
    // Locks mutex
    pthread_mutex_lock(&sharedMemory->mutex);

    // Checks if key already exists
    int i = 0;
    while (i < sharedMemory->maxAlertKeyInfo && strcmp(sharedMemory->alertKeyInfoArray[i].key, "") != 0) {
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            pthread_mutex_unlock(&sharedMemory->mutex);
            return 1;
        }
        i++;
    }

    // Check if key is not present in Sensor array
    int j = 0;
    while (j < sharedMemory->sensorCount) {
        if (strcmp(sharedMemory->sensorArray[j].key, key) == 0) {
            break;
        }
        j++;
    }

    // If it's no present, quits
    if (j == sharedMemory->sensorCount) {
        pthread_mutex_unlock(&sharedMemory->mutex);
        return 2;
    }

    // Check if it's full
    if (i == sharedMemory->maxAlertKeyInfo) {
        pthread_mutex_unlock(&sharedMemory->mutex);
        return 3;
    }

    // Insert new alert key
    strcpy(sharedMemory->alertKeyInfoArray[i].id, id);
    strcpy(sharedMemory->alertKeyInfoArray[i].key, key);
    sharedMemory->alertKeyInfoArray[i].console_id = console_id;
    sharedMemory->alertKeyInfoArray[i].min = min;
    sharedMemory->alertKeyInfoArray[i].max = max;

    pthread_mutex_unlock(&sharedMemory->mutex);
    return 0;
}

// Function to remove an alert key
int remove_alert_key(SharedMemory *sharedMemory, char *id) {
    // Locks mutex
    pthread_mutex_lock(&sharedMemory->mutex);

    // Checks if key exists
    int i = 0;
    while (i < sharedMemory->maxAlertKeyInfo && strcmp(sharedMemory->alertKeyInfoArray[i].id, id) != 0) {
        i++;
    }

    // Check if the key wasn't found
    if (i == sharedMemory->maxAlertKeyInfo) {
        pthread_mutex_unlock(&sharedMemory->mutex);
        return 1;
    }

    // Remove alert key
    strcpy(sharedMemory->alertKeyInfoArray[i].id, "");
    strcpy(sharedMemory->alertKeyInfoArray[i].key, "");
    sharedMemory->alertKeyInfoArray[i].console_id = 0;
    sharedMemory->alertKeyInfoArray[i].min = 0;
    sharedMemory->alertKeyInfoArray[i].max = 0;

    pthread_mutex_unlock(&sharedMemory->mutex);
    return 0;
}

WorkerSHM *create_worker_queue(int nr_workers) {
    // Create the shared memory segment
    size_t shmsize = sizeof(WorkerSHM) + (sizeof(WorkerAvailability) * nr_workers);
    int shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0666);
    if (shmid == -1) {
        sprintf(log_buffer, "WORKER SHM: ERROR CREATING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    sprintf(log_buffer, "WORKER SHARED MEMORY %d CREATED\n", shmid);
    log_writer(log_buffer);

    // Attach the shared memory segment
    WorkerSHM *worker_shm = (WorkerSHM *) shmat(shmid, NULL, 0);
    if (worker_shm == (void *) -1) {
        sprintf(log_buffer, "WORKER SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex and condition variable
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_setpshared(&attrs, PTHREAD_PROCESS_SHARED);

    pthread_condattr_t cond_attrs;
    pthread_condattr_init(&cond_attrs);
    pthread_condattr_setpshared(&cond_attrs,PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&worker_shm->mutex, &attrs);
    pthread_cond_init(&worker_shm->cond, &cond_attrs);
    worker_shm->nr_workers = nr_workers;
    worker_shm->front = worker_shm->rear = -1;
    worker_shm->size = 0;
    worker_shm->shmid = shmid;

    // Initialize the queue
    WorkerAvailability *workerAvailability = (WorkerAvailability *) ((char*)worker_shm + sizeof(WorkerSHM));
    worker_shm->workerAvailability = workerAvailability;

    // Preloading workers queue
    for (int i = 0; i < nr_workers; i++){
        enqueue_worker(worker_shm, i);
    }

    return worker_shm;
}

// Function to attach shared memory
WorkerSHM *attach_worker_queue(int shmid) {
    WorkerSHM *worker_shm = (WorkerSHM *)shmat(shmid, NULL, 0);
    if (worker_shm == (void *)-1) {
        sprintf(log_buffer, "SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    } else {
        sprintf(log_buffer, "WORKER %d ATTACHED TO QUEUE %d\n", getpid(), shmid);
        log_writer(log_buffer);
    }
    return worker_shm;
}

void detach_worker_queue(WorkerSHM *worker_shm) {
    if (shmdt(worker_shm) == -1) {
        sprintf(log_buffer, "SHM: ERROR DETACHING SHARED MEMORY %d\n", worker_shm->shmid);
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
}

void remove_worker_queue(WorkerSHM *worker_shm) {
    int storedShmid = worker_shm->shmid; // Otherwise unaccessible after detach

    // Detach shared memory
    detach_worker_queue(worker_shm);

    // Remove shared memory
    if (shmctl(storedShmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        sprintf(log_buffer, "SHM: ERROR REMOVING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    } else {
        sprintf(log_buffer, "SHARED MEMORY %d REMOVED\n", storedShmid);
        log_writer(log_buffer);
    }
}

void print_worker_queue(WorkerSHM *worker_shm) {
    // Acquire the mutex before accessing the shared memory
    pthread_mutex_lock(&worker_shm->mutex);

    for(int i = 0; i < worker_shm->size; i++) {
        // Calculate the index of the current element in the queue
        int index = (worker_shm->front + i) % worker_shm->nr_workers;
        
        // Print the worker ID
        printf("%d ", worker_shm->workerAvailability[index].available);
    }
    printf("\n");
    
    pthread_mutex_unlock(&worker_shm->mutex);
}

void enqueue_worker(WorkerSHM *worker_shm, int worker_id) {
    // Acquire the mutex before modifying the shared memory
    pthread_mutex_lock(&worker_shm->mutex);

    // Increment the rear index, circularly
    if(worker_shm->size == 0) {
        worker_shm->front = worker_shm->rear = 0;
    } else {
        worker_shm->rear = (worker_shm->rear + 1) % worker_shm->nr_workers;
    }
    
    // Set the available slot to the new worker ID
    worker_shm->workerAvailability[worker_shm->rear].available = worker_id;
    worker_shm->size++;
    
    // Signal the condition variable to wake up any waiting threads
    pthread_cond_signal(&worker_shm->cond);
    pthread_mutex_unlock(&worker_shm->mutex);
}

int dequeue_worker(WorkerSHM *worker_shm) {
    // Acquire the mutex before modifying the shared memory
    pthread_mutex_lock(&worker_shm->mutex);
    
    // Check if the queue is empty
    while (worker_shm->size == 0) {
        pthread_cond_wait(&worker_shm->cond, &worker_shm->mutex);
    }
    
    int worker_id = worker_shm->workerAvailability[worker_shm->front].available;
    
    worker_shm->front = (worker_shm->front + 1) % worker_shm->nr_workers;
    worker_shm->size--;
    pthread_mutex_unlock(&worker_shm->mutex);
    
    return worker_id;
}