#include "sys_manager.h"
#include "sys_shm.h"

// System global variables [created in sys_manager.c]
extern char log_buffer[BUFFER_MESSAGE];

// Function to create shared memory
SharedMemory* create_shm(int maxSensorKeyInfo, int maxAlertKeyInfo) {
    size_t shmsize = sizeof(SharedMemory) + (sizeof(SensorKeyInfo) * maxSensorKeyInfo) + (sizeof(AlertKeyInfo) * maxAlertKeyInfo);
    int shmid = shmget(IPC_PRIVATE, shmsize, 0666 | IPC_CREAT);
    if (shmid == -1) {
        sprintf(log_buffer, "SHM: ERROR CREATING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    sprintf(log_buffer, "SHARED MEMORY %d CREATED\n", shmid);
    log_writer(log_buffer);

    // Object to be shared
    SharedMemory *sharedMemory = (SharedMemory *) shmat(shmid, NULL, 0);
    if (sharedMemory == (void *) -1) {
        sprintf(log_buffer, "SHM: ERROR ATTACHING SHARED MEMORY\n");
        log_writer(log_buffer);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    pthread_mutex_init(&sharedMemory->mutex, NULL);
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
        printf("  Key: %s, Last Value: %d, Min Value: %d, Max Value: %d, Average Value: %.2f, Update Count: %d\n",
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
        printf("  Key: %s, Min Value: %.2f, Max Value: %.2f\n",
            sharedMemory->alertKeyInfoArray[i].key,
            sharedMemory->alertKeyInfoArray[i].min,
            sharedMemory->alertKeyInfoArray[i].max);
    }
    
    pthread_mutex_unlock(&sharedMemory->mutex);
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

void insert_alert_key(SharedMemory *sharedMemory, char *key, float min, float max) {
    pthread_mutex_lock(&sharedMemory->mutex);

    int i = 0;
    while (i < sharedMemory->maxAlertKeyInfo && strcmp(sharedMemory->alertKeyInfoArray[i].key, "") != 0) {
        printf("Alert key: %s\n", sharedMemory->alertKeyInfoArray[i].key);
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            // An alert key with the same name already exists
            pthread_mutex_unlock(&sharedMemory->mutex);
            return;
        }
        i++;
    }

    // Insert new alert key
    strcpy(sharedMemory->alertKeyInfoArray[i].key, key);
    sharedMemory->alertKeyInfoArray[i].min = min;
    sharedMemory->alertKeyInfoArray[i].max = max;

    pthread_mutex_unlock(&sharedMemory->mutex);
}

// Remove uma AlertKeyInfo com a chave especificada da estrutura SharedMemory
void remove_alert_key(SharedMemory *sharedMemory, char *key) {
    // Encontra o índice da chave no array
    int index = -1;
    for (int i = 0; i < sharedMemory->maxAlertKeyInfo; i++) {
        printf("Alert key: %s\n", sharedMemory->alertKeyInfoArray[i].key);
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

void sensor_key(SharedMemory* sharedMemory, char* key, int new_value) {
    // Procura pelo nó que possui a chave desejada
    for (int i = 0; i < sharedMemory->maxSensorKeyInfo; i++) {
        if (strcmp(sharedMemory->sensorKeyInfoArray[i].key, key) == 0) {
            // Encontrou a chave, atualiza o valor
            sharedMemory->sensorKeyInfoArray[i].lastValue = new_value;
            return;
        }
    }

    strcpy(sharedMemory->sensorKeyInfoArray[sharedMemory->maxSensorKeyInfo].key, key);
    sharedMemory->sensorKeyInfoArray[sharedMemory->maxSensorKeyInfo].lastValue = new_value;
    sharedMemory->maxSensorKeyInfo++;
}

void alert_key(SharedMemory* sharedMemory, char* key, float new_min, float new_max) {
    for (int i = 0; i < sharedMemory->maxAlertKeyInfo; i++) {
        if (strcmp(sharedMemory->alertKeyInfoArray[i].key, key) == 0) {
            sharedMemory->alertKeyInfoArray[i].min = new_min;
            sharedMemory->alertKeyInfoArray[i].max = new_max;
            return;
        }
    }

    strcpy(sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].key, key);
    sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].min = new_min;
    sharedMemory->alertKeyInfoArray[sharedMemory->maxAlertKeyInfo].max = new_max;
    sharedMemory->maxAlertKeyInfo++;
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
    pthread_mutex_init(&worker_shm->mutex, NULL);
    pthread_cond_init(&worker_shm->cond, NULL);
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
        sprintf(log_buffer, "SHARED MEMORY %d ATTACHED PID: %d\n", shmid, getpid());
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

    printf("Worker Queue:\n");
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