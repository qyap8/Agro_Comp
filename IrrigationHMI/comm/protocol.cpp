#include "protocol.h"

namespace protocol {

uint16_t crc16_modbus(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

std::vector<uint8_t> buildFrame(uint8_t addr, uint8_t cmd, const uint8_t *payload, size_t len) {
    std::vector<uint8_t> frame;
    frame.reserve(len + 4);
    frame.push_back(addr);
    frame.push_back(cmd);
    for (size_t i = 0; i < len; ++i) {
        frame.push_back(payload[i]);
    }
    uint16_t crc = crc16_modbus(frame.data(), frame.size());
    frame.push_back(crc & 0xFF);
    frame.push_back((crc >> 8) & 0xFF);
    return frame;
}

bool validateFrame(const uint8_t *frame, size_t len) {
    if (len < 4) {
        return false;
    }
    uint16_t expected = (static_cast<uint16_t>(frame[len - 1]) << 8) | frame[len - 2];
    uint16_t actual = crc16_modbus(frame, len - 2);
    return expected == actual;
}

} // namespace protocol
