#pragma once

#include <Arduino.h>

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

struct Received {
  MeshHeader header;
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
