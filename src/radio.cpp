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

static uint16_t payloadHash(const uint8_t* data, int len) {
  uint16_t h = 0;
  for (int i = 0; i < len; i++)
    h = (uint16_t)((h << 5) ^ (h >> 11) ^ data[i]);
  return h;
}

void setupRadio() {
  Serial.print(F("Initializing ... "));
  int state;
  if (currentMode == MODE_MESHCORE) {
    // MeshCore US defaults: 910.525 MHz, BW=62.5 kHz, SF=7, CR=5, sync=0x12 (private)
    state = radio.begin(910.525f, 62.5f, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 22, 16);
  } else {
    // Meshtastic: 906.875 MHz, BW=250 kHz, SF=11, CR=5, sync=0x2B
    state = radio.begin(906.875f, 250.f, 11, 5, 0x2b, 22, 16);
  }
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
  if (!receivedFlag) return false;
  receivedFlag = false;

  byte buf[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
  int numBytes = radio.getPacketLength();
  int state = radio.readData(buf, numBytes);
  unsigned long now = millis();

  float rssi = radio.getRSSI();
  float snr  = radio.getSNR();
  float err  = radio.getFrequencyError();

  if (currentMode == MODE_MESHCORE) {
    if (state == RADIOLIB_ERR_NONE && numBytes >= 2) {
      uint8_t hdr = buf[0];
      bool has_tc = (hdr & 0x03) == 0x00 || (hdr & 0x03) == 0x03;
      int i = 1 + (has_tc ? 4 : 0);
      if (i < numBytes) {
        uint8_t pl = buf[i];
        uint8_t hash_size  = (pl >> 6) + 1;
        uint8_t hash_count = pl & 63;
        int path_bytes = hash_count * hash_size;
        int payload_start = i + 1 + path_bytes;

        MCHeader mc;
        mc.header       = hdr;
        mc.path_len     = pl;
        mc.payload_hash = payloadHash(buf + payload_start, numBytes - payload_start);

        insertReceived(mc, rssi, snr, err, now);

        Serial.printf("MC Packet:\t[%s] type=%s path=%dx%d payload=%d\n",
          has_tc ? "TC" : "FL",
          MCHeader::payloadTypeName(mc.payloadType()),
          hash_count, hash_size, numBytes - payload_start);
        Serial.printf("\t\tRSSI: %.0f dBm\tSNR: %.1f dB\tErr: %.1f Hz\n", rssi, snr, err);
        return true;
      }
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      insertReceived(MCHeader{}, rssi, snr, err, now, true);
      Serial.printf("MC CRC error\tRSSI: %.0f dBm\tSNR: %.1f dB\tErr: %.1f Hz\n", rssi, snr, err);
      return true;
    }
    Serial.print(F("MC failed, code ")); Serial.println(state);
    return false;
  }

  // Meshtastic
  if (state == RADIOLIB_ERR_NONE && numBytes >= (int)sizeof(MeshHeader)) {
    MeshHeader* header = reinterpret_cast<MeshHeader*>(buf);
    insertReceived(*header, rssi, snr, err, now);
    Serial.printf("Packet:\t\t%08x->%08x (%08x|%02x) %d/%d\n",
      header->from, header->dest, header->id, header->hash, header->hops, header->orig_hops);
    Serial.printf("\t\tRSSI: %.1f dBm\tSNR: %.1f dB\tErr: %.1f Hz\n", rssi, snr, err);
    return true;
  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    insertReceived(MeshHeader{}, rssi, snr, err, now, true);
    Serial.printf("CRC error\tRSSI: %.1f dBm\tSNR: %.1f dB\tErr: %.1f Hz\n", rssi, snr, err);
    return true;
  }
  Serial.print(F("failed, code ")); Serial.println(state);
  return false;
}
