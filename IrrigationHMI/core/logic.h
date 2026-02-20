#pragma once

#include <queue>

#include "event_bus.h"
#include "state.h"
#include "../comm/rs485_transport.h"

namespace core {

struct ValveCmd {
  uint8_t zone = 0;
  bool open = false;
  uint16_t widthMs = 200;
};

class Logic {
 public:
  void begin(SystemState* state, EventBus* bus, comm::Rs485Transport* rs485);
  void onEvent(const Event& ev);
  void tick();

 private:
  void enqueueZone(uint8_t zone, bool open);
  void enqueueCloseAll();
  void pumpSafety();
  void handleDiscover();

  SystemState* state_ = nullptr;
  EventBus* bus_ = nullptr;
  comm::Rs485Transport* rs485_ = nullptr;

  std::queue<ValveCmd> q_;
  bool valveBusy_ = false;
  uint32_t valveDoneAt_ = 0;
};

}  // namespace core
