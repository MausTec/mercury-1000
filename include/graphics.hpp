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
void graphics_fill_pattern(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint8_t pattern);
void graphics_draw_modal(u8g2_uint_t border_px);
size_t graphics_draw_scrolling_text(const char *text, size_t line_start, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);

#endif