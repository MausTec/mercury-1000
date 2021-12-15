#include "ui.hpp"
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include <cstring>

#include "images/bt_icon_0.h"
#include "images/bt_icon_1.h"
#include "images/rj_icon_0.h"
#include "images/rj_icon_1.h"

static const char* TAG = "UI:MANAGER";

static UI::Page* current_page = nullptr;
static UI::Menu* current_menu = nullptr;

static char _toast[UI_TOAST_MAX_LEN + 1];
static unsigned long _toast_expires_ms = 0;
static bool _toast_allow_clear = false;
static int _toast_line_start = 0;

void ui_init(void) {
    M1K_HAL_ERRCHK(m1k_hal_register_button_cb(M1K_HAL_BUTTON_ANY, ui_handle_click));
    M1K_HAL_ERRCHK(m1k_hal_register_encoder_change(ui_handle_encoder));

    ui_clear_toast(false);
}

void ui_open_page(UI::Page* page) {
    if (current_menu != nullptr) {
        ui_open_menu(nullptr);
    }

    if (current_page != nullptr) {
        current_page->exit();
    }

    current_page = page;

    if (current_page != nullptr) {
        current_page->enter();
    }
}

void ui_open_menu(UI::Menu* menu, bool save_history) {
    if (current_menu != nullptr) {
        current_menu->exit();
    }

    UI::Menu* previous_menu = current_menu;
    current_menu = menu;
    if (current_menu != nullptr) {
        current_menu->enter(previous_menu, save_history);
    }
}

void ui_toast(const char* msg, unsigned long delay_ms, bool allow_clear) {
    unsigned long millis = esp_timer_get_time() / 1000UL;
    strlcpy(_toast, msg, UI_TOAST_MAX_LEN);
    _toast_expires_ms = delay_ms ? millis + delay_ms : 0;
    _toast_allow_clear = allow_clear;
    _toast_line_start = 0;
}

void ui_clear_toast(bool rerender) {
    _toast[0] = '\0';
    _toast_expires_ms = 0;
    _toast_allow_clear = false;
    _toast_line_start = 0;

    if (rerender) ui_rerender();
}

void ui_tick(void) {
    unsigned long millis = esp_timer_get_time() / 1000UL;

    // Expire any stale toast:
    if (_toast_expires_ms > 0 && millis > _toast_expires_ms) {
        ui_clear_toast(false);
    }

    if (current_menu != nullptr) {
        current_menu->loop();
    }

    if (current_page != nullptr) {
        current_page->loop();
    }
}

void ui_handle_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt) {
    if (_toast[0] != '\0') {
        if (_toast_allow_clear) {
            ui_clear_toast(true);
        }

        return;
    }

    if (current_menu != nullptr) {
        return current_menu->on_click(button, evt);
    }

    if (current_page != nullptr) {
        return current_page->on_click(button, evt);
    }
}

void ui_handle_encoder(int difference) {
    if (_toast[0] != '\0') {
        _toast_line_start += difference;
        if (_toast_line_start < 0) _toast_line_start = 0;
        return;
    }

    if (current_menu != nullptr) {
        return current_menu->on_encoder(difference);
    }

    if (current_page != nullptr) {
        return current_page->on_encoder(difference);
    }
}

void ui_render_static(u8g2_t* display) {
    int width = m1k_hal_get_display_width();
    int height = m1k_hal_get_display_height();
    int left = m1k_hal_get_display_left();

    if (_toast[0] != '\0') {
        // draw toasts
        const int border = 3;
        graphics_draw_modal(border);
        graphics_draw_scrolling_text(_toast, _toast_line_start, border + 2, border, width - border - 4, height - border - 1);
    } else {
        // no toast, draw icons
        graphics_draw_image(width - 8, 0, &RJ_ICON_0);
        graphics_draw_image(width - 18, 0, &BT_ICON_1);
    }
}

void ui_rerender(void) {
    if (current_menu != nullptr) {
        return current_menu->rerender();
    }

    current_page->render();
}

bool ui_has_menu_open(void) {
    return current_menu != nullptr;
}

bool ui_has_input_open(void) {
    //return current_input != nullptr;
    return false;
}