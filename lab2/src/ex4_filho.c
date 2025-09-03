#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct
{
  int valor;
  int seq;
} shm_data;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Uso: %s <shmid>\n", argv[0]);
    exit(1);
  }

  int shmid = atoi(argv[1]);
  shm_data *m = (shm_data *)shmat(shmid, NULL, 0);

  srand(time(NULL) ^ getpid());

  while (1)
  {
    sleep(rand() % 5 + 1);
    m->valor = rand() % 100;
    m->seq++;
    printf("\033[35mFilho %d\033[0m escreveu valor \033[32m%d\033[0m (seq=%d)\n",
           getpid(), m->valor, m->seq);
  }
  shmdt(m);
  return 0;
}
