#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "../header_files/ppm.h"
#include "../header_files/io_utils.h"

#define BUFFER_SIZE 1024

#define PPM_MAX_ARGS 2
#define PPM_COMMAND_FAILED 0
#define PPM_COMMAND_SUCCEEDED 1
#define PPM_FILE_SCANNING 1

#define MAGIC_NUMBER_FLAG 0
#define DIMENSIONS_FLAG 1
#define CHANNEL_MAX_VALUE_FLAG 2
#define PX_RASTER_FLAG 3

typedef int8_t (*PpmReadHandler)(PpmBuilder *);

typedef struct {
    uint8_t argc;
    const char *argv[PPM_MAX_ARGS];
} PpmArgs;

typedef enum {
    PPM_INIT_STATE,
    WAITING_MAGIC_NUMBER, WAITING_IMG_DIMENSIONS, WAITING_CHANNEL_MAX_VALUE, WAITING_PX_RASTER,
    PPM_CLOSING_STATE
} PpmState;

typedef struct Ppm{
    char *path;
    char magic_bytes[3];
    uint32_t width, height;
    uint8_t max_value_channel;
    RgbPixel *pixel_raster;
} Ppm;

typedef struct PpmBuilder{
    FILE *fp;
    Ppm *img;
    PpmState state;
    int8_t img_flags;
} PpmBuilder;

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
    while (token && ppm_args.argc < PPM_MAX_ARGS) {
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

// PpmBuilder Functions
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
        if (!strcmp(magic_number_token, "P6")) {
            strncpy(img->magic_bytes, magic_number_token, sizeof(img->magic_bytes) - 1);
            img->magic_bytes[2] = '\0';
            // Set magic number flag
            ppm_builder->img_flags |= (1 << MAGIC_NUMBER_FLAG);
        }
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

// PpmBuilder Constructor
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

// PpmBuilder Destructor
PpmBuilder *free_ppm_builder(PpmBuilder *ppm_builder)
{
    if (!ppm_builder) {
        return NULL;
    }

    fclose(ppm_builder->fp);
    free(ppm_builder);
    return NULL;
}

// Ppm Getters
const char *get_ppm_magic_bytes(const Ppm *img)
{
    return (img) ? img->magic_bytes : NULL;
}

char *get_ppm_path(const Ppm *img)
{
    return (img) ? img->path : NULL;
}

uint32_t get_ppm_width(const Ppm *img)
{
    return (img) ? img->width : 0;
}

uint32_t get_ppm_height(const Ppm *img)
{
    return (img) ? img->height : 0;
}

uint8_t get_ppm_max_value_channel(const Ppm *img)
{
    return (img) ? img->max_value_channel : 0;
}

RgbPixel *get_ppm_px_raster(const Ppm *img)
{
    return (img) ? img->pixel_raster : NULL;
}


RgbPixel *clone_pixel_raster(Ppm *img)
{
    if (!img) {
        return NULL;
    }

    uint32_t pixels = img->width * img->height;
    RgbPixel *pixel_raster_clone = malloc(pixels * sizeof(RgbPixel));
    if (!pixel_raster_clone) {
        return NULL;
    }
    memcpy(pixel_raster_clone, img->pixel_raster, pixels * sizeof(RgbPixel));
    return pixel_raster_clone;
}

void overwrite_pixel_raster(Ppm *img, RgbPixel *pixel_raster)
{
    if (!img || !pixel_raster) {
        return;
    }

    uint32_t pixels = img->width * img->height;
    memcpy(img->pixel_raster, pixel_raster, pixels * sizeof(RgbPixel));
}

// Ppm Constructor
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
    // Memory allocation for the path of hte .ppm img
    img->path = malloc(strlen(path) + 1);
    if (!img->path) {
        strcpy(img->path, path);
        free_ppm_builder(ppm_builder);
        return NULL;
    }
    strcpy(img->path, path);
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

// Ppm Destructor
Ppm *close_ppm_file(Ppm *ppm)
{
    if (!ppm) {
        return NULL;
    }

    free(ppm->path);
    free(ppm->pixel_raster);
    free(ppm);
    return NULL;
}

RgbPixel pixel_gamma_correction(RgbPixel *px, double gamma)
{
    RgbPixel new_px = {0, 0, 0};
    if (!px) {
        return new_px;
    }
    
    new_px.red_channel = channel_gamma_correction(px->red_channel, gamma);
    new_px.green_channel = channel_gamma_correction(px->green_channel, gamma);
    new_px.blue_channel = channel_gamma_correction(px->blue_channel, gamma);

    return new_px;
}

uint8_t channel_gamma_correction(uint8_t channel, double gamma)
{
    double normalized = channel / 255.0;
    double corrected = pow(normalized, gamma);
    return round(corrected * 255);
}