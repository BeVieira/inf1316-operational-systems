#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int segmento1, segmento2, segmento3, contador;
    int *matriz1, *matriz2, *matriz3, *cont;
    int id,status;

    segmento1 = shmget(IPC_PRIVATE, 9*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento2 = shmget(IPC_PRIVATE, 9*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento3 = shmget(IPC_PRIVATE, 9*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    contador = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    matriz2 = (int (*)[3])shmat(segmento2, 0, 0);
    matriz3 = (int (*)[3])shmat(segmento3, 0, 0);
    matriz1 = (int (*)[3])shmat(segmento1, 0, 0);
    cont = (int *)shmat(contador, 0, 0);
    *cont = 0;

    if (matriz1  < 0 || matriz2 < 0 || matriz3 < 0 || contador < 0){
        printf("Erro ao criar segmento\n");
        exit(1);
    }
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            matriz1[i][j] = rand()%10;
            matriz2[i][j] = rand()%10;
            matriz3[i][j] = 0;
        }
    }

    for (int i = 0; i < 3; i++){
        id = fork()
        for (int j = 0; j < 3; j++){
            printf("%d\t", matriz1[i][j]);
        }
        printf("\n");
    }
    
    if (id = fork() < 0){
        printf("Erro ao criar processo filho\n");
        exit(1);
    } else if (id == 0){ //Processo filho
        for (int j = 0; j < 3; j++){
            matriz3[cont][j] = matriz1[cont][j] + matriz2[cont][j];
        }
    } else { //Processo pai
        id = wait (&status);
        *cont++;
        if (*cont < 3) {
            id = fork();
        }
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                printf("%d\t", matriz3[i][j]);
            }
            printf("\n");
        }
    }
}