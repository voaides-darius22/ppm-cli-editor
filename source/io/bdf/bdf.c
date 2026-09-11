#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../../header_files/data_structure/hash_table.h"
#include "../../../header_files/data_structure/trie.h"
#include "../../../header_files/utils/io_utils.h"
#include "../../../header_files/io/app_file.h"
#include "../../../header_files/io/app_file_protected.h"
#include "../../../header_files/io/bdf/bdf_glyph.h"
#include "../../../header_files/io/bdf/bdf.h"

#define MAX_ARGS 8
#define ASCII_TABLE_SIZE 256
#define SCANNING 1

// Font Flags
#define STARTFONT_FLAG 0
#define FONT_NAME_FLAG 1
#define CHARS_FLAG 2
#define GLYPHS_FLAG 3 // This flag activates when all glyphs have been processed
#define ENDFONT_FLAG 4
#define BDF_FILE_VALID 31

typedef struct BdfArgs{
    uint8_t argc;
    char *argv[MAX_ARGS];
} BdfArgs;

typedef struct Bdf{
    AppFile base;
    // Bdf Data Protected
    float version;
    char *name;
    int32_t nglyphs;
    HashTable *glyphs;
} Bdf;

static const AppFileVTable BdfFileVTable = {
    .load_file = load_bdf,
    .destroy_file = bdf_dtor
};

typedef enum {
    INIT_STATE, WAITING_FONT_NAME, WAITING_NGLYPHS, WAITING_GLYPH_NAME, 
    WAITING_GLYPH_SETTINGS, WAITING_GLYPH_BITMAP, END_READING_GLYPH, CLOSING_STATE
} BdfFontState;

typedef struct {
    Glyph *current_glyph;
    int32_t processed_glyphs;
    int8_t glyph_flags;
} BdfGlyphBuilder;

typedef struct {
    FILE *fp;
    Bdf *file;
    BdfFontState state;
    BdfGlyphBuilder glyph_builder;
    int8_t font_flags;
} BdfBuilder;

typedef int8_t (*BdfHandler)(BdfBuilder *, BdfArgs);

static uint32_t hash_bdf_glyph_helper(const void *key, uint32_t capacity)
{
    const uint16_t encoding = *(const uint16_t *)key;
    return (capacity - encoding % capacity) % capacity;
}

static BdfArgs bdf_tokenizer(char *buffer)
{
    BdfArgs bdf_args;
    memset(&bdf_args, 0, sizeof(bdf_args));

    if (!buffer) {
        return bdf_args;
    }

    char *token = strtok(buffer, " ");
    while (token && bdf_args.argc < MAX_ARGS) {
        bdf_args.argv[bdf_args.argc] = token;
        bdf_args.argc++;
        token = strtok(NULL, " ");
    }
    return bdf_args;
}

// Bdf State Functions
static int8_t start_bdf_font(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return 0;
    }

    if (builder->state != INIT_STATE) {
        return 0;
    }

    Bdf *font = builder->file;
    const uint32_t capacity = ASCII_TABLE_SIZE;
    font->glyphs = create_hash_table(capacity, hash_bdf_glyph_helper);
    if (!font->glyphs) {
        free(font);
        return 0;
    }
    char *version_token = bdf_args.argv[0];
    font->version = atof(version_token);

    builder->font_flags |= (1 << STARTFONT_FLAG);
    return 1;    
}

static int8_t set_bdf_font_name(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return 0;
    }
    
    if (builder->state != WAITING_FONT_NAME) {
        return 0;
    }

    Bdf *font = builder->file;
    char *name = bdf_args.argv[0];
    font->name = malloc(strlen(name) + 1);
    if (!font->name) {
        return 0;
    }
    strcpy(font->name, name);

    builder->font_flags |= (1 << FONT_NAME_FLAG);
    return 1;
}

static int8_t set_bdf_font_nglyphs(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return 0;
    }

    if (builder->state != WAITING_NGLYPHS) {
        return 0;
    }

    char *nglyphs_token = bdf_args.argv[0];
    Bdf *font = builder->file;
    font->nglyphs = atoi(nglyphs_token);
    if (font->nglyphs <= 0) {
        return 0;
    }
    
    builder->font_flags |= (1 << CHARS_FLAG);
    return 1;
}

