#include "user_console.h"

// Global variables for the console_id and the pipe file descriptor
// Console elements
int console_id;
int console_fd;

// Threads
pthread_t reader_thread;
pthread_t writer_thread;

// Message queue
int msgid;

// Function to handle the SIGINT signal
void handle_sigint() {
    pthread_cancel(reader_thread);
    pthread_cancel(writer_thread);
    close(console_fd);
    exit(EXIT_SUCCESS);
}

int float_validation(const char* str) {
    char* end;
    strtod(str, &end);
    if (*end == '\0') {
        return 1;
    } else {
        return 0;
    }
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
    int pos = sprintf(result, "%d#", console_id);

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
    if (strcmp(argv[0], "EXIT") == 0 && argc == 1) {
        return strcpy(command, "EXIT");
    } else if (strcmp(argv[0], "STATS") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "RESET") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "SENSORS") == 0 && argc == 1) {
        return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "ADD_ALERT") == 0 && argc == 5) {
        // Check if id is alphanumeric
        if (alnum_validation(argv[1], 0) && alnum_validation(argv[2], 0) && 
        float_validation(argv[3]) && float_validation(argv[4]) &&
        atof(argv[3]) < atof(argv[4]))
            return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "REMOVE_ALERT") == 0 && argc == 2) {
        // Check if id is alphanumeric
        if (alnum_validation(argv[1], 0))
            return pipe_format(command, argv, argc);
    } else if (strcmp(argv[0], "LIST_ALERTS") == 0 && argc == 1) {
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
    printf("PIPE [%d] OPENED\n", console_fd);
    printf("CONSOLE [%d] CONNECTED TO SERVER\n\n", console_id);
    
    // Print menu
    printf("MENU OPTIONS:\n");
    printf("* EXIT\n");
    printf("* STATS\n");
    printf("* SENSORS\n");
    printf("* RESET\n");
    printf("* LIST_ALERTS\n");
    printf("* ADD_ALERT [ID] [KEY] [MIN] [MAX]\n");
    printf("* REMOVE_ALERT [ID]\n\n");

    // Send messages to pipe
    while (1) {
        // Read command from stdin
        char command[BUFFER_MESSAGE];
        printf("> ");
        fgets(command, BUFFER_MESSAGE, stdin);
        
        // Ignore casing
        for (int i = 0; command[i] != '\0'; i++) {
            command[i] = toupper(command[i]);
        }

        // Validate and format or exit
        if (command_validation(command) == NULL) {
            printf("INCORRECT COMMAND: %s\n", command);
            continue;
        } else if (strcmp(command, "EXIT") == 0){
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
        memset(command, 0, sizeof(command));
    }
    return NULL;
}

void *reader_function() {
    // Message queue
    msgqueue msg;

    while (1) {
        int result = msgrcv(msgid, &msg, sizeof(msg), console_id, 0);
        if (result < 0) {
            printf("MESSAGE QUEUE CLOSED [HOME_IOT]. EXITING\n");
            handle_sigint(0);
        }
        if (strcmp(msg.msg_text, "END") == 0) {
            printf("> ");
            fflush(stdout);
        } else if (strcmp(msg.msg_text, "OK") == 0 || strcmp(msg.msg_text, "ERROR") == 0) {
            printf("%s\n", msg.msg_text);
            printf("> ");
            fflush(stdout);
        } else printf("%s\n", msg.msg_text);
    }
    
    return NULL;
}

void main_initializer(char *argv[]) {
    // Check if the console_id is valid
    if (!alnum_validation(argv[1], 1)) {
        printf("CONSOLE_ID SHOULD ONLY CONTAIN NUMERIC CHARACTERS\n");
        exit(EXIT_FAILURE);
    } else {
        console_id = atoi(argv[1]);
    }

    // Check if the console_id is valid
    if (console_id < 1) {
        printf("CONSOLE ID SHOULD BE OVER 0\n");
        exit(EXIT_FAILURE);
    }

    // Create threads for reader and writer and wait for them to finish
    int status;

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

    // Create message queue
    key_t key = ftok(".", 'a');
    msgid = msgget(key, IPC_CREAT | 0666);
    
    // Run user_console
    main_initializer(argv);
    return 0;
}