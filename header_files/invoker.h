#pragma once

#include "command.h"
#include "singly_linked_list.h"

typedef struct Invoker {
    SList *undo_stack, *redo_stack;
    void (*invoke)(Command *cmd, struct Invoker *self);
} Invoker;

Invoker *create_invoker(void);
Invoker *free_invoker(Invoker *invoker);
void invoke(Command *cmd, Invoker *self);