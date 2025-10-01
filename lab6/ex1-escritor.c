#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_NOME "minhaFifo"

int main() {
    int fd;
    char buffer[1024];

    fd = open(FIFO_NOME, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    printf("Escritor pronto. Digite suas mensagens (Ctrl+D para sair):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        write(fd, buffer, strlen(buffer));
    }

    printf("\nSaindo do escritor.\n");
    close(fd);

    return 0;
}