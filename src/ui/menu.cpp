#include "ui.hpp"
#include "ui/menu.hpp"

#include <cstring>

using namespace UI;

Menu::Menu(const char* title, menu_config_t* cfg) : config(cfg) {
    strlcpy(this->title, title, MENU_TITLE_MAXLEN);
};

void Menu::enter(Menu* previous, bool save_history) {
    if (save_history) {
        this->previous_menu = previous;
    }

    this->clear_items();

    if (this->config->enter_cb != nullptr) {
        this->config->enter_cb(this);
    }

    this->render();
}

void Menu::rerender(void) {
    this->clear_items();

    if (this->config->enter_cb != nullptr) {
        this->config->enter_cb(this);
    }

    this->render();
}

void Menu::render(void) {
    u8g2_t* display = m1k_hal_get_display_ptr();
    const int left = m1k_hal_get_display_left();
    const int right = left + m1k_hal_get_display_width();
    const int icon_width = 10 * 2;
    const int menu_start_y = 9;

    u8g2_ClearBuffer(display);

    const size_t num_lines = 3;

    // render menu title
    u8g2_SetFont(display, u8g2_font_4x6_mf);
    u8g2_DrawStr(display, left + 1, 6, this->title);
    u8g2_DrawLine(display, left, 7, right - icon_width, 7);

    { // render menu items
        menu_item_t* ptr = this->current;

        for (int i = 0; i < num_lines / 2; i++) {
            if (ptr->_prev != nullptr) ptr = ptr->_prev;
        }

        for (int i = 0; i < num_lines; i++) {
            if (ptr == nullptr) break;
            int baseline = (menu_start_y + 6) + (8 * i);

            if (ptr == this->current) {
                u8g2_SetDrawColor(display, 0x01);
                u8g2_DrawBox(display, left, baseline - 6, right - left - 3, 7);
            }

            u8g2_SetFontMode(display, 0x01);
            u8g2_SetDrawColor(display, 0x02);
            u8g2_DrawStr(display, left + 1, baseline, ptr->name);
            u8g2_SetDrawColor(display, 0x01);

            ptr = ptr->_next;
        }
    }

    { // render scrollbar
        size_t item_count = this->count_items();
        size_t current_index = this->current_index();
        size_t scrollbar_height = m1k_hal_get_display_height() - menu_start_y;
        size_t scrollbar_top = menu_start_y;

        if (item_count > num_lines) {
            size_t overflow = item_count - num_lines;
            size_t movement_height = scrollbar_height;
            scrollbar_height *= ((float) num_lines / item_count);
            movement_height -= scrollbar_height;

            // figure out what % we are through the menu
            float nperc = 0.0;

            if (current_index > num_lines / 2) {
                nperc = (float) current_index / (item_count - (num_lines / 2));
            }

            scrollbar_top += (movement_height * nperc);
        }

        u8g2_DrawBox(display, right - 2, scrollbar_top, 2, scrollbar_height);
    }

    ui_render_static(display);
    u8g2_SendBuffer(display);
}

void Menu::loop(void) {

}

void Menu::exit(void) {
    this->clear_items();
}

void Menu::on_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt) {
    if (evt != M1K_HAL_BUTTON_EVT_PRESS) {
        return;
    }

    switch (button) {
    case M1K_HAL_BUTTON_MENU:
        confirm_selection();
        break;
    case M1K_HAL_BUTTON_AIROUT:
        ui_open_menu(this->previous_menu, false);
        break;
    case M1K_HAL_BUTTON_AIRIN:
        confirm_selection();
        break;
    default:
        break;
    }
}

void Menu::on_encoder(int dist) {
    if (dist < 0) {
        select_prev(abs(dist));
    } else {
        select_next(dist);
    }
}

void Menu::add_item(const char* name, menu_item_cb cb, void* ptr_arg, int i_arg) {
    menu_item_t* ptr = this->first;
    menu_item_t* current = (menu_item_t*) malloc(sizeof(menu_item_t));

    strlcpy(current->name, name, MENU_TITLE_MAXLEN);
    current->cb = cb;
    current->i_arg = i_arg;
    current->ptr_arg = ptr_arg;
    current->_next = nullptr;

    if (ptr == nullptr) {
        current->_prev = nullptr;
        this->first = current;
        this->last = current;
        this->current = current;
        return;
    }

    while (ptr->_next != nullptr) {
        ptr = ptr->_next;
    }

    current->_prev = ptr;
    ptr->_next = current;
    this->last = current;
}

void Menu::clear_items(void) {
    menu_item_t* ptr = this->first;

    while (ptr != nullptr) {
        menu_item_t* tmp = ptr;
        ptr = ptr->_next;
        free(tmp);
    }

    this->first = nullptr;
    this->current = nullptr;
}

size_t Menu::current_index(void) {
    size_t n = 0;
    menu_item_t* ptr = this->first;

    while (ptr != nullptr && ptr != this->current) {
        n++;
        ptr = ptr->_next;
    }

    return n;
}

size_t Menu::count_items(void) {
    size_t n = 0;
    menu_item_t* ptr = this->first;

    while (ptr != nullptr) {
        n++;
        ptr = ptr->_next;
    }

    return n;
}

bool Menu::has_items(void) {
    return this->first != nullptr;
}

void Menu::select_next(int dist) {
    menu_item_t* ptr = this->current;

    for (int i = 0; i < dist; i++) {
        if (ptr == nullptr || ptr->_next == nullptr) {
            ptr = this->last;
        } else {
            ptr = ptr->_next;
        }
    }

    this->current = ptr;
    this->render();
}

void Menu::select_prev(int dist) {
    menu_item_t* ptr = this->current;

    for (int i = 0; i < dist; i++) {
        if (ptr == nullptr || ptr->_prev == nullptr) {
            ptr = this->first;
        } else {
            ptr = ptr->_prev;
        }
    }

    this->current = ptr;
    this->render();
}

void Menu::confirm_selection(void) {
    if (this->current == nullptr) {
        return;
    }

    if (this->current->cb != nullptr) {
        this->current->cb(this, this->current->ptr_arg, this->current->i_arg);
    }
}