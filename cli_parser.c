#include <stdio.h>
#include <string.h>

#include "cli_parser.h"

CliArgs cli_tokenizer(char *command)
{
    CliArgs cmd_data;
    memset(&cmd_data, 0, sizeof(cmd_data));
    
    char *cli_token = strtok(command, " ");
    while (cli_token && cmd_data.argc < CLI_MAX_ARGS) {
        cmd_data.argv[cmd_data.argc++] = cli_token;
        cli_token = strtok(NULL, " ");
    }
    
    return cmd_data;
}