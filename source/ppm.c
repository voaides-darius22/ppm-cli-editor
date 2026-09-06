#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "../header_files/ppm.h"
#include "../header_files/io_utils.h"

#define BUFFER_SIZE 1024

PpmArgs ppm_tokenizer(char *ppm_input)
{
    PpmArgs ppm_args;
    memset(&ppm_args, 0, sizeof(ppm_args));

    if (!ppm_input) {
        return ppm_args;
    }

    // Checking if the input line contains comments => ignore the rest of the line after #
    char *comment = strchr(ppm_input, '#');
    if (comment) {
        *comment = '\0';
    }

    char *token = strtok(ppm_input, " ");
    while (token) {
        ppm_args.argv[ppm_args.argc] = token;
        ppm_args.argc++;
        token = strtok(NULL, " ");
    }

    return ppm_args;
}

void ppm_change_state(PpmBuilder *ppm_builder)
{
    switch (ppm_builder->img_flags) {
        case 0: {ppm_builder->state = WAITING_MAGIC_NUMBER; break;}
        case 1: {ppm_builder->state = WAITING_IMG_DIMENSIONS; break;}
        case 3: {ppm_builder->state = WAITING_CHANNEL_MAX_VALUE; break;}
        case 7: {ppm_builder->state = WAITING_PX_RASTER; break;}
        case 15: {ppm_builder->state = PPM_CLOSING_STATE; break;}
    }
}

PpmReadHandler get_ppm_read_handler(PpmBuilder *ppm_builder)
{
    PpmReadHandler handler = NULL;
    switch (ppm_builder->state) {
        case WAITING_MAGIC_NUMBER: {handler = read_ppm_magic_number; break;}
        case WAITING_IMG_DIMENSIONS: {handler = read_ppm_dimensions; break;}
        case WAITING_CHANNEL_MAX_VALUE: {handler = read_ppm_max_channel_value; break;}
        case WAITING_PX_RASTER: {handler = read_ppm_px_raster; break;}
    }
    return handler;
}

PpmBuilder *create_ppm_builder(const char *path)
{
    if (!path) {
        return NULL;
    }

    PpmBuilder *ppm_builder = calloc(1, sizeof(*ppm_builder));
    if (!ppm_builder) {
        return NULL;
    }

    ppm_builder->fp = fopen(path, "rb");
    if (!ppm_builder->fp) {
        free(ppm_builder);
        return NULL;
    }

    return ppm_builder;
}

PpmBuilder *free_ppm_builder(PpmBuilder *ppm_builder)
{
    if (!ppm_builder) {
        return NULL;
    }

    fclose(ppm_builder->fp);
    free(ppm_builder);
    return NULL;
}

int8_t read_ppm_magic_number(PpmBuilder *ppm_builder)
{
    if (ppm_builder->state != WAITING_MAGIC_NUMBER) {
        return PPM_COMMAND_FAILED;
    }

    Ppm *img = ppm_builder->img;
    char *ppm_input = read_file_line(ppm_builder->fp);
    if (!ppm_input) {
        return PPM_COMMAND_FAILED;
    }

    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 1) {
        const char *magic_number_token = ppm_args.argv[0];
        strncpy(img->magic_bytes, magic_number_token, sizeof(img->magic_bytes) - 1);
        img->magic_bytes[2] = '\0';
        // Set magic number flag
        ppm_builder->img_flags |= (1 << MAGIC_NUMBER_FLAG);
    }
    
    free(ppm_input);
    return PPM_COMMAND_SUCCEEDED;
}

int8_t read_ppm_dimensions(PpmBuilder *ppm_builder)
{
    if (ppm_builder->state != WAITING_IMG_DIMENSIONS) {
        return PPM_COMMAND_FAILED;
    }

    Ppm *img = ppm_builder->img;
    char *ppm_input = read_file_line(ppm_builder->fp);
    if (!ppm_input) {
        return PPM_COMMAND_FAILED;
    }

    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 2) {
        const char *width_token = ppm_args.argv[0];
        const char *height_token = ppm_args.argv[1];
        img->width = atoi(width_token);
        img->height = atoi(height_token);
        // Set dimensions flag
        ppm_builder->img_flags |= (1 << DIMENSIONS_FLAG);
    }

    free(ppm_input);
    return PPM_COMMAND_SUCCEEDED;
}

int8_t read_ppm_max_channel_value(PpmBuilder *ppm_builder)
{
    if (ppm_builder->state != WAITING_CHANNEL_MAX_VALUE) {
        return PPM_COMMAND_FAILED;
    }

    Ppm *img = ppm_builder->img;
    char *ppm_input = read_file_line(ppm_builder->fp);
    if (!ppm_input) {
        return PPM_COMMAND_FAILED;
    }
    
    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 1) {
        const char *channel_max_value_token = ppm_args.argv[0];
        img->max_value_channel = atoi(channel_max_value_token);
        // Set channel max value flag
        ppm_builder->img_flags |= (1 << CHANNEL_MAX_VALUE_FLAG);
    }

    free(ppm_input);
    return PPM_COMMAND_SUCCEEDED;
}

int8_t read_ppm_px_raster(PpmBuilder *ppm_builder)
{
    if (ppm_builder->state != WAITING_PX_RASTER) {
        return PPM_COMMAND_FAILED;
    }

    Ppm *img = ppm_builder->img;
    uint32_t pixels = img->width * img->height;
    img->pixel_raster = malloc(pixels * sizeof(RgbPixel));
    if (!img->pixel_raster) {
        return PPM_COMMAND_FAILED;
    }
    fread(img->pixel_raster, sizeof(RgbPixel), pixels, ppm_builder->fp);
    // Set pixel raster flag
    ppm_builder->img_flags |= (1 << PX_RASTER_FLAG);
    return PPM_COMMAND_SUCCEEDED;
}

Ppm *open_ppm_file(const char *path)
{
    PpmBuilder *ppm_builder = create_ppm_builder(path);
    if (!ppm_builder) {
        return NULL;
    }

    // Memory allocation for .ppm img
    Ppm *img = calloc(1, sizeof(*img));
    if (!img) {
        free_ppm_builder(ppm_builder);
        return NULL;
    }
    ppm_builder->img = img;

    while (PPM_FILE_SCANNING) {
        // Checking if the file reached EOF
        if (feof(ppm_builder->fp)) {
            break;
        }

        ppm_change_state(ppm_builder);
        // Checking if the ppm builder's state is CLOSING_STATE
        if (ppm_builder->state == PPM_CLOSING_STATE) {
            break;
        }

        PpmReadHandler read_exec = get_ppm_read_handler(ppm_builder);
        int8_t exit_code = read_exec(ppm_builder);
        if (!exit_code) {
            free_ppm_builder(ppm_builder);
            return close_ppm_file(img);
        }
    }

    // Checking if all img flags are set (validating file)
    if (ppm_builder->img_flags != 15) {
        img = close_ppm_file(img);
    }

    free_ppm_builder(ppm_builder);
    return img;
}

Ppm *close_ppm_file(Ppm *ppm)
{
    if (!ppm) {
        return NULL;
    }

    free(ppm->pixel_raster);
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

    fwrite(ppm->magic_bytes, sizeof(char), strlen(ppm->magic_bytes), fp);
    putc('\n', fp);
    sprintf(buffer, "%d %d\n%d\n", ppm->width, ppm->height, ppm->max_value_channel);
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    uint32_t pixels = ppm->width * ppm->height;
    fwrite(ppm->pixel_raster, sizeof(*ppm->pixel_raster), pixels, fp);
    fclose(fp);
}