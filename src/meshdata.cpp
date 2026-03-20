#include "meshdata.h"

static int receivedBufferLen = 0;
static Received receivedBuffer[ReceivedBufferLen];

int receivedCount() {
  return receivedBufferLen;
}

const Received& receivedAt(int index) {
  return receivedBuffer[index];
}

void insertReceived(const MeshHeader& header, float rssi, float snr, float err, unsigned long now, bool bad) {
  if (receivedBufferLen + 1 >= ReceivedBufferLen) {
    const int ChunkToRemove = 10;
    receivedBufferLen -= ChunkToRemove;
    memmove(receivedBuffer, receivedBuffer + ChunkToRemove, sizeof(Received) * receivedBufferLen);
  }
  int index = receivedBufferLen++;

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

  Received& r = receivedBuffer[index];
  r.header = header;
  r.time = now;
  r.rssi = rssi;
  r.snr = snr;
  r.err = err;
  r.bad = bad;
}
