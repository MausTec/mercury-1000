#include "ui.hpp"
#include "menus.hpp"
#include "pressure_manager.hpp"

#include <stdio.h>

using namespace UI;

static void on_set_hold_point(Menu *p, void *ptr, int i) {
    if (i == PM_SEEK_DISABLED) {
        pressure_manager_clear_target_mean();
    } else {
        double mean = pressure_manager_get_mean();
        pressure_manager_set_target_mean(mean);
    }

    p->rerender();
}

static void on_system_info(Menu *p, void *ptr, int i) {
    printf("Entered System Information: %d\n", i);
}

static void on_enter(Menu *p) {
    if (pressure_manager_get_seek_status() == PM_SEEK_DISABLED) {
        p->add_item("Set Hold Point", &on_set_hold_point, nullptr, PM_SEEK_AT_SET_POINT);
    } else {
        p->add_item("Clear Hold Point", &on_set_hold_point, nullptr, PM_SEEK_DISABLED);
    }
    
    //p->add_item("Pressure Manager Configuration", &on_pressure_mgr_cfg, nullptr, NULL);
    //p->add_item("Accessory Port", &on_accessory, nullptr, NULL);

    //p->add_item("System Information", &on_system_info, nullptr, NULL);
    //p->add_item("Check for Updates", &on_system_info, nullptr, NULL);
}

static menu_config_t config = {
    .enter_cb = &on_enter,
    .exit_cb = nullptr,
};

Menu Menus::MainMenu("Main Menu", &config);