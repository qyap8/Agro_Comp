#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include <esp_lcd_panel_ops.h>

namespace drivers {

struct DisplayConfig {
  int hres = 800;
  int vres = 480;
  int bufferLines = 60;
  int backlightPin = 2;
  bool backlightActiveHigh = true;
};

class DisplayDriver {
 public:
  bool begin(const DisplayConfig& cfg = DisplayConfig());
  lv_disp_t* lvDisplay() { return lv_disp_; }
  void setBrightness(uint8_t percent);

 private:
  static void flushCb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);

  DisplayConfig cfg_;
  esp_lcd_panel_handle_t panel_ = nullptr;
  lv_disp_draw_buf_t drawBuf_;
  lv_disp_drv_t dispDrv_;
  lv_disp_t* lv_disp_ = nullptr;

  lv_color_t* buf1_ = nullptr;
  lv_color_t* buf2_ = nullptr;
};

extern DisplayDriver gDisplay;

}  // namespace drivers
