#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../header_files/command.h"

CliArgs *cli_tokenizer(char *cli_input)
{
    CliArgs *cmd_args = calloc(1, sizeof(*cmd_args));
    if (!cmd_args) {
        return NULL;
    } else if (!cli_input) {
        return cmd_args;
    }

    char *ch = cli_input;
    while (*ch != '\0' && cmd_args->argc < CMD_MAX_ARGS) {
        while (isspace(*ch)) {
            ch++;
        }

        if (*ch == '\0') {
            break;
        } else if (*ch == '\"') {
            // Delim Quotes 
            ch++;
            char *token_value = ch;
            int32_t token_length = 0;
            
            while (*ch != '\0' && *ch != '\"') {
                token_length++;
                ch++;
            }

            if (*ch == '\"') {
                *ch = '\0';
                ch++;
            }

            if (token_length) {
                cmd_args->argv[cmd_args->argc] = malloc(token_length + 1);
                if (!cmd_args->argv[cmd_args->argc]) {
                    return free_cli_args(cmd_args);
                }
                strcpy(cmd_args->argv[cmd_args->argc++], token_value);
            }
        } else {
            char *token_value = ch;
            int32_t token_length = 0;
            while (!isspace(*ch) && *ch != '\0') {
                token_length++;
                ch++;
            }
        
            if (isspace(*ch)) {
                *ch = '\0';
                ch++;
            }

            if (token_length) {
                cmd_args->argv[cmd_args->argc] = malloc(token_length + 1);
                if (!cmd_args->argv[cmd_args->argc]) {
                    return free_cli_args(cmd_args);
                }
                strcpy(cmd_args->argv[cmd_args->argc++], token_value);
            }
        }
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