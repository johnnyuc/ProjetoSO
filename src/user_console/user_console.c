/**********************************************
* Author: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

#include "user_console.h"

// Global variables for the console_id and the pipe file descriptor
char *console_id[MAX_LEN];
int console_fd;

// Function to handle the SIGINT signal
void handle_sigint(int sig) {
    // Check if pthread_cancel is needed
    close(console_fd);
    printf("EXITING, CODE %d\n", sig);
    exit(EXIT_SUCCESS);
}

// Function to validate the format console_id
int alnum_validation(const char *str, int onlynum) {
    // Check if the string is numeric
    if (onlynum) {
        // Limits
        if (strlen(str) > MIN_LEN) {
            return 0;
        }
        // Check if the character is a number
        for (int i = 0; str[i] != '\0'; i++) {
            if (!isdigit(str[i])) {
                return 0;
            }
        }
        return 1;
    }
    
    // Check if the string is alphanumeric
    else {
        // Limits
        if (strlen(str) < MIN_LEN || strlen(str) > MAX_LEN) {
            return 0;
        }
        // Check if the character is alphanumeric
        for (int i = 0; str[i] != '\0'; i++) {
            if (!isalnum(str[i])) {
                return 0;
            }
        }
        return 1;
    }
}

// Function to format the command to be sent to the server
char *pipe_format(char *result, char argv[MAX_ARGS][MAX_LEN], int argc) {
    // Prepend *console_id
    int pos = sprintf(result, "%s#", *console_id);

    // Convert the arguments to uppercase and join them with #
    for (int i = 0; i < argc; i++) {
        char *str = argv[i];
        while (*str) {
            result[pos++] = toupper((unsigned char)*str++);
        }
        if (i != argc - 1) {
            result[pos++] = '#';
        }
    }
    result[pos] = '\0';

    // Remove trailing # 
    // TODO: probably not needed
    if (pos > 0 && result[pos - 1] == '#') {
        result[pos - 1] = '\0';
    }
    return result;
}

// Function to validate the command and its arguments
// TODO: add more validation, including casing
char *command_validation(char *command) {
    char argv[MAX_ARGS][MAX_LEN];
    int argc = 0;
    char *token;

    // Splitting the command into arguments using space as the delimiter
    token = strtok(command, " ");

    // Splitter
    while (token != NULL && argc < MAX_ARGS) {
        // Remove trailing and leading spaces
        size_t len = strcspn(token, " \n\r\t");
        strncpy(argv[argc], token, len);
        argv[argc][len] = '\0';
        argc++;
        token = strtok(NULL, " ");
    }

    // Validate the command and its arguments
    // 0: command, 1: id, 2: key, 3: min_value, 4: max_value
    if (strcmp(argv[0], "exit") == 0 && argc == 1) {
        return strcpy(command, "exit");
    } else if (strcmp(argv[0], "stats") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "reset") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "sensors") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "add_alert") == 0 && argc == 5) {
        // Check if id is alphanumeric
        if (alnum_validation(argv[1], 0) && alnum_validation(argv[2], 0) && 
        alnum_validation(argv[3], 1) && alnum_validation(argv[4], 1))
            return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "remove_alert") == 0 && argc == 2) {
        // Check if id is alphanumeric
        if (alnum_validation(argv[1], 0))
            return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "list_alerts") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    }

    // Presentation of invalid command and return NULL
    command[strcspn(command, "\n")] = '\0';
    return NULL;
}

void *writer_function() {
    // Open pipe
    console_fd = open(CONSOLE_PIPE, O_WRONLY);
    if (console_fd < 0) {
        printf("ERROR ACCESSING PIPE\n");
        exit(EXIT_FAILURE);
    }
    
    // Announce connection to server
    printf("PIPE OPENED, FD: %d\n", console_fd);
    printf("CONSOLE %s CONNECTED TO SERVER\n\n", *console_id);
    
    // Print menu
    printf("Menu options:\n");
    printf("* exit\n");
    printf("* stats\n");
    printf("* sensors\n");
    printf("* reset\n");
    printf("* list_alerts\n");
    printf("* add_alert [id] [key] [min] [max]\n");
    printf("* remove_alert [id]\n\n");

    // Send messages to pipe
    while (1) {
        // Read command from stdin
        char command[BUFFER_MESSAGE];
        printf("> ");
        fgets(command, BUFFER_MESSAGE, stdin);

        // Validate and format or exit
        if (command_validation(command) == NULL) {
            printf("INCORRECT COMMAND: %s\n", command);
            continue;
        } else if (strcmp(command, "exit") == 0){
            handle_sigint(0);
            break;
        }

        // Send command to pipe
        int write_code = write(console_fd, command, strlen(command));
        if (write_code < 0) {
            if (errno == EPIPE) {  // pipe closed by server 
                printf("PIPE CLOSED. EXITING.\n");
                break;
            } else {
                printf("ERROR SENDING DATA. EXITING\n");
                break;
            }
        }
    }
    return NULL;
}

void *reader_function() {
    // Code to be implemented
    // Message queue
    return NULL;
}

void main_initializer(char *argv[]) {
    // Define console_id
    *console_id = argv[1];

    // Check if the console_id is valid
    if (strlen(*console_id) < MIN_LEN || strlen(*console_id) > MAX_LEN) {
        printf("CONSOLE LENGTH SHOULD BE BETWEEN 3 AND 32\n");
        exit(EXIT_FAILURE);
    }

    // Check if the console_id is valid
    if (!alnum_validation(*console_id, 0)) {
        printf("CONSOLE_ID SHOULD ONLY CONTAIN ALPHANUMERIC CHARACTERS\n");
        exit(EXIT_FAILURE);
    }

    // Create threads for reader and writer and wait for them to finish
    int status;
    pthread_t reader_thread, writer_thread;

    status = pthread_create(&reader_thread, NULL, reader_function, NULL);
    if (status != 0) {
        perror("ERROR CREATING READER THREAD");
        exit(EXIT_FAILURE);
    }

    status = pthread_create(&writer_thread, NULL, writer_function, NULL);
    if (status != 0) {
        perror("ERROR CREATING WRITER THREAD");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish - should never happen, SIGINT will kill them
    pthread_join(reader_thread, NULL);
    pthread_join(writer_thread, NULL);
}

int main(int argc, char *argv[]) {
    // Verify if the number of arguments is correct
    if (argc != 2) {
        printf("SYNTAX: %s {CONSOLE_ID}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Configure signal handlers
    signal(SIGINT, handle_sigint);

    // Run user_console
    main_initializer(argv);
    return 0;
}