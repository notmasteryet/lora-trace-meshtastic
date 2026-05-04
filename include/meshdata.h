#pragma once

#include <stdint.h>

enum Mode { MODE_MESHTASTIC, MODE_MESHCORE };
extern Mode currentMode;

struct MeshHeader {
  uint32_t dest;
  uint32_t from;
  uint32_t id;
  union {
    uint8_t flags;
    struct {
      uint8_t hops : 3;
      uint8_t want_ack : 1;
      uint8_t mqtt : 1;
      uint8_t orig_hops : 3;
    };
  };
  uint8_t hash;
  uint16_t align;  // = 0

  bool operator==(const MeshHeader& other) const {
    return id == other.id && dest == other.dest && from == other.from && flags == other.flags && hash == other.hash;
  }

  bool isSame(const MeshHeader& other) const {
    return id == other.id && dest == other.dest && from == other.from && hash == other.hash;
  }
};

// MeshCore over-the-air header fields extracted for display and dedup
struct MCHeader {
  uint8_t header;
  uint8_t path_len;
  uint16_t payload_hash;  // simple hash of payload bytes for dedup

  uint8_t routeType() const { return header & 0x03; }
  uint8_t payloadType() const { return (header >> 2) & 0x0F; }

  bool isSame(const MCHeader& o) const {
    return payloadType() == o.payloadType() && payload_hash == o.payload_hash;
  }

  static const char* payloadTypeName(uint8_t t) {
    switch (t) {
      case 0x00: return "RQ";
      case 0x01: return "RS";
      case 0x02: return "TX";
      case 0x03: return "AK";
      case 0x04: return "AD";
      case 0x05: return "GR";
      case 0x06: return "DA";
      case 0x07: return "AN";
      case 0x08: return "PT";
      case 0x09: return "TR";
      case 0x0A: return "MP";
      case 0x0B: return "CT";
      case 0x0F: return "RW";
      default:   return "??";
    }
  }
};

struct Received {
  union {
    MeshHeader header;  // MODE_MESHTASTIC
    MCHeader mc;        // MODE_MESHCORE
  };
  unsigned long time;
  float rssi;
  float snr;
  float err;
  bool bad;
};

constexpr int ReceivedBufferLen = 128;

int receivedCount();
const Received& receivedAt(int index);
void insertReceived(const MeshHeader& header, float rssi, float snr, float err, unsigned long now, bool bad = false);
void insertReceived(const MCHeader& mc, float rssi, float snr, float err, unsigned long now, bool bad = false);
