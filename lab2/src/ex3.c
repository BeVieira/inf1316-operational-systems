#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

typedef struct {
    int n;       // tamanho do vetor
    int key;     // chave a buscar
    int data[];  // vetor flexível
} shm_block;

int main(int argc, char *argv[]) {
    int NPROCS = 4;                            // pelo menos 4
    int N = 100;                               // tamanho do vetor (ajuste se quiser)
    int key = (argc >= 2) ? atoi(argv[1]) : -1;

    // aloca memória compartilhada suficiente para struct + vetor
    int shmid = shmget(IPC_PRIVATE, sizeof(shm_block) + N * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    shm_block *blk = (shm_block*) shmat(shmid, NULL, 0);
    if ((void*)blk == (void*)-1) { perror("shmat"); exit(1); }

    // inicializa
    blk->n = N;
    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++) blk->data[i] = rand() % 1000;

    // se não passou chave, escolhe uma que com certeza existe (posição aleatória)
    if (key < 0) key = blk->data[rand() % N];
    blk->key = key;

    printf("BUSCA PARALELA — N=%d, processos=%d, key=%d\n", N, NPROCS, key);

    // divide o vetor em faixas
    int chunk = (N + NPROCS - 1) / NPROCS;

    // cria filhos
    for (int p = 0; p < NPROCS; p++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            // Filho: busca na sua faixa
            int start = p * chunk;
            int end   = start + chunk;
            if (end > N) end = N;

            for (int i = start; i < end; i++) {
                if (blk->data[i] == blk->key) {
                    printf("[PID %d] Encontrou key=%d na posicao %d (faixa %d..%d)\n",
                           getpid(), blk->key, i, start, end-1);
                    // não sai imediatamente: poderia haver várias ocorrências; remova o break se quiser listar todas
                    // break;
                }
            }
            // desanexa e sai
            shmdt(blk);
            _exit(0);
        }
    }

    // Pai: espera todos
    for (int p = 0; p < NPROCS; p++) wait(NULL);

    // limpeza
    shmdt(blk);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
