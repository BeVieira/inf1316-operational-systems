#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main(void) {
    pid_t f1, f2;

    if ((f1 = fork()) == 0) {
        while (1) {
            printf("Filho 1 em execução...\n");
            fflush(stdout);
            sleep(1);
        }
    }

    if ((f2 = fork()) == 0) {
        while (1) {
            printf("Filho 2 em execução...\n");
            fflush(stdout);
            sleep(1);
        }
    }

    
    sleep(1);            
    kill(f1, SIGSTOP);    
    int i;
    for (i = 0; i < 10; i++) {
        kill(f2, SIGCONT);
        sleep(1);
        kill(f2, SIGSTOP); 

        kill(f1, SIGCONT); 
        sleep(1);
        kill(f1, SIGSTOP); 
    }

    kill(f1, SIGKILL);
    kill(f2, SIGKILL);

    wait(NULL);
    wait(NULL);

    printf("Pai finalizou os filhos após 10 trocas.\n");
    return 0;
}
