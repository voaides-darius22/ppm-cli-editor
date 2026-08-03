#pragma once

#include "command.h"

typedef struct CliEngine CliEngine;

uint8_t execute_load(Command *self);
void undo_load(Command *self);
void load_destructor(Command *self);
Command *create_load_command(CliEngine *sys, CliArgs *cmd_args);

uint8_t execute_lsystem(Command *self);
void undo_lsystem(Command *self);
void lsystem_destructor(Command *self);
Command *create_lsystem_command(CliEngine *sys, CliArgs *cmd_args);

uint8_t execute_derive(Command *self);
void derive_destructor(Command *self);
Command *create_derive_command(CliEngine *sys, CliArgs *cmd_args);

Command *create_bitcheck_command(CliEngine *sys, CliArgs *cmd_args);