#include "ui.hpp"
#include "menus.hpp"
#include "pressure_manager.hpp"
#include "websocket_manager.h"

#include <stdio.h>

using namespace UI;

static const char *_link_srv[] = {
    "link.maustec.net",
    "192.168.1.3"
};

static const int _link_port[] = {
    443,
    8080
};

static void on_set_hold_point(Menu *p, void *ptr, int i) {
    if (i == PM_SEEK_DISABLED) {
        pressure_manager_clear_target_mean();
    } else {
        double mean = pressure_manager_get_mean();
        pressure_manager_set_target_mean(mean);
    }

    p->reenter();
}

static void on_maus_disconnect(Menu *p, void *ptr, int i) {
    ui_toast("Disconnecting from Maus-Link Server...", 0, UI_TOAST_PERMANENT);
    // websocket_manager_disconnect_remote();
    p->reenter();
}

static void on_maus_view_key(Menu *p, void *ptr, int i) {
    if (websocket_manager_get_status() == WM_REMOTE_CONNECTED) {
        const char *device_kay = websocket_manager_get_remote_device_key();
        // TODO: Show remote URL here as well if custom?
        ui_toastf("Connected to Maus-Link.\nDevice Key: %s\nPress any key to continue...", 0, UI_TOAST_NOFLAG, device_kay);
    } else {
        ui_toast("Unable to connect to Maus-Link:\n%s\nPress any key to continue...", 0, UI_TOAST_NOFLAG);
    }
}

static void on_maus_connect(Menu *p, void *ptr, int i) {
    ui_toast("Connecting to server...", 0, UI_TOAST_PERMANENT);
    websocket_manager_connect_to_remote(_link_srv[i], _link_port[i]);

    wm_remote_conn_status_t status = websocket_manager_wait_for_status(WM_REMOTE_WAITING_FOR_KEY | WM_REMOTE_CONNECTED, 10000);

    if (status == WM_REMOTE_WAITING_FOR_KEY) {
        ui_toast("Connecting to server...\nGetting device key...", 0, UI_TOAST_PERMANENT);
        websocket_manager_wait_for_status(WM_REMOTE_CONNECTED, 10000);
    }

    p->reenter();
    on_maus_view_key(p, ptr, i);
}

static void on_system_info(Menu *p, void *ptr, int i) {
    printf("Entered System Information: %d\n", i);
    ui_toastf("Mercury 1000\nFirmware: %s\nSerial: %s\nPress any key to continue", 0, UI_TOAST_NOFLAG, "-", "-");
}

static void on_enter(Menu *p) {
    if (pressure_manager_get_seek_status() == PM_SEEK_DISABLED) {
        p->add_item("Set Hold Point", &on_set_hold_point, nullptr, PM_SEEK_AT_SET_POINT);
    } else {
        p->add_item("Clear Hold Point", &on_set_hold_point, nullptr, PM_SEEK_DISABLED);
    }

    if (websocket_manager_get_status() == WM_REMOTE_CONNECTED) {
        p->add_item("Disconnect Maus-Link Node", &on_maus_disconnect, nullptr, 0);
        p->add_item("View Maus-Link Device Key", &on_maus_view_key, nullptr, 0);
    } else {
        p->add_item("Connect to Maus-Link Server", &on_maus_connect, nullptr, 0);
        // TODO: Add support for custom remote server lists.
        //p->add_item("Connect to Maus-Link [DEV]", &on_maus_connect, nullptr, 1);
    }
    
    //p->add_item("Pressure Manager Configuration", &on_pressure_mgr_cfg, nullptr, 0);
    //p->add_item("Accessory Port", &on_accessory, nullptr, 0);

    p->add_item("System Information", &on_system_info, nullptr, 0);
    //p->add_item("Check for Updates", &on_system_info, nullptr, 0);
}

static menu_config_t config = {
    .enter_cb = &on_enter,
    .exit_cb = nullptr,
};

Menu Menus::MainMenu("Main Menu", &config);