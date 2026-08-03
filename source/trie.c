#include <stdio.h>
#include <stdlib.h>

#include "../header_files/trie.h"

// TrieNode Functions
TrieNode *create_trie_node(char character, uint8_t end, void *value)
{
    TrieNode *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->character = character;
    new_node->end = end;
    new_node->value = value;

    return new_node;
}

uint32_t num_of_kids(const TrieNode *node)
{
    if (!node || !node->children) {
        return 0;
    }
    return node->children->size;
}

void insert_child(TrieNode *parent, TrieNode *child)
{
    if (!parent || !child) {
        return;
    }

    if (!parent->children) {
        // Memory allocation for the children doubly linked list
        parent->children = create_dlist();
        if (!parent->children) {
            return;
        }
    }
    insert_dlist_node(parent->children, child, 0);
}

TrieNode *get_child(const TrieNode *root, char character)
{
    if (!root || !root->children) {
        return NULL;
    }

    DListNode *iter = root->children->front;
    while (iter) {
        TrieNode *child = iter->value;
        if (child->character == character) {
            return child;
        }
        iter = iter->next;
    }
    return NULL;
}

void insert_word(TrieNode *root, const char *word, void *value)
{
    if (!root || !word) {
        return;
    }

    TrieNode *current_node = root;
    for (int i = 0; word[i] != '\0'; i++) {
        TrieNode *child = get_child(current_node, word[i]);
        if (!child) {
            child = create_trie_node(word[i], 0, NULL);
            insert_child(current_node, child);
        }
        current_node = child;
    }
    current_node->end = 1;
    current_node->value = value;
}

uint8_t contains_word(TrieNode *root, const char *word)
{
    if (!root || !word) {
        return 0;
    }

    TrieNode *current_node = root;
    for (int i = 0; word[i] != '\0'; i++) {
        TrieNode *child = get_child(current_node, word[i]);
        if (!child) {
            return 0;
        }
        current_node = child;
    }
    return (current_node->end) ? 1 : 0;
}

void *get_word_value(TrieNode *root, const char *word)
{
    if (!root || !word) {
        return NULL;
    }

    TrieNode *current_node = root;
    for (int i = 0; word[i] != '\0'; i++) {
        TrieNode *child = get_child(current_node, word[i]);
        if (!child) {
            return NULL;
        }
        current_node = child;
    }
    return (current_node->end) ? current_node->value : NULL;
}

TrieNode *free_trie(TrieNode *root, free_func free_helper)
{
    if (!root) {
        return NULL;
    }

    if (!root->children) {
        if (free_helper && root->value) {
            free_helper(root->value);
        }
        free(root);
        return NULL;
    }

    DListNode *iter = root->children->front;
    while (iter) {
       TrieNode *child = free_trie((TrieNode *)iter->value, free_helper);
       iter = iter->next;
    }

    root->children = free_dlist(root->children, NULL);
    if (root->value && free_helper) {
        free_helper(root->value);
    }
    free(root);
    return NULL;
}