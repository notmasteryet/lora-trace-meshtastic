#pragma once

#include <Arduino.h>
#include "board_config.h"

inline uint8_t getEncoderPos() {
  return (digitalRead(ENC_A_PIN) ? 3 : 0) ^ (digitalRead(ENC_B_PIN) ? 1 : 0);
}
volatile uint8_t encoderPos;

volatile uint16_t encoderValue = 0;

void IRAM_ATTR encoderISR() {
  uint8_t pos = getEncoderPos();
  switch ((pos - encoderPos) & 3) {
    case 0: return;
    case 1: encoderValue++; break;
    case 3: encoderValue--; break;
  }
  encoderPos = pos;
}

void setupEncoder() {
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  encoderPos = getEncoderPos();

  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B_PIN), encoderISR, CHANGE);
}
