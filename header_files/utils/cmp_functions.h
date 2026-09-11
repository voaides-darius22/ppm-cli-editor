#pragma once
#include <stdint.h>

typedef uint8_t (*CmpHandler)(const void *value_1, const void *value_2);

uint8_t cmp_hash_key_default(const void *value_1, const void *value_2);
