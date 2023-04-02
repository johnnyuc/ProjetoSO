#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

void *thread_function(void *arg) {
    printf("Thread iniciada!\n");
    sleep(3); // espera 3 segundos
    printf("Thread finalizada!\n");
    pthread_exit(NULL); // encerra thread
}

int main() {
    pid_t pid;
    pthread_t thread;
    int rc;
    pid = fork();
    if (pid == 0) { // processo filho
        printf("Processo filho iniciado!\n");
        rc = pthread_create(&thread, NULL, thread_function, NULL); // cria thread
        if (rc) { // erro na criação da thread
            printf("Erro ao criar thread! Código de retorno: %d\n", rc);
            exit(-1);
        }
        pthread_join(thread, NULL); // espera pela thread terminar
        printf("Processo filho finalizado!\n");
        exit(0); // encerra processo filho
    } else if (pid > 0) { // processo pai
        printf("Processo pai iniciado!\n");
        printf("Aguardando processo filho...\n");
        wait(NULL); // espera pelo processo filho
        printf("Processo filho encerrado!\n");
        rc = pthread_create(&thread, NULL, thread_function, NULL); // cria thread
        if (rc) { // erro na criação da thread
            printf("Erro ao criar thread! Código de retorno: %d\n", rc);
            exit(-1);
        }
        pthread_join(thread, NULL); // espera pela thread terminar
        printf("Processo pai finalizado!\n");
    } else { // erro ao criar processo
        printf("Erro ao criar processo!\n");
        return 1;
    }
    return 0;
}