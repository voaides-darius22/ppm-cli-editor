#pragma once
#include <stdint.h>
#include <stdio.h>

#include "hash_table.h"

typedef struct Lsystem Lsystem;

// Lsystem Getters
char *get_lsystem_path(const Lsystem *lsys);
char *get_lsystem_axiom(const Lsystem *lsys);
int32_t get_lsystem_num_of_rules(const Lsystem *lsys);
HashTable *get_lsystem_rules_table(const Lsystem *lsys);

// Lsystem Constructor
Lsystem *open_lsystem_file(const char *path);

// Lsystem Destructor
Lsystem *close_lsystem_file(Lsystem *lsys);

// derive lsys will return the Nth derivative of the Lsystem
char *derive_lsys(const Lsystem *lsys, uint32_t n);