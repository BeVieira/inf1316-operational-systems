#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    pid_t p[3];

    p[0] = fork();
    if (p[0] == 0) execl("./infinito1", "infinito1", NULL);

    p[1] = fork();
    if (p[1] == 0) execl("./infinito2", "infinito2", NULL);

    p[2] = fork();
    if (p[2] == 0) execl("./infinito3", "infinito3", NULL);

    sleep(1);

    for (int i = 0; i < 3; i++) kill(p[i], SIGSTOP);

    int i = 0;
    while (1) {
        kill(p[i], SIGCONT);

        if (i == 0) sleep(1);
        else sleep(2);

        kill(p[i], SIGSTOP);

        i = (i + 1) % 3;
    }

    for (int i = 0; i < 3; i++) wait(NULL);
    return 0;
}