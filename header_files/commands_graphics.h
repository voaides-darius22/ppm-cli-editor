#pragma once
#include "command.h"

typedef struct CliEngine CliEngine;

uint8_t execute_turtle(Command *self);
void undo_turtle(Command *self);
void turtle_destructor(Command *self);
Command *create_turtle_command(CliEngine *sys, CliArgs *cmd_args);
Command *create_font_command(CliEngine *sys, CliArgs *cmd_args);
Command *create_type_command(CliEngine *sys, CliArgs *cmd_args);