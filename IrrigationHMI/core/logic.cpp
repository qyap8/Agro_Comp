#include "logic.h"

namespace core {

void Logic::begin(SystemState* state, EventBus* bus, comm::Rs485Transport* rs485) {
  state_ = state;
  bus_ = bus;
  rs485_ = rs485;

  state_->pumpRelayOn = false;
  state_->pumpDcOn = false;
  if (state_->settings.closeAllOnBoot) enqueueCloseAll();
}

void Logic::enqueueZone(uint8_t zone, bool open) {
  if (zone < 1 || zone > kMaxZones) return;
  q_.push({zone, open, state_->settings.pulseWidthMs});
}

void Logic::enqueueCloseAll() {
  for (uint8_t z = 1; z <= kMaxZones; ++z) enqueueZone(z, false);
}

void Logic::handleDiscover() {
  comm::Frame req;
  req.dst = 0xFF;
  req.src = 0x01;
  req.cmd = comm::Cmd::DISCOVER;

  comm::Frame rsp;
  if (rs485_->request(req, &rsp, 2, 150) && rsp.cmd == comm::Cmd::HELLO && rsp.payload.size() >= 11) {
    ModuleInfo m;
    memcpy(m.uid, rsp.payload.data(), 6);
    m.type = rsp.payload[6];
    m.caps = rsp.payload[7] | (rsp.payload[8] << 8);
    m.fwVer = rsp.payload[9] | (rsp.payload[10] << 8);
    m.addr = rsp.src;
    m.lastSeenMs = millis();
    m.online = true;

    xSemaphoreTake(state_->mutex, portMAX_DELAY);
    state_->modules.push_back(m);
    state_->comm.ok++;
    state_->addLog("Module discovered addr=" + String(m.addr));
    xSemaphoreGive(state_->mutex);
  } else {
    xSemaphoreTake(state_->mutex, portMAX_DELAY);
    state_->comm.timeouts++;
    state_->addLog("Discover timeout");
    xSemaphoreGive(state_->mutex);
  }
}

void Logic::onEvent(const Event& ev) {
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  switch (ev.type) {
    case EventType::ZONE_SET:
      enqueueZone(ev.a, ev.b != 0);
      break;
    case EventType::STOP_ALL:
      enqueueCloseAll();
      break;
    case EventType::PUMP_TEST:
      state_->pumpTestMode = (ev.a != 0);
      break;
    case EventType::SETTINGS_PULSE_WIDTH:
      state_->settings.pulseWidthMs = ev.v;
      break;
    case EventType::SETTINGS_CLOSE_BOOT:
      state_->settings.closeAllOnBoot = (ev.a != 0);
      break;
    case EventType::SETTINGS_BRIGHTNESS:
      state_->settings.brightness = (uint8_t)ev.v;
      break;
    case EventType::LOG_CLEAR:
      state_->clearLog();
      break;
    case EventType::DISCOVER:
      xSemaphoreGive(state_->mutex);
      handleDiscover();
      return;
    default:
      break;
  }
  xSemaphoreGive(state_->mutex);
}

void Logic::pumpSafety() {
  bool anyOpen = false;
  for (const auto& z : state_->zones) anyOpen |= z.isOpen;
  bool allow = anyOpen || state_->pumpTestMode;
  state_->pumpRelayOn = allow;
  state_->pumpDcOn = allow;
}

void Logic::tick() {
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  uint32_t now = millis();

  if (valveBusy_ && now >= valveDoneAt_) valveBusy_ = false;

  if (!valveBusy_ && !q_.empty()) {
    ValveCmd c = q_.front();
    q_.pop();

    uint8_t module = state_->zones[c.zone - 1].moduleAddr;
    comm::Frame req;
    req.dst = module;
    req.src = 0x01;
    req.cmd = comm::Cmd::VALVE_PULSE;
    req.payload = {c.zone, (uint8_t)(c.open ? 1 : 0), (uint8_t)(c.widthMs & 0xFF), (uint8_t)(c.widthMs >> 8)};
    rs485_->request(req, nullptr, 1, 100);

    state_->zones[c.zone - 1].isOpen = c.open;
    state_->addLog(String(c.open ? "OPEN Z" : "CLOSE Z") + String(c.zone));

    valveBusy_ = true;
    valveDoneAt_ = now + c.widthMs + 40;
  }

  pumpSafety();
  xSemaphoreGive(state_->mutex);
}

}  // namespace core
