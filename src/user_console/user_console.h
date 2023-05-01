#ifndef IOT_PROJECT_USER_CONSOLE_H
#define IOT_PROJECT_USER_CONSOLE_H

// Includes
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strlen, strcmp, strcpy, strtok etc
#include <ctype.h> // isalnum, isdigit, toupper
#include <signal.h> // signal
#include <pthread.h> // pthread_create, pthread_cancel
#include <unistd.h> // open, close, read, write
#include <fcntl.h> // O_RDONLY O_WRONLY
#include <errno.h> // errno

// Defines
#define CONSOLE_PIPE "CONSOLE_PIPE"
#define BUFFER_MESSAGE 256
#define MAX_ARGS 5
#define MIN_LEN 3
#define MAX_LEN 32

#endif //IOT_PROJECT_USER_CONSOLE_H