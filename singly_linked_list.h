#include "free_functions.h"
#include "cmp_functions.h"

typedef struct SListNode {
    void *value;
    struct SListNode *next;
} SListNode;

// SListNode Functions
SListNode *create_slist_node(void *value);
void *free_slist_node(SListNode *node);

typedef struct List {
    SListNode *head;
} List;

// List Functions
List *create_slist(void);
int is_empty_list(List *list);
void *remove_slist_node(List *list, const void *value, cmp_func cmp_helper);
SListNode *contains_slist_node(List *list, const void *value, cmp_func cmp_helper);

// Stack Behavior Functions
void push_slist(List *list, void *value);
void *pop_slist(List *list);

List *free_slist(List *list, free_func free_helper);