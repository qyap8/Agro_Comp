#pragma once

#include <Arduino.h>

namespace core {

enum class EventType : uint8_t {
  NONE,
  ZONE_SET,
  STOP_ALL,
  PUMP_TEST,
  DISCOVER,
  ASSIGN_ADDR,
  STATUS_GET,
  SETTINGS_PULSE_WIDTH,
  SETTINGS_CLOSE_BOOT,
  SETTINGS_BRIGHTNESS,
  LOG_CLEAR,
};

struct Event {
  EventType type = EventType::NONE;
  uint8_t a = 0;
  uint8_t b = 0;
  uint16_t v = 0;
  uint8_t uid[6] = {0};
};

class EventBus {
 public:
  bool begin(size_t depth = 64);
  bool publish(const Event& ev, TickType_t timeout = 0);
  bool consume(Event& ev, TickType_t timeout = 0);

 private:
  QueueHandle_t q_ = nullptr;
};

}  // namespace core
