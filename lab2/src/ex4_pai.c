#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct
{
  int valor;
  int seq;
} shm_data;

int main()
{
  int shmid1, shmid2;
  shm_data *m1, *m2;

  shmid1 = shmget(IPC_PRIVATE, sizeof(shm_data), IPC_CREAT | 0666);
  shmid2 = shmget(IPC_PRIVATE, sizeof(shm_data), IPC_CREAT | 0666);

  if (shmid1 == -1 || shmid2 == -1)
  {
    perror("Erro ao criar a memória compartilhada");
    exit(EXIT_FAILURE);
  }

  m1 = (shm_data *)shmat(shmid1, NULL, 0);
  m2 = (shm_data *)shmat(shmid2, NULL, 0);

  if (m1 == (void *)-1 || m2 == (void *)-1)
  {
    perror("Erro ao anexar a memória compartilhada");
    exit(EXIT_FAILURE);
  }

  m1->seq = 0;
  m2->seq = 0;

  if (fork() == 0)
  {
    char shmid_str[10];
    sprintf(shmid_str, "%d", shmid1);
    execl("./bin/ex4_filho", "ex4_filho", shmid_str, (char *)NULL);
    perror("Erro ao executar o arquivo ex4_filho. Filho 1");
    exit(EXIT_FAILURE);
  }

  if (fork() == 0)
  {
    char shmid_str[10];
    sprintf(shmid_str, "%d", shmid2);
    execl("./bin/ex4_filho", "ex4_filho", shmid_str, (char *)NULL);
    perror("Erro ao executar o arquivo ex4_filho. Filho 2");
    exit(EXIT_FAILURE);
  }

  int last1 = 0, last2 = 0;
  while (1)
  {
    if (m1->seq != last1 && m2->seq != last2)
    {
      printf("\x1b[32mProduto = %d x %d = %d\x1b[0m\n",
             m1->valor, m2->valor, m1->valor * m2->valor);
      last1 = m1->seq;
      last2 = m2->seq;
    }
    sleep(1);
  }

  shmdt(m1);
  shmdt(m2);
  shmctl(shmid1, IPC_RMID, NULL);
  shmctl(shmid2, IPC_RMID, NULL);

  return 0;
}
