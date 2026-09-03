#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/hash_table.h"

#define HASH_MULTIPLIER 31

uint32_t hash_default_helper(const void *key, uint32_t capacity)
{
    const char *string_key = key;
    uint32_t hash_value = 0;
    for (int i = 0; string_key[i] != '\0'; i++) {
        hash_value = hash_value * HASH_MULTIPLIER + string_key[i];
    }
    return hash_value % capacity;
}

// HashTablePair Functions
HashTablePair *create_hash_table_node(void *key, void *value)
{
    HashTablePair *new_pair = calloc(1, sizeof(*new_pair));
    if (!new_pair) {
        return NULL;
    }

    new_pair->key = key;
    new_pair->value = value;
    return new_pair;
}

void *free_hash_table_pair(HashTablePair *pair, FreeHandler free_key_helper)
{
    if (!pair || !free_key_helper) {
        return NULL;
    }

    void *tmp = pair->value; 
    if (pair->key) {
        free_key_helper(pair->key);
    }
    free(pair);
    return tmp;
}

// HashTable Functions
HashTable *create_hash_table(uint32_t capacity, HashHandler hash_helper)
{
    if (!hash_helper) {
        return NULL;
    }

    HashTable *new_table = calloc(1, sizeof(*new_table));
    if (!new_table) {
        return NULL;
    }

    new_table->capacity = capacity;
    new_table->hash_helper = hash_helper;
    new_table->buckets = calloc(capacity, sizeof(*new_table->buckets));
    
    if (!new_table->buckets) {
        free(new_table);
        return NULL;
    }

    for (int i = 0; i < capacity; i++) {
        new_table->buckets[i] = create_slist();
        // Checking if the memory allocation of a bucket has failed
        if (!new_table->buckets[i]) {
            for (int j = 0; j < i; j++) {
                free(new_table->buckets[j]);
            }
            free(new_table);
            return NULL;
        }
    }

    return new_table;
}

void *put(HashTable *table, void *key, void *value, CmpHandler cmp_hash_key_func)
{
    if (!table || !key || !cmp_hash_key_func) {
        return NULL;
    }

    uint32_t index = table->hash_helper(key, table->capacity);    
    
    // Checking if the hash table contains already a node with the same key
    SListNode *bucket = contains_slist_node(table->buckets[index], key, cmp_hash_key_func);
    if (bucket) {
        HashTablePair *pair = bucket->value;
        void *old_value = pair->value;
        pair->value = value;
        return old_value;
    }

    // Memory allocation for the key-value pair
    HashTablePair *new_pair = create_hash_table_node(key, value);
    if (!new_pair) {
        return NULL;
    }
    push_slist(table->buckets[index], new_pair);
    return NULL;
}

void *get(const HashTable *table, const void *key, CmpHandler cmp_hash_key_func)
{
    if (!table || !key || !cmp_hash_key_func) {
        return NULL;
    }

    uint32_t index = table->hash_helper(key, table->capacity);
    SListNode *bucket = contains_slist_node(table->buckets[index], key, cmp_hash_key_func);
    if (!bucket) {
        return NULL;
    }
    HashTablePair *pair = bucket->value;
    return pair->value;
}

void *remove_pair(
    HashTable *table, const void *key,
    CmpHandler cmp_hash_key_func,
    FreeHandler free_key_helper
){
    if (!table || !key || !cmp_hash_key_func || !free_key_helper) {
        return NULL;
    }

    uint32_t index = table->hash_helper(key, table->capacity);
    HashTablePair *pair = remove_slist_node(table->buckets[index], key, cmp_hash_key_func);
    return free_hash_table_pair(pair, free_key_helper);
}

HashTable *free_hash_table(
    HashTable *table, 
    FreeHandler free_value_helper,
    FreeHandler free_key_helper
){
    if (!table || !free_value_helper || !free_key_helper) {
        return NULL;
    }

    uint32_t capacity = table->capacity;
    for (int i = 0; i < capacity; i++) {
        SList *bucket = table->buckets[i];
        while (!is_empty_slist(bucket)) {
            HashTablePair *pair = pop_slist(bucket);
            void *value = free_hash_table_pair(pair, free_key_helper);
            if (free_value_helper && value) {
                free_value_helper(value);
            }
        }
        free(bucket);
    }
    free(table->buckets);
    free(table);
    return NULL;
}