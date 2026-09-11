#pragma once

#include "../data_structure/trie.h"
#include "invoker.h"
#include "system_data.h"

#define KILL_SYSTEM_SIGNAL 0
#define ERROR_SYSTEM_SIGNAL 1
#define SUCCEED_SYSTEM_SIGNAL 2

typedef struct CliEngine CliEngine;

// CliEngine Methods
void cli_engine_dtor(CliEngine **self);
CliEngine *cli_engine_ctor(void);
const TrieNode *access_cmd_trie(CliEngine *self);
const Invoker *access_invoker(CliEngine *self);
const Appdata *access_appdata(CliEngine *self);
int8_t cli_parser(CliEngine *self);