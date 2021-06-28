#include "ui.hpp"
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"

static const char* TAG = "UI:MANAGER";

static UI::Page* current_page = nullptr;
// static UI::Menu *current_menu = nullptr;

void ui_init(void) {
    M1K_HAL_ERRCHK(m1k_hal_register_button_hold(M1K_HAL_BUTTON_MENU, ui_handle_click));
    M1K_HAL_ERRCHK(m1k_hal_register_button_press(M1K_HAL_BUTTON_MENU, ui_handle_click));
    M1K_HAL_ERRCHK(m1k_hal_register_encoder_change(ui_handle_encoder));
}

void ui_open_page(UI::Page* page) {
    if (current_page != nullptr) {
        current_page->exit();
    }

    current_page = page;
    current_page->enter();
}

void ui_tick(void) {
    if (current_page != nullptr) {
        current_page->loop();
    }
}

void ui_handle_click(m1k_hal_button_t button, bool is_hold) {
    if (current_page != nullptr) {
        current_page->on_click(button, is_hold);
    }
}

void ui_handle_encoder(int difference) {
    if (current_page != nullptr) {
        current_page->on_encoder(difference);
    }
}
