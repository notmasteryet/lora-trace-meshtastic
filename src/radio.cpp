#include <Arduino.h>
#include <RadioLib.h>
#include "board_config.h"
#include "meshdata.h"
#include "radio.h"

static SX1262 radio = new Module(RADIO_CS, RADIO_IRQ, RADIO_RST, RADIO_BUSY);

static volatile bool receivedFlag = false;

void IRAM_ATTR setFlag() {
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

bool checkRadio() {
  if (receivedFlag) {
    receivedFlag = false;

    byte buf[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    int numBytes = radio.getPacketLength();
    int state = radio.readData(buf, numBytes);
    unsigned long now = millis();

    if (state == RADIOLIB_ERR_NONE && numBytes >= sizeof(MeshHeader)) {
      MeshHeader* header = reinterpret_cast<MeshHeader*>(buf);
      float rssi = radio.getRSSI();
      float snr  = radio.getSNR();
      float err  = radio.getFrequencyError();

      insertReceived(*header, rssi, snr, err, now);

      Serial.print(F("Packet:\t\t"));
      Serial.printf("%08x->%08x (%08x|%02x) %d/%d\n", header->from, header->dest, header->id, header->hash, header->hops, header->orig_hops);
      Serial.print(F("\t\tRSSI:\t")); Serial.print(rssi); Serial.print(F(" dBm "));
      Serial.print(F("\tSNR:\t"));   Serial.print(snr);  Serial.print(F(" dB"));
      Serial.print(F("\tErr:\t"));   Serial.print(err);  Serial.println(F(" Hz"));
      return true;
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      float rssi = radio.getRSSI();
      float snr  = radio.getSNR();
      float err  = radio.getFrequencyError();
      insertReceived(MeshHeader{}, rssi, snr, err, now, true);
      Serial.print(F("CRC error\t"));
      Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.print(F(" dBm "));
      Serial.print(F("\tSNR: "));  Serial.print(snr);  Serial.print(F(" dB"));
      Serial.print(F("\tErr: "));  Serial.print(err);  Serial.println(F(" Hz"));
      return true;
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
  }
  return false;
}
