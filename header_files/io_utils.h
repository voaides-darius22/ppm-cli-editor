#pragma once
#include <stdio.h>

typedef void (*print_func)(void *ptr);

void clean_fgets_input(char *buffer, FILE *fp);
