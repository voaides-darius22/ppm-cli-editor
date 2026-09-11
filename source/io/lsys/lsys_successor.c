#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../header_files/io/lsys/lsys_successor.h"

// LsystemSuccesorRule Methods
LsystemSuccessorRule *create_lsys_successor(const char *succesor_str)
{
    LsystemSuccessorRule *self = (succesor_str) ? malloc(sizeof(LsystemSuccessorRule)) : NULL;
    if (!self) {
        return NULL;
    }

    self->length = strlen(succesor_str);
    self->successor = malloc(self->length + 1);
    if (!self->successor) {
        free(self);
        return NULL;
    }
    strcpy(self->successor, succesor_str);
    return self;
}

void free_lsys_successor(void *ptr)
{
    LsystemSuccessorRule *self = ptr;
    if (!self) {
        return;
    }
    free(self->successor);
    free(self);
}