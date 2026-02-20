#include "rs485_transport.h"

namespace comm {

bool Rs485Transport::begin(const Rs485Config& cfg) {
  cfg_ = cfg;
  if (cfg_.uartNum == 1) {
    serial_ = &Serial1;
  } else {
    serial_ = &Serial2;
  }

  if (cfg_.dePin >= 0) {
    pinMode(cfg_.dePin, OUTPUT);
    digitalWrite(cfg_.dePin, LOW);
  }

  if (!cfg_.mockMode) {
    serial_->begin(cfg_.baud, SERIAL_8N1, cfg_.rxPin, cfg_.txPin);
  }
  return true;
}

void Rs485Transport::setTxMode(bool tx) {
  if (cfg_.dePin >= 0) {
    digitalWrite(cfg_.dePin, tx ? HIGH : LOW);
  }
}

bool Rs485Transport::sendFrame(const Frame& frame) {
  if (cfg_.mockMode) return true;
  auto bytes = encodeFrame(frame);
  setTxMode(true);
  serial_->write(bytes.data(), bytes.size());
  serial_->flush();
  setTxMode(false);
  return true;
}

bool Rs485Transport::readFrame(Frame& frame, uint32_t timeoutMs) {
  if (cfg_.mockMode) return false;
  uint32_t start = millis();
  std::vector<uint8_t> buf;
  while (millis() - start < timeoutMs) {
    while (serial_->available()) {
      buf.push_back(serial_->read());
      if (buf.size() >= 8 && decodeFrame(buf.data(), buf.size(), frame)) {
        return true;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  return false;
}

bool Rs485Transport::mockHandle(const Frame& req, Frame& rsp) {
  rsp.dst = req.src;
  rsp.src = 0x31;
  rsp.cmd = req.cmd;

  if (req.cmd == Cmd::DISCOVER) {
    rsp.cmd = Cmd::HELLO;
    rsp.payload = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x01, 0x0F, 0x00, 0x01, 0x02};
    return true;
  }

  rsp.payload = {0x00};
  return true;
}

bool Rs485Transport::sendWithRetry(const Frame& frame, Frame* response, uint8_t retries, uint32_t timeoutMs) {
  for (uint8_t i = 0; i <= retries; ++i) {
    if (cfg_.mockMode) {
      if (!response) return true;
      return mockHandle(frame, *response);
    }

    sendFrame(frame);
    if (!response) return true;

    if (readFrame(*response, timeoutMs)) {
      return true;
    }
  }
  return false;
}

void Rs485Transport::loop() {
  // reserved for async polling if needed
}

}  // namespace comm
