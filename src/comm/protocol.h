#pragma once

#include <Arduino.h>
#include <vector>

namespace comm {

enum class Cmd : uint8_t {
  DISCOVER = 0x01,
  HELLO = 0x02,
  ASSIGN_ADDR = 0x03,
  VALVE_PULSE = 0x10,
  CLOSE_ALL = 0x11,
  PUMP_RELAY_SET = 0x12,
  PUMP_DC_SET = 0x13,
  STATUS_GET = 0x20,
  IDENTIFY = 0x21,
};

struct Frame {
  uint8_t dst = 0;
  uint8_t src = 0;
  Cmd cmd = Cmd::DISCOVER;
  std::vector<uint8_t> payload;
};

uint16_t crc16_modbus(const uint8_t* data, size_t len);
std::vector<uint8_t> encodeFrame(const Frame& f);
bool decodeFrame(const uint8_t* data, size_t len, Frame& out);

}  // namespace comm
