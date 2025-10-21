#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
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

static void ensure_fifo(const char *path){
    struct stat st;
    if (stat(path, &st) == 0) {
        if (!S_ISFIFO(st.st_mode)) {
            fprintf(stderr, "[IRQ] %s existe e não é FIFO\n", path);
            exit(1);
        }
    } else {
        if (mkfifo(path, 0600) < 0 && errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    int period_ms = parse_arg_int("--period", 500, argc, argv);
    int p1 = parse_arg_int("--p1", 10, argc, argv); // prob% IRQ1
    int p2 = parse_arg_int("--p2", 5,  argc, argv); // prob% IRQ2
    int seed = parse_arg_int("--seed", 777, argc, argv);
    if (p1 < 0) p1=0; if (p2<0) p2=0;
    srand((unsigned)seed);

    ensure_fifo(FIFO_IRQ);
    int fd_irq = -1;
    for (int tries=0; tries<50 && fd_irq<0; tries++){
        fd_irq = open(FIFO_IRQ, O_WRONLY);
        if (fd_irq < 0) usleep(20000);
    }
    if (fd_irq < 0) { perror("[IRQ] open FIFO_IRQ"); return 2; }

    printf("[IRQ] InterController v%s running period=%dms p1=%d%% p2=%d%%\n",
           KERNELSIM_VERSION, period_ms, p1, p2);

    char line[16];
    while (1) {
        // IRQ0 (timer)
        strcpy(line, "IRQ0\n"); write(fd_irq, line, strlen(line));
        // IRQ1?
        if ((rand()%100) < p1) { strcpy(line, "IRQ1\n"); write(fd_irq, line, strlen(line)); }
        // IRQ2?
        if ((rand()%100) < p2) { strcpy(line, "IRQ2\n"); write(fd_irq, line, strlen(line)); }
        usleep(period_ms * 1000);
    }
    return 0;
}