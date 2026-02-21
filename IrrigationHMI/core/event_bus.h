// EventBus: очередь событий между UI и core-логикой (без жёсткой связности).
#pragma once

#include <Arduino.h>
#include "state.h"

enum class AppEventType : uint8_t {
    ZoneToggle,
    StopAll,
    PumpTest,
    RescanModules,
    AssignModuleAddress,
    SaveSchedule,
    DeleteSchedule,
    SetPulseWidth,
    SetCloseAllOnBoot,
    UiRefresh,
};

struct AppEvent {
    AppEventType type;
    uint8_t zone = 0;
    uint8_t moduleAddress = 0;
    uint8_t newAddress = 0;
    app::Schedule schedule{};
    uint16_t value16 = 0;
    bool valueBool = false;
};

class EventBus {
public:
    bool begin(size_t queueLen = 32);
    bool publish(const AppEvent &event, TickType_t timeout = 0);
    bool consume(AppEvent &event, TickType_t timeout = portMAX_DELAY);

private:
    QueueHandle_t queue_ = nullptr;
};

