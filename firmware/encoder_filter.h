#pragma once

#include <Arduino.h>

class EncoderFilter {
 public:
  void begin(uint8_t pinA, uint8_t pinB) {
    pinA_ = pinA;
    pinB_ = pinB;
    pinMode(pinA_, INPUT_PULLUP);
    pinMode(pinB_, INPUT_PULLUP);
    prevAB_ = (readA() << 1) | readB();
  }

  int8_t readStep() {
    uint8_t ab = (readA() << 1) | readB();
    uint8_t index = (prevAB_ << 2) | ab;
    prevAB_ = ab;

    static const int8_t table[16] = {
      0, -1, 1, 0,
      1, 0, 0, -1,
      -1, 0, 0, 1,
      0, 1, -1, 0
    };

    int8_t delta = table[index];
    if (delta == 0) return 0;

    unsigned long now = millis();
    if (now - lastEdgeMs_ < 1) return 0; // anti-bounce
    lastEdgeMs_ = now;

    acc_ += delta;
    if (acc_ >= detentThreshold_) {
      acc_ = 0;
      return 1;
    }
    if (acc_ <= -detentThreshold_) {
      acc_ = 0;
      return -1;
    }
    return 0;
  }

  void setDetentThreshold(int8_t t) {
    detentThreshold_ = (t < 1) ? 1 : t;
  }

 private:
  uint8_t pinA_ = 0;
  uint8_t pinB_ = 0;
  uint8_t prevAB_ = 0;
  int8_t acc_ = 0;
  int8_t detentThreshold_ = 4;
  unsigned long lastEdgeMs_ = 0;

  uint8_t readA() const { return digitalRead(pinA_) ? 1 : 0; }
  uint8_t readB() const { return digitalRead(pinB_) ? 1 : 0; }
};
