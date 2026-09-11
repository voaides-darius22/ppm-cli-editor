#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header_files/command/commands_io.h"
#include "../../header_files/command/command_protected.h"
#include "../../header_files/io/app_file_protected.h"
#include "../../header_files/engine/system_data.h"
#include "../../header_files/io/ppm/ppm.h"
#include "../../header_files/io/lsys/lsys.h"

// If LoadPackage type is BDF/LSYS => data member will contain previous File Path
// Else if LoadPackage type is PPM => data member will contain previous Ppm Img
typedef struct LoadPackage {
    AppFileType type;
    void *data;
} LoadPackage;

static LoadPackage *create_load_package(int8_t type, AppFile *file)
{
    // Checking if the file type is valid
    if (type != LSYSTEM && type != PPM && type != BDF) {
        return NULL;
    }

    // Memory allocation for LoadPackage
    LoadPackage *pack = calloc(1, sizeof(LoadPackage));
    if (!pack) {
        return NULL;
    }
    pack->type = type;

    if (type == PPM) {
        // Load Package will contain the previous .ppm file that the system loaded
        pack->data = file;
    } else {
        // Checking if the system contains already .lsys or .bdf file => Memory allocation
        // for file path
        const char *path = get_app_file_path(file);
        if (path) {
            pack->data = malloc(strlen(path) + 1);
            if (!pack->data) {
                free(pack);
                return NULL;
            }
            strcpy(pack->data, path);
        }
    }
    return pack;
}

static void free_load_package(LoadPackage **addr_pack)
{
    LoadPackage *pack = (addr_pack) ? *addr_pack : NULL;
    if (!pack) {
        return;
    }

    if (pack->type == PPM) {
        app_file_dtor((AppFile**)&pack->data);
    } else {
        free(pack->data);
    }
    free(pack);
    *addr_pack = NULL;
}

static int8_t execute_load(Command *self)
{
    const Appdata *appdata = get_cmd_receiver(self);
    const CliArgs *args = get_cmd_args(self);
    
    // Loading the new file
    const char *path = args->argv[0];
    AppFile *new_file = app_file_factory(path);
    if (!new_file) {
        printf("Failed to load %s\n", path);
        return 0;
    }
    
    int8_t file_type = get_app_file_type(new_file);
    // Getting the address of the previous file that the system loaded
    AppFile **previous_file_addr = get_file_addr(appdata, file_type);
    // Memory allocation for LoadPackage
    LoadPackage *pack = create_load_package(file_type, *previous_file_addr);
    if (!pack) {
        app_file_dtor(&new_file);
        printf("Failed to load %s\n", path);
        return 0;
    }
    set_cmd_memento(self, pack);
    
    // Checking if the type of the new file is .lsys or .bdf => previous file must be closed
    if (file_type != PPM) {
        app_file_dtor(previous_file_addr);
    }
    // Setting the current file in appdata
    *previous_file_addr = new_file;
    return 1;
}

static void undo_load(Command *self)
{
    const Appdata *appdata = get_cmd_receiver(self);
    const LoadPackage *pack = get_cmd_memento(self);
    // Loading the previous file
    AppFile *previous_file = (pack->type == PPM) ? pack->data : app_file_factory(pack->data);
    // Closing the current file from appdata
    AppFile **current_file_addr = get_file_addr(appdata, pack->type);
    app_file_dtor(current_file_addr); 
    // Setting the previous file as the current in appdata
    *current_file_addr = previous_file;
    if (previous_file && pack->type == PPM) {
        printf(
            "Loaded %s (PPM image %dx%d)\n", 
            get_app_file_path(previous_file), get_ppm_width((const Ppm*)previous_file), 
            get_ppm_height((const Ppm*)previous_file)
        );
    }
    // If the LoadPackage type is different from PPM => data must be freed
    // Otherwise the data must not be freed because it contains the previous .ppm file that now
    // is the current file
    if (pack->type != PPM) {
        free(pack->data);
    }
    free((LoadPackage*)pack);
    set_cmd_memento(self, NULL);
}

static void load_destructor(Command **self)
{
    Command *cmd = (self) ? *self : NULL;
    if (!cmd) {
        return;
    }

    // Memento will contain the LoadPackage
    const LoadPackage *pack = get_cmd_memento(cmd);
    free_load_package((LoadPackage**)&pack);
}

static const CommandVTable LoadVTable = {
    .execute = execute_load,
    .undo = undo_load,
    .destroy_cmd = load_destructor
};

Command *create_load_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: LOAD <file_path>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    return cmd_ctor(1, (Appdata*)appdata, args, &LoadVTable);
}

static int8_t execute_derive(Command *self)
{
    AppFile *lsys = *(AppFile**)get_cmd_receiver(self);
    if (!lsys) {
        printf("No L-system loaded\n");
        return 0;
    }

    const CliArgs *args = get_cmd_args(self);
    uint32_t n = atoi(args->argv[0]);
    char *derivative = derive_lsys((const Lsystem*)lsys, n);
    if (derivative) {
        printf("%s\n", derivative);
    }
    free(derivative);
    return 1;
}

static const CommandVTable DeriveVTable = {
    .execute = execute_derive,
    .undo = NULL,
    .destroy_cmd = NULL
};

Command *create_derive_command(CliEngine *sys, CliArgs *args)
{
    if (!args || args->argc != 1) {
        printf("Error: Invalid number of arguments (%d given)\n", args->argc);
        printf("Usage: DERIVE <N>\n");
        return NULL;
    }

    const Appdata *appdata = access_appdata(sys);
    Command *cmd = cmd_ctor(0, get_file_addr(appdata, LSYSTEM), args, &DeriveVTable);
    return cmd;
}

Command *create_bitcheck_command(CliEngine *sys, CliArgs *args)
{
    return NULL;
}