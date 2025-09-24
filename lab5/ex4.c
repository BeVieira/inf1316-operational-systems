#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int fd[2];
    pid_t pid1, pid2, pid_writer;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Criar primeiro leitor
    pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(1);
    }
    if (pid1 == 0) {
        // Filho leitor 1
        close(fd[1]);
        char buffer[64];
        while (1) {
            int n = read(fd[0], buffer, sizeof(buffer)-1);
            if (n <= 0) break;
            buffer[n] = '\0';
            printf("Leitor 1 leu: %s", buffer);
            fflush(stdout);
            sleep(2); 
        }
        close(fd[0]);
        exit(0);
    }

    // Criar segundo leitor
    pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }
    if (pid2 == 0) {
        // Filho leitor 2
        close(fd[1]);
        char buffer[64];
        while (1) {
            int n = read(fd[0], buffer, sizeof(buffer)-1);
            if (n <= 0) break;
            buffer[n] = '\0';
            printf("Leitor 2 leu: %s", buffer);
            fflush(stdout);
            sleep(2);
        }
        close(fd[0]);
        exit(0);
    }

    // Criar escritor
    pid_writer = fork();
    if (pid_writer == -1) {
        perror("fork");
        exit(1);
    }
    if (pid_writer == 0) {
        // Filho escritor
        close(fd[0]);
        int i = 1;
        while (i <= 10) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Mensagem %d\n", i++);
            write(fd[1], msg, strlen(msg));
            printf("Escritor escreveu: %s", msg);
            fflush(stdout);
            sleep(1); 
        }
        close(fd[1]);
        exit(0);
    }

    // Processo pai
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}
