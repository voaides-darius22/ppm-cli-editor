#include "singly_linked_list.h"
#include "free_functions.h"

typedef struct Queue {
    SListNode *front, *rear;
} Queue;

// Queue Functions
Queue *create_queue(void);
uint8_t is_empty_queue(const Queue *q);
void enqueue(Queue *q, void *value);
void *dequeue(Queue *q);
Queue *free_queue(Queue *q, free_func free_helper);