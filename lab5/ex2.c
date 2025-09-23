#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
 
int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[1024];
 
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
 
    pid = fork();
 
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
 
    if (pid > 0) {
        // Processo PAI
        close(pipefd[0]);
 
        FILE *entrada = fopen("entrada.txt", "r");
        if (entrada == NULL) {
            perror("fopen entrada.txt");
            exit(EXIT_FAILURE);
        }
 
        while (fgets(buffer, sizeof(buffer), entrada) != NULL) {
            write(pipefd[1], buffer, strlen(buffer));
        }
 
        fclose(entrada);
        close(pipefd[1]); 
 
        wait(NULL);
 
    } else {
        // Processo FILHO
        close(pipefd[1]);
 
        FILE *saida = fopen("saida.txt", "w");
        if (saida == NULL) {
            perror("fopen saida.txt");
            exit(EXIT_FAILURE);
        }
 
        dup2(pipefd[0], STDIN_FILENO);
 
        close(pipefd[0]);
 
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            fprintf(saida, "%s", buffer);
        }
 
        fclose(saida);
        exit(EXIT_SUCCESS);
    }
 
    return 0;
}