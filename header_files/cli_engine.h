#pragma once
#include <stdint.h>

#include "trie.h"
#include "invoker.h"
#include "system_data.h"

#define CLI_INPUT_MAX_SIZE 1024

#define KILL_SYSTEM_SIGNAL 0
#define ERROR_SYSTEM_SIGNAL 1
#define SUCCEED_SYSTEM_SIGNAL 2

typedef struct CliEngine {
    TrieNode *cmd_trie;
    Invoker *cmd_invoker;
    SystemData *app_data;
} CliEngine;

uint8_t cli_parser(CliEngine *sys);

typedef Command* (*CommandConstructor)(CliEngine *, CliArgs *);

// create_cmd_trie function will init a prefix tree data structure (Trie) that will contain
// the name of the commands and pointers to command constructors stored in terminal nodes
TrieNode *create_cmd_trie(void);
TrieNode *free_cmd_trie(TrieNode *cmd_trie);
CliEngine *create_cli_engine(void);
CliEngine *free_cli_engine(CliEngine *sys);
