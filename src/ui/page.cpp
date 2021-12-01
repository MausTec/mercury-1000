#include "ui/page.hpp"
#include "m1k-hal.hpp"
#include "ui.hpp"

namespace UI {
    void Page::enter(void) {
        if (config->enter_cb != nullptr) config->enter_cb(this);
        render();
    }

    void Page::render(void) {
        if (ui_has_menu_open() || ui_has_input_open()) {
            return;
        }
        
        if (config->render_cb != nullptr) {
            auto display = m1k_hal_get_display_ptr();
            display->clearBuffer();
            config->render_cb(display, this);
            ui_render_static(display);
            display->sendBuffer();
        }
    }

    void Page::loop(void) {
        if (config->loop_cb != nullptr) config->loop_cb(this);
    }

    void Page::exit(void) {
        if (config->exit_cb != nullptr) config->exit_cb(this);
    }

    void Page::on_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt) {
        if (config->button_cb != nullptr) config->button_cb(button, evt, this);
    }

    void Page::on_encoder(int difference) {
        if (config->encoder_cb != nullptr) config->encoder_cb(difference, this);
    }
}