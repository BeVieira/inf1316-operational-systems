#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

#define KEY_SHM 1234
#define KEY_SEM 5678
#define BUFFER_SIZE 16

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void P(int semid, int num) {
    struct sembuf op = {num, -1, 0};
    semop(semid, &op, 1);
}

void V(int semid, int num) {
    struct sembuf op = {num, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    int shmid = shmget(KEY_SHM, BUFFER_SIZE, 0666 | IPC_CREAT);
    char *buffer = (char *) shmat(shmid, NULL, 0);
    int semid = semget(KEY_SEM, 2, 0666 | IPC_CREAT); // sem[0]=leitor, sem[1]=impressor

    union semun semUnion;
    semUnion.val = 1; semctl(semid, 0, SETVAL, semUnion); // leitor começa
    semUnion.val = 0; semctl(semid, 1, SETVAL, semUnion); // impressor espera

    if (fork() == 0) { // Processo leitor
        while (1) {
            P(semid, 0);
            for (int i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = getchar();
                if (buffer[i] == EOF) exit(0);
            }
            V(semid, 1);
        }
    } else { // Processo impressor
        while (1) {
            P(semid, 1);
            write(1, buffer, BUFFER_SIZE);
            fflush(stdout);
            V(semid, 0);
        }
    }
    return 0;
}
