#pragma once

#include <queue>

#include "comm/rs485_transport.h"
#include "core/event_bus.h"
#include "core/state.h"

namespace core {

struct ValveCommand {
  uint8_t zone = 0;
  bool open = false;
  uint16_t widthMs = 200;
};

class Logic {
 public:
  void begin(SystemState* state, EventBus* bus, comm::Rs485Transport* transport);
  void processEvent(const Event& ev);
  void tick();

 private:
  void enqueueZone(uint8_t zone, bool open);
  void sendCloseAll();
  void updatePumpSafety();
  void discoverModules();

  SystemState* state_ = nullptr;
  EventBus* bus_ = nullptr;
  comm::Rs485Transport* transport_ = nullptr;

  std::queue<ValveCommand> valveQueue_;
  bool valveBusy_ = false;
  uint32_t valveDoneAt_ = 0;
};

}  // namespace core
