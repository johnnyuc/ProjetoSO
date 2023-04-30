/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "sys_manager.h"
#include "sys_shm.h"

#include <errno.h>

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
    sharedMemory->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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
    } else {
        sprintf(log_buffer, "SHARED MEMORY %d ATTACHED PID: %d\n", shmid, getpid());
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

    // Free allocated memory
    free(sharedMemory->sensorKeyInfoArray);
    free(sharedMemory->alertKeyInfoArray);

    // Detach shared memory
    detach_shm(sharedMemory);

    // Remove shared memory
    int stillExists = shmget(storedShmid, 0, 0);
    if (stillExists == -1) {
        if (errno == ENOENT) {
            printf("Shared memory segment %d has been successfully removed.\n", storedShmid);
        } else {
            perror("shmget");
            exit(EXIT_FAILURE);
        }
    } else {
        if (shmctl(storedShmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            sprintf(log_buffer, "SHM: ERROR REMOVING SHARED MEMORY\n");
            log_writer(log_buffer);
            exit(EXIT_FAILURE);
        }
    }
}

// Missing functions to read, write, remove, update and search for sensor and alert key info

void insert_sensor_key(SharedMemory* sharedMemory, char* key, int lastValue, int minValue, int maxValue, double averageValue, int updateCount) {
    pthread_mutex_lock(&sharedMemory->mutex);

    int i = 0;
    while (i < sharedMemory->maxSensorKeyInfo && sharedMemory->sensorKeyInfoArray[i].key[0] != '\0') {
        if (strcmp(sharedMemory->sensorKeyInfoArray[i].key, key) == 0) {
            // A sensor key with the same name already exists
            pthread_mutex_unlock(&sharedMemory->mutex);
            return;
        }
        i++;
    }

    if (i == sharedMemory->maxSensorKeyInfo) {
        // Array is full
        pthread_mutex_unlock(&sharedMemory->mutex);
        return;
    }

    // Insert new sensor key
    strcpy(sharedMemory->sensorKeyInfoArray[i].key, key);
    sharedMemory->sensorKeyInfoArray[i].lastValue = lastValue;
    sharedMemory->sensorKeyInfoArray[i].minValue = minValue;
    sharedMemory->sensorKeyInfoArray[i].maxValue = maxValue;
    sharedMemory->sensorKeyInfoArray[i].averageValue = averageValue;
    sharedMemory->sensorKeyInfoArray[i].updateCount = updateCount;

    pthread_mutex_unlock(&sharedMemory->mutex);
}

void insert_alert_key(SharedMemory* sharedMemory, char* key, float min, float max) {
    pthread_mutex_lock(&sharedMemory->mutex);

    int i = 0;
    while (i < sharedMemory->maxAlertKeyInfo && sharedMemory->alertKeyInfoArray[i].key[0] != '\0') {
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            // An alert key with the same name already exists
            pthread_mutex_unlock(&sharedMemory->mutex);
            return;
        }
        i++;
    }

    if (i == sharedMemory->maxAlertKeyInfo) {
        // Array is full
        pthread_mutex_unlock(&sharedMemory->mutex);
        return;
    }

    // Insert new alert key
    strcpy(sharedMemory->alertKeyInfoArray[i].key, key);
    sharedMemory->alertKeyInfoArray[i].min = min;
    sharedMemory->alertKeyInfoArray[i].max = max;

    pthread_mutex_unlock(&sharedMemory->mutex);
}

// Remove a SensorKeyInfo com a chave especificada da estrutura SharedMemory
void remove_sensor_key(SharedMemory *sharedMemory, char *key) {
    // Encontra o índice da chave no array
    int index = -1;
    for (int i = 0; i < sharedMemory->maxSensorKeyInfo; i++) {
        if (strcmp(sharedMemory->sensorKeyInfoArray[i].key, key) == 0) {
            index = i;
            break;
        }
    }

    // Se a chave não foi encontrada, retorna
    if (index == -1) {
        printf("Chave de sensor não encontrada: %s\n", key);
        return;
    }

    // Remove a chave e atualiza o tamanho do array
    pthread_mutex_lock(&sharedMemory->mutex);
    memmove(&sharedMemory->sensorKeyInfoArray[index], &sharedMemory->sensorKeyInfoArray[index+1], 
            (sharedMemory->maxSensorKeyInfo - index - 1) * sizeof(SensorKeyInfo));
    sharedMemory->maxSensorKeyInfo--;
    pthread_mutex_unlock(&sharedMemory->mutex);

    printf("Chave de sensor removida: %s\n", key);
}

