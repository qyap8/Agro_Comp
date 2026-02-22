#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include "state.h"
#include "event_bus.h"
#include "../comm/rs485_transport.h"

namespace app {

class LogicController {
public:
    bool begin(SystemState *state, EventBus *bus, RS485Transport *transport);
    void tick();
    void handleEvent(const AppEvent &event);
    SemaphoreHandle_t stateMutex() const { return stateMutex_; }

private:
    void loadSettings();
    void saveWifiCreds(const char *ssid, const char *pass);
    void sendSetChannelState(uint8_t moduleAddr, uint8_t channelId, bool state);
    void discoverModules();
    void updateModulePresence();
    void wifiAutoConnect();
    void startApMode();
    void startWebServer();
    void handleWebServer();
    void applyLanguage(Lang lang);
    String buildStateJson();

    SystemState *state_ = nullptr;
    EventBus *bus_ = nullptr;
    RS485Transport *transport_ = nullptr;
    SemaphoreHandle_t stateMutex_ = nullptr;

    Preferences prefs_;
    WebServer web_{80};
    bool webStarted_ = false;
    uint32_t lastDiscoverMs_ = 0;
};

} // namespace app
