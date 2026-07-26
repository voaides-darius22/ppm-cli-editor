#pragma once
#include <stdint.h>

typedef struct RgbPixel {
    uint8_t red_channel;
    uint8_t green_channel;
    uint8_t blue_channel;
} RgbPixel;

typedef struct Ppm {
    char magic_bytes[3];
    uint32_t width, height;
    uint8_t max_value_channel;
    RgbPixel *pixel_buffer;
} Ppm;

Ppm *open_ppm_file(const char *path);
Ppm *close_ppm_file(Ppm *ppm);
void write_ppm_file(const char *path, Ppm *ppm);