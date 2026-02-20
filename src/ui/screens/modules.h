#pragma once

#include <lvgl.h>

#include "core/event_bus.h"
#include "core/state.h"

namespace ui {

class ModulesScreen {
 public:
  lv_obj_t* create(lv_obj_t* parent, core::EventBus* bus);
  void refresh(const core::SystemState& state);

 private:
  static void rescanCb(lv_event_t* e);
  lv_obj_t* root_ = nullptr;
  lv_obj_t* list_ = nullptr;
  core::EventBus* bus_ = nullptr;
};

}  // namespace ui