// Remove uma AlertKeyInfo com a chave especificada da estrutura SharedMemory
void remove_alert_key(SharedMemory *sharedMemory, char *key) {
    // Encontra o índice da chave no array
    int index = -1;
    for (int i = 0; i < sharedMemory->maxAlertKeyInfo; i++) {
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            index = i;
            break;
        }
    }

    // Se a chave não foi encontrada, retorna
    if (index == -1) {
        printf("Chave de alerta não encontrada: %s\n", key);
        return;
    }

    // Remove a chave e atualiza o tamanho do array
    pthread_mutex_lock(&sharedMemory->mutex);
    memmove(&sharedMemory->alertKeyInfoArray[index], &sharedMemory->alertKeyInfoArray[index+1], 
            (sharedMemory->maxAlertKeyInfo - index - 1) * sizeof(AlertKeyInfo));
    sharedMemory->maxAlertKeyInfo--;
    pthread_mutex_unlock(&sharedMemory->mutex);

    printf("Chave de alerta removida: %s\n", key);
}

void update_sensor_key(SharedMemory* sharedMemory, char* key, int new_value) {
    // Procura pelo nó que possui a chave desejada
    for (int i = 0; i < sharedMemory->maxSensorKeyInfo; i++) {
        if (strcmp(sharedMemory->sensorKeyInfoArray[i].key, key) == 0) {
            // Encontrou a chave, atualiza o valor
            sharedMemory->sensorKeyInfoArray[i].lastValue = new_value;
            return;
        }
    }

    // Se a chave não foi encontrada, pode ser tratado como um erro ou adicionada ao array
    printf("Chave %s não encontrada\n", key);
    // ou pode adicionar a nova chave ao array:
    // sharedMemory->sensorKeyInfoArray[sharedMemory->maxSensorKeyInfo].key = key;
    // sharedMemory->sensorKeyInfoArray[sharedMemory->maxSensorKeyInfo].lastValue = new_value;
    // sharedMemory->maxSensorKeyInfo++;
}

void update_alert_key(SharedMemory* sharedMemory, char* key, float new_min, float new_max) {
    // Procura pelo nó que possui a chave desejada
    for (int i = 0; i < sharedMemory->maxAlertKeyInfo; i++) {
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            // Encontrou a chave, atualiza os valores
            sharedMemory->alertKeyInfoArray[i].min = new_min;
            sharedMemory->alertKeyInfoArray[i].max = new_max;
            return;
        }
    }

    // Se a chave não foi encontrada, pode ser tratado como um erro ou adicionada ao array
    printf("Chave %s não encontrada\n", key);
    // ou pode adicionar a nova chave ao array:
    // sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].key = key;
    // sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].min = new_min;
    // sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].max = new_max;
    // sharedMemory->maxAlertKeyInfo++;
}

WorkerSHM *create_worker_queue(int nr_workers) {
    // Create the shared memory segment
    int shmid = shmget(IPC_PRIVATE, sizeof(WorkerSHM), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment
    WorkerSHM *worker_shm = (WorkerSHM *) shmat(shmid, NULL, 0);
    if (worker_shm == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex and condition variable
    pthread_mutex_init(&worker_shm->mutex, NULL);
    pthread_cond_init(&worker_shm->cond, NULL);

    // Initialize the queue
    worker_shm->workerAvailability = (WorkerAvailability *) calloc(nr_workers, sizeof(WorkerAvailability));
    if (worker_shm->workerAvailability == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    // Initialize parameters
    worker_shm->nr_workers = nr_workers;
    worker_shm->shmid = shmid;

    // Preloading workers queue
    for (int i = 0; i < nr_workers; i++)
        enqueue_worker(worker_shm, i);

    return worker_shm;
}

// Function to attach shared memory
WorkerSHM *attach_worker_shm(int shmid) {
    WorkerSHM *worker_shm = (WorkerSHM *)shmat(shmid, NULL, 0);
    if (worker_shm == (void *)-1) {
        sprintf(log_buffer, "SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    } else {
        sprintf(log_buffer, "SHARED MEMORY %d ATTACHED PID: %d\n", shmid, getpid());
        log_writer(log_buffer);
    }
    return worker_shm;
}

void detach_worker_shm(WorkerSHM *worker_shm) {
    if (shmdt(worker_shm) == -1) {
        sprintf(log_buffer, "SHM: ERROR DETACHING SHARED MEMORY %d\n", worker_shm->shmid);
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }
}

void remove_worker_queue(WorkerSHM *worker_shm) {
    int storedShmid = worker_shm->shmid; // Otherwise unaccessible after detach

    // Free allocated memory
    free(worker_shm->workerAvailability);

    // Detach shared memory
    detach_worker_shm(worker_shm);

    // Remove shared memory
    int stillExists = shmget(storedShmid, 0, 0);
    if (stillExists == -1) {
        if (errno == ENOENT) {
            printf("Shared memory segment %d has been successfully removed.\n", storedShmid);
        } else {
            perror("shmget");
            exit(EXIT_FAILURE);
        }
    } else {
        if (shmctl(storedShmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            sprintf(log_buffer, "SHM: ERROR REMOVING SHARED MEMORY\n");
            log_writer(log_buffer);
            exit(EXIT_FAILURE);
        }
    }
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