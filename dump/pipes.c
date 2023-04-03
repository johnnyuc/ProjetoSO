#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct {
  int a;
  int b;
} numbers;

void worker();
void master();

// descritor de ficheiro para o pipe, channel[0] para leitura e channel[1] para escrita
int channel[2];

int main() {
  // cria um pipe
  pipe(channel);

 // fork cria um processo filho
  if (fork() == 0) {
    // executa a função worker
    worker();
    // encerra o processo filho, quando tiver acabado, mas até lá o pai tem que esperar
    exit(0);
  }
  // executa a função master
  master();
  
  // espera pelo processo filho
  wait(NULL);
  
  return 0;
}

void worker() {
  // struct com 2 números
  numbers n;

  // fecha o descritor de escrita, porque não escreve para o pipe
  // apenas lê e com base nos valores que recebe, escreve no stdout
  close(channel[1]);

  while (1) {
    read(channel[0], &n, sizeof(numbers));
    printf("[WORKER] Received (%d,%d) from master to add. Result=%d\n", 
           n.a, n.b, n.a+n.b);
  }
}

void master() {
  // struct com 2 números
  numbers n;
 
  // fecha o descritor de leitura, porque não lê do pipe
  // só enviará valores para o pipe para o worker somar
  close(channel[0]);
 
  while (1) {
    n.a = rand() % 100;
    n.b = rand() % 100;
 
    printf("[MASTER] Sending (%d,%d) for WORKER to add\n", n.a, n.b);
    write(channel[1], &n, sizeof(numbers));
    sleep(2);
  }
}
