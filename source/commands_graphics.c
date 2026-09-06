#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../header_files/commands_graphics.h"
#include "../header_files/turtle_graphics.h"
#include "../header_files/lsys.h"
#include "../header_files/cli_engine.h"

uint8_t execute_turtle(Command *self)
{
    SystemData *app_data = self->receiver;
    char **argv = self->cmd_args->argv;
    
    // Unpacking turtle arguments
    double x = atof(argv[0]);
    double y = atof(argv[1]);
    uint32_t offset_step = atol(argv[2]);
    uint16_t orientation = atoi(argv[3]);
    uint8_t angular_step = atoi(argv[4]);

    Turtle *turtle = create_turtle(x, y, orientation, offset_step, angular_step);
    if (!turtle) {
        return EXECUTE_COMMAND_FAILED;
    }

    // Computing the Nth derivative of the Lsystem
    uint32_t n = atoi(argv[5]);
    char *derivative = derive_lsys(app_data->lsys_file, n);
    if (!derivative) {
        free(turtle);
        return EXECUTE_COMMAND_FAILED;
    }

    // Memory allocation for graphic system
    GraphicSystem *graphic_system = create_graphic_system(app_data->ppm_file, turtle);
    if (!graphic_system) {
        free(turtle);
        free(derivative);
        return EXECUTE_COMMAND_FAILED;
    }

    RgbPixel color;
    memset(&color, 0, sizeof(color));
    color.red_channel = atoi(argv[6]);
    color.green_channel = atoi(argv[7]);
    color.blue_channel = atoi(argv[8]);

    // Turtle Parser
    for (int i = 0; derivative[i] != '\0'; i++) {
        int8_t symbol = derivative[i];
        switch (symbol) {
            case 'F': {
                TurtlePosition old_pos = turtle_move(graphic_system);
                TurtlePosition new_pos = turtle->current_state.pos;
                draw_line(graphic_system->img, old_pos, new_pos, color);
                break;
            }
            case '+': {
                increase_orientation(graphic_system);
                break;
            }
            case '-': {
                decrease_orientation(graphic_system);
                break;
            }
            case '[': {
                add_state(graphic_system);
                break;
            }
            case ']': {
                TurtleState *state = get_state(graphic_system);
                if (state) {
                    turtle->current_state = *state;
                }
                remove_state(graphic_system);
                break;
            }
            default: {
                break;
            }
        }
    }
    close_graphic_system(graphic_system);
    free(derivative);
    printf("Drawing done\n");
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_turtle(Command *self)
{
    SystemData *app_data = self->receiver;
    Ppm *img = app_data->ppm_file;
    RgbPixel *previous_pixel_buffer = self->memento;
    uint32_t pixels = img->width * img->height;
    memcpy(img->pixel_buffer, previous_pixel_buffer, pixels * sizeof(*img->pixel_buffer));
}

void turtle_destructor(Command *self)
{
    // Memento member will store the previous version of the img's pixel buffer
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_turtle_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 9) {
        return NULL;
    }

    SystemData *app_data = sys->app_data;
    // Checking if the system contains a .ppm, .lsys files opened
    if (!app_data->ppm_file) {
        printf("No image loaded\n");
        return EXECUTE_COMMAND_FAILED;
    }
    if (!app_data->lsys_file) {
        printf("No L-system loaded\n");
        return EXECUTE_COMMAND_FAILED;
    }


    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    Ppm *img = app_data->ppm_file;
    uint32_t pixels = img->width * img->height;
    cmd->memento = malloc(pixels * sizeof(*img->pixel_buffer));
    if (!cmd->memento) {
        free(cmd);
        return NULL;
    }
    memcpy(cmd->memento, img->pixel_buffer, pixels * sizeof(*img->pixel_buffer));

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_turtle;
    cmd->undo = undo_turtle;
    cmd->destructor = turtle_destructor;
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_font(Command *self)
{
    SystemData *app_data = self->receiver;
    const char *path = self->cmd_args->argv[0];

    // Opening the new .bdf file
    Bdf *new_font = open_bdf_font(path);
    if (!new_font) {
        printf("Failed to load %s\n", path);
        return EXECUTE_COMMAND_FAILED;
    }

    char *old_bdf_file_path = NULL;
    // Memento will store the old file path of the bdf file instead of storing the file
    if (app_data->font_file) {
        old_bdf_file_path = malloc(strlen(app_data->font_file->file_path) + 1);
        if (!old_bdf_file_path) {
            close_bdf_font(new_font);
            printf("Failed to load %s\n", path);
            return EXECUTE_COMMAND_FAILED;
        }
        strcpy(old_bdf_file_path, app_data->font_file->file_path);
        self->memento = old_bdf_file_path;
        close_bdf_font(app_data->font_file);
    }
    
    app_data->font_file = new_font;
    printf("Loaded %s (bitmap font %s)\n", path, new_font->name);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_font(Command *self)
{
    SystemData *app_data = self->receiver;
    Bdf *old_file = open_bdf_font((const char *)self->memento);
    close_bdf_font(app_data->font_file);
    app_data->font_file = old_file;
}

void font_destructor(Command *self)
{
    // Memento member could have memory allocated for a path of an old .bdf file
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_font_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 1) {
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
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_type(Command *self)
{
    SystemData *app_data = self->receiver;
    Bdf *font = app_data->font_file;
    Ppm *img = app_data->ppm_file;
    
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
    // Checking if the .bdf font contains all chars needed
    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        if (!get(font->glyphs, &ch, cmp_hash_key_bdf_glyph)) {
            printf("No glyph found for %c\n", (char)ch);
            return EXECUTE_COMMAND_FAILED;
        }
    }

    for (int i = 0; i < str_length; i++) {
        int16_t ch = string[i];
        BdfGlyph *glyph = get(font->glyphs, &ch, cmp_hash_key_bdf_glyph);
        if (!does_bdf_glyph_fit(img, cursor_x, cursor_y, glyph)) {
            printf("Glyph %s doesn't fit in the img\n", glyph->name);
            break;
        }

        // Bottom left px of the glyph relative to img
        int32_t bottom_left_corner_x = cursor_x + glyph->bbx.BBxoff;
        int32_t bottom_left_corner_y = cursor_y + glyph->bbx.BByoff;
        int32_t bottom_left_corner_offset = bottom_left_corner_y * img->width + bottom_left_corner_x;
        RgbPixel *bottom_left_pixel = &img->pixel_buffer[bottom_left_corner_offset];

        int32_t glyph_padding = compute_glyph_padding(glyph->bbx.BBw);
        int32_t bitmap_row_length = (glyph_padding + glyph->bbx.BBw) / CHAR_BIT;
        
        // Drawing glyph (drawing direction is bottom-up & left-right)
        for (int glyph_row = glyph->bbx.BBh - 1, i = 0; glyph_row >= 0; glyph_row--, i++) {
            RgbPixel *px = bottom_left_pixel - i * img->width; 
            // Iterating through glyph's bitmap rows
            // Compute offset for row
            int8_t *bitmap_row = glyph->bitmap + glyph_row * bitmap_row_length;
            int32_t scanned_bits = 0;
            // Iterating through glyph's bitmap collumns (bytes)
            for (int collumn = 0; collumn < bitmap_row_length; collumn++) {
                int8_t bitmap_byte = bitmap_row[collumn];
                for (int idx = CHAR_BIT - 1; idx >= 0 && scanned_bits < glyph->bbx.BBw; idx--) {
                    scanned_bits++;
                    int8_t bit = (bitmap_byte >> idx) & 1;
                    // Checking if the bit is set => pixel must be colored
                    if (bit) {
                        *px = color;
                    }
                    px++;
                }
            }
        }
        // Moving cursor position after writing a glyph
        cursor_x += glyph->dwidth.dwx;
        cursor_y += glyph->dwidth.dwy;
    }

    printf("Text written\n");
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_type(Command *self)
{
    SystemData *app_data = self->receiver;
    Ppm *img = app_data->ppm_file;
    RgbPixel *previous_pixel_buffer = self->memento;
    uint32_t pixels = img->width * img->height;
    memcpy(img->pixel_buffer, previous_pixel_buffer, pixels * sizeof(*img->pixel_buffer));
}

void type_destructor(Command *self)
{
    free(self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_type_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 6) {
        return NULL;
    }

    SystemData *app_data = sys->app_data;
    Bdf *font = app_data->font_file;
    Ppm *img = app_data->ppm_file;

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

    uint32_t pixels = img->width * img->height;
    cmd->memento = malloc(pixels * sizeof(*img->pixel_buffer));
    if (!cmd->memento) {
        free(cmd);
        return NULL;
    }
    memcpy(cmd->memento, img->pixel_buffer, pixels * sizeof(*img->pixel_buffer));

    cmd->undoable = UNDOABLE;
    cmd->execute = execute_type;
    cmd->undo = undo_type;
    cmd->destructor = type_destructor;
    cmd->receiver = app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}