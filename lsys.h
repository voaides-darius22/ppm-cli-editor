#include <stdint.h>
#include <stdio.h>

#include "hash_table.h"

typedef struct Lsystem {
    char *axiom;
    HashTable *rules;
} Lsystem;

Lsystem *open_lsystem_file(const char *path);
Lsystem *close_lsystem_file(Lsystem *lsys);