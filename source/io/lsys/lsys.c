#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "../../../header_files/io/lsys/lsys.h"
#include "../../../header_files/io/lsys/lsys_successor.h"
#include "../../../header_files/utils/io_utils.h"
#include "../../../header_files/data_structure/queue.h"
#include "../../../header_files/data_structure/hash_table.h"
#include "../../../header_files/io/app_file_protected.h"

#define BUFFER_SIZE 1024
#define LSYSTEM_SYMBOL_SIZE 2

static const AppFileVTable LsystemFileVTable = {
    .load_file = load_lsys,
    .destroy_file = lsys_dtor
};

typedef struct Lsystem {
    AppFile base;
    // Lsystem Data Protected
    char *axiom;
    int32_t nrules;
    HashTable *rules;
} Lsystem;


static uint32_t hash_lsys_helper(const void *key, uint32_t capacity)
{
    // Key (Lsystem symbol)
    return (*(const char *)key / HASH_MULTIPLIER) % capacity;
}

static uint8_t cmp_hash_key_lsys(const void *value_1, const void *value_2)
{
    if (!value_1 || !value_2) {
        return 0;
    }

    const char *symbol_1 = ((HashTablePair *)value_1)->key;
    const char *symbol_2 = value_2;
    return (*symbol_1 == *symbol_2) ? 0 : 1;
}

// Lsystem Methods
Lsystem *lsys_ctor(const char *path)
{
    // .lsys file Configuration
    Lsystem *file = (path) ? calloc(1, sizeof(Lsystem)) : NULL;
    init_file_app((AppFile**)&file, path, &LsystemFileVTable, LSYSTEM);
    if (!file) {
        return NULL;
    }
    // File Loading
    app_file_load((AppFile**)&file);
    return file;
}

void lsys_dtor(AppFile **self)
{
    Lsystem *file = (self) ? (Lsystem *)(*self) : NULL;
    if (!file) {
        return;
    }
    free(file->axiom);
    free_hash_table(file->rules, free_lsys_successor, free);
}

void load_lsys(AppFile **self)
{
    // File Opening (Read Mode)
    const char *path = (self) ? get_app_file_path(*self) : NULL;
    if (!path) {
        return;
    }
    FILE *fp = fopen(path, "r"); 
    if (!fp) {
        return;
    }
    Lsystem *file = (Lsystem*)*self;
    char buffer[BUFFER_SIZE];
    
    // Reading lsystem axiom
    fgets(buffer, BUFFER_SIZE, fp);
    clean_fgets_input(buffer, fp);
    
    // Memory allocation for lsystem axiom
    file->axiom = malloc(strlen(buffer) + 1);
    if (!file->axiom) {
        fclose(fp);
        app_file_dtor(self);
        return;
    }
    strcpy(file->axiom, buffer);

    // Reading lsystem nrules
    fgets(buffer, BUFFER_SIZE, fp);
    clean_fgets_input(buffer, fp);
    file->nrules = atoi(buffer);

    // Memory allocation for rules table
    const uint32_t PRINTABLE_ASCII_CHARS = 95;
    file->rules = create_hash_table(PRINTABLE_ASCII_CHARS, hash_lsys_helper);
    if (!file->rules) {
        fclose(fp);
        app_file_dtor(self);
        return;
    }

    for (int i = 0; i < file->nrules; i++) {
        // Reading lsystem rule (symbol + successor)
        fgets(buffer, BUFFER_SIZE, fp);
        clean_fgets_input(buffer, fp);
        char *token = strtok(buffer, " ");
        // Memory allocation for symbol
        char *symbol = (token) ? malloc(sizeof(char)) : NULL;
        if (!symbol) {
            fclose(fp);
            app_file_dtor(self);
            return;
        }
        *symbol = *token;
        token = strtok(NULL, " ");
        // Memory allocation for successor
        LsystemSuccessorRule *successor = (token) ? create_lsys_successor(token) : NULL;
        if (!successor) {
            free(symbol);
            fclose(fp);
            app_file_dtor(self);
            return;
        }
        put(file->rules, symbol, successor, cmp_hash_key_lsys);
    }
    
    fclose(fp);
    printf(
        "Loaded %s (L-system with %d rules)\n", 
        get_app_file_path(*self), get_lsystem_num_of_rules((const Lsystem *)*self)
    );
}

const char *get_lsystem_axiom(const Lsystem *self)
{
    return (self) ? self->axiom : NULL;
}

int32_t get_lsystem_num_of_rules(const Lsystem *self)
{
    return (self) ? self->nrules : -1;
}

const HashTable *get_lsystem_rules_table(const Lsystem *self)
{
    return (self) ? self->rules : NULL;
}

char *derive_lsys(const Lsystem *self, uint32_t n)
{
    if (!self) {
        return NULL;
    }

    // Checking if the for no derivation
    if (n == 0) {
        char *derivative = malloc(strlen(self->axiom) + 1);
        if (!derivative) {
            return NULL;
        }
        strcpy(derivative, self->axiom);
        return derivative;
    }

    Queue queue_slots[2];
    memset(queue_slots, 0, sizeof(queue_slots));
   
    // Initialising the first queue slot with the lsystem axiom
    uint32_t axiom_length = strlen(self->axiom);
    for (int i = 0; i < axiom_length; i++) {
        enqueue(&queue_slots[0], &self->axiom[i]);
    }

    uint32_t derivative_length;
    for (int i = 0; i < n; i++) {
        derivative_length = 0;
        uint8_t index_1 = i % 2;
        uint8_t index_2 = !index_1;
        while (!is_empty_queue(&queue_slots[index_1])) {
            char *symbol = dequeue(&queue_slots[index_1]);
            LsystemSuccessorRule *value = get(self->rules, symbol, cmp_hash_key_lsys);
            // Checking if the production rule exists
            if (value) {
                derivative_length += value->length;
                for (int j = 0; j < value->length; j++) {
                    enqueue(&queue_slots[index_2], &value->successor[j]);
                }
            } else {
                enqueue(&queue_slots[index_2], symbol);
                derivative_length++;
            } 
        }
    }
    
    uint8_t result_index = n % 2;
    char *derivative = malloc(derivative_length + 1);
    if (!derivative) {
        while (!is_empty_queue(&queue_slots[result_index])) {
            dequeue(&queue_slots[result_index]);
        }
        return NULL;
    }

    derivative[derivative_length] = '\0';
    for (int i = 0; i < derivative_length; i++) {
        derivative[i] = *(char *)dequeue(&queue_slots[result_index]);
    }
    return derivative; 
}