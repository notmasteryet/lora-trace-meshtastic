#pragma once

#include <RadioLib.h>
#include "board_config.h"
#include "meshdata.h"

SX1262 radio = new Module(RADIO_CS, RADIO_IRQ, RADIO_RST, RADIO_BUSY);

volatile bool receivedFlag = false;
ICACHE_RAM_ATTR
void setFlag(void) {
  receivedFlag = true;
}

void setupRadio() {
  Serial.print(F("Initializing ... "));
  int state = radio.begin(906.875f, 250.f, 11, 5, 0x2b, 22, 16);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  radio.setPacketReceivedAction(setFlag);

  Serial.print(F("Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
}

int allocReceived() {
  if (receivedBufferLen + 1 >= ReceivedBufferLen) {
    const int ChunkToRemove = 10;
    receivedBufferLen -= ChunkToRemove;
    memmove(receivedBuffer, receivedBuffer + ChunkToRemove, sizeof(Received) * receivedBufferLen);
  }
  return receivedBufferLen++;
}

bool checkRadio() {
  if (receivedFlag) {
    receivedFlag = false;

    byte buf[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    int numBytes = radio.getPacketLength();
    int state = radio.readData(buf, numBytes);
    unsigned long now = millis();

    if (state == RADIOLIB_ERR_NONE && numBytes >= sizeof(MeshHeader)) {
      MeshHeader* header = reinterpret_cast<MeshHeader*>(buf);
      int index = allocReceived();
      for (int i = index; i > 0; --i) {
        if ((now - receivedBuffer[i - 1].time) > 5000) break;
        if (header->isSame(receivedBuffer[i - 1].header)) {
          if (index > i) {
            memmove(&receivedBuffer[i + 1], &receivedBuffer[i], (index - i) * sizeof(Received));
            index = i;
          }
          break;
        }
      }
      Received& r = receivedBuffer[index];
      memcpy(&r.header, header, sizeof(MeshHeader));
      r.time = now;
      r.rssi = radio.getRSSI();
      r.snr = radio.getSNR();
      r.err = radio.getFrequencyError();

      Serial.print(F("Packet:\t\t"));
      Serial.printf("%08x->%08x (%08x|%02x) %d/%d\n", header->from, header->dest, header->id, header->hash, header->hops, header->orig_hops);

      Serial.print(F("\t\tRSSI:\t"));
      Serial.print(r.rssi);
      Serial.print(F(" dBm "));

      Serial.print(F("\tSNR:\t"));
      Serial.print(r.snr);
      Serial.print(F(" dB"));

      Serial.print(F("\tErr:\t"));
      Serial.print(r.err);
      Serial.println(F(" Hz"));
      return true;
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      Serial.println(F("CRC error!"));
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
  }
  return false;
}