static int8_t set_bdf_glyph_name(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return 0;
    }

    if (builder->state != WAITING_GLYPH_NAME) {
        return 0;
    }

    // Memory allocation for a new glyph
    builder->glyph_builder.current_glyph = calloc(1, sizeof(Glyph));
    Glyph *glyph = builder->glyph_builder.current_glyph;
    if (!glyph) {
        return 0;
    }

    char *glyph_name = bdf_args.argv[0];
    glyph->name = malloc(strlen(glyph_name) + 1);
    if (!glyph_name) {
        free(glyph);
        builder->glyph_builder.current_glyph = NULL;
        return 0;
    }
    strcpy(glyph->name, glyph_name);

    // Set name member flag
    builder->glyph_builder.glyph_flags |= (1 << GLYPH_NAME_FLAG);
    return 1;
}

static int8_t set_bdf_glyph_encoding(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 1) {
        return 0;
    }

    if (builder->state != WAITING_GLYPH_SETTINGS) {
        return 0;
    }

    char *encoding_token = bdf_args.argv[0];
    Glyph *glyph = builder->glyph_builder.current_glyph;
    glyph->encoding = atoi(encoding_token);

    // Set encoding member flag
    builder->glyph_builder.glyph_flags |= (1 << GLYPH_ENCODING_FLAG);
    return 1;
}

static int8_t set_bdf_glyph_dwidth(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 2) {
        return 0;
    }

    if (builder->state != WAITING_GLYPH_SETTINGS) {
        printf("%d\n", builder->state);
        return 0;
    }

    int32_t dwx = atoi(bdf_args.argv[0]);
    int32_t dwy = atoi(bdf_args.argv[1]);

    Glyph *glyph = builder->glyph_builder.current_glyph;
    glyph->dwidth = create_dwidth(dwx, dwy);

    // Set dwidth member flag
    builder->glyph_builder.glyph_flags |= (1 << GLYPH_DWIDTH_FLAG);
    return 1;
}

static int8_t set_bdf_glyph_bbx(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 4) {
        return 0;
    }

    if (builder->state != WAITING_GLYPH_SETTINGS) {
        return 0;
    }

    int32_t BBw = atoi(bdf_args.argv[0]);
    int32_t BBh = atoi(bdf_args.argv[1]);
    int32_t BBxoff = atoi(bdf_args.argv[2]);
    int32_t BByoff = atoi(bdf_args.argv[3]);

    Glyph *glyph = builder->glyph_builder.current_glyph;
    glyph->bbx = create_bbx(BBw, BBh, BBxoff, BByoff);

    // Set bbx member flag
    builder->glyph_builder.glyph_flags |= (1 << GLYPH_BBX_FLAG);
    return 1;
}

static int8_t set_bdf_glyph_bitmap(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return 0;
    }

    if (builder->state != WAITING_GLYPH_BITMAP) {
        return 0;
    }

    Glyph *glyph = builder->glyph_builder.current_glyph;
    
    // Memory allocation for glyph's bitmap
    int32_t padding = compute_glyph_padding(glyph);
    int32_t row_length = (padding + glyph->bbx.BBw) / CHAR_BIT;
    glyph->bitmap = calloc(row_length * glyph->bbx.BBh, sizeof(*glyph->bitmap));
    if (!glyph->bitmap) {
        return 0;
    }

    // Reading glyph's bitmap
    for (int i = 0; i < glyph->bbx.BBh; i++) {
        char *bitmap_row = glyph->bitmap + i * row_length;
        for (int j = 0; j < row_length; j++) {
            if (fscanf(builder->fp, " %2hhx", &bitmap_row[j]) == EOF) {
                free(glyph->bitmap);
                glyph->bitmap = NULL;
                return 0;
            }
        }
    }
    
    // Set bitmap member flag
    builder->glyph_builder.glyph_flags |= (1 << GLYPH_BITMAP_FLAG);
    return 1;
}

static int8_t add_bdf_glyph(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return 0;
    }

    if (builder->state != END_READING_GLYPH) {
        return 0;
    }

    Bdf *font = builder->file;
    Glyph *glyph = builder->glyph_builder.current_glyph;

    // Memory allocation for glyph key (encoding)
    int16_t *encoding_key = malloc(sizeof(*encoding_key));
    if (!encoding_key) {
        return 0;
    }
    *encoding_key = glyph->encoding;

    Glyph *old_glyph = put(font->glyphs, encoding_key, glyph, cmp_hash_key_bdf_glyph);
    // Checking if the bdf font contains already a glyph with the same encoding key (overwrite)
    if (old_glyph) {
        free_glyph(old_glyph);
        free(encoding_key);
    }

    builder->glyph_builder.processed_glyphs++;    
    // Resetting Glyph Builder
    builder->glyph_builder.current_glyph = NULL;
    builder->glyph_builder.glyph_flags = 0;

    if (builder->glyph_builder.processed_glyphs == font->nglyphs) {
        builder->font_flags |= (1 << GLYPHS_FLAG);
    }
    return 1;
}

