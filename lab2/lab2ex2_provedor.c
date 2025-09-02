#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY 8752

int main(int argc, char *argv[]){
    int segmento;
    char *shmaddr;
    char mensagem[1024];

    segmento = shmget(KEY, 1024, IPC_CREAT | 0666);
    if (segmento == -1) {
        perror("shmget");
        exit(1);
    }

    shmaddr = shmat(segmento, NULL, 0);
    if (shmaddr == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    printf("Digite uma mensagem: ");
    fgets(mensagem, 1024, stdin);


    strcpy(shmaddr, mensagem);

    printf("Mensagem salva na memória compartilhada: %s\n", shmaddr);

    shmdt(shmaddr);

    return 0;
}