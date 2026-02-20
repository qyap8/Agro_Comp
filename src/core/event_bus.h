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
  STATUS_POLL,
  SETTINGS_CHANGED,
  UI_REFRESH,
  LOG_CLEAR,
};

struct Event {
  EventType type = EventType::NONE;
  uint8_t a = 0;
  uint8_t b = 0;
  uint16_t value = 0;
  uint8_t uid[6] = {0};
};

class EventBus {
 public:
  bool begin(size_t queueDepth = 32);
  bool publish(const Event& ev, TickType_t timeoutTicks = 0);
  bool consume(Event& ev, TickType_t timeoutTicks = portMAX_DELAY);

 private:
  QueueHandle_t queue_ = nullptr;
};

}  // namespace core
