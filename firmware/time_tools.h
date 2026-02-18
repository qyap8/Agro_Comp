#pragma once

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>

inline void applyTimezoneOffsetMinutes(int minutesOffset) {
  char sign = (minutesOffset >= 0) ? '-' : '+'; // POSIX TZ sign reversed
  int absMin = abs(minutesOffset);
  int hh = absMin / 60;
  int mm = absMin % 60;
  char tz[20];
  snprintf(tz, sizeof(tz), "UTC%c%02d:%02d", sign, hh, mm);
  setenv("TZ", tz, 1);
  tzset();
}

inline String localTimeString() {
  struct tm t;
  if (!getLocalTime(&t)) return "--:--:--";
  char buf[12];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  return String(buf);
}

inline bool setManualDateTime(int year, int month, int day, int hour, int minute, int second) {
  struct tm t = {};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  t.tm_sec = second;
  time_t epoch = mktime(&t);
  if (epoch <= 0) return false;
  timeval tv = {epoch, 0};
  settimeofday(&tv, nullptr);
  return true;
}

inline bool dayAllowed(uint8_t mask, int wday) {
  int monBased = (wday == 0) ? 6 : (wday - 1); // Sun=0 -> 6
  return (mask & (1 << monBased)) != 0;
}
