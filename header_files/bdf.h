#pragma once

#include <stdio.h>
#include <stdint.h>
#include "hash_table.h"
#include "trie.h"
#include "io_utils.h"

#define BDF_MAX_ARGS 8
#define ASCII_TABLE_SIZE 256
#define BDF_FILE_SCANNING 1
#define BDF_CMD_FAILED 0
#define BDF_CMD_SUCCEEDED 1

// Glyph Flags
#define GLYPH_NAME_FLAG 0
#define GLYPH_ENCODING_FLAG 1
#define GLYPH_DWIDTH_FLAG 2
#define GLYPH_BBX_FLAG 3
#define GLYPH_BITMAP_FLAG 4

// Font Flags
#define STARTFONT_FLAG 0
#define FONT_NAME_FLAG 1
#define CHARS_FLAG 2
#define GLYPHS_FLAG 3 // This flag activates when all glyphs have been processed
#define ENDFONT_FLAG 4

// DWidth contains the cursor offset settings after writing a glyph 
typedef struct {
    int32_t dwx, dwy;
} DWidth;

DWidth create_dwidth(int32_t dwx, int32_t dwy);

// Bounding Box contains the dimensions of the glyph matrix and the position of the bottom-left
// corner of the glyph
typedef struct {
    int32_t BBw, BBh;
    int32_t BBxoff, BByoff;
} Bbx;

Bbx create_bbx(int32_t BBw, int32_t BBh, int32_t BBxoff, int32_t BByoff);

typedef struct {
    char *name;
    int16_t encoding;
    DWidth dwidth;
    Bbx bbx;
    int8_t *bitmap;
} BdfGlyph;

uint32_t hash_bdf_glyph_helper(const void *key, uint32_t capacity);
uint32_t compute_glyph_padding(int32_t BBw);
uint8_t cmp_hash_key_bdf_glyph(const void *value_1, const void *value_2);
void free_bdf_glyph(void *glyph_ptr);

typedef struct {
    uint8_t argc;
    char *argv[BDF_MAX_ARGS];
} BdfArgs;
BdfArgs bdf_tokenizer(char *bdf_line_args);

typedef struct {
    char *file_path;
    float version;
    char *name;
    int32_t nglyphs;
    HashTable *glyphs;
} Bdf;

typedef enum {
    INIT_STATE,
    WAITING_FONT_NAME, WAITING_NGLYPHS,
    WAITING_GLYPH_NAME, WAITING_GLYPH_SETTINGS, WAITING_GLYPH_BITMAP,
    END_READING_GLYPH,
    CLOSING_STATE
} BdfFontState;

typedef struct {
    BdfGlyph *current_glyph;
    uint32_t processed_glyphs;
    int8_t glyph_flags;
} BdfGlyphBuilder;

TrieNode *create_bdf_dictionary(void);

typedef struct {
    FILE *fp;
    BdfFontState state;
    Bdf *font;
    BdfGlyphBuilder glyph_builder;
    int8_t font_flags;
} BdfBuilder;

void bdf_change_state(BdfBuilder *bdf_builder);
BdfBuilder *create_bdf_builder(const char *path);
BdfBuilder *free_bdf_builder(BdfBuilder *bdf_builder);

typedef int8_t (*BdfHandler)(BdfBuilder *, BdfArgs);

int8_t start_bdf_font(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_font_name(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_font_nglyphs(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_glyph_name(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_glyph_encoding(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_glyph_dwidth(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_glyph_bbx(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t set_bdf_glyph_bitmap(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t add_bdf_glyph(BdfBuilder *bdf_builder, BdfArgs bdf_args);
int8_t end_bdf_font(BdfBuilder *bdf_builder, BdfArgs bdf_args);

Bdf *open_bdf_font(const char *path);
Bdf *close_bdf_font(Bdf *font);