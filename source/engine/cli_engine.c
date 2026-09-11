#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../header_files/data_structure/trie.h"
#include "../../header_files/utils/io_utils.h"
#include "../../header_files/engine/cli_engine.h"
#include "../../header_files/command/commands_graphics.h"
#include "../../header_files/command/commands_io.h"
#include "../../header_files/command/commands_system.h"

typedef Command* (*CommandConstructor)(CliEngine *, CliArgs *);

#define BUFFER_SIZE 1024

typedef struct CliEngine {
    TrieNode *cmd_trie;
    Invoker *invoker;
    Appdata *appdata;
} CliEngine;

// create_cmd_trie function will init a prefix tree data structure (Trie) that will contain
// the name of the commands and pointers to command constructors stored in terminal nodes
static TrieNode *cmd_trie_ctor(void)
{
    TrieNode *cmd_trie_root = create_trie_node('\0', 0, NULL);
    if (!cmd_trie_root) {
        return NULL;
    }

    const char *cmd_name[] = {
        "UNDO", "REDO", "DERIVE", "LOAD", "SAVE", "TURTLE",
        "TYPE", "BITCHECK", "EXIT", "GRAYSCALE", "BRIGHTNESS"
    };

    CommandConstructor cmd_constructors[] = {
        create_undo_command,  create_redo_command,create_derive_command, 
        create_load_command, create_save_command, create_turtle_command, 
        create_type_command, create_bitcheck_command, create_exit_command, 
        create_grayscale_command, create_brightness_command
    };

    uint32_t num_of_cmd_constructors = sizeof(cmd_name) / sizeof(*cmd_name);
    for (int i = 0; i < num_of_cmd_constructors; i++) {
        insert_word(cmd_trie_root, cmd_name[i], cmd_constructors[i]);
    }
    return cmd_trie_root;
}

static void cmd_trie_dtor(TrieNode **self)
{
    TrieNode *root = (self) ? *self : NULL;
    free_trie(root, NULL);
    *self = NULL;
}

// CliEngine Methods
void cli_engine_dtor(CliEngine **self)
{
    CliEngine *engine = (self) ? *self : NULL;
    if (!engine) {
        return;
    }

    cmd_trie_dtor(&engine->cmd_trie);
    invoker_dtor(&engine->invoker);
    appdata_dtor(&engine->appdata);
    free(engine);
    *self = NULL;
}

CliEngine *cli_engine_ctor(void)
{
    CliEngine *engine = calloc(1, sizeof(CliEngine));
    if (!engine) {
        return NULL;
    }

    engine->cmd_trie = cmd_trie_ctor();
    if (!engine->cmd_trie) {
        cli_engine_dtor(&engine);
        return NULL;
    }

    engine->invoker = invoker_ctor();
    if (!engine->invoker) {
        cli_engine_dtor(&engine);
        return NULL;
    }

    engine->appdata = appdata_ctor();
    if (!engine->appdata) {
        cli_engine_dtor(&engine);
        return NULL;
    }

    return engine;
}

const TrieNode *access_cmd_trie(CliEngine *self)
{
    return (self) ? self->cmd_trie : NULL;
}

const Invoker *access_invoker(CliEngine *self)
{
    return (self) ? self->invoker : NULL;
}

const Appdata *access_appdata(CliEngine *self)
{
    return (self) ? self->appdata : NULL;
}

static void show_cmd_name_with_prefix(TrieNode *cmd_trie, char *prefix)
{
    if (!prefix) {
        return;
    }

    int32_t prefix_len = strlen(prefix);
    TrieNode *parent = cmd_trie;
    for (int i = 0; i < prefix_len; i++) {
        prefix[i] = toupper(prefix[i]);
        TrieNode *child = get_child(parent, prefix[i]);
        if (!child) {
            return;
        }
        parent = child;
    }

    const int8_t MAX_CMD_NAMES = 8;
    int32_t len = 0;
    char *words[MAX_CMD_NAMES], buffer[BUFFER_SIZE];
    strcpy(buffer, prefix); 
    get_words(parent, words, &len, buffer, prefix_len - 1, MAX_CMD_NAMES);    

    if (!len) {
        return;
    }
    
    // Printing cmd names
    printf("Do you mean? ");
    for (int i = 0; i < len; i++) {
        printf("\"%s\" ", words[i]);
        // Free the memory that has been allocated for the cmd name
        free(words[i]);
    }
    printf("\n");
    return;
}

int8_t cli_parser(CliEngine *self)
{
    char cli_input[BUFFER_SIZE];
    fgets(cli_input, BUFFER_SIZE, stdin);
    clean_fgets_input(cli_input, stdin);

    // Get command name
    char *space = strchr(cli_input, ' ');
    char *cli_input_line_args = NULL;
    char *cmd_name = cli_input;

    if (space) {
        *space = '\0';
        cli_input_line_args = space + 1;
    }

    CommandConstructor constructor = get_word_value(self->cmd_trie, cmd_name);
    if (!constructor) {
        if (*cmd_name) {
            printf("%s: command not found\n", cmd_name);
            show_cmd_name_with_prefix(self->cmd_trie, cmd_name); 
        }
        return ERROR_SYSTEM_SIGNAL;
    }

    CliArgs *args = cli_tokenizer(cli_input_line_args);
    Command *cmd = constructor(self, args);
    if (!cmd) {
        free_cli_args(&args);
        return ERROR_SYSTEM_SIGNAL;
    }
    invoke(cmd, self->invoker);
    return (constructor != create_exit_command) ? SUCCEED_SYSTEM_SIGNAL : KILL_SYSTEM_SIGNAL;
}