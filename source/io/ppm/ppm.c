#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "../../../header_files/io/app_file_protected.h"
#include "../../../header_files/io/ppm/ppm.h"
#include "../../../header_files/utils/io_utils.h"
#include "../../../header_files/io/ppm/img_processing.h"

#define BUFFER_SIZE 1024
#define MAX_ARGS 2
#define SCANNING 1

// Ppm Flags
#define MAGIC_NUMBER_FLAG 0
#define DIMENSIONS_FLAG 1
#define CHANNEL_MAX_VALUE_FLAG 2
#define PX_RASTER_FLAG 3
#define PPM_FILE_VALID 15

typedef struct {
    uint8_t argc;
    const char *argv[MAX_ARGS];
} PpmArgs;

typedef enum PpmState {
    PPM_INIT_STATE, WAITING_MAGIC_NUMBER, WAITING_IMG_DIMENSIONS, WAITING_CHANNEL_MAX_VALUE, 
    WAITING_PX_RASTER, PPM_CLOSING_STATE
} PpmState;

typedef struct ImageGeometry {
    uint32_t original_width, original_height;
    uint32_t export_width, export_height;
    double angle;
} ImageGeometry;

typedef struct ImageVisualProperties {
    double brightness_level;
} ImageVisualProperties;

typedef struct Ppm{
    AppFile base;
    // Ppm Data Protected
    char magic_bytes[3];
    uint8_t max_value_channel;
    ImageGeometry geometry;
    ImageVisualProperties visual;
    RgbPixel *original_pixel_raster;
    RgbPixel *export_pixel_raster;
} Ppm;

typedef struct {
    FILE *fp;
    Ppm *file;
    PpmState state;
    int8_t img_flags;
} PpmBuilder;

typedef int8_t (*PpmReadHandler)(PpmBuilder *);

static const AppFileVTable PpmFileVTable = {
    .load_file = load_ppm,
    .destroy_file = ppm_dtor
};

static PpmArgs ppm_tokenizer(char *ppm_input)
{
    PpmArgs ppm_args;
    memset(&ppm_args, 0, sizeof(ppm_args));

    if (!ppm_input) { return ppm_args; }

    // Checking if the input line contains comments => ignore the rest of the line after #
    char *comment = strchr(ppm_input, '#');
    if (comment) {
        *comment = '\0';
    }

    char *token = strtok(ppm_input, " ");
    while (token && ppm_args.argc < MAX_ARGS) {
        ppm_args.argv[ppm_args.argc] = token;
        ppm_args.argc++;
        token = strtok(NULL, " ");
    }

    return ppm_args;
}

// PpmBuilder Functions
static void ppm_change_state(PpmBuilder *builder)
{
    switch (builder->img_flags) {
        case 0: { builder->state = WAITING_MAGIC_NUMBER; break; }
        case 1: { builder->state = WAITING_IMG_DIMENSIONS; break; }
        case 3: { builder->state = WAITING_CHANNEL_MAX_VALUE; break; }
        case 7: { builder->state = WAITING_PX_RASTER; break; }
        case 15: { builder->state = PPM_CLOSING_STATE; break; }
    }
}

static int8_t read_ppm_magic_number(PpmBuilder *builder)
{
    if (builder->state != WAITING_MAGIC_NUMBER) { return 0; }

    Ppm *img = builder->file;
    char *ppm_input = read_file_line(builder->fp);
    if (!ppm_input) { return 0; }

    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 1) {
        const char *magic_number_token = ppm_args.argv[0];
        if (!strcmp(magic_number_token, "P6")) {
            strncpy(img->magic_bytes, magic_number_token, sizeof(img->magic_bytes) - 1);
            img->magic_bytes[2] = '\0';
            // Set magic number flag
            builder->img_flags |= (1 << MAGIC_NUMBER_FLAG);
        }
    }
    
    free(ppm_input);
    return 1;
}

static int8_t read_ppm_dimensions(PpmBuilder *builder)
{
    if (builder->state != WAITING_IMG_DIMENSIONS) { return 0; }

    Ppm *img = builder->file;
    char *ppm_input = read_file_line(builder->fp);
    if (!ppm_input) { return 0; }

    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 2) {
        img->geometry.original_width = atoi(ppm_args.argv[0]);
        img->geometry.original_height = atoi(ppm_args.argv[1]);
        // Set dimensions flag
        builder->img_flags |= (1 << DIMENSIONS_FLAG);
    }

    free(ppm_input);
    return 1;
}

