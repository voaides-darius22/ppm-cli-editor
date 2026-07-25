#pragma once
#include <stdint.h>

#include "singly_linked_list.h"
#include "io_utils.h"

typedef uint32_t (*hash_func)(const char *key, uint32_t capacity);

uint32_t hash_helper(const char *key, uint32_t capacity);

typedef struct HashTablePair {
    char *key;
    void *value;
} HashTablePair;

// HashTablePair Functions
HashTablePair *create_hash_table_node(const char *key, void *value);
void *free_hash_table_pair(HashTablePair *pair);

typedef struct HashTable {
    List **buckets;
    uint32_t capacity;
    hash_func hash_helper;
} HashTable;

// HashTable Functions
HashTable *create_hash_table(uint32_t capacity, hash_func hash_helper);
// Put function will overwrite the hash table node that contains the same key if exists
// and will return the old value
void *put(HashTable *table, const char *key, void *value);
void *get(const HashTable *table, const char *key);
void *remove_pair(HashTable *table, const char *key);
HashTable *free_hash_table(HashTable *table, free_func free_helper);

