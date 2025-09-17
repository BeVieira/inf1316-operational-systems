#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void tratador(int sinal) {
    printf("Sinal %d recebido!\n", sinal);
    fflush(stdout);
}

int main(void) {
    void (*p)(int);

    p = signal(SIGKILL, tratador);

    printf("PID = %d\n", getpid());
    fflush(stdout);
    
    return 0;
}
