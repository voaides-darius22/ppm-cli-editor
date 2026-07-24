#include <string.h>

#include "io_utils.h"

void clean_fgets_input(char *buffer, FILE *fp)
{
    if (!buffer || !fp) {
        return;
    }

    char *newline = strchr(buffer, '\n');
    if (newline) {
        *newline = '\0';
    } else {
        // Consume remaining characters
        int c;
        while (c = fgetc(fp), c != '\n' && c != EOF);
    }
}