#include <cstring>
#include "meshdata.h"

static int receivedBufferLen = 0;
static Received receivedBuffer[ReceivedBufferLen];

int receivedCount() {
  return receivedBufferLen;
}

const Received& receivedAt(int index) {
  return receivedBuffer[index];
}

// Prepare a slot at `index`, shifting existing entries if needed.
// Returns a reference to the slot, pre-filled with signal data.
static Received& prepSlot(int index, float rssi, float snr, float err, unsigned long now, bool bad) {
  Received& r = receivedBuffer[index];
  r.time = now;
  r.rssi = rssi;
  r.snr  = snr;
  r.err  = err;
  r.bad  = bad;
  return r;
}

static int allocSlot() {
  if (receivedBufferLen + 1 >= ReceivedBufferLen) {
    const int ChunkToRemove = 10;
    receivedBufferLen -= ChunkToRemove;
    memmove(receivedBuffer, receivedBuffer + ChunkToRemove, sizeof(Received) * receivedBufferLen);
  }
  return receivedBufferLen++;
}

void insertReceived(const MeshHeader& header, float rssi, float snr, float err, unsigned long now, bool bad) {
  int index = allocSlot();

  if (!bad) {
    for (int i = index; i > 0; --i) {
      if ((now - receivedBuffer[i - 1].time) > 5000) break;
      if (header.isSame(receivedBuffer[i - 1].header)) {
        if (index > i) {
          memmove(&receivedBuffer[i + 1], &receivedBuffer[i], (index - i) * sizeof(Received));
          index = i;
        }
        break;
      }
    }
  }

  Received& r = prepSlot(index, rssi, snr, err, now, bad);
  r.header = header;
}

void insertReceived(const MCHeader& mc, float rssi, float snr, float err, unsigned long now, bool bad) {
  int index = allocSlot();

  if (!bad) {
    for (int i = index; i > 0; --i) {
      if ((now - receivedBuffer[i - 1].time) > 5000) break;
      if (mc.isSame(receivedBuffer[i - 1].mc)) {
        if (index > i) {
          memmove(&receivedBuffer[i + 1], &receivedBuffer[i], (index - i) * sizeof(Received));
          index = i;
        }
        break;
      }
    }
  }

  Received& r = prepSlot(index, rssi, snr, err, now, bad);
  r.mc = mc;
}
