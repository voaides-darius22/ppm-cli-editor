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
    if (!req) { return NULL; }

    req->file_1 = file_1;
    req->file_2 = file_2;
    return req;
}

static int8_t execute_turtle(Command *self)
{
    TurtleRequiredFiles *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    Lsystem *file_2 = req->file_2;

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(file_1);
    if (!previous_pixel_raster) { return 0; }
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
    if (!derivative) { return 0; }

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
    TurtleRequiredFiles *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    RgbPixel *previous_pixel_raster = get_cmd_memento(self); 
    const CliArgs *args = get_cmd_args(self);
    overwrite_pixel_raster(file_1, previous_pixel_raster);
    printf(
        "TURTLE %s %s %s %s %s %s %s %s %s has been canceled\n", 
        args->argv[0], args->argv[1], args->argv[2], args->argv[3], args->argv[4], 
        args->argv[5], args->argv[6], args->argv[7], args->argv[8]
    );
    free(previous_pixel_raster);
    set_cmd_memento(self, NULL);
}

static void turtle_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free(get_cmd_memento(*self));
    // Receiver member will store TurtleRequiredFiles
    free(get_cmd_receiver(*self));
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
    if (!ppm) { printf("No image loaded\n"); }
    if (!lsys) { printf("No L-system loaded\n"); }

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
    if (!req) { return NULL; }

    req->file_1 = file_1;
    req->file_2 = file_2;
    return req;
}

static int8_t execute_type(Command *self)
{
    TypeRequiredFile *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    Bdf *file_2 = req->file_2;

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(file_1);
    if (!previous_pixel_raster) { return 0; }
    set_cmd_memento(self, previous_pixel_raster);

    // Getting ppm members
    uint32_t width = get_img_width(file_1);
    uint32_t height = get_img_height(file_1);
    RgbPixel *pixel_raster = get_img_original_raster(file_1);
    
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
        int32_t bottom_left_corner_offset = bottom_left_corner_y * width + bottom_left_corner_x;
        RgbPixel *bottom_left_pixel = &pixel_raster[bottom_left_corner_offset];

        // Getting bdf members
        int32_t BBh = get_glyph_BBh(glyph);
        int8_t *bitmap = get_glyph_bitmap(glyph);
        int32_t BBw = get_glyph_BBw(glyph);

        int32_t glyph_padding = compute_glyph_padding(glyph);
        int32_t bitmap_row_length = (glyph_padding + BBw) / CHAR_BIT;
        
        // Drawing glyph (drawing direction is bottom-up & left-right)
        for (int glyph_row = BBh - 1, i = 0; glyph_row >= 0; glyph_row--, i++) {
            RgbPixel *px = bottom_left_pixel - i * width; 
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
    TypeRequiredFile *req = get_cmd_receiver(self);
    Ppm *file_1 = req->file_1;
    RgbPixel *previous_pixel_raster = get_cmd_memento(self);
    overwrite_pixel_raster(file_1, previous_pixel_raster);
    free(previous_pixel_raster);
    set_cmd_memento(self, NULL);
    printf("Text has been removed\n");
}

static void type_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free(get_cmd_memento(*self));
    // Receiver member will store TypeRequiredFiles
    free(get_cmd_receiver(*self));
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

    if (!ppm) { printf("No image loaded\n"); }
    if (!bdf) { printf("No font loaded\n"); }

    TypeRequiredFile *req = (ppm && bdf) ? create_type_required_files((Ppm*)ppm, (Bdf*)bdf): NULL;
    return (req) ? cmd_ctor(1, req, args, &TypeVTable): NULL;
}

static int8_t execute_grayscale(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    int32_t pixels = get_img_width(img) * get_img_height(img);

    // Memory allocation for the pixel raster of the previous img (Memento)
    RgbPixel *previous_pixel_raster = clone_pixel_raster(img);
    if (!previous_pixel_raster) { return 0; }
    set_cmd_memento(self, previous_pixel_raster);

    // BT.601 Standard (Broadcasting Service Television)
    // Luminance Coefficients (Multipliers)
    const float red_mul = 0.299;
    const float green_mul = 0.587;
    const float blue_mul = 0.114;

    RgbPixel *pixel_raster = get_img_original_raster(img);    
    for (int i = 0; i < pixels; i++) {
        RgbPixel *current_pixel = &pixel_raster[i];
        // Computing gray luminance
        uint8_t Y = round(
            red_mul * current_pixel->red_channel + 
            green_mul * current_pixel->green_channel + 
            blue_mul *current_pixel->blue_channel
        );
        RgbPixel new_pixel = {Y, Y, Y};
        *current_pixel = new_pixel; 
    }

    printf("Grayscale filter has been applied\n");
    return 1;
}

void undo_grayscale(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    RgbPixel *previous_pixel_raster = get_cmd_memento(self);
    overwrite_pixel_raster(img, previous_pixel_raster);
    free(previous_pixel_raster);
    set_cmd_memento(self, NULL);
    printf("Grayscale filter has been removed\n");
}

static void grayscale_destructor(Command **self)
{
    // Memento member will store the pixel raster of the previous image
    free(get_cmd_memento(*self));
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
    AppFile *ppm = *get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(1, ppm, args, &GrayscaleVTable);
}

static int8_t execute_brightness(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    const CliArgs *args = get_cmd_args(self);
    
    double new_brightness_level = atof(args->argv[0]);
    if (new_brightness_level < 0 || new_brightness_level > 100) {
        printf("Brightness level must be between [0, 100]\n");
        return 0;
    }

    double *current_brightness_level = malloc(sizeof(double));
    if (!current_brightness_level) { return 0; }
    *current_brightness_level = get_img_brightness_level(img);
    set_cmd_memento(self, current_brightness_level);

    set_img_brightness_level(img, new_brightness_level);
    printf("Image brightness level has been adjusted to %g%%\n", new_brightness_level);
    return 1;
}

static void undo_brightness(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    double *previous_brightness_level = get_cmd_memento(self);
    set_img_brightness_level(img, *previous_brightness_level);
    printf("Image brightness level has be restored to %g%%\n", *previous_brightness_level);
    free(previous_brightness_level);
    set_cmd_memento(self, NULL);
}

static void brightness_destructor(Command **self)
{
    // Memento member will store the previous brightness level of the img
    free(get_cmd_memento(*self));
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
    AppFile *ppm = *get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(1, ppm, args, &BrightnessVTable);
}

typedef struct CropPackage {
    uint32_t width, height;
    RgbPixel *original_pixel_raster;
} CropPackage;

static CropPackage *create_crop_package(const Ppm *img)
{
    CropPackage *pack = (img) ? malloc(sizeof(CropPackage)) : NULL;
    if (!pack) { return NULL; }

    pack->width = get_img_width(img);
    pack->height = get_img_height(img);
    pack->original_pixel_raster = clone_pixel_raster(img);
    if (!pack->original_pixel_raster) {
        free(pack);
        return NULL;
    }

    return pack;
}

static void free_crop_package(CropPackage **addr_pack)
{
    CropPackage *pack = (addr_pack) ? *addr_pack : NULL;
    if (!pack) { return; }

    free(pack->original_pixel_raster);
    free(pack);
    *addr_pack = NULL;
}

static int8_t execute_crop(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    CropPackage *pack = create_crop_package(img);
    if (!pack) { return 0; }
    set_cmd_memento(self, pack);
    
    // Unpacking Crop arguments (top-left & bottom-right corners)
    const CliArgs *args = get_cmd_args(self);
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
    if (x1 < 0 || y1 < 0 ||
        x1 + cropped_width > pack->width || y1 + cropped_height > pack->height) {
        printf("Corners overflow the image dimension\n");
        return 0;
    }
    
    // Memory allocation for the cropped pixel raster
    int32_t crop_pixels = cropped_width * cropped_height;
    RgbPixel *cropped_pixel_raster = malloc(crop_pixels * sizeof(RgbPixel));
    if (!cropped_pixel_raster) { return 0; }
    for (int i = 0; i < cropped_height; i++) {
        int32_t offset = (y1 + i) * pack->width + x1;
        memcpy(
            cropped_pixel_raster + i * cropped_width,
            pack->original_pixel_raster + offset,
            cropped_width * sizeof(RgbPixel)
        );
    }
    set_img_raster(img, cropped_width, cropped_height, cropped_pixel_raster);

    printf("Image has been cropped to (%dx%d)\n", cropped_width, cropped_height);
    return 1;
}

static void undo_crop(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    CropPackage *pack = get_cmd_memento(self);
    set_img_raster(img, pack->width, pack->height, pack->original_pixel_raster);
    printf("Image has been restored to (%dx%d)\n", pack->width, pack->height);
    free(pack);
    set_cmd_memento(self, NULL);    
}

static void crop_destructor(Command **self)
{
    // Memento will store the CropPackage of the img
    CropPackage *pack = get_cmd_memento(*self);
    free_crop_package(&pack);
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
    AppFile *ppm = *get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }

    return cmd_ctor(1, ppm, args, &CropVTable);
}

static int8_t execute_rotate(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    
    // Unpacking rotate argument
    const CliArgs *args = get_cmd_args(self);
    double new_angle = atof(args->argv[0]);
    
    double *current_angle = malloc(sizeof(double));
    if (!current_angle) { return 0; }
    *current_angle = get_img_angle(img);
    set_cmd_memento(self, current_angle);

    add_img_angle(img, new_angle);
    printf("Image angle has been modified with %g°\n", new_angle);
    return 1;
}

static void undo_rotate(Command *self)
{
    Ppm *img = get_cmd_receiver(self);
    double *previous_angle = get_cmd_memento(self);
    set_img_angle(img, *previous_angle);
    printf("Image angle has been restored to %g°\n", *previous_angle);
    free(previous_angle);
    set_cmd_memento(self, NULL);
}

static void rotate_destructor(Command **self)
{
    // Memento member will store the previous angle of the img
    free(get_cmd_memento(*self));
    set_cmd_memento(*self, NULL);
}

static const CommandVTable RotateVTable = {
    .execute = execute_rotate,
    .undo = undo_rotate,
    .destroy_cmd = rotate_destructor
};

Command *create_rotate_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: ROTATE <angle>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    AppFile *ppm = *get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }

    return cmd_ctor(1, ppm, args, &RotateVTable);
}