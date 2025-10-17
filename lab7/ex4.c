#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAXFILA 8
#define TOTAL_ELEMENTOS 64
#define NUM_PRODUTORES 2
#define NUM_CONSUMIDORES 2

int buffer[MAXFILA];
int in = 0;
int out = 0;
int count = 0;

pthread_mutex_t lock;
pthread_cond_t cheio, vazio;

void *produtor(void *arg) {
    int id = *(int *)arg;
    for (int i = 1; i <= TOTAL_ELEMENTOS / NUM_PRODUTORES; i++) {
        pthread_mutex_lock(&lock);

        while (count == MAXFILA) {
            pthread_cond_wait(&cheio, &lock);
        }

        int item = rand() % 1000;
        buffer[in] = item;
        in = (in + 1) % MAXFILA;
        count++;

        printf("Produtor %d produziu: %d (itens na fila: %d)\n", id, item, count);

        pthread_cond_signal(&vazio);
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    int id = *(int *)arg;
    for (int i = 1; i <= TOTAL_ELEMENTOS / NUM_CONSUMIDORES; i++) {
        pthread_mutex_lock(&lock);

        while (count == 0) {
            pthread_cond_wait(&vazio, &lock);
        }

        int item = buffer[out];
        out = (out + 1) % MAXFILA;
        count--;

        printf("Consumidor %d consumiu: %d (itens na fila: %d)\n", id, item, count);

        pthread_cond_signal(&cheio);
        pthread_mutex_unlock(&lock);

        sleep(2);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t produtores[NUM_PRODUTORES];
    pthread_t consumidores[NUM_CONSUMIDORES];
    int ids_prod[NUM_PRODUTORES];
    int ids_cons[NUM_CONSUMIDORES];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cheio, NULL);
    pthread_cond_init(&vazio, NULL);

    printf("Iniciando múltiplos produtores e consumidores...\n");

    for (int i = 0; i < NUM_PRODUTORES; i++) {
        ids_prod[i] = i + 1;
        pthread_create(&produtores[i], NULL, produtor, &ids_prod[i]);
    }

    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids_cons[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &ids_cons[i]);
    }

    for (int i = 0; i < NUM_PRODUTORES; i++)
        pthread_join(produtores[i], NULL);

    for (int i = 0; i < NUM_CONSUMIDORES; i++)
        pthread_join(consumidores[i], NULL);

    printf("Todos os produtores e consumidores terminaram.\n");

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cheio);
    pthread_cond_destroy(&vazio);

    return 0;
}