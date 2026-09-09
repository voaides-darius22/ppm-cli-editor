#pragma once
#include <stdint.h>

#include "trie.h"
#include "invoker.h"
#include "system_data.h"

#define KILL_SYSTEM_SIGNAL 0
#define ERROR_SYSTEM_SIGNAL 1
#define SUCCEED_SYSTEM_SIGNAL 2

typedef struct CliEngine CliEngine;
typedef Command* (*CommandConstructor)(CliEngine *, CliArgs *);

// CliEngine Getters
TrieNode *access_cmd_trie(CliEngine *sys);
Invoker *access_invoker(CliEngine *sys);
SystemData *access_appdata(CliEngine *sys);

// CliEngine Parser
int8_t cli_parser(CliEngine *sys);

// CliEngine Constructor
CliEngine *create_cli_engine(void);

// CliEngine Destructor
CliEngine *free_cli_engine(CliEngine *sys);
