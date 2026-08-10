#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/commands_graphics.h"
#include "../header_files/turtle_graphics.h"
#include "../header_files/lsys.h"
#include "../header_files/cli_engine.h"

uint8_t execute_turtle(Command *self)
{
    SystemData *app_data = self->receiver;
    int8_t **argv = self->cmd_args->argv;
    
    // Unpacking turtle arguments
    double x = atof(argv[1]);
    double y = atof(argv[2]);
    uint32_t offset_step = atol(argv[3]);
    uint16_t orientation = atoi(argv[4]);
    uint8_t angular_step = atoi(argv[5]);

    Turtle *turtle = create_turtle(x, y, orientation, offset_step, angular_step);
    if (!turtle) {
        return EXECUTE_COMMAND_FAILED;
    }

    // Computing the Nth derivative of the Lsystem
    uint32_t n = atoi(argv[6]);
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
    color.red_channel = atoi(argv[7]);
    color.green_channel = atoi(argv[8]);
    color.blue_channel = atoi(argv[9]);

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
    if (!sys || !cmd_args || cmd_args->argc != 10) {
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

Command *create_font_command(CliEngine *sys, CliArgs *cmd_args)
{
    return NULL;
}

Command *create_type_command(CliEngine *sys, CliArgs *cmd_args)
{
    return NULL;
}