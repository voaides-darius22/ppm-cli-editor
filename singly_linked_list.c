#include <stdio.h>
#include <stdlib.h>

#include "singly_linked_list.h"

// SListNode Functions
SListNode *create_slist_node(void *value)
{
    SListNode *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->value = value;
    return new_node;
}

void *free_slist_node(SListNode *node)
{
    if (!node) {
        return NULL;
    }

    void *tmp = node->value;
    free(node);
    return tmp;
}

// List Functions
List *create_slist(void)
{
    return calloc(1, sizeof(List));
}

uint8_t is_empty_list(const List *list)
{
    return !list || list->head == NULL;
}

void *remove_slist_node(List *list, const void *value, cmp_func cmp_helper)
{
    if (!list || is_empty_list(list) || !cmp_helper) {
        return NULL;
    }

    SListNode *prev_node = list->head, *next_node = prev_node->next;
    
    // Checking if the first node contains the value that needs to be removed
    if (cmp_helper(prev_node->value, value) == 0) {
        list->head = next_node;
        return free_slist_node(prev_node);
    }

    while (next_node) {
        if (cmp_helper(next_node->value, value) == 0) {
            prev_node->next = next_node->next;
            return free_slist_node(next_node);
        }
        prev_node = next_node;
        next_node = next_node->next; 
    }

    return NULL;
}

SListNode *contains_slist_node(const List *list, const void *value, cmp_func cmp_helper)
{
    if (!list || is_empty_list(list) || !cmp_helper) {
        return NULL;
    }

    SListNode *iter = list->head;
    while (iter) {
        if (!cmp_helper(iter->value, value)) {
            return iter;
        }
        iter = iter->next;
    }
    return NULL;
}

// Stack Behavior Functions
void push_slist(List *list, void *value)
{
    if (!list) {
        return;
    }

    SListNode *new_node = create_slist_node(value);
    if (!new_node) {
        return;
    }

    new_node->next = list->head;
    list->head = new_node;
}

void *pop_slist(List *list)
{
    if (!list || is_empty_list(list)) {
        return NULL;
    }

    SListNode *tmp = list->head;
    list->head = tmp->next;
    return free_slist_node(tmp);
}

List *free_slist(List *list, free_func free_helper)
{
    if (!list) {
        return list;
    }

    void *value = NULL;

    while (!is_empty_list(list)) {
        value = pop_slist(list);
        if (free_helper) {
            free_helper(value);
        }
    }

    free(list);
    return NULL;
}