#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"
#include "ui.hpp"
#include "pages.hpp"
#include "pressure_manager.hpp"
#include "tscode_manager.h"
#include "esp_task_wdt.h"
#include "version.h"
#include "config.hpp"

#include "wifi_manager.h"
#include "websocket_manager.h"

TaskHandle_t PressureMgrTask;
void pressure_mgr_task(void* param);

extern "C" void app_main() {
    m1k_hal_init();

    // Populate the global config struct with the default file:
    config_init();
    config_load_from_nvfs(CONFIG_DEFAULT_FILE, &Config);

    ui_init();
    tscode_manager_init();

    xTaskCreatePinnedToCore(
        pressure_mgr_task,
        "PressureMgrTask",
        4095,
        NULL,
        0,
        &PressureMgrTask,
        0
    );

    printf("Maus-Tec Electronics Presents:\n");
    printf("Mercury 1000\n");
    printf("Firmware version: %s\n", VERSION);
    printf("m1k-hal version: %s\n", m1k_hal_get_version());

    char config_str[512] = "";
    config_serialize(&Config, config_str, 512);
    printf("Config Loaded:\n%s\n", config_str);

    wifi_manager_init();
    wifi_manager_connect_to_ap("Here Be Dragons", "RAWR!barkbark");

    ui_open_page(&Pages::Splash);

    printf("Startup sequence complete.\n");

    for (;;) {
        m1k_hal_tick();
        ui_tick();
        tscode_manager_tick();

        vTaskDelay(1);
    }
}

void pressure_mgr_task(void* param) {
    for (;;) {
        pressure_manager_tick();
        vTaskDelay(1);
    }
}