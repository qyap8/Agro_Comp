#pragma once

#include <Arduino.h>
#include <esp_lcd_panel_ops.h>
#include <lvgl.h>

namespace drivers {

struct DisplayConfig {
  int hres = 800;
  int vres = 480;
  int bufLines = 60;
  int backlightPin = -1;
  bool backlightActiveHigh = true;
};

class DisplayDriver {
 public:
  bool begin(const DisplayConfig& cfg);
  void setBrightness(uint8_t pct);

 private:
#if LVGL_VERSION_MAJOR >= 9
  static void flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* pxMap);
  lv_display_t* display_ = nullptr;
#else
  static void flushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* colorP);
  lv_disp_draw_buf_t drawBuf_;
  lv_disp_drv_t dispDrv_;
#endif

  DisplayConfig cfg_;
  esp_lcd_panel_handle_t panel_ = nullptr;
  lv_color_t* buf1_ = nullptr;
  lv_color_t* buf2_ = nullptr;
};

}  // namespace drivers
