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
    int bytes_lidos;

    if (mkfifo(FIFO_NOME, S_IRUSR | S_IWUSR) == -1) {
        perror("mkfifo");
    }

    printf("Aguardando um escritor...\n");
    
    fd = open(FIFO_NOME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("Leitor pronto. Lendo mensagens da FIFO:\n");

    while ((bytes_lidos = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_lidos] = '\0';
        printf("Recebido: %s", buffer);
    }

    close(fd);
    unlink(FIFO_NOME);

    return 0;
}