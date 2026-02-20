#pragma once

#include <Arduino.h>

#include "protocol.h"

namespace comm {

struct Rs485Config {
  uint8_t uartNum = 2;
  int txPin = -1;
  int rxPin = -1;
  int dePin = -1;
  uint32_t baud = 115200;
  bool mockMode = true;
  uint8_t localAddr = 0x01;
};

class Rs485Transport {
 public:
  bool begin(const Rs485Config& cfg);
  bool sendFrame(const Frame& frame);
  bool readFrame(Frame& out, uint32_t timeoutMs);
  bool request(const Frame& req, Frame* rsp, uint8_t retries = 2, uint32_t timeoutMs = 120);
  void loop();

 private:
  void setDir(bool tx);
  bool mockReply(const Frame& req, Frame& rsp);

  HardwareSerial* serial_ = nullptr;
  Rs485Config cfg_;
};

}  // namespace comm
