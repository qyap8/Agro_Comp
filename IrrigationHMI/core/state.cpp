#include "state.h"

#include "../config.h"

namespace core {

void SystemState::begin() {
  mutex = xSemaphoreCreateMutex();
  prefs_.begin("irr_hmi", false);

  settings.pulseWidthMs = prefs_.getUShort("pulse", PULSE_WIDTH_DEFAULT_MS);
  settings.closeAllOnBoot = prefs_.getBool("closeBoot", CLOSE_ALL_ON_BOOT_DEFAULT);
  settings.brightness = prefs_.getUChar("bright", 100);
  settings.languageRU = prefs_.getBool("langRU", true);
  settings.mockMode = prefs_.getBool("mock", MOCK_MODE_DEFAULT);

  for (uint8_t i = 0; i < kMaxZones; ++i) {
    zones[i].id = i + 1;
    zones[i].moduleAddr = (i / 4) + 1;
    zones[i].name = "Zone " + String(i + 1);
  }

  loadSchedules();
  addLog("State initialized");
}

void SystemState::loadSchedules() {
  schedules.clear();
  uint8_t cnt = prefs_.getUChar("sc_cnt", 0);
  for (uint8_t i = 0; i < cnt; ++i) {
    String key = "sc_" + String(i);
    String packed = prefs_.getString(key.c_str(), "");
    if (packed.isEmpty()) continue;

    Schedule s;
    uint8_t en = 1;
    int got = sscanf(packed.c_str(), "%hu,%hhu,%hhu,%hhu,%hu,%hu,%hhu", &s.id, &s.daysMask, &s.hour, &s.minute,
                     &s.durationMin, &s.zoneMask, &en);
    if (got >= 6) {
      s.enabled = (en != 0);
      schedules.push_back(s);
    }
  }
}

void SystemState::saveSchedules() {
  prefs_.putUChar("sc_cnt", (uint8_t)schedules.size());
  for (size_t i = 0; i < schedules.size(); ++i) {
    String key = "sc_" + String(i);
    const auto& s = schedules[i];
    String packed = String(s.id) + "," + String(s.daysMask) + "," + String(s.hour) + "," + String(s.minute) +
                    "," + String(s.durationMin) + "," + String(s.zoneMask) + "," + String(s.enabled ? 1 : 0);
    prefs_.putString(key.c_str(), packed);
  }
}

void SystemState::addLog(const String& msg) {
  if (log.size() >= 200) {
    log.erase(log.begin());
  }
  log.push_back({millis(), msg});
}

void SystemState::clearLog() { log.clear(); }

}  // namespace core
