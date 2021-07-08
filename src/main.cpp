#include <Arduino.h>
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"
#include "ui.hpp"
#include "pages.hpp"
#include "pressure_manager.hpp"
#include "esp_task_wdt.h"

TaskHandle_t PressureMgrTask;
void pressure_mgr_task(void *param);

void setup() {
  Serial.begin(256000);

  m1k_hal_init();
  ui_init();

  xTaskCreatePinnedToCore(
    pressure_mgr_task,
    "PressureMgrTask",
    4095,
    NULL,
    0,
    &PressureMgrTask,
    0
  );

  Serial.println("Maus-Tec Electronics Presents:");
  Serial.println("Mercury 1000");
  Serial.print("m1k-hal version: ");
  Serial.println(m1k_hal_get_version());

  ui_open_page(&Pages::Splash);
}

void loop() {
  m1k_hal_tick();
  ui_tick();
}

void pressure_mgr_task(void *param) {
  for(;;) {
    pressure_manager_tick();
    vTaskDelay(1);
  }
}