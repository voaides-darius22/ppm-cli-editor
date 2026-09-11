#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "../../../header_files/io/bdf/bdf_glyph.h"
#include "../../../header_files/io/ppm/ppm.h"
#include "../../../header_files/data_structure/hash_table.h"

DWidth create_dwidth(int32_t dwx, int32_t dwy)
{
    DWidth dwidth = {.dwx = dwx, .dwy = dwy};
    return dwidth;
}

Bbx create_bbx(int32_t BBw, int32_t BBh, int32_t BBxoff, int32_t BByoff)
{
    Bbx bbx = {.BBw = BBw, .BBh = BBh, .BBxoff = BBxoff, .BByoff = BByoff};
    return bbx;
}

// Glyph Methods
int32_t get_glyph_dwx(const Glyph *self)
{
    return (self) ? self->dwidth.dwx : 0;
}

int32_t get_glyph_dwy(const Glyph *self)
{
    return (self) ? self->dwidth.dwy : 0;
}

int32_t get_glyph_BBw(const Glyph *self)
{
    return (self) ? self->bbx.BBw : 0;
}

int32_t get_glyph_BBh(const Glyph *self)
{
    return (self) ? self->bbx.BBh : 0;
}

int32_t get_glyph_BBxoff(const Glyph *self)
{
    return (self) ? self->bbx.BBxoff : 0;
}

int32_t get_glyph_BByoff(const Glyph *self)
{
    return (self) ? self->bbx.BByoff : 0;
}

const char *get_glyph_name(const Glyph *self)
{
    return (self) ? self->name : NULL;
}

int16_t get_glyph_encoding(const Glyph *self)
{
    return (self) ? self->encoding : 0;
}

int8_t *get_glyph_bitmap(const Glyph *self)
{
    return (self) ? self->bitmap : NULL;
}

int8_t does_bdf_glyph_fit(Ppm *img, int32_t origin_x, int32_t origin_y, Glyph *self)
{
    if (!self) {
        return 0;
    }

    int32_t bottom_left_corner_x = origin_x + self->bbx.BBxoff;
    int32_t bottom_left_corner_y = origin_y + self->bbx.BByoff;
    int32_t top_right_corner_x = bottom_left_corner_x + self->bbx.BBw;
    int32_t top_right_corner_y = bottom_left_corner_y - self->bbx.BBh;

    if (bottom_left_corner_x < 0 || top_right_corner_y < 0) {
        return 0;
    }

    if (top_right_corner_x >= get_ppm_width(img) || bottom_left_corner_y >= get_ppm_height(img)) {
        return 0;
    }

    return 1;
}

int32_t compute_glyph_padding(Glyph *self)
{   
    int32_t BBw = self->bbx.BBw;
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

void free_glyph(void *self)
{
    
    Glyph *glyph = (self) ? self : NULL;
    if (!glyph) {
        return;
    }
    
    free(glyph->name);
    free(glyph->bitmap);
    free(glyph);
}