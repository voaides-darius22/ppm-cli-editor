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
int8_t export_ppm(const char *path, Ppm *self);
const char *get_ppm_magic_bytes(const Ppm *self);
uint32_t get_img_width(const Ppm *self);
uint32_t get_img_height(const Ppm *self);
uint32_t get_img_export_width(const Ppm *self);
uint32_t get_img_export_height(const Ppm *self);
uint8_t get_ppm_max_value_channel(const Ppm *self);
RgbPixel *get_img_original_raster(const Ppm *self);
RgbPixel *get_img_export_raster(const Ppm *self);
double get_img_brightness_level(const Ppm *self);
double get_img_angle(const Ppm *self);
RgbPixel *clone_pixel_raster(const Ppm *self);
void overwrite_pixel_raster(Ppm *self, const RgbPixel *pixel_raster);
void set_img_raster(
    Ppm *self, 
    uint32_t width, uint32_t height, 
    RgbPixel *pixel_raster
);
void set_img_export_raster(Ppm *self, RgbPixel *export_pixel_raster);
void set_img_export_width(Ppm *self, uint32_t export_width);
void set_img_export_height(Ppm *self, uint32_t export_height);
void set_img_brightness_level(Ppm *self, double brightness_level);
void add_img_angle(Ppm *self, double angle);
void set_img_angle(Ppm *self, double angle);
RgbPixel pixel_gamma_correction(RgbPixel *px, double gamma);
uint8_t channel_gamma_correction(uint8_t channel, double gamma);