#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define MAX_PROG_LEN 10 // Max terms in a "program"
#define MAX_PROCS 10 // Max number of processes
#define QUANTUM 40 // Time quantum, ms

#define MIN(x,y) ((x)<(y)?(x):(y)) // Compute the minimum

enum process_state {
    READY,
    SLEEPING,
    EXITED
};

struct process {
    int pid;
    enum process_state state;
    int sleep_time_remaining;
    int awake_time_remaining;
    int program_counter;
    int program[MAX_PROG_LEN + 1];
};

struct process process_table[MAX_PROCS];


void parse_program(char *arg, int pid)
{
    char *token;
    int i = 0;

    if ((token = strtok(arg, ",")) != NULL) {
        do {
            process_table[pid].program[i++] = atoi(token);
        } while ((token = strtok(NULL, ",")) != NULL);
    }
    process_table[pid].program[i] = 0;
}


void initialize_process(int pid)
{
    process_table[pid].pid = pid;
    process_table[pid].state = READY;
    process_table[pid].sleep_time_remaining = 0;
    process_table[pid].awake_time_remaining = process_table[pid].program[0];
    process_table[pid].program_counter = 0;
}


int all_exited(int num_procs)
{
    for (int i = 0; i < num_procs; i++) {
        if (process_table[i].state != EXITED) {
            return 0;
        }
    }
    return 1;
}


void wake_up_processes(struct queue *q, int num_procs)
{
    for (int i = 0; i < num_procs; i++) {
        if (process_table[i].state == SLEEPING &&
            process_table[i].sleep_time_remaining <= 0) {

            process_table[i].program_counter++;

            if (process_table[i].program[process_table[i].program_counter] == 0) {
                process_table[i].state = EXITED;
                printf("PID %d: Exiting\n", process_table[i].pid);
            } else {
                process_table[i].state = READY;
                process_table[i].awake_time_remaining =
                    process_table[i].program[process_table[i].program_counter];
                printf("PID %d: Waking up for %d ms\n",
                    process_table[i].pid, process_table[i].awake_time_remaining);
                queue_enqueue(q, &process_table[i]);
            }
        }
    }
}


int find_next_wakeup_time(int num_procs)
{
    int min_time = -1;

    for (int i = 0; i < num_procs; i++) {
        if (process_table[i].state == SLEEPING) {
            if (min_time == -1 || process_table[i].sleep_time_remaining < min_time) {
                min_time = process_table[i].sleep_time_remaining;
            }
        }
    }

    return min_time;
}


void advance_time(int time_delta, int num_procs, int *clock)
{
    *clock += time_delta;

    for (int i = 0; i < num_procs; i++) {
        if (process_table[i].state == SLEEPING) {
            process_table[i].sleep_time_remaining -= time_delta;
        }
    }
}


void run_process(struct process *p, struct queue *q, int num_procs, int *clock)
{
    int run_time = MIN(QUANTUM, p->awake_time_remaining);

    p->awake_time_remaining -= run_time;

    for (int i = 0; i < num_procs; i++) {
        if (process_table[i].state == SLEEPING) {
            process_table[i].sleep_time_remaining -= run_time;
        }
    }

    *clock += run_time;

    if (p->awake_time_remaining > 0) {
        queue_enqueue(q, p);
    } else {
        p->program_counter++;

        if (p->program[p->program_counter] == 0) {
            p->state = EXITED;
            printf("PID %d: Exiting\n", p->pid);
        } else {
            p->state = SLEEPING;
            p->sleep_time_remaining = p->program[p->program_counter];
            printf("PID %d: Sleeping for %d ms\n", p->pid, p->sleep_time_remaining);
        }
    }

    printf("PID %d: Ran for %d ms\n", p->pid, run_time);
}

/**
 * Main.
 */
int main(int argc, char **argv)
{
    int clock = 0;
    int num_procs = argc - 1;

    struct queue *q = queue_new();


    for (int i = 0; i < num_procs; i++) {
        parse_program(argv[i + 1], i);
        initialize_process(i);
    }

    for (int i = 0; i < num_procs; i++) {
        queue_enqueue(q, &process_table[i]);
    }

    while (!all_exited(num_procs)) {
        if (queue_is_empty(q)) {
            int next_wakeup = find_next_wakeup_time(num_procs);
            advance_time(next_wakeup, num_procs, &clock);
        }

        printf("=== Clock %d ms ===\n", clock);
        wake_up_processes(q, num_procs);

        struct process *runner = queue_dequeue(q);
        if (runner == NULL) {
            continue;
        }

        printf("PID %d: Running\n", runner->pid);
        run_process(runner, q, num_procs, &clock);
    }
    queue_free(q);
}
