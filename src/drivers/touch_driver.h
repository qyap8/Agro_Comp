#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

namespace drivers {

struct TouchPoint {
  uint16_t x = 0;
  uint16_t y = 0;
  bool pressed = false;
};

class TouchDriver {
 public:
  bool begin(int sda = 8, int scl = 9, uint32_t freq = 400000);
  bool readPoint(TouchPoint& p);
  bool registerLvglIndev();

 private:
  static void indevReadCb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
  bool writeReg16(uint16_t reg, uint8_t value);
  bool readRegs(uint16_t reg, uint8_t* buf, size_t len);

  uint8_t i2cAddr_ = 0x5D;
  lv_indev_drv_t indevDrv_;
  lv_indev_t* indev_ = nullptr;
};

extern TouchDriver gTouch;

}  // namespace drivers
