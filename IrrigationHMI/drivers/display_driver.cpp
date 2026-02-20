#include "display_driver.h"

#include <esp_heap_caps.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_idf_version.h>
#include <esp_system.h>
#include <esp_rom_sys.h>

#include "../config.h"

namespace drivers {

bool DisplayDriver::begin(const DisplayConfig& cfg) {
  cfg_ = cfg;
  const bool hasPsram = psramFound() && ESP.getPsramSize() > 0;

  // This RGB panel requires a scanout framebuffer. Without PSRAM, stable operation is not possible.
  if (!hasPsram) {
    Serial.println("[display] ERROR: PSRAM is required for RGB framebuffer on 800x480 panel");
    esp_rom_printf("[display] ERROR: PSRAM is required for RGB framebuffer on 800x480 panel\n");
    return false;
  }

  esp_lcd_rgb_panel_config_t c = {};
  c.clk_src = LCD_CLK_SRC_DEFAULT;
  c.data_width = 16;
  c.psram_trans_align = 64;
  c.sram_trans_align = 64;

  c.num_fbs = 1;
  c.flags.fb_in_psram = 1;
#if ESP_IDF_VERSION_MAJOR >= 5
  c.bounce_buffer_size_px = cfg_.hres * 20;
#endif

  c.pclk_gpio_num = LCD_PIN_PCLK;
  c.hsync_gpio_num = LCD_PIN_HSYNC;
  c.vsync_gpio_num = LCD_PIN_VSYNC;
  c.de_gpio_num = LCD_PIN_DE;
  c.disp_gpio_num = -1;

  int d[16] = {LCD_PIN_B0, LCD_PIN_B1, LCD_PIN_B2, LCD_PIN_B3, LCD_PIN_B4, LCD_PIN_G0, LCD_PIN_G1, LCD_PIN_G2,
               LCD_PIN_G3, LCD_PIN_G4, LCD_PIN_G5, LCD_PIN_R0, LCD_PIN_R1, LCD_PIN_R2, LCD_PIN_R3, LCD_PIN_R4};
  for (int i = 0; i < 16; ++i) c.data_gpio_nums[i] = d[i];

  c.timings.pclk_hz = LCD_PCLK_HZ;
  c.timings.h_res = cfg_.hres;
  c.timings.v_res = cfg_.vres;
  c.timings.hsync_back_porch = LCD_HSYNC_BP;
  c.timings.hsync_front_porch = LCD_HSYNC_FP;
  c.timings.hsync_pulse_width = LCD_HSYNC_PW;
  c.timings.vsync_back_porch = LCD_VSYNC_BP;
  c.timings.vsync_front_porch = LCD_VSYNC_FP;
  c.timings.vsync_pulse_width = LCD_VSYNC_PW;
  c.timings.flags.pclk_active_neg = true;

  Serial.printf("[display] RGB ctrl pins PCLK=%d HSYNC=%d VSYNC=%d DE=%d\n", c.pclk_gpio_num, c.hsync_gpio_num,
                c.vsync_gpio_num, c.de_gpio_num);
  esp_rom_printf("[display] RGB ctrl pins PCLK=%d HSYNC=%d VSYNC=%d DE=%d\n", c.pclk_gpio_num, c.hsync_gpio_num,
                 c.vsync_gpio_num, c.de_gpio_num);
  Serial.printf("[display] psramFound=%d freeHeap=%u freePsram=%u mode=FB_PSRAM\n", (int)psramFound(),
                (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getFreePsram());
  esp_rom_printf("[display] psramFound=%d freeHeap=%u freePsram=%u mode=FB_PSRAM\n", (int)psramFound(),
                 (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getFreePsram());

  if (esp_lcd_new_rgb_panel(&c, &panel_) != ESP_OK) {
    Serial.println("[display] esp_lcd_new_rgb_panel failed");
    return false;
  }
  if (esp_lcd_panel_init(panel_) != ESP_OK) {
    Serial.println("[display] esp_lcd_panel_init failed");
    return false;
  }
  esp_lcd_panel_disp_on_off(panel_, true);

  size_t pxCount = cfg_.hres * cfg_.bufLines;
  uint32_t caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
  buf1_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), caps);
  buf2_ = (lv_color_t*)heap_caps_malloc(pxCount * sizeof(lv_color_t), caps);
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
