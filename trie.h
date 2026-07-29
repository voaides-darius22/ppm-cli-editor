#pragma once
#include <stdint.h>

#include "free_functions.h"
#include "doubly_linked_list.h"

typedef struct TrieNode {
    char character;
    uint8_t end;
    void *value;
    DList *children;
} TrieNode;

// TrieNode Functions
TrieNode *create_trie_node(char character, uint8_t end, void *value);
uint32_t num_of_kids(const TrieNode *node);
// The return value of the insert_child function will be NULL if the insertion succeeded, otherwise
// it will be the TrieNode value pointer
void insert_child(TrieNode *parent, TrieNode *child);
TrieNode *get_child(const TrieNode *root, char character);
void insert_word(TrieNode *root, const char *word, void *value);
uint8_t contains_word(TrieNode *root, const char *word);
TrieNode *free_trie(TrieNode *root, free_func free_helper);