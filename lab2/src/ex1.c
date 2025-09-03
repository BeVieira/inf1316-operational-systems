#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define TAM 3

int main(int argc, char *argv[])
{
  int segmento1, segmento2, segmento3;
  int (*matriz1)[TAM], (*matriz2)[TAM], (*matriz3)[TAM];
  int pid, status;

  segmento1 = shmget(IPC_PRIVATE, TAM * TAM * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  segmento2 = shmget(IPC_PRIVATE, TAM * TAM * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  segmento3 = shmget(IPC_PRIVATE, TAM * TAM * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if (segmento1 < 0 || segmento2 < 0 || segmento3 < 0)
  {
    printf("Erro ao criar segmento\n");
    exit(1);
  }

  matriz1 = (int (*)[TAM])shmat(segmento1, 0, 0);
  matriz2 = (int (*)[TAM])shmat(segmento2, 0, 0);
  matriz3 = (int (*)[TAM])shmat(segmento3, 0, 0);

  if (*matriz1 == (void *)-1 || *matriz2 == (void *)-1 || *matriz3 == (void *)-1)
  {
    perror("Erro ao anexar segmento");
    exit(1);
  }

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      matriz1[i][j] = rand() % 10;
      matriz2[i][j] = rand() % 10;
      matriz3[i][j] = 0;
    }
  }

  for (int i = 0; i < TAM; i++)
  {
    pid = fork();
    if (pid < 0)
    {
      printf("Erro ao criar processo filho\n");
      exit(1);
    }
    else if (pid == 0)
    { // Processo filho
      printf("Processo filho calculando linha %d\n", i + 1);
      for (int j = 0; j < 3; j++)
      {
        matriz3[i][j] = matriz1[i][j] + matriz2[i][j];
      }
      exit(0);
    }
    else
    { // Processo pai
      printf("Processo pai\n");
      pid = wait(&status);
    }
  }

  printf("Matriz 1:\n");
  for (int i = 0; i < TAM; i++)
  {
    for (int j = 0; j < TAM; j++)
    {
      printf("%d\t", matriz1[i][j]);
    }
    printf("\n");
  }

  printf("\nMatriz 2:\n");
  for (int i = 0; i < TAM; i++)
  {
    for (int j = 0; j < TAM; j++)
    {
      printf("%d\t", matriz2[i][j]);
    }
    printf("\n");
  }

  printf("\nMatriz Soma:\n");
  for (int i = 0; i < TAM; i++)
  {
    for (int j = 0; j < TAM; j++)
    {
      printf("%d\t", matriz3[i][j]);
    }
    printf("\n");
  }

  shmdt(matriz1);
  shmdt(matriz2);
  shmdt(matriz3);
  shmctl(segmento1, IPC_RMID, NULL);
  shmctl(segmento2, IPC_RMID, NULL);
  shmctl(segmento3, IPC_RMID, NULL);

  return 0;
}