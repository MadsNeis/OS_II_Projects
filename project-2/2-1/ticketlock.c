// Madison Neiswonger
// CS 474

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#define MAX_THREADS 20

atomic_int next_ticket = 0;
atomic_int now_serving = 0;

void lock(int tid) {
    int my_ticket = atomic_fetch_add(&next_ticket, 1);

    printf("Thread %d: Grabbed ticket %d\n", tid, my_ticket);

    while (atomic_load(&now_serving) != my_ticket) {
    }
}

void unlock(void) {
    atomic_fetch_add(&now_serving, 1);
}

void *run(void *args)
{
    int id = *(int*)args;

    printf("Thread %d: Locking\n", id);
    lock(id);

    printf("Thread %d: Running\n", id);
    usleep(50000);

    printf("Thread %d: Unlocking\n", id);
    unlock();

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t[MAX_THREADS];
    int id[MAX_THREADS];

    if (argc != 2) {
        fprintf(stderr, "usage: ticketlock threadcount\n");
        return 1;
    }

    int thread_count = atoi(argv[1]);

    if (thread_count < 1 || thread_count > MAX_THREADS) {
        fprintf(stderr, "ticketlock: thread count must be betweeen 1 and %d\n",
            MAX_THREADS);
        return 2;
    }

    for (int i = 0; i < thread_count; i++) {
        id[i] = i;
        pthread_create(t + i, NULL, run, id + i);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(t[i], NULL);
    }
}