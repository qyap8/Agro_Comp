/*
  IrrigationHMI.ino

  Required board settings (Arduino IDE):
  - Board: ESP32S3 Dev Module (or Waveshare ESP32-S3-Touch-LCD-7 profile if available)
  - ESP32 Arduino core: 2.0.14+
  - PSRAM: Enabled (OPI PSRAM)
  - Flash size: 8MB (or board default)

  Library requirements:
  - lvgl v8.x
  - ESP32 Arduino core with esp_lcd support

  RS485 termination:
  - Enable the 120-ohm termination jumper ONLY if this device is at one end of the RS485 bus.
  - Keep termination OFF for mid-bus nodes.
*/

#include <Arduino.h>
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
  delay(50);

  gState.begin();
  gBus.begin(64);

  lv_init();

  drivers::DisplayConfig dcfg;
  dcfg.hres = LCD_HRES;
  dcfg.vres = LCD_VRES;
  dcfg.bufLines = LVGL_BUF_LINES;
  dcfg.backlightPin = LCD_BL_PIN;
  dcfg.backlightActiveHigh = LCD_BL_ACTIVE_HIGH;
  gDisplay.begin(dcfg);

  gTouch.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL, TOUCH_I2C_FREQ);
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
