#include <stdint.h>

typedef uint8_t (*cmp_func)(const void *value_1, const void *value_2);

uint8_t cmp_hash_table_node(const void *value_1, const void *value_2);
