#include <algorithm>
#include "logic.h"
#include "../comm/protocol.h"

namespace app {

bool LogicController::begin(SystemState *state, EventBus *bus, RS485Transport *transport) {
    state_ = state;
    bus_ = bus;
    transport_ = transport;

    prefs_.begin("irrig", false);
    state_->settings.pulseWidthMs = prefs_.getUShort("pulse", APP_DEFAULT_PULSE_WIDTH_MS);
    state_->settings.closeAllOnBoot = prefs_.getBool("close_boot", true);
    loadSchedules();

    state_->pumpRelay = false;
    state_->pumpDc = false;
    addEvent(*state_, "Boot: pumps OFF");

    if (state_->settings.closeAllOnBoot) {
        enqueueCloseAll();
        addEvent(*state_, "Boot: queued sequential close for all zones");
    }

    return true;
}

void LogicController::enqueueCloseAll() {
    for (uint8_t i = 0; i < APP_MAX_ZONES; ++i) {
        valveQueue_.push_back({i, false});
    }
}

void LogicController::tick() {
    processValveQueue();
    state_->comm = transport_->stats();
}

void LogicController::processValveQueue() {
    if (valveQueue_.empty()) {
        return;
    }
    if ((millis() - lastValveCommandMs_) < state_->settings.pulseWidthMs) {
        return;
    }

    ValveCommand cmd = valveQueue_.front();
    valveQueue_.pop_front();
    setZone(cmd.zone, cmd.open);
    lastValveCommandMs_ = millis();
}

void LogicController::setZone(uint8_t zone, bool open) {
    if (zone >= APP_MAX_ZONES) {
        return;
    }

    uint8_t payload[2] = {zone, static_cast<uint8_t>(open ? 1 : 0)};
    auto frame = protocol::buildFrame(1, 0x10, payload, sizeof(payload));
    transport_->sendFrame(frame);

    state_->zones[zone] = open;

    bool anyZoneOn = false;
    for (bool z : state_->zones) {
        if (z) {
            anyZoneOn = true;
            break;
        }
    }

    state_->pumpRelay = anyZoneOn;
    state_->pumpDc = anyZoneOn;
    state_->mode = anyZoneOn ? SystemMode::Watering : SystemMode::Idle;

    addEvent(*state_, String("Zone ") + String(zone + 1) + (open ? " OPEN" : " CLOSE"));
}

void LogicController::handleEvent(const AppEvent &event) {
    switch (event.type) {
        case AppEventType::ZoneToggle: {
            bool target = !state_->zones[event.zone];
            valveQueue_.push_back({event.zone, target});
            break;
        }
        case AppEventType::StopAll:
            enqueueCloseAll();
            break;
        case AppEventType::PumpTest:
            state_->pumpRelay = !state_->pumpRelay;
            state_->pumpDc = state_->pumpRelay;
            addEvent(*state_, String("Pump test: ") + (state_->pumpRelay ? "ON" : "OFF"));
            break;
        case AppEventType::RescanModules:
            state_->modules.clear();
            for (uint8_t i = 0; i < 4; ++i) {
                ModuleInfo m{};
                m.uid = 0xABC000 + i;
                m.address = i + 1;
                m.firmware = "1.0." + String(i);
                m.online = true;
                state_->modules.push_back(m);
            }
            addEvent(*state_, "Module rescan complete");
            break;
        case AppEventType::AssignModuleAddress:
            for (auto &m : state_->modules) {
                if (m.address == event.moduleAddress) {
                    m.address = event.newAddress;
                    addEvent(*state_, "Module address reassigned");
                    break;
                }
            }
            break;
        case AppEventType::SaveSchedule: {
            bool updated = false;
            for (auto &s : state_->schedules) {
                if (s.id == event.schedule.id) {
                    s = event.schedule;
                    updated = true;
                    break;
                }
            }
            if (!updated && state_->schedules.size() < APP_MAX_SCHEDULES) {
                state_->schedules.push_back(event.schedule);
            }
            saveSchedules();
            addEvent(*state_, "Schedule saved");
            break;
        }
        case AppEventType::DeleteSchedule:
            state_->schedules.erase(
                std::remove_if(state_->schedules.begin(), state_->schedules.end(),
                               [&](const Schedule &s) { return s.id == event.schedule.id; }),
                state_->schedules.end());
            saveSchedules();
            addEvent(*state_, "Schedule deleted");
            break;
        case AppEventType::SetPulseWidth:
            state_->settings.pulseWidthMs = constrain(event.value16, APP_MIN_PULSE_WIDTH_MS, APP_MAX_PULSE_WIDTH_MS);
            prefs_.putUShort("pulse", state_->settings.pulseWidthMs);
            break;
        case AppEventType::SetCloseAllOnBoot:
            state_->settings.closeAllOnBoot = event.valueBool;
            prefs_.putBool("close_boot", state_->settings.closeAllOnBoot);
            break;
        default:
            break;
    }
}

void LogicController::saveSchedules() {
    prefs_.putUChar("sched_cnt", state_->schedules.size());
    for (uint8_t i = 0; i < state_->schedules.size(); ++i) {
        const Schedule &s = state_->schedules[i];
        String key = "sch" + String(i);
        uint8_t packed[9] = {
            static_cast<uint8_t>(s.id & 0xFF),
            static_cast<uint8_t>((s.id >> 8) & 0xFF),
            s.enabled,
            s.zone,
            s.daysMask,
            s.hour,
            s.minute,
            static_cast<uint8_t>(s.durationMin & 0xFF),
            static_cast<uint8_t>((s.durationMin >> 8) & 0xFF)
        };
        prefs_.putBytes(key.c_str(), packed, sizeof(packed));
    }
}

void LogicController::loadSchedules() {
    state_->schedules.clear();
    uint8_t count = prefs_.getUChar("sched_cnt", 0);
    for (uint8_t i = 0; i < count && i < APP_MAX_SCHEDULES; ++i) {
        String key = "sch" + String(i);
        uint8_t packed[9]{};
        if (prefs_.getBytes(key.c_str(), packed, sizeof(packed)) == sizeof(packed)) {
            Schedule s{};
            s.id = packed[0] | (static_cast<uint16_t>(packed[1]) << 8);
            s.enabled = packed[2];
            s.zone = packed[3];
            s.daysMask = packed[4];
            s.hour = packed[5];
            s.minute = packed[6];
            s.durationMin = packed[7] | (static_cast<uint16_t>(packed[8]) << 8);
            state_->schedules.push_back(s);
        }
    }
}

} // namespace app
