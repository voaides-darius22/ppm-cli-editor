#pragma once

#include "command.h"
#include "../engine/cli_engine.h"

Command *create_load_command(CliEngine *sys, CliArgs *args);
Command *create_derive_command(CliEngine *sys, CliArgs *args);
Command *create_bitcheck_command(CliEngine *sys, CliArgs *args);