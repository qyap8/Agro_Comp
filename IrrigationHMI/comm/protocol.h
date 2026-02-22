#pragma once

#include <Arduino.h>
#include <vector>

namespace protocol {

uint16_t crc16_modbus(const uint8_t *data, size_t len);
std::vector<uint8_t> buildFrame(uint8_t addr, uint8_t cmd, const uint8_t *payload, size_t len);
bool validateFrame(const uint8_t *frame, size_t len);

} // namespace protocol
