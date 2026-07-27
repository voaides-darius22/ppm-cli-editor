#pragma once
#include <stdint.h>

#define CLI_MAX_ARGS 32

typedef struct CliArgs {
    uint32_t argc;
    uint8_t *argv[CLI_MAX_ARGS];
} CliArgs;

CliArgs cli_tokenizer(char *command);