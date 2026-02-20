/*
  IrrigationHMI.ino

  Required board settings (Arduino IDE):
  - Board: ESP32S3 Dev Module (or Waveshare ESP32-S3-Touch-LCD-7 profile if available)
  - ESP32 Arduino core: 2.0.14+
  - PSRAM: Enabled (OPI PSRAM)
  - Flash size: 16MB (WROOM N16R8)

  Library requirements:
  - lvgl v8.x
  - ESP32 Arduino core with esp_lcd support

  RS485 termination:
  - Enable the 120-ohm termination jumper ONLY if this device is at one end of the RS485 bus.
  - Keep termination OFF for mid-bus nodes.
*/

#include <Arduino.h>
#include <esp_rom_sys.h>
#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif
#include <lvgl.h>

#include "config.h"
#include "core/state.h"
#include "core/event_bus.h"
#include "core/logic.h"
#include "comm/rs485_transport.h"
#include "drivers/display_driver.h"
#include "drivers/touch_driver.h"
#include "ui/ui_app.h"

static core::SystemState gState;
static core::EventBus gBus;
static core::Logic gLogic;
static comm::Rs485Transport gRs485;
static drivers::DisplayDriver gDisplay;
static drivers::TouchDriver gTouch;
static ui::UiApp gUi;

static comm::Rs485Config makeRs485Config() {
  comm::Rs485Config cfg;
  cfg.uartNum = RS485_UART_NUM;
  cfg.txPin = RS485_TX_PIN;
  cfg.rxPin = RS485_RX_PIN;
  cfg.dePin = RS485_DE_PIN;
  cfg.baud = RS485_BAUD;
  cfg.mockMode = gState.settings.mockMode;
  cfg.localAddr = 0x01;
  return cfg;
}

static void ui_task(void* arg) {
  (void)arg;
  gUi.begin(&gState, &gBus);

  uint32_t lastRefresh = 0;
  while (true) {
    lv_tick_inc(5);
    lv_timer_handler();

    if (millis() - lastRefresh >= UI_REFRESH_MS) {
      gUi.refresh();
      lastRefresh = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

static void logic_task(void* arg) {
  (void)arg;
  core::Event ev;
  while (true) {
    while (gBus.consume(ev, 0)) {
      gLogic.onEvent(ev);
    }
    gLogic.tick();
    vTaskDelay(pdMS_TO_TICKS(LOGIC_TICK_MS));
  }
}

static void comm_task(void* arg) {
  (void)arg;
  uint32_t lastStatusPoll = 0;
  while (true) {
    gRs485.loop();

    if (millis() - lastStatusPoll >= 1000) {
      core::Event ev;
      ev.type = core::EventType::STATUS_GET;
      gBus.publish(ev);
      lastStatusPoll = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(COMM_TICK_MS));
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(50));
  esp_rom_printf("[boot] setup start\n");

  Serial.printf("[boot] PSRAM %s, size=%u bytes\n", psramFound() ? "FOUND" : "MISSING", (unsigned)ESP.getPsramSize());
  esp_rom_printf("[boot] PSRAM %s size=%u\n", psramFound() ? "FOUND" : "MISSING", (unsigned)ESP.getPsramSize());
  Serial.printf("[boot] Pins RGB PCLK=%d HSYNC=%d VSYNC=%d DE=%d | TP SDA=%d SCL=%d | RS485 TX=%d RX=%d\n",
                LCD_PIN_PCLK, LCD_PIN_HSYNC, LCD_PIN_VSYNC, LCD_PIN_DE, TOUCH_I2C_SDA, TOUCH_I2C_SCL,
                RS485_TX_PIN, RS485_RX_PIN);
  if (LCD_BL_PIN < 0) {
    Serial.println("[boot] Backlight control is on CH422G EXIO2 (not yet controlled by this firmware)");
  }
  if (!psramFound()) {
    Serial.println("[boot] ERROR: PSRAM not detected. For ESP32-S3 Dev Module set PSRAM=OPI and Flash=16MB.");
    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
  }
  gState.begin();
  gBus.begin(64);

  lv_init();

  drivers::DisplayConfig dcfg;
  dcfg.hres = LCD_HRES;
  dcfg.vres = LCD_VRES;
  dcfg.bufLines = LVGL_BUF_LINES;
  dcfg.backlightPin = LCD_BL_PIN;
  dcfg.backlightActiveHigh = LCD_BL_ACTIVE_HIGH;
  if (!gDisplay.begin(dcfg)) {
    Serial.println("[boot] Display init failed. Halting.");
    esp_rom_printf("[boot] Display init failed. Halting.\n");
    while (true) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  if (!gTouch.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL, TOUCH_I2C_FREQ)) {
    Serial.println("[boot] Touch init failed");
  }
  gTouch.registerLvglIndev();

  gRs485.begin(makeRs485Config());
  gLogic.begin(&gState, &gBus, &gRs485);

  xTaskCreatePinnedToCore(ui_task, "ui_task", 8192, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(logic_task, "logic_task", 6144, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(comm_task, "comm_task", 4096, nullptr, 1, nullptr, 0);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}


// Arduino IDE does not always compile nested subfolders as separate translation units.
// Force-link project modules by including implementation units here.
#include "core/state.cpp"
#include "core/event_bus.cpp"
#include "core/logic.cpp"

#include "comm/protocol.cpp"
#include "comm/rs485_transport.cpp"

#include "drivers/display_driver.cpp"
#include "drivers/touch_driver.cpp"

#include "ui/ui_app.cpp"
#include "ui/screens/dashboard.cpp"
#include "ui/screens/manual.cpp"
#include "ui/screens/schedules.cpp"
#include "ui/screens/modules.cpp"
#include "ui/screens/settings.cpp"
#include "ui/screens/diagnostics.cpp"
