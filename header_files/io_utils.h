#pragma once
#include <stdio.h>
#include <stdint.h>

typedef void (*print_func)(void *ptr);
#define BUFFER_MAX_SIZE 1024

void clean_fgets_input(char *buffer, FILE *fp);
char *read_file_line(FILE *fp);