#pragma once
#include "../../header_files/io/app_file.h"

typedef struct Appdata Appdata;

// Appdata Methods
Appdata *appdata_ctor(void);
void appdata_dtor(Appdata **self);
AppFile **get_file_addr(const Appdata *self, int8_t file_type);
