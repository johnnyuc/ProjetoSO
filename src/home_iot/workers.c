#include "workers.h"

int creates_workers(int nr_workers) {

    pid_t pid;
    for (int i = 0; i < nr_workers; i++) {
        pid = fork();
        if (pid == 0) {
            worker_tasks();
            exit(0);
        }
    }
 
    // À espera que todos os processos filhos terminem
    for (int i = 0; i < nr_workers; i++) {
        wait(NULL);
    }
}

void worker_tasks() {

}