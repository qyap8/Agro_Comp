#include "event_bus.h"

namespace core {

bool EventBus::begin(size_t queueDepth) {
  queue_ = xQueueCreate(queueDepth, sizeof(Event));
  return queue_ != nullptr;
}

bool EventBus::publish(const Event& ev, TickType_t timeoutTicks) {
  if (!queue_) return false;
  return xQueueSend(queue_, &ev, timeoutTicks) == pdTRUE;
}

bool EventBus::consume(Event& ev, TickType_t timeoutTicks) {
  if (!queue_) return false;
  return xQueueReceive(queue_, &ev, timeoutTicks) == pdTRUE;
}

}  // namespace core
