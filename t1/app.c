#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "common.h"

static int parse_arg_int(const char *key, int def, int argc, char **argv) {
    size_t klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return atoi(argv[i] + (int)klen + 1);
        }
    }
    return def;
}

int main(int argc, char **argv) {
    const char *name = (argc > 1 && argv[1] && argv[1][0]) ? argv[1] : "A?";
    int maxiter = parse_arg_int("--max", 20, argc, argv);
    int seed = parse_arg_int("--seed", 123, argc, argv);
    int p_io = 15; // % chance de syscall por iteração

    srand((unsigned)seed ^ (unsigned)getpid());

    // tenta abrir a FIFO criada pelo kernel
    int fd_req = -1;
    for (int tries=0; tries<50 && fd_req<0; tries++){
        fd_req = open(FIFO_SYSREQ, O_WRONLY);
        if (fd_req < 0) usleep(20000);
    }
    if (fd_req < 0) { perror("[APP] open FIFO_SYSREQ"); return 2; }

    // começa parado; kernel dá SIGCONT
    raise(SIGSTOP);

    for (int i = 1; i <= maxiter; i++) {
        printf("[%s] tick %d/%d (pid=%d)\n", name, i, maxiter, getpid());
        fflush(stdout);

        if ((rand()%100) < p_io) {
            int dev = (rand()%100 < 67) ? 1 : 2;
            char ops[4] = {OP_R, OP_W, OP_X, OP_R};
            char op = ops[rand()%4];
            char line[128];
            int n = snprintf(line, sizeof(line),
                "SYS pid=%d dev=%d op=%c pc=%d name=%s\n",
                (int)getpid(), dev, op, i, name);
            if (write(fd_req, line, n) >= 0) {
                printf("[%s] syscall D%d %c (PC=%d)\n", name, dev, op, i);
            }
            // bloqueia até kernel/IRQ nos acordar
            raise(SIGSTOP);
        }

        usleep(90000); // trabalho "CPU"
    }
    printf("[%s] done.\n", name);
    close(fd_req);
    return 0;
}