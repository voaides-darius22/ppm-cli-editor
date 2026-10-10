#include <stdio.h>
#include <stdlib.h>
#include "../../header_files/command/command_protected.h"
#include "../../header_files/command/commands_system.h"
#include "../../header_files/io/ppm/ppm.h"
#include "../../header_files/engine/cli_engine.h"
#include "../../header_files/engine/invoker.h"
#include "../../header_files/engine/system_data.h"

static int8_t execute_undo(Command *self)
{
    Invoker *invoker = get_cmd_receiver(self);
    Command *cmd = pop_invoker_stack(invoker, UNDO_STACK);
    // Checking if an undoable command has been executed
    if (!cmd) {
        printf("Nothing to undo\n");
        return 0;
    }
    undo_cmd(cmd);
    push_invoker_stack(invoker, REDO_STACK, cmd);
    return 1;
}

static const CommandVTable UndoVTable = {
    .execute = execute_undo,
    .undo = NULL,
    .destroy_cmd = NULL
};

Command *create_undo_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 0) {
        printf("Error: UNDO accepts no arguments (%d given)\n", args->argc);
        return NULL;
    }
    return cmd_ctor(0, (Invoker*)access_invoker(sys), args, &UndoVTable);
}

static int8_t execute_redo(Command *self)
{
    Invoker *invoker = get_cmd_receiver(self);
    Command *cmd = pop_invoker_stack(invoker, REDO_STACK);
    // Checking if the redo stack contains undoable commands
    if (!cmd) {
        printf("Nothing to redo\n");
        return 0;
    }
    execute_cmd(cmd);
    push_invoker_stack(invoker, UNDO_STACK, cmd);
    return 1;
}

static const CommandVTable RedoVTable = {
    .execute = execute_redo,
    .undo = NULL,
    .destroy_cmd = NULL
};

Command *create_redo_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 0) {
        printf("Error: REDO accepts no arguments (%d given)\n", args->argc);
        return NULL;
    }
    return cmd_ctor(0, (Invoker*)access_invoker(sys), args, &RedoVTable);
}

static int8_t execute_save(Command *self)
{
    Ppm *img = get_cmd_receiver(self);

    // Unpacking save argument (file-path)
    const CliArgs *args = get_cmd_args(self);
    const char *path = args->argv[0];
    
    // Exporting the .ppm img
    if (!export_ppm(path, img)) { return 0; }
    printf("Saved %s\n", path);
    return 1; 
}

static const CommandVTable SaveVTable = {
    .execute = execute_save,
    .undo = NULL,
    .destroy_cmd = NULL
};

Command *create_save_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: SAVE <file_path>\n");
        return NULL;
    }
    
    const Appdata *appdata = access_appdata(sys);
    AppFile *ppm = *get_file_addr(appdata, PPM);
    if (!ppm) {
        printf("No image loaded\n");
        return NULL;
    }
    return cmd_ctor(0, ppm, args, &SaveVTable);
}

int8_t execute_exit(Command *self)
{
    CliEngine *sys = get_cmd_receiver(self);
    cli_engine_dtor(&sys);
    return 1;
}

static const CommandVTable ExitVTable = {
    .execute = execute_exit,
    .undo = NULL,
    .destroy_cmd = NULL
};

Command *create_exit_command(CliEngine *sys, CliArgs *args)
{
    if (args->argc != 0) {
        printf("Error: EXIT accepts no arguments (%d given)\n", args->argc);
        return NULL;
    }
    return cmd_ctor(0, sys, args, &ExitVTable);
}
