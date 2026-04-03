#include "queue.h"
#include <stdlib.h>

/**
 * Construct a new queue.
 */
struct queue *queue_new(void) 
{
    struct queue *q = malloc(sizeof *q);
    q->head = NULL;

    return q;
}

/**
 * Free a queue.
 *
 * Frees all the nodes (but not their payloads!)
 */
void queue_free(struct queue *q) 
{
    while (q->head != NULL) {
        struct node *n = q->head;
        q->head = n->next;
        free(n);
    }

    free(q);
}

/**
 * True if the queue is empty.
 */
int queue_is_empty(struct queue *q)
{
    return q->head == NULL;
}

/**
 * Enqueue an item.
 */
void queue_enqueue(struct queue *q, void *p)
{
    struct node *n = malloc(sizeof *n);
    n->payload = p;

    if (queue_is_empty(q))
        q->head = q->tail = n;
    else {
        q->tail->next = n;
        q->tail = n;
    }

    n->next = NULL;
}

/**
 * Dequeue an item.
 */
void *queue_dequeue(struct queue *q)
{
    struct node *n = q->head;

    if (n == NULL)
        return n;

    q->head = n->next;

    void *p = n->payload;

    free(n);

    return p;
}

