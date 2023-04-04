#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 100

typedef struct {
    // Estrutura do USER_CONSOLE
    int command;
    char arg1[20];
    char arg2[20];
} USER_CONSOLE;

typedef struct {
    // Estrutura do SENSOR
    int id;
    float value;
} SENSOR;

typedef struct {
    void** queue;
    int max_size;
    int curr_size;
} INTERNAL_QUEUE;

INTERNAL_QUEUE* create_internal_queue(int max_size) {
    INTERNAL_QUEUE* q = malloc(sizeof(INTERNAL_QUEUE));
    q->queue = malloc(max_size * sizeof(void*));
    q->max_size = max_size;
    q->curr_size = 0;
    return q;
}

void push_internal_queue(INTERNAL_QUEUE* q, void* data) {
    if (q->curr_size == q->max_size) {
        printf("Internal queue is full\n");
        return;
    }
    q->queue[q->curr_size++] = data;
}

void* pop_internal_queue(INTERNAL_QUEUE* q) {
    if (q->curr_size == 0) {
        printf("Internal queue is empty\n");
        return NULL;
    }
    void* data = q->queue[0];
    for (int i = 1; i < q->curr_size; i++) {
        q->queue[i-1] = q->queue[i];
    }
    q->curr_size--;
    return data;
}


void print_internal_queue(INTERNAL_QUEUE* q) {
    printf("Internal queue size: %d\n", q->curr_size);
    printf("Internal queue content:\n");
    for (int i = 0; i < q->curr_size; i++) {
        void* data = q->queue[i];

        SENSOR* s = (SENSOR*)data;
        USER_CONSOLE* uc = (USER_CONSOLE*)data;
        if (uc != NULL) {
            printf("USER_CONSOLE: command=%d, arg1=%s, arg2=%s\n", uc->command, uc->arg1, uc->arg2);
        } else if (s != NULL) {
            printf("SENSOR: id=%d, value=%.2f\n", s->id, s->value);
        }
    }
}

int main() {
    // Criar internal queue
    INTERNAL_QUEUE* q = create_internal_queue(MAX_SIZE);
    printf("AA\n");
    print_internal_queue(q);

    // Criar USER_CONSOLE
    USER_CONSOLE* uc = malloc(sizeof(USER_CONSOLE));
    uc->command = 1;
    strcpy(uc->arg1, "foo");
    strcpy(uc->arg2, "bar");
    printf("BB\n");
    // Adicionar USER_CONSOLE à fila
    push_internal_queue(q, uc);

    // Criar SENSOR
    SENSOR* s = malloc(sizeof(SENSOR));
    s->id = 2;
    s->value = 3.14;

    // Adicionar SENSOR à fila
    push_internal_queue(q, s);

    // Imprimir conteúdo da fila
    print_internal_queue(q);

    // Remover um elemento da fila e imprimir
    void* data = pop_internal_queue(q);
    if (data != NULL) {
        USER_CONSOLE* uc = (USER_CONSOLE*) data;
        SENSOR* s = (SENSOR*) data;
        if (uc != NULL) {
            printf("Removed USER_CONSOLE: command=%d, arg1=%s, arg2=%s\n", uc->command, uc->arg1, uc->arg2);
        } else if (s != NULL) {
            printf("Removed SENSOR: id=%d, value=%.2f\n", s->id, s->value);
        }
    }

    // Imprimir conteúdo da fila novamente
    print_internal_queue(q);

    // Liberar memória alocada
    free(uc);
    free(s);
    free(q->queue);
    free(q);

    return 0;
}
