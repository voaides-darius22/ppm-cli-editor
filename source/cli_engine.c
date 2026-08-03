#include <stdio.h>
#include <stdlib.h>

#include "../header_files/io_utils.h"
#include "../header_files/cli_engine.h"
#include "../header_files/commands_graphics.h"
#include "../header_files/commands_io.h"
#include "../header_files/commands_system.h"

TrieNode *create_cmd_trie(void)
{
    TrieNode *cmd_trie_root = create_trie_node('\0', 0, NULL);
    if (!cmd_trie_root) {
        return NULL;
    }

    const char *cmd_name[] = {
        "UNDO", "REDO", "LSYSTEM", "DERIVE", "LOAD", "SAVE", "TURTLE", "FONT",
        "TYPE", "BITCHECK", "EXIT"
    };

    CommandConstructor cmd_constructors[] = {
        create_undo_command,  create_redo_command, create_lsystem_command,
        create_derive_command, create_load_command, create_save_command,
        create_turtle_command, create_font_command, create_type_command,
        create_bitcheck_command, create_exit_command
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

    engine->cmd_invoker = create_invoker();
    if (!engine->cmd_invoker) {
        return free_cli_engine(engine);
    }

    engine->app_data = create_system_data();
    if (!engine->app_data) {
        return free_cli_engine(engine);
    }

    return engine;
}

CliEngine *free_cli_engine(CliEngine *engine)
{
    if (!engine) {
        return NULL;
    }

    if (engine->cmd_trie) {
        engine->cmd_trie = free_cmd_trie(engine->cmd_trie);
    }

    if (engine->cmd_invoker) {
        engine->cmd_invoker = free_invoker(engine->cmd_invoker);
    }

    if (engine->app_data) {
        engine->app_data = free_system_data(engine->app_data);
    }

    free(engine);
    return NULL;
}

uint8_t cli_parser(CliEngine *sys)
{
    char cli_input[CLI_INPUT_MAX_SIZE];
    fgets(cli_input, CLI_INPUT_MAX_SIZE, stdin);
    clean_fgets_input(cli_input, stdin);

    CliArgs *cmd_args = cli_tokenizer(cli_input);
    if (!cmd_args) {
        return ERROR_SYSTEM_SIGNAL;
    }

    const char *cmd_name = cmd_args->argv[0];
    CommandConstructor constructor = get_word_value(sys->cmd_trie, cmd_name);
    if (!constructor) {
        free_cli_args(cmd_args);
        return ERROR_SYSTEM_SIGNAL;
    }
    Command *cmd = constructor(sys, cmd_args);
    if (!cmd) {
        free_cli_args(cmd_args);
        return ERROR_SYSTEM_SIGNAL;
    }
    sys->cmd_invoker->invoke(cmd, sys->cmd_invoker);
    return (constructor != create_exit_command) ? SUCCEED_SYSTEM_SIGNAL : KILL_SYSTEM_SIGNAL;
}