#include "state.h"

namespace core {

SystemState::SystemState() {
  for (uint8_t i = 0; i < kMaxZones; ++i) {
    zones[i].id = i + 1;
    zones[i].moduleAddr = (i / 4) + 1;
    zones[i].name = "Zone " + String(i + 1);
  }
}

void SystemState::begin() {
  mutex = xSemaphoreCreateMutex();
  prefs_.begin("irrigation", false);
  settings.pulseWidthMs = prefs_.getUShort("pulse", 200);
  settings.closeAllOnBoot = prefs_.getBool("closeBoot", true);
  settings.mockMode = prefs_.getBool("mock", true);
  settings.brightness = prefs_.getUChar("bright", 100);
  settings.languageRu = prefs_.getBool("langRu", false);
  loadSchedules();
}

void SystemState::loadSchedules() {
  schedules.clear();
  const size_t count = prefs_.getUChar("scCount", 0);
  for (size_t i = 0; i < count; ++i) {
    String key = "sc" + String(i);
    String packed = prefs_.getString(key.c_str(), "");
    if (packed.isEmpty()) continue;

    Schedule s;
    uint8_t en = 1;
    if (sscanf(packed.c_str(), "%hu,%hhu,%hhu,%hhu,%hhu,%hu,%hhu", &s.id, &s.zoneId, &s.daysMask,
               &s.hour, &s.minute, &s.durationMin, &en) >= 6) {
      s.enabled = en != 0;
      schedules.push_back(s);
      nextScheduleId_ = max<uint16_t>(nextScheduleId_, s.id + 1);
    }
  }
}

void SystemState::saveSchedules() {
  prefs_.putUChar("scCount", schedules.size());
  for (size_t i = 0; i < schedules.size(); ++i) {
    String key = "sc" + String(i);
    const auto& s = schedules[i];
    String packed = String(s.id) + "," + String(s.zoneId) + "," + String(s.daysMask) + "," +
                    String(s.hour) + "," + String(s.minute) + "," + String(s.durationMin) + "," +
                    String(s.enabled ? 1 : 0);
    prefs_.putString(key.c_str(), packed);
  }
}

void SystemState::addLog(const String& msg) {
  if (eventLog.size() >= 200) {
    eventLog.erase(eventLog.begin());
  }
  eventLog.push_back({millis(), msg});
}

void SystemState::clearLog() { eventLog.clear(); }

}  // namespace core
