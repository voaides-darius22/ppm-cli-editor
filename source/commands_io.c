#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header_files/commands_io.h"
#include "../header_files/ppm.h"
#include "../header_files/lsys.h"
#include "../header_files/cli_engine.h"

uint8_t execute_load(Command *self)
{
    Ppm **addr_img = self->receiver;
    const char *path = self->cmd_args->argv[0];
    
    // Opening the new .ppm img
    Ppm *new_img = open_ppm_file(path);
    if (!new_img) {
        printf("Failed to load %s\n", path);
        return EXECUTE_COMMAND_FAILED;
    }

    // Memento member will store the previous img
    self->memento = *addr_img;
    *addr_img = new_img;
    printf("Loaded %s (PPM image %dx%d)\n", path, get_ppm_width(new_img), get_ppm_height(new_img));
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_load(Command *self)
{
    Ppm **addr_img = self->receiver;
    close_ppm_file(*addr_img);
    *addr_img = self->memento;
    if (*addr_img) {
        char *path = get_ppm_path(*addr_img);
        printf("Loaded %s (PPM image %dx%d)\n", path, get_ppm_width(*addr_img), get_ppm_height(*addr_img));
    } else {
        printf("No image loaded\n");
    }
    self->memento = NULL;
}

void load_destructor(Command *self)
{
    self->memento = close_ppm_file((Ppm *)self->memento);
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_load_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: LOAD <file_path>\n");
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
    cmd->receiver = get_addr_of_ppm_file(access_appdata(sys));
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_lsystem(Command *self)
{
    Lsystem **addr_lsys = self->receiver;
    const char *path = self->cmd_args->argv[0];
    // Opening the new .lsys file
    Lsystem *new_lsys = open_lsystem_file(path);
    if (!new_lsys) {
        printf("Failed to load %s\n", path);
        return EXECUTE_COMMAND_FAILED;
    }

    // Memento member will store the path of the previous .lsys file
    if (*addr_lsys) {
        Lsystem *old_file = *addr_lsys;
        char *old_file_path = malloc(strlen(get_lsystem_path(old_file)) + 1);
        if (!old_file_path) {
            close_lsystem_file(new_lsys);
            printf("Failed to load %s\n", path);
            return EXECUTE_COMMAND_FAILED;
        }
        strcpy(old_file_path, get_lsystem_path(old_file));
        self->memento = old_file_path;
        // Closing the previous .lsys file
        close_lsystem_file(old_file);
    }

    *addr_lsys = new_lsys;
    printf("Loaded %s (L-system with %d rules)\n", path, get_lsystem_num_of_rules(new_lsys));
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_lsystem(Command *self)
{
    Lsystem **addr_lsys = self->receiver;
    Lsystem *current_file = *addr_lsys;
    // Opening the previous .lsys file
    *addr_lsys = open_lsystem_file((const char *)self->memento);
    if (*addr_lsys) {
        char *path = get_lsystem_path(*addr_lsys);
        printf("Loaded %s (L-system with %d rules)\n", path, get_lsystem_num_of_rules(*addr_lsys));
    } else {
        printf("No L-system loaded\n");
    }
    close_lsystem_file(current_file);
    free(self->memento);
    self->memento = NULL;
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
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: LSYSTEM <file_path>\n");
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
    cmd->receiver = get_addr_of_lsystem_file(access_appdata(sys));
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_derive(Command *self)
{
    Lsystem **addr_lsys = self->receiver;
    if (!*addr_lsys) {
        printf("No L-system loaded\n");
        return EXECUTE_COMMAND_FAILED;
    }
    uint32_t n = atoi(self->cmd_args->argv[0]);
    char *derivative = derive_lsys(*addr_lsys, n);
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
    if (!sys || !cmd_args) {
        return NULL;
    }

    if (cmd_args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", cmd_args->argc);
        printf("Usage: DERIVE <N>\n");
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = !UNDOABLE;
    cmd->execute = execute_derive;
    cmd->destructor = derive_destructor;
    cmd->receiver = get_addr_of_lsystem_file(access_appdata(sys));
    cmd->cmd_args = cmd_args;
    return cmd;
}

Command *create_bitcheck_command(CliEngine *sys, CliArgs *cmd_args)
{
    return NULL;
}