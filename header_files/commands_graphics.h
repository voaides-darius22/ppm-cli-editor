#pragma once
#include "command.h"

typedef struct CliEngine CliEngine;

Command *create_turtle_command(CliEngine *sys, CliArgs *cmd_args);
Command *create_font_command(CliEngine *sys, CliArgs *cmd_args);
Command *create_type_command(CliEngine *sys, CliArgs *cmd_args);