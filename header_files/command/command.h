#pragma once

#include <stdint.h>

#define MAX_ARGS 16 

typedef struct CliArgs {
    uint32_t argc;
    char *argv[MAX_ARGS];
} CliArgs;

typedef struct Command Command;

CliArgs *cli_tokenizer(char *cli_input);
void free_cli_args(CliArgs **self);

// Command Methods
void cmd_dtor(Command **self);
int8_t is_cmd_undoable(Command *self);
const void *get_cmd_memento(Command *self);
void set_cmd_memento(Command *self, void *memento); 
const void *get_cmd_receiver(Command *self);
const CliArgs *get_cmd_args(Command *self);
int8_t execute_cmd(Command *self);
void undo_cmd(Command *self);