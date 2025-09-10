#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void trataDivisaoPorZero(int sinal);

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <num1> <num2>\n", argv[0]);
        return 1;
    }
    
    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    signal(SIGFPE, trataDivisaoPorZero);
    
    printf("Soma: %d + %d = %d\n", num1, num2, num1 + num2);
    printf("Subtração: %d - %d = %d\n", num1, num2, num1 - num2);
    printf("Multiplicação: %d * %d = %d\n", num1, num2, num1 * num2);
    
    printf("Divisão: %d / %d = ", num1, num2);
    printf("%d\n", num1 / num2);

    return 0;
}

void trataDivisaoPorZero(int sinal) {
    printf("Erro: Divisão por zero não é permitida.\n");
    exit(1);
}   
