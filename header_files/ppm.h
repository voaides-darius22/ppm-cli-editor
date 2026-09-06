#pragma once
#include <stdio.h>
#include <stdint.h>

#define PPM_MAX_ARGS 2
#define PPM_COMMAND_FAILED 0
#define PPM_COMMAND_SUCCEEDED 1

#define MAGIC_NUMBER_FLAG 0
#define DIMENSIONS_FLAG 1
#define CHANNEL_MAX_VALUE_FLAG 2
#define PX_RASTER_FLAG 3

#define PPM_FILE_SCANNING 1

typedef struct {
    uint8_t red_channel;
    uint8_t green_channel;
    uint8_t blue_channel;
} RgbPixel;

typedef struct {
    char magic_bytes[3];
    uint32_t width, height;
    uint8_t max_value_channel;
    RgbPixel *pixel_raster;
} Ppm;

typedef struct {
    uint8_t argc;
    const char *argv[PPM_MAX_ARGS];
} PpmArgs;

PpmArgs ppm_tokenizer(char *ppm_input);

typedef enum {
    PPM_INIT_STATE,
    WAITING_MAGIC_NUMBER, WAITING_IMG_DIMENSIONS, WAITING_CHANNEL_MAX_VALUE, WAITING_PX_RASTER,
    PPM_CLOSING_STATE
} PpmState;

typedef struct {
    FILE *fp;
    Ppm *img;
    PpmState state;
    int8_t img_flags;
} PpmBuilder;

PpmBuilder *create_ppm_builder(const char *path);
PpmBuilder *free_ppm_builder(PpmBuilder *ppm_builder);

typedef int8_t (*PpmReadHandler)(PpmBuilder *);
void ppm_change_state(PpmBuilder *ppm_builder);
PpmReadHandler get_ppm_read_handler(PpmBuilder *ppm_builder);

int8_t read_ppm_magic_number(PpmBuilder *ppm_builder);
int8_t read_ppm_dimensions(PpmBuilder *ppm_builder);
int8_t read_ppm_max_channel_value(PpmBuilder *ppm_builder);
int8_t read_ppm_px_raster(PpmBuilder *ppm_builder);

Ppm *open_ppm_file(const char *path);
Ppm *close_ppm_file(Ppm *ppm);
void write_ppm_file(const char *path, Ppm *ppm);