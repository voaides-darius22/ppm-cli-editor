#pragma once

#include <stdint.h>

#define CMD_MAX_ARGS 16 
#define UNDOABLE 1

#define EXECUTE_COMMAND_SUCCEEDED 1
#define EXECUTE_COMMAND_FAILED 0

typedef struct CliArgs {
    uint32_t argc;
    int8_t *argv[CMD_MAX_ARGS];
} CliArgs;
// Reminder: argv[0] == cmd_name

CliArgs *cli_tokenizer(char *cli_input);
CliArgs *free_cli_args(CliArgs *cmd_args);

typedef struct Command {
    uint8_t undoable;
    uint8_t (*execute)(struct Command *self);
    void (*undo)(struct Command *self);
    void (*destructor)(struct Command *self);
    void *receiver;
    // Memento Member will store a previous state, if the undo operation can't
    // restore 100% to the previous state
    void *memento; 
    CliArgs *cmd_args;
} Command;

