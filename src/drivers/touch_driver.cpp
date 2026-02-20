#include "touch_driver.h"

namespace drivers {

TouchDriver gTouch;
static TouchDriver* s_touch = nullptr;

bool TouchDriver::begin(int sda, int scl, uint32_t freq) {
  s_touch = this;
  Wire.begin(sda, scl, freq);  // board I2C level set to 3.3V

  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) {
    i2cAddr_ = 0x5D;
  } else {
    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() != 0) return false;
    i2cAddr_ = 0x14;
  }

  // Set status register to 0 (ready)
  writeReg16(0x814E, 0);
  return true;
}

bool TouchDriver::writeReg16(uint16_t reg, uint8_t value) {
  Wire.beginTransmission(i2cAddr_);
  Wire.write(reg >> 8);
  Wire.write(reg & 0xFF);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool TouchDriver::readRegs(uint16_t reg, uint8_t* buf, size_t len) {
  Wire.beginTransmission(i2cAddr_);
  Wire.write(reg >> 8);
  Wire.write(reg & 0xFF);
  if (Wire.endTransmission(false) != 0) return false;

  size_t got = Wire.requestFrom((int)i2cAddr_, (int)len);
  if (got != len) return false;
  for (size_t i = 0; i < len; ++i) buf[i] = Wire.read();
  return true;
}

bool TouchDriver::readPoint(TouchPoint& p) {
  uint8_t st = 0;
  if (!readRegs(0x814E, &st, 1)) return false;
  if ((st & 0x80) == 0 || (st & 0x0F) == 0) {
    p.pressed = false;
    return true;
  }

  uint8_t data[8] = {0};
  if (!readRegs(0x8150, data, sizeof(data))) return false;
  p.x = data[1] << 8 | data[0];
  p.y = data[3] << 8 | data[2];
  p.pressed = true;
  writeReg16(0x814E, 0);
  return true;
}

void TouchDriver::indevReadCb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
  (void)indev_drv;
  TouchPoint p;
  if (!s_touch->readPoint(p)) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  data->state = p.pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  data->point.x = p.x;
  data->point.y = p.y;
}

bool TouchDriver::registerLvglIndev() {
  lv_indev_drv_init(&indevDrv_);
  indevDrv_.type = LV_INDEV_TYPE_POINTER;
  indevDrv_.read_cb = indevReadCb;
  indev_ = lv_indev_drv_register(&indevDrv_);
  return indev_ != nullptr;
}

}  // namespace drivers
