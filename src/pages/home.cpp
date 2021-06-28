#include "ui/page.hpp"
#include "pages.hpp"

using namespace UI;
static const char *TAG = "PAGE:HOME";

static void render(m1k_hal_display_t *display, Page *page) {
    display->setDrawColor(1);
    display->setFont(u8g2_font_6x12_tr);
    display->drawStr(m1k_hal_get_display_left(), 7, "Home Page Rendered");
}

static void exit(Page *page) {
    ESP_LOGE(TAG, "HOME EXIT");
}

static void enter(Page *page) {
    ESP_LOGE(TAG, "HOME ENTER");
}

static page_config_t config = {
    .enter_cb = &enter,
    .render_cb = &render,
    .loop_cb = nullptr,
    .exit_cb = &exit,
};

Page Pages::Home(&config);