#pragma once

#include "../header_files/ppm.h"
#include "../header_files/singly_linked_list.h"

typedef struct GraphicSystem GraphicSystem;

// GraphicSystem Constructor
GraphicSystem *create_graphic_system(
    Ppm *img,
    // Turtle Arguments
    double x, double y, uint32_t offset_step, uint16_t orientation , uint8_t angular_step
);

// GraphicSystem Destructor
GraphicSystem *close_graphic_system(GraphicSystem *graphic_system);

// Turtle Parser
void turtle_parser(char *derivative, GraphicSystem *graphic_system, RgbPixel color);