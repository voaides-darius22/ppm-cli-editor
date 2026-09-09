#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/io_utils.h"
#include "../header_files/cli_engine.h"
#include "../header_files/commands_graphics.h"
#include "../header_files/commands_io.h"
#include "../header_files/commands_system.h"

#define CLI_INPUT_MAX_SIZE 1024

typedef struct CliEngine {
    TrieNode *cmd_trie;
    Invoker *invoker;
    SystemData *appdata;
} CliEngine;

// CliEngine Getters
TrieNode *access_cmd_trie(CliEngine *sys)
{
    return (sys) ? sys->cmd_trie : NULL;
}

Invoker *access_invoker(CliEngine *sys)
{
    return (sys) ? sys->invoker : NULL;
}

SystemData *access_appdata(CliEngine *sys)
{
    return (sys) ? sys->appdata : NULL;
}

// create_cmd_trie function will init a prefix tree data structure (Trie) that will contain
// the name of the commands and pointers to command constructors stored in terminal nodes
TrieNode *create_cmd_trie(void)
{
    TrieNode *cmd_trie_root = create_trie_node('\0', 0, NULL);
    if (!cmd_trie_root) {
        return NULL;
    }

    const char *cmd_name[] = {
        "UNDO", "REDO", "LSYSTEM", "DERIVE", "LOAD", "SAVE", "TURTLE", "FONT",
        "TYPE", "BITCHECK", "EXIT", "GRAYSCALE", "BRIGHTNESS"
    };

    CommandConstructor cmd_constructors[] = {
        create_undo_command,  create_redo_command, create_lsystem_command,
        create_derive_command, create_load_command, create_save_command,
        create_turtle_command, create_font_command, create_type_command,
        create_bitcheck_command, create_exit_command, create_grayscale_command,
        create_brightness_command
    };

    uint32_t num_of_cmd_constructors = sizeof(cmd_name) / sizeof(*cmd_name);
    for (int i = 0; i < num_of_cmd_constructors; i++) {
        insert_word(cmd_trie_root, cmd_name[i], cmd_constructors[i]);
    }
    return cmd_trie_root;
}

TrieNode *free_cmd_trie(TrieNode *cmd_trie)
{
    if (!cmd_trie) {
        return NULL;
    }
    return free_trie(cmd_trie, NULL); 
}

// CliEngine Constructor
CliEngine *create_cli_engine(void)
{
    CliEngine *engine = calloc(1, sizeof(*engine));
    if (!engine) {
        return NULL;
    }

    engine->cmd_trie = create_cmd_trie();
    if (!engine->cmd_trie) {
        return free_cli_engine(engine);
    }

    engine->invoker = create_invoker();
    if (!engine->invoker) {
        return free_cli_engine(engine);
    }

    engine->appdata = create_system_data();
    if (!engine->appdata) {
        return free_cli_engine(engine);
    }

    return engine;
}

// CliEngine Destructor
CliEngine *free_cli_engine(CliEngine *engine)
{
    if (!engine) {
        return NULL;
    }

    if (engine->cmd_trie) {
        free_cmd_trie(engine->cmd_trie);
    }

    if (engine->invoker) {
        free_invoker(engine->invoker);
    }

    if (engine->appdata) {
        free_system_data(engine->appdata);
    }

    free(engine);
    return NULL;
}

// CliEngine Parser
int8_t cli_parser(CliEngine *sys)
{
    char cli_input[CLI_INPUT_MAX_SIZE];
    fgets(cli_input, CLI_INPUT_MAX_SIZE, stdin);
    clean_fgets_input(cli_input, stdin);

    // Get command name
    char *space = strchr(cli_input, ' ');
    char *cli_input_line_args = NULL;
    char *cmd_name = cli_input;

    if (space) {
        *space = '\0';
        cli_input_line_args = space + 1;
    }

    CommandConstructor constructor = get_word_value(sys->cmd_trie, cmd_name);
    if (!constructor) {
        if (*cmd_name) {
            printf("%s: command not found\n", cmd_name);
        }
        return ERROR_SYSTEM_SIGNAL;
    }

    CliArgs *cmd_args = cli_tokenizer(cli_input_line_args);
    Command *cmd = constructor(sys, cmd_args);
    if (!cmd) {
        free_cli_args(cmd_args);
        return ERROR_SYSTEM_SIGNAL;
    }
    sys->invoker->invoke(cmd, sys->invoker);
    return (constructor != create_exit_command) ? SUCCEED_SYSTEM_SIGNAL : KILL_SYSTEM_SIGNAL;
}