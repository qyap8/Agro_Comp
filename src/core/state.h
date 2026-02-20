#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

namespace core {

static constexpr uint8_t kMaxZones = 16;
static constexpr uint8_t kMaxModules = 32;

struct ZoneState {
  uint8_t id = 0;
  uint8_t moduleAddr = 0;
  String name;
  bool isOpen = false;
};

struct ModuleInfo {
  uint8_t uid[6] = {0};
  uint8_t addr = 0;
  uint8_t type = 0;
  uint16_t caps = 0;
  uint16_t fw = 0;
  uint32_t lastSeenMs = 0;
  bool online = false;
};

struct Schedule {
  uint16_t id = 0;
  uint8_t zoneId = 0;
  uint8_t daysMask = 0;   // bit0=Mon ... bit6=Sun
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint16_t durationMin = 0;
  bool enabled = true;
};

struct CommStats {
  uint32_t ok = 0;
  uint32_t crcErr = 0;
  uint32_t timeouts = 0;
};

struct Settings {
  uint16_t pulseWidthMs = 200;
  bool closeAllOnBoot = true;
  uint8_t brightness = 100;
  bool languageRu = false;
  bool mockMode = true;
};

struct EventLogEntry {
  uint32_t timestampMs = 0;
  String text;
};

class SystemState {
 public:
  SystemState();

  void begin();
  void loadSchedules();
  void saveSchedules();

  ZoneState zones[kMaxZones];
  std::vector<ModuleInfo> modules;
  std::vector<Schedule> schedules;
  std::vector<EventLogEntry> eventLog;

  bool pumpRelayOn = false;
  bool pumpDcOn = false;
  bool pumpTestMode = false;

  CommStats commStats;
  Settings settings;
  String runStatus = "Idle";

  SemaphoreHandle_t mutex = nullptr;

  void addLog(const String& msg);
  void clearLog();

 private:
  Preferences prefs_;
  uint16_t nextScheduleId_ = 1;
};

}  // namespace core
