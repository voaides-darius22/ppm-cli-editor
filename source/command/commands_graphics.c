#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "../../header_files/command/command.h"
#include "../../header_files/command/command_protected.h"
#include "../../header_files/command/commands_graphics.h"
#include "../../header_files/utils/turtle_graphics.h"
#include "../../header_files/io/ppm/ppm.h"
#include "../../header_files/io/lsys/lsys.h"
#include "../../header_files/io/bdf/bdf.h"
#include "../../header_files/io/bdf/bdf_glyph.h"
#include "../../header_files/engine/cli_engine.h"
#include "../../header_files/engine/system_data.h"
#include "../../header_files/data_structure/hash_table.h"

typedef struct TurtleRequiredFiles {
    Ppm *file_1;
    Lsystem *file_2;
} TurtleRequiredFiles;

static TurtleRequiredFiles *create_turtle_required_files(Ppm *file_1, Lsystem *file_2)
{
    TurtleRequiredFiles *req = (file_1 && file_2) ? malloc(sizeof(TurtleRequiredFiles)) : NULL;
    if (!req) {
        return NULL;
    }

    req->file_1 = file_1;
    req->file_2 = file_2;
    return req;
}

static int8_t execute_turtle(Command *self)
{
    const TurtleRequiredFiles *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    Lsystem *file_2 = req->file_2;

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(file_1);
    if (!previous_pixel_raster) {
        return 0;        
    }
    set_cmd_memento(self, previous_pixel_raster);
    
    // Unpacking turtle arguments
    const CliArgs *args = get_cmd_args(self);
    double x = atof(args->argv[0]);
    double y = atof(args->argv[1]);
    uint32_t offset_step = atol(args->argv[2]);
    uint16_t orientation = atoi(args->argv[3]);
    uint8_t angular_step = atoi(args->argv[4]);
    uint32_t n = atoi(args->argv[5]);
    RgbPixel color;
    memset(&color, 0, sizeof(RgbPixel));
    color.red_channel = atoi(args->argv[6]);
    color.green_channel = atoi(args->argv[7]);
    color.blue_channel = atoi(args->argv[8]);
    
    // Computing the Nth derivative of the Lsystem
    char *derivative = derive_lsys(file_2, n);
    if (!derivative) {
        return 0;
    }

    // Memory allocation for graphic system
    GraphicSystem *graphic_system = create_graphic_system(
        file_1, x, y, offset_step, orientation, angular_step
    );
    if (!graphic_system) {
        free(derivative);
        return 0;
    }

    // Turtle Parser
    turtle_parser(derivative, graphic_system, color);
    close_graphic_system(graphic_system);
    free(derivative);
    printf("Drawing done\n");
    return 1;
}

static void undo_turtle(Command *self)
{
    const TurtleRequiredFiles *req = get_cmd_receiver(self);
    Ppm *file_1 = (Ppm*)req->file_1;
    const RgbPixel *previous_pixel_raster = get_cmd_memento(self); 
    const CliArgs *args = get_cmd_args(self);
    overwrite_pixel_raster(file_1, previous_pixel_raster);
    printf(
        "TURTLE %s %s %s %s %s %s %s %s %s has been canceled\n", 
        args->argv[0], args->argv[1], args->argv[2], args->argv[3], args->argv[4], 
        args->argv[5], args->argv[6], args->argv[7], args->argv[8]
    );
    free((RgbPixel*)previous_pixel_raster);
    set_cmd_memento(self, NULL);
}

static void turtle_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free((RgbPixel*)get_cmd_memento(*self));
    // Receiver member will store TurtleRequiredFiles
    free((TurtleRequiredFiles*)get_cmd_receiver(*self));
}

static const CommandVTable TurtleVTable = {
    .execute = execute_turtle,
    .undo = undo_turtle,
    .destroy_cmd = turtle_destructor
};

Command *create_turtle_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 9) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: TURTLE <x> <y> <offset_step> <orientation> <angular_step> <n> <R> <G> <B>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile *ppm = *get_file_addr(appdata, PPM);
    AppFile *lsys = *get_file_addr(appdata, LSYSTEM);

    // Checking if the system contains the required files for turtle
    if (!ppm) {
        printf("No image loaded\n");
    }

    if (!lsys) {
        printf("No L-system loaded\n");
    }

    TurtleRequiredFiles *req = create_turtle_required_files((Ppm*)ppm, (Lsystem*)lsys);
    return (req) ? cmd_ctor(1, req, args, &TurtleVTable) : NULL;
}

