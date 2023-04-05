#ifndef IOT_PROJECT_WORKERS_H
#define IOT_PROJECT_WORKERS_H

#include <stdlib.h>
#include <pthread.h>

int create_workers(int nr_workers);
void worker_tasks();


#endif //IOT_PROJECT_WORKERS_H