static int8_t end_bdf_font(BdfBuilder *builder, BdfArgs bdf_args)
{
    if (bdf_args.argc != 0) {
        return 0;
    }

    if (builder->state != CLOSING_STATE) {
        return 0;
    }

    builder->font_flags |= (1 << ENDFONT_FLAG);
    return 1;
}

static TrieNode *create_bdf_dictionary(void)
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

static void bdf_change_state(BdfBuilder *builder)
{
    switch (builder->font_flags) {
        case 0: {builder->state = INIT_STATE; break;}
        case 1: {builder->state = WAITING_FONT_NAME; break;}
        case 3: {builder->state = WAITING_NGLYPHS; break;}
        case 7: {
            int8_t glyph_flags = builder->glyph_builder.glyph_flags;
            if (glyph_flags == 0) {
                builder->state = WAITING_GLYPH_NAME;
            } else if (glyph_flags == 15) {
                builder->state = WAITING_GLYPH_BITMAP;
            } else if (glyph_flags == 31) {
                builder->state = END_READING_GLYPH;
            } else {
                builder->state = WAITING_GLYPH_SETTINGS;
            }
            break;
        }
        case 15: {builder->state = CLOSING_STATE; break;}
    }
}

static void free_bdf_builder(BdfBuilder **addr_builder)
{
    if (!addr_builder || !*addr_builder) {
        return;
    }

    BdfBuilder *builder = *addr_builder;
    if (builder->fp) {
        fclose(builder->fp);
    }
    free_glyph(builder->glyph_builder.current_glyph);
    free(builder);
    *addr_builder = NULL;
}

static BdfBuilder *create_bdf_builder(Bdf *self)
{
    BdfBuilder *builder = (self) ? calloc(1, sizeof(BdfBuilder)) : NULL;
    if (!builder) {
        return NULL;
    }
    
    // File Opening (Read Mode)
    const char *path = get_app_file_path((const AppFile*)self);
    builder->fp = fopen(path, "r");
    if (!builder->fp) {
        free_bdf_builder(&builder);
        return NULL;
    }
    
    builder->file = self;
    return builder;
}

// Bdf Methods
float get_bdf_font_version(const Bdf *self)
{
    return (self) ? self->version : 0;
}

const char *get_bdf_font_name(const Bdf *self)
{
    return (self) ? self->name : NULL;
}

int32_t get_bdf_font_nglyphs(const Bdf *self)
{
    return (self) ? self->nglyphs : 0;
}

const HashTable *get_bdf_font_glyphs_table(const Bdf *self)
{
    return (self) ? self->glyphs : NULL;
}

Bdf *bdf_ctor(const char *path)
{
    Bdf *file = (path) ? calloc(1, sizeof(Bdf)) : NULL;
    init_file_app((AppFile**)&file, path, &BdfFileVTable, BDF);
    if (!file) {
        return NULL;
    }
    // File Loading
    app_file_load((AppFile**)&file);
    return file;
}

void bdf_dtor(AppFile **self)
{
    Bdf *file = (self) ? (Bdf*)*self : NULL;
    free(file->name);
    free_hash_table(file->glyphs, free_glyph, free);
}

void load_bdf(AppFile **self)
{
    BdfBuilder *builder = (self) ? create_bdf_builder((Bdf*)*self) : NULL;
    if (!builder) {
        app_file_dtor((AppFile**)self);
        return;
    }

    // Memory allocation for bdf dictionary (contains Bdf Arguments)
    TrieNode *bdf_dictionary = create_bdf_dictionary();
    if (!bdf_dictionary) {
        free_bdf_builder(&builder);
        app_file_dtor((AppFile**)self);
        return;
    }

    while (SCANNING) {
        bdf_change_state(builder);
        char *bdf_input = read_file_line(builder->fp);
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
            int8_t bdf_exit_code = bdf_exec(builder, bdf_args);
            // Checking if bdf exec failed
            if (!bdf_exit_code) {
                free(bdf_input);
                free_bdf_builder(&builder);
                free_trie(bdf_dictionary, NULL);
                app_file_dtor((AppFile **)self);
                return;
            }
        }
        free(bdf_input);
    }

    // Checking if all font flags are set (validating file)
    if (builder->font_flags != BDF_FILE_VALID) {
        app_file_dtor((AppFile **)self);
    } else {
        printf(
            "Loaded %s (bitmap font %s)\n", 
            get_app_file_path(*self), get_bdf_font_name((const Bdf*)*self)
        );
    }
    free_trie(bdf_dictionary, NULL);
    free_bdf_builder(&builder);
}