#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header_files/engine/invoker.h"
#include "../../header_files/data_structure/singly_linked_list.h"

typedef struct Invoker {
    SList *undo_stack, *redo_stack;
} Invoker;

Invoker *invoker_ctor(void)
{
    Invoker *invoker = calloc(1, sizeof(Invoker));
    if (!invoker) {
        return NULL;
    }
    
    invoker->undo_stack = create_slist();
    if (!invoker->undo_stack) {
        invoker_dtor(&invoker);
    }

    invoker->redo_stack = create_slist();
    if (!invoker->redo_stack) {
        invoker_dtor(&invoker);
    }

    return invoker;
}

static void free_invoker_stack(SList **addr_stack)
{
    SList *stack = (addr_stack) ? *addr_stack : NULL;
    if (!stack) {
        return;
    }

    while (!is_empty_slist(stack)) {
        Command *cmd = pop_slist(stack);
        cmd_dtor(&cmd);
    }

    free(stack);
    *addr_stack = NULL;
}

void invoker_dtor(Invoker **self)
{
    Invoker *invoker = (self) ? *self : NULL;
    if (!invoker) {
        return;
    }

    SList *undo_stack = invoker->undo_stack;
    SList *redo_stack = invoker->redo_stack;
    free_invoker_stack(&undo_stack);
    free_invoker_stack(&redo_stack);
    free(invoker);
    *self = NULL;
}

void invoke(Command *cmd, Invoker *self)
{
    if (!cmd || !self) {
        return;
    }

    int8_t exit_code = execute_cmd(cmd);
    // Checking if the command is undoable and its execute succeeded
    if (is_cmd_undoable(cmd) && exit_code) {
        // Clearing redo stack
        SList *redo_stack = self->redo_stack, *undo_stack = self->undo_stack;
        while (!is_empty_slist(redo_stack)) {
            Command *redo_cmd = pop_slist(redo_stack);
            cmd_dtor(&redo_cmd);
        }
        push_slist(undo_stack, cmd);
        return;
    }
    // Command is undoable but its execute failed or the command it's not undoable
    cmd_dtor(&cmd);
}

Command *pop_invoker_stack(Invoker *self, int8_t stack_type)
{
    if (!self) {
        return NULL;
    }
    // Selecting invoker's stack (UNDO / REDO Stack)
    SList *stack = (stack_type == UNDO_STACK) ? self->undo_stack : self->redo_stack;
    return (Command*)pop_slist(stack);
}

void push_invoker_stack(Invoker *self, int8_t stack_type, Command *cmd)
{
    if (!self || !cmd) {
        return;
    }
    // Selecting invoker's stack (UNDO / REDO Stack)
    SList *stack = (stack_type == UNDO_STACK) ? self->undo_stack : self->redo_stack;
    push_slist(stack, (void*)cmd);
}