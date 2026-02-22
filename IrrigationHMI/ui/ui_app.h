#pragma once

#include <lvgl.h>
#include "../core/state.h"
#include "../core/event_bus.h"

namespace ui {

struct UiContext {
    app::SystemState *state = nullptr;
    EventBus *bus = nullptr;
    SemaphoreHandle_t stateMutex = nullptr;
};

const char *tr(app::Lang lang, const char *key);
void ui_init(UiContext *ctx);
void ui_refresh();
void ui_toast(const char *msg);

// Глобальная всплывающая клавиатура для всех вкладок.
void ui_attach_keyboard(lv_obj_t *textarea);
void ui_keyboard_hide();

} // namespace ui
