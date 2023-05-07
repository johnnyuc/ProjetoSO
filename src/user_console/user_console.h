/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#ifndef IOT_PROJECT_USER_CONSOLE_H
#define IOT_PROJECT_USER_CONSOLE_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Defines
#define CONSOLE_PIPE "CONSOLE_PIPE"
#define BUFFER_MESSAGE 256
#define MAX_LEN 32
#define MIN_LEN 3
#define MAX_ARGS 5

// Structs
typedef struct msgqueue {
    long msg_type;
    char msg_text[BUFFER_MESSAGE];
} msgqueue;

#endif //IOT_PROJECT_USER_CONSOLE_H