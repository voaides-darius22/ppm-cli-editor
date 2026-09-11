#pragma once
#include <stdint.h>
#include "../ppm/ppm.h"

// Glyph Flags
#define GLYPH_NAME_FLAG 0
#define GLYPH_ENCODING_FLAG 1
#define GLYPH_DWIDTH_FLAG 2
#define GLYPH_BBX_FLAG 3
#define GLYPH_BITMAP_FLAG 4

// DWidth contains the cursor offset settings after writing a glyph
typedef struct Dwidth{
    int32_t dwx, dwy;
} DWidth;

DWidth create_dwidth(int32_t dwx, int32_t dwy);

// Bounding Box contains the dimensions of the glyph matrix and the position of the bottom-left
// corner of the glyph
typedef struct BBx{
    int32_t BBw, BBh;
    int32_t BBxoff, BByoff;
} Bbx;

Bbx create_bbx(int32_t BBw, int32_t BBh, int32_t BBxoff, int32_t BByoff);

typedef struct Glyph{
    char *name;
    int16_t encoding;
    DWidth dwidth;
    Bbx bbx;
    int8_t *bitmap;
} Glyph;

// Glyph Methods
int32_t get_glyph_dwx(const Glyph *self);
int32_t get_glyph_dwy(const Glyph *self);
int32_t get_glyph_BBw(const Glyph *self);
int32_t get_glyph_BBh(const Glyph *self);
int32_t get_glyph_BBxoff(const Glyph *self);
int32_t get_glyph_BByoff(const Glyph *self);
const char *get_glyph_name(const Glyph *self);
int16_t get_glyph_encoding(const Glyph *self);
int8_t *get_glyph_bitmap(const Glyph *self);
int8_t does_bdf_glyph_fit(Ppm *img, int32_t origin_x, int32_t origin_y, Glyph *self);
int32_t compute_glyph_padding(Glyph *self);
uint8_t cmp_hash_key_bdf_glyph(const void *value_1, const void *value_2);
void free_glyph(void *self);