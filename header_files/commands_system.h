#pragma once
#include "command.h"

typedef struct CliEngine CliEngine;

uint8_t execute_undo(Command *self);
void undo_destructor(Command *self);
Command *create_undo_command(CliEngine *sys, CliArgs *cmd_args);

uint8_t execute_redo(Command *self);
void redo_destructor(Command *self);
Command *create_redo_command(CliEngine *sys, CliArgs *cmd_args);

uint8_t execute_save(Command *self);
void save_destructor(Command *self);
Command *create_save_command(CliEngine *sys, CliArgs *cmd_args);

uint8_t execute_exit(Command *self);
void exit_destructor(Command *self);
Command *create_exit_command(CliEngine *sys, CliArgs *cmd_args);