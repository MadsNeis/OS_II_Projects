#ifndef QUEUE_H
#define QUEUE_H

struct node {
    void *payload;
    struct node *next;
};

struct queue {
    struct node *head, *tail;
};

struct queue *queue_new(void);
void queue_free(struct queue *q);
int queue_is_empty(struct queue *q);
void queue_enqueue(struct queue *q, void *p);
void *queue_dequeue(struct queue *q);

#endif
