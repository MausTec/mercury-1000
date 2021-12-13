#ifndef __graphics_hpp
#define __graphics_hpp

#include "m1k-hal.hpp"
#include "u8g2.h"

typedef const unsigned char graphics_image_data_t;

struct graphics_image {
    u8g2_uint_t width;
    u8g2_uint_t height;
    graphics_image_data_t *bitmap;
};


typedef struct graphics_image graphics_image_t;

void graphics_draw_image(u8g2_uint_t x, u8g2_uint_t y, graphics_image_t *image);

#endif