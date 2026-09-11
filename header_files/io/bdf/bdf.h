#pragma once

#include "../app_file.h"
typedef struct Bdf Bdf;

// Bdf Methods
Bdf *bdf_ctor(const char *path);
void bdf_dtor(AppFile **self);
void load_bdf(AppFile **self);
float get_bdf_font_version(const Bdf *self);
const char *get_bdf_font_name(const Bdf *self);
int32_t get_bdf_font_nglyphs(const Bdf *self);
const HashTable *get_bdf_font_glyphs_table(const Bdf *self);