#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

namespace core {

static constexpr uint8_t kMaxZones = 16;

struct ZoneState {
  uint8_t id = 0;
  uint8_t moduleAddr = 1;
  String name;
  bool isOpen = false;
};

struct ModuleInfo {
  uint8_t uid[6] = {0};
  uint8_t addr = 0;
  uint8_t type = 0;
  uint16_t caps = 0;
  uint16_t fwVer = 0;
  uint32_t lastSeenMs = 0;
  bool online = false;
};

struct Schedule {
  uint16_t id = 0;
  uint8_t daysMask = 0;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint16_t durationMin = 10;
  uint16_t zoneMask = 0;
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
  bool languageRU = true;
  bool mockMode = true;
};

struct LogEntry {
  uint32_t tsMs = 0;
  String text;
};

class SystemState {
 public:
  void begin();
  void loadSchedules();
  void saveSchedules();

  void addLog(const String& msg);
  void clearLog();

  ZoneState zones[kMaxZones];
  std::vector<ModuleInfo> modules;
  std::vector<Schedule> schedules;
  std::vector<LogEntry> log;

  bool pumpRelayOn = false;
  bool pumpDcOn = false;
  bool pumpTestMode = false;
  String runStatus = "Idle";

  CommStats comm;
  Settings settings;

  SemaphoreHandle_t mutex = nullptr;

 private:
  Preferences prefs_;
};

}  // namespace core
