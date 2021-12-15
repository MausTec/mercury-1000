#ifndef __ui_manager_hpp
#define __ui_manager_hpp

#include "ui/page.hpp"
#include "ui/menu.hpp"
#include "pages.hpp"

#define UI_TOAST_MAX_LEN 120

void ui_init(void);
void ui_open_page(UI::Page *page);
void ui_open_menu(UI::Menu *menu, bool save_history = true);
void ui_close_menu(void);
void ui_toast(const char *msg, unsigned long delay_ms, bool allow_clear);
void ui_clear_toast(bool rerender);

void ui_tick(void);
void ui_handle_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt);
void ui_handle_encoder(int difference);
void ui_rerender(void);
void ui_render_static(u8g2_t* display);

bool ui_has_menu_open(void);
bool ui_has_input_open(void);

#endif