#include <stdio.h>
#include <stdlib.h>

#define CHAVE_MAX_LENGTH 20
#define MAX_NODES 300
#define MAX_ALERTS 10

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef struct SensorInfo {
    int lastValue;
    int minValue;
    int maxValue;
    double averageValue;
    int updateCount;
} SensorInfo;

typedef struct Node {
    int id;
    char chave[CHAVE_MAX_LENGTH];
    SensorInfo sensorInfo;
    struct Node *next;
} Node;

typedef struct HashTable {
    int size;
    Node **table;
} HashTable;

typedef struct Alert {
    int id;
    char chave[CHAVE_MAX_LENGTH];
    float min;
    float max;
} Alert;

typedef struct SharedMemory {
    HashTable hashTable;
    Node nodes[MAX_NODES];
    int nextFreeNode;
    Alert alerts[MAX_ALERTS];
    int nextFreeAlert;
} SharedMemory;

void update_sensor_info(SensorInfo *sensorInfo, int value) {
    sensorInfo->lastValue = value;
    sensorInfo->minValue = (sensorInfo->updateCount == 0) ? value : min(sensorInfo->minValue, value);
    sensorInfo->maxValue = (sensorInfo->updateCount == 0) ? value : max(sensorInfo->maxValue, value);
    sensorInfo->averageValue = ((sensorInfo->averageValue * sensorInfo->updateCount) + value) / (sensorInfo->updateCount + 1);
    sensorInfo->updateCount++;
}

void init_shared_memory(SharedMemory *sharedMemory) {
    sharedMemory->hashTable.size = MAX_NODES;
    sharedMemory->hashTable.table = (Node **) &sharedMemory->nodes;
    sharedMemory->nextFreeNode = 0;

    for (int i = 0; i < sharedMemory->hashTable.size; i++) {
        sharedMemory->hashTable.table[i] = NULL;
    }

    sharedMemory->nextFreeAlert = 0;
}

Node *get_free_node(SharedMemory *sharedMemory) {
    if (sharedMemory->nextFreeNode >= MAX_NODES) {
        // Não há mais espaço livre para novos nós
        return NULL;
    }

    Node *freeNode = &sharedMemory->nodes[sharedMemory->nextFreeNode];
    sharedMemory->nextFreeNode++;

    return freeNode;
}

Alert *get_free_alert(SharedMemory *sharedMemory) {
    if (sharedMemory->nextFreeAlert >= MAX_ALERTS) {
        // Não há mais espaço livre para novos alertas
        return NULL;
    }

    Alert *freeAlert = &sharedMemory->alerts[sharedMemory->nextFreeAlert];
    sharedMemory->nextFreeAlert++;

    return freeAlert;
}

void test_shared_memory() {
    // Inicializa a memória compartilhada com dados dummy
    SharedMemory sharedMemory;
    init_shared_memory(&sharedMemory);

    // Insere alguns nós de teste na memória compartilhada
    for (int i = 0; i < 5; i++) {
        Node *node = get_free_node(&sharedMemory);
        if (node) {
            node->id = i;
            snprintf(node->chave, CHAVE_MAX_LENGTH, "Chave%d", i);
            update_sensor_info(&node->sensorInfo, i * 10);
            node->next = NULL;
        }
    }

    // Insere alguns alertas de teste na memória compartilhada
    for (int i = 0; i < 3; i++) {
        Alert *alert = get_free_alert(&sharedMemory);
        if (alert) {
            alert->id = i;
            snprintf(alert->chave, CHAVE_MAX_LENGTH, "Chave%d", i);
            alert->min = i * 5.0f;
            alert->max = i * 15.0f;
        }
    }

    // Exibe o conteúdo da memória compartilhada
    printf("Nodes:\n");
    for (int i = 0; i < sharedMemory.nextFreeNode; i++) {
        Node *node = &sharedMemory.nodes[i];
        printf("Node ID: %d, Chave: %s, Last Value: %d\n", node->id, node->chave, node->sensorInfo.lastValue);
    }

    printf("\nAlerts:\n");
    for (int i = 0; i < sharedMemory.nextFreeAlert; i++) {
        Alert *alert = &sharedMemory.alerts[i];
        printf("Alert ID: %d, Chave: %s, Min: %.2f, Max: %.2f\n", alert->id, alert->chave, alert->min, alert->max);
    }
}

int main() {
    test_shared_memory();

    return 0;
}