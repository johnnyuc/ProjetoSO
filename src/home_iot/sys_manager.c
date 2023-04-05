/**********************************************
* Autor: António Silva 2020238160            *
* Autor: Johnny Fernandes 2021190668         *
* LEI UC 2022-23 - Sistemas Operativos        *
**********************************************/

// Includes
#include "sys_manager.h"
#include "threads.h"
#include "workers.h"
#include "shared_memory.h"
//#include "message_queue.h"

// Variáveis globais
pthread_mutex_t log_writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

char message[BUFFER_MESSAGE];
ConfigValues config_vals;

// Função de carregamento das configurações
ConfigValues config_loader(char* filepath) {
    ConfigValues values = {0};
    FILE* config_file = fopen(filepath, "r");
    if (config_file == NULL) {
        printf("Failed to open config file. System manager now exiting...\n");
        exit(1);
    }
    char line[BUFFER_MESSAGE];
    int filler = 0;
    while (fgets(line, sizeof(line), config_file)) {
        char* token = strtok(line, " \t\r\n");  // para qualquer divisão de espaço
        if (token == NULL) {
            continue;  // pula linhas vazias, comentários e outros
        }
        int value = atoi(token);

        // Incrementalmente preenche a estrutura de configuração
        if (values.queue_size == 0) {
            values.queue_size = value;
        } else if (values.nr_workers == 0) {
            values.nr_workers = value;
        } else if (values.max_shmkeys == 0) {
            values.max_shmkeys = value;
        } else if (values.max_sensors == 0) {
            values.max_sensors = value;
        } else if (values.max_alerts == 0) {
            values.max_alerts = value;
        }
        filler++;
    }

    // Verifica se todos os valores foram preenchidos devidamente
    if (filler != 5) {
        printf("Invalid config file. System manager now exiting...\n");
        exit(1);
    }

    fclose(config_file);
    return values;
}

// Função de escrita em ficheiro log
void log_writer(char* message) {
    // Se o ficheiro log estiver ocupado, não faz nada e aguarda
    // Obriga a manter a ordem de escrita corretamente entre ecrã e log file
    pthread_mutex_lock(&log_writer_mutex);

    // Timestamp - Vai buscar o tempo atual
    // https://en.cppreference.com/w/c/chrono/strftime
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);

    // Formata o tempo como uma string
    char timestamp[BUFFER_TIME];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", tm_info);

    // Escreve a mensagem no ecrã (deve preceder a escrita em log)
    printf("[%s] %s", timestamp, message);

    // Escreve a mensagem no ficheiro log
    FILE* log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        printf("LOG ERROR: Could not write to file!\n");
        return;
    }
    fprintf(log_file, "[%s] %s", timestamp, message);
    fclose(log_file);

    // Destranca o mutex
    pthread_mutex_unlock(&log_writer_mutex);

    memset(message, 0, BUFFER_MESSAGE); // limpa o buffer de mensagens
}


// Função Main
int main(int argc, char *argv[]) {
    // Verifica se o número correto de parâmetors foi passado
    if (argc != 2) {
        char message[BUFFER_MESSAGE];
        sprintf(message, "Invalid set of parameters. System manager now exiting...\n");
        log_writer(message);
        exit(1);
    }

    config_vals = config_loader(argv[1]);
    main_initializer();

    return 0;
}



// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// Inicializador principal das funções do sistema
void main_initializer() {
    create_threads();
    create_workers(config_vals.nr_workers);
}
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------


