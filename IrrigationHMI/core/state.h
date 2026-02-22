#pragma once

#include <Arduino.h>
#include <array>
#include <vector>
#include "../config.h"

#define APP_HAS_SCREEN_TIMEOUT_SETTING 1

namespace app {

enum class SystemMode : uint8_t { Idle, Online, Error };

enum class Lang : uint8_t { EN = 0, ES = 1 };

struct ChannelInfo {
    uint8_t id = 0;
    String name;
    bool state = false;
};

struct ModuleInfo {
    uint32_t uid = 0;
    uint8_t address = 0;
    String firmware;
    bool online = false;
    std::vector<ChannelInfo> channels;
    uint32_t lastSeenMs = 0;
};

struct CommStats {
    uint32_t txFrames = 0;
    uint32_t rxFrames = 0;
    uint32_t crcErrors = 0;
    uint32_t timeouts = 0;
};

struct WifiState {
    bool connected = false;
    bool apMode = false;
    String ssid;
    IPAddress ip;
};


struct TimeState {
    String hhmm = "--:--";
    String dayName = "---";
    bool synced = false;
};

struct WeatherState {
    bool valid = false;
    String summary = "N/A";
    float temperatureC = NAN;
    uint32_t updatedAtMs = 0;
};

struct Settings {
    uint32_t rs485Baud = APP_RS485_BAUD_DEFAULT;
    Lang language = Lang::EN;
    String wifiSsid;
    String wifiPass;
    uint16_t screenTimeoutSec = 60; // 0 = never sleep
};

struct EventRecord {
    uint32_t ts = 0;
    String text;
};

struct SystemState {
    SystemMode mode = SystemMode::Idle;
    std::vector<ModuleInfo> modules;
    CommStats comm;
    WifiState wifi;
    Settings settings;
    TimeState time;
    WeatherState weather;
    std::array<EventRecord, APP_EVENT_LOG_CAPACITY> eventLog{};
    uint16_t eventHead = 0;
    uint16_t eventCount = 0;
};

void addEvent(SystemState &state, const String &text);

} // namespace app
