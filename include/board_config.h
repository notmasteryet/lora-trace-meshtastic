#pragma once

#include <OLEDDisplay.h>

// ─── Heltec WiFi LoRa 32 V3 (ESP32-S3 + SX1262) ────────────────────────────
#if defined(BOARD_HELTEC_V3)

  // SX1262 radio pins
  #define RADIO_CS    8
  #define RADIO_IRQ   14
  #define RADIO_RST   12
  #define RADIO_BUSY  13

  // Rotary encoder pins
  #define ENC_A_PIN   47
  #define ENC_B_PIN   48

// ─── Add new boards here ─────────────────────────────────────────────────────
// #elif defined(BOARD_TTGO_LORA32_V1)
//   ...

#else
  #error "No board defined. Add -DBOARD_<NAME> to build_flags in platformio.ini."
#endif

OLEDDisplay* createDisplay();
