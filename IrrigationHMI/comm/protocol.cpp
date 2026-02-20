#include "protocol.h"

namespace comm {

uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
    }
  }
  return crc;
}

std::vector<uint8_t> encodeFrame(const Frame& f) {
  std::vector<uint8_t> out;
  out.reserve(8 + f.payload.size());
  out.push_back(0xAA);
  out.push_back(0x55);
  out.push_back(f.dst);
  out.push_back(f.src);
  out.push_back((uint8_t)f.cmd);
  out.push_back((uint8_t)f.payload.size());
  out.insert(out.end(), f.payload.begin(), f.payload.end());
  uint16_t crc = crc16_modbus(out.data() + 2, out.size() - 2);
  out.push_back((uint8_t)(crc & 0xFF));
  out.push_back((uint8_t)(crc >> 8));
  return out;
}

bool decodeFrame(const uint8_t* data, size_t len, Frame& out) {
  if (len < 8) return false;
  if (data[0] != 0xAA || data[1] != 0x55) return false;

  uint8_t payLen = data[5];
  if (len < (size_t)(8 + payLen)) return false;

  uint16_t got = data[6 + payLen] | (data[7 + payLen] << 8);
  uint16_t calc = crc16_modbus(data + 2, 4 + payLen);
  if (got != calc) return false;

  out.dst = data[2];
  out.src = data[3];
  out.cmd = (Cmd)data[4];
  out.payload.assign(data + 6, data + 6 + payLen);
  return true;
}

}  // namespace comm
