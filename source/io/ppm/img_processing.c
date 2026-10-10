#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../../header_files/io/ppm/img_processing.h"

void img_brightness_adjustment(Ppm *img)
{
    if (!img) { return; }
    
    // Checking if the brightness level is set to default
    double brightness_level = get_img_brightness_level(img);
    if (brightness_level == 50.0) { return; }

    int32_t pixels = get_img_export_width(img) * get_img_export_height(img);
    RgbPixel *export_pixel_raster = get_img_export_raster(img);
    
    const double brightness_base = 4.0;
    float gamma = pow(brightness_base, (50 - brightness_level) / 50);
    
    for (int i = 0; i < pixels; i++) {
        RgbPixel *current_pixel = &export_pixel_raster[i];
        export_pixel_raster[i] = pixel_gamma_correction(current_pixel, gamma);
    }
}

void process_img_rotation(Ppm *img)
{
    if (!img) { return; }

    const double PI = 3.1415926535;
    // Converting from degrees to radians
    double angle_in_rad = get_img_angle(img) * PI / 180.0;
    double _sin = sin(angle_in_rad), _cos = cos(angle_in_rad);

    uint32_t width = get_img_width(img);
    uint32_t height = get_img_height(img);
    
    // Computing the coordinates of the img's old center
    int32_t old_cx = width / 2;
    int32_t old_cy = height / 2;

    // Computing the bounding-box dimensions for the rotated img
    uint32_t bbw = round(width * fabs(_cos) + height * fabs(_sin));
    uint32_t bbh = round(width * fabs(_sin) + height * fabs(_cos));
    uint32_t pixels = bbw * bbh;

    // Computing the coordinates of the img's new center
    int32_t new_cx = bbw / 2;
    int32_t new_cy = bbh / 2;
    
    // Memory allocation for the exporting pixel raster
    RgbPixel *export_pixel_raster = malloc(pixels * sizeof(RgbPixel));
    if (!export_pixel_raster) { return; }
    set_img_export_raster(img, export_pixel_raster);
    set_img_export_width(img, bbw);
    set_img_export_height(img, bbh);

    const RgbPixel *original_pixel_raster = get_img_original_raster(img);

    // Backward Image Warping
    for (unsigned int i = 0; i < pixels; i++) {
        int32_t new_x = i % bbw;
        int32_t new_y = i / bbw;
        
        int32_t old_x = round((new_x - new_cx) * _cos + (new_y - new_cy) * _sin + old_cx);
        int32_t old_y = round(-(new_x - new_cx) * _sin + (new_y - new_cy) * _cos + old_cy);
        
        if (old_x >= 0 && old_x < width && old_y >= 0 && old_y < height) {
            export_pixel_raster[i] =  original_pixel_raster[old_y * width + old_x];
        } else {
            RgbPixel blank_pixel = {255, 255, 255};
            export_pixel_raster[i] = blank_pixel;
        }
    }
}