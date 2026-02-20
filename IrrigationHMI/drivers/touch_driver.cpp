#include "touch_driver.h"

namespace drivers {

static TouchDriver* sTouch = nullptr;

bool TouchDriver::begin(int sda, int scl, uint32_t freq) {
  sTouch = this;
  Wire.begin(sda, scl, freq);

  Wire.beginTransmission(0x5D);
  if (Wire.endTransmission() == 0) {
    addr_ = 0x5D;
  } else {
    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() != 0) return false;
    addr_ = 0x14;
  }

  writeReg8(0x814E, 0x00);
  return true;
}

bool TouchDriver::writeReg8(uint16_t reg, uint8_t val) {
  Wire.beginTransmission(addr_);
  Wire.write((uint8_t)(reg >> 8));
  Wire.write((uint8_t)(reg & 0xFF));
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool TouchDriver::readRegs(uint16_t reg, uint8_t* out, size_t len) {
  Wire.beginTransmission(addr_);
  Wire.write((uint8_t)(reg >> 8));
  Wire.write((uint8_t)(reg & 0xFF));
  if (Wire.endTransmission(false) != 0) return false;

  if (Wire.requestFrom((int)addr_, (int)len) != (int)len) return false;
  for (size_t i = 0; i < len; ++i) out[i] = (uint8_t)Wire.read();
  return true;
}

bool TouchDriver::readPoint(TouchPoint& p) {
  uint8_t status = 0;
  if (!readRegs(0x814E, &status, 1)) return false;

  if ((status & 0x80) == 0 || (status & 0x0F) == 0) {
    p.pressed = false;
    return true;
  }

  uint8_t d[8] = {0};
  if (!readRegs(0x8150, d, sizeof(d))) return false;

  p.x = (uint16_t)d[0] | ((uint16_t)d[1] << 8);
  p.y = (uint16_t)d[2] | ((uint16_t)d[3] << 8);
  p.pressed = true;

  writeReg8(0x814E, 0x00);
  return true;
}

void TouchDriver::lvglReadCb(lv_indev_drv_t* indev, lv_indev_data_t* data) {
  (void)indev;
  TouchPoint p;
  if (!sTouch || !sTouch->readPoint(p)) {
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
  indevDrv_.read_cb = lvglReadCb;
  return lv_indev_drv_register(&indevDrv_) != nullptr;
}

}  // namespace drivers
