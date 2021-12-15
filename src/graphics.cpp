#include "graphics.hpp"

#include "esp_log.h"

static const char* TAG = "graphics";

void graphics_draw_image(u8g2_uint_t x, u8g2_uint_t y, graphics_image_t* image) {
    u8g2_t* display = m1k_hal_get_display_ptr();
    x += m1k_hal_get_display_left();
    u8g2_DrawXBM(display, x, y, image->width, image->height, image->bitmap);
}

void graphics_fill_pattern(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint8_t pattern) {
    u8g2_t* display = m1k_hal_get_display_ptr();
    x += m1k_hal_get_display_left();

    for (u8g2_uint_t cx = x; cx < x + w; cx++) {
        for (u8g2_uint_t cy = y; cy < y + h; cy++) {
            if ((cx + cy) % pattern == 0) {
                u8g2_DrawPixel(display, cx, cy);
            }
        }
    }
}

void graphics_draw_modal(u8g2_uint_t border_px) {
    const int width = m1k_hal_get_display_width();
    const int height = m1k_hal_get_display_height();
    const int left = m1k_hal_get_display_left();

    u8g2_t* display = m1k_hal_get_display_ptr();

    graphics_fill_pattern(0, 0, width, height, 3);
    u8g2_SetDrawColor(display, 1);
    u8g2_DrawFrame(display, left + border_px, border_px, width - (2 * border_px), height - (2 * border_px));
    u8g2_SetDrawColor(display, 0);
    u8g2_DrawFrame(display, left + border_px - 1, border_px - 1, width - (2 * (border_px - 1)), height - (2 * (border_px - 1)));
    u8g2_DrawBox(display, left + border_px + 1, border_px + 1, width - (2 * (border_px + 1)), height - (2 * (border_px + 1)));
}

void graphics_draw_scrolling_text(const char *text, size_t line_start, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h) {
    const int width = m1k_hal_get_display_width();
    const int height = m1k_hal_get_display_height();
    const int left = m1k_hal_get_display_left();

    u8g2_t* display = m1k_hal_get_display_ptr();

    u8g2_SetFont(display, u8g2_font_5x7_tf);
    u8g2_SetDrawColor(display, 1);

    // Get Font / Line Measurements
    const int th = 1 + u8g2_GetAscent(display) - u8g2_GetDescent(display);
    const int max_lines = h / th;

    char line_buf[max_lines][40];
    size_t curr_line = 0;
    size_t text_idx = 0;
    size_t scroll_idx = 0;

    while (text[text_idx] != '\0') {
        u8g2_uint_t line_w = 0;
        size_t line_idx = 0;

        while (line_w < w) {
            if (text[text_idx] == '\0') {
                break;
            }

            if (text[text_idx] == '\n') {
                text_idx++;
                break;
            }

            line_w += u8g2_GetGlyphWidth(display, text[text_idx]);

            if (line_w < w) {
                // commit char add
                line_buf[scroll_idx][line_idx++] = text[text_idx++];
            }
        }

        line_buf[scroll_idx][line_idx] = '\0';
        curr_line++;

        if (curr_line <= line_start) {
            continue;
        }

        if (curr_line > max_lines) {
            // we need to know how many more lines come
            break;
        }

        scroll_idx = (scroll_idx + 1) % max_lines;
    }

    for (int i = 0; i < max_lines; i++) {
        u8g2_DrawStr(display, left + x, y + (th * (i + 1)), line_buf[(i + scroll_idx) % max_lines]);
    }

    // TODO: Add a scrollbar and prevent scrolling past the end by I guess keeping a line buffer THEN drawing?
}