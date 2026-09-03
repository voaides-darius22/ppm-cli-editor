#pragma once

#include "lsys.h"
#include "ppm.h"
#include "bdf.h"

typedef struct SystemData {
    Lsystem *lsys_file;
    Ppm *ppm_file;
    Bdf *font_file;
} SystemData;

SystemData *create_system_data(void);
SystemData *free_system_data(SystemData *app_data);