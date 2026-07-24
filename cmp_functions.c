#include <stdio.h>
#include <string.h>

#include "cmp_functions.h"
#include "hash_table.h"

uint8_t cmp_hash_table_node(const void *value_1, const void *value_2)
{
    if (!value_1 || !value_2) {
        return 0;
    }

    const HashTablePair *ptr_1 = value_1;
    const char *ptr_2 = value_2;

    return (!strcmp(ptr_1->key, ptr_2)) ? 0 : 1;
}