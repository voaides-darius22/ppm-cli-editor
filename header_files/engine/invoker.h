#pragma once

#include "../command/command.h"
typedef struct Invoker Invoker;

typedef enum StackType {
    UNDO_STACK, REDO_STACK
} StackType;

// Invoker Methods
Invoker *invoker_ctor(void);
void invoker_dtor(Invoker **self);
void invoke(Command *cmd, Invoker *self);
Command *pop_invoker_stack(Invoker *self, int8_t stack_type);
void push_invoker_stack(Invoker *self, int8_t stack_type, Command *cmd);