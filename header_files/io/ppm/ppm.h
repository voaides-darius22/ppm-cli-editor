#pragma once
#include <stdio.h>
#include <stdint.h>
#include "../app_file.h"

typedef struct Ppm Ppm;

typedef struct {
    uint8_t red_channel;
    uint8_t green_channel;
    uint8_t blue_channel;
} RgbPixel;

// Ppm Methods
Ppm *ppm_ctor(const char *path);
void ppm_dtor(AppFile **self);
void load_ppm(AppFile **self);
void export_ppm_file(const char *path, Ppm *self);
const char *get_ppm_magic_bytes(const Ppm *self);
uint32_t get_ppm_width(const Ppm *self);
uint32_t get_ppm_height(const Ppm *self);
uint8_t get_ppm_max_value_channel(const Ppm *self);
RgbPixel *get_ppm_px_raster(const Ppm *self);
RgbPixel *clone_pixel_raster(Ppm *self);
void overwrite_pixel_raster(Ppm *self, const RgbPixel *pixel_raster);
void set_ppm_px_raster(
    Ppm *self, 
    uint32_t width, uint32_t height, 
    RgbPixel *pixel_raster
);
RgbPixel pixel_gamma_correction(RgbPixel *px, double gamma);
uint8_t channel_gamma_correction(uint8_t channel, double gamma);