static int8_t read_ppm_max_channel_value(PpmBuilder *builder)
{
    if (builder->state != WAITING_CHANNEL_MAX_VALUE) { return 0; }

    Ppm *img = builder->file;
    char *ppm_input = read_file_line(builder->fp);
    if (!ppm_input) { return 0; }
    
    PpmArgs ppm_args = ppm_tokenizer(ppm_input);
    if (ppm_args.argc == 1) {
        const char *channel_max_value_token = ppm_args.argv[0];
        img->max_value_channel = atoi(channel_max_value_token);
        // Set channel max value flag
        builder->img_flags |= (1 << CHANNEL_MAX_VALUE_FLAG);
    }

    free(ppm_input);
    return 1;
}

static int8_t read_ppm_px_raster(PpmBuilder *builder)
{
    if (builder->state != WAITING_PX_RASTER) { return 0; }

    Ppm *img = builder->file;
    uint32_t pixels = img->geometry.original_width * img->geometry.original_height;
    img->original_pixel_raster = malloc(pixels * sizeof(RgbPixel));
    if (!img->original_pixel_raster) { return 0; }
    fread(img->original_pixel_raster, sizeof(RgbPixel), pixels, builder->fp);
    // Set pixel raster flag
    builder->img_flags |= (1 << PX_RASTER_FLAG);
    return 1;
}

static PpmReadHandler get_ppm_read_handler(PpmBuilder *builder)
{
    PpmReadHandler handler = NULL;
    switch (builder->state) {
        case WAITING_MAGIC_NUMBER: { handler = read_ppm_magic_number; break; }
        case WAITING_IMG_DIMENSIONS: { handler = read_ppm_dimensions; break; }
        case WAITING_CHANNEL_MAX_VALUE: { handler = read_ppm_max_channel_value; break; }
        case WAITING_PX_RASTER: { handler = read_ppm_px_raster; break; }
    }
    return handler;
}

static void free_ppm_builder(PpmBuilder **addr_builder)
{
    if (!addr_builder || !*addr_builder) { return; }

    PpmBuilder *builder = *addr_builder;
    if (builder->fp) {
        fclose(builder->fp);
    }
    free(builder);
    *addr_builder = NULL;
}

static PpmBuilder *create_ppm_builder(Ppm *self)
{
    PpmBuilder *builder = (self) ? calloc(1, sizeof(PpmBuilder)) : NULL;
    if (!builder) { return NULL; }

    // File Opening (Read Mode)
    const char *path = get_app_file_path((const AppFile *)self);
    builder->fp = fopen(path, "rb");
    if (!builder->fp) {
        free_ppm_builder(&builder);
        return NULL;
    }

    builder->file = self;
    return builder;
}

// Ppm Methods
Ppm *ppm_ctor(const char *path)
{
    // .ppm File Configuration
    Ppm *img = (path) ? calloc(1, sizeof(Ppm)) : NULL;
    init_file_app((AppFile**)&img, path, &PpmFileVTable, PPM);
    if (!img) { return NULL; }

    // Setting default visual properties
    img->visual.brightness_level = 50;

    // Setting default geometry properties
    img->geometry.angle = 0;
    
    // File Loading
    app_file_load((AppFile**)&img);
    return img;
}

void ppm_dtor(AppFile **self)
{
    Ppm *img = (self) ? (Ppm*)*self : NULL;
    if (!img) { return; }
    free(img->original_pixel_raster);
}

void load_ppm(AppFile **self)
{
    PpmBuilder *builder = (self) ? create_ppm_builder((Ppm*)*self) : NULL;
    if (!builder) {
        app_file_dtor(self);
        return;
    }

    while (SCANNING) {
        // Checking if the file reached EOF
        if (feof(builder->fp)) { break; }

        ppm_change_state(builder);
        // Checking if the ppm builder's state is CLOSING_STATE
        if (builder->state == PPM_CLOSING_STATE) { break; }

        PpmReadHandler read_exec = get_ppm_read_handler(builder);
        int8_t exit_code = read_exec(builder);
        if (!exit_code) {
            free_ppm_builder(&builder);
            app_file_dtor(self);
            return;
        }
    }

    // Checking if all img flags are set (validating file)
    if (builder->img_flags != PPM_FILE_VALID) {
        app_file_dtor(self);
    } else {
        printf(
            "Loaded %s (PPM image %dx%d)\n", 
            get_app_file_path(*self), get_img_width((const Ppm*)*self), 
            get_img_height((const Ppm*)*self)
        );
    }
    free_ppm_builder(&builder);
}

