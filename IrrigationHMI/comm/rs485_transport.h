#pragma once

#include <Arduino.h>
#include <vector>
#include "../config.h"
#include "../core/state.h"

class RS485Transport {
public:
    bool begin(uint32_t baudrate = APP_RS485_BAUD_DEFAULT);
    bool sendFrame(const std::vector<uint8_t> &frame);
    bool readFrame(std::vector<uint8_t> &out, uint32_t timeoutMs = 30);
    app::CommStats &stats();

private:
    HardwareSerial serial_{2};
    app::CommStats stats_{};
};
