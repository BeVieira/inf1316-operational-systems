#define KEY_SHM 1234
#define KEY_SEM 5678

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void P(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void V(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}
