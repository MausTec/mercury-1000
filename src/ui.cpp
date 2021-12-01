#include "ui.hpp"
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"

#include "images/bt_icon_0.h"
#include "images/bt_icon_1.h"
#include "images/rj_icon_0.h"
#include "images/rj_icon_1.h"

static const char* TAG = "UI:MANAGER";

static UI::Page* current_page = nullptr;
static UI::Menu *current_menu = nullptr;

static char* _toast[40];

void ui_init(void) {
    M1K_HAL_ERRCHK(m1k_hal_register_button_cb(M1K_HAL_BUTTON_ANY, ui_handle_click));
    M1K_HAL_ERRCHK(m1k_hal_register_encoder_change(ui_handle_encoder));

    _toast[0] = '\0';
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

    UI::Menu *previous_menu = current_menu;
    current_menu = menu;
    if (current_menu != nullptr) {
        current_menu->enter(previous_menu, save_history);
    }
}

void ui_tick(void) {
    if (current_menu != nullptr) {
        current_menu->loop();
    }

    if (current_page != nullptr) {
        current_page->loop();
    }
}

void ui_handle_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt) {
    if (current_menu != nullptr) {
        return current_menu->on_click(button, evt);
    }

    if (current_page != nullptr) {
        return current_page->on_click(button, evt);
    }
}

void ui_handle_encoder(int difference) {
    if (current_menu != nullptr) {
        return current_menu->on_encoder(difference);
    }

    if (current_page != nullptr) {
        return current_page->on_encoder(difference);
    }
}

void ui_render_static(m1k_hal_display_t* display) {
    int width = m1k_hal_get_display_width();
    graphics_draw_image(width - 8, 0, &RJ_ICON_0);
    graphics_draw_image(width - 18, 0, &BT_ICON_1);
}

bool ui_has_menu_open(void) {
    return current_menu != nullptr;
}

bool ui_has_input_open(void) {
    //return current_input != nullptr;
    return false;
}