#pragma once

#include "lsys.h"
#include "ppm.h"

typedef struct SystemData {
    Lsystem *lsys_file;
    Ppm *ppm_file;
} SystemData;

SystemData *create_system_data(void);
SystemData *free_system_data(SystemData *app_data);