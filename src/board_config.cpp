#include <Arduino.h>
#include "board_config.h"

#if defined(BOARD_HELTEC_V3)

#include <SSD1306Wire.h>

// SDA_OLED, SCL_OLED, RST_OLED, Vext defined by the board's pins_arduino.h
OLEDDisplay* createDisplay() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW);
  delay(20);
  digitalWrite(RST_OLED, HIGH);
  delay(100);
  return new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, GEOMETRY_128_64, I2C_ONE, 500000);
}

#endif
