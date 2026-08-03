#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "../header_files/ppm.h"
#include "../header_files/io_utils.h"

#define BUFFER_SIZE 1024

Ppm *open_ppm_file(const char *path)
{
    if (!path) {
        return NULL;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }

    char buffer[BUFFER_SIZE];

    Ppm *new_file = calloc(1, sizeof(*new_file));
    if (!new_file) {
        fclose(fp);
        return NULL;
    }

    // Reading ppm signature
    memset(new_file->magic_bytes, 0, sizeof(new_file->magic_bytes));
    fgets(new_file->magic_bytes, sizeof(new_file->magic_bytes), fp);
    clean_fgets_input(new_file->magic_bytes, fp);

    // Reading img dimensions
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    clean_fgets_input(buffer, fp);
    char *dimension_token = strtok(buffer, " "); // width token
    if (!dimension_token) {
        fclose(fp);
        return close_ppm_file(new_file);
    }
    new_file->width = atoi(dimension_token);
    dimension_token = strtok(NULL, " "); // height token
    if (!dimension_token) {
        fclose(fp);
        return close_ppm_file(new_file);
    }
    new_file->height = atoi(dimension_token);

    // Reading the max value of a rgb pixel channel
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    new_file->max_value_channel = atoi(buffer);

    // Reading img pixels
    uint32_t pixels = new_file->width * new_file->height;
    new_file->pixel_buffer = malloc(pixels * sizeof(*new_file->pixel_buffer));
    if (!new_file->pixel_buffer) {
        fclose(fp);
        return close_ppm_file(new_file);
    }
    fread(new_file->pixel_buffer, sizeof(*new_file->pixel_buffer), pixels, fp);
    fclose(fp);
    return new_file;
}

Ppm *close_ppm_file(Ppm *ppm)
{
    if (!ppm) {
        return NULL;
    }

    free(ppm->pixel_buffer);
    free(ppm);
    return NULL;
}

void write_ppm_file(const char *path, Ppm *ppm)
{
    if (!path || !ppm) {
        return;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return;
    }

    char buffer[BUFFER_SIZE];

    fwrite(ppm->magic_bytes, sizeof(uint8_t), strlen(ppm->magic_bytes), fp);
    putc('\n', fp);
    sprintf(buffer, "%d %d\n%d\n", ppm->width, ppm->height, ppm->max_value_channel);
    fwrite(buffer, sizeof(uint8_t), strlen(buffer), fp);
    uint32_t pixels = ppm->width * ppm->height;
    fwrite(ppm->pixel_buffer, sizeof(*ppm->pixel_buffer), pixels, fp);
    fclose(fp);
}