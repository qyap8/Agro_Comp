#pragma once

#include <Arduino.h>
#include "state.h"

enum class AppEventType : uint8_t {
    DiscoverModules,
    SetChannelState,
    ModuleRescan,
    WifiConnect,
    WifiStartAp,
    WifiSaveCreds,
    SetLanguage,
};

struct AppEvent {
    AppEventType type;
    uint8_t moduleAddr = 0;
    uint8_t channelId = 0;
    bool valueBool = false;
    uint16_t value16 = 0;
    app::Lang language = app::Lang::EN;
    char ssid[33]{};
    char pass[65]{};
};

class EventBus {
public:
    bool begin(size_t queueLen = 48);
    bool publish(const AppEvent &event, TickType_t timeout = 0);
    bool consume(AppEvent &event, TickType_t timeout = portMAX_DELAY);

private:
    QueueHandle_t queue_ = nullptr;
};
