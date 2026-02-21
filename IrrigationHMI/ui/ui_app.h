#pragma once

#include <lvgl.h>
#include "../core/state.h"
#include "../core/event_bus.h"

namespace ui {

struct UiContext {
    app::SystemState *state = nullptr;
    EventBus *bus = nullptr;
};

void ui_init(UiContext *ctx);
void ui_refresh();

} // namespace ui
