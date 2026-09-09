#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../header_files/turtle_graphics.h"

typedef struct TurtlePosition {
    double x;
    double y;
} TurtlePosition;

typedef struct TurtleState {
    TurtlePosition pos;
    uint16_t orientation;
} TurtleState;

typedef struct TurtleSettings {
    uint32_t offset_step;
    uint8_t angular_step;
} TurtleSettings;

typedef struct Turtle {
    TurtleState current_state;
    TurtleSettings settings;
} Turtle;

typedef struct GraphicSystem {
    Ppm *img;
    Turtle *turtle;
    SList *states;
} GraphicSystem;

TurtlePosition create_turtle_position(double x, double y)
{
    TurtlePosition new_position;
    new_position.x = x;
    new_position.y = y;
    return new_position;
}

TurtleSettings create_turtle_settings(uint32_t offset_step, uint8_t angular_step)
{
    TurtleSettings settings;
    settings.offset_step = offset_step;
    settings.angular_step = angular_step;
    return settings;
}

Turtle *create_turtle(
    double x, double y, uint32_t offset_step, 
    uint16_t orientation, uint8_t angular_step
){
    Turtle *turtle = calloc(1, sizeof(*turtle));
    if (!turtle) {
        return NULL;
    }
    turtle->current_state.pos = create_turtle_position(x, y);
    turtle->current_state.orientation = orientation;
    turtle->settings = create_turtle_settings(offset_step, angular_step);
    return turtle;
}

GraphicSystem *create_graphic_system(
    Ppm *img,
    // Turtle Arguments
    double x, double y, uint32_t offset_step, uint16_t orientation , uint8_t angular_step
){
    if (!img) {
        return NULL;
    }

    // Creating Turtle
    Turtle *turtle = create_turtle(x, y, offset_step, orientation, angular_step);
    if (!turtle) {
        return NULL;
    }

    // Graphic System Configuration
    GraphicSystem *graphic_system = calloc(1, sizeof(GraphicSystem));
    if (!graphic_system) {
        free(turtle);
        return NULL;
    }
    graphic_system->turtle = turtle;
    graphic_system->img = img;

    graphic_system->states = create_slist();
    if (!graphic_system->states) {
        return close_graphic_system(graphic_system);
    }
    return graphic_system;
}

GraphicSystem *close_graphic_system(GraphicSystem *graphic_system)
{
    if (!graphic_system) {
        return NULL;
    }

    if (graphic_system->turtle) {
        free(graphic_system->turtle);
    }
    
    if (graphic_system->states) {
        free_slist(graphic_system->states, free);
    }
    
    free(graphic_system);
    return NULL;
}

// Turtle Instructions
void add_state(GraphicSystem *graphic_system)
{
    TurtleState *new_state = calloc(1, sizeof(*new_state));
    if (!new_state) {
        return;
    }
    *new_state = graphic_system->turtle->current_state;
    push_slist(graphic_system->states, new_state);
}

TurtleState *get_state(GraphicSystem *graphic_system)
{
    return (graphic_system) ? peek(graphic_system->states) : NULL;
}

void remove_state(GraphicSystem *graphic_system)
{
    TurtleState *state = pop_slist(graphic_system->states);
    free(state);
}

void increase_orientation(GraphicSystem *graphic_system)
{
    Turtle *turtle = graphic_system->turtle;
    int16_t new_orientation = turtle->current_state.orientation;
    new_orientation += turtle->settings.angular_step;
    turtle->current_state.orientation = new_orientation % 360;
}

void decrease_orientation(GraphicSystem *graphic_system)
{
    Turtle *turtle = graphic_system->turtle;
    int16_t new_orientation = turtle->current_state.orientation;
    new_orientation -= turtle->settings.angular_step;
    // Reminder: Orientation could become a negative integer => modulo problems
    turtle->current_state.orientation = (new_orientation % 360 + 360) % 360;
}

TurtlePosition turtle_move(GraphicSystem *graphic_system)
{
    TurtleState *current_state = &graphic_system->turtle->current_state;
    TurtleSettings *settings = &graphic_system->turtle->settings;
    TurtlePosition old_pos = current_state->pos;
    const double PI = 3.1415926535;
    // Transforming from degrees in radians
    double angle = (current_state->orientation * PI) / 180;
    // Computing the new coordinates of the turtle
    current_state->pos.y = old_pos.y + settings->offset_step * sin(angle);
    current_state->pos.x = old_pos.x + settings->offset_step * cos(angle);
    return old_pos;
}

void draw_line(Ppm *img, TurtlePosition pos_1, TurtlePosition pos_2, RgbPixel color)
{    
    // Converting coordinates to integers
    int32_t x0 = round(pos_1.x);
    int32_t y0 = round(pos_1.y);
    int32_t x1 = round(pos_2.x);
    int32_t y1 = round(pos_2.y);
    
    int32_t dx = abs(x1 - x0);
    int8_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int8_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    RgbPixel *pixel_raster = get_ppm_px_raster(img);
    int32_t img_width = get_ppm_width(img);
    int32_t img_height = get_ppm_height(img); 

    while (1) {
        // Draw Pixel
        // Computing the offset of the pixel (.ppm img contains a linear buffer)
        if (x0 >= 0 && x0 < img_width && y0 >= 0 && y0 < img_height) {
            uint32_t offset = (img_height - y0 - 1) * img_width+ x0;
            RgbPixel *pixel = &pixel_raster[offset];
            // Changing the pixel color
            *pixel = color;
        }
        
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void turtle_parser(char *derivative, GraphicSystem *graphic_system, RgbPixel color)
{
    Turtle *turtle = graphic_system->turtle;
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
}