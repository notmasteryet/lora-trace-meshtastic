#pragma once

// ─── Heltec WiFi LoRa 32 V3 (ESP32-S3 + SX1262) ────────────────────────────
#if defined(BOARD_HELTEC_V3)

  #include <OLEDDisplay.h>
  #include <SSD1306Wire.h>

  // SX1262 radio pins
  #define RADIO_CS    8
  #define RADIO_IRQ   14
  #define RADIO_RST   12
  #define RADIO_BUSY  13

  // Rotary encoder pins
  #define ENC_A_PIN   47
  #define ENC_B_PIN   48

  // SDA_OLED, SCL_OLED, RST_OLED, Vext are defined by the board's pins_arduino.h
  inline OLEDDisplay* createDisplay() {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    pinMode(RST_OLED, OUTPUT);
    digitalWrite(RST_OLED, LOW);
    delay(20);
    digitalWrite(RST_OLED, HIGH);
    delay(100);
    return new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, GEOMETRY_128_64, I2C_ONE, 500000);
  }

// ─── Add new boards here ─────────────────────────────────────────────────────
// #elif defined(BOARD_TTGO_LORA32_V1)
//   ...

#else
  #error "No board defined. Add -DBOARD_<NAME> to build_flags in platformio.ini."
#endif
