#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/command.h"

CliArgs *cli_tokenizer(char *cli_input)
{
    if (!cli_input) {
        return NULL;
    }
    CliArgs *cmd_args = calloc(1, sizeof(*cmd_args));
    if (!cmd_args) {
        return NULL;
    }
    
    char *token = strtok(cli_input, " ");
    while (token && cmd_args->argc < CMD_MAX_ARGS) {
        cmd_args->argv[cmd_args->argc] = malloc(strlen(token) + 1);
        // Checking if the memory allocation of an argument has failed
        if (!cmd_args->argv[cmd_args->argc]) {
            uint32_t argc = cmd_args->argc;
            for (int i = 0; i < argc; i++) {
                free(cmd_args->argv[i]);
            }
            free(cmd_args);
            return NULL;
        }
        strcpy(cmd_args->argv[cmd_args->argc++], token);
        token = strtok(NULL, " ");
    }
    return cmd_args;
}

CliArgs *free_cli_args(CliArgs *cmd_args)
{
    if (!cmd_args) {
        return NULL;
    }

    uint32_t argc = cmd_args->argc;
    for (int i = 0; i < argc; i++) {
        free(cmd_args->argv[i]);
    }
    free(cmd_args);
    return NULL;
}