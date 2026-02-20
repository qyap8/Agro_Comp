#include "logic.h"

namespace core {

void Logic::begin(SystemState* state, EventBus* bus, comm::Rs485Transport* transport) {
  state_ = state;
  bus_ = bus;
  transport_ = transport;

  state_->pumpRelayOn = false;
  state_->pumpDcOn = false;
  if (state_->settings.closeAllOnBoot) {
    sendCloseAll();
  }
}

void Logic::enqueueZone(uint8_t zone, bool open) {
  if (zone == 0 || zone > kMaxZones) return;
  valveQueue_.push({zone, open, state_->settings.pulseWidthMs});
}

void Logic::sendCloseAll() {
  for (uint8_t z = 1; z <= kMaxZones; ++z) {
    enqueueZone(z, false);
  }
}

void Logic::discoverModules() {
  comm::Frame req;
  req.dst = 0xFF;
  req.src = 0x01;
  req.cmd = comm::Cmd::DISCOVER;
  comm::Frame rsp;
  if (transport_->sendWithRetry(req, &rsp, 2, 150) && rsp.cmd == comm::Cmd::HELLO && rsp.payload.size() >= 11) {
    ModuleInfo m;
    memcpy(m.uid, rsp.payload.data(), 6);
    m.type = rsp.payload[6];
    m.caps = rsp.payload[7] | (rsp.payload[8] << 8);
    m.fw = rsp.payload[9] | (rsp.payload[10] << 8);
    m.addr = rsp.src;
    m.lastSeenMs = millis();
    m.online = true;

    xSemaphoreTake(state_->mutex, portMAX_DELAY);
    state_->modules.push_back(m);
    state_->commStats.ok++;
    state_->addLog("Module discovered addr=" + String(m.addr));
    xSemaphoreGive(state_->mutex);
  } else {
    xSemaphoreTake(state_->mutex, portMAX_DELAY);
    state_->commStats.timeouts++;
    state_->addLog("DISCOVER timeout");
    xSemaphoreGive(state_->mutex);
  }
}

void Logic::processEvent(const Event& ev) {
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  switch (ev.type) {
    case EventType::ZONE_SET:
      enqueueZone(ev.a, ev.b != 0);
      break;
    case EventType::STOP_ALL:
      sendCloseAll();
      break;
    case EventType::PUMP_TEST:
      state_->pumpTestMode = ev.a;
      break;
    case EventType::DISCOVER:
      xSemaphoreGive(state_->mutex);
      discoverModules();
      return;
    case EventType::SETTINGS_CHANGED:
      state_->settings.pulseWidthMs = ev.value;
      break;
    case EventType::LOG_CLEAR:
      state_->clearLog();
      break;
    default:
      break;
  }
  xSemaphoreGive(state_->mutex);
}

void Logic::updatePumpSafety() {
  bool anyOpen = false;
  for (const auto& z : state_->zones) {
    anyOpen |= z.isOpen;
  }

  bool allow = anyOpen || state_->pumpTestMode;
  state_->pumpRelayOn = allow;
  state_->pumpDcOn = allow;
}

void Logic::tick() {
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  uint32_t now = millis();

  if (valveBusy_ && now >= valveDoneAt_) {
    valveBusy_ = false;
  }

  if (!valveBusy_ && !valveQueue_.empty()) {
    auto cmd = valveQueue_.front();
    valveQueue_.pop();

    comm::Frame req;
    req.dst = state_->zones[cmd.zone - 1].moduleAddr;
    req.src = 0x01;
    req.cmd = comm::Cmd::VALVE_PULSE;
    req.payload = {cmd.zone, (uint8_t)(cmd.open ? 1 : 0), (uint8_t)(cmd.widthMs & 0xFF), (uint8_t)(cmd.widthMs >> 8)};
    transport_->sendWithRetry(req, nullptr, 1, 100);

    state_->zones[cmd.zone - 1].isOpen = cmd.open;
    state_->addLog(String(cmd.open ? "OPEN Z" : "CLOSE Z") + String(cmd.zone));
    valveBusy_ = true;
    valveDoneAt_ = now + cmd.widthMs + 50;
  }

  updatePumpSafety();
  xSemaphoreGive(state_->mutex);
}

}  // namespace core
