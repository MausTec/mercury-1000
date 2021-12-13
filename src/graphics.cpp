#include "graphics.hpp"

void graphics_draw_image(u8g2_uint_t x, u8g2_uint_t y, graphics_image_t *image) 
{
    auto display = m1k_hal_get_display_ptr();
    auto offset = m1k_hal_get_display_left();
    u8g2_DrawXBM(display, x + offset, y, image->width, image->height, image->bitmap);
}
