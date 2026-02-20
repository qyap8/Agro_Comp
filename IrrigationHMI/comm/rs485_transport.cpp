#include "rs485_transport.h"

namespace comm {

bool Rs485Transport::begin(const Rs485Config& cfg) {
  cfg_ = cfg;
  serial_ = (cfg_.uartNum == 1) ? &Serial1 : &Serial2;

  if (cfg_.dePin >= 0) {
    pinMode(cfg_.dePin, OUTPUT);
    digitalWrite(cfg_.dePin, LOW);
  }

  if (!cfg_.mockMode) {
    serial_->begin(cfg_.baud, SERIAL_8N1, cfg_.rxPin, cfg_.txPin);
  }
  return true;
}

void Rs485Transport::setDir(bool tx) {
  if (cfg_.dePin >= 0) {
    digitalWrite(cfg_.dePin, tx ? HIGH : LOW);
  }
}

bool Rs485Transport::sendFrame(const Frame& frame) {
  if (cfg_.mockMode) return true;
  auto bytes = encodeFrame(frame);
  setDir(true);
  serial_->write(bytes.data(), bytes.size());
  serial_->flush();
  setDir(false);
  return true;
}

bool Rs485Transport::readFrame(Frame& out, uint32_t timeoutMs) {
  if (cfg_.mockMode) return false;
  uint32_t t0 = millis();
  std::vector<uint8_t> buf;
  while (millis() - t0 < timeoutMs) {
    while (serial_->available()) {
      buf.push_back((uint8_t)serial_->read());
      if (buf.size() >= 8 && decodeFrame(buf.data(), buf.size(), out)) {
        return true;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  return false;
}

bool Rs485Transport::mockReply(const Frame& req, Frame& rsp) {
  rsp.dst = req.src;
  rsp.src = 0x31;
  rsp.cmd = req.cmd;
  rsp.payload = {0x00};

  if (req.cmd == Cmd::DISCOVER) {
    rsp.cmd = Cmd::HELLO;
    rsp.payload = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10, 0x0F, 0x00, 0x01, 0x00};
  }
  return true;
}

bool Rs485Transport::request(const Frame& req, Frame* rsp, uint8_t retries, uint32_t timeoutMs) {
  for (uint8_t i = 0; i <= retries; ++i) {
    if (cfg_.mockMode) {
      if (!rsp) return true;
      return mockReply(req, *rsp);
    }

    sendFrame(req);
    if (!rsp) return true;
    if (readFrame(*rsp, timeoutMs)) return true;
  }
  return false;
}

void Rs485Transport::loop() {
  // reserved for async RX processing
}

}  // namespace comm
