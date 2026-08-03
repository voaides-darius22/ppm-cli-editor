#include <stdio.h>
#include <stdlib.h>

#include "../header_files/commands_io.h"
#include "../header_files/ppm.h"
#include "../header_files/lsys.h"
#include "../header_files/cli_engine.h"

uint8_t execute_load(Command *self)
{
    SystemData *app_data = self->receiver;
    const char *path = self->cmd_args->argv[1];
    
    Ppm *new_img = open_ppm_file(path);
    if (!new_img) {
        printf("Failed to load %s\n", path);
        return !EXECUTE_COMMAND_SUCCEEDED;
    }
    printf("Loaded %s (PPM image %dx%d)\n", path, new_img->width, new_img->height);
    self->memento = app_data->ppm_file;
    app_data->ppm_file = new_img;
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_load(Command *self)
{
    SystemData *app_data = self->receiver;
    app_data->ppm_file = close_ppm_file(app_data->ppm_file);
    // Restore to the previous state of the .ppm file
    app_data->ppm_file = self->memento;
}

void load_destructor(Command *self)
{
    Ppm *img = self->memento;
    close_ppm_file(img);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_load_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 2) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }
    
    cmd->undoable = TRUE;
    cmd->execute = execute_load;
    cmd->undo = undo_load;
    cmd->destructor = load_destructor;
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_lsystem(Command *self)
{
    SystemData *app_data = self->receiver;
    const char *path = self->cmd_args->argv[1];
    Lsystem *new_lsys = open_lsystem_file(path);
    if (!path) {
        printf("Failed to load %s\n", path);
        return !EXECUTE_COMMAND_SUCCEEDED;
    }
    printf("Loaded %s (L-system with %d rules)\n", path, new_lsys->nrules);
    self->memento = app_data->lsys_file;
    app_data->lsys_file = new_lsys;
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_lsystem(Command *self)
{
    SystemData *app_data = self->receiver;
    app_data->lsys_file = close_lsystem_file(app_data->lsys_file);
    app_data->lsys_file = self->memento;
}

void lsystem_destructor(Command *self)
{
    Lsystem *lsys = self->memento;
    close_lsystem_file(lsys);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_lsystem_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 2) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }
    
    cmd->undoable = TRUE;
    cmd->execute = execute_lsystem;
    cmd->undo = undo_lsystem;
    cmd->destructor = lsystem_destructor;
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_derive(Command *self)
{
    SystemData *app_data = self->receiver;
    Lsystem *lsys_file = app_data->lsys_file;
    if (!lsys_file) {
        printf("No L-system loaded\n");
        return !EXECUTE_COMMAND_SUCCEEDED;
    }
    uint32_t n = atoi(self->cmd_args->argv[1]);
    char *derivative = derive_lsys(lsys_file, n);
    if (derivative) {
        printf("%s\n", derivative);
    }
    free(derivative);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void derive_destructor(Command *self)
{
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_derive_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 2) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = !TRUE;
    cmd->execute = execute_derive;
    cmd->destructor = derive_destructor;
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

Command *create_bitcheck_command(CliEngine *sys, CliArgs *cmd_args)
{
    return NULL;
}