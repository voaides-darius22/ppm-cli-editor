#pragma once

typedef enum AppFileType {
    LSYSTEM, PPM, BDF
} AppFileType;
typedef struct AppFile AppFile;

// AppFile Methods
void app_file_dtor(AppFile **self);
void app_file_load(AppFile **self);
const char *get_app_file_path(const AppFile *self);
int8_t get_app_file_type(const AppFile *self);
AppFile *app_file_factory(const char *path);