#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        return EXECUTE_COMMAND_FAILED;
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
    
    cmd->undoable = UNDOABLE;
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
    char *old_lsys_file_path = NULL;

    // Memento will store the old file path of the lsys file instead of storing the file
    if (app_data->lsys_file) {
        char *old_lsys_file_path = malloc(strlen(app_data->lsys_file->file_path) + 1);
        if (!old_lsys_file_path) {
            printf("Failed to load %s\n", path);
            return EXECUTE_COMMAND_FAILED;
        }
        strcpy(old_lsys_file_path, app_data->lsys_file->file_path);
        self->memento = old_lsys_file_path;
        close_lsystem_file(app_data->lsys_file);
    }
    
    // Opening the new .lsys file
    Lsystem *new_lsys = open_lsystem_file(path);
    if (!new_lsys) {
        printf("Failed to load %s\n", path);
        free(old_lsys_file_path);
        return EXECUTE_COMMAND_FAILED;
    }
    printf("Loaded %s (L-system with %d rules)\n", path, new_lsys->nrules);
    app_data->lsys_file = new_lsys;
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_lsystem(Command *self)
{
    SystemData *app_data = self->receiver;
    Lsystem *old_file = open_lsystem_file((const char *)self->memento);
    close_lsystem_file(app_data->lsys_file);
    app_data->lsys_file = old_file;
}

void lsystem_destructor(Command *self)
{
    // Memento member could have memory allocated for a path of an old .lsys file
    free(self->memento);
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
    
    cmd->undoable = UNDOABLE;
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
        return EXECUTE_COMMAND_FAILED;
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

    cmd->undoable = !UNDOABLE;
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