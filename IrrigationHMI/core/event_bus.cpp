#include "event_bus.h"

namespace core {

bool EventBus::begin(size_t depth) {
  q_ = xQueueCreate(depth, sizeof(Event));
  return q_ != nullptr;
}

bool EventBus::publish(const Event& ev, TickType_t timeout) {
  if (!q_) return false;
  return xQueueSend(q_, &ev, timeout) == pdTRUE;
}

bool EventBus::consume(Event& ev, TickType_t timeout) {
  if (!q_) return false;
  return xQueueReceive(q_, &ev, timeout) == pdTRUE;
}

}  // namespace core
