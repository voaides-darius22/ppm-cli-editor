//#include <stdio.h>
#include <stdlib.h>

#include "../header_files/system_data.h"

typedef struct SystemData {
    Lsystem *lsys_file;
    Ppm *ppm_file;
    Bdf *font_file;
} SystemData;

// SystemData Getters
Lsystem **get_addr_of_lsystem_file(SystemData *appdata)
{
    return (appdata) ? &appdata->lsys_file : NULL;
}

Ppm **get_addr_of_ppm_file(SystemData *appdata)
{
    return (appdata) ? &appdata->ppm_file : NULL;
}

Bdf **get_addr_of_bdf_file(SystemData *appdata)
{
    return (appdata) ? &appdata->font_file : NULL;
}

SystemData *create_system_data(void)
{
    return calloc(1, sizeof(SystemData));
}

SystemData *free_system_data(SystemData *appdata)
{
    if (!appdata) {
        return NULL;
    }

    if (appdata->lsys_file) {
        close_lsystem_file(appdata->lsys_file);
    }

    if (appdata->ppm_file) {
        close_ppm_file(appdata->ppm_file);
    }

    if (appdata->font_file) {
        close_bdf_font(appdata->font_file); 
    }

    free(appdata);
    return NULL;
}