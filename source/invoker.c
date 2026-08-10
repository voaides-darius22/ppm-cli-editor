#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/invoker.h"

Invoker *create_invoker(void)
{
    Invoker *invoker = calloc(1, sizeof(*invoker));
    if (!invoker) {
        return NULL;
    }
    invoker->invoke = invoke;
    
    invoker->undo_stack = create_slist();
    if (!invoker->undo_stack) {
        return free_invoker(invoker);
    }

    invoker->redo_stack = create_slist();
    if (!invoker->redo_stack) {
        return free_invoker(invoker);
    }

    return invoker;
}

Invoker *free_invoker(Invoker *invoker)
{
    if (!invoker) {
        return NULL;
    }

    if (invoker->undo_stack) {
        while (!is_empty_slist(invoker->undo_stack)) {
            Command *cmd = pop_slist(invoker->undo_stack);
            cmd->destructor(cmd);
        }
        free(invoker->undo_stack);
    }

    if (invoker->redo_stack) {
        while (!is_empty_slist(invoker->redo_stack)) {
            Command *cmd = pop_slist(invoker->redo_stack);
            cmd->destructor(cmd);
        }
        free(invoker->redo_stack);
    }

    free(invoker);
    return NULL;
}

void invoke(Command *cmd, Invoker *self)
{
    if (!cmd || !self) {
        return;
    }

    uint8_t exit_code = cmd->execute(cmd);
    // Checking if the command is undoable and its execute succeeded
    if (cmd->undoable && exit_code) {
        // Clearing redo stack
        while (!is_empty_slist(self->redo_stack)) {
            Command *cmd = pop_slist(self->redo_stack);
            cmd->destructor(cmd);
        }
        push_slist(self->undo_stack, cmd);
        return;
    }
    // Command is undoable but its execute failed or the command it's not undoable
    cmd->destructor(cmd);
}