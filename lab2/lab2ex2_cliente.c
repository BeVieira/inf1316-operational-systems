#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY 8752

int main(int argc, char *argv[]){
    int segmento;
    char *endereco;

    segmento = shmget(KEY, 1024, 0666);
    if (segmento == -1) {
        perror("Erro ao criar o segmento de memória compartilhada");
        exit(1);
    }

    endereco = (char *) shmat(segmento, NULL, 0);
    if (endereco == (char *) -1) {
        perror("Erro ao anexar a memória compartilhada");
        exit(1);
    }

    printf("Mensagem lida da memória compartilhada: %s\n", endereco);

    shmdt(endereco);

    return 0;
}