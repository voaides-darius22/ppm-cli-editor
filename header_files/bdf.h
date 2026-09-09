#pragma once

#include "ppm.h"
#include "hash_table.h"
#include "trie.h"
#include "io_utils.h"

typedef struct BdfGlyph BdfGlyph;
typedef struct Bdf Bdf;
typedef struct BdfArgs BdfArgs;
typedef struct BdfBuilder BdfBuilder;

// BdfGlyph Getters
int32_t get_glyph_dwx(const BdfGlyph *glyph);
int32_t get_glyph_dwy(const BdfGlyph *glyph);
int32_t get_glyph_BBw(const BdfGlyph *glyph);
int32_t get_glyph_BBh(const BdfGlyph *glyph);
int32_t get_glyph_BBxoff(const BdfGlyph *glyph);
int32_t get_glyph_BByoff(const BdfGlyph *glyph);
char *get_glyph_name(const BdfGlyph *glyph);
int16_t get_glyph_encoding(const BdfGlyph *glyph);
int8_t *get_glyph_bitmap(const BdfGlyph *glyph);

// BdfGlyph Functions
int8_t does_bdf_glyph_fit(Ppm *img, int32_t origin_x, int32_t origin_y, BdfGlyph *glyph);
int32_t compute_glyph_padding(BdfGlyph *glyph);
uint8_t cmp_hash_key_bdf_glyph(const void *value_1, const void *value_2);

// Bdf Getters
char *get_bdf_font_path(const Bdf *font);
float get_bdf_font_version(const Bdf *font);
char *get_bdf_font_name(const Bdf *font);
int32_t get_bdf_font_num_of_glyphs(const Bdf *font);
HashTable *get_bdf_font_glyphs_table(const Bdf *font);

// Bdf Constructor
Bdf *open_bdf_font(const char *path);

// Bdf Destructor
Bdf *close_bdf_font(Bdf *font);