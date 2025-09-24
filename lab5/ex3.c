#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid1, pid2;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Criar o primeiro processo (produtor: ps)
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        // Processo filho 1
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);

        execlp("ps", "ps", NULL);
        perror("execlp ps");
        exit(1);
    }

    // Criar o segundo processo (consumidor: wc)
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {
        // Processo filho 2
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]); 
        close(fd[0]); 

        execlp("wc", "wc", NULL);
        perror("execlp wc");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