typedef struct TypeRequiredFiles {
    Ppm *file_1;
    Bdf *file_2;
} TypeRequiredFile;

TypeRequiredFile *create_type_required_files(Ppm *file_1, Bdf *file_2)
{
    TypeRequiredFile *req = (file_1 && file_2) ? malloc(sizeof(TypeRequiredFile)) : NULL;
    if (!req) {
        return NULL;
    }

    req->file_1 = file_1;
    req->file_2 = file_2;
    return req;
}

static int8_t execute_type(Command *self)
{
    const TypeRequiredFile *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    Bdf *file_2 = req->file_2;

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(file_1);
    if (!previous_pixel_raster) {
        return 0;        
    }
    set_cmd_memento(self, previous_pixel_raster);

    // Getting ppm members
    uint32_t img_width = get_ppm_width(file_1);
    uint32_t img_height = get_ppm_height(file_1);
    RgbPixel *pixel_raster = get_ppm_px_raster(file_1);
    
    // Unpacking type arguments
    const CliArgs *args = get_cmd_args(self);
    char *string = strtok(args->argv[0], "\"");
    int32_t cursor_x = atoi(args->argv[1]);
    int32_t cursor_y = atoi(args->argv[2]);
    RgbPixel color;
    memset(&color, 0, sizeof(RgbPixel));
    color.red_channel = atoi(args->argv[3]);
    color.green_channel = atoi(args->argv[4]);
    color.blue_channel = atoi(args->argv[5]);

    int32_t str_length = strlen(string);
    const HashTable *glyphs = get_bdf_font_glyphs_table(file_2);
    // Checking if the .bdf font contains all the required chars
    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        if (!get(glyphs, &ch, cmp_hash_key_bdf_glyph)) {
            printf("No glyph found for %c\n", (char)ch);
            return 0;
        }
    }

    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        Glyph *glyph = get(glyphs, &ch, cmp_hash_key_bdf_glyph);
        if (!does_bdf_glyph_fit(file_1, cursor_x, cursor_y, glyph)) {
            printf("Glyph %s doesn't fit in the img\n", get_glyph_name(glyph));
            break;
        }

        // Bottom left px of the glyph relative to img
        int32_t bottom_left_corner_x = cursor_x + get_glyph_BBxoff(glyph);
        int32_t bottom_left_corner_y = cursor_y + get_glyph_BByoff(glyph);
        int32_t bottom_left_corner_offset = bottom_left_corner_y * img_width + bottom_left_corner_x;
        RgbPixel *bottom_left_pixel = &pixel_raster[bottom_left_corner_offset];

        // Getting bdf members
        int32_t BBh = get_glyph_BBh(glyph);
        int8_t *bitmap = get_glyph_bitmap(glyph);
        int32_t BBw = get_glyph_BBw(glyph);

        int32_t glyph_padding = compute_glyph_padding(glyph);
        int32_t bitmap_row_length = (glyph_padding + BBw) / CHAR_BIT;
        
        // Drawing glyph (drawing direction is bottom-up & left-right)
        for (int glyph_row = BBh - 1, i = 0; glyph_row >= 0; glyph_row--, i++) {
            RgbPixel *px = bottom_left_pixel - i * img_width; 
            // Iterating through glyph's bitmap rows
            // Compute offset for row
            int8_t *bitmap_row = bitmap + glyph_row * bitmap_row_length;
            int32_t scanned_bits = 0;
            // Iterating through glyph's bitmap collumns (bytes)
            for (int collumn = 0; collumn < bitmap_row_length; collumn++) {
                int8_t bitmap_byte = bitmap_row[collumn];
                for (int idx = CHAR_BIT - 1; idx >= 0 && scanned_bits < BBw; idx--) {
                    scanned_bits++;
                    uint8_t bit = (bitmap_byte >> idx) & 1;
                    // Checking if the bit is set => pixel must be colored
                    if (bit) {
                        *px = color;
                    }
                    px++;
                }
            }
        }
        // Moving cursor position after writing a glyph
        cursor_x += get_glyph_dwx(glyph);
        cursor_y += get_glyph_dwy(glyph);
    }

    printf("Text written\n");
    return 1;
}

static void undo_type(Command *self)
{
    const TypeRequiredFile *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    const RgbPixel *previous_pixel_raster = get_cmd_memento(self);
    overwrite_pixel_raster(file_1, previous_pixel_raster);
    free((RgbPixel*)previous_pixel_raster);
    set_cmd_memento(self, NULL);
    printf("Text has been removed\n");
}

