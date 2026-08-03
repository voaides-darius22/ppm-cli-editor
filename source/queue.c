#include <stdio.h>
#include <stdlib.h>

#include "../header_files/queue.h"

// Queue Functions
Queue *create_queue(void)
{
    return calloc(1, sizeof(Queue));
}

uint8_t is_empty_queue(const Queue *q)
{
    return q->front == NULL;
}

void enqueue(Queue *q, void *value)
{
    if (!q) {
        return;
    }

    SListNode *new_node = create_slist_node(value);
    if (!new_node) {
        return;
    }

    if (is_empty_queue(q)) {
        q->front = q->rear = new_node;
        return;
    }

    q->rear->next = new_node;
    q->rear = new_node;
}

void *dequeue(Queue *q)
{
    if (!q || is_empty_queue(q)) {
        return NULL;
    }

    SListNode *tmp = q->front;
    q->front = tmp->next;
    
    // Checking if the node that will be removed is the last node from the queue
    if (!q->front) {
        q->rear = NULL;
    }

    void *value = free_slist_node(tmp);
    return value;
}

Queue *free_queue(Queue *q, free_func free_helper)
{
    if (!q) {
        return NULL;
    }

    while (!is_empty_queue(q)) {
        void *value = dequeue(q);
        if (value && free_helper) {
            free_helper(value);
        }
    }

    free(q);
    return NULL;
}