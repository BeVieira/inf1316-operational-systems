#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define EVER ;;

static volatile sig_atomic_t start_flag = 0;
static volatile sig_atomic_t stop_flag = 0;
static volatile sig_atomic_t exit_flag = 0;
static enum { IDLE=0, IN_CALL=1 } state = IDLE;
static struct timespec t_start;

void set_flag(int signo) {
    if (signo == SIGUSR1) start_flag = 1;
    else if (signo == SIGUSR2) stop_flag = 1;
    else if (signo == SIGTERM || signo == SIGINT) exit_flag = 1;
}

long diff_seconds(struct timespec a, struct timespec b) {
    long sec = b.tv_sec - a.tv_sec;
    long nsec = b.tv_nsec - a.tv_nsec;
    if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
    return sec;
}

void compute_and_print_price(long secs) {
    long first = secs > 60 ? 60 : secs;
    long rest  = secs > 60 ? secs - 60 : 0;
    long cents = first * 2 + rest * 1;
    long reais = cents / 100;
    long centavos = cents % 100;
    printf("Ligação encerrada: duração = %lds | Preço = R$%ld,%02ld\n", secs, reais, centavos);
    fflush(stdout);
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = set_flag;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);

    fflush(stdout);

    for(EVER) {
        pause();
        if (exit_flag) exit(0);
        if (start_flag) {
            start_flag = 0;
            if (state == IDLE) {
                clock_gettime(CLOCK_MONOTONIC, &t_start);
                state = IN_CALL;
                printf("Chamada iniciada\n");
                fflush(stdout);
            }
        }
        if (stop_flag) {
            stop_flag = 0;
            if (state == IN_CALL) {
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                long secs = diff_seconds(t_start, now);
                compute_and_print_price(secs);
                state = IDLE;
            }
        }
    }
}
