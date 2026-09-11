#pragma once

#include "command.h"
#include "../engine/cli_engine.h"

Command *create_undo_command(CliEngine *sys, CliArgs *args);
Command *create_redo_command(CliEngine *sys, CliArgs *args);
Command *create_save_command(CliEngine *sys, CliArgs *args);
Command *create_exit_command(CliEngine *sys, CliArgs *args);