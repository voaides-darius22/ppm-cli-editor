#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "../header_files/commands_graphics.h"
#include "../header_files/turtle_graphics.h"
#include "../header_files/lsys.h"
#include "../header_files/cli_engine.h"

uint8_t execute_turtle(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    // Memento will store the previous pixel raster of the image
    self->memento = clone_pixel_raster(img);
    if (!self->memento) {
        return EXECUTE_COMMAND_FAILED;
    }

    char **argv = self->cmd_args->argv;
    // Unpacking turtle arguments
    double x = atof(argv[0]);
    double y = atof(argv[1]);
    uint32_t offset_step = atol(argv[2]);
    uint16_t orientation = atoi(argv[3]);
    uint8_t angular_step = atoi(argv[4]);

    // Computing the Nth derivative of the Lsystem
    uint32_t n = atoi(argv[5]);
    char *derivative = derive_lsys(*get_addr_of_lsystem_file(appdata), n);
    if (!derivative) {
        return EXECUTE_COMMAND_FAILED;
    }

    // Memory allocation for graphic system
    GraphicSystem *graphic_system = create_graphic_system(
        img, x, y, offset_step, orientation, angular_step
    );
    if (!graphic_system) {
        free(derivative);
        return EXECUTE_COMMAND_FAILED;
    }

    RgbPixel color;
    memset(&color, 0, sizeof(color));
    color.red_channel = atoi(argv[6]);
    color.green_channel = atoi(argv[7]);
    color.blue_channel = atoi(argv[8]);

    // Turtle Parser
    turtle_parser(derivative, graphic_system, color);
    close_graphic_system(graphic_system);
    free(derivative);
    printf("Drawing done\n");
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_turtle(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    overwrite_pixel_raster(img, (RgbPixel*)self->memento);
    free(self->memento);
    self->memento = NULL;
}

void turtle_destructor(Command *self)
{
    // Memento member could store the previous version of the img's pixel buffer
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_turtle_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 9) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: TURTLE <x> <y> <offset_step> <orientation> <angular_step> <n> <R> <G> <B>\n");
        return NULL;
    }

    SystemData *appdata = access_appdata(sys);
    // Checking if the system contains a .ppm, .lsys files opened
    if (!*get_addr_of_ppm_file(appdata)) {
        printf("No image loaded\n");
        return EXECUTE_COMMAND_FAILED;
    }
    if (!*get_addr_of_lsystem_file(appdata)) {
        printf("No L-system loaded\n");
        return EXECUTE_COMMAND_FAILED;
    }


    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_turtle;
    cmd->undo = undo_turtle;
    cmd->destructor = turtle_destructor;
    cmd->receiver = appdata;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_font(Command *self)
{
    Bdf **addr_font = self->receiver;
    Bdf *old_font = *addr_font;
    const char *path = self->cmd_args->argv[0];

    // Opening the new .bdf font
    Bdf *new_font = open_bdf_font(path);
    if (!new_font) {
        printf("Failed to load %s\n", path);
        return EXECUTE_COMMAND_FAILED;
    }

    // Memento will store the old file path of the bdf font
    if (old_font) {
        char *old_file_path = malloc(strlen(get_bdf_font_path(old_font)) + 1);
        if (!old_file_path) {
            close_bdf_font(new_font);
            printf("Failed to load %s\n", path);
            return EXECUTE_COMMAND_FAILED;
        }
        strcpy(old_file_path, get_bdf_font_path(old_font));
        self->memento = old_file_path;
        close_bdf_font(old_font);
    }
    
    *addr_font = new_font;
    printf("Loaded %s (bitmap font %s)\n", path, get_bdf_font_name(new_font));
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_font(Command *self)
{
    Bdf **addr_font = self->receiver;
    Bdf *old_font = open_bdf_font((const char *)self->memento);
    close_bdf_font(*addr_font);
    *addr_font = old_font;
    free(self->memento);
    self->memento = NULL;
}

void font_destructor(Command *self)
{
    // Memento member could have memory allocated for a path of an old .bdf font
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_font_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: FONT <file_path>\n");
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_font;
    cmd->undo = undo_font;
    cmd->destructor = font_destructor;
    cmd->receiver = get_addr_of_bdf_file(access_appdata(sys));
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_type(Command *self)
{
    SystemData *appdata = self->receiver;
    Bdf *font = *get_addr_of_bdf_file(appdata);
    Ppm *img = *get_addr_of_ppm_file(appdata);

    uint32_t img_width = get_ppm_width(img);
    uint32_t img_height = get_ppm_height(img);
    RgbPixel *pixel_raster = get_ppm_px_raster(img);
    
    // Unpacking type arguments
    char *string = strtok(self->cmd_args->argv[0], "\"");
    int32_t cursor_x = atoi(self->cmd_args->argv[1]);
    int32_t cursor_y = atoi(self->cmd_args->argv[2]);
    RgbPixel color;
    memset(&color, 0, sizeof(color));
    color.red_channel = atoi(self->cmd_args->argv[3]);
    color.green_channel = atoi(self->cmd_args->argv[4]);
    color.blue_channel = atoi(self->cmd_args->argv[5]);

    int32_t str_length = strlen(string);
    HashTable *glyphs = get_bdf_font_glyphs_table(font);
    // Checking if the .bdf font contains all chars needed
    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        if (!get(glyphs, &ch, cmp_hash_key_bdf_glyph)) {
            printf("No glyph found for %c\n", (char)ch);
            return EXECUTE_COMMAND_FAILED;
        }
    }

    // Memento will store the previous pixel raster of the image
    self->memento = clone_pixel_raster(img);
    if (!self->memento) {
        return EXECUTE_COMMAND_FAILED;
    }

    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        BdfGlyph *glyph = get(glyphs, &ch, cmp_hash_key_bdf_glyph);
        if (!does_bdf_glyph_fit(img, cursor_x, cursor_y, glyph)) {
            printf("Glyph %s doesn't fit in the img\n", get_glyph_name(glyph));
            break;
        }

        // Bottom left px of the glyph relative to img
        int32_t bottom_left_corner_x = cursor_x + get_glyph_BBxoff(glyph);
        int32_t bottom_left_corner_y = cursor_y + get_glyph_BByoff(glyph);
        int32_t bottom_left_corner_offset = bottom_left_corner_y * img_width + bottom_left_corner_x;
        RgbPixel *bottom_left_pixel = &pixel_raster[bottom_left_corner_offset];

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
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_type(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    overwrite_pixel_raster(img, (RgbPixel*)self->memento);
    free(self->memento);
    self->memento = NULL;
}

void type_destructor(Command *self)
{
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_type_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 6) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: TYPE \"<string>\" <start_x> <start_y> <R> <G> <B>\n");
        return NULL;
    }

    SystemData *appdata = access_appdata(sys);
    Bdf *font = *get_addr_of_bdf_file(appdata);
    Ppm *img = *get_addr_of_ppm_file(appdata);

    // Checking if the system has loaded a .ppm img and a .bdf font
    uint8_t required_files = 1;
    if (!img) {
        printf("No image loaded\n");
        required_files = 0;
    }

    if (!font) {
        printf("No font loaded\n");
        required_files = 0;
    }

    if (!required_files) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_type;
    cmd->undo = undo_type;
    cmd->destructor = type_destructor;
    cmd->receiver = appdata;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_grayscale(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    int32_t pixels = get_ppm_width(img) * get_ppm_height(img);

    // Memento will store the previous pixel raster of the image
    self->memento = clone_pixel_raster(img);
    if (!self->memento) {
        return EXECUTE_COMMAND_FAILED;
    }

    // BT.601 Standard (Broadcasting Service Television)
    // Luminance Coefficients (Multipliers)
    const float red_mul = 0.299;
    const float green_mul = 0.587;
    const float blue_mul = 0.114;

    RgbPixel *pixel_raster = get_ppm_px_raster(img);    
    for (int i = 0; i < pixels; i++) {
        RgbPixel *px = &pixel_raster[i];
        // Computing gray luminance
        uint8_t Y = round(red_mul * px->red_channel + green_mul * px->green_channel + blue_mul *px->blue_channel);
        RgbPixel new_px = {Y, Y, Y};
        *px = new_px; 
    }

    printf("Grayscale filter has been applied\n");
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_grayscale(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    overwrite_pixel_raster(img, (RgbPixel*)self->memento);
    free(self->memento);
    self->memento = NULL;
}

void grayscale_destructor(Command *self)
{
    free(self->memento);
    free(self);
}

Command *create_grayscale_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 0) {
        printf("Error: GRAYSCALE accepts no arguments (%d given)\n", cmd_args->argc);
        return NULL;
    }

    SystemData *appdata = access_appdata(sys);
    Ppm *img = *get_addr_of_ppm_file(appdata);

    // Checking if the system has loaded a .ppm img
    if (!img) {
        printf("No image loaded\n");
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_grayscale;
    cmd->undo = undo_grayscale;
    cmd->destructor = grayscale_destructor;
    cmd->receiver = appdata;
    free_cli_args(cmd_args);
    return cmd;
}

