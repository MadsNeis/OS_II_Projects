#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#define MAX_THREADS 10
#define WRITER_BIT (1 << 31)

_Atomic(int) lock_var = 0;
_Atomic(int) reader_count;
int writer_count;

void read_lock(void)
{
    int expected;
    do {
        do {
            expected = atomic_load(&lock_var);
        } while (expected & WRITER_BIT);

    } while (!atomic_compare_exchange_weak(&lock_var, &expected, expected + 1));
}

void read_unlock(void)
{
    atomic_fetch_sub(&lock_var, 1);
}

void write_lock(void)
{
    int expected;
    do {
        do {
            expected = atomic_load(&lock_var);
        } while (expected != 0);

    } while (!atomic_compare_exchange_weak(&lock_var, &expected, WRITER_BIT));
}

void write_unlock(void)
{
    atomic_fetch_and(&lock_var, ~WRITER_BIT);
}

// Do not change the code below this point!

void *run_reader(void *args)
{
    int id = *(int*)args;

    read_lock();
    printf("Reader thread %d: In reader critical section\n", id);
    reader_count++;
    printf("%d reader threads in the reader critical section\n", reader_count);
    usleep(50000);
    reader_count--;
    read_unlock();

    return NULL;
}

void *run_writer(void *args)
{
    int id = *(int*)args;

    write_lock();
    printf("Writer thread %d: In critical section\n", id);
    writer_count++;
    if (writer_count > 1)
        printf("Yikes! %d writers in the writer critical section!\n",
            writer_count);
    if (reader_count > 0)
        printf("Yikes! %d readers in the writer critical section!\n",
            reader_count);
    usleep(50000);
    writer_count--;
    write_unlock();

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tr[MAX_THREADS], tw[MAX_THREADS];
    int idr[MAX_THREADS], idw[MAX_THREADS];

    if (argc != 3) {
        fprintf(stderr, "usage: %s readercount writercount\n", argv[0]);
        return 1;
    }

    int reader_thread_count = atoi(argv[1]);
    int writer_thread_count = atoi(argv[2]);

    if (reader_thread_count < 1 || reader_thread_count > MAX_THREADS) {
        fprintf(stderr, "%s: reader thread count must be betweeen 1 and %d\n",
            argv[0], MAX_THREADS);
        return 2;
    }

    if (writer_thread_count < 1 || writer_thread_count > MAX_THREADS) {
        fprintf(stderr, "%s: writer thread count must be betweeen 1 and %d\n",
            argv[0], MAX_THREADS);
        return 2;
    }

    for (int i = 0; i < reader_thread_count; i++) {
        idr[i] = i;
        pthread_create(tr + i, NULL, run_reader, idr + i);
    }

    for (int i = 0; i < writer_thread_count; i++) {
        idw[i] = 1000 + i;
        pthread_create(tw + i, NULL, run_writer, idw + i);
    }

    for (int i = 0; i < reader_thread_count; i++) {
        pthread_join(tr[i], NULL);
    }

    for (int i = 0; i < writer_thread_count; i++) {
        pthread_join(tw[i], NULL);
    }
}