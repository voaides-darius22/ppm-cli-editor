#pragma once

#include "command.h"
#include "../engine/cli_engine.h"

Command *create_turtle_command(CliEngine *sys, CliArgs *args);
Command *create_type_command(CliEngine *sys, CliArgs *args);
Command *create_grayscale_command(CliEngine *sys, CliArgs *args);
Command *create_brightness_command(CliEngine *sys, CliArgs *args);
Command *create_crop_command(CliEngine *sys, CliArgs *args);