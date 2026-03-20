#include <Arduino.h>

#include "board_config.h"
#include "meshdata.h"
#include "encoder.h"
#include "radio.h"

static OLEDDisplay* display;

int table_offset = 0;
const int table_height = 5;

void paintTable() {
  int offset = table_offset;
  const int height = table_height;
  int count = receivedCount();
  int len = min(height, count - offset);
  if (len <= 0) {
    display->drawString(3, 0, "(No entries)");
    return;
  }
  for (int i = 0; i < len; i++) {
    int y = i * 13;
    const Received& r = receivedAt(offset + i);
    display->drawString(3, y, String(r.rssi, 0));
    display->drawString(37, y, String(r.snr, 1));
    display->drawString(70, y, String(r.err, 1));
    display->drawString(110, y, (r.bad ? "!" : "") + String(r.header.hops));
    if (offset + i > 0 && r.header.isSame(receivedAt(offset + i - 1).header)) {
        display->drawLine(0, max(0, y - 6), 0, y + 6);
    }
  }
  if (offset + height < count &&
      receivedAt(offset + height).header.isSame(receivedAt(offset + height - 1).header)) {
    int y = height * 13;
    display->drawLine(0, y - 6, 0, y);
  }
  if (count > height) {
    int scaled_size = 13 * height;
    int mark_size = height * scaled_size / count;
    int mark_pos = offset * scaled_size / count;
    display->drawLine(127, mark_pos, 127, mark_pos + mark_size);
  }
}

void setup() {
  Serial.begin(9600);
  setupEncoder();
  setupRadio();

  display = createDisplay();
  display->init();
  display->clear();
  display->display();

  display->setContrast(255);
  display->flipScreenVertically();
  display->setFont(ArialMT_Plain_10);
}

bool invalidateScreen = true;

void invalidate() {
  if (!invalidateScreen) return;
  display->clear();
  paintTable();
  display->display();
}

uint16_t lastEncoder = 0;
void checkEncoder() {
  uint16_t current = encoderRead();
  int16_t delta = int16_t(current - lastEncoder);
  if (abs(delta) < 2) return;
  lastEncoder = current;
  invalidateScreen = true;
  int count = receivedCount();
  if (delta < -10) {
    table_offset = 0;
  } else if (delta > 10) {
    table_offset = count - table_height;
  } else {
    table_offset = min(max(0, count - table_height), max(0, table_offset + delta / 2));
  }
}

void loop() {
  checkEncoder();
  if (checkRadio()) {
    int count = receivedCount();
    if (count > table_height && count - table_height - 1 <= table_offset) {
      table_offset = count - table_height;
    }
    invalidateScreen = true;
  }

  if (invalidateScreen) {
    invalidate();
    invalidateScreen = false;
  }
}
