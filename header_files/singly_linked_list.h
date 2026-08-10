#pragma once
#include <stdint.h>

#include "free_functions.h"
#include "cmp_functions.h"

typedef struct SListNode {
    void *value;
    struct SListNode *next;
} SListNode;

// SListNode Functions
SListNode *create_slist_node(void *value);
void *free_slist_node(SListNode *node);

typedef struct SList {
    SListNode *head;
} SList;

// SList Functions
SList *create_slist(void);
uint8_t is_empty_slist(const SList *list);
void *remove_slist_node(SList *list, const void *value, CmpHandler cmp_helper);
SListNode *contains_slist_node(const SList *list, const void *value, CmpHandler cmp_helper);

// Stack Behavior Functions
void push_slist(SList *list, void *value);
void *pop_slist(SList *list);
void *peek(const SList *list);
SList *free_slist(SList *list, free_func free_helper);