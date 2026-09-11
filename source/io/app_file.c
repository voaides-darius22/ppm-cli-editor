#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../header_files/io/app_file.h"
#include "../../header_files/io/app_file_protected.h"
#include "../../header_files/io/lsys/lsys.h"
#include "../../header_files/io/ppm/ppm.h"
#include "../../header_files/io/bdf/bdf.h"

typedef struct AppFileProtected {
    char *path;
    AppFileType type;
} AppFileProtected;

// AppFile Methods
void init_file_app(AppFile **self, const char *path, const AppFileVTable *vptr, int8_t type)
{
    AppFile *file = (self) ? *self : NULL;
    if (!file || !path || !vptr) {
        app_file_dtor(self);
        return;
    }

    file->vptr = (AppFileVTable*)vptr;
    // Memory allocation for protected data
    file->protected = calloc(1, sizeof(AppFileProtected));
    if (!file->protected) {
        app_file_dtor(self);
        return;
    }
    file->protected->type = type;

    // Memory allocation for file path
    file->protected->path = malloc(strlen(path) + 1);
    if (!file->protected->path) {
        app_file_dtor(self);
        return;
    }
    strcpy(file->protected->path, path);
}

void app_file_dtor(AppFile **self)
{
    AppFile *file = (self) ? *self : NULL;
    if (!file) {
        return;
    }

    // Calling Virtual Destructor
    if (file->vptr->destroy_file) {
        file->vptr->destroy_file(self);
    }
    
    // Base Destructor
    free(file->protected->path);
    free(file->protected);
    free(file);
    *self = NULL;
}

void app_file_load(AppFile **self)
{
    AppFile *file = (self) ? *self : NULL;
    if (!file) {
        return;
    }
    file->vptr->load_file(self);
}

const char *get_app_file_path(const AppFile *self)
{
    return (self && self->protected) ? self->protected->path : NULL;
}

int8_t get_app_file_type(const AppFile *self)
{
    return (self) ? self->protected->type : -1; 
}

AppFile *app_file_factory(const char *path)
{
    if (!path) {
        return NULL;
    } else if (strstr(path, ".lsys")) {
        return (AppFile*)lsys_ctor(path);
    } else if (strstr(path, ".ppm")) {
        return (AppFile*)ppm_ctor(path);
    } else if (strstr(path, ".bdf")) {
        return (AppFile*)bdf_ctor(path);
    }
    return NULL;
}