#pragma once

#include <stdint.h>
#include "free_functions.h"

typedef struct DListNode {
    void *value;
    struct DListNode *prev, *next;
} DListNode;

// DListNode Functions
DListNode *create_dlist_node(void *value);
void *free_dlist_node(DListNode *node);

typedef struct DList {
    DListNode *front, *rear;
    uint32_t size;
} DList;

// DList Functions
DList *create_dlist(void);
uint8_t is_empty_dlist(const DList *list);
void insert_dlist_node(DList *list, void *value, int32_t pos);
DListNode *find_dlist_node(const DList *list, int32_t pos);
void *remove_dlist_node(DList *list, int32_t pos);
DList *free_dlist(DList *list, free_func free_helper);