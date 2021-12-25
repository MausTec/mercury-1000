#include "ui.hpp"
#include "menus.hpp"
#include "pressure_manager.hpp"
#include "websocket_manager.h"
#include "wifi_manager.h"
#include "update_manager.h"
#include "config.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
    ui_toast("Disconnection failed.", 0, UI_TOAST_APPEND | UI_TOAST_ANYKEY);
    p->reenter();
}

static void on_maus_view_key(Menu *p, void *ptr, int i) {
    if (websocket_manager_get_status() == WM_REMOTE_CONNECTED) {
        const char *device_kay = websocket_manager_get_remote_device_key();
        // TODO: Show remote URL here as well if custom?
        ui_toastf("Connected to Maus-Link.\nDevice Key: %s...", 0, UI_TOAST_ANYKEY, device_kay);
    } else {
        ui_toast("Unable to connect to Maus-Link:\n%s...", 0, UI_TOAST_ANYKEY);
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

static void on_wifi_connect(Menu *p, void *ptr, int i) {
    ui_toast("Connecting to WiFi...", 0, UI_TOAST_PERMANENT);
    Config.wifi_on = true;
    esp_err_t err = wifi_manager_connect_to_ap(Config.wifi_ssid, Config.wifi_key);

    if (err == ESP_OK) {
        int attempts = 0;
        while (wifi_manager_get_status() != WIFI_MANAGER_CONNECTED) {
            if (attempts++ > 100) {
                goto wifi_conn_err;
            }

            vTaskDelay(100 * portTICK_PERIOD_MS);
        }

        ui_toast("Connected!", 0, UI_TOAST_ANYKEY | UI_TOAST_APPEND);
        config_save_to_nvfs(CONFIG_DEFAULT_FILE, &Config);
        p->reenter();
        return;
    }

wifi_conn_err:
    ui_toast("Error connecting to WiFi.", 0, UI_TOAST_ANYKEY | UI_TOAST_APPEND);
}

static void on_wifi_disconnect(Menu *p, void *ptr, int i) {

}

static void on_update_check(Menu *p, void *ptr, int i) {
    ui_toast("Checking for updates...", 0, UI_TOAST_PERMANENT);
    um_update_status_t status = update_manager_check_for_updates();

    if (status == UM_UPDATE_IS_CURRENT) {
        ui_toast("Device is up to date", 0, UI_TOAST_APPEND | UI_TOAST_ANYKEY);
    } else if (status == UM_UPDATE_AVAILABLE) {
        ui_toast("Device updates available", 0, UI_TOAST_APPEND | UI_TOAST_ANYKEY);
        p->reenter();
    } else {
        ui_toast("Device update possible?", 0, UI_TOAST_APPEND | UI_TOAST_ANYKEY);
        p->reenter();
    }
}

static void on_update_install(Menu *p, void *ptr, int i) {
    ui_toast("Installing updates...", 0, UI_TOAST_PERMANENT);
    esp_err_t err = update_manager_update_from_web();

    if (err == ESP_OK) {
        ui_toast("\nUpdates complete.\nRestarting...", 0, UI_TOAST_PERMANENT | UI_TOAST_APPEND);
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        esp_restart();
        for (;;) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } else {
        ui_toast("Installing updates failed.", 0, UI_TOAST_APPEND | UI_TOAST_ANYKEY);
    }
}

static void on_system_info(Menu *p, void *ptr, int i) {
    ui_toastf("Mercury 1000\nFirmware: %s\nSerial: %s", 0, UI_TOAST_ANYKEY, "-", "-");
}

static void on_enter(Menu *p) {
    if (pressure_manager_get_seek_status() == PM_SEEK_DISABLED) {
        p->add_item("Set Hold Point", &on_set_hold_point, nullptr, PM_SEEK_AT_SET_POINT);
    } else {
        p->add_item("Clear Hold Point", &on_set_hold_point, nullptr, PM_SEEK_DISABLED);
    }

    if (wifi_manager_get_status() == WIFI_MANAGER_CONNECTED) {
        // p->add_item("Disconnect from WiFi", &on_wifi_disconnect, nullptr, 0);

        if (websocket_manager_get_status() == WM_REMOTE_CONNECTED) {
            p->add_item("Disconnect Maus-Link Node", &on_maus_disconnect, nullptr, 0);
            p->add_item("View Maus-Link Device Key", &on_maus_view_key, nullptr, 0);
        } else {
            p->add_item("Connect to Maus-Link Server", &on_maus_connect, nullptr, 0);
            // TODO: Add support for custom remote server lists.
            //p->add_item("Connect to Maus-Link [DEV]", &on_maus_connect, nullptr, 1);
        }

        p->add_item("Check for Updates", &on_update_check, nullptr, 0);

        if (update_manager_get_status() == UM_UPDATE_AVAILABLE) {
            p->add_item("Install Firmware Updates", &on_update_install, nullptr, 0);
        }
    } else {
        if (Config.wifi_ssid[0] != '\0' && Config.wifi_key[0] != '\0') {
            p->add_item("Connect to saved WiFi", &on_wifi_connect, nullptr, 0);
        }
    }
    
    //p->add_item("Pressure Manager Configuration", &on_pressure_mgr_cfg, nullptr, 0);
    //p->add_item("Accessory Port", &on_accessory, nullptr, 0);

    p->add_item("System Information", &on_system_info, nullptr, 0);
}

static menu_config_t config = {
    .enter_cb = &on_enter,
    .exit_cb = nullptr,
};

Menu Menus::MainMenu("Main Menu", &config);