#include <Arduino.h>
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"
#include "ui.hpp"
#include "pages.hpp"

void setup() {
  Serial.begin(115200);

  m1k_hal_init();
  ui_init();

  Serial.println("Maus-Tec Electronics Presents:");
  Serial.println("Mercury 1000");
  Serial.print("m1k-hal version: ");
  Serial.println(m1k_hal_get_version());

  ui_open_page(&Pages::Home);
}

void loop() {
  m1k_hal_tick();
  ui_tick();
}