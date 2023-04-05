#include "threads.h"

int create_threads() {
    // Creates console_reader thread
    pthread_t console_reader;
    pthread_create(&console_reader, NULL, console_reader, NULL);

    // Creates sensor_reader thread
    pthread_t sensor_reader;
    pthread_create(&sensor_reader, NULL, sensor_reader, NULL);

    // Creates dispatcher thread
    pthread_t dispatcher;
    pthread_create(&dispatcher, NULL, dispatcher, NULL);

}

void *console_reader(void *arg) {
    // O que é que o console reader faz
}

void *sensor_reader(void *arg) {
    // O que é que o sensor reader faz
}

void *dispatcher(void *arg) {
    // O que é que o dispatcher faz
}