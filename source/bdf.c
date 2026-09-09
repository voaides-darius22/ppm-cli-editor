#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../header_files/bdf.h"

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

// Glyph Position
#define VALID_GLYPH_POSITION 1

typedef int8_t (*BdfHandler)(BdfBuilder *, BdfArgs);

typedef struct BdfArgs{
    uint8_t argc;
    char *argv[BDF_MAX_ARGS];
} BdfArgs;

// DWidth contains the cursor offset settings after writing a glyph
typedef struct Dwidth{
    int32_t dwx, dwy;
} DWidth;

// Bounding Box contains the dimensions of the glyph matrix and the position of the bottom-left
// corner of the glyph
typedef struct BBx{
    int32_t BBw, BBh;
    int32_t BBxoff, BByoff;
} Bbx;

typedef struct BdfGlyph{
    char *name;
    int16_t encoding;
    DWidth dwidth;
    Bbx bbx;
    int8_t *bitmap;
} BdfGlyph;

typedef struct Bdf{
    char *file_path;
    float version;
    char *name;
    int32_t nglyphs;
    HashTable *glyphs;
} Bdf;

typedef struct {
    BdfGlyph *current_glyph;
    uint32_t processed_glyphs;
    int8_t glyph_flags;
} BdfGlyphBuilder;

typedef enum {
    INIT_STATE,
    WAITING_FONT_NAME, WAITING_NGLYPHS,
    WAITING_GLYPH_NAME, WAITING_GLYPH_SETTINGS, WAITING_GLYPH_BITMAP,
    END_READING_GLYPH,
    CLOSING_STATE
} BdfFontState;

typedef struct BdfBuilder{
    FILE *fp;
    BdfFontState state;
    Bdf *font;
    BdfGlyphBuilder glyph_builder;
    int8_t font_flags;
} BdfBuilder;

DWidth create_dwidth(int32_t dwx, int32_t dwy)
{
    DWidth dwidth;
    dwidth.dwx = dwx;
    dwidth.dwy = dwy;
    return dwidth;
}

Bbx create_bbx(int32_t BBw, int32_t BBh, int32_t BBxoff, int32_t BByoff)
{
    Bbx bbx;
    bbx.BBw = BBw;
    bbx.BBh = BBh;
    bbx.BBxoff = BBxoff;
    bbx.BByoff = BByoff;
    return bbx;
}

// BdfGlyph Getters
int32_t get_glyph_dwx(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->dwidth.dwx : 0;
}

int32_t get_glyph_dwy(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->dwidth.dwy : 0;
}

int32_t get_glyph_BBw(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->bbx.BBw : 0;
}

int32_t get_glyph_BBh(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->bbx.BBh : 0;
}

int32_t get_glyph_BBxoff(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->bbx.BBxoff : 0;
}

int32_t get_glyph_BByoff(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->bbx.BByoff : 0;
}

char *get_glyph_name(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->name : NULL;
}

int16_t get_glyph_encoding(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->encoding : -1;
}

int8_t *get_glyph_bitmap(const BdfGlyph *glyph)
{
    return (glyph) ? glyph->bitmap : NULL;
}

// BdfGlyph Functions
int8_t does_bdf_glyph_fit(Ppm *img, int32_t origin_x, int32_t origin_y, BdfGlyph *glyph)
{
    if (!img) {
        return !VALID_GLYPH_POSITION;
    }

    int32_t bottom_left_corner_x = origin_x + glyph->bbx.BBxoff;
    int32_t bottom_left_corner_y = origin_y + glyph->bbx.BByoff;
    int32_t top_right_corner_x = bottom_left_corner_x + glyph->bbx.BBw;
    int32_t top_right_corner_y = bottom_left_corner_y - glyph->bbx.BBh;

    if (bottom_left_corner_x < 0 || top_right_corner_y < 0) {
        return !VALID_GLYPH_POSITION;
    }

    if (top_right_corner_x >= get_ppm_width(img) || bottom_left_corner_y >= get_ppm_height(img)) {
        return !VALID_GLYPH_POSITION;
    }

    return VALID_GLYPH_POSITION;
}

uint32_t hash_bdf_glyph_helper(const void *key, uint32_t capacity)
{
    const uint16_t encoding = *(const uint16_t *)key;
    return (capacity - encoding % capacity) % capacity;
}

int32_t compute_glyph_padding(BdfGlyph *glyph)
{   
    int32_t BBw = glyph->bbx.BBw;
    return (CHAR_BIT - (BBw % CHAR_BIT)) % CHAR_BIT;  
}

uint8_t cmp_hash_key_bdf_glyph(const void *value_1, const void *value_2)
{
    if (!value_1 || !value_2) {
        return 0;
    }

    const HashTablePair *pair = value_1;
    const int16_t encoding_1 = *(const int16_t *)pair->key;
    const int16_t encoding_2 = *(const int16_t *)value_2;
    return (encoding_1 == encoding_2) ? 0 : 1;
}

