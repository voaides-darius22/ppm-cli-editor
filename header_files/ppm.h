#pragma once
#include <stdio.h>
#include <stdint.h>

typedef struct Ppm Ppm;
typedef struct PpmBuilder PpmBuilder;

typedef struct {
    uint8_t red_channel;
    uint8_t green_channel;
    uint8_t blue_channel;
} RgbPixel;

// Ppm Getters
const char *get_ppm_magic_bytes(const Ppm *img);
char *get_ppm_path(const Ppm *img);
uint32_t get_ppm_width(const Ppm *img);
uint32_t get_ppm_height(const Ppm *img);
uint8_t get_ppm_max_value_channel(const Ppm *img);
RgbPixel *get_ppm_px_raster(const Ppm *img);
RgbPixel *clone_pixel_raster(Ppm *img);
void overwrite_pixel_raster(Ppm *img, RgbPixel *pixel_raster);

// Ppm Constructor
Ppm *open_ppm_file(const char *path);
void write_ppm_file(const char *path, Ppm *ppm);

// Ppm Destructor
Ppm *close_ppm_file(Ppm *ppm);

RgbPixel pixel_gamma_correction(RgbPixel *px, double gamma);
uint8_t channel_gamma_correction(uint8_t channel, double gamma);