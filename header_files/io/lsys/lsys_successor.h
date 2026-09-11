#pragma once

#include <stdint.h>

typedef struct {
    char *successor;
    uint32_t length;
} LsystemSuccessorRule;

LsystemSuccessorRule *create_lsys_successor(const char *succesor_str);
void free_lsys_successor(void *self);