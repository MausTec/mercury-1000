#include "ui.hpp"
#include "menus.hpp"

#include <stdio.h>

using namespace UI;

static void on_system_info(Menu *p, void *ptr, int i) {
    printf("Entered System Information: %d\n", i);
}

static void on_enter(Menu *p) {
    p->add_item("System Information", &on_system_info, nullptr, 0x69);
    p->add_item("Check for Updates", &on_system_info, nullptr, 0x69);
    p->add_item("WiFi On", &on_system_info, nullptr, 0x69);
    p->add_item("WiFi Off", &on_system_info, nullptr, 0x69);
    p->add_item("Give A Blowjob", &on_system_info, nullptr, 0x69);
}

static menu_config_t config = {
    .enter_cb = &on_enter,
    .exit_cb = nullptr,
};

Menu Menus::MainMenu("Main Menu", &config);