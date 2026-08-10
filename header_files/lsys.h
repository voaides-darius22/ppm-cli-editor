#pragma once
#include <stdint.h>
#include <stdio.h>

#include "hash_table.h"

typedef struct LsystemSuccessorRule {
    char *successor;
    uint32_t length;
} LsystemSuccessorRule;

// SuccessorDataRule Functions
LsystemSuccessorRule *create_successor_data(const char *buffer);
void free_successor_data(void *ptr);

typedef struct Lsystem {
    char *file_path;
    char *axiom;
    uint32_t nrules;
    HashTable *rules;
} Lsystem;

// Lsystem File Functions
Lsystem *open_lsystem_file(const char *path);
Lsystem *close_lsystem_file(Lsystem *lsys);
// derive lsys will return the Nth derivative of the Lsystem
char *derive_lsys(const Lsystem *lsys, uint32_t n);