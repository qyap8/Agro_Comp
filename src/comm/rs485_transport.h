#pragma once

#include <Arduino.h>
#include <vector>

#include "protocol.h"

namespace comm {

struct Rs485Config {
  uint8_t uartNum = 2;
  int txPin = 17;
  int rxPin = 18;
  int dePin = -1;  // -1 means auto direction hardware
  uint32_t baud = 115200;
  bool mockMode = true;
  uint8_t localAddr = 0x01;
};

class Rs485Transport {
 public:
  bool begin(const Rs485Config& cfg);
  bool sendWithRetry(const Frame& frame, Frame* response, uint8_t retries = 2, uint32_t timeoutMs = 120);
  bool readFrame(Frame& frame, uint32_t timeoutMs = 20);
  bool sendFrame(const Frame& frame);

  void loop();

 private:
  HardwareSerial* serial_ = nullptr;
  Rs485Config cfg_;

  bool mockHandle(const Frame& req, Frame& rsp);
  void setTxMode(bool tx);
};

}  // namespace comm
