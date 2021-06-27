#include <Arduino.h>
#include "m1k-hal.hpp"
#include "m1k-hal-strings.hpp"

void btn_press(m1k_hal_button_t button, bool is_hold);

void setup() {
  Serial.begin(115200);

  m1k_hal_init();

  m1k_hal_register_button_hold(M1K_HAL_BUTTON_MENU, btn_press);
  m1k_hal_register_button_press(M1K_HAL_BUTTON_MENU, btn_press);

  Serial.println("Maus-Tec Electronics Presents:");
  Serial.println("Mercury 1000 HAL Test Firmware");
  Serial.print("m1k-hal version: ");
  Serial.println(m1k_hal_get_version());
}

void loop() {
  // put your main code here, to run repeatedly:
}

void btn_press(m1k_hal_button_t button, bool is_hold) {
  Serial.print(is_hold ? "HOLD " : "PRESS ");
  Serial.println(m1k_hal_button_str[button]);
}