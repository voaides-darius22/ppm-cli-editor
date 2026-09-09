#pragma once

#include "lsys.h"
#include "ppm.h"
#include "bdf.h"

typedef struct SystemData SystemData;

// SystemData Getters
Lsystem **get_addr_of_lsystem_file(SystemData *appdata);
Ppm **get_addr_of_ppm_file(SystemData *appdata);
Bdf **get_addr_of_bdf_file(SystemData *appdata);

// SystemData Constructor
SystemData *create_system_data(void);

// SystemData Destructor
SystemData *free_system_data(SystemData *appdata);