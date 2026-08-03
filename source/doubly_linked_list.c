#include <stdio.h>
#include <stdlib.h>

#include "../header_files/doubly_linked_list.h"

// DListNode Functions
DListNode *create_dlist_node(void *value)
{
    DListNode *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }
    new_node->value = value;
    return new_node;
}

void *free_dlist_node(DListNode *node)
{
    if (!node) {
        return NULL;
    }

    void *tmp = node->value;
    free(node);
    return tmp;
}

// DList Functions
DList *create_dlist(void)
{
    return calloc(1, sizeof(DList));
}

uint8_t is_empty_dlist(const DList *list)
{
    return list->front == NULL;
}

void insert_dlist_node(DList *list, void *value, int32_t pos)
{
    if (!list || pos < 0) {
        return;
    }

    DListNode *new_node = create_dlist_node(value);
    if (!new_node) {
        return;
    }

    // Edge cases for inserting the new node
    if (is_empty_dlist(list)) {
        list->front = list->rear = new_node;
        list->size++;
        return;
    } else if (pos >= list->size) {
        new_node->prev = list->rear;
        list->rear->next = new_node;
        list->rear = new_node;
        list->size++;
        return;
    }

    DListNode *current_node = find_dlist_node(list, pos);
    new_node->prev = current_node->prev;
    if (!current_node->prev) {
        list->front = new_node;    
    } else {
        current_node->prev->next = new_node;
    }
    new_node->next = current_node;
    current_node->prev = new_node;
    list->size++;
}

DListNode *find_dlist_node(const DList *list, int32_t pos)
{
    if (!list || pos < 0 || pos >= list->size) {
        return NULL;
    }

    // Choosing the starting node to iterate through list
    uint32_t middle = list->size / 2;
    uint8_t reverse = (pos < middle) ? 0 : 1;
    DListNode *iter = (!reverse) ? list->front : list->rear;
    
    if (reverse) {
        pos = list->size - pos - 1;
    }

    for (int i = 0; i < pos && iter; i++) {
        iter = (!reverse) ? iter->next : iter->prev; 
    }

    return iter;
}

void *remove_dlist_node(DList *list, int32_t pos)
{
    DListNode *current_node = find_dlist_node(list, pos);
    if (!current_node) {
        return NULL;
    }

    void *tmp = current_node->value;
    if (!current_node->prev) {
        list->front = current_node->next;
    } else {
        current_node->prev->next = current_node->next;
    }

    if (!current_node->next) {
        list->rear = current_node->prev;
    } else {
        current_node->next->prev = current_node->prev;
    }

    list->size--;
    free(current_node);
    return tmp;
}

DList *free_dlist(DList *list, free_func free_helper)
{
    if (!list) {
        return NULL;
    }

    DListNode *iter = list->front, *tmp;
    while (iter) {
        if (iter->value && free_helper) {
            free_helper(iter->value);
        }
        tmp = iter;
        iter = iter->next;
        free(tmp);
    }

    free(list);
    return NULL;
}
