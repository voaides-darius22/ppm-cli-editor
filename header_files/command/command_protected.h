#pragma once
#include "command.h"

typedef struct CommandProtected CommandProtected;

typedef struct CommandVTable {
    int8_t (*execute)(Command *self);
    void (*undo)(Command *self);
    void (*destroy_cmd)(Command **self);
} CommandVTable;

typedef struct Command {
    CommandVTable *vptr;
    CommandProtected *protected;
} Command;

Command *cmd_ctor(int8_t undoable, void *receiver, CliArgs *args, const CommandVTable *vptr);