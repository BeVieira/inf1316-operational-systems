#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "memoria.h"

int main() {
    int shmid = shmget(KEY_SHM, sizeof(int), 0666);
    int *valor = (int *) shmat(shmid, NULL, 0);
    int semid = semget(KEY_SEM, 1, 0666);

    while (1) {
        P(semid);
        (*valor) += 5;
        printf("Somou 5 → valor = %d\n", *valor);
        V(semid);
        sleep(1);
    }
}
