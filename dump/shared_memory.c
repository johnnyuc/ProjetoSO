// Producer/Consumer using POSIX named semaphores
// Buffer with N slots
// 1 producer and 1 consumer
// Compile as: gcc -Wall -pthread sem_test-posix_named.c -o sem_test-posix_named

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <semaphore.h>

#define N 5
#define TOTAL_VALUES 10
typedef struct {
	int buf[N];
	int write_pos, read_pos;
} mem_struct;

int shmid, *write_pos, *read_pos, *buf;
mem_struct* mem;
sem_t *empty, *full, * mutex;

void init() {
	shmid = shmget(IPC_PRIVATE, sizeof(mem_struct), IPC_CREAT|0700);
	if (shmid < 1) exit(0);
	mem = (mem_struct*) shmat(shmid, NULL, 0);
	if (mem < (mem_struct*) 1) exit(0);

	sem_unlink("EMPTY");
	empty=sem_open("EMPTY",O_CREAT|O_EXCL,0700,N);
	sem_unlink("FULL");
	full=sem_open("FULL",O_CREAT|O_EXCL,0700,0);
	sem_unlink("MUTEX");
	mutex=sem_open("MUTEX",O_CREAT|O_EXCL,0700,1);

	mem->write_pos = mem->read_pos = 0;
	write_pos = &mem->write_pos;
	read_pos = &mem->read_pos;
	buf = (int*)&mem->buf;
}

void terminate() {
  sem_close(empty);
  sem_close(full);
  sem_close(mutex);
  sem_unlink("EMPTY");
  sem_unlink("FULL");
  sem_unlink("MUTEX");
  shmctl(shmid, IPC_RMID, NULL);
}

void put_element(int e) {
  sem_wait(empty);
  sem_wait(mutex);

  mem->buf[*write_pos] = e;
  *write_pos = (*write_pos+1) % N;

  sem_post(mutex);
  sem_post(full);
}

int get_element() {
  sem_wait(full);
  sem_wait(mutex);

  int e = mem->buf[*read_pos];
  *read_pos = (*read_pos+1) % N;

  sem_post(mutex);
  sem_post(empty);

  return e;
}

void producer() {
  int i;
  for (i=TOTAL_VALUES; i>0; i--) {
    printf("[PRODUCER] Writing %d\n", i);
    put_element(i);
  }
}

void consumer() {
  int i;
  for (i=0; i<TOTAL_VALUES; i++) {
    int e = get_element();
    printf("[CONSUMER] Retrieved %d\n", e);
    sleep(1);
  }
  terminate();
}

int main(int argc, char* argv[]) {
  init();

  if (fork() == 0) {
    producer();
    exit(0);
  }
  else {
    consumer();
    exit(0);
  }
}
