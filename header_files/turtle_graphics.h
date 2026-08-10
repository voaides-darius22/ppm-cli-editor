#pragma once

#include "../header_files/ppm.h"
#include "../header_files/singly_linked_list.h"

typedef struct TurtlePosition {
    double x;
    double y;
} TurtlePosition;

TurtlePosition create_turtle_position(double x, double y);

typedef struct TurtleState {
    TurtlePosition pos;
    uint16_t orientation;
} TurtleState;

typedef struct TurtleSettings {
    uint32_t offset_step;
    uint8_t angular_step;
} TurtleSettings;

TurtleSettings create_turtle_settings(uint32_t offset_step, uint8_t angular_step);

typedef struct Turtle {
    TurtleState current_state;
    TurtleSettings settings;
} Turtle;

Turtle *create_turtle(
    double x, double y, uint16_t orientation, 
    uint32_t offset_step, uint8_t angular_step
);

typedef struct GraphicSystem {
    Ppm *img;
    Turtle *turtle;
    SList *states;
} GraphicSystem;

GraphicSystem *create_graphic_system(Ppm *img, Turtle *turtle);
GraphicSystem *close_graphic_system(GraphicSystem *graphic_system);
// Turtle Instructions
void add_state(GraphicSystem *graphic_system);
TurtleState *get_state(GraphicSystem *graphic_system);
void remove_state(GraphicSystem *graphic_system);
void increase_orientation(GraphicSystem *graphic_system);
void decrease_orientation(GraphicSystem *graphic_system);
void draw_line(Ppm *img, TurtlePosition pos_1, TurtlePosition pos_2, RgbPixel color);
// turtle_move function will change turtle's position and will return its old position
TurtlePosition turtle_move(GraphicSystem *graphic_system);
