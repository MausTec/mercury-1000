#ifndef __ui_menu_hpp
#define __ui_menu_hpp

#include "m1k-hal.hpp"

#define MENU_TITLE_MAXLEN 40

namespace UI {
    class Menu;

    typedef void (*menu_event_cb)(Menu* p);
    typedef void (*menu_item_cb)(Menu* p, void* ptr_arg, int i_arg);

    struct menu_config {
        menu_event_cb enter_cb;
        menu_event_cb exit_cb;
    };

    typedef struct menu_config menu_config_t;

    struct menu_item {
        char name[MENU_TITLE_MAXLEN + 1];
        menu_item_cb cb;
        void* ptr_arg;
        int i_arg;

        struct menu_item* _next = nullptr;
        struct menu_item* _prev = nullptr;
    };

    typedef struct menu_item menu_item_t;

    class Menu {
    public:
        Menu(const char *title, menu_config_t* cfg);

        void enter(Menu* previous = nullptr, bool save_history = true);
        void render(void);
        void reenter(void);
        void loop(void);
        void exit(void);
        void on_click(m1k_hal_button_t button, m1k_hal_button_evt_t evt);
        void on_encoder(int);

        // These are used inside the lifecycle:
        void clear_items(void);
        void add_item(const char* name, menu_item_cb cb, void* ptr_arg = nullptr, int i_arg = 0);
        size_t count_items(void);
        size_t current_index(void);
        bool has_items(void);

    protected:

        void select_next(int dist = 1);
        void select_prev(int dist = 1);
        void confirm_selection(void);

    private:
        char title[MENU_TITLE_MAXLEN + 1] = "";
        menu_config_t* config;
        Menu* previous_menu = nullptr;

        menu_item_t* first;
        menu_item_t* last;
        menu_item_t* current;
    };
}

#endif