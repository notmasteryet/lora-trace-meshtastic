#include <Arduino.h>
#include "board_config.h"
#include "encoder.h"

static volatile uint8_t encoderPos;
static volatile uint16_t encoderValue = 0;

uint16_t encoderRead() {
  return encoderValue;
}

static inline uint8_t getEncoderPos() {
  return (digitalRead(ENC_A_PIN) ? 3 : 0) ^ (digitalRead(ENC_B_PIN) ? 1 : 0);
}

void IRAM_ATTR encoderISR() {
  uint8_t pos = getEncoderPos();
  switch ((pos - encoderPos) & 3) {
    case 0: return;
    case 1: encoderValue = encoderValue + 1; break;
    case 3: encoderValue = encoderValue - 1; break;
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