int8_t export_ppm(const char *path, Ppm *self)
{
    // File Opening (Write Mode)
    FILE *fp = (self && path) ? fopen(path, "wb") : NULL;
    if (!fp) { 
        printf("Image couldn't be exported\n"); 
        return 0; 
    }

    // Memory allocation for export raster & geometry adjustments
    process_img_rotation(self);
    if (!self->export_pixel_raster) {
        printf("Image couldn't be exported\n"); 
        return 0; 
    }
    int32_t pixels = self->geometry.export_width * self->geometry.export_height;
    
    // Applying color adjustments
    img_brightness_adjustment(self);

    char buffer[BUFFER_SIZE];
    fwrite(self->magic_bytes, sizeof(char), strlen(self->magic_bytes), fp);
    putc('\n', fp);
    sprintf(
        buffer, "%d %d\n%d\n", 
        self->geometry.export_width, self->geometry.export_height, 
        self->max_value_channel
    );
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fwrite(self->export_pixel_raster, sizeof(RgbPixel), pixels, fp);
    fclose(fp);

    free(self->export_pixel_raster);
    self->export_pixel_raster = NULL;
    self->geometry.export_width = 0;
    self->geometry.export_height = 0;

    return 1;
}

const char *get_ppm_magic_bytes(const Ppm *self)
{
    return (self) ? self->magic_bytes : NULL;
}

uint32_t get_img_width(const Ppm *self)
{
    return (self) ? self->geometry.original_width : 0;
}

uint32_t get_img_height(const Ppm *self)
{
    return (self) ? self->geometry.original_height : 0;
}

uint32_t get_img_export_width(const Ppm *self)
{
    return (self) ? self->geometry.export_width : 0;
}

uint32_t get_img_export_height(const Ppm *self)
{
    return (self) ? self->geometry.export_height : 0;
}

uint8_t get_ppm_max_value_channel(const Ppm *self)
{
    return (self) ? self->max_value_channel : 0;
}

RgbPixel *get_img_original_raster(const Ppm *self)
{
    return (self) ? self->original_pixel_raster : NULL;
}

RgbPixel *get_img_export_raster(const Ppm *self)
{
    return (self) ? self->export_pixel_raster : NULL;
}

double get_img_brightness_level(const Ppm *self)
{
    return (self) ? self->visual.brightness_level : 0;
}

double get_img_angle(const Ppm *self)
{
    return (self) ? self->geometry.angle : 0;
}

void add_img_angle(Ppm *self, double angle)
{
    if (!self) { return; }
    self->geometry.angle += angle;
    self->geometry.angle = fmod(self->geometry.angle, 360.0);
}

void set_img_angle(Ppm *self, double angle)
{
    if (!self) { return; }
    self->geometry.angle = fmod(angle, 360.0);
}

RgbPixel *clone_pixel_raster(const Ppm *self)
{
    if (!self) { return NULL; }

    uint32_t pixels = self->geometry.original_width * self->geometry.original_height;
    RgbPixel *pixel_raster_clone = malloc(pixels * sizeof(RgbPixel));
    if (!pixel_raster_clone) { return NULL; }
    memcpy(pixel_raster_clone, self->original_pixel_raster, pixels * sizeof(RgbPixel));
    return pixel_raster_clone;
}

void overwrite_pixel_raster(Ppm *self, const RgbPixel *pixel_raster)
{
    if (!self || !pixel_raster) { return; }

    uint32_t pixels = self->geometry.original_width * self->geometry.original_height;
    memcpy(self->original_pixel_raster, pixel_raster, pixels * sizeof(RgbPixel));
}

void set_img_raster(
    Ppm *self, 
    uint32_t width, uint32_t height, 
    RgbPixel *pixel_raster
) {
    if (!self) { return; }

    self->geometry.original_width = width;
    self->geometry.original_height = height;
    if (self->original_pixel_raster) {
        free(self->original_pixel_raster);
    }
    self->original_pixel_raster = pixel_raster;
}

void set_img_export_raster(Ppm *self, RgbPixel *export_pixel_raster)
{
    if (!self) { return; }
    self->export_pixel_raster = export_pixel_raster;
}

void set_img_export_width(Ppm *self, uint32_t export_width)
{
    if (!self) { return; }
    self->geometry.export_width = export_width;
}

void set_img_export_height(Ppm *self, uint32_t export_height)
{
    if (!self) { return; }
    self->geometry.export_height = export_height;
}

void set_img_brightness_level(Ppm *self, double brightness_level)
{
    if (!self) { return; }
    self->visual.brightness_level = brightness_level;
}

RgbPixel pixel_gamma_correction(RgbPixel *px, double gamma)
{
    RgbPixel new_px = {0, 0, 0};
    if (!px) { return new_px; }
    
    new_px.red_channel = channel_gamma_correction(px->red_channel, gamma);
    new_px.green_channel = channel_gamma_correction(px->green_channel, gamma);
    new_px.blue_channel = channel_gamma_correction(px->blue_channel, gamma);

    return new_px;
}

uint8_t channel_gamma_correction(uint8_t channel, double gamma)
{
    double normalized = channel / 255.0;
    double corrected = pow(normalized, gamma);
    return round(corrected * 255);
}