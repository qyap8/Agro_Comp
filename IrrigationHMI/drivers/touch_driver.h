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
  bool begin(int sda, int scl, uint32_t freq);
  bool readPoint(TouchPoint& p);
  bool registerLvglIndev();

 private:
  bool writeReg8(uint16_t reg, uint8_t val);
  bool readRegs(uint16_t reg, uint8_t* out, size_t len);
#if LVGL_VERSION_MAJOR >= 9
  static void lvglReadCb(lv_indev_t* indev, lv_indev_data_t* data);
#else
  static void lvglReadCb(lv_indev_drv_t* indev, lv_indev_data_t* data);
  lv_indev_drv_t indevDrv_;
#endif

  uint8_t addr_ = 0x5D;
};

}  // namespace drivers
