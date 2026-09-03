#include <stdio.h>
#include <stdlib.h>

#include "../header_files/system_data.h"

SystemData *create_system_data(void)
{
    return calloc(1, sizeof(SystemData));
}

SystemData *free_system_data(SystemData *app_data)
{
    if (!app_data) {
        return NULL;
    }

    if (app_data->lsys_file) {
        close_lsystem_file(app_data->lsys_file);
    }

    if (app_data->ppm_file) {
        close_ppm_file(app_data->ppm_file);
    }

    if (app_data->font_file) {
        close_bdf_font(app_data->font_file); 
    }

    free(app_data);
    return NULL;
}