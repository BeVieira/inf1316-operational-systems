#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "memoria.h"

int main() {
    int shmid = shmget(KEY_SHM, sizeof(int), 0666 | IPC_CREAT);
    int *valor = (int *) shmat(shmid, NULL, 0);

    int semid = semget(KEY_SEM, 1, 0666 | IPC_CREAT);
    union semun semUnion;
    semUnion.val = 1;
    semctl(semid, 0, SETVAL, semUnion);

    *valor = 0;

    while (1) {
        P(semid);
        (*valor) += 1;
        printf("Somou 1 → valor = %d\n", *valor);
        V(semid);
        sleep(1);
    }
}
