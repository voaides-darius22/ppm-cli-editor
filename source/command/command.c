#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../header_files/command/command.h"
#include "../../header_files/command/command_protected.h"

typedef struct CommandProtected {
    int8_t undoable;
    void *receiver;
    // Memento Member will store a previous state, if the undo operation can't
    // restore 100% to the previous state
    // Reminder: Memento can also store some helper data
    void *memento;
    CliArgs *args;
} CommandProtected;

CliArgs *cli_tokenizer(char *cli_input)
{
    CliArgs *args = calloc(1, sizeof(*args));
    if (!args) {
        return NULL;
    } else if (!cli_input) {
        return args;
    }

    char *ch = cli_input;
    while (*ch != '\0' && args->argc < MAX_ARGS) {
        while (isspace(*ch)) {
            ch++;
        }

        if (*ch == '\0') {
            break;
        } else if (*ch == '\"') {
            // Delim Quotes 
            ch++;
            char *token_value = ch;
            int32_t token_length = 0;
            
            while (*ch != '\0' && *ch != '\"') {
                token_length++;
                ch++;
            }

            if (*ch == '\"') {
                *ch = '\0';
                ch++;
            }

            if (token_length) {
                args->argv[args->argc] = malloc(token_length + 1);
                if (!args->argv[args->argc]) {
                    free_cli_args(&args);
                }
                strcpy(args->argv[args->argc++], token_value);
            }
        } else {
            char *token_value = ch;
            int32_t token_length = 0;
            while (!isspace(*ch) && *ch != '\0') {
                token_length++;
                ch++;
            }
        
            if (isspace(*ch)) {
                *ch = '\0';
                ch++;
            }

            if (token_length) {
                args->argv[args->argc] = malloc(token_length + 1);
                if (!args->argv[args->argc]) {
                    free_cli_args(&args);
                }
                strcpy(args->argv[args->argc++], token_value);
            }
        }
    }
    return args;
}

void free_cli_args(CliArgs **self)
{
    CliArgs *args = (self) ? *self : NULL;
    if (!args) {
        return;
    }

    uint32_t argc = args->argc;
    for (int i = 0; i < argc; i++) {
        free(args->argv[i]);
    }

    free(args);
    *self = NULL;
}

// Command Methods
Command *cmd_ctor(int8_t undoable, void *receiver, CliArgs *args, const CommandVTable *vptr)
{
    Command *cmd = (receiver && vptr) ? calloc(1, sizeof(Command)) : NULL;
    if (!cmd) {
        return NULL;
    }

    cmd->protected = calloc(1, sizeof(CommandProtected));
    if (!cmd->protected) {
        cmd_dtor(&cmd);
    }

    cmd->protected->undoable = undoable;
    cmd->protected->receiver = receiver;
    cmd->protected->args = args;
    cmd->vptr = (CommandVTable*)vptr;
}

void cmd_dtor(Command **self)
{
    Command *cmd = (self) ? *self : NULL;
    if (cmd && cmd->vptr->destroy_cmd) {
        cmd->vptr->destroy_cmd(self);
    }
    
    free_cli_args(&cmd->protected->args);
    free(cmd->protected);
    free(cmd);
    *self = NULL;
}

int8_t is_cmd_undoable(Command *self)
{
    return (self) ? self->protected->undoable : 0;
}

const void *get_cmd_memento(Command *self)
{
    return (self) ? self->protected->memento : NULL;
}

void set_cmd_memento(Command *self, void *memento)
{
    if (self) {
        self->protected->memento = memento;
    }
} 

const void *get_cmd_receiver(Command *self)
{
    return (self) ? self->protected->receiver : NULL;
}

const CliArgs *get_cmd_args(Command *self)
{
    return (self) ? self->protected->args : NULL;
}

int8_t execute_cmd(Command *self)
{
    return (self) ? self->vptr->execute(self) : 0;
}

void undo_cmd(Command *self)
{
    if (is_cmd_undoable(self)) {
        self->vptr->undo(self);
    }
}

