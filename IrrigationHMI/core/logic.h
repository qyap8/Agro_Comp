// LogicController: бизнес-логика полива, очередь клапанов и сохранение настроек.
#pragma once

#include <Arduino.h>
#include <deque>
#include <Preferences.h>
#include "state.h"
#include "event_bus.h"
#include "../comm/rs485_transport.h"

namespace app {

struct ValveCommand {
    uint8_t zone = 0;
    bool open = false;
};

class LogicController {
public:
    bool begin(SystemState *state, EventBus *bus, RS485Transport *transport);
    void tick();
    void handleEvent(const AppEvent &event);
    void enqueueCloseAll();

private:
    void processValveQueue();
    void setZone(uint8_t zone, bool open);
    void saveSchedules();
    void loadSchedules();

    SystemState *state_ = nullptr;
    EventBus *bus_ = nullptr;
    RS485Transport *transport_ = nullptr;
    std::deque<ValveCommand> valveQueue_;
    uint32_t lastValveCommandMs_ = 0;
    Preferences prefs_;
};

} // namespace app