static void type_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free((RgbPixel*)get_cmd_memento(*self));
    // Receiver member will store TurtleRequiredFiles
    free((TurtleRequiredFiles*)get_cmd_receiver(*self));
}

static const CommandVTable TypeVTable = {
    .execute = execute_type,
    .undo = undo_type,
    .destroy_cmd = type_destructor
};

Command *create_type_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 6) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: TYPE \"<string>\" <start_x> <start_y> <R> <G> <B>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile *ppm = *get_file_addr(appdata, PPM);
    AppFile *bdf = *get_file_addr(appdata, BDF);

    if (!ppm) {
        printf("No image loaded\n");
    }

    if (!bdf) {
        printf("No font loaded\n");
    }

    TypeRequiredFile *req = (ppm && bdf) ? create_type_required_files((Ppm*)ppm, (Bdf*)bdf): NULL;
    return (req) ? cmd_ctor(1, req, args, &TypeVTable): NULL;
}

static int8_t execute_grayscale(Command *self)
{
    Ppm **file = (Ppm**)get_cmd_receiver(self);
    int32_t pixels = get_ppm_width(*file) * get_ppm_height(*file);

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(*file);
    if (!previous_pixel_raster) {
        return 0;        
    }
    set_cmd_memento(self, previous_pixel_raster);

    // BT.601 Standard (Broadcasting Service Television)
    // Luminance Coefficients (Multipliers)
    const float red_mul = 0.299;
    const float green_mul = 0.587;
    const float blue_mul = 0.114;

    RgbPixel *pixel_raster = get_ppm_px_raster(*file);    
    for (int i = 0; i < pixels; i++) {
        RgbPixel *px = &pixel_raster[i];
        // Computing gray luminance
        uint8_t Y = round(red_mul * px->red_channel + green_mul * px->green_channel + blue_mul *px->blue_channel);
        RgbPixel new_px = {Y, Y, Y};
        *px = new_px; 
    }

    printf("Grayscale filter has been applied\n");
    return 1;
}

void undo_grayscale(Command *self)
{
    Ppm **file = (Ppm**)get_cmd_receiver(self);
    const RgbPixel *previous_pixel_raster = get_cmd_memento(self);
    overwrite_pixel_raster(*file, previous_pixel_raster);
    free((RgbPixel*)previous_pixel_raster);
    set_cmd_memento(self, NULL);
    printf("Grayscale filter has been removed\n");
}

static void grayscale_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free((RgbPixel*)get_cmd_memento(*self));
    set_cmd_memento(*self, NULL);
}

static const CommandVTable GrayscaleVTable = {
    .execute = execute_grayscale,
    .undo = undo_grayscale,
    .destroy_cmd = grayscale_destructor
};

Command *create_grayscale_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 0) {
        printf("Error: GRAYSCALE accepts no arguments (%d given)\n", args->argc);
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile **ppm = get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(1, ppm, args, &GrayscaleVTable);
}

static int8_t execute_brightness(Command *self)
{
    Ppm **file = (Ppm**)get_cmd_receiver(self);
    int32_t pixels = get_ppm_width(*file) * get_ppm_height(*file);

     // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(*file);
    if (!previous_pixel_raster) {
        return 0;        
    }
    set_cmd_memento(self, previous_pixel_raster);
    
    // Unpacking Brightness arguments
    const CliArgs *args = get_cmd_args(self);
    double brightness_level = atof(args->argv[0]);
    if (brightness_level < 0 || brightness_level > 100) {
        printf("Brightness level must be between [0, 100]\n");
        return 0;
    }

    const double brightness_base = 4.0;
    float gamma = pow(brightness_base, (50 - brightness_level) / 50);
    
    RgbPixel *pixel_raster = get_ppm_px_raster(*file);
    for (int i = 0; i < pixels; i++) {
        RgbPixel *px = &pixel_raster[i];
        *px = pixel_gamma_correction(px, gamma);
    }
    printf("Image brightness level has been adjusted to %g%%\n", brightness_level);
    return 1;
}

static void undo_brightness(Command *self)
{
    Ppm **file = (Ppm**)get_cmd_receiver(self);
    const RgbPixel *previous_pixel_raster = get_cmd_memento(self);
    overwrite_pixel_raster(*file, (RgbPixel*)previous_pixel_raster);
    const CliArgs *args = get_cmd_args(self);
    double brightness_level = atof(args->argv[0]);
    if (brightness_level >= 50) {
        printf("Image brightness level has been reset (%g%%)\n", 50 - brightness_level);
    } else {
        printf("Image brightness level has been reset (+%g%%)\n", 50 - brightness_level);
    }
    free((RgbPixel*)previous_pixel_raster);
    set_cmd_memento(self, NULL);
}

static void brightness_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free((RgbPixel*)get_cmd_memento(*self));
    set_cmd_memento(*self, NULL);
}

static const CommandVTable BrightnessVTable = {
    .execute = execute_brightness,
    .undo = undo_brightness,
    .destroy_cmd = brightness_destructor
};

Command *create_brightness_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: BRIGHTNESS <level>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile **ppm = get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(1, ppm, args, &BrightnessVTable);
}

typedef struct PpmMetadata {
    uint32_t width, height;
    RgbPixel *pixel_raster;
} PpmMetadata;

static PpmMetadata *create_ppm_metadata(const Ppm *file)
{
    PpmMetadata *mtd = (file) ? malloc(sizeof(PpmMetadata)) : NULL;
    if (!mtd) {
        return NULL;
    }

    mtd->width = get_ppm_width(file);
    mtd->height = get_ppm_height(file);
    mtd->pixel_raster = clone_pixel_raster((Ppm*)file);
    if (!mtd->pixel_raster) {
        free(mtd);
        return NULL;
    }

    return mtd;
}

static void free_ppm_metadata(PpmMetadata **addr_mtd)
{
    PpmMetadata *mtd = (addr_mtd) ? *addr_mtd : NULL;
    if (!mtd) {
        return;
    }

    free(mtd->pixel_raster);
    free(mtd);
    *addr_mtd = NULL;
}

static int8_t execute_crop(Command *self)
{
    const Ppm *file = *(Ppm**)get_cmd_receiver(self);
    PpmMetadata *mtd = create_ppm_metadata(file);
    if (!mtd) {
        return 0;
    }
    set_cmd_memento(self, mtd);
    const CliArgs *args = get_cmd_args(self);

    // Unpacking Crop arguments (top-left & bottom-right corners)
    int32_t x1 = atoi(args->argv[0]);
    int32_t y1 = atoi(args->argv[1]);
    int32_t x2 = atoi(args->argv[2]);
    int32_t y2 = atoi(args->argv[3]);

    int32_t cropped_width = x2 - x1 + 1;
    int32_t cropped_height = y2 - y1 + 1;
    // Checking if the position of the corners are valid
    if (cropped_width <= 0 || cropped_height <= 0) {
        printf("Top-Left corner is lower than Bottom-Right corner\n");
        return 0;
    }
    if (x1 + cropped_width > mtd->width || y1 + cropped_height > mtd->height) {
        printf("Corners overflow the image dimension\n");
        return 0;
    }
    
    // Memory allocation for the cropped pixel raster
    int32_t crop_pixels = cropped_width * cropped_height;
    RgbPixel *cropped_pixel_raster = malloc(crop_pixels * sizeof(RgbPixel));
    if (!cropped_pixel_raster) {
        return 0;
    }
    for (int i = 0; i < cropped_height; i++) {
        int32_t offset = (y1 + i) * mtd->width + x1;
        memcpy(
            cropped_pixel_raster + i * cropped_width,
            mtd->pixel_raster + offset,
            cropped_width * sizeof(RgbPixel)
        );
    }

    set_ppm_px_raster(
        (Ppm*)file, 
        (uint32_t)cropped_width, (uint32_t)cropped_height, 
        cropped_pixel_raster
    );

    printf("Image has been cropped to (%dx%d)\n", cropped_width, cropped_height);
    return 1;
}

static void undo_crop(Command *self)
{
    const Ppm *file = *(Ppm**)get_cmd_receiver(self);
    const PpmMetadata *mtd = get_cmd_memento(self);
    printf(
        "(PPM image %dx%d) has been restored to (%dx%d)\n",
        get_ppm_width(file), get_ppm_height(file),
        mtd->width, mtd->height
    );
    set_ppm_px_raster((Ppm*)file, mtd->width, mtd->height, mtd->pixel_raster);
    free((PpmMetadata*)mtd);
    set_cmd_memento(self, NULL);
}

static void crop_destructor(Command **self)
{
    // Memento will store the metadata of the previous ppm image
    PpmMetadata *mtd = (PpmMetadata*)get_cmd_memento(*self);
    free_ppm_metadata(&mtd);
    set_cmd_memento(*self, NULL);
}

static const CommandVTable CropVTable = {
    .execute = execute_crop,
    .undo = undo_crop,
    .destroy_cmd = crop_destructor
};

Command *create_crop_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 4) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: CROP <x1> <y1> <x2> <y2>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile **ppm = get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(1, ppm, args, &CropVTable);
}