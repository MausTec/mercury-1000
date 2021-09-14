#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"
#include "ui.hpp"
#include "pages.hpp"
#include "pressure_manager.hpp"
#include "tscode_manager.h"
#include "esp_task_wdt.h"
#include "wifi_manager.h"

TaskHandle_t PressureMgrTask;
void pressure_mgr_task(void *param);

void setup() {
  m1k_hal_init();
  ui_init();
  tscode_manager_init();
  wifi_manager_init();

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
  printf("m1k-hal version: %s\n", m1k_hal_get_version());

  ui_open_page(&Pages::Splash);
}

void loop() {
  m1k_hal_tick();
  ui_tick();
  tscode_manager_tick();
}

extern "C" {
  void app_main(void) {
    setup();
    for (;;) {
      loop();
      vTaskDelay(1);
    }
  }
}

void pressure_mgr_task(void *param) {
  for(;;) {
    pressure_manager_tick();
    vTaskDelay(1);
  }
}