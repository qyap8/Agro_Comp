#include <Arduino.h>
#include <lvgl.h>
#include <vector>
#include "esp_display_panel.hpp"
#include "lvgl_port_compat.h"

#include "config.h"
#include "ui/ui_app.h"
#include "core/state.h"
#include "core/logic.h"
#include "core/event_bus.h"
#include "comm/rs485_transport.h"

using namespace esp_panel::board;

Board *board = new Board();

static app::SystemState g_state;
static EventBus g_bus;
static RS485Transport g_transport;
static app::LogicController g_logic;
static ui::UiContext g_uiCtx;

static void logic_task(void *arg) {
    (void)arg;
    AppEvent ev{};
    for (;;) {
        // Логическая задача: принимает события UI и обновляет состояние контроллера.
        while (g_bus.consume(ev, 0)) {
            g_logic.handleEvent(ev);
        }
        g_logic.tick();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void comm_task(void *arg) {
    (void)arg;
    std::vector<uint8_t> rx;
    for (;;) {
        // Коммуникационная задача: неблокирующий опрос RS485.
        g_transport.readFrame(rx, 20);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void setup() {
    Serial.begin(115200);

    // Инициализация дисплейной платы строго через ESP32_Display_Panel.
    board->init();
    board->begin();
    lvgl_port_init(board->getLCD(), board->getTouch());

    g_bus.begin();
    g_transport.begin(APP_RS485_BAUD_DEFAULT);
    g_logic.begin(&g_state, &g_bus, &g_transport);

    g_uiCtx.state = &g_state;
    g_uiCtx.bus = &g_bus;
    g_uiCtx.stateMutex = g_logic.stateMutex();

    // Любые изменения LVGL делаем под lock/unlock.
    lvgl_port_lock(0);
    ui::ui_init(&g_uiCtx);
    lvgl_port_unlock();

    xTaskCreatePinnedToCore(logic_task, "logic_task", APP_LOGIC_TASK_STACK, nullptr, APP_LOGIC_TASK_PRIO, nullptr, 1);
    xTaskCreatePinnedToCore(comm_task, "comm_task", APP_COMM_TASK_STACK, nullptr, APP_COMM_TASK_PRIO, nullptr, 1);
}

void loop() {
    static uint32_t lastRefresh = 0;
    if (millis() - lastRefresh > 250) {
        lastRefresh = millis();
        // Периодическое обновление виджетов из актуального SystemState.
        lvgl_port_lock(0);
        ui::ui_refresh();
        lvgl_port_unlock();
    }
    delay(5);
}
