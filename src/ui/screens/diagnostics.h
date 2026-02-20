#pragma once

#include <lvgl.h>

#include "core/event_bus.h"
#include "core/state.h"

namespace ui {

class DiagnosticsScreen {
 public:
  lv_obj_t* create(lv_obj_t* parent, core::EventBus* bus);
  void refresh(const core::SystemState& state);

 private:
  static void clearCb(lv_event_t* e);

  lv_obj_t* root_ = nullptr;
  lv_obj_t* logList_ = nullptr;
  lv_obj_t* counters_ = nullptr;
  core::EventBus* bus_ = nullptr;
};

}  // namespace ui
