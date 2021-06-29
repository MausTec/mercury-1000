#include "ui/page.hpp"
#include "pages.hpp"
#include "graphics.hpp"

#include "images/mt_8bit.h"
#include "images/milker_icon_0.h"
#include "images/milker_icon_1.h"
#include "images/milker_icon_2.h"
#include "images/milker_icon_3.h"

using namespace UI;
static const char* TAG = "PAGE:HOME";

static struct {
    int speed = 0;
    long last_icon_change_us = 0;
    int icon_step = 0;
    bool icon_desc = false;
} state;

static graphics_image_t *MILKER_ICONS[4] = {
    &MILKER_ICON_0,
    &MILKER_ICON_1,
    &MILKER_ICON_2,
    &MILKER_ICON_3,
};

static void render(m1k_hal_display_t* display, Page* page) {
    auto milker_icon = MILKER_ICONS[state.icon_step];
    auto width = m1k_hal_get_display_width() - milker_icon->width - MT_8BIT.width;

    display->setDrawColor(1);
    graphics_draw_image(0, 0, milker_icon);
    graphics_draw_image(milker_icon->width + (width / 2), 3, &MT_8BIT);    
}

static void loop(Page* page) {
    if (millis() - state.last_icon_change_us > 300) {
        state.last_icon_change_us = millis();
        auto icon_step = state.icon_step + (state.icon_desc ? -1 : 1);
        if (icon_step < 0) {
            state.icon_desc = false;
            icon_step = 1;
        } else if (icon_step > 3) {
            icon_step = 2;
            state.icon_desc = true;
        }
        state.icon_step = icon_step;
        page->render();
    }
}

static void exit(Page* page) {
    state.speed = 0;
    ESP_LOGE(TAG, "HOME EXIT");
}

static void enter(Page* page) {
    state.speed = 0;
    ESP_LOGE(TAG, "HOME ENTER");
}

static void encoder_change(int difference, Page* page) {
    state.speed += difference;
    page->render();
}

static page_config_t config = {
    .enter_cb = &enter,
    .render_cb = &render,
    .loop_cb = &loop,
    .exit_cb = &exit,
    .button_cb = nullptr,
    .encoder_cb = &encoder_change,
};

Page Pages::Home(&config);