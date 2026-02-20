#include "display_driver.h"

#include <esp_heap_caps.h>
#include <esp_lcd_panel_rgb.h>

namespace drivers {

DisplayDriver gDisplay;

static DisplayDriver* s_inst = nullptr;

bool DisplayDriver::begin(const DisplayConfig& cfg) {
  cfg_ = cfg;
  s_inst = this;
  lv_init();

  // TODO: verify exact Waveshare ESP32-S3-Touch-LCD-7 GPIO map for your board revision.
  esp_lcd_rgb_panel_config_t panel_config = {};
  panel_config.data_width = 16;
  panel_config.psram_trans_align = 64;
  panel_config.num_fbs = 0;  // LVGL own double buffering
  panel_config.clk_src = LCD_CLK_SRC_DEFAULT;
  panel_config.disp_gpio_num = -1;
  panel_config.pclk_gpio_num = 41;
  panel_config.vsync_gpio_num = 40;
  panel_config.hsync_gpio_num = 39;
  panel_config.de_gpio_num = 42;
  panel_config.data_gpio_nums[0] = 14;
  panel_config.data_gpio_nums[1] = 38;
  panel_config.data_gpio_nums[2] = 18;
  panel_config.data_gpio_nums[3] = 17;
  panel_config.data_gpio_nums[4] = 10;
  panel_config.data_gpio_nums[5] = 39;
  panel_config.data_gpio_nums[6] = 0;
  panel_config.data_gpio_nums[7] = 45;
  panel_config.data_gpio_nums[8] = 48;
  panel_config.data_gpio_nums[9] = 47;
  panel_config.data_gpio_nums[10] = 21;
  panel_config.data_gpio_nums[11] = 1;
  panel_config.data_gpio_nums[12] = 2;
  panel_config.data_gpio_nums[13] = 42;
  panel_config.data_gpio_nums[14] = 41;
  panel_config.data_gpio_nums[15] = 40;

  panel_config.timings.pclk_hz = 12 * 1000 * 1000;
  panel_config.timings.h_res = cfg_.hres;
  panel_config.timings.v_res = cfg_.vres;
  panel_config.timings.hsync_back_porch = 40;
  panel_config.timings.hsync_front_porch = 20;
  panel_config.timings.hsync_pulse_width = 1;
  panel_config.timings.vsync_back_porch = 8;
  panel_config.timings.vsync_front_porch = 4;
  panel_config.timings.vsync_pulse_width = 1;
  panel_config.timings.flags.pclk_active_neg = true;

  if (esp_lcd_new_rgb_panel(&panel_config, &panel_) != ESP_OK) return false;
  if (esp_lcd_panel_init(panel_) != ESP_OK) return false;
  esp_lcd_panel_disp_on_off(panel_, true);

  size_t pxCount = cfg_.hres * cfg_.bufferLines;
  buf1_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buf2_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf1_ || !buf2_) return false;

  lv_disp_draw_buf_init(&drawBuf_, buf1_, buf2_, pxCount);
  lv_disp_drv_init(&dispDrv_);
  dispDrv_.hor_res = cfg_.hres;
  dispDrv_.ver_res = cfg_.vres;
  dispDrv_.flush_cb = flushCb;
  dispDrv_.draw_buf = &drawBuf_;
  dispDrv_.user_data = this;
  lv_disp_ = lv_disp_drv_register(&dispDrv_);

  if (cfg_.backlightPin >= 0) {
    pinMode(cfg_.backlightPin, OUTPUT);
    digitalWrite(cfg_.backlightPin, cfg_.backlightActiveHigh ? HIGH : LOW);
  }

  return true;
}

void DisplayDriver::setBrightness(uint8_t percent) {
  // TODO: replace with LEDC PWM brightness control if BL pin supports dimming.
  if (cfg_.backlightPin < 0) return;
  digitalWrite(cfg_.backlightPin, (percent > 0) == cfg_.backlightActiveHigh ? HIGH : LOW);
}

void DisplayDriver::flushCb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
  auto* self = static_cast<DisplayDriver*>(disp_drv->user_data);
  esp_lcd_panel_draw_bitmap(self->panel_, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
  lv_disp_flush_ready(disp_drv);
}

}  // namespace drivers
