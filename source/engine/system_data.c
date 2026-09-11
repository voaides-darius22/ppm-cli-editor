#include <stdio.h>
#include <stdlib.h>
#include "../../header_files/engine/system_data.h"

typedef struct Appdata {
    AppFile *lsys;
    AppFile *ppm;
    AppFile *bdf;
} Appdata;

// Appdata Methods
Appdata *appdata_ctor(void)
{
    return calloc(1, sizeof(Appdata));
}

void appdata_dtor(Appdata **self)
{
    Appdata *appdata = (self) ? *self : NULL;
    if (!appdata) {
        return;
    }

    // Deleting Files
    app_file_dtor(&appdata->lsys);
    app_file_dtor(&appdata->bdf);
    app_file_dtor(&appdata->ppm);
    free(appdata);
    *self = NULL;
}

AppFile **get_file_addr(const Appdata *self, int8_t file_type)
{
    if (!self) {
        return NULL;
    }

    switch (file_type) {
        case LSYSTEM: {return &((Appdata*)self)->lsys;}
        case PPM:  {return &((Appdata*)self)->ppm;}
        case BDF: {return &((Appdata*)self)->bdf;}
    }
    return NULL;
}