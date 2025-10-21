#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "common.h"

typedef enum { P_READY=0, P_RUNNING=1, P_BLOCK=2, P_TERM=3 } pstate_t;

typedef struct {
    pid_t pid;
    char  name[16];
    pstate_t st;
    int pc_iter;
    int preemptions;
    int d1_count;
    int d2_count;
    char blk_op;
    int  blk_dev;
} pcb_t;

// fila circular simples
typedef struct {
    int q[128];
    int head, tail, size, cap;
} q_t;

static void q_init(q_t *q, int cap) { q->head=q->tail=q->size=0; q->cap=cap; }
static int q_empty(q_t *q){ return q->size==0; }
static int q_push(q_t *q, int v){ if(q->size==q->cap) return -1; q->q[q->tail]=(v); q->tail=(q->tail+1)%q->cap; q->size++; return 0; }
static int q_pop(q_t *q){ if(q->size==0) return -1; int v=q->q[q->head]; q->head=(q->head+1)%q->cap; q->size--; return v; }

static volatile sig_atomic_t tick_flag = 0;
static volatile sig_atomic_t sigint_flag = 0;

static void on_alarm(int signo){ (void)signo; tick_flag = 1; }
static void on_sigint(int signo){ (void)signo; sigint_flag = 1; }

static int parse_arg_int(const char *key, int def, int argc, char **argv) {
    size_t klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return atoi(argv[i] + (int)klen + 1);
        }
    }
    return def;
}

static int parse_arg_str(const char *key, const char *def, int argc, char **argv, char *out, size_t outsz) {
    size_t klen = strlen(key);
    const char *val = def;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            val = argv[i] + (int)klen + 1;
            break;
        }
    }
    if (out && outsz) snprintf(out, outsz, "%s", val);
    return 0;
}

static void ensure_fifo(const char *path){
    struct stat st;
    if (stat(path, &st) == 0) {
        if (!S_ISFIFO(st.st_mode)) {
            fprintf(stderr, "[KERNEL] %s existe e não é FIFO\n", path);
            exit(1);
        }
    } else {
        if (mkfifo(path, 0600) < 0 && errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
}

static void reap_children(pcb_t *tbl, int n, int *alive) {
    int status;
    while (1) {
        pid_t done = waitpid(-1, &status, WNOHANG);
        if (done <= 0) break;
        for (int i = 0; i < n; i++) {
            if (tbl[i].pid == done && tbl[i].st != P_TERM) {
                tbl[i].st = P_TERM;
                (*alive)--;
                if (WIFEXITED(status)) {
                    printf("[KERNEL] %-3s PID=%d EXIT=%d (alive=%d)\n",
                           tbl[i].name, (int)done, WEXITSTATUS(status), *alive);
                } else if (WIFSIGNALED(status)) {
                    printf("[KERNEL] %-3s PID=%d SIG=%d (alive=%d)\n",
                           tbl[i].name, (int)done, WTERMSIG(status), *alive);
                } else {
                    printf("[KERNEL] %-3s PID=%d ended (alive=%d)\n",
                           tbl[i].name, (int)done, *alive);
                }
                break;
            }
        }
    }
}

static void print_status(pcb_t *tbl, int n, int running_idx) {
    printf("\n=== STATUS (SIGINT) ===\n");
    printf("Idx  Name  PID     State      PC   D1   D2   Preempt  Running?\n");
    for (int i = 0; i < n; i++) {
        const char *st = (tbl[i].st==P_READY?"READY":tbl[i].st==P_RUNNING?"RUNNING":tbl[i].st==P_BLOCK?"BLOCK":"TERM");
        printf("%-4d %-4s %-7d %-9s %-4d %-4d %-4d %-8d %s",
               i, tbl[i].name, (int)tbl[i].pid, st, tbl[i].pc_iter,
               tbl[i].d1_count, tbl[i].d2_count, tbl[i].preemptions, (i==running_idx?"<--":""));
        if (tbl[i].st==P_BLOCK) printf("  (D%d/%c)", tbl[i].blk_dev, tbl[i].blk_op);
        printf("\n");
    }
    printf("=======================\n\n");
}

int main(int argc, char **argv) {
    int N = parse_arg_int("--n", 5, argc, argv);
    int slice = parse_arg_int("--slice", 250, argc, argv);
    int maxiter = parse_arg_int("--max",  20,  argc, argv);
    int seed = parse_arg_int("--seed", 42,  argc, argv);
    char timer_mode[16]; parse_arg_str("--timer", "internal", argc, argv, timer_mode, sizeof(timer_mode));
    if (N <= 0) N = 5; if (slice < 10) slice = 10; if (maxiter < 1) maxiter = 5;

    printf("[KERNEL] KernelSim v%s — Etapas 4–8 (preempção, IPC, IRQ, inspeção, relatório)\n", KERNELSIM_VERSION);
    printf("[KERNEL] N=%d slice=%dms max=%d seed=%d timer=%s\n", N, slice, maxiter, seed, timer_mode);

    srand((unsigned)seed);

    pcb_t *tbl = calloc(N, sizeof(pcb_t));
    if (!tbl) { perror("calloc"); return 1; }

    // FIFOs para syscalls e IRQs
    ensure_fifo(FIFO_SYSREQ);
    ensure_fifo(FIFO_IRQ);

    int fd_sysreq_r = open(FIFO_SYSREQ, O_RDONLY | O_NONBLOCK);
    int fd_sysreq_w_keep = open(FIFO_SYSREQ, O_WRONLY | O_NONBLOCK); // mantém aberto
    int fd_irq_r = open(FIFO_IRQ, O_RDONLY | O_NONBLOCK);
    int fd_irq_w_keep = open(FIFO_IRQ, O_WRONLY | O_NONBLOCK); // mantém aberto
    if (fd_sysreq_r < 0 || fd_irq_r < 0) { perror("open FIFO (read)"); return 1; }

    // cria apps (cada um começa com SIGSTOP)
    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {
            char name[16], maxs[32], seeds[32];
            snprintf(name, sizeof(name), "A%d", i+1);
            snprintf(maxs, sizeof(maxs), "--max=%d", maxiter);
            snprintf(seeds, sizeof(seeds), "--seed=%u", (unsigned)seed + (unsigned)i*97u);
            execl("./app", "app", name, maxs, seeds, (char*)NULL);
            execlp("app", "app", name, maxs, seeds, (char*)NULL);
            perror("exec app");
            _exit(127);
        }
        tbl[i].pid = pid;
        snprintf(tbl[i].name, sizeof(tbl[i].name), "A%d", i+1);
        tbl[i].st  = P_READY;
        tbl[i].pc_iter = 0;
        tbl[i].preemptions = 0;
        tbl[i].d1_count = tbl[i].d2_count = 0;
        tbl[i].blk_op = 0; tbl[i].blk_dev = 0;
        printf("[KERNEL] forked %-3s => PID=%d\n", tbl[i].name, (int)pid);
    }

    // timer interno (SIGALRM) e SIGINT (inspeção)
    struct sigaction sa = {0};
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    if (strcmp(timer_mode, "internal")==0) {
        sigaction(SIGALRM, &sa, NULL);
        struct itimerval tv;
        tv.it_interval.tv_sec = slice/1000;
        tv.it_interval.tv_usec = (slice%1000)*1000;
        tv.it_value = tv.it_interval;
        setitimer(ITIMER_REAL, &tv, NULL);
    }
    struct sigaction sb = {0};
    sb.sa_handler = on_sigint;
    sigemptyset(&sb.sa_mask);
    sigaction(SIGINT, &sb, NULL);

    // filas
    q_t ready; q_init(&ready, 128);
    q_t qd1; q_init(&qd1, 128);
    q_t qd2; q_init(&qd2, 128);
    for (int i = 0; i < N; i++) q_push(&ready, i);

    int alive = N;
    int running = -1;

    // inicia o primeiro
    if (!q_empty(&ready)) {
        running = q_pop(&ready);
        kill(tbl[running].pid, SIGCONT);
        tbl[running].st = P_RUNNING;
        printf("[KERNEL] RUN %-3s  (PID=%d)\n", tbl[running].name, (int)tbl[running].pid);
    }

    char buf[256];
    while (alive > 0) {
        // coleta filhos que terminaram
        reap_children(tbl, N, &alive);
        if (alive == 0) break;

        // lê syscalls
        ssize_t n = read(fd_sysreq_r, buf, sizeof(buf)-1);
        if (n > 0) {
            buf[n] = 0;
            char *line = strtok(buf, "\n");
            while (line) {
                pid_t rpid=0; int dev=0; char op=0; int pc=0; char nm[16]={0};
                if (sscanf(line, "SYS pid=%d dev=%d op=%c pc=%d name=%15s", &rpid, &dev, &op, &pc, nm) == 5) {
                    for (int i = 0; i < N; i++) if (tbl[i].pid == rpid && tbl[i].st != P_TERM) {
                        tbl[i].pc_iter = pc;
                        tbl[i].blk_op = op; tbl[i].blk_dev = dev;
                        tbl[i].st = P_BLOCK;
                        if (i == running) { kill(tbl[i].pid, SIGSTOP); running = -1; }
                        if (dev==1) q_push(&qd1, i); else q_push(&qd2, i);
                        printf("[KERNEL] BLOCK %-3s on D%d/%c (PC=%d)\n", tbl[i].name, dev, op, pc);
                        break;
                    }
                }
                line = strtok(NULL, "\n");
            }
        }

        // lê IRQs
        ssize_t m = read(fd_irq_r, buf, sizeof(buf)-1);
        if (m > 0) {
            buf[m] = 0;
            char *line = strtok(buf, "\n");
            while (line) {
                if (!strncmp(line, "IRQ0", 4)) {
                    tick_flag = 1; // timer externo para teste
                } else if (!strncmp(line, "IRQ1", 4)) {
                    int idx = q_pop(&qd1);
                    if (idx >= 0 && tbl[idx].st == P_BLOCK) {
                        tbl[idx].st = P_READY; tbl[idx].blk_op=0; tbl[idx].blk_dev=0;
                        tbl[idx].d1_count++;
                        q_push(&ready, idx);
                        printf("[IRQ1] D1 done -> READY %-3s\n", tbl[idx].name);
                    }
                } else if (!strncmp(line, "IRQ2", 4)) {
                    int idx = q_pop(&qd2);
                    if (idx >= 0 && tbl[idx].st == P_BLOCK) {
                        tbl[idx].st = P_READY; tbl[idx].blk_op=0; tbl[idx].blk_dev=0;
                        tbl[idx].d2_count++;
                        q_push(&ready, idx);
                        printf("[IRQ2] D2 done -> READY %-3s\n", tbl[idx].name);
                    }
                }
                line = strtok(NULL, "\n");
            }
        }

        // inspeção por SIGINT
        if (sigint_flag) { print_status(tbl, N, running); sigint_flag = 0; }

        // preempção (SIGALRM ou IRQ0)
        if (tick_flag) {
            tick_flag = 0;
            if (running >= 0 && tbl[running].st == P_RUNNING) {
                kill(tbl[running].pid, SIGSTOP);
                tbl[running].st = P_READY;
                tbl[running].preemptions++;
                q_push(&ready, running);
                printf("[KERNEL] preempt %-3s -> READY\n", tbl[running].name);
            }
            running = -1;
            while (!q_empty(&ready)) {
                int cand = q_pop(&ready);
                if (tbl[cand].st == P_READY) {
                    running = cand;
                    kill(tbl[running].pid, SIGCONT);
                    tbl[running].st = P_RUNNING;
                    printf("[KERNEL] RUN %-3s  (PID=%d)\n", tbl[running].name, (int)tbl[running].pid);
                    break;
                }
            }
        }

        // se timer externo, dá um descanso pro loop
        if (strcmp(timer_mode, "internal") != 0) usleep(2000);

        reap_children(tbl, N, &alive);
    }

    // relatório final
    printf("\n=== RELATÓRIO FINAL ===\n");
    int tot_pre=0, tot_d1=0, tot_d2=0;
    for (int i = 0; i < N; i++) {
        printf("%-3s pid=%d preempt=%d D1=%d D2=%d lastPC=%d\n",
               tbl[i].name, (int)tbl[i].pid, tbl[i].preemptions, tbl[i].d1_count, tbl[i].d2_count, tbl[i].pc_iter);
        tot_pre += tbl[i].preemptions; tot_d1 += tbl[i].d1_count; tot_d2 += tbl[i].d2_count;
    }
    printf("TOTAL: preempt=%d D1=%d D2=%d\n", tot_pre, tot_d1, tot_d2);
    printf("=======================\n");

    // cleanup
    close(fd_sysreq_r); if (fd_sysreq_w_keep>=0) close(fd_sysreq_w_keep);
    close(fd_irq_r); if (fd_irq_w_keep>=0) close(fd_irq_w_keep);
    free(tbl);
    return 0;
}