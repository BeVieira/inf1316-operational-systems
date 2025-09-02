#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY 8752

int main(int argc, char *argv[]){
    int segmento;
    char *shmaddr;

    segmento = shmget(KEY, 1024, 0666);
    if (segmento == -1) {
        perror("Erro em shmget");
        exit(1);
    }

    shmaddr = (char *) shmat(segmento, NULL, 0);
    if (shmaddr == (char *) -1) {
        perror("Erro em shmat");
        exit(1);
    }

    printf("Mensagem lida da memória compartilhada: %s\n", shmaddr);

    shmdt(shmaddr);

    return 0;
}