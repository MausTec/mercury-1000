#include "ui.hpp"
#include "version.h"
#include "images/mt_8bit.h"

#include "esp_timer.h"

using namespace UI;
static const char *TAG = "PAGE:SPLASH";

static struct {
    int step = 0;
    long delay_us = 0;
} state;

static void enter(Page* page) {
    state.delay_us = esp_timer_get_time();
}

static void render(u8g2_t* display, Page* page) {
    u8g2_SetDrawColor(display, 1);

    if (state.step == 0) {
        auto width = m1k_hal_get_display_width() - MT_8BIT.width;
        graphics_draw_image(width / 2, 3, &MT_8BIT);
    } else {
        const char *product_name = "MERCURY 1000";
        const char *product_rev = VERSION;
        auto left = m1k_hal_get_display_left();
        auto dwidth = m1k_hal_get_display_width();

        u8g2_SetFont(display, u8g2_font_freedoomr10_tu);
        auto width = dwidth - u8g2_GetStrWidth(display, product_name);
        u8g2_DrawStr(display, left + (width / 2), 15, product_name);
        
        u8g2_DrawHLine(display, left + 10, m1k_hal_get_display_height() - 15, dwidth - 20);

        u8g2_SetFont(display, u8g2_font_6x10_tf);
        width = dwidth - u8g2_GetStrWidth(display, product_rev);
        u8g2_DrawStr(display, left + (width / 2), m1k_hal_get_display_height() - 3, product_rev);
    }
}

static void loop(Page* page) {
    if (esp_timer_get_time() - state.delay_us > 2 * 1000000) {
        state.step++;
        state.delay_us = esp_timer_get_time();
        if (state.step == 2) {
            ui_open_page(&Pages::Home);
        } else {
            page->render();
        }
    }
}

static void exit(Page* page) {
    // we actually don't need to fade out here because uh
    // well you see
    //       the display is so slow it looks like it did anyway
    // gfx_fade_out(1000);
}

static page_config_t config = {
    .enter_cb = &enter,
    .render_cb = &render,
    .loop_cb = &loop,
    .exit_cb = &exit,
    .button_cb = nullptr,
    .encoder_cb = nullptr,
};

Page Pages::Splash(&config);