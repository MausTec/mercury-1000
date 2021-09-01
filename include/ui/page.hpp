#ifndef __ui_page_hpp
#define __ui_page_hpp

#include "m1k-hal.hpp"

namespace UI {
    class Page;

    typedef void (*page_event_cb)(Page *p);
    typedef void (*page_render_cb)(m1k_hal_display_t *display, Page *p);
    typedef void (*page_encoder_cb)(int difference, Page *p);
    typedef void (*page_button_cb)(m1k_hal_button_t button, m1k_hal_button_evt_t evt, Page *p);

    struct page_config {
        page_event_cb enter_cb;
        page_render_cb render_cb;
        page_event_cb  loop_cb;
        page_event_cb exit_cb;
        page_button_cb button_cb;
        page_encoder_cb encoder_cb;
    };

    typedef struct page_config page_config_t;

    /**
     * This literally just delegates calls to the struct. I could almost certainly just
     * remove this whole thing and use a struct directly. I'm searching for syntax sugar.
     */
    class Page {
        public:
            Page(page_config_t *cfg) : config(cfg) {};

            void enter(void);
            void render(void);
            void loop(void);
            void exit(void);
            void on_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt);
            void on_encoder(int);

        private:
            page_config_t *config;
    };
}

#endif