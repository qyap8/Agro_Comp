#include "rs485_transport.h"
#include "protocol.h"

bool RS485Transport::begin(uint32_t baudrate) {
    serial_.begin(baudrate, SERIAL_8N1, APP_RS485_RX_PIN, APP_RS485_TX_PIN);
    return true;
}

bool RS485Transport::sendFrame(const std::vector<uint8_t> &frame) {
    if (frame.empty()) {
        return false;
    }
    size_t written = serial_.write(frame.data(), frame.size());
    serial_.flush();
    if (written == frame.size()) {
        stats_.txFrames++;
        return true;
    }
    return false;
}

bool RS485Transport::readFrame(std::vector<uint8_t> &out, uint32_t timeoutMs) {
    out.clear();
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        while (serial_.available()) {
            out.push_back(static_cast<uint8_t>(serial_.read()));
        }
        if (out.size() >= 4) {
            if (protocol::validateFrame(out.data(), out.size())) {
                stats_.rxFrames++;
                return true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (!out.empty()) {
        stats_.crcErrors++;
    } else {
        stats_.timeouts++;
    }
    return false;
}

app::CommStats &RS485Transport::stats() {
    return stats_;
}
