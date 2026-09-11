#pragma once
#include <stdint.h>
#include "../../data_structure/hash_table.h"
#include "../app_file.h"

typedef struct Lsystem Lsystem;

// Lsystem Methods
Lsystem *lsys_ctor(const char *path);
void lsys_dtor(AppFile **self);
void load_lsys(AppFile **self);
const char *get_lsystem_axiom(const Lsystem *self);
int32_t get_lsystem_num_of_rules(const Lsystem *self);
const HashTable *get_lsystem_rules_table(const Lsystem *self);
// derive lsys will return the Nth derivative of the Lsystem
char *derive_lsys(const Lsystem *self, uint32_t n);