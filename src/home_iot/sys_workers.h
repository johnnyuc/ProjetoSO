// Author: Johnny Fernandes 2021190668
// LEI UC 2022-23 - Sistemas Operativos

#ifndef IOT_PROJECT_WORKERS_H
#define IOT_PROJECT_WORKERS_H

// Includes
#include <stdlib.h> // Used for NULL
#include <unistd.h> // Used for fork
#include <sys/wait.h> // Used for wait

// Functions
void create_unnamed_pipes(int **pipes_fd, int nr_workers);
int create_workers(int nr_workers, int shmid);
int worker_tasks(int shmid, int *pipe_fd);

#endif //IOT_PROJECT_WORKERS_H