// BdfGlyph Destructor
void free_bdf_glyph(void *glyph_ptr)
{
    if (!glyph_ptr) {
        return;
    }

    BdfGlyph *glyph = glyph_ptr;
    if (glyph->name) {
        free(glyph->name);
    }
    
    if (glyph->bitmap) {
        free(glyph->bitmap);
    }
    
    free(glyph);
}

BdfArgs bdf_tokenizer(char *bdf_line_args)
{
    BdfArgs bdf_args;
    memset(&bdf_args, 0, sizeof(bdf_args));

    if (!bdf_line_args) {
        return bdf_args;
    }

    char *token = strtok(bdf_line_args, " ");
    while (token && bdf_args.argc < BDF_MAX_ARGS) {
        bdf_args.argv[bdf_args.argc] = token;
        bdf_args.argc++;
        token = strtok(NULL, " ");
    }
    return bdf_args;
}

// BdfBuilder Functions
int8_t start_bdf_font(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != INIT_STATE) {
        return BDF_CMD_FAILED;
    }

    Bdf *font = bdf_builder->font;
    const uint32_t capacity = ASCII_TABLE_SIZE;
    font->glyphs = create_hash_table(capacity, hash_bdf_glyph_helper);
    if (!font->glyphs) {
        free(font);
        return BDF_CMD_FAILED;
    }
    char *version_token = bdf_args.argv[0];
    font->version = atof(version_token);

    bdf_builder->font_flags |= (1 << STARTFONT_FLAG);
    return BDF_CMD_SUCCEEDED;    
}

int8_t set_bdf_font_name(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return BDF_CMD_FAILED;
    }
    
    if (bdf_builder->state != WAITING_FONT_NAME) {
        return BDF_CMD_FAILED;
    }

    Bdf *font = bdf_builder->font;
    char *name = bdf_args.argv[0];
    font->name = malloc(strlen(name) + 1);
    if (!font->name) {
        return BDF_CMD_FAILED;
    }
    strcpy(font->name, name);

    bdf_builder->font_flags |= (1 << FONT_NAME_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_font_nglyphs(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_NGLYPHS) {
        return BDF_CMD_FAILED;
    }

    char *nglyphs_token = bdf_args.argv[0];
    Bdf *font = bdf_builder->font;
    font->nglyphs = atoi(nglyphs_token);
    if (font->nglyphs <= 0) {
        return BDF_CMD_FAILED;
    }
    
    bdf_builder->font_flags |= (1 << CHARS_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_glyph_name(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_GLYPH_NAME) {
        return BDF_CMD_FAILED;
    }

    // Memory allocation for a new glyph
    bdf_builder->glyph_builder.current_glyph = calloc(1, sizeof(BdfGlyph));
    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;
    if (!glyph) {
        return BDF_CMD_FAILED;
    }

    char *glyph_name = bdf_args.argv[0];
    glyph->name = malloc(strlen(glyph_name) + 1);
    if (!glyph_name) {
        free(glyph);
        bdf_builder->glyph_builder.current_glyph = NULL;
        return BDF_CMD_FAILED;
    }
    strcpy(glyph->name, glyph_name);

    // Set name member flag
    bdf_builder->glyph_builder.glyph_flags |= (1 << GLYPH_NAME_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_glyph_encoding(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_GLYPH_SETTINGS) {
        return BDF_CMD_FAILED;
    }

    char *encoding_token = bdf_args.argv[0];
    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;
    glyph->encoding = atoi(encoding_token);

    // Set encoding member flag
    bdf_builder->glyph_builder.glyph_flags |= (1 << GLYPH_ENCODING_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_glyph_dwidth(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 2) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_GLYPH_SETTINGS) {
        printf("%d\n", bdf_builder->state);
        return BDF_CMD_FAILED;
    }

    int32_t dwx = atoi(bdf_args.argv[0]);
    int32_t dwy = atoi(bdf_args.argv[1]);

    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;
    glyph->dwidth = create_dwidth(dwx, dwy);

    // Set dwidth member flag
    bdf_builder->glyph_builder.glyph_flags |= (1 << GLYPH_DWIDTH_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_glyph_bbx(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 4) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_GLYPH_SETTINGS) {
        return BDF_CMD_FAILED;
    }

    int32_t BBw = atoi(bdf_args.argv[0]);
    int32_t BBh = atoi(bdf_args.argv[1]);
    int32_t BBxoff = atoi(bdf_args.argv[2]);
    int32_t BByoff = atoi(bdf_args.argv[3]);

    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;
    glyph->bbx = create_bbx(BBw, BBh, BBxoff, BByoff);

    // Set bbx member flag
    bdf_builder->glyph_builder.glyph_flags |= (1 << GLYPH_BBX_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t set_bdf_glyph_bitmap(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != WAITING_GLYPH_BITMAP) {
        return BDF_CMD_FAILED;
    }

    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;
    
    // Memory allocation for glyph's bitmap
    int32_t padding = compute_glyph_padding(glyph);
    int32_t row_length = (padding + glyph->bbx.BBw) / CHAR_BIT;
    glyph->bitmap = calloc(row_length * glyph->bbx.BBh, sizeof(*glyph->bitmap));
    if (!glyph->bitmap) {
        return BDF_CMD_FAILED;
    }

    // Reading glyph's bitmap
    for (int i = 0; i < glyph->bbx.BBh; i++) {
        char *bitmap_row = glyph->bitmap + i * row_length;
        for (int j = 0; j < row_length; j++) {
            if (fscanf(bdf_builder->fp, " %2hhx", &bitmap_row[j]) == EOF) {
                free(glyph->bitmap);
                glyph->bitmap = NULL;
                return BDF_CMD_FAILED;
            }
        }
    }
    
    // Set bitmap member flag
    bdf_builder->glyph_builder.glyph_flags |= (1 << GLYPH_BITMAP_FLAG);
    return BDF_CMD_SUCCEEDED;
}

int8_t add_bdf_glyph(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != END_READING_GLYPH) {
        return BDF_CMD_FAILED;
    }

    Bdf *font = bdf_builder->font;
    BdfGlyph *glyph = bdf_builder->glyph_builder.current_glyph;

    // Memory allocation for glyph key (encoding)
    int16_t *encoding_key = malloc(sizeof(*encoding_key));
    if (!encoding_key) {
        return BDF_CMD_FAILED;
    }
    *encoding_key = glyph->encoding;

    BdfGlyph *old_glyph = put(font->glyphs, encoding_key, glyph, cmp_hash_key_bdf_glyph);
    // Checking if the bdf font contains already a glyph with the same encoding key (overwrite)
    if (old_glyph) {
        free_bdf_glyph(old_glyph);
        free(encoding_key);
    }

    bdf_builder->glyph_builder.processed_glyphs++;    
    // Resetting Glyph Builder
    bdf_builder->glyph_builder.current_glyph = NULL;
    bdf_builder->glyph_builder.glyph_flags = 0;

    if (bdf_builder->glyph_builder.processed_glyphs == font->nglyphs) {
        bdf_builder->font_flags |= (1 << GLYPHS_FLAG);
    }
    return BDF_CMD_SUCCEEDED;
}

int8_t end_bdf_font(BdfBuilder *bdf_builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return BDF_CMD_FAILED;
    }

    if (bdf_builder->state != CLOSING_STATE) {
        return BDF_CMD_FAILED;
    }

    bdf_builder->font_flags |= (1 << ENDFONT_FLAG);
    return BDF_CMD_SUCCEEDED;
}

TrieNode *create_bdf_dictionary(void)
{
    TrieNode *root = create_trie_node('\0', 0, NULL);
    const char *bdf_attributes[] = {
        "STARTFONT", "FONT", "CHARS", "ENDFONT",
        "STARTCHAR", "ENCODING", "DWIDTH", "BBX", "BITMAP", "ENDCHAR"
    };
    BdfHandler bdf_handlers[] = {
        start_bdf_font, set_bdf_font_name, set_bdf_font_nglyphs, end_bdf_font,
        set_bdf_glyph_name, set_bdf_glyph_encoding, set_bdf_glyph_dwidth,
        set_bdf_glyph_bbx, set_bdf_glyph_bitmap, add_bdf_glyph
    };

    uint32_t num_of_bdf_handlers = sizeof(bdf_handlers) / sizeof(*bdf_handlers);
    for (int i = 0; i < num_of_bdf_handlers; i++) {
        insert_word(root, bdf_attributes[i], bdf_handlers[i]);
    }

    return root;
}

void bdf_change_state(BdfBuilder *bdf_builder)
{
    switch (bdf_builder->font_flags) {
        case 0: {bdf_builder->state = INIT_STATE; break;}
        case 1: {bdf_builder->state = WAITING_FONT_NAME; break;}
        case 3: {bdf_builder->state = WAITING_NGLYPHS; break;}
        case 7: {
            int8_t glyph_flags = bdf_builder->glyph_builder.glyph_flags;
            if (glyph_flags == 0) {
                bdf_builder->state = WAITING_GLYPH_NAME;
            } else if (glyph_flags == 15) {
                bdf_builder->state = WAITING_GLYPH_BITMAP;
            } else if (glyph_flags == 31) {
                bdf_builder->state = END_READING_GLYPH;
            } else {
                bdf_builder->state = WAITING_GLYPH_SETTINGS;
            }
            break;
        }
        case 15: {bdf_builder->state = CLOSING_STATE; break;}
    }
}

// BdfBuilder Destructor
BdfBuilder *free_bdf_builder(BdfBuilder *bdf_builder)
{
    if (!bdf_builder) {
        return NULL;
    }

    if (bdf_builder->fp) {
        fclose(bdf_builder->fp);
    }

    if (bdf_builder->font) {
        close_bdf_font(bdf_builder->font);
    }

    if (bdf_builder->glyph_builder.current_glyph) {
        free_bdf_glyph(bdf_builder->glyph_builder.current_glyph);
    }

    free(bdf_builder);
    return NULL;
}

// BdfBuilder Constructor
BdfBuilder *create_bdf_builder(const char *path)
{
    if (!path) {
        return NULL;
    }
    
    BdfBuilder *bdf_builder = calloc(1, sizeof(*bdf_builder));
    if (!bdf_builder) {
        return NULL;
    }

    bdf_builder->fp = fopen(path, "r");
    if (!bdf_builder->fp) {
        return free_bdf_builder(bdf_builder);
    }

    return bdf_builder;
}

// Bdf Getters
char *get_bdf_font_path(const Bdf *font)
{
    return (font) ? font->file_path : NULL;
}

float get_bdf_font_version(const Bdf *font)
{
    return (font) ? font->version : -1;
}

char *get_bdf_font_name(const Bdf *font)
{
    return (font) ? font->name : NULL;
}

int32_t get_bdf_font_num_of_glyphs(const Bdf *font)
{
    return (font) ? font->nglyphs : -1;
}

HashTable *get_bdf_font_glyphs_table(const Bdf *font)
{
    return (font) ? font->glyphs : NULL;
}

// Bdf Constructor
Bdf *open_bdf_font(const char *path)
{
    if (!path) {
        return NULL;
    }

    TrieNode *bdf_dictionary = create_bdf_dictionary();
    if (!bdf_dictionary) {
        return NULL;
    }

    Bdf *font = calloc(1, sizeof(*font));
    if (!font) {
        free_trie(bdf_dictionary, NULL);
        return NULL;
    }

    font->file_path = malloc(strlen(path) + 1);
    if (!font->file_path) {
        free_trie(bdf_dictionary, NULL);
        return close_bdf_font(font);
    }
    strcpy(font->file_path, path);

    BdfBuilder *bdf_builder = create_bdf_builder(path);
    if (!bdf_builder) {
        free_trie(bdf_dictionary, NULL);
        return close_bdf_font(font);
    }

    bdf_builder->font = font;
    while (BDF_FILE_SCANNING) {
        bdf_change_state(bdf_builder);
        char *bdf_input = read_file_line(bdf_builder->fp);
        if (!bdf_input) {
            break;
        }

        // Get bdf attribute & argument
        char *space = strchr(bdf_input, ' ');
        char *bdf_line_args = NULL; // The rest of the line
        if (space) {
            *space = '\0';
            bdf_line_args = space + 1;
        }
        char *bdf_attribute = bdf_input;

        BdfHandler bdf_exec = get_word_value(bdf_dictionary, bdf_attribute);
        if (bdf_exec) {
            BdfArgs bdf_args;
            if (bdf_exec == set_bdf_font_name || bdf_exec == set_bdf_glyph_name) {
                if (bdf_line_args) {
                    bdf_args.argc = 1;
                    bdf_args.argv[0] = bdf_line_args;
                }
            } else {
                // Extracting bdf arguments
                bdf_args = bdf_tokenizer(bdf_line_args);
            }
            int8_t bdf_exit_code = bdf_exec(bdf_builder, bdf_args);
            // Checking if bdf exec failed
            if (!bdf_exit_code) {
                free(bdf_input);
                free_bdf_builder(bdf_builder);
                free_trie(bdf_dictionary, NULL);
                return NULL;
            }
        }
        free(bdf_input);
    }

    free_trie(bdf_dictionary, NULL);

    // Checking if all font flags are set (validating file)
    if (bdf_builder->font_flags != 31) {
        free_bdf_builder(bdf_builder);
        return NULL;
    } else {
        fclose(bdf_builder->fp);
        free(bdf_builder);
    }

    return font;
}

// Bdf Destructor
Bdf *close_bdf_font(Bdf *font)
{
    if (!font) {
        return NULL;
    }

    if (font->file_path) {
        free(font->file_path);
    }

    if (font->name) {
        free(font->name);
    }

    if (font->glyphs) {
        free_hash_table(font->glyphs, free_bdf_glyph, free);
    }

    free(font);
    return NULL;
}



