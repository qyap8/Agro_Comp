#include "protocol.h"

namespace comm {

uint16_t crc16_modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x0001) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
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
  out.push_back(static_cast<uint8_t>(f.cmd));
  out.push_back(static_cast<uint8_t>(f.payload.size()));
  out.insert(out.end(), f.payload.begin(), f.payload.end());
  uint16_t crc = crc16_modbus(out.data() + 2, out.size() - 2);
  out.push_back(crc & 0xFF);
  out.push_back((crc >> 8) & 0xFF);
  return out;
}

bool decodeFrame(const uint8_t* data, size_t len, Frame& out) {
  if (len < 8 || data[0] != 0xAA || data[1] != 0x55) return false;
  const uint8_t payloadLen = data[5];
  if (len < (size_t)(8 + payloadLen)) return false;

  uint16_t got = data[6 + payloadLen] | (data[7 + payloadLen] << 8);
  uint16_t calc = crc16_modbus(data + 2, 4 + payloadLen);
  if (got != calc) return false;

  out.dst = data[2];
  out.src = data[3];
  out.cmd = static_cast<Cmd>(data[4]);
  out.payload.assign(data + 6, data + 6 + payloadLen);
  return true;
}

}  // namespace comm
