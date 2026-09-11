#pragma once
#include <stdint.h>
#include "app_file.h"

typedef struct AppFileProtected AppFileProtected;

typedef struct AppFileVTable {
    void (*load_file)(AppFile **self);
    void (*destroy_file)(AppFile **self);
} AppFileVTable;

typedef struct AppFile {
    AppFileVTable *vptr;
    AppFileProtected *protected;
} AppFile;

void init_file_app(AppFile **self, const char *path, const AppFileVTable *vptr, int8_t type);