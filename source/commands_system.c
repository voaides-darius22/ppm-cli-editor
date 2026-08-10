#include <stdio.h>
#include <stdlib.h>

#include "../header_files/commands_system.h"
#include "../header_files/cli_engine.h"
#include "../header_files/ppm.h"

uint8_t execute_undo(Command *self)
{
    Invoker *invoker = self->receiver;
    Command *cmd = pop_slist(invoker->undo_stack);
    // Checking if an undoable command has been executed
    if (!cmd) {
        return EXECUTE_COMMAND_FAILED;
    }
    cmd->undo(cmd);
    push_slist(invoker->redo_stack, cmd);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void undo_destructor(Command *self)
{
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_undo_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 1) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }
    cmd->undoable = !UNDOABLE;
    cmd->execute = execute_undo;
    cmd->destructor = undo_destructor;
    cmd->receiver = sys->cmd_invoker;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_redo(Command *self)
{
    Invoker *invoker = self->receiver;
    Command *cmd = pop_slist(invoker->redo_stack);
    // Checking if redo stack contains undoable commands
    if (!cmd) {
        return EXECUTE_COMMAND_FAILED;
    }
    cmd->execute(cmd);
    push_slist(invoker->undo_stack, cmd);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void redo_destructor(Command *self)
{
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_redo_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 1) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }
    cmd->undoable = !UNDOABLE;
    cmd->execute = execute_redo;
    cmd->destructor = redo_destructor;
    cmd->receiver = sys->cmd_invoker;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_save(Command *self)
{
    SystemData *app_data = self->receiver;
    Ppm *img = app_data->ppm_file;
    if (img) {
        const char *path = self->cmd_args->argv[1];
        write_ppm_file(path, img);
        printf("Saved %s\n", path);
        return EXECUTE_COMMAND_SUCCEEDED;
    } else {
        printf("No image loaded\n");
    }
    return EXECUTE_COMMAND_FAILED;
}

void save_destructor(Command *self)
{
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_save_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 2) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    cmd->undoable = !UNDOABLE;
    cmd->execute = execute_save;
    cmd->destructor = save_destructor;
    cmd->receiver = sys->app_data;
    cmd->cmd_args = cmd_args;
    return cmd;
}

uint8_t execute_exit(Command *self)
{
    CliEngine *sys = self->receiver;
    free_cli_engine(sys);
    return EXECUTE_COMMAND_SUCCEEDED;
}

void exit_destructor(Command *self)
{
    free_cli_args(self->cmd_args);
    free(self);
}

Command *create_exit_command(CliEngine *sys, CliArgs *cmd_args)
{
    if (!sys || !cmd_args || cmd_args->argc != 1) {
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(*cmd));
    if (!cmd) {
        return NULL;
    }

    cmd->undoable = !UNDOABLE;
    cmd->execute = execute_exit;
    cmd->destructor = exit_destructor;
    cmd->receiver = sys;
    cmd->cmd_args = cmd_args;
    return cmd;
}
