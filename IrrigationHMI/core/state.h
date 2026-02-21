#pragma once

#include <Arduino.h>
#include <array>
#include <vector>
#include "../config.h"

namespace app {

enum class SystemMode : uint8_t {
    Idle,
    Watering,
    Error
};

struct ModuleInfo {
    uint32_t uid = 0;
    uint8_t address = 0;
    String firmware;
    bool online = false;
};

struct Schedule {
    uint16_t id = 0;
    bool enabled = true;
    uint8_t zone = 0;
    uint8_t daysMask = 0x7F;
    uint8_t hour = 6;
    uint8_t minute = 0;
    uint16_t durationMin = 10;
};

struct CommStats {
    uint32_t txFrames = 0;
    uint32_t rxFrames = 0;
    uint32_t crcErrors = 0;
    uint32_t timeouts = 0;
};

struct Settings {
    uint16_t pulseWidthMs = APP_DEFAULT_PULSE_WIDTH_MS;
    bool closeAllOnBoot = true;
    uint8_t brightness = 100;
    uint8_t language = 0;
};

struct EventRecord {
    uint32_t ts = 0;
    String text;
};

struct SystemState {
    SystemMode mode = SystemMode::Idle;
    std::array<bool, APP_MAX_ZONES> zones{};
    bool pumpRelay = false;
    bool pumpDc = false;
    std::vector<ModuleInfo> modules;
    std::vector<Schedule> schedules;
    CommStats comm;
    Settings settings;
    std::array<EventRecord, APP_EVENT_LOG_CAPACITY> eventLog{};
    uint16_t eventHead = 0;
    uint16_t eventCount = 0;
};

void addEvent(SystemState &state, const String &text);

} // namespace app
