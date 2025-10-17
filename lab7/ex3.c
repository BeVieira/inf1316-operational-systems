#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAXFILA 8
#define TOTAL_ELEMENTOS 64

int buffer[MAXFILA];
int in = 0;
int out = 0;
int count = 0;

pthread_mutex_t lock;
pthread_cond_t cheio, vazio;

void *produtor(void *arg) {
    for (int i = 1; i <= TOTAL_ELEMENTOS; i++) {
        pthread_mutex_lock(&lock);

        while (count == MAXFILA) {
            pthread_cond_wait(&cheio, &lock);
        }

        buffer[in] = i;
        in = (in + 1) % MAXFILA;
        count++;

        printf("Produtor produziu: %d (itens na fila: %d)\n", i, count);

        pthread_cond_signal(&vazio);
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    for (int i = 1; i <= TOTAL_ELEMENTOS; i++) {
        pthread_mutex_lock(&lock);

        while (count == 0) {
            pthread_cond_wait(&vazio, &lock);
        }

        int item = buffer[out];
        out = (out + 1) % MAXFILA;
        count--;

        printf("Consumidor consumiu: %d (itens na fila: %d)\n", item, count);

        pthread_cond_signal(&cheio);
        pthread_mutex_unlock(&lock);

        sleep(2);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t tProdutor, tConsumidor;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cheio, NULL);
    pthread_cond_init(&vazio, NULL);

    printf("Iniciando produtor e consumidor...\n");

    pthread_create(&tProdutor, NULL, produtor, NULL);
    pthread_create(&tConsumidor, NULL, consumidor, NULL);

    pthread_join(tProdutor, NULL);
    pthread_join(tConsumidor, NULL);

    printf("Produção e consumo finalizados!\n");

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cheio);
    pthread_cond_destroy(&vazio);

    return 0;
}
