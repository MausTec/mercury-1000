#ifndef __ui_manager_hpp
#define __ui_manager_hpp

#include "ui/page.hpp"
#include "pages.hpp"

void ui_init(void);
void ui_open_page(UI::Page *page);
//void ui_open_menu(UI::Menu *menu);
//void ui_close_menu(void);
//void ui_toast(const char *msg);

void ui_tick(void);
void ui_handle_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt);
void ui_handle_encoder(int difference);

#endif