uint8_t execute_brightness(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    int32_t pixels = get_ppm_width(img) * get_ppm_height(img);

    double brightness_level = atof(self->cmd_args->argv[0]);
    if (brightness_level < 0 || brightness_level > 100) {
        printf("Brightness level must be between [0, 100]\n");
        return EXECUTE_COMMAND_FAILED;
    }

    // Memento will store the previous pixel raster of the image
    self->memento = clone_pixel_raster(img);
    if (!self->memento) {
        return EXECUTE_COMMAND_FAILED;
    }

    const double brightness_base = 4.0;
    float gamma = pow(brightness_base, (50 - brightness_level) / 50);
    
    RgbPixel *pixel_raster = get_ppm_px_raster(img);
    for (int i = 0; i < pixels; i++) {
        RgbPixel *px = &pixel_raster[i];
        *px = pixel_gamma_correction(px, gamma);
    }
    
    printf("Image brightness level has been adjusted to %g%%\n", brightness_level);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_brightness(Command *self)
{
    SystemData *appdata = self->receiver;
    Ppm *img = *get_addr_of_ppm_file(appdata);
    overwrite_pixel_raster(img, (RgbPixel*)self->memento);
    free(self->memento);
    self->memento = NULL;
}

void brightness_destructor(Command *self)
{
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_brightness_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: BRIGHTNESS <level>\n");
        return NULL;
    }

    SystemData *appdata = access_appdata(sys);
    Ppm *img = *get_addr_of_ppm_file(appdata);

    // Checking if the system has loaded a .ppm img
    if (!img) {
        printf("No image loaded\n");
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_brightness;
    cmd->undo = undo_brightness;
    cmd->destructor = brightness_destructor;
    cmd->receiver = access_appdata(sys);
    cmd->cmd_args = cmd_args;
    return cmd;
}