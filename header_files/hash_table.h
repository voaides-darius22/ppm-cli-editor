#pragma once
#include <stdint.h>

#include "singly_linked_list.h"
#include "io_utils.h"

typedef uint32_t (*HashHandler)(const void *key, uint32_t capacity);

// hash_default_helper computes bucket index for string keys
uint32_t hash_default_helper(const void *key, uint32_t capacity);

typedef struct HashTablePair {
    void *key;
    void *value;
} HashTablePair;

// HashTablePair Functions
HashTablePair *create_hash_table_node(void *key, void *value);
void *free_hash_table_pair(HashTablePair *pair, FreeHandler free_key_helper);

typedef struct HashTable {
    SList **buckets;
    uint32_t capacity;
    HashHandler hash_helper;
} HashTable;

// HashTable Functions
HashTable *create_hash_table(uint32_t capacity, HashHandler hash_helper);
// Put function will overwrite the hash table node that contains the same key if exists
// and will return the old value
void *put(HashTable *table, void *key, void *value, CmpHandler cmp_hash_key_func);
void *get(const HashTable *table, const void *key, CmpHandler cmp_hash_key_func);
void *remove_pair(
    HashTable *table, const void *key,
    CmpHandler cmp_hash_key_func,
    FreeHandler free_key_helper
);
HashTable *free_hash_table(
    HashTable *table, 
    FreeHandler free_value_helper,
    FreeHandler free_key_helper
);

