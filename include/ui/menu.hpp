#ifndef __ui_menu_hpp
#define __ui_menu_hpp

#include "m1k-hal.hpp"

namespace UI {
    class Menu;

    typedef void (*menu_event_cb)(Menu *p);
    typedef void (*menu_render_cb)(m1k_hal_display_t *display, Menu *p);
    typedef void (*menu_encoder_cb)(int difference, Menu *p);
    typedef void (*menu_button_cb)(m1k_hal_button_t button, m1k_hal_button_evt_t evt, Menu *p);

    struct menu_config {
        menu_event_cb enter_cb;
        menu_render_cb render_cb;
        menu_event_cb  loop_cb;
        menu_event_cb exit_cb;
        menu_button_cb button_cb;
        menu_encoder_cb encoder_cb;
    };

    typedef struct menu_config menu_config_t;

    /**
     * This literally just delegates calls to the struct. I could almost certainly just
     * remove this whole thing and use a struct directly. I'm searching for syntax sugar.
     */
    class Menu {
        public:
            Menu(menu_config_t *cfg) : config(cfg) {};

            void enter(void);
            void render(void);
            void loop(void);
            void exit(void);
            void on_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt);
            void on_encoder(int);

        private:
            menu_config_t *config;
    };
}

#endif