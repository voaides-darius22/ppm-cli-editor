#include <string.h>
#include <stdlib.h>

#include "../header_files/io_utils.h"

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

char *read_file_line(FILE *fp)
{
    if (!fp) {
        return NULL;
    }

    char buffer[BUFFER_MAX_SIZE], *line = NULL, *newline = NULL;
    uint32_t line_length = 0;
    do {
        if (!fgets(buffer, BUFFER_MAX_SIZE, fp)) {
            return line;
        }

        newline = strchr(buffer, '\n');
        if (newline) {
            *newline = '\0';
        }

        uint32_t current_read_len = strlen(buffer);
        char *tmp = realloc(line, line_length + current_read_len + 1);
        if (!tmp) {
            free(line);
            return NULL;
        }
        line = tmp;
        memmove(line + line_length, buffer, current_read_len + 1);
        line_length += current_read_len;
    } while (!newline);

    return line;
}