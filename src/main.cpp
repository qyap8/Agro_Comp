/*
Board notes (Arduino IDE):
- Board: ESP32S3 Dev Module (or Waveshare ESP32-S3 7" variant if available)
- ESP32 core: 2.0.14+
- PSRAM: Enabled (OPI PSRAM)
- Flash: 8MB (or board default)
- LVGL: v8.x
- Mock mode: core::SystemState.settings.mockMode (default true) and Rs485Config.mockMode
- RS-485 pins/UART/DE: see makeRs485Config() below
*/

#include <Arduino.h>
#include <lvgl.h>

#include "comm/rs485_transport.h"
#include "core/event_bus.h"
#include "core/logic.h"
#include "core/state.h"
#include "drivers/display_driver.h"
#include "drivers/touch_driver.h"
#include "ui/ui_app.h"

static core::SystemState gState;
static core::EventBus gBus;
static core::Logic gLogic;
static comm::Rs485Transport gRs485;
static ui::UiApp gUi;

static comm::Rs485Config makeRs485Config() {
  comm::Rs485Config cfg;
  cfg.uartNum = 2;
  cfg.txPin = 43;   // TODO: set to board UART2 TX pin connected to RS485 transceiver
  cfg.rxPin = 44;   // TODO: set to board UART2 RX pin connected to RS485 transceiver
  cfg.dePin = -1;   // TODO: set DE/RE pin if manual direction is required
  cfg.baud = 115200;
  cfg.mockMode = gState.settings.mockMode;
  return cfg;
}

void ui_task(void*) {
  gUi.begin(&gState, &gBus);
  uint32_t lastRefresh = 0;

  while (true) {
    lv_tick_inc(5);
    lv_timer_handler();
    if (millis() - lastRefresh >= 200) {
      gUi.refresh();
      lastRefresh = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void logic_task(void*) {
  core::Event ev;
  while (true) {
    while (gBus.consume(ev, 0)) {
      gLogic.processEvent(ev);
    }
    gLogic.tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void comm_task(void*) {
  while (true) {
    gRs485.loop();
    static uint32_t lastPoll = 0;
    if (millis() - lastPoll > 1000) {
      core::Event ev;
      ev.type = core::EventType::STATUS_POLL;
      gBus.publish(ev);
      lastPoll = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);

  gState.begin();
  gBus.begin(64);

  drivers::DisplayConfig dcfg;
  dcfg.bufferLines = 60;
  drivers::gDisplay.begin(dcfg);

  drivers::gTouch.begin();
  drivers::gTouch.registerLvglIndev();

  gRs485.begin(makeRs485Config());
  gLogic.begin(&gState, &gBus, &gRs485);

  xTaskCreatePinnedToCore(ui_task, "ui", 8192, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(logic_task, "logic", 6144, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(comm_task, "comm", 4096, nullptr, 1, nullptr, 0);
}

void loop() { vTaskDelay(portMAX_DELAY); }
