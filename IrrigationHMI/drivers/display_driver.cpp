#include "display_driver.h"

#include <esp_heap_caps.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_idf_version.h>

namespace drivers {

bool DisplayDriver::begin(const DisplayConfig& cfg) {
  cfg_ = cfg;

  // TODO: verify this default pin map against your exact Waveshare board revision.
  esp_lcd_rgb_panel_config_t c = {};
  c.clk_src = LCD_CLK_SRC_DEFAULT;
  c.data_width = 16;
  c.psram_trans_align = 64;

#if ESP_IDF_VERSION_MAJOR >= 5
  // Use LVGL-owned draw buffers and prevent esp_lcd from allocating full internal frame buffers
  // (prevents `no mem for frame buffer` on some Arduino/ESP32 toolchains).
  c.num_fbs = 0;
  c.flags.no_fb = 1;
#else
  // On older cores keep a single panel framebuffer, preferably in PSRAM.
  c.num_fbs = 1;
  c.flags.fb_in_psram = 1;
#endif

  c.pclk_gpio_num = 41;
  c.hsync_gpio_num = 39;
  c.vsync_gpio_num = 40;
  c.de_gpio_num = 42;
  c.disp_gpio_num = -1;

  int d[16] = {14, 38, 18, 17, 10, 39, 0, 45, 48, 47, 21, 1, 2, 42, 41, 40};
  for (int i = 0; i < 16; ++i) c.data_gpio_nums[i] = d[i];

  c.timings.pclk_hz = 12000000;
  c.timings.h_res = cfg_.hres;
  c.timings.v_res = cfg_.vres;
  c.timings.hsync_back_porch = 40;
  c.timings.hsync_front_porch = 20;
  c.timings.hsync_pulse_width = 1;
  c.timings.vsync_back_porch = 8;
  c.timings.vsync_front_porch = 4;
  c.timings.vsync_pulse_width = 1;
  c.timings.flags.pclk_active_neg = true;

  if (esp_lcd_new_rgb_panel(&c, &panel_) != ESP_OK) {
    Serial.println("[display] esp_lcd_new_rgb_panel failed (check PSRAM and RGB timings)");
    return false;
  }
  if (esp_lcd_panel_init(panel_) != ESP_OK) {
    Serial.println("[display] esp_lcd_panel_init failed");
    return false;
  }
  esp_lcd_panel_disp_on_off(panel_, true);

  size_t pxCount = cfg_.hres * cfg_.bufLines;
  buf1_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buf2_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf1_ || !buf2_) return false;

#if LVGL_VERSION_MAJOR >= 9
  display_ = lv_display_create(cfg_.hres, cfg_.vres);
  if (!display_) return false;
  lv_display_set_buffers(display_, buf1_, buf2_, pxCount * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display_, flushCb);
  lv_display_set_user_data(display_, this);
#else
  lv_disp_draw_buf_init(&drawBuf_, buf1_, buf2_, pxCount);
  lv_disp_drv_init(&dispDrv_);
  dispDrv_.hor_res = cfg_.hres;
  dispDrv_.ver_res = cfg_.vres;
  dispDrv_.draw_buf = &drawBuf_;
  dispDrv_.flush_cb = flushCb;
  dispDrv_.user_data = this;
  lv_disp_drv_register(&dispDrv_);
#endif

  if (cfg_.backlightPin >= 0) {
    pinMode(cfg_.backlightPin, OUTPUT);
    digitalWrite(cfg_.backlightPin, cfg_.backlightActiveHigh ? HIGH : LOW);
  }
  return true;
}

void DisplayDriver::setBrightness(uint8_t pct) {
  if (cfg_.backlightPin < 0) return;
  // TODO: replace by LEDC PWM for smooth brightness control.
  bool on = pct > 0;
  digitalWrite(cfg_.backlightPin, on == cfg_.backlightActiveHigh ? HIGH : LOW);
}

#if LVGL_VERSION_MAJOR >= 9
void DisplayDriver::flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* pxMap) {
  auto* self = static_cast<DisplayDriver*>(lv_display_get_user_data(disp));
  if (!self || !self->panel_) {
    lv_display_flush_ready(disp);
    return;
  }
  esp_lcd_panel_draw_bitmap(self->panel_, area->x1, area->y1, area->x2 + 1, area->y2 + 1, pxMap);
  lv_display_flush_ready(disp);
}
#else
void DisplayDriver::flushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* colorP) {
  auto* self = static_cast<DisplayDriver*>(drv->user_data);
  if (!self || !self->panel_) {
    lv_disp_flush_ready(drv);
    return;
  }
  esp_lcd_panel_draw_bitmap(self->panel_, area->x1, area->y1, area->x2 + 1, area->y2 + 1, colorP);
  lv_disp_flush_ready(drv);
}
#endif

}  // namespace drivers
