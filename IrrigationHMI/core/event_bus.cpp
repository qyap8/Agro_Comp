#include "event_bus.h"

bool EventBus::begin(size_t queueLen) {
    if (queue_ != nullptr) {
        return true;
    }
    queue_ = xQueueCreate(queueLen, sizeof(AppEvent));
    return queue_ != nullptr;
}

bool EventBus::publish(const AppEvent &event, TickType_t timeout) {
    if (queue_ == nullptr) {
        return false;
    }
    return xQueueSend(queue_, &event, timeout) == pdTRUE;
}

bool EventBus::consume(AppEvent &event, TickType_t timeout) {
    if (queue_ == nullptr) {
        return false;
    }
    return xQueueReceive(queue_, &event, timeout) == pdTRUE